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


#ifndef GIFIOHANDLER_H
#define GIFIOHANDLER_H

#include <QImageIOHandler>

class CGIFIOHandler : public QImageIOHandler
{
public:
    CGIFIOHandler();
    ~CGIFIOHandler();

    static bool canWrite( QIODevice * device );

    virtual bool canRead() const override;
    virtual bool read( QImage * image ) override;
    virtual bool write( const QImage & image ) override;

    virtual QVariant option( ImageOption option ) const override;
    virtual void setOption( ImageOption option, const QVariant & value ) override;
    virtual bool supportsOption( ImageOption option ) const override;

    virtual bool jumpToNextImage() override;
    virtual bool jumpToImage( int imageNumber ) override;
    virtual int loopCount() const override;
    virtual int imageCount() const override;
    virtual int nextImageDelay() const override; // returns ms
    virtual int currentImageNumber() const override;
    virtual QRect currentImageRect() const override;
};


#endif // CBIFIOHandler_P_H
