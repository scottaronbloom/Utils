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
#ifndef __MKVUTILS_H
#define __MKVUTILS_H

#include "SABUtilsExport.h"

#include <cstdint>
#include <unordered_map>
#include <map>
#include <vector>
#include <QString>

namespace MediaInfoDLL
{
    enum stream_t :int;
}

namespace NSABUtils
{
    SABUTILS_EXPORT int64_t getNumberOfSeconds( const QString & fileName );
    SABUTILS_EXPORT int64_t getNumberOfMSecs( const QString & fileName );

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
        eDiscnumber
    };
    QString toString( MediaInfoDLL::stream_t );
    struct SABUTILS_EXPORT SStreamData
    {
        SStreamData( MediaInfoDLL::stream_t type, const QString & name, int num );
        MediaInfoDLL::stream_t fStreamType;
        QString fStreamName;
        int fStreamNum{ 0 };
        std::vector< std::pair< QString, QString > > fStreamData;
        std::map< QString, QString > fStreamDataMap;
    };


    struct SABUTILS_EXPORT SAllMediaInfo
    {
        QString fVersion;
        std::list< SStreamData > fData;
    };


    SABUTILS_EXPORT QString displayName( EMediaTags tag );
    SABUTILS_EXPORT EMediaTags fromDisplayName( const QString & tag );
    SABUTILS_EXPORT QString mediaInfoName( EMediaTags tag );
    SABUTILS_EXPORT QString setMKVName( EMediaTags tag );
    
    SABUTILS_EXPORT QString getMediaTag( const QString & fileName, NSABUtils::EMediaTags tag );
    SABUTILS_EXPORT std::unordered_map< EMediaTags, QString > getMediaTags( const QString & fileName, const std::list< NSABUtils::EMediaTags > & tags = {} );
    SABUTILS_EXPORT std::unordered_map< EMediaTags, QString > getSettableMediaTags( const QString & fileName );
    SABUTILS_EXPORT SAllMediaInfo getAllMediaInfo( const QString & fileName );
    SABUTILS_EXPORT bool setMediaTags( const QString & fileName, const std::unordered_map< EMediaTags, QString > & tags, const QString & mkvPropEdit, QString * msg=nullptr );
}

namespace std
{
    template <>
    struct hash<NSABUtils::EMediaTags>
    {
        std::size_t operator()( const NSABUtils::EMediaTags & ii ) const
        {
            auto tmp = static_cast<int>(ii);
            return std::hash< int >()( tmp );
        }
    };
}

#endif
