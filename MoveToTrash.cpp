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
        bool moveToTrashImpl( const QString &path, std::shared_ptr< SRecycleOptions > options );
        bool moveToTrash( const QFileInfo &info, std::shared_ptr< SRecycleOptions > options )
        {
            return moveToTrash( info.absoluteFilePath(), options );
        }
        bool moveToTrash( const std::string &path, std::shared_ptr< SRecycleOptions > options )
        {
            return moveToTrash( QString::fromStdString( path ), options );
        }
        bool moveToTrash( const QString &path, std::shared_ptr< SRecycleOptions > options )
        {
            QFileInfo fi( path );
            auto nativePath = QDir::toNativeSeparators( fi.absoluteFilePath() );
            if ( options->fVerbose )
                std::cout << "Starting Recycle of '" << nativePath.toStdString() << "'." << std::endl;

            if ( !moveToTrashImpl( nativePath, options ) )
            {
                std::cerr << "Could not move '" << path.toStdString() << "' to the recycle bin." << std::endl;
                if ( options->fDeleteOnRecycleFailure )
                    return QFile::remove( path );
                else
                    return false;
            }
            return true;
        }
    }
}
