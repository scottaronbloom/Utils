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

#ifndef __HASHUTILS_H
#define __HASHUTILS_H

#include "SABUtilsExport.h"

#include <QHash>
#include <initializer_list>
#include <functional>

namespace NSABUtils
{
    template< typename T >
    inline std::size_t HashCombine( std::size_t seed, const T &item )
    {
        std::hash< T > hasher;
        auto currValue = hasher( item );

        std::size_t retVal = seed;
        retVal ^= currValue + 0x9e489236 + ( retVal << 6 ) + ( retVal >> 2 );

        return retVal;
    }

    template< typename T >
    inline auto HashCombine( size_t seed, const std::initializer_list< T > &values, bool needsHash ) -> typename std::enable_if< std::is_same< std::size_t, T >::value, size_t >::type
    {
        std::hash< T > hasher;
        std::size_t retVal = seed;
        for ( auto ii = values.begin(); ii != values.end(); ++ii )
        {
            auto currValue = needsHash ? hasher( *ii ) : *ii;
            retVal ^= currValue + 0x9e489236 + ( retVal << 6 ) + ( retVal >> 2 );
        }
        return retVal;
    }

    template< typename T >
    inline auto HashCombine( const std::initializer_list< T > &values, bool needsHash )   // only use when needsHash == false
        -> typename std::enable_if< std::is_same< std::size_t, T >::value, size_t >::type
    {
        return HashCombine( 0, values, needsHash );
    }

    template< typename T >
    inline std::size_t HashCombine( size_t seed, const std::initializer_list< T > &values )
    {
        std::hash< T > hasher;
        std::size_t retVal = seed;
        for ( auto ii = values.begin(); ii != values.end(); ++ii )
        {
            auto currValue = hasher( *ii );
            retVal ^= currValue + 0x9e489236 + ( retVal << 6 ) + ( retVal >> 2 );
        }
        return retVal;
    }

    template< typename T >
    inline std::size_t HashCombine( const std::initializer_list< T > &values )
    {
        return HashCombine( 0, values );
    }

    template< typename T >
    inline std::size_t HashCombine( T h1, T h2, bool needsHash )
    {
        return HashCombine( 0, { h1, h2 }, needsHash );
    }

    template< typename T1, typename T2 >
    inline std::size_t HashCombine( const std::pair< T1, T2 > &key )
    {
        return NSABUtils::HashCombine( NSABUtils::HashCombine( 0, { key.first } ), { key.second } );
    }

    namespace
    {
        template< class Tuple, size_t Index = std::tuple_size< Tuple >::value - 1 >
        struct HashValueImpl
        {
            static void apply( size_t &seed, Tuple const &tuple )
            {
                HashValueImpl< Tuple, Index - 1 >::apply( seed, tuple );
                NSABUtils::HashCombine( seed, { std::get< Index >( tuple ) } );
            }
        };

        template< class Tuple >
        struct HashValueImpl< Tuple, 0 >
        {
            static void apply( size_t &seed, Tuple const &tuple ) { NSABUtils::HashCombine( seed, { std::get< 0 >( tuple ) } ); }
        };
    }

    template< typename... TT >
    inline std::size_t HashCombine( const std::tuple< TT... > &tt )
    {
        size_t seed = 0;
        HashValueImpl< std::tuple< TT... > >::apply( seed, tt );
        return seed;
    };

}
#endif
