// The MIT License( MIT )
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

#include "DelayLineEdit.h"

#include <QTimer>

CDelayLineEdit::CDelayLineEdit( QWidget* parent ) :
    CDelayLineEdit( QString(), 250, parent )
{
}

CDelayLineEdit::CDelayLineEdit( const QString & text, QWidget* parent ) :
    CDelayLineEdit( text, 250, parent )
{
}

CDelayLineEdit::CDelayLineEdit( const QString& text, int delayMS, QWidget* parent ) :
    QLineEdit( text, parent ),
    fDelayMS( delayMS )
{
    connect( this, &QLineEdit::textChanged, this, &CDelayLineEdit::slotTextChanged );
    connect( this, &QLineEdit::textEdited, this, &CDelayLineEdit::slotTextEdited );

    fChangedTimer = new QTimer( this );
    fChangedTimer->setSingleShot( true );
    connect( fChangedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotChangedTimerTimeout );

    fEditedTimer = new QTimer( this );
    fEditedTimer->setSingleShot( true );
    connect( fEditedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotEditTimerTimeout );

    setDelay( delayMS );
}

void CDelayLineEdit::setText( const QString & text, bool emitSignal )
{
    if ( emitSignal )
        QLineEdit::setText( text );
    else
    {
        disconnect( this, &QLineEdit::textChanged, this, &CDelayLineEdit::slotTextChanged );
        disconnect( this, &QLineEdit::textEdited, this, &CDelayLineEdit::slotTextEdited );

        QLineEdit::setText( text );

        connect( this, &QLineEdit::textChanged, this, &CDelayLineEdit::slotTextChanged );
        connect( this, &QLineEdit::textEdited, this, &CDelayLineEdit::slotTextEdited );
    }
}

void CDelayLineEdit::setDelay( int delayMS )
{
    fDelayMS = delayMS;
    bool isActive = fChangedTimer->isActive();
    fChangedTimer->stop();
    fChangedTimer->setInterval( fDelayMS );
    if ( isActive )
        fChangedTimer->start();
    isActive = fEditedTimer->isActive();
    fEditedTimer->stop();
    fEditedTimer->setInterval( fDelayMS );
    if ( isActive )
        fEditedTimer->start();
}

CDelayLineEdit::~CDelayLineEdit()
{
}

void CDelayLineEdit::slotTextChanged()
{
    fChangedTimer->stop();
    fChangedTimer->start();
}

void CDelayLineEdit::slotTextEdited()
{
    fEditedTimer->stop();
    fEditedTimer->start();
}


void CDelayLineEdit::slotChangedTimerTimeout()
{
    emit sigTextChanged( text() );
}

void CDelayLineEdit::slotEditTimerTimeout()
{
    emit sigTextEdited( text() );
}