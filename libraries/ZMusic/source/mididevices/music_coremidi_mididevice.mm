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

#include <array>
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
	CoreMIDIDevice(int deviceID, bool precache);
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
	bool Pause(bool paused) override;
	bool FakeVolume() override;
	void InitPlayback() override;
	void PrecacheInstruments(const uint16_t* instruments, int count) override;

protected:
	bool PullEvent();

	// CoreMIDI handles
	MIDIClientRef midiClient;
	MIDIPortRef midiOutPort;
	MIDIEndpointRef midiDestination;
	int deviceID;

	// Event handling
	enum EventType { TempoEv, MidiMsgEv, NoEvent };
	union EventMsg
	{
		uint32_t tempo;
		uint8_t* data;
	};
	struct CurrentEvent
	{
		EventType EventType;
		EventMsg EventMsg;
		uint32_t length;
	};
	CurrentEvent CurrentEvent;
	std::array<uint8_t, 3> ShortMsgBuffer;
	void PrepareTempo(uint32_t tempo);
	void PrepareMidiMsg(uint8_t* msg, uint32_t length);
	void HandleCurrentEvent();
	void SendMIDIData(const uint8_t* data, size_t length, MIDITimeStamp timestamp);

	// Threading
	std::thread PlayerThread;
	volatile bool Exit;
	std::condition_variable ExitCond;
	std::mutex Mutex;

	bool isOpen;
	bool Precache;

	// Timing
	int Tempo;
	int InitialTempo;
	int Division;
	MidiHeader* Events; // Linked list of MIDI headers
	uint32_t Position; // Current position in the MidiHeader buffer
	uint32_t PositionOffset;
	uint32_t CurrentEventTickDelta;

	// Thread functions
	static void PlayerThreadProc(CoreMIDIDevice* device);
	void PlayerLoop();
};

//==========================================================================
//
// CoreMIDIDevice :: Constructor
//
//==========================================================================

CoreMIDIDevice::CoreMIDIDevice(int deviceID, bool precache)
	: deviceID(deviceID)
	, midiClient(0)
	, midiOutPort(0)
	, midiDestination(0)
	, isOpen(false)
	, InitialTempo(500000)      // Default: 120 BPM (500,000 µs per quarter note)
	, Division(100)       // Default PPQN
	, Events(nullptr)
	, Position(0)
	, Precache(precache)
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
	if (isOpen)
		return 0;

	OSStatus status;

	// Create MIDI client
	status = MIDIClientCreate(CFSTR("ZMusic"), nullptr, nullptr, &midiClient);
	if (status != noErr)
	{
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Failed to create MIDI client (error %d)\n", (int)status);
		return -1;
	}

	// Create output port
	status = MIDIOutputPortCreate(midiClient, CFSTR("ZMusic Program Music"), &midiOutPort);
	if (status != noErr)
	{
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Failed to create output port (error %d)\n", (int)status);
		MIDIClientDispose(midiClient);
		midiClient = 0;
		return -1;
	}

	// Get destination endpoint by device ID
	ItemCount destCount = MIDIGetNumberOfDestinations();
	if (deviceID < 0 || deviceID >= (int)destCount)
	{
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Invalid device ID %d (available: %d)\n", deviceID, (int)destCount);
		MIDIPortDispose(midiOutPort);
		MIDIClientDispose(midiClient);
		midiOutPort = 0;
		midiClient = 0;
		return -1;
	}

	midiDestination = MIDIGetDestination(deviceID);
	if (midiDestination == 0)
	{
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Failed to get destination for device %d\n", deviceID);
		MIDIPortDispose(midiOutPort);
		MIDIClientDispose(midiClient);
		midiOutPort = 0;
		midiClient = 0;
		return -1;
	}

	isOpen = true;
	return 0;
}

//==========================================================================
//
// CoreMIDIDevice :: Close
//
//==========================================================================

void CoreMIDIDevice::Close()
{
	if (!isOpen)
		return;

	// Stop player thread
	Stop();

	// Dispose CoreMIDI objects
	if (midiOutPort != 0)
	{
		MIDIPortDispose(midiOutPort);
		midiOutPort = 0;
	}

	if (midiClient != 0)
	{
		MIDIClientDispose(midiClient);
		midiClient = 0;
	}

	midiDestination = 0;
	isOpen = false;
}

//==========================================================================
//
// CoreMIDIDevice :: IsOpen
//
//==========================================================================

bool CoreMIDIDevice::IsOpen() const
{
	return isOpen;
}

//==========================================================================
//
// CoreMIDIDevice :: GetTechnology
//
//==========================================================================

int CoreMIDIDevice::GetTechnology() const
{
	// Query if device is offline/virtual
	if (midiDestination != 0)
	{
		SInt32 offline = 0;
		MIDIObjectGetIntegerProperty(midiDestination, kMIDIPropertyOffline, &offline);
		return offline ? MIDIDEV_SWSYNTH : MIDIDEV_MIDIPORT;
	}
	return MIDIDEV_MIDIPORT;
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
// CoreMIDIDevice :: StreamOut
//
// Queue MIDI data for asynchronous playback
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
// Queue MIDI data for synchronous playback
//
//==========================================================================

int CoreMIDIDevice::StreamOutSync(MidiHeader* header)
{
	return StreamOut(header);
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
	if (!isOpen || PlayerThread.joinable())
	{
		return -1;
	}
	Exit = false;
	PlayerThread = std::thread(PlayerThreadProc, this);
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
	Exit = true;
	ExitCond.notify_all();
	if (PlayerThread.joinable())
	{
		PlayerThread.join();
	}

	MIDIFlushOutput(midiDestination); // Drop pending events.

	// Send All Notes Off and Reset All Controllers
	for (int channel = 0; channel < 16; ++channel)
	{
		ShortMsgBuffer = { (uint8_t)(0xB0 | channel), 123, 0 };
		SendMIDIData(ShortMsgBuffer.data(), 3, 0);  // All Notes Off
		ShortMsgBuffer = { (uint8_t)(0xB0 | channel), 121, 0 };
		SendMIDIData(ShortMsgBuffer.data(), 3, 0);  // Reset All Controllers
	}
}

//==========================================================================
//
// CoreMIDIDevice :: Pause
//
// Pause/resume playback
//
//==========================================================================

bool CoreMIDIDevice::Pause(bool paused)
{
	return false; // We don support pausing
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
// CoreMIDIDevice :: InitPlayback
//
// Initialize playback state
//
//==========================================================================

void CoreMIDIDevice::InitPlayback()
{
	Exit = false;
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
	uint8_t bank[16] = {0};
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
				ShortMsgBuffer = { MIDI_CTRLCHANGE | 9, 0, banknum };
				SendMIDIData(ShortMsgBuffer.data(), 3, 0);
				bank[9] = banknum;
			}
			ShortMsgBuffer = { MIDI_NOTEON | 9, instr, 1 };
			SendMIDIData(ShortMsgBuffer.data(), 3, 0);
		}
		else
		{ // Melodic
			if (bank[chan] != banknum)
			{
				ShortMsgBuffer = { MIDI_CTRLCHANGE | 9, 0, banknum };
				SendMIDIData(ShortMsgBuffer.data(), 3, 0);
				bank[chan] = banknum;
			}
			ShortMsgBuffer = { (uint8_t)(MIDI_PRGMCHANGE | chan), (uint8_t)instruments[i] };
			SendMIDIData(ShortMsgBuffer.data(), 2, 0);
			ShortMsgBuffer = { (uint8_t)(MIDI_NOTEON | chan), 60, 1 };
			SendMIDIData(ShortMsgBuffer.data(), 3, 0);
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
				ShortMsgBuffer = { (uint8_t)(MIDI_CTRLCHANGE | chan), 123, 0 };
				SendMIDIData(ShortMsgBuffer.data(), 3, 0);
			}
			// And now chan is back at 0, ready to start the cycle over.
		}
	}
	// Make sure all channels are set back to bank 0.
	for (i = 0; i < 16; ++i)
	{
		if (bank[i] != 0)
		{
			ShortMsgBuffer = { MIDI_CTRLCHANGE | 9, 0, 0 };
			SendMIDIData(ShortMsgBuffer.data(), 3, 0);
		}
	}
}

//==========================================================================
//
// CoreMIDIDevice :: PlayTick
//
// Plays all events up to the current tick.
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
		{	// This ensures that we always have 2 unused buffers after 1 is used up.
			// omit this nested "if" block if you want to use up the 2 buffers before requesting new buffers
			Callback(CallbackData);
		}
	}

	if (!Events)
	{	// No events in the new buffer
		return false;
	}

	uint32_t* event = (uint32_t*)(Events->lpData + Position);
	CurrentEventTickDelta = event[0]; // First 4 bytes of event

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
		int long_msg_len = MEVENT_EVENTPARM(event[2]);
		uint8_t* long_msg_data = (uint8_t*)&event[3];
		// Ensure valid sysex message
		if (long_msg_len > 2 && long_msg_data[0] == 0xF0 && long_msg_data[long_msg_len - 1] == 0xF7)
		{
			PrepareMidiMsg(long_msg_data, long_msg_len);
			break;
		}
	}
	case 0: // Short MIDI message (note on/off, control change, etc.)
	{
		// event[2] contains the 1, 2, or 3 byte MIDI message
		ShortMsgBuffer = {	(uint8_t)(event[2] & 0xff), // Status
							(uint8_t)((event[2] >> 8) & 0xff), // Data 1
							(uint8_t)((event[2] >> 16) & 0xff) }; // Data 2

		int msgLen = 0;
		if (ShortMsgBuffer[0] >= 0xF0) // System messages
		{
			if (ShortMsgBuffer[0] == 0xF0 || ShortMsgBuffer[0] == 0xF7) msgLen = 1; // Start/Stop/Continue/Timing/Active Sensing/Reset (1 byte)
			else if (ShortMsgBuffer[0] == 0xF1 || ShortMsgBuffer[0] == 0xF3) msgLen = 2; // Time Code Quarter Frame, Song Select (2 bytes)
			else if (ShortMsgBuffer[0] == 0xF2) msgLen = 3; // Song Position Pointer (3 bytes)
			else msgLen = 1; // Default to 1 for other unknown system messages
		}
		else if (ShortMsgBuffer[0] >= 0xC0 && ShortMsgBuffer[0] < 0xE0) // Program Change or Channel Aftertouch (2 bytes)
		{
			msgLen = 2;
		}
		else // Note On/Off, Poly Aftertouch, Control Change, Pitch Bend (3 bytes)
		{
			msgLen = 3;
		}
		PrepareMidiMsg(ShortMsgBuffer.data(), msgLen);
		break;
	}
	default:
		CurrentEvent.EventType = NoEvent;
	}

	// Indicate that an event was processed and potentially more are available in the current tick.
	// The PlayerLoop will decide when to call PlayTick again.
	return true;
}

//==========================================================================
//
// CoreMIDIDevice :: PlayerThreadProc
//
// Static thread entry point
//
//==========================================================================

void CoreMIDIDevice::PlayerThreadProc(CoreMIDIDevice* device)
{
	device->PlayerLoop();
}

//==========================================================================
//
// CoreMIDIDevice :: PlayerLoop
//
// Main player thread loop - processes MIDI events from queue
//
//==========================================================================

void CoreMIDIDevice::PlayerLoop()
{
	std::unique_lock<std::mutex> lock(Mutex);
	std::chrono::nanoseconds buffer_step(40000000);

	Tempo = InitialTempo;
	// Initialize midi clock with current host time
	MIDITimeStamp buffer_timestamp = AudioConvertHostTimeToNanos(AudioGetCurrentHostTime());

	// Process all available events and schedule them with CoreMIDI
	while (!Exit)
	{
		if (!PullEvent())
		{
			ExitCond.wait_for(lock, buffer_step);
			continue;
		}

		// CoreAudio and CoreMidi work in nano seconds so multiply by 1000.
		MIDITimeStamp current_ev_timestamp = buffer_timestamp + CurrentEventTickDelta * Tempo / Division * 1000;

		auto time_until_current_ev = std::chrono::nanoseconds(current_ev_timestamp - AudioConvertHostTimeToNanos(AudioGetCurrentHostTime()));
		auto schedule_time = time_until_current_ev - buffer_step;
		if (schedule_time >= buffer_step)
		{    // Try to keep buffered events under 2x buffer_step
			if (ExitCond.wait_for(lock, schedule_time) == std::cv_status::no_timeout)
			{
				continue;
			}
		}
		if (time_until_current_ev < std::chrono::nanoseconds::zero())
		{	// Can be triggered on playback start.
			// Message shouldn't be shown by default like other midi backends here.
			ZMusic_Printf(ZMUSIC_MSG_NOTIFY, "CoreMidi backend underrun by %d nanoseconds!\n", time_until_current_ev.count());
		}

		// Handle CurrentEvent
		switch (CurrentEvent.EventType)
		{
		case TempoEv:
			Tempo = CurrentEvent.EventMsg.tempo;
			break;
		case MidiMsgEv:
			SendMIDIData(CurrentEvent.EventMsg.data, CurrentEvent.length, AudioConvertNanosToHostTime(current_ev_timestamp));
			break;
		case NoEvent:
		default:
			;
		}
		buffer_timestamp = current_ev_timestamp;
		Position += PositionOffset;
	}
}

//==========================================================================
//
// CoreMIDIDevice :: PrepareTempo and PrepareMidiMsg
//
// Prepare pulled event to be handled later
//
//==========================================================================

void CoreMIDIDevice::PrepareTempo(const uint32_t tempo)
{
	CurrentEvent.EventType = TempoEv;
	CurrentEvent.EventMsg.tempo = tempo;
}
void CoreMIDIDevice::PrepareMidiMsg(uint8_t* msg, uint32_t length)
{
	CurrentEvent.EventType = MidiMsgEv;
	CurrentEvent.EventMsg.data = msg;
	CurrentEvent.length = length;
}

//==========================================================================
//
// CoreMIDIDevice :: SendMIDIData
//
// Send raw MIDI data to the CoreMIDI output port
//
//==========================================================================

void CoreMIDIDevice::SendMIDIData(const uint8_t* data, size_t length, MIDITimeStamp timestamp)
{
	if (!isOpen || midiOutPort == 0 || midiDestination == 0)
		return;

	// The required size for the MIDIPacketList is the size of the list itself
	// plus the size of the packet header and the actual MIDI data.
	size_t requiredSize = offsetof(MIDIPacketList, packet) + offsetof(MIDIPacket, data) + length;

	// Use a stack buffer for small messages to avoid heap allocation (fast path).
	Byte small_buffer[256];

	// Choose the buffer to use.
	Byte* buffer;
	std::vector<Byte> large_buffer; // Will be used only if needed.

	if (requiredSize > sizeof(small_buffer))
	{
		try
		{
			large_buffer.resize(requiredSize);
			buffer = large_buffer.data();
		}
		catch (const std::bad_alloc&)
		{
			ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: Failed to allocate memory for large MIDI message.\n");
			return;
		}
	}
	else
	{
		buffer = small_buffer;
	}

	MIDIPacketList* packetList = (MIDIPacketList*)buffer;
	MIDIPacket* packet = MIDIPacketListInit(packetList);

	// Add the MIDI data to the packet list. The size passed to MIDIPacketListAdd
	// is the total size of the buffer we have available.
	packet = MIDIPacketListAdd(packetList, (buffer == small_buffer) ? sizeof(small_buffer) : requiredSize, packet,
								timestamp, length, data);

	if (packet != nullptr)
	{
		OSStatus status = MIDISend(midiOutPort, midiDestination, packetList);
		if (status != noErr)
		{
			ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: MIDISend failed (error %d)\n", (int)status);
		}
	}
	else
	{
		// This should ideally not happen with dynamic allocation, but we keep the check for safety.
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "CoreMIDI: MIDIPacketListAdd failed unexpectedly.\n");
	}
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
