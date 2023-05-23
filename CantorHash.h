#ifndef __CANTORHASH_H
#define __CANTORHASH_H

// The MIT License( MIT )
//
// Copyright( c ) 2020-2021 Scott Aron Bloom
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "MetaUtils.h"

#include <cstdint>
#include <tuple>
#include <type_traits>
#include <iterator>
#include <tuple>

namespace NSABUtils
{
    // base implementation
    // usage value = cantorHash( lhs, rhs );
    template< typename T1, typename T2 >
    inline std::enable_if_t< std::is_integral_v< T1 > && std::is_integral_v< T2 >, size_t > cantorHash( const T1 &lhs, const T2 &rhs )
    {
        static_assert( std::is_integral_v< T1 > );
        static_assert( std::is_integral_v< T2 > );

        return ( ( ( lhs + rhs + 1 ) * ( lhs + rhs ) ) / 2 ) + rhs;
    }

    template< typename T1, typename T2 >
    inline std::enable_if_t< std::is_integral_v< T1 > && std::is_integral_v< T2 >, size_t > cantorHash( const std::pair< T1, T2 > &values )
    {
        static_assert( std::is_integral_v< T1 > );
        static_assert( std::is_integral_v< T2 > );
        return cantorHash( values.first, values.second );
    }

    // used when only 1 value
    // usage value = cantorHash( value );
    // equivilent to cantorHash( value, 0 );
    template< typename T1 >
    inline std::enable_if_t< !is_container_v< T1 > && !is_tuple_v< T1 >, size_t > cantorHash( const T1 &value )
    {
        static_assert( std::is_integral_v< T1 > );

        return cantorHash( value, 0 );
    }

    // 2 values, 1 fixed 1 variadic of length 1
    // value = cantorHash( lhs, rhs );
    template< typename T, typename... Targs >
    inline std::enable_if_t< ( sizeof...( Targs ) == 1 ), size_t > cantorHash( T lhs, T rhs, Targs... Fargs )
    {
        return cantorHash( lhs, cantorHash( rhs, Fargs... ) );
    }

    // 3 or more values, 1 fixed, 2 ore more variadic
    // value = cantorHash( lhs, rhs );
    template< typename T, typename... Targs >
    inline std::enable_if_t< ( sizeof...( Targs ) >= 2 ), size_t > cantorHash( T lhs, Targs... Fargs )
    {
        return cantorHash( lhs, cantorHash( Fargs... ) );
    }

    // doesnt check for bi-drectional
    // however if its a 1 way only iterator, then --rbegin will error out
    // computes cantorHash( end-3, cantorHash( end-2, end-1) ) recursively until end-3 == begin
    // usage
    // std::container< T > values = {1,2,3};
    // value = cantorHash( values.begin(), values.end() );
    template< class Iterator >
    inline typename std::enable_if_t< is_iterator_v< Iterator >, size_t > cantorHash( Iterator begin, const Iterator &end )
    {
        static_assert( std::is_integral_v< typename std::iterator_traits< Iterator >::value_type > );
        if ( begin == end )
            return 0;

        if ( std::distance( begin, end ) == 1 )
            return cantorHash( *begin, 0 );

        auto rbegin = end;
        size_t retVal = -1;
        do
        {
            --rbegin;   // if you are getting asyntax error here, your iterator pair is one direction only
            if ( retVal == -1 )
                retVal = *rbegin;
            else
                retVal = cantorHash( *rbegin, retVal );
        }
        while ( rbegin != begin );

        return retVal;
    }

    // for use with C++ initalizer lists
    // computes cantorHash( end-3, cantorHash( end-2, end-1) ) recursively until end-3 == begin
    // usage
    // value = cantorHash( {1,2,3} );
    template< typename T >
    inline typename std::enable_if_t< (std::is_integral_v< T >), size_t > cantorHash( const std::initializer_list< T > &value )
    {
        return cantorHash( value.begin(), value.end() );
    }

    // computes cantorHash( end-3, cantorHash( end-2, end-1) ) recursively until end-3 == begin
    // usage
    // std::container< T > values = {1,2,3};
    // value = cantorHash( values );
    template< typename T1 >
    inline std::enable_if_t< is_container_v< T1 >, size_t > cantorHash( const T1 &value )
    {
        return cantorHash( value.begin(), value.end() );
    }

    // expands to
    // cantorHash( std::get< 0 >( tuple ), std::get< 1 >( tuple ), std::get< 2 >( tuple ), .... std::get< sz - 1 >( tuple ) )
    // typically used with cantorHash( std::tuple( ... ) );
    template< typename Tp, size_t... II >
    inline std::enable_if_t< is_tuple_v< Tp >, size_t > cantorHash( const Tp &tuple, const std::index_sequence< II... > & /*indexSeq*/ )
    {
        return cantorHash( std::get< II >( tuple )... );
    }

    // usage
    // value = cantorHash( std::make_tuple( 1,2,3 ) );
    template< typename Tp >
    inline std::enable_if_t< is_tuple_v< Tp >, size_t > cantorHash( const Tp &tuple )
    {
        constexpr auto sz = std::tuple_size< Tp >::value;
        auto revSequence = std::make_index_sequence< sz >{};
        return cantorHash( tuple, revSequence );
    }
}
#endif
