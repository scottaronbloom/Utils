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

#include "SABUtilsExport.h"

#include <string>
#include <list>
#include <set>
#include <unordered_map>
#include <optional>
#include <QStringList>
#include <QFileDevice>
#include <QList>

class QFileInfo;
class QDateTime;
class QString;
class QDir;

namespace NSABUtils
{
    namespace NFileUtils
    {
        SABUTILS_EXPORT void extractFilePath(const std::string& pathName, std::string* dirPath = nullptr, std::string* fileName = nullptr, std::string* ext = nullptr);
        SABUTILS_EXPORT bool extCompare(const std::string& pattern, const std::string& extension, bool wildcards);
        SABUTILS_EXPORT bool fileCompare(const std::string& pattern, const std::string& filename, bool wildcards);

        SABUTILS_EXPORT std::string changeExtension(const std::string& pathName, const std::string& newExt);
        SABUTILS_EXPORT bool isAbsPath(const std::string& pathName);
        SABUTILS_EXPORT bool isRelativePath(const std::string& pathName);
        SABUTILS_EXPORT std::string getAbsoluteFilePath(const std::string& relativePath);
        SABUTILS_EXPORT std::string getAbsoluteFilePath(const std::string& directory, const std::string& relativePath);
        SABUTILS_EXPORT QString getRelativePath(const QDir& dir, const QString& absPath);
        SABUTILS_EXPORT std::string getRelativePath(const std::string& absPath, const std::string& dir = std::string());
        SABUTILS_EXPORT std::string getFileName(const std::string& fileName);
        SABUTILS_EXPORT QString driveSpec(const QString& path);
        SABUTILS_EXPORT bool exists(const std::string& path);
        SABUTILS_EXPORT bool isReadable(const std::string& path);
        SABUTILS_EXPORT bool isRegularFile(const std::string& path);
        SABUTILS_EXPORT bool isDirectory(const std::string& path);

        SABUTILS_EXPORT bool exists(const QString& path);
        SABUTILS_EXPORT bool isReadable(const QString& path);
        SABUTILS_EXPORT bool isRegularFile(const QString& path);
        SABUTILS_EXPORT bool isDirectory(const QString& path);

        SABUTILS_EXPORT bool renameFile(const std::string& oldfileName, const std::string& newFileName, bool force = false);
        SABUTILS_EXPORT std::string getWd();
        SABUTILS_EXPORT std::string tilda2Home(const std::string& fileName);
        SABUTILS_EXPORT std::string JoinPaths(const std::string& dir, const std::string& file);
        SABUTILS_EXPORT bool mkdir(std::string& dir, bool makeParents = true); // dir gets set to absolute path
        SABUTILS_EXPORT bool mkdir(const std::string& dir, bool makeParents = true); //
        SABUTILS_EXPORT bool pathCompare(const std::string& lhs, const std::string& rhs);  // return is lhs is the same path as rhs
        SABUTILS_EXPORT std::string normalizePath(const std::string& path, const std::string& relToDir = std::string()); // removes ".." and "." replaces all "\" with "/"

        SABUTILS_EXPORT bool removePath(const QString& item);
        SABUTILS_EXPORT bool removePath(const std::string& item);
        SABUTILS_EXPORT bool removeInsideOfDir(const QString& dirStr);
        SABUTILS_EXPORT bool removeInsideOfDir(const std::string& dir);
        SABUTILS_EXPORT bool copy(const std::string& fileName, const std::string& newFileName);

        SABUTILS_EXPORT QString canonicalFilePath(const QString& fileName);
        SABUTILS_EXPORT std::string canonicalFilePath(const std::string& fileName);

        // %FN for filename, %TS for timestamp
        // empty -> %FN.bak
        SABUTILS_EXPORT bool backup(const std::string& fileName, const std::string& msg, bool useTrash = false, const std::string& format = std::string(), bool moveFile = true);
        SABUTILS_EXPORT bool backup(const QString& fileName, const std::string& msg, bool useTrash = false, const std::string& format = std::string(), bool moveFile = true);

        SABUTILS_EXPORT std::list< std::string > getSubDirs(const std::string& dir, bool recursive, bool includeTopDir);
        SABUTILS_EXPORT std::list< std::string > getDirsFromPath(const std::string& searchPath);
        SABUTILS_EXPORT std::string getPathFromDirs(const std::list< std::string >& dirs);

        SABUTILS_EXPORT bool isBinaryFile(const std::string& fileName); // if any char in the first 100 characters is non std::isprint return true
        SABUTILS_EXPORT bool isBinaryFile(const std::string& fileName, const std::string& relToDir); // if any char in the first 100 characters is non std::isprint return true

        // searches for environmental vars inside filenames of the form
        // $foo or %foo% \$foo \%foo\%
        // the variable itself can be surrounded by {} or () or \{\} \(\)
        //
        // it gets the prefix (before the start) extracts the variable name and then recursively calls itself 
        // on the suffix.
        SABUTILS_EXPORT QString expandEnvVars(const QString& fileName, std::set< QString >* envVars = nullptr);
        SABUTILS_EXPORT QString gSoftenPath(const QString& xFileName, const std::set< QString >& xEnvVars, bool forceUnix = false); // force unix just helps in unit testing

        SABUTILS_EXPORT QStringList dumpResources(bool ignoreInternal = true);
        SABUTILS_EXPORT QStringList dumpResources(const QDir& xDir, bool ignoreInternal = true);

        SABUTILS_EXPORT std::unordered_map< QFileDevice::FileTime, QDateTime > timeStamps(const QString& path, const std::list< QFileDevice::FileTime >& timeStampsToGet);
        SABUTILS_EXPORT std::unordered_map< QFileDevice::FileTime, QDateTime > timeStamps(const QString& path);
        SABUTILS_EXPORT QDateTime timeStamp(const QString& path, QFileDevice::FileTime = QFileDevice::FileTime::FileModificationTime);
        SABUTILS_EXPORT QDateTime oldestTimeStamp(const QString& path); // returns the oldest time based on QFile Device::FileTime types
        SABUTILS_EXPORT bool setTimeStamp(const QString& path, const QFileInfo& srcPath, QString* msg = nullptr); // uses the ts info on srcPath setting it on path for all filetimes
        SABUTILS_EXPORT bool setTimeStamp(const QString& path, const QDateTime& ts, bool allTimeStamps, QString* msg = nullptr); // if ts is not valid uses current dt, if allTimeStamps is false use FileModificationTime
        SABUTILS_EXPORT bool setTimeStamp(const QString& path, const QDateTime& ts, QFileDevice::FileTime ft = QFileDevice::FileTime::FileModificationTime, QString* msg = nullptr); // if ts is not valid uses current dt
        SABUTILS_EXPORT bool setTimeStamp(const QString& path, QFileDevice::FileTime ft = QFileDevice::FileTime::FileModificationTime, QString* msg = nullptr); // uses QDateTime::currentDateTime
        SABUTILS_EXPORT bool setTimeStamp(const QString& path, bool allTimeStamps, QString* msg = nullptr); // uses QDateTime::currentDateTime
        SABUTILS_EXPORT bool setTimeStamps(const QString& path, const std::unordered_map< QFileDevice::FileTime, QDateTime >& timeStamps, QString* msg = nullptr);

        SABUTILS_EXPORT QString byteSizeString(const QFileInfo& fi, bool prettyPrint = true, bool byteBased = true, uint8_t precision = 1); // pretty print use suffixes, bytesize=true means using 1024 vs 1000 based suffixes bytsize is ignored if prettyprint is false, precision is 0, 1, 2 or 3 for number of decimal places in a pretty print
        SABUTILS_EXPORT QString byteSizeString(uint64_t size, bool prettyPrint = true, bool byteBased = true, uint8_t precision = 1);

        enum class EAttribute
        {
            eArchive,
            eHidden,
            eSystem,
            eReadOnly
        };
        SABUTILS_EXPORT bool fileHasAttribute(const QFileInfo& file, EAttribute attribute);

        SABUTILS_EXPORT bool isArchiveFile(const QFileInfo& file);
        SABUTILS_EXPORT bool isHiddenFile(const QFileInfo& file);
        SABUTILS_EXPORT bool isSystemFile(const QFileInfo& file);
        SABUTILS_EXPORT bool isReadOnlyFile(const QFileInfo& file);

        SABUTILS_EXPORT bool compareTimeStamp(const QDateTime& lhs, const QDateTime& rhs, int toleranceInSecs);
        SABUTILS_EXPORT bool compareTimeStamp(const QFileInfo& lhs, const QFileInfo& rhs, int toleranceInSecs, QFileDevice::FileTime timeToCheck);
        SABUTILS_EXPORT bool compareTimeStamp(const QFileInfo& lhs, const QFileInfo& rhs, int toleranceInSecs, const std::list< QFileDevice::FileTime > timeStampsToCheck);

        SABUTILS_EXPORT QString getWindowsError(int errorCode); // returns empty string for non-windows

        SABUTILS_EXPORT QString getCorrectPathCase(QString path); // note, on linux returns path, windows does the actual analysis, and returns the absolute path

        SABUTILS_EXPORT std::optional< QList< QFileInfo > > findAllFiles(const QDir& dir, const QStringList& nameFilters, bool recursive, bool sortByName = false, QString* errorMsg = nullptr);
        SABUTILS_EXPORT bool isIPAddressNetworkPath(const QFileInfo& info);

        SABUTILS_EXPORT std::tuple< uint16_t, uint16_t, uint16_t, uint16_t > getVersionInfoFromFile(const QString& fileName, bool& aOK);
        SABUTILS_EXPORT std::pair< uint32_t, uint32_t > getVersionInfoFromFile32(const QString& fileName, bool& aOK); // the 32 bit version returns 2 32 bit values, the hiword of the first is the major version, loword is the minor, hi and low of the second value is the patch
    }
}
#endif
