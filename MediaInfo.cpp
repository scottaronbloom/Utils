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

#include "MediaInfo.h"
#include "MKVUtils.h"
#include "FileBasedCache.h"
#include "FFMpegFormats.h"

#include "utils.h"
#include "FileUtils.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThreadPool>
#include <QTimer>

#include "MediaInfoDLL/MediaInfoDLL_Static.h"

namespace NSABUtils
{
    class CStreamData
    {
    public:
        CStreamData() {}
        CStreamData( MediaInfoDLL::MediaInfo *mediaInfo, EStreamType type, int num );

        void addData( const QString &name, const QString &value );

        QString value( const QString &key ) const;
        QString value( EMediaTags key ) const;

        size_t size() const { return fStreamData.size(); }
        std::pair< QString, QString > operator[]( size_t idx ) const { return fStreamData[ idx ]; }

    private:
        EStreamType fStreamType{ EStreamType::eGeneral };
        int fStreamNum{ 0 };
        std::vector< std::pair< QString, QString > > fStreamData;
        std::map< QString, QString > fStreamDataMap;
        std::map< EMediaTags, QString > fKnownTagStreamDataMap;
    };

    QString displayName( EMediaTags tag )
    {
        switch ( tag )
        {
            case EMediaTags::eFileName:
                return QObject::tr( "Filename" );
            case EMediaTags::eTitle:
                return QObject::tr( "Title" );
            case EMediaTags::eLengthMS:
                return QObject::tr( "Length (ms)" );
            case EMediaTags::eLengthS:
                return QObject::tr( "Length (secs)" );
            case EMediaTags::eLength:
                return QObject::tr( "Length" );
            case EMediaTags::eDate:
                return QObject::tr( "Media Date" );
            case EMediaTags::eComment:
                return QObject::tr( "Comment" );
            case EMediaTags::eBPM:
                return QObject::tr( "BPM" );
            case EMediaTags::eArtist:
                return QObject::tr( "Artist" );
            case EMediaTags::eComposer:
                return QObject::tr( "Composer" );
            case EMediaTags::eGenre:
                return QObject::tr( "Genre" );
            case EMediaTags::eTrack:
                return QObject::tr( "Track" );
            case EMediaTags::eAlbum:
                return QObject::tr( "Album" );
            case EMediaTags::eAlbumArtist:
                return QObject::tr( "Album Artist" );
            case EMediaTags::eDiscnumber:
                return QObject::tr( "Disc Number" );
            default:
                return QObject::tr( "" );
        }
    }

    EMediaTags fromDisplayName( const QString &tag )
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

    QString getMKVEditName( EMediaTags tag )
    {
        switch ( tag )
        {
            case EMediaTags::eTitle:
                return QObject::tr( "TITLE" );
            case EMediaTags::eDate:
                return QObject::tr( "DATE_RECORDED" );
            case EMediaTags::eComment:
                return QObject::tr( "COMMENT" );
            case EMediaTags::eBPM:
                return QObject::tr( "BPM" );
            case EMediaTags::eArtist:
                return QObject::tr( "ARTIST" );
            case EMediaTags::eComposer:
                return QObject::tr( "COMPOSER" );
            case EMediaTags::eGenre:
                return QObject::tr( "GENRE" );
            case EMediaTags::eTrack:
                return QObject::tr( "TRACK" );
            case EMediaTags::eAlbum:
                return QObject::tr( "ALBUM" );
            case EMediaTags::eAlbumArtist:
                return QObject::tr( "ALBUM_ARTIST" );
            case EMediaTags::eDiscnumber:
                return QObject::tr( "DISC_NUMBER" );
            default:
                return QObject::tr( "" );
        }
    }

    bool isSettableTag( EMediaTags tag )
    {
        switch ( tag )
        {
            case EMediaTags::eTitle:
            case EMediaTags::eDate:
            case EMediaTags::eComment:
            case EMediaTags::eBPM:
            case EMediaTags::eArtist:
            case EMediaTags::eComposer:
            case EMediaTags::eGenre:
            case EMediaTags::eTrack:
            case EMediaTags::eAlbum:
            case EMediaTags::eAlbumArtist:
            case EMediaTags::eDiscnumber:
                return true;
            default:
                return false;
        }
    }

    QString toString( MediaInfoDLL::stream_t whichStream )
    {
        switch ( whichStream )
        {
            case MediaInfoDLL::Stream_General:
                return "General";
            case MediaInfoDLL::Stream_Video:
                return "Video";
            case MediaInfoDLL::Stream_Audio:
                return "Audio";
            case MediaInfoDLL::Stream_Text:
                return "Text";
            case MediaInfoDLL::Stream_Other:
                return "Other";
            case MediaInfoDLL::Stream_Image:
                return "Image";
            case MediaInfoDLL::Stream_Menu:
                return "Menu";
            default:
                return {};
        }
    }

    MediaInfoDLL::stream_t getMediaInfoStreamType( EStreamType streamType )
    {
        switch ( streamType )
        {
            case EStreamType::eGeneral:
                return MediaInfoDLL ::stream_t::Stream_General;
            case EStreamType::eVideo:
                return MediaInfoDLL ::stream_t::Stream_Video;
            case EStreamType::eAudio:
                return MediaInfoDLL ::stream_t::Stream_Audio;
            case EStreamType::eText:
                return MediaInfoDLL ::stream_t::Stream_Text;
            case EStreamType::eOther:
                return MediaInfoDLL ::stream_t::Stream_Other;
            case EStreamType::eImage:
                return MediaInfoDLL ::stream_t::Stream_Image;
            case EStreamType::eMenu:
                return MediaInfoDLL ::stream_t::Stream_Menu;
            default:
                return MediaInfoDLL::Stream_General;
        }
    }

    CStreamData::CStreamData( MediaInfoDLL::MediaInfo *mediaInfo, EStreamType type, int num ) :
        fStreamType( type ),
        fStreamNum( num )
    {
        if ( !mediaInfo )
            return;

        auto whichStream = getMediaInfoStreamType( type );
        auto streamName = QString::fromStdWString( mediaInfo->Get( whichStream, num, __T( "StreamKind" ) ) );
        auto paramCount = mediaInfo->Count_Get( whichStream, num );
        for ( int ii = 0; ii < paramCount; ++ii )
        {
            auto name = QString::fromStdWString( mediaInfo->Get( whichStream, num, ii, MediaInfoDLL::Info_Name ) ).trimmed();
            auto value = QString::fromStdWString( mediaInfo->Get( whichStream, num, ii ) ).trimmed();
            addData( name, value );
        }
    }

    void CStreamData::addData( const QString &name, const QString &value )
    {
        fStreamData.emplace_back( std::make_pair( name, value ) );
        fStreamDataMap[ name ] = value.trimmed();
    }

    QString CStreamData::value( const QString &key ) const
    {
        auto pos = fStreamDataMap.find( key );
        if ( key == "CodecID" )
        {
            auto pos2 = fStreamDataMap.find( "FFMpegCodec" );
            if ( pos2 != fStreamDataMap.end() )
                pos = pos2;
        }
        if ( key == "Resolution" )
        {
            auto width = value( mediaInfoTagName( EMediaTags::eWidth ) );
            auto height = value( mediaInfoTagName( EMediaTags::eHeight ) );
            if ( !width.isEmpty() && !height.isEmpty() )
            {
                return QString( "%1x%2" ).arg( width ).arg( height );
            }
        }

        if ( pos == fStreamDataMap.end() )
            return {};

        return ( *pos ).second;
    }

    QString CStreamData::value( EMediaTags key ) const
    {
        auto pos = fKnownTagStreamDataMap.find( key );
        if ( pos == fKnownTagStreamDataMap.end() )
            return {};

        return ( *pos ).second;
    }

    class CMediaInfoImpl
    {
    public:
        static CFileBasedCache< std::shared_ptr< CMediaInfoImpl > > sMediaInfoCache;
        static QString sFFProbeEXE;

        static std::shared_ptr< CMediaInfoImpl > createImpl()
        {
            auto retVal = sMediaInfoCache.find( QString() );
            if ( !retVal )
            {
                static std::shared_ptr< CMediaInfoImpl > sNullImpl = std::make_shared< NSABUtils::CMediaInfoImpl >();
                retVal = sNullImpl;
                sMediaInfoCache.add( retVal );
            }
            return retVal;
        }

        static std::shared_ptr< CMediaInfoImpl > createImpl( const QFileInfo &fi, bool loadNow ) { return createImpl( fi.absoluteFilePath(), loadNow ); }

        static std::shared_ptr< CMediaInfoImpl > createImpl( const QString &path, bool loadNow )
        {
            auto retVal = sMediaInfoCache.find( path );
            if ( !retVal )
            {
                retVal = std::make_shared< NSABUtils::CMediaInfoImpl >( path );
                if ( loadNow )
                    retVal->load();
                sMediaInfoCache.add( retVal );
            }
            return retVal;
        }

        CMediaInfoImpl() {}
        CMediaInfoImpl( const QFileInfo &fi ) :
            CMediaInfoImpl( fi.absoluteFilePath() )
        {
        }

        CMediaInfoImpl( const QString &fileName ) :
            fFileName( fileName )
        {
        }

        ~CMediaInfoImpl() {}

        bool isQueued() const { return fQueued; };
        void setQueued( bool value ) { fQueued = value; }
        void queueLoad( CMediaInfo *mediaInfo )
        {
            if ( !aOK() && !isQueued() )
            {
                setQueued( true );
                QThreadPool::globalInstance()->start(
                    [ this, mediaInfo ]()
                    {
                        qDebug() << "Loading media info for" << fFileName;
                        load();
                        qDebug() << "Finished loading media info for" << fFileName;
                        mediaInfo->emitMediaInfoLoaded( fFileName );
                    } );
            }
        }

        bool load()
        {
            fAOK = initMediaInfo();
            fAOK = loadCodecsFromFFProbe() && fAOK;
            return fAOK;
        }

        bool aOK() const { return fAOK; }
        QString fileName() const { return fFileName; }
        QString version() const { return fVersion; }

        bool initMediaInfo()
        {
            auto mediaInfo = std::make_unique< MediaInfoDLL::MediaInfo >();

            fVersion = QString::fromStdWString( mediaInfo->Option( __T( "Info_Version" ), __T( "0.7.13;MediaInfoDLL_Example_MSVC;0.7.13" ) ) );
            fAOK = mediaInfo->Open( fFileName.toStdWString() ) != 0;
            if ( fAOK )
            {
                for ( auto ii = EStreamType::eGeneral; ii <= EStreamType::eMenu; ii = static_cast< EStreamType >( static_cast< int >( ii ) + 1 ) )
                {
                    fData[ ii ] = getStreamData( mediaInfo.get(), ii );
                }
            }
            mediaInfo->Close();
            return fAOK;
        }

        std::vector< std::shared_ptr< CStreamData > > getStreamData( MediaInfoDLL::MediaInfo *mediaInfo, EStreamType whichStream ) const
        {
            if ( !fAOK || !mediaInfo )
                return {};

            std::vector< std::shared_ptr< CStreamData > > retVal;

            auto mediaInfoStream = getMediaInfoStreamType( whichStream );
            auto streamCount = mediaInfo->Count_Get( mediaInfoStream );
            for ( int jj = 0; jj < streamCount; ++jj )
            {
                auto data = std::make_shared< CStreamData >( mediaInfo, whichStream, jj );
                retVal.emplace_back( data );
            }
            return retVal;
        }

        bool isAudioCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const
        {
            auto values = QStringList()   //
                          << findAllValues( EStreamType::eAudio, mediaInfoTagName( NSABUtils::EMediaTags::eAllAudioCodecs ) )   //
                          << findAllValues( EStreamType::eAudio, "Format" )   //
                          << findAllValues( EStreamType::eAudio, "InternetMediaType" )   //
                ;

            for ( auto &&value : values )
            {
                if ( isCodec( checkCodecName, value, ffmpegFormats ) )
                    return true;
            }
            return false;
        }

        bool isCodec( const QString &checkCodecName, const QString &mediaCodecName, CFFMpegFormats *ffmpegFormats ) const
        {
            Q_ASSERT( ffmpegFormats != nullptr );
            return ffmpegFormats->isCodec( checkCodecName, mediaCodecName );
        }

        bool isFormat( const QString &formatName, CFFMpegFormats *ffmpegFormats ) const
        {
            Q_ASSERT( ffmpegFormats != nullptr );
            return ffmpegFormats->isFormat( fFileName, formatName );
        }

        QString findFirstValue( EStreamType whichStream, const QString &key ) const
        {
            auto pos = fData.find( whichStream );
            if ( pos == fData.end() )
                return {};

            for ( auto &&currStream : ( *pos ).second )
            {
                auto value = currStream->value( key );
                if ( value.isEmpty() )
                    continue;
                return value;
            }
            return {};
        }

        size_t numStreams( EStreamType whichStream ) const
        {
            auto pos = fData.find( whichStream );
            if ( pos == fData.end() )
                return 0;
            return ( *pos ).second.size();
        }

        QString findFirstValue( EStreamType whichStream, EMediaTags key ) const
        {
            auto pos = fData.find( whichStream );
            if ( pos == fData.end() )
                return {};

            for ( auto &&currStream : ( *pos ).second )
            {
                auto value = currStream->value( mediaInfoTagName( key ) );
                if ( value.isEmpty() )
                    continue;
                return value;
            }
            return {};
        }

        QStringList findAllValues( EStreamType whichStream, EMediaTags key ) const { return findAllValues( whichStream, mediaInfoTagName( key ) ); }
        QStringList findAllValues( EStreamType whichStream, const QString &key ) const
        {
            auto pos = fData.find( whichStream );
            if ( pos == fData.end() )
                return {};

            QStringList retVal;
            for ( auto &&currStream : ( *pos ).second )
            {
                auto value = currStream->value( key );
                if ( value.isEmpty() )
                    continue;
                retVal << value;
            }

            return retVal;
        }

        std::shared_ptr< CStreamData > streamData( EStreamType whichStream, size_t streamNum ) const
        {
            auto pos = fData.find( whichStream );
            if ( pos == fData.end() )
                return {};

            if ( streamNum >= ( *pos ).second.size() )
                return {};
            return ( *pos ).second[ streamNum ];
        }

        void cleanUpValues( std::unordered_map< EMediaTags, QString > &retVal ) const
        {
            auto cleanFunc = [ &retVal ]( EMediaTags tag, std::function< QString( uint64_t ) > newStringFunc )
            {
                auto pos = retVal.find( tag );
                if ( pos != retVal.end() )
                {
                    bool aOK;
                    uint64_t numMSecs = ( *pos ).second.toInt( &aOK );
                    if ( !aOK )
                        numMSecs = 0;

                    auto newString = newStringFunc( numMSecs );
                    ( *pos ).second = newString;
                }
            };

            cleanFunc(
                EMediaTags::eLengthS,
                []( uint64_t numMSecs )
                {
                    auto numSecs = numMSecs / 1000;
                    return QString::number( numSecs );
                } );

            cleanFunc(
                EMediaTags::eLength,
                []( uint64_t numMSecs )
                {
                    NSABUtils::CTimeString ts( numMSecs );
                    return ts.toString( "hh:mm:ss" );
                } );

            cleanFunc( EMediaTags::eVideoBitrateString, []( uint64_t bitRate ) { return NSABUtils::NFileUtils::byteSizeString( bitRate, true, false, 3, true, "B/s" ); } );

            cleanFunc( EMediaTags::eAudioSampleRateString, []( uint64_t bitRate ) { return NSABUtils::NFileUtils::byteSizeString( bitRate, true, false, 2, true, "Hz" ); } );

            cleanFunc( EMediaTags::eOverAllBitrateString, []( uint64_t bitRate ) { return NSABUtils::NFileUtils::byteSizeString( bitRate, true, false, 3, true, "B/s" ); } );
        }

        std::unordered_map< NSABUtils::EMediaTags, QString > getMediaTags( const std::list< NSABUtils::EMediaTags > &tags ) const
        {
            auto realTags = tags;
            if ( tags.empty() )
            {
                realTags = {
                    EMediaTags::eFileName,   //
                    EMediaTags::eTitle,   //
                    EMediaTags::eLengthMS,   //
                    EMediaTags::eLengthS,   //
                    EMediaTags::eLength,   //
                    EMediaTags::eDate,   //
                    EMediaTags::eComment,   //
                    EMediaTags::eBPM,   //
                    EMediaTags::eArtist,   //
                    EMediaTags::eComposer,   //
                    EMediaTags::eGenre,   //
                    EMediaTags::eTrack,   //
                    EMediaTags::eAlbum,   //
                    EMediaTags::eAlbumArtist,   //
                    EMediaTags::eDiscnumber,   //
                    EMediaTags::eAspectRatio,   //
                    EMediaTags::eWidth,   //
                    EMediaTags::eHeight,   //
                    EMediaTags::eResolution,   //
                    EMediaTags::eFirstVideoCodec,   //
                    EMediaTags::eFirstAudioCodec,   //
                    EMediaTags::eVideoBitrate,   //
                    EMediaTags::eAudioSampleRate,   //
                    EMediaTags::eAudioSampleRate,   //

                };
            }

            std::unordered_map< NSABUtils::EMediaTags, QString > retVal;
            for ( auto &&ii : realTags )
            {
                if ( ii == NSABUtils::EMediaTags::eNumVideoStreams )
                {
                    retVal[ ii ] = QString::number( numStreams( EStreamType::eVideo ) );
                }
                else if ( ii == NSABUtils::EMediaTags::eNumAudioStreams )
                {
                    retVal[ ii ] = QString::number( numStreams( EStreamType::eAudio ) );
                }
                else if ( ii == NSABUtils::EMediaTags::eNumSubtitleStreams )   //
                {
                    retVal[ ii ] = QString::number( numStreams( EStreamType::eText ) );
                }
                else if (
                    ( ii == NSABUtils::EMediaTags::eWidth )   //
                    || ( ii == NSABUtils::EMediaTags::eHeight )   //
                    || ( ii == NSABUtils::EMediaTags::eAspectRatio )   //
                    || ( ii == NSABUtils::EMediaTags::eFirstVideoCodec )   //
                    || ( ii == NSABUtils::EMediaTags::eAllVideoCodecs )   //
                    || ( ii == NSABUtils::EMediaTags::eResolution )   //
                    || ( ii == NSABUtils::EMediaTags::eVideoBitrate ) || ( ii == NSABUtils::EMediaTags::eVideoBitrateString ) )
                {
                    QString value;
                    if ( ii == NSABUtils::EMediaTags::eAllVideoCodecs )
                    {
                        auto values = findAllValues( EStreamType::eVideo, ii );
                        value = values.join( ", " );
                    }
                    else
                    {
                        value = findFirstValue( EStreamType::eVideo, ii );
                        if ( value.isEmpty() && ( ( ii == NSABUtils::EMediaTags::eVideoBitrate ) || ( ii == NSABUtils::EMediaTags::eVideoBitrateString ) ) )
                        {
                            value = findFirstValue( EStreamType::eGeneral, EMediaTags::eOverAllBitrate );
                        }
                    }
                    retVal[ ii ] = value;
                }
                else if (
                    ( ii == NSABUtils::EMediaTags::eFirstAudioCodec )   //
                    || ( ii == NSABUtils::EMediaTags::eAllAudioCodecs )   //
                    || ( ii == NSABUtils::EMediaTags::eAudioSampleRate )   //
                    || ( ii == NSABUtils::EMediaTags::eAudioSampleRateString )   //
                )
                {
                    QString value;
                    if ( ii == NSABUtils::EMediaTags::eAllAudioCodecs )
                    {
                        auto values = findAllValues( EStreamType::eAudio, ii );
                        value = values.join( ", " );
                    }
                    else
                        value = findFirstValue( EStreamType::eAudio, ii );
                    retVal[ ii ] = value;
                }
                else if ( ( ii == NSABUtils::EMediaTags::eFirstSubtitle )
                    || ( ii == NSABUtils::EMediaTags::eAllSubtitles ) )
                {
                    QString value;
                    if ( ii == NSABUtils::EMediaTags::eAllSubtitles )
                    {
                        auto values = findAllValues( EStreamType::eText, ii );
                        value = values.join( ", " );
                    }
                    else
                        value = findFirstValue( EStreamType::eText, ii );
                    retVal[ ii ] = value;
                }
                else
                {
                    auto value = findFirstValue( EStreamType::eGeneral, ii );
                    retVal[ ii ] = value;
                }
            }

            cleanUpValues( retVal );
            return retVal;
        }

        bool validateFFProbeEXE()
        {
            if ( sFFProbeEXE.isEmpty() )
                return false;

            auto fi = QFileInfo( sFFProbeEXE );
            bool aOK = fi.exists() && fi.isExecutable();
            return aOK;
        }

        bool loadCodecsFromFFProbe()
        {
            if ( !validateFFProbeEXE() )
                return true;

            auto args = QStringList()   //
                        << "-v"
                        << "quiet"   //
                        << "-print_format"
                        << "json"   //
                        << "-show_format"   //
                        << "-show_streams"   //
                        << fFileName;

            QProcess process;
            //process.setProcessChannelMode( QProcess::MergedChannels );
            process.start( sFFProbeEXE, args );
            if ( !process.waitForFinished( -1 ) || ( process.exitStatus() != QProcess::NormalExit ) || ( process.exitCode() != 0 ) )
            {
                return false;
            }
            auto data = process.readAll();

            auto doc = QJsonDocument::fromJson( data );
            if ( !doc.object().contains( "streams" ) )
                return false;

            //qDebug().noquote().nospace() << doc.toJson( QJsonDocument::JsonFormat::Indented );

            std::map< EStreamType, int > streamCount;

            auto streams = doc[ "streams" ].toArray();
            for ( auto &&ii : streams )
            {
                auto stream = ii.toObject();
                //qDebug().noquote().noquote() << QJsonDocument( stream ).toJson( QJsonDocument::Indented );

                auto codecType = stream[ "codec_type" ].toString();
                //auto streamNum = stream[ "index" ].toInt();
                auto codec = stream[ "codec_name" ].toString();

                EStreamType streamType;
                if ( codecType == "general" )
                    streamType = EStreamType::eGeneral;
                else if ( codecType == "video" )
                    streamType = EStreamType::eVideo;
                else if ( codecType == "audio" )
                    streamType = EStreamType::eAudio;
                else if ( codecType == "subtitle" )
                    streamType = EStreamType::eText;
                else if ( codecType == "image" )
                    streamType = EStreamType::eImage;
                else if ( codecType == "menu" )
                    streamType = EStreamType::eMenu;
                else
                    streamType = EStreamType::eOther;

                auto streamNum = streamCount[ streamType ]++;

                auto streamData = this->streamData( streamType, streamNum );
                if ( !streamData )
                    continue;

                streamData->addData( "FFMpegCodec", codec );
            }
            return true;
        }

        QString fVersion;
        QString fFileName;

        std::unordered_map< EStreamType, std::vector< std::shared_ptr< CStreamData > > > fData;
        std::list< std::tuple< EStreamType, int, QString > > fFFProbeData;
        bool fAOK{ false };
        bool fQueued{ false };
    };

    CFileBasedCache< std::shared_ptr< CMediaInfoImpl > > CMediaInfoImpl::sMediaInfoCache;
    QString CMediaInfoImpl::sFFProbeEXE;

    void CMediaInfo::setFFProbeEXE( const QString &path )
    {
        CMediaInfoImpl::sFFProbeEXE = path;
    }

    QString CMediaInfo::ffprobeEXE()
    {
        return CMediaInfoImpl::sFFProbeEXE;
    }

    CMediaInfo::CMediaInfo() :
        fImpl( nullptr )
    {
        fImpl = CMediaInfoImpl::createImpl();
    }

    CMediaInfo::CMediaInfo( const QString &fileName, bool loadNow /*= true*/ ) :
        fImpl( nullptr )
    {
        fImpl = CMediaInfoImpl::createImpl( fileName, loadNow );
    }

    CMediaInfo::CMediaInfo( const QFileInfo &fi, bool loadNow /*= true*/ ) :
        fImpl( nullptr )
    {
        fImpl = CMediaInfoImpl::createImpl( fi, loadNow );
    }

    bool CMediaInfo::load()
    {
        return fImpl->load();
    }

    void CMediaInfo::queueLoad()
    {
        return fImpl->queueLoad( this );
    }

    CMediaInfo ::~CMediaInfo()
    {
    }

    bool CMediaInfo ::aOK() const
    {
        return fImpl->fAOK;
    }

    QString CMediaInfo ::fileName() const
    {
        return fImpl->fileName();
    }

    QString CMediaInfo::version() const
    {
        return fImpl->version();
    }

    bool CMediaInfo::isHEVCCodec( QString mediaCodecName, CFFMpegFormats *ffmpegFormats )
    {
        return fImpl->isCodec( "hevc", mediaCodecName, ffmpegFormats );
    }

    bool CMediaInfo::isVideoCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const
    {
        return fImpl->isCodec( checkCodecName, getMediaTag( NSABUtils::EMediaTags::eFirstVideoCodec ), ffmpegFormats );
    }

    bool CMediaInfo::isFormat( const QString &formatName, CFFMpegFormats *ffmpegFormats ) const
    {
        return fImpl->isFormat( formatName, ffmpegFormats );
    }

    bool CMediaInfo::isAudioCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const
    {
        return fImpl->isAudioCodec( checkCodecName, ffmpegFormats );
    }

    bool CMediaInfo::isCodec( const QString &checkCodecName, const QString &mediaCodecName, CFFMpegFormats *ffmpegFormats )
    {
        return fImpl->isCodec( checkCodecName, mediaCodecName, ffmpegFormats );
    }

    int64_t CMediaInfo::getBitRate() const
    {
        auto value = getMediaTag( EMediaTags::eVideoBitrate );
        bool aOK;
        auto retVal = value.toLongLong( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
    }

    int64_t CMediaInfo::getNumberOfSeconds() const
    {
        auto value = getMediaTag( EMediaTags::eLengthS );
        bool aOK;
        auto retVal = value.toLongLong( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
    }

    int64_t CMediaInfo::getNumberOfSeconds( const QString &fileName )
    {
        auto mediaInfo = CMediaInfo( fileName );
        return mediaInfo.getNumberOfSeconds();
    }

    int64_t CMediaInfo::getNumberOfMSecs() const
    {
        auto value = getMediaTag( EMediaTags::eLengthMS );
        bool aOK;
        auto retVal = value.toLongLong( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
    }

    int64_t CMediaInfo::getNumberOfMSecs( const QString &fileName )
    {
        auto mediaInfo = CMediaInfo( fileName );
        return mediaInfo.getNumberOfMSecs();
    }

    QString CMediaInfo::getMediaTag( const QString &path, NSABUtils::EMediaTags tag )
    {
        auto mediaInfo = CMediaInfo( path );
        if ( !mediaInfo.aOK() )
            return {};

        return mediaInfo.getMediaTag( tag );
    }

    QString CMediaInfo::getMediaTag( NSABUtils::EMediaTags tag ) const
    {
        auto tmp = getMediaTags( { tag } );
        auto pos = tmp.find( tag );
        if ( pos == tmp.end() )
            return {};
        return ( *pos ).second;
    }

    std::unordered_map< NSABUtils::EMediaTags, QString > CMediaInfo::getMediaTags( const QString &path, const std::list< NSABUtils::EMediaTags > &tags )
    {
        auto mediaInfo = CMediaInfo( path );
        if ( !mediaInfo.aOK() )
            return {};

        return mediaInfo.getMediaTags( tags );
    }

    std::unordered_map< NSABUtils::EMediaTags, QString > CMediaInfo::getMediaTags( const std::list< NSABUtils::EMediaTags > &tags ) const
    {
        return fImpl->getMediaTags( tags );
    }

    void CMediaInfo::emitMediaInfoLoaded( const QString &fileName )
    {
        emit sigMediaInfoLoaded( fileName );
    }

    int CMediaInfo::numAudioStreams() const
    {
        return getMediaTag( EMediaTags::eNumAudioStreams ).toInt();
    }

    int CMediaInfo::numVideoStreams() const
    {
        return getMediaTag( EMediaTags::eNumVideoStreams ).toInt();
    }

    int CMediaInfo::numSubtitleStreams() const
    {
        return getMediaTag( EMediaTags::eNumSubtitleStreams ).toInt();
    }

    QString mediaInfoTagName( EMediaTags tag )
    {
        switch ( tag )
        {
            case EMediaTags::eFileName:
                return "Filename";
            case EMediaTags::eTitle:
                return "Title";
            case EMediaTags::eLengthMS:
                return "Duration";
            case EMediaTags::eLengthS:
                return "Duration";
            case EMediaTags::eLength:
                return "Duration";
            case EMediaTags::eDate:
                return "DATE_RECORDED";
            case EMediaTags::eComment:
                return "Comment";
            case EMediaTags::eBPM:
                return "BPM";
            case EMediaTags::eArtist:
                return "ARTIST";
            case EMediaTags::eComposer:
                return "COMPOSER";
            case EMediaTags::eGenre:
                return "GENRE";
            case EMediaTags::eTrack:
                return "Track/Position";
            case EMediaTags::eAlbum:
                return "Album";
            case EMediaTags::eAlbumArtist:
                return "Album Artist";
            case EMediaTags::eDiscnumber:
                return "Disc Number";
            case EMediaTags::eAspectRatio:
                return "AspectRatio";
            case EMediaTags::eWidth:
                return "Width";
            case EMediaTags::eHeight:
                return "Height";
            case EMediaTags::eResolution:
                return "Resolution";
            case EMediaTags::eFirstVideoCodec:
            case EMediaTags::eAllVideoCodecs:
            case EMediaTags::eFirstAudioCodec:
            case EMediaTags::eAllAudioCodecs:
                return "CodecID";
            case EMediaTags::eFirstSubtitle:
            case EMediaTags::eAllSubtitles:
                return "Language";
            case EMediaTags::eVideoBitrate:
                return "BitRate";
            case EMediaTags::eVideoBitrateString:
                return "BitRate";
            case EMediaTags::eOverAllBitrate:
                return "OverallBitRate";
            case EMediaTags::eOverAllBitrateString:
                return "OverallBitRate";
            case EMediaTags::eAudioSampleRate:
                return "SamplingRate";
            case EMediaTags::eAudioSampleRateString:
                return "SamplingRate";
            default:
                return QObject::tr( "" );
        }
    }

    EMediaTags fromTagName( const QString &tag )
    {
        static std::unordered_map< QString, EMediaTags > sMap;
        if ( sMap.empty() )
        {
            for ( auto &&ii = EMediaTags::eFileName; ii < EMediaTags::eLastTag; ii = static_cast< EMediaTags >( static_cast< int >( ii ) + 1 ) )
            {
                auto string = mediaInfoTagName( ii );
                sMap[ string ] = ii;
            }
        }
        auto pos = sMap.find( tag );
        if ( pos == sMap.end() )
            return {};
        return ( *pos ).second;
    }

    std::unordered_map< NSABUtils::EMediaTags, QString > CMediaInfo::getSettableMediaTags() const
    {
        return getMediaTags(
            { EMediaTags::eTitle,   //
              EMediaTags::eDate,   //
              EMediaTags::eComment,   //
              EMediaTags::eBPM,   //
              EMediaTags::eArtist,   //
              EMediaTags::eComposer,   //
              EMediaTags::eGenre,   //
              EMediaTags::eTrack,   //
              EMediaTags::eAlbum,   //
              EMediaTags::eAlbumArtist,   //
              EMediaTags::eDiscnumber } );
    }
}
