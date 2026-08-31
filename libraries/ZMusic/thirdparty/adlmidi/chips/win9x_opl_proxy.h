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

#ifndef WIN9X_OPL_PROXY_H
#define WIN9X_OPL_PROXY_H

#include "opl_chip_base.h"

class Win9x_OPL_Proxy : public OPLChipBaseT<Win9x_OPL_Proxy>
{
    void *m_chip;
    void initChip();
    void closeChip();

public:
    Win9x_OPL_Proxy();
    virtual ~Win9x_OPL_Proxy() override;

    void startChip();

    bool canSetOplAddress() const;
    void setOplAddress(uint16_t address);

    bool canRunAtPcmRate() const override { return false; }
    void setRate(uint32_t /*rate*/) override {}
    void reset() override {}
    void writeReg(uint16_t addr, uint8_t data) override;
    void nativePreGenerate() override {}
    void nativePostGenerate() override {}
    void nativeGenerate(int16_t *frame) override;
    const char *emulatorName() override;
    ChipType chipType() override;
    bool hasFullPanning() override;
};

#endif // WIN9X_OPL_PROXY_H
