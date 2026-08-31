/*
** music_coremidi_mididevice.mm
** Provides access to CoreMIDI on macOS for hardware MIDI playback
**
**---------------------------------------------------------------------------
** Copyright 2025 GZDoom Maintainers and Contributors
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include <CoreMIDI/CoreMIDI.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach_time.h>
#include <CoreAudio/HostTime.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "mididevice.h"
#include "zmusic/mididefs.h"
#include "zmusic/mus2midi.h"
#include "zmusic/zmusic_internal.h"

//==========================================================================
//
// CoreMIDIDevice - CoreMIDI implementation for macOS
//
// Based on WinMIDIDevice (Windows) and AlsaMIDIDevice (Linux) patterns
//
//==========================================================================

class CoreMIDIDevice : public MIDIDevice
{
public:
	CoreMIDIDevice(int dev_id, bool precache);
	~CoreMIDIDevice();

	int Open() override;
	void Close() override;
	bool IsOpen() const override;
	int GetTechnology() const override;
	int SetTempo(int tempo) override;
	int SetTimeDiv(int timediv) override;
	int StreamOut(MidiHeader* data) override;
	int StreamOutSync(MidiHeader* data) override;
	int Resume() override;
	void Stop() override;
	bool FakeVolume() override;
	bool Pause(bool paused) override;
	void InitPlayback() override;
	void PrecacheInstruments(const uint16_t* instruments, int count) override;

protected:
	bool Precache;

	bool PullEvent();
	void PlayerLoop();

	// Event handling
	void PrepareTempo(uint32_t tempo);
	void PrepareShortMsg(uint32_t msg);
	void PrepareLongMsg(const uint8_t* long_msg, uint32_t length);
	void HandleEvent(const uint32_t* data, ByteCount word_count, MIDITimeStamp timestamp);
	void SendImmediateShortMsg(uint32_t command, uint32_t data1 = 0, uint32_t data2 = 0);

	// PulledEvent structure to hold the next event to be processed
	enum EventType { EVENT_TEMPO, EVENT_MESSAGE, EVENT_NOP };
	struct
	{
		union
		{
			uint32_t tempo;
			uint32_t msg_buffer[64];
		} data;
		EventType type;
		ByteCount word_count;
		uint32_t tick_delta;
	} PulledEvent;

	// CoreMIDI handles
	inline static MIDIClientRef MidiClient = 0;
	inline static MIDIPortRef MidiOutPort = 0;
	MIDIEndpointRef MidiDestination;
	int DeviceID;

	// Threading
	std::thread PlayerThread;
	std::atomic<bool> Exit;
	std::mutex Mutex;
	std::condition_variable ExitCond;

	// Timing
	int64_t InitialTempo;
	int64_t Tempo;
	int64_t Division;

	// ZMusic MidiHeader data
	MidiHeader* Events; // Linked list of MIDI headers akin to win32 MIDIHDR
	uint32_t Position; // Current position in the MidiHeader buffer
	uint32_t PositionOffset;
};

//==========================================================================
//
// CoreMIDIDevice :: Constructor
//
//==========================================================================

CoreMIDIDevice::CoreMIDIDevice(int dev_id, bool precache)
	: DeviceID{dev_id}
	, MidiDestination{0}
	, InitialTempo{500000}      // Default: 120 BPM (500,000 µs per quarter note)
	, Division{100}       // Default PPQN
	, Events{nullptr}
	, Position{0}
	, Precache{precache}
{
}

//==========================================================================
//
// CoreMIDIDevice :: Destructor
//
//==========================================================================

CoreMIDIDevice::~CoreMIDIDevice()
{
	Close();
}

//==========================================================================
//
// CoreMIDIDevice :: Open
//
// Opens the MIDI device and connects to the specified endpoint
//
//==========================================================================

int CoreMIDIDevice::Open()
{
	if (MidiDestination)
		return 0;

	OSStatus status;

	if (!MidiClient)
	{
		// Create MIDI client
		status = MIDIClientCreate(CFSTR("ZMusic"), nullptr, nullptr, &MidiClient);
		if (status != noErr)
		{
			ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Failed to create MIDI client (error %d)\n", (int)status);
			return -1;
		}
	}

	if (!MidiOutPort)
	{
		// Create output port
		status = MIDIOutputPortCreate(MidiClient, CFSTR("ZMusic Program Music"), &MidiOutPort);
		if (status != noErr)
		{
			ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Failed to create output port (error %d)\n", (int)status);
			return -1;
		}
	}

	// Get destination endpoint by device ID
	ItemCount midiout_device_count = MIDIGetNumberOfDestinations();
	if (DeviceID < 0 || DeviceID >= (int)midiout_device_count)
	{
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Invalid device ID %d (available: %d)\n", DeviceID, (int)midiout_device_count);
		return -1;
	}

	MidiDestination = MIDIGetDestination(DeviceID);
	if (!MidiDestination)
	{
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Failed to get destination for device %d\n", DeviceID);
		return -1;
	}

	return 0;
}

//==========================================================================
//
// CoreMIDIDevice :: Close
//
//==========================================================================

void CoreMIDIDevice::Close()
{
	if (!MidiDestination)
	{
		return;
	}
	// Stop player thread
	Stop();
	MidiDestination = 0;
}

//==========================================================================
//
// CoreMIDIDevice :: IsOpen
//
//==========================================================================

bool CoreMIDIDevice::IsOpen() const
{
	return MidiDestination;
}

//==========================================================================
//
// CoreMIDIDevice :: GetTechnology
//
//==========================================================================

int CoreMIDIDevice::GetTechnology() const
{
	// Query if device is offline/virtual
	if (MidiDestination != 0)
	{
		SInt32 offline = 0;
		MIDIObjectGetIntegerProperty(MidiDestination, kMIDIPropertyOffline, &offline);
		return offline ? MIDIDEV_SWSYNTH : MIDIDEV_MIDIPORT;
	}
	return MIDIDEV_MIDIPORT;
}

//==========================================================================
//
// CoreMIDIDevice :: FakeVolume
//
// CoreMIDI doesn't support volume control directly
//
//==========================================================================

bool CoreMIDIDevice::FakeVolume()
{
	return true;  // No true volume control support, so fake volume
}

//==========================================================================
//
// CoreMIDIDevice :: SetTempo
//
// Sets the playback tempo (microseconds per quarter note)
//
//==========================================================================

int CoreMIDIDevice::SetTempo(int tempo)
{
	InitialTempo = tempo;
	return 0;
}

//==========================================================================
//
// CoreMIDIDevice :: SetTimeDiv
//
// Sets the time division (PPQN - pulses per quarter note)
//
//==========================================================================

int CoreMIDIDevice::SetTimeDiv(int timediv)
{
	Division = timediv;
	return 0;
}

//==========================================================================
//
// CoreMIDIDevice :: PrecacheInstruments
//
// This is meant to mirror WinMIDIDevice::PrecacheInstruments
//
//==========================================================================

void CoreMIDIDevice::PrecacheInstruments(const uint16_t* instruments, int count)
{
	// Setting snd_midiprecache to false disables this precaching, since it
	// does involve sleeping for more than a miniscule amount of time.
	if (!Precache)
	{
		return;
	}
	uint8_t bank[16] = {};
	uint8_t i, chan;

	for (i = 0, chan = 0; i < count; ++i)
	{
		uint8_t instr = instruments[i] & 127;
		uint8_t banknum = (instruments[i] >> 7) & 127;
		uint8_t percussion = instruments[i] >> 14;

		if (percussion)
		{
			if (bank[9] != banknum)
			{
				SendImmediateShortMsg(MIDI_CTRLCHANGE | 9, 0, banknum);
				bank[9] = banknum;
			}
			SendImmediateShortMsg(MIDI_NOTEON | 9, instr, 1);
		}
		else
		{ // Melodic
			if (bank[chan] != banknum)
			{
				SendImmediateShortMsg(MIDI_CTRLCHANGE | 9, 0, banknum);
				bank[chan] = banknum;
			}
			SendImmediateShortMsg(MIDI_PRGMCHANGE | chan, instruments[i]);
			SendImmediateShortMsg(MIDI_NOTEON | chan, 60, 1);
			if (++chan == 9)
			{ // Skip the percussion channel
				chan = 10;
			}
		}
		// Once we've got an instrument playing on each melodic channel, sleep to give
		// the driver time to load the instruments. Also do this for the final batch
		// of instruments.
		if (chan == 16 || i == count - 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			for (chan = 15; chan-- != 0; )
			{
				// Turn all notes off
				SendImmediateShortMsg(MIDI_CTRLCHANGE | chan, 123, 0);
			}
			// And now chan is back at 0, ready to start the cycle over.
		}
	}
	// Make sure all channels are set back to bank 0.
	for (i = 0; i < 16; ++i)
	{
		if (bank[i] != 0)
		{
			SendImmediateShortMsg(MIDI_CTRLCHANGE | 9, 0, 0);
		}
	}
}

//==========================================================================
//
// CoreMIDIDevice :: InitPlayback
//
// Initialize playback state
//
//==========================================================================

void CoreMIDIDevice::InitPlayback()
{
	Exit.store(false, std::memory_order_relaxed);
}

//==========================================================================
//
// CoreMIDIDevice :: Resume
//
// Start or resume playback
//
//==========================================================================

int CoreMIDIDevice::Resume()
{
	if (!MidiDestination || PlayerThread.joinable())
	{
		return -1;
	}
	Exit.store(false, std::memory_order_relaxed);
	PlayerThread = std::thread(&CoreMIDIDevice::PlayerLoop, this);
	return 0;
}

//==========================================================================
//
// CoreMIDIDevice :: Stop
//
// Stop playback
//
//==========================================================================

void CoreMIDIDevice::Stop()
{
	Exit.store(true, std::memory_order_relaxed);
	ExitCond.notify_all();
	if (PlayerThread.joinable())
	{
		PlayerThread.join();
	}
	MIDIFlushOutput(MidiDestination); // Drop pending events.

	// Reset all channels to prevent hanging notes
	for (int channel = 0; channel < 16; ++channel)
	{
		SendImmediateShortMsg(MIDI_CTRLCHANGE | channel, 123, 0); // All Notes Off
		SendImmediateShortMsg(MIDI_CTRLCHANGE | channel, 121, 0);  // Reset All Controllers
	}
}

//==========================================================================
//
// CoreMIDIDevice :: Pause
//
// We cannot pause so just always return false
//
//==========================================================================

bool CoreMIDIDevice::Pause(bool paused)
{
	return false;
}

//==========================================================================
//
// CoreMIDIDevice :: StreamOut
//
// Gets new midi buffers
//
//==========================================================================

int CoreMIDIDevice::StreamOut(MidiHeader* header)
{
	header->lpNext = nullptr;
	if (Events == nullptr)
	{
		Events = header;
		Position = 0;
	}
	else
	{
		MidiHeader** p;
		for (p = &Events; *p != nullptr; p = &(*p)->lpNext)
		{ }
		*p = header;
	}
	return 0;
}

//==========================================================================
//
// CoreMIDIDevice :: StreamOutSync
//
//==========================================================================

int CoreMIDIDevice::StreamOutSync(MidiHeader* header)
{
	return StreamOut(header);
}

//==========================================================================
//
// CoreMIDIDevice :: PullEvent
//
// Pulls next event from MidiHeader buffer
//
//==========================================================================

bool CoreMIDIDevice::PullEvent()
{
	if (!Events && Callback)
	{	// No events in the current MidiHeader, request next buffer
		Callback(CallbackData);
	}

	if (!Events)
	{	// No events available to process.
		return false;
	}

	if (Position >= Events->dwBytesRecorded)
	{	// All events in the buffer were used, point to next buffer
		Events = Events->lpNext;
		Position = 0;
		if (Callback)
		{	// This ensures that we always have the maximum number of unused buffers (most likely 2) after 1 is used up.
			// omit this nested "if" block if you want to use up all buffers before requesting new buffers
			Callback(CallbackData);
		}
	}

	if (!Events)
	{	// No events in the new buffer
		return false;
	}

	const uint32_t* event = (uint32_t*)(Events->lpData + Position);
	PulledEvent.tick_delta = event[0]; // First 4 bytes of event

	// Get event size to advance Position
	if (event[2] < 0x80000000) // Short message (event[2] is the combined status/data bytes)
	{
		PositionOffset = 12; // 4 bytes delta time, 4 bytes reserved, 4 bytes MIDI message (up to 3 bytes + padding)
	}
	else // Long message or meta-event (event[2] holds type and parameter length)
	{
		PositionOffset = 12 + ((MEVENT_EVENTPARM(event[2]) + 3) & ~3);
	}

	// Pulling event out of buffer
	switch (MEVENT_EVENTTYPE(event[2]))
	{
	case MEVENT_TEMPO:
		// Tempo change event, update our internal calculation for future events
		PrepareTempo(MEVENT_EVENTPARM(event[2]));
		break;
	case MEVENT_LONGMSG:
		{	// Long MIDI message (SysEx, etc.), data starts after event[3]
			uint32_t long_msg_len = MEVENT_EVENTPARM(event[2]);
			const uint8_t* long_msg_data = (uint8_t*)&event[3];
			// Ensure valid sysex message
			if (long_msg_len > 2 && long_msg_data[0] == 0xF0 && long_msg_data[long_msg_len - 1] == 0xF7)
			{	// Strip sysex start (0xF0) and end (0xF7) bytes
				PrepareLongMsg(long_msg_data + 1, long_msg_len - 2);
			}
			else
			{
				PulledEvent.type = EVENT_NOP;
			}
			break;
		}
	case MEVENT_SHORTMSG:
		{	// MIDI 1.0 voice msg type (0x2) | Group (0x0) | the remaining 24 bits are raw MIDI 1.0 bytes
			PrepareShortMsg(0x20 << 24 | CFSwapInt32(event[2]) >> 8);
			break;
		}
	default:
		PulledEvent.type = EVENT_NOP;
	}

	// Indicate that an event was processed.
	return true;
}

//==========================================================================
//
// CoreMIDIDevice :: PlayerLoop
//
// Main player thread loop
//
//==========================================================================

void CoreMIDIDevice::PlayerLoop()
{
	std::unique_lock<std::mutex> lock{Mutex};
	using namespace std::literals::chrono_literals;
	constexpr std::chrono::nanoseconds buffer_step = 40ms;

	Tempo = InitialTempo;
	// Initialize midi clock with current host time, CoreAudio and CoreMidi work in nano seconds.
	std::chrono::nanoseconds buffer_timestamp{AudioConvertHostTimeToNanos(AudioGetCurrentHostTime())};

	// Process all available events and schedule them with CoreMIDI
	while (!Exit.load(std::memory_order_relaxed))
	{
		if (!PullEvent())
		{
			ExitCond.wait_for(lock, buffer_step);
			continue;
		}

		// Multiply by 1000 to convert to nanoseconds, multiplication is done before any division to be accurate to the nanosecond.
		std::chrono::nanoseconds pulled_ev_time_delta{1000 * PulledEvent.tick_delta * Tempo / Division};
		auto pulled_ev_timestamp = buffer_timestamp + pulled_ev_time_delta;
		std::chrono::nanoseconds current_timestamp{AudioConvertHostTimeToNanos(AudioGetCurrentHostTime())};
		auto time_until_pulled_ev = pulled_ev_timestamp - current_timestamp;
		auto schedule_time = time_until_pulled_ev - buffer_step;
		if (schedule_time >= buffer_step)
		{	// Try to keep buffered events under 2x buffer_step
			if (ExitCond.wait_for(lock, schedule_time) == std::cv_status::no_timeout)
			{
				continue;
			}
		}
		if (time_until_pulled_ev < std::chrono::nanoseconds::zero())
		{	// Can be triggered on playback start.
			// Message shouldn't be shown by default like other midi backends here.
			ZMusic_Printf(ZMUSIC_MSG_DEBUG, "CoreMidi backend underrun by %d nanoseconds!\n", time_until_pulled_ev.count());
		}

		// Handle PulledEvent
		switch (PulledEvent.type)
		{
		case EVENT_TEMPO:
			Tempo = PulledEvent.data.tempo;
			break;
		case EVENT_MESSAGE:
			HandleEvent(PulledEvent.data.msg_buffer, PulledEvent.word_count, AudioConvertNanosToHostTime(pulled_ev_timestamp.count()));
			break;
		case EVENT_NOP:
		default:
			;
		}
		buffer_timestamp = pulled_ev_timestamp;
		Position += PositionOffset;
	}
}

//==========================================================================
//
// CoreMIDIDevice :: PrepareTempo and PrepareShortMsg
//
// Prepare pulled event to be handled later
//
//==========================================================================

void CoreMIDIDevice::PrepareTempo(const uint32_t tempo)
{
	PulledEvent.type = EVENT_TEMPO;
	PulledEvent.data.tempo = tempo;
}
void CoreMIDIDevice::PrepareShortMsg(uint32_t msg)
{
	PulledEvent.type = EVENT_MESSAGE;
	PulledEvent.data.msg_buffer[0] = msg;
	PulledEvent.word_count = 1;
}

//==========================================================================
//
// CoreMIDIDevice :: PrepareLongMsg
//
// Prepares MIDI sysex messages by packing them into UMPs (Universal Midi Packets)
// sysex UMPs must always come in pairs of 32-bit structures called UMPs
// the first 2 bytes of the first UMP contain metadata to identify the UMP
// the last 2 bytes and the entirety of the second UMP (4 bytes) 2 + 4 = 6 bytes
// are for the raw sysex message each UMP packed in the native endianness of the machine
//
//==========================================================================

void CoreMIDIDevice::PrepareLongMsg(const uint8_t* long_msg, uint32_t length)
{
	uint ump_count = (length / 6) * 2;
	if (length % 6)
	{
		ump_count += 2;
	}
	if (ump_count > 64)
	{	// Max capacity of 1 MIDIEventPacket is 64 32-bit words, thus max size sysex is 64 / 2 x 6 = 192 bytes
		// for larger messages a bigger buffer could be allocated and type punned to MIDIEventList for up to 65,536 bytes
		// and for even larger sysex messages we could split it over successive invocations of MIDIEventListAdd.
		// Nonetheless since sysex messages here typically do not get past 11 bytes, I think neither solution is worth implementing.
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMidi: message needs %u UMPs, exceeding MIDIEventPacket capacity: 64\n", ump_count);
		PulledEvent.type = EVENT_NOP;
		return;
	}
	auto remaining_bytes = length;
	auto index_ptr = long_msg;
	size_t msg_buffer_index = 0;
	while (remaining_bytes > 0)
	{
		// Determine how many bytes go into this UMP pair (up to 6 bytes)
		// Note: keep it uint32_t because it will be bitshifted and or'ed to construct the UMP
		uint32_t chunk_size = std::min<uint32_t>(6, remaining_bytes);
	
		// Determine UMP Status (third 4 bits)
		uint32_t status;
		if (length <= 6)
		{
			status = 0x0; // Complete System Exclusive Message fits in one UMP pair
		}
		else if (index_ptr == long_msg)
		{
			status = 0x1; // Start UMP
		}
		else if (remaining_bytes > 6)
		{
			status = 0x2; // Continue UMP
		}
		else
		{
			status = 0x3; // End UMP
		}
	
		// Initialize buffer with 0s
		uint32_t buffer[6] = {};
		// Copy bytes from index_ptr and expand them to 32 bits for bitshifting and or'ing later
		// when chunk_size is < 6 the extra bytes are left as 0s, this padding is part of the spec; the second UMP is needed even if it's all 0s
		for (uint32_t i = 0; i < chunk_size; ++i)
		{
			buffer[i] = index_ptr[i];
		}

		// (sysex msg type (0x3) | Group (0x0)) = 8 bits | Status = 4 bits | # of bytes = 4 bits | first 2 bytes from buffer
		const uint32_t ump_1 = 0x30 << 24 | status << 20 | chunk_size << 16 | buffer[0] << 8 | buffer[1];

		// last 4 bytes from buffer
		const uint32_t ump_2 = buffer[2] << 24 | buffer[3] << 16 | buffer[4] << 8 | buffer[5];
	
		PulledEvent.data.msg_buffer[msg_buffer_index] = ump_1;
		++msg_buffer_index;
		PulledEvent.data.msg_buffer[msg_buffer_index] = ump_2;
		++msg_buffer_index;

		remaining_bytes -= chunk_size;
		index_ptr += chunk_size;
	}

	PulledEvent.type = EVENT_MESSAGE;
	PulledEvent.word_count = ump_count;
}

//==========================================================================
//
// CoreMIDIDevice :: HandleEvent
//
// Schedules MIDI events to be sent to the output port
//
//==========================================================================

void CoreMIDIDevice::HandleEvent(const uint32_t* data, ByteCount word_count, MIDITimeStamp timestamp)
{
	MIDIEventList event_list = {};
	MIDIEventPacket* event_packet = MIDIEventListInit(&event_list, kMIDIProtocol_1_0);

	// Add the event to the event list.
	event_packet = MIDIEventListAdd(&event_list, sizeof(MIDIEventList::packet), event_packet, timestamp, word_count, data);

	if (event_packet != nullptr)
	{
		OSStatus status = MIDISendEventList(MidiOutPort, MidiDestination, &event_list);
		if (status != noErr)
		{
			ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: \"MIDISendEventList\" failed with error: %d\n", (int)status);
		}
	}
	else
	{
		// Should never happen as long as (word_count <= 64)
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: \"MIDIEventListAdd\" failed unexpectedly.\n");
	}
}

//==========================================================================
//
// CoreMIDIDevice :: SendImmediateShortMsg
//
// For use with PrecacheInstruments and Stop messages.
//
//==========================================================================

void CoreMIDIDevice::SendImmediateShortMsg(uint32_t command, uint32_t data1, uint32_t data2)
{
	const uint32_t msg = 0x20 << 24 | command << 16 | data1 << 8 | data2;
	HandleEvent(&msg, 1, 0);
}

//==========================================================================
//
// CreateCoreMIDIDevice
//
// Factory function to create a CoreMIDI device instance
//
//==========================================================================

MIDIDevice* CreateCoreMIDIDevice(int mididevice)
{
	return new CoreMIDIDevice(mididevice, miscConfig.snd_midiprecache);
}
