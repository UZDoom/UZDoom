/*
 * Copyright (C) 2023 nukeykt
 *
 * This file is part of YM3812-LLE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  YM3812 emulator
 *  Thanks:
 *      Travis Goodspeed:
 *          YM3812 decap and die shot
 *
 */

#pragma once

#ifndef NUKED_FMOPL2_H
#define NUKED_FMOPL2_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    uint_fast32_t mclk;
    uint_fast32_t address;
    uint_fast32_t data_i;
    uint_fast32_t ic;
    uint_fast32_t cs;
    uint_fast32_t rd;
    uint_fast32_t wr;
} fmopl2_input_t;

typedef struct
{
    fmopl2_input_t input;

    uint_fast32_t mclk1;
    uint_fast32_t mclk2;
    uint_fast32_t clk1;
    uint_fast32_t clk2;

    uint_fast32_t prescaler_reset_l[2];
    uint_fast32_t prescaler_cnt[2];
    uint_fast32_t prescaler_l1[2];
    uint_fast32_t prescaler_l2[2];

    uint_fast32_t reset1;

    uint_fast32_t fsm_reset_l[2];
    uint_fast32_t fsm_reset; /* wire */
    uint_fast32_t fsm_cnt1[2];
    uint_fast32_t fsm_cnt2[2];
    uint_fast32_t fsm_cnt1_of; /* wire */
    uint_fast32_t fsm_cnt2_of; /* wire */
    uint_fast32_t fsm_sel[13];
    uint_fast32_t fsm_cnt; /* wire */
    uint_fast32_t fsm_ch_out;
    uint_fast32_t fsm_do_fb;
    uint_fast32_t fsm_load_fb;
    uint_fast32_t fsm_l1[2];
    uint_fast32_t fsm_l2[2];
    uint_fast32_t fsm_l3[2];
    uint_fast32_t fsm_l4[2];
    uint_fast32_t fsm_l5[2];
    uint_fast32_t fsm_l6[2];
    uint_fast32_t fsm_out[16];

    uint_fast32_t io_rd;
    uint_fast32_t io_wr;
    uint_fast32_t io_cs;
    uint_fast32_t io_a0;

    uint_fast32_t io_read0;
    uint_fast32_t io_read1;
    uint_fast32_t io_write;
    uint_fast32_t io_write0;
    uint_fast32_t io_write1;
    uint_fast32_t io_dir;
    uint_fast32_t io_data;

    uint_fast32_t data_latch;

    uint_fast32_t write0;
    uint_fast32_t write0_sr;
    uint_fast32_t write0_latch[6];
    uint_fast32_t write1;
    uint_fast32_t write1_sr;
    uint_fast32_t write1_latch[6];

    uint_fast32_t reg_sel1;
    uint_fast32_t reg_sel2;
    uint_fast32_t reg_sel3;
    uint_fast32_t reg_sel4;
    uint_fast32_t reg_sel8;
    uint_fast32_t reg_selbd;
    uint_fast32_t reg_test;
    uint_fast32_t reg_timer1;
    uint_fast32_t reg_timer2;
    uint_fast32_t reg_notesel;
    uint_fast32_t reg_csm;
    uint_fast32_t reg_da;
    uint_fast32_t reg_dv;
    uint_fast32_t rhythm;
    uint_fast32_t reg_rh_kon;
    uint_fast32_t reg_sel4_wr; /* wire */
    uint_fast32_t reg_sel4_rst; /* wire */
    uint_fast32_t reg_t1_mask;
    uint_fast32_t reg_t2_mask;
    uint_fast32_t reg_t1_start;
    uint_fast32_t reg_t2_start;
    uint_fast32_t reg_mode_b3;
    uint_fast32_t reg_mode_b4;

    uint_fast32_t t1_cnt[2];
    uint_fast32_t t2_cnt[2];
    uint_fast32_t t1_of[2];
    uint_fast32_t t2_of[2];
    uint_fast32_t t1_status;
    uint_fast32_t t2_status;
    uint_fast32_t unk_status1;
    uint_fast32_t unk_status2;
    uint_fast32_t timer_st_load_l;
    uint_fast32_t timer_st_load;
    uint_fast32_t t1_start;
    uint_fast32_t t1_start_l[2];
    uint_fast32_t t2_start_l[2];
    uint_fast32_t t1_load; /* wire */
    uint_fast32_t csm_load_l;
    uint_fast32_t csm_load;
    uint_fast32_t csm_kon;
    uint_fast32_t rh_sel0;
    uint_fast32_t rh_sel[2];

    uint_fast32_t keyon_comb;
    uint_fast32_t address;
    uint_fast32_t address_valid;
    uint_fast32_t address_valid_l[2];
    uint_fast32_t address_valid2;
    uint_fast32_t data;
    uint_fast32_t slot_cnt1[2];
    uint_fast32_t slot_cnt2[2];
    uint_fast32_t slot_cnt;
    uint_fast32_t sel_ch;

    uint_fast32_t ch_fnum[10][2];
    uint_fast32_t ch_block[3][2];
    uint_fast32_t ch_keyon[2];
    uint_fast32_t ch_connect[2];
    uint_fast32_t ch_fb[3][2];
    uint_fast32_t op_multi[4][2];
    uint_fast32_t op_ksr[2];
    uint_fast32_t op_egt[2];
    uint_fast32_t op_vib[2];
    uint_fast32_t op_am[2];
    uint_fast32_t op_tl[6][2];
    uint_fast32_t op_ksl[2][2];
    uint_fast32_t op_ar[4][2];
    uint_fast32_t op_dr[4][2];
    uint_fast32_t op_sl[4][2];
    uint_fast32_t op_rr[4][2];
    uint_fast32_t op_wf[2][2];
    uint_fast32_t op_mod[2];
    uint_fast32_t op_value; /* wire */

    uint_fast32_t eg_load1_l;
    uint_fast32_t eg_load1;
    uint_fast32_t eg_load2_l;
    uint_fast32_t eg_load2;
    uint_fast32_t eg_load3_l;
    uint_fast32_t eg_load3;

    uint_fast32_t trem_carry[2];
    uint_fast32_t trem_value[2];
    uint_fast32_t trem_dir[2];
    uint_fast32_t trem_step;
    uint_fast32_t trem_out;
    uint_fast32_t trem_of[2];

    uint_fast32_t eg_timer[2];
    uint_fast32_t eg_timer_masked[2];
    uint_fast32_t eg_carry[2];
    uint_fast32_t eg_mask[2];
    uint_fast32_t eg_subcnt[2];
    uint_fast32_t eg_subcnt_l[2];
    uint_fast32_t eg_sync_l[2];
    uint_fast32_t eg_timer_low;
    uint_fast32_t eg_shift;
    uint_fast32_t eg_state[2][2];
    uint_fast32_t eg_level[9][2];
    uint_fast32_t eg_out[2];
    uint_fast32_t eg_dokon; /* wire */
    uint_fast32_t eg_mute[2];

    uint_fast32_t block;
    uint_fast32_t fnum;
    uint_fast32_t keyon;
    uint_fast32_t connect;
    uint_fast32_t connect_l[2];
    uint_fast32_t fb;
    uint_fast32_t fb_l[2][2];
    uint_fast32_t multi;
    uint_fast32_t ksr;
    uint_fast32_t egt;
    uint_fast32_t vib;
    uint_fast32_t am;
    uint_fast32_t tl;
    uint_fast32_t ksl;
    uint_fast32_t ar;
    uint_fast32_t dr;
    uint_fast32_t sl;
    uint_fast32_t rr;
    uint_fast32_t wf;

    uint_fast32_t lfo_cnt[2];
    uint_fast32_t t1_step; /* wire */
    uint_fast32_t t2_step; /* wire */
    uint_fast32_t am_step; /* wire */
    uint_fast32_t vib_step; /* wire */
    uint_fast32_t vib_cnt[2];
    uint_fast32_t pg_phase[19][2];
    uint_fast32_t dbg_serial[2];

    uint_fast32_t noise_lfsr[2];

    uint_fast32_t hh_load;
    uint_fast32_t tc_load;
    uint_fast32_t hh_bit2;
    uint_fast32_t hh_bit3;
    uint_fast32_t hh_bit7;
    uint_fast32_t hh_bit8;
    uint_fast32_t tc_bit3;
    uint_fast32_t tc_bit5;
    uint_fast32_t op_logsin[2];
    uint_fast32_t op_shift[2];
    uint_fast32_t op_pow[2];
    uint_fast32_t op_mute[2];
    uint_fast32_t op_sign[2];
    uint_fast32_t op_fb[2][13][2];

    uint_fast32_t pg_out; /* wire */
    uint_fast32_t pg_out_rhy; /* wire */

    uint_fast32_t accm_value[2];
    uint_fast32_t accm_shifter[2];
    uint_fast32_t accm_load1_l;
    uint_fast32_t accm_load1;
    uint_fast32_t accm_clamplow;
    uint_fast32_t accm_clamphigh;
    uint_fast32_t accm_top;
    uint_fast32_t accm_sel[2];
    uint_fast32_t accm_mo[2];

    uint_fast32_t o_sh;
    uint_fast32_t o_mo;
    uint_fast32_t o_irq_pull;
    uint_fast32_t o_sy;

    uint_fast32_t data_o;
    uint_fast32_t data_z;

    uint_fast32_t o_clk1;
    uint_fast32_t o_clk2;
    uint_fast32_t o_reset1;
    uint_fast32_t o_write0;
    uint_fast32_t o_write1;
    uint_fast32_t o_data_latch;

} fmopl2_t;

extern void FMOPL2_Clock(fmopl2_t *chip);

#ifdef __cplusplus
}
#endif

#endif /* NUKED_FMOPL2_H */
