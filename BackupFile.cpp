// The MIT License( MIT )
//
// Copyright( c ) 2023 Scott Aron Bloom
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

#include "BackupFile.h"
#include "MoveToTrash.h"
#include "FileUtils.h"

#include <QDateTime>
#include <QFileInfo>

namespace NSABUtils
{
    namespace NFileUtils
    {
        bool backup( const std::string &fileName, const std::string &format /*= "%FN.bak"*/, std::string *msg /*= nullptr*/, bool keepBackups /*= true*/, bool useTrash /*= false*/, EMoveOrCopy moveOrCopyFile /*= EMoveOrCopy::eMove*/ )
        {
            QString lclErrorMsg;
            auto retVal = backup( QString::fromStdString( fileName ), QString::fromStdString( format ), &lclErrorMsg, keepBackups, useTrash, moveOrCopyFile );
            if ( msg )
                *msg = lclErrorMsg.toStdString();
            return retVal;
        }

        bool backup( const QFileInfo & fileInfo, const QString &format /*= "%FN.bak"*/, QString *msg /*= nullptr*/, bool keepBackups /*= true*/, bool useTrash /*= false*/, EMoveOrCopy moveOrCopyFile /*= EMoveOrCopy::eMove*/ )
        {
            return backup( fileInfo.absoluteFilePath(), format, msg, keepBackups, useTrash, moveOrCopyFile );
        }

        bool removeFile( const QString & backupFile, bool useTrash, QString * msg )
        {
            if ( !QFileInfo::exists( backupFile ) )
                return true;

            bool aOK = false;
            if ( useTrash )
                aOK = NFileUtils::moveToTrash( backupFile, msg );
            else
                aOK = NFileUtils::removePath( backupFile, msg );
            return aOK;
        }

        bool backup( const QString &fileName, const QString &format /*= "%FN.bak"*/, QString *msg /*= nullptr*/, bool keepBackups /*= true*/, bool useTrash /*= false*/, EMoveOrCopy moveOrCopyFile /*= EMoveOrCopy::eMove */ )
        {
            if ( !QFileInfo( fileName ).exists() )
                return true;

            auto backupFile = format;
            if ( backupFile.isEmpty() )
                backupFile = "%FN.bak"; // filename(num).bak

            auto pos = backupFile.indexOf( "%FN" );
            if ( pos != -1 )
            {
                backupFile = backupFile.replace( pos, 3, fileName );
            }

            pos = backupFile.indexOf( "%TS" );
            if ( pos != -1 )
            {
                backupFile = backupFile.replace( pos, 3, QDateTime::currentDateTime().toString( "ddMMyyyy_hhmmss.zzz" ) );
            }

            if ( backupFile == fileName )
                backupFile = fileName + ".bak"; // someones is trying to backup the backup
            auto backupFileTemplate = backupFile;

            // backup already exists, move it out of the way
            auto backupFI = QFileInfo( backupFile );
            if ( backupFI.exists() )
            {
                if ( keepBackups )
                {
                    auto backupNum = 1;
                    while ( backupFI.exists() )
                    {
                        auto baseName = QFileInfo( backupFileTemplate ).baseName();
                        auto suffix = QFileInfo( backupFileTemplate ).suffix();
                        backupFile = QString( "%1 (%2).%3" ).arg( baseName ).arg( backupNum++ ).arg( suffix );
                        backupFI = QFileInfo( backupFile );
                    }
                }
                else
                {
                    bool aOK = removeFile( backupFile, useTrash, msg );
                    if ( !aOK )
                        return false;
                }
            }
            Q_ASSERT( !backupFI.exists() );

            auto fi = QFile( fileName );
            bool retVal = false;
            if ( moveOrCopyFile == EMoveOrCopy::eMove )
                retVal = fi.rename( backupFile );
            else
                retVal = fi.copy( backupFile );
            if ( !retVal && msg )
                *msg = fi.errorString();
            return retVal;
        }
    }
}
