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

#ifndef __BACKUPFILE_H
#define __BACKUPFILE_H

#include "SABUtilsExport.h"

#include <string>
#include <QString>
class QFileInfo;

namespace NSABUtils
{
    namespace NFileUtils
    {
        enum class EMoveOrCopy
        {
            eMove,
            eCopy
        };
        // fileName -> Original filename
        // format -> format of backup file, %FN for filename, %TS for timestamp, default is %FN.bak
        // msg -> error message on failure
        // keepBackup -> if backup exists keep it, add (NUM) to the basename, false means delete old backup
        // useTrash -> if removing a file use the trash rather than destroy the file
        // move -> if EMoveOrCopy::eMove moves the souurce file, otherwise copies it

        SABUTILS_EXPORT bool backup( const std::string &fileName, const std::string &format = "%FN.bak", std::string *msg = nullptr, bool keepBackups = true, bool useTrash = false, EMoveOrCopy moveOrCopyFile = EMoveOrCopy::eMove );
        SABUTILS_EXPORT bool backup( const QFileInfo &fileInfo, const QString &format = "%FN.bak", QString *msg = nullptr, bool keepBackups = true, bool useTrash = false, EMoveOrCopy moveOrCopyFile = EMoveOrCopy::eMove );
        SABUTILS_EXPORT bool backup( const QString &fileName, const QString &format = "%FN.bak", QString *msg = nullptr, bool keepBackups = true, bool useTrash = false, EMoveOrCopy moveOrCopyFile = EMoveOrCopy::eMove );
    }
}
#endif
