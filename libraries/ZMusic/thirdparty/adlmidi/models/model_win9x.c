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
 *            Windows 9x FM drivers frequency model            *
 ***************************************************************/

static const uint_fast8_t s_9x_opl_pitchfrac = 8;

static const uint_fast32_t s_9x_opl_freq[12] =
{
    0xAB7, 0xB5A, 0xC07, 0xCBE, 0xD80, 0xE4D, 0xF27, 0x100E, 0x1102, 0x1205, 0x1318, 0x143A
};

static const int32_t s_9x_opl_uppitch = 31;
static const int32_t s_9x_opl_downpitch = 27;

static uint_fast32_t s_9x_opl_applypitch(uint_fast32_t freq, int_fast32_t pitch)
{
    int32_t diff;

    if(pitch > 0)
    {
        diff = (pitch * s_9x_opl_uppitch) >> s_9x_opl_pitchfrac;
        freq += (diff * freq) >> 15;
    }
    else if (pitch < 0)
    {
        diff = (-pitch * s_9x_opl_downpitch) >> s_9x_opl_pitchfrac;
        freq -= (diff * freq) >> 15;
    }

    return freq;
}

uint16_t oplModel_9xFreq(double tone, uint32_t *mul_offset)
{
    uint_fast32_t note, freq, freqpitched, octave, block, bendMsb, bendLsb;
    int_fast32_t bend;
    double bendDec;

    *mul_offset = 0;

    note = (uint_fast32_t)(tone >= 12 ? tone - 12 : tone);
    bendDec = tone - (int)tone; /* 0.0 ± 1.0 - one halftone */

    if(bendDec > 0.5)
    {
        note += 1;
        bendDec -= 1.0;
    }

    bend = (int_fast32_t)(bendDec * 4096) + 8192; /* convert to MIDI standard value */

    bendMsb = (bend >> 7) & 0x7F;
    bendLsb = (bend & 0x7F);

    bend = (bendMsb << 9) | (bendLsb << 2);
    bend = (int16_t)(uint16_t)(bend + 0x8000);

    octave = note / 12;
    freq = s_9x_opl_freq[note % 12];
    if(octave < 5)
        freq >>= (5 - octave);
    else if (octave > 5)
        freq <<= (octave - 5);

    freqpitched = s_9x_opl_applypitch(freq, bend);
    freqpitched *= 2;

    block = 1;
    while(freqpitched > 0x3ff)
    {
        freqpitched /= 2;
        ++block;
    }

    while(block > 7)
    {
        ++(*mul_offset);
        --block;
    }

    return freqpitched | (block << 10);
}

/***************************************************************
 *                    Win9x volume formula                     *
 ***************************************************************/

static const uint_fast32_t s_w9x_generic_fm_volume_model[32] =
{
    40, 36, 32, 28, 23, 21, 19, 17,
    15, 14, 13, 12, 11, 10, 9,  8,
    7,  6,  5,  5,  4,  4,  3,  3,
    2,  2,  1,  1,  1,  0,  0,  0
};

static const uint_fast32_t s_w9x_sb16_volume_model[32] =
{
    80, 63, 40, 36, 32, 28, 23, 21,
    19, 17, 15, 14, 13, 12, 11, 10,
    9,  8,  7,  6,  5,  5,  4,  4,
    3,  3,  2,  2,  1,  1,  0,  0
};


void oplModel_9xGenericVolume(struct OPLVolume_t *v)
{
    uint_fast32_t volume, car = v->tlCar, mod = v->tlMod;

    volume = (v->chVol * v->chExpr * v->masterVolume) / 16129;
    volume = s_w9x_generic_fm_volume_model[volume >> 2];

    if(v->doCar)
    {
        car += volume + s_w9x_generic_fm_volume_model[v->vel >> 2];

        if(car > 0x3F)
            car = 0x3F;

        v->tlCar = car;
    }

    if(v->doMod)
    {
        mod += volume + s_w9x_generic_fm_volume_model[v->vel >> 2];

        if(mod > 0x3F)
            mod = 0x3F;

        v->tlMod = mod;
    }
}

void oplModel_9xSB16Volume(struct OPLVolume_t *v)
{
    uint_fast32_t volume, car = v->tlCar, mod = v->tlMod;

    volume = (v->chVol * v->chExpr * v->masterVolume) / 16129;
    volume = s_w9x_sb16_volume_model[volume >> 2];

    if(v->doCar)
    {
        car += volume + s_w9x_sb16_volume_model[v->vel >> 2];

        if(car > 0x3F)
            car = 0x3F;

        v->tlCar = car;
    }

    if(v->doMod)
    {
        mod += volume + s_w9x_sb16_volume_model[v->vel >> 2];

        if(mod > 0x3F)
            mod = 0x3F;

        v->tlMod = mod;
    }
}
