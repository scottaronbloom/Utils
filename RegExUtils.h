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

#ifndef __REGEXUTILS_H
#define __REGEXUTILS_H

#include "SABUtilsExport.h"
#include <string>
#include <QString>
#include <list>
#include <optional>

class QRegularExpression;

namespace NSABUtils
{
    namespace NStringUtils
    {
        SABUTILS_EXPORT bool regExEqual( const std::string & lhs, const std::string & rhs );

        SABUTILS_EXPORT bool isExactMatchRegEx( const std::string & data, const std::string & pattern, bool nocase );
        SABUTILS_EXPORT std::list< std::string > splitStringRegEx( const std::string & string, const std::string & regex, bool nocase = false, bool skipEmpty = false ); // split based on regex

        SABUTILS_EXPORT bool  matchRegExpr( const char * s1, const char * s2 );

        SABUTILS_EXPORT QString encodeRegEx( const char * inString );
        SABUTILS_EXPORT QString encodeRegEx( QString inString );
        SABUTILS_EXPORT std::string encodeRegEx( const std::string & inString );

        SABUTILS_EXPORT std::string addToRegEx( std::string oldRegEx, const std::string & regEx );

        SABUTILS_EXPORT bool isSpecialRegExChar( char ch, bool includeDotSlash = true );
        SABUTILS_EXPORT bool isSpecialRegExChar( const QChar & ch, bool includeDotSlash = true );

        // used in the regExReplace and regExReplaceAll
        // $$ - is replaced with the literal $
        // $& - is replaced with with captured text
        // $N - is replaced with the Nth captured text
        // ${name} - is replaced with the named capture
        SABUTILS_EXPORT std::optional< QString > replaceMatch( const QString & replacement, QRegularExpressionMatch & match );

        // replaces all occurences of pattern in input with replacement
        SABUTILS_EXPORT std::optional< QString >  regExReplace( const QString & input, const QString & pattern, const QString & replacement );
        SABUTILS_EXPORT std::optional< QString >  regExReplace( const QString & input, const QRegularExpression & pattern, const QString & replacement );

    }
}
#endif 


