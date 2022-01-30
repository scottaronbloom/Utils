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

#ifndef __FROMSTRING_H
#define __FROMSTRING_H 

#include "SABUtilsExport.h"

#include <string>
#include <sstream>
namespace NSABUtils
{

    template <class T>
    bool fromString(T & retVal, const char * arg)
    {
        if (!arg || !*arg)
            return false;
        std::stringstream ss(arg);
        ss >> retVal;
        char c;
        if (ss.fail() || ss.get(c))
            return false;
        return true;
    }

    template <class T>
    bool fromString(T & retVal, const std::string & arg)
    {
        return fromString(retVal, arg.c_str());
    }

    SABUTILS_EXPORT bool fromString(long & retVal, const char * arg, int base);
    SABUTILS_EXPORT bool fromString(long & retVal, const std::string & arg, int base);
    SABUTILS_EXPORT bool fromString(long & retVal, const std::string & arg);

    SABUTILS_EXPORT bool fromString(int & retVal, const char * arg, int base);
    SABUTILS_EXPORT bool fromString(int & retVal, const char * arg);
    SABUTILS_EXPORT bool fromString(int & retVal, const std::string & arg, int base);
    SABUTILS_EXPORT bool fromString(int & retVal, const std::string & arg);

    SABUTILS_EXPORT bool fromString(double & retVal, const std::string & arg);
    SABUTILS_EXPORT bool fromString(double & retVal, const char * arg);
    SABUTILS_EXPORT bool fromString(float & retVal, const std::string & arg);
    SABUTILS_EXPORT bool fromString(float & retVal, const char * arg);
    SABUTILS_EXPORT bool fromString(bool & retVal, const char * arg);
    SABUTILS_EXPORT bool fromString(bool & retVal, const std::string & arg);
}
#endif 

