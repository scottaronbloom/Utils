// The MIT License( MIT )
//
// Copyright( c ) 2023 Scott Aron Bloom
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

#include "LineEditWithSuffix.h"
#include <QLineEdit>
#include <QWidgetAction>
#include <QLabel>
#include <QString>
#include <QList>
#include <QDebug>
#include <QFontMetrics>
#include <QEvent>

namespace NSABUtils
{
    CLineEditWithSuffix::CLineEditWithSuffix( QWidget *parent ) :
        CLineEditWithSuffix( QString(), QString(), parent )
    {
    }

    CLineEditWithSuffix::CLineEditWithSuffix( const QString &text, QWidget *parent /*= nullptr */ ) :
        CLineEditWithSuffix( text, QString(), parent )
    {
    }

    CLineEditWithSuffix::CLineEditWithSuffix( const QString &text, const QString &suffix, QWidget *parent ) :
        QLineEdit( text, parent )
    {
        setSuffix( suffix );
    }

    CLineEditWithSuffix::~CLineEditWithSuffix()
    {
    }

    void CLineEditWithSuffix::setSuffix( const QString &suffix )
    {
        if ( suffix == this->suffix() )
            return;

        if ( suffix.isEmpty() )
        {
            if ( fSuffixLabel )
            {
                delete fSuffixLabel;
                fSuffixLabel = nullptr;
            }
        }
        else
        {
            if ( !fSuffixLabel )
                fSuffixLabel = new QLabel( suffix, this );
            else
                fSuffixLabel->setText( suffix );

            fSuffixLabel->show();
        }
        positionSuffixLabel();
    }

    void CLineEditWithSuffix::positionSuffixLabel()
    {
        int rhsMargin = 0;
        if ( fSuffixLabel && !fSuffixLabel->text().isEmpty() )
        {
            auto myRect = rect();
            QFontMetrics fm( fSuffixLabel->font() );

            auto labelRect = fm.boundingRect( fSuffixLabel->text() );
            rhsMargin = ( labelRect.width() + ( 2 * 4 ) );   // 4 pixel margin on either side of the text

            auto currGeom = fSuffixLabel->geometry();
            QRect widgetGeometry( QPoint( rect().width() - rhsMargin, ( rect().height() - labelRect.height() ) / 2 ), labelRect.size() );
            fSuffixLabel->setGeometry( widgetGeometry );
        }
        setTextMargins( 0, 0, rhsMargin, 0 );
    }

    QString CLineEditWithSuffix::suffix() const
    {
        if ( fSuffixLabel )
            return fSuffixLabel->text();
        return {};
    }

    bool CLineEditWithSuffix::event( QEvent *e )
    {
        if ( e->type() == QEvent::Resize )
        {
            positionSuffixLabel();
        }
        return QLineEdit::event( e );
    }
}
