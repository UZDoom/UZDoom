/*
 * Interfaces over Yamaha OPL2 (YM3812) and Yamaha OPL3 (YMF262) chip emulators
 *
 * Copyright (c) 2017-2026 Vitaly Novichkov (Wohlstand)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "nuked_opl2.h"
#include "nuked/nukedopl2.h"
#include <cstring>

NukedOPL2::NukedOPL2() :
    OPLChipBaseT()
{
    m_chip = new opl2_chip;
    NukedOPL2::setRate(m_rate);
}

NukedOPL2::~NukedOPL2()
{
    opl2_chip *chip_r = reinterpret_cast<opl2_chip*>(m_chip);
    delete chip_r;
}

void NukedOPL2::setRate(uint32_t rate)
{
    OPLChipBaseT::setRate(rate);
    opl2_chip *chip_r = reinterpret_cast<opl2_chip*>(m_chip);
    std::memset(chip_r, 0, sizeof(opl2_chip));
    OPL2_Reset(chip_r, rate);
}

void NukedOPL2::reset()
{
    OPLChipBaseT::reset();
    opl2_chip *chip_r = reinterpret_cast<opl2_chip*>(m_chip);
    std::memset(chip_r, 0, sizeof(opl2_chip));
    OPL2_Reset(chip_r, m_rate);
}

void NukedOPL2::writeReg(uint16_t addr, uint8_t data)
{
    opl2_chip *chip_r = reinterpret_cast<opl2_chip*>(m_chip);
    OPL2_WriteRegBuffered(chip_r, addr, data);
}

void NukedOPL2::writePan(uint16_t /*addr*/, uint8_t /*data*/)
{
    /* Monophonic! */
}

void NukedOPL2::nativeGenerate(int16_t *frame)
{
    opl2_chip *chip_r = reinterpret_cast<opl2_chip*>(m_chip);
    OPL2_Generate(chip_r, frame);
    frame[1] = frame[0];
}

const char *NukedOPL2::emulatorName()
{
    return "Nuked OPL2 Lite";
}

bool NukedOPL2::hasFullPanning()
{
    return false;
}

OPLChipBase::ChipType NukedOPL2::chipType()
{
    return CHIPTYPE_OPL2;
}
