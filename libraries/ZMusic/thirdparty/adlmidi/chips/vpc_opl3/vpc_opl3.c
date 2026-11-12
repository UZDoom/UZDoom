/*
 * VPC OPL3 - Reverse-engineering of legacy OPL3 emulator from VirtualPC
 * Copyright (C) 2025-2025 Nuke.YKT
 *
 * This file is part of VPC OPL3.
 *
 * VPC OPL3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * VPC OPL3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with VPC OPL3. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
/* #include <math.h> */
#include <limits.h>
#include "vpc_opl3.h"

static const uint32_t mult_table[16] = { 1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30 };
static const uint32_t ksl_table[16] = { 0, 144, 192, 222, 240, 258, 270, 282, 288, 300, 306, 312, 318, 324, 330, 336 };

static const uint32_t attack_table1[76] =
{
    8820000, 8820000, 8820000, 8820000, 31160, 24838, 20773, 17612, 15580,
    12419, 10387, 8806, 7790, 6210, 5194, 4403, 3895, 3105, 2597, 2202,
    1949, 1553, 1299, 1101, 974, 777, 650, 551, 487, 389, 325, 276, 244,
    195, 163, 138, 122, 98, 82, 69, 61, 49, 41, 35, 31, 25, 21, 18, 16,
    13, 11, 9, 8, 7, 6, 5, 5, 4, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1
};

static const uint32_t attack_table2[76] =
{
    8820000, 8820000, 8820000, 8820000, 16348, 12735, 10929, 9574, 8174,
    6368, 5465, 4787, 4087, 3184, 2733, 2394, 2044, 1592, 1366, 1197, 1022,
    796, 683, 599, 511, 399, 342, 300, 256, 200, 171, 150, 128, 100, 86,
    75, 64, 50, 43, 38, 32, 25, 22, 19, 16, 13, 11, 10, 9, 7, 6, 5, 5,
    4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static const uint32_t decrel_table1[76] =
{
    8820000, 8820000, 8820000, 8820000, 433070, 346365, 288563, 247469,
    216535, 173183, 144282, 123735, 108268, 86592, 72141, 61868, 54134,
    43296, 36071, 30934, 27067, 21648, 18036, 15467, 13534, 10824, 9018,
    7734, 6767, 5412, 5391, 3867, 3384, 2706, 2255, 1934, 1692, 1353, 1128,
    967, 846, 677, 564, 484, 423, 339, 282, 242, 212, 170, 142, 121, 106,
    85, 71, 61, 53, 43, 36, 31, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27
};

static const uint32_t decrel_table2[76] =
{
    8820000, 8820000, 8820000, 8820000, 90543, 72480, 60739, 52158, 45272,
    36240, 30370, 26079, 22636, 18120, 15185, 13040, 11318, 9060, 7593,
    6520, 5659, 4530, 3797, 3260, 2830, 2265, 1899, 1630, 1415, 1133, 950,
    815, 708, 567, 475, 408, 354, 284, 238, 204, 177, 142, 119, 102, 89,
    71, 60, 51, 45, 36, 30, 26, 23, 18, 15, 13, 12, 9, 8, 7, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
};

#if 0 /* Unused */
static const double attack_time2[64] =
{
    0.00, 0.00, 0.00, 0.00,
    0.11, 0.11, 0.14, 0.19,
    0.22, 0.26, 0.31, 0.37,
    0.43, 0.49, 0.61, 0.73,
    0.85, 0.97, 1.13, 1.45,
    1.70, 1.94, 2.26, 2.90,
    3.39, 3.87, 4.51, 5.79,
    6.78, 7.74, 9.02, 11.58,
    13.57, 15.49, 18.05, 23.17,
    27.14, 30.98, 36.10, 46.34,
    54.27, 61.95, 72.19, 92.67,
    108.54, 123.90, 144.38, 185.34,
    217.09, 247.81, 288.77, 370.69,
    434.18, 495.62, 577.54, 741.38,
    868.35, 991.23, 1155.07, 1482.75,
    800000.00, 800000.00, 800000.00, 800000.00
};

static const double attack_time1[64] =
{
    0.00, 0.00, 0.00, 0.00,
    0.20, 0.24, 0.30, 0.38,
    0.42, 0.46, 0.56, 0.70,
    0.80, 0.92, 1.12, 1.40,
    1.56, 1.84, 2.20, 2.76,
    3.12, 3.68, 4.40, 5.52,
    6.24, 7.36, 8.80, 11.04,
    12.48, 14.72, 17.60, 22.08,
    24.96, 29.44, 35.20, 44.16,
    49.92, 58.88, 70.40, 88.32,
    99.84, 117.76, 140.80, 176.76,
    199.68, 235.52, 281.60, 353.28,
    399.36, 471.04, 563.20, 706.56,
    798.72, 942.08, 1126.40, 1413.12,
    1597.44, 1884.16, 2252.80, 2826.24,
    800000.00, 800000.00, 800000.00, 800000.00
};

static const double decrel_time2[64] =
{
    0.51, 0.51, 0.51, 0.51,
    0.58, 0.69, 0.81, 1.01,
    1.15, 1.35, 1.62, 2.02,
    2.32, 2.68, 3.22, 4.02,
    4.62, 5.38, 6.42, 8.02,
    9.24, 10.76, 12.84, 16.04,
    18.48, 21.52, 25.68, 32.08,
    36.96, 43.04, 51.36, 64.16,
    73.92, 86.08, 102.72, 128.32,
    147.84, 172.16, 205.44, 256.64,
    295.68, 344.34, 410.88, 513.28,
    591.36, 688.64, 821.76, 1026.56,
    1182.72, 1377.28, 1643.52, 2053.12,
    2365.44, 2754.56, 3287.04, 4106.24,
    4730.88, 5509.12, 6574.08, 8212.48,
    800000.00, 800000.00, 800000.00, 800000.00
};

static const double decrel_time1[64] =
{
    2.40, 2.40, 2.40, 2.40,
    2.74, 3.20, 3.84, 4.80,
    5.48, 6.40, 7.68, 9.60,
    10.96, 12.80, 15.36, 19.20,
    21.92, 25.56, 30.68, 38.36,
    43.84, 51.12, 61.36, 76.72,
    87.68, 102.24, 122.72, 153.44,
    175.36, 204.48, 245.44, 306.88,
    350.72, 488.96, 490.88, 613.76,
    701.44, 817.92, 981.76, 1227.52,
    1402.88, 1635.84, 1963.52, 2455.04,
    2805.76, 3271.68, 3927.04, 4910.08,
    5611.52, 6543.36, 7854.08, 9820.16,
    11223.04, 13086.72, 15708.16, 19640.32,
    22446.08, 26173.44, 31416.32, 39280.64,
    800000.00, 800000.00, 800000.00, 800000.00
};
#endif

static const uint32_t eg_table[256] =
{
    12288, 10947, 9752, 8688, 7740, 6896, 6144, 5473, 4876, 4344, 3870,
    3448, 3072, 2736, 2438, 2172, 1935, 1724, 1536, 1368, 1219, 1086, 967,
    862, 768, 684, 609, 543, 483, 431, 384, 342, 304, 271, 241, 215, 192,
    171, 152, 135, 120, 107, 96, 85, 76, 67, 60, 53, 48, 42, 38, 33, 30,
    26, 24, 21, 19, 16, 15, 13, 12, 10, 9, 8, 7, 6, 6, 5, 4, 4, 3, 3, 3,
    2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct vslot
{
    void *chip;
    uint8_t am;
    uint8_t vib;
    uint8_t egt;
    uint8_t ksr;
    uint8_t mult;
    uint8_t ksl_shift;
    uint8_t tl;
    uint8_t ar;
    uint8_t dr;
    uint8_t sl;
    uint8_t rr;
    uint8_t wf;
    uint32_t pg_phase;
    uint32_t unk1; /*uint32_t tremolo;*/
    uint8_t eg_on;
    uint8_t eg_state;
    uint32_t eg_level;
    uint32_t eg_inc;
    uint32_t eg_atime;
    uint16_t pg_fnum;
    uint16_t fnum;
    uint8_t key;
    uint8_t block;
    uint8_t dam;
    uint8_t dvb;
    uint8_t nts;
};

struct channel
{
    void *chip;
    void *op1;
    void *op2;
    uint8_t op1id;
    uint8_t op2id;
    uint16_t fnum;
    uint8_t fb;
    uint8_t con;
    uint8_t cha;
    uint8_t chb;
    int32_t chanout[1024];
};

struct vpc_opl
{
    uint8_t lsi;
    uint8_t opl3;
    uint8_t wfmask;
    int32_t buff[1024];
    struct channel channels[18];
    struct vslot slots[36];

    /* Resampler */
    int32_t out_samples[2];
    int32_t out_samplecnt;
    int32_t out_rateratio;

    /* Out buffer */
    int16_t accbuff[2][1024];
    size_t accbuff_size;
    size_t accbuff_off;
};

#define rsm_frac 10
#define rsm_dur 1024


/*
// Waveform 0: 2 periods
//  _     _
// / \   / \
//    \_/   \_/
*/

static int32_t calc_wf0(uint16_t phase)
{
    int32_t wf;
    uint16_t neg = phase & 0x8000;
    uint16_t inv = phase & 0x4000;

    phase &= 0x3fff;

    if(inv)
        phase ^= 0x3fff;

    wf = (phase * (0x30000 - ((unsigned int)(phase * phase) >> 12))) >> 16;

    if(neg)
        wf *= -1;

    return wf;
}

/*
// Waveform 1: 2 periods
//  _     _
// / \___/ \___
//
*/

static int32_t calc_wf1(uint16_t phase)
{
    int32_t wf = 0;
    uint16_t neg = phase & 0x8000;
    uint16_t inv = phase & 0x4000;

    phase &= 0x3fff;

    if(inv)
        phase ^= 0x3fff;

    if(!neg)
        wf = (phase * (0x30000 - ((unsigned int)(phase * phase) >> 12))) >> 16;

    return wf;
}

/*
// Waveform 2: 2 periods
//  _  _  _  _
// / \/ \/ \/ \
//
*/

static int32_t calc_wf2(uint16_t phase)
{
    int32_t wf;
    uint16_t inv = phase & 0x4000;

    phase &= 0x3fff;

    if(inv)
        phase ^= 0x3fff;

    wf = (phase * (0x30000 - ((unsigned int)(phase * phase) >> 12))) >> 16;

    return wf;
}

/*
// Waveform 3: 2 periods
//
// /|_/|_/|_/|_
//
*/

static int32_t calc_wf3(uint16_t phase)
{
    int32_t wf = 0;
    uint16_t inv = phase & 0x4000;

    phase &= 0x3fff;

    if(!inv)
        wf = (phase * (0x30000 - ((unsigned int)(phase * phase) >> 12))) >> 16;

    return wf;
}

/*
// Waveform 4: 2 periods
//  _           _
// / \   ______/ \   ______
//    \_/         \_/
*/

static int32_t calc_wf4(uint16_t phase)
{
    int32_t wf = 0;
    uint16_t neg = phase & 0x8000;

    phase &= 0x7fff;

    if(!neg)
        wf = calc_wf0(phase * 2);

    return wf;
}

/*
// Waveform 5: 2 periods
//  _  _        _  _
// / \/ \______/ \/ \______
//
*/

static int32_t calc_wf5(uint16_t phase)
{
    int32_t wf = 0;
    uint16_t neg = phase & 0x8000;

    phase &= 0x7fff;

    if(!neg)
        wf = calc_wf2(phase * 2);

    return wf;
}

/*
// Waveform 6: 2 periods
//  __    __
// |  |  |  |
//    |__|  |__|
*/

static int32_t calc_wf6(uint16_t phase)
{
    uint16_t neg = phase & 0x8000;
    int32_t wf = 0;

    if(!neg)
        wf = 32768;
    else
        wf = -32768;

    return wf;
}

/*
// Waveform 7: 2 periods
//
// |\____ |\____
//       \|     \|
*/

static int32_t calc_wf7(uint16_t phase)
{
    int32_t wf;
    uint16_t neg = phase & 0x8000;
    phase &= 0x7fff;

    if(!neg)
        phase ^= 0x7fff;

    wf = phase * (((unsigned int)(phase * phase) >> 15)) >> 15;;

    if(neg)
        wf *= -1;

    return wf;
}

static void do_slrr(struct vslot *op, uint8_t data)
{
    uint8_t sl = (data >> 4) * 3;
    uint8_t rr = data & 0xf;
    op->rr = rr;
    op->sl = sl;
}

static void do_ardr(struct vslot *op, uint8_t data)
{
    uint8_t ar = data >> 4;
    uint8_t dr = data & 0xf;
    op->dr = dr;
    op->ar = ar;
}

static void do_ksltl(struct vslot *op, uint8_t data)
{
    uint8_t tl, ksl = data >> 6;

    switch(ksl)
    {
    case 3:
        op->ksl_shift = 0;
        break;
    case 2:
        op->ksl_shift = 1;
        break;
    case 1:
        op->ksl_shift = 2;
        break;
    case 0:
        op->ksl_shift = 24;
        break;
    }

    tl = data & 0x3f;

    op->tl = (tl * 3) / 4;
}

static void do_avekm(struct vslot *op, uint8_t data)
{
    uint8_t am, vib, egt, ksr, mult;

    am = data >> 7;
    op->am = am;
    vib = (data >> 6) & 1;

    op->vib = vib;
    egt = (data >> 5) & 1;

    op->egt = egt;
    ksr = (data >> 4) & 1;
    mult = data & 0xf;

    op->mult = mult;
    op->ksr = ksr;
}

static void do_wf(struct vslot *op, uint8_t data)
{
    uint8_t wf = data & 0x7;
    op->wf = wf;
}

static void do_wfmask(struct vpc_opl *chip)
{
    if(chip->lsi & 0x20)
        chip->wfmask = 4 * chip->opl3 + 3;
    else
        chip->wfmask = 0x0;
}

static void do_fnum(struct channel *chan, uint8_t data)
{
    struct vslot *op1, *op2;
    uint16_t fnum = (chan->fnum & 0x300) | data;

    chan->fnum = fnum;
    op1 = (struct vslot *)chan->op1;
    op2 = (struct vslot *)chan->op2;
    op1->fnum = fnum;
    op2->fnum = fnum;
    fnum = 49716 * fnum / 11025;
    op1->pg_fnum = fnum;
    op2->pg_fnum = fnum;
}

static void do_block(struct channel *chan, uint8_t data)
{
    struct vslot *op1, *op2;
    uint16_t fnum = ((data << 8) & 0x300) | (chan->fnum & 0xff);
    uint8_t block, key;

    chan->fnum = fnum;
    op1 = (struct vslot *)chan->op1;
    op2 = (struct vslot *)chan->op2;
    op1->fnum = fnum;
    op2->fnum = fnum;
    fnum = 49716 * fnum / 11025;
    op1->pg_fnum = fnum;
    op2->pg_fnum = fnum;

    block = (data >> 2) & 0x7;
    op1->block = block;
    op2->block = block;
    key = (data >> 5) & 0x1;

    if(key != op1->key)
    {
        op1->eg_atime = 0;

        if(key)
        {
            op1->eg_level = 0x6000000;
            op1->eg_inc = 0;
            op1->eg_state = 0;

            if(!op1->eg_on)
                op1->pg_phase = 0;

            op1->eg_on = 1;
        }
        else
            op1->eg_state = 3;
    }

    op1->key = key;

    if(key != op2->key)
    {
        op2->eg_atime = 0;

        if(key)
        {
            op2->eg_level = 0x6000000;
            op2->eg_inc = 0;
            op2->eg_state = 0;

            if(!op2->eg_on)
                op2->pg_phase = 0;

            op2->eg_on = 1;
        }
        else
            op2->eg_state = 3;
    }
    op2->key = key;
}

#if 0 /* Turned into pre-computed static consts, see build_table.c */
static void init_egtables()
{
    int i;

    for(i = 0; i < 64; i++)
    {
        decrel_table2[i] = (uint32_t)(ceil(decrel_time2[63 - i] * 1000.0 / 90.70294784580499));
        decrel_table1[i] = (uint32_t)(ceil(decrel_time1[63 - i] * 1000.0 / 90.70294784580499));
        attack_table2[i] = (uint32_t)(ceil(attack_time2[63 - i] * 1000.0 / 90.70294784580499));
        attack_table1[i] = (uint32_t)(ceil(attack_time1[63 - i] * 1000.0 / 90.70294784580499));
    }

    for(i = 64; i < 76; i++)
    {
        decrel_table2[i] = decrel_table2[63];
        decrel_table1[i] = decrel_table1[63];
        attack_table2[i] = attack_table2[63];
        attack_table1[i] = attack_table1[63];
    }

    for(i = 0; i < 76; i++)
    {
        if(decrel_table2[i] == 0)
            decrel_table2[i] = 1;
        if(decrel_table1[i] == 0)
            decrel_table1[i] = 1;
        if(attack_table2[i] == 0)
            attack_table2[i] = 1;
        if(attack_table1[i] == 0)
            attack_table1[i] = 1;
    }

    for(i = 0; i < 256; i++)
        eg_table[i] = (uint32_t)(1.0 / pow(2.0, i / 6.0) * 16384.0 * 0.75);
}
#endif

static void calc_attack(struct vslot *op, uint8_t rate, uint8_t time)
{
    int32_t delta = -(int32_t)op->eg_level;
    const uint32_t *time_base = attack_table2;
    uint32_t fatime;
    int32_t fadd, atime;

    if(time)
        time_base = attack_table1;

    fatime = time_base[rate + op->ar * 4];
    if(fatime == 0)
        fatime = 1;

    fadd = -0x6000000 / fatime;
    op->eg_inc = fadd;

    if(fadd == 0)
        atime = INT_MAX;
    else
        atime = delta / fadd;

    if(atime <= 0)
        atime = 1;

    op->eg_atime = atime;
    op->eg_inc = delta / atime;
}

static void calc_decay(struct vslot *op, uint8_t rate, uint8_t time)
{
    int32_t sl = (op->sl << 20);
    int32_t delta = sl - op->eg_level;
    const uint32_t *time_base = decrel_table1;
    uint32_t fatime;
    int32_t fadd, atime;

    if(time)
        time_base = decrel_table2;

    fatime = time_base[rate + op->dr * 4];
    if(fatime == 0)
        fatime = 1;

    fadd = sl / fatime;
    op->eg_inc = fadd;

    if(fadd == 0)
        atime = INT_MAX;
    else
        atime = delta / fadd;

    if(atime <= 0)
        atime = 1;

    op->eg_atime = atime;
    op->eg_inc = delta / atime;
}

static void calc_release(struct vslot *op, uint8_t rate, uint8_t time)
{
    int32_t rr = op->rr;
    int32_t delta = 0x6000000 - op->eg_level;
    int32_t atime;
    const uint32_t *time_base;
    uint32_t fatime;
    int32_t fadd;

    if(rr != 0)
    {
        time_base = decrel_table1;
        if(time)
            time_base = decrel_table2;

        fatime = time_base[rate + rr * 4];
        if(fatime == 0)
            fatime = 1;

        fadd = ((96 - (int32_t)op->sl) << 20) / fatime;
        op->eg_inc = fadd;

        if(fadd == 0)
            atime = INT_MAX;
        else
            atime = delta / fadd;

        if(atime <= 0)
            atime = 1;
    }
    else
        atime = 1;

    op->eg_atime = atime;
    op->eg_inc = delta / atime;
}

static void advance_eg(struct vslot *slot, int32_t *outbuff, uint32_t len, int32_t *wave, uint8_t type)
{
    uint32_t att, ksr, i = 0, j;
    uint8_t egat, eg_cstate;
    int32_t ksl, eg_inc, eg_atime, eg_att, eg_rtime, eg_fatt;

    ksl = ksl_table[slot->fnum >> 6] + (slot->block * 3 - 21) * 16;

    if(ksl < 0)
        ksl = 0;

    att = (ksl >> (slot->ksl_shift + 3)) + slot->tl;
    ksr = ((slot->fnum >> (slot->nts + 8)) & 1) + slot->block * 2;

    if(!slot->ksr)
        ksr >>= 2;

    egat = !type;

    while(i < len)
    {
        eg_inc = slot->eg_inc;
        eg_atime = slot->eg_atime;
        eg_att = (att << 20) + slot->eg_level;

        if(eg_atime <= 0)
            eg_rtime = eg_atime;
        else if((uint32_t)eg_atime <= len - i)
            eg_rtime = eg_atime;
        else
            eg_rtime = len - i;

        slot->eg_atime -= eg_rtime;

        for(j = i; j < i + eg_rtime; j++)
        {
            if(egat)
                outbuff[j] += (wave[j] * (int32_t)eg_table[eg_att >> 20]) >> 14;
            else
                outbuff[j] = (wave[j] * (int32_t)eg_table[eg_att >> 20]) >> 14;
            eg_att += eg_inc;
        }

        eg_fatt = slot->eg_level + eg_rtime * eg_inc;
        slot->eg_level = eg_fatt;

        if(slot->eg_atime <= 0)
        {
            eg_cstate = slot->eg_state;
            slot->eg_state++;

            switch(eg_cstate)
            {
            case 0:
                calc_attack(slot, ksr, type);
                break;
            case 1:
                calc_decay(slot, ksr, type);
                break;
            case 2:
                slot->eg_inc = 0;
                slot->eg_atime = -(slot->egt) & 0x7fffffff;
                break;
            case 3:
                calc_release(slot, ksr, type);
                break;
            case 4:
                if(egat == 0)
                    memset(&outbuff[i], 0, (len - i) * 4);
                slot->eg_on = 0;
                return;
                break;
            }
        }

        i += eg_rtime;
    }
}

static void advance_slot(struct vslot *slot, int32_t *outbuff, uint32_t len, int32_t *modulator, int8_t modshr, uint8_t type)
{
    uint32_t i, pg_inc;
    struct vpc_opl *chip;
    uint8_t wf;
    int8_t rmodshr;

    pg_inc = ((mult_table[slot->mult] * ((slot->pg_fnum << slot->block)) >> 4)) >> 1;
    chip = (struct vpc_opl *)slot->chip;
    wf = slot->wf & chip->wfmask;
    rmodshr = 4 - modshr;

    switch(wf)
    {
    case 0:
        for(i = 0; i < len; i++)
        {
            if(modulator)
                chip->buff[i] = calc_wf0(slot->pg_phase + ((modulator[i] * 16) >> rmodshr));
            else
                chip->buff[i] = calc_wf0(slot->pg_phase);
            slot->pg_phase += pg_inc;
        }
        break;
    case 1:
        for(i = 0; i < len; i++)
        {
            if(modulator)
                chip->buff[i] = calc_wf1(slot->pg_phase + (modulator[i] * 16 >> rmodshr));
            else
                chip->buff[i] = calc_wf1(slot->pg_phase);
            slot->pg_phase += pg_inc;
        }
        break;
    case 2:
        for(i = 0; i < len; i++)
        {
            if(modulator)
                chip->buff[i] = calc_wf2(slot->pg_phase + (modulator[i] * 16 >> rmodshr));
            else
                chip->buff[i] = calc_wf2(slot->pg_phase);
            slot->pg_phase += pg_inc;
        }
        break;
    case 3:
        for(i = 0; i < len; i++)
        {
            if(modulator)
                chip->buff[i] = calc_wf3(slot->pg_phase + (modulator[i] * 16 >> rmodshr));
            else
                chip->buff[i] = calc_wf3(slot->pg_phase);
            slot->pg_phase += pg_inc;
        }
        break;
    case 4:
        for(i = 0; i < len; i++)
        {
            if(modulator)
                chip->buff[i] = calc_wf4(slot->pg_phase + (modulator[i] * 16 >> rmodshr));
            else
                chip->buff[i] = calc_wf4(slot->pg_phase);
            slot->pg_phase += pg_inc;
        }
        break;
    case 5:
        for(i = 0; i < len; i++)
        {
            if(modulator)
                chip->buff[i] = calc_wf5(slot->pg_phase + (modulator[i] * 16 >> rmodshr));
            else
                chip->buff[i] = calc_wf5(slot->pg_phase);
            slot->pg_phase += pg_inc;
        }
        break;
    case 6:
        for(i = 0; i < len; i++)
        {
            if(modulator)
                chip->buff[i] = calc_wf6(slot->pg_phase + (modulator[i] * 16 >> rmodshr));
            else
                chip->buff[i] = calc_wf6(slot->pg_phase);
            slot->pg_phase += pg_inc;
        }
        break;
    case 7:
        for(i = 0; i < len; i++)
        {
            if(modulator)
                chip->buff[i] = calc_wf7(slot->pg_phase + (modulator[i] * 16 >> rmodshr));
            else
                chip->buff[i] = calc_wf7(slot->pg_phase);
            slot->pg_phase += pg_inc;
        }
        break;
    }

    advance_eg(slot, outbuff, len, chip->buff, type);
}

static void advance_chan(struct channel *chan, int32_t *outbuff, uint32_t len)
{
    struct vslot r;
    struct vslot *op1 = (struct vslot *)chan->op1;
    struct vslot *op2 = (struct vslot *)chan->op2;
    int8_t fb;

    if(op1->eg_on == 0 && op2->eg_on == 0)
        return;

    memcpy(&r, chan->op1, sizeof(struct vslot));
    advance_slot(&r, chan->chanout, len, 0, 0, 1);

    fb = chan->fb - 6;

    if(chan->con)
    {
        advance_slot((struct vslot *)chan->op1, outbuff, len, chan->chanout, fb, 0);
        advance_slot((struct vslot *)chan->op2, outbuff, len, 0, 0, 0);
    }
    else
    {
        advance_slot((struct vslot *)chan->op1, chan->chanout, len, chan->chanout, fb, 1);
        advance_slot((struct vslot *)chan->op2, outbuff, len, chan->chanout, 3, 0);
    }
}

void *vpc_opl_init()
{
    struct vpc_opl *chip = (struct vpc_opl *)malloc(sizeof(struct vpc_opl));
    /* init_egtables();*/
    vpc_opl_reset(chip);

    return chip;
}

void vpc_opl_free(void *opl3)
{
    struct vpc_opl *chip = (struct vpc_opl *)opl3;
    free(chip);
}

static void rate_reset(struct vpc_opl *chip)
{
    chip->out_samples[0] = chip->out_samples[1] = 0;
    chip->out_samplecnt = 0;
}

void vpc_opl_set_rate(void *opl3, uint32_t rate)
{
    struct vpc_opl *chip = (struct vpc_opl *)opl3;
    chip->out_samples[0] = chip->out_samples[1] = 0;
    chip->out_samplecnt = 0;
    chip->out_rateratio = (rate << rsm_frac) / 11025;
}

void vpc_opl_reset(void *opl3)
{
    int i;
    uint8_t opbase;

    struct vpc_opl *chip = (struct vpc_opl *)opl3;
    memset(chip, 0, sizeof(struct vpc_opl));
    /* memset(chip,0,sizeof(chip)); */

    for(i = 0; i < 36; i++)
    {
        chip->slots[i].chip = chip;
        chip->slots[i].eg_level = 0x6000000;
    }

    for(i = 0; i < 18; i++)
    {
        chip->channels[i].chip = chip;
        opbase = (i / 3) * 6 + (i % 3);
        chip->channels[i].op1 = &chip->slots[opbase];
        chip->channels[i].op1id = opbase;
        chip->channels[i].op2 = &chip->slots[opbase + 3];
        chip->channels[i].op1id = opbase + 3;
        chip->channels[i].cha = 1;
        chip->channels[i].chb = 1;
    }

    vpc_opl_set_rate(chip, 49716);
    rate_reset(chip);
}

void vpc_opl_writereg(void *opl3, uint16_t regg, uint8_t data)
{
    struct vpc_opl *chip = (struct vpc_opl *)opl3;
    uint8_t hb, nts, dam, dvb, hr, opbase, chbase;
    uint32_t i;

    hb = (regg >> 8) & 0x1;
    regg &= 0xff;

    switch(regg)
    {
    case 0x1:
        if(hb == 0)
        {
            chip->lsi = data;
            do_wfmask(chip);
        }
        break;

    case 0x5:
        if(hb == 1)
        {
            chip->opl3 = hb & 0x1;
            do_wfmask(chip);
        }
        break;

    case 0x8:
        if(hb == 0)
        {
            nts = (data >> 6) & 0x1;

            for(i = 0; i < 36; i++)
                chip->slots[i].nts = nts;

            break;
        }
        break;

    case 0xbd:
        if(hb == 0)
        {
            dam = data >> 7;
            dvb = (data >> 6) & 0x1;

            for(i = 0; i < 36; i++)
            {
                chip->slots[i].dam = dam;
                chip->slots[i].dvb = dvb;
            }
            break;
        }
        /* fallthrough */

    default:
        hr = regg >> 4;
        opbase = regg & 0x1f;
        chbase = regg & 0xf;

        if(opbase > 5)
        {
            if(opbase > 13)
            {
                if(opbase > 21)
                    opbase = 18;
                else
                    opbase -= 4;
            }
            else
                opbase -= 2;
        }

        if(hb)
        {
            opbase += 18;
            chbase += 9;
        }

        switch(hr)
        {
        case 0xa:
            if(chbase < 18)
                do_fnum(&chip->channels[chbase], data);
            break;
        case 0xb:
            if(chbase < 18)
                do_block(&chip->channels[chbase], data);
            break;
        case 0xc:
            if(chbase < 18)
            {
                chip->channels[chbase].cha = (data >> 4) & 0x1;
                chip->channels[chbase].chb = (data >> 5) & 0x1;
                chip->channels[chbase].fb = (data >> 1) & 0x7;
                chip->channels[chbase].con = data & 0x1;
            }
            break;
        case 0xe:
        case 0xf:
            if(opbase < 36)
                do_wf(&chip->slots[opbase], data);
            break;
        case 0x8:
        case 0x9:
            if(opbase < 36)
                do_slrr(&chip->slots[opbase], data);
            break;
        case 0x6:
        case 0x7:
            if(opbase < 36)
                do_ardr(&chip->slots[opbase], data);
            break;
        case 0x4:
        case 0x5:
            if(opbase < 36)
                do_ksltl(&chip->slots[opbase], data);
            break;
        case 0x2:
        case 0x3:
            if(opbase < 36)
                do_avekm(&chip->slots[opbase], data);
            break;
        }
    }

    return;
}

static short limshort(int a)
{
    if(a > 32767)
        a = 32767;
    else if(a < -32768)
        a = -32768;

    return (short)(a);
}

static void vpc_opl_fetch(struct vpc_opl *chip, size_t ln)
{
    int32_t outbuff[1024];
    uint32_t j, k;

    if(ln >= 1024)
        ln = 1024; /* Can only fetch 1024 bytes at once! */

    memset(chip->accbuff[0], 0, 2 * ln);
    memset(chip->accbuff[1], 0, 2 * ln);

    for(j = 0; j < 18; j++)
    {
        memset(outbuff, 0, 4 * ln);
        advance_chan(&chip->channels[j], outbuff, ln);

        for(k = 0; k < ln; k++)
        {
            if(chip->opl3 == 1)
            {
                chip->accbuff[0][k] = limshort(chip->accbuff[0][k] + chip->channels[j].cha * outbuff[k] / 4);
                chip->accbuff[1][k] = limshort(chip->accbuff[1][k] + chip->channels[j].chb * outbuff[k] / 4);
            }
            else
            {
                chip->accbuff[0][k] = limshort(chip->accbuff[0][k] + outbuff[k] / 4);
                chip->accbuff[1][k] = limshort(chip->accbuff[1][k] + outbuff[k] / 4);
            }
        }
    }

    chip->accbuff_off = 0;
    chip->accbuff_size = ln;
}

void vpc_opl_getoutput(void *opl3, int16_t *buffer, uint32_t len)
{
    uint32_t i;
    struct vpc_opl *chip = (struct vpc_opl *)opl3;

    for(i = 0; i < len; ++i)
    {
        if(chip->accbuff_size == 0 || chip->accbuff_off >= chip->accbuff_size)
            vpc_opl_fetch(chip, (size_t)(len * rsm_dur / chip->out_rateratio));

        while(chip->out_samplecnt < rsm_dur)
        {
            chip->out_samples[0] = chip->accbuff[0][chip->accbuff_off];
            chip->out_samples[1] = chip->accbuff[1][chip->accbuff_off];
            chip->out_samplecnt += chip->out_rateratio;
            ++chip->accbuff_off;
        }

        *buffer++ = (chip->out_samples[0] >> 1);
        *buffer++ = (chip->out_samples[1] >> 1);
        chip->out_samplecnt -= rsm_dur;
    }
}
