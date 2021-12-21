// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#include <QImage>
#include <QDataStream>
#include <initializer_list>
#include <optional>

class QString;
class QIODevice;

namespace NUtils
{
    struct SGIFPalette
    {
        SGIFPalette( const uint8_t * prevImage, const QImage & image, uint8_t bitDepth, bool dither );
        ~SGIFPalette();

        void getChangedPixels( const uint8_t * prevImage, uint8_t * currImage, int & numPixels );
        void splitPalette( const uint8_t * image, int numPixels, int firstELT, int lastELT, int splitELT, int splitDIST, int treeNodeNum );

        void setRGBToMinMax( const uint8_t * image, int numPixels, int location, bool min );
        void setRGBToAverage( const uint8_t * image, int numPixels, int location );

        int partition( int left, int right, const int elt, int pivot );
        void partitionByMedian( int left, int right, int com, int neededCenter );
        void closestColor( int32_t rr, int32_t gg, int32_t bb, int treeNodeNumber, uint32_t & bestIndex, uint32_t & bestDifference ) const;
        void swap( int pix1, int pix2 );

        bool write( QDataStream & ds );
            
        uint8_t fBitDepth{ 8 };

        void setRed( int location, uint8_t val );
        void setBlue( int location, uint8_t val );
        void setGreen( int location, uint8_t val );

        uint8_t fRed[256];
        uint8_t fGreen[256];
        uint8_t fBlue[256];

        // from online
        // use a kd tree over the RGB space in a heap fashion
        // left of child node is nodeNum * 2, right is NodeNum*2+1
        // nodes 256-2511 are the leaves containing a color
        uint8_t fTreeSplitELT[256];
        uint8_t fTreeSplit[256];
        bool fDither{ false };
        uint8_t * fTmpImage{ nullptr };
    };
   
    class CGIFWriter
    {
    public:
        CGIFWriter();
        ~CGIFWriter();

        static int kTransparentIndex;

        CGIFWriter( const QString & filename );
        CGIFWriter( QIODevice * device );

        void setFileName( const QString & fileName );
        void setDevice( QIODevice * device );

        // must be called before first writeImage!! 
        // numberOfLoops = 0 means infinite
        void setIsMultiFrame( bool value, uint16_t numberOfLoops=0)
        {
            fNumberOfLoops = numberOfLoops;
            fIsMultiFrame = value;
        } 
        bool isMultiFrame() const { return fIsMultiFrame; } // must be set before first writeImage!!

        bool writeImage( const QImage & frame );
        bool writeImage( const QImage & image, std::optional< uint32_t > delay ); // isMultiFrame must be true if delay has value

        bool hasDelay() const { return fDelay.has_value(); }
        void setDelay( int delay ) { fDelay = delay; }
        uint32_t delay() const { return fDelay.has_value() ? fDelay.value() : std::numeric_limits< uint32_t >::max(); }

        void setDither( bool dither ) { fDither = dither; } // default false
        bool dither() const { return fDither; }

        void setFlipImage( bool flipImage ) { fFlipImage = flipImage; } // default false
        bool flipImage() const { return fFlipImage; }

        void setBitDepth( uint8_t bitDepth ) { fBitDepth = bitDepth; }
        uint8_t bitDepth() const { return fBitDepth; }

        static [[nodiscard]] uint8_t * imageToPixels( const QImage & image ); // allocates the space, user is responsible for memory deletion using array delete
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
        bool writeEnd();
        [[nodiscard]] bool status() const;

        void ditherImage( const uint8_t * prevImage );
        void updateQuant( int32_t * quantPixels, int loc, int32_t rErr, int32_t gErr, int32_t bErr );
        void thresholdImage( const uint8_t * prevImage );
        bool writeLZW( uint32_t left, uint32_t top );

        bool writeHeader();

        int numPixels() const;

        bool fDeleteDevice{ false };
        bool fHeaderWritten{ false };
        bool fIsMultiFrame{ false };
        bool fFirstFrame{ true };
        uint16_t fNumberOfLoops{ 0 }; // infinite
        QIODevice * fDevice{ nullptr };
        QImage fCurrImage;
        QDataStream fDataStream;
        uint8_t * fPrevFrameData{ nullptr };
        uint8_t fBitDepth{ 8 };
        bool fDither{ false };
        bool fFlipImage{ false };
        std::optional< uint32_t > fDelay;

        std::unique_ptr< SGIFPalette > fPalette;
    };
}

#endif
