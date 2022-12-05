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

#include "RegExUtils.h"
#include "StringUtils.h"
#include <QRegularExpression>

namespace NSABUtils
{
    namespace NStringUtils
    {
        bool regExEqual( const std::string & lhs, const std::string & rhs )
        {
            if ( lhs == rhs )
                return true;

            auto lhsSplit = stripParen( splitString( lhs, "|" ) );
            auto lhsExps = std::set< std::string >( lhsSplit.begin(), lhsSplit.end() );

            auto rhsSplit = stripParen( splitString( rhs, "|" ) );
            auto rhsExps = std::set< std::string >( rhsSplit.begin(), rhsSplit.end() );

            return lhsExps == rhsExps;
        }


        bool isExactMatchRegEx( const std::string & data, const std::string & pattern, bool nocase )
        {
            std::string regEx = "\\A(" + pattern + ")\\z";
            QRegularExpression regExp( QString::fromStdString( regEx ) );
            if ( !regExp.isValid() )
                return false;
            if ( nocase )
                regExp.setPatternOptions( QRegularExpression::CaseInsensitiveOption );
            auto match = regExp.match( QString::fromStdString( data ), 0, QRegularExpression::PartialPreferCompleteMatch );
            return match.hasMatch();
        }

        std::list< std::string > splitStringRegEx( const std::string & string, const std::string & pattern, bool nocase, bool skipEmpty )
        {
            QRegularExpression regExp( QString::fromStdString( pattern ) );
            if ( nocase )
                regExp.setPatternOptions( QRegularExpression::CaseInsensitiveOption );

            Q_ASSERT( regExp.isValid() );
            if ( !regExp.isValid() )
            {
                QString error = regExp.errorString();
                int offset = regExp.patternErrorOffset();
                (void)error;
                (void)offset;
                return{};
            }

            QStringList tmp = QString::fromStdString( string ).split( regExp, skipEmpty ? TSkipEmptyParts : TKeepEmptyParts );
            std::list< std::string > retVal;
            for ( auto & ii : tmp )
            {
                retVal.push_back( ii.toStdString() );
            }
            return retVal;
        }

        bool  matchRegExpr( const char * s1, const char * s2 )
        {
            const char * last_star = 0;
            while ( *s1 && *s2 )
                if ( *s1 == *s2 || *s2 == '?' )
                {
                    s1++;
                    s2++;
                }
                else if ( *s2 == '*' )
                {
                    if ( !s2[ 1 ] ) return true;
                    while ( *s1 && *s1 != s2[ 1 ] )
                        ++s1;
                    last_star = s2;
                    ++s2;
                }
                else if ( last_star )
                {
                    s2 = last_star; // revert back to the last '*' in s2
                    last_star = 0;
                }
                else break;

            if ( !*s1 )
            {
                return ( !*s2 || !strcmp( s2, "*" ) || !strcmp( s2, "?" ) ) ? true : false;
            }
            else return ( !strcmp( s2, "*" ) || ( strlen( s1 ) == 1 && !strcmp( s2, "?" ) ) ) ? true : false;
        }

        QString encodeRegEx( QString retVal )
        {
            QRegularExpression regEx( "([\\^\\$\\.\\*\\+\\?\\|\\(\\)\\[\\]\\{\\}\\\\])" );
            retVal.replace( regEx, "\\\\1" );
            return retVal;
        }

        QString encodeRegEx( const char * inString )
        {
            return encodeRegEx( QString( inString ) );
        }

        std::string encodeRegEx( const std::string & inString )
        {
            return encodeRegEx( QString::fromStdString( inString ) ).toStdString();
        }

        bool isSpecialRegExChar( const QChar & ch, bool includeDotSlash )
        {
            return isSpecialRegExChar( ch.toLatin1(), includeDotSlash );
        }

        bool isSpecialRegExChar( char ch, bool includeDotSlash )
        {
            if ( ( ch == '\\' ) || ( ch == '.' ) )
                return includeDotSlash;
            return  ( ch == '[' )
                || ( ch == ']' )
                || ( ch == '^' )
                || ( ch == '$' )
                || ( ch == '|' )
                || ( ch == '?' )
                || ( ch == '*' )
                || ( ch == '+' )
                || ( ch == '(' )
                || ( ch == ')' )
                ;
        }

        std::string addToRegEx( std::string oldRegEx, const std::string & regEx )
        {
            auto subExps = splitString( oldRegEx, "|" );
            auto newSubExps = splitString( regEx, "|" );
            subExps.insert( subExps.end(), newSubExps.cbegin(), newSubExps.cend() );
            if ( subExps.size() > 1 )
            {
                for ( auto & ii : subExps )
                {
                    if ( *ii.begin() != '(' && *ii.rbegin() != ')' )
                        ii = "(" + ii + ")";
                }
            }

            auto uniqueSub = std::set< std::string >( subExps.begin(), subExps.end() );
            oldRegEx = joinString( uniqueSub, "|" );
            return oldRegEx;
        }


        std::optional< QString > replaceMatch( const QString & replacement, QRegularExpressionMatch & match )
        {
            QString retVal;
            auto len = replacement.length();
            int ii = 0;
            for ( ; ii < ( len - 1 ); ++ii )
            {
                auto curr = replacement[ ii ];
                auto next = replacement[ ii + 1 ];
                if ( curr != QChar( '$' ) )
                {
                    for ( ; ii < len && ( replacement[ ii ] != QChar( '$' ) ); ++ii )
                        retVal += replacement[ ii ];
                    ii--;
                }
                else if ( next == QChar( '$' ) )
                {
                    retVal += curr;
                    ++ii;
                }
                else if ( next == QChar( '&' ) )
                {
                    retVal += match.captured();
                    ++ii;
                }
                else if ( next.isNumber() )
                {
                    QString number;
                    for ( ; ( ii + 1 ) < len && replacement[ ii + 1 ].isNumber(); ++ii )
                    {
                        number += replacement[ ii + 1 ];
                    }

                    bool aOK;
                    auto num = number.toInt( &aOK );
                    if ( !aOK )
                        return {};
                    retVal += match.captured( num );
                }
                else if ( next == QChar( '{' ) )
                {
                    QString name;
                    ++ii;
                    for( ; ( ii + 1 ) < len && ( replacement[ ii + 1 ] != QChar( '}' ) ); ++ii )
                    {
                        name += replacement[ ii + 1 ];
                    }
                    retVal += match.captured( name );
                    ii++; // trailing }
                }
            }
            if ( ii == ( len - 1 ) )
                retVal += replacement[ len - 1 ];
            return retVal;
        }

        std::optional< QString > regExReplace( const QString & input, const QString & pattern, const QString & replacement )
        {
            return regExReplace( input, QRegularExpression( pattern ), replacement );
        }

        std::optional< QString > regExReplace( const QString & input, const QRegularExpression & regEx, const QString & replacement )
        {
            if ( !regEx.isValid() )
                return {};

            QString retVal;
            auto ii = regEx.globalMatch( input );
            int prevPos = 0;
            int capNum = 1;
            while ( ii.hasNext() )
            {
                auto match = ii.next();
                auto startPos = match.capturedStart( 0 );
                auto endPos = match.capturedEnd( 0 );
                auto len = match.capturedLength( 0 );
                auto cap = match.captured( 0 );

                //qDebug().noquote().nospace() << capNum++ << " Start: " << startPos << " End: " << endPos << " Length: " << len << " Text: '" << cap << "'";
                retVal += input.mid( prevPos, startPos - prevPos );
                auto curr = replaceMatch( replacement, match );
                if ( curr.has_value() )
                    retVal += curr.value();
                else
                    return {};
                prevPos = endPos;
            }
            return retVal;
        }
    }
}
