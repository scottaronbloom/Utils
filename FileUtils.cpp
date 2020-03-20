// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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
#include "StringUtils.h"

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
#ifdef _WINDOWS
#include <io.h>     // for _access()
#include <direct.h> // for chdir
#include <Windows.h>
#define access _access
#define chdir _chdir
#define unlink _unlink
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#else
#include <unistd.h> // for access()
#include <wordexp.h>
#include <sys/stat.h>
#endif


#include <map>
#include <cctype>

namespace NFileUtils
{
using namespace std;

std::string getFileName( const std::string & path )
{
    std::string::size_type pos = path.find_last_of( "/\\" );
    if ( pos == std::string::npos )
        return path;
    return path.substr( pos+1 );
}

//////////////////////////////////////////////////////////////////////////
// extractFilePath
// 
// Decompose a file pathname into a file pathname, the
// base filename and the extension. dirPath will be the 
// directory path, fileName will be the filename, 
// and ext will be the file extension. 
//////////////////////////////////////////////////////////////////////////
void extractFilePath(const std::string & pathName,std::string * dirPath, std::string * outFileName, std::string * ext)
{
    if ( dirPath )
        *dirPath = std::string();
    if ( ext )
        *ext = std::string();
    if ( outFileName )
        *outFileName = std::string();

    std::string fileName = pathName;

    // Get last directory specifier
    string::size_type idx = pathName.find_last_of( "/\\" );
    if (idx != string::npos)
    {
        if ( dirPath )
            *dirPath = pathName.substr(0,idx);
        if ( outFileName )
            *outFileName = fileName = pathName.substr(idx+1);
    }

    idx = fileName.rfind(".");
    if (idx != string::npos)
    {
        if ( ext )
            *ext = fileName.substr(idx+1);
        if ( outFileName )
            *outFileName = fileName = fileName.substr(0,idx);
    }
}

bool extCompare( const std::string & pattern, const std::string & extension, bool wildcards )
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

bool fileCompare( const std::string & pattern, const std::string & filename, bool wildcards )
{
    if ( pattern.empty() || filename.empty() )
        return false;

    QString qPattern = QString::fromStdString( pattern );
    auto patternList = qPattern.split( QRegularExpression( "[\\\\/]" ), QString::SkipEmptyParts );
    if ( patternList.isEmpty() )
        return false;

    QString qFileName = QString::fromStdString( filename );
    auto fileNameList = qFileName.split( QRegularExpression( "[\\\\/]" ), QString::SkipEmptyParts );
    if ( fileNameList.isEmpty() )
        return false;

    auto currPattern = patternList.size() - 1;
    auto currFileName = fileNameList.size() - 1;

    bool matches = true;
    for( ; matches && ((currPattern >= 0) && (currFileName >= 0)); currPattern--, currFileName-- )
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

bool isRelativePath( const std::string & relPath )
{
    if( relPath.empty() )
        return false;
    return !isAbsPath( relPath );
}

bool isAbsPath( const std::string & relPath )
{
    if ( relPath.empty() )
        return false;
    size_t lhs = 0;
    if ( *relPath.begin() == '"' && *relPath.rbegin() == '"' )
        lhs++;
    char firstChar = ::toupper( relPath[ lhs ] );
    bool isAbs = firstChar == '/' || firstChar == '\\' || 
        ( ( relPath.length() >= ( lhs + 3 ) ) && ( firstChar >= 'A' && firstChar <= 'Z' ) && relPath[ lhs + 1 ] == ':' && ( relPath[ lhs + 2 ] == '/' || relPath[ lhs + 2 ] == '\\' ) );
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

std::string getAbsoluteFilePath( const std::string & relFilePath )
{
    return getAbsoluteFilePath( getWd(), relFilePath );
}

std::string getRelativePath( const std::string & absPath, const std::string & dir )
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
#if defined(Q_OS_WIN)
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

bool isBinaryFile( const std::string & fileName ) // if any char in the first 100 characters is non std::isprint return true
{
    return isBinaryFile( fileName, std::string() );
}

bool isBinaryFile( const std::string & fileName, const std::string & relToDir ) // if any char in the first 100 characters is non std::isprint return true
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
        bool isAsciiChar = ( ( uch >=9 ) && ( uch <= 13 ) );
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

QString getRelativePath( const QDir & absDir, const QString & path )
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

    if ( fileDrive.toLower() != dirDrive.toLower()
            || ( file.startsWith( QLatin1String( "//" ) )
            && !dir.startsWith( QLatin1String( "//" ) ) ) )
            return file;

    dir.remove( 0, dirDrive.size() );
    if ( !fileDriveMissing )
        file.remove( 0, fileDrive.size() );
#endif

    QString result;
#if defined(Q_OS_WIN)
    QStringList dirElts = dir.split( QLatin1Char( '/' ), QString::SkipEmptyParts );
    QStringList fileElts = file.split( QLatin1Char( '/' ), QString::SkipEmptyParts );
#else
    QVector<QStringRef> dirElts = dir.splitRef( QLatin1Char( '/' ), QString::SkipEmptyParts );
    QVector<QStringRef> fileElts = file.splitRef( QLatin1Char( '/' ), QString::SkipEmptyParts );
#endif
    int ii = 0;
    while ( ii < dirElts.size() && ii < fileElts.size() &&
#if defined(Q_OS_WIN)
            dirElts.at( ii ).toLower() == fileElts.at( ii ).toLower() )
#else
            dirElts.at( ii ) == fileElts.at( ii ))
#endif
            ++ii;

    if ( !fileDriveMissing && ( ii == 0 ) && !dirElts.isEmpty() ) // has a drive AND goes all the way to the root level AND the drive is not at root
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

std::string getAbsoluteFilePath(const std::string & dir, const std::string & relFilePath )
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
bool exists(const std::string & name)
{
    if (name.empty())
        return false;
    if (access(name.c_str(), F_OK) == -1)
        return false;
    return true;

}

bool isReadable( const std::string & name )
{
    if (name.empty())
        return false;
    if (access(name.c_str(), R_OK) == -1)
        return false;
    return true;

}

//////////////////////////////////////////////////////////////////////////
// Check to see if the named file is a regular File. Return true
// if it is
//

//
//////////////////////////////////////////////////////////////////////////
bool isRegularFile(const std::string & name)
{
    if (name.empty())
        return false;
    QFileInfo fi( QString::fromStdString( name ) );
    return fi.isFile();
}

//////////////////////////////////////////////////////////////////////////
// 
// Check to see if the named file is a directory File. Return true
// if it is
//

//
//////////////////////////////////////////////////////////////////////////
bool isDirectory( const std::string & name)
{
    if (name.empty())
        return false;
    QFileInfo fi( QString::fromStdString( name ) );
    return fi.isDir();

}

bool renameFile(const std::string & from,const std::string & to, bool force)
{
    if ( force && exists( to ) )
    {
        if ( !removeFile( to ) )
        {
            fprintf( stderr, "Error deleting file '%s' to rename '%s' to '%s'\n", to.c_str(), from.c_str(), to.c_str() ); 
            return false;
        }
    }

    if (rename( from.c_str(), to.c_str())==-1)
    {
        fprintf( stderr, "Error renaming file '%s' to '%s'\n", from.c_str(), to.c_str() ); 
        return false;
    }
    return true;
}

bool copyFile(const std::string & from,const std::string & to, bool force)
{
    if ( force && exists( to ) )
    {
        if ( !removeFile( to ) )
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
// removeFile() : remove the file 'fileName'
//////////////////////////////////////////////////////////////////////////
bool removeFile(const std::string & fileName)
{
    return unlink( fileName.c_str() ) == 0;
}

//////////////////////////////////////////////////////////////////////////
// changeDir() : change the working directory
//////////////////////////////////////////////////////////////////////////
int changeDir(const std::string & newDir)
{
    return (chdir(newDir.c_str()));
}

//////////////////////////////////////////////////////////////////////////
// getWd() : return the working directory in 'buffer'. With Windows,
// you have to specify the maximum size of the buffer in maxLen.
//////////////////////////////////////////////////////////////////////////
std::string getWd()
{
    std::string retVal;
#ifdef _WINDOWS
    char * buffer;
    if ( ( buffer = _getcwd( nullptr, 0 ) ) != nullptr )
    {
        retVal = buffer;
        free( buffer );
    }
#else
    size_t sz = pathconf( ".", _PC_PATH_MAX );
    char * buff = new char[ sz + 1 ];
    getcwd( buff, sz );
    retVal = buff;
    delete [] buff;
#endif
    return canonicalFilePath( retVal );
}

/******************************************************************
Function: tilda2Home
Date: 8/8/07
Comments: 
* xlate ~<user> name to user's home directory

Parameters:

Revision History:

*******************************************************************/
std::string tilda2Home( const std::string & fileName )
{
    std::string retVal = fileName;
#ifndef _WINDOWS
    wordexp_t result;
    if ( wordexp( fileName.c_str(), &result, 0 ) != 0 )
        return fileName;
    if ( result.we_wordv[ 0 ] )
        retVal = result.we_wordv[ 0 ];
    wordfree( &result );
#endif
    return retVal;
}

bool remove( const QString & entry )
{
    QFileInfo fi( entry );
    if ( fi.isFile() )
    {
        bool success = QFile::remove( fi.absoluteFilePath() );
        return success;
    }
    else if ( fi.isDir() )
    {
        bool aOK = removeInsideOfDir( fi.absoluteFilePath() );
        if ( aOK )
        {
            QDir dir( fi.absoluteFilePath() );
#ifdef _WIN32
            QString fileName = dir.absolutePath().replace( "/", "\\" );
            aOK = ::RemoveDirectoryW( ( LPCWSTR )fileName.utf16() ) != 0;
#else
            QString subDirName = dir.dirName();
            dir.cdUp();
            aOK = dir.rmdir( subDirName );
#endif
        }
        return aOK;
    }
    return !fi.exists();
}

bool remove( const std::string & dir )
{
    return remove( QString::fromStdString( dir ) );
}

bool removeInsideOfDir( const QString & dirStr )
{
    QDir dir( dirStr );
    QFileInfoList entries = dir.entryInfoList( QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files );
    bool aOK = true;
    for( int ii = 0; aOK && ii < entries.size(); ++ii )
    {
        aOK = remove( entries[ ii ].absoluteFilePath() );
    }

    return aOK;
}

bool removeInsideOfDir( const std::string & dir )
{
    return removeInsideOfDir( QString::fromStdString( dir ) );
}

std::string canonicalFilePath( const std::string & fileName )
{
    return canonicalFilePath( QString::fromStdString( fileName ) ).toStdString();
}

QString canonicalFilePath( const QString & fileName )
{
    QFileInfo fi( QFileInfo( fileName ).absoluteFilePath() );
    QString retVal;
    if ( !fi.exists() )
        retVal = fi.absoluteFilePath();
    else
    {
        retVal = fi.canonicalFilePath();
#ifdef _WIN32
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

bool backup( const QString & fileName, const std::string & msg, bool useTrash, const std::string & format, bool moveFile )
{
    return backup( fileName.toStdString(), msg, useTrash, format, moveFile );
}

// format 
// %FN for filename, %TS for timestamp
// empty -> %f.bak
bool backup( const std::string & fileName, const std::string & msg, bool useTrash, const std::string & format, bool moveFile )
{
    if( NFileUtils::exists( fileName ) )
    {
        std::ostringstream oss;
        std::string backupFile = format;
        if ( backupFile.empty() )
            backupFile = "%FN.bak";

        auto pos = backupFile.find( "%FN" );
        if ( pos != std::string::npos )
        {
            backupFile = backupFile.replace( pos, 3, fileName );
        }

        pos = backupFile.find( "%TS" );
        if ( pos != std::string::npos )
        {
            backupFile = backupFile.replace( pos, 3, QDateTime::currentDateTime().toString( "ddMMyyyy_hhmmss.zzz" ).toStdString() );
        }

        if ( backupFile == fileName )
            backupFile = fileName + ".bak";

        if ( useTrash && NFileUtils::exists( backupFile ) )
            NFileUtils::moveToTrash( backupFile );
        else
            NFileUtils::remove( backupFile );

        if ( moveFile )
            return NFileUtils::renameFile( fileName, fileName + ".bak", true );
        else
            return NFileUtils::copyFile( fileName, fileName + ".bak", true );
    }
    else
        return true;
}

bool mkdir( const std::string & relDir, bool makeParents )
{
    std::string tmpDir = relDir;
    return mkdir( tmpDir, makeParents );
}

bool mkdir( std::string & dirName, bool makeParents )
{
    QDir dir( QString::fromStdString( dirName ) );
    dirName = dir.absolutePath().toStdString();
    if ( makeParents )
        return dir.mkpath( "." );
    else
        return dir.mkdir( "." );
}

// lhs can be a pattern
bool pathCompare( const std::string & lhs, const std::string & rhs )
{
    std::string lhsNormal = getAbsoluteFilePath( lhs );
    std::string rhsNormal = getAbsoluteFilePath( rhs );
    if ( ( *lhsNormal.rbegin() == '/' ) || ( *lhsNormal.rbegin() == '\\' ) )
        lhsNormal.erase( lhsNormal.begin() + lhsNormal.length() - 1 );
    if ( ( *rhsNormal.rbegin() == '/' ) || ( *rhsNormal.rbegin() == '\\' ) )
        rhsNormal.erase( rhsNormal.begin() + rhsNormal.length() - 1 );
    return lhsNormal == rhsNormal;
}

std::string normalizePath( const std::string & path, const std::string & relToDir )
{
    if ( path.empty() )
        return std::string();

    static std::map< std::string, std::string > sNormalizedPaths;

    std::map< std::string, std::string >::iterator ii = sNormalizedPaths.find( path );
    if ( ii != sNormalizedPaths.end() )
    {
        return (*ii).second;
    }

    std::string retVal = NStringUtils::stripQuotes( path );

    bool hasTilda = false;
    bool hasDot = false;
    for( std::string::iterator ii = retVal.begin(); ii != retVal.end(); ++ii )
    {
        if ( *ii == '\\' )
            *ii = '/';
        hasTilda = hasTilda || ( *ii == '~' );
        hasDot = hasDot || (*ii == '.' );
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
        for( std::list< std::string >::iterator ii = split.begin(); ii != split.end(); ++ii )
        {
            if ( (*ii).empty()) 
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

std::string JoinPaths( const std::string & dir, const std::string & inFile )
{
    std::string retVal = NStringUtils::stripQuotes( dir );
    std::string file = NStringUtils::stripQuotes( inFile );
    if ( ( !retVal.empty() && ( (*retVal.rbegin()) != '/' ) && ( (*retVal.rbegin()) != '\\' ) ) &&
         ( !file.empty()   && ( (*file.begin()) != '/' ) && ( (*file.begin()) != '\\' ) ) 
         )
        retVal += '/';
    retVal += file;
    return retVal;
}

std::string changeExtension( const std::string & path, const std::string & newExt )
{
    std::string dir;
    std::string fileName;
    std::string ext;
    extractFilePath( path, &dir, &fileName, &ext );

    return dir + "/" + fileName + "." + newExt;
}

bool copy( const std::string & fileName, const std::string & newFileName )
{
    return QFile::copy( QString::fromStdString( fileName ), QString::fromStdString( newFileName ) );
}

std::list< std::string > getSubDirs( const std::string & dirString, bool recursive, bool includeTopDir )
{
    QDir dir( QString::fromStdString( dirString ) ); 
    if ( !dir.exists() )
        return std::list< std::string >();

    QDirIterator::IteratorFlags flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;

    std::list< std::string > retVal;
    if ( includeTopDir )
        retVal = { dirString };
    QDirIterator di( dir.absolutePath(), QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable, flags );
    while( di.hasNext() )
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


std::list< std::string > getDirsFromPath( const std::string & searchPath )
{
    if( searchPath.empty() )
        return std::list< std::string >();

    char splitChar = ':';
#ifdef Q_OS_WIN
    splitChar = ';';
#endif
    auto paths = NStringUtils::splitString( searchPath, splitChar, true );
    return paths;
}

std::string getPathFromDirs( const std::list< std::string > & dirs )
{
    char joinChar = ':';
#ifdef Q_OS_WIN
    joinChar = ';';
#endif
    auto retVal = NStringUtils::joinString( dirs, joinChar, true );
    return retVal;
}

#ifndef BUILDING_VERIFIC_APP
bool moveToTrashImpl( const QString & fileName );
bool moveToTrash( const QString & fileName )
{
    if ( !moveToTrashImpl( fileName ) )
        return remove( fileName );
    return true;
}
bool moveToTrash( const std::string & fileName )
{
    return moveToTrash( QString::fromStdString( fileName ) );
}
#endif

static std::unordered_set< std::string, NStringUtils::noCaseStringHash, NStringUtils::noCaseStringEq > sSystemLibDirs;
static std::unordered_map< std::string, std::string > sSystemFileMap;

std::string getSystemFileName( const std::string & fileName, const std::string & relToDir )
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

void addSystemFileDirectories( const std::list< std::string > & dirs )
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

}


