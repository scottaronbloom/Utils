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

#include "../CantorHash.h"

#include "gtest/gtest.h"

namespace
{
    TEST( TestCantorHash, TestCantorHash )
    {
        EXPECT_EQ( 2, NSABUtils::cantorHash( { 1 } ) );

        EXPECT_EQ( 8, NSABUtils::cantorHash( { 1, 2 } ) );
        EXPECT_NE( 8, NSABUtils::cantorHash( { 2, 1 } ) );

        EXPECT_EQ( 69, NSABUtils::cantorHash( { 1, 2, 3 } ) );
        EXPECT_NE( 69, NSABUtils::cantorHash( { 1, 3, 2 } ) );
        EXPECT_NE( 69, NSABUtils::cantorHash( { 3, 1, 2 } ) );
        EXPECT_NE( 69, NSABUtils::cantorHash( { 3, 2, 1 } ) );
        EXPECT_NE( 69, NSABUtils::cantorHash( { 2, 1, 3 } ) );
        EXPECT_NE( 69, NSABUtils::cantorHash( { 2, 3, 1 } ) );

    }
}

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    int retVal = RUN_ALL_TESTS();
    return retVal;
}
