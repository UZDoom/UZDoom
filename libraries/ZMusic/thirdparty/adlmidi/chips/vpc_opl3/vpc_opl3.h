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

#ifndef VPC_OPL3_H
#define VPC_OPL3_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* vpc_opl_init();
void vpc_opl_free(void *opl3);

void vpc_opl_set_rate(void *opl3, uint32_t rate);

void vpc_opl_reset(void *opl3);

void vpc_opl_writereg(void *opl3, uint16_t regg, uint8_t data);
void vpc_opl_getoutput(void *opl3, int16_t *buffer, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* VPC_OPL3_H */
