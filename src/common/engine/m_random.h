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

class FRandom
{
public:
	FRandom() : FRandom(false) {}
	FRandom(const char* name) : FRandom(name, false) {}
	~FRandom();

	int Seed() const
	{
		auto s32 = this->s32();
		return s32[0] ^ s32[1];
	}

	uint64_t s = 0;
	std::array<uint32_t, 2> s32() const {
		return {
			static_cast<uint32_t>(this->s),
			static_cast<uint32_t>(this->s >> 32)
		};
	}

	void from_s32(std::array<uint32_t, 2> s32) {
		this->s = static_cast<uint64_t>(s32[0]) | (static_cast<uint64_t>(s32[1]) << 32);
	}

	uint64_t GenRand64() {
		return (uint64_t(GenRand32()) << 32) + uint64_t(GenRand32());
	}

	uint32_t GenRand32() {
		// this is the PCG-XSH-RR RNG algorithm
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

	// Returns a random number in the range [0,bound).
	//
	// `bound` must not be `0`.
	uint32_t GenRand32BoundExclusive(uint32_t bound)
	{
		// this algorithm is designed to produce the required number without
		// any bias. it does so by rejection sampling, by carving out a range
		// within the integers where the size of the range is a multiple of
		// `bound`, and then rejection sampling until a random number falls
		// into this range, then taking it modulo `bound`.

		// this line looks weird  - but it's effectively a way to calculate
		// `2^32 % bound` without going into 64-bit arithmetic. in unsigned
		// 32-bit arithmetic, `-bound == 2^32 - bound`. subtracting `bound`
		// doesn't change the result modulo `bound` so this is the same number
		// as `2^32 % bound`.
		auto threshold = -bound % bound;

		// then sample an integer until it's in the range `[2^32 % bound,
		// 2^32)`. this range has size `2^32 - (2^32 % bound)`. this number
		// itself is a multiple of `bound`, because `2^32 % bound` is, by
		// definition, the remainder of `2^32` after division by `bound` - so
		// subtracting it from `2^32` gives a number divisible by `bound`.
		//
		// worth noting that this for loop is very likely to terminate on the
		// first iteration for most bounds. if not the first, then probably the
		// second. the worst possible case is `bound == 2^31 + 1`, at which
		// point the threshold becomes `2^31 - 1`. this gives the loop a ~1/2
		// chance of terminating every time, and so some probability maths
		// shows the worst-case expected value of the number of loops is ~2. in
		// most actual cases, the loop has a high chance of just succeeding
		// instantly.
		for (;;) {
			auto r = GenRand32();
			if (r >= threshold) {
				// then, take the output modulo bound. there's no longer any
				// potential bias in the range, so this gets every answer with
				// equal likelihood
				return r % bound;
			}
		}
	}
	uint32_t operator() (uint32_t bound) {
		return this->GenRand32BoundExclusive(bound);
	}

	// Returns a random number in the range [min, max].
	int32_t GenRand32MinMaxInclusive(int32_t min, int32_t max) {
		if (min == max) {
			return min;
		}
		if (min > max) {
			std::swap(min, max);
		}
		// some 32-bit signed integers are far enough away from each other that
		// the difference needs the extra space of an unsigned integer. as a
		// result, this entire function just does everything unsigned and lets
		// modular arithmetic handle the details
		auto lo = static_cast<uint32_t>(min);
		auto hi = static_cast<uint32_t>(max);

		// we're going to generate a number in [lo, hi], so we can instead do
		// one in [0, hi - lo] and add lo. but the bound generating function
		// uses an exclusive range, so instead needs to be [0, hi - lo + 1).
		// note that hi can actually be below lo after casting to unsigned -
		// but the maths works out fine anyway
		auto bound = hi - lo + 1;
		// bound calculation overflow check - can only happen if hi is int max
		// & lo is int min, so generate on full range if so
		if (bound == 0) {
			return static_cast<int32_t>(this->GenRand32());
		}
		// if lo was originally a negative number and hi is positive then it's
		// actually as-designed for this to overflow. this is defined behavior
		// in C++ whereas signed overflow technically isn't
		auto result = this->GenRand32BoundExclusive(bound) + lo;

		return static_cast<int32_t>(result);
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
