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

namespace NSABUtils
{
    template< typename T1, typename T2 >
    inline typename std::enable_if< ( std::is_integral< T1 >::value && std::is_integral< T2 >::value ), uint64_t >::type cantorHash( const T1 & lhs, const T2 & rhs )
    {
        return ( lhs + rhs + 1 ) * ( lhs + rhs ) / 2 + rhs;
    }

    template< typename T1, typename T2 >
    inline typename std::enable_if< ( std::is_integral< T1 >::value && std::is_integral< T2 >::value ), uint64_t >::type cantorHash( const std::pair< T1, T2 > &values )
    {
        return cantorHash( values.first, values.second );
    }

    template< typename T >
    inline typename std::enable_if< ( std::is_integral< T >::value ), uint64_t >::type cantorHash( const std::initializer_list< T > &value )
    {
        auto ii = value.begin();
        auto end = value.end();
        // can not happen since an empty initializer list returns false for is integral
        //if ( ii == end ) 
        //    return 0;

        uint64_t retVal = *ii;
        ++ii;
        if ( ii == end )
            return cantorHash( 0, retVal );

        for ( ; ii != end; ++ii )
            retVal = cantorHash( retVal, *ii );

        return retVal;
    }
}
#endif   
