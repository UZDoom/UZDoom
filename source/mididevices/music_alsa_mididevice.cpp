/*
** Provides an ALSA implementation of a MIDI output device.
**
**---------------------------------------------------------------------------
** Copyright 2008-2010 Marisa Heit
** Copyright 2020 Petr Mrazek
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

#if defined __linux__ && defined HAVE_SYSTEM_MIDI

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "mididevice.h"
#include "zmusic/mus2midi.h"
#include "zmusic_internal.h"

#include "music_alsa_state.h"
#include <alsa/asoundlib.h>

class AlsaMIDIDevice : public MIDIDevice
{
public:
	AlsaMIDIDevice(int dev_id, bool precache);
	~AlsaMIDIDevice();
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
	void HandleEvent(snd_seq_event_t &event, uint32_t tick);
	void SendImmediateShortMsg(uint8_t command, uint8_t data1 = 0, uint8_t data2 = 0);
	snd_midi_event_t* Coder = nullptr;

	// PulledEvent structure to hold the next event to be processed
	struct PulledEvent
	{
		snd_seq_event_t Event;
		uint32_t TickDelta;
	};
	PulledEvent PulledEvent;

	// Alsa sequencer handles
	AlsaSequencer &sequencer;
	const static int IntendedPortId = 0;
	bool Connected = false;
	int PortId = -1;
	int QueueId = -1;

	int DestinationClientId;
	int DestinationPortId;
	int Technology;

	// Threading
	std::thread PlayerThread;
	std::atomic<bool> Exit;
	std::mutex Mutex;
	std::condition_variable ExitCond;

	// Timing
	int InitialTempo = 500000;
	int Tempo;
	int Division = 100; // PPQN

	// ZMusic MidiHeader data
	MidiHeader* Events = nullptr;
	uint32_t Position = 0;
	uint32_t PositionOffset;
};

AlsaMIDIDevice::AlsaMIDIDevice(int dev_id, bool precache) : sequencer(AlsaSequencer::Get())
{
	auto & internalDevices = sequencer.GetInternalDevices();
	auto & device = internalDevices.at(dev_id);
	DestinationClientId = device.ClientID;
	DestinationPortId = device.PortNumber;
	Precache = precache;
	Technology = device.GetDeviceClass();
}

AlsaMIDIDevice::~AlsaMIDIDevice()
{
	Close();
}

int AlsaMIDIDevice::Open()
{
	if (!sequencer.IsOpen())
	{
		return 1;
	}

	if (PortId < 0)
	{
		snd_seq_port_info_t* pinfo;
		snd_seq_port_info_alloca(&pinfo);

		snd_seq_port_info_set_port(pinfo, IntendedPortId);
		snd_seq_port_info_set_port_specified(pinfo, 1);

		snd_seq_port_info_set_name(pinfo, "ZMusic Program Music");

		snd_seq_port_info_set_capability(pinfo, 0);
		snd_seq_port_info_set_type(pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

		snd_midi_event_new(3, &Coder); // 3 Bytes for short messages.
		snd_midi_event_init(Coder);

		int err = 0;
		err = snd_seq_create_port(sequencer.handle, pinfo);
		if (err) { return err; }
		PortId = IntendedPortId;
	}

	if (QueueId < 0)
	{
		QueueId = snd_seq_alloc_named_queue(sequencer.handle, "ZMusic Program Queue");
	}

	if (!Connected)
	{
		Connected = (snd_seq_connect_to(sequencer.handle, PortId, DestinationClientId, DestinationPortId) == 0);
	}
	return 0;
}

void AlsaMIDIDevice::Close()
{
	if (Connected)
	{
		snd_seq_disconnect_to(sequencer.handle, PortId, DestinationClientId, DestinationPortId);
		Connected = false;
	}
	if (QueueId >= 0)
	{
		snd_seq_free_queue(sequencer.handle, QueueId);
		QueueId = -1;
	}
	if (PortId >= 0)
	{
		snd_seq_delete_port(sequencer.handle, PortId);
		PortId = -1;
	}
	if (Coder)
	{
		snd_midi_event_free(Coder);
		Coder = nullptr;
	}
}

bool AlsaMIDIDevice::IsOpen() const
{
	return Connected;
}

int AlsaMIDIDevice::GetTechnology() const
{
	return Technology;
}

bool AlsaMIDIDevice::FakeVolume()
{
	return true; // No true volume control support, so fake volume
}

int AlsaMIDIDevice::SetTempo(int tempo)
{
	InitialTempo = tempo;
	return 0;
}

int AlsaMIDIDevice::SetTimeDiv(int timediv)
{
	Division = timediv;
	return 0;
}

// This is meant to mirror WinMIDIDevice::PrecacheInstruments
void AlsaMIDIDevice::PrecacheInstruments(const uint16_t* instruments, int count)
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

void AlsaMIDIDevice::InitPlayback()
{
	Exit.store(false, std::memory_order_relaxed);
}

int AlsaMIDIDevice::Resume()
{
	if (!Connected || PlayerThread.joinable())
	{
		return 1;
	}
	Exit.store(false, std::memory_order_relaxed);
	PlayerThread = std::thread(&AlsaMIDIDevice::PlayerLoop, this);
	return 0;
}

void AlsaMIDIDevice::Stop()
{
	Exit.store(true, std::memory_order_relaxed);
	ExitCond.notify_all();
	if (PlayerThread.joinable())
	{
		PlayerThread.join();
	}
	snd_seq_drop_output(sequencer.handle); // This drops events in the sequencer, the sequencer is still usable
	snd_seq_stop_queue(sequencer.handle, QueueId, nullptr);
	snd_seq_drain_output(sequencer.handle);

	// Reset all channels to prevent hanging notes
	for (int channel = 0; channel < 16; ++channel)
	{
		SendImmediateShortMsg(MIDI_CTRLCHANGE | channel, 123, 0); // All Notes Off
		SendImmediateShortMsg(MIDI_CTRLCHANGE | channel, 121, 0);  // Reset All Controllers
	}
	snd_seq_sync_output_queue(sequencer.handle);
}

bool AlsaMIDIDevice::Pause(bool paused)
{
	return false; // Pausing is not supported
}

int AlsaMIDIDevice::StreamOut(MidiHeader* header)
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

int AlsaMIDIDevice::StreamOutSync(MidiHeader* header)
{
	return StreamOut(header);
}

bool AlsaMIDIDevice::PullEvent()
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

	uint32_t* event = (uint32_t*)(Events->lpData + Position);
	PulledEvent.TickDelta = event[0]; // First 4 bytes of event

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
		snd_seq_ev_set_queue_tempo(&PulledEvent.Event, QueueId, MEVENT_EVENTPARM(event[2]));
		break;
	case MEVENT_LONGMSG: // SysEx message...
		{
			int long_msg_len = MEVENT_EVENTPARM(event[2]);
			uint8_t* long_msg_data = (uint8_t*)&event[3];
			// Ensure valid sysex message
			if (long_msg_len > 2 && long_msg_data[0] == 0xF0 && long_msg_data[long_msg_len - 1] == 0xF7)
			{
				snd_seq_ev_set_sysex(&PulledEvent.Event, long_msg_len, (void*)long_msg_data);
			}
			else
			{
				PulledEvent.Event.type = SND_SEQ_EVENT_NONE;
			}
			break;
		}
	case MEVENT_SHORTMSG:
		{
			uint8_t msg[3] = {	(uint8_t)(event[2] & 0xff), // Status
								(uint8_t)((event[2] >> 8) & 0xff), // Data 1
								(uint8_t)((event[2] >> 16) & 0xff) }; // Data 2

			// This silently ignores extra bytes, so no message length logic is needed.
			snd_midi_event_encode(Coder, msg, 3, &PulledEvent.Event);
			break;
		}
	default: // We didn't really recognize the event, treat it as a NOP
		PulledEvent.Event.type = SND_SEQ_EVENT_NONE;
	}
	return true;
}

/*
 * Pumps events from the input to the output in a worker thread.
 * It tries to keep the amount of events (time-wise) in the ALSA sequencer queue to be between 40 and 80ms by sleeping where necessary.
 * This means Alsa can play them safely without running out of things to do, and we have good control over the events themselves (volume, pause, etc.).
 */
void AlsaMIDIDevice::PlayerLoop()
{
	std::unique_lock<std::mutex> lock(Mutex);
	const std::chrono::microseconds buffer_step(40000);

	// TODO: fill in error handling throughout this.
	snd_seq_queue_tempo_t* tempo;
	snd_seq_queue_tempo_alloca(&tempo);
	snd_seq_queue_tempo_set_tempo(tempo, InitialTempo);
	snd_seq_queue_tempo_set_ppq(tempo, Division);
	snd_seq_set_queue_tempo(sequencer.handle, QueueId, tempo);

	snd_seq_start_queue(sequencer.handle, QueueId, nullptr);
	snd_seq_drain_output(sequencer.handle);

	Tempo = InitialTempo;
	int buffered_ticks = 0;

	snd_seq_queue_status_t* status;
	snd_seq_queue_status_malloc(&status);

	snd_seq_ev_clear(&PulledEvent.Event);

	while (!Exit.load(std::memory_order_relaxed))
	{
		// if we reach the end of events, await our doom at a steady rate while looking for more events
		if (!PullEvent())
		{
			ExitCond.wait_for(lock, buffer_step);
			continue;
		}

		// Figure out if we should sleep (the event is too far in the future for us to care), and for how long
		int pulled_event_tick = buffered_ticks + PulledEvent.TickDelta;
		snd_seq_get_queue_status(sequencer.handle, QueueId, status);
		int queue_tick = snd_seq_queue_status_get_tick_time(status);
		int ticks_until_pulled_ev = pulled_event_tick - queue_tick;
		auto time_until_pulled_ev = std::chrono::microseconds(ticks_until_pulled_ev * Tempo / Division);
		auto schedule_time = time_until_pulled_ev - buffer_step;
		if (schedule_time >= buffer_step)
		{
			if (ExitCond.wait_for(lock, schedule_time) == std::cv_status::no_timeout)
			{
				continue;
			}
		}
		if (ticks_until_pulled_ev < 0)
		{	// Can be triggered on playback start.
			// Message shouldn't be shown by default like other midi backends here.
			ZMusic_Printf(ZMUSIC_MSG_DEBUG, "Alsa sequencer underrun: %d ticks!\n", ticks_until_pulled_ev);
		}

		// We found an event worthy of sending to the sequencer
		HandleEvent(PulledEvent.Event, pulled_event_tick);
		buffered_ticks = pulled_event_tick;
		Position += PositionOffset;
	}

	snd_seq_queue_status_free(status);
}

// Requires QueueId to be started first for non-zero tick positioned events.
void AlsaMIDIDevice::HandleEvent(snd_seq_event_t &event, uint32_t tick)
{
	if (event.type == SND_SEQ_EVENT_NONE)
	{	// NOP event, clear event handle and return.
		snd_seq_ev_clear(&event);
		return;
	}
	snd_seq_ev_set_source(&event, PortId);
	snd_seq_ev_set_subs(&event);
	if (event.type == SND_SEQ_EVENT_TEMPO)
	{
		Tempo = event.data.queue.param.value;
		event.dest.client = SND_SEQ_CLIENT_SYSTEM;
		event.dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
	}
	snd_seq_ev_schedule_tick(&event, QueueId, false, tick);
	int result = snd_seq_event_output(sequencer.handle, &event);
	if (result < 0)
	{
		ZMusic_Printf(ZMUSIC_MSG_ERROR, "Alsa sequencer did not accept event: error %d!\n", result);
	}
	snd_seq_drain_output(sequencer.handle);
	snd_seq_ev_clear(&event);
}

// For use with PrecacheInstruments and Stop messages.
void AlsaMIDIDevice::SendImmediateShortMsg(uint8_t command, uint8_t data1, uint8_t data2)
{
	uint8_t msg[3] = { command, data1, data2 };
	snd_seq_event_t event;
	snd_seq_ev_clear(&event);
	snd_midi_event_encode(Coder, msg, 3, &event);
	HandleEvent(event, 0);
}

MIDIDevice* CreateAlsaMIDIDevice(int mididevice)
{
	return new AlsaMIDIDevice(mididevice, miscConfig.snd_midiprecache);
}
#endif
