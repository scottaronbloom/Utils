#ifndef __METAUTILS_H
#define __METAUTILS_H

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
#include <iterator>
#include <tuple>

namespace NSABUtils
{
    // used to determine if container T has an const_iterator
    // usage std::enable_if< has_const_iterator< T >::value, void >
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

    template< class T >
    constexpr bool has_const_iterator_v = has_const_iterator< T >::value;

    // used to determine if type T has a begin and end that can be used for iteration
    // usage std::enable_if< has_begin_end< T >::value, void >
    template< typename T >
    struct has_begin_end
    {
        template< typename C >
        static char ( &f( typename std::enable_if< std::is_same_v< decltype( static_cast< typename C::const_iterator ( C::* )() const >( &C::begin ) ), typename C::const_iterator ( C::* )() const >, void >::type * ) )[ 1 ];

        template< typename C >
        static char ( &f( ... ) )[ 2 ];

        template< typename C >
        static char ( &g( typename std::enable_if< std::is_same_v< decltype( static_cast< typename C::const_iterator ( C::* )() const >( &C::end ) ), typename C::const_iterator ( C::* )() const >, void >::type * ) )[ 1 ];

        template< typename C >
        static char ( &g( ... ) )[ 2 ];

        static bool const beg_value = sizeof( f< T >( 0 ) ) == 1;
        static bool const end_value = sizeof( g< T >( 0 ) ) == 1;
        static bool const value = beg_value && end_value;
    };

    template< class T >
    constexpr bool has_begin_end_v = has_begin_end< T >::value;

    // used to determin if type T is a container
    // a container as defined as having a begin and end as well as a const_iterator
    // potential usage std::enable_if< is_container< T >, void >
    template< typename T >
    struct is_container : std::integral_constant< bool, has_const_iterator_v< T > && has_begin_end_v< T > >
    {
    };

    template< class T >
    constexpr bool is_container_v = is_container< T >::value;

    // used to determine if type T is an iterator
    // potential usage std::enable_if< is_iterator< T >::value, void >
    // fallback value is false
    template< class T, class = void >
    struct is_iterator : std::false_type
    {
    };

    // used to determine if type T is an iterator
    // potential usage std::enable_if< is_iterator< T >::value, void >
    // when type T has the listed iterator traits, then this template structure matches (as opposed to the default one) which means the ::value data member will be true
    template< class T >
    struct is_iterator<
        T, std::void_t<
               typename std::iterator_traits< T >::difference_type,   //
               typename std::iterator_traits< T >::pointer,   //
               typename std::iterator_traits< T >::reference,   //
               typename std::iterator_traits< T >::value_type,   //
               typename std::iterator_traits< T >::iterator_category   //
               > > : std::true_type
    {
    };

    // helper so you dont need ::value
    template< class T >
    constexpr bool is_iterator_v = is_iterator< T >::value;

    template< typename >
    struct is_tuple : std::false_type
    {
    };

    template< typename... T >
    struct is_tuple< std::tuple< T... > > : std::true_type
    {
    };

    template< typename T >
    constexpr bool is_tuple_v = is_tuple< T >::value;

}

#endif
