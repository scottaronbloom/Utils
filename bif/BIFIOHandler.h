// The MIT License( MIT )
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


#ifndef BIFIOHANDLER_H
#define BIFIOHANDLER_H

#include <QImageIOHandler>
namespace NSABUtils
{
    namespace NBIF
    {
        class CFile;

        class CIOHandler : public QImageIOHandler
        {
        public:
            CIOHandler();
            ~CIOHandler();

            bool canRead() const override;
            bool read( QImage * image ) override;
            bool write( const QImage & image ) override;

            static bool canRead( QIODevice * device );

            QVariant option( ImageOption option ) const override;
            void setOption( ImageOption option, const QVariant & value ) override;
            bool supportsOption( ImageOption option ) const override;

            int imageCount() const override;
            int loopCount() const override;
            int nextImageDelay() const override; // returns ms
            int currentImageNumber() const override;
            bool jumpToImage( int imageNumber ) override;
            bool jumpToNextImage() override;
            QRect currentImageRect() const override;

            void setLoopCount( int loopCount );
        private:
            NBIF::CFile * fBIFFile;
            int fCurrentFrame{ -1 };
        };
    }
}

#endif // CBIFIOHandler_P_H
