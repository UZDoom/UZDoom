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

#pragma once
#ifndef OPN_FREQ_TABLES_H
#define OPN_FREQ_TABLES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/***************************************************************
 *                     Frequency models                        *
 ***************************************************************/

/**
 * @brief Generic frequency formula for OPN2 chips
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPN chips
 */
extern uint16_t opnModel_genericFreqOPN2(double tone, uint32_t *mul_offset);

/**
 * @brief Generic frequency formula for OPNA chips
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPN chips
 */
extern uint16_t opnModel_genericFreqOPNA(double tone, uint32_t *mul_offset);



/***************************************************************
 *                   Volume scaling models                     *
 ***************************************************************/

/**
 * @brief Volume calculation context
 */
struct OPNVolume_t
{
    /*! Input MIDI key velocity */
    uint_fast8_t vel;
    /*! Input MIDI channel volume (CC7) */
    uint_fast8_t chVol;
    /*! Input MIDI channel expression (CC11) */
    uint_fast8_t chExpr;
    /*! Master volume level (0...127) */
    uint_fast8_t masterVolume;
    /*! OPN Voice mode (see OPNVoiceModes structure) */
    uint_fast8_t algorithm;
    /*! Total level byte for every operator (should be cleaned from KSL bits) */
    uint_fast8_t tlOp[4];
    /*! 0 - don't alternate operator, 1 - apply change to operator */
    unsigned int doOp[4];
};

/**
 * @brief Generic volume model
 * @param v [inout] Volume calculation context
 */
extern void opnModel_genericVolume(struct OPNVolume_t *v);

/**
 * @brief OPL native volume model
 * @param v [inout] Volume calculation context
 */
extern void opnModel_nativeVolume(struct OPNVolume_t *v);

/**
 * @brief Original DMX volume model
 * @param v [inout] Volume calculation context
 */
extern void opnModel_dmxLikeVolume(struct OPNVolume_t *v);

/**
 * @brief Original Apogee Sound System volume model
 * @param v [inout] Volume calculation context
 */
extern void opnModel_apogeeLikeVolume(struct OPNVolume_t *v);

/**
 * @brief SoundBlaster 16 FM Win9x volume model
 * @param v [inout] Volume calculation context
 */
extern void opnModel_9xLikeVolume(struct OPNVolume_t *v);



/***************************************************************
 *              XG CC74 Brightness scale formula               *
 ***************************************************************/

/**
 * @brief Converts XG brightness (CC74) controller value into OPL volume for the modulator
 * @param brightness Value of CC74 (0 - 127)
 * @return Converted result (0 - 63)
 */
extern uint_fast16_t opnModels_xgBrightnessToOPN(uint_fast16_t brightness);



#ifdef __cplusplus
}
#endif

#endif /* OPN_FREQ_TABLES_H */
