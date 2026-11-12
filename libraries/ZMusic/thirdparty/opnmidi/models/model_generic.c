/*
 * OPN2/OPNA models library - a set of various conversion models for OPL-family chips
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
#include <math.h>   /* log() */
#include "opn_models.h"


/***************************************************************
 *                     Generic volume formula                  *
 ***************************************************************/

void opnModel_genericVolume(struct OPNVolume_t *v)
{
    const double c1 = 11.541560327111707;
    const double c2 = 1.601379199767093e+02;
    const uint_fast32_t minVolume = 1108075; /* 8725 * 127 */
    double lv;
    uint_fast32_t volume = 0;
    size_t i;

    volume = v->vel * v->masterVolume * v->chVol * v->chExpr;

    if(volume > minVolume)
    {
        lv = log((double)volume);
        volume = (uint_fast32_t)(lv * c1 - c2) * 2;

        if(volume > 127)
            volume = 127;
    }
    else
        volume = 0;

    for(i = 0; i < 4; ++i)
    {
        if(v->doOp[i])
            v->tlOp[i] = 127 - (volume * (127 - (v->tlOp[i] & 127))) / 127;
    }
}

void opnModel_nativeVolume(struct OPNVolume_t *v)
{
    uint_fast32_t volume = 0;
    size_t i;

    volume = v->vel * v->chVol * v->chExpr;
    /* 4096766 = (127 * 127 * 127) * 2 */
    volume = (volume * v->masterVolume) / 4096766;

    /* OPN has 0~127 range. As 0...63 is almost full silence, but at 64 to 127 is very closed to OPL3, just add 64.*/
    if(volume > 0)
        volume += 64;

    if(volume > 127)
        volume = 127;

    for(i = 0; i < 4; ++i)
    {
        if(v->doOp[i])
            v->tlOp[i] = 127 - (volume * (127 - (v->tlOp[i] & 127))) / 127;
    }
}



/***************************************************************
 *                     XG CC74 Brightness                      *
 ***************************************************************/

/*! Pre-computed table of XG Brightness for OPL family chips.
    Result of:
    ```
    double b = (double)(brightness);
    double ret = round(127.0 * sqrt(b * (1.0 / 127.0)));
    return (uint_fast32_t)(ret);
    ```
*/
static uint8_t s_xgBrightness[] =
{
    0,  11, 16, 20, 23, 25, 28, 30, 32, 34,
    36, 37, 39, 41, 42, 44, 45, 46, 48, 49,
    50, 52, 53, 54, 55, 56, 57, 59, 60, 61,
    62, 63, 64, 65, 66, 67, 68, 69, 69, 70,
    71, 72, 73, 74, 75, 76, 76, 77, 78, 79,
    80, 80, 81, 82, 83, 84, 84, 85, 86, 87,
    87, 88, 89, 89, 90, 91, 92, 92, 93, 94,
    94, 95, 96, 96, 97, 98, 98, 99, 100,100,
    101,101,102,103,103,104,105,105,106,106,
    107,108,108,109,109,110,110,111,112,112,
    113,113,114,114,115,115,116,117,117,118,
    118,119,119,120,120,121,121,122,122,123,
    123,124,124,125,125,126,126,127
};

uint_fast16_t opnModels_xgBrightnessToOPN(uint_fast16_t brightness)
{
    return s_xgBrightness[brightness & 0xFF];
}
