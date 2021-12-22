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


#include "GIFIOHandler.h"
#include "../GIFWriter.h"
#include <QImage>


CGIFIOHandler::CGIFIOHandler()
{
}

CGIFIOHandler::~CGIFIOHandler()
{
}

bool CGIFIOHandler::canRead() const
{
    return false;
}

bool CGIFIOHandler::canWrite(QIODevice * device)
{
    if ( !device )
    {
        qWarning( "CBIFIOHandler::canWrite() called with no device" );
        return false;
    }

    if ( device->isWritable() )
    {
        qWarning( "CBIFIOHandler::canWrite() called on a non-writable device" );
        return false;
    }

    return true;
}

bool CGIFIOHandler::read( QImage * /*image*/ )
{
    return false;
}

bool CGIFIOHandler::jumpToImage(int imageNumber)
{
    return QImageIOHandler::jumpToImage( imageNumber );
}

bool CGIFIOHandler::jumpToNextImage()
{
    return QImageIOHandler::jumpToNextImage();
}

QRect CGIFIOHandler::currentImageRect() const
{
    return QImageIOHandler::currentImageRect();
}

bool CGIFIOHandler::write(const QImage & image)
{
    NUtils::CGIFWriter writer( device() );
    writer.setDither( true );
    writer.setFlipImage( false );
    writer.setBitDepth( 8 );
    writer.setDelay( 0 );
    return writer.writeImage( image );
}

bool CGIFIOHandler::supportsOption(ImageOption option) const
{
    return option == Size;
}

QVariant CGIFIOHandler::option(ImageOption option) const
{
    return QImageIOHandler::option( option );
    //switch(option)
    //{
    //case Size:
    //    return QSize();
    //    //fGIFFile->readHeader(device());
    //    //return fGIFFile->imageSize();
    //case Animation:
    //    return true;
    //default:
    //    break;
    //}

    //return QVariant();
}

void CGIFIOHandler::setOption(ImageOption option, const QVariant &value)
{
    return QImageIOHandler::setOption( option, value );
    //(void)option;
    //(void)value;
}

int CGIFIOHandler::nextImageDelay() const
{
    return 50;
    //return fGIFFile->imageDelay();  
}

int CGIFIOHandler::imageCount() const
{ 
    return QImageIOHandler::imageCount();

    //if (!fGIFFile || !fGIFFile->isValid())
    //    return 0;

    //fGIFFile->readHeader(device());
    //return static_cast< int >( fGIFFile->imageCount() );
}

int CGIFIOHandler::loopCount() const
{
    return QImageIOHandler::loopCount();

    //if (!fGIFFile || !fGIFFile->isValid())
    //    return 0;

    //fGIFFile->readHeader(device());
    //return static_cast< int >( fGIFFile->imageCount() );
}


int CGIFIOHandler::currentImageNumber() const
{
    return QImageIOHandler::imageCount();
}
