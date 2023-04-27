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

        void setIsDefault( bool value ) { fIsDefault = value; }
        bool isDefault() const { return fIsDefault; }
        void addData( const QString &name, const QString &value );

        QString value( const QString &key ) const;
        QString value( EMediaTags key ) const;

        QStringList values( const std::list< QString > &keys ) const
        {
            QStringList retVal;
            for ( auto &&key : keys )
            {
                auto value = this->value( key );
                retVal << std::move( value );
            }

            return retVal;
        }

        size_t size() const { return fStreamData.size(); }
        std::pair< QString, QString > operator[]( size_t idx ) const { return fStreamData[ idx ]; }

    private:
        EStreamType fStreamType{ EStreamType::eGeneral };
        int fStreamNum{ 0 };
        bool fIsDefault{ false };
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
        if ( key == "AudioCodecIDDisp" )
        {
            auto codec = value( "CodecID" );   // get the codec
            auto numChannels = value( mediaInfoTagName( EMediaTags::eAudioChannelCount ) );
            bool aOK = true;
            auto channelCount = numChannels.toInt( &aOK );
            if ( !numChannels.isEmpty() && aOK )
            {
                QString channelString;
                if ( channelCount > 2 )
                    channelString = QString( " %2.1" ).arg( channelCount - 1 );
                else if ( channelCount == 2 )
                    channelString = QString( "stereo" );
                else
                    channelString = QString( "mono" );
                return QString( "%1 %2" ).arg( codec ).arg( channelString );
            }
            else
                return codec;
        }
        else if ( key == "Resolution" )
        {
            auto width = value( mediaInfoTagName( EMediaTags::eWidth ) );
            auto height = value( mediaInfoTagName( EMediaTags::eHeight ) );
            if ( !width.isEmpty() && !height.isEmpty() )
            {
                return QString( "%1x%2" ).arg( width ).arg( height );
            }
        }

        auto pos = fStreamDataMap.find( key );
        if ( key == "CodecID" )
        {
            auto pos2 = fStreamDataMap.find( "FFMpegCodec" );
            if ( pos2 != fStreamDataMap.end() )
                pos = pos2;
        }

        if ( pos == fStreamDataMap.end() )
            return {};

        return ( *pos ).second;
    }

    //QString CStreamData::value( EMediaTags key ) const
    //{
    //    auto pos = fKnownTagStreamDataMap.find( key );
    //    if ( pos == fKnownTagStreamDataMap.end() )
    //        return {};

    //    return ( *pos ).second;
    //}

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
                static std::shared_ptr< CMediaInfoImpl > sNullImpl = std::make_shared< CMediaInfoImpl >();
                retVal = sNullImpl;
                sMediaInfoCache.add( retVal );
            }
            return retVal;
        }

        static std::shared_ptr< CMediaInfoImpl > createImpl( const QFileInfo &fi, bool loadNow )
        {
            auto retVal = sMediaInfoCache.find( fi.absoluteFilePath() );
            if ( !retVal )
            {
                retVal = std::make_shared< CMediaInfoImpl >( fi );
                if ( loadNow )
                    retVal->load();
                sMediaInfoCache.add( retVal );
            }
            return retVal;
        }

        static bool mediaExists( const QFileInfo &fi ) { return sMediaInfoCache.contains( fi.absoluteFilePath() ); }

        static std::shared_ptr< CMediaInfoImpl > createImpl( const QString &path, bool loadNow ) { return createImpl( std::move( QFileInfo( path ) ), loadNow ); }

        CMediaInfoImpl() {}
        CMediaInfoImpl( const QFileInfo &fi ) :
            fFileName( fi.absoluteFilePath() )
        {
        }

        ~CMediaInfoImpl() {}

        bool isQueued() const { return fQueued; };
        void setQueued( bool value ) { fQueued = value; }
        bool queueLoad( std::function< void( const QString & ) > onFinish )
        {
            QFileInfo fi( fFileName );
            if ( !fi.exists() || !fi.isReadable() )
                return false;

            if ( !aOK() && !isQueued() )
            {
                setQueued( true );
                QThreadPool::globalInstance()->start(
                    [ this, onFinish ]()
                    {
                        qDebug() << "Loading media info for" << fFileName;
                        if ( !load() )
                        {
                            qDebug() << "Failed to get media info for" << fFileName;
                            return;
                        }
                        qDebug() << "Finished loading media info for" << fFileName;
                        setQueued( false );
                        onFinish( fFileName );
                    } );
                return true;
            }
            return false;
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

        bool hasVideoCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const
        {
            auto values = findAllValues( EStreamType::eVideo, mediaInfoTagName( EMediaTags::eAllVideoCodecs ) );
            for ( auto &&value : values )
            {
                if ( isCodec( checkCodecName, value, ffmpegFormats ) )
                    return true;
            }
            return false;
        }

        bool isDefaultCodec( const QString &checkCodecName, std::optional< int > maxNumChannels, CFFMpegFormats *ffmpegFormats ) const
        {
            auto values = QStringList()   //
                          << findDefaultValues( EStreamType::eAudio, { EMediaTags::eAllAudioCodecs, EMediaTags::eAudioChannelCount } )   //
                          << findDefaultValues( EStreamType::eAudio, { "Format", mediaInfoTagName( EMediaTags::eAudioChannelCount ) } )   //
                          << findDefaultValues( EStreamType::eAudio, { "InternetMediaType", mediaInfoTagName( EMediaTags::eAudioChannelCount ) } )   //
                ;

            for ( auto ii = 0; ii < values.count(); ii += 2 )
            {
                auto codecName = values[ ii ];
                if ( codecName.isEmpty() )
                    continue;

                if ( !isCodec( checkCodecName, codecName, ffmpegFormats ) )
                    continue;

                if ( !maxNumChannels.has_value() )
                    return true;

                bool aOK = true;
                auto numChannels = values[ ii + 1 ].toInt( &aOK );
                if ( values[ ii + 1 ].isEmpty() || !aOK )
                    continue;

                if ( numChannels <= maxNumChannels.value() )
                    return true;
            }
            return false;
        }

        bool isCodec( const QString &checkCodecName, const QString &mediaCodecName, CFFMpegFormats *ffmpegFormats ) const
        {
            Q_ASSERT( ffmpegFormats != nullptr );
            return ffmpegFormats->isCodec( checkCodecName, mediaCodecName );
        }

        bool isContainerFormat( const QString &formatName, CFFMpegFormats *ffmpegFormats ) const
        {
            Q_ASSERT( ffmpegFormats != nullptr );
            return ffmpegFormats->isContainerFormat( fFileName, formatName );
        }

        size_t defaultStreamNum( EStreamType whichStream ) const
        {
            auto pos = fDefaultStreams.find( whichStream );
            if ( pos == fDefaultStreams.end() )
                return 0;
            return ( *pos ).second;
        }

        size_t numStreams( EStreamType whichStream ) const
        {
            auto &&streamData = getAllStreamData( whichStream );
            return streamData.size();
        }

        std::vector< std::shared_ptr< CStreamData > > getAllStreamData( EStreamType whichStream ) const
        {
            auto pos = fData.find( whichStream );
            if ( pos == fData.end() )
                return {};

            auto &&streamData = ( *pos ).second;
            return streamData;
        }

        std::shared_ptr< CStreamData > getStreamData( EStreamType whichStream, size_t streamNum ) const
        {
            auto &&streamData = getAllStreamData( whichStream );
            if ( streamData.empty() )
                return {};

            if ( streamNum == -1 )   // use default Stream
            {
                streamNum = defaultStreamNum( whichStream );
            }
            Q_ASSERT( streamNum < streamData.size() );
            if ( streamNum >= streamData.size() )
                streamNum = 0;

            auto stream = streamData[ streamNum ];
            return stream;
        }

        std::shared_ptr< CStreamData > getDefaultStreamData( EStreamType whichStream ) const { return getStreamData( whichStream, -1 ); }

        // finds the value on the default stream
        QString findDefaultValue( EStreamType whichStream, EMediaTags key ) const { return findValue( whichStream, -1, key ); }
        QString findDefaultValue( EStreamType whichStream, const QString &key ) const { return findValue( whichStream, -1, key ); }

        QString findValue( EStreamType whichStream, size_t streamNum, EMediaTags key ) const { return findValue( whichStream, streamNum, mediaInfoTagName( key ) ); }
        QString findValue( EStreamType whichStream, size_t streamNum, const QString &key ) const
        {
            auto stream = getStreamData( whichStream, streamNum );
            if ( !stream )
                return {};

            auto value = stream->value( key );
            if ( value.isEmpty() )
            {
                auto &&streamData = getAllStreamData( whichStream );
                for ( auto &&currStream : streamData )
                {
                    value = currStream->value( key );
                    if ( value.isEmpty() )
                        continue;
                }
            }
            return value;
        }

        QStringList findDefaultValues( EStreamType whichStream, const std::list< EMediaTags > &keys ) const { return findDefaultValues( whichStream, toStringList( keys ) ); }
        QStringList findDefaultValues( EStreamType whichStream, const std::list< QString > &keys ) const
        {
            auto stream = getDefaultStreamData( whichStream );
            if ( !stream )
                return {};

            return stream->values( keys );
        }

        QStringList findAllValues( EStreamType whichStream, const std::list< QString > &keys ) const
        {
            auto &&streamData = getAllStreamData( whichStream );

            QStringList retVal;
            for ( auto &&currStream : streamData )
            {
                retVal << currStream->values( keys );
            }

            return retVal;
        }

        QStringList findAllValues( EStreamType whichStream, const std::list< EMediaTags > &keys ) const { return findAllValues( whichStream, std::move( toStringList( keys ) ) ); }
        QStringList findAllValues( EStreamType whichStream, EMediaTags key ) const { return findAllValues( whichStream, mediaInfoTagName( key ) ); }
        QStringList findAllValues( EStreamType whichStream, const QString &key ) const { return findAllValues( whichStream, std::list< QString >( { key } ) ); }

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
                    CTimeString ts( numMSecs );
                    return ts.toString( "hh:mm:ss" );
                } );

            cleanFunc( EMediaTags::eVideoBitrateString, []( uint64_t bitRate ) { return NFileUtils::byteSizeString( bitRate, true, false, 3, true, "bps" ); } );
            cleanFunc( EMediaTags::eAudioSampleRateString, []( uint64_t bitRate ) { return NFileUtils::byteSizeString( bitRate, true, false, 2, true, "Hz" ); } );
            cleanFunc( EMediaTags::eOverAllBitrateString, []( uint64_t bitRate ) { return NFileUtils::byteSizeString( bitRate, true, false, 3, true, "bps" ); } );
        }

        QString getMediaTag( size_t streamNum, EMediaTags tag ) const
        {
            QString retVal;
            if ( tag == EMediaTags::eNumVideoStreams )
            {
                retVal = QString::number( numStreams( EStreamType::eVideo ) );
            }
            else if ( tag == EMediaTags::eNumAudioStreams )
            {
                retVal = QString::number( numStreams( EStreamType::eAudio ) );
            }
            else if ( tag == EMediaTags::eNumSubtitleStreams )   //
            {
                retVal = QString::number( numStreams( EStreamType::eText ) );
            }
            else if ( tag == EMediaTags::eDefaultAudioStream )
            {
                retVal = QString::number( defaultStreamNum( EStreamType::eAudio ) );
            }
            else if ( tag == EMediaTags::eDefaultVideoStream )
            {
                retVal = QString::number( defaultStreamNum( EStreamType::eVideo ) );
            }
            else if ( tag == EMediaTags::eDefaultSubtitleStream )   //
            {
                retVal = QString::number( defaultStreamNum( EStreamType::eText ) );
            }
            else if (
                ( tag == EMediaTags::eWidth )   //
                || ( tag == EMediaTags::eHeight )   //
                || ( tag == EMediaTags::eAspectRatio )   //
                || ( tag == EMediaTags::eDisplayAspectRatio )   //
                || ( tag == EMediaTags::eVideoCodec )   //
                || ( tag == EMediaTags::eFirstVideoCodec )   //
                || ( tag == EMediaTags::eAllVideoCodecs )   //
                || ( tag == EMediaTags::eResolution )   //
                || ( tag == EMediaTags::eVideoBitrate )   //
                || ( tag == EMediaTags::eVideoBitrateString )   //
                || ( tag == EMediaTags::eHDRInfo )   //
                || ( tag == EMediaTags::eBitsPerPixel )   //
                || ( tag == EMediaTags::eBitDepth )   //
                || ( tag == EMediaTags::eFrameRate )   //
                || ( tag == EMediaTags::eFrameRateNum )   //
                || ( tag == EMediaTags::eFrameRateDen )   //
                || ( tag == EMediaTags::eScanType )   //
            )
            {
                QString value;
                if ( tag == EMediaTags::eAllVideoCodecs )
                {
                    auto values = findAllValues( EStreamType::eVideo, tag );
                    auto defStreamNum = static_cast< int >( defaultStreamNum( EStreamType::eVideo ) );
                    values[ defStreamNum ] += " (Default)";
                    value = values.join( ", " );
                    if ( values.count() > 1 )
                        value = QString( "(%1) %2" ).arg( values.count() ).arg( value );
                }
                else
                {
                    value = findValue( EStreamType::eVideo, streamNum, tag );
                    if ( value.isEmpty() )
                    {
                        if ( ( tag == EMediaTags::eVideoBitrate ) || ( tag == EMediaTags::eVideoBitrateString ) )
                        {
                            value = findValue( EStreamType::eGeneral, streamNum, EMediaTags::eOverAllBitrate );
                        }
                        else if ( tag == EMediaTags::eAspectRatio )
                        {
                            value = findValue( EStreamType::eVideo, streamNum, EMediaTags::eDisplayAspectRatio );
                        }
                        else if ( tag == EMediaTags::eFrameRate )
                        {
                            auto numerator = 1.0 * findValue( EStreamType::eVideo, streamNum, EMediaTags::eFrameRateNum ).toLongLong();
                            auto denominator = 1.0 * findValue( EStreamType::eVideo, streamNum, EMediaTags::eFrameRateDen ).toLongLong();
                            auto retVal = static_cast< uint64_t >( numerator * 1000.0 / denominator );
                            value = QString::number( retVal );
                            value.insert( value.length() - 3, "." );
                        }
                        else if ( tag == EMediaTags::eBitsPerPixel )
                            value = QString::number( 3 * findValue( EStreamType::eVideo, streamNum, EMediaTags::eBitDepth ).toInt() );
                    }
                }
                retVal = value;
            }
            else if (
                ( tag == EMediaTags::eFirstAudioCodec )   //
                || ( tag == EMediaTags::eAudioCodec )   //
                || ( tag == EMediaTags::eAllAudioCodecsDisp )   //
                || ( tag == EMediaTags::eFirstAudioCodecDisp )   //
                || ( tag == EMediaTags::eAudioCodecDisp )   //
                || ( tag == EMediaTags::eAllAudioCodecs )   //
                || ( tag == EMediaTags::eAudioSampleRate )   //
                || ( tag == EMediaTags::eAudioSampleRateString )   //
                || ( tag == EMediaTags::eAudioChannelCount ) )
            {
                QString value;
                if ( tag == EMediaTags::eAllAudioCodecs )
                {
                    auto values = findAllValues( EStreamType::eAudio, tag );
                    auto defStreamNum = static_cast< int >( defaultStreamNum( EStreamType::eAudio ) );
                    values[ defStreamNum ] += " (Default)";
                    value = values.join( ", " );
                    if ( values.count() > 1 )
                        value = QString( "(%1) %2" ).arg( values.count() ).arg( value );
                }
                else if ( tag == EMediaTags::eAllAudioCodecsDisp )
                {
                    auto values = findAllValues( EStreamType::eAudio, tag );
                    auto defStreamNum = static_cast< int >( defaultStreamNum( EStreamType::eAudio ) );
                    values[ defStreamNum ] += " (Default)";
                    value = values.join( ", " );
                    if ( values.count() > 1 )
                        value = QString( "(%1) %2" ).arg( values.count() ).arg( value );
                }
                else
                    value = findValue( EStreamType::eAudio, streamNum, tag );
                retVal = value;
            }
            else if (
                ( tag == EMediaTags::eSubtitleLanguage )   //
                || ( tag == EMediaTags::eFirstSubtitleLanguage )   //
                || ( tag == EMediaTags::eAllSubtitleLanguages )   //
                || ( tag == EMediaTags::eSubtitleCodec )   //
                || ( tag == EMediaTags::eFirstSubtitleCodec )   //
                || ( tag == EMediaTags::eAllSubtitleCodecs ) )
            {
                QString value;
                if ( tag == EMediaTags::eAllSubtitleLanguages )
                {
                    auto values = findAllValues( EStreamType::eText, tag );
                    auto defStreamNum = static_cast< int >( defaultStreamNum( EStreamType::eText ) );
                    values[ defStreamNum ] += " (Default)";
                    value = values.join( ", " );
                    if ( values.count() > 1 )
                        value = QString( "(%1) %2" ).arg( values.count() ).arg( value );
                }
                else if ( tag == EMediaTags::eAllSubtitleCodecs )
                {
                    auto values = findAllValues( EStreamType::eText, tag );
                    auto defStreamNum = static_cast< int >( defaultStreamNum( EStreamType::eText ) );
                    values[ defStreamNum ] += " (Default)";
                    value = values.join( ", " );
                    if ( values.count() > 1 )
                        value = QString( "(%1) %2" ).arg( values.count() ).arg( value );
                }
                else
                    value = findValue( EStreamType::eText, streamNum, tag );
                retVal = value;
            }
            else
            {
                auto value = findValue( EStreamType::eGeneral, streamNum, tag );
                retVal = value;
            }
            return retVal;
        }

        std::unordered_map< EMediaTags, QString > getMediaTags( size_t streamNum, const std::list< EMediaTags > &tags ) const
        {
            auto realTags = getRealTags( tags );

            std::unordered_map< EMediaTags, QString > retVal;
            for ( auto &&ii : realTags )
            {
                auto value = getMediaTag( streamNum, ii );
                retVal[ ii ] = value;
            }

            cleanUpValues( retVal );
            return retVal;
        }

        std::unordered_map< EMediaTags, QString > getMediaTags( const std::list< EMediaTags > &tags ) const { return getMediaTags( -1, tags ); }

        std::list< EMediaTags > getRealTags( const std::list< EMediaTags > &tags ) const
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
                    EMediaTags::eFirstAudioCodecDisp,   //
                    EMediaTags::eVideoBitrate,   //
                    EMediaTags::eAudioSampleRate,   //
                    EMediaTags::eAudioSampleRate   //
                };
            }
            return realTags;
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

                auto streamData = this->getStreamData( streamType, streamNum );
                if ( !streamData )
                    continue;

                streamData->addData( "FFMpegCodec", codec );

                if ( !stream.contains( "disposition" ) )
                    continue;

                auto disposition = stream[ "disposition" ].toObject();
                auto defaultDisp = disposition[ "default" ].toInt() != 0;
                streamData->addData( "Disposition_Default", QString( "%1" ).arg( defaultDisp ) );
                streamData->setIsDefault( defaultDisp );
                if ( defaultDisp )
                    fDefaultStreams[ streamType ] = streamNum;
            }
            return true;
        }

        QString fVersion;
        QString fFileName;

        std::unordered_map< EStreamType, size_t > fDefaultStreams;
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

    CMediaInfo::CMediaInfo( const QString &fileName ) :   // loads immediately use the mgr for delayed load
        CMediaInfo( fileName, true )
    {
    }

    CMediaInfo::CMediaInfo( const QFileInfo &fi ) :
        CMediaInfo( fi, true )
    {
    }

    bool CMediaInfo::load()
    {
        return fImpl->load();
    }

    bool CMediaInfo::queueLoad( std::function< void( const QString &fileName ) > onFinish )
    {
        return fImpl->queueLoad( onFinish );
    }

    bool CMediaInfo::isQueued() const
    {
        return fImpl->isQueued();
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

    bool CMediaInfo::hasVideoCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const
    {
        return fImpl->hasVideoCodec( checkCodecName, ffmpegFormats );
    }

    bool CMediaInfo::isContainerFormat( const QString &formatName, CFFMpegFormats *ffmpegFormats ) const
    {
        return fImpl->isContainerFormat( formatName, ffmpegFormats );
    }

    bool CMediaInfo::isDefaultAudioCodecAAC( CFFMpegFormats *ffmpegFormats, int maxNumChannels ) const
    {
        return fImpl->isDefaultCodec( "aac", maxNumChannels, ffmpegFormats );
    }

    bool CMediaInfo::isDefaultAudioCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const
    {
        return fImpl->isDefaultCodec( checkCodecName, {}, ffmpegFormats );
    }

    bool CMediaInfo::isCodec( const QString &checkCodecName, const QString &mediaCodecName, CFFMpegFormats *ffmpegFormats )
    {
        return fImpl->isCodec( checkCodecName, mediaCodecName, ffmpegFormats );
    }

    uint64_t CMediaInfo::getUncompressedBitRate() const
    {
        return getResolutionInfo().idealBitrate();
    }

    SResolutionInfo CMediaInfo::getResolutionInfo() const
    {
        SResolutionInfo retVal;
        retVal.fResolution = getResolution();
        retVal.fFPS = getFrameRate();
        retVal.fBitsPerPixel = getBitsPerPixel();
        retVal.fInterlaced = getInterlaced();
        return retVal;
    }

    uint64_t CMediaInfo::getBitRate() const
    {
        auto value = getMediaTag( EMediaTags::eVideoBitrate );
        bool aOK;
        auto retVal = value.toULongLong( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
    }

    int CMediaInfo::getBitsPerPixel() const
    {
        auto value = getMediaTag( EMediaTags::eBitsPerPixel );
        bool aOK;
        auto retVal = value.toInt( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
    }

    double CMediaInfo::getFrameRate() const
    {
        auto value = getMediaTag( EMediaTags::eFrameRate );
        bool aOK;
        auto retVal = value.toDouble( &aOK );
        if ( !aOK )
            return 0;

        return retVal;
    }

    bool CMediaInfo::getInterlaced() const
    {
        auto value = getMediaTag( EMediaTags::eScanType ).toLower();
        return value == "interlaced";
    }

    std::pair< int, int > CMediaInfo::getResolution() const
    {
        auto value = getMediaTag( EMediaTags::eWidth );
        bool aOK;
        auto width = value.toLongLong( &aOK );
        if ( !aOK )
            return { 0, 0 };

        value = getMediaTag( EMediaTags::eHeight );
        auto height = value.toLongLong( &aOK );
        if ( !aOK )
            return { 0, 0 };

        return { width, height };
    }

    SResolutionInfo CMediaInfo::k8KResolution = { { 7680, 4320 }, false, 60.0, 30 };
    SResolutionInfo CMediaInfo::k4KResolution = { { 3840, 2160 }, false, 60.0, 24 };
    SResolutionInfo CMediaInfo::k1080pResolution = { { 1920, 1080 }, false, 60.0, 24 };
    SResolutionInfo CMediaInfo::k1080iResolution = { { 1920, 1080 }, true, 30.0, 24 };
    SResolutionInfo CMediaInfo::k720Resolution = { { 1280, 720 }, false, 24.0, 24 };
    SResolutionInfo CMediaInfo::k480Resolution = { { 640, 480 }, false, 24.0, 24 };

    uint64_t SResolutionInfo::idealBitrate() const
    {
        uint64_t uncompressed = fResolution.first;
        uncompressed *= fResolution.second;
        if ( fInterlaced )
            uncompressed /= 2;
        uncompressed *= fBitsPerPixel;
        uncompressed *= fFPS;
        return uncompressed;
    }

    bool SResolutionInfo::isGreaterThan4kResolution( double threshold /*= 0.2 */ ) const
    {
        return isGreaterThanResolution( CMediaInfo::k4KResolution.fResolution, threshold );
    }

    bool SResolutionInfo::is4kResolution( double threshold /* = 0.2 */ ) const
    {
        return isResolution( CMediaInfo::k4KResolution.fResolution, threshold );
    }

    bool SResolutionInfo::isGreaterThanHDResolution( double threshold /* = 0.0 */ ) const
    {
        return isGreaterThanResolution( CMediaInfo::k1080pResolution.fResolution, threshold );
    }

    bool SResolutionInfo::isHDResolution( double threshold /* = 0.2 */ ) const
    {
        return isResolution( CMediaInfo::k1080pResolution.fResolution, threshold );
    }

    bool SResolutionInfo::isSubHDResolution( double threshold /* = 0.2 */ ) const
    {
        return isLessThanResolution( CMediaInfo::k1080pResolution.fResolution, threshold );
    }

    bool SResolutionInfo::isResolution( const std::pair< int, int > &targetRes, double threshold /* = 0.2 */ ) const
    {
        auto myPixels = fResolution.first * fResolution.second;
        auto targetPixels = targetRes.first * targetRes.second;
        auto targetPixelsMin = ( 1.0 - threshold ) * targetPixels;
        auto targetPixelsMax = ( 1.0 + threshold ) * targetPixels;

        return ( myPixels >= targetPixelsMin ) && ( myPixels <= targetPixelsMax );
    }

    bool SResolutionInfo::isGreaterThanResolution( const std::pair< int, int > &targetRes, double threshold /* = 0.2 */ ) const
    {
        auto myPixels = fResolution.first * fResolution.second;
        auto targetPixels = targetRes.first * targetRes.second;
        auto targetPixelsMin = ( 1.0 + threshold ) * targetPixels;

        return ( myPixels > targetPixelsMin );
    }

    bool SResolutionInfo::isLessThanResolution( const std::pair< int, int > &targetRes, double threshold /* = 0.2 */ ) const
    {
        auto myPixels = fResolution.first * fResolution.second;
        auto targetPixels = targetRes.first * targetRes.second;
        auto targetPixelsMax = ( 1.0 - threshold ) * targetPixels;

        return ( myPixels < targetPixelsMax );
    }

    bool CMediaInfo::isGreaterThan4kResolution( double threshold /*= 0.2 */ ) const
    {
        return getResolutionInfo().isGreaterThan4kResolution( threshold );
    }

    bool CMediaInfo::is4kResolution( double threshold /* = 0.2 */ ) const
    {
        return getResolutionInfo().is4kResolution( threshold );
    }

    bool CMediaInfo::isHDResolution( double threshold /* = 0.2 */ ) const
    {
        return getResolutionInfo().isHDResolution( threshold );
    }

    bool CMediaInfo::isSubHDResolution( double threshold /* = 0.2 */ ) const
    {
        return getResolutionInfo().isSubHDResolution( threshold );
    }
    bool CMediaInfo::isGreaterThanHDResolution( double threshold /* = 0.0 */ ) const
    {
        return getResolutionInfo().isGreaterThanHDResolution( threshold );
    }

    bool CMediaInfo::isResolution( const std::pair< int, int > &targetRes, double threshold /* = 0.2 */ ) const
    {
        return getResolutionInfo().isResolution( targetRes, threshold );
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

    QString CMediaInfo::getMediaTag( const QString &path, EMediaTags tag )
    {
        auto mediaInfo = CMediaInfo( path );
        if ( !mediaInfo.aOK() )
            return {};

        return mediaInfo.getMediaTag( tag );
    }

    QString CMediaInfo::getMediaTag( EMediaTags tag ) const
    {
        return getMediaTag( -1, tag );
    }

    QString CMediaInfo::getMediaTag( size_t streamNum, EMediaTags tag ) const
    {
        auto tmp = fImpl->getMediaTags( streamNum, { tag } );
        auto pos = tmp.find( tag );
        if ( pos == tmp.end() )
            return {};
        return ( *pos ).second;
    }

    std::unordered_map< EMediaTags, QString > CMediaInfo::getMediaTags( const QString &path, const std::list< EMediaTags > &tags )
    {
        auto mediaInfo = CMediaInfo( path );
        if ( !mediaInfo.aOK() )
            return {};

        return mediaInfo.getMediaTags( tags );
    }

    std::unordered_map< EMediaTags, QString > CMediaInfo::getMediaTags( const std::list< EMediaTags > &tags ) const
    {
        return fImpl->getMediaTags( tags );
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

    int CMediaInfo::defaultAudioStream() const
    {
        return getMediaTag( EMediaTags::eDefaultAudioStream ).toInt();
    }

    int CMediaInfo::audioChannelCount( size_t streamNum ) const
    {
        return getMediaTag( streamNum, EMediaTags::eAudioChannelCount ).toInt();
    }

    int CMediaInfo::defaultVideoStream() const
    {
        return getMediaTag( EMediaTags::eDefaultVideoStream ).toInt();
    }

    int CMediaInfo::defaultSubtitleStream() const
    {
        return getMediaTag( EMediaTags::eDefaultSubtitleStream ).toInt();
    }

    QStringList CMediaInfo::allSubtitleCodecs() const
    {
        return fImpl->findAllValues( EStreamType::eText, EMediaTags::eAllSubtitleCodecs );
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
            case EMediaTags::eDisplayAspectRatio:
                return "DisplayAspectRatio";
            case EMediaTags::eWidth:
                return "Width";
            case EMediaTags::eHeight:
                return "Height";
            case EMediaTags::eResolution:
                return "Resolution";
            case EMediaTags::eVideoCodec:
            case EMediaTags::eFirstVideoCodec:
            case EMediaTags::eAllVideoCodecs:
            case EMediaTags::eAudioCodec:
            case EMediaTags::eFirstAudioCodec:
            case EMediaTags::eAllAudioCodecs:
            case EMediaTags::eSubtitleCodec:
            case EMediaTags::eFirstSubtitleCodec:
            case EMediaTags::eAllSubtitleCodecs:
                return "CodecID";
            case EMediaTags::eAudioChannelCount:
                return "Channel(s)";
            case EMediaTags::eAudioCodecDisp:
            case EMediaTags::eFirstAudioCodecDisp:
            case EMediaTags::eAllAudioCodecsDisp:
                return "AudioCodecIDDisp";
            case EMediaTags::eSubtitleLanguage:
            case EMediaTags::eFirstSubtitleLanguage:
            case EMediaTags::eAllSubtitleLanguages:
                return "Language";
            case EMediaTags::eVideoBitrate:
                return "BitRate";
            case EMediaTags::eVideoBitrateString:
                return "BitRate";
            case EMediaTags::eHDRInfo:
                return "HDR_Format/String";
            case EMediaTags::eBitsPerPixel:
                return "BitsPerPixel";
            case EMediaTags::eBitDepth:
                return "BitDepth";
            case EMediaTags::eFrameRate:
                return "FrameRate";
            case EMediaTags::eFrameRateNum:
                return "FrameRate_Num";
            case EMediaTags::eFrameRateDen:
                return "FrameRate_Den";
            case EMediaTags::eScanType:
                return "ScanType";
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

    std::list< QString > toStringList( const std::list< EMediaTags > &keys )
    {
        std::list< QString > tmp;
        for ( auto &&ii : keys )
            tmp.emplace_back( mediaInfoTagName( ii ) );
        return tmp;
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

    std::unordered_map< EMediaTags, QString > CMediaInfo::getSettableMediaTags() const
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

    CMediaInfoMgr *CMediaInfoMgr::instance()
    {
        static CMediaInfoMgr retVal;
        return &retVal;
    }

    std::shared_ptr< CMediaInfo > CMediaInfoMgr::getMediaInfo( const QString &fileName )   // creates a mediainfo, then loads it in a background thread, then CMediaInfo will emit when its finished
    {
        return getMediaInfo( std::move( QFileInfo( fileName ) ) );
    }

    std::shared_ptr< CMediaInfo > CMediaInfoMgr::getMediaInfo( const QFileInfo &fi )
    {
        auto retVal = std::shared_ptr< CMediaInfo >( new CMediaInfo( fi, false ) );
        connect( retVal.get(), &CMediaInfo::sigMediaLoaded, this, &CMediaInfoMgr::slotMediaLoaded );
        fMutex.lock();
        fQueuedMediaInfo[ fi.absoluteFilePath() ] = retVal;
        fMutex.unlock();

        auto queued = retVal->queueLoad( [ this ]( const QString &fileName ) { slotMediaLoaded( fileName ); } );
        if ( !queued )
        {
            removeFromMediaInfoQueue( fi.absoluteFilePath() );
        }

        return retVal;
    }

    bool CMediaInfoMgr::isMediaCached( const QString &fileName ) const
    {
        return isMediaCached( std::move( QFileInfo( fileName ) ) );
    }

    bool CMediaInfoMgr::isMediaCached( const QFileInfo &fi ) const
    {
        return CMediaInfoImpl::mediaExists( fi );
    }

    void CMediaInfoMgr::slotMediaLoaded( const QString &fileName )
    {
        removeFromMediaInfoQueue( fileName );
        emit sigMediaLoaded( fileName );
    }

    void CMediaInfoMgr::removeFromMediaInfoQueue( const QString &fileName )
    {
        fMutex.lock();
        auto pos = fQueuedMediaInfo.find( fileName );
        if ( pos != fQueuedMediaInfo.end() )
            fQueuedMediaInfo.erase( pos );
        fMutex.unlock();
    }

}
