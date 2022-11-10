// The MIT License( MIT )
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


#include "BIFIOHandler.h"
#include "SABUtils/BIFFile.h"

#include <QImage>

namespace NSABUtils
{
    namespace NBIF
    {
        CIOHandler::CIOHandler()
            : fBIFFile( new NBIF::CFile )
        {
        }

        CIOHandler::~CIOHandler()
        {
            delete fBIFFile;
        }

        bool CIOHandler::canRead() const
        {
            if ( !canRead( device() ) )
                return false;

            if ( ( fCurrentFrame >= 0 ) && ( fCurrentFrame >= fBIFFile->imageCount() ) )
                return false;

            if ( fBIFFile->state() != NBIF::CFile::EState::eError )
            {
                setFormat( "bif" );
                return true;
            }
            return false;
        }

        bool CIOHandler::canRead( QIODevice * device )
        {
            if ( !device )
            {
                qWarning( "CBIFIOHandler::canRead() called with no device" );
                return false;
            }

            if ( !device->seek( 0 ) )
                return false;

            QByteArray magicNumber = device->peek( 8 );
            bool isBIF = NBIF::CFile::validateMagicNumber( magicNumber );
            return isBIF;
        }

        bool CIOHandler::read( QImage * image )
        {
            if ( !fBIFFile )
                return false;

            if ( !canRead() )
                return false;

            // read the next image;
            auto retVal = fBIFFile->read( device(), ( fCurrentFrame < 0 ) ? 0 : fCurrentFrame );
            if ( retVal.first )
            {
                fCurrentFrame++;
                *image = retVal.second;
            }
            return retVal.first;
        }

        bool CIOHandler::jumpToImage( int imageNumber )
        {
            if ( !fBIFFile )
                return false;
            if ( imageNumber < 0 )
                return false;
            if ( imageNumber >= fBIFFile->imageCount() )
                return false;

            fCurrentFrame = imageNumber;
            return true;
        }

        bool CIOHandler::jumpToNextImage()
        {
            return QImageIOHandler::jumpToNextImage();
        }

        QRect CIOHandler::currentImageRect() const
        {
            return QImageIOHandler::currentImageRect();
        }

        bool CIOHandler::write( const QImage & image )
        {
            (void)image;
            return false;
        }

        bool CIOHandler::supportsOption( ImageOption option ) const
        {
            return
                option == Size
                || option == Animation;
            ;
        }

        QVariant CIOHandler::option( ImageOption option ) const
        {
            switch ( option )
            {
                case Size:
                    fBIFFile->readHeader( device() );
                    return fBIFFile->imageSize();
                case Animation:
                    return true;
                default:
                    break;
            }

            return QVariant();
        }

        void CIOHandler::setOption( ImageOption option, const QVariant & value )
        {
            (void)option;
            (void)value;
        }

        int CIOHandler::nextImageDelay() const
        {
            if ( !fBIFFile )
                return 0;
            return fBIFFile->imageDelay();
        }

        int CIOHandler::imageCount() const
        {
            if ( !fBIFFile || !fBIFFile->isValid() )
                return 0;

            fBIFFile->readHeader( device() );
            return static_cast<int>( fBIFFile->imageCount() );
        }

        void CIOHandler::setLoopCount( int loopCount )
        {
            if ( fBIFFile )
                fBIFFile->setLoopCount( loopCount );
        }

        int CIOHandler::loopCount() const
        {
            return fBIFFile ? fBIFFile->loopCount() : -1;
        }

        int CIOHandler::currentImageNumber() const
        {
            return fCurrentFrame;
        }
    }
}
