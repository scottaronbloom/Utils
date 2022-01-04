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

#include "DelayLineEdit.h"
#include "BackgroundFileCheck.h"

#include <QTimer>
#include <QKeyEvent>
#include <QThreadPool>
#include "QtUtils.h"

namespace NSABUtils
{
    CDelayLineEdit::CDelayLineEdit(QWidget* parent) :
        CDelayLineEdit(QString(), 250, parent)
    {
    }

    CDelayLineEdit::CDelayLineEdit(const QString & text, QWidget* parent) :
        CDelayLineEdit(text, 250, parent)
    {
    }

    CDelayLineEdit::CDelayLineEdit(const QString& text, int delayMS, QWidget* parent) :
        QLineEdit(text, parent),
        fDelayMS(delayMS)
    {
        connectToEditor(true);

        fChangedTimer = new QTimer(this);
        fChangedTimer->setSingleShot(true);
        connect(fChangedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotChangedTimerTimeout);

        fEditedTimer = new QTimer(this);
        fEditedTimer->setSingleShot(true);
        connect(fEditedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotEditTimerTimeout);

        setDelay(delayMS);
    }

    void CDelayLineEdit::setText(const QString &text)
    {
        if (!isVisible())
        {
            connectToEditor(false);
        }

        QLineEdit::setText(text);

        if (!isVisible())
        {
            connectToEditor(true);
        }
    }

    void CDelayLineEdit::setDelay(int delayMS)
    {
        fDelayMS = delayMS;
        NSABUtils::updateTimer(fDelayMS, fChangedTimer);
        NSABUtils::updateTimer(fDelayMS, fEditedTimer);
    }

    CDelayLineEdit::~CDelayLineEdit()
    {
    }

    void CDelayLineEdit::slotTextChanged()
    {
        fChangedTimer->stop();
        fChangedTimer->start();
        setLineEditColor(ELineEditStatus::ePending);
    }

    void CDelayLineEdit::slotTextEdited()
    {
        fEditingFinished = false;
        fEditedTimer->stop();
        fEditedTimer->start();
        setLineEditColor(ELineEditStatus::ePending);
    }

    void CDelayLineEdit::keyPressEvent(QKeyEvent * event)
    {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            fEditingFinished = true;
            if (fStatus == ELineEditStatus::eOK)
                emit sigFinishedEditingAfterDelay( text() );
        }
        QLineEdit::keyPressEvent(event);
    }

    void CDelayLineEdit::slotChangedTimerTimeout()
    {
        changeTimeout( fIsOK.first ? fIsOK.first( text() ) : true );
    }

    void CDelayLineEdit::changeTimeout( bool aOK )
    {
        setLineEditColor( aOK );
        emit sigTextChangedAfterDelay(text());
    }

    void CDelayLineEdit::slotEditTimerTimeout()
    {
        editTimeout( fIsOK.first ? fIsOK.first( text() ) : true );
    }

    void CDelayLineEdit::editTimeout( bool aOK )
    {
        setLineEditColor( aOK );
        emit sigTextEditedAfterDelay( text() );
        if ( fEditingFinished )
            emit sigFinishedEditingAfterDelay( text() );
    }

    void CDelayLineEdit::connectToEditor(bool connectOrDisconnect)
    {
        if (connectOrDisconnect)
        {
            connect(this, &QLineEdit::textChanged, this, &CDelayLineEdit::slotTextChanged);
            connect(this, &QLineEdit::textEdited, this, &CDelayLineEdit::slotTextEdited);
        }
        else
        {
            disconnect(this, &QLineEdit::textChanged, this, &CDelayLineEdit::slotTextChanged);
            disconnect(this, &QLineEdit::textEdited, this, &CDelayLineEdit::slotTextEdited);
        }
    }

    void CDelayLineEdit::setLineEditColor(bool aOK)
    {
        setLineEditColor(aOK ? ELineEditStatus::eOK : ELineEditStatus::eNotOK);
    }

    void CDelayLineEdit::setLineEditColor(ELineEditStatus status)
    {
        fStatus = status;
        setToolTip(QString());
        if (status == ELineEditStatus::ePending)
            setStyleSheet("QLineEdit { background-color: #b7bfaf }");
        else if (status == ELineEditStatus::eOK)
            setStyleSheet("QLineEdit { background-color: white }");
        else if (status == ELineEditStatus::eNotOK)
        {
            auto errorMsg = fIsOK.second;
            if (!errorMsg.isEmpty())
            {
                if (errorMsg.contains("%1"))
                    errorMsg = errorMsg.arg(text());
                setToolTip(errorMsg);
            }
            setStyleSheet("QLineEdit { background-color: red }");
        }
    }

    CPathBasedDelayLineEdit::CPathBasedDelayLineEdit( QWidget * parent /*= nullptr */ ) :
        CDelayLineEdit( parent )
    {
        init();
    }

    CPathBasedDelayLineEdit::CPathBasedDelayLineEdit( const QString & text, QWidget * parent /*= nullptr */ ) :
        CDelayLineEdit( text, parent )
    {
        init();
    }

    CPathBasedDelayLineEdit::CPathBasedDelayLineEdit( const QString & text, int delayMS, QWidget * parent /*= nullptr */ ) :
        CDelayLineEdit( text, delayMS, parent )
    {
        init();
    }

    void CPathBasedDelayLineEdit::init()
    {
        fFileChecker = new CBackgroundFileCheck;
        connect( fFileChecker, &CBackgroundFileCheck::sigFinished, this, &CPathBasedDelayLineEdit::slotFileCheckFinished );
    
        disconnect( fChangedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotChangedTimerTimeout );
        disconnect( fEditedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotEditTimerTimeout );

        connect( fChangedTimer, &QTimer::timeout, this, &CPathBasedDelayLineEdit::slotChangedTimerTimeout );
        connect( fEditedTimer, &QTimer::timeout, this, &CPathBasedDelayLineEdit::slotEditTimerTimeout );
    }

    CPathBasedDelayLineEdit::~CPathBasedDelayLineEdit()
    {
        delete fFileChecker;
    }

    void CPathBasedDelayLineEdit::setDelay( int delayMS )
    {
        fFileChecker->setTimeOut( delayMS / 2 );
        CDelayLineEdit::setDelay( delayMS );
    }

    bool CPathBasedDelayLineEdit::checkExists() const
    {
        return fFileChecker->checkExists();
    }

    void CPathBasedDelayLineEdit::setCheckExists( bool val )
    {
        fFileChecker->setCheckExists( val );
    }

    bool CPathBasedDelayLineEdit::checkIsBundle() const
    {
        return fFileChecker->checkIsBundle();
    }

    void CPathBasedDelayLineEdit::setCheckIsBundle( bool val )
    {
        fFileChecker->setCheckIsBundle( val );
    }

    bool CPathBasedDelayLineEdit::checkIsDir() const
    {
        return fFileChecker->checkIsDir();
    }

    void CPathBasedDelayLineEdit::setCheckIsDir( bool val )
    {
        fFileChecker->setCheckIsDir( val );
    }

    bool CPathBasedDelayLineEdit::checkIsExecutable() const
    {
        return fFileChecker->checkIsExecutable();
    }

    void CPathBasedDelayLineEdit::setCheckIsExecutable( bool val )
    {
        fFileChecker->setCheckIsExecutable( val );
    }

    bool CPathBasedDelayLineEdit::checkIsFile() const
    {
        return fFileChecker->checkIsFile();
    }

    void CPathBasedDelayLineEdit::setCheckIsFile( bool val )
    {
        fFileChecker->setCheckIsFile( val );
    }

    bool CPathBasedDelayLineEdit::checkIsHidden() const
    {
        return fFileChecker->checkIsHidden();
    }

    void CPathBasedDelayLineEdit::setCheckIsHidden( bool val )
    {
        fFileChecker->setCheckIsHidden( val );
    }

    bool CPathBasedDelayLineEdit::checkIsJunction() const
    {
        return fFileChecker->checkIsJunction();
    }

    void CPathBasedDelayLineEdit::setCheckIsJunction( bool val )
    {
        fFileChecker->setCheckIsJunction( val );
    }

    bool CPathBasedDelayLineEdit::checkIsReadable() const
    {
        return fFileChecker->checkIsReadable();
    }

    void CPathBasedDelayLineEdit::setCheckIsReadable( bool val )
    {
        fFileChecker->setCheckIsReadable( val );
    }

    bool CPathBasedDelayLineEdit::checkIsShortcut() const
    {
        return fFileChecker->checkIsShortcut();
    }

    void CPathBasedDelayLineEdit::setCheckIsShortcut( bool val )
    {
        fFileChecker->setCheckIsShortcut( val );
    }

    bool CPathBasedDelayLineEdit::checkIsSymLink() const
    {
        return fFileChecker->checkIsSymLink();
    }

    void CPathBasedDelayLineEdit::setCheckIsSymLink( bool val )
    {
        fFileChecker->setCheckIsSymLink( val );
    }

    bool CPathBasedDelayLineEdit::checkIsSymbolicLink() const
    {
        return fFileChecker->checkIsSymbolicLink();
    }

    void CPathBasedDelayLineEdit::setCheckIsSymbolicLink( bool val )
    {
        fFileChecker->setCheckIsSymbolicLink( val );
    }

    bool CPathBasedDelayLineEdit::checkIsWritable() const
    {
        return fFileChecker->checkIsWritable();
    }

    void CPathBasedDelayLineEdit::setCheckIsWritable( bool val )
    {
        fFileChecker->setCheckIsWritable( val );
    }

    QFile::Permissions CPathBasedDelayLineEdit::checkPermissions() const
    {
        return fFileChecker->checkPermissions();
    }

    void CPathBasedDelayLineEdit::setCheckPermissions( QFile::Permissions val )
    {
        fFileChecker->setCheckPermissions( val );
    }

    bool CPathBasedDelayLineEdit::useNTFSPermissions() const
    {
        return fFileChecker->useNTFSPermissions();
    }

    void CPathBasedDelayLineEdit::setUseNTFSPermissions( bool val )
    {
        fFileChecker->setUseNTFSPermissions( val );
    }

    void CPathBasedDelayLineEdit::slotChangedTimerTimeout()
    {
        fChanged = true;
        fFileChecker->checkPath( text() );
    }

    void CPathBasedDelayLineEdit::slotEditTimerTimeout()
    {
        fEdited = true;
        fFileChecker->checkPath( text() );
    }

    void CPathBasedDelayLineEdit::slotFileCheckFinished( bool aOK )
    {
        fMessage = fFileChecker->msg();

        if ( fEdited )
        {
            editTimeout( aOK );
            fEdited = false;
        }

        if ( fChanged )
        {
            changeTimeout( aOK );
            fChanged = false;
        }
    }
}

