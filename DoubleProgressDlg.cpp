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

#include "DoubleProgressDlg.h"

#include <QScopedValueRollback>
#include <QTimer>
#include <QDebug>
#include <QShortcut>
#include <QStyle>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QCoreApplication>
#include <QProgressBar>

namespace NSABUtils
{
    const int gDefaultMinimumDuration{ 4000 };
    const int gMinWaitTime{ 50 };

    struct SBarInfo;
    class CDoubleProgressDlgImpl
    {
    public:
        CDoubleProgressDlgImpl(const QString & title, const QString & subTitle, int min, int max, CDoubleProgressDlg * dlg);
        ~CDoubleProgressDlgImpl();

        void updateTitleBar();
        void updateOnSetValue(bool primValueChanged);

        void setSingleProgressBarMode(bool value);

        void setCancelButtonText(QString label, bool init);
        void setCancelButton(QPushButton * button);

        void stopAutoShowTimer()
        {
            fForceTimer->stop();
        }

        void layout();
        void ensureSizeIsAtLeastSizeHint();
        void adoptChildWidget(QWidget * child);

        bool fCanceled{ false };
        bool fSingleProgressBarMode{ false };

        QTimer * fForceTimer{ nullptr };
        QElapsedTimer fStartPrimaryTime;
        QElapsedTimer fStartSecondaryTime;
        int fMinimumDuration{ 4000 };
        bool fForceHide{ false };
        bool fShownOnce{ false };
        bool fSetValueCalled{ false };
        bool fProcessingEvents{ false };
        bool fAutoReset{ true };
        bool fAutoClose{ true };

        CDoubleProgressDlg * fDialog{ nullptr };

        QLabel * fTitle{ nullptr };
        QLabel * fSubTitle{ nullptr };
        std::pair< bool, QPushButton * > fCancelButton{ true, nullptr };

        std::unique_ptr< SBarInfo > fPrimaryBar;
        std::unique_ptr< SBarInfo > fSecondaryBar;

        QShortcut * fEscapeShortcut{ nullptr };
    };

    struct SBarInfo
    {
        SBarInfo(CDoubleProgressDlgImpl * impl, int min = 0, int max = 100, const QString & format = QString("  %v of %m (%p%)  ")) :
            fRange({ min, max }),
            fFormat(format),
            fImpl(impl)
        {}

        void init()
        {
            if (fBar)
                return;

            fBar = new QProgressBar(fImpl->fDialog);
            fBar->setRange(fRange.first, fRange.second);
            fBar->setFormat(fFormat);

            fLabel = new QLabel(fImpl->fDialog->tr("Progress:"), fImpl->fDialog);
            fImpl->layout();
        }

        int value() const
        {
            return fBar ? fBar->value() : 0;
        }
        int min() const
        {
            return fRange.first;
        }
        int max() const
        {
            return fRange.second;
        }
        void setFormat(const QString & format)
        {
            fFormat = format;
            if (fBar)
                fBar->setFormat(format);
            fImpl->updateTitleBar();
        }

        QString format() const
        {
            return fFormat;
        }
        void reset() const
        {
            if (fBar)
                fBar->reset();
        }

        void setRange(int min, int max)
        {
            fRange = { min, max };
            if (fBar)
                fBar->setRange(min, max);
            fImpl->updateTitleBar();
        }

        void setValue(int value, bool isPrimary)
        {
            init();
            fBar->setValue(value);
            fImpl->updateOnSetValue(isPrimary);
            fImpl->updateTitleBar();
        }

        explicit operator bool() const noexcept
        {
            return fBar != nullptr && fLabel != nullptr;
        }

        bool isVisible() const
        {
            if (*this)
                return fBar->isVisible();
            return false;
        }

        void setVisible(bool visible)
        {
            if (visible)
                init();
            if (*this)
            {
                fBar->setVisible(visible);
                fLabel->setVisible(visible);
            }

            fImpl->updateTitleBar();
        }

        QString getProgressString() const
        {
            if (!*this)
                return QString();

            auto format = fImpl->fDialog->tr("%1").arg(fBar->format().trimmed());

            format.replace("%v", QString::number(fBar->value() / fEventsPerIncrement));
            format.replace("%m", QString::number(fBar->maximum() / fEventsPerIncrement ));
            format.replace("%p", QString::number(fBar->maximum() ? (100 * fBar->value() / fBar->maximum()) : 0));
            return format;
        }

        bool isValueMinimum() const
        {
            if (!*this)
                return false;

            return fBar->value() == fBar->minimum();
        }

        bool isValueMaximum() const
        {
            if (!*this)
                return false;
            return fBar->value() == fBar->maximum();
        }

        QSize sizeHint() const
        {
            if (!*this)
                return QSize(0, 0);

            int horizontalSpacing = fImpl->fDialog->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing, nullptr, fImpl->fDialog);

            auto tmp = fLabel->text();
            auto sz1 = fLabel->sizeHint();
            auto sz2 = fBar->sizeHint();

            auto retVal = QSize(sz1.width() + horizontalSpacing + sz2.width(), std::max(sz1.height(), sz2.height()));
            return retVal;
        }

        void setGeometry(QRect totalRect)
        {
            if (!*this)
                return;

            int horizontalSpacing = fImpl->fDialog->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing, nullptr, fImpl->fDialog);

            int labelW = fLabel->minimumSizeHint().width();
            QRect rect(totalRect);
            rect.setWidth(labelW);
            fLabel->setGeometry(rect);

            rect.setX(rect.x() + labelW + horizontalSpacing);
            rect.setWidth(totalRect.width() - labelW);

            fBar->setGeometry(rect);
        }

        inline void setGeometry(int x, int y, int w, int h)
        {
            return setGeometry(QRect(x, y, w, h));
        }

        void setLabel(const QString & text)
        {
            if (*this)
                fLabel->setText(text);
        }
        void setEventsPerIncrement( int value ) { fEventsPerIncrement = value; }
        int  eventsPerIncrement() const { return fEventsPerIncrement; }
    private:
        std::pair< int, int > fRange{ 0,100 };
        QProgressBar * fBar{ nullptr };
        QLabel * fLabel{ nullptr };
        QString fFormat;
        int fEventsPerIncrement{ 1 };
        CDoubleProgressDlgImpl * fImpl{ nullptr };
    };

    CDoubleProgressDlg::CDoubleProgressDlg(const QString & text, const QString & subTitle, const QString & cancelText, int min, int max, QWidget * parent, Qt::WindowFlags f) :
        QDialog(parent, f),
        fImpl(new CDoubleProgressDlgImpl(text, subTitle, min, max, this))
    {
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

        fImpl->setCancelButtonText(cancelText, true);
    }

    CDoubleProgressDlg::CDoubleProgressDlg(const QString & text, const QString & cancelText, int min, int max, QWidget * parent, Qt::WindowFlags f) :
        CDoubleProgressDlg(text, QString(), cancelText, min, max, parent, f)
    {
    }

    CDoubleProgressDlg::CDoubleProgressDlg(const QString & text, const QString & cancelText, QWidget * parent, Qt::WindowFlags f) :
        CDoubleProgressDlg(text, cancelText, 0, 100, parent, f)
    {}

    CDoubleProgressDlg::CDoubleProgressDlg(QWidget * parent, Qt::WindowFlags f) :
        CDoubleProgressDlg(QString(), QString(), parent, f)
    {}

    CDoubleProgressDlg::~CDoubleProgressDlg()
    {
    }

    void CDoubleProgressDlg::cancel()
    {
        slotCanceled();
    }

    void CDoubleProgressDlg::slotCanceled()
    {
        fImpl->fForceHide = true;
        reset( true );
        fImpl->fForceHide = false;
        fImpl->fCanceled = true;
    }

    void CDoubleProgressDlg::closeEvent(QCloseEvent * e)
    {
        emit canceled();
        QDialog::closeEvent(e);
    }

    void CDoubleProgressDlg::setPrimaryValue(int value)
    {
        fImpl->fPrimaryBar->setValue(value, true);
    }

    int CDoubleProgressDlg::primaryValue() const
    {
        return fImpl->fPrimaryBar->value();
    }

    void CDoubleProgressDlg::setPrimaryRange(int min, int max)
    {
        fImpl->fPrimaryBar->setRange(min, max);
        setPrimaryVisible( std::abs( max - min ) > 1 );
    }

    int CDoubleProgressDlg::primaryMin() const
    {
        return fImpl->fPrimaryBar->min();
    }

    int CDoubleProgressDlg::primaryMax() const
    {
        return fImpl->fPrimaryBar->max();
    }

    void CDoubleProgressDlg::setPrimaryFormat(const QString & format)
    {
        fImpl->fPrimaryBar->setFormat(format);
    }

    QString CDoubleProgressDlg::primaryFormat() const
    {
        return fImpl->fPrimaryBar->format();
    }

    void CDoubleProgressDlg::setSecondaryValue(int value)
    {
        fImpl->fSecondaryBar->setValue(value, false);
    }

    int CDoubleProgressDlg::secondaryValue() const
    {
        return fImpl->fSecondaryBar->value();
    }

    void CDoubleProgressDlg::setSecondaryRange(int min, int max)
    {
        fImpl->fSecondaryBar->setRange(min, max);
    }

    int CDoubleProgressDlg::secondaryMin() const
    {
        return fImpl->fSecondaryBar->min();
    }

    int CDoubleProgressDlg::secondaryMax() const
    {
        return fImpl->fSecondaryBar->max();
    }

    void CDoubleProgressDlg::setSecondaryFormat(const QString & format)
    {
        fImpl->fSecondaryBar->setFormat(format);
    }

    QString CDoubleProgressDlg::secondaryFormat() const
    {
        return fImpl->fSecondaryBar->format();
    }

    void CDoubleProgressDlg::setPrimaryProgressLabel(const QString & label)
    {
        fImpl->fPrimaryBar->setLabel(label);
        fImpl->ensureSizeIsAtLeastSizeHint();
    }

    void CDoubleProgressDlg::setSecondaryProgressLabel(const QString & label)
    {
        fImpl->fSecondaryBar->setLabel(label);
        fImpl->ensureSizeIsAtLeastSizeHint();
    }

    void CDoubleProgressDlg::setTitle(const QString & label)
    {
        fImpl->fTitle->setText(label);
        fImpl->ensureSizeIsAtLeastSizeHint();
    }

    void CDoubleProgressDlg::setSubTitle(const QString & label)
    {
        fImpl->fSubTitle->setText(label);
        fImpl->ensureSizeIsAtLeastSizeHint();
    }

    QString CDoubleProgressDlg::title() const
    {
        return fImpl->fTitle->text();
    }

    QString CDoubleProgressDlg::subTitle() const
    {
        return fImpl->fSubTitle->text();
    }

    void CDoubleProgressDlg::setCancelButtonText(const QString & text)
    {
        fImpl->setCancelButtonText(text, false);
    }

    QString CDoubleProgressDlg::cancelText() const
    {
        if (fImpl->fCancelButton.second)
            return fImpl->fCancelButton.second->text();
        return QString();
    }

    bool CDoubleProgressDlg::wasCanceled() const
    {
        return fImpl->fCanceled;
    }

    void CDoubleProgressDlg::setSingleProgressBarMode(bool value)
    {
        fImpl->setSingleProgressBarMode(value);
    }

    bool CDoubleProgressDlg::singleProgressBarMode() const
    {
        return fImpl->fSingleProgressBarMode;
    }

    void CDoubleProgressDlg::reset( bool canceled )
    {
        if (fImpl->fAutoClose || fImpl->fForceHide)
            hide();
        fImpl->fPrimaryBar->reset();
        fImpl->fSecondaryBar->reset();
        fImpl->fCanceled = canceled;
        fImpl->fForceTimer->stop();
        fImpl->fShownOnce = false;
        fImpl->fSetValueCalled = false;
    }

    void CDoubleProgressDlg::setMinimumDuration(int msec)
    {
        fImpl->fMinimumDuration = msec;
        if ( ( fImpl->fPrimaryBar->value() == -1 ) || fImpl->fPrimaryBar->isValueMinimum())
        {
            fImpl->fForceTimer->stop();
            if ( msec >= 0 )
                fImpl->fForceTimer->start(msec);
        }
    }

    int CDoubleProgressDlg::minumumDuration() const
    {
        return fImpl->fMinimumDuration;
    }

    void CDoubleProgressDlg::setAutoClose(bool autoClose)
    {
        fImpl->fAutoClose = autoClose;
    }

    bool CDoubleProgressDlg::autoClose() const
    {
        return fImpl->fAutoClose;
    }

    void CDoubleProgressDlg::setAutoReset(bool autoReset)
    {
        fImpl->fAutoReset = autoReset;
    }

    bool CDoubleProgressDlg::autoReset() const
    {
        return fImpl->fAutoReset;
    }

    int CDoubleProgressDlg::value() const
    {
        return primaryValue();
    }

    void CDoubleProgressDlg::setPrimaryEventsPerIncrement( int value )
    {
        fImpl->fPrimaryBar->setEventsPerIncrement( value );
    }

    int CDoubleProgressDlg::primaryEventsPerIncrement() const
    {
        return fImpl->fPrimaryBar->eventsPerIncrement();
    }

    void CDoubleProgressDlg::setPrimaryVisible( bool visible )
    {
        return fImpl->fPrimaryBar->setVisible( visible );
    }

    void CDoubleProgressDlg::setSecondaryEventsPerIncrement( int value )
    {
        fImpl->fSecondaryBar->setEventsPerIncrement( value );
    }

    int CDoubleProgressDlg::secondaryEventsPerIncrement() const
    {
        return fImpl->fSecondaryBar->eventsPerIncrement();
    }

    void CDoubleProgressDlg::stopAutoShowTimer() const // when you set a value, sometiems it can trigger a show you dont want
    {
        return fImpl->stopAutoShowTimer();
    }

    void CDoubleProgressDlg::setSecondaryVisible( bool visible )
    {
        return fImpl->fSecondaryBar->setVisible( visible );
    }

    void CDoubleProgressDlg::slotForceShow()
    {
        fImpl->fForceTimer->stop();
        if (fImpl->fShownOnce || fImpl->fCanceled)
            return;
        show();
        fImpl->fShownOnce = true;
    }

    void CDoubleProgressDlg::setCancelButton(QPushButton * button)
    {
        fImpl->setCancelButton(button);
    }

    QPushButton * CDoubleProgressDlg::cancelButton() const
    {
        return fImpl->fCancelButton.second;
    }

    void CDoubleProgressDlg::resizeEvent(QResizeEvent *)
    {
        fImpl->layout();
    }

    void CDoubleProgressDlg::changeEvent(QEvent * ev)
    {
        if (ev->type() == QEvent::StyleChange)
        {
            fImpl->layout();
        }
        //else if ( ev->type() == QEvent::LanguageChange )
        //{
        //    fImpl->retranslateStrings();
        //}
        QDialog::changeEvent(ev);
    }

    void CDoubleProgressDlg::showEvent(QShowEvent * e)
    {
        QDialog::showEvent(e);
        fImpl->ensureSizeIsAtLeastSizeHint();
        fImpl->fForceTimer->stop();
    }

    QSize CDoubleProgressDlg::sizeHint() const
    {
        auto tmp = fImpl->fTitle->text();
        auto titleSize = fImpl->fTitle->sizeHint();
        auto subTitleSize = fImpl->fSubTitle->isVisible() ? fImpl->fSubTitle->sizeHint() : QSize(0, 0);

        auto primBarAndLabelSize = fImpl->fPrimaryBar->sizeHint();
        auto secBarAndLabelSize = fImpl->fSecondaryBar->sizeHint();

        int marginBottom = style()->pixelMetric(QStyle::PM_LayoutBottomMargin, 0, this);
        int vSpacing = style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing, 0, this);
        int marginLeft = style()->pixelMetric(QStyle::PM_LayoutLeftMargin, 0, this);
        int marginRight = style()->pixelMetric(QStyle::PM_LayoutRightMargin, 0, this);

        int numHeights = 2;
        int height = marginBottom * 2 + titleSize.height() + subTitleSize.height() + primBarAndLabelSize.height() + secBarAndLabelSize.height();

        if (fImpl->fSubTitle->isVisible())
            numHeights++;
        if (*fImpl->fSecondaryBar)
            numHeights++;
        if (fImpl->fCancelButton.second)
        {
            height += fImpl->fCancelButton.second->sizeHint().height();
            numHeights++;
        }

        height += (numHeights - 1) * vSpacing;

        int width = primBarAndLabelSize.width();
        width = std::max(width, secBarAndLabelSize.width());
        width = std::max(width, titleSize.width());
        width = std::max(width, subTitleSize.width());
        width += marginLeft + marginRight;
        width = std::max(200, width);

        return QSize(width, height);
    }

    CDoubleProgressDlgImpl::CDoubleProgressDlgImpl(const QString & title, const QString & subTitle, int min, int max, CDoubleProgressDlg * dlg) :
        fDialog(dlg),
        fPrimaryBar(new SBarInfo(this)),
        fSecondaryBar(new SBarInfo(this))
    {
        fTitle = new QLabel(title, dlg);
        fTitle->setTextFormat(Qt::TextFormat::RichText);
        fTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        fSubTitle = new QLabel(subTitle, dlg);
        fSubTitle->setTextFormat(Qt::TextFormat::RichText);
        fSubTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        fPrimaryBar->init();
        fPrimaryBar->setRange(min, max);

        QObject::connect(fDialog, &CDoubleProgressDlg::canceled, fDialog, &CDoubleProgressDlg::slotCanceled);

        fForceTimer = new QTimer(fDialog);
        QObject::connect(fForceTimer, &QTimer::timeout, fDialog, &CDoubleProgressDlg::slotForceShow);

        fStartPrimaryTime.start();
        fStartSecondaryTime.start();
        fForceTimer->start(fMinimumDuration);
    }

    CDoubleProgressDlgImpl::~CDoubleProgressDlgImpl()
    {

    }

    void CDoubleProgressDlgImpl::updateTitleBar()
    {
        auto format = fDialog->tr("Progress: ");
        bool hasSecondary = false;
        if (fSecondaryBar->isVisible())
        {
            format += fDialog->tr("%1").arg(fSecondaryBar->getProgressString());
            hasSecondary = true;
        }
        if (fPrimaryBar->isVisible())
        {
            if (hasSecondary)
                format += " - ";
            format += fDialog->tr("%2").arg(fPrimaryBar->getProgressString());
        }
        fDialog->setWindowTitle(format);
    }

    void CDoubleProgressDlgImpl::updateOnSetValue(bool primValueChanged)
    {
        if (fShownOnce)
        {
            if (fDialog->isModal() && !fProcessingEvents)
            {
                const QScopedValueRollback< bool > guard(fProcessingEvents, true);
                QCoreApplication::processEvents();
            }
        }
        else
        {
            if ( ( !fSetValueCalled && ( ( fPrimaryBar->value() == 0 ) && ( fSecondaryBar->value() == 0 ) ) )
                || ( fPrimaryBar->isValueMinimum() && fSecondaryBar->isValueMinimum() ) )
            {
                if (primValueChanged)
                    fStartPrimaryTime.start();
                else
                    fStartSecondaryTime.start();
                fForceTimer->start(fMinimumDuration);
                fSetValueCalled = true;
                return;
            }
            else
            {
                fSetValueCalled = true;
                bool needShow = false;
                auto elapsed = primValueChanged ? fStartPrimaryTime.elapsed() : fStartSecondaryTime.elapsed();
                if (elapsed > fMinimumDuration)
                    needShow = true;
                else
                {
                    if (elapsed > gMinWaitTime)
                    {
                        auto min = primValueChanged ? fPrimaryBar->min() : fSecondaryBar->min();
                        auto max = primValueChanged ? fPrimaryBar->max() : fSecondaryBar->max();
                        auto value = primValueChanged ? fPrimaryBar->value() : fSecondaryBar->value();

                        auto totalSteps = max - min;
                        auto progress = value - min;
                        if (progress == 0)
                            progress = 1;

                        int estimate = 0;
                        if ((totalSteps - progress) >= (INT_MAX / elapsed))
                            estimate = (totalSteps - progress) / (progress * elapsed);
                        else
                            estimate = elapsed * (totalSteps - progress) / progress;

                        needShow = estimate >= fMinimumDuration;
                    }
                    if (needShow)
                    {
                        ensureSizeIsAtLeastSizeHint();
                        fDialog->show();
                        fShownOnce = true;
                    }
                }
            }
        }

        if (fAutoReset && primValueChanged && fPrimaryBar->isValueMaximum())
            fDialog->reset( false );
    }

    void CDoubleProgressDlgImpl::ensureSizeIsAtLeastSizeHint()
    {
        auto size = fDialog->sizeHint();
        if (fDialog->isVisible())
            size = size.expandedTo(fDialog->size());
        fDialog->resize(size);
    }

    void CDoubleProgressDlgImpl::setCancelButtonText(QString text, bool init)
    {
        if (init && text.isNull())
            text = fDialog->tr("Cancel");

        if (text.isNull())
        {
            setCancelButton(nullptr);
        }
        else
        {
            if (!fCancelButton.second)
                setCancelButton(new QPushButton(fDialog));
            fCancelButton.second->setText(text);
        }
        ensureSizeIsAtLeastSizeHint();
    }


    void CDoubleProgressDlgImpl::setSingleProgressBarMode(bool value)
    {
        fSingleProgressBarMode = value;
        fSecondaryBar->setVisible(!fSingleProgressBarMode);
    }


    void CDoubleProgressDlgImpl::setCancelButton(QPushButton * button)
    {
        if (button == fCancelButton.second)
        {
            qDebug() << "CDoubleProgressDlg::setCancelButton: Attempt to set the same button again";
            return;
        }
        delete fCancelButton.second;
        fCancelButton.second = button;
        if (fCancelButton.second)
        {
            fDialog->connect(fCancelButton.second, &QPushButton::clicked, fDialog, &CDoubleProgressDlg::canceled);
            fEscapeShortcut = new QShortcut(QKeySequence::Cancel, fDialog, SIGNAL(canceled()));
        }
        else
        {
            delete fEscapeShortcut;
            fEscapeShortcut = nullptr;
        }
        adoptChildWidget(fCancelButton.second);
    }

    void CDoubleProgressDlgImpl::adoptChildWidget(QWidget * child)
    {
        if (child)
        {
            if (child->parentWidget() == fDialog)
                child->hide(); // until after ensureSizeIsAtLeastSizeHint()
            else
                child->setParent(fDialog, { });
        }
        ensureSizeIsAtLeastSizeHint();
        if (child)
            child->show();
    }

    void CDoubleProgressDlgImpl::layout()
    {
        int verticalSpacing = fDialog->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing, nullptr, fDialog);
        int bottomMargin = fDialog->style()->pixelMetric(QStyle::PM_LayoutBottomMargin, nullptr, fDialog);
        int leftMargin = std::min(fDialog->width() / 10, fDialog->style()->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr, fDialog));
        int rightMargin = std::min(fDialog->width() / 10, fDialog->style()->pixelMetric(QStyle::PM_LayoutRightMargin, nullptr, fDialog));

        int additionalSpacing = 0;
        QSize cancelSize = fCancelButton.second ? fCancelButton.second->sizeHint() : QSize(0, 0);
        QSize primaryBarHeight = fPrimaryBar->sizeHint();
        QSize secondaryBarHeight = fSecondaryBar->sizeHint();

        int cancelSizeWithSpace = 0;
        int labelHeight = 0;

        // Find spacing and sizes that fit.  It is important that a progress
        // dialog can be made very small if the user demands it so.
        for (int attempt = 5; attempt--;)
        {
            cancelSizeWithSpace = fCancelButton.second ? cancelSize.height() + verticalSpacing : 0;
            labelHeight = std::max(0, fDialog->height() - bottomMargin - primaryBarHeight.height() - secondaryBarHeight.height() - verticalSpacing - cancelSizeWithSpace);

            if ((2 * labelHeight + verticalSpacing) < fDialog->height() / 4)
            {
                // Getting cramped
                verticalSpacing /= 2;
                bottomMargin /= 2;
                if (fCancelButton.second)
                    cancelSize.setHeight(std::max(4, cancelSize.height() - verticalSpacing - 2));
                primaryBarHeight.setHeight(std::max(4, primaryBarHeight.height() - verticalSpacing - 1));
                secondaryBarHeight.setHeight(std::max(4, secondaryBarHeight.height() - verticalSpacing - 1));
            }
            else
            {
                break;
            }
        }

        if (fCancelButton.second)
        {
            int x = 0;
            const bool centered = bool(fDialog->style()->styleHint(QStyle::SH_ProgressDialog_CenterCancelButton, nullptr, fDialog));
            if (centered)
                x = fDialog->width() / 2 - cancelSize.width() / 2;
            else
                x = fDialog->width() - rightMargin - cancelSize.width();

            fCancelButton.second->setGeometry(
                x, fDialog->height() - bottomMargin - cancelSize.height(),
                cancelSize.width(), cancelSize.height());
        }

        fTitle->setGeometry(leftMargin, additionalSpacing, fDialog->width() - leftMargin - rightMargin, labelHeight);
        //if ( fSubTitle )
        //    fSubTitle->setGeometry( leftMargin, additionalSpacing, fDialog->width() - leftMargin - rightMargin, labelHeight );

        auto primGeom = QRect(leftMargin, labelHeight + verticalSpacing + additionalSpacing, fDialog->width() - leftMargin - rightMargin, primaryBarHeight.height());
        auto secondGeom = QRect(primGeom.left(), primGeom.y() + primGeom.height() + verticalSpacing, primGeom.width(), secondaryBarHeight.height());

        fPrimaryBar->setGeometry(primGeom);
        fSecondaryBar->setGeometry(secondGeom);
    }
}
