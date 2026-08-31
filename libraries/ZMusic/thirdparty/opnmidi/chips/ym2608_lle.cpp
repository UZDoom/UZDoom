/*
 * Interfaces over Yamaha OPL3 (YMF262) chip emulators
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

#include "ym2608_lle.h"
#include "nuked_lle/nopna.h"
#include <cstring>

Ym2608LLEOPNA::Ym2608LLEOPNA(OPNFamily f) :
    OPNChipBaseT(f)
{
    m_chip = nopna_init(m_clock, m_rate);
    Ym2608LLEOPNA::setRate(m_rate, m_clock);
}

Ym2608LLEOPNA::~Ym2608LLEOPNA()
{
    nopna_shutdown(m_chip);
}

void Ym2608LLEOPNA::setRate(uint32_t rate, uint32_t clock)
{
    OPNChipBaseT::setRate(rate, clock);
    nopna_set_rate(m_chip, clock, rate);
    nopna_write_buffered(m_chip, 0, 0x29);
    nopna_write_buffered(m_chip, 1, 0x9f);
}

void Ym2608LLEOPNA::reset()
{
    OPNChipBaseT::reset();
    nopna_reset(m_chip);
    nopna_write_buffered(m_chip, 0, 0x29);
    nopna_write_buffered(m_chip, 1, 0x9f);
}

void Ym2608LLEOPNA::writeReg(uint32_t port, uint16_t addr, uint8_t data)
{
    nopna_write_buffered(m_chip, 0 + (port) * 2, addr);
    nopna_write_buffered(m_chip, 1 + (port) * 2, data);
}

void Ym2608LLEOPNA::writePan(uint16_t addr, uint8_t data)
{
    (void)addr;
    (void)data;
    // Uninmplemented
}

void Ym2608LLEOPNA::nativeGenerate(int16_t *frame)
{
    nopna_getsample_one_native(m_chip, frame);
}

const char *Ym2608LLEOPNA::emulatorName()
{
    return "YM2608-LLE OPNA";
}

bool Ym2608LLEOPNA::hasFullPanning()
{
    return false;
}
