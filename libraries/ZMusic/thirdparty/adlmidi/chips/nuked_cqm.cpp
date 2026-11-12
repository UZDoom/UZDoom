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

#include "nuked_cqm.h"
#include "nuked_cqm/cqm.h"
#include <cstring>

NukedCQM::NukedCQM() :
    OPLChipBaseT()
{
    m_chip = new cqm_t;
    NukedCQM::setRate(m_rate);
}

NukedCQM::~NukedCQM()
{
    cqm_t *chip_r = reinterpret_cast<cqm_t*>(m_chip);
    delete chip_r;
}

void NukedCQM::setRate(uint32_t rate)
{
    OPLChipBaseT::setRate(rate);
    cqm_t *chip_r = reinterpret_cast<cqm_t*>(m_chip);
    std::memset(chip_r, 0, sizeof(cqm_t));
    CQM_Reset(chip_r, rate, nativeRate);
}

void NukedCQM::reset()
{
    OPLChipBaseT::reset();
    cqm_t *chip_r = reinterpret_cast<cqm_t*>(m_chip);
    std::memset(chip_r, 0, sizeof(cqm_t));
    CQM_Reset(chip_r, m_rate, nativeRate);
}

void NukedCQM::writeReg(uint16_t addr, uint8_t data)
{
    cqm_t *chip_r = reinterpret_cast<cqm_t*>(m_chip);
    CQM_WriteRegBuffered(chip_r, addr, data);
}

void NukedCQM::writePan(uint16_t addr, uint8_t data)
{
    (void)addr; (void)data;
    // cqm_t *chip_r = reinterpret_cast<cqm_t*>(m_chip);
    // CQM_WritePan(chip_r, addr, data);
}

void NukedCQM::nativeGenerate(int16_t *frame)
{
    cqm_t *chip_r = reinterpret_cast<cqm_t*>(m_chip);
    CQM_Generate(chip_r, frame);
    frame[0] /= 2;
    frame[1] /= 2;
}

const char *NukedCQM::emulatorName()
{
    return "Nuked CQM";
}

bool NukedCQM::hasFullPanning()
{
    return false;
}

OPLChipBase::ChipType NukedCQM::chipType()
{
    return CHIPTYPE_OPL3;
}
