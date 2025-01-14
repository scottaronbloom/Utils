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

#include "FileUtils.h"
#include "MoveToTrash.h"
#include "StringUtils.h"
#include "utils.h"
#include "WindowsError.h"

#include <Qt>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDirIterator>
#include <QRegExp>
#include <QStringRef>
#include <QVector>
#include <QRegularExpression>

#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <algorithm>
#ifdef Q_OS_WINDOWS
    #include <qt_windows.h>
#else
    #include <wordexp.h>
    #include <sys/stat.h>
#endif

#include <map>
#include <cctype>
#pragma comment( lib, "version.lib" )

namespace NSABUtils
{
    namespace NFileUtils
    {
        std::string getFileName( const std::string &path )
        {
            std::string::size_type pos = path.find_last_of( "/\\" );
            if ( pos == std::string::npos )
                return path;
            return path.substr( pos + 1 );
        }

        //////////////////////////////////////////////////////////////////////////
        // extractFilePath
        //
        // Decompose a file pathname into a file pathname, the
        // base filename and the extension. dirPath will be the
        // directory path, fileName will be the filename,
        // and ext will be the file extension.
        //////////////////////////////////////////////////////////////////////////
        void extractFilePath( const std::string &pathName, std::string *dirPath, std::string *outFileName, std::string *ext )
        {
            if ( dirPath )
                *dirPath = std::string();
            if ( ext )
                *ext = std::string();
            if ( outFileName )
                *outFileName = std::string();

            std::string fileName = pathName;

            // Get last directory specifier
            auto idx = pathName.find_last_of( "/\\" );
            if ( idx != std::string::npos )
            {
                if ( dirPath )
                    *dirPath = pathName.substr( 0, idx );
                if ( outFileName )
                    *outFileName = fileName = pathName.substr( idx + 1 );
            }

            idx = fileName.rfind( "." );
            if ( idx != std::string::npos )
            {
                if ( ext )
                    *ext = fileName.substr( idx + 1 );
                if ( outFileName )
                    *outFileName = fileName = fileName.substr( 0, idx );
            }
        }

        bool extCompare( const std::string &pattern, const std::string &extension, bool wildcards )
        {
            if ( pattern.empty() || extension.empty() )
                return false;
            size_t patternStartPos = 0;
            if ( pattern[ 0 ] == '.' )
                patternStartPos = 1;

            size_t extStartPos = 0;
            if ( extension[ 0 ] == '.' )
                extStartPos = 1;

            QString qPattern = QString::fromStdString( pattern.substr( patternStartPos, pattern.length() ) );
            QString qExt = QString::fromStdString( extension.substr( extStartPos, extension.length() ) );

            if ( wildcards )
            {
                QRegExp regExp( qPattern, Qt::CaseSensitivity::CaseInsensitive, QRegExp::PatternSyntax::WildcardUnix );
                return regExp.exactMatch( qExt );
            }
            else
                return qPattern == qExt;
        }

        bool fileCompare( const std::string &pattern, const std::string &filename, bool wildcards )
        {
            if ( pattern.empty() || filename.empty() )
                return false;

            QString qPattern = QString::fromStdString( pattern );
            auto patternList = qPattern.split( QRegularExpression( "[\\\\/]" ), NStringUtils::TSkipEmptyParts );
            if ( patternList.isEmpty() )
                return false;

            QString qFileName = QString::fromStdString( filename );
            auto fileNameList = qFileName.split( QRegularExpression( "[\\\\/]" ), NStringUtils::TSkipEmptyParts );
            if ( fileNameList.isEmpty() )
                return false;

            auto currPattern = patternList.size() - 1;
            auto currFileName = fileNameList.size() - 1;

            bool matches = true;
            for ( ; matches && ( ( currPattern >= 0 ) && ( currFileName >= 0 ) ); currPattern--, currFileName-- )
            {
                QString pattern = patternList[ currPattern ];
                QString fileName = fileNameList[ currFileName ];
                if ( wildcards )
                {
                    QRegExp regExp( pattern, Qt::CaseSensitivity::CaseInsensitive, QRegExp::PatternSyntax::WildcardUnix );
                    matches = regExp.exactMatch( fileName );
                }
                else
                    matches = pattern == fileName;
            }

            return matches;
        }

        bool isRelativePath( const std::string &relPath )
        {
            if ( relPath.empty() )
                return false;
            return !isAbsPath( relPath );
        }

        bool isAbsPath( const std::string &relPath )
        {
            if ( relPath.empty() )
                return false;
            size_t lhs = 0;
            if ( *relPath.begin() == '"' && *relPath.rbegin() == '"' )
                lhs++;
            char firstChar = ::toupper( relPath[ lhs ] );
            bool isAbs = firstChar == '/' || firstChar == '\\' || ( ( relPath.length() >= ( lhs + 3 ) ) && ( firstChar >= 'A' && firstChar <= 'Z' ) && relPath[ lhs + 1 ] == ':' && ( relPath[ lhs + 2 ] == '/' || relPath[ lhs + 2 ] == '\\' ) );
            return isAbs;
        }

        // -------------------------------------------------------------------------------
        // getAbsoluteFilePath()
        //
        // If the relativeFilePath has ".." or "." wildcard characters, adapt the
        // file path to the absolute path. Return the new file path.
        //
        // Parameters:
        //      directory - an absolute directory path
        //    relativeFilePath - a relative path with ".." or "." characters.
        //
        // Return the new absolute directory file path.
        //
        // Examples:
        //        directory: /home/firstdir/secondDir    relativeFilePath: ../test.v
        //        returns:   /home/firstdir/test.v
        //
        //      directory: /home/firstdir/secondDir relativeFilePath: test.v
        //        returns: /home/firstdir/secondDir/test.v
        //
        //      directory: /home/firstdir/secondDir relativeFilePath: thirdDir/test.v
        //        returns: /home/firstdir/secondDir/thirdDir/test.v
        //
        //      directory: /home/firstdir/secondDir relativeFilePath: /thirdDir/test.v
        //        returns: /thirdDir/test.v
        //        directory: \home/firstdir\secondDir relativeFilePath: C:\thirdDir\test.v
        //        returns: C:\thirdDir\test.v
        //        In the above two cases, we treat the relativeFilePath as the absolute path.
        // -------------------------------------------------------------------------------

        std::string getAbsoluteFilePath( const std::string &relFilePath )
        {
            return getAbsoluteFilePath( getWd(), relFilePath );
        }

        std::string getRelativePath( const std::string &absPath, const std::string &dir )
        {
            std::string retVal;
            if ( !absPath.empty() )
            {
                QDir absDir = dir.empty() ? QDir::currentPath() : QString::fromStdString( dir );
                retVal = getRelativePath( absDir, QString::fromStdString( absPath ) ).toStdString();
            }
            if ( retVal.empty() )
                retVal = absPath;
            return retVal;
        }

        QString driveSpec( const QString &path )
        {
#if defined( Q_OS_WIN )
            if ( path.size() < 2 )
                return QString();
            char c = path.at( 0 ).toLatin1();
            if ( c < 'a' && c > 'z' && c < 'A' && c > 'Z' )
                return QString();
            if ( path.at( 1 ).toLatin1() != ':' )
                return QString();
            return path.mid( 0, 2 );
#else
            (void)path;
            return QString();
#endif
        }

        bool isBinaryFile( const std::string &fileName )   // if any char in the first 100 characters is non std::isprint return true
        {
            return isBinaryFile( fileName, std::string() );
        }

        bool isBinaryFile( const std::string &fileName, const std::string &relToDir )   // if any char in the first 100 characters is non std::isprint return true
        {
            auto fullPath = fileName;
            if ( isRelativePath( fileName ) && !relToDir.empty() )
            {
                fullPath = relToDir + "/" + fullPath;
            }

            std::ifstream ifs( fullPath, std::ios::binary | std::ios::in );
            if ( !ifs.is_open() )
                return false;

            size_t chNum = 0;
            char ch;
            while ( ifs.get( ch ) && !ifs.eof() && ifs.good() )
            {
                auto uch = static_cast< unsigned char >( ch );
                bool isAsciiChar = ( ( uch >= 9 ) && ( uch <= 13 ) );
                isAsciiChar = isAsciiChar || ( ( uch >= 32 ) && ( uch <= 126 ) );
                isAsciiChar = isAsciiChar || ( ( uch >= 128 ) /*&& ( ch <= 255 )*/ );

                if ( !isAsciiChar )
                {
                    ifs.close();
                    return true;
                }
                if ( chNum++ > 500 )
                    break;
            }
            ifs.close();
            return false;
        }

        QString expandEnvVars( const QString &fileName, std::set< QString > *envVars )
        {
            static QStringList regExStrings = {
                "\\\\?\\$\\\\?\\((?<regex>\\w*)\\\\?\\)"   // handles $(foo) and \$\(foo\)
                ,
                "\\\\?\\$\\\\?\\{(?<regex>\\w*)\\\\?\\}"   // handles ${foo} and \$\{foo\}
                ,
                "\\\\?\\%\\\\?\\((?<regex>\\w*)\\\\?\\)\\\\?\\%"   // handles %(foo)% and \%\(foo\)\%
                ,
                "\\\\?\\%\\\\?\\{(?<regex>\\w*)\\\\?\\}\\\\?\\%"   // handles %foo% and \%\{foo\}\%
                ,
                "\\\\?\\$(?<regex>\\w*)"   // handles $foo and \$foo
                ,
                "\\\\?\\%(?<regex>\\w*)\\\\?\\%"   // handles %foo% and \%foo\%
            };

            if ( envVars )
                envVars->clear();

            for ( int ii = 0; ii < regExStrings.count(); ++ii )
            {
                auto regExString = regExStrings[ ii ];
                QRegularExpression regExp( regExString );
                assert( regExp.isValid() );
                QRegularExpressionMatch match;
                auto lPos = fileName.indexOf( regExp, 0, &match );
                if ( lPos != -1 )
                {
                    // found a match
                    auto prefix = fileName.left( lPos );
                    auto envVar = match.captured( "regex" );
                    if ( envVars && ( envVars->find( envVar ) == envVars->end() ) )
                        envVars->insert( envVar );
                    auto envVarValue = qgetenv( qPrintable( envVar ) );
                    auto remaining = fileName.mid( lPos + match.capturedLength() );
                    return prefix + envVarValue + expandEnvVars( remaining );
                }
            }

            return fileName;
        }

        QString gSoftenPath( const QString &xFileName, const std::set< QString > &xEnvVars, bool forceUnix )
        {
            auto lRetVal = xFileName;
            for ( auto &&ii : xEnvVars )
            {
                auto lValue = qgetenv( qPrintable( ii ) );

                auto pos = lRetVal.indexOf( lValue );
                if ( pos != -1 )
                {
                    auto lVarName = QString( "${%1}" );
#ifdef Q_OS_WIN
                    if ( !forceUnix )
                        lVarName = QString( "%%1%" );
#else
                    (void)forceUnix;
#endif
                    lVarName = lVarName.arg( ii );
                    lRetVal.replace( pos, lValue.length(), lVarName );
                }
            }

            return lRetVal;
        }

        QString byteSizeString( const QFileInfo &fi, bool prettyPrint, bool byteSize, uint8_t precision, bool spaceBeforeSuffix, const QString &typeNameSuffix )
        {
            return byteSizeString( fi.size(), prettyPrint, byteSize, precision, spaceBeforeSuffix, typeNameSuffix );
        }

        bool compareTimeStamp( const QFileInfo &lhs, const QFileInfo &rhs, int toleranceInSecs, QFileDevice::FileTime timeToCheck )
        {
            return compareTimeStamp( lhs, rhs, toleranceInSecs, std::list< QFileDevice::FileTime >{ timeToCheck } );
        }

        bool compareTimeStamp( const QDateTime &lhs, const QDateTime &rhs, int toleranceInSecs )
        {
            if ( lhs == rhs )
                return true;

            return std::abs( lhs.secsTo( rhs ) ) <= toleranceInSecs;
        }

        bool compareTimeStamp( const QFileInfo &lhs, const QFileInfo &rhs, int toleranceInSecs, const std::list< QFileDevice::FileTime > timeStampsToCheck )
        {
            auto lhsTimeStamps = timeStamps( lhs.absoluteFilePath(), timeStampsToCheck );
            auto rhsTimeStamps = timeStamps( rhs.absoluteFilePath(), timeStampsToCheck );
            if ( lhsTimeStamps.size() != rhsTimeStamps.size() )
                return false;
            for ( auto &&ii : lhsTimeStamps )
            {
                auto jj = rhsTimeStamps.find( ii.first );
                if ( jj == rhsTimeStamps.end() )
                    return false;

                if ( !compareTimeStamp( ii.second, ( *jj ).second, toleranceInSecs ) )
                    return false;
            }
            return true;
        }

        SABUTILS_EXPORT bool isIPAddressNetworkPath( const QFileInfo &info )
        {
            auto path = info.absoluteFilePath();
            if ( !path.startsWith( "//" ) && !path.startsWith( R"(\\)" ) )
                return false;

            auto block = "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)";
            auto regExStr = QString( R"((//|\\\\)%1.%1.%1.%1(/|\\))" ).arg( block );
            auto match = QRegularExpression( regExStr ).match( path );
            if ( match.hasMatch() && match.capturedStart() == 0 )
                return true;
            return false;
        }

        template< typename T >
        std::pair< T, T > correctFixedPointRemainder( T inValue, uint8_t precisionIn, uint8_t precisionOut )
        {
            // if ( precisionIn == precisionOut )
            //     return std::make_pair( inValue, 0 );
            if ( precisionOut > precisionIn )
                return std::make_pair( inValue, 0 );

            auto maxOut = NSABUtils::power( 10, precisionOut );
            auto divBy = NSABUtils::power( 10, precisionIn - precisionOut );
            auto half = divBy / 2;

            auto value = inValue;
            T overflow{ 0 };
            if ( divBy != 0 )
            {
                auto remainder = value % divBy;
                auto integral = value / divBy;
                if ( half && ( remainder >= half ) )
                {
                    if ( integral )
                        integral++;
                    else
                        overflow++;
                    if ( integral >= maxOut )
                    {
                        overflow++;
                        integral = 0;
                    }
                }
                value = integral;
            }
            return std::make_pair( value, overflow );
        }

        QString byteSizeString( uint64_t size, bool prettyPrint, bool byteSize, uint8_t precision, bool spaceBeforeSuffix, const QString &typeNameSuffix )
        {
            if ( !prettyPrint )
            {
                QLocale locale;
                return locale.toString( static_cast< qulonglong >( size ) );
            }

            auto suffixes = std::vector< QString >( { "", "k", "M", "G", "T", "P", "E", "Z", "Y" } );

            auto base = static_cast< uint64_t >( byteSize ? 1024 : 1000 );
            auto suffixPos = 0U;
            while ( ( size >= ( base * base ) ) && ( suffixPos < suffixes.size() ) )
            {
                auto remainder = size % base;
                size -= remainder;
                size /= base;
                suffixPos++;
            }
            uint64_t remainder = 0;
            if ( size >= base )
            {
                remainder = size % base;
                size /= base;
                suffixPos++;
            }

            // remainder will be from 0 to 999(1023)
            // we only want
            uint64_t overflow = 0;
            std::tie( remainder, overflow ) = correctFixedPointRemainder( remainder, 3, precision );
            size += overflow;

            auto suffix = suffixes[ suffixPos ];
            auto realSuffix = QString( "%1%2%3%4" ).arg( spaceBeforeSuffix ? " " : "" ).arg( suffix ).arg( ( !byteSize && typeNameSuffix == "B" ) ? "i" : "" ).arg( typeNameSuffix );

            QLocale locale;
            auto retVal = QString( "%1%2%3" ).arg( locale.toString( static_cast< qulonglong >( size ) ) ).arg( remainder != 0 ? QString( ".%1" ).arg( remainder ) : QString() ).arg( realSuffix );
            return retVal;
        }

        QString getRelativePath( const QDir &absDir, const QString &path )
        {
            QString dir = QDir::cleanPath( absDir.absolutePath() );
            QString file = QDir::cleanPath( path );

            if ( QDir::isRelativePath( file ) || QDir::isRelativePath( dir ) )
                return file;

            bool fileDriveMissing = false;
#ifdef Q_OS_WIN
            QString dirDrive = driveSpec( dir );
            QString fileDrive = driveSpec( file );

            if ( fileDrive.isEmpty() )
            {
                fileDrive = dirDrive;
                fileDriveMissing = true;
            }

            if ( fileDrive.toLower() != dirDrive.toLower() || ( file.startsWith( QLatin1String( "//" ) ) && !dir.startsWith( QLatin1String( "//" ) ) ) )
                return file;

            dir.remove( 0, dirDrive.size() );
            if ( !fileDriveMissing )
                file.remove( 0, fileDrive.size() );
#endif

            QString result;
#if defined( Q_OS_WIN )
            QStringList dirElts = dir.split( QLatin1Char( '/' ), NStringUtils::TSkipEmptyParts );
            QStringList fileElts = file.split( QLatin1Char( '/' ), NStringUtils::TSkipEmptyParts );
#else
            QVector< QStringRef > dirElts = dir.splitRef( QLatin1Char( '/' ), NStringUtils::TSkipEmptyParts );
            QVector< QStringRef > fileElts = file.splitRef( QLatin1Char( '/' ), NStringUtils::TSkipEmptyParts );
#endif
            int ii = 0;
            while ( ii < dirElts.size() && ii < fileElts.size() &&
#if defined( Q_OS_WIN )
                    dirElts.at( ii ).toLower() == fileElts.at( ii ).toLower() )
#else
                    dirElts.at( ii ) == fileElts.at( ii ) )
#endif
                ++ii;

            if ( !fileDriveMissing && ( ii == 0 ) && !dirElts.isEmpty() )   // has a drive AND goes all the way to the root level AND the drive is not at root
                return path;

            for ( int jj = 0; jj < dirElts.size() - ii; ++jj )
                result += QLatin1String( "../" );

            for ( int jj = ii; jj < fileElts.size(); ++jj )
            {
                result += fileElts.at( jj );
                if ( jj < fileElts.size() - 1 )
                    result += QLatin1Char( '/' );
            }

            if ( result.isEmpty() )
                return QLatin1String( "." );
            return result;
        }

        std::string getAbsoluteFilePath( const std::string &dir, const std::string &relFilePath )
        {
            if ( isAbsPath( relFilePath ) )
                return normalizePath( relFilePath );

            std::string retVal = JoinPaths( dir, relFilePath );
            return normalizePath( retVal );
        }

        //////////////////////////////////////////////////////////////////////////
        // exists
        //
        // Check to see if the named file exists. Return true
        // if it exists
        //////////////////////////////////////////////////////////////////////////
        bool exists( const std::string &name )
        {
            return exists( QString::fromStdString( name ) );
        }

        bool exists( const QString &name )
        {
            if ( name.isEmpty() )
                return false;
            return QFileInfo( name ).exists();
        }

        bool isReadable( const std::string &name )
        {
            return isReadable( QString::fromStdString( name ) );
        }

        bool isReadable( const QString &name )
        {
            if ( name.isEmpty() )
                return false;
            return QFileInfo( name ).isReadable();
        }
        bool isRegularFile( const std::string &name )
        {
            return isRegularFile( QString::fromStdString( name ) );
        }

        bool isRegularFile( const QString &name )
        {
            if ( name.isEmpty() )
                return false;
            return QFileInfo( name ).isFile();
        }

        bool isDirectory( const std::string &name )
        {
            return isDirectory( QString::fromStdString( name ) );
        }

        bool isDirectory( const QString &name )
        {
            if ( name.isEmpty() )
                return false;
            return QFileInfo( name ).isDir();
        }

        bool renameFile( const std::string &from, const std::string &to, bool force )
        {
            if ( force && exists( to ) )
            {
                if ( !removePath( to ) )
                {
                    fprintf( stderr, "Error deleting file '%s' to rename '%s' to '%s'\n", to.c_str(), from.c_str(), to.c_str() );
                    return false;
                }
            }

            if ( rename( from.c_str(), to.c_str() ) == -1 )
            {
                fprintf( stderr, "Error renaming file '%s' to '%s'\n", from.c_str(), to.c_str() );
                return false;
            }
            return true;
        }

        bool copyFile( const std::string &from, const std::string &to, bool force )
        {
            if ( force && exists( to ) )
            {
                if ( !removePath( to ) )
                {
                    fprintf( stderr, "Error deleting file '%s' to rename '%s' to '%s'\n", to.c_str(), from.c_str(), to.c_str() );
                    return false;
                }
            }

            if ( !QFile::copy( QString::fromStdString( from ), QString::fromStdString( to ) ) )
            {
                fprintf( stderr, "Error copying file '%s' to '%s'\n", from.c_str(), to.c_str() );
                return false;
            }
            return true;
        }

        //////////////////////////////////////////////////////////////////////////
        // getWd() : return the working directory in 'buffer'. With Windows,
        // you have to specify the maximum size of the buffer in maxLen.
        //////////////////////////////////////////////////////////////////////////
        std::string getWd()
        {
            auto retVal = QDir::currentPath();
            return canonicalFilePath( retVal.toStdString() );
        }

        /******************************************************************
        Function: tilda2Home
        Date: 8/8/07
        Comments:
        * xlate ~<user> name to user's home directory

        Parameters:

        Revision History:

        *******************************************************************/
        std::string tilda2Home( const std::string &fileName )
        {
            std::string retVal = fileName;
#ifndef Q_OS_WINDOWS
            wordexp_t result;
            if ( wordexp( fileName.c_str(), &result, 0 ) != 0 )
                return fileName;
            if ( result.we_wordv[ 0 ] )
                retVal = result.we_wordv[ 0 ];
            wordfree( &result );
#endif
            return retVal;
        }

        std::string canonicalFilePath( const std::string &fileName )
        {
            return canonicalFilePath( QString::fromStdString( fileName ) ).toStdString();
        }

        QString canonicalFilePath( const QString &fileName )
        {
            QFileInfo fi( QFileInfo( fileName ).absoluteFilePath() );
            QString retVal;
            if ( !fi.exists() )
                retVal = fi.absoluteFilePath();
            else
            {
                retVal = fi.canonicalFilePath();
#ifdef Q_OS_WIN
                if ( !retVal.isEmpty() && ( retVal.length() < 2 || retVal[ 1 ] != ':' ) )
                {
                    if ( retVal.mid( 0, 3 ).toLower() == "unc" )
                        retVal.remove( 0, 3 );
                    if ( retVal[ 0 ] == '\\' || retVal[ 0 ] == '/' )
                        retVal.remove( 0, 1 );
                    retVal = fi.absoluteFilePath().mid( 0, 3 ) + retVal;
                }
#endif
            }
            return retVal;
        }

        bool mkdir( const std::string &relDir, bool makeParents )
        {
            std::string tmpDir = relDir;
            return mkdir( tmpDir, makeParents );
        }

        bool mkdir( std::string &dirName, bool makeParents )
        {
            QDir dir( QString::fromStdString( dirName ) );
            dirName = dir.absolutePath().toStdString();
            if ( makeParents )
                return dir.mkpath( "." );
            else
                return dir.mkdir( "." );
        }

        // lhs can be a pattern
        bool pathCompare( const std::string &lhs, const std::string &rhs )
        {
            std::string lhsNormal = getAbsoluteFilePath( lhs );
            std::string rhsNormal = getAbsoluteFilePath( rhs );
            if ( ( *lhsNormal.rbegin() == '/' ) || ( *lhsNormal.rbegin() == '\\' ) )
                lhsNormal.erase( lhsNormal.begin() + lhsNormal.length() - 1 );
            if ( ( *rhsNormal.rbegin() == '/' ) || ( *rhsNormal.rbegin() == '\\' ) )
                rhsNormal.erase( rhsNormal.begin() + rhsNormal.length() - 1 );
            return lhsNormal == rhsNormal;
        }

        std::string normalizePath( const std::string &path, const std::string &relToDir )
        {
            if ( path.empty() )
                return std::string();

            static std::map< std::string, std::string > sNormalizedPaths;

            std::map< std::string, std::string >::iterator ii = sNormalizedPaths.find( path );
            if ( ii != sNormalizedPaths.end() )
            {
                return ( *ii ).second;
            }

            std::string retVal = NStringUtils::stripQuotes( path );

            bool hasTilda = false;
            bool hasDot = false;
            for ( std::string::iterator ii = retVal.begin(); ii != retVal.end(); ++ii )
            {
                if ( *ii == '\\' )
                    *ii = '/';
                hasTilda = hasTilda || ( *ii == '~' );
                hasDot = hasDot || ( *ii == '.' );
            }

            if ( hasTilda )
                retVal = tilda2Home( retVal );

            if ( isRelativePath( retVal ) && !relToDir.empty() )
                retVal = JoinPaths( relToDir, retVal );

            bool hasNonDotDot = false;
            if ( hasDot )
            {
                bool leadingSlash = *retVal.begin() == '/';
                bool trailingSlash = *retVal.rbegin() == '/';

                std::list< std::string > split = NStringUtils::splitString( retVal, '/', false );

                std::list< std::string > tmp;
                for ( std::list< std::string >::iterator ii = split.begin(); ii != split.end(); ++ii )
                {
                    if ( ( *ii ).empty() )
                        continue;
                    if ( *ii == "." )
                    {
                        if ( leadingSlash || !tmp.empty() )
                        {
                            continue;
                        }
                    }
                    else if ( *ii == ".." )
                    {
                        if ( !tmp.empty() && hasNonDotDot )
                        {
                            tmp.pop_back();
                            continue;
                        }
                    }

                    if ( *ii != ".." && *ii != "." )
                        hasNonDotDot = true;
                    tmp.push_back( *ii );
                }
                if ( tmp.empty() )
                {
                    std::string currDir = normalizePath( relToDir.empty() ? getWd() : relToDir );
                    tmp = NStringUtils::splitString( currDir, '/', false );
                }
                retVal = NStringUtils::joinString( tmp, '/', true );
                if ( leadingSlash )
                {
                    retVal = NFileUtils::JoinPaths( "/", retVal );
                }
                if ( trailingSlash )
                {
                    retVal = NFileUtils::JoinPaths( retVal, "/" );
                }
            }
            sNormalizedPaths[ path ] = retVal;
            return retVal;
        }

        std::string JoinPaths( const std::string &dir, const std::string &inFile )
        {
            std::string retVal = NStringUtils::stripQuotes( dir );
            std::string file = NStringUtils::stripQuotes( inFile );
            if ( ( !retVal.empty() && ( ( *retVal.rbegin() ) != '/' ) && ( ( *retVal.rbegin() ) != '\\' ) ) && ( !file.empty() && ( ( *file.begin() ) != '/' ) && ( ( *file.begin() ) != '\\' ) ) )
                retVal += '/';
            retVal += file;
            return retVal;
        }

        std::string changeExtension( const std::string &path, const std::string &newExt )
        {
            std::string dir;
            std::string fileName;
            std::string ext;
            extractFilePath( path, &dir, &fileName, &ext );

            return dir + "/" + fileName + "." + newExt;
        }

        bool copy( const std::string &fileName, const std::string &newFileName )
        {
            return QFile::copy( QString::fromStdString( fileName ), QString::fromStdString( newFileName ) );
        }

        std::list< std::string > getSubDirs( const std::string &dirString, bool recursive, bool includeTopDir )
        {
            QDir dir( QString::fromStdString( dirString ) );
            if ( !dir.exists() )
                return std::list< std::string >();

            QDirIterator::IteratorFlags flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;

            std::list< std::string > retVal;
            if ( includeTopDir )
                retVal = { dirString };
            QDirIterator di( dir.absolutePath(), QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable, flags );
            while ( di.hasNext() )
            {
                QString str = di.next();
                retVal.push_back( str.toStdString() );
                if ( recursive )
                {
                    auto subs = getSubDirs( str.toStdString(), true, false );
                    retVal.insert( retVal.end(), subs.begin(), subs.end() );
                }
            }
            return retVal;
        }

        std::list< std::string > getDirsFromPath( const std::string &searchPath )
        {
            if ( searchPath.empty() )
                return std::list< std::string >();

            char splitChar = ':';
#ifdef Q_OS_WIN
            splitChar = ';';
#endif
            auto paths = NStringUtils::splitString( searchPath, splitChar, true );
            return paths;
        }

        std::string getPathFromDirs( const std::list< std::string > &dirs )
        {
            char joinChar = ':';
#ifdef Q_OS_WIN
            joinChar = ';';
#endif
            auto retVal = NStringUtils::joinString( dirs, joinChar, true );
            return retVal;
        }

        static std::unordered_set< std::string, NStringUtils::noCaseStringHash, NStringUtils::noCaseStringEq > sSystemLibDirs;
        static std::unordered_map< std::string, std::string > sSystemFileMap;

        std::string getSystemFileName( const std::string &fileName, const std::string &relToDir )
        {
            static std::map< std::string, std::string > sFileNameMap;
            std::map< std::string, std::string >::iterator ii = sFileNameMap.find( fileName );
            if ( ii == sFileNameMap.end() )
            {
                QDir cwd;
                if ( relToDir.empty() )
                    cwd.setPath( QDir::currentPath() );
                else
                    cwd = QDir( QString::fromStdString( relToDir ) );

                QString fn = QString::fromStdString( NFileUtils::canonicalFilePath( fileName ) );
                std::string relPath;
                if ( !fn.isEmpty() )
                    relPath = NFileUtils::getRelativePath( cwd, fn ).toStdString();

                if ( relPath.empty() )
                    relPath = fileName;

                sFileNameMap[ fileName ] = relPath;
                return relPath;
            }
            else
                return ( *ii ).second;
            return fileName;
        }

        void addSystemFileDirectories( const std::list< std::string > &dirs )
        {
            if ( sSystemLibDirs.empty() )
            {
                sSystemLibDirs = std::unordered_set< std::string, NStringUtils::noCaseStringHash, NStringUtils::noCaseStringEq >( { "vhdl_packages", "verilog_packages", "ISE", "vivado", "vivado_2014_4", "vivado_2015_2", "15_0", "ProASIC3", "altera_packages" } );
            }
            for ( auto ii : dirs )
            {
                sSystemLibDirs.insert( ii );
                std::replace( ii.begin(), ii.end(), '.', '_' );
                sSystemLibDirs.insert( ii );
            }
        }

        QStringList dumpResources( const QDir &resourceDir, bool ignoreInternal )
        {
            QStringList retVal;

            auto tmp = resourceDir.absolutePath();
            QDirIterator it(
                resourceDir.absolutePath(),
                QStringList() << "*"
                              << "*.*",
                QDir::NoDotAndDotDot | QDir::AllEntries );
            while ( it.hasNext() )
            {
                QString path = it.next();
                QString realPath = path;
                if ( realPath.isEmpty() )
                    continue;
                if ( ignoreInternal && ( realPath.startsWith( ":/trolltech" ) || realPath.startsWith( ":/webkit" ) || realPath.startsWith( ":/http:" ) || realPath.startsWith( ":/qt-project.org" ) || realPath.startsWith( ":/qpdf" ) ) )
                    continue;

                QFileInfo fi( path );
                if ( fi.isDir() )
                {
                    retVal << dumpResources( QDir( path ), ignoreInternal );
                }
                else
                {
                    retVal << realPath;
                }
            }
            return retVal;
        }

        QStringList dumpResources( bool ignoreInternal )
        {
            return dumpResources( QDir( ":/" ), ignoreInternal );
        }

        bool setTimeStamp( const QString &path, bool allTimeStamps, QString *msg )
        {
            return setTimeStamp( path, QDateTime::currentDateTime(), allTimeStamps, msg );
        }

        bool setTimeStamp( const QString &path, QFileDevice::FileTime ft, QString *msg )
        {
            return setTimeStamp( path, QDateTime::currentDateTime(), ft, msg );
        }

        bool setTimeStamp( const QString &path, const QDateTime &dt, bool allTimeStamps, QString *msg )
        {
            if ( !allTimeStamps )
                return setTimeStamp( path, dt, msg );

            bool retVal = setTimeStamp( path, dt, QFileDevice::FileAccessTime, msg );
#ifdef Q_OS_WIN
            retVal = retVal && setTimeStamp( path, dt, QFileDevice::FileBirthTime, msg );
#endif

#ifndef Q_OS_WIN
            retVal = retVal && setTimeStamp( path, dt, QFileDevice::FileMetadataChangeTime, msg );
#endif
            retVal = retVal && setTimeStamp( path, dt, QFileDevice::FileModificationTime, msg );
            return retVal;
        }

#ifdef Q_OS_WINDOWS
        static inline bool toFileTime( const QDateTime &date, FILETIME *fileTime )
        {
            SYSTEMTIME sTime;
            if ( date.timeSpec() == Qt::LocalTime )
            {
                SYSTEMTIME lTime;
                const QDate d = date.date();
                const QTime t = date.time();

                lTime.wYear = d.year();
                lTime.wMonth = d.month();
                lTime.wDay = d.day();
                lTime.wHour = t.hour();
                lTime.wMinute = t.minute();
                lTime.wSecond = t.second();
                lTime.wMilliseconds = t.msec();
                lTime.wDayOfWeek = d.dayOfWeek() % 7;

                if ( !::TzSpecificLocalTimeToSystemTime( 0, &lTime, &sTime ) )
                    return false;
            }
            else
            {
                QDateTime utcDate = date.toUTC();
                const QDate d = utcDate.date();
                const QTime t = utcDate.time();

                sTime.wYear = d.year();
                sTime.wMonth = d.month();
                sTime.wDay = d.day();
                sTime.wHour = t.hour();
                sTime.wMinute = t.minute();
                sTime.wSecond = t.second();
                sTime.wMilliseconds = t.msec();
                sTime.wDayOfWeek = d.dayOfWeek() % 7;
            }

            return ::SystemTimeToFileTime( &sTime, fileTime );
        }

        bool setDirTimeStamp( HANDLE fHandle, const QDateTime &newDate, QFileDevice::FileTime time, QString *msg )
        {
            FILETIME sysTime;
            FILETIME *pLastWrite = NULL;
            FILETIME *pLastAccess = NULL;
            FILETIME *pCreationTime = NULL;

            switch ( time )
            {
                case QFileDevice::FileTime::FileModificationTime:
                    pLastWrite = &sysTime;
                    break;

                case QFileDevice::FileTime::FileAccessTime:
                    pLastAccess = &sysTime;
                    break;

                case QFileDevice::FileTime::FileBirthTime:
                    pCreationTime = &sysTime;
                    break;

                default:
                    if ( msg )
                        *msg = getWindowsError( ERROR_INVALID_PARAMETER );
                    return false;
            }

            if ( !toFileTime( newDate, &sysTime ) )
                return false;

            if ( !::SetFileTime( fHandle, pCreationTime, pLastAccess, pLastWrite ) )
            {
                if ( msg )
                    *msg = getWindowsError( ::GetLastError() );
                return false;
            }
            return true;
        }
#endif

        bool setDirTimeStamp( const QString &path, const QDateTime &dt, QFileDevice::FileTime ft, QString *msg )
        {
            bool retVal = false;
#ifdef Q_OS_WINDOWS
            auto handle = CreateFileW( (wchar_t *)path.utf16(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL );

            if ( handle == INVALID_HANDLE_VALUE )
            {
                if ( msg )
                    *msg = "Could not open directory";
                return false;
            }
            retVal = setDirTimeStamp( handle, dt, ft, msg );
            CloseHandle( handle );
#else
            (void)path;
            (void)dt;
            (void)ft;
            (void)msg;
            retVal = true;
#endif
            return retVal;
        }

        bool setTimeStamp( const QString &path, const QDateTime &dt, QFileDevice::FileTime ft, QString *msg )
        {
            if ( dt.isNull() || !dt.isValid() )
            {
                return setTimeStamp( path, ft, msg );
            }

            if ( QFileInfo( path ).isDir() )
            {
                return setDirTimeStamp( path, dt, ft, msg );
            }
            QFile file( path );
            if ( !file.exists() )
            {
                if ( msg )
                    *msg = "File does not exist";
                return false;
            }
            auto aOK = file.open( QFile::ReadWrite | QFile::ExistingOnly );
            if ( !aOK )
            {
                if ( msg )
                    *msg = "Could not open file to read information. Please check permissions.";
                return false;
            }
            aOK = file.setFileTime( dt, ft );
            if ( !aOK )
            {
                if ( msg )
                    *msg = file.errorString();
            }
            return aOK;
        }

        bool setTimeStamp( const QString &path, const QFileInfo &reference, QString *msg )
        {
            if ( !reference.exists() || !reference.isReadable() )
                return false;

            QFileInfo tgt( path );
            if ( !tgt.exists() || !tgt.isWritable() )
                return false;

            QFile refFile( reference.absoluteFilePath() );
            bool aOK = true;

            auto ts = refFile.fileTime( QFileDevice::FileAccessTime );
            if ( ts.isValid() )
                aOK = aOK && setTimeStamp( path, ts, QFileDevice::FileAccessTime, msg );

#ifdef Q_OS_WIN
            ts = refFile.fileTime( QFileDevice::FileBirthTime );
            if ( ts.isValid() )
                aOK = aOK && setTimeStamp( path, ts, QFileDevice::FileBirthTime, msg );
#endif

#ifndef Q_OS_WIN
            ts = refFile.fileTime( QFileDevice::FileMetadataChangeTime );
            if ( ts.isValid() )
                aOK = aOK && setTimeStamp( path, ts, QFileDevice::FileMetadataChangeTime, msg );
#endif

            ts = refFile.fileTime( QFileDevice::FileModificationTime );
            if ( ts.isValid() )
                aOK = aOK && setTimeStamp( path, ts, QFileDevice::FileModificationTime, msg );

            return aOK;
        }

        bool setTimeStamps( const QString &path, const std::unordered_map< QFileDevice::FileTime, QDateTime > &timeStamps, QString *msg )
        {
            bool aOK = true;
            for ( auto &&ii : timeStamps )
            {
                aOK = aOK && setTimeStamp( path, ii.second, ii.first, msg );
                if ( !aOK )
                    return false;
            }
            return aOK;
        }

        QDateTime timeStamp( const QString &path, QFileDevice::FileTime whichTimeStamp )
        {
            auto fi = QFileInfo( path );
            if ( !fi.exists() )
                return {};
            return fi.fileTime( whichTimeStamp );
        }

        std::unordered_map< QFileDevice::FileTime, QDateTime > timeStamps( const QString &path, const std::list< QFileDevice::FileTime > &timeStampsToGet )
        {
            auto fi = QFileInfo( path );
            if ( !fi.exists() )
                return {};
            std::unordered_map< QFileDevice::FileTime, QDateTime > retVal;

            for ( auto &&ii : timeStampsToGet )
            {
                auto currTime = fi.fileTime( ii );
                // qDebug() << "AccessTime: " << currTime;
                retVal[ ii ] = currTime;
            }

            return retVal;
        }

        std::unordered_map< QFileDevice::FileTime, QDateTime > timeStamps( const QString &path )
        {
            std::list< QFileDevice::FileTime > timeStampsToGet;

            timeStampsToGet.push_back( QFile::FileAccessTime );
#ifdef Q_OS_WIN
            timeStampsToGet.push_back( QFile::FileBirthTime );
#else
            timeStampsToGet.push_back( QFile::FileMetadataChangeTime );
#endif
            timeStampsToGet.push_back( QFile::FileModificationTime );
            return timeStamps( path, timeStampsToGet );
        }

        QDateTime oldestTimeStamp( const QString &path )
        {
            auto allTimes = timeStamps( path );

            QDateTime retVal;
            for ( auto &&ii : allTimes )
            {
                if ( !retVal.isValid() || ( retVal < ii.second ) )
                    retVal = ii.second;
            }
            return retVal;
        }

        bool fileHasAttribute( const QFileInfo &file, EAttribute attribute )
        {
            bool retVal = false;
#ifdef Q_OS_WIN
            DWORD winAttribute = 0;
            switch ( attribute )
            {
                case EAttribute::eArchive:
                    winAttribute = FILE_ATTRIBUTE_ARCHIVE;
                    break;
                case EAttribute::eReadOnly:
                    winAttribute = FILE_ATTRIBUTE_READONLY;
                    break;
                case EAttribute::eSystem:
                    winAttribute = FILE_ATTRIBUTE_SYSTEM;
                    break;
                case EAttribute::eHidden:
                    winAttribute = FILE_ATTRIBUTE_HIDDEN;
                    break;
                default:
                    retVal = false;
                    break;
            }
            auto fileName = file.absoluteFilePath();
            DWORD attr = GetFileAttributesW( (WCHAR *)fileName.utf16() );
            if ( attr != INVALID_FILE_ATTRIBUTES )
                retVal = ( attr & winAttribute ) != 0;
#else
            switch ( attribute )
            {
                case EAttribute::eReadOnly:
                    retVal = !file.isReadable();
                    break;
                case EAttribute::eHidden:
                    retVal = file.fileName().startsWith( "." );
                    break;
                default:
                    retVal = false;
                    break;
            }
#endif
            return retVal;
        }

        bool isArchiveFile( const QFileInfo &file )
        {
            return fileHasAttribute( file, EAttribute::eArchive );
        }

        bool isSystemFile( const QFileInfo &file )
        {
            return fileHasAttribute( file, EAttribute::eSystem );
        }

        bool isHiddenFile( const QFileInfo &file )
        {
            return fileHasAttribute( file, EAttribute::eHidden );
        }

        bool isReadOnlyFile( const QFileInfo &file )
        {
            return fileHasAttribute( file, EAttribute::eReadOnly );
        }

        QString getCorrectPathCase( const QDir &dir, const QStringList &childPaths )   // note, on linux returns path, windows does the actual analysis
        {
            if ( !dir.exists() )
                return {};

            if ( childPaths.isEmpty() )
                return dir.absolutePath();

            auto entries = dir.entryList( QStringList() << childPaths[ 0 ] );
            if ( entries.isEmpty() )
                return {};

            QString nextChild;
            if ( entries.size() == 1 )
                nextChild = entries[ 0 ];
            else
            {
                for ( auto &&ii : qAsConst( entries ) )
                {
                    if ( ii == childPaths[ 0 ] )
                    {
                        nextChild = ii;
                        break;
                    }
                }
                if ( nextChild.isEmpty() )
                    nextChild = entries[ 0 ];
            }

            auto children = childPaths.mid( 1 );
            if ( children.isEmpty() )
                return dir.absoluteFilePath( nextChild );
            return getCorrectPathCase( QDir( dir.absoluteFilePath( nextChild ) ), children );
        }

        QString getCorrectPathCase( QString path )   // note, on linux returns path, windows does the actual analysis
        {
#ifndef Q_OS_WINDOWS
            return path;
#else
            path = QDir::toNativeSeparators( QFileInfo( path.toLower() ).absoluteFilePath() );

            static std::unordered_map< QString, QString > sMap;
            auto pos = sMap.find( path );
            if ( pos != sMap.end() )
                return ( *pos ).second;

            static QString sDoubleSeparator = QString( "%1%1" ).arg( QDir::separator() );
            auto retVal = path;
            QStringList parts;
            if ( retVal.startsWith( sDoubleSeparator ) )   // network path, first part is //XXXXX not //
            {
                auto pos = retVal.indexOf( QDir::separator(), 2 );
                if ( pos == -1 )
                {
                    retVal = retVal.toLower();
                    if ( !QFileInfo( retVal ).exists() )
                        retVal = QString();
                    sMap[ path ] = retVal;
                    return retVal;
                }
                else
                {
                    parts.push_back( retVal.left( pos ).toLower() );
                    parts << retVal.mid( pos + 1 ).split( QDir::separator() );
                }
            }
            else
                parts = retVal.split( QDir::separator() );

            parts.removeAll( QString() );

            if ( parts.isEmpty() )
            {
                sMap[ path ] = QString();
                return {};
            }

            if ( ( parts[ 0 ].length() == 2 ) && ( parts[ 0 ][ 1 ] == ":" ) )
            {
                parts[ 0 ] = parts[ 0 ].toUpper() + QDir::separator();
                if ( parts.length() == 1 )
                {
                    sMap[ path ] = retVal;
                    return retVal;
                }
            }
            QDir currDir;
            int startPart = 0;
            for ( startPart = 0; startPart < parts.length(); ++startPart )
            {
                auto currKey = parts.mid( 0, startPart + 1 ).join( QDir::separator() );
                if ( currKey.length() >= 4 && currKey.mid( 2, 2 ) == sDoubleSeparator )
                    currKey.remove( 2, 1 );

                auto pos = sMap.find( currKey );
                if ( pos == sMap.end() )
                    break;
                retVal = ( *pos ).second;
            }
            Q_ASSERT( startPart < parts.length() );
            if ( startPart >= parts.length() )   // no missing key was found
                return {};

            if ( startPart == 0 )
            {
                retVal = parts[ 0 ];
                sMap[ parts[ 0 ] ] = retVal.replace( "\\", "/" );
                startPart++;
            }

            for ( int ii = startPart; ii < parts.length(); ++ii )
            {
                auto currKey = parts.mid( 0, ii + 1 ).join( QDir::separator() );
                if ( currKey.length() >= 4 && currKey.mid( 2, 2 ) == sDoubleSeparator )
                    currKey.remove( 2, 1 );

                currDir = QDir( retVal );
                auto entries = currDir.entryInfoList( QDir::AllEntries | QDir::NoDotAndDotDot );
                std::optional< QFileInfo > currFi;
                for ( auto &&jj : qAsConst( entries ) )
                {
                    if ( jj.fileName().compare( parts[ ii ], Qt::CaseInsensitive ) == 0 )
                    {
                        currFi = jj;
                        break;
                    }
                }

                if ( !currFi.has_value() )
                {
                    sMap[ currKey ] = QString();
                    return {};
                }

                retVal = currFi.value().absoluteFilePath();
                sMap[ currKey ] = retVal;
            }
            sMap[ path ] = retVal;
            return retVal;
#endif
        }

        std::pair< uint32_t, uint32_t > getVersionInfoFromFile32( const QString &fileName, bool &aOK )
        {
            uint32_t hi{ 0 };
            uint32_t low{ 0 };
            aOK = false;

            QVarLengthArray< wchar_t > fnName( fileName.size() + 1 );
            fnName[ fileName.toWCharArray( fnName.data() ) ] = 0;

            // allocate a block of memory for the version info
            DWORD versionInfoSize = GetFileVersionInfoSizeW( fnName.data(), nullptr );
            if ( versionInfoSize == 0 )
            {
                qDebug() << QString( "GetFileVersionInfoSize failed with error %1" ).arg( getWindowsError( ::GetLastError() ) );
                return { hi, low };
            }

            QVarLengthArray< BYTE > info( static_cast< int >( versionInfoSize ) );
            // load the version info
            if ( !GetFileVersionInfoW( fnName.data(), NULL, versionInfoSize, info.data() ) )
            {
                qDebug() << QString( "GetFileVersionInfo failed with error %1\n" ).arg( getWindowsError( ::GetLastError() ) );
                return { hi, low };
            }

            UINT size;
            DWORD *fi;
            if ( !VerQueryValueW( info.data(), L"\\", reinterpret_cast< void ** >( &fi ), &size ) || !size )
            {
                qDebug() << QString( "Can't obtain ProductVersion from resources" );
                return { hi, low };
            }

            const VS_FIXEDFILEINFO *verInfo = reinterpret_cast< const VS_FIXEDFILEINFO * >( fi );
            hi = verInfo->dwProductVersionMS;
            low = verInfo->dwProductVersionLS;
            aOK = true;
            return { hi, low };
        }

        std::tuple< uint16_t, uint16_t, uint16_t, uint16_t > getVersionInfoFromFile( const QString &fileName, bool &aOK )
        {
            uint16_t major{ 0 };
            uint16_t minor{ 0 };
            uint16_t patchHi{ 0 };
            uint16_t patchLow{ 0 };

            auto value = getVersionInfoFromFile32( fileName, aOK );
            if ( aOK )
            {
                major = HIWORD( value.first );
                minor = LOWORD( value.first );
                patchHi = HIWORD( value.second );
                patchLow = LOWORD( value.second );
            }
            return { major, minor, patchHi, patchLow };
        }
    }
}
