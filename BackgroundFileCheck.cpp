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
        stop();
    }

    void CBackgroundFileCheck::checkPath()
    {
        dumpDebug( "Check Path Started: " );;
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
        dumpDebug( "checkPathInternal Started: " );
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

        dumpDebug( "checkPathInternal exists: " );
        auto fi = QFileInfo( fPathName );
        if ( canContinue() && checkExists() && !fi.exists() )
            return std::make_pair( false, tr( "Path does not exist" ) );

        dumpDebug( "checkPathInternal isDir: " );
        if ( canContinue() && checkIsDir() && !fi.isDir() )
            return std::make_pair( false, tr( "Pathname is not a directory" ) );

        dumpDebug( "checkPathInternal isFile: " );
        if ( canContinue() && checkIsFile() && !fi.isFile() )
            return std::make_pair( false, tr( "Pathname is not a file" ) );

        dumpDebug( "checkPathInternal bundle: " );
        if ( canContinue() && checkIsBundle() && !fi.isBundle() )
            return std::make_pair( false, tr( "Pathname is not a bundle" ) );


        dumpDebug( "checkPathInternal isExe: " );
        if ( canContinue() && checkIsExecutable() && !fi.isExecutable() )
            return std::make_pair( false, tr( "Pathname is not executable (or have execute permissions)" ) );

        dumpDebug( "checkPathInternal isHidden: " );
        if ( canContinue() && checkIsHidden() && !fi.isHidden() )
            return std::make_pair( false, tr( "Pathname is not hidden" ) );

        dumpDebug( "checkPathInternal isJunction: " );
        if ( canContinue() && checkIsJunction() && !fi.isJunction() )
            return std::make_pair( false, tr( "Pathname is not a junction" ) );

        dumpDebug( "checkPathInternal isReadable: " );
        if ( canContinue() && checkIsReadable() && !fi.isReadable() )
            return std::make_pair( false, tr( "Pathname is not readable" ) );

        dumpDebug( "checkPathInternal isShortcut: " );
        if ( canContinue() && checkIsShortcut() && !fi.isShortcut() )
            return std::make_pair( false, tr( "Pathname is not a shortcut" ) );

        dumpDebug( "checkPathInternal isSymlink: " );
        if ( canContinue() && checkIsSymLink() && !fi.isSymLink() )
            return std::make_pair( false, tr( "Pathname is not a symbolic link or shortcut" ) );

        dumpDebug( "checkPathInternal isSymboliclink: " );
        if ( canContinue() && checkIsSymbolicLink() && !fi.isSymbolicLink() )
            return std::make_pair( false, tr( "Pathname is not a symbolic link" ) );

        dumpDebug( "checkPathInternal isWritable: " );
        if ( canContinue() && checkIsWritable() && !fi.isWritable() )
            return std::make_pair( false, tr( "Pathname is not writeable" ) );

        dumpDebug( "checkPathInternal checkPerms: " );
        if ( canContinue() && (checkPermissions() != 0) && !fi.permission( fCheckPermissions ) )
            return std::make_pair( false, tr( "Pathname does not have the proper permissions" ) );

        if ( useNTFSPermissions() )
        {
            qt_ntfs_permission_lookup--;
        }
        dumpDebug( "checkPathInternal finished: " );
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
        dumpDebug( "slotTimeout: " );
        fTimedOut = true;
        if ( fResultWatcher->isRunning() || fResultWatcher->isPaused() )
            fResultWatcher->cancel();
    }

    void CBackgroundFileCheck::stop()
    {
        dumpDebug( "stop: " );
        fStopped = true;
        if ( fResultWatcher->isRunning() || fResultWatcher->isPaused() )
            fResultWatcher->cancel();
        dumpDebug( "end stop: " );
    }
#ifdef _DEBUG
//#define DEBUG_BACKGROUND_FILE_CHECK
#endif

    void CBackgroundFileCheck::dumpDebug( const QString & msg ) const
    {
#ifdef DEBUG_BACKGROUND_FILE_CHECK
        static std::map< const QObject *, int > hitCounts;
        hitCounts[this]++;
        qDebug() << this << hitCounts[this] << msg << fPathName << fStopped << fTimedOut << fResultWatcher->isRunning() << fResultWatcher->isPaused();
#else
        (void)msg;
#endif
    }

}

