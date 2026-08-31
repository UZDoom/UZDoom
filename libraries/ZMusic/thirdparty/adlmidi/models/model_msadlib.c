/*
 * OPL2/OPL3 models library - a set of various conversion models for OPL-family chips
 *
 * Copyright (c) 2025-2026 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "opl_models.h"

/***************************************************************
 *                  MS AdLib frequency model                   *
 ***************************************************************/

static uint16_t s_msAdLibFreqTable[25][12] =
{
    {0x157, 0x16C, 0x181, 0x198, 0x1B1, 0x1CB, 0x1E6, 0x203, 0x222, 0x243, 0x266, 0x28A},
    {0x158, 0x16D, 0x183, 0x19A, 0x1B2, 0x1CC, 0x1E8, 0x205, 0x224, 0x245, 0x267, 0x28C},
    {0x159, 0x16D, 0x183, 0x19A, 0x1B3, 0x1CD, 0x1E9, 0x206, 0x225, 0x246, 0x269, 0x28D},
    {0x15A, 0x16E, 0x184, 0x19B, 0x1B4, 0x1CE, 0x1EA, 0x207, 0x226, 0x247, 0x26A, 0x28F},
    {0x15A, 0x16F, 0x185, 0x19C, 0x1B5, 0x1CF, 0x1EB, 0x208, 0x227, 0x248, 0x26B, 0x291},
    {0x15B, 0x170, 0x186, 0x19D, 0x1B6, 0x1D0, 0x1EC, 0x20A, 0x229, 0x24A, 0x26D, 0x292},
    {0x15C, 0x171, 0x187, 0x19F, 0x1B7, 0x1D2, 0x1ED, 0x20B, 0x22A, 0x24B, 0x26E, 0x294},
    {0x15D, 0x172, 0x188, 0x19F, 0x1B8, 0x1D3, 0x1EF, 0x20C, 0x22C, 0x24D, 0x270, 0x295},
    {0x15E, 0x173, 0x189, 0x1A0, 0x1B9, 0x1D4, 0x1F0, 0x20D, 0x22D, 0x24E, 0x271, 0x297},
    {0x15F, 0x174, 0x18A, 0x1A1, 0x1BA, 0x1D5, 0x1F1, 0x20F, 0x22E, 0x250, 0x273, 0x299},
    {0x15F, 0x174, 0x18B, 0x1A2, 0x1BB, 0x1D6, 0x1F2, 0x210, 0x22F, 0x251, 0x274, 0x29A},
    {0x160, 0x175, 0x18C, 0x1A3, 0x1BC, 0x1D7, 0x1F3, 0x211, 0x231, 0x252, 0x276, 0x29C},
    {0x161, 0x176, 0x18D, 0x1A4, 0x1BD, 0x1D8, 0x1F4, 0x212, 0x232, 0x254, 0x277, 0x29D},
    {0x162, 0x177, 0x18E, 0x1A5, 0x1BF, 0x1D9, 0x1F6, 0x214, 0x234, 0x255, 0x279, 0x29F},
    {0x163, 0x178, 0x18E, 0x1A6, 0x1C0, 0x1DA, 0x1F7, 0x215, 0x235, 0x257, 0x27A, 0x2A0},
    {0x164, 0x179, 0x18F, 0x1A7, 0x1C1, 0x1DB, 0x1F8, 0x216, 0x236, 0x258, 0x27C, 0x2A2},
    {0x164, 0x17A, 0x190, 0x1A8, 0x1C2, 0x1DD, 0x1F9, 0x217, 0x237, 0x259, 0x27D, 0x2A3},
    {0x165, 0x17B, 0x191, 0x1A9, 0x1C3, 0x1DE, 0x1FA, 0x219, 0x239, 0x25B, 0x27F, 0x2A5},
    {0x166, 0x17B, 0x192, 0x1AA, 0x1C4, 0x1DF, 0x1FB, 0x21A, 0x23A, 0x25C, 0x280, 0x2A7},
    {0x167, 0x17C, 0x193, 0x1AB, 0x1C5, 0x1E0, 0x1FD, 0x21B, 0x23B, 0x25E, 0x282, 0x2A8},
    {0x168, 0x17D, 0x194, 0x1AC, 0x1C6, 0x1E1, 0x1FE, 0x21C, 0x23C, 0x25F, 0x283, 0x2AA},
    {0x168, 0x17E, 0x195, 0x1AD, 0x1C7, 0x1E2, 0x1FF, 0x21D, 0x23E, 0x260, 0x285, 0x2AB},
    {0x169, 0x17F, 0x196, 0x1AE, 0x1C8, 0x1E3, 0x200, 0x21F, 0x23F, 0x262, 0x286, 0x2AD},
    {0x16A, 0x180, 0x197, 0x1AF, 0x1C9, 0x1E4, 0x201, 0x220, 0x241, 0x263, 0x288, 0x2AF},
    {0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x221, 0x242, 0x264, 0x289, 0x2B0},
};

static const int32_t MSADLIB_NR_STEP_PITCH = 25;
static const int32_t MSADLIB_PRANGE = 50;

#define OPL_HIWORD(x)  ((uint16_t)((((uint32_t)x) >> 16) & 0xFFFF))
#define OPL_LOWORD(x)  ((uint16_t)((uint32_t)x & 0xFFFF))
#define OPL_LOBYTE(x)  ((uint8_t)((uint16_t)x & 0xFF))
#define OPL_HIBYTE(x)  ((uint8_t)(((uint16_t)x >> 8) & 0xFF))

uint16_t oplModel_msAdLibFreq(double tone, uint32_t *mul_offset)
{
    int_fast32_t note, halfToneoffset, octave, t1, t2, delta;
    uint32_t dw;
    uint16_t bend, freq;
    double bendDec;

    *mul_offset = 0;

    note = (int_fast32_t)(tone);
    bendDec = tone - (int_fast32_t)tone; /* 0.0 ± 1.0 - one halftone */

    if(bendDec > 0.5)
    {
        note += 1;
        bendDec -= 1.0;
    }

    bend = (uint16_t)((bendDec * 4096) + 8192); /* convert to MIDI standard value */

    if(note < 12)
        note = 0;
    else
        note -= 12;

    dw = (uint32_t)((int32_t)((int16_t)bend - 0x2000) * MSADLIB_PRANGE);
    t1 = (int16_t)((OPL_LOBYTE(OPL_HIWORD(dw)) << 8) | OPL_HIBYTE(OPL_LOWORD(dw))) >> 5;

    if(t1 < 0)
    {
        t2 = MSADLIB_NR_STEP_PITCH - 1 - t1;
        halfToneoffset = -(t2 / MSADLIB_NR_STEP_PITCH);
        delta = (t2 - MSADLIB_NR_STEP_PITCH + 1) % MSADLIB_NR_STEP_PITCH;
        if(delta)
            delta = MSADLIB_NR_STEP_PITCH - delta;
    }
    else
    {
        halfToneoffset = t1 / MSADLIB_NR_STEP_PITCH;
        delta = t1 % MSADLIB_NR_STEP_PITCH;
    }

    note += halfToneoffset;
    freq = s_msAdLibFreqTable[delta][note % 12];
    octave = note / 12;

    while(octave > 7)
    {
        ++(*mul_offset);
        --octave;
    }

    return freq | (octave << 10);
}


/***************************************************************
 *                  MS AdLib volume formula                    *
 ***************************************************************/

static const uint8_t s_msadlib_volume_table[128] =
{
    0,   0,   65,  65,  66,  66,  67,  67,  /* 0 - 7 */
    68,  68,  69,  69,  70,  70,  71,  71,  /* 8 - 15 */
    72,  72,  73,  73,  74,  74,  75,  75,  /* 16 - 23 */
    76,  76,  77,  77,  78,  78,  79,  79,  /* 24 - 31 */
    80,  80,  81,  81,  82,  82,  83,  83,  /* 32 - 39 */
    84,  84,  85,  85,  86,  86,  87,  87,  /* 40 - 47 */
    88,  88,  89,  89,  90,  90,  91,  91,  /* 48 - 55 */
    92,  92,  93,  93,  94,  94,  95,  95,  /* 56 - 63 */
    96,  96,  97,  97,  98,  98,  99,  99,  /* 64 - 71 */
    100, 100, 101, 101, 102, 102, 103, 103,  /* 72 - 79 */
    104, 104, 105, 105, 106, 106, 107, 107,  /* 80 - 87 */
    108, 108, 109, 109, 110, 110, 111, 111,  /* 88 - 95 */
    112, 112, 113, 113, 114, 114, 115, 115,  /* 96 - 103 */
    116, 116, 117, 117, 118, 118, 119, 119,  /* 104 - 111 */
    120, 120, 121, 121, 122, 122, 123, 123,  /* 112 - 119 */
    124, 124, 125, 125, 126, 126, 127, 127   /* 120 - 127 */
};

void oplModel_msAdLibVolume(struct OPLVolume_t *v)
{
    uint_fast32_t volume, outVol;

    volume = (v->chVol * v->chExpr * v->masterVolume) / 16129;
    volume = (v->vel * volume) / 127;

    if(volume > 127)
        volume = 127;

    volume = s_msadlib_volume_table[volume];

    if(v->doMod)
    {
        outVol = 63 - (v->tlMod & 0x3f);
        outVol *= volume;
        outVol += (outVol + 0x7F);
        v->tlMod = 63 - outVol / (2 * 0x7F);
    }

    if(v->doCar)
    {
        outVol = 63 - (v->tlCar & 0x3f);
        outVol *= volume;
        outVol += (outVol + 0x7F);
        v->tlCar = 63 - outVol / (2 * 0x7F);
    }
}
