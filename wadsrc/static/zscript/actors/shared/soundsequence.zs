/*
** soundsequence.zs
**
** Actors for independently playing sound sequences in a map.
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
** A SoundSequence actor has two modes of operation:
**
**   1. If the sound sequence assigned to it has a slot, then a separate
**      SoundSequenceSlot actor is spawned (if not already present), and
**      this actor's sound sequence is added to its list of choices. This
**      actor is then destroyed, never to be heard from again. The sound
**      sequence for the slot is automatically played on the new
**      SoundSequenceSlot actor, and it should at some point execute the
**      randomsequence command so that it can pick one of the other
**      sequences to play. The slot sequence should also end with restart
**      so that more than one sequence will have a chance to play.
**
**      In this mode, it is very much like world $ambient sounds defined
**      in SNDINFO but more flexible.
**
**   2. If the sound sequence assigned to it has no slot, then it will play
**      the sequence when activated and cease playing the sequence when
**      deactivated.
**
**      In this mode, it is very much like point $ambient sounds defined
**      in SNDINFO but more flexible.
**
** To assign a sound sequence, set the SoundSequence's first argument to
** the ID of the corresponding environment sequence you want to use. If
** that sequence is a multiple-choice sequence, then the second argument
** selects which choice it picks.
*/

class AmbientSound : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOSECTOR
		+DONTSPLASH
		+NOTONAUTOMAP
	}
	
	native void MarkAmbientSounds();
	override native void Tick();
	override native void Activate(Actor activator);
	override native void Deactivate(Actor activator);
	
	override void BeginPlay ()
	{
		Super.BeginPlay ();
		Activate (NULL);
	}
	
	override void MarkPrecacheSounds()
	{
		Super.MarkPrecacheSounds();
		MarkAmbientSounds();
	}
}

class AmbientSoundNoGravity : AmbientSound
{
	default
	{
		+NOGRAVITY
	}
}

class SoundSequenceSlot : Actor
{
	default
	{
		+NOSECTOR
		+NOBLOCKMAP
		+DONTSPLASH
		+NOTONAUTOMAP
	}
	
	SeqNode sequence;
}

class SoundSequence : Actor
{
	default
	{
		+NOSECTOR
		+NOBLOCKMAP
		+DONTSPLASH
		+NOTONAUTOMAP
	}
	
	//==========================================================================
	//
	// ASoundSequence :: Destroy
	//
	//==========================================================================

	override void OnDestroy ()
	{
		StopSoundSequence ();
		Super.OnDestroy();
	}

	//==========================================================================
	//
	// ASoundSequence :: PostBeginPlay
	//
	//==========================================================================

	override void PostBeginPlay ()
	{
		Name slot = SeqNode.GetSequenceSlot (args[0], SeqNode.ENVIRONMENT);

		if (slot != 'none')
		{ // This is a slotted sound, so add it to the master for that slot
			SoundSequenceSlot master;
			let locator = ThinkerIterator.Create("SoundSequenceSlot");

			while ((master = SoundSequenceSlot(locator.Next ())))
			{
				if (master.Sequence.GetSequenceName() == slot)
				{
					break;
				}
			}
			if (master == NULL)
			{
				master = SoundSequenceSlot(Spawn("SoundSequenceSlot"));
				master.Sequence = master.StartSoundSequence (slot, 0);
			}
			master.Sequence.AddChoice (args[0], SeqNode.ENVIRONMENT);
			Destroy ();
		}
	}

	//==========================================================================
	//
	// ASoundSequence :: MarkPrecacheSounds
	//
	//==========================================================================

	override void MarkPrecacheSounds()
	{
		Super.MarkPrecacheSounds();
		SeqNode.MarkPrecacheSounds(args[0], SeqNode.ENVIRONMENT);
	}

	//==========================================================================
	//
	// ASoundSequence :: Activate
	//
	//==========================================================================

	override void Activate (Actor activator)
	{
		StartSoundSequenceID (args[0], SeqNode.ENVIRONMENT, args[1]);
	}

	//==========================================================================
	//
	// ASoundSequence :: Deactivate
	//
	//==========================================================================

	override void Deactivate (Actor activator)
	{
		StopSoundSequence ();
	}

	
}

// Heretic Sound sequences -----------------------------------------------------------

class HereticSoundSequence1 : SoundSequence
{
	default
	{
		Args 0;
	}
}

class HereticSoundSequence2 : SoundSequence
{
	default
	{
		Args 1;
	}
}

class HereticSoundSequence3 : SoundSequence
{
	default
	{
		Args 2;
	}
}

class HereticSoundSequence4 : SoundSequence
{
	default
	{
		Args 3;
	}
}

class HereticSoundSequence5 : SoundSequence
{
	default
	{
		Args 4;
	}
}

class HereticSoundSequence6 : SoundSequence
{
	default
	{
		Args 5;
	}
}

class HereticSoundSequence7 : SoundSequence
{
	default
	{
		Args 6;
	}
}

class HereticSoundSequence8 : SoundSequence
{
	default
	{
		Args 7;
	}
}

class HereticSoundSequence9 : SoundSequence
{
	default
	{
		Args 8;
	}
}

class HereticSoundSequence10 : SoundSequence
{
	default
	{
		Args 9;
	}
}

