/*
** x86.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#include <stdlib.h>
#include <string.h>
#include "x86.h"

CPUInfo CPU;

#if !defined(__amd64__) && !defined(__i386__) && !defined(_M_IX86) && !defined(_M_X64)
void CheckCPUID(CPUInfo *cpu)
{
	memset(cpu, 0, sizeof(*cpu));
	cpu->DataL1LineSize = CPUInfo::AssumedDefaultCacheLineSizeBytes;
}

FString DumpCPUInfo(const CPUInfo *cpu, bool brief)
{
	return FString();
}
#else

#ifdef _MSC_VER
#include <intrin.h>
#endif
#include <emmintrin.h>


#ifdef __GNUC__
#define __cpuidex(output, func, subfunc) \
	__asm__ __volatile__("cpuid" \
	: "=a" ((output)[0]), "=b" ((output)[1]), "=c" ((output)[2]), "=d" ((output)[3]) \
	: "a" (func), "c" (subfunc));
#define __cpuid(output, func) __cpuidex(output, func, 0)
#endif

void CheckCPUID(CPUInfo *cpu)
{
	int foo[4];
	unsigned int maxext;

	memset(cpu, 0, sizeof(*cpu));

	cpu->DataL1LineSize = CPUInfo::AssumedDefaultCacheLineSizeBytes;

	// Get vendor ID
	__cpuid(foo, 0);
	const int maxid = foo[0];
	cpu->dwVendorID[0] = foo[1];
	cpu->dwVendorID[1] = foo[3];
	cpu->dwVendorID[2] = foo[2];
	if (foo[1] == MAKE_ID('A','u','t','h') &&
		foo[3] == MAKE_ID('e','n','t','i') &&
		foo[2] == MAKE_ID('c','A','M','D'))
	{
		cpu->bIsAMD = true;
	}

	// Get features flags and other info
	__cpuid(foo, 1);
	cpu->FeatureFlags[0] = foo[1];	// Store brand index and other stuff
	cpu->FeatureFlags[1] = foo[2];	// Store extended feature flags
	cpu->FeatureFlags[2] = foo[3];	// Store feature flags

	cpu->HyperThreading = (foo[3] & (1 << 28)) > 0;

	// If CLFLUSH instruction is supported, get the real cache line size.
	if (foo[3] & (1 << 19))
	{
		cpu->DataL1LineSize = (foo[1] & 0xFF00) >> (8 - 3);
	}

	cpu->Stepping = foo[0] & 0x0F;
	cpu->Type = (foo[0] & 0x3000) >> 12;	// valid on Intel only
	cpu->Model = (foo[0] & 0xF0) >> 4;
	cpu->Family = (foo[0] & 0xF00) >> 8;

	if (cpu->Family == 15)
	{ // Add extended family.
		cpu->Family += (foo[0] >> 20) & 0xFF;
	}
	if (cpu->Family == 6 || cpu->Family == 15)
	{ // Add extended model ID.
		cpu->Model |= (foo[0] >> 12) & 0xF0;
	}

	// Check for extended functions.
	__cpuid(foo, 0x80000000);
	maxext = (unsigned int)foo[0];

	if (maxext >= 0x80000004)
	{ // Get processor brand string.
		__cpuid((int *)&cpu->dwCPUString[0], 0x80000002);
		__cpuid((int *)&cpu->dwCPUString[4], 0x80000003);
		__cpuid((int *)&cpu->dwCPUString[8], 0x80000004);
	}

	if (cpu->bIsAMD)
	{
		if (maxext >= 0x80000005)
		{ // Get data L1 cache info.
			__cpuid(foo, 0x80000005);
			cpu->AMD_DataL1Info = foo[2];
		}
		if (maxext >= 0x80000001)
		{ // Get AMD-specific feature flags.
			__cpuid(foo, 0x80000001);
			cpu->AMDStepping = foo[0] & 0x0F;
			cpu->AMDModel = (foo[0] & 0xF0) >> 4;
			cpu->AMDFamily = (foo[0] & 0xF00) >> 8;

			if (cpu->AMDFamily == 15)
			{ // Add extended model and family.
				cpu->AMDFamily += (foo[0] >> 20) & 0xFF;
				cpu->AMDModel |= (foo[0] >> 12) & 0xF0;
			}
			cpu->FeatureFlags[3] = foo[3];	// AMD feature flags
		}
	}

	if (maxid >= 7)
	{
		__cpuidex(foo, 7, 0);
		cpu->FeatureFlags[4] = foo[1];
		cpu->FeatureFlags[5] = foo[2];
		cpu->FeatureFlags[6] = foo[3];

		__cpuidex(foo, 7, 1);
		cpu->FeatureFlags[7] = foo[0];
	}
}

FString DumpCPUInfo(const CPUInfo *cpu, bool brief)
{
	char cpustring[4*4*3+1];

	// Why does Intel right-justify this string (on P4s)
	// or add extra spaces (on Cores)?
	const char *f = cpu->CPUString;
	char *t;

	// Skip extra whitespace at the beginning.
	while (*f == ' ')
	{
		++f;
	}

	// Copy string to temp buffer, but condense consecutive
	// spaces to a single space character.
	for (t = cpustring; *f != '\0'; ++f)
	{
		if (*f == ' ' && *(f - 1) == ' ')
		{
			continue;
		}
		*t++ = *f;
	}
	*t = '\0';

	FString out;
	if (cpu->VendorID[0])
	{
		out.Format("CPU Vendor ID: %s\n", cpu->VendorID);
		if (cpustring[0])
		{
			out.AppendFormat("  Name: %s\n", cpustring);
		}
		if (cpu->bIsAMD)
		{
			out.AppendFormat("  Family %d (%d), Model %d, Stepping %d\n",
				cpu->Family, cpu->AMDFamily, cpu->AMDModel, cpu->AMDStepping);
		}
		else
		{
			out.AppendFormat("  Family %d, Model %d, Stepping %d\n",
				cpu->Family, cpu->Model, cpu->Stepping);
		}
		out.AppendFormat("  Features:");
		if (cpu->bSSE2)			out += (" SSE2");
		if (cpu->bSSE3)			out += (" SSE3");
		if (cpu->bSSSE3)		out += (" SSSE3");
		if (cpu->bSSE41)		out += (" SSE4.1");
		if (cpu->bSSE42)		out += (" SSE4.2");
		if (cpu->bAVX)			out += (" AVX");
		if (cpu->bAVX2)			out += (" AVX2");
		if (cpu->bAVX512_F)		out += (" AVX512");
		if (cpu->bF16C)			out += (" F16C");
		if (cpu->bFMA3)			out += (" FMA3");
		if (cpu->bBMI1)			out += (" BMI1");
		if (cpu->bBMI2)			out += (" BMI2");
		if (cpu->HyperThreading)	out += (" HyperThreading");
		out += ("\n");
	}
	return out;
}

#endif
