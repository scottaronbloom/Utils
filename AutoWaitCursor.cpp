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

#include "AutoWaitCursor.h"
#include <QApplication>

namespace NSABUtils
{

    CAutoWaitCursor::CAutoWaitCursor( QObject *revertOnShowWidget ) :
        CAutoWaitCursor( revertOnShowWidget, Qt::WaitCursor )
    {
    }

    CAutoWaitCursor::CAutoWaitCursor( QObject *revertOnShowWidget, Qt::CursorShape cursorShape ) :
        QObject( nullptr ),
        fRestoreOnOpenWidget( revertOnShowWidget )
    {
        if ( !dynamic_cast< QApplication * >( QCoreApplication::instance() ) )
            return;

        QCursor *cursor = QApplication::overrideCursor();
        QApplication::setOverrideCursor( cursorShape );
        if ( !cursor )
            qApp->processEvents();
        if ( revertOnShowWidget )
        {
            revertOnShowWidget->installEventFilter( this );
        }
        qApp->processEvents();
    }

    bool CAutoWaitCursor::eventFilter( QObject *obj, QEvent *event )
    {
        if ( obj == fRestoreOnOpenWidget )
        {
            if ( event->type() == QEvent::Show )
            {
                restore();
            }
        }
        return QObject::eventFilter( obj, event );
    }

    CAutoWaitCursor::~CAutoWaitCursor()
    {
        restore();
    }

    void CAutoWaitCursor::restore()
    {
        if ( !dynamic_cast< QApplication * >( QCoreApplication::instance() ) )
            return;

        QApplication::restoreOverrideCursor();
        qApp->processEvents();
    }

    bool CAutoWaitCursor::active()
    {
        if ( !dynamic_cast< QApplication * >( QCoreApplication::instance() ) )
            return false;

        return QApplication::overrideCursor() != nullptr;
    }

}
