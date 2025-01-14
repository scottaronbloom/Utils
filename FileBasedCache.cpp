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

#include "FileBasedCache.h"
//#include "MKVUtils.h"
//
//#include "utils.h"
//#include "FileUtils.h"
//#include <QFileInfo>
//#include <QRegularExpression>
//#include <QDebug>
//
//#include "MediaInfoDLL/MediaInfoDLL_Static.h"

namespace NSABUtils
{
    SFileBasedCacheNode::SFileBasedCacheNode( const QFileInfo &fileInfo ) :
        fFileInfo( fileInfo ),
        fDateTime( fileInfo.fileTime( QFileDevice::FileModificationTime ) ),
        fSize( fileInfo.size() )
    {
    }

    SFileBasedCacheNode::SFileBasedCacheNode( const QString &path ) :
        SFileBasedCacheNode( QFileInfo( path ) )
    {
    }

    bool SFileBasedCacheNode::operator==( const SFileBasedCacheNode &rhs ) const
    {
        bool aOK = fFileInfo == rhs.fFileInfo;
        if ( pathOnlySearch() || rhs.pathOnlySearch() )
            return aOK;

        aOK = aOK && ( fDateTime == rhs.fDateTime );
        aOK = aOK && ( fSize == rhs.fSize );

        return aOK;
    }

    std::size_t SFileBasedCacheNode::hash() const
    {
        return std::hash< QString >()( fFileInfo.absoluteFilePath() );
    }
}