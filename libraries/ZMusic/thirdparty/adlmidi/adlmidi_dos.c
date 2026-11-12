/*
 * libADLMIDI is a free Software MIDI synthesizer library with OPL3 emulation
 *
 * Original ADLMIDI code: Copyright (c) 2010-2014 Joel Yliluoma <bisqwit@iki.fi>
 * ADLMIDI Library API:   Copyright (c) 2015-2026 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Library is based on the ADLMIDI, a MIDI player for Linux and Windows with OPL3 emulation:
 * http://iki.fi/bisqwit/source/adlmidi.html
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dpmi.h>
#include <sys/segments.h>
#include "adlmidi_dos.h"


ADL_Bool adl_dpmi_lock_memory(void *address, size_t size)
{
    unsigned long baseaddr;
    __dpmi_meminfo mem;

    if(__dpmi_get_segment_base_address(_my_ds(), &baseaddr) == -1)
        return ADL_FALSE;

    mem.handle = 0;
    mem.address = baseaddr + (ADL_SIntPtr)address;
    mem.size = size;

    return __dpmi_lock_linear_region(&mem) != -1;
}

ADL_Bool adl_dpmi_lock_memory_code(void *address, size_t size)
{
    unsigned long baseaddr;
    __dpmi_meminfo mem;

    if(__dpmi_get_segment_base_address(_my_ds(), &baseaddr) == -1)
        return ADL_FALSE;

    mem.handle = 0;
    mem.address = baseaddr + (ADL_SIntPtr)address;
    mem.size = size;

    return __dpmi_lock_linear_region(&mem) != -1;
}

ADL_Bool adl_dpmi_lock_region(void *begin, void *end)
{
    return adl_dpmi_lock_memory(begin, (ADL_UIntPtr)end - (ADL_UIntPtr)begin);
}

ADL_Bool adl_dpmi_lock_code_region(void *begin, void *end)
{
    return adl_dpmi_lock_memory_code(begin, (ADL_UIntPtr)end - (ADL_UIntPtr)begin);
}

ADL_Bool adl_dpmi_lock_code_region_fn(ADL_LockFnPtr fn_begin, ADL_LockFnPtr fn_end)
{
    return adl_dpmi_lock_code_region((void*)(ADL_UIntPtr)fn_begin, (void*)(ADL_UIntPtr)fn_end);
}

ADL_Bool adl_dpmi_unlock_memory(void *address, size_t size)
{
    unsigned long baseaddr;
    __dpmi_meminfo mem;

    if(__dpmi_get_segment_base_address(_my_ds(), &baseaddr) == -1)
        return ADL_FALSE;

    mem.handle = 0;
    mem.address = baseaddr + (ADL_SIntPtr)address;
    mem.size = size;

    return __dpmi_unlock_linear_region(&mem) != -1;
}

ADL_Bool adl_dpmi_unlock_memory_code(void *address, size_t size)
{
    unsigned long baseaddr;
    __dpmi_meminfo mem;

    if(__dpmi_get_segment_base_address(_my_ds(), &baseaddr) == -1)
        return ADL_FALSE;

    mem.handle = 0;
    mem.address = baseaddr + (ADL_SIntPtr)address;
    mem.size = size;

    return __dpmi_unlock_linear_region(&mem) != -1;
}

ADL_Bool adl_dpmi_unlock_region(void *begin, void *end)
{
    return adl_dpmi_unlock_memory(begin, (ADL_UIntPtr)end - (ADL_UIntPtr)begin);
}

ADL_Bool adl_dpmi_unlock_code_region(void *begin, void *end)
{
    return adl_dpmi_unlock_memory_code(begin, (ADL_UIntPtr)end - (ADL_UIntPtr)begin);
}

ADL_Bool adl_dpmi_unlock_code_region_fn(ADL_LockFnPtr fn_begin, ADL_LockFnPtr fn_end)
{
    return adl_dpmi_unlock_code_region((void*)(ADL_UIntPtr)fn_begin, (void*)(ADL_UIntPtr)fn_end);
}
