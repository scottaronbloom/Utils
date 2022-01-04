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
                emit sigFinishedEditingAfterDelay();
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
            emit sigFinishedEditingAfterDelay();
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

    CFileBasedDelayLineEdit::CFileBasedDelayLineEdit( QWidget * parent /*= nullptr */ ) :
        CDelayLineEdit( parent )
    {
        init();
    }

    CFileBasedDelayLineEdit::CFileBasedDelayLineEdit( const QString & text, QWidget * parent /*= nullptr */ ) :
        CDelayLineEdit( text, parent )
    {
        init();
    }

    CFileBasedDelayLineEdit::CFileBasedDelayLineEdit( const QString & text, int delayMS, QWidget * parent /*= nullptr */ ) :
        CDelayLineEdit( text, delayMS, parent )
    {
        init();
    }

    void CFileBasedDelayLineEdit::init()
    {
        fFileChecker = new CBackgroundFileCheck;
        connect( fFileChecker, &CBackgroundFileCheck::sigFinished, this, &CFileBasedDelayLineEdit::slotFileCheckFinished );
    
        disconnect( fChangedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotChangedTimerTimeout );
        disconnect( fEditedTimer, &QTimer::timeout, this, &CDelayLineEdit::slotEditTimerTimeout );

        connect( fChangedTimer, &QTimer::timeout, this, &CFileBasedDelayLineEdit::slotChangedTimerTimeout );
        connect( fEditedTimer, &QTimer::timeout, this, &CFileBasedDelayLineEdit::slotEditTimerTimeout );
    }

    CFileBasedDelayLineEdit::~CFileBasedDelayLineEdit()
    {
        delete fFileChecker;
    }

    void CFileBasedDelayLineEdit::setDelay( int delayMS )
    {
        fFileChecker->setTimeOut( delayMS / 2 );
        CDelayLineEdit::setDelay( delayMS );
    }

    bool CFileBasedDelayLineEdit::checkExists() const
    {
        return fFileChecker->checkExists();
    }

    void CFileBasedDelayLineEdit::setCheckExists( bool val )
    {
        fFileChecker->setCheckExists( val );
    }

    bool CFileBasedDelayLineEdit::checkIsBundle() const
    {
        return fFileChecker->checkIsBundle();
    }

    void CFileBasedDelayLineEdit::setCheckIsBundle( bool val )
    {
        fFileChecker->setCheckIsBundle( val );
    }

    bool CFileBasedDelayLineEdit::checkIsDir() const
    {
        return fFileChecker->checkIsDir();
    }

    void CFileBasedDelayLineEdit::setCheckIsDir( bool val )
    {
        fFileChecker->setCheckIsDir( val );
    }

    bool CFileBasedDelayLineEdit::checkIsExecutable() const
    {
        return fFileChecker->checkIsExecutable();
    }

    void CFileBasedDelayLineEdit::setCheckIsExecutable( bool val )
    {
        fFileChecker->setCheckIsExecutable( val );
    }

    bool CFileBasedDelayLineEdit::checkIsFile() const
    {
        return fFileChecker->checkIsFile();
    }

    void CFileBasedDelayLineEdit::setCheckIsFile( bool val )
    {
        fFileChecker->setCheckIsFile( val );
    }

    bool CFileBasedDelayLineEdit::checkIsHidden() const
    {
        return fFileChecker->checkIsHidden();
    }

    void CFileBasedDelayLineEdit::setCheckIsHidden( bool val )
    {
        fFileChecker->setCheckIsHidden( val );
    }

    bool CFileBasedDelayLineEdit::checkIsJunction() const
    {
        return fFileChecker->checkIsJunction();
    }

    void CFileBasedDelayLineEdit::setCheckIsJunction( bool val )
    {
        fFileChecker->setCheckIsJunction( val );
    }

    bool CFileBasedDelayLineEdit::checkIsReadable() const
    {
        return fFileChecker->checkIsReadable();
    }

    void CFileBasedDelayLineEdit::setCheckIsReadable( bool val )
    {
        fFileChecker->setCheckIsReadable( val );
    }

    bool CFileBasedDelayLineEdit::checkIsShortcut() const
    {
        return fFileChecker->checkIsShortcut();
    }

    void CFileBasedDelayLineEdit::setCheckIsShortcut( bool val )
    {
        fFileChecker->setCheckIsShortcut( val );
    }

    bool CFileBasedDelayLineEdit::checkIsSymLink() const
    {
        return fFileChecker->checkIsSymLink();
    }

    void CFileBasedDelayLineEdit::setCheckIsSymLink( bool val )
    {
        fFileChecker->setCheckIsSymLink( val );
    }

    bool CFileBasedDelayLineEdit::checkIsSymbolicLink() const
    {
        return fFileChecker->checkIsSymbolicLink();
    }

    void CFileBasedDelayLineEdit::setCheckIsSymbolicLink( bool val )
    {
        fFileChecker->setCheckIsSymbolicLink( val );
    }

    bool CFileBasedDelayLineEdit::checkIsWritable() const
    {
        return fFileChecker->checkIsWritable();
    }

    void CFileBasedDelayLineEdit::setCheckIsWritable( bool val )
    {
        fFileChecker->setCheckIsWritable( val );
    }

    QFile::Permissions CFileBasedDelayLineEdit::checkPermissions() const
    {
        return fFileChecker->checkPermissions();
    }

    void CFileBasedDelayLineEdit::setCheckPermissions( QFile::Permissions val )
    {
        fFileChecker->setCheckPermissions( val );
    }

    bool CFileBasedDelayLineEdit::useNTFSPermissions() const
    {
        return fFileChecker->useNTFSPermissions();
    }

    void CFileBasedDelayLineEdit::setUseNTFSPermissions( bool val )
    {
        fFileChecker->setUseNTFSPermissions( val );
    }

    void CFileBasedDelayLineEdit::slotChangedTimerTimeout()
    {
        fChanged = true;
        fFileChecker->checkPath( text() );
    }

    void CFileBasedDelayLineEdit::slotEditTimerTimeout()
    {
        fEdited = true;
        fFileChecker->checkPath( text() );
    }

    void CFileBasedDelayLineEdit::slotFileCheckFinished( bool aOK )
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

