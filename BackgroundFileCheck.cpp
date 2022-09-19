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
#include "RevertValue.h"

#include "SABUtils/FileUtils.h"

#include <QThread>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>
#include <optional>

#ifdef Q_OS_WINDOWS
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

#ifdef _DEBUG
//#define DEBUG_BACKGROUND_FILE_CHECK
#endif

namespace NSABUtils
{

    class CBackgroundFileCheckImpl
    {
    public:
        explicit CBackgroundFileCheckImpl( CBackgroundFileCheck * parent );

        ~CBackgroundFileCheckImpl()
        {
            fTimer->stop();
            fThread->terminate();
            delete fThread;
        }

        void dumpDebug( const QString & msg ) const;

        bool canContinue() const
        {
            return !fStopped && !fTimedOut;
        }

        void stop( std::optional< bool > stopped, std::optional< bool > timedOut )
        {
            fStopped = stopped.has_value() ? stopped.value() : fStopped;
            fTimedOut = timedOut.has_value() ? timedOut.value() : fTimedOut;
            fTimer->stop();
            if ( fThread->isRunning() )
            {
                reinitThread();
            }
            if ( fStopped )
                fRetVal = std::make_pair( false, QString( "File Checking Stopped" ) );
            if ( fTimedOut )
                fRetVal = std::make_pair( false, QString( "Path checking timed out, check network connection" ) );
        }

        void initThread()
        {
            fThread = new CBackgroundFileCheckThread( this );
            QObject::connect(fThread, &CBackgroundFileCheckThread::finished, fParent, &CBackgroundFileCheck::slotFinished);
        }

        void reinitThread()
        {
            QObject::disconnect(fThread, &CBackgroundFileCheckThread::finished, fParent, &CBackgroundFileCheck::slotFinished);
            fThread->terminate();
            fThread->deleteLater();
            fThread = nullptr;
            initThread();
        }

        int fTimeOut{ 5000 };
        bool fStopped{ false };
        bool fTimedOut{ false };
        QString fPathName;

        bool fCheckExists{ true };
        bool fCheckIsBundle{ false };
        bool fCheckIsDir{ false };
        bool fCheckIsExecutable{ false };
        bool fCheckIsFile{ false };
        bool fCheckIsHidden{ false };
#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        bool fCheckIsJunction{ false };
#endif
        bool fCheckIsReadable{ false };
#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        bool fCheckIsShortcut{ false };
#endif
        bool fCheckIsSymLink{ false };
#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        bool fCheckIsSymbolicLink{ false };
#endif
        bool fCheckIsWritable{ false };
        QFile::Permissions fCheckPermissions;
        bool fUseNTFSPermissions{
#ifdef Q_OS_WINDOWS
        true
#else
        false
#endif
        };

        std::pair< bool, QString > fRetVal;
        CBackgroundFileCheck * fParent{ nullptr };
        CBackgroundFileCheckThread * fThread{ nullptr };
        QTimer * fTimer{ nullptr };
    };

    CBackgroundFileCheckThread::CBackgroundFileCheckThread( CBackgroundFileCheckImpl * impl ) :
        QThread( nullptr ),
        fImpl( impl )
    {
    }

    void CBackgroundFileCheckThread::run()
    {
        fImpl->fRetVal = std::make_pair( false, QString() );
        fImpl->dumpDebug( "checkPathInternal Started: " );
        fImpl->fStopped = false;
        fImpl->fTimedOut = false;
        if ( fImpl->fPathName.isEmpty() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is empty" ) );
            return;
        }

        if ( NSABUtils::NFileUtils::isIPAddressNetworkPath( fImpl->fPathName ) )
        {
            fImpl->fRetVal = std::make_pair( true, QString() );
            return;
        }

#ifdef Q_OS_WINDOWS
        CRevertValue revertValue( qt_ntfs_permission_lookup );
        if ( fImpl->fUseNTFSPermissions )
        {
            qt_ntfs_permission_lookup++;
        }
#endif

        fImpl->dumpDebug( "checkPathInternal exists: " );
        auto fi = QFileInfo( fImpl->fPathName );
        if ( fImpl->canContinue() && fImpl->fCheckExists && !fi.exists() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Path does not exist" ) );
            return;
        }

        fImpl->dumpDebug( "checkPathInternal isDir: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsDir && !fi.isDir() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not a directory" ) );
            return;
        }

        fImpl->dumpDebug( "checkPathInternal isFile: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsFile && !fi.isFile() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not a file" ) );
            return;
        }

        fImpl->dumpDebug( "checkPathInternal bundle: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsBundle && !fi.isBundle() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not a bundle" ) );
            return;
        }


        fImpl->dumpDebug( "checkPathInternal isExe: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsExecutable && !fi.isExecutable() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not executable (or have execute permissions)" ) );
            return;
        }

        fImpl->dumpDebug( "checkPathInternal isHidden: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsHidden && !fi.isHidden() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not hidden" ) );
            return;
        }

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        fImpl->dumpDebug( "checkPathInternal isJunction: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsJunction && !fi.isJunction() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not a junction" ) );
            return;
        }
#endif

        fImpl->dumpDebug( "checkPathInternal isReadable: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsReadable && !fi.isReadable() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not readable" ) );
            return;
        }

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        fImpl->dumpDebug( "checkPathInternal isShortcut: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsShortcut && !fi.isShortcut() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not a shortcut" ) );
            return;
        }
#endif
        fImpl->dumpDebug( "checkPathInternal isSymlink: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsSymLink && !fi.isSymLink() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not a symbolic link or shortcut" ) );
            return;
        }

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        fImpl->dumpDebug( "checkPathInternal isSymboliclink: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsSymbolicLink && !fi.isSymbolicLink() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not a symbolic link" ) );
            return;
        }
#endif
        fImpl->dumpDebug( "checkPathInternal isWritable: " );
        if ( fImpl->canContinue() && fImpl->fCheckIsWritable && !fi.isWritable() )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname is not writeable" ) );
            return;
        }

        fImpl->dumpDebug( "checkPathInternal checkPerms: " );
        if ( fImpl->canContinue() && (fImpl->fCheckPermissions != 0) && !fi.permission( fImpl->fCheckPermissions ) )
        {
            fImpl->fRetVal = std::make_pair( false, tr( "Pathname does not have the proper permissions" ) );
            return;
        }

        fImpl->dumpDebug( "checkPathInternal finished: " );
        if ( fImpl->fStopped )
        {
            fImpl->fRetVal = std::make_pair( false, QString( "Background Checking Stopped" ) );
            return;
        }
        if ( fImpl->fTimedOut )
        {
            fImpl->fRetVal = std::make_pair( false, QString( "Background Checking timed out" ) );
            return;
        }

        fImpl->fRetVal = std::make_pair( true, QString() );
    }

    void CBackgroundFileCheckImpl::dumpDebug( const QString & msg ) const
    {
#ifdef DEBUG_BACKGROUND_FILE_CHECK
        static std::unordered_map< const CBackgroundFileCheckImpl *, int > hitCounts;
        hitCounts[this]++;
        qDebug() << this << hitCounts[this] << msg << fPathName << fStopped << fTimedOut << fThread->isRunning();
#else
        (void)msg;
#endif
    }

    CBackgroundFileCheckImpl::CBackgroundFileCheckImpl( CBackgroundFileCheck * parent )
    {
        fParent = parent;
        initThread();

        fTimer = new QTimer( parent );
        fTimer->setInterval( fTimeOut );
        fTimer->setSingleShot( true );
        QObject::connect( fTimer, &QTimer::timeout, parent, &CBackgroundFileCheck::slotTimeout );
    }

    CBackgroundFileCheck::CBackgroundFileCheck( QObject * parent ) :
        QObject( parent )
    {
        fImpl = new CBackgroundFileCheckImpl( this );
    }

    CBackgroundFileCheck::~CBackgroundFileCheck()
    {
        stop( false );
        delete fImpl;
    }

    void CBackgroundFileCheck::checkPath( const QString & pathName )
    {
        setPathName( pathName );
        return checkPath();
    }

    int CBackgroundFileCheck::timeOut() const
    {
        return fImpl ->fTimeOut;
    }

    void CBackgroundFileCheck::setTimeOut( int val )
    {
        fImpl->fTimeOut = val;
        bool isRunning = fImpl->fTimer->isActive();
        fImpl->fTimer->stop();
        fImpl->fTimer->setInterval( val );
        if ( isRunning )
            fImpl->fTimer->start();
    }

    QString CBackgroundFileCheck::pathName() const
    {
        return fImpl->fPathName;
    }

    void CBackgroundFileCheck::setPathName( const QString & pathName )
    {
        fImpl->fPathName = pathName;
        stop( false );
    }

    bool CBackgroundFileCheck::checkExists() const
    {
        return fImpl->fCheckExists;
    }

    void CBackgroundFileCheck::setCheckExists( bool val )
    {
        fImpl->fCheckExists = val;
    }

    bool CBackgroundFileCheck::checkIsBundle() const
    {
        return fImpl->fCheckIsBundle;
    }

    void CBackgroundFileCheck::setCheckIsBundle( bool val )
    {
        fImpl->fCheckIsBundle = val;
    }

    bool CBackgroundFileCheck::checkIsDir() const
    {
        return fImpl->fCheckIsDir;
    }

    void CBackgroundFileCheck::setCheckIsDir( bool val )
    {
        fImpl->fCheckIsDir = val;
    }

    bool CBackgroundFileCheck::checkIsExecutable() const
    {
        return fImpl->fCheckIsExecutable;
    }

    void CBackgroundFileCheck::setCheckIsExecutable( bool val )
    {
        fImpl->fCheckIsExecutable = val;
    }

    bool CBackgroundFileCheck::checkIsFile() const
    {
        return fImpl->fCheckIsFile;
    }

    void CBackgroundFileCheck::setCheckIsFile( bool val )
    {
        fImpl->fCheckIsFile = val;
    }

    bool CBackgroundFileCheck::checkIsHidden() const
    {
        return fImpl->fCheckIsHidden;
    }

    void CBackgroundFileCheck::setCheckIsHidden( bool val )
    {
        fImpl->fCheckIsHidden = val;
    }

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
    bool CBackgroundFileCheck::checkIsJunction() const
    {
        return fImpl->fCheckIsJunction;
    }

    void CBackgroundFileCheck::setCheckIsJunction( bool val )
    {
        fImpl->fCheckIsJunction = val;
    }
#endif

    bool CBackgroundFileCheck::checkIsReadable() const
    {
        return fImpl->fCheckIsReadable;
    }

    void CBackgroundFileCheck::setCheckIsReadable( bool val )
    {
        fImpl->fCheckIsReadable = val;
    }

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
    bool CBackgroundFileCheck::checkIsShortcut() const
    {
        return fImpl->fCheckIsShortcut;
    }

    void CBackgroundFileCheck::setCheckIsShortcut( bool val )
    {
        fImpl->fCheckIsShortcut = val;
    }
#endif

    bool CBackgroundFileCheck::checkIsSymLink() const
    {
        return fImpl->fCheckIsSymLink;
    }

    void CBackgroundFileCheck::setCheckIsSymLink( bool val )
    {
        fImpl->fCheckIsSymLink = val;
    }

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
    bool CBackgroundFileCheck::checkIsSymbolicLink() const
    {
        return fImpl->fCheckIsSymbolicLink;
    }

    void CBackgroundFileCheck::setCheckIsSymbolicLink( bool val )
    {
        fImpl->fCheckIsSymbolicLink = val;
    }
#endif

    bool CBackgroundFileCheck::checkIsWritable() const
    {
        return fImpl->fCheckIsWritable;
    }

    void CBackgroundFileCheck::setCheckIsWritable( bool val )
    {
        fImpl->fCheckIsWritable = val;
    }

    QFile::Permissions CBackgroundFileCheck::checkPermissions() const
    {
        return fImpl->fCheckPermissions;
    }

    void CBackgroundFileCheck::setCheckPermissions( QFile::Permissions val )
    {
        fImpl->fCheckPermissions = val;
    }

    bool CBackgroundFileCheck::useNTFSPermissions() const
    {
        return fImpl->fUseNTFSPermissions;
    }

    void CBackgroundFileCheck::setUseNTFSPermissions( bool val )
    {
        fImpl->fUseNTFSPermissions = val;
    }

    QString CBackgroundFileCheck::msg() const
    {
        return fImpl->fRetVal.second;
    }

    void CBackgroundFileCheck::checkPath()
    {
        fImpl->fTimer->start();
        fImpl->fThread->start();
    }

    void CBackgroundFileCheck::slotTimeout()
    {
        fImpl->dumpDebug( "start timeout: " );
        fImpl->stop( {}, true );
        fImpl->dumpDebug( "end timeout: " );
        slotFinished();
    }

    void CBackgroundFileCheck::slotFinished()
    {
        fImpl->stop( {}, {} ); // stops the timer only,to prevent a timeout later
        emit sigFinished( fImpl->fRetVal.first, fImpl->fRetVal.second );
    }

    void CBackgroundFileCheck::stop( bool markAsStopped )
    {
        fImpl->dumpDebug( "stop: " );
        if ( markAsStopped )
            fImpl->stop( true, {} );
        else
            fImpl->stop( {}, {} );
        fImpl->dumpDebug( "end stop: " );
    }
}

