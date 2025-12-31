/*
** x86.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
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
**
**---------------------------------------------------------------------------
**
*/

#ifndef X86_H
#define X86_H

#include "basics.h"
#include "zstring.h"

#ifdef __cpp_lib_hardware_interference_size
#include "version"
#include <new>
#endif

struct CPUInfo	// 92 bytes
{
	union
	{
		char VendorID[16];
		uint32_t dwVendorID[4];
	};
	union
	{
		char CPUString[48];
		uint32_t dwCPUString[12];
	};

	uint8_t Stepping;
	uint8_t Model;
	uint8_t Family;
	uint8_t Type;
	uint8_t HyperThreading;

	union
	{
		struct
		{
			uint8_t BrandIndex;
			uint8_t CLFlush;
			uint8_t CPUCount;
			uint8_t APICID;

			uint32_t bSSE3:1;
			uint32_t bPCLMULQDQ:1;
			uint32_t bDTES64:1;
			uint32_t bMONITOR:1;
			uint32_t bDSCPL:1;
			uint32_t bVMX:1;
			uint32_t bSMX:1;
			uint32_t bEST:1;
			uint32_t bTM2:1;
			uint32_t bSSSE3:1;
			uint32_t bCNXTID:1;
			uint32_t bSDBG:1;
			uint32_t bFMA3:1;
			uint32_t bCX16:1;
			uint32_t bXTPR:1;
			uint32_t bPDCM:1;
			uint32_t bReverved1:1;
			uint32_t bPCID:1;
			uint32_t bDCA:1;
			uint32_t bSSE41:1;
			uint32_t bSSE42:1;
			uint32_t bX2APIC:1;
			uint32_t bMOVBE:1;
			uint32_t bPOPCNT:1;
			uint32_t bTSCDL:1;
			uint32_t bAES:1;
			uint32_t bXSAVE:1;
			uint32_t bOSXSAVE:1;
			uint32_t bAVX:1;
			uint32_t bF16C:1;
			uint32_t bRDRND:1;
			uint32_t bHypervisor:1;

			uint32_t bFPU:1;
			uint32_t bVME:1;
			uint32_t bDE:1;
			uint32_t bPSE:1;
			uint32_t bRDTSC:1;
			uint32_t bMSR:1;
			uint32_t bPAE:1;
			uint32_t bMCE:1;
			uint32_t bCX8:1;
			uint32_t bAPIC:1;
			uint32_t bReserved2:1;
			uint32_t bSEP:1;
			uint32_t bMTRR:1;
			uint32_t bPGE:1;
			uint32_t bMCA:1;
			uint32_t bCMOV:1;
			uint32_t bPAT:1;
			uint32_t bPSE36:1;
			uint32_t bPSN:1;
			uint32_t bCFLUSH:1;
			uint32_t bReserved3:1;
			uint32_t bDS:1;
			uint32_t bACPI:1;
			uint32_t bMMX:1;
			uint32_t bFXSR:1;
			uint32_t bSSE:1;
			uint32_t bSSE2:1;
			uint32_t bSS:1;
			uint32_t bHTT:1;
			uint32_t bTM:1;
			uint32_t bReserved4:1;
			uint32_t bPBE:1;

			uint32_t DontCare2:22;
			uint32_t bMMXPlus:1;		// AMD's MMX extensions
			uint32_t bMMXAgain:1;		// Just a copy of bMMX above
			uint32_t DontCare3:6;
			uint32_t b3DNowPlus:1;
			uint32_t b3DNow:1;

			uint32_t bFSGSBASE:1;
			uint32_t bIA32_TSC_ADJUST:1;
			uint32_t bSGX:1;
			uint32_t bBMI1:1;
			uint32_t bHLE:1;
			uint32_t bAVX2:1;
			uint32_t bFDP_EXCPTN_ONLY:1;
			uint32_t bSMEP:1;
			uint32_t bBMI2:1;
			uint32_t bERMS:1;
			uint32_t bINVPCID:1;
			uint32_t bRTM:1;
			uint32_t bPQM:1;
			uint32_t bFPU_CS_DS:1;
			uint32_t bMPX:1;
			uint32_t bPQE:1;
			uint32_t bAVX512_F:1;
			uint32_t bAVX512_DQ:1;
			uint32_t bRDSEED:1;
			uint32_t bADX:1;
			uint32_t bSMAP:1;
			uint32_t bAVX512_IFMA:1;
			uint32_t bPCOMMIT:1;
			uint32_t bCLFLUSHOPT:1;
			uint32_t bCLWB:1;
			uint32_t bINTEL_PT:1;
			uint32_t bAVX512_PF:1;
			uint32_t bAVX512_ER:1;
			uint32_t bAVX512_CD:1;
			uint32_t bSHA:1;
			uint32_t bAVX512_BW:1;
			uint32_t bAVX512_VL:1;

			uint32_t bPREFETCHWT1:1;
			uint32_t bAVX512_VBMI:1;
			uint32_t bUMIP:1;
			uint32_t bPKU:1;
			uint32_t bOSPKE:1;
			uint32_t bWAITPKG:1;
			uint32_t bAVX512_VBMI2:1;
			uint32_t bCET_SS:1;
			uint32_t bGFNI:1;
			uint32_t bVAES:1;
			uint32_t bVPCLMULQDQ:1;
			uint32_t bAVX512_VNNI:1;
			uint32_t bAVX512_BITALG:1;
			uint32_t bReserved5:1;
			uint32_t bAVX512_VPOPCNTDQ:1;
			uint32_t bReserved6:1;
			uint32_t b5L_PAGING:1;
			uint32_t MAWAU:5;
			uint32_t bRDPID:1;
			uint32_t bReserved7:1;
			uint32_t bReserved8:1;
			uint32_t bCLDEMOTE:1;
			uint32_t bReserved9:1;
			uint32_t bMOVDIRI:1;
			uint32_t bMOVDIR64B:1;
			uint32_t bENQCMD:1;
			uint32_t bSGX_LC:1;
			uint32_t bPKS:1;

			uint32_t bReserved10:1;
			uint32_t bReserved11:1;
			uint32_t bAVX512_4VNNIW:1;
			uint32_t bAVX512_4FMAPS:1;
			uint32_t bFSRM:1;
			uint32_t bReserved12:1;
			uint32_t bReserved13:1;
			uint32_t bReserved14:1;
			uint32_t bAVX512_VP2INTERSECT:1;
			uint32_t bSRBDS_CTRL:1;
			uint32_t bMD_CLEAR:1;
			uint32_t bReserved15:1;
			uint32_t bReserved16:1;
			uint32_t bTSX_FORCE_ABORT:1;
			uint32_t bSERIALIZE:1;
			uint32_t bHYBRID:1;
			uint32_t bTSXLDTRK:1;
			uint32_t bReserved17:1;
			uint32_t bPCONFIG:1;
			uint32_t bLBR:1;
			uint32_t bCET_IBT:1;
			uint32_t bReserved18:1;
			uint32_t bAMX_BF16:1;
			uint32_t bReserved19:1;
			uint32_t bAMX_TILE:1;
			uint32_t bAMX_INT8:1;
			uint32_t bIBRS_IBPB:1;
			uint32_t bSTIBP:1;
			uint32_t bL1D_FLUSH:1;
			uint32_t bIA32_ARCH_CAPABILITIES:1;
			uint32_t bIA32_CORE_CAPABILITIES:1;
			uint32_t bSSBD:1;

			uint32_t DontCare4:5;
			uint32_t bAVX512_BF16:1;
			uint32_t DontCare5:26;
		};
		uint32_t FeatureFlags[8];
	};

	uint8_t AMDStepping;
	uint8_t AMDModel;
	uint8_t AMDFamily;
	uint8_t bIsAMD;

	union
	{
		struct
		{
			uint8_t DataL1LineSize;
			uint8_t DataL1LinesPerTag;
			uint8_t DataL1Associativity;
			uint8_t DataL1SizeKB;
		};
		uint32_t AMD_DataL1Info;
	};
#ifdef __cpp_lib_hardware_interference_size
	static inline constexpr size_t AssumedDefaultCacheLineSizeBytes = std::hardware_destructive_interference_size;
#else
	static inline constexpr size_t AssumedDefaultCacheLineSizeBytes = 64;
#endif
};


extern CPUInfo CPU;

void CheckCPUID (CPUInfo *cpu);
FString DumpCPUInfo (const CPUInfo *cpu, bool brief = false);

#endif

