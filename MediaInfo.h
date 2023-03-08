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
#include <unordered_map>
#include <map>
#include <memory>

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
        eVideoCodec,
        eAudioCodec,
        eVideoBitrate, // if not found falls back to overall bitrate
        eVideoBitrateString,
        eOverAllBitrate,
        eOverAllBitrateString,
        eAudioBitrate, 
        eAudioBitrateString,
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
    class SABUTILS_EXPORT CStreamData
    {
    public:
        CStreamData() {}
        CStreamData( MediaInfoDLL::MediaInfo *mediaInfo, EStreamType type, int num );

        QString value( const QString &key ) const;
        QString value( EMediaTags key ) const;

        size_t size() const { return fStreamData.size(); }
        std::pair< QString, QString > operator[]( size_t idx ) const { return fStreamData[ idx ]; }

    private:
        EStreamType fStreamType{ EStreamType::eGeneral };
        QString fStreamName;
        int fStreamNum{ 0 };
        std::vector< std::pair< QString, QString > > fStreamData;
        std::map< QString, QString > fStreamDataMap;
        std::map< EMediaTags, QString > fKnownTagStreamDataMap;
    };

    class SABUTILS_EXPORT CMediaInfo
    {
    public:
        CMediaInfo() {}
        CMediaInfo( const QString &fileName );
        ~CMediaInfo();
        
        bool aOK() const { return fAOK; }

        QString version() const { return fVersion; }
        bool isHEVCVideo() const;

        std::vector< std::shared_ptr< CStreamData > > getStreamData( EStreamType whichStream ) const;

        std::unordered_map< EMediaTags, QString > getSettableMediaTags() const;
        QString getMediaTag( EMediaTags tag ) const;
        std::unordered_map< EMediaTags, QString > getMediaTags( const std::list< EMediaTags > &tags = {} ) const;

        int64_t getBitRate() const;

        int64_t getNumberOfSeconds() const;
        int64_t getNumberOfMSecs() const;

        static bool isHEVCVideo( QString codecName );
        static int64_t getNumberOfSeconds( const QString &fileName );
        static int64_t getNumberOfMSecs( const QString &fileName );

        static QString getMediaTag( const QString &fileName, EMediaTags tag );
        static std::unordered_map< EMediaTags, QString > getMediaTags( const QString &path, const std::list< EMediaTags > &tags );
    private :
        void initMediaInfo();
        QString findFirstValue( EStreamType whichStream, const EMediaTags &key ) const;
        void cleanUpValues( std::unordered_map< EMediaTags, QString > &retVal ) const;

        QString fVersion;
        QString fFileName;
        std::shared_ptr< MediaInfoDLL::MediaInfo > fMediaInfo;

        std::unordered_map< EStreamType, std::vector< std::shared_ptr< CStreamData > > > fData;
        bool fAOK{ false };
    };
}
#endif
