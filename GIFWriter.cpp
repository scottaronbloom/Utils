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

#include "GIFWriter.h"
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QDebug>
namespace NUtils
{
    int CGIFWriter::kTransparentIndex{ 0 };

    CGIFWriter::CGIFWriter()
    {
    }
    
    CGIFWriter::CGIFWriter( const QString & filename ) :
        CGIFWriter( new QFile( filename ) )
    {
        fDeleteDevice = true;
    }

    CGIFWriter::CGIFWriter( QIODevice * device ) :
        fDevice( device ),
        fDataStream( device )
    {
    }
    
    CGIFWriter::~CGIFWriter()
    {
        setDevice( nullptr );
        if ( fPrevFrameData )
            delete [] fPrevFrameData;
    }
    
    void CGIFWriter::setFileName( const QString & fileName )
    {
        setDevice( new QFile( fileName ) );
        fDeleteDevice = true;
    }
    
    void CGIFWriter::setDevice( QIODevice * device )
    {
        if ( fDeleteDevice )
            delete fDevice;
        fDevice = device;
        fDataStream.setDevice( fDevice );

        fDeleteDevice = false;
    }

    bool CGIFWriter::status() const
    {
        return status( fDataStream );
    }

    bool CGIFWriter::status( const QDataStream & ds )
    {
        return ds.status() == QDataStream::Ok;
    }

    bool CGIFWriter::writeChar( uint8_t ch )
    {
        return writeChar( ch, fDataStream );
    }

    bool CGIFWriter::writeChar( uint8_t ch, QDataStream & ds )
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

    bool CGIFWriter::writeInt( uint16_t val, QDataStream & ds )
    {
        if ( !status( ds ) )
            return false;
        ds << (uint8_t)(val & 0xff);
        ds << (uint8_t)((val >> 8) & 0xff);
        return status( ds );
    }

    bool CGIFWriter::writeString( const char * str )
    {
        return writeString( str, fDataStream );
    }

    bool CGIFWriter::writeString( const char * str, QDataStream & ds )
    {
        uint len = qstrlen( str );
        return writeRaw( str, len, ds );
    }

    bool CGIFWriter::writeRaw( const char * str, int len )
    {
        return writeRaw( str, len, fDataStream );
    }

    bool CGIFWriter::writeRaw( const char * str, int len, QDataStream & ds )
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

        if ( !fDevice )
            return false;

        if ( !fDevice->isOpen() || !fDevice->isWritable() )
            return false;

        fFirstFrame = true;
        fPrevFrameData = new uint8_t[numPixels() * (uint8_t)4];

        writeString( "GIF89a" );
        writeInt( fCurrImage.width() );
        writeInt( fCurrImage.height() );

        writeChar( 0xf0 ); // global color table of 2 entries
        writeChar( 0x0 ); // background color
        writeChar( 0x0 ); // pixes are squares, its in the format....

        // global palette
        // color 0 is black
        writeChar( 0x0 );
        writeChar( 0x0 );
        writeChar( 0x0 );

        // color 1 is black
        writeChar( 0x0 );
        writeChar( 0x0 );
        writeChar( 0x0 );

        if ( fIsMultiFrame )
        {
            writeChar( 0x21 ); // extension block
            writeChar( 0xff ); // its an app specific extension
            writeChar( 11 ); // length 11
            writeString( "NETSCAPE2.0" );
            writeChar( 3 ); // 3 bytes of NETSCAPE2.0 data
            writeChar( 1 ); // interwebs say so
            writeInt( fNumberOfLoops );
            writeChar( 0 ); // end of extension block
        }
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

    bool CGIFWriter::writeImage( const QImage & frame )
    {
        return writeImage( frame, {} );
    }

    bool CGIFWriter::writeImage( const QImage & image, std::optional< uint32_t > delay )
    {
        if ( delay.has_value() )
            fDelay = delay;
        if ( fDelay.has_value() )
        {
            if ( !fIsMultiFrame )
                return false;
        }
        fCurrImage = image;
        writeHeader();
        writeCurrImage();
        writeEnd();

        return status();
    }

    uint8_t * CGIFWriter::imageToPixels( const QImage & image ) // allocates the space, user is responsible for memory deletion using array delete
    {
        Q_ASSERT( sizeof( uchar ) == sizeof( uint8_t ) );

        auto imageSize = static_cast<size_t>(image.width() * image.height() * 4 * sizeof( uint8_t ));
        auto retVal = new uint8_t[imageSize];
        std::memcpy( retVal, image.bits(), imageSize );
        return retVal;
    }

    bool CGIFWriter::pixelCompare( const uint8_t * lhs, const uint8_t * rhs, int pixelNum )
    {
        return (lhs[pixelNum] == rhs[pixelNum])
            && (lhs[pixelNum + 1] == rhs[pixelNum + 1])
            && (lhs[pixelNum + 2] == rhs[pixelNum + 2]);
    }

    bool CGIFWriter::pixelCompare( const uint8_t * lhs, const uint8_t * rhs )
    {
        return pixelCompare( lhs, rhs, 0 );
    }

    bool CGIFWriter::pixelCompare( const uint8_t * lhs, const std::initializer_list< uint32_t > & rhs )
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

    void CGIFWriter::ditherImage( const uint8_t * prevImage )
    {
        Q_ASSERT( fPalette );
        if ( !fPalette )
            return;

        auto numPixels = this->numPixels();
       
        auto quantPixels = new int32_t[sizeof( int32_t ) * numPixels * 4 ];
        auto imagePixels = CGIFWriter::imageToPixels( fCurrImage );

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            auto pix = imagePixels[ii];
            auto pix256 = static_cast<uint32_t>(pix) * 256;
            quantPixels[ii] = pix256;
        }

        for( int currRow = 0; currRow < fCurrImage.height(); ++currRow )
        {
            for ( int currCol = 0; currCol < fCurrImage.width(); ++currCol )
            {
                auto pixelNumber = (currRow * fCurrImage.width()) + currCol;
                auto byteNumber = 4 * pixelNumber;
                auto nextPixel = quantPixels + byteNumber;
                const auto lastPix = prevImage ? ( prevImage + byteNumber ) : nullptr;

                uint32_t rr = (nextPixel[0] + 127) / 256;
                uint32_t gg = (nextPixel[1] + 127) / 256;
                uint32_t bb = (nextPixel[2] + 127) / 256;

                if ( prevImage && 
                     (pixelCompare( prevImage, { rr, gg, bb } ) ) )
                {
                    fPrevFrameData[0] = rr;
                    fPrevFrameData[1] = gg;
                    fPrevFrameData[2] = bb;
                    fPrevFrameData[3] = kTransparentIndex;
                    continue;
                }

                uint32_t bestIndex = kTransparentIndex;
                uint32_t bestDifference = 1000000;
                fPalette->closestColor( rr, gg, bb, 1, bestIndex, bestDifference );

                int32_t rErr = nextPixel[0] - (int32_t)fPalette->fRed[bestIndex] * 256;
                int32_t gErr = nextPixel[0] - (int32_t)fPalette->fGreen[bestIndex] * 256;
                int32_t bErr = nextPixel[0] - (int32_t)fPalette->fBlue[bestIndex] * 256;

                nextPixel[0] = fPalette->fRed[bestIndex];
                nextPixel[1] = fPalette->fGreen[bestIndex];
                nextPixel[2] = fPalette->fBlue[bestIndex];
                nextPixel[3] = bestIndex;

                // propagate the error to the adjacent locations
                auto quantLoc7 = pixelNumber + 1;  // to the right
                auto quantLoc3 = pixelNumber + fCurrImage.width() - 1; // next rowleft
                auto quantLoc5 = pixelNumber + fCurrImage.width();     // next row
                auto quantLoc1 = pixelNumber + fCurrImage.width() + 1; // next row righ

                updateQuant( quantPixels, quantLoc7, rErr, gErr, bErr );
                updateQuant( quantPixels, quantLoc3, rErr, gErr, bErr );
                updateQuant( quantPixels, quantLoc5, rErr, gErr, bErr );
                updateQuant( quantPixels, quantLoc1, rErr, gErr, bErr );
            }

        }

        for ( int ii = 0; ii < numPixels * 4; ++ii )
        {
            fPrevFrameData[ii] = static_cast<uint8_t>(quantPixels[ii]);
        }
        delete[] imagePixels;
        delete[] quantPixels;
    }

    void CGIFWriter::updateQuant( int32_t * quantPixels, int loc, int32_t rErr, int32_t gErr, int32_t bErr )
    {
        if ( loc < numPixels() )
        {
            auto pixel = quantPixels + loc;
            pixel[0] += std::max( -pixel[0], rErr * 7 / 16 );
            pixel[1] += std::max( -pixel[1], gErr * 7 / 16 );
            pixel[2] += std::max( -pixel[2], bErr * 7 / 16 );
        }
    }

    void CGIFWriter::thresholdImage( const uint8_t * prevImage )
    {
        auto numPixels = this->numPixels();

        auto imagePixels = CGIFWriter::imageToPixels( fCurrImage );

        auto imageLoc = imagePixels;
        auto lastLoc = prevImage;
        auto outLoc = fPrevFrameData;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            if ( lastLoc && pixelCompare( imageLoc, lastLoc ) )
            {
                outLoc[0] = imageLoc[0];
                outLoc[1] = imageLoc[1];
                outLoc[2] = imageLoc[2];
                outLoc[3] = kTransparentIndex;
            }
            else
            {
                uint32_t bestIndex = kTransparentIndex;
                uint32_t bestDifference = 1000000;
                fPalette->closestColor( imageLoc[0], imageLoc[1], imageLoc[2], 1, bestIndex, bestDifference );
                outLoc[0] = fPalette->fRed[bestIndex];
                outLoc[1] = fPalette->fGreen[bestIndex];
                outLoc[2] = fPalette->fBlue[bestIndex];
                outLoc[3] = bestIndex;
            }

            if ( lastLoc )
                lastLoc += 4;
            outLoc += 4;
            imageLoc += 4;
        }
    }

    struct SLZWNode
    {
        uint16_t fNext[256];
    };

    struct SBitStatus
    {
        SBitStatus( QDataStream & ds ) :
            fDataStream( ds )
        {}

        void write( uint32_t bit )
        {
            bit = bit & 1;
            bit = bit << fBitIndex;
            fByte |= bit;

            ++fBitIndex;
            if ( fBitIndex > 7 )
            {
                fChunk[fChunkIndex++] = fByte;
                fBitIndex = 0;
                fByte = 0;
            }
        }

        void write()
        {
            CGIFWriter::writeChar( fChunkIndex, fDataStream );
            CGIFWriter::writeRaw( (const char *)fChunk, fChunkIndex, fDataStream );

            fBitIndex = 0;
            fByte = 0;
            fChunkIndex = 0;
        }

        bool write( uint32_t code, uint32_t length )
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
            return CGIFWriter::status( fDataStream );
        }



        QDataStream & fDataStream;
        uint8_t fBitIndex{ 0 };
        uint8_t fByte{ 0 };
        uint32_t fChunkIndex{ 0 };
        uint8_t fChunk[256] = { 0 };
    };

    bool CGIFWriter::writeLZW( uint32_t left, uint32_t top ) 
    {
        if ( !status() )
            return false;

        writeChar( 0x21 );
        writeChar( 0xf9 );
        writeChar( 0x04 );
        writeChar( 0x05 );
        if ( hasDelay() )
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
        uint32_t maxCode = clearCode;

        SBitStatus bitStatus( fDataStream );
        bitStatus.write( clearCode, codeSize );

        auto imagePixels = imageToPixels( fCurrImage );
        auto height = fCurrImage.height();
        auto width = fCurrImage.width();
        for ( int currRow = 0; currRow < height; ++currRow )
        {
            for ( int currCol = 0; currCol < width; ++currCol )
            {
                auto pixelNumber = (currRow * width) + currCol;
                auto byteNumber = 4 * pixelNumber;

                auto nextValue = fFlipImage ? imagePixels[((height - 1 - currRow) * width + currCol) * 4 + 3] : imagePixels[byteNumber + 3];
                if ( currCode < 0 )
                {
                    currCode = nextValue;
                }
                else if ( codeTree[ currCode ].fNext[ nextValue ] )
                {
                    currCode = codeTree[currCode].fNext[nextValue];
                }
                else
                {
                    bitStatus.write( currCode, codeSize );
                    codeTree[currCode].fNext[nextValue] = ++maxCode;

                    if ( maxCode >= (1ul << codeSize) )
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
        return status();
    }

    bool CGIFWriter::writeCurrImage()
    {
        if ( !status() )
            return false;
        
        auto prevImage = fFirstFrame ? nullptr : fPrevFrameData;
        fFirstFrame = false;

        fPalette = std::make_unique< SGIFPalette >( prevImage, fCurrImage, fBitDepth, dither() );

        if ( dither() )
            ditherImage( prevImage );
        else
            thresholdImage( prevImage );

        return writeLZW( 0, 0 );
    }

    SGIFPalette::SGIFPalette( const uint8_t * prevImage, const QImage & image, uint8_t bitDepth, bool dither ) :
        fBitDepth( bitDepth ),
        fDither( dither )
    {
        fImageWidth = image.width();
        fTmpImage = CGIFWriter::imageToPixels( image );
        int numPixels = image.width() * image.height();

        getChangedPixels( prevImage, fTmpImage, numPixels );
        const auto lastELT = 1 << bitDepth;
        const auto splitELT = lastELT / 2;
        const auto splitDist = splitELT / 2;

        splitPalette( fTmpImage, numPixels, 1, lastELT, splitELT, splitDist, 1 );
        uint32_t pos = (uint32_t)1 << (bitDepth - 1);
        fTreeSplit[pos] = 0;
        fTreeSplitELT[pos] = 0;
        setRed( 0, 0 );
        setGreen( 0, 0 );
        setBlue( 0, 0 );
    }

    SGIFPalette::~SGIFPalette()
    {
        if ( fTmpImage )
            delete[] fTmpImage;
    }

    void SGIFPalette::splitPalette( uint8_t * image, int numPixels, int firstELT, int lastELT, int splitELT, int splitDIST, int treeNodeNum )
    {
        static int hitCount = 0;

        //qDebug().noquote().nospace() << "HitCount: " << hitCount++
        //    << " GifSplitPalette: "
        //    << " NumPixels: " << numPixels
        //    << " firstElt: " << firstELT
        //    << " lastElt: " << lastELT
        //    << " splitElt: " << splitELT
        //    << " splitDist: " << splitDIST
        //    << " treeNode: " << treeNodeNum
        //    << " buildForDither: " << fDither
        //    ;

        //dumpArray( image, numPixels * 4 );
        if ( hitCount == 7 )
            int xyz = 0;
        //dump();

        if ( (lastELT <= firstELT) || (numPixels == 0) )
            return;
        if ( lastELT == (firstELT + 1) )
        {
            if ( fDither )
            {
                if ( firstELT == 1 )
                {
                    setRGBToMinMax( image, numPixels, firstELT, true );
                    return;
                }

                if ( firstELT == (1 << fBitDepth) - 1 )
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

        fTreeSplitELT[treeNodeNum] = splitOffset;
        fTreeSplit[treeNodeNum] = image[subPixelsA * 4 + splitOffset];

        splitPalette( image, subPixelsA, firstELT, splitELT, splitELT - splitDIST, splitDIST / 2, treeNodeNum * 2 );
        splitPalette( image + subPixelsA * 4, subPixelsB, splitELT, lastELT, splitELT + splitDIST, splitDIST / 2, (treeNodeNum * 2) + 1 );
    }
    
    std::tuple< uint8_t, int, int > SGIFPalette::compuiteRGBRanges( int numPixels, const uint8_t * image, int splitELT, int firstELT, int lastELT )
    {
        uint8_t minR = 255;
        uint8_t minG = 255;
        uint8_t minB = 255;

        uint8_t maxR = 0;
        uint8_t maxG = 0;
        uint8_t maxB = 0;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            auto r = image[ii * 4 + 0];
            auto g = image[ii * 4 + 1];
            auto b = image[ii * 4 + 2];

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
        if ( (rRange > bRange) && (rRange > gRange) )
            splitOffset = 0;

        auto subPixelsA = numPixels * (splitELT - firstELT) / (lastELT - firstELT);
        auto subPixelsB = numPixels - subPixelsA;
        return std::make_tuple( splitOffset, subPixelsA, subPixelsB );
    }

    void SGIFPalette::swap( uint8_t * image, int lhs, int rhs )
    {
        //static int hitCount = 0;
        //qDebug().noquote().nospace() << "HitCount: " << hitCount++
        //    << " swap: "
        //    << " pixA: " << lhs
        //    << " pixB: " << rhs
        //    ;

        //qDebug().noquote().nospace() << "BEFORE:";
        //qDebug().noquote().nospace() << "ImageA: " << dumpArray( fTmpImage + lhs * 4, 4 );
        //qDebug().noquote().nospace() << "ImageB: " << dumpArray( fTmpImage + rhs * 4, 4 );

        for( int ii = 0; ii <= 3; ++ii )
            std::swap( image[(lhs * 4)+ii], image[(rhs * 4) + ii] );

        //qDebug().noquote().nospace() << "AFTER:";
        //qDebug().noquote().nospace() << "ImageA: " << dumpArray( fTmpImage + lhs * 4, 4 );
        //qDebug().noquote().nospace() << "ImageB: " << dumpArray( fTmpImage + rhs * 4, 4 );
    }

    int SGIFPalette::partition( uint8_t * image, int left, int right, const int elt, int pivot )
    {
        static int hitCount = 0;
        //qDebug().noquote().nospace() << "HitCount: " << hitCount++
        //    << " partition: "
        //    << " left: " << left
        //    << " right: " << right
        //    << " elt: " << elt
        //    << " pivot: " << pivot
        //    ;
        //dump();
        //qDebug().noquote().nospace() << "Before Image: " << dumpArray( image + left, (right - left + 1)*4, ((right - left)*4)/ 5 );
        if ( hitCount == 90 )
            int xyz = 0;

        //qDebug().noquote().nospace() << "Around pivot point: " << dumpArray( image + std::max( 0, (pivot*4) - 10 ) + left, 20, 20 );
        auto pivotValue = image[(pivot * 4) + elt];
        swap( image, pivot, right - 1 );
        auto storedIndex = left;
        bool split = false;
        for ( int ii = left; ii < right - 1; ++ii )
        {
            auto val = image[(ii * 4) + elt];
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
        //qDebug().noquote().nospace() << "After Image: " << dumpArray( image + left, (right - left + 1)*4, ((right - left) *4 )/ 5 );
        return storedIndex;
    }

    void SGIFPalette::partitionByMedian( uint8_t * image, int left, int right, int com, int neededCenter )
    {
        static int hitCount = 0;
        //qDebug().noquote().nospace() << "HitCount: " << hitCount++
        //    << " GifPartitionByMedian: "
        //    << " left: " << left
        //    << " right: " << right
        //    << " com: " << com
        //    << " neededCenter: " << neededCenter
        //    ;
        if ( hitCount == 94 )
            int xyz = 0;
        //dumpImage( image + left, (right - left + 1) * 4 );
        //dump();

        if ( left < right-1 )
        {
            auto pivot = left + (right - left) / 2;
            pivot = partition( image, left, right, com, pivot );
            if ( pivot > neededCenter )
                partitionByMedian( image, left, pivot, com, neededCenter );

            if (pivot < neededCenter )
                partitionByMedian( image, pivot + 1, right, com, neededCenter );
        }
    }


    void SGIFPalette::setRGBToAverage( const uint8_t * image, int numPixels, int location )
    {
        //qDebug().noquote().nospace() << "Before Set to Average:";
        //dump();
        //qDebug().noquote().nospace() << dumpArray( image, numPixels*4, fImageWidth );

        uint64_t r = 0;
        uint64_t g = 0;
        uint64_t b = 0;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            r += image[ii * 4 + 0];
            g += image[ii * 4 + 1];
            b += image[ii * 4 + 2];
        }

        r += ((uint64_t)numPixels) / 2;
        g += ((uint64_t)numPixels) / 2;
        b += ((uint64_t)numPixels) / 2;

        r /= (uint64_t)numPixels;
        g /= (uint64_t)numPixels;
        b /= (uint64_t)numPixels;

        setRed( location, r );
        setGreen( location, g );
        setBlue( location, b );

        //qDebug().noquote().nospace() << "After Set to Average:";
        //dump();
        //qDebug().noquote().nospace() << dumpArray( image, numPixels * 4, fImageWidth );
    }

    void SGIFPalette::closestColor( int32_t rr, int32_t gg, int32_t bb, int treeNodeNumber, uint32_t & bestIndex, uint32_t & bestDifference ) const
    {
        if ( treeNodeNumber > (1 << fBitDepth) - 1 )
        {
            int index = treeNodeNumber - (1 << fBitDepth);
            if ( index != CGIFWriter::kTransparentIndex )
            {
                auto rError = rr - (int32_t)fRed[index];
                auto gError = gg - (int32_t)fGreen[index];
                auto bError = bb - (int32_t)fBlue[index];

                auto diff = static_cast<uint32_t>(std::abs( rError ) + std::abs( gError ) + std::abs( bError ));
                if ( diff < bestDifference )
                {
                    bestIndex = index;
                    bestDifference = diff;
                }
            }
        }
        else
        {
            int32_t comps[3] = { rr, gg, bb };
            uint32_t splitCompare = comps[fTreeSplitELT[treeNodeNumber]];
            auto splitPos = fTreeSplit[treeNodeNumber];
            if ( splitPos > splitCompare )
            {
                closestColor( rr, gg, bb, treeNodeNumber * 2, bestIndex, bestDifference );
                if ( bestDifference > (splitPos - splitCompare) )
                {
                    closestColor( rr, gg, bb, (treeNodeNumber * 2) + 1, bestIndex, bestDifference );
                }
            }
            else
            {
                closestColor( rr, gg, bb, (treeNodeNumber * 2) + 1, bestIndex, bestDifference );
                if ( bestDifference > (splitPos - splitCompare) )
                {
                    closestColor( rr, gg, bb, treeNodeNumber * 2, bestIndex, bestDifference );
                }
            }
        }
    }

    void SGIFPalette::setRGBToMinMax( const uint8_t * image, int numPixels, int location, bool min )
    {
        //qDebug().noquote().nospace() << "Before Set to " << (min ? "Min" : "Max") << ": ";
        //dump();
        //qDebug().noquote().nospace() << dumpArray( image, numPixels*4, fImageWidth );
        uint8_t r = min ? 255 : 0;
        uint8_t g = min ? 255 : 0;
        uint8_t b = min ? 255 : 0;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            auto currR = image[ii * 4 + 0];
            auto currG = image[ii * 4 + 1];
            auto currB = image[ii * 4 + 2];
            r = min ? std::min( r, currR ) : std::max( r, currR );
            g = min ? std::min( g, currG ) : std::max( g, currG );
            b = min ? std::min( b, currB ) : std::max( b, currB );
        }

        setRed(location, r );
        setGreen( location, g );
        setBlue( location, b );

        //qDebug().noquote().nospace() << "After: ";
        //dump();
        //qDebug().noquote().nospace() << dumpArray( image, numPixels*4, fImageWidth );
    }

    void SGIFPalette::getChangedPixels( const uint8_t * prevImage, uint8_t * currImage, int & numPixels )
    {
        if ( !prevImage )
            return;

        int retVal = 0;
        auto writeImagePos = currImage;

        for ( int ii = 0; ii < numPixels; ++ii )
        {
            if ( !CGIFWriter::pixelCompare( prevImage, currImage, ii ) )
            {
                writeImagePos[0] = currImage[ii];
                writeImagePos[1] = currImage[ii + 1];
                writeImagePos[2] = currImage[ii + 2];

                writeImagePos += 4;
                ++retVal;
            }
        }
        numPixels = retVal;
    }

    bool SGIFPalette::write( QDataStream & ds )
    {
        CGIFWriter::writeChar( 0x80 + fBitDepth - 1, ds );

        CGIFWriter::writeChar( 0, ds );
        CGIFWriter::writeChar( 0, ds );
        CGIFWriter::writeChar( 0, ds );

        for ( int ii = 1; CGIFWriter::status( ds ) && ( ii < (1 << fBitDepth) ); ++ii )
        {
            auto rr = fRed[ii];
            auto gg = fGreen[ii];
            auto bb = fBlue[ii];

            CGIFWriter::writeChar( rr, ds );
            CGIFWriter::writeChar( gg, ds );
            CGIFWriter::writeChar( bb, ds );

        }
        return CGIFWriter::status( ds );
    }


    void SGIFPalette::setRed( int location, uint8_t val )
    {
        static int hitCount = 0;
        if ( hitCount == 3 )
            int xyz = 0;

        //qDebug().noquote().nospace() << "HitCount: " << hitCount << " RED: location: " << location << " value: " << val;
        fRed[location] = val;
        hitCount++;
    }

    void SGIFPalette::setBlue( int location, uint8_t val )
    {
        //qDebug().noquote().nospace() << "BLUE: location: " << location << " value: " << val;
        fBlue[location] = val;
    }

    void SGIFPalette::setGreen( int location, uint8_t val )
    {
        //qDebug().noquote().nospace() << "GREEN: location: " << location << " value: " << val;
        fGreen[location] = val;
    }
    
    void SGIFPalette::dumpIt()
    {
        qDebug().noquote().nospace() << dumpText();
    }

    QString SGIFPalette::dumpText() const
    {
        QString retVal;
        retVal = "Red:\n" + dumpArray( fRed, fRed, 256 ) + "\n"
            + "Blue:\n" + dumpArray( fBlue, fBlue, 256 ) + "\n"
            + "Green:\n" + dumpArray( fGreen, fGreen, 256 ) + "\n"
            ;
        return retVal;
    }

    void SGIFPalette::dumpImage( const uint8_t * arr, int size ) const
    {
        qDebug().nospace().noquote() << dumpArray( arr, size, fImageWidth );
    }

    QString SGIFPalette::dumpArray( const uint8_t * arr, int size, int rowCount /*= 20 */ ) const
    {
        return dumpArray( arr, fTmpImage, size, rowCount );
    }

    QString SGIFPalette::dumpArray( const uint8_t * arr, const uint8_t * baseArray, int size, int rowCount/*=20 */ ) const
    {
        static int hitCount = 0;
        auto retVal = QString( "HitCount: %1 - Array: 0x%2\n" ).arg( hitCount++ ).arg( arr - baseArray, 8, 16, QChar( '0' ) );

        int colCount = 0;
        for ( int ii = 0; ii < size; ++ii )
        {
            if ( colCount == 0 )
                retVal += QString( "0x%1-0x%2: " ).arg( ii, 3, 16, QChar( '0' ) ).arg( ii + 20, 3, 16, QChar( '0' ) ).toUpper();
            else
                retVal += " ";

            auto curr = QString( "%1" ).arg( arr[ii], 2, 16, QChar( '0' ) ).toUpper();
            retVal += curr;

            if ( colCount == rowCount - 1 )
            {
                retVal += "\n";
                colCount = -1;
            }
            colCount++;
        }
        return retVal;
    }
}

