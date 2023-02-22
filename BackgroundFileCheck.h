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
#include <QThread>

#include "SABUtilsExport.h"

namespace NSABUtils
{
    class CBackgroundFileCheckImpl;
    class SABUTILS_EXPORT CBackgroundFileCheck : public QObject
    {
        Q_OBJECT;
    public:
        CBackgroundFileCheck(QObject* parent = nullptr);
        virtual ~CBackgroundFileCheck();

        void stop(bool markAsStopped = true);

        void checkPath();
        void checkPath(const QString& pathName);

    Q_SIGNALS:
        void sigFinished(bool aOK, const QString& msg);

    public:
        int timeOut() const;
        void setTimeOut(int val);

        QString pathName() const;
        void setPathName(const QString& pathName);

        bool checkExists() const;
        void setCheckExists(bool val);

        bool checkIsBundle() const;
        void setCheckIsBundle(bool val);

        bool checkIsDir() const;
        void setCheckIsDir(bool val);

        bool checkIsExecutable() const;
        void setCheckIsExecutable(bool val);

        bool checkIsFile() const;
        void setCheckIsFile(bool val);

        bool checkIsHidden() const;
        void setCheckIsHidden(bool val);

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        bool checkIsJunction() const;
        void setCheckIsJunction(bool val);
#endif

        bool checkIsReadable() const;
        void setCheckIsReadable(bool val);

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        bool checkIsShortcut() const;
        void setCheckIsShortcut(bool val);
#endif

        bool checkIsSymLink() const;
        void setCheckIsSymLink(bool val);

#if QT_VERSION >= QT_VERSION_CHECK( 5,15, 0 )
        bool checkIsSymbolicLink() const;
        void setCheckIsSymbolicLink(bool val);
#endif
        bool checkIsWritable() const;
        void setCheckIsWritable(bool val);

        QFile::Permissions checkPermissions() const;
        void setCheckPermissions(QFile::Permissions val);

        bool useNTFSPermissions() const;
        void setUseNTFSPermissions(bool val);

        QString msg() const;
    public Q_SLOTS:
        void slotFinished();
        void slotTimeout();
    private:
        CBackgroundFileCheckImpl* fImpl{ nullptr };
    };

    class SABUTILS_EXPORT CBackgroundFileCheckThread : public QThread
    {
    public:
        CBackgroundFileCheckThread(CBackgroundFileCheckImpl* impl);
        virtual void run() override;
    private:
        CBackgroundFileCheckImpl* fImpl{ nullptr };
    };
}
#endif

