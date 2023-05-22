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

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace NSABUtils
{
    template< typename T1, typename T2 >
    inline int64_t cantorHash( const T1 &lhs, const T2 &rhs )
    {
        static_assert( std::is_integral< T1 >::value );
        static_assert( std::is_integral< T2 >::value );

        return ( lhs + rhs + 1 ) * ( lhs + rhs ) / 2 + rhs;
    }

    template< typename T1, typename T2 >
    inline int64_t cantorHash( const std::pair< T1, T2 > &values )
    {
        static_assert( std::is_integral< T1 >::value );
        static_assert( std::is_integral< T2 >::value );
        return cantorHash( values.first, values.second );
    }

    template< typename T >
    struct has_const_iterator
    {
    private:
        typedef char yes;
        typedef struct
        {
            char array[ 2 ];
        } no;

        template< typename C >
        static yes test( typename C::const_iterator * );
        template< typename C >
        static no test( ... );

    public:
        static const bool value = sizeof( test< T >( 0 ) ) == sizeof( yes );
        typedef T type;
    };

    template< typename T >
    struct has_begin_end
    {
        template< typename C >
        static char ( &f( typename std::enable_if< std::is_same< decltype( static_cast< typename C::const_iterator ( C::* )() const >( &C::begin ) ), typename C::const_iterator ( C::* )() const >::value, void >::type * ) )[ 1 ];

        template< typename C >
        static char ( &f( ... ) )[ 2 ];

        template< typename C >
        static char ( &g( typename std::enable_if< std::is_same< decltype( static_cast< typename C::const_iterator ( C::* )() const >( &C::end ) ), typename C::const_iterator ( C::* )() const >::value, void >::type * ) )[ 1 ];

        template< typename C >
        static char ( &g( ... ) )[ 2 ];

        static bool const beg_value = sizeof( f< T >( 0 ) ) == 1;
        static bool const end_value = sizeof( g< T >( 0 ) ) == 1;
    };

    template< typename T >
    struct is_container : std::integral_constant< bool, has_const_iterator< T >::value && has_begin_end< T >::beg_value && has_begin_end< T >::end_value >
    {
    };

    template< typename T1 >
    inline std::enable_if_t< !is_container< T1 >::value, int64_t > cantorHash( const T1 &rhs )
    {
        static_assert( std::is_integral< T1 >::value );

        return cantorHash( 0, rhs );
    }

    template< typename T, typename... Targs >
    inline std::enable_if_t< ( sizeof...( Targs ) == 1 ), int64_t > cantorHash( T lhs, T rhs, Targs... Fargs )
    {
        return cantorHash( lhs, cantorHash( rhs, Fargs... ) );
    }

    template< typename T, typename... Targs >
    inline std::enable_if_t< ( sizeof...( Targs ) >= 2 ), int64_t > cantorHash( T lhs, Targs... Fargs )
    {
        return cantorHash( lhs, cantorHash( Fargs... ) );
    }

    // default case
    template< class T, class = void >
    struct is_iterator : std::false_type
    {
    };

    // specialization
    template< class T >
    struct is_iterator<
        T, std::void_t<
               typename std::iterator_traits< T >::difference_type, typename std::iterator_traits< T >::pointer, typename std::iterator_traits< T >::reference, typename std::iterator_traits< T >::value_type,
               typename std::iterator_traits< T >::iterator_category > > : std::true_type
    {
    };

    template< class T >
    constexpr bool is_iterator_v = is_iterator< T >::value;

    template< class Iterator >
    inline typename std::enable_if_t< is_iterator_v< Iterator >, int64_t > cantorHash( Iterator begin, const Iterator &end )
    {
        static_assert( std::is_integral< typename std::iterator_traits< Iterator >::value_type >::value );
        if ( begin == end )
            return 0;

        auto len = std::distance( begin, end );
        if ( len == 1 )
            return cantorHash( *begin, 0 );

        auto rbegin = end;
        int64_t retVal = -1;
        do
        {
            --rbegin;
            if ( retVal == -1 )
                retVal = *rbegin;
            else
                retVal = cantorHash( *rbegin, retVal );
        }
        while ( rbegin != begin );

        return retVal;
    }

    template< typename T >
    inline typename std::enable_if_t< ( std::is_integral< T >::value ), int64_t > cantorHash( const std::initializer_list< T > &value )
    {
        return cantorHash( value.begin(), value.end() );
    }

    template< typename T1 >
    inline std::enable_if_t< is_container< T1 >::value, int64_t > cantorHash( const T1 &value )
    {
        return cantorHash( value.begin(), value.end() );
    }
}
#endif
