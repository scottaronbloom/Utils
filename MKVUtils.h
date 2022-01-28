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

#include <cstdint>
#include <unordered_map>
#include <QString>
namespace NSABUtils
{
    int64_t getNumberOfSeconds( const QString & fileName );
    int64_t getNumberOfMSecs( const QString & fileName );

    enum class EMediaTags
    {
        eArtist,
        eBPM,
        eComment,
        eComposer,
        eGenre,
        eTitle,
        eTrack,
        eDateRecorded,
        eLength
    };

    std::unordered_map< QString, QString > getMediaTags( const QString & fileName, const QString & ffprobe );

    bool setMediaTags( const QString & fileName, const std::unordered_map< QString, QString > & tags, const QString & mkvPropEdit, QString * msg=nullptr );
}

#endif
