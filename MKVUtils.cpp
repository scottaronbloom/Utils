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
#include "MediaInfoDLL/MediaInfoDLL_Static.h"
#include "MediaInfo/MediaInfoList.h"
#include "SABUtilsResources.h"
#include <iosfwd>
#include <iomanip>

namespace NSABUtils
{

    QString toString( MediaInfoDLL::stream_t whichStream )
    {
        switch ( whichStream )
        {
            case MediaInfoDLL::Stream_General: return "General";
            case MediaInfoDLL::Stream_Video: return "Video";
            case MediaInfoDLL::Stream_Audio: return "Audio";
            case MediaInfoDLL::Stream_Text: return "Text";
            case MediaInfoDLL::Stream_Other: return "Other";
            case MediaInfoDLL::Stream_Image: return "Image";
            case MediaInfoDLL::Stream_Menu: return "Menu";
            default:
                return {};
        }
    }


    SAllMediaInfo getAllMediaInfo( const QString & fileName )
    {
        SAllMediaInfo retVal;

        MediaInfoDLL::MediaInfo MI;
        retVal.fVersion = QString::fromStdWString( MI.Option( __T( "Info_Version" ), __T( "0.7.13;MediaInfoDLL_Example_MSVC;0.7.13" ) ) );

        MI.Open( fileName.toStdWString() );

        for ( MediaInfoDLL::stream_t ii = MediaInfoDLL::Stream_General; ii < MediaInfoDLL::Stream_Max; ii = static_cast<MediaInfoDLL::stream_t>(static_cast<int>(ii) + 1) )
        {
            auto streamCount = MI.Count_Get( ii );
            for ( int jj = 0; jj < streamCount; ++jj )
            {
                auto streamName = QString::fromStdWString( MI.Get( ii, jj, __T( "StreamKind" ) ) );
                SStreamData data( ii, streamName, jj );
                auto paramCount = MI.Count_Get( ii, jj );
                for ( int kk = 0; kk < paramCount; ++kk )
                {
                    auto name = QString::fromStdWString( MI.Get( ii, jj, kk, MediaInfoDLL::Info_Name ) );
                    auto value = QString::fromStdWString( MI.Get( ii, jj, kk ) );
                    data.fStreamData.emplace_back( std::make_pair( name, value ) );
                    data.fStreamDataMap[name] = value;
                }
                retVal.fData.emplace_back( data );
            }
        }
        MI.Close();
        return retVal;
    }

    int64_t getNumberOfSeconds( const QString & fileName )
    {
        auto value = getMediaTag( fileName, EMediaTags::eLengthS );
        bool aOK;
        int retVal = value.toInt( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
    }

    int64_t getNumberOfMSecs( const QString & fileName )
    {
        auto value = getMediaTag( fileName, EMediaTags::eLengthS );
        bool aOK;
        int retVal = value.toInt( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
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
                retVal[generalParams[i].toUpper()] = resList[i];
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

    void updateLengths( std::unordered_map<NSABUtils::EMediaTags, QString> & retVal )
    {
        auto pos = retVal.find( EMediaTags::eLengthS );
        if ( pos != retVal.end() )
        {
            bool aOK;
            uint64_t numMSecs = (*pos).second.toInt( &aOK );
            if ( !aOK )
                numMSecs = 0;
            auto numSecs = numMSecs / 1000;
            (*pos).second = QString::number( numMSecs );
        }
        pos = retVal.find( EMediaTags::eLength );
        if ( pos != retVal.end() )
        {
            bool aOK;
            uint64_t numMSecs = (*pos).second.toInt( &aOK );
            if ( !aOK )
                numMSecs = 0;

            NSABUtils::CTimeString ts( numMSecs );
            (*pos).second = ts.toString( "hh:mm:ss" );
        }
    }

    QString getMediaTag( const QString & path, NSABUtils::EMediaTags tag )
    {
        auto tmp = getMediaTags( path, { tag } );
        auto pos = tmp.find( tag );
        if ( pos == tmp.end() )
            return {};
        return (*pos).second;
    }

    std::unordered_map< NSABUtils::EMediaTags, QString > getSettableMediaTags( const QString & path )
    {
        return getMediaTags( path,
                             {
                                 EMediaTags::eTitle,
                                 EMediaTags::eDate,
                                 EMediaTags::eComment,
                                 EMediaTags::eBPM,
                                 EMediaTags::eArtist,
                                 EMediaTags::eComposer,
                                 EMediaTags::eGenre,
                                 EMediaTags::eTrack,
                                 EMediaTags::eAlbum,
                                 EMediaTags::eAlbumArtist,
                                 EMediaTags::eDiscnumber
                             }
        );
    }
    std::unordered_map< NSABUtils::EMediaTags, QString > getMediaTags( const QString & path, const std::list< NSABUtils::EMediaTags > & tags )
    {
        //auto tmp = getAllMediaInfo( path );

        MediaInfoDLL::MediaInfo MI;
        MI.Option( __T( "ParseSpeed" ), __T( "0" ) );
        MI.Option( __T( "Language" ), __T( "raw" ) );
        MI.Option( __T( "ReadByHuman" ), __T( "0" ) );
        MI.Option( __T( "Legacy" ), __T( "0" ) );

        // This causes MediaInfo to open all files in the directory
        auto nFiles = MI.Open( path.toStdWString() /*, MediaInfoLib::FileOption_NoRecursive*/ );
        if ( nFiles != 1 )
            return {};

        auto realTags = tags;
        if ( tags.empty() )
        {
            realTags =
            {
                EMediaTags::eFileName,
                EMediaTags::eTitle,
                EMediaTags::eLengthMS,
                EMediaTags::eLengthS,
                EMediaTags::eLength,
                EMediaTags::eDate,
                EMediaTags::eComment,
                EMediaTags::eBPM,
                EMediaTags::eArtist,
                EMediaTags::eComposer,
                EMediaTags::eGenre,
                EMediaTags::eTrack,
                EMediaTags::eAlbum,
                EMediaTags::eAlbumArtist,
                EMediaTags::eDiscnumber
            };
        }

        std::unordered_map< NSABUtils::EMediaTags, QString > retVal;
        for ( auto && ii : realTags )
        {
            auto streamCount = MI.Count_Get( MediaInfoDLL::Stream_General );
            for ( int jj = 0; jj < streamCount; ++jj )
            {
                auto value = QString::fromStdWString( MI.Get( MediaInfoDLL::Stream_General, jj, mediaInfoName( ii ).toStdWString() ) );
                retVal[ii] = value;
                if ( !value.isEmpty() )
                    break;
            }
        }

        updateLengths( retVal );
        return retVal;
    }

    QString setMKVName( EMediaTags tag )
    {
        switch ( tag )
        {
            case EMediaTags::eTitle: return QObject::tr( "TITLE" );
            case EMediaTags::eDate: return QObject::tr( "DATE_RECORDED" );
            case EMediaTags::eComment: return QObject::tr( "COMMENT" );
            case EMediaTags::eBPM: return QObject::tr( "BPM" );
            case EMediaTags::eArtist: return QObject::tr( "ARTIST" );
            case EMediaTags::eComposer: return QObject::tr( "COMPOSER" );
            case EMediaTags::eGenre: return QObject::tr( "GENRE" );
            case EMediaTags::eTrack: return QObject::tr( "TRACK" );
            case EMediaTags::eAlbum: return QObject::tr( "ALBUM" );
            case EMediaTags::eAlbumArtist: return QObject::tr( "ALBUM_ARTIST" );
            case EMediaTags::eDiscnumber: return QObject::tr( "DISC_NUMBER" );
            default:
                return QObject::tr( "" );
        }
    }

    QString displayName( EMediaTags tag )
    {
        switch ( tag )
        {
            case EMediaTags::eFileName: return QObject::tr( "Filename" );
            case EMediaTags::eTitle: return QObject::tr( "Title" );
            case EMediaTags::eLengthMS: return QObject::tr( "Length (ms)" );
            case EMediaTags::eLengthS: return QObject::tr( "Length (secs)" );
            case EMediaTags::eLength: return QObject::tr( "Length" );
            case EMediaTags::eDate: return QObject::tr( "Media Date" );
            case EMediaTags::eComment: return QObject::tr( "Comment" );
            case EMediaTags::eBPM: return QObject::tr( "BPM" );
            case EMediaTags::eArtist: return QObject::tr( "Artist" );
            case EMediaTags::eComposer: return QObject::tr( "Composer" );
            case EMediaTags::eGenre: return QObject::tr( "Genre" );
            case EMediaTags::eTrack: return QObject::tr( "Track" );
            case EMediaTags::eAlbum: return QObject::tr( "Album" );
            case EMediaTags::eAlbumArtist: return QObject::tr( "Album Artist" );
            case EMediaTags::eDiscnumber: return QObject::tr( "Disc Number" );
            default:
                return QObject::tr( "" );
        }
    }


    EMediaTags fromDisplayName( const QString & tag )
    {
        if ( tag == QObject::tr( "Filename" ) )
            return EMediaTags::eFileName;
        if ( tag == QObject::tr( "Title" ) )
            return EMediaTags::eTitle;
        if ( tag == QObject::tr( "Length (ms)" ) )
            return EMediaTags::eLengthMS;
        if ( tag == QObject::tr( "Length (secs)" ) )
            return EMediaTags::eLengthS;
        if ( tag == QObject::tr( "Length" ) )
            return EMediaTags::eLength;
        if ( tag == QObject::tr( "Media Date" ) )
            return EMediaTags::eDate;
        if ( tag == QObject::tr( "Comment" ) )
            return EMediaTags::eComment;
        if ( tag == QObject::tr( "BPM" ) )
            return EMediaTags::eBPM;
        if ( tag == QObject::tr( "Artist" ) )
            return EMediaTags::eArtist;
        if ( tag == QObject::tr( "Composer" ) )
            return EMediaTags::eComposer;
        if ( tag == QObject::tr( "Genre" ) )
            return EMediaTags::eGenre;
        if ( tag == QObject::tr( "Track" ) )
            return EMediaTags::eTrack;
        if ( tag == QObject::tr( "Album" ) )
            return EMediaTags::eAlbum;
        if ( tag == QObject::tr( "Album Artist" ) )
            return EMediaTags::eAlbumArtist;
        if ( tag == QObject::tr( "Disc Number" ) )
            return EMediaTags::eDiscnumber;

        return EMediaTags::eFileName;
    }

    QString mediaInfoName( EMediaTags tag )
    {
        switch ( tag )
        {
            case EMediaTags::eFileName: return "Filename";
            case EMediaTags::eTitle: return "Title";
            case EMediaTags::eLengthMS: return "Duration";
            case EMediaTags::eLengthS: return "Duration";
            case EMediaTags::eLength: return "Duration";
            case EMediaTags::eDate: return "DATE_RECORDED";
            case EMediaTags::eComment: return "Comment";
            case EMediaTags::eBPM: return "BPM";
            case EMediaTags::eArtist: return "ARTIST";
            case EMediaTags::eComposer: return "COMPOSER";
            case EMediaTags::eGenre: return "GENRE";
            case EMediaTags::eTrack: return "Track/Position";
            case EMediaTags::eAlbum: return "Album";
            case EMediaTags::eAlbumArtist: return "Album Artist";
            case EMediaTags::eDiscnumber: return "Disc Number";
            default:
                return QObject::tr( "" );
        }
    }


    bool setMediaTags( const QString & fileName, const std::unordered_map< EMediaTags, QString > & newTagValues, const QString & mkvPropEdit, QString * msg/*=nullptr */ )
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

        auto currentValues = getSettableMediaTags( fileName );
        for ( auto && ii : newTagValues )
            currentValues[ii.first] = ii.second;

        std::unordered_map< QString, QString > stringBasedTags;
        for ( auto && ii : currentValues )
        {
            auto name = setMKVName( ii.first );
            if ( name.isEmpty() )
            {
                if ( msg )
                    *msg = QObject::tr( "Tag %1 is invalid" ).arg( displayName( ii.first ) );
                return false;
            }
            stringBasedTags[name] = ii.second;
        }

        auto newTitle = QString();
        auto pos = newTagValues.find( EMediaTags::eTitle );
        if ( pos != newTagValues.end() )
            newTitle = (*pos).second;

        if ( newTitle.isEmpty() )
            newTitle = QFileInfo( fileName ).baseName();

        auto xml = file.readAll();
        for ( auto && ii : stringBasedTags )
        {
            xml.replace( ("%" + ii.first + "%").toUtf8(), ii.second.toUtf8() );
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
            //<< "--set"
            //<< QString( "title=%2" ).arg( newTitle )
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

    SStreamData::SStreamData( MediaInfoDLL::stream_t type, const QString & streamName, int num ) :
        fStreamType( type ),
        fStreamName( streamName ),
        fStreamNum( num )
    {
    }

}