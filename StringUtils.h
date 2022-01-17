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
#include <QString>

#include "EnumUtils.h"
#include "StringComparisonClasses.h"

namespace NSABUtils
{
    namespace NStringUtils
    {
#if (QT_VERSION < QT_VERSION_CHECK( 5, 15, 0 ))
#define TSkipEmptyParts QString::SkipEmptyParts
#define TKeepEmptyParts QString::KeepEmptyParts
#else
#define TSkipEmptyParts Qt::SkipEmptyParts
#define TKeepEmptyParts Qt::KeepEmptyParts
#endif

        static const std::size_t sMAXLINE{ 2048 };
        inline bool isWhiteSpace(char ch) { return (ch == ' ') || (ch == '\t'); }
        inline bool isWhiteSpace(const char * s) { return s && isWhiteSpace(*s); }
        inline bool isWhiteSpace(const std::string::iterator & s) { return isWhiteSpace(*s); }
        inline bool isWhiteSpace(const std::string::const_iterator & s) { return isWhiteSpace(*s); }
        inline bool isWhiteSpaceN(char ch) { return isWhiteSpace(ch) || (ch == '\n'); }
        inline bool isWhiteSpaceN(const char * s) { return s && isWhiteSpaceN(*s); }
        inline bool isWhiteSpaceN(const std::string::iterator & s) { return isWhiteSpaceN(*s); }
        inline bool isWhiteSpaceN(const std::string::const_iterator & s) { return isWhiteSpaceN(*s); }
        inline bool isOperator(char ch) { return (ch == '=') || (ch == '!') || (ch == '>') || (ch == '<'); }
        inline bool isOperator(const char * s) { return s && isOperator(*s); }
        inline bool isOperator(const std::string::iterator & s) { return isOperator(*s); }
        inline bool isOperator(const std::string::const_iterator & s) { return isOperator(*s); }

        bool regExEqual(const std::string & lhs, const std::string & rhs);

        bool isExactMatchRegEx(const std::string & data, const std::string & pattern, bool nocase);

        std::string getVAString(const char * fmt, va_list marker);
        std::string getFMTString(const char * fmt, ...);
        enum class EPadType
        {
            eLeftJustify,
            eRightJustify,
            eCenter
        };
        std::string PadString(const std::string & str, size_t max, EPadType padType = EPadType::eCenter, char padChar = ' ');
        QString PadString(const QString & str, size_t max, EPadType padType = EPadType::eCenter, char padChar = ' ');
        void stripLF(char * line);
        void stripLF(std::string & line);
        void strip(std::string & inStr, char value);
        std::string stripInline(const std::string & inStr, char value);

        bool getOnOffValue(const char * value, bool & aOK, bool defaultVal = true);
        bool getOnOffValue(const std::string & str, bool & aOK, bool defaultVal = true);

        // get the serial number from a string with a possible prefix, e.g., getSerialNum("inv_23", "inv_") == 23.  If prefix doesn't match or error, return UNASSIGNED.
        long getSerialNum(const std::string &str, const std::string &prefix);

        void padBinary(std::string &data, size_t maxSize, bool isSigned);
        std::string binToHex(const std::string & string);
        template< typename T >
        std::string intToHex(T value, int numDigits, int * aOK = nullptr)
        {
            std::string stringRep(numDigits, '0');
            for (int ii = 0; ii < numDigits; ++ii)
            {
                T currBits = 0x0F & value;
                char hexVal = 0;
                if (currBits <= 9)
                    hexVal = currBits + '0';
                else
                    hexVal = currBits - 10 + 'A';

                stringRep[numDigits - ii - 1] = hexVal;
                value = value >> 4;
            }
            stringRep = "0x" + stringRep;
            if (aOK)
                *aOK = (value == 0);

            return stringRep;
        }

        template< typename T >
        std::string intToHex(T value)
        {
            auto numDigits = 2 * sizeof(value);
            return intToHex(value, numDigits);
        }

        // if any char is NOT a 0 or 1 returns false
        // if there are more than 64 bits, return false since it can not be reproduced as a integral value in C++
        // does NOT support 0000_1111_0000
        // must be all 0s or 1s to be numeric
        bool isNumericString(const std::string & constString, uint64_t & val, unsigned int & numBits);

        std::string stripBlanksHead(const std::string & inStr);
        std::string stripBlanksTail(const std::string & inStr);
        std::string stripBlanks(const std::string & inStr);

        std::string stripQuotes(const std::string & text, const char * quotes = "\"\'");
        QString stripQuotes(const QString & text, const char * quotes = "\"\'");
        std::string stripQuotes(const char * text, const char * quotes = "\"\'");

        std::string stripQuotes(const std::string & text, char quote);
        QString stripQuotes(const QString & text, char quote);
        std::string stripQuotes(const char * text, char quote);

        bool isQuoted(const std::string & text, const char * quotes = "\"\'");
        bool isQuoted(const QString & text, const char * quotes = "\"\'");
        bool isQuoted(const char * text, const char * quotes = "\"\'");

        bool isQuoted(const std::string & text, char quote);
        bool isQuoted(const QString & text, char quote);
        bool isQuoted(const char * text, char quote);

        void stripBlanksInline(std::string & inStr);
        std::string stripAllBlanksAndQuotes(const std::string &text);

        bool isLowerCaseString(const std::string & text);
        bool hasLowerCaseChars(const std::string & text);

        bool containsSubString(const std::string & str, const std::string & substr, std::string::size_type *ind = nullptr);
        bool hasPrefixSubString(const std::string & str, const std::string & prefix);
        bool hasSuffixSubString(const std::string & str, const std::string & suffix);

        std::string replaceAllNot(const std::string & inString, const std::string & notOf, char to);

        void replaceAll(char * str, char from, char to);

        std::string replaceAll(std::string& str, char from, char to);
        std::string replaceAll(std::string& str, const std::string& from, const std::string& to);
        std::string replaceAll(std::string& str, const std::string& from, char to);
        std::string replaceAll(std::string& str, char from, const std::string& to);

        std::string replaceAll(const std::string& str, char from, char to);
        std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);
        std::string replaceAll(const std::string& str, const std::string& from, char to);
        std::string replaceAll(const std::string& str, char from, const std::string& to);

        std::string expandEnvVariable(const std::string & string, std::string * msg = nullptr, bool * aOK = nullptr);

        int count_identifiers(const char *stmt);
        bool get_string_token(FILE *fp, char *s);
        bool has_suffix(const char *str, const char *suffix);
        bool has_suffix(const std::string & str, const char *suffix);

        bool hasWildCardSuffixSubString(const std::string & str);
        bool containsWildCardCharacters(const std::string & str);

        std::string stripHead(const std::string & head, const std::string & name, bool * found = nullptr);
        std::list< std::string >      splitString(const std::string & string, char delim, bool skipEmpty = false, bool keepQuoted = false, bool stripQuotes = false); // split based on char
        std::list< std::string >      splitString(const std::string & string, const std::string & oneOfdelim, bool skipEmpty = false, bool keepQuoted = false, bool stripQuotes = false); // split based on one char of
        std::list< std::string > splitStringRegEx(const std::string & string, const std::string & regex, bool nocase = false, bool skipEmpty = false); // split based on regex

        std::string joinString(const std::set< std::string, noCaseStringCmp > & list, const std::string & delim, bool condenseBlanks = false);
        std::string joinString(const std::set< std::string, noCaseStringCmp > & list, char delim, bool condenseBlanks = false);
        std::string joinString(const std::set< std::string > & list, const std::string & delim, bool condenseBlanks = false);
        std::string joinString(const std::set< std::string > & list, char delim, bool condenseBlanks = false);
        std::string joinString(const std::list< std::string > & list, const std::string & delim, bool condenseBlanks = false);
        std::string joinString(const std::pair< std::string, std::string > & list, const std::string & delim, bool condenseBlanks = false);
        std::string joinString(const std::list< std::string > & list, char delim, bool condenseBlanks = false);
        std::string joinString(const std::vector< std::string > & list, const std::string & delim, bool condenseBlanks = false);
        std::string joinString(const std::vector< std::string > & list, char delim, bool condenseBlanks = false);

        std::string joinString(const char * lhs, const char * rhs, char delim);
        std::string joinString(const char * lhs, const char * rhs, const char * delim);
        std::string joinString(const std::string & lhs, const std::string & rhs, char delim);
        std::string joinString(const std::string & lhs, const std::string & rhs, const char * delim);
        std::string joinString(const std::string & lhs, const std::string & rhs, const std::string & delim);

        std::string writeEscaped(const std::string & s, bool escapeWhitespace = false);
        std::string writeQuotedStringForXml(const std::string & name);

        template< typename T >
        std::string toBinString(T val, size_t len = -1)
        {
            if (len == -1)
                len = sizeof(val) * CHAR_BIT;
            std::string retVal;
            retVal.reserve(len);

            for (size_t ii = 0; ii < len; ++ii, val >>= 1)
            {
                retVal.insert(retVal.begin(), (val & 1) ? '1' : '0');
            }
            return retVal;
        }

        std::string hexToBin(const std::string & in, bool * aOK = nullptr);
        std::string decToBin(const std::string & in, bool * aOK = nullptr);
        std::string octToBin(const std::string & in, bool * aOK = nullptr);
        std::string hexToBin(const std::string & in, size_t len, bool * aOK = nullptr);
        std::string decToBin(const std::string & in, size_t len, bool * aOK = nullptr);
        std::string octToBin(const std::string & in, size_t len, bool * aOK = nullptr);
        std::string AsciiToBin(const std::string & in);

        int hexToInt(const char *id, bool * aOK = nullptr);
        template< typename T >
        bool hexToInt(T & retVal, const std::string & value)
        {
            retVal = 0;
            size_t begin = 0;
            while (begin < value.length() && ((value[begin] == ' ') || (value[begin] == '\t')))
                begin++;

            if (begin == value.length() || (value[begin] != '0'))
                return false;
            ++begin;

            if (begin == value.length() || ((value[begin] != 'x') && (value[begin] != 'X')))
                return false;
            ++begin;

            size_t end = value.length() - 1;
            while (end >= begin && ((value[end] == ' ') || (value[end] == '\t') || (value[end] == ';')))
            {
                if (end == 0)
                    break;
                end--;
            }

            int index = 0;
            while (end >= begin)
            {
                char ch = ::tolower(value[end]);
                T currVal = 0;
                if (ch >= '0' && ch <= '9')
                    currVal = ch - '0';
                else if (ch >= 'a' && ch <= 'f')
                    currVal = ch - 'a' + 10;
                else if ((ch != ' ') && (ch != '\t') && (ch != ';'))
                {
                    retVal = 0;
                    return false;
                }
                currVal = currVal << (index * 4);
                retVal |= currVal;

                if (end == 0)
                    break;
                end--;
                if ((ch != ' ') && (ch != '\t') && (ch != ';'))
                    index++;
            }

            return true;
        }
        bool  matchRegExpr(const char* s1, const char *s2);

        bool stringCompare(const std::string & s1, const std::string & s2, bool caseInsensitive = true);

        int strNCaseCmp(const char* s1, const char* s2, size_t n);
        int strNCaseCmp(const std::string & s1, const char* s2, size_t n);
        int strNCaseCmp(const char* s1, const std::string & s2, size_t n);
        int strNCaseCmp(const std::string & s1, const std::string & s2, size_t n);
        int strCaseCmp(const char* s1, const char* s2);
        int strCaseCmp(const std::string & s1, const char* s2);
        int strCaseCmp(const char* s1, const std::string & s2);
        int strCaseCmp(const std::string & s1, const std::string & s2);
        std::string::size_type strCaseFind(const std::string & s1, const std::string & substr);
        bool strCaseSuffix(const std::string & s1, const std::string & substr);

        bool strEqual(const char* s1, const char* s2, bool caseInsensitive = false);
        bool strEqual(const std::string & s1, const char* s2, bool caseInsensitive = false);
        bool strEqual(const char* s1, const std::string & s2, bool caseInsensitive = false);
        bool strEqual(const std::string & s1, const std::string & s2, bool caseInsensitive = false);
        bool strNEqual(const char* s1, const char* s2, size_t len, bool caseInsensitive = false);
        bool strNEqual(const std::string & s1, const char* s2, size_t len, bool caseInsensitive = false);
        bool strNEqual(const char* s1, const std::string & s2, size_t len, bool caseInsensitive = false);
        bool strNEqual(const std::string & s1, const std::string & s2, size_t len, bool caseInsensitive = false);

        char * get_identifier_from_string(const char *string, char *id);
        std::string get_identifier_from_string_std(const std::string & string, std::string & id);
        std::string strip_terminal(const std::string & token, const std::string & term);

        char * convert_to_lower_case(char * string);
        std::string tolower(std::string s); // use copy semantic
        std::string toupper(std::string s);

        template< typename T >
        void dumpList(const std::vector< T > & aList)
        {
            typedef typename std::vector< T >::const_iterator Iter;

            for (Iter ii = aList.begin(); ii != aList.end(); ++ii)
            {
                std::cout << "  " << *ii << std::endl;
            }
        }

        template< typename T >
        void dumpList(const std::list< T > & aList)
        {
            typedef typename std::list< T >::const_iterator Iter;

            for (Iter ii = aList.begin(); ii != aList.end(); ++ii)
            {
                std::cout << "  " << *ii << std::endl;
            }
        }
        bool matchKeyWord(const std::string & line, const std::string & key);

        std::string presentationFormat(const std::string & format);

        bool is_number(const std::string & str);

        QString encodeRegEx(const char * inString);
        QString encodeRegEx(QString inString);
        std::string encodeRegEx(const std::string & inString);
        std::string addToRegEx(std::string oldRegEx, const std::string & regEx);

        std::string left(std::string inString, size_t len);
        std::string right(std::string inString, size_t len);
        std::string stripHierName(std::string objectName, const std::string & hierSep, bool stripArrayInfo);

        std::string binaryAttrToASCII(const std::string & bString);
        std::string binaryToASCII(const std::string & bString, bool & aOK);

        std::list< std::string > splitSDCPattern(const std::string & pattern, bool regExp, char hsc, bool & aOK, std::string * msg);
        QStringList splitSDCPattern(const QString & pattern, bool regexp, char hsc, bool & aOK, QString * msg);

        std::list< std::string > splitSDCPattern(const std::string & pattern, bool regExp, const char * hsc, bool & aOK, std::string * msg);
        QStringList splitSDCPattern(const QString & pattern, bool regexp, const char * hsc, bool & aOK, QString * msg);

        char IsSwitch(const char * str);
        char IsSwitch(const std::string & str);

        bool isSeparatorEscaped(const std::string & name, char secondSep = 0);
        std::string separatorEscape(const std::string & name, char secondSep = 0);
        size_t findLastSeparator(const std::string & str, char separator, size_t offset = std::string::npos);
        size_t findSeparator(const std::string & str, char separator, size_t offset = std::string::npos);

        template< typename T >
        T binToDec(const std::string & inStr, bool isSigned, bool * aOK = nullptr)
        {
            if (aOK)
                *aOK = false;
            std::string string = stripInline(inStr, '_');

            auto maxLen = sizeof(T) * CHAR_BIT;

            if (string.size() > maxLen)
            {
                return 0;
            }

            if (aOK)
                *aOK = true;

            if (inStr.empty())
                return 0;


            bool negate = false;
            if (isSigned && string[0] == '1')
            {
                negate = true;
            }

            T retVal = 0;
            for (size_t ii = 0; ii < string.length(); ++ii)
            {
                unsigned val = (string[ii] - '0');
                val = negate ? (val == 0 ? 1 : 0) : val;
                retVal = (retVal << 1) | val;
            }
            if (negate)
            {
                retVal += 1;
                retVal *= -1;
            }
            return retVal;
        }


        template <class T>
        std::vector<bool> toBoolVector(T val, int len = -1)
        {
            if (len == -1)
                len = sizeof(val);
            std::vector<bool> retVal;
            retVal.reserve(len);

            for (int ii = 0; ii < len; ++ii, val >>= 1)
            {
                bool value = (val & 1) ? true : false;

                retVal.insert(retVal.begin(), value);
            }
            return retVal;
        }

        QStringList asReport(const QStringList & header, const QStringList & subHeader, const QList< QStringList > & data, bool sortData);

        bool validateBase64String(const char * str, size_t len = std::string::npos);
        bool validateBase64String(const std::string & str);
        bool validateUUEncodeString(const char * str, size_t len = std::string::npos);
        bool validateUUEncodeString(const std::string & str);
        bool validateQuotedPrintableString(const char * str, size_t len = std::string::npos);
        bool validateQuotedPrintableString(const std::string & str);

        bool isSpecialRegExChar(char ch, bool includeDotSlash = true);
        bool isSpecialRegExChar(const QChar & ch, bool includeDotSlash = true);

        int romanToDecimal( QString string, bool & aOK ); // only valid for roman numbers to 3999 as 4000 requires a vinculum
        bool isRomanNumeral( const QString & string );
            
        QString transformTitle( const QString & title, bool ignoreAllCase = true );
        QString titleCase(const QString &string, bool ignoreAllCase = true);
        const std::unordered_set< QString > &unimportantWords();

        std::unordered_set< QString > getImportantWords(const QString &string, bool stripPunctuation);
        bool isSimilar(const QString &lhs, const QString &rhs, bool inorder); // is every important word in the rhs in the left
    }
}
#endif 


