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
#ifndef __MEDIAINFO_H
#define __MEDIAINFO_H

#include "SABUtilsExport.h"

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QFileInfo>
#include <unordered_map>
#include <map>
#include <memory>
#include <QObject>

class QFileInfo;
namespace MediaInfoDLL
{
    class MediaInfo;
}

namespace NSABUtils
{
    enum class EStreamType
    {
        eGeneral,
        eVideo,
        eAudio,
        eText,
        eOther,
        eImage,
        eMenu
    };
}

namespace std
{
    template<>
    struct hash< NSABUtils::EStreamType >
    {
        std::size_t operator()( const NSABUtils::EStreamType &ii ) const
        {
            auto tmp = static_cast< int >( ii );
            return std::hash< int >()( tmp );
        }
    };
}

namespace NSABUtils
{
    class CFFMpegFormats;
    enum class EMediaTags
    {
        eFileName,
        eTitle,
        eLengthMS,
        eLengthS,
        eLength,
        eDate,
        eComment,
        eBPM,
        eArtist,
        eComposer,
        eGenre,
        eTrack,
        eAlbum,
        eAlbumArtist,
        eDiscnumber,
        eAspectRatio,
        eWidth,
        eHeight,
        eResolution,
        eFirstVideoCodec,
        eAllVideoCodecs,
        eFirstAudioCodec,
        eAllAudioCodecs,
        eVideoBitrate,   // if not found falls back to overall bitrate
        eVideoBitrateString,
        eOverAllBitrate,
        eOverAllBitrateString,
        eAudioSampleRate,
        eAudioSampleRateString,
        eNumVideoStreams,
        eNumAudioStreams,
        eNumSubtitleStreams,
        eFirstSubtitle,
        eAllSubtitles,
        eLastTag
    };
    SABUTILS_EXPORT QString displayName( EMediaTags tag );
    SABUTILS_EXPORT EMediaTags fromDisplayName( const QString &tag );
    SABUTILS_EXPORT QString getMKVEditName( EMediaTags tag );
    SABUTILS_EXPORT bool isSettableTag( EMediaTags tag );
    SABUTILS_EXPORT QString mediaInfoTagName( EMediaTags tag );
    SABUTILS_EXPORT EMediaTags fromTagName( const QString &tag );

}

namespace std
{
    template<>
    struct hash< NSABUtils::EMediaTags >
    {
        std::size_t operator()( const NSABUtils::EMediaTags &ii ) const
        {
            auto tmp = static_cast< int >( ii );
            return std::hash< int >()( tmp );
        }
    };
}

namespace NSABUtils
{
    enum class EMediaTags;
    class CStreamData;
    class CMediaInfoImpl;
    class SABUTILS_EXPORT CMediaInfo : public QObject
    {
        Q_OBJECT;

    public:
        static void setFFProbeEXE( const QString &path );
        static QString ffprobeEXE();

        CMediaInfo();
        CMediaInfo( const QString &fileName, bool loadNow = true );
        CMediaInfo( const QFileInfo &fi, bool loadNow = true );
        ~CMediaInfo();

        bool load();
        void queueLoad();
        bool aOK() const;

        QString fileName() const;
        QString version() const;

        bool isAudioCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const;
        bool isVideoCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const;
        bool isFormat( const QString &formatName, CFFMpegFormats *ffmpegFormats ) const;
        bool isCodec( const QString &checkCodecName, const QString &mediaCodecName, CFFMpegFormats *ffmpegFormats );
        bool isHEVCCodec( QString mediaCodecName, CFFMpegFormats *ffmpegFormats );

        std::unordered_map< EMediaTags, QString > getSettableMediaTags() const;
        QString getMediaTag( EMediaTags tag ) const;
        std::unordered_map< EMediaTags, QString > getMediaTags( const std::list< EMediaTags > &tags = {} ) const;

        int64_t getBitRate() const;

        int64_t getNumberOfSeconds() const;
        int64_t getNumberOfMSecs() const;

        static int64_t getNumberOfSeconds( const QString &fileName );
        static int64_t getNumberOfMSecs( const QString &fileName );

        static QString getMediaTag( const QString &fileName, EMediaTags tag );
        static std::unordered_map< EMediaTags, QString > getMediaTags( const QString &path, const std::list< EMediaTags > &tags );

        void emitMediaInfoLoaded( const QString &fileName );

        int numAudioStreams() const;
        int numVideoStreams() const;
        int numSubtitleStreams() const;
    Q_SIGNALS:
        void sigMediaInfoLoaded( const QString &fileName );

    private:
        std::shared_ptr< CMediaInfoImpl > fImpl;
    };
}

#endif
