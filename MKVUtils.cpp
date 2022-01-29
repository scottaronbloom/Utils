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
#include "utils.h"

#include <QFileInfo>
#include <QString>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QTemporaryFile>

#define UNICODE
#include "MediaInfoDLL/MediaInfoDLL_Static.h"
#include "MediaInfo/MediaInfoList.h"

static void initResources()
{
    Q_INIT_RESOURCE( SABUtils );
}

std::unordered_map< QString, QString > knownMKVTags()
{
    static std::unordered_map< QString, QString > sKnownTags =
    {
         //{ "ALBUM", "" }
        //,{ "ALBUM_ARTIST", "" }
         { "ARTIST", "" }
        ,{ "BPM", "" }
        ,{ "COMMENT", "" }
        ,{ "COMPOSER", "" }
        //,{ "DISC_NUMBER", "" }
        ,{ "GENRE", "" }
        ,{ "TITLE", "" }
        ,{ "TRACK", "" }
        ,{ "DATE_RECORDED", "" }
    };
    return sKnownTags;
}

std::unordered_map< QString, QString > getMediaInfo( const QString & path )
{
        // Create Options for MediaInfo, there is a bunch more but these seem somewhat most relevant
    QStringList generalParams;
    generalParams 
        << "FileName"
        << "Duration"
        << "Title"
        << "DATE_RECORDED"
        << "Comment"
        << "BPM" 
        << "ARTIST" 
        << "COMPOSER" 
        << "GENRE"
        << "Track/Position"
        ;

    //generalParams << "FileName"
    //    << "BPM" << "Comment" << "ARTIST" << "COMPOSER" << "DATE_RECORDED" << "GENRE"
    //    << "Duration" << "Title" << "Track/Position"
    //    ;

    //auto known = knownMKVTags();
    //for ( auto && ii : known )
    //{
    //    generalParams << ii.first;
    //}

        //<< "Album"
        //<< "Director"
        //<< "Arranger"
        //<< "Performer"
        //<< "Performer"
        //<< "Conductor"
        //<< "Comment"
        //<< "Copyright"
        //<< "Recorded_Date"
        //<< "Title_More"
        //<< "Director"
        //<< "TermsOfUse"
        //<< "Tagged_Date"
        //<< "Encoded_Application"
        //<< "Origin"
        //<< "Genre"
        //<< "Grouping"
        //<< "HostComputer"
        //<< "Title_More"
        //<< "Keywords"
        //<< "Lyrics"
        //<< "Make"
        //<< "Model"
        //<< "Title"
        //<< "Original/Performer"
        //<< "Producer"
        //<< "Product"
        //<< "Performer"
        //<< "Comment"
        //<< "SoundEngineer"
        //<< "Conductor"
        //<< "DistributedBy"
        //<< "Subtitle"
        //<< "Encoded_Application"
        //<< "Encoded_Application"
        //<< "Track/Url"
        //<< "Warning"
        //<< "Composer"
        //<< "ExecutiveProducer"
        //<< "Album/Performer"
        //<< "AppleStoreAccountType"
        //<< "Album"
        //<< "AppleStoreAccount"
        //<< "AlbumTitleID"
        //<< "Performer"
        //<< "Category"
        //<< "AppleStoreCatalogID"
        //<< "Compilation"
        //<< "Copyright"
        //<< "Description"
        //<< "Part"
        //<< "Title_More"
        //<< "EpisodeGlobalUniqueID"
        //<< "Flavour"
        //<< "Genre"
        //<< "GenreID"
        //<< "Grouping"
        //<< "HDVideo"
        //<< "iTunesU"
        //<< "Keyword"
        //<< "LongDescription"
        //<< "Title"
        //<< "Podcast"
        //<< "Performer"
        //<< "PlayListID"
        //<< "PurchaseDate"
        //<< "PodcastURL"
        //<< "Rating"
        //<< "Rating"
        //<< "Description"
        //<< "AppleStoreCountry"
        //<< "Album/Performer/Sort"
        //<< "Album/Sort"
        //<< "Performer/Sort"
        //<< "Composer/Sort"
        //<< "Title/Sort"
        //<< "Title/Sort"
        //<< "ContentType"
        //<< "Title"
        //<< "Encoded_Application"
        //<< "BPM"
        //<< "Track"
        //<< "Part_ID"
        //<< "Part"
        //<< "TVNetworkName"
        //<< "Collection"
        //<< "Season"
        //<< "Vendor"
        //<< "Recorded_Date"
        //<< "Recorded_Date"
        //;
    QString generalInform = QStringLiteral( "General;" ) + "%" + generalParams.join( "%|%" ) + "%";
    generalInform += "\\n";
    
    MediaInfoDLL::MediaInfo MI;
    MI.Option( __T( "ParseSpeed" ), __T( "0" ) );
    MI.Option( __T( "Language" ), __T( "raw" ) );
    MI.Option( __T( "ReadByHuman" ), __T( "0" ) );
    MI.Option( __T( "Legacy" ), __T( "0" ) );

    // This causes MediaInfo to open all files in the directory
    auto nFiles = MI.Open( path.toStdWString() /*, MediaInfoLib::FileOption_NoRecursive*/ );
    if ( nFiles != 1 )
        return {};
    // Now we query MediaInfoLib for the data we are interested and receive everything in one string
    MI.Option( QStringLiteral( "Inform" ).toStdWString(), generalInform.toStdWString() );
    QString informOptionResult = QString::fromStdWString( MI.Inform() );
    // Done - be good and close the MediaInfo object
    MI.Close();

    std::unordered_map< QString, QString > retVal;
    QStringList informResult = informOptionResult.split( '\n', QString::SkipEmptyParts );
    foreach( QString res, informResult )
    {
        QStringList resList = res.split( "|" );
        Q_ASSERT( resList.count() == generalParams.count() );
        for ( int i = 0; i < resList.count(); ++i )
        {
            retVal[ generalParams[i].toUpper() ] = resList[i];
        }
    }

    auto pos = retVal.find( "DURATION" );
    if ( pos != retVal.end() )
    {
        bool aOK;
        uint64_t numMSecs = (*pos).second.toInt( &aOK );
        if ( !aOK )
            numMSecs = 0;

        NSABUtils::CTimeString ts( numMSecs );
        retVal["LENGTH"] = ts.toString( "hh:mm:ss" );
    }
    return retVal;
}


namespace NSABUtils
{
    int64_t getNumberOfSeconds( const QString & fileName )
    {
        return getNumberOfMSecs( fileName ) / 1000;
    }

    int64_t getNumberOfMSecs( const QString & fileName )
    {
        auto values = getMediaInfo( fileName );
        auto pos = values.find( "Duration" );
        if ( pos == values.end() )
            return 0;
        bool aOK;
        int retVal = (*pos).second.toInt( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
    }

    std::unordered_map< QString, QString > getMediaTags( const QString & fileName )
    {
        return getMediaInfo( fileName );
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

        auto knownTags = knownMKVTags();

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
}