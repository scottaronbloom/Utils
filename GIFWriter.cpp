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

#include "GIFWriter.h"
#include "QtUtils.h"

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QProgressDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>

#ifdef _ALLOW_OPENSOURCEGIFWRITER_H
    #include "gif/gif-h/gif.h"
#endif

namespace NSABUtils
{
    struct SGIFPalette
    {
        SGIFPalette( const uint8_t *prevImage, const QImage &image, uint8_t bitDepth, bool dither );
        ~SGIFPalette();

        void getChangedPixels( const uint8_t *prevImage, uint8_t *currImage, int &numPixels );
        void splitPalette( uint8_t *image, int numPixels, int firstELT, int lastELT, int splitELT, int splitDIST, int treeNodeNum );

        std::tuple< uint8_t, int, int > compuiteRGBRanges( int numPixels, const uint8_t *image, int splitELT, int firstELT, int lastELT );

        void setRGBToMinMax( const uint8_t *image, int numPixels, int location, bool min );
        void setRGBToAverage( const uint8_t *image, int numPixels, int location );

        int partition( uint8_t *image, int left, int right, const int elt, int pivot );
        void partitionByMedian( uint8_t *image, int left, int right, int com, int neededCenter );
        void closestColor( int32_t rr, int32_t gg, int32_t bb, int treeNodeNumber, uint32_t &bestIndex, uint32_t &bestDifference ) const;
        void swap( uint8_t *image, int pix1, int pix2 );

        bool write( QDataStream &ds );

        uint8_t fBitDepth{ 8 };

        void setRed( int location, uint8_t val );
        void setBlue( int location, uint8_t val );
        void setGreen( int location, uint8_t val );

        void dumpIt();
        QString dumpText() const;

        uint8_t fRed[ 256 ]{ 0 };
        uint8_t fGreen[ 256 ]{ 0 };
        uint8_t fBlue[ 256 ]{ 0 };

        // from online
        // use a kd tree over the RGB space in a heap fashion
        // left of child node is nodeNum * 2, right is NodeNum*2+1
        // nodes 256-2511 are the leaves containing a color
        uint8_t fTreeSplitELT[ 256 ]{ 0 };
        uint8_t fTreeSplit[ 256 ]{ 0 };
        bool fDither{ false };
        uint8_t *fTmpImage{ nullptr };
        int fImageWidth{ 0 };
    };

    int CGIFWriter::kTransparentIndex{ 0 };

    CGIFWriter::CGIFWriter()
    {
    }

    CGIFWriter::CGIFWriter( const QString &filename ) :
        CGIFWriter( new QFile( filename ) )
    {
        fDeleteDevice = true;
        fDevice->open( QIODevice::WriteOnly | QIODevice::Truncate );
    }

    CGIFWriter::CGIFWriter( QIODevice *device ) :
        fDevice( device ),
        fDataStream( device )
    {
    }

    CGIFWriter::~CGIFWriter()
    {
        setDevice( nullptr );
        if ( fPrevFrameData )
            delete[] fPrevFrameData;
    }

    void CGIFWriter::setFileName( const QString &fileName )
    {
        setDevice( new QFile( fileName ) );
        fDeleteDevice = true;
    }

    void CGIFWriter::setDevice( QIODevice *device )
    {
        if ( fDeleteDevice )
            delete fDevice;
        fDevice = device;
        fDataStream.setDevice( fDevice );

        fDeleteDevice = false;
    }

    void CGIFWriter::close()
    {
        fDevice->close();
    }

    bool CGIFWriter::status() const
    {
        return status( fDataStream );
    }

    bool CGIFWriter::status( const QDataStream &ds )
    {
        return ds.status() == QDataStream::Ok && ds.device()->isOpen() && ds.device()->isWritable();
    }

    bool CGIFWriter::writeChar( uint8_t ch )
    {
        return writeChar( ch, fDataStream );
    }

    bool CGIFWriter::writeChar( uint8_t ch, QDataStream &ds )
    {
        if ( !status( ds ) )
            return false;
        ds << ch;
        return status( ds );
    }

    bool CGIFWriter::writeInt( uint16_t val )
    {
        return writeInt( val, fDataStream );
    }

    bool CGIFWriter::writeInt( uint16_t val, QDataStream &ds )
    {
        if ( !status( ds ) )
            return false;
        ds << (uint8_t)( val & 0xff );
        ds << (uint8_t)( ( val >> 8 ) & 0xff );
        return status( ds );
    }

    bool CGIFWriter::writeString( const char *str )
    {
        return writeString( str, fDataStream );
    }

    bool CGIFWriter::writeString( const char *str, QDataStream &ds )
    {
        uint len = qstrlen( str );
        return writeRaw( str, len, ds );
    }

    bool CGIFWriter::writeRaw( const char *str, int len )
    {
        return writeRaw( str, len, fDataStream );
    }

    bool CGIFWriter::writeRaw( const char *str, int len, QDataStream &ds )
    {
        if ( !status( ds ) )
            return false;
        ds.writeRawData( str, len );
        return status( ds );
    }

    bool CGIFWriter::writeHeader()
    {
        if ( fHeaderWritten )
            return true;

        if ( !status() )
            return false;

        if ( !fDevice )
            return false;

        if ( !fDevice->isOpen() || !fDevice->isWritable() )
            return false;

        fFirstFrame = true;
        auto numBytes = numPixels() * (uint8_t)4;
        fPrevFrameData = new uint8_t[ numBytes ];
        std::memset( fPrevFrameData, 0, numBytes );

        writeString( "GIF" );   // signature
        writeString( "89a" );   // version
        writeInt( fCurrImage.width() );
        writeInt( fCurrImage.height() );

        writeChar( 0xf0 );   // global color table of 2 entries
        writeChar( 0x0 );   // background color
        writeChar( 0x0 );   // pixel aspect ratio

        // global palette
        // color 0 is black
        writeChar( 0x0 );
        writeChar( 0x0 );
        writeChar( 0x0 );

        // color 1 is black
        writeChar( 0x0 );
        writeChar( 0x0 );
        writeChar( 0x0 );

        writeChar( 0x21 );   // extension block
        writeChar( 0xff );   // its an app specific extension
        writeChar( 11 );   // length 11
        writeString( "NETSCAPE2.0" );
        writeChar( 3 );   // 3 bytes of NETSCAPE2.0 data
        writeChar( 1 );   // interwebs say so
        writeInt( fLoopCount );
        writeChar( 0 );   // end of extension block

        fHeaderWritten = true;
        return status();
    }

    bool CGIFWriter::writeEnd()
    {
        if ( !status() )
            return false;

        writeChar( 0x3b );

        return status();
    }

    int CGIFWriter::numPixels() const
    {
        return fCurrImage.width() * fCurrImage.height();
    }

    bool CGIFWriter::writeImage( const QImage &image, bool lastFrame )
    {
        fCurrImage = image;
        writeHeader();
        if ( !status() )
            return false;

        writeCurrImage();

        if ( lastFrame )
            writeEnd();

        return status();
    }

    bool CGIFWriter::pixelCompare( const uint8_t *lhs, const uint8_t *rhs, int pixelNum )
    {
        return ( lhs[ pixelNum ] == rhs[ pixelNum ] ) && ( lhs[ pixelNum + 1 ] == rhs[ pixelNum + 1 ] ) && ( lhs[ pixelNum + 2 ] == rhs[ pixelNum + 2 ] );
    }

    bool CGIFWriter::pixelCompare( const uint8_t *lhs, const uint8_t *rhs )
    {
        return pixelCompare( lhs, rhs, 0 );
    }

    bool CGIFWriter::pixelCompare( const uint8_t *lhs, const std::initializer_list< uint32_t > &rhs )
    {
        if ( rhs.size() != 3 )
            return false;
        auto ii = rhs.begin();
        auto jj = lhs;
        for ( ; ii != rhs.end(); ++jj, ++ii )
        {
            if ( *jj != *ii )
                return false;
        }
        return true;
    }

    void CGIFWriter::ditherImage( const uint8_t *prevImage )
    {
        Q_ASSERT( fPalette );
        if ( !fPalette )
            return;

        auto numPixels = this->numPixels();

        auto quantPixels = new int32_t[ sizeof( int32_t ) * numPixels * 4 ];   // has to support more than 8 bits
        auto imagePixels = NSABUtils::imageToPixels( fCurrImage );

        for ( int ii = 0; ii < 4 * numPixels; ++ii )
        {
            auto pix = imagePixels[ ii ];
            auto pix256 = static_cast< uint32_t >( pix ) * 256;
            quantPixels[ ii ] = pix256;
        }

        for ( int currRow = 0; currRow < fCurrImage.height(); ++currRow )
        {
            for ( int currCol = 0; currCol < fCurrImage.width(); ++currCol )
            {
                auto pixelNumber = ( currRow * fCurrImage.width() ) + currCol;
                auto byteNumber = 4 * pixelNumber;
                int32_t *nextPixel = quantPixels + byteNumber;
                const auto lastPix = prevImage ? ( prevImage + byteNumber ) : nullptr;

                uint32_t rr = ( nextPixel[ 0 ] + 127 ) / 256;
                uint32_t gg = ( nextPixel[ 1 ] + 127 ) / 256;
                uint32_t bb = ( nextPixel[ 2 ] + 127 ) / 256;

                if ( prevImage && ( pixelCompare( prevImage, { rr, gg, bb } ) ) )
                {
                    nextPixel[ 0 ] = rr;
                    nextPixel[ 1 ] = gg;
                    nextPixel[ 2 ] = bb;
                    nextPixel[ 3 ] = kTransparentIndex;
                    continue;
                }

                uint32_t bestIndex = kTransparentIndex;
                uint32_t bestDifference = 1000000;
                fPalette->closestColor( rr, gg, bb, 1, bestIndex, bestDifference );

                int32_t rErr = nextPixel[ 0 ] - (int32_t)fPalette->fRed[ bestIndex ] * 256;
                int32_t gErr = nextPixel[ 1 ] - (int32_t)fPalette->fGreen[ bestIndex ] * 256;
                int32_t bErr = nextPixel[ 2 ] - (int32_t)fPalette->fBlue[ bestIndex ] * 256;

                nextPixel[ 0 ] = fPalette->fRed[ bestIndex ];
                nextPixel[ 1 ] = fPalette->fGreen[ bestIndex ];
                nextPixel[ 2 ] = fPalette->fBlue[ bestIndex ];
                nextPixel[ 3 ] = bestIndex;

                // propagate the error to the adjacent locations
                auto quantLoc7 = pixelNumber + 1;   // to the right
                auto quantLoc3 = pixelNumber + fCurrImage.width() - 1;   // next rowleft
                auto quantLoc5 = pixelNumber + fCurrImage.width();   // next row
                auto quantLoc1 = pixelNumber + fCurrImage.width() + 1;   // next row righ

                updateQuant( quantPixels, quantLoc7, rErr, gErr, bErr, 7 );
                updateQuant( quantPixels, quantLoc3, rErr, gErr, bErr, 3 );
                updateQuant( quantPixels, quantLoc5, rErr, gErr, bErr, 5 );
                updateQuant( quantPixels, quantLoc1, rErr, gErr, bErr, 1 );
            }
        }

        int numBytes = 0;
        for ( int ii = 0; ii < numPixels * 4; ++ii )
        {
            fPrevFrameData[ ii ] = static_cast< uint8_t >( quantPixels[ ii ] );
            numBytes++;
        }

        delete[] imagePixels;
        delete[] quantPixels;
    }

    void CGIFWriter::updateQuant( int32_t *quantPixels, int loc, int32_t rErr, int32_t gErr, int32_t bErr, int quantMultiplier )
    {
        if ( loc < numPixels() )
        {
            auto pixel = quantPixels + 4 * loc;
            rErr = rErr * quantMultiplier / 16;
            gErr = gErr * quantMultiplier / 16;
            bErr = bErr * quantMultiplier / 16;

            pixel[ 0 ] += std::max( -pixel[ 0 ], rErr );
            pixel[ 1 ] += std::max( -pixel[ 1 ], gErr );
            pixel[ 2 ] += std::max( -pixel[ 2 ], bErr );
        }
    }

    void CGIFWriter::thresholdImage( const uint8_t *prevImage )
    {
        auto numPixels = this->numPixels();

        auto imagePixels = NSABUtils::imageToPixels( fCurrImage );

        auto imageLoc = imagePixels;
        auto lastLoc = prevImage;
        auto outLoc = fPrevFrameData;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            if ( lastLoc && pixelCompare( imageLoc, lastLoc ) )
            {
                outLoc[ 0 ] = imageLoc[ 0 ];
                outLoc[ 1 ] = imageLoc[ 1 ];
                outLoc[ 2 ] = imageLoc[ 2 ];
                outLoc[ 3 ] = kTransparentIndex;
            }
            else
            {
                uint32_t bestIndex = 1;
                uint32_t bestDifference = 1000000;
                fPalette->closestColor( imageLoc[ 0 ], imageLoc[ 1 ], imageLoc[ 2 ], 1, bestIndex, bestDifference );
                outLoc[ 0 ] = fPalette->fRed[ bestIndex ];
                outLoc[ 1 ] = fPalette->fGreen[ bestIndex ];
                outLoc[ 2 ] = fPalette->fBlue[ bestIndex ];
                outLoc[ 3 ] = bestIndex;
            }

            if ( lastLoc )
                lastLoc += 4;
            outLoc += 4;
            imageLoc += 4;
        }
    }

    struct SLZWNode
    {
        uint16_t fNext[ 256 ];
    };

    struct SBitStatus
    {
        SBitStatus( QDataStream &ds ) :
            fDataStream( ds )
        {
        }

        void write( uint32_t bit )
        {
            //qDebug().noquote().nospace() << "WriteBit: " << bit;
            bit = bit & 1;
            bit = bit << fBitIndex;
            fByte |= bit;

            ++fBitIndex;
            if ( fBitIndex > 7 )
            {
                fChunk[ fChunkIndex++ ] = fByte;
                fBitIndex = 0;
                fByte = 0;
            }
            //dump();
        }

        void write()
        {
            //qDebug().noquote().nospace() << "WriteChunk: " << fBitIndex << " " << fByte << " " << fChunkIndex;
            CGIFWriter::writeChar( fChunkIndex, fDataStream );
            CGIFWriter::writeRaw( (const char *)fChunk, fChunkIndex, fDataStream );

            fBitIndex = 0;
            fByte = 0;
            fChunkIndex = 0;
            //dump();
        }

        void writeFooter( int currCode, uint32_t codeSize, uint32_t clearCode, int minCodeSize )
        {
            write( currCode, codeSize );
            write( clearCode, codeSize );
            write( clearCode + 1, minCodeSize + 1 );
            while ( fBitIndex )
                write( 0 );
            if ( fChunkIndex )
                write();
        }

        void write( uint32_t code, uint32_t length )
        {
            for ( uint32_t ii = 0; ii < length; ++ii )
            {
                write( code );
                code = code >> 1;
                if ( fChunkIndex == 255 )
                {
                    write();
                }
            }
            //dump();
        }

        void dump() const
        {
            qDebug().noquote().nospace() << "dump: " << fBitIndex << " " << fByte << " " << fChunkIndex;
            qDebug().noquote().nospace() << NSABUtils::dumpArray( "Palette Status", fChunk, fChunk, 256, true, 32 );
        }

        QDataStream &fDataStream;
        uint8_t fBitIndex{ 0 };
        uint8_t fByte{ 0 };
        uint32_t fChunkIndex{ 0 };
        uint8_t fChunk[ 256 ] = { 0 };
    };

    bool CGIFWriter::writeLZW( uint32_t left, uint32_t top, const uint8_t *imagePixels )
    {
        if ( !status() )
            return false;

        writeChar( 0x21 );
        writeChar( 0xf9 );
        writeChar( 0x04 );
        writeChar( 0x05 );
        writeInt( delay() );
        writeChar( kTransparentIndex );
        writeChar( 0 );

        writeChar( 0x2c );

        writeInt( left );
        writeInt( top );

        writeInt( fCurrImage.width() );
        writeInt( fCurrImage.height() );

        fPalette->write( fDataStream );

        const auto minCodeSize = fPalette->fBitDepth;
        const auto clearCode = 1 << fPalette->fBitDepth;

        writeChar( minCodeSize );

        std::vector< SLZWNode > codeTree( 4096 );
        int currCode = -1;
        uint32_t codeSize = minCodeSize + 1;
        uint32_t maxCode = clearCode + 1;

        SBitStatus bitStatus( fDataStream );
        bitStatus.write( clearCode, codeSize );

        auto height = fCurrImage.height();
        auto width = fCurrImage.width();
        for ( int currRow = 0; currRow < height; ++currRow )
        {
            for ( int currCol = 0; currCol < width; ++currCol )
            {
                auto pixelNumber = ( currRow * width ) + currCol;
                auto byteNumber = 4 * pixelNumber;

                auto nextValue = fFlipImage ? imagePixels[ ( ( height - 1 - currRow ) * width + currCol ) * 4 + 3 ] : imagePixels[ byteNumber + 3 ];

                if ( currCode < 0 )
                {
                    currCode = nextValue;
                }
                else if ( codeTree[ currCode ].fNext[ nextValue ] )
                {
                    currCode = codeTree[ currCode ].fNext[ nextValue ];
                }
                else
                {
                    bitStatus.write( currCode, codeSize );
                    codeTree[ currCode ].fNext[ nextValue ] = ++maxCode;

                    if ( maxCode >= ( 1ul << codeSize ) )
                        codeSize++;
                    if ( maxCode == 4095 )
                    {
                        bitStatus.write( clearCode, codeSize );
                        codeTree = std::vector< SLZWNode >( 4096 );
                        codeSize = minCodeSize + 1;
                        maxCode = clearCode + 1;
                    }
                    currCode = nextValue;
                }
            }
        }
        bitStatus.writeFooter( currCode, codeSize, clearCode, minCodeSize );
        writeChar( 0 );
        return status();
    }

    bool CGIFWriter::writeCurrImage()
    {
        if ( !status() )
            return false;

        auto prevImage = fFirstFrame ? nullptr : fPrevFrameData;
        fFirstFrame = false;

        fPalette = std::make_unique< SGIFPalette >( dither() ? nullptr : prevImage, fCurrImage, fBitDepth, dither() );

        if ( dither() )
            ditherImage( prevImage );
        else
            thresholdImage( prevImage );

        return writeLZW( 0, 0, fPrevFrameData );
    }

    SGIFPalette::SGIFPalette( const uint8_t *prevImage, const QImage &image, uint8_t bitDepth, bool dither ) :
        fBitDepth( bitDepth ),
        fDither( dither )
    {
        fImageWidth = image.width();
        fTmpImage = NSABUtils::imageToPixels( image );
        int numPixels = image.width() * image.height();

        getChangedPixels( prevImage, fTmpImage, numPixels );

        const auto lastELT = 1 << bitDepth;
        const auto splitELT = lastELT / 2;
        const auto splitDist = splitELT / 2;

        splitPalette( fTmpImage, numPixels, 1, lastELT, splitELT, splitDist, 1 );
        uint32_t pos = (uint32_t)1 << ( bitDepth - 1 );
        fTreeSplit[ pos ] = 0;
        fTreeSplitELT[ pos ] = 0;
        setRed( 0, 0 );
        setGreen( 0, 0 );
        setBlue( 0, 0 );
    }

    SGIFPalette::~SGIFPalette()
    {
        if ( fTmpImage )
            delete[] fTmpImage;
    }

    void SGIFPalette::splitPalette( uint8_t *image, int numPixels, int firstELT, int lastELT, int splitELT, int splitDIST, int treeNodeNum )
    {
        if ( ( lastELT <= firstELT ) || ( numPixels == 0 ) )
            return;
        if ( lastELT == ( firstELT + 1 ) )
        {
            if ( fDither )
            {
                if ( firstELT == 1 )
                {
                    setRGBToMinMax( image, numPixels, firstELT, true );
                    return;
                }

                if ( firstELT == ( 1 << fBitDepth ) - 1 )
                {
                    setRGBToMinMax( image, numPixels, firstELT, false );
                    return;
                }
            }
            setRGBToAverage( image, numPixels, firstELT );
            return;
        }

        uint8_t splitOffset = -1;
        int subPixelsA = -1;
        int subPixelsB = -1;
        std::tie( splitOffset, subPixelsA, subPixelsB ) = compuiteRGBRanges( numPixels, image, splitELT, firstELT, lastELT );

        partitionByMedian( image, 0, numPixels, splitOffset, subPixelsA );

        fTreeSplitELT[ treeNodeNum ] = splitOffset;
        fTreeSplit[ treeNodeNum ] = image[ subPixelsA * 4 + splitOffset ];

        splitPalette( image, subPixelsA, firstELT, splitELT, splitELT - splitDIST, splitDIST / 2, treeNodeNum * 2 );
        splitPalette( image + subPixelsA * 4, subPixelsB, splitELT, lastELT, splitELT + splitDIST, splitDIST / 2, ( treeNodeNum * 2 ) + 1 );
    }

    std::tuple< uint8_t, int, int > SGIFPalette::compuiteRGBRanges( int numPixels, const uint8_t *image, int splitELT, int firstELT, int lastELT )
    {
        uint8_t minR = 255;
        uint8_t minG = 255;
        uint8_t minB = 255;

        uint8_t maxR = 0;
        uint8_t maxG = 0;
        uint8_t maxB = 0;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            auto r = image[ ii * 4 + 0 ];
            auto g = image[ ii * 4 + 1 ];
            auto b = image[ ii * 4 + 2 ];

            maxR = std::max( r, maxR );
            minR = std::min( r, minR );

            maxG = std::max( g, maxG );
            minG = std::min( g, minG );

            maxB = std::max( b, maxB );
            minB = std::min( b, minB );
        }

        auto rRange = maxR - minR;
        auto gRange = maxG - minG;
        auto bRange = maxB - minB;

        uint8_t splitOffset = 1;
        if ( bRange > gRange )
            splitOffset = 2;
        if ( ( rRange > bRange ) && ( rRange > gRange ) )
            splitOffset = 0;

        auto subPixelsA = numPixels * ( splitELT - firstELT ) / ( lastELT - firstELT );
        auto subPixelsB = numPixels - subPixelsA;
        return std::make_tuple( splitOffset, subPixelsA, subPixelsB );
    }

    void SGIFPalette::swap( uint8_t *image, int lhs, int rhs )
    {
        for ( int ii = 0; ii <= 3; ++ii )
            std::swap( image[ ( lhs * 4 ) + ii ], image[ ( rhs * 4 ) + ii ] );
    }

    int SGIFPalette::partition( uint8_t *image, int left, int right, const int elt, int pivot )
    {
        auto pivotValue = image[ ( pivot * 4 ) + elt ];
        swap( image, pivot, right - 1 );
        auto storedIndex = left;
        bool split = false;
        for ( int ii = left; ii < right - 1; ++ii )
        {
            auto val = image[ ( ii * 4 ) + elt ];
            if ( val < pivotValue )
            {
                swap( image, ii, storedIndex );
                ++storedIndex;
            }
            else if ( val == pivotValue )
            {
                if ( split )
                {
                    swap( image, ii, storedIndex );
                    ++storedIndex;
                }
                split = !split;
            }
        }
        swap( image, storedIndex, right - 1 );
        return storedIndex;
    }

    void SGIFPalette::partitionByMedian( uint8_t *image, int left, int right, int com, int neededCenter )
    {
        if ( left < right - 1 )
        {
            auto pivot = left + ( right - left ) / 2;
            pivot = partition( image, left, right, com, pivot );
            if ( pivot > neededCenter )
                partitionByMedian( image, left, pivot, com, neededCenter );

            if ( pivot < neededCenter )
                partitionByMedian( image, pivot + 1, right, com, neededCenter );
        }
    }

    void SGIFPalette::setRGBToAverage( const uint8_t *image, int numPixels, int location )
    {
        uint64_t r = 0;
        uint64_t g = 0;
        uint64_t b = 0;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            r += image[ ii * 4 + 0 ];
            g += image[ ii * 4 + 1 ];
            b += image[ ii * 4 + 2 ];
        }

        r += ( (uint64_t)numPixels ) / 2;
        g += ( (uint64_t)numPixels ) / 2;
        b += ( (uint64_t)numPixels ) / 2;

        r /= (uint64_t)numPixels;
        g /= (uint64_t)numPixels;
        b /= (uint64_t)numPixels;

        setRed( location, r );
        setGreen( location, g );
        setBlue( location, b );
    }

    void SGIFPalette::closestColor( int32_t rr, int32_t gg, int32_t bb, int treeNodeNumber, uint32_t &bestIndex, uint32_t &bestDifference ) const
    {
        if ( treeNodeNumber > ( 1 << fBitDepth ) - 1 )
        {
            int index = treeNodeNumber - ( 1 << fBitDepth );
            if ( index != CGIFWriter::kTransparentIndex )
            {
                auto rError = rr - (int32_t)fRed[ index ];
                auto gError = gg - (int32_t)fGreen[ index ];
                auto bError = bb - (int32_t)fBlue[ index ];

                auto diff = static_cast< uint32_t >( std::abs( rError ) + std::abs( gError ) + std::abs( bError ) );
                if ( diff < bestDifference )
                {
                    bestIndex = index;
                    bestDifference = diff;
                }
            }
        }
        else
        {
            int32_t comps[ 3 ] = { rr, gg, bb };
            uint32_t splitCompare = comps[ fTreeSplitELT[ treeNodeNumber ] ];
            auto splitPos = fTreeSplit[ treeNodeNumber ];
            if ( splitPos > splitCompare )
            {
                closestColor( rr, gg, bb, treeNodeNumber * 2, bestIndex, bestDifference );
                if ( bestDifference > ( splitPos - splitCompare ) )
                {
                    closestColor( rr, gg, bb, ( treeNodeNumber * 2 ) + 1, bestIndex, bestDifference );
                }
            }
            else
            {
                closestColor( rr, gg, bb, ( treeNodeNumber * 2 ) + 1, bestIndex, bestDifference );
                if ( bestDifference > ( splitCompare - splitPos ) )
                {
                    closestColor( rr, gg, bb, treeNodeNumber * 2, bestIndex, bestDifference );
                }
            }
        }
    }

    void SGIFPalette::setRGBToMinMax( const uint8_t *image, int numPixels, int location, bool min )
    {
        uint8_t r = min ? 255 : 0;
        uint8_t g = min ? 255 : 0;
        uint8_t b = min ? 255 : 0;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            auto currR = image[ ii * 4 + 0 ];
            auto currG = image[ ii * 4 + 1 ];
            auto currB = image[ ii * 4 + 2 ];
            r = min ? std::min( r, currR ) : std::max( r, currR );
            g = min ? std::min( g, currG ) : std::max( g, currG );
            b = min ? std::min( b, currB ) : std::max( b, currB );
        }

        setRed( location, r );
        setGreen( location, g );
        setBlue( location, b );
    }

    void SGIFPalette::getChangedPixels( const uint8_t *prevImage, uint8_t *currImage, int &numPixels )
    {
        if ( !prevImage )
            return;

        int retVal = 0;

        auto writePos = currImage;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            if ( !CGIFWriter::pixelCompare( prevImage, currImage, ii ) )
            {
                writePos[ 0 ] = currImage[ ii ];
                writePos[ 1 ] = currImage[ ii + 1 ];
                writePos[ 2 ] = currImage[ ii + 2 ];

                ++retVal;
                writePos += 4;
            }
            prevImage += 4;
            currImage += 4;
        }
        numPixels = retVal;
    }

    bool SGIFPalette::write( QDataStream &ds )
    {
        CGIFWriter::writeChar( 0x80 + fBitDepth - 1, ds );

        CGIFWriter::writeChar( 0, ds );
        CGIFWriter::writeChar( 0, ds );
        CGIFWriter::writeChar( 0, ds );

        for ( int ii = 1; CGIFWriter::status( ds ) && ( ii < ( 1 << fBitDepth ) ); ++ii )
        {
            auto rr = fRed[ ii ];
            auto gg = fGreen[ ii ];
            auto bb = fBlue[ ii ];

            CGIFWriter::writeChar( rr, ds );
            CGIFWriter::writeChar( gg, ds );
            CGIFWriter::writeChar( bb, ds );
        }
        return CGIFWriter::status( ds );
    }

    void SGIFPalette::setRed( int location, uint8_t val )
    {
        fRed[ location ] = val;
    }

    void SGIFPalette::setBlue( int location, uint8_t val )
    {
        fBlue[ location ] = val;
    }

    void SGIFPalette::setGreen( int location, uint8_t val )
    {
        fGreen[ location ] = val;
    }

    void SGIFPalette::dumpIt()
    {
        qDebug().noquote().nospace() << dumpText();
    }

    QString SGIFPalette::dumpText() const
    {
        QString retVal;
        retVal = "Red:\n" + NSABUtils::dumpArray( "Palette Red", fRed, fRed, 256, true ) + "\n" + "Blue:\n" + NSABUtils::dumpArray( "Palette Blue", fBlue, fBlue, 256, true ) + "\n" + "Green:\n" + NSABUtils::dumpArray( "Palette Green", fGreen, fGreen, 256, true ) + "\n";
        return retVal;
    }

    bool CGIFWriter::saveToGIF( QWidget *parent, const QString &fileName, const QList< QImage > &images, bool useNew, bool dither, bool flipImage, int loopCount, int delay, std::function< void( size_t min, size_t max ) > setRange, std::function< void( size_t curr ) > setCurr, std::function< bool() > wasCancelledFunc )
    {
        if ( fileName.isEmpty() )
            return true;

        if ( QFileInfo( fileName ).exists() )
        {
            if ( QMessageBox::warning( parent, QObject::tr( "File already exists" ), QObject::tr( "'%1' already exists, are you sure?" ).arg( QFileInfo( fileName ).fileName() ), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
            {
                return false;
            }
        }

        setRange( 0, images.size() );

        std::unique_ptr< CGIFWriter > newWriter;
#ifdef gif_h
        std::unique_ptr< GifWriter > oldWriter;
#endif
        if ( useNew )
        {
            newWriter = std::make_unique< CGIFWriter >( fileName );
            newWriter->setDither( dither );
            newWriter->setFlipImage( flipImage );
            newWriter->setBitDepth( 8 );
            newWriter->setLoopCount( loopCount );
            newWriter->setDelay( delay );
        }
#ifdef gif_h
        else
        {
            oldWriter = std::make_unique< GifWriter >();
            oldWriter->firstFrame = false;
            oldWriter->f = nullptr;
            oldWriter->oldImage = nullptr;
            GifBegin( oldWriter.get(), qPrintable( fn ), fBIF->imageSize().width(), fBIF->imageSize().height(), delay(), 8, dither(), loopCount() );
        }
#endif

        bool wasCancelled = false;
        int frame = 0;
        bool aOK = true;
        for ( auto &&image : images )
        {
            setCurr( frame++ );
            wasCancelled = wasCancelledFunc();

            if ( image.isNull() )
            {
                if ( QMessageBox::warning( parent, QObject::tr( "BIF File has empty image" ), QObject::tr( "Image #%1 is null, skipped.  Continue?" ).arg( frame ), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
                {
                    aOK = false;
                    break;
                }
            }

            if ( wasCancelledFunc() )
            {
                wasCancelled = true;
                break;
            }

            if ( newWriter )
            {
                if ( !newWriter->writeImage( image, false ) )
                {
                    if ( QMessageBox::warning( parent, QObject::tr( "Problem writing frame" ), QObject::tr( "Image #%1 was not written.  Continue?" ).arg( frame ), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
                    {
                        aOK = false;
                        break;
                    }
                }
            }
#ifdef gif_h
            else
            {
                auto imageData = NQtUtils::imageToPixels( image );
                GifWriteFrame( oldWriter.get(), imageData, image.width(), image.height(), delay(), flipImage(), 8, dither() );
                delete[] imageData;
            }
#endif
            if ( wasCancelledFunc() )
            {
                wasCancelled = true;
                break;
            }
        }

        if ( aOK && !wasCancelled )
        {
            if ( newWriter )
                newWriter->writeEnd();
#ifdef gif_h
            else
                GifEnd( oldWriter.get() );
#endif
        }
        else
        {
            QFile::remove( fileName );
        }
        return !wasCancelled && aOK;
    }
}
