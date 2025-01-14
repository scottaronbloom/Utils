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

#include "ButtonEnabler.h"

#include <QAbstractItemView>
#include <QAbstractButton>
#include <QLineEdit>
#include <QFile>
#include <QAction>

namespace NSABUtils
{
    CButtonEnabler::CButtonEnabler( QAbstractItemView *view, QAbstractButton *btn, QObject *parent ) :
        QObject( parent ),
        fButton( btn )
    {
        if ( parent == nullptr )
            setParent( btn );
        if ( view->selectionModel() )
            connect( view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CButtonEnabler::slotSelectionChanged );
        slotReset();
        if ( view->selectionModel() )
            slotSelectionChanged( view->selectionModel()->selection(), QItemSelection() );
        else
            slotSelectionChanged( QItemSelection(), QItemSelection() );
    }

    CButtonEnabler::CButtonEnabler( QAbstractItemView *view, QAction *action, QObject *parent ) :
        QObject( parent ),
        fAction( action )
    {
        if ( parent == nullptr )
            setParent( action );
        if ( view->selectionModel() )
            connect( view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CButtonEnabler::slotSelectionChanged );
        slotReset();
        if ( view->selectionModel() )
            slotSelectionChanged( view->selectionModel()->selection(), QItemSelection() );
        else
            slotSelectionChanged( QItemSelection(), QItemSelection() );
    }

    CButtonEnabler::CButtonEnabler( QLineEdit *le, QAbstractButton *btn, QObject *parent ) :
        QObject( parent ),
        fButton( btn )
    {
        if ( parent == nullptr )
            setParent( btn );
        connect( le, &QLineEdit::textChanged, this, &CButtonEnabler::slotTextChanged );
        slotReset();
        slotTextChanged( le->text() );
    }

    CButtonEnabler::CButtonEnabler( QLineEdit *le, QAction *action, QObject *parent ) :
        QObject( parent ),
        fAction( action )
    {
        if ( parent == nullptr )
            setParent( action );
        connect( le, &QLineEdit::textChanged, this, &CButtonEnabler::slotTextChanged );
        slotReset();
        slotTextChanged( le->text() );
    }

    void CButtonEnabler::slotReset()
    {
        setEnabled( false );
    }

    void CButtonEnabler::setEnabled( bool enabled )
    {
        if ( fButton )
            fButton->setEnabled( enabled );
        if ( fAction )
            fAction->setEnabled( enabled );
    }

    void CButtonEnabler::slotSelectionChanged( const QItemSelection &selected, const QItemSelection & )
    {
        bool enabled = selected.count() && selected.first().isValid();
        if ( !enabled )
        {
            auto selectionModel = dynamic_cast< QItemSelectionModel * >( sender() );
            if ( selectionModel )
            {
                auto idxs = selectionModel->selectedIndexes();
                enabled = idxs.count() && idxs.first().isValid();
            }
        }
        setEnabled( enabled );
    }

    void CButtonEnabler::slotTextChanged( const QString &txt )
    {
        bool enabled = !txt.isEmpty();
        if ( fLineEditIsFile && enabled )
        {
            enabled = QFile( txt ).exists();
        }
        setEnabled( enabled );
    }
}
