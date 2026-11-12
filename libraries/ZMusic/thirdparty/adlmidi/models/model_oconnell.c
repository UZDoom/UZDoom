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

/*! Constant +32 drums boost */
const uint16_t s_drumBoost = 32;

static const uint16_t masterFreqs[] =
{
    0x158, 0x16D, 0x183, 0x19A, 0x1B2, 0x1CC, 0x1E7,
    0x204, 0x223, 0x244, 0x266, 0x28A
};

static const uint8_t s_velocTbl[128] =
{
    0x00, 0x08, 0x0A, 0x0F, 0x13, 0x16, 0x18, 0x1A,
    0x1C, 0x1D, 0x1E, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x24, 0x25, 0x26, 0x27, 0x27, 0x28, 0x28, 0x29,
    0x2A, 0x2A, 0x2B, 0x2B, 0x2C, 0x2C, 0x2C, 0x2D,
    0x2D, 0x2E, 0x2E, 0x2E, 0x2F, 0x2F, 0x2F, 0x30,
    0x30, 0x30, 0x31, 0x31, 0x31, 0x32, 0x32, 0x32,
    0x32, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34,
    0x34, 0x35, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37,
    0x38, 0x38, 0x38, 0x38, 0x38, 0x39, 0x39, 0x39,
    0x39, 0x39, 0x39, 0x39, 0x3A, 0x3A, 0x3A, 0x3A,
    0x3A, 0x3A, 0x3A, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B,
    0x3B, 0x3B, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
    0x3C, 0x3C, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D,
    0x3D, 0x3D, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E,
    0x3E, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F
};


uint16_t oplModel_OConnellFreq(double tone, uint32_t *mul_offset)
{
    uint16_t wOctv, wFreq, wNewFreq, bRange = 2;
    uint_fast32_t note, idx, wAmt;
    int_fast32_t pitch;
    double bendDec;
    long lDiff;

    *mul_offset = 0;

    note = (uint_fast32_t)tone;
    bendDec = tone - (int)tone;

    if(bendDec > 0.5)
    {
        note += 1;
        bendDec -= 1.0;
    }

    pitch = (int_fast32_t)(bendDec * 4096) + 8192;

    wOctv = note / 12;

    while(wOctv > 7)
    {
        ++(*mul_offset);
        --wOctv;
    }

    if(wOctv > 0)
        --wOctv; /* Drop Octave (FM Chip uses 48 as MID C) */

    wFreq = masterFreqs[note % 12];

    if(pitch > 0x2000)
    {
        wAmt = pitch - 0x2000;
        idx = (note + bRange) % 12;
        wNewFreq = masterFreqs[idx];

        if(wNewFreq <= wFreq)
            wNewFreq <<= 1;

        lDiff = (long)(wNewFreq - wFreq) * wAmt;
        wNewFreq = (uint16_t)(lDiff >> 13) + wFreq;
        while(wNewFreq > 0x3FF)
        {
            if(wOctv < 7)
                ++wOctv;
            else
                ++(*mul_offset);
            wNewFreq >>= 1;
        }

        wFreq = wNewFreq;
    }
    else if(pitch < 0x2000)
    {
        wAmt = 0x2000 - pitch;
        idx = note > bRange ? (note - bRange) % 12 : 0;
        wNewFreq = masterFreqs[idx];

        if(wNewFreq >= wFreq)
            wNewFreq >>= 1;

        lDiff = (long)(wFreq - wNewFreq) * wAmt;
        wNewFreq = wFreq - (uint16_t)(lDiff >> 13);
        while(wNewFreq < masterFreqs[0])
        {
            if(wOctv > 0)
            {
                --wOctv;
                wNewFreq <<= 1;
            }
            else
                wNewFreq = masterFreqs[0];
        }

        wFreq = wNewFreq;
    }

    return wFreq | (wOctv << 10);
}


void oplModel_OConnellVolume(struct OPLVolume_t *v)
{
    uint_fast32_t volume = 0, work;

    volume = (uint_fast32_t)s_velocTbl[v->vel] * (uint_fast32_t)s_velocTbl[v->chVol] *
             (uint_fast32_t)s_velocTbl[v->chExpr] * (uint_fast32_t)s_velocTbl[v->masterVolume];

    if(v->isDrum)
    {
        volume >>= 19;
        volume += 2;
    }
    else
    {
        volume >>= 18;
        volume += 3; /* Maximum result is 60, so, compensate that! */
    }

    if(v->doCar)
    {
        work = volume * (63 - v->tlCar);
        work >>= 6;

        if(v->isDrum)
            work += s_drumBoost;

        if(work > 63)
            work = 63;
        v->tlCar = 63 - work;
    }

    if(v->doMod)
    {
        work = volume * (63 - v->tlMod);
        work >>= 6;

        if(v->isDrum)
            work += s_drumBoost;

        if(work > 63)
            work = 63;
        v->tlMod = 63 - work;
    }
}
