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

#pragma once
#ifndef OPL_FREQ_TABLES_H
#define OPL_FREQ_TABLES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/***************************************************************
 *                     Frequency models                        *
 ***************************************************************/

/**
 * @brief Generic frequency formula
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPL chips
 */
extern uint16_t oplModel_genericFreq(double tone, uint32_t *mul_offset);

/**
 * @brief Frequency formula that replicates behaviour of the DMX library
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPL chips
 */
extern uint16_t oplModel_dmxFreq(double tone, uint32_t *mul_offset);

/**
 * @brief Frequency formula that replicates behaviour of the Apogee Sound System library
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPL chips
 */
extern uint16_t oplModel_apogeeFreq(double tone, uint32_t *mul_offset);

/**
 * @brief Frequency formula that replicates behaviour of Windows 9x OPL2/OPL3 drivers
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPL chips
 */
extern uint16_t oplModel_9xFreq(double tone, uint32_t *mul_offset);

/**
 * @brief Frequency formula that replicates behaviour of the HMI Sound Operating System library
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPL chips
 */
extern uint16_t oplModel_hmiFreq(double tone, uint32_t *mul_offset);

/**
 * @brief Frequency formula that replicates behaviour of the Audio Interfaces Library (Miles Sound System)
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPL chips
 */
extern uint16_t oplModel_ailFreq(double tone, uint32_t *mul_offset);

/**
 * @brief Frequency formula that replicates behaviour of AdLib, Sound Blaster 1.x / 2.x drivers for Windows 3.x
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPL chips
 */
extern uint16_t oplModel_msAdLibFreq(double tone, uint32_t *mul_offset);

/**
 * @brief Frequency formula that replicates behaviour of AdLib, Sound Blaster 1.x / 2.x drivers for Windows 3.x
 * @param tone MIDI Note semi-tone with detune (decimal is a detune)
 * @param mul_offset !REQUIRED! A pointer to the frequency multiplier offset if note is too high
 * @return FNum+Block value compatible to OPL chips
 */
extern uint16_t oplModel_OConnellFreq(double tone, uint32_t *mul_offset);


/***************************************************************
 *                   Volume scaling models                     *
 ***************************************************************/

/**
 * @brief OPL Voice mode
 */
enum OPLVoiceModes
{
    OPLVoice_MODE_2op_FM = 0,
    OPLVoice_MODE_2op_AM = 1,
    OPLVoice_MODE_4op_1_2_FM_FM = 2,
    OPLVoice_MODE_4op_1_2_AM_FM = 3,
    OPLVoice_MODE_4op_1_2_FM_AM = 4,
    OPLVoice_MODE_4op_1_2_AM_AM = 5,
    OPLVoice_MODE_4op_3_4_FM_FM = 6,
    OPLVoice_MODE_4op_3_4_AM_FM = 7,
    OPLVoice_MODE_4op_3_4_FM_AM = 8,
    OPLVoice_MODE_4op_3_4_AM_AM = 9
};

/**
 * @brief Volume calculation context
 */
struct OPLVolume_t
{
    /*! Input MIDI key velocity */
    uint_fast8_t vel;
    /*! Input MIDI channel volume (CC7) */
    uint_fast8_t chVol;
    /*! Input MIDI channel expression (CC11) */
    uint_fast8_t chExpr;
    /*! Master volume level (0...127) */
    uint_fast8_t masterVolume;
    /*! OPL Voice mode (see OPLVoiceModes structure) */
    uint_fast8_t voiceMode;
    /*! Feedback+Connection OPL byte. Used by old HMI SOS volume model only. */
    uint_fast8_t fbConn;
    /*! Total level byte for modulator (should be cleaned from KSL bits) */
    uint_fast8_t tlMod;
    /*! Total level byte for carrier (should be cleaned from KSL bits) */
    uint_fast8_t tlCar;
    /*! 0 - don't alternate modulator, 1 - apply change to modulator */
    unsigned int doMod;
    /*! 0 - don't alternate carrier, 1 - apply change to carrier */
    unsigned int doCar;
    /*! Is a percussion instrument played? Used by old HMI SOS volume model only. */
    unsigned int isDrum;
};

/**
 * @brief Generic volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_genericVolume(struct OPLVolume_t *v);

/**
 * @brief OPL native volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_nativeVolume(struct OPLVolume_t *v);

/**
 * @brief RSXX specific volume model. It should never been referred via WOPL bank format.
 * @param v [inout] Volume calculation context
 */
extern void oplModel_rsxxVolume(struct OPLVolume_t *v);


/**
 * @brief Original DMX volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_dmxOrigVolume(struct OPLVolume_t *v);

/**
 * @brief Fixed DMX volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_dmxFixedVolume(struct OPLVolume_t *v);


/**
 * @brief Original Apogee Sound System volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_apogeeOrigVolume(struct OPLVolume_t *v);

/**
 * @brief Fixed Apogee Sound System volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_apogeeFixedVolume(struct OPLVolume_t *v);


/**
 * @brief Generic FM Win9x volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_9xGenericVolume(struct OPLVolume_t *v);

/**
 * @brief SoundBlaster 16 FM Win9x volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_9xSB16Volume(struct OPLVolume_t *v);


/**
 * @brief Audio Interfaces Library volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_ailVolume(struct OPLVolume_t *v);


/**
 * @brief Old HMI Sound Operating System volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_sosOldVolume(struct OPLVolume_t *v);

/**
 * @brief New HMI Sound Operating System volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_sosNewVolume(struct OPLVolume_t *v);


/**
 * @brief Win3x MS AdLib volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_msAdLibVolume(struct OPLVolume_t *v);



/**
 * @brief Jamie O'Connell's FM Synth driver volume model
 * @param v [inout] Volume calculation context
 */
extern void oplModel_OConnellVolume(struct OPLVolume_t *v);




/***************************************************************
 *              XG CC74 Brightness scale formula               *
 ***************************************************************/

/**
 * @brief Converts XG brightness (CC74) controller value into OPL volume for the modulator
 * @param brightness Value of CC74 (0 - 127)
 * @return Converted result (0 - 63)
 */
extern uint_fast16_t oplModels_xgBrightnessToOPL(uint_fast16_t brightness);



#ifdef __cplusplus
}
#endif

#endif /* OPL_FREQ_TABLES_H */
