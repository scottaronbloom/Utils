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

#include <QFileInfo>
#include <QString>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QTemporaryFile>


static void initResources()
{
    Q_INIT_RESOURCE( SABUtils );
}

namespace NSABUtils
{
    int64_t getNumberOfSeconds( const QString & fileName, const QString & ffprobe )
    {
        QFileInfo fi = QFileInfo( ffprobe );
        if ( !fi.exists() || !fi.isReadable() || !fi.isExecutable() || !fi.isFile() )
            return 0;

        fi = QFileInfo( fileName );
        if ( !fi.exists() || !fi.isReadable() || !fi.isFile() )
            return 0;
        
        auto args = QStringList()
            << "-v" << "error"
            << "-show_entries" << "format=duration"
            << "-of" << "default=noprint_wrappers=1:nokey=1"
            << fileName
            ;

        QProcess process;
        process.start( ffprobe, args );

        if ( !process.waitForFinished( -1 ) || (process.exitStatus() != QProcess::NormalExit) || (process.exitCode() != 0) )
        {
            return 0;
        }
        auto out = process.readAllStandardOutput();
        auto pos = out.indexOf( '.' );
        out = out.left( pos );
        bool aOK;
        int retVal = out.toInt( &aOK );
        if ( !aOK )
            return 0;
        return retVal;
    }

    bool setMediaTags( const QString & fileName, const std::unordered_map< QString, QString > & tags, const QString & mkvPropEdit, QString * msg/*=nullptr */ )
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

        std::unordered_map< QString, QString > knownTags =
        {
             { "ALBUM", "" }
            ,{"ALBUM_ARTIST", "" }
            ,{ "ARTIST", "" }
            ,{ "BPM", "" }
            ,{ "COMMENT", "" }
            ,{ "COMPOSER", "" }
            ,{ "DISC_NUMBER", "" }
            ,{ "GENRE", "" }
            ,{ "TITLE", "" }
            ,{ "TRACK", "" }
            ,{ "YEAR", "" }
        };

        for ( auto && ii : tags )
        {
            auto pos = knownTags.find( ii.first.toUpper() );
            if ( pos == knownTags.end() )
            {
                if ( msg )
                    *msg = QObject::tr( "Tag %1 is invalid" ).arg( ii.first );
            }
            else
                knownTags[ii.first] = ii.second;
        }

        auto title = knownTags["TITLE"];
        if ( title.isEmpty() )
            title = QFileInfo( fileName ).baseName();

        auto xml = file.readAll();
        for ( auto && ii : knownTags )
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

        auto args = QStringList()
            << fileName
            << "--tags"
            << QString( "global:%1" ).arg( tmpFileName )
            << "--edit"
            << "info"
            << "--set"
            << QString( "title=%2" ).arg( title )
            ;
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

    std::unordered_map< QString, QString > getMediaTags( const QString & fileName, const QString & ffprobe )
    {
        QFileInfo fi = QFileInfo( ffprobe );
        if ( !fi.exists() || !fi.isReadable() || !fi.isExecutable() || !fi.isFile() )
            return {};

        fi = QFileInfo( fileName );
        if ( !fi.exists() || !fi.isReadable() || !fi.isFile() )
            return {};

        auto args = QStringList()
            << "-v" << "error"
            << "-print_format" << "json"
            << "-show_format" 
            << fileName
            ;

        QProcess process;
        process.start( ffprobe, args );

        if ( !process.waitForFinished( -1 ) || (process.exitStatus() != QProcess::NormalExit) || (process.exitCode() != 0) )
        {
            return {};
        }
        auto out = process.readAllStandardOutput();

        auto doc = QJsonDocument::fromJson( out );
        if ( doc.isNull() )
            return {};
        if ( !doc.object().contains( "format" ) )
            return {};
        auto format = doc.object()["format"];
        if ( format.isNull() || !format.isObject() )
            return {};

        if ( !format.toObject().contains( "tags" ) )
            return {};

        auto tags = format.toObject()["tags"];
        if ( !tags.isObject() )
            return {};

        std::unordered_map< QString, QString > retVal;
        auto tagsObj = tags.toObject();
        for ( auto && ii = tagsObj.begin(); ii != tagsObj.end(); ++ii )
        {
            auto key = ii.key().toLower();
            auto variant = ii.value();
            QString value;
            if ( variant.isString() )
                value = variant.toString();
            else if ( variant.isBool() )
                value = variant.toBool() ? "true" : "false";
            else if ( variant.isDouble() )
                value = QString::number( variant.toDouble() );
            else if ( variant.isNull() )
                value = "";
            retVal[key] = value;
        }

        return retVal;
    }
}