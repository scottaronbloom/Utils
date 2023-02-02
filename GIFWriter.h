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

#ifndef __GIFWRITER_H
#define __GIFWRITER_H

#include "SABUtilsExport.h"

#include <QImage>
#include <QDataStream>
#include <initializer_list>
#include <optional>
#include <memory>

class QString;
class QIODevice;

namespace NSABUtils
{
    struct SGIFPalette;

    class SABUTILS_EXPORT CGIFWriter
    {
    public:
        static int kTransparentIndex;

        CGIFWriter();
        CGIFWriter( const QString & filename );
        CGIFWriter( QIODevice * device );

        ~CGIFWriter();

        void setFileName( const QString & fileName );
        void setDevice( QIODevice * device );
        void close();

        uint16_t loopCount() const { return fLoopCount; }
        void setLoopCount( uint16_t loopCount )
        {
            fLoopCount = loopCount;
        }

        bool writeHeader();

        bool writeImage( const QImage & image, bool lastImage = true );

        bool writeEnd();

        void setDelay( int delay ) { fDelay = delay; }
        uint32_t delay() const { return fDelay; }

        void setDither( bool dither ) { fDither = dither; } // default false
        bool dither() const { return fDither; }

        void setFlipImage( bool flipImage ) { fFlipImage = flipImage; } // default false
        bool flipImage() const { return fFlipImage; }

        void setBitDepth( uint8_t bitDepth ) { fBitDepth = bitDepth; }
        uint8_t bitDepth() const { return fBitDepth; }

        static bool pixelCompare( const uint8_t * lhs, const uint8_t * rhs, int pixelNum );
        static bool pixelCompare( const uint8_t * lhs, const uint8_t * rhs ); // pixel is at its 0,1,2
        static bool pixelCompare( const uint8_t * lhs, const std::initializer_list< uint32_t > & rhs ); // pixel is at its 0,1,2 of lhs

        static bool status( const QDataStream & ds );
        static bool writeChar( uint8_t ch, QDataStream & ds );
        static bool writeInt( uint16_t value, QDataStream & ds );
        static bool writeString( const char * str, QDataStream & ds );
        static bool writeRaw( const char * str, int len, QDataStream & ds );
    private:
        bool writeCurrImage();
        bool writeChar( uint8_t ch );
        bool writeInt( uint16_t value );
        bool writeString( const char * str );
        bool writeRaw( const char * str, int len );
        [[nodiscard]] bool status() const;
        void ditherImage( const uint8_t * prevImage );
        void updateQuant( int32_t * quantPixels, int loc, int32_t rErr, int32_t gErr, int32_t bErr, int quantMultiplier );
        void thresholdImage( const uint8_t * prevImage );
        bool writeLZW( uint32_t left, uint32_t top, const uint8_t * prevImage );

        int numPixels() const;

        bool fDeleteDevice{ false };
        bool fHeaderWritten{ false };
        bool fFirstFrame{ true };
        uint16_t fLoopCount{ 0 }; // infinite
        QIODevice * fDevice{ nullptr };
        QImage fCurrImage;
        QDataStream fDataStream;
        uint8_t * fPrevFrameData{ nullptr };
        uint8_t fBitDepth{ 8 };
        bool fDither{ false };
        bool fFlipImage{ false };
        uint32_t fDelay{ 5 };

        std::unique_ptr< SGIFPalette > fPalette;
    };
}
#endif
