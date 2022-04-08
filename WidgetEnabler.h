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

#ifndef __WIDGETENABLER_H
#define __WIDGETENABLER_H

#include "SABUtilsExport.h"

#include <QObject>
#include <initializer_list>
class QCheckBox;
class QGroupBox;
namespace NSABUtils
{
    class SABUTILS_EXPORT CWidgetEnabler : public QObject
    {
        Q_OBJECT;
        CWidgetEnabler( const std::pair< QCheckBox *, QGroupBox * > & checker,  const std::initializer_list< QWidget * > & widgets, QObject * parent = nullptr );
    public:
        CWidgetEnabler( QCheckBox * btn, QWidget * widget, QObject * parent = nullptr );
        CWidgetEnabler( QCheckBox * btn, const std::initializer_list< QWidget * > & widgets, QObject * parent = nullptr );

        CWidgetEnabler( QGroupBox * gb, QWidget * widget, QObject * parent = nullptr );
        CWidgetEnabler( QGroupBox * gb, const std::initializer_list< QWidget * > & widgets, QObject * parent = nullptr );

        void setEnableOnPartial( bool value ) { fEnableOnPartial = value; };
        bool enableOnPartial() const { return fEnableOnPartial; }
    public Q_SLOTS:
        void slotReset();

    private Q_SLOTS:
        void slotCheckStateChanged( int newState );
        void slotSetWidgetsEnabled( bool clicked );
    private:
        Qt::CheckState checkState() const;
        Qt::CheckState fInitState{ Qt::CheckState::Unchecked };
        bool fEnableOnPartial{ false };
        std::pair< QCheckBox *, QGroupBox * > fChecker{ nullptr, nullptr };
        std::list< QWidget * > fWidgets;
    };
}
#endif

