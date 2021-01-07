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

#include "utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace NUtils
{
    int fromChar( char ch, int base, bool& aOK )
    {
        aOK = false;
        if ( base < 2 || base > 36 )
            return 0;

        if ( ch == '-' || ch == '_' )
        {
            aOK = true;
            return 1;
        }
        // only short cut if its '0' - '9'
        if ( ( ch >= '0' ) && ( ch <= '9' ) && ch <= ( '0' + ( base - 1 ) ) )
        {
            aOK = true;
            return ( ch - '0' );
        }

        if ( base <= 10 )
            return 0;

        ch = std::tolower( ch );
        auto maxChar = 'a' + base;

        if ( ( ch >= 'a' ) && ( ch <= maxChar ) )
        {
            aOK = true;
            return 10 + ch - 'a';
        }
        return 0;
    }

    char toChar( int value )
    {
        if ( ( value >= 0 ) && ( value < 10 ) )
            return '0' + value;
        // over 10, must use chars

        return 'a' + value - 10;
    }

    void toDigits( int64_t val, int base, std::pair< int8_t *, uint32_t > & retVal, size_t & numDigits, bool * aOK )
    {
        numDigits = 0;
        if ( aOK )
            *aOK = true;
        do
        {
            int64_t quotient = val / base;
            int8_t remainder = static_cast< int8_t >( val % base );
            retVal.first[ numDigits++ ] = remainder;
            if ( numDigits > retVal.second ) // not a string, can use the whole array
            {
                if ( aOK )
                    *aOK = false;
                return;
            }
            val = quotient;
        } while ( val != 0 );
    }

    std::string toString( int64_t val, int base )
    {
        std::string retVal;
        bool isNeg = val < 0;
        val = std::abs( val );
        do
        {
            int64_t quotient = val / base;
            int remainder = val % base;
            retVal.insert( retVal.begin(), toChar( remainder ) );
            val = quotient;
        } while ( val != 0 );
        if ( isNeg )
            retVal.insert( retVal.begin(), '-' );
        return retVal;
    }

    int64_t fromString( const std::string& str, int base )
    {
        int64_t retVal = 0;
        bool aOK = false;
        for ( size_t ii = 0; ii < str.length(); ++ii )
        {
            auto currChar = str[ ii ];
            int64_t currVal = fromChar( currChar, base, aOK );
            if ( !aOK )
            {
                std::cerr << "Invalid character: " << currChar << std::endl;
                return 0;
            }
            retVal = ( retVal * base ) + currVal;
        }
        return retVal;
    }

    std::string getTimeString( const std::pair< std::chrono::system_clock::time_point, std::chrono::system_clock::time_point >& startEndTime, bool reportTotalSeconds, bool highPrecision )
    {
        auto duration = startEndTime.second - startEndTime.first;
        return getTimeString( duration, reportTotalSeconds, highPrecision );
    }

    double getSeconds( const std::chrono::system_clock::duration& duration, bool highPrecision )
    {
        double totalSeconds = 1.0 * std::chrono::duration_cast<std::chrono::seconds>( duration ).count();
        if ( highPrecision )
            totalSeconds = std::chrono::duration_cast<std::chrono::duration< double, std::micro >>( duration ).count() / 1000000.0;
        return totalSeconds;
    }

    std::string getTimeString( const std::chrono::system_clock::duration& duration, bool reportTotalSeconds, bool highPrecision )
    {
        auto totalSeconds = getSeconds( duration, highPrecision );
        auto hrs = static_cast< int64_t >( std::chrono::duration_cast<std::chrono::hours>( duration ).count() );
        auto mins = static_cast<int64_t>( std::chrono::duration_cast<std::chrono::minutes>( duration ).count() - ( hrs * 60 ) );
        double secs = 1.0 * std::chrono::duration_cast<std::chrono::seconds>( duration ).count();
        if ( highPrecision )
            secs = ( std::chrono::duration_cast<std::chrono::duration< double, std::micro >>( duration ).count() ) / 1000000.0;
        secs -= ( ( mins * 60 ) + ( hrs * 3600 ) );

        std::ostringstream oss;
        if ( hrs > 0 )
        {
            oss << hrs << " hour";
            if ( hrs != 1 )
                oss << "s";
            oss << ", ";
        }

        if ( mins > 0 )
        {
            oss << mins << " minute";
            if ( mins != 1 )
                oss << "s";
            oss << ", ";
        }

        if ( highPrecision )
        {
            oss.setf( std::ios::fixed, std::ios::floatfield );
            oss.precision( 6 );
        }

        oss << secs << " second";

        if ( secs != 1 )
            oss << "s";
        if ( reportTotalSeconds && ( totalSeconds > 60 ) )
        {
            oss << ", (" << totalSeconds << " second";
            if ( totalSeconds != 1 )
                oss << "s";
            oss << ")";
        }
        return oss.str();
    }

    bool isNarcissisticDigits( int64_t val, int base, bool& aOK )
    {
        aOK = true;
        int8_t rawDigits[ 4096 ] = {0};
        size_t numDigits;
        auto digits = std::make_pair( rawDigits, static_cast< uint32_t >( 4096 ) );
        toDigits( val, base, digits, numDigits );

        int64_t sumOfPowers = 0;
        int64_t value = 0;
        for ( int64_t ii = numDigits - 1; ii >= 0; --ii )
        {
            sumOfPowers += NUtils::power( rawDigits[ ii ], numDigits );

            value = ( value * base ) + rawDigits[ ii ];
        }

        auto retVal = ( value == sumOfPowers );
        return retVal;
    }

    // for 1000000
    //0, 1, 2, 3, 4
    //5, 6, 7, 8, 9
    //153, 370, 371, 407, 1634
    //8208, 9474, 54748, 92727, 93084
    //548834
    bool isNarcissistic( int64_t val, int base, bool& aOK )
    {
        return isNarcissisticDigits( val, base, aOK );
    }

    std::list< int64_t > computeFactors( int64_t num, bool properFactors )
    {
        std::list< int64_t > retVal;
        std::list< int64_t > retVal2;
        retVal.push_back( 1 );
        retVal2.push_back( num );

        // only need to go to half way point
        auto lastNum = ( num / 2 ) + ( ( ( num % 2 ) == 0 ) ? 0 : 1 );
        for ( int64_t ii = 2; ii < lastNum; ++ii )
        {
            if ( ( num % ii ) == 0 )
            {
                retVal.push_back( ii );
                auto other = num / ii;
                lastNum = std::min( lastNum, other );
                retVal2.push_front( other );
            }
        }

        if ( *retVal.rbegin() == *retVal2.begin() )
            retVal2.pop_front();
        retVal.insert( retVal.end(), retVal2.begin(), retVal2.end() );
        retVal.sort();

        if ( properFactors && !retVal.empty() )
            retVal.pop_back();
        return retVal;
    }

    std::list< int64_t > computePrimeFactors( int64_t num )
    {
        std::list< int64_t > retVal;

        while ( ( num % 2 ) == 0 )
        {
            retVal.push_back( 2 );
            num = num / 2;
        }

        int64_t lastNum = static_cast< int64_t >( std::floor( std::sqrt( num ) ) );

        for ( int64_t ii = 3; ii <= lastNum; ii = ii + 2 )
        {
            while ( ( num % ii ) == 0 )
            {
                retVal.push_back( ii );
                num = num / ii;
            }
        }
        if ( num > 2 )
            retVal.push_back( num );
        return retVal;
    }
    
    std::pair< int64_t, std::list< int64_t > > getSumOfFactors( int64_t curr, bool properFactors )
    {
        auto factors = computeFactors( curr, properFactors );
        int64_t sum = 0;
        for ( auto ii : factors )
            sum += ii;
        return std::make_pair( sum, factors );
    }

    std::pair< bool, std::list< int64_t > > isPerfect( int64_t num )
    {
        auto sum = getSumOfFactors( num, true );
        return std::make_pair( sum.first == num, sum.second );
    }

    bool isSemiPerfect( const std::vector< int64_t >& factors, size_t n, int64_t num )
    {
        if ( num == 0 )
            return true;
        if ( n == 0 && num != 0 )
            return false;

        if ( factors[ n - 1 ] > num )
            return isSemiPerfect( factors, n - 1, num );
        return isSemiPerfect( factors, n - 1, num )
            || isSemiPerfect( factors, n - 1, num - factors[ n - 1 ] );
    }

    std::pair< bool, std::list< int64_t > > isSemiPerfect( int64_t num )
    {
        auto sum = getSumOfFactors( num, true );
        auto factors = std::vector< int64_t >( { sum.second.begin(), sum.second.end() } );
        auto isSemiPerfect = NUtils::isSemiPerfect( factors, factors.size(), num );
        return std::make_pair( isSemiPerfect, sum.second );
    }

    std::pair< bool, std::list< int64_t > > isAbundant( int64_t num )
    {
        auto sum = getSumOfFactors( num, true );
        return std::make_pair( sum.first > num, sum.second );
    }

    long double factorial( int64_t num )
    {
        double retVal = 1.0;
        for ( int64_t ii = num; ii > 0; --ii)
        {
            retVal *= ii;
        }
        return retVal;
    }

    uint64_t numCombinations( int64_t numPossible, int64_t numSelections )
    {
        auto t1 = factorial( numPossible );
        auto t2 = factorial( numSelections );
        auto t3 = factorial( numPossible - numSelections );
        auto t4 = t1/( t2*t3 );
        auto retVal = static_cast< uint64_t >( t4 );
        return retVal;
    }
}
