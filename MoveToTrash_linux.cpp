// The MIT License( MIT )
//
// Copyright( c ) 2022 Scott Aron Bloom
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

#include <QString>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#include <QUrl>

namespace NFileUtils
{
    bool moveToTrashImpl( const QString & fileName, QString * msg, std::shared_ptr< SRecycleOptions > options )
    {
        QFileInfo fi( fileName );
        if ( !fi.exists() )
        {
            if ( msg )
                *msg = QObject::tr( "File or Directory '%1' does not exist." ).arg( fileName );

            return true;
        }

        auto currentTime = QDateTime::currentDateTime();

        QString trashFilePath = QDir::homePath() + "/.local/share/Trash/files/";    // trash file path contain delete files
        QString trashInfoPath = QDir::homePath() + "/.local/share/Trash/info/";     // trash info path contain delete files information

        if ( !NFileUtils::mkdir( trashFilePath.toStdString(), true ) )
        {
            if ( msg )
                *msg = QString::tr( "Could not make directory '%1'" ).arg( trashFilePath );
            return false;
        }
        if ( !NFileUtils::mkdir( trashInfoPath.toStdString(), true ) )
        {
            if ( msg )
                *msg = QString::tr( "Could not make directory '%1'" ).arg( trashInfoPath );
            return false;
        }

        // create file format for trash info file----- START
        QFile infoFile( trashInfoPath + fi.fileName() + ".trashinfo" );     //filename+extension+.trashinfo //  create file information file in /.local/share/Trash/info/ folder


        if ( !infoFile.open( QIODevice::WriteOnly ) || !infoFile.isOpen() )
        {
            if ( msg )
                *msg = QString::tr( "Could not open file '%1' for write" ).arg( infoFile.absoluteFilePath() );
            return false;
        }


        QTextStream stream( &infoFile );         // for write data on open file

        stream << "[Trash Info]" << "\n";
        stream << "Path=" + QString( QUrl::toPercentEncoding( fi.absoluteFilePath(), "~_-./" ) ) << "\n";     // convert path string in percentage decoding scheme string
        stream << "DeletionDate=" + currentTime.toString( "yyyy-MM-dd" ) + "T" + currentTime.toString( "hh:mm:ss" ) << "\n";      // get date and time format YYYY-MM-DDThh:mm:ss

        infoFile.close();

        // create info file format of trash file----- END

        QDir file;
        auto retVal = file.rename( fi.absoluteFilePath(), trashFilePath + fi.fileName() );  // rename(file old path, file trash path)
        if ( !retVal && msg )
            *msg = QObject::tr( "Could not rename file '%1' to '%2'" ).arg( fi.absoluteFilePath() ).arg( trashFilePath + fi.fileName() );
        return retVal;
    }
}


