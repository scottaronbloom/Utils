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

#include "../utils.h"
#include "../WordExp.h"
#include "../QtUtils.h"
#include "../FileUtils.h"

#include <QCoreApplication>
#include <string>
#include <memory>
#include <filesystem>
#include "gtest/gtest.h"

////template< typename T1, typename T2 >
//bool operator==( faketype, faketype ) { return true; }
//bool operator==( const std::pair< int64_t, std::list< int64_t > > & lhs, const std::pair< int64_t, std::list< int64_t > >& rhs )
//{
//    return false;
//}

void PrintTo( const QString & str, ::std::ostream * oss )
{
    *oss << qPrintable( str );
}

::std::ostream & operator<<( ::std::ostream & oss, const QString & str )
{
    oss << qPrintable( str );
    return oss;
}

namespace 
{
    TEST( TestUtils, TestListIndex )
    {
        std::list< std::string > lst = { "a", "b", "c", "d", "e" };
        EXPECT_EQ( "a", NSABUtils::indexInList( 0, lst ) );
        EXPECT_EQ( "b", NSABUtils::indexInList( 1, lst ) );
        EXPECT_EQ( "c", NSABUtils::indexInList( 2, lst ) );
        EXPECT_EQ( "d", NSABUtils::indexInList( 3, lst ) );
        EXPECT_EQ( "e", NSABUtils::indexInList( 4, lst ) );
        EXPECT_EQ( std::string(), NSABUtils::indexInList( 5, lst ) );
    }

    TEST( TestUtils, TestReplaceInList )
    {
        auto tmp = QStringList() << "a" << "b" << "c" << "d" << "e";
        auto t2 = QStringList() << "z" << "y" << "x" << "w" << "v";

        auto t3 = NSABUtils::replaceInList( tmp, 1, 2, t2, 3 ); // replaces b,c with z,y,x yeilding a, z, y, x, d, e

        EXPECT_EQ( 6, t3.length() );
        EXPECT_EQ( "a", t3[ 0 ] );
        EXPECT_EQ( "z", t3[ 1 ] );
        EXPECT_EQ( "y", t3[ 2 ] );
        EXPECT_EQ( "x", t3[ 3 ] );
        EXPECT_EQ( "d", t3[ 4 ] );
        EXPECT_EQ( "e", t3[ 5 ] );

        t3 = NSABUtils::replaceInList( tmp, 1, 6, t2, 3 ); // replaces b,c, d, e with z,y,x yeilding a, z, y, x

        EXPECT_EQ( 4, t3.length() );
        EXPECT_EQ( "a", t3[ 0 ] );
        EXPECT_EQ( "z", t3[ 1 ] );
        EXPECT_EQ( "y", t3[ 2 ] );
        EXPECT_EQ( "x", t3[ 3 ] );

        t3 = NSABUtils::replaceInList( tmp, 3, 6, t2, 5); // replaces d, e with z,y,x,q,v yeilding a, b, c, z, y, x, w, v

        EXPECT_EQ( 8, t3.length() );
        EXPECT_EQ( "a", t3[ 0 ] );
        EXPECT_EQ( "b", t3[ 1 ] );
        EXPECT_EQ( "c", t3[ 2 ] );
        EXPECT_EQ( "z", t3[ 3 ] );
        EXPECT_EQ( "y", t3[ 4 ] );
        EXPECT_EQ( "x", t3[ 5 ] );
        EXPECT_EQ( "w", t3[ 6 ] );
        EXPECT_EQ( "v", t3[ 7 ] );
    }

    TEST( TestUtils, TestReplaceInListStd )
    {
        auto tmp = std::list< std::string >{ "a", "b", "c", "d", "e" };
        auto t2 = std::list< std::string >{ "z", "y", "x", "w", "v" };

        auto t3 = NSABUtils::replaceInList( tmp, 1, 2, t2, 3 ); // replaces b,c with z,y,x yeilding a, z, y, x, d, e

        EXPECT_EQ( 6, t3.size() );
        auto pos = t3.begin();
        EXPECT_EQ( "a", *pos++ );
        EXPECT_EQ( "z", *pos++ );
        EXPECT_EQ( "y", *pos++ );
        EXPECT_EQ( "x", *pos++ );
        EXPECT_EQ( "d", *pos++ );
        EXPECT_EQ( "e", *pos++ );

        t3 = NSABUtils::replaceInList( tmp, 1, 6, t2, 3 ); // replaces b,c, d, e with z,y,x yeilding a, z, y, x

        EXPECT_EQ( 4, t3.size() );
        pos = t3.begin();
        EXPECT_EQ( "a", *pos++ );
        EXPECT_EQ( "z", *pos++ );
        EXPECT_EQ( "y", *pos++ );
        EXPECT_EQ( "x", *pos++ );

        t3 = NSABUtils::replaceInList( tmp, 3, 6, t2, 5 ); // replaces d, e with z,y,x,q,v yeilding a, b, c, z, y, x, w, v

        EXPECT_EQ( 8, t3.size() );
        pos = t3.begin();
        EXPECT_EQ( "a", *pos++ );
        EXPECT_EQ( "b", *pos++ );
        EXPECT_EQ( "c", *pos++ );
        EXPECT_EQ( "z", *pos++ );
        EXPECT_EQ( "y", *pos++ );
        EXPECT_EQ( "x", *pos++ );
        EXPECT_EQ( "w", *pos++ );
        EXPECT_EQ( "v", *pos++ );
    }

    TEST(TestUtils, power )
    {
        EXPECT_DOUBLE_EQ( 100.0, NSABUtils::power( 10.0, 2.0 ) );
        EXPECT_EQ( 100, NSABUtils::power( 10, 2 ) );
        EXPECT_EQ( 1, NSABUtils::power( 10, 0 ) );
        EXPECT_EQ( 99, NSABUtils::power( 99, 1 ) );
        EXPECT_EQ( 0, NSABUtils::power( 0, 10293 ) );
        EXPECT_EQ( 1, NSABUtils::power( 1, 999 ) );
    }

    TEST( TestUtils, fromChar )
    {
        bool aOK;
        EXPECT_EQ( 1, NSABUtils::fromChar( '-', 16, aOK ) );
        EXPECT_TRUE( aOK );

        EXPECT_EQ( 1, NSABUtils::fromChar( '_', 16, aOK ) );
        EXPECT_TRUE( aOK );

        EXPECT_EQ( 0, NSABUtils::fromChar( '0' + 15, 12, aOK ) );
        EXPECT_FALSE( aOK );

        EXPECT_EQ( 0, NSABUtils::fromChar( '0' + 11, 12, aOK ) );
        EXPECT_FALSE( aOK );

        EXPECT_EQ( 11, NSABUtils::fromChar( 'b', 12, aOK ) );
        EXPECT_TRUE( aOK );

        for( int base = 2; base < 36; ++base )
        {
            for( int jj = 0; ( jj < 9 ) && ( jj < base ); ++jj )
            {
                EXPECT_EQ( jj, NSABUtils::fromChar( '0' + jj, base, aOK ) ) << "Failed: Char: " << (char)jj << " Base: " << base;
                EXPECT_TRUE( aOK );
            }
            for( int jj = 'a'; jj < 'a' + ( base - 10 ); ++jj )
            {
                EXPECT_EQ( 10 + jj - 'a', NSABUtils::fromChar( jj, base, aOK ) ) << "Failed: Char: " << (char)jj << " Base: " << base;
                EXPECT_TRUE( aOK );
            }
            for ( int jj = 'A'; jj < 'A' + ( base - 10 ); ++jj )
            {
                EXPECT_EQ( 10 + jj - 'A', NSABUtils::fromChar( jj, base, aOK ) ) << "Failed: Char: " << (char)jj << " Base: " << base;
                EXPECT_TRUE( aOK );
            }
        }
    }

    TEST( TestUtils, toChar )
    {
        for ( int jj = 0; jj < 36; ++jj )
        {
            char tmp1 = 'a' + jj - 10;
            (void)tmp1;
            char tmp2 = '0' + jj;
            (void)tmp2;
            if ( jj <= 9 )
                EXPECT_EQ( '0' + jj, NSABUtils::toChar( jj ) ) << "Failed: Value: " << jj;
            else
                EXPECT_EQ( 'a' + jj - 10, NSABUtils::toChar( jj ) ) << "Failed: Value: " << jj;
        }
    }
    TEST( TestUtils, toDigits )
    {
        int8_t digits[ 10 ] = {0};
        size_t numDigits = 0;
        auto retVal = std::make_pair( digits, static_cast< uint32_t >( 2 ) );
        bool aOK;
        NSABUtils::toDigits( 101, 10, retVal, numDigits, &aOK );
        EXPECT_FALSE( aOK );

        retVal = std::make_pair( digits, 3 );
        NSABUtils::toDigits( 123, 10, retVal, numDigits, &aOK );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( 3, numDigits );
        EXPECT_EQ( 3, digits[ 0 ] );
        EXPECT_EQ( 2, digits[ 1 ] );
        EXPECT_EQ( 1, digits[ 2 ] );

        retVal = std::make_pair( digits, 10 );
        NSABUtils::toDigits( 1234567890, 10, retVal, numDigits, &aOK );
        EXPECT_TRUE( aOK );
        ASSERT_EQ( 10, numDigits );
        EXPECT_EQ( 0, digits[ 0 ] );
        EXPECT_EQ( 9, digits[ 1 ] );
        EXPECT_EQ( 8, digits[ 2 ] );
        EXPECT_EQ( 7, digits[ 3 ] );
        EXPECT_EQ( 6, digits[ 4 ] );
        EXPECT_EQ( 5, digits[ 5 ] );
        EXPECT_EQ( 4, digits[ 6 ] );
        EXPECT_EQ( 3, digits[ 7 ] );
        EXPECT_EQ( 2, digits[ 8 ] );
        EXPECT_EQ( 1, digits[ 9 ] );
    }

    TEST( TestUtils, toString )
    {
        EXPECT_EQ( "a", NSABUtils::toString( 10, 16 ) );
        EXPECT_EQ( "ff", NSABUtils::toString( 255, 16 ) );
        EXPECT_EQ( "1234567890", NSABUtils::toString( 1234567890, 10 ) );
    }
    
    TEST( TestUtils, fromString )
    {
        EXPECT_EQ( 10, NSABUtils::fromString( "a", 16 ) );
        EXPECT_EQ( 255, NSABUtils::fromString( "ff", 16 ) );
        EXPECT_EQ( 1234567890, NSABUtils::fromString( "1234567890", 10 ) );
    }

    TEST( TestUtils, computeFactors )
    {
        EXPECT_EQ( std::list< int64_t >( { 1, 2, 7, 14 } ), NSABUtils::computeFactors( 14 ) );
        EXPECT_EQ( std::list< int64_t >( { 1, 3, 7, 21 } ), NSABUtils::computeFactors( 21 ) );
        EXPECT_EQ( std::list< int64_t >( { 1, 2, 4, 5, 8, 10, 20, 25, 40, 50, 100, 200 } ), NSABUtils::computeFactors( 200 ) );
        EXPECT_EQ( std::list< int64_t >( { 1, 3, 5, 15, 823, 2469, 4115, 12345 } ), NSABUtils::computeFactors( 12345 ) );
    }

    TEST( TestUtils, computePrimeFactors )
    {
        EXPECT_EQ( std::list< int64_t >( { 2, 7 } ), NSABUtils::computePrimeFactors( 14 ) );
        EXPECT_EQ( std::list< int64_t >( { 3, 7 } ), NSABUtils::computePrimeFactors( 21 ) );
        EXPECT_EQ( std::list< int64_t >( { 2, 2, 2, 5, 5 } ), NSABUtils::computePrimeFactors( 200 ) );
        EXPECT_EQ( std::list< int64_t >( { 3, 5, 823 } ), NSABUtils::computePrimeFactors( 12345 ) );
    }

    TEST( TestUtils, getSumOfFactors )
    {
        EXPECT_EQ( std::make_pair( (int64_t)10, std::list< int64_t >( { 1, 2, 7 } ) ), NSABUtils::getSumOfFactors( 14, true ) );
        EXPECT_EQ( std::make_pair( (int64_t)11, std::list< int64_t >( { 1, 3, 7 } ) ), NSABUtils::getSumOfFactors( 21, true ) );
        EXPECT_EQ( std::make_pair( (int64_t)265, std::list< int64_t >( { 1, 2, 4, 5, 8, 10, 20, 25, 40, 50, 100 } ) ), NSABUtils::getSumOfFactors( 200, true ) );
        EXPECT_EQ( std::make_pair( (int64_t)7431, std::list< int64_t >( { 1, 3, 5, 15, 823, 2469, 4115 } ) ), NSABUtils::getSumOfFactors( 12345, true ) );
    }

    TEST( TestUtils, isNarcissistic )
    {
        std::vector< std::pair< int, std::vector< std::string > > > numbers
        {
             {  2, { "1" } }
            ,{  3, { "1", "2", "12", "22", "122" } }
            ,{  4, { "1", "2", "3", "130", "131", "203", "223", "313", "332", "1103", "3303" } }
            ,{  5, { "1", "2", "3", "4", "23", "33", "103", "433", "2124", "2403", "3134", "124030", "124031", "242423" } }
            ,{  6, { "1", "2", "3", "4", "5", "243", "514", "14340", "14341", "14432", "23520", "23521", "44405", "435152", "5435254", "12222215", "555435035" } }
            ,{  7, { "1", "2", "3", "4", "5", "6", "13", "34", "44", "63", "250", "251", "305", "505", "12205", "12252", "13350", "13351", "15124", "36034" } }
            ,{  8, { "1", "2", "3", "4", "5", "6", "7", "24", "64", "134", "205", "463", "660", "661" } }
            ,{  9, { "1", "2", "3", "4", "5", "6", "7", "8", "45", "55", "150", "151", "570", "571", "2446", "12036", "12336", "14462" } }
            ,{ 10, { "1", "2", "3", "4", "5", "6", "7", "8", "9", "153", "370", "371", "407", "1634", "8208", "9474", "54748", "92727", "93084", "548834" } }
            ,{ 11, { "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "56", "66", "105", "307", "708", "966", "A06", "A64", "8009", "11720", "11721", "12470" } }
            ,{ 12, { "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "25", "A5", "577", "668", "A83", "14765", "938A4", "369862", "A2394A" } }
            ,{ 13, { "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "14", "36", "67", "77", "A6", "C4", "490", "491", "509", "B85", "3964", "22593", "5B350" } }
            ,{ 14, { "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "136", "409", "74AB5", "153A632" } }
            ,{ 15, { "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "78", "88", "C3A", "D87", "1774", "E819", "E829", "7995C", "829BB", "A36BC" } }
            ,{ 16, { "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", 
            "156", "173", "208", "248", "285", "4A5", "5B0", "5B1", "60B", "64B", "8C0", "8C1", 
            "99a", "aa9", "ac3", "ca8", "e69", "ea0", "ea1", "b8d2", "13579", "2b702", "2b722", 
            "5a07c", "5a47c", "c00e0", "c00e1", "c04e0", "c04e1", "c60e7", "c64e7", 
            "c80e0", "c80e1", "c84e0", "c84e1", "de030" } }
        };

        bool aOK;
        for ( auto&& ii : numbers )
        {
            for( auto && jj : ii.second )
            {
                auto decValue = NSABUtils::fromString( jj, ii.first );
                EXPECT_TRUE( NSABUtils::isNarcissistic( decValue, ii.first, aOK ) ) << "base=" << ii.first << " Number: " << jj << "(" << decValue << ")";
                EXPECT_TRUE( aOK );
            }
        }

    }
    TEST( TestUtils, isSemiPerfect )
    {
        EXPECT_EQ( std::make_pair( false, std::list< int64_t >( { 1, 2, 7 } ) ), NSABUtils::isSemiPerfect( 14 ) );
        EXPECT_EQ( std::make_pair( false, std::list< int64_t >( { 1, 3, 7 } ) ), NSABUtils::isSemiPerfect( 21 ) );
        EXPECT_EQ( std::make_pair( true, std::list< int64_t >( { 1, 2, 4, 5, 8, 10, 20, 25, 40, 50, 100 } ) ), NSABUtils::isSemiPerfect( 200 ) );
        EXPECT_EQ( std::make_pair( false, std::list< int64_t >( { 1, 3, 5, 15, 823, 2469, 4115 } ) ), NSABUtils::isSemiPerfect( 12345 ) );

        std::vector< int64_t > numbers = 
        {
            6, 12, 18, 20, 24, 28, 30, 
            36, 40, 42, 48, 54, 56, 60, 
            66, 72, 78, 80, 84, 88, 90, 
            96, 100, 102, 104, 108, 112, 
            114, 120, 126, 132, 138, 140, 
            144, 150, 156, 160, 162, 168, 
            174, 176, 180, 186, 192, 196, 
            198, 200, 204, 208, 210, 216, 
            220, 222, 224, 228, 234, 240, 
            246, 252, 258, 260, 264, 272, 304, 350
        };
        for( auto && ii : numbers )
        {
            EXPECT_EQ( true, NSABUtils::isSemiPerfect( ii ).first );
        }
    }

    TEST( TestUtils, isPerfect )
    {
        std::vector< int64_t > numbers =
        {
            6, 28, 496, 8128, 33550336, 
            8589869056, 
            137438691328, 
            //2305843008139952128
            //2658455991569831744654692615953842176, 
            //191561942608236107294793378084303638130997321548169216 
        };
        for ( auto&& ii : numbers )
        {
            EXPECT_EQ( true, NSABUtils::isPerfect( ii ).first );
        }
    }

    TEST( TestUtils, isAbundant )
    {
        std::vector< int64_t > numbers =
        {
            12, 18, 20, 24, 30, 36, 40, 42, 
            48, 54, 56, 60, 66, 70, 72, 78, 
            80, 84, 88, 90, 96, 100, 102, 104, 
            108, 112, 114, 120, 126, 132, 138, 
            140, 144, 150, 156, 160, 162, 168, 
            174, 176, 180, 186, 192, 196, 198, 
            200, 204, 208, 210, 216, 220, 222, 
            224, 228, 234, 240, 246, 252, 258, 
            260, 264, 270 
        };
        for ( auto&& ii : numbers )
        {
            EXPECT_EQ( true, NSABUtils::isAbundant( ii ).first );
        }
    }

    TEST( TestUtils, getNumberListString )
    {
        std::vector< int64_t > numbers =
        {
            -12, -18, 20, 24, 30, 36, 40, 42
        };

        EXPECT_EQ( "    -12, -18, 20, 24, 30\n    36, 40, 42", NSABUtils::getNumberListString( numbers, 10 ) );
        EXPECT_EQ( "    -14(=-12), -22(=-18), 24(=20), 30(=24), 36(=30)\n    44(=36), 50(=40), 52(=42)", NSABUtils::getNumberListString( numbers, 8 ) );
    }

#if __cplusplus > 201703L
    TEST( TestUtils, findLargestIndexInBitSet )
    {
        EXPECT_EQ( -99, NSABUtils::findLargestIndexInBitSet( std::bitset< 16 >() ).value_or( -99 ) );
        EXPECT_EQ( 7, NSABUtils::findLargestIndexInBitSet( std::bitset< 16 >( 255 ) ).value_or( -99 ) );
        EXPECT_EQ( 15, NSABUtils::findLargestIndexInBitSet( std::bitset< 16 >( 32768   ) ).value_or( -99 ) );
    }

    TEST( TestUtils, findSmallestIndexInBitSet )
    {
        EXPECT_EQ( -99, NSABUtils::findSmallestIndexInBitSet( std::bitset< 16 >() ).value_or( -99 ) );
        EXPECT_EQ( 0, NSABUtils::findSmallestIndexInBitSet( std::bitset< 16 >( 255 ) ).value_or( -99 ) );
        EXPECT_EQ( 15, NSABUtils::findSmallestIndexInBitSet( std::bitset< 16 >( 32768 ) ).value_or( -99 ) );
    }

    TEST(TestUtils, TestCombinationalString)
    {
        std::vector< std::string > arr = { "1", "2", "3" };
        auto numCombinations = NUtils::numCombinations(3, 3);
        auto combinations = NUtils::allCombinations(arr, 3);
    }

    TEST( TestUtils, TestCombinational )
    {
        std::vector< int > arr = { 1, 2, 3, 4, 5, 6, 7 };
        auto combinations = NSABUtils::allCombinations( arr, 5 );
        EXPECT_EQ( 21, combinations.size() );
        auto ii = 0;
        EXPECT_EQ( std::vector< int >( { 1, 2, 3, 4, 5 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 3, 4, 6 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 3, 4, 7 } ), combinations[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 2, 3, 5, 6 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 3, 5, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 3, 6, 7 } ), combinations[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 5, 6 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 5, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 6, 7 } ), combinations[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 2, 5, 6, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 5, 6 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 5, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 4, 5, 6, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 3, 4, 5, 6 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 3, 4, 5, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 3, 4, 6, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 3, 5, 6, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 4, 5, 6, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 3, 4, 5, 6, 7 } ), combinations[ ii++ ] );
        EXPECT_EQ( 21, ii );
    }
#endif

    TEST( TestUtils, TestCartiseanProduct )
    {
        std::vector< std::list< int > > arr = { { 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }, { 8, 9, 10 } };

        auto products = NSABUtils::cartiseanProduct( arr );
        ASSERT_EQ( 24, products.size() );
        auto ii = 0;
        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 4, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 2, 5, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 5, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 5, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 2, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 5, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 10 } ), products[ ii++ ] );
    }

    TEST( TestUtils, TestCartiseanProductFirstMulti )
    {
        std::vector< std::list< int > > arr = { { 1, 2 }, { 3, 4 }, { 5, 6 }, { 7 }, { 8, 9, 10 } };

        auto products = NSABUtils::cartiseanProduct( arr );
        ASSERT_EQ( 24, products.size() );
        auto ii = 0;
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 6, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 6, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 6, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 4, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 4, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 4, 5, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 4, 6, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 4, 6, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 4, 6, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 2, 3, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 3, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 3, 5, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 2, 3, 6, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 3, 6, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 3, 6, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 2, 4, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 4, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 4, 5, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 2, 4, 6, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 4, 6, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 2, 4, 6, 7, 10 } ), products[ ii++ ] );
    }

    TEST( TestUtils, TestCartiseanProductWDupe )
    {
        std::vector< std::list< int > > arr = { { 1 }, { 1, 3 }, { 4, 5 }, { 6, 7 }, { 8, 9, 10 } };

        auto products = NSABUtils::cartiseanProduct( arr );
        ASSERT_EQ( 24, products.size() );
        auto ii = 0;
        EXPECT_EQ( std::vector< int >( { 1, 1, 4, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 1, 4, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 1, 4, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 1, 4, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 1, 4, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 1, 4, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 1, 5, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 1, 5, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 1, 5, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 1, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 1, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 1, 5, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 10 } ), products[ ii++ ] );
    }

    TEST( TestUtils, TestCartiseanProductNoDupe )
    {
        std::vector< std::list< int > > arr = { { 1 }, { 1, 3 }, { 4, 5 }, { 6, 7 }, { 8, 9, 10 } };

        std::function< bool( const std::vector< int >& curr, const int & obj ) > func = []( const std::vector< int >& curr, const int& obj )
        {
            return std::find( curr.begin(), curr.end(), obj ) == curr.end();
        };

        auto products = NSABUtils::cartiseanProduct( arr, func );
        ASSERT_EQ( 12, products.size() );
        auto ii = 0;
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 4, 7, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 6, 10 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 8 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 9 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3, 5, 7, 10 } ), products[ ii++ ] );
    }

#if __cplusplus > 201703L
    TEST( TestUtils, TestCartiseanSquare )
    {
        auto arr = std::vector< int >( { 0, 1, 2, 3 } );

        auto products = NSABUtils::allCombinations( arr, 2 );

        // (0,1) (0,2) (0,3) (1,2) (1,3) (2,3)
        EXPECT_EQ( 6, products.size() ) << products;
        size_t ii = 0;

        EXPECT_EQ( std::vector< int >( { 0, 1 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 0, 2 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 0, 3 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 1, 2 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 3 } ), products[ ii++ ] );

        EXPECT_EQ( std::vector< int >( { 2, 3 } ), products[ ii++ ] );

        products = NSABUtils::allCombinations( arr, 3 );
        EXPECT_EQ( 4, products.size() ) << products;
        ii = 0;
        EXPECT_EQ( std::vector< int >( { 0, 1, 2 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 0, 1, 3 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 0, 2, 3 } ), products[ ii++ ] );
        EXPECT_EQ( std::vector< int >( { 1, 2, 3 } ), products[ ii++ ] );

        products = NSABUtils::allCombinations( arr, 4 );
        EXPECT_EQ( 1, products.size() ) << products;
        ii = 0;
        EXPECT_EQ( std::vector< int >( { 0, 1, 2, 3 } ), products[ ii++ ] );
    }
#endif

#ifdef WIN32
#define HOME_DIR R"(C:\Users\scott.TOWEL42)"

    TEST( TestUtils, TestWordExp )
    {
        using namespace NSABUtils;
        bool aOK = false;
        EXPECT_EQ( HOME_DIR, CWordExp::getHomeDir( "scott", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( "", CWordExp::getHomeDir( "unknown", &aOK ) );
        EXPECT_FALSE( aOK );

        EXPECT_EQ( HOME_DIR, CWordExp::expandTildePath( "~scott", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( HOME_DIR "\\", CWordExp::expandTildePath( "~scott/", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( HOME_DIR "\\", CWordExp::expandTildePath( "~scott\\", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( HOME_DIR "\\", CWordExp::expandTildePath( "~\\", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( HOME_DIR "\\", CWordExp::expandTildePath( "~/", &aOK ) );
        EXPECT_TRUE( aOK );

        EXPECT_EQ( "~unknown/", CWordExp::expandTildePath( "~unknown/", &aOK ) );
        EXPECT_FALSE( aOK );
        EXPECT_EQ( "~unknown\\", CWordExp::expandTildePath( "~unknown\\", &aOK ) );
        EXPECT_FALSE( aOK );

        EXPECT_EQ( "scott", CWordExp::getUserName() );
        //EXPECT_EQ( "thor", CWordExp::getHostName() );

        //CWordExp wordExp( "%HOMEDRIVE%%HOMEPATH%/*/sb" );
        ////ASSERT_EQ( 1, wordExp.getAbsoluteFilePaths( &aOK ).size() );
        //EXPECT_TRUE( aOK );
        //EXPECT_EQ( "C:\\Users\\scott\\source\\sb", wordExp.getAbsoluteFilePaths()[ 0 ] );

        //CWordExp wordExp1( "e:/*/*/sb/*" );
        //EXPECT_GE( 64, wordExp1.getAbsoluteFilePaths( &aOK ).size() );
        //EXPECT_TRUE( aOK );
    }
#elif DONTTEST
    TEST( TestUtils, TestWordExp )
    {
        bool aOK = false;
        EXPECT_EQ( QString( "/home/scott" ), CWordExp::getHomeDir( "scott", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( "", CWordExp::getHomeDir( "unknown", &aOK ) );
        EXPECT_FALSE( aOK );

        EXPECT_EQ( "/home/scott", CWordExp::expandTildePath( "~scott", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( "/home/scott/", CWordExp::expandTildePath( "~scott/", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( "/home/scott/", CWordExp::expandTildePath( "~scott\\", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( "/home/scott/", CWordExp::expandTildePath( "~\\", &aOK ) );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( "/home/scott/", CWordExp::expandTildePath( "~/", &aOK ) );
        EXPECT_TRUE( aOK );

        EXPECT_EQ( "~unknown/", CWordExp::expandTildePath( "~unknown/", &aOK ) );
        EXPECT_FALSE( aOK );
        EXPECT_EQ( "~unknown\\", CWordExp::expandTildePath( "~unknown\\", &aOK ) );
        EXPECT_FALSE( aOK );

        EXPECT_EQ( "scott", CWordExp::getUserName() );
        EXPECT_EQ( "localhost.localdomain", CWordExp::getHostName() );

        CWordExp wordExp( "$HOME/*/sb" );
        ASSERT_EQ( 1, wordExp.getAbsoluteFilePaths( &aOK ).size() );
        EXPECT_TRUE( aOK );
        EXPECT_EQ( "/home/scott/fuckit/sb", wordExp.getAbsoluteFilePaths()[ 0 ] );

        CWordExp wordExp1( "/*/*/sb/*" );
        EXPECT_EQ( 17, wordExp1.getAbsoluteFilePaths( &aOK ).size() );
        EXPECT_TRUE( aOK );
    }
#endif

    TEST( TestUtils, TestExpandEnvVars )
    {
        qputenv( "FOOBAR", "ENVVAR" );
        std::set< QString > envVars;
        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "$FOOBAR", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo$FOOBAR/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );

        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "\\$FOOBAR", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo\\$FOOBAR/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );

        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "\\$(FOOBAR)", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo\\$\\(FOOBAR\\)/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "\\$\\(FOOBAR\\)", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo\\$\\(FOOBAR\\)/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );

        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "${FOOBAR}", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo$\\{FOOBAR\\}/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "$\\{FOOBAR\\}", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo$\\{FOOBAR\\}/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );

        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "%FOOBAR%", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo%FOOBAR%/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );

        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "\\%FOOBAR\\%", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo\\%FOOBAR\\%/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );

        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "\\%(FOOBAR)\\%", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo\\%\\(FOOBAR\\)\\%/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "\\%\\(FOOBAR\\)\\%", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo\\%\\(FOOBAR\\)\\%/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );

        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "%{FOOBAR}%", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo%\\{FOOBAR\\}%/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "ENVVAR", NSABUtils::NFileUtils::expandEnvVars( "%\\{FOOBAR\\}%", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
        EXPECT_EQ( "fooENVVAR/bar", NSABUtils::NFileUtils::expandEnvVars( "foo%\\{FOOBAR\\}%/bar", &envVars ) );
        EXPECT_EQ( 1, envVars.size() );
        EXPECT_EQ( "FOOBAR", *envVars.begin() );
    }

    TEST( TestUtils, TestSoftenVars )
    {
        qputenv( "HOME", "/home/sbloom" );
        qputenv( "BAR", "bar" );

        EXPECT_EQ( "${HOME}/foo/bar", NSABUtils::NFileUtils::gSoftenPath( "/home/sbloom/foo/bar", { "HOME" }, true ) );
        EXPECT_EQ( "${HOME}/foo/${BAR}", NSABUtils::NFileUtils::gSoftenPath( "/home/sbloom/foo/bar", { "HOME", "BAR" }, true ) );

#ifdef WIN32
        EXPECT_EQ( "%HOME%/foo/bar", NSABUtils::NFileUtils::gSoftenPath( "/home/sbloom/foo/bar", { "HOME" } ) );
        EXPECT_EQ( "%HOME%/foo/%BAR%", NSABUtils::NFileUtils::gSoftenPath( "/home/sbloom/foo/bar", { "HOME", "BAR" } ) );
#else
        EXPECT_EQ( "${HOME}/foo/bar", NSABUtils::NFileUtils::gSoftenPath( "/home/sbloom/foo/bar", { "HOME" } ) );
        EXPECT_EQ( "${HOME}/foo/${BAR}", NSABUtils::NFileUtils::gSoftenPath( "/home/sbloom/foo/bar", { "HOME", "BAR" } ) );
#endif
    }


    TEST( TestUtils, TestFileSizeString )
    {
        // base 1000
        EXPECT_EQ( QString( "324.579KB" ), NSABUtils::NFileUtils::fileSizeString( 324579, true, false, 3U ) );
        EXPECT_EQ( QString( "324.58KB" ), NSABUtils::NFileUtils::fileSizeString( 324579, true, false, 2U ) );
        EXPECT_EQ( QString( "324.6KB" ), NSABUtils::NFileUtils::fileSizeString( 324579, true, false, 1U ) );
        EXPECT_EQ( QString( "325KB" ), NSABUtils::NFileUtils::fileSizeString( 324579, true, false, 0U ) );

        // base 1024
        EXPECT_EQ( QString( "316.995KiB" ), NSABUtils::NFileUtils::fileSizeString( 324579, true, true, 3U ) );
        EXPECT_EQ( QString( "317KiB" ), NSABUtils::NFileUtils::fileSizeString( 324579, true, true, 2U ) );
        EXPECT_EQ( QString( "317KiB" ), NSABUtils::NFileUtils::fileSizeString( 324579, true, true, 1U ) );
        EXPECT_EQ( QString( "317KiB" ), NSABUtils::NFileUtils::fileSizeString( 324579, true, true, 0U ) );
    }

    TEST( TestUtils, TestTimeString )
    {
        EXPECT_EQ( QString( "00:00:00:01.001 (1,001 seconds)" ), NSABUtils::CTimeString( 1001 ).toString() );
    }
	//CFlowStateDefItem * CFlowWidgetImpl::mRemoveFromTopLevelItems( CFlowStateDefItem * xItem )
	//{
	//	auto first = std::find_if( fTopLevelItems.begin(), fTopLevelItems.end(), gPageEquals( xItem ) );
	//	if ( first != fTopLevelItems.end() )
	//	{
	//		for ( auto ii = first; ++ii != fTopLevelItems.end(); )
	//		{
	//			if ( ( *ii ).get() != xItem )
	//			{
	//				*first++ = std::move( *ii );
	//			}
	//		}
	//	}
	//	fTopLevelItems.erase( first );
	//}

    TEST( TestUtils, Help )
    {
        std::vector< int > vector = { 1, 2, 3, 4, 5, 6, 7, 8 };

        auto isInRange = []( int min, int max )
        {
            return [min, max]( int ii ) { return ( ii >= min ) && ( ii <= max ); };
        };

        std::vector< int > lTmp;
        auto first = std::find_if( vector.begin(), vector.end(), isInRange( 2, 4 ) );
        //vector.erase( std::remove_if( vector.begin(), vector.end(), isInRange( 2, 4 ) ), vector.end() );
        if ( first != vector.end() )
        {
            lTmp.push_back( *first );
            for ( auto ii = first; ++ii != vector.end(); )
            {
                if ( !isInRange( 2, 4 )( *ii ) )
                {
                    *first++ = std::move( *ii );
                }
                else
                {
                    lTmp.push_back( *ii );
                }
            }
        }
        vector.erase( first, vector.end() );
        EXPECT_EQ( 3, lTmp.size() );
        EXPECT_EQ( 5, vector.size() );
    }



}


int main( int argc, char **argv ) {
    QCoreApplication appl( argc, argv );
    ::testing::InitGoogleTest( &argc, argv );
    int retVal = RUN_ALL_TESTS();
    return retVal;
}



