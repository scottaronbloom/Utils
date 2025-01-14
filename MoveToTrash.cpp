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

#include "MoveToTrash.h"

#include <QFileInfo>
#include <QDir>

#include <iostream>

namespace NSABUtils
{
    namespace NFileUtils
    {
        bool moveToTrash( const QFileInfo &info, QString *msg, std::shared_ptr< SRecycleOptions > options )
        {
            return moveToTrash( info.absoluteFilePath(), msg, options );
        }

        bool moveToTrash( const std::string &path, std::string *msg, std::shared_ptr< SRecycleOptions > options )
        {
            QString lclMsg;
            bool retVal = moveToTrash( QString::fromStdString( path ), &lclMsg, options );
            if ( msg )
                *msg = lclMsg.toStdString();
            return retVal;
        }

        bool moveToTrashImpl( const QString &path, QString *msg, std::shared_ptr< SRecycleOptions > options );
        bool moveToTrash( const QString &path, QString *msg, std::shared_ptr< SRecycleOptions > options )
        {
            QFileInfo fi( path );
            auto nativePath = QDir::toNativeSeparators( fi.absoluteFilePath() );
            if ( options->fVerbose )
                std::cout << "Starting Recycle of '" << nativePath.toStdString() << "'." << std::endl;

            bool aOK = moveToTrashImpl( nativePath, msg, options );
            if ( !aOK )
            {
                if ( msg )
                    *msg = QObject::tr( "Could not move '%1' to the recycle bin." ).arg( path );
                if ( options->fDeleteOnRecycleFailure )
                {
                    aOK = QFile::remove( path );
                    if ( !aOK )
                        *msg += QObject::tr( "\nCould not remove '%1'." ).arg( path );
                }
            }
            return aOK;
        }
    }
}
