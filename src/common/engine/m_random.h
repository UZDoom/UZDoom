/*
** m_random.h
**
** Random number generators
**
**---------------------------------------------------------------------------
**
** Copyright 2002-2016 Marisa Heit
** Copyright 2009-2016 Christoph Oelckers
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
*/

#ifndef __M_RANDOM__
#define __M_RANDOM__

#include <stdio.h>
#include <array>
#include "basics.h"
#include "tarray.h"

class FSerializer;

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

class FRandom
{
public:
	FRandom() : FRandom(false) {}
	FRandom(const char* name) : FRandom(name, false) {}
	~FRandom();

	int Seed() const
	{
		return s32[0] ^ s32[1];
	}

	union {
		std::array<uint32_t, 2> s32;
		uint64_t s;
	};

	uint64_t GenRand64() {
		return (uint64_t(GenRand32()) << 32) + uint64_t(GenRand32());
	}

	uint32_t GenRand32() {
		uint64_t oldstate = s;
		s = oldstate * 6364136223846793005ULL + 0xda3e39cb94b95bdbULL;
		uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
		uint32_t rot = oldstate >> 59u;
		return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
	}

	// Returns a random number in the range [0,255]
	int operator()()
	{
		return GenRand32() & 255;
	}

	// Returns a random number in the range [0,bound)
	int operator() (int bound)
	{
		uint32_t threshold = -bound % bound;

		for (;;) {
			uint32_t r = GenRand32();
			if (r >= threshold)
				return r % bound;
		}
	}

	// Returns rand# - rand#
	int Random2()
	{
		return Random2(255);
	}

// Returns (rand# & mask) - (rand# & mask)
	int Random2(int mask)
	{
		int t = GenRand32() & mask & 255;
		return t - (GenRand32() & mask & 255);
	}

	// HITDICE macro used in Heretic and Hexen
	int HitDice(int count)
	{
		return (1 + (GenRand32() & 7)) * count;
	}

	int Random()				// synonym for ()
	{
		return operator()();
	}

	double RandomFloat() {
		return GenRand32() * ldexp(1, -32);
	}

	double RandomFloat(double r0, double r1) {
		auto t = RandomFloat();
		auto ret = r0 * (1.0 - t) + r1 * t;
		return ret;
	}

	void Init(uint32_t seed);

	// Static interface
	static void StaticClearRandom ();
	static void StaticReadRNGState (FSerializer &arc);
	static void StaticWriteRNGState (FSerializer &file);
	static FRandom *StaticFindRNG(const char *name, bool client);
	static void RollbackRNGState(FSerializer& arc);

#ifndef NDEBUG
	static void StaticPrintSeeds ();
#endif

protected:
	FRandom(bool client);
	FRandom(const char* name, bool client);

private:
#ifndef NDEBUG
	const char *Name;
#endif
	FRandom *Next;
	uint32_t NameCRC;
	bool bClient;

	static FRandom *RNGList, *CRNGList;
};

class FCRandom : public FRandom
{
public:
	FCRandom() : FRandom(true) {}
	FCRandom(const char* name) : FRandom(name, true) {}
};

extern uint32_t rngseed;			// The starting seed (not part of state)

extern uint32_t staticrngseed;		// Static rngseed that can be set by the user
extern bool use_staticrng;


// M_Random can be used for numbers that do not affect gameplay
extern FCRandom M_Random;

#endif
