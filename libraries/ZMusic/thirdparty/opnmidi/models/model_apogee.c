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
#include "opn_models.h"

void opnModel_apogeeLikeVolume(struct OPNVolume_t *v)
{
    uint_fast32_t volume, op;
    size_t i;

    volume = (v->chVol * v->chExpr * v->masterVolume / 16129);

    /* OPN has 0~127 range. As 0...63 is almost full silence, but at 64 to 127 is very closed to OPL3, just add 64.*/
    if(volume > 127)
        volume = 127;

    for(i = 0; i < 4; ++i)
    {
        if(v->doOp[i])
        {
            op = v->tlOp[i];
            op = (127 - op) / 2;
            op *= v->vel + 0x80;
            op = (volume * op) >> 15;
            op = op ^ 63;
            op = (64 - op);

            if(op > 0)
                op += 64;

            if(op > 127)
                op = 127;

            v->tlOp[i] = 127 - op;
        }
    }
}
