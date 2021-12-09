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
#include <QKeyEvent>
#include "QtUtils.h"

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
    connectToEditor( true );

    fChangedTimer = new QTimer( this );
    fChangedTimer->setSingleShot( true );
    connect( fChangedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotChangedTimerTimeout );

    fEditedTimer = new QTimer( this );
    fEditedTimer->setSingleShot( true );
    connect( fEditedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotEditTimerTimeout );

    setDelay( delayMS );
}

void CDelayLineEdit::setText( const QString &text )
{
    if ( !isVisible() )
    {
        connectToEditor( false );
    }

    QLineEdit::setText( text );

    if ( !isVisible() )
    {
        connectToEditor( true );
    }
}

void CDelayLineEdit::setDelay( int delayMS )
{
    fDelayMS = delayMS;
    NQtUtils::updateTimer( fDelayMS, fChangedTimer );
    NQtUtils::updateTimer( fDelayMS, fEditedTimer );
}

CDelayLineEdit::~CDelayLineEdit()
{
}

void CDelayLineEdit::slotTextChanged()
{
    fChangedTimer->stop();
    fChangedTimer->start();
    setLineEditColor( ELineEditStatus::ePending );
}

void CDelayLineEdit::slotTextEdited()
{
    fEditingFinished = false;
    fEditedTimer->stop();
    fEditedTimer->start();
    setLineEditColor( ELineEditStatus::ePending );
}

void CDelayLineEdit::keyPressEvent( QKeyEvent * event )
{
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
        fEditingFinished = true;
        if ( fStatus == ELineEditStatus::eOK )
            emit sigFinishedEditingAfterDelay();
    }
}

void CDelayLineEdit::slotChangedTimerTimeout()
{
    setLineEditColor( fIsOK.first ? fIsOK.first( text() ) : true );
    emit sigTextChangedAfterDelay( text() );
}

void CDelayLineEdit::slotEditTimerTimeout()
{
    setLineEditColor( fIsOK.first ? fIsOK.first( text() ) : true );
    emit sigTextEditedAfterDelay( text() );
    if ( fEditingFinished )
        emit sigFinishedEditingAfterDelay();
}

void CDelayLineEdit::connectToEditor( bool connectOrDisconnect )
{
    if ( connectOrDisconnect )
    {
        connect( this, &QLineEdit::textChanged, this, &CDelayLineEdit::slotTextChanged );
        connect( this, &QLineEdit::textEdited, this, &CDelayLineEdit::slotTextEdited );
    }
    else
    {
        disconnect( this, &QLineEdit::textChanged, this, &CDelayLineEdit::slotTextChanged );
        disconnect( this, &QLineEdit::textEdited, this, &CDelayLineEdit::slotTextEdited );
    }
}

void CDelayLineEdit::setLineEditColor( bool aOK )
{
    setLineEditColor( aOK ? ELineEditStatus::eOK : ELineEditStatus::eNotOK );
}

void CDelayLineEdit::setLineEditColor( ELineEditStatus status )
{
    fStatus = status;
    setToolTip( QString() );
    if ( status == ELineEditStatus::ePending )
        setStyleSheet( "QLineEdit { background-color: #b7bfaf }" );
    else if ( status == ELineEditStatus::eOK )
        setStyleSheet( "QLineEdit { background-color: white }" );
    else if ( status == ELineEditStatus::eNotOK )
    {
        auto errorMsg = fIsOK.second;
        if ( !errorMsg.isEmpty() )
        {
            if ( errorMsg.contains( "%1" ) )
                errorMsg = errorMsg.arg( text() );
            setToolTip( errorMsg );
        }
        setStyleSheet( "QLineEdit { background-color: red }" );
    }
}

