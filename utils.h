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

#ifndef __UTILS_H
#define __UTILS_H

#include <cinttypes>
#include <cstdarg>
#include <string>
#include <chrono>
#include <cmath>
#include <list>
#include <utility>
#include <vector>
#include <bitset>
#include <functional>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <QString>
class QDateTime;
#if __cplusplus > 201703L
#include <optional>
#endif

template< typename T >
std::ostream& operator<<( std::ostream& oss, const std::vector< T >& values )
{
    oss << "{ ";
    bool first = true;

    for ( auto&& ii : values )
    {
        if ( !first )
            oss << ", ";
        first = false;
        oss << ii;
    }
    oss << " }";
    return oss;
}

template< typename T >
std::ostream& operator<<( std::ostream& oss, const std::vector< std::vector< T > >& values )
{
    oss << "{ ";
    bool first = true;

    for ( auto&& ii : values )
    {
        if ( !first )
            oss << ", ";
        first = false;
        oss << ii;
    }
    oss << " }";
    return oss;
}

template< typename T >
std::ostream& operator<<( std::ostream& oss, const std::list< std::vector< T > >& values )
{
    oss << "{ ";
    bool first = true;

    for ( auto&& ii : values )
    {
        if ( !first )
            oss << ", ";
        first = false;
        oss << ii;
    }
    oss << " }";
    return oss;
}

inline std::ostream & sabsabDebugStreamInternal(){ return std::cout; }
#  undef sabDebugStream
#if defined(SAB_DEBUG_TRACE)
#  define sabDebugStream sabsabDebugStreamInternal
#else
#  define sabDebugStream while (false) sabsabDebugStreamInternal
#endif

namespace NUtils
{
template< typename T >
T indexInList( std::size_t index, const std::list< T > & list )
{
    auto pos = list.begin();
    while ( index && (pos != list.end()) )
    {
        pos++;
        index--;
    }
    if ( pos != list.end() )
        return *pos;
    return T();
}

template< typename T1, typename T2 >
using TLargestType = typename std::conditional< ( sizeof( T1 ) >= sizeof( T2 ) ), T1, T2 >::type;

template < typename T1, typename T2 >
// FLOATING POINT just use std::pow 
// the return type is the larger of the two T1 and T2 types
auto power( T1 x, T2 y )
-> typename std::enable_if< std::is_floating_point<T1>::value || std::is_floating_point<T2>::value, TLargestType< T1, T2 > >::type
{
    return std::pow( x, y );
}

template < typename T1, typename T2>
// Integral types
// the return type is the larger of the two T1 and T2 types
auto power( T1 x, T2 y )
-> typename std::enable_if< std::is_integral<T1>::value && std::is_integral<T2>::value, TLargestType< T1, T2 > >::type
{
    if ( y == 0 )
        return 1;
    if ( y == 1 )
        return x;
    if ( x == 0 )
        return 0;
    if ( x == 1 )
        return 1;

    TLargestType< T1, T2 > retVal = 1;
    for ( T2 ii = 0; ii < y; ++ii )
        retVal *= x;
    return retVal;
}

int fromChar( char ch, int base, bool& aOK );
char toChar( int value );

void toDigits( int64_t val, int base, std::pair< int8_t*, uint32_t > & retVal, size_t& numDigits, bool * aOK = nullptr );
std::string toString( int64_t val, int base );
int64_t fromString( const std::string& str, int base );
std::string getTimeString( const std::pair< std::chrono::system_clock::time_point, std::chrono::system_clock::time_point >& startEndTime, bool reportTotalSeconds, bool highPrecision );
std::string getTimeString( const std::chrono::system_clock::duration& duration, bool reportTotalSeconds=false, bool highPrecision=true );
double getSeconds( const std::chrono::system_clock::duration& duration, bool highPrecision );

QString getTimeString( const QDateTime& startTime, const QDateTime& endTime, bool reportTotalSeconds = false, bool highPrecision = true );
QString secsToString( qint64 seconds );

std::list< int64_t > computeFactors( int64_t num, bool properFactors=false );
std::list< int64_t > computePrimeFactors( int64_t num );

bool isNarcissistic( int64_t val, int base, bool& aOK );
// return Value, the list of factors since the factors are often needed
std::pair< int64_t, std::list< int64_t > > getSumOfFactors( int64_t curr, bool properFactors );
std::pair< bool, std::list< int64_t > > isSemiPerfect( int64_t num );
std::pair< bool, std::list< int64_t > > isPerfect( int64_t num );
std::pair< bool, std::list< int64_t > > isAbundant( int64_t num );

bool isSemiPerfect( const std::vector< int64_t >& numbers, size_t n, int64_t num );

template< typename T >
std::string getNumberListString( const T & numbers, int base )
{
    std::ostringstream oss;
    bool first = true;
    size_t ii = 0;
    std::string str;
    for ( auto&& currVal : numbers )
    {
        if ( ii && ( ii % 5 == 0 ) )
        {
            oss << "\n";
            first = true;
        }
        if ( !first )
            oss << ", ";
        else
            oss << "    ";
        first = false;

        oss << NUtils::toString( currVal, base );
        if ( base != 10 )
            oss << "(=" << currVal << ")";
        ii++;
    }
    return oss.str();
}

#if __cplusplus > 201703L
template <size_t N>
constexpr std::optional<size_t> findLargestIndexInBitSet( const std::bitset<N> & set )
{
    if ( set.size() == 0 )
        return std::nullopt;

    for ( size_t ii = set.size() - 1; ii >= 0; --ii )
    {
        if ( set.test( ii ) )
        {
            return std::optional<size_t>( ii );
        }
        if ( ii == 0 )
            break;
    }
    return std::nullopt;
}
template <size_t N>
constexpr std::optional<size_t> findSmallestIndexInBitSet( const std::bitset<N>& set )
{
    if ( set.size() == 0 )
        return std::nullopt;

    for ( size_t ii = 0; ii < set.size(); ++ii )
    {
        if ( set.test( ii ) )
        {
            return std::optional<size_t>( ii );
        }
    }
    return std::nullopt;
}
#endif
template< typename T >
void combinationUtil( const std::vector< T >& arr, std::vector< T >& data,
                      size_t start, size_t end,
                      size_t index, size_t r, const std::function< void( const std::vector< T > & sub ) >& func )
{
    // Current combination is ready 
    // to be printed, print it  
    if ( index == r )
    {
        if ( func )
        {
            func( data );
        }
        return;
    }

    // replace index with all possible  
    // elements. The condition "end-i+1 >= r-index" 
    // makes sure that including one element  
    // at index will make a combination with  
    // remaining elements at remaining positions  
    for ( size_t i = start; ( i <= end ) && ( end - i + 1 ) >= ( r - index ); i++ )
    {
        data[ index ] = arr[ i ];
        combinationUtil( arr, data, i + 1, end, index + 1, r, func );
    }
}

#if __cplusplus > 201703L
template< typename T >
void allCombinations( const std::vector< T >& arr, size_t r, const std::function< void( const std::vector< T > & sub ) >& func )
{
    // A temporary array to store 
    // all combination one by one  
    std::vector< T > data( r );

    // Print all combination using 
    // temprary array 'data[]'  
    combinationUtil( arr, data, 0, arr.size() - 1, 0, r, func );
}

template< typename T >
std::vector< std::vector< T > > allCombinations( const std::vector< T >& arr, size_t r, const std::pair< bool, size_t > & report = std::make_pair( false, 1 ) )
{
    std::vector< std::vector< T > > combinations;
    NUtils::allCombinations< T >( arr, r,
                                    [ &combinations, &report = std::as_const( report ) ]( const std::vector< T >& sub )
                                    {
                                        combinations.push_back( sub );
                                        if ( report.first && ( ( combinations.size() % report.second ) == 0 ) )
                                            sabDebugStream() << "Generating combination: " << combinations.size() << "\n";
                                    } );
    return combinations;
}
#endif

long double factorial( int64_t num );
uint64_t numCombinations( int64_t numPossible, int64_t numSelections );

template< typename T >
std::vector< std::vector< T > > addVectorElementToSets( const std::vector< std::vector< T > >& currentSets, const std::list< T >& rhs, const std::function< bool( const std::vector< T >& curr, const T & obj ) > & addToResult = std::function< bool( const std::vector< T >& curr, const T& obj ) >() )
{
    std::vector< std::vector< T > > retVal;
    for( auto && ii : currentSets )
    {
        for( auto && jj : rhs )
        {
            if ( addToResult && !addToResult( ii, jj ) )
                continue;
            auto temp = ii;
            temp.push_back( jj );
            retVal.push_back( temp );
        }
    }
    return retVal;
}

template< typename T >
std::vector< std::vector< T > > cartiseanProduct( const std::vector< std::list< T > > & arr, const std::function< bool( const std::vector< T >& curr, const T& obj ) >& addToResult = std::function< bool( const std::vector< T >& curr, const T& obj ) >() )
{
    if ( arr.empty() )
        return {};


    std::vector< std::vector< T > > retVal;
    for ( auto&& ii : arr[0] )
    {
        retVal.push_back( { ii } );
    }

    for ( size_t ii = 1; ii < arr.size(); ++ii )
    {
        retVal = addVectorElementToSets( retVal, arr[ ii ], addToResult );
    }
    return retVal;
}

template< typename T >
std::pair< typename std::list< T >::const_iterator, typename std::list< T >::const_iterator > mid( const std::list< T > & inList, int xFirst, int xCount = -1 )
{
    using listType = std::list< T >;
    using iter = typename listType::const_iterator;

    iter pos1;
    if ( xFirst >= inList.size() )
        pos1 = inList.cend();
    else
        pos1 = std::next( inList.cbegin(), xFirst );

    iter pos2;
    if ( ( xCount == -1 ) || ( ( xFirst + xCount ) >= inList.size() ) )
        pos2 = inList.cend();
    else 
        pos2 = std::next( inList.cbegin(), xFirst + xCount );

    return std::make_pair( pos1, pos2 );
}

template< typename T >
std::list< T > operator +( const std::list< T > & lhs, const std::pair< typename std::list< T >::const_iterator, typename std::list< T >::const_iterator > & rhs )
{
    auto lRetVal = lhs;
    lRetVal.insert( lRetVal.end(), rhs.first, rhs.second );
    return lRetVal;
}

template< typename T >
std::list< T > replaceInList( const std::list< T > & inList, int xFirst, int xCount, const std::list< T > & values, int xNum = -1 )
{
    auto prefix = NUtils::mid( inList, 0, xFirst );
    auto mid = NUtils::mid( values, 0, xNum );
    auto suffix = NUtils::mid( inList, xFirst + xCount );

    auto lRetVal = std::list< T >( { prefix.first, prefix.second } ) + mid + suffix; //( { prefix.first, prefix.second } );
    return lRetVal;
}

}
#endif
