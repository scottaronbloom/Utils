//
// Copyright( c ) 2020-2021 Scott Aron Bloom
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

#ifndef __MKVREADER_MKVREADER_H
#define __MKVREADER_MKVREADER_H

#include <memory>
#include <vector>
#include <QString>
#include <QFile>
namespace NSABUtils
{
    namespace NMKVReader
    {
        class CInfo;
        class CTrack;
        class CAttachment;
        class CChapter;
        class CTag;

        class CMKVFile
        {
        public:
            CMKVFile();
            CMKVFile( const QString & fileName );
            virtual ~CMKVFile();

            bool open( const QString & fileName );

            void close();
            QString errorMsg() const { return fErrorMsg; };
        private:
            QFile fFile;
            QString fErrorMsg;

            std::shared_ptr< CInfo > fInfo;
            std::vector< std::shared_ptr< CTrack > > fTracks;
            std::vector< std::shared_ptr< CAttachment > > fAttachments;
            std::vector< std::shared_ptr< CChapter > > fChapters;
            std::vector< std::shared_ptr< CTag > > fTags;

        };

    }

}


#endif
