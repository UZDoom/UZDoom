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

#ifndef MY_PTR_HPP_THING
#define MY_PTR_HPP_THING

#include <stddef.h>
#include <stdlib.h>

/*
  Generic deleters for smart pointers
 */
template <class T>
struct My_DefaultDelete
{
    void operator()(T *x) { delete x; }
};

template <class T>
struct My_DefaultArrayDelete
{
    void operator()(T *x) { delete[] x; }
};

struct My_CDelete
{
    void operator()(void *x) { free(x); }
};

/*
    Safe unique pointer for C++98, non-copyable but swappable.
*/
template< class T, class Deleter = My_DefaultDelete<T> >
class My_UPtr
{
    T *m_p;
public:
    explicit My_UPtr(T *p = NULL)
        : m_p(p) {}
    ~My_UPtr()
    {
        reset();
    }

    void reset(T *p = NULL)
    {
        if(p != m_p)
        {
            if(m_p)
            {
                Deleter del;
                del(m_p);
            }
            m_p = p;
        }
    }

    T *get() const
    {
        return m_p;
    }

    T *release()
    {
        T *ret = m_p;
        m_p = NULL;
        return ret;
    }

    T &operator*() const
    {
        return *m_p;
    }

    T *operator->() const
    {
        return m_p;
    }

    T &operator[](size_t index) const
    {
        return m_p[index];
    }

private:
    My_UPtr(const My_UPtr &);
    My_UPtr &operator=(const My_UPtr &);
};

#endif // MY_PTR_HPP_THING
