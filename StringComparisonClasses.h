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

#ifndef __STRINGCOMPARISONCLASSES_H
#define __STRINGCOMPARISONCLASSES_H

#include "SABUtilsExport.h"

#include <string>
#include <QString>
namespace NSABUtils
{
    namespace NStringUtils
    {
        // Functors
        // Functor noCaseStringCmp: case-less string comparisons
        class SABUTILS_EXPORT noCaseStringCmp
        {
        public:
            bool operator() (const std::string& s1, const std::string& s2) const;
        };

        class SABUTILS_EXPORT noCaseStringEq
        {
        public:
            bool operator() (const std::string& s1, const std::string& s2) const;
        };

        struct SABUTILS_EXPORT noCaseStringHash
        {
            size_t operator()(const std::string& s) const;
        };

        class SABUTILS_EXPORT noCaseQStringCmp
        {
        public:
            bool operator() (const QString& s1, const QString& s2) const;
        };

        class SABUTILS_EXPORT noCaseQStringEq
        {
        public:
            bool operator() (const QString& s1, const QString& s2) const;
        };

        struct SABUTILS_EXPORT noCaseQStringHash
        {
            size_t operator()(const QString& s) const;
        };
    }
}
#endif 

