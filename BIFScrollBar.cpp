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

#include "BIFScrollBar.h"
#include "BIFFile.h"

#include <QDebug>
#include <QToolTip>
#include <QBuffer>
#include <QByteArray>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOption>

namespace NSABUtils
{
    namespace NBIF
    {
        CScrollBar::CScrollBar(QWidget * parent /*= 0 */) :
            QScrollBar(parent)
        {
            init();
        }

        CScrollBar::CScrollBar(Qt::Orientation orientation, QWidget * parent /*= nullptr */) :
            QScrollBar(orientation, parent)
        {
            init();
        }

        void CScrollBar::init()
        {
            connect(this, &QScrollBar::valueChanged, this, &CScrollBar::slotValueChanged);
            connect(this, &QScrollBar::sliderMoved, this, &CScrollBar::slotSliderMoved);
            connect(this, &QScrollBar::sliderPressed, this, &CScrollBar::slotSliderPressed);
            connect(this, &QScrollBar::sliderReleased, this, &CScrollBar::slotSliderReleased);
        }

        void CScrollBar::setBIFFile(std::shared_ptr< CFile > bifFile)
        {
            fBIFFile = bifFile;
            slotValueChanged(0);
        }

        void CScrollBar::slotValueChanged(int value)
        {
            updateValue(value);
        }

        void CScrollBar::slotSliderMoved(int value)
        {
            updateValue(value);
        }

        void CScrollBar::updateValue(int value)
        {
            if (!fBIFFile)
                return;

            setCurrentImageNum(-1);
            int min = this->minimum();
            int max = this->maximum();

            if (min == max)
                return;

            auto percentage = 1.0 * value / (max - min);

            auto numImagesLoaded = fBIFFile->lastImageLoaded();
            auto imageNum = static_cast<int>(percentage * numImagesLoaded);
            if (imageNum >= numImagesLoaded)
                imageNum = numImagesLoaded;
            setCurrentImageNum(imageNum);
        }

        int CScrollBar::pixelPosToRangeValue(int pos) const
        {
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            
            auto rect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, this);

            int total = 0;
            int spanMin = 0;
            int spanMax = 0;
            if (orientation() == Qt::Horizontal)
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
                if (opt.upsideDown)
                    return max;
                else
                    return min;
            }
            auto range = max - min;
            if ( pos >= spanMax )
            {
                if (opt.upsideDown)
                    return min;
                else
                    return max;
            }

            auto percentage = 100.0*pos / (spanMax - spanMin);
            auto tmp = static_cast<int>(percentage * (range) / 100.0);
            auto value = opt.upsideDown ? max - tmp : tmp + min;
            return value;
        }

        void CScrollBar::updateImageFromPos()
        {
            auto currPos = mapFromGlobal(QCursor::pos());
            int value = 0;
            if (orientation() == Qt::Horizontal)
                value = pixelPosToRangeValue(currPos.x());
            else
                value = pixelPosToRangeValue(currPos.y());

            updateValue(value);
        }


        void CScrollBar::slotSliderPressed()
        {
            fShowImage = fBIFFile.operator bool();
            updateImage();
        }

        void CScrollBar::slotSliderReleased()
        {
            fShowImage = false;
            updateImage();
        }

        bool CScrollBar::event(QEvent*event)
        {
            if (event->type() == QEvent::HoverEnter)
            {
                if (fUpdateOnScrollOnly)
                {
                    fShowImage = true;
                    updateImageFromPos();
                }
            }
            else if (event->type() == QEvent::HoverLeave)
            {
                if (fUpdateOnScrollOnly)
                {
                    fShowImage = true;
                    updateImageFromPos();
                }
            }
            else if (event->type() == QEvent::HoverMove)
            {
                if (fUpdateOnScrollOnly)
                {
                    updateImageFromPos();
                }
            }

            return QScrollBar::event(event);
        }

        void CScrollBar::setCurrentImageNum(int imageNum)
        {
            fCurrentImageNum = imageNum;
            updateImage();
        }

        void CScrollBar::updateImage()
        {
            if (!fShowImage)
                QToolTip::hideText();
            else if (fBIFFile)
            {
                fCurrentImage = fBIFFile->image(fCurrentImageNum);
                if (!fCurrentImage.isNull())
                {
                    QByteArray data;
                    QBuffer buffer(&data);
                    fCurrentImage.save(&buffer, "PNG");

                    auto html = QString("<img src='data:image/png;base64, %1'>%2").arg(QString(data.toBase64())).arg(message());
                    QToolTip::showText(QCursor::pos(), html, this);
                }
            }
        }

        QString CScrollBar::message() const
        {
            if (fCurrentImageNum == -1)
                return QString();

            QString format = fMsgFormat;
            auto num = fCurrentImageNum;
            auto numImages = fBIFFile->lastImageLoaded();
            auto percentage = static_cast<int>(100.0*num / 1.0*numImages);

            format = format.replace("%v", QString::number( num ));
            format = format.replace("%m", QString::number(numImages));
            format = format.replace("%p", QString::number(percentage));

            return format;
        }
    }
}