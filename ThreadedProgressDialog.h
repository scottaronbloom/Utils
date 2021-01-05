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

#ifndef __THREADEDPROGRESSDIALOG_H
#define __THREADEDPROGRESSDIALOG_H

#include <QProgressDialog>
#include <QEventLoop>
#include <functional>
#include <memory>

namespace NSABUtils
{
using TVoidFunction = std::function< void() >;
class CThreadedProgressDialogImpl;
class CThreadedProgressDialog : public QProgressDialog
{
    Q_OBJECT;
public:
    CThreadedProgressDialog( TVoidFunction xFunc, const QString& xLabelText, const QString& xCancelButtonText, int xMinimum, int xMaximum, QWidget* xParent = nullptr, Qt::WindowFlags xFlags = Qt::WindowFlags() );
    CThreadedProgressDialog( TVoidFunction xFunc, QWidget* xParent = nullptr, Qt::WindowFlags xFlags = Qt::WindowFlags() );
    ~CThreadedProgressDialog();

    void setCancelButtonText( const QString& xText );
    QString cancelButtonText() const;

    void mSetHasCancel( bool xHasCancel );

    int exec() override;
private:
    std::unique_ptr< CThreadedProgressDialogImpl > dImpl;
};

class CThreadedEventLoopImpl;
class CThreadedEventLoop : public QEventLoop
{
    Q_OBJECT;
public:
    CThreadedEventLoop( TVoidFunction xFunc, QObject* xParent = nullptr );
    ~CThreadedEventLoop();

    int exec( QEventLoop::ProcessEventsFlags flags = AllEvents );
public Q_SLOTS:
    void mExit(); // calls exit();
private:
    std::unique_ptr< CThreadedEventLoopImpl > dImpl;
};

}
#endif
