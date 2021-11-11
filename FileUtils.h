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

#ifndef __FILEUTILS_H
#define __FILEUTILS_H
#include <string>
#include <list>
#include <set>
#include <unordered_map>
#include <QStringList>
#include <QFileDevice>
class QFileInfo;
class QDateTime;
class QString;
class QDir;

namespace NFileUtils
{
    void extractFilePath( const std::string & pathName,std::string * dirPath = nullptr, std::string * fileName = nullptr, std::string * ext = nullptr);
    bool extCompare( const std::string & pattern, const std::string & extension, bool wildcards );
    bool fileCompare( const std::string & pattern, const std::string & filename, bool wildcards );

    std::string changeExtension( const std::string & pathName, const std::string & newExt );
    bool isAbsPath( const std::string & pathName );
    bool isRelativePath( const std::string & pathName );
    std::string getAbsoluteFilePath( const std::string & relativePath );
    std::string getAbsoluteFilePath( const std::string & directory, const std::string & relativePath );
    QString getRelativePath( const QDir & dir, const QString & absPath );
    std::string getRelativePath( const std::string & absPath, const std::string & dir=std::string() );
    std::string getFileName( const std::string & fileName );
    QString driveSpec( const QString &path );
    bool exists(const std::string & path);
    bool isReadable( const std::string & path );
    bool isRegularFile( const std::string & path );
    bool isDirectory( const std::string & path ); 
    bool removeFile(const std::string & fileName);
    bool renameFile(const std::string & oldfileName, const std::string & newFileName, bool force=false); 
    int changeDir(const std::string & newDir);
    std::string getWd();
    bool get_line_from_file(FILE *fp,std::string & line, int& line_no );
    std::string tilda2Home( const std::string & fileName ); 
    std::string JoinPaths( const std::string & dir, const std::string & file );
    bool mkdir( std::string & dir, bool makeParents=true ); // dir gets set to absolute path
    bool mkdir( const std::string & dir, bool makeParents=true ); //
    bool pathCompare( const std::string & lhs, const std::string & rhs );  // return is lhs is the same path as rhs
    std::string normalizePath( const std::string & path, const std::string & relToDir=std::string() ); // removes ".." and "." replaces all "\" with "/"

    bool remove( const std::string & item );
    bool removeInsideOfDir( const QString & dirStr );
    bool removeInsideOfDir( const std::string & dir );
    bool copy( const std::string & fileName, const std::string & newFileName );
    QString canonicalFilePath( const QString & fileName );
    std::string canonicalFilePath( const std::string & fileName );

    // %FN for filename, %TS for timestamp
    // empty -> %FN.bak
    bool backup( const std::string & fileName, const std::string & msg, bool useTrash=false, const std::string & format=std::string(), bool moveFile=true );
    bool backup( const QString & fileName, const std::string & msg, bool useTrash=false, const std::string & format=std::string(), bool moveFile=true );

    std::list< std::string > getSubDirs( const std::string & dir, bool recursive, bool includeTopDir ); 
    std::list< std::string > getDirsFromPath( const std::string & searchPath );
    std::string getPathFromDirs( const std::list< std::string > & dirs );

    bool moveToTrash( const QString & fileName );
    bool moveToTrash( const std::string & fileName );

    bool isBinaryFile( const std::string & fileName ); // if any char in the first 100 characters is non std::isprint return true
    bool isBinaryFile( const std::string & fileName, const std::string & relToDir ); // if any char in the first 100 characters is non std::isprint return true

    // searches for environmental vars inside filenames of the form
    // $foo or %foo% \$foo \%foo\%
    // the variable itself can be surrounded by {} or () or \{\} \(\)
    //
    // it gets the prefix (before the start) extracts the variable name and then recursively calls itself 
    // on the suffix.
    QString expandEnvVars( const QString & fileName, std::set< QString > * envVars = nullptr );
    QString gSoftenPath( const QString & xFileName, const std::set< QString > & xEnvVars, bool forceUnix = false ); // force unix just helps in unit testing

    QStringList dumpResources( bool ignoreInternal = true );
    QStringList dumpResources( const QDir & xDir, bool ignoreInternal=true );

    std::unordered_map< QFileDevice::FileTime, QDateTime > timeStamps( const QString &path );
    QDateTime oldestTimeStamp( const QString &path ); // returns the oldest time based on QFileDevice::FileTime types
    bool setTimeStamp(const QString& path, const QFileInfo & srcPath, QString * msg = nullptr ); // uses the ts info on srcPath setting it on path for all filetimes
    bool setTimeStamp(const QString& path, const QDateTime& ts, bool allTimeStamps, QString* msg = nullptr); // if ts is not valid uses current dt, if allTimeStamps is false use FileModificationTime
    bool setTimeStamp(const QString& path, const QDateTime & ts, QFileDevice::FileTime ft = QFileDevice::FileTime::FileModificationTime, QString* msg = nullptr); // if ts is not valid uses current dt
    bool setTimeStamp(const QString& path, QFileDevice::FileTime ft = QFileDevice::FileTime::FileModificationTime, QString* msg = nullptr); // uses QDateTime::currentDateTime
    bool setTimeStamp(const QString& path, bool allTimeStamps, QString* msg = nullptr); // uses QDateTime::currentDateTime
    bool setTimeStamps( const QString &path, const std::unordered_map< QFileDevice::FileTime, QDateTime > & timeStamps, QString *msg = nullptr ); 

    QString fileSizeString( const QFileInfo & fi, bool prettyPrint = true, bool byteBased=true, uint8_t precision=1 ); // pretty print use suffixes, bytesize=true means using 1024 vs 1000 based suffixes bytsize is ignored if prettyprint is false, precision is 0, 1, 2 or 3 for number of decimal places in a pretty print
    QString fileSizeString( uint64_t size, bool prettyPrint = true, bool byteBased = true, uint8_t precision=1 );
}
#endif
