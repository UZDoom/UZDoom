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
#include "basics.h"
#include "tarray.h"
#include "sfmt/SFMTObj.h"

class FSerializer;

class FRandom : public SFMTObj
{
public:
	FRandom() : FRandom(false) {}
	FRandom(const char* name) : FRandom(name, false) {}
	~FRandom();

	int Seed() const
	{
		return sfmt.u[0] + idx;
	}

	// Returns a random number in the range [0,255]
	int operator()()
	{
		return GenRand32() & 255;
	}

	// Returns a random number in the range [0,mod)
	int operator() (int mod)
	{
		return (0 == mod)
			? 0
			: (GenRand32() % mod);
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

	void Init(uint32_t seed);

	/* These real versions are due to Isaku Wada */
	/** generates a random number on [0,1]-real-interval */
	static inline double ToReal1(uint32_t v)
	{
		return v * (1.0/4294967295.0); 
		/* divided by 2^32-1 */ 
	}

	/** generates a random number on [0,1]-real-interval */
	inline double GenRand_Real1()
	{
		return ToReal1(GenRand32());
	}

	/** generates a random number on [0,1)-real-interval */
	static inline double ToReal2(uint32_t v)
	{
		return v * (1.0/4294967296.0); 
		/* divided by 2^32 */
	}

	/** generates a random number on [0,1)-real-interval */
	inline double GenRand_Real2()
	{
		return ToReal2(GenRand32());
	}

	/** generates a random number on (0,1)-real-interval */
	static inline double ToReal3(uint32_t v)
	{
		return (((double)v) + 0.5)*(1.0/4294967296.0); 
		/* divided by 2^32 */
	}

	/** generates a random number on (0,1)-real-interval */
	inline double GenRand_Real3(void)
	{
		return ToReal3(GenRand32());
	}
	/** These real versions are due to Isaku Wada */

	/** generates a random number on [0,1) with 53-bit resolution*/
	static inline double ToRes53(uint64_t v) 
	{ 
		return v * (1.0/18446744073709551616.0L);
	}

	/** generates a random number on [0,1) with 53-bit resolution from two
	 * 32 bit integers */
	static inline double ToRes53Mix(uint32_t x, uint32_t y) 
	{ 
		return ToRes53(x | ((uint64_t)y << 32));
	}

	/** generates a random number on [0,1) with 53-bit resolution
	 */
	inline double GenRand_Res53(void) 
	{ 
		return ToRes53(GenRand64());
	} 

	/** generates a random number on [0,1) with 53-bit resolution
		using 32bit integer.
	 */
	inline double GenRand_Res53_Mix() 
	{ 
		uint32_t x, y;

		x = GenRand32();
		y = GenRand32();
		return ToRes53Mix(x, y);
	}

	// Static interface
	static void StaticClearRandom ();
	static void StaticReadRNGState (FSerializer &arc);
	static void StaticWriteRNGState (FSerializer &file);
	static FRandom *StaticFindRNG(const char *name, bool client);
	static void SaveRNGState(TArray<FRandom>& backups);
	static void RestoreRNGState(TArray<FRandom>& backups);

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
