// The MIT License( MIT )
//
// Copyright( c ) 2020-2022 Scott Aron Bloom
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

#include "MKVUtils.h"
#include "MediaInfo.h"

#include "SABUtilsResources.h"

#include <QObject>
#include <QFileInfo>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QTemporaryFile>

namespace NSABUtils
{
    bool setMediaTags( const QString &fileName, const std::unordered_map< EMediaTags, QString > &newTagValues, const QString &mkvPropEdit, QString *msg /*=nullptr */ )
    {
        initResources();
        if ( !QFileInfo( fileName ).isFile() )
            return true;

        if ( !QFileInfo( mkvPropEdit ).isExecutable() )
        {
            if ( msg )
                *msg = QObject::tr( "MKVPropEdit not found or is not an executable" );
            return false;
        }

        auto file = QFile( ":/SABUtilsResources/BlankMKVTags.xml" );
        if ( !file.open( QFile::ReadOnly ) )
        {
            if ( msg )
                *msg = QObject::tr( "Internal error, could not open blank tags file" );
            return false;
        }

        auto mediaInfo = CMediaInfo( fileName );
        if ( !mediaInfo.aOK() )
        {
            if ( msg )
                *msg = QObject::tr( "Internal error, Could not load current values" );
            return false;
        }
        auto currentValues = mediaInfo.getSettableMediaTags();
        for ( auto &&ii : newTagValues )
            currentValues[ ii.first ] = ii.second;

        std::unordered_map< QString, QString > stringBasedTags;
        for ( auto &&ii : currentValues )
        {
            auto name = getMKVEditName( ii.first );
            if ( name.isEmpty() )
            {
                if ( msg )
                    *msg = QObject::tr( "Tag %1 is invalid" ).arg( displayName( ii.first ) );
                return false;
            }
            stringBasedTags[ name ] = ii.second;
        }

        auto newTitle = QString();
        auto pos = currentValues.find( EMediaTags::eTitle );
        if ( pos != currentValues.end() )
            newTitle = ( *pos ).second;

        if ( newTitle.isEmpty() )
            newTitle = QFileInfo( fileName ).baseName();

        auto xml = file.readAll();
        for ( auto &&ii : stringBasedTags )
        {
            xml.replace( ( "%" + ii.first + "%" ).toUtf8(), ii.second.toUtf8() );
        }

        auto templateName = QDir( QDir::tempPath() ).absoluteFilePath( "XXXXXX.xml" );
        QTemporaryFile tmpFile( templateName );
        auto tmplate = tmpFile.fileTemplate();
        if ( !tmpFile.open() )
        {
            if ( msg )
                *msg = QObject::tr( "Internal error, could not open blank tags file" );
            return false;
        }

        tmpFile.write( xml );
        auto tmpFileName = tmpFile.fileName();
        tmpFile.close();

        auto args = QStringList() << fileName << "--tags" << QString( "global:%1" ).arg( tmpFileName ) << "--edit"
                                  << "info"
                                  << "--set" << QString( "title=%2" ).arg( newTitle );
        auto retVal = QProcess::execute( mkvPropEdit, args );

        if ( retVal == -1 )
        {
            if ( msg )
                *msg = "MKVPropEdit crashed";
        }
        else if ( retVal == -2 )
        {
            if ( msg )
                *msg = "MKVPropEdit could not be started";
        }
        else if ( retVal != 0 )
        {
            if ( msg )
                *msg = "MKVPropEdit returned with an unknown error";
        }

        return retVal == 0;
    }
    
        std::vector< double > getChapterStarts( const QString &fileName, const QString &ffprobeExe, QString &msg )
    {
        auto args = QStringList() << "-i" << QString( "file:\"%1\"" ).arg( fileName ) << "-threads"
                                  << "0"
                                  << "-v"
                                  << "info"
                                  << "-print_format"
                                  << "json"
                                  << "-show_chapters";

        QProcess process;
        process.setNativeArguments( args.join( " " ) );
        //process.setProcessChannelMode( QProcess::MergedChannels );
        process.start( ffprobeExe, QStringList() );
        if ( !process.waitForFinished( -1 ) || ( process.exitStatus() != QProcess::NormalExit ) || ( process.exitCode() != 0 ) )
        {
            auto out = process.readAllStandardOutput();
            auto err = process.readAllStandardError();
            msg = QString( "Error running ffprobe '%1' - " ).arg( ffprobeExe ).arg( QString( err ) );
            return {};
        }
        auto data = process.readAll();

        auto doc = QJsonDocument::fromJson( data );
        if ( !doc.object().contains( "chapters" ) )
            return {};

        auto chapters = doc[ "chapters" ].toArray();
        std::vector< double > retVal;
        for ( auto &&chapter : chapters )
        {
            auto curr = chapter.toObject();
            qDebug().noquote().noquote() << QJsonDocument( curr ).toJson( QJsonDocument::Indented );
            if ( retVal.empty() )
            {
                auto startTime = curr[ "start_time" ].toVariant().toDouble();
                retVal.push_back( startTime );
            }
            auto endTime = curr[ "end_time" ].toVariant().toDouble();
            retVal.push_back( endTime );
        }

        return retVal;
    }
}