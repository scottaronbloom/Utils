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
#ifndef __BACKGROUNDFILECHECK_H
#define __BACKGROUNDFILECHECK_H
#include <QObject>
#include <QFile>

#include <QFutureWatcher>
#include <QFuture>

namespace NSABUtils
{
    class CBackgroundFileCheck  : public QObject
    {
        Q_OBJECT;
    public:
        CBackgroundFileCheck( QObject * parent = nullptr );
        ~CBackgroundFileCheck();
        
        void stop();

        void checkPath();
        void checkPath( const QString & pathName );
    Q_SIGNALS:
        void sigFinished( bool aOK, const QString & msg );

    public:
        int timeOut() const { return fTimeOut; }
        void setTimeOut( int val ) { fTimeOut = val; }

        QString pathName() const { return fPathName; }
        void setPathName( const QString & pathName );

        bool checkExists() const { return fCheckExists; }
        void setCheckExists( bool val ) { fCheckExists = val; }

        bool checkIsBundle() const { return fCheckIsBundle; }
        void setCheckIsBundle( bool val ) { fCheckIsBundle = val; }

        bool checkIsDir() const { return fCheckIsDir; }
        void setCheckIsDir( bool val ) { fCheckIsDir = val; }

        bool checkIsExecutable() const { return fCheckIsExecutable; }
        void setCheckIsExecutable( bool val ) { fCheckIsExecutable = val; }

        bool checkIsFile() const { return fCheckIsFile; }
        void setCheckIsFile( bool val ) { fCheckIsFile = val; }

        bool checkIsHidden() const { return fCheckIsHidden; }
        void setCheckIsHidden( bool val ) { fCheckIsHidden = val; }

        bool checkIsJunction() const { return fCheckIsJunction; }
        void setCheckIsJunction( bool val ) { fCheckIsJunction = val; }

        bool checkIsReadable() const { return fCheckIsReadable; }
        void setCheckIsReadable( bool val ) { fCheckIsReadable = val; }

        bool checkIsShortcut() const { return fCheckIsShortcut; }
        void setCheckIsShortcut( bool val ) { fCheckIsShortcut = val; }

        bool checkIsSymLink() const { return fCheckIsSymLink; }
        void setCheckIsSymLink( bool val ) { fCheckIsSymLink = val; }

        bool checkIsSymbolicLink() const { return fCheckIsSymbolicLink; }
        void setCheckIsSymbolicLink( bool val ) { fCheckIsSymbolicLink = val; }

        bool checkIsWritable() const { return fCheckIsWritable; }
        void setCheckIsWritable( bool val ) { fCheckIsWritable = val; }

        QFile::Permissions checkPermissions() const { return fCheckPermissions; }
        void setCheckPermissions( QFile::Permissions val ) { fCheckPermissions = val; }

        bool useNTFSPermissions() const { return fUseNTFSPermissions; }
        void setUseNTFSPermissions( bool val ) { fUseNTFSPermissions = val; }

        QString msg() const { return fMessage; }
    private Q_SLOTS:
        void slotTimeout();
        void slotCanceled();
        void slotResultsReady( int index );
    private:
        bool canContinue() const;
        std::pair< bool, QString > checkPathInternal();

        int fTimeOut{ 5000 };
        bool fStopped{ false };
        bool fTimedOut{ false };
        QString fPathName;
        QString fMessage;

        bool fCheckExists{ true };
        bool fCheckIsBundle{ false };
        bool fCheckIsDir{ false };
        bool fCheckIsExecutable{ false };
        bool fCheckIsFile{ false };
        bool fCheckIsHidden{ false };
        bool fCheckIsJunction{ false };
        bool fCheckIsReadable{ false };
        bool fCheckIsShortcut{ false };
        bool fCheckIsSymLink{ false };
        bool fCheckIsSymbolicLink{ false };
        bool fCheckIsWritable{ false };
        QFile::Permissions fCheckPermissions;
        bool fUseNTFSPermissions{
#ifdef Q_OS_WINDOWS
        true
#else
        false
#endif
        };
    private:
        QFutureWatcher< std::pair< bool, QString > > * fResultWatcher{ nullptr };
    };
}
#endif

