// The MIT License( MIT )
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
#include <QObject>
#include <QMutex>
#include <unordered_map>
#include <map>
#include <memory>
#include <set>

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
        eDisplayAspectRatio,
        eWidth,
        eHeight,
        eResolution,
        eFirstVideoCodec,
        eAllVideoCodecs,
        eFirstAudioCodec,
        eFirstAudioCodecDisp,
        eAllAudioCodecs,
        eAllAudioCodecsDisp,
        eVideoBitrate,   // if not found falls back to overall bitrate
        eVideoBitrateString,
        eOverAllBitrate,
        eOverAllBitrateString,
        eBitsPerPixel,
        eBitDepth,
        eFrameRate,
        eFrameRateNum,
        eFrameRateDen,
        eNumChannels,
        eAudioSampleRate,
        eAudioSampleRateString,
        eNumVideoStreams,
        eNumAudioStreams,
        eNumSubtitleStreams,
        eFirstSubtitleLanguage,
        eAllSubtitleLanguages,
        eFirstSubtitleCodec,
        eAllSubtitleCodecs,
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
        friend class CMediaInfoMgr;

    private:
        [[nodiscard]] bool queueLoad( std::function< void( const QString &fileName ) > onFinish );
        CMediaInfo();
        CMediaInfo( const QString &fileName, bool delayLoad );
        CMediaInfo( const QFileInfo &fi, bool delayLoad );

    public:
        static std::pair< int, int > k8KResolution;
        static std::pair< int, int > k4KResolution;
        static std::pair< int, int > kHDResolution;
        static std::pair< int, int > k720Resolution;
        static std::pair< int, int > k480Resolution;

        static void setFFProbeEXE( const QString &path );
        static QString ffprobeEXE();

        CMediaInfo( const QString &fileName );
        CMediaInfo( const QFileInfo &fi );
        ~CMediaInfo();

        bool load();
        bool aOK() const;
        bool isQueued() const;

        QString fileName() const;
        QString version() const;

        bool hasAudioCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const;
        bool hasAACCodec( CFFMpegFormats *ffmpegFormats, int maxNumChannels ) const;
        bool hasVideoCodec( const QString &checkCodecName, CFFMpegFormats *ffmpegFormats ) const;
        bool isContainerFormat( const QString &formatName, CFFMpegFormats *ffmpegFormats ) const;
        bool isCodec( const QString &checkCodecName, const QString &mediaCodecName, CFFMpegFormats *ffmpegFormats );
        bool isHEVCCodec( QString mediaCodecName, CFFMpegFormats *ffmpegFormats );

        std::unordered_map< EMediaTags, QString > getSettableMediaTags() const;
        QString getMediaTag( EMediaTags tag ) const;
        std::unordered_map< EMediaTags, QString > getMediaTags( const std::list< EMediaTags > &tags = {} ) const;

        uint64_t getBitRate() const;
        uint64_t getUncompressedBitRate() const;   // calculated value = width * height * bits per pixel * frames per second
        std::pair< int, int > getResolution() const;
        double getFrameRate() const;
        int getBitsPerPixel() const;

        // threshold is on total pixels per frame
        bool is4kOrBetterResolution( double threshold = 0.2 ) const;
        bool is4kResolution( double threshold = 0.2 ) const;
        bool isHDResolution( double threshold = 0.2 ) const;
        bool isSubHDResolution( double threshold = 0.2 ) const;

        bool isGreaterThanHDResolution( double threshold = 0.0 ) const;   // threshold is EITHER resolution (width or height), so answer is true for 1920-threshold OR 1080- threshold or greater

        bool isResolution( const std::pair< int, int > &target, double threshold = 0.2 ) const;

        int64_t getNumberOfSeconds() const;
        int64_t getNumberOfMSecs() const;

        static int64_t getNumberOfSeconds( const QString &fileName );
        static int64_t getNumberOfMSecs( const QString &fileName );

        static QString getMediaTag( const QString &fileName, EMediaTags tag );
        static std::unordered_map< EMediaTags, QString > getMediaTags( const QString &path, const std::list< EMediaTags > &tags );

        int numAudioStreams() const;
        int numVideoStreams() const;
        int numSubtitleStreams() const;

        QStringList allSubtitleCodecs() const;
    Q_SIGNALS:
        void sigMediaLoaded( const QString &fileName );

    private:
        bool isResolutionOrGreater( const std::pair< int, int > &min, double threshold = 0.2 ) const;
        bool isResolutionOrLess( const std::pair< int, int > &max, double threshold = 0.2 ) const;

        std::shared_ptr< CMediaInfoImpl > fImpl;
    };

    class SABUTILS_EXPORT CMediaInfoMgr : public QObject
    {
        CMediaInfoMgr() {}
        Q_OBJECT;

    public:
        static CMediaInfoMgr *instance();
        virtual ~CMediaInfoMgr() override {}

        std::shared_ptr< CMediaInfo > getMediaInfo( const QString &fileName );   // creates a mediainfo, then loads it in a background thread, then CMediaInfo will emit when its finished
        std::shared_ptr< CMediaInfo > getMediaInfo( const QFileInfo &fi );

        bool isMediaCached( const QString &fileName ) const;
        bool isMediaCached( const QFileInfo &fi ) const;

    public Q_SLOTS:
        void slotMediaLoaded( const QString &fileName );
    Q_SIGNALS:
        void sigMediaLoaded( const QString &fileName );

    private:
        void removeFromMediaInfoQueue( const QString &fileName );

        QMutex fMutex;
        std::unordered_map< QString, std::shared_ptr< CMediaInfo > > fQueuedMediaInfo;   // store here while media is loading
    };
}

#endif
