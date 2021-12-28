//
// Copyright( c ) 2021 Scott Aron Bloom
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

#include "ImageScrollBar.h"

#ifdef BIF_SCROLLBAR_SUPPORT
#include "BIFFile.h"
#include "BIFModel.h"
#endif

#include <QDebug>
#include <QToolTip>
#include <QBuffer>
#include <QByteArray>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOption>

namespace NSABUtils
{
    CImageScrollBar::CImageScrollBar( QWidget * parent /*= 0 */ ) :
        QScrollBar( parent )
    {
        init();
    }

    CImageScrollBar::CImageScrollBar( Qt::Orientation orientation, QWidget * parent /*= nullptr */ ) :
        QScrollBar( orientation, parent )
    {
        init();
    }

    void CImageScrollBar::init()
    {
        connect( this, &QScrollBar::valueChanged, this, &CImageScrollBar::slotValueChanged );
        connect( this, &QScrollBar::sliderMoved, this, &CImageScrollBar::slotSliderMoved );
        connect( this, &QScrollBar::sliderPressed, this, &CImageScrollBar::slotSliderPressed );
        connect( this, &QScrollBar::sliderReleased, this, &CImageScrollBar::slotSliderReleased );
    }

#ifdef BIF_SCROLLBAR_SUPPORT
    void CImageScrollBar::setBIFFile( std::shared_ptr< NBIF::CFile > bifFile )
    {
        fBIFFile = bifFile;
        slotValueChanged( 0 );
    }

    void CImageScrollBar::setBIFModel( std::shared_ptr< NBIF::CModel > bifModel )
    {
        fBIFModel = bifModel;
        slotValueChanged( 0 );
    }
#endif

    void CImageScrollBar::setImages( const std::vector< QImage > & images )
    {
        fImages = images;
    }

    void CImageScrollBar::setImages( const std::list< QImage > & images )
    {
        fImages = { images.begin(), images.end() };
    }

    bool CImageScrollBar::hasImages() const
    {
#ifdef BIF_SCROLLBAR_SUPPORT
        if ( fBIFFile )
            return true;
        else if ( fBIFModel )
            return true;
        else
#endif            
            return fImages.empty() == false;

    }

    void CImageScrollBar::slotValueChanged( int value )
    {
        updateValue( value );
    }

    void CImageScrollBar::slotSliderMoved( int value )
    {
        updateValue( value );
    }

    void CImageScrollBar::updateValue( int value )
    {
        if ( !hasImages() )
            return;

        setCurrentImageNum( -1 );
        int min = this->minimum();
        int max = this->maximum();

        if ( min == max )
            return;

        auto percentage = 1.0 * value / (max - min);

        auto numImagesLoaded = numImages();
        auto imageNum = static_cast<int>(percentage * numImagesLoaded);
        if ( imageNum >= numImagesLoaded )
            imageNum = numImagesLoaded;
        setCurrentImageNum( imageNum );
    }

    int CImageScrollBar::pixelPosToRangeValue( int pos ) const
    {
        QStyleOptionSlider opt;
        initStyleOption( &opt );

        auto rect = style()->subControlRect( QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, this );

        int total = 0;
        int spanMin = 0;
        int spanMax = 0;
        if ( orientation() == Qt::Horizontal )
        {
            spanMin = rect.x();
            spanMax = rect.x() + rect.width();
            total = rect.width();
        }
        else
        {
            spanMin = rect.y();
            spanMax = rect.y() + rect.height();
            total = rect.height();
        }


        auto min = minimum();
        auto max = maximum();
        if ( pos <= spanMin || total <= 0 ) // smaller than it should be
        {
            if ( opt.upsideDown )
                return max;
            else
                return min;
        }
        auto range = max - min;
        if ( pos >= spanMax )
        {
            if ( opt.upsideDown )
                return min;
            else
                return max;
        }

        auto percentage = 100.0 * pos / (spanMax - spanMin);
        auto tmp = static_cast<int>(percentage * (range) / 100.0);
        auto value = opt.upsideDown ? max - tmp : tmp + min;
        return value;
    }

    void CImageScrollBar::updateImageFromPos()
    {
        auto currPos = mapFromGlobal( QCursor::pos() );
        int value = 0;
        if ( orientation() == Qt::Horizontal )
            value = pixelPosToRangeValue( currPos.x() );
        else
            value = pixelPosToRangeValue( currPos.y() );

        updateValue( value );
    }


    void CImageScrollBar::slotSliderPressed()
    {
        fShowImage = hasImages();
        updateImage();
    }

    void CImageScrollBar::slotSliderReleased()
    {
        fShowImage = false;
        updateImage();
    }

    bool CImageScrollBar::event( QEvent * event )
    {
        if ( event->type() == QEvent::HoverEnter )
        {
            if ( fUpdateOnScrollOnly )
            {
                fShowImage = true;
                updateImageFromPos();
            }
        }
        else if ( event->type() == QEvent::HoverLeave )
        {
            if ( fUpdateOnScrollOnly )
            {
                fShowImage = true;
                updateImageFromPos();
            }
        }
        else if ( event->type() == QEvent::HoverMove )
        {
            if ( fUpdateOnScrollOnly )
            {
                updateImageFromPos();
            }
        }

        return QScrollBar::event( event );
    }

    void CImageScrollBar::setCurrentImageNum( int imageNum )
    {
        fCurrentImageNum = imageNum;
        updateImage();
    }

    void CImageScrollBar::updateImage()
    {
        if ( !fShowImage )
            QToolTip::hideText();
        else if ( hasImages() )
        {
            fCurrentImage = getImage( fCurrentImageNum );
            if ( !fCurrentImage.isNull() )
            {
                QByteArray data;
                QBuffer buffer( &data );
                fCurrentImage.save( &buffer, "PNG" );

                auto html = QString( "<img src='data:image/png;base64, %1'>%2" ).arg( QString( data.toBase64() ) ).arg( message() );
                QToolTip::showText( QCursor::pos(), html, this );
            }
        }
    }

    QString CImageScrollBar::message() const
    {
        if ( fCurrentImageNum == -1 )
            return QString();

        QString format = fMsgFormat;
        auto num = fCurrentImageNum;
        auto numImages = this->numImages();
        auto percentage = static_cast<int>(100.0 * num / 1.0 * numImages);

        format = format.replace( "%v", QString::number( num ) );
        format = format.replace( "%m", QString::number( numImages ) );
        format = format.replace( "%p", QString::number( percentage ) );

        return format;
    }

    int CImageScrollBar::numImages() const
    {
        if ( !hasImages() )
             return 0;
#ifdef BIF_SCROLLBAR_SUPPORT
        if ( fBIFFile )
            return fBIFFile->lastImageLoaded();
        else if ( fBIFModel )
            return fBIFModel->rowCount();
        else
#endif            
            return static_cast< int >( fImages.size() );
    }

    QImage CImageScrollBar::getImage( int imageNum ) const
    {
        if ( imageNum >= numImages() )
            return QImage();

#ifdef BIF_SCROLLBAR_SUPPORT
        if ( fBIFFile )
            return fBIFFile->image( imageNum );
        else if ( fBIFModel )
        {
            return fBIFModel->index( imageNum ).data( NBIF::CModel::ECustomRoles::eImage ).value< QImage >();
    }
        else
#endif            
            return fImages[imageNum];
    }

}