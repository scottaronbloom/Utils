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


#include "BackgroundFileCheck.h"
#include <QThread>
#include <QFileInfo>
#include <QFuture>
#include <QtConcurrent>

extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;

namespace NSABUtils
{
    CBackgroundFileCheck::CBackgroundFileCheck( QObject * parent ) :
        QObject( parent )
    {
        fResultWatcher = new QFutureWatcher< std::pair< bool, QString > >( this );
        connect( fResultWatcher, &QFutureWatcher< std::pair< bool, QString > >::resultReadyAt, this, &CBackgroundFileCheck::slotResultsReady );
        connect( fResultWatcher, &QFutureWatcher< std::pair< bool, QString > >::canceled, this, &CBackgroundFileCheck::slotCanceled );
    }

    CBackgroundFileCheck::~CBackgroundFileCheck()
    {
        stop();
    }

    void CBackgroundFileCheck::checkPath( const QString & pathName )
    {
        setPathName( pathName );
        return checkPath();
    }

    void CBackgroundFileCheck::setPathName( const QString & pathName )
    {
        fPathName = pathName;
    }

    void CBackgroundFileCheck::checkPath()
    {
        auto future = QtConcurrent::run( this, &CBackgroundFileCheck::checkPathInternal );
        fResultWatcher->setFuture( future );
        QTimer::singleShot( timeOut(), this, &CBackgroundFileCheck::slotTimeout );
    }

    void CBackgroundFileCheck::slotCanceled()
    {
        if ( fStopped )
            emit sigFinished( false, QString( "Background Checking Stopped" ) );
        if ( fTimedOut )
            emit sigFinished( false, QString( "Background Checking timed out" ) );
    }

    void CBackgroundFileCheck::slotResultsReady( int /*index*/ )
    {
        auto result = fResultWatcher->result();
        emit sigFinished( result.first, result.second );
    }

    std::pair< bool, QString > CBackgroundFileCheck::checkPathInternal()
    {
        fStopped = false;
        fTimedOut = false;
        if ( fPathName.isEmpty() )
        {
            return std::make_pair( false, tr( "Pathname is empty" ) );
        }

        if ( useNTFSPermissions() )
        {
            qt_ntfs_permission_lookup++;
        }

        auto fi = QFileInfo( fPathName );
        if ( canContinue() && checkExists() && !fi.exists() )
            return std::make_pair( false, tr( "Path does not exist" ) );

        if ( canContinue() && checkIsBundle() && !fi.isBundle() )
            return std::make_pair( false, tr( "Pathname is not a bundle" ) );

        if ( canContinue() && checkIsDir() && !fi.isDir() )
            return std::make_pair( false, tr( "Pathname is not a directory" ) );

        if ( canContinue() && checkIsExecutable() && !fi.isExecutable() )
            return std::make_pair( false, tr( "Pathname is not executable (or have execute permissions)" ) );

        if ( canContinue() && checkIsFile() && !fi.isFile() )
            return std::make_pair( false, tr( "Pathname is not a file" ) );

        if ( canContinue() && checkIsHidden() && !fi.isHidden() )
            return std::make_pair( false, tr( "Pathname is not hidden" ) );

        if ( canContinue() && checkIsJunction() && !fi.isJunction() )
            return std::make_pair( false, tr( "Pathname is not a junction" ) );

        if ( canContinue() && checkIsReadable() && !fi.isReadable() )
            return std::make_pair( false, tr( "Pathname is not readable" ) );

        if ( canContinue() && checkIsShortcut() && !fi.isShortcut() )
            return std::make_pair( false, tr( "Pathname is not a shortcut" ) );

        if ( canContinue() && checkIsSymLink() && !fi.isSymLink() )
            return std::make_pair( false, tr( "Pathname is not a symbolic link or shortcut" ) );

        if ( canContinue() && checkIsSymbolicLink() && !fi.isSymbolicLink() )
            return std::make_pair( false, tr( "Pathname is not a symbolic link" ) );

        if ( canContinue() && checkIsWritable() && !fi.isWritable() )
            return std::make_pair( false, tr( "Pathname is not writeable" ) );

        if ( canContinue() && (checkPermissions() != 0) && !fi.permission( fCheckPermissions ) )
            return std::make_pair( false, tr( "Pathname does not have the proper permissions" ) );

        if ( useNTFSPermissions() )
        {
            qt_ntfs_permission_lookup--;
        }
        if ( fStopped )
            return std::make_pair( false, QString( "Background Checking Stopped" ) );
        if ( fTimedOut )
            return std::make_pair( false, QString( "Background Checking timed out" ) );

        return std::make_pair( true, QString() );
    }

    bool CBackgroundFileCheck::canContinue() const
    {
        return !fStopped && !fTimedOut;
    }

    void CBackgroundFileCheck::slotTimeout()
    {
        fTimedOut = true;
        if ( fResultWatcher->isRunning() || fResultWatcher->isPaused() )
            fResultWatcher->cancel();
    }

    void CBackgroundFileCheck::stop()
    {
        fStopped = true;
        if ( fResultWatcher->isRunning() || fResultWatcher->isPaused() )
            fResultWatcher->cancel();
    }
}

