/*
 * BW_Midi_Sequencer - MIDI Sequencer for C++
 *
 * Copyright (c) 2015-2026 Vitaly Novichkov <admin@wohlnet.ru>
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
#if !defined(BW_MIDISEQ_DPMI_ALLOC_HPP)
#define BW_MIDISEQ_DPMI_ALLOC_HPP

// #define DPMI_DEBUG

#if defined(__DJGPP__)
#include <cstddef>
#include <cstdlib>
#include <new>
#include <stdint.h>
#include <limits>

#include <dpmi.h>
#include <sys/segments.h>

#ifdef DPMI_DEBUG
#   include <stdio.h>
#endif

struct dpmi_allocator_impl
{
    static bool dpmi_lock_memory(void *address, size_t size)
    {
        unsigned long baseaddr;
        __dpmi_meminfo mem;

        if(__dpmi_get_segment_base_address(_my_ds(), &baseaddr) == -1)
            return false;

        mem.handle = 0;
        mem.address = baseaddr + (intptr_t)address;
        mem.size = size;

        return __dpmi_lock_linear_region(&mem) != -1;
    }

    static bool dpmi_unlock_memory(void *address, size_t size)
    {
        unsigned long baseaddr;
        __dpmi_meminfo mem;

        if(__dpmi_get_segment_base_address(_my_ds(), &baseaddr) == -1)
            return false;

        mem.handle = 0;
        mem.address = baseaddr + (intptr_t)address;
        mem.size = size;

        return __dpmi_unlock_linear_region(&mem) != -1;
    }

    static bool dpmi_lock_region(void *begin, void *end)
    {
        char *b, *e;

        b = (char*)begin;
        e = (char*)end;

        return dpmi_lock_memory(begin, (e - b));
    }

    static bool dpmi_unlock_region(void *begin, void *end)
    {
        char *b, *e;

        b = (char*)begin;
        e = (char*)end;

        return dpmi_unlock_memory(begin, (e - b));
    }
};

template<class T>
void midi_dpmi_lock_class_code()
{
    void (T::* lock_begin)() = &T::dpmi_lock_begin;
    void (T::* lock_end)() = &T::dpmi_lock_end;
    dpmi_allocator_impl::dpmi_lock_region((void*&)lock_begin, (void*&)lock_end);
}

template<class T>
void midi_dpmi_unlock_class_code()
{
    void (T::* lock_begin)() = &T::dpmi_lock_begin;
    void (T::* lock_end)() = &T::dpmi_lock_end;
    dpmi_allocator_impl::dpmi_unlock_region((void*&)lock_begin, (void*&)lock_end);
}


template <typename T>
struct dpmi_allocator
{
    typedef T value_type;
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;

#if __cplusplus <= 201703L
    // These were removed for C++20.
    typedef T*       pointer;
    typedef const T* const_pointer;
    typedef T&       reference;
    typedef const T& const_reference;

    template<typename _Tp1>
    struct rebind
    {
        typedef dpmi_allocator<_Tp1> other;
    };
#endif


    dpmi_allocator() {}

    template <class U>
    dpmi_allocator(const dpmi_allocator<U>&) {}

    T* allocate(std::size_t n)
    {
        const size_t len = n * sizeof(T);
        void *ptr = std::malloc(len);

        if(ptr)
        {
            if(!dpmi_allocator_impl::dpmi_lock_memory(ptr, len))
            {
                std::free(ptr);
                throw std::bad_alloc();
            }

#ifdef DPMI_DEBUG
            printf("%s: DPMI locked: 0x%08X (%d byte(s))\n", __PRETTY_FUNCTION__, (unsigned int)(uintptr_t)ptr, (int)len);
#endif

            return static_cast<T*>(ptr);
        }

        throw std::bad_alloc();
        return NULL;
    }

    void deallocate(T* ptr, std::size_t n)
    {
        const size_t len = n * sizeof(T);
        dpmi_allocator_impl::dpmi_unlock_memory(ptr, len);
#ifdef DPMI_DEBUG
        printf("%s, DPMI unlocked: 0x%08X (%d byte(s))\n", __PRETTY_FUNCTION__, (unsigned int)(uintptr_t)ptr, (int)len);
#endif
        std::free(ptr);
    }

#if __cplusplus <= 201703L
#   if __cplusplus <= 201103L
    void construct(pointer p, const_reference val)
    {
        ::new((void*)p) T(val);
    }

    void destroy(pointer p)
    {
        p->~T();
    }

    size_type max_size() const throw()
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }
#   else
    template< class U, class... Args >
    void construct( U* p, Args&&... args )
    {
        ::new((void*)p) U(std::forward<Args>(args)...);
    }

    template< class U >
    void destroy(U* p)
    {
        p->~U();
    }

    size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }
#   endif
#endif
};

template <typename T, typename U>
inline bool operator == (const dpmi_allocator<T>&, const dpmi_allocator<U>&)
{
    return true;
}

template <typename T, typename U>
inline bool operator != (const dpmi_allocator<T>& a, const dpmi_allocator<U>& b)
{
    return !(a == b);
}

#else
#   define dpmi_allocator std::allocator /* Use standard allocator anywhere also */
#endif

#endif /* BW_MIDISEQ_DPMI_ALLOC_HPP */
