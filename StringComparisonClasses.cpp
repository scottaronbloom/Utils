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

#include "StringComparisonClasses.h"
#include "StringUtils.h"
#include <QHash>

namespace NSABUtils
{
    namespace NStringUtils
    {
        size_t noCaseStringHash::operator()(const std::string& s) const
        {
            std::hash<std::string> hash_fn;
            return hash_fn(tolower(s));
        }

        bool noCaseStringCmp::operator() (const std::string& s1, const std::string& s2) const
        {
            return strCaseCmp(s1, s2) < 0;
        }

        bool noCaseStringEq::operator() (const std::string& s1, const std::string& s2) const
        {
            return strCaseCmp(s1, s2) == 0;
        }

        size_t noCaseQStringHash::operator()(const QString& s) const
        {
            return static_cast<size_t>(qHash(s));
        }

        bool noCaseQStringCmp::operator() (const QString& s1, const QString& s2) const
        {
            return s1.compare(s2, Qt::CaseInsensitive) < 0;
        }

        bool noCaseQStringEq::operator() (const QString& s1, const QString& s2) const
        {
            return s1.compare(s2, Qt::CaseInsensitive) == 0;
        }
    }

}
