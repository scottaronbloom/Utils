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

#include "DelaySpinBox.h"
#include "QtUtils.h"

#include <QTimer>

namespace NSABUtils
{
    CDelaySpinBox::CDelaySpinBox(QWidget* parent) :
        CDelaySpinBox(250, parent)
    {
    }

    CDelaySpinBox::CDelaySpinBox(int delayMS, QWidget* parent) :
        QSpinBox(parent)
    {
#if QT_VERSION > QT_VERSION_CHECK(5,14,0)
        connect(this, &QSpinBox::textChanged, this, &CDelaySpinBox::slotTextChanged);
#endif
        connect(this, QOverload<int>::of(&QSpinBox::valueChanged), this, &CDelaySpinBox::slotValueChanged);
        connect(this, &QSpinBox::editingFinished, this, &CDelaySpinBox::slotEditingFinished);

        fValueChangedTimer = new QTimer(this);
        fValueChangedTimer->setSingleShot(true);
        connect(fValueChangedTimer, &QTimer::timeout, this, &CDelaySpinBox::slotValueChangedTimerTimeout);

        fEditFinishedTimer = new QTimer(this);
        fEditFinishedTimer->setSingleShot(true);
        connect(fEditFinishedTimer, &QTimer::timeout, this, &CDelaySpinBox::slotEditingFinishedTimerTimeout);

#if QT_VERSION > QT_VERSION_CHECK(5,14,0)
        fTextChangedTimer = new QTimer(this);
        fTextChangedTimer->setSingleShot(true);
        connect(fTextChangedTimer, &QTimer::timeout, this, &CDelaySpinBox::slotTextChangedTimerTimeout);
#endif

        setDelay(delayMS);
    }

    CDelaySpinBox::~CDelaySpinBox()
    {
    }

    void CDelaySpinBox::setValue(int value)
    {
#if QT_VERSION > QT_VERSION_CHECK(5,14,0)
        disconnect(this, &QSpinBox::textChanged, this, &CDelaySpinBox::slotTextChanged);
#endif

        disconnect(this, QOverload<int>::of(&QSpinBox::valueChanged), this, &CDelaySpinBox::slotValueChanged);
        disconnect(this, &QSpinBox::editingFinished, this, &CDelaySpinBox::slotEditingFinished);

        QSpinBox::setValue(value);

#if QT_VERSION > QT_VERSION_CHECK(5,14,0)
        connect(this, &QSpinBox::textChanged, this, &CDelaySpinBox::slotTextChanged);
#endif
        connect(this, QOverload<int>::of(&QSpinBox::valueChanged), this, &CDelaySpinBox::slotValueChanged);
        connect(this, &QSpinBox::editingFinished, this, &CDelaySpinBox::slotEditingFinished);
    }

    void CDelaySpinBox::setDelay(int delayMS)
    {
        fDelayMS = delayMS;
        NSABUtils::updateTimer(fDelayMS, fValueChangedTimer);
        NSABUtils::updateTimer(fDelayMS, fEditFinishedTimer);
#if QT_VERSION > QT_VERSION_CHECK(5,14,0)
        NSABUtils::updateTimer(fDelayMS, fTextChangedTimer);
#endif
    }

    void CDelaySpinBox::slotValueChanged()
    {
        fValueChangedTimer->stop();
        fValueChangedTimer->start();
    }

#if QT_VERSION > QT_VERSION_CHECK(5,14,0)
    void CDelaySpinBox::slotTextChanged()
    {
        fTextChangedTimer->stop();
        fTextChangedTimer->start();
    }
#endif

    void CDelaySpinBox::slotEditingFinished()
    {
        fEditFinishedTimer->stop();
        fEditFinishedTimer->start();
    }

    void CDelaySpinBox::slotValueChangedTimerTimeout()
    {
        emit sigValueChangedAfterDelay(value());
    }

#if QT_VERSION > QT_VERSION_CHECK(5,14,0)
    void CDelaySpinBox::slotTextChangedTimerTimeout()
    {
        emit sigTextChangedAfterDelay(text());
    }
#endif

    void CDelaySpinBox::slotEditingFinishedTimerTimeout()
    {
        emit sigEditingFinishedAfterDelay();
    }
}