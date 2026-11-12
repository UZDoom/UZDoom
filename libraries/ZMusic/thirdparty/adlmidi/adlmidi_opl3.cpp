/*
 * libADLMIDI is a free Software MIDI synthesizer library with OPL3 emulation
 *
 * Original ADLMIDI code: Copyright (c) 2010-2014 Joel Yliluoma <bisqwit@iki.fi>
 * ADLMIDI Library API:   Copyright (c) 2015-2026 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Library is based on the ADLMIDI, a MIDI player for Linux and Windows with OPL3 emulation:
 * http://iki.fi/bisqwit/source/adlmidi.html
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "adlmidi_opl3.hpp"
#include "adlmidi_private.hpp"
#include <stdlib.h>
#include <cassert>

#include "models/opl_models.h"


#ifdef ENABLE_HW_OPL_DOS
#   include "chips/dos_hw_opl.h"
#else
#   if defined(ADLMIDI_DISABLE_NUKED_EMULATOR) && \
       defined(ADLMIDI_DISABLE_DOSBOX_EMULATOR) && \
       defined(ADLMIDI_DISABLE_OPAL_EMULATOR) && \
       defined(ADLMIDI_DISABLE_JAVA_EMULATOR) && \
       defined(ADLMIDI_DISABLE_ESFMU_EMULATOR) && \
       defined(ADLMIDI_DISABLE_MAME_OPL2_EMULATOR) && \
       defined(ADLMIDI_DISABLE_YMFM_EMULATOR) && \
       !defined(ADLMIDI_ENABLE_OPL2_LLE_EMULATOR) && \
       !defined(ADLMIDI_ENABLE_OPL3_LLE_EMULATOR) && \
       !defined(ADLMIDI_ENABLE_HW_SERIAL)
#       error "No emulators enabled. You must enable at least one emulator to use this library!"
#   endif

// Nuked OPL3 emulator, Most accurate, but requires the powerful CPU
#   ifndef ADLMIDI_DISABLE_NUKED_EMULATOR
#       include "chips/nuked_opl3.h"
#       include "chips/nuked_opl3_fast.h"
#       include "chips/nuked_opl2.h"
#       include "chips/nuked_cqm.h"
#   endif

// DosBox 0.74 OPL3 emulator, Well-accurate and fast
#   ifndef ADLMIDI_DISABLE_DOSBOX_EMULATOR
#       include "chips/dosbox_opl3.h"
#       include "chips/dosbox_opl2.h"
#   endif

// Opal emulator
#   ifndef ADLMIDI_DISABLE_OPAL_EMULATOR
#       include "chips/opal_opl3.h"
#   endif

// Java emulator
#   ifndef ADLMIDI_DISABLE_JAVA_EMULATOR
#       include "chips/java_opl3.h"
#   endif

// ESFMu emulator
#   ifndef ADLMIDI_DISABLE_ESFMU_EMULATOR
#       include "chips/esfmu_opl3.h"
#   endif

// MAME OPL2 emulator
#   ifndef ADLMIDI_DISABLE_MAME_OPL2_EMULATOR
#       include "chips/mame_opl2.h"
#   endif

// YMFM emulators
#   ifndef ADLMIDI_DISABLE_YMFM_EMULATOR
#       include "chips/ymfm_opl2.h"
#       include "chips/ymfm_opl3.h"
#   endif

// Nuked OPL2 LLE emulator
#   ifdef ADLMIDI_ENABLE_OPL2_LLE_EMULATOR
#       include "chips/ym3812_lle.h"
#   endif

// Nuked OPL3 LLE emulator
#   ifdef ADLMIDI_ENABLE_OPL3_LLE_EMULATOR
#       include "chips/ymf262_lle.h"
#   endif

// HW OPL Serial
#   ifdef ADLMIDI_ENABLE_HW_SERIAL
#       include "chips/opl_serial_port.h"
#   endif
#endif

static const unsigned adl_emulatorSupport = 0
#ifndef ENABLE_HW_OPL_DOS
#   ifndef ADLMIDI_DISABLE_NUKED_EMULATOR
    | (1u << ADLMIDI_EMU_NUKED) | (1u << ADLMIDI_EMU_NUKED_FAST) | (1u << ADLMIDI_EMU_NUKED_OPL2_LITE) | (1u << ADLMIDI_EMU_NUKED_CQM)
#   endif

#   ifndef ADLMIDI_DISABLE_DOSBOX_EMULATOR
    | (1u << ADLMIDI_EMU_DOSBOX)
    | (1u << ADLMIDI_EMU_DOSBOX_OPL2)
#   endif

#   ifndef ADLMIDI_DISABLE_OPAL_EMULATOR
    | (1u << ADLMIDI_EMU_OPAL)
#   endif

#   ifndef ADLMIDI_DISABLE_ESFMU_EMULATOR
    | (1u << ADLMIDI_EMU_ESFMu)
#   endif

#   ifndef ADLMIDI_DISABLE_JAVA_EMULATOR
    | (1u << ADLMIDI_EMU_JAVA)
#   endif

#   ifndef ADLMIDI_DISABLE_MAME_OPL2_EMULATOR
    | (1u << ADLMIDI_EMU_MAME_OPL2)
#   endif

#   ifndef ADLMIDI_DISABLE_YMFM_EMULATOR
    | (1u << ADLMIDI_EMU_YMFM_OPL2)
    | (1u << ADLMIDI_EMU_YMFM_OPL3)
#   endif

#   ifdef ADLMIDI_ENABLE_OPL2_LLE_EMULATOR
    | (1u << ADLMIDI_EMU_NUKED_OPL2_LLE)
#   endif

#   ifdef ADLMIDI_ENABLE_OPL3_LLE_EMULATOR
    | (1u << ADLMIDI_EMU_NUKED_OPL3_LLE)
#   endif
#endif
;

//! Check emulator availability
bool adl_isEmulatorAvailable(int emulator)
{
    return (adl_emulatorSupport & (1u << (unsigned)emulator)) != 0;
}

//! Find highest emulator
int adl_getHighestEmulator()
{
    int emu = -1;
    for(unsigned m = adl_emulatorSupport; m > 0; m >>= 1)
        ++emu;
    return emu;
}

//! Find lowest emulator
int adl_getLowestEmulator()
{
    int emu = -1;
    unsigned m = adl_emulatorSupport;
    if(m > 0)
    {
        for(emu = 0; (m & 1) == 0; m >>= 1)
            ++emu;
    }
    return emu;
}

static const struct OplTimbre c_defaultInsCache = { 0x1557403,0x005B381, 0x49,0x80, 0x4, +0 };

//! Per-channel and per-operator registers map
static const uint16_t g_operatorsMap[(NUM_OF_CHANNELS + NUM_OF_RM_CHANNELS) * 2] =
{
    // Channels 0-2
    0x000, 0x003, 0x001, 0x004, 0x002, 0x005, // operators  0, 3,  1, 4,  2, 5
    // Channels 3-5
    0x008, 0x00B, 0x009, 0x00C, 0x00A, 0x00D, // operators  6, 9,  7,10,  8,11
    // Channels 6-8
    0x010, 0x013, 0x011, 0x014, 0x012, 0x015, // operators 12,15, 13,16, 14,17
    // Same for second card
    0x100, 0x103, 0x101, 0x104, 0x102, 0x105, // operators 18,21, 19,22, 20,23
    0x108, 0x10B, 0x109, 0x10C, 0x10A, 0x10D, // operators 24,27, 25,28, 26,29
    0x110, 0x113, 0x111, 0x114, 0x112, 0x115, // operators 30,33, 31,34, 32,35

    //==For Rhythm-mode percussions
    // Channel 18
    0x010, 0x013,  // operators 12,15
    // Channel 19
    0xFFF, 0x014,  // operator 16
    // Channel 19
    0x012, 0xFFF,  // operator 14
    // Channel 19
    0xFFF, 0x015,  // operator 17
    // Channel 19
    0x011, 0xFFF,  // operator 13

    //==For Rhythm-mode percussions in CMF, snare and cymbal operators has inverted!
    0x010, 0x013,  // operators 12,15
    // Channel 19
    0x014, 0xFFF,  // operator 16
    // Channel 19
    0x012, 0xFFF,  // operator 14
    // Channel 19
    0x015, 0xFFF,  // operator 17
    // Channel 19
    0x011, 0xFFF   // operator 13
};

//! Channel map to register offsets
static const uint16_t g_channelsMap[NUM_OF_CHANNELS] =
{
    0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008, // 0..8
    0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, // 9..17 (secondary set)
    0x006, 0x007, 0x008, 0x008, 0x008 // <- hw percussions, hihats and cymbals using tom-tom's channel as pitch source
};

//! Channel map to register offsets (separated copy for panning and for CMF)
static const uint16_t g_channelsMapPan[NUM_OF_CHANNELS] =
{
    0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008, // 0..8
    0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, // 9..17 (secondary set)
    0x006, 0x007, 0x008, 0xFFF, 0xFFF // <- hw percussions, 0xFFF = no support for pitch/pan
};

//! Channel map to register offsets (separated copy for feedback+connection bits)
static const uint16_t g_channelsMapFBConn[NUM_OF_CHANNELS] =
{
    0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008, // 0..8
    0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, // 9..17 (secondary set)
    0x006, 0xFFF, 0xFFF, 0xFFF, 0xFFF // <- hw percussions, 0xFFF = no support for pitch/pan
};

/*
    In OPL3 mode:
         0    1    2    6    7    8     9   10   11    16   17   18
       op0  op1  op2 op12 op13 op14  op18 op19 op20  op30 op31 op32
       op3  op4  op5 op15 op16 op17  op21 op22 op23  op33 op34 op35
         3    4    5                   13   14   15
       op6  op7  op8                 op24 op25 op26
       op9 op10 op11                 op27 op28 op29
    Ports:
        +0   +1   +2  +10  +11  +12  +100 +101 +102  +110 +111 +112
        +3   +4   +5  +13  +14  +15  +103 +104 +105  +113 +114 +115
        +8   +9   +A                 +108 +109 +10A
        +B   +C   +D                 +10B +10C +10D

    Percussion:
      bassdrum = op(0): 0xBD bit 0x10, operators 12 (0x10) and 15 (0x13) / channels 6, 6b
      snare    = op(3): 0xBD bit 0x08, operators 16 (0x14)               / channels 7b
      tomtom   = op(4): 0xBD bit 0x04, operators 14 (0x12)               / channels 8
      cym      = op(5): 0xBD bit 0x02, operators 17 (0x17)               / channels 8b
      hihat    = op(2): 0xBD bit 0x01, operators 13 (0x11)               / channels 7


    In OPTi mode ("extended FM" in 82C924, 82C925, 82C931 chips):
         0   1   2    3    4    5    6    7     8    9   10   11   12   13   14   15   16   17
       op0 op4 op6 op10 op12 op16 op18 op22  op24 op28 op30 op34 op36 op38 op40 op42 op44 op46
       op1 op5 op7 op11 op13 op17 op19 op23  op25 op29 op31 op35 op37 op39 op41 op43 op45 op47
       op2     op8      op14      op20       op26      op32
       op3     op9      op15      op21       op27      op33    for a total of 6 quad + 12 dual
    Ports: ???
*/


enum
{
    MasterVolumeDefault = 127
};

enum
{
    OPL_PANNING_LEFT  = 0x10,
    OPL_PANNING_RIGHT = 0x20,
    OPL_PANNING_BOTH  = 0x30
};


static OplInstMeta makeEmptyInstrument()
{
    OplInstMeta ins;
    memset(&ins, 0, sizeof(OplInstMeta));
    ins.flags = OplInstMeta::Flag_NoSound;
    return ins;
}

const OplInstMeta OPL3::m_emptyInstrument = makeEmptyInstrument();

OPL3::OPL3() :
#ifdef ENABLE_HW_OPL_DOS
    m_dpmi_locker(this),
#endif
#ifdef ADLMIDI_ENABLE_HW_SERIAL
    m_serial(false),
    m_serialBaud(0),
    m_serialProtocol(0),
#endif
    m_softPanningSup(false),
    m_currentChipType((int)OPLChipBase::CHIPTYPE_OPL3),
    m_perChipChannels(OPL3_CHANNELS_RHYTHM_BASE),
    m_numChips(1),
    m_numFourOps(0),
    m_deepTremoloMode(false),
    m_deepVibratoMode(false),
    m_rhythmMode(false),
    m_softPanning(false),
    m_masterVolume(MasterVolumeDefault),
    m_musicMode(MODE_MIDI),
    m_volumeScale(VOLUME_Generic),
    m_getFreq(&oplModel_genericFreq),
    m_getVolume(&oplModel_genericVolume),
    m_channelAlloc(ADLMIDI_ChanAlloc_AUTO)
{
    m_insBankSetup.volumeModel = OPL3::VOLUME_Generic;
    m_insBankSetup.deepTremolo = false;
    m_insBankSetup.deepVibrato = false;
    m_insBankSetup.scaleModulators = false;
    m_insBankSetup.mt32defaults = false;

#ifdef DISABLE_EMBEDDED_BANKS
    m_embeddedBank = CustomBankTag;
#else
    setEmbeddedBank(0);
#endif
}

OPL3::~OPL3()
{
    m_curState.clear();

#ifdef ENABLE_HW_OPL_DOS
    silenceAll();
    writeRegI(0, 0x0BD, 0);
    writeRegI(0, 0x104, 0);
    writeRegI(0, 0x105, 0);
    silenceAll();
#endif
}

bool OPL3::setupLocked()
{
    return (m_musicMode == MODE_CMF ||
            m_musicMode == MODE_IMF ||
            m_musicMode == MODE_RSXX);
}

void OPL3::setFrequencyModel(VolumesScale model)
{
    m_volumeScale = model;

    // Use different frequency formulas in depend on a volume model
    switch(m_volumeScale)
    {
    case VOLUME_DMX:
        m_getFreq = &oplModel_dmxFreq;
        m_getVolume = &oplModel_dmxOrigVolume;
        break;

    case VOLUME_DMX_FIXED:
        m_getFreq = &oplModel_dmxFreq;
        m_getVolume = &oplModel_dmxFixedVolume;
        break;

    case VOLUME_APOGEE:
        m_getFreq = &oplModel_apogeeFreq;
        m_getVolume = &oplModel_apogeeOrigVolume;
        break;

    case VOLUME_APOGEE_FIXED:
        m_getFreq = &oplModel_apogeeFreq;
        m_getVolume = &oplModel_apogeeFixedVolume;
        break;

    case VOLUME_9X:
        m_getFreq = &oplModel_9xFreq;
        m_getVolume = &oplModel_9xSB16Volume;
        break;

    case VOLUME_9X_GENERIC_FM:
        m_getFreq = &oplModel_9xFreq;
        m_getVolume = &oplModel_9xGenericVolume;
        break;

    case VOLUME_HMI:
        m_getFreq = &oplModel_hmiFreq;
        m_getVolume = &oplModel_sosNewVolume;
        break;

    case VOLUME_HMI_OLD:
        m_getFreq = &oplModel_hmiFreq;
        m_getVolume = &oplModel_sosOldVolume;
        break;

    case VOLUME_AIL:
        m_getFreq = &oplModel_ailFreq;
        m_getVolume = &oplModel_ailVolume;
        break;

    case VOLUME_MS_ADLIB:
        m_getFreq = &oplModel_msAdLibFreq;
        m_getVolume = &oplModel_msAdLibVolume;
        break;

    case VOLUME_NATIVE:
        m_getFreq = &oplModel_genericFreq;
        m_getVolume = &oplModel_nativeVolume;
        break;

    case VOLUME_RSXX:
        m_getFreq = &oplModel_genericFreq;
        m_getVolume = &oplModel_rsxxVolume;
        break;

    case VOLUME_IMF_CREATOR:
        m_getFreq = &oplModel_hmiFreq;
        m_getVolume = &oplModel_dmxFixedVolume;
        break;

    case VOLUME_OCONNELL:
        m_getFreq = &oplModel_OConnellFreq;
        m_getVolume = &oplModel_OConnellVolume;
        break;

    default:
        m_getFreq = &oplModel_genericFreq;
        m_getVolume = &oplModel_genericVolume;
    }
}

void OPL3::setEmbeddedBank(uint32_t bank)
{
#ifndef DISABLE_EMBEDDED_BANKS
    m_embeddedBank = bank;
    //Embedded banks are supports 128:128 GM set only
    m_insBanks.clear();

    if(bank >= static_cast<uint32_t>(g_embeddedBanksCount))
        return;

    const BanksDump::BankEntry &bankEntry = g_embeddedBanks[m_embeddedBank];
    m_insBankSetup.deepTremolo = ((bankEntry.bankSetup >> 8) & 0x01) != 0;
    m_insBankSetup.deepVibrato = ((bankEntry.bankSetup >> 8) & 0x02) != 0;
    m_insBankSetup.mt32defaults = ((bankEntry.bankSetup >> 8) & 0x04) != 0;
    m_insBankSetup.volumeModel = (bankEntry.bankSetup & 0xFF);
    m_insBankSetup.scaleModulators = false;

    for(int ss = 0; ss < 2; ss++)
    {
        bank_count_t maxBanks = ss ? bankEntry.banksPercussionCount : bankEntry.banksMelodicCount;
        bank_count_t banksOffset = ss ? bankEntry.banksOffsetPercussive : bankEntry.banksOffsetMelodic;

        for(bank_count_t bankID = 0; bankID < maxBanks; bankID++)
        {
            size_t bankIndex = g_embeddedBanksMidiIndex[banksOffset + bankID];
            const BanksDump::MidiBank &bankData = g_embeddedBanksMidi[bankIndex];
            size_t bankMidiIndex = static_cast<size_t>((bankData.msb * 256) + bankData.lsb) + (ss ? static_cast<size_t>(PercussionTag) : 0);
            Bank &bankTarget = m_insBanks[bankMidiIndex];

            for(size_t instId = 0; instId < 128; instId++)
            {
                midi_bank_idx_t instIndex = bankData.insts[instId];
                if(instIndex < 0)
                {
                    bankTarget.ins[instId].flags = OplInstMeta::Flag_NoSound;
                    continue;
                }
                BanksDump::InstrumentEntry instIn = g_embeddedBanksInstruments[instIndex];
                OplInstMeta &instOut = bankTarget.ins[instId];

                adlFromInstrument(instIn, instOut);
            }
        }
    }

    // Reset caches once bank is changed
    clearInstCache();

#else
    ADL_UNUSED(bank);
#endif
}

void OPL3::clearInstCache()
{
    m_insCache.fill(&c_defaultInsCache);
    m_insCacheModified.fill(false);
}

void OPL3::writeReg(size_t chip, uint16_t address, uint8_t value)
{
    m_chips[chip]->writeReg(address, value);
}

void OPL3::writeRegI(size_t chip, uint32_t address, uint32_t value)
{
    m_chips[chip]->writeReg(static_cast<uint16_t>(address), static_cast<uint8_t>(value));
}

void OPL3::writePan(size_t chip, uint32_t address, uint32_t value)
{
    m_chips[chip]->writePan(static_cast<uint16_t>(address), static_cast<uint8_t>(value));
}


void OPL3::noteOff(size_t c)
{
    size_t chip = c / NUM_OF_CHANNELS, cc = c % NUM_OF_CHANNELS;

    if(cc >= OPL3_CHANNELS_RHYTHM_BASE)
    {
        if(m_rhythmMode)
        {
            m_regBD[chip] &= ~(0x10 >> (cc - OPL3_CHANNELS_RHYTHM_BASE));
            writeRegI(chip, 0xBD, m_regBD[chip]);
        }

        return;
    }
    else if(m_currentChipType == OPLChipBase::CHIPTYPE_OPL2 && cc >= NUM_OF_OPL2_CHANNELS)
        return;

    writeRegI(chip, 0xB0 + g_channelsMap[cc], m_keyBlockFNumCache[c] & 0xDF);
}

void OPL3::noteOn(size_t c1, size_t c2, double tone)
{
    size_t chip = c1 / NUM_OF_CHANNELS, cc1 = c1 % NUM_OF_CHANNELS, cc2 = c2 % NUM_OF_CHANNELS;
    size_t chan2 = c2 < m_insCache.size ? c2 : 0;
    uint32_t chn = m_musicMode == MODE_CMF ? g_channelsMapPan[cc1] : g_channelsMap[cc1];
    uint32_t mul_offset = 0;
    uint16_t ftone = 0;
    const OplTimbre *patch1 = m_insCache[c1];
    const OplTimbre *patch2 = m_insCache[chan2];
    char cacheModded[2] = {m_insCacheModified[c1], m_insCacheModified[chan2]};

    if(tone < 0.0)
        tone = 0.0; // Lower than 0 is impossible!

    // Use different frequency formulas in depend on a volume model
    ftone = m_getFreq(tone, &mul_offset);

    if(cc1 < OPL3_CHANNELS_RHYTHM_BASE)
    {
        ftone |= 0x2000u; /* Key-ON [KON] */

        const bool natural_4op = (m_channelCategory[c1] == ChanCat_4op_First);
        const size_t opsCount = natural_4op ? 4 : 2;
        const uint16_t op_addr[4] =
        {
            g_operatorsMap[cc1 * 2 + 0], g_operatorsMap[cc1 * 2 + 1],
            g_operatorsMap[cc2 * 2 + 0], g_operatorsMap[cc2 * 2 + 1]
        };
        const uint32_t ops[4] =
        {
            patch1->modulator_E862 & 0xFF,
            patch1->carrier_E862 & 0xFF,
            patch2->modulator_E862 & 0xFF,
            patch2->carrier_E862 & 0xFF
        };

        for(size_t op = 0; op < opsCount; op++)
        {
            if(op_addr[op] == 0xFFF)
                continue;

            size_t modIdx = op > 1 ? chan2 : c1;

            if(mul_offset > 0)
            {
                uint32_t dt  = ops[op] & 0xF0;
                uint32_t mul = ops[op] & 0x0F;

                if((mul + mul_offset) > 0x0F)
                {
                    mul_offset = 0;
                    mul = 0x0F;
                }

                writeRegI(chip, 0x20 + op_addr[op],  (dt | (mul + mul_offset)) & 0xFF);
                m_insCacheModified[modIdx] = true;
            }
            else if(cacheModded[op > 1 ? 1 : 0])
            {
                writeRegI(chip, 0x20 + op_addr[op],  ops[op] & 0xFF);
                m_insCacheModified[modIdx] = false;
            }
        }
    }

    if(chn != 0xFFF)
    {
        writeRegI(chip , 0xA0 + chn, (ftone & 0xFF));
        writeRegI(chip , 0xB0 + chn, (ftone >> 8) & 0xFF);
        m_keyBlockFNumCache[c1] = (ftone >> 8);
    }

    if(m_rhythmMode && cc1 >= OPL3_CHANNELS_RHYTHM_BASE)
    {
        m_regBD[chip] |= (0x10 >> (cc1 - OPL3_CHANNELS_RHYTHM_BASE));
        writeRegI(chip, 0x0BD, m_regBD[chip]);
        //x |= 0x800; // for test
    }
}

void OPL3::touchNote(size_t c,
                     uint_fast32_t velocity,
                     uint_fast32_t channelVolume,
                     uint_fast32_t channelExpression,
                     uint_fast32_t brightness,
                     bool isDrum)
{
    size_t chip = c / NUM_OF_CHANNELS, cc = c % NUM_OF_CHANNELS;
    const OplTimbre &adli = *m_insCache[c];
    size_t cmf_offset = ((m_musicMode == MODE_CMF) && cc >= OPL3_CHANNELS_RHYTHM_BASE) ? 10 : 0;
    uint16_t o1 = g_operatorsMap[cc * 2 + 0 + cmf_offset];
    uint16_t o2 = g_operatorsMap[cc * 2 + 1 + cmf_offset];
    uint8_t  srcMod = adli.modulator_40,
             srcCar = adli.carrier_40;
    const uint_fast8_t kslMod = srcMod & 0xC0;
    const uint_fast8_t kslCar = srcCar & 0xC0;
    OPLVolume_t vol =
    {
        (uint_fast8_t)(velocity & 0x7F),
        (uint_fast8_t)(channelVolume & 0x7F),
        (uint_fast8_t)(channelExpression & 0x7F),
        (uint_fast8_t)(m_masterVolume & 0x7F),
        1, // 2-op AM
        (uint_fast8_t)adli.feedconn,
        (uint_fast8_t)(srcMod & 0x3F),
        (uint_fast8_t)(srcCar & 0x3F),
        0, // Do Modulator
        0, // Do Carrier
        (unsigned int)isDrum
    };

    uint_fast8_t modulator;
    uint_fast8_t carrier;

    static const bool do_ops[10][2] =
    {
        { false, true },  /* 2 op FM */
        { true,  true },  /* 2 op AM */
        { false, false }, /* 4 op FM-FM ops 1&2 */
        { true,  false }, /* 4 op AM-FM ops 1&2 */
        { false, true  }, /* 4 op FM-AM ops 1&2 */
        { true,  false }, /* 4 op AM-AM ops 1&2 */
        { false, true  }, /* 4 op FM-FM ops 3&4 */
        { false, true  }, /* 4 op AM-FM ops 3&4 */
        { false, true  }, /* 4 op FM-AM ops 3&4 */
        { true,  true  }  /* 4 op AM-AM ops 3&4 */
    };

    if(m_currentChipType == OPLChipBase::CHIPTYPE_OPL2 && m_channelCategory[c] == ChanCat_None)
        return; // Do nothing

    if(m_channelCategory[c] == ChanCat_Regular ||
       m_channelCategory[c] == ChanCat_Rhythm_Bass)
    {
        vol.voiceMode = adli.feedconn & 1; // 2-op FM or 2-op AM
    }
    else if(m_channelCategory[c] == ChanCat_4op_First ||
            m_channelCategory[c] == ChanCat_4op_Second)
    {
        const OplTimbre *i0, *i1;

        if(m_channelCategory[c] == ChanCat_4op_First)
        {
            i0 = &adli;
            i1 = m_insCache[c + 3];
            vol.voiceMode = OPLVoice_MODE_4op_1_2_FM_FM; // 4-op xx-xx ops 1&2
        }
        else
        {
            i0 = m_insCache[c - 3];
            i1 = &adli;
            vol.voiceMode = OPLVoice_MODE_4op_3_4_FM_FM; // 4-op xx-xx ops 3&4
        }

        vol.voiceMode += (i0->feedconn & 1) + (i1->feedconn & 1) * 2;
    }

    vol.doMod = do_ops[vol.voiceMode][0] || m_scaleModulators;
    vol.doCar = do_ops[vol.voiceMode][1] || m_scaleModulators;

    // ------ Mix volumes and compute average ------
    m_getVolume(&vol);

    if(brightness != 127 && !isDrum)
    {
        brightness = oplModels_xgBrightnessToOPL(brightness);

        if(!vol.doMod)
            vol.tlMod = 63 - brightness + (brightness * vol.tlMod) / 63;

        if(!vol.doCar)
            vol.tlCar = 63 - brightness + (brightness * vol.tlCar) / 63;
    }

    modulator = (kslMod & 0xC0) | (vol.tlMod & 63);
    carrier = (kslCar & 0xC0) | (vol.tlCar & 63);

    if(o1 != 0xFFF)
        writeRegI(chip, 0x40 + o1, modulator);

    if(o2 != 0xFFF)
        writeRegI(chip, 0x40 + o2, carrier);
}

void OPL3::setPatch(size_t c, const OplTimbre *instrument)
{
    size_t chip = c / NUM_OF_CHANNELS, cc = c % NUM_OF_CHANNELS, cmf_offset;
    uint32_t x, y;
    uint16_t o1, o2, fbconn_reg = 0x00;
    static const uint8_t data[4] = {0x20, 0x60, 0x80, 0xE0};
    uint8_t fbconn = 0;

    if(m_insCache[c] == instrument)
        return; // Already up to date!

    m_insCache[c] = instrument;
    m_insCacheModified[c] = false;

    cmf_offset = ((m_musicMode == MODE_CMF) && (cc >= OPL3_CHANNELS_RHYTHM_BASE)) ? 10 : 0;
    o1 = g_operatorsMap[cc * 2 + 0 + cmf_offset];
    o2 = g_operatorsMap[cc * 2 + 1 + cmf_offset];
    x = instrument->modulator_E862, y = instrument->carrier_E862;
    fbconn_reg = 0x00;

    if(m_channelCategory[c] == ChanCat_Regular && (m_volumeScale == VOLUME_DMX || m_volumeScale == VOLUME_DMX_FIXED))
    {
        // Also write dummy volume value
        if(o1 != 0xFFF)
            writeRegI(chip, 0x40 + o1, instrument->modulator_40);

        if(o2 != 0xFFF)
            writeRegI(chip, 0x40 + o2, instrument->carrier_40);
    }

    for(size_t a = 0; a < 4; ++a, x >>= 8, y >>= 8)
    {
        if(o1 != 0xFFF)
            writeRegI(chip, data[a] + o1, x & 0xFF);

        if(o2 != 0xFFF)
            writeRegI(chip, data[a] + o2, y & 0xFF);
    }

    if(g_channelsMapFBConn[cc] != 0xFFF)
    {
        fbconn |= instrument->feedconn;
        fbconn_reg = 0xC0 + g_channelsMapFBConn[cc];
    }

    if(m_currentChipType != OPLChipBase::CHIPTYPE_OPL2 && g_channelsMapPan[cc] != 0xFFF)
    {
        fbconn |= (m_regC0[c] & OPL_PANNING_BOTH);
        if(!fbconn_reg)
            fbconn_reg = 0xC0 + g_channelsMapPan[cc];
    }

    if(fbconn_reg != 0x00)
        writeRegI(chip, fbconn_reg, fbconn);
}

void OPL3::setPan(size_t c, uint8_t value)
{
    size_t chip = c / NUM_OF_CHANNELS, cc = c % NUM_OF_CHANNELS;

    if(m_currentChipType == OPLChipBase::CHIPTYPE_OPL2)
    {
        m_regC0[c] = OPL_PANNING_BOTH;
        return; // OPL2 chip doesn't support panning at all
    }

    if(g_channelsMapPan[cc] != 0xFFF)
    {
#ifndef ENABLE_HW_OPL_DOS
        if (m_softPanningSup && m_softPanning)
        {
            writePan(chip, g_channelsMapPan[cc], value);
            m_regC0[c] = OPL_PANNING_BOTH;
            writeRegI(chip, 0xC0 + g_channelsMapPan[cc], m_insCache[c]->feedconn | OPL_PANNING_BOTH);
        }
        else
        {
#endif
            uint8_t panning = 0;
            if(value  < 64 + 16) panning |= OPL_PANNING_LEFT;
            if(value >= 64 - 16) panning |= OPL_PANNING_RIGHT;
            m_regC0[c] = panning;
            writePan(chip, g_channelsMapPan[cc], 64);
            writeRegI(chip, 0xC0 + g_channelsMapPan[cc], m_insCache[c]->feedconn | panning);
#ifndef ENABLE_HW_OPL_DOS
        }
#endif
    }
}

void OPL3::silenceAll() // Silence all OPL channels.
{
    if(m_chips.empty())
        return; // Can't since no chips initialized

    commitDeepFlags(); // If rhythm-mode is active, this will off all the rhythm keys

    for(size_t c = 0; c < m_numChannels; ++c)
    {
        size_t chip = c / NUM_OF_CHANNELS, cc = c % NUM_OF_CHANNELS;
        size_t cmf_offset = ((m_musicMode == MODE_CMF) && cc >= OPL3_CHANNELS_RHYTHM_BASE) ? 10 : 0;
        uint16_t o1 = g_operatorsMap[cc * 2 + 0 + cmf_offset];
        uint16_t o2 = g_operatorsMap[cc * 2 + 1 + cmf_offset];

        noteOff(c);

        if(o1 != 0xFFF)
        {
            writeRegI(chip, 0x40 + o1, 0x3F);
            writeRegI(chip, 0x80 + o1, 0xFF);
        }

        if(o2 != 0xFFF)
        {
            writeRegI(chip, 0x40 + o2, 0x3F);
            writeRegI(chip, 0x80 + o1, 0xFF);
        }

        m_insCacheModified[c] = true;
    }
}

void OPL3::updateChannelCategories()
{
    if(m_numFourOps > m_numChips * 6)
        m_numFourOps = m_numChips * 6; // Can't set more than chips can offer!

    const uint32_t fours = (m_currentChipType != OPLChipBase::CHIPTYPE_OPL2) ? m_numFourOps : 0;

    for(uint32_t chip = 0, fours_left = fours; chip < m_numChips; ++chip)
    {
        m_regBD[chip] = (m_deepTremoloMode * 0x80 + m_deepVibratoMode * 0x40 + m_rhythmMode * 0x20);
        writeRegI(chip, 0x0BD, m_regBD[chip]);
        uint32_t fours_this_chip = std::min(fours_left, static_cast<uint32_t>(6u));
        if(m_currentChipType != OPLChipBase::CHIPTYPE_OPL2)
            writeRegI(chip, 0x104, (1 << fours_this_chip) - 1);
        fours_left -= fours_this_chip;
    }

    for(size_t p = 0, a = 0, n = m_numChips; a < n; ++a)
    {
        for(size_t b = 0; b < OPL3_CHANNELS_RHYTHM_BASE; ++b, ++p)
        {
            if(m_currentChipType == OPLChipBase::CHIPTYPE_OPL2 && b >= NUM_OF_OPL2_CHANNELS)
                m_channelCategory[p] = ChanCat_None;
            else
                m_channelCategory[p] = ChanCat_Regular;

            if(m_rhythmMode && b >= 6 && b < 9)
                m_channelCategory[p] = ChanCat_Rhythm_Secondary;
        }

        if(!m_rhythmMode)
        {
            for(size_t b = 0; b < NUM_OF_RM_CHANNELS; ++b)
                m_channelCategory[p++] = ChanCat_Rhythm_Secondary;
        }
        else
        {
            for(size_t b = 0; b < NUM_OF_RM_CHANNELS; ++b)
                m_channelCategory[p++] = (ChanCat_Rhythm_Bass + b);
        }
    }

    uint32_t nextfour = 0;
    for(uint32_t a = 0; a < fours; ++a)
    {
        m_channelCategory[nextfour] = ChanCat_4op_First;
        m_channelCategory[nextfour + 3] = ChanCat_4op_Second;

        switch(a % 6)
        {
        case 0:
        case 1:
            nextfour += 1;
            break;
        case 2:
            nextfour += 9 - 2;
            break;
        case 3:
        case 4:
            nextfour += 1;
            break;
        case 5:
            nextfour += NUM_OF_CHANNELS - 9 - 2;
            break;
        }
    }

/**/
/*
    In two-op mode, channels 0..8 go as follows:
                  Op1[port]  Op2[port]
      Channel 0:  00  00     03  03
      Channel 1:  01  01     04  04
      Channel 2:  02  02     05  05
      Channel 3:  06  08     09  0B
      Channel 4:  07  09     10  0C
      Channel 5:  08  0A     11  0D
      Channel 6:  12  10     15  13
      Channel 7:  13  11     16  14
      Channel 8:  14  12     17  15
    In four-op mode, channels 0..8 go as follows:
                  Op1[port]  Op2[port]  Op3[port]  Op4[port]
      Channel 0:  00  00     03  03     06  08     09  0B
      Channel 1:  01  01     04  04     07  09     10  0C
      Channel 2:  02  02     05  05     08  0A     11  0D
      Channel 3:  CHANNEL 0 SECONDARY
      Channel 4:  CHANNEL 1 SECONDARY
      Channel 5:  CHANNEL 2 SECONDARY
      Channel 6:  12  10     15  13
      Channel 7:  13  11     16  14
      Channel 8:  14  12     17  15
     Same goes principally for channels 9-17 respectively.
    */
}

void OPL3::commitDeepFlags()
{
    for(size_t chip = 0; chip < m_numChips; ++chip)
    {
        m_regBD[chip] = (m_deepTremoloMode * 0x80 + m_deepVibratoMode * 0x40 + m_rhythmMode * 0x20);
        writeRegI(chip, 0x0BD, m_regBD[chip]);
    }
}

void OPL3::setVolumeScaleModel(ADLMIDI_VolumeModels volumeModel)
{
    switch(volumeModel)
    {
    default:
    case ADLMIDI_VolumeModel_AUTO://Do nothing until restart playing
        break;

    case ADLMIDI_VolumeModel_Generic:
        setFrequencyModel(OPL3::VOLUME_Generic);
        break;

    case ADLMIDI_VolumeModel_NativeOPL3:
        setFrequencyModel(OPL3::VOLUME_NATIVE);
        break;

    case ADLMIDI_VolumeModel_DMX:
        setFrequencyModel(OPL3::VOLUME_DMX);
        break;

    case ADLMIDI_VolumeModel_APOGEE:
        setFrequencyModel(OPL3::VOLUME_APOGEE);
        break;

    case ADLMIDI_VolumeModel_9X:
        setFrequencyModel(OPL3::VOLUME_9X);
        break;

    case ADLMIDI_VolumeModel_DMX_Fixed:
        setFrequencyModel(OPL3::VOLUME_DMX_FIXED);
        break;

    case ADLMIDI_VolumeModel_APOGEE_Fixed:
        setFrequencyModel(OPL3::VOLUME_APOGEE_FIXED);
        break;

    case ADLMIDI_VolumeModel_AIL:
        setFrequencyModel(OPL3::VOLUME_AIL);
        break;

    case ADLMIDI_VolumeModel_9X_GENERIC_FM:
        setFrequencyModel(OPL3::VOLUME_9X_GENERIC_FM);
        break;

    case ADLMIDI_VolumeModel_HMI:
        setFrequencyModel(OPL3::VOLUME_HMI);
        break;

    case ADLMIDI_VolumeModel_HMI_OLD:
        setFrequencyModel(OPL3::VOLUME_HMI_OLD);
        break;

    case ADLMIDI_VolumeModel_MS_ADLIB:
        setFrequencyModel(OPL3::VOLUME_MS_ADLIB);
        break;

    case ADLMIDI_VolumeModel_IMF_Creator:
        setFrequencyModel(OPL3::VOLUME_IMF_CREATOR);
        break;

    case ADLMIDI_VolumeModel_OConnell:
        setFrequencyModel(OPL3::VOLUME_OCONNELL);
        break;
    }
}

ADLMIDI_VolumeModels OPL3::getVolumeScaleModel()
{
    switch(m_volumeScale)
    {
    default:
    case OPL3::VOLUME_Generic:
        return ADLMIDI_VolumeModel_Generic;
    case OPL3::VOLUME_RSXX:
    case OPL3::VOLUME_NATIVE:
        return ADLMIDI_VolumeModel_NativeOPL3;
    case OPL3::VOLUME_DMX:
        return ADLMIDI_VolumeModel_DMX;
    case OPL3::VOLUME_APOGEE:
        return ADLMIDI_VolumeModel_APOGEE;
    case OPL3::VOLUME_9X:
        return ADLMIDI_VolumeModel_9X;
    case OPL3::VOLUME_DMX_FIXED:
        return ADLMIDI_VolumeModel_DMX_Fixed;
    case OPL3::VOLUME_APOGEE_FIXED:
        return ADLMIDI_VolumeModel_APOGEE_Fixed;
    case OPL3::VOLUME_AIL:
        return ADLMIDI_VolumeModel_AIL;
    case OPL3::VOLUME_9X_GENERIC_FM:
        return ADLMIDI_VolumeModel_9X_GENERIC_FM;
    case OPL3::VOLUME_HMI:
        return ADLMIDI_VolumeModel_HMI;
    case OPL3::VOLUME_HMI_OLD:
        return ADLMIDI_VolumeModel_HMI_OLD;
    case OPL3::VOLUME_MS_ADLIB:
        return ADLMIDI_VolumeModel_MS_ADLIB;
    case OPL3::VOLUME_IMF_CREATOR:
        return ADLMIDI_VolumeModel_IMF_Creator;
    case OPL3::VOLUME_OCONNELL:
        return ADLMIDI_VolumeModel_OConnell;
    }
}

void OPL3::clearChips()
{
    for(size_t i = 0; i < m_chips.size; i++)
        m_chips[i].reset(NULL);

    m_chips.clear();
}

void OPL3::reset(int emulator, unsigned long PCM_RATE, void *audioTickHandler)
{
    bool rebuild_needed = m_curState.cmp(emulator, m_numChips);

    if(rebuild_needed)
        clearChips();

#if !defined(ADLMIDI_AUDIO_TICK_HANDLER)
    (void)audioTickHandler;
#endif



    if(rebuild_needed)
    {
        m_insCache.clear();
        m_insCacheModified.clear();
        m_keyBlockFNumCache.clear();
        m_regBD.clear();
        m_regC0.clear();
        m_channelCategory.clear();
        m_chips.resize(m_numChips);
    }
    else
    {
        if(!m_chips.empty())
            silenceAll(); // Ensure no junk will be played after bank change

        m_insCache.fill(&c_defaultInsCache);
        m_insCacheModified.fill(false);
        m_channelCategory.fill(0);
        m_keyBlockFNumCache.fill(0);
        m_regBD.fill(0);
        m_regC0.fill(OPL_PANNING_BOTH);
    }

#ifdef ADLMIDI_ENABLE_HW_SERIAL
    if(emulator >= 0) // If less than zero - it's hardware synth!
        m_serial = false;
#endif

    if(rebuild_needed)
    {
        m_numChannels = m_numChips * NUM_OF_CHANNELS;

        m_insCache.resize_fill(m_numChannels, &c_defaultInsCache);
        m_insCacheModified.resize_fill(m_numChannels, false);
        m_channelCategory.resize_fill(m_numChannels, 0);
        m_keyBlockFNumCache.resize_fill(m_numChannels, 0);
        m_regBD.resize_fill(m_numChips, 0);
        m_regC0.resize_fill(m_numChips * m_numChannels, OPL_PANNING_BOTH);
    }

    if(!rebuild_needed)
    {
        bool newRate = m_curState.cmp_rate(PCM_RATE);

        for(size_t i = 0; i < m_numChips; ++i)
        {
            if(newRate)
                m_chips[i]->setRate(PCM_RATE);

            initChip(i);
        }
    }
    else for(size_t i = 0; i < m_numChips; ++i)
    {
#ifdef ADLMIDI_ENABLE_HW_SERIAL
        if(emulator < 0)
        {
            OPL_SerialPort *serial = new OPL_SerialPort;
            serial->connectPort(m_serialName, m_serialBaud, m_serialProtocol);
            m_chips[i].reset(serial);
            initChip(i);
            break; // Only one REAL chip!
        }
#endif

        OPLChipBase *chip;
#ifdef ENABLE_HW_OPL_DOS
        chip = new DOS_HW_OPL();

#else // ENABLE_HW_OPL_DOS
        switch(emulator)
        {
        default:
            assert(false);
            abort();
#ifndef ADLMIDI_DISABLE_NUKED_EMULATOR
        case ADLMIDI_EMU_NUKED: /* Latest Nuked OPL3 */
            chip = new NukedOPL3;
            break;
        case ADLMIDI_EMU_NUKED_FAST: /* Nuked OPL3 Fast modified and optimised by tgies */
            chip = new NukedOPL3v174;
            break;
        case ADLMIDI_EMU_NUKED_OPL2_LITE: /* Nuked OPL2 Lite */
            chip = new NukedOPL2;
            break;
        case ADLMIDI_EMU_NUKED_CQM: /* Nuked CQM */
            chip = new NukedCQM;
            break;
#endif
#ifndef ADLMIDI_DISABLE_DOSBOX_EMULATOR
        case ADLMIDI_EMU_DOSBOX:
            chip = new DosBoxOPL3;
            break;

        case ADLMIDI_EMU_DOSBOX_OPL2:
            chip = new DosBoxOPL2;
            break;
#endif
#ifndef ADLMIDI_DISABLE_OPAL_EMULATOR
        case ADLMIDI_EMU_OPAL:
            chip = new OpalOPL3;
            break;
#endif
#ifndef ADLMIDI_DISABLE_JAVA_EMULATOR
        case ADLMIDI_EMU_JAVA:
            chip = new JavaOPL3;
            break;
#endif
#ifndef ADLMIDI_DISABLE_ESFMU_EMULATOR
        case ADLMIDI_EMU_ESFMu:
            chip = new ESFMuOPL3;
            break;
#endif
#ifndef ADLMIDI_DISABLE_MAME_OPL2_EMULATOR
        case ADLMIDI_EMU_MAME_OPL2:
            chip = new MameOPL2;
            break;
#endif
#ifndef ADLMIDI_DISABLE_YMFM_EMULATOR
        case ADLMIDI_EMU_YMFM_OPL2:
            chip = new YmFmOPL2;
            break;
        case ADLMIDI_EMU_YMFM_OPL3:
            chip = new YmFmOPL3;
            break;
#endif
#ifdef ADLMIDI_ENABLE_OPL2_LLE_EMULATOR
        case ADLMIDI_EMU_NUKED_OPL2_LLE:
            chip = new Ym3812LLEOPL2;
            break;
#endif
#ifdef ADLMIDI_ENABLE_OPL3_LLE_EMULATOR
        case ADLMIDI_EMU_NUKED_OPL3_LLE:
            chip = new Ymf262LLEOPL3;
            break;
#endif
        }
#endif // ENABLE_HW_OPL_DOS

        m_chips[i].reset(chip);
        chip->setChipId((uint32_t)i);
        chip->setRate((uint32_t)PCM_RATE);

#ifndef ENABLE_HW_OPL_DOS
        if(m_runAtPcmRate)
            chip->setRunningAtPcmRate(true);
#endif

#   if defined(ADLMIDI_AUDIO_TICK_HANDLER) && !defined(ENABLE_HW_OPL_DOS)
        chip->setAudioTickHandlerInstance(audioTickHandler);
#   endif

        initChip(i);
    }

    updateChannelCategories();
    silenceAll();
}

void OPL3::toggleOPL3(bool en)
{
    if(m_currentChipType == (int)OPLChipBase::CHIPTYPE_OPL2)
        return; // Impossible to do on OPL2 chips

    const uint16_t enOPL3 = en ? 1u : 0u;
    const uint16_t data_opl3[] =
    {
        0x004, 96, 0x004, 128,        // Pulse timer
        0x105, 0, 0x105, 1, 0x105, 0, // Pulse OPL3 enable
        0x001, 32, 0x105, enOPL3,          // Enable wave, OPL3 extensions
        0x08, 0                       // CSW/Note Sel
    };
    const size_t data_opl3_size = sizeof(data_opl3) / sizeof(uint16_t);

    for(size_t c = 0; c < m_numChips; ++c)
    {
        for(size_t a = 0; a < data_opl3_size; a += 2)
            writeRegI(c, data_opl3[a], (data_opl3[a + 1]));
    }
}

void OPL3::initChip(size_t chip)
{
    static const uint16_t data_opl3[] =
    {
        0x004, 96, 0x004, 128,        // Pulse timer
        0x105, 0, 0x105, 1, 0x105, 0, // Pulse OPL3 enable
        0x001, 32, 0x105, 1,          // Enable wave, OPL3 extensions
        0x08, 0                       // CSW/Note Sel
    };
    static const size_t data_opl3_size = sizeof(data_opl3) / sizeof(uint16_t);

    static const uint16_t data_opl2[] =
    {
        0x004, 96, 0x004, 128,          // Pulse timer
        0x001, 32,                      // Enable wave
        0x08, 0                         // CSW/Note Sel
    };
    static const size_t data_opl2_size = sizeof(data_opl2) / sizeof(uint16_t);

    // Report does emulator/interface supports full-panning stereo or not
    if(chip == 0)
    {
        m_softPanningSup = m_chips[chip]->hasFullPanning();
        m_currentChipType = (int)m_chips[chip]->chipType();
        m_perChipChannels = OPL3_CHANNELS_RHYTHM_BASE;

        if(m_currentChipType == OPLChipBase::CHIPTYPE_OPL2)
        {
            m_perChipChannels = NUM_OF_OPL2_CHANNELS;
            m_numFourOps = 0; // Can't have 4ops on OPL2 chip
        }
    }

    /* Clean-up channels from any playing junk sounds */
    for(size_t a = 0; a < m_perChipChannels; ++a)
    {
        writeRegI(chip, 0x20 + g_operatorsMap[a * 2], 0x00);
        writeRegI(chip, 0x20 + g_operatorsMap[(a * 2) + 1], 0x00);
        writeRegI(chip, 0xA0 + g_channelsMap[a], 0x00);
        writeRegI(chip, 0xB0 + g_channelsMap[a], 0x00);
    }

    if(m_currentChipType == OPLChipBase::CHIPTYPE_OPL2)
    {
        for(size_t a = 0; a < data_opl2_size; a += 2)
            writeRegI(chip, data_opl2[a], (data_opl2[a + 1]));
    }
    else
    {
        for(size_t a = 0; a < data_opl3_size; a += 2)
            writeRegI(chip, data_opl3[a], (data_opl3[a + 1]));
    }
}

#ifdef ADLMIDI_ENABLE_HW_SERIAL
void OPL3::resetSerial(const std::string &serialName, unsigned int baud, unsigned int protocol)
{
    m_serial = true;
    m_serialName = serialName;
    m_serialBaud = baud;
    m_serialProtocol = protocol;
    m_numChips = 1; // Only one chip!
    m_softPanning = false; // Soft-panning doesn't work on hardware
    reset(-1, 0, NULL);
}
#endif
