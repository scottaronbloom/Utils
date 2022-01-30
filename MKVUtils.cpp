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
#include <iosfwd>
#include <iomanip>

static void initResources()
{
    Q_INIT_RESOURCE( SABUtils );
}

//QString toString( MediaInfoDLL::stream_t whichStream )
//{
//    switch( whichStream )
//    {
//        case MediaInfoDLL::Stream_General: return "General";
//        case MediaInfoDLL::Stream_Video: return "Video";
//        case MediaInfoDLL::Stream_Audio: return "Audio";
//        case MediaInfoDLL::Stream_Text: return "Text";
//        case MediaInfoDLL::Stream_Other: return "Other";
//        case MediaInfoDLL::Stream_Image: return "Image";
//        case MediaInfoDLL::Stream_Menu: return "Menu";
//        default:
//            return QString();
//    }
//}

//QString test( const QString & fileName )
//{
//        //Information about MediaInfo
//    MediaInfoDLL::MediaInfo MI;
//    QString retVal = QString::fromStdWString( MI.Option( __T( "Info_Version" ), __T( "0.7.13;MediaInfoDLL_Example_MSVC;0.7.13" ) ) );
//
//    //retVal += __T( "\n\nInfo_Parameters\n" );
//    //retVal += MI.Option( __T( "Info_Parameters" ) );
//
//    //retVal += __T( "\n\nInfo_Codecs\n" );
//    //retVal += MI.Option( __T( "Info_Codecs" ) );
//
//    ////An example of how to use the library
//    //retVal += __T( "\n\nOpen\n" );
//    MI.Open( fileName.toStdWString() );
//
//    //retVal += __T( "\n\nInform with Complete=false\n" );
//    //MI.Option( __T( "Complete" ) );
//    //retVal += MI.Inform();
//
//    //retVal += __T( "\n\nInform with Complete=true\n" );
//    //MI.Option( __T( "Complete" ), __T( "1" ) );
//    //retVal += MI.Inform();
//
//    //retVal += __T( "\n\nCustom Inform\n" );
//    //MI.Option( __T( "Inform" ), __T( "General;Example : FileSize=%FileSize%" ) );
//    //retVal += MI.Inform();
//
//    //retVal += __T( "\n\nGet with Stream=General and Parameter=\"FileSize\"\n" );
//    //retVal += MI.Get( MediaInfoDLL::Stream_General, 0, __T( "FileSize" ), MediaInfoDLL::Info_Text, MediaInfoDLL::Info_Name );
//
//    for ( MediaInfoDLL::stream_t ii = MediaInfoDLL::Stream_General; ii < MediaInfoDLL::Stream_Max; ii = static_cast<MediaInfoDLL::stream_t>( static_cast< int >( ii ) + 1 ) )
//    {
//        retVal += "===========================================\n";
//        retVal += "Stream: " + toString( ii ) + "\n";
//        retVal += "===========================================\n";
//        bool aOK;
//        auto streamCount = QString::fromStdWString( MI.Get( ii, 0, __T( "StreamCount" ) ) ).toInt( &aOK );
//        if ( !aOK )
//            streamCount = 1;
//        for ( int jj = 0; jj < streamCount; ++jj )
//        {
//            retVal += "===========================================\n";
//            retVal += QString( "Stream #%1 of %2\n" ).arg( jj ).arg( streamCount );
//            retVal += "===========================================\n";
//            auto paramCount = QString::fromStdWString( MI.Get( ii, jj, __T( "Count" ) ) ).toInt( &aOK );
//            if ( !aOK )
//                paramCount = 0;
//            for ( int kk = 0; kk < paramCount; ++kk )
//            {
//                auto curr = QString( "Param #%1 of %2 = %3" ).arg( kk ).arg( paramCount ).arg( MI.Get( ii, jj, kk ) );
//                qDebug() << curr;
//                retVal += curr + "\n";
//            }
//        }
//    }
//
//
////    retVal += __T( "\n\nCount_Get with StreamKind=Stream_Audio\n" );
////#ifdef __MINGW32__
////    Char * C1 = new Char[33];
////    _itot( MI.Count_Get( Stream_Audio ), C1, 10 );
////    retVal += C1;
////    delete[] C1;
////#else
////    std::wstringstream SS;
////    SS << std::setbase( 10 ) << MI.Count_Get( MediaInfoDLL::Stream_Audio );
////    retVal += SS.str();
////#endif
//
//    //retVal += __T( "\n\nGet with Stream=General and Parameter=\"AudioCount\"\n" );
//    //retVal += MI.Get( MediaInfoDLL::Stream_General, 0, __T( "AudioCount" ), MediaInfoDLL::Info_Text, MediaInfoDLL::Info_Name );
//
//    //retVal += __T( "\n\nGet with Stream=Audio and Parameter=\"StreamCount\"\n" );
//    //retVal += MI.Get( MediaInfoDLL::Stream_Audio, 0, __T( "StreamCount" ), MediaInfoDLL::Info_Text, MediaInfoDLL::Info_Name );
//
//    //retVal += __T( "\n\nClose\n" );
//    //MI.Close();
//
//    return retVal;
//}

namespace NSABUtils
{
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

    std::unordered_map< NSABUtils::EMediaTags, QString > getMediaTags( const QString & path, const std::list< NSABUtils::EMediaTags > & tags )
    {
        getMediaInfo( path );

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
        QStringList generalParams;
        for ( auto && ii : realTags )
        {
            generalParams << mediaInfoName( ii );
        }

        auto generalInform = QString( "General;" ) + "%" + generalParams.join( "%|%" ) + "%\\n";
        MI.Option( QStringLiteral( "Inform" ).toStdWString(), generalInform.toStdWString() );
        auto informOptionResult = QString::fromStdWString( MI.Inform() );

        std::unordered_map< NSABUtils::EMediaTags, QString > retVal;
        QStringList informResult = informOptionResult.split( '\n', QString::SkipEmptyParts );
        for( auto && ii : informResult )
        {
            auto resList = ii.split( "|" );
            Q_ASSERT( resList.count() == static_cast< int >(realTags.size() ) );
            auto jj = realTags.begin();
            auto kk = resList.begin();
            for( ; jj != realTags.end() && kk != resList.end(); ++jj, ++kk )
            {
                retVal[*jj] = *kk;
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


    bool setMediaTags( const QString & fileName, const std::unordered_map< EMediaTags, QString > & tags, const QString & mkvPropEdit, QString * msg/*=nullptr */ )
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

        std::unordered_map< QString, QString > realTags;
        for ( auto && ii : tags )
        {
            auto name = setMKVName( ii.first );
            if ( name.isEmpty() )
            {
                if ( msg )
                    *msg = QObject::tr( "Tag %1 is invalid" ).arg( displayName( ii.first ) );
                return false;
            }
            realTags[ name ] = ii.second;
        }
        if ( tags.find( EMediaTags::eTitle ) == tags.end() )
            realTags[ setMKVName( EMediaTags::eTitle ) ] = QFileInfo( fileName ).baseName();

        auto xml = file.readAll();
        for ( auto && ii : realTags )
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
            << QString( "title=%2" ).arg( realTags[ setMKVName( EMediaTags::eTitle ) ] )
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