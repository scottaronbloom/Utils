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

#ifndef __STRINGUTILS_H
#define __STRINGUTILS_H

#include "SABUtilsExport.h"
#include "nodiscard.h"

#include <list>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <climits>
#include <cstdint>
#include <unordered_set>
#include <chrono>
#include <cctype>
#include <QString>
#include <cctype>
#include <optional>

#include "EnumUtils.h"
#include "StringComparisonClasses.h"
#include "nodiscard.h"

#include <locale>

class QRegularExpression;

namespace NSABUtils
{
    namespace NStringUtils
    {
#if (QT_VERSION < QT_VERSION_CHECK( 5, 14, 0 ))
        constexpr QString::SplitBehavior TSkipEmptyParts = QString::SkipEmptyParts;
        constexpr QString::SplitBehavior TKeepEmptyParts = QString::KeepEmptyParts;
#else
        constexpr Qt::SplitBehavior TSkipEmptyParts = Qt::SkipEmptyParts;
        constexpr Qt::SplitBehavior TKeepEmptyParts = Qt::KeepEmptyParts;
#endif

        static const std::size_t sMAXLINE{ 2048 };
        SABUTILS_EXPORT inline bool isWhiteSpace( char ch ) { return ( ch == ' ' ) || ( ch == '\t' ); }
        SABUTILS_EXPORT inline bool isWhiteSpace( const char * s ) { return s && isWhiteSpace( *s ); }
        SABUTILS_EXPORT inline bool isWhiteSpace( const std::string::iterator & s ) { return isWhiteSpace( *s ); }
        SABUTILS_EXPORT inline bool isWhiteSpace( const std::string::const_iterator & s ) { return isWhiteSpace( *s ); }
        SABUTILS_EXPORT inline bool isWhiteSpaceN( char ch ) { return isWhiteSpace( ch ) || ( ch == '\n' ); }
        SABUTILS_EXPORT inline bool isWhiteSpaceN( const char * s ) { return s && isWhiteSpaceN( *s ); }
        SABUTILS_EXPORT inline bool isWhiteSpaceN( const std::string::iterator & s ) { return isWhiteSpaceN( *s ); }
        SABUTILS_EXPORT inline bool isWhiteSpaceN( const std::string::const_iterator & s ) { return isWhiteSpaceN( *s ); }
        SABUTILS_EXPORT inline bool isOperator( char ch ) { return ( ch == '=' ) || ( ch == '!' ) || ( ch == '>' ) || ( ch == '<' ); }
        SABUTILS_EXPORT inline bool isOperator( const char * s ) { return s && isOperator( *s ); }
        SABUTILS_EXPORT inline bool isOperator( const std::string::iterator & s ) { return isOperator( *s ); }
        SABUTILS_EXPORT inline bool isOperator( const std::string::const_iterator & s ) { return isOperator( *s ); }

        SABUTILS_EXPORT std::string getVAString( const char * fmt, va_list marker );
        SABUTILS_EXPORT std::string getFMTString( const char * fmt, ... );
        enum class EPadType
        {
            eLeftJustify,
            eRightJustify,
            eCenter
        };
        SABUTILS_EXPORT std::string PadString( const std::string & str, size_t max, EPadType padType = EPadType::eCenter, char padChar = ' ' );
        SABUTILS_EXPORT QString PadString( const QString & str, size_t max, EPadType padType = EPadType::eCenter, char padChar = ' ' );
        SABUTILS_EXPORT void stripLF( char * line );
        SABUTILS_EXPORT void stripLF( std::string & line );
        SABUTILS_EXPORT void strip( std::string & inStr, char value );
        SABUTILS_EXPORT std::string stripInline( const std::string & inStr, char value );

        SABUTILS_EXPORT bool getOnOffValue( const char * value, bool & aOK, bool defaultVal = true );
        SABUTILS_EXPORT bool getOnOffValue( const std::string & str, bool & aOK, bool defaultVal = true );

        // get the serial number from a string with a possible prefix, e.g., getSerialNum("inv_23", "inv_") == 23.  If prefix doesn't match or error, return UNASSIGNED.
        SABUTILS_EXPORT long getSerialNum( const std::string & str, const std::string & prefix );

        SABUTILS_EXPORT void padBinary( std::string & data, size_t maxSize, bool isSigned );
        SABUTILS_EXPORT std::string binToHex( const std::string & string );
        template< typename T >
        std::string intToHex( T value, int numDigits, int * aOK = nullptr )
        {
            std::string stringRep( numDigits, '0' );
            for ( int ii = 0; ii < numDigits; ++ii )
            {
                T currBits = 0x0F & value;
                char hexVal = 0;
                if ( currBits <= 9 )
                    hexVal = currBits + '0';
                else
                    hexVal = currBits - 10 + 'A';

                stringRep[ numDigits - ii - 1 ] = hexVal;
                value = value >> 4;
            }
            stringRep = "0x" + stringRep;
            if ( aOK )
                *aOK = ( value == 0 );

            return stringRep;
        }

        template< typename T >
        std::string intToHex( T value )
        {
            auto numDigits = 2 * sizeof( value );
            return intToHex( value, numDigits );
        }

        // if any char is NOT a 0 or 1 returns false
        // if there are more than 64 bits, return false since it can not be reproduced as a integral value in C++
        // does NOT support 0000_1111_0000
        // must be all 0s or 1s to be numeric
        SABUTILS_EXPORT bool isNumericString( const std::string & constString, uint64_t & val, unsigned int & numBits );

        SABUTILS_EXPORT std::string stripBlanksHead( const std::string & inStr );
        SABUTILS_EXPORT std::string stripBlanksTail( const std::string & inStr );
        SABUTILS_EXPORT std::string stripBlanks( const std::string & inStr );

        SABUTILS_EXPORT std::string stripQuotes( const std::string & text, const char * quotes = "\"\'" );
        SABUTILS_EXPORT QString stripQuotes( const QString & text, const char * quotes = "\"\'" );
        SABUTILS_EXPORT std::string stripQuotes( const char * text, const char * quotes = "\"\'" );

        SABUTILS_EXPORT std::string stripQuotes( const std::string & text, char quote );
        SABUTILS_EXPORT QString stripQuotes( const QString & text, char quote );
        SABUTILS_EXPORT std::string stripQuotes( const char * text, char quote );

        SABUTILS_EXPORT bool isQuoted( const std::string & text, const char * quotes = "\"\'" );
        SABUTILS_EXPORT bool isQuoted( const QString & text, const char * quotes = "\"\'" );
        SABUTILS_EXPORT bool isQuoted( const char * text, const char * quotes = "\"\'" );

        SABUTILS_EXPORT bool isQuoted( const std::string & text, char quote );
        SABUTILS_EXPORT bool isQuoted( const QString & text, char quote );
        SABUTILS_EXPORT bool isQuoted( const char * text, char quote );

        SABUTILS_EXPORT void stripBlanksInline( std::string & inStr );
        SABUTILS_EXPORT std::string stripAllBlanksAndQuotes( const std::string & text );

        SABUTILS_EXPORT bool isLowerCaseString( const std::string & text );
        SABUTILS_EXPORT bool hasLowerCaseChars( const std::string & text );

        SABUTILS_EXPORT bool containsSubString( const std::string & str, const std::string & substr, std::string::size_type * ind = nullptr );
        SABUTILS_EXPORT bool hasPrefixSubString( const std::string & str, const std::string & prefix );
        SABUTILS_EXPORT bool hasSuffixSubString( const std::string & str, const std::string & suffix );

        SABUTILS_EXPORT std::string replaceAllNot( const std::string & inString, const std::string & notOf, char to );

        SABUTILS_EXPORT void replaceAll( char * str, char from, char to );

        SABUTILS_EXPORT std::string replaceAll( std::string & str, char from, char to );
        SABUTILS_EXPORT std::string replaceAll( std::string & str, const std::string & from, const std::string & to );
        SABUTILS_EXPORT std::string replaceAll( std::string & str, const std::string & from, char to );
        SABUTILS_EXPORT std::string replaceAll( std::string & str, char from, const std::string & to );

        SABUTILS_EXPORT std::string replaceAll( const std::string & str, char from, char to );
        SABUTILS_EXPORT std::string replaceAll( const std::string & str, const std::string & from, const std::string & to );
        SABUTILS_EXPORT std::string replaceAll( const std::string & str, const std::string & from, char to );
        SABUTILS_EXPORT std::string replaceAll( const std::string & str, char from, const std::string & to );

        SABUTILS_EXPORT std::string expandEnvVariable( const std::string & string, std::string * msg = nullptr, bool * aOK = nullptr );

        SABUTILS_EXPORT int count_identifiers( const char * stmt );
        SABUTILS_EXPORT bool get_string_token( FILE * fp, char * s );
        SABUTILS_EXPORT bool has_suffix( const char * str, const char * suffix );
        SABUTILS_EXPORT bool has_suffix( const std::string & str, const char * suffix );

        SABUTILS_EXPORT bool hasWildCardSuffixSubString( const std::string & str );
        SABUTILS_EXPORT bool containsWildCardCharacters( const std::string & str );

        SABUTILS_EXPORT std::string stripHead( const std::string & head, const std::string & name, bool * found = nullptr );
        SABUTILS_EXPORT std::list< std::string >      splitString( const std::string & string, char delim, bool skipEmpty = false, bool keepQuoted = false, bool stripQuotes = false ); // split based on char
        SABUTILS_EXPORT std::list< std::string >      splitString( const std::string & string, const std::string & oneOfdelim, bool skipEmpty = false, bool keepQuoted = false, bool stripQuotes = false ); // split based on one char of

        SABUTILS_EXPORT std::list< std::string > stripParen( const std::list< std::string > & list );

        SABUTILS_EXPORT std::string joinString( const std::set< std::string, noCaseStringCmp > & list, const std::string & delim, bool condenseBlanks = false );
        SABUTILS_EXPORT std::string joinString( const std::set< std::string, noCaseStringCmp > & list, char delim, bool condenseBlanks = false );
        SABUTILS_EXPORT std::string joinString( const std::set< std::string > & list, const std::string & delim, bool condenseBlanks = false );
        SABUTILS_EXPORT std::string joinString( const std::set< std::string > & list, char delim, bool condenseBlanks = false );
        SABUTILS_EXPORT std::string joinString( const std::list< std::string > & list, const std::string & delim, bool condenseBlanks = false );
        SABUTILS_EXPORT std::string joinString( const std::pair< std::string, std::string > & list, const std::string & delim, bool condenseBlanks = false );
        SABUTILS_EXPORT std::string joinString( const std::list< std::string > & list, char delim, bool condenseBlanks = false );
        SABUTILS_EXPORT std::string joinString( const std::vector< std::string > & list, const std::string & delim, bool condenseBlanks = false );
        SABUTILS_EXPORT std::string joinString( const std::vector< std::string > & list, char delim, bool condenseBlanks = false );

        SABUTILS_EXPORT std::string joinString( const char * lhs, const char * rhs, char delim );
        SABUTILS_EXPORT std::string joinString( const char * lhs, const char * rhs, const char * delim );
        SABUTILS_EXPORT std::string joinString( const std::string & lhs, const std::string & rhs, char delim );
        SABUTILS_EXPORT std::string joinString( const std::string & lhs, const std::string & rhs, const char * delim );
        SABUTILS_EXPORT std::string joinString( const std::string & lhs, const std::string & rhs, const std::string & delim );

        SABUTILS_EXPORT std::string writeEscaped( const std::string & s, bool escapeWhitespace = false );
        SABUTILS_EXPORT std::string writeQuotedStringForXml( const std::string & name );

        template< typename T >
        std::string toBinString( T val, size_t len = -1 )
        {
            if ( len == -1 )
                len = sizeof( val ) * CHAR_BIT;
            std::string retVal;
            retVal.reserve( len );

            for ( size_t ii = 0; ii < len; ++ii, val >>= 1 )
            {
                retVal.insert( retVal.begin(), ( val & 1 ) ? '1' : '0' );
            }
            return retVal;
        }

        SABUTILS_EXPORT std::string hexToBin( const std::string & in, bool * aOK = nullptr );
        SABUTILS_EXPORT std::string decToBin( const std::string & in, bool * aOK = nullptr );
        SABUTILS_EXPORT std::string octToBin( const std::string & in, bool * aOK = nullptr );
        SABUTILS_EXPORT std::string hexToBin( const std::string & in, size_t len, bool * aOK = nullptr );
        SABUTILS_EXPORT std::string decToBin( const std::string & in, size_t len, bool * aOK = nullptr );
        SABUTILS_EXPORT std::string octToBin( const std::string & in, size_t len, bool * aOK = nullptr );
        SABUTILS_EXPORT std::string AsciiToBin( const std::string & in );

        SABUTILS_EXPORT int hexToInt( const char * id, bool * aOK = nullptr );
        template< typename T >
        bool hexToInt( T & retVal, const std::string & value )
        {
            retVal = 0;
            size_t begin = 0;
            while ( begin < value.length() && ( ( value[ begin ] == ' ' ) || ( value[ begin ] == '\t' ) ) )
                begin++;

            if ( begin == value.length() || ( value[ begin ] != '0' ) )
                return false;
            ++begin;

            if ( begin == value.length() || ( ( value[ begin ] != 'x' ) && ( value[ begin ] != 'X' ) ) )
                return false;
            ++begin;

            size_t end = value.length() - 1;
            while ( end >= begin && ( ( value[ end ] == ' ' ) || ( value[ end ] == '\t' ) || ( value[ end ] == ';' ) ) )
            {
                if ( end == 0 )
                    break;
                end--;
            }

            int index = 0;
            while ( end >= begin )
            {
                char ch = ::tolower( value[ end ] );
                T currVal = 0;
                if ( ch >= '0' && ch <= '9' )
                    currVal = ch - '0';
                else if ( ch >= 'a' && ch <= 'f' )
                    currVal = ch - 'a' + 10;
                else if ( ( ch != ' ' ) && ( ch != '\t' ) && ( ch != ';' ) )
                {
                    retVal = 0;
                    return false;
                }
                currVal = currVal << ( index * 4 );
                retVal |= currVal;

                if ( end == 0 )
                    break;
                end--;
                if ( ( ch != ' ' ) && ( ch != '\t' ) && ( ch != ';' ) )
                    index++;
            }

            return true;
        }

        SABUTILS_EXPORT bool stringCompare( const std::string & s1, const std::string & s2, bool caseInsensitive = true );

        SABUTILS_EXPORT int strNCaseCmp( const char * s1, const char * s2, size_t n );
        SABUTILS_EXPORT int strNCaseCmp( const std::string & s1, const char * s2, size_t n );
        SABUTILS_EXPORT int strNCaseCmp( const char * s1, const std::string & s2, size_t n );
        SABUTILS_EXPORT int strNCaseCmp( const std::string & s1, const std::string & s2, size_t n );
        SABUTILS_EXPORT int strCaseCmp( const char * s1, const char * s2 );
        SABUTILS_EXPORT int strCaseCmp( const std::string & s1, const char * s2 );
        SABUTILS_EXPORT int strCaseCmp( const char * s1, const std::string & s2 );
        SABUTILS_EXPORT int strCaseCmp( const std::string & s1, const std::string & s2 );
        SABUTILS_EXPORT std::string::size_type strCaseFind( const std::string & s1, const std::string & substr );
        SABUTILS_EXPORT bool strCaseSuffix( const std::string & s1, const std::string & substr );

        SABUTILS_EXPORT bool strEqual( const char * s1, const char * s2, bool caseInsensitive = false );
        SABUTILS_EXPORT bool strEqual( const std::string & s1, const char * s2, bool caseInsensitive = false );
        SABUTILS_EXPORT bool strEqual( const char * s1, const std::string & s2, bool caseInsensitive = false );
        SABUTILS_EXPORT bool strEqual( const std::string & s1, const std::string & s2, bool caseInsensitive = false );
        SABUTILS_EXPORT bool strNEqual( const char * s1, const char * s2, size_t len, bool caseInsensitive = false );
        SABUTILS_EXPORT bool strNEqual( const std::string & s1, const char * s2, size_t len, bool caseInsensitive = false );
        SABUTILS_EXPORT bool strNEqual( const char * s1, const std::string & s2, size_t len, bool caseInsensitive = false );
        SABUTILS_EXPORT bool strNEqual( const std::string & s1, const std::string & s2, size_t len, bool caseInsensitive = false );

        SABUTILS_EXPORT char * get_identifier_from_string( const char * string, char * id );
        SABUTILS_EXPORT std::string get_identifier_from_string_std( const std::string & string, std::string & id );
        SABUTILS_EXPORT std::string strip_terminal( const std::string & token, const std::string & term );

        SABUTILS_EXPORT char * convert_to_lower_case( char * string );
        SABUTILS_EXPORT std::string tolower( std::string s ); // use copy semantic
        SABUTILS_EXPORT std::string toupper( std::string s );

        template< typename T >
        void dumpList( const std::vector< T > & aList )
        {
            typedef typename std::vector< T >::const_iterator Iter;

            for ( Iter ii = aList.begin(); ii != aList.end(); ++ii )
            {
                std::cout << "  " << *ii << std::endl;
            }
        }

        template< typename T >
        void dumpList( const std::list< T > & aList )
        {
            typedef typename std::list< T >::const_iterator Iter;

            for ( Iter ii = aList.begin(); ii != aList.end(); ++ii )
            {
                std::cout << "  " << *ii << std::endl;
            }
        }
        SABUTILS_EXPORT bool matchKeyWord( const std::string & line, const std::string & key );

        SABUTILS_EXPORT std::string presentationFormat( const std::string & format );

        SABUTILS_EXPORT bool is_number( const std::string & str );

        SABUTILS_EXPORT std::string left( std::string inString, size_t len );
        SABUTILS_EXPORT std::string right( std::string inString, size_t len );
        SABUTILS_EXPORT std::string stripHierName( std::string objectName, const std::string & hierSep, bool stripArrayInfo );

        SABUTILS_EXPORT std::string binaryAttrToASCII( const std::string & bString );
        SABUTILS_EXPORT std::string binaryToASCII( const std::string & bString, bool & aOK );

        SABUTILS_EXPORT std::list< std::string > splitSDCPattern( const std::string & pattern, bool regExp, char hsc, bool & aOK, std::string * msg );
        SABUTILS_EXPORT QStringList splitSDCPattern( const QString & pattern, bool regexp, char hsc, bool & aOK, QString * msg );

        SABUTILS_EXPORT std::list< std::string > splitSDCPattern( const std::string & pattern, bool regExp, const char * hsc, bool & aOK, std::string * msg );
        SABUTILS_EXPORT QStringList splitSDCPattern( const QString & pattern, bool regexp, const char * hsc, bool & aOK, QString * msg );

        SABUTILS_EXPORT char IsSwitch( const char * str );
        SABUTILS_EXPORT char IsSwitch( const std::string & str );

        SABUTILS_EXPORT bool isSeparatorEscaped( const std::string & name, char secondSep = 0 );
        SABUTILS_EXPORT std::string separatorEscape( const std::string & name, char secondSep = 0 );
        SABUTILS_EXPORT size_t findLastSeparator( const std::string & str, char separator, size_t offset = std::string::npos );
        SABUTILS_EXPORT size_t findSeparator( const std::string & str, char separator, size_t offset = std::string::npos );

        template< typename T >
        T binToDec( const std::string & inStr, bool isSigned, bool * aOK = nullptr )
        {
            if ( aOK )
                *aOK = false;
            std::string string = stripInline( inStr, '_' );

            auto maxLen = sizeof( T ) * CHAR_BIT;

            if ( string.size() > maxLen )
            {
                return 0;
            }

            if ( aOK )
                *aOK = true;

            if ( inStr.empty() )
                return 0;


            bool negate = false;
            if ( isSigned && string[ 0 ] == '1' )
            {
                negate = true;
            }

            T retVal = 0;
            for ( auto && ii : string )
            {
                unsigned val = ( ii - '0' );
                val = negate ? ( val == 0 ? 1 : 0 ) : val;
                retVal = ( retVal << 1 ) | val;
            }
            if ( negate )
            {
                retVal += 1;
                retVal *= -1;
            }
            return retVal;
        }


        template <class T>
        std::vector<bool> toBoolVector( T val, int len = -1 )
        {
            if ( len == -1 )
                len = sizeof( val );
            std::vector<bool> retVal;
            retVal.reserve( len );

            for ( int ii = 0; ii < len; ++ii, val >>= 1 )
            {
                bool value = ( val & 1 ) ? true : false;

                retVal.insert( retVal.begin(), value );
            }
            return retVal;
        }

        SABUTILS_EXPORT QStringList asReport( const QStringList & header, const QStringList & subHeader, const QList< QStringList > & data, bool sortData );

        SABUTILS_EXPORT bool validateBase64String( const char * str, size_t len = std::string::npos );
        SABUTILS_EXPORT bool validateBase64String( const std::string & str );
        SABUTILS_EXPORT bool validateUUEncodeString( const char * str, size_t len = std::string::npos );
        SABUTILS_EXPORT bool validateUUEncodeString( const std::string & str );
        SABUTILS_EXPORT bool validateQuotedPrintableString( const char * str, size_t len = std::string::npos );
        SABUTILS_EXPORT bool validateQuotedPrintableString( const std::string & str );

        SABUTILS_EXPORT int romanToDecimal( QString string, bool & aOK ); // only valid for roman numbers to 3999 as 4000 requires a vinculum
        SABUTILS_EXPORT bool isRomanNumeral( const QString & string );

        SABUTILS_EXPORT QString transformTitle( const QString & title, bool ignoreAllCase = true );
        SABUTILS_EXPORT QString titleCase( const QString & string, bool ignoreAllCase = true );
        SABUTILS_EXPORT const std::unordered_set< QString > & unimportantWords();

        SABUTILS_EXPORT std::unordered_set< QString > getImportantWords( const QString & string, bool stripPunctuation );
        SABUTILS_EXPORT bool isSimilar( const QString & lhs, const QString & rhs, bool inorder ); // is every important word in the rhs in the left

        template< typename T >
        T NODISCARD rtrim( T string )
        {
            string.erase( std::find_if( string.rbegin(), string.rend(), []( unsigned char ch )
                          {
                              return !std::isspace( ch );
                          } ).base(), string.end() );
            return string;
        }

        template< typename T >
        T NODISCARD ltrim( T string )
        {
            string.erase( string.begin(), std::find_if( string.begin(), string.end(), []( unsigned char ch )
                          {
                              return !std::isspace( ch );
                          } ) );
            return string;
        }

        template< typename T >
        T NODISCARD trim( T string )
        {
            auto retVal = rtrim( string );
            retVal = ltrim( retVal );
            return retVal;
        }


        SABUTILS_EXPORT double cleanPercentage( double in );

        SABUTILS_EXPORT std::string getPercentageAsString( double value );

        SABUTILS_EXPORT QString removeDiacriticalCharacters( const QString & str );
        SABUTILS_EXPORT bool isDiacriticalCharacter( const QChar & ch, QString * ascii = nullptr );

        SABUTILS_EXPORT bool startsOrEndsWithNumber( const QString & string, QString * number = nullptr, QString * extra = nullptr, bool * numIsPrefix = nullptr ); // number_extra or extra_number, prefix = true means the number is a prefix

        SABUTILS_EXPORT bool isValidEmailAddress( const QString & email );
    }
}
#endif 


