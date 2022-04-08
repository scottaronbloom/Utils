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

#ifndef __IMAGESCROLLBAR_H
#define __IMAGESCROLLBAR_H

#include "SABUtilsExport.h"

#include <QScrollBar>
#include <QImage>
namespace NSABUtils
{
#ifdef BIF_SCROLLBAR_SUPPORT
    namespace NBIF
    {
        class CFile;
        class CModel;
    }
#endif
    class SABUTILS_EXPORT CImageScrollBar : public QScrollBar
    {
        Q_OBJECT;
    public:
        CImageScrollBar( QWidget * parent = nullptr );
        CImageScrollBar( Qt::Orientation orientation, QWidget * parent = nullptr );

#ifdef BIF_SCROLLBAR_SUPPORT
        void setBIFFile( std::shared_ptr< NBIF::CFile > bifFile );
        void setBIFModel( std::shared_ptr< NBIF::CModel > bifModel );
#endif
        void setImages( const std::list< QImage > & images );
        void setImages( const std::vector< QImage > & images );

        void setMessageFormat( const QString & format ) { fMsgFormat = format; }
        QString messageFormat() const { return fMsgFormat; }

        void setUpdateOnScrollOnly( bool updateOnScrollOnly ) { fUpdateOnScrollOnly = updateOnScrollOnly; }
        bool hasImages() const;
    private Q_SLOTS:
        void slotValueChanged( int value );
        void slotSliderMoved( int value );
        void slotSliderPressed();
        void slotSliderReleased();
    private:
        virtual bool event( QEvent * event ) override;

        QString message() const;
        int numImages() const;

        void updateImage();

        QImage getImage( int imageNum ) const;

        void updateImageFromPos();
        void updateValue( int value );

        void setCurrentImageNum( int num );
        void init();
        int pixelPosToRangeValue( int pos ) const;

        int fCurrentImageNum{ -1 };
        bool fShowImage{ false };
        QImage fCurrentImage;
        QString fMsgFormat{ "Image #%v of %m" };
        bool fUpdateOnScrollOnly{ true };

#ifdef BIF_SCROLLBAR_SUPPORT
        std::shared_ptr< NBIF::CFile > fBIFFile;
        std::shared_ptr< NBIF::CModel > fBIFModel;
#endif
        std::vector< QImage > fImages;
    };
}
#endif
