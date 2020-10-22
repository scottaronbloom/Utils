// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#include "ThreadedProgressDialog.h"
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QPushButton>
#include <QApplication>

namespace NSABUtils
{
class CThreadedProgressDialogImpl
{
public:
    CThreadedProgressDialogImpl( TVoidFunction xFunc, CThreadedProgressDialog* xParent ) :
        dFunction( xFunc ),
        dParent( xParent )
    {
    }

    void mRunIt()
    {
        dWatcher = new QFutureWatcher<void>( dParent );
        QObject::connect( dWatcher, &QFutureWatcher< void >::finished, dParent, &CThreadedProgressDialog::close );
        QObject::connect( dWatcher, &QFutureWatcher< void >::finished, dParent, &QFutureWatcher< void >::deleteLater );

        auto lFuture = QtConcurrent::run( QThreadPool::globalInstance(), dFunction );
        dWatcher->setFuture( lFuture );
    }
    QString dCancelButtonText{ QObject::tr( "&Cancel" ) };
    TVoidFunction dFunction;
    CThreadedProgressDialog* dParent{ nullptr };
    QFutureWatcher<void>* dWatcher{ nullptr };
};

CThreadedProgressDialog::CThreadedProgressDialog( TVoidFunction xFunc, const QString& xLabelText, const QString& xCancelButtonText, int xMinimum, int xMaximum, QWidget* xParent /*= nullptr*/, Qt::WindowFlags xFlags /*= Qt::WindowFlags() */ ) :
    QProgressDialog( xLabelText, xCancelButtonText, xMinimum, xMaximum, xParent, xFlags ),
    dImpl( new CThreadedProgressDialogImpl( xFunc, this ) )
{
    setWindowModality( Qt::WindowModal );
    mSetHasCancel( false );
}

CThreadedProgressDialog::CThreadedProgressDialog( TVoidFunction xFunc, QWidget* xParent /*= nullptr*/, Qt::WindowFlags xFlags /*= Qt::WindowFlags() */ ) :
    QProgressDialog( xParent, xFlags ),
    dImpl( new CThreadedProgressDialogImpl( xFunc, this ) )
{
    setWindowModality( Qt::WindowModal );
    mSetHasCancel( false );
    setMinimum( 0 );
    setMaximum( 0 );
}

CThreadedProgressDialog::~CThreadedProgressDialog()
{
}

void CThreadedProgressDialog::mSetHasCancel( bool xHasCancel )
{
    if ( xHasCancel )
        setCancelButton( new QPushButton( dImpl->dCancelButtonText ) );
    else
        setCancelButton( nullptr );
}

QString CThreadedProgressDialog::cancelButtonText() const
{
    return dImpl->dCancelButtonText;
}

void CThreadedProgressDialog::setCancelButtonText( const QString& xText )
{
    dImpl->dCancelButtonText = xText;
    QProgressDialog::setCancelButtonText( xText );
}

int CThreadedProgressDialog::exec()
{
    dImpl->mRunIt();
    return QProgressDialog::exec();
}

class CThreadedEventLoopImpl
{
public:
    CThreadedEventLoopImpl( TVoidFunction xFunc, CThreadedEventLoop* xParent ) :
        dFunction( xFunc ),
        dParent( xParent )
    {
    }

    void mRunIt()
    {
        dWatcher = new QFutureWatcher<void>( dParent );
        QObject::connect( dWatcher, &QFutureWatcher< void >::finished, dParent, &CThreadedEventLoop::mExit );
        QObject::connect( dWatcher, &QFutureWatcher< void >::finished, dParent, &QFutureWatcher< void >::deleteLater );

        auto lFuture = QtConcurrent::run( QThreadPool::globalInstance(), dFunction );
        dWatcher->setFuture( lFuture );
    }
    QString dCancelButtonText{ QObject::tr( "&Cancel" ) };
    TVoidFunction dFunction;
    CThreadedEventLoop* dParent{ nullptr };
    QFutureWatcher<void>* dWatcher{ nullptr };
};

CThreadedEventLoop::CThreadedEventLoop( TVoidFunction xFunc, QObject * xParent /*= nullptr*/ ) :
    QEventLoop( xParent ),
    dImpl( new CThreadedEventLoopImpl( xFunc, this ) )
{
}


CThreadedEventLoop::~CThreadedEventLoop()
{
}

void CThreadedEventLoop::mExit()
{
    return QEventLoop::exit();
}

int CThreadedEventLoop::exec( QEventLoop::ProcessEventsFlags flags )
{
    QApplication::setOverrideCursor( Qt::WaitCursor ); 
    dImpl->mRunIt();
    int lRetVal = QEventLoop::exec( flags );
    QApplication::restoreOverrideCursor();
    return lRetVal;
}
}

//