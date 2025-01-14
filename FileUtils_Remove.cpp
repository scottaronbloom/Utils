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

#include <QFileInfo>
#include <QDir>

namespace NSABUtils
{
    namespace NFileUtils
    {
        bool removePath( const std::string &path, std::string *msg )
        {
            QString lclMsg;
            auto retVal = removePath( QString::fromStdString( path ), &lclMsg );
            if ( !retVal && msg )
                *msg = lclMsg.toStdString();
            return retVal;
        }

        bool removePath( const QString &path, QString *msg )
        {
            QFileInfo fi( path );
            if ( !fi.exists() )
                return true;

            bool success = false;
            if ( fi.isFile() )
            {
                auto file = QFile( fi.absoluteFilePath() );
                success = file.remove();
                if ( !success && msg )
                    *msg = QObject::tr( "Could not remove file '%1'" ).arg( path );
            }
            else if ( fi.isDir() )
            {
                QDir dir( path );
                success = dir.removeRecursively();
                if ( !success && msg )
                    *msg = QObject::tr( "Could remove directory (recursively) '%1'" ).arg( path );
            }
            return success;
        }

        bool removeInsideOfDir( const QString &dirStr, QString *msg )
        {
            QDir dir( dirStr );
            auto retVal = dir.removeRecursively();
            if ( !retVal && msg )
                *msg = QObject::tr( "Could not remove '%1' recursively" ).arg( dirStr );
            return retVal;
        }

        bool removeInsideOfDir( const std::string &dir, std::string *msg )
        {
            QString lclMsg;
            bool retVal = removeInsideOfDir( QString::fromStdString( dir ), &lclMsg );
            if ( msg )
                *msg = lclMsg.toStdString();
            return retVal;
        }
    }
}
