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

#include "WidgetEnabler.h"
#include <QAbstractButton>
#include <QGroupBox>

namespace NSABUtils
{
    CWidgetEnabler::CWidgetEnabler( QAbstractButton *btn, QWidget *widget, QObject *parent ) :
        CWidgetEnabler( btn, { widget }, parent )
    {
    }

    CWidgetEnabler::CWidgetEnabler( QAbstractButton *checkBox, const std::initializer_list< QWidget * > &widgets, QObject *parent ) :
        CWidgetEnabler( { checkBox, nullptr }, widgets, parent )
    {
    }

    CWidgetEnabler::CWidgetEnabler( QGroupBox *gb, QWidget *widget, QObject *parent ) :
        CWidgetEnabler( gb, { widget }, parent )
    {
    }

    CWidgetEnabler::CWidgetEnabler( QGroupBox *gb, const std::initializer_list< QWidget * > &widgets, QObject *parent ) :
        CWidgetEnabler( { nullptr, gb }, widgets, parent )
    {
    }

    CWidgetEnabler::CWidgetEnabler( const std::pair< QAbstractButton *, QGroupBox * > &checker, const std::initializer_list< QWidget * > &widgets, QObject *parent ) :
        QObject( parent ),
        fChecker( checker ),
        fWidgets( widgets )
    {
        Q_ASSERT( ( fChecker.first != nullptr ) || ( fChecker.second != nullptr ) );
        Q_ASSERT( !fWidgets.empty() );
        if ( parent == nullptr )
        {
            setParent( fChecker.first ? static_cast< QWidget * >( fChecker.first ) : fChecker.second );
        }

        fInitState = checkState();
        if ( fChecker.first )
        {
            connect( fChecker.first, &QAbstractButton::toggled, this, &CWidgetEnabler::slotButtonToggled );
            slotButtonToggled( fInitState );
        }
        else if ( fChecker.second )
        {
            Q_ASSERT( fChecker.second->isCheckable() );
            connect( fChecker.second, &QGroupBox::clicked, this, &CWidgetEnabler::slotSetWidgetsEnabled );
            slotSetWidgetsEnabled( fInitState );
        }
    }

    bool CWidgetEnabler::checkState() const
    {
        if ( fChecker.first )
            return fChecker.first->isChecked();
        else
            return fChecker.second->isChecked();
    }

    void CWidgetEnabler::slotReset()
    {
        if ( fChecker.first )
            fChecker.first->setChecked( fInitState );
        else
            fChecker.second->setChecked( fInitState );
    }

    void CWidgetEnabler::slotButtonToggled( bool checked )
    {
        slotSetWidgetsEnabled( checked );
    }

    void CWidgetEnabler::slotSetWidgetsEnabled( bool enabled )
    {
        for ( auto &&ii : fWidgets )
        {
            ii->setEnabled( enabled );
        }
    }
}
