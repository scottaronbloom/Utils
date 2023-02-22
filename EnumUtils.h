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

#ifndef __ENUMUTILS_H
#define __ENUMUTILS_H

#include "SABUtilsExport.h"

#include <type_traits>
#define DECLARE_ENUM_FUNCS_LOGIC( EnumName ) \
    SABUTILS_EXPORT inline EnumName operator&( const EnumName &lhs, const EnumName &rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        IntType rhsVal = static_cast< IntType >( rhs ); \
        EnumName retVal = static_cast< EnumName >( lhsVal & rhsVal ); \
        return retVal; \
    } \
    SABUTILS_EXPORT inline EnumName operator&=( EnumName &lhs, const EnumName &rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        IntType rhsVal = static_cast< IntType >( rhs ); \
        lhs = static_cast< EnumName >( lhsVal & rhsVal ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator|( const EnumName &lhs, const EnumName &rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        IntType rhsVal = static_cast< IntType >( rhs ); \
        EnumName retVal = static_cast< EnumName >( lhsVal | rhsVal ); \
        return retVal; \
    } \
    SABUTILS_EXPORT inline EnumName operator|=( EnumName &lhs, const EnumName &rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        IntType rhsVal = static_cast< IntType >( rhs ); \
        lhs = static_cast< EnumName >( lhsVal | rhsVal ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator^( const EnumName &lhs, const EnumName &rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        IntType rhsVal = static_cast< IntType >( rhs ); \
        EnumName retVal = static_cast< EnumName >( lhsVal ^ rhsVal ); \
        return retVal; \
    } \
    SABUTILS_EXPORT inline EnumName operator^=( EnumName &lhs, const EnumName &rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        IntType rhsVal = static_cast< IntType >( rhs ); \
        lhs = static_cast< EnumName >( lhsVal ^ rhsVal ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator~( const EnumName &lhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        EnumName retVal = static_cast< EnumName >( ~lhsVal ); \
        return retVal; \
    }

#define DECLARE_ENUM_FUNCS_ARITHMETIC( EnumName ) \
    SABUTILS_EXPORT inline EnumName operator-( const EnumName &lhs, const EnumName &rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        IntType rhsVal = static_cast< IntType >( rhs ); \
        EnumName retVal = static_cast< EnumName >( lhsVal - rhsVal ); \
        return retVal; \
    } \
    SABUTILS_EXPORT inline EnumName operator-( const EnumName &lhs, size_t rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        EnumName retVal = static_cast< EnumName >( lhsVal - rhs ); \
        return retVal; \
    } \
    SABUTILS_EXPORT inline EnumName operator+( const EnumName &lhs, const EnumName &rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        IntType rhsVal = static_cast< IntType >( rhs ); \
        EnumName retVal = static_cast< EnumName >( lhsVal + rhsVal ); \
        return retVal; \
    } \
    SABUTILS_EXPORT inline EnumName operator+( const EnumName &lhs, size_t rhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        IntType lhsVal = static_cast< IntType >( lhs ); \
        EnumName retVal = static_cast< EnumName >( lhsVal + rhs ); \
        return retVal; \
    } \
    SABUTILS_EXPORT inline EnumName operator++( EnumName &lhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        auto val = static_cast< IntType >( lhs ); \
        lhs = static_cast< EnumName >( val + 1 ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator--( EnumName &lhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        auto val = static_cast< IntType >( lhs ); \
        lhs = static_cast< EnumName >( val - 1 ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator++( EnumName &lhs, int n ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        auto val = static_cast< IntType >( lhs ); \
        lhs = static_cast< EnumName >( val + ( ( n != 0 ) ? n : 1 ) ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator--( EnumName &lhs, int n ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        auto val = static_cast< IntType >( lhs ); \
        lhs = static_cast< EnumName >( val - ( ( n != 0 ) ? n : 1 ) ); \
        return lhs; \
    }

#define DECLARE_ENUM_FUNCS_ARITHMETIC_ONEHOT( EnumName ) \
    SABUTILS_EXPORT inline EnumName operator--( EnumName &lhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        auto val = static_cast< IntType >( lhs ); \
        lhs = static_cast< EnumName >( val >> 1 ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator++( EnumName &lhs ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        auto val = static_cast< IntType >( lhs ); \
        lhs = static_cast< EnumName >( val << 1 ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator--( EnumName &lhs, int n ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        auto val = static_cast< IntType >( lhs ); \
        lhs = static_cast< EnumName >( val >> ( ( n != 0 ) ? n : 1 ) ); \
        return lhs; \
    } \
    SABUTILS_EXPORT inline EnumName operator++( EnumName &lhs, int n ) \
    { \
        using IntType = std::underlying_type< EnumName >::type; \
        auto val = static_cast< IntType >( lhs ); \
        lhs = static_cast< EnumName >( val << ( ( n != 0 ) ? n : 1 ) ); \
        return lhs; \
    }

#define DECLARE_ENUM_ITERATORS( EnumName, First, Last ) \
    SABUTILS_EXPORT inline EnumName operator*( EnumName val ) \
    { \
        return val; \
    } \
    SABUTILS_EXPORT inline EnumName begin( EnumName ) \
    { \
        return First; \
    } \
    SABUTILS_EXPORT inline EnumName end( EnumName ) \
    { \
        EnumName tmp = Last; \
        return ++tmp; \
    }

#define DECLARE_ENUM_FUNCS( EnumName, First, Last ) \
    DECLARE_ENUM_FUNCS_LOGIC( EnumName ) \
    DECLARE_ENUM_FUNCS_ITERATORS( EnumName, First, Last ) \
    DECLARE_ENUM_FUNCS_ARITHMETIC( EnumName )

#endif
