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

#include <string.h>
#include "vpc_opl3_emu.h"
#include "vpc_opl3/vpc_opl3.h"

VpcOPL3::VpcOPL3() :
    OPLChipBaseBufferedT(),
    m_chip(vpc_opl_init())
{
    VpcOPL3::reset();
}

VpcOPL3::~VpcOPL3()
{
    if(m_chip)
        vpc_opl_free(m_chip);

    m_chip = NULL;
}

void VpcOPL3::setRate(uint32_t rate)
{
    OPLChipBaseBufferedT::setRate(rate);
    vpc_opl_reset(m_chip);
}

void VpcOPL3::reset()
{
    OPLChipBaseBufferedT::reset();
    vpc_opl_reset(m_chip);
}

void VpcOPL3::writeReg(uint16_t addr, uint8_t data)
{
    vpc_opl_writereg(m_chip, addr, data);
}

void VpcOPL3::writePan(uint16_t /*addr*/, uint8_t /*data*/)
{}

void VpcOPL3::nativeGenerateN(int16_t *output, size_t frames)
{
    vpc_opl_getoutput(m_chip, output, frames);
}

const char *VpcOPL3::emulatorName()
{
    return "VirtuaPC OPL3";
}

OPLChipBase::ChipType VpcOPL3::chipType()
{
    return CHIPTYPE_OPL3;
}

bool VpcOPL3::hasFullPanning()
{
    return false;
}
