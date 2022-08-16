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

#include "HyperLinkLineEdit.h"

#include <QStyle>
#include <QStyleOptionFrame>
#include <QApplication>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QDesktopServices>
#include <QDebug>

CHyperLinkLineEdit::CHyperLinkLineEdit( QWidget * parent /*= nullptr */ ) :
    QTextEdit( parent )
{
    QSizePolicy sizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    sizePolicy.setHorizontalStretch( 0 );
    sizePolicy.setVerticalStretch( 0 );
    sizePolicy.setHeightForWidth( this->sizePolicy().hasHeightForWidth() );

    setSizePolicy( sizePolicy );

    setLineWrapMode( QTextEdit::NoWrap );
    setWordWrapMode( QTextOption::WrapMode::NoWrap );

    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    document()->setMaximumBlockCount( 1 );

    setMaximumHeight( sizeHint().height() );
    setAcceptRichText( true );

    setMouseTracking( true );
    Q_ASSERT( parent );
    if ( parent )
        parent->installEventFilter( this );
}

CHyperLinkLineEdit::CHyperLinkLineEdit( const QString & text, QWidget * parent /*= nullptr */ ) :
    CHyperLinkLineEdit( parent )
{
    setText( text );
}

CHyperLinkLineEdit::~CHyperLinkLineEdit()
{

}

void CHyperLinkLineEdit::keyPressEvent( QKeyEvent * event )
{
    if ( ( event->key() == Qt::Key_Return )
         || ( event->key() == Qt::Key_Enter ) )
    {
        event->ignore();
        return;
    }
    else if ( event->key() == Qt::Key_Tab )
    {
        event->ignore();
        return;
    }
    else if ( event->key() == Qt::Key_Backtab )
    {
        event->ignore();
        return;
    }
    QTextEdit::keyPressEvent( event );
}

void CHyperLinkLineEdit::mouseMoveEvent( QMouseEvent * event )
{
    fAnchor = this->anchorAt( event->pos() );
    if ( !fAnchor.isEmpty() )
    {
        if ( !QApplication::overrideCursor() )
            QApplication::setOverrideCursor( Qt::PointingHandCursor );
    }
    else
        QApplication::restoreOverrideCursor();
}

bool CHyperLinkLineEdit::eventFilter( QObject * watched, QEvent * /*event*/ )
{
    if ( watched != this )
    {
        auto globalPos = QCursor::pos();
        auto myPos = this->mapFromGlobal( globalPos );
        if ( !rect().contains( myPos ) )
        {
            QApplication::restoreOverrideCursor();
        }
    }
    return false;
}

void CHyperLinkLineEdit::mouseReleaseEvent( QMouseEvent * /*event*/ )
{
    if ( !fAnchor.isEmpty() )
    {
        if ( fAnchor.indexOf( "://" ) == -1 )
            fAnchor = "http://" + fAnchor;
        QDesktopServices::openUrl( QUrl( fAnchor ) );
        QApplication::restoreOverrideCursor();
    }
}

QSize CHyperLinkLineEdit::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm( font() );
    const int iconSize = style()->pixelMetric( QStyle::PM_SmallIconSize, nullptr, this );
    
    //const QMargins tm = d->effectiveTextMargins();

    int h = qMax( fm.height(), qMax( 14, iconSize - 2 ) ) + 2 * 1 /*QLineEditPrivate::verticalMargin*/
        //+ tm.left() + tm.right()
        + 1 /* topmargin() */ + 1 /*bottommargin() */;

    int w = fm.horizontalAdvance( QLatin1Char( 'x' ) ) * 17 + 2 * 2 /*QLineEditPrivate::horizontalMargin */
        //+ tm.left() + tm.right()
        + 0 /* topmargin() */ + 0 /*bottommargin() */;

    QStyleOptionFrame opt;
    initStyleOption( &opt );
    return ( style()->sizeFromContents( QStyle::CT_LineEdit, &opt, QSize( w, h ).
             expandedTo( QApplication::globalStrut() ), this ) );
}

QString CHyperLinkLineEdit::text() const
{
    return toPlainText();
}

QString CHyperLinkLineEdit::addText( const QString & text, bool & urlFound ) const
{
    auto pos = text.indexOf( QRegularExpression( R"(\S)" ) );
    if ( pos == -1 )
        return text;
    auto pre = text.left( pos );
    auto remaining = text.mid( pos );
    bool isURL = false;
    isURL = isURL || remaining.startsWith( "www" );
    isURL = isURL || remaining.startsWith( "http://" );
    isURL = isURL || remaining.startsWith( "https://" );
    if ( !isURL )
    {
        auto match = QRegularExpression( R"(\:\d+)" ).match( remaining );
        if ( match.hasMatch() && match.capturedEnd() == remaining.length() )
            isURL = true;
    }

    if ( isURL )
    {
        auto retVal = QString( R"(%1<a href="%2">%2</a>)" ).arg( pre ).arg( remaining );
        urlFound = true;
        return retVal;
    }
    return text;
}

void CHyperLinkLineEdit::setText( const QString & text )
{
    QString actualString;
    int prevPos = 0;
    auto pos = text.indexOf( QRegularExpression( R"(\s)" ), prevPos );
    bool urlFound = false;
    while ( pos != -1 )
    {
        actualString += addText( text.mid( prevPos, pos - prevPos ), urlFound );
        prevPos = pos;
        pos = text.indexOf( QRegularExpression( R"(\s)" ), prevPos+1 );
    }

    actualString += addText( text.mid( prevPos ), urlFound );
    if ( urlFound )
        QTextEdit::setHtml( actualString );
    else
        QTextEdit::setPlainText( actualString );
}

