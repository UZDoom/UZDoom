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

#include <stddef.h>
#include "opl_models.h"

/***************************************************************
 *         HMI Sound Operating System frequency model          *
 ***************************************************************/

static const size_t s_hmi_freqtable_size = 103;
static uint_fast32_t s_hmi_freqtable[103] =
{
    0x0157, 0x016B, 0x0181, 0x0198, 0x01B0, 0x01CA, 0x01E5, 0x0202, 0x0220, 0x0241, 0x0263, 0x0287,
    0x0557, 0x056B, 0x0581, 0x0598, 0x05B0, 0x05CA, 0x05E5, 0x0602, 0x0620, 0x0641, 0x0663, 0x0687,
    0x0957, 0x096B, 0x0981, 0x0998, 0x09B0, 0x09CA, 0x09E5, 0x0A02, 0x0A20, 0x0A41, 0x0A63, 0x0A87,
    0x0D57, 0x0D6B, 0x0D81, 0x0D98, 0x0DB0, 0x0DCA, 0x0DE5, 0x0E02, 0x0E20, 0x0E41, 0x0E63, 0x0E87,
    0x1157, 0x116B, 0x1181, 0x1198, 0x11B0, 0x11CA, 0x11E5, 0x1202, 0x1220, 0x1241, 0x1263, 0x1287,
    0x1557, 0x156B, 0x1581, 0x1598, 0x15B0, 0x15CA, 0x15E5, 0x1602, 0x1620, 0x1641, 0x1663, 0x1687,
    0x1957, 0x196B, 0x1981, 0x1998, 0x19B0, 0x19CA, 0x19E5, 0x1A02, 0x1A20, 0x1A41, 0x1A63, 0x1A87,
    0x1D57, 0x1D6B, 0x1D81, 0x1D98, 0x1DB0, 0x1DCA, 0x1DE5, 0x1E02, 0x1E20, 0x1E41, 0x1E63, 0x1E87,
    0x1EAE, 0x1EB7, 0x1F02, 0x1F30, 0x1F60, 0x1F94, 0x1FCA
};

const size_t s_hmi_bendtable_size = 12;
static uint_fast32_t s_hmi_bendtable[12] =
{
    0x144, 0x132, 0x121, 0x110, 0x101, 0xf8, 0xe5, 0xd8, 0xcc, 0xc1, 0xb6, 0xac
};

#define hmi_range_fix(formula, maxVal) \
    ( \
        (formula) < 0 ? \
        0 : \
        ( \
            (formula) >= (int32_t)maxVal ? \
            (int32_t)maxVal : \
            (formula) \
        )\
    )

static uint_fast32_t s_hmi_bend_calc(uint_fast32_t bend, int_fast32_t note)
{
    const int_fast32_t midi_bend_range = 1;
    uint_fast32_t bendFactor, outFreq, fmOctave, fmFreq, newFreq, idx;
    int_fast32_t noteMod12;

    note -= 12;
    /*
    while(doNote >= 12) // ugly way to MOD 12
        doNote -= 12;
    */
    noteMod12 = (note % 12);

    outFreq = s_hmi_freqtable[note];

    fmOctave = outFreq & 0x1c00;
    fmFreq = outFreq & 0x3ff;

    if(bend < 64)
    {
        bendFactor = ((63 - bend) * 1000) >> 6;

        idx = hmi_range_fix(note - midi_bend_range, s_hmi_freqtable_size);
        newFreq = outFreq - s_hmi_freqtable[idx];

        if(newFreq > 719)
        {
            newFreq = fmFreq - s_hmi_bendtable[midi_bend_range - 1];
            newFreq &= 0x3ff;
        }

        newFreq = (newFreq * bendFactor) / 1000;
        outFreq -= newFreq;
    }
    else
    {
        bendFactor = ((bend - 64) * 1000) >> 6;

        idx = hmi_range_fix(note + midi_bend_range, s_hmi_freqtable_size);
        newFreq = s_hmi_freqtable[idx] - outFreq;

        if(newFreq > 719)
        {
            idx = hmi_range_fix(11 - noteMod12, s_hmi_bendtable_size);
            fmFreq = s_hmi_bendtable[idx];
            outFreq = (fmOctave + 1024) | fmFreq;

            idx = hmi_range_fix(note + midi_bend_range, s_hmi_freqtable_size);
            newFreq = s_hmi_freqtable[idx] - outFreq;
        }

        newFreq = (newFreq * bendFactor) / 1000;
        outFreq += newFreq;
    }

    return outFreq;
}
#undef hmi_range_fix

uint16_t oplModel_hmiFreq(double tone, uint32_t *mul_offset)
{
    int_fast32_t note, bend, octave, octaveOffset = 0;
    uint_fast32_t inFreq, freq;
    double bendDec;

    *mul_offset = 0;

    note = (int_fast32_t)(tone);
    bendDec = tone - (int)tone; /* 0.0 ± 1.0 - one halftone */

    if(bendDec > 0.5)
    {
        note += 1;
        bendDec -= 1.0;
    }

    bend = (int_fast32_t)(bendDec * 64.0) + 64;

    while(note < 12)
    {
        --octaveOffset;
        note += 12;
    }

    while(note > 114)
    {
        ++octaveOffset;
        note -= 12;
    }

    if(bend == 64)
        inFreq = s_hmi_freqtable[note - 12];
    else
        inFreq = s_hmi_bend_calc(bend, note);

    freq = inFreq & 0x3FF;
    octave = (inFreq >> 10) & 0x07;
    octave += octaveOffset;

    if(octave < 0)
        octave = 0; /* Don't wrap everything to top! */

    while(octave > 7)
    {
        ++(*mul_offset);
        --octave;
    }

    return freq | (octave << 10);
}


/***************************************************************
 *                  HMI SOS volume formula                     *
 ***************************************************************/

static const uint_fast32_t s_hmi_volume_table[64] =
{
    0x3F, 0x3A, 0x35, 0x30, 0x2C, 0x29, 0x25, 0x24,
    0x23, 0x22, 0x21, 0x20, 0x1F, 0x1E, 0x1D, 0x1C,
    0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14,
    0x13, 0x12, 0x11, 0x10, 0x0F, 0x0E, 0x0E, 0x0D,
    0x0D, 0x0C, 0x0C, 0x0B, 0x0B, 0x0A, 0x0A, 0x09,
    0x09, 0x08, 0x08, 0x07, 0x07, 0x06, 0x06, 0x06,
    0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x03,
    0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x00,
};

void oplModel_sosOldVolume(struct OPLVolume_t *v)
{
    uint_fast32_t volume, outVol;

    volume = (v->chVol * v->chExpr * v->masterVolume) / 16129;
    volume = (((volume * 128) / 127) * v->vel) >> 7;

    if(volume > 127)
        volume = 127;

    volume = s_hmi_volume_table[volume >> 1];

    if(v->fbConn == 0 && !v->isDrum)
    {
        outVol = (v->chVol * v->chExpr * 64) / 16129;
        outVol = (((outVol * 128) / 127) * v->vel) >> 7;
        outVol = s_hmi_volume_table[outVol >> 1];

        outVol = (64 - outVol) << 1;
        outVol *= (64 - v->tlCar);
        v->tlMod = (8192 - outVol) >> 7;
    }

    if(v->isDrum) /* TODO: VERIFY A CORRECTNESS OF THIS!!! */
        outVol = (64 - s_hmi_volume_table[v->vel >> 1]) << 1;
    else
        outVol = (64 - volume) << 1;

    outVol *= (64 - v->tlCar);
    v->tlCar = (8192 - outVol) >> 7;
}

void oplModel_sosNewVolume(struct OPLVolume_t *v)
{
    uint_fast32_t volume, outVol;

    volume = (v->chVol * v->chExpr * v->masterVolume) / 16129;
    volume = (((volume * 128) / 127) * v->vel) >> 7;

    if(volume > 127)
        volume = 127;

    volume = s_hmi_volume_table[volume >> 1];

    if(v->doMod)
    {
        outVol = (64 - volume) << 1;
        outVol *= (64 - v->tlMod);
        v->tlMod = (8192 - outVol) >> 7;
    }

    if(v->doCar)
    {
        outVol = (64 - volume) << 1;
        outVol *= (64 - v->tlCar);
        v->tlCar = (8192 - outVol) >> 7;
    }
}
