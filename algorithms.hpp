/*
    Copyright (C) 2014  Jeremy W. Murphy <jeremy.william.murphy@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file algorithms.hpp
 * @brief General algorithms.
 */

#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include <unordered_set>
#include <iterator>

namespace jwm
{
    template <typename I, typename N, typename O>
    // requires: InputIterator(I), I::DifferenceType(N), OutputIterator(O)
    O representative_element(I first, N n, O result)
    {
        typedef typename std::iterator_traits<I>::value_type value_type;
        std::unordered_set<value_type> seen;
        for(N i = 0; i < n; ++i)
        {
            if(seen.find(*first) == std::end(seen))
            {
                *result++ = i;
                seen.insert(*first);
            }
            ++first;
        }
        return result;
    }
    
    
    template <typename I, typename O>
    // requires: InputIterator(I), OutputIterator(O)
    O representative_element(I first, I last, O result)
    {
        typedef typename std::iterator_traits<I>::value_type value_type;
        typedef typename std::iterator_traits<I>::difference_type N;
        std::unordered_set<value_type> seen;
        for(N n = 0; first != last; ++first, ++n)
        {
            if(seen.find(*first) == std::end(seen))
            {
                *result++ = n;
                seen.insert(*first);
            }
            ++first;
        }
        return result;
    }
}

#endif