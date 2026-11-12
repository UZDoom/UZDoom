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

#ifndef ADLMIDI_ARR_HPP_THING
#define ADLMIDI_ARR_HPP_THING

#include <stddef.h>
#include <cstdlib>

/**
 * \file adlmidi_arr.hpp
 * \brief A custom and simple std::vector-like dynamic array
 */

template<class T, bool is_class=false>
struct adl_array
{
#if defined(__DJGPP__)
    void dpmi_lock_begin() {}
#endif

    T *data;
    size_t size;

    adl_array() :
        data(NULL),
        size(0)
    {}

    ~adl_array()
    {
        clear();
    }

    bool empty()
    {
        return size == 0;
    }

    void fill(const T &value)
    {
        for(size_t i = 0; i < size; ++i)
            data[i] = value;
    }

    void resize_fill(size_t count, const T &value)
    {
        if(data)
            clear();

        data = (T*)std::malloc(count * sizeof(T));
        size = count;

#ifdef ENABLE_HW_OPL_DOS
        adl_dpmi_lock_memory(data, count * sizeof(T));
#endif

        for(size_t i = 0; i < size; ++i)
        {
            if(is_class)
                new (data + i) T();
            data[i] = value;
        }
    }

    void resize(size_t count)
    {
        if(data)
            clear();

        data = (T*)std::malloc(count * sizeof(T));
        size = count;
#ifdef ENABLE_HW_OPL_DOS
        adl_dpmi_lock_memory(data, count * sizeof(T));
#endif

        if(is_class)
        {
            for(size_t i = 0; i < size; ++i)
                new (data + i) T();
        }
    }

    void expand(size_t count)
    {
        T *old_data = data;

        if(size > count)
            return; // Nothing to expand!

        data = (T*)std::malloc(count * sizeof(T));

#ifdef ENABLE_HW_OPL_DOS
        adl_dpmi_lock_memory(data, count * sizeof(T));
#endif

        if(is_class)
        {
            // Copy old data
            for(size_t i = 0; i < size; ++i)
                new (data + i) T(old_data[i]);

            // Initialize new data
            for(size_t i = size; i < count; ++i)
                new (data + i) T();

            // Delete old data
            for(size_t i = 0; i < size; ++i)
                old_data[i].~T();
        }
        else
        {
            // Copy old data
            for(size_t i = 0; i < size; ++i)
                data[i] = old_data[i];
        }

#ifdef ENABLE_HW_OPL_DOS
        adl_dpmi_unlock_memory(old_data, size * sizeof(T));
#endif
        std::free(old_data);

        size = count;
    }

    void clear()
    {
        if(data)
        {
            if(is_class)
            {
                for(size_t i = 0; i < size; ++i)
                    data[i].~T();
            }

#ifdef ENABLE_HW_OPL_DOS
            adl_dpmi_unlock_memory(data, size * sizeof(T));
#endif
            std::free(data);
        }

        data = NULL;
        size = 0;
    }

    T &operator[](size_t i)
    {
        return data[i];
    }

    const T &operator[](size_t i) const
    {
        return data[i];
    }

#if defined(__DJGPP__)
    void dpmi_lock_end() {}
#endif
};

#endif //ADLMIDI_ARR_HPP_THING
