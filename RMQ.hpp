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
 * @file RMQ.hpp
 * @brief Range minimum query.
 */

#ifndef RMQ_HPP
#define RMQ_HPP

#include <boost/concept_check.hpp>
#include <boost/concept/requires.hpp>
#include <boost/concept/assert.hpp>

#include <iterator>
#include <algorithm>
#include <cassert>
#include <cmath>

#include "algorithms.hpp"


namespace general
{
    /**
     * @defgroup RMQ_algorithms Range minimum query
     * @ingroup non_mutating_algorithms
     */
    
    
    /**
     * @brief           Create a Sparse Table of indexes for RMQ from the input container A.
     * @ingroup         RMQ_algorithms
     * @tparam C0       Read-only random-access container.
     * @tparam C1       Mutable random-access 2d array.
     * @param A         Input array.
     * @param M         Sparse Table of A.
     */
    template <typename C0, typename C1>
    void index_preprocess_sparse_table(C0 const &A, C1 &M)
    {
        BOOST_CONCEPT_ASSERT((boost::RandomAccessContainer<C0>));
        // require: C1 is a 2d array.
        
        if(A.size() > 2)
        {
            typedef typename C0::size_type size_type;
            char unsigned j = 1;
            
            for(size_type i = 0; i < A.size() - 1; i++)
                M[j][i] = A[i] <= A[i + 1] ? i : i + 1;
            
            auto const n = A.size();
            auto const lowerlogn = lower_log2(n);
            auto prev_block_length = 2u;
            
            for(j = 2; j <= lowerlogn; j++)
            {
                auto const block_length = pow2(j);
                auto const last_pos = n - block_length + 1u;
                
                for(std::size_t i = 0; i != last_pos; i++)
                {
                    auto const &M1 = M[j - 1u][i], 
                                &M2 = M[j - 1u][i + prev_block_length];
                    M[j][i] = A[M2] < A[M1] ? M2 : M1;
                }
                prev_block_length = block_length;
            }
        }
    }

    
    /**
     * @brief           Create a Sparse Table of iterators for RMQ from the range [first, last).
     * @ingroup         RMQ_algorithms
     * @tparam I        Forward Iterator.
     * @tparam C        Mutable random-access container of I.
     * @param first     Beginning of range.
     * @param last      End of range.
     * 
     * Time complexity: Θ(n lg n)
     */
    template <typename I, typename C>
    void preprocess_sparse_table(I first, I last, C &M)
    {
        BOOST_CONCEPT_ASSERT((boost::ForwardIterator<I>));
        BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessContainer<C>));
        
        if(first != last && std::next(first) != last && std::next(std::next(first)) != last)
        {
            for(auto second = std::next(first); second != last; ++first, ++second)
                M.push_back(*first <= *second ? first : second);
            
            auto const n = M.size() + 1u;
            auto const lowerlogn = general::lower_log2(n);
            auto block_length_2 = 2ul;
            
            for(char unsigned j = 2; j <= lowerlogn; j++) // Θ(lg n)
            {
                auto const block_length = 2 * block_length_2;;
                auto const last_pos = n - block_length + 1u;
                auto Mi = M.size() - (n - block_length_2 + 1u); // Use index because of iterator invalidation.

                for(std::size_t i = 0; i != last_pos; i++) // Θ(n)
                {
                    auto const &M1 = M[Mi], &M2 = M[Mi + block_length_2];
                    M.push_back(*M2 < *M1 ? M2 : M1);
                    ++Mi;
                }
                block_length_2 = block_length;
            }
        }
    }
    
    
    template <typename N0, typename N1, typename N2>
    inline
    auto translate_sparse_table(N0 i, N1 j, N2 n) -> decltype(j * n + i)
    {
        BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<N0>));
        BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<N1>));
        BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<N2>));

        return j == N1(1) ? i : ((j - N1(1)) * n) - (general::pow2(j) - j - N1(1)) + i;
    }
    
    
    /** @brief Perform range minimum query on a Sparse Table.
     *  @ingroup RMQ_algorithms
     *  @tparam N0 Index type
     *  @tparam N1 Container size type (usually std::size_t)
     *  @tparam C Container type
     *  @param i Lower bound index of range query
     *  @param j Upper bound index of range query
     *  @param n Size of original input problem
     *  @param sparse_table The Sparse Table to be queried
     * 
     *  Time complexity: Θ(1)
     */
    template <typename N0, typename N1, typename C>
    typename C::value_type query_sparse_table(N0 i, N0 j, N1 n, C const &sparse_table)
    {
        BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<N0>));
        BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<N1>));
        BOOST_CONCEPT_ASSERT((boost::RandomAccessContainer<C>));
        
        // requires: [i, j] is a valid range.
        assert(i <= j);
        assert(j < n);
        
        if (i == j)
            return sparse_table[i];

        auto const k = lower_log2(j - i + 1);
        auto const  x = translate_sparse_table(i, k, n), 
                    y = translate_sparse_table(j - pow2(k) + N0(1), k, n);
        auto const &Mx = sparse_table[x], &My = sparse_table[y];
        return *My < *Mx ? My : Mx;
    }
    

    template <typename N, typename C0, typename C1>
    typename C1::element index_query_sparse_table(N i, N j, C0 const &A, C1 const &M)
    {
        BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<N>));
        BOOST_CONCEPT_ASSERT((boost::RandomAccessContainer<C0>));
        
        // requires: [i, j] is a valid range.
        assert(i <= j);
        
        if (i == j)
            return i;
        
        auto const r = j - i + 1;
        auto const k = lower_log2(r);
        auto const x = M[k][i], y = M[k][i + (r - 2 * k)];
        auto const &Mx = A[x], &My = A[y];
        return My < Mx ? y : x;
    }
    
    // The beginnings of a convenience class to wrap it all up in, incomplete.
    
    template <typename I>
    class sparse_table
    {
        BOOST_CONCEPT_ASSERT((boost::ForwardIterator<I>));

        std::vector<I> M;
        
    public:
        sparse_table(I first, I last)
        {
            preprocess_sparse_table(first, last, M);
        }
        
        
        void preprocess(I first, I last)
        {
            M.clear();
            preprocess_sparse_table(first, last, M);
        }
        
        
        template <typename N>
        // requires:    UnsignedIntegral(N)
        I query(N i, N j)
        {
            return query_sparse_table(i, j, M);
        }
    };
    
    
    template <typename I>
    sparse_table<I> make_sparse_table(I first, I last)
    {
        return sparse_table<I>(first, last);
    }
}

#endif
