// The MIT License( MIT )
//
// Copyright( c ) 2022 Scott Aron Bloom
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

#include "SelectFileUrl.h"
#include "FileUtils.h"
#include <QFileInfo>
#include <QUrl>
#include <QThread>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <shellapi.h>
#include <combaseapi.h>
#include <shtypes.h>
#include <ShlObj_core.h>
#else
#define S_OK 0
#endif

namespace NSABUtils
{
#ifndef Q_OS_WIN
#define S_OK 0
#endif
    CSelectFileUrl::CSelectFileUrl(QObject* parent) :
        QObject(parent)
    {
#ifdef Q_OS_WIN
        QDesktopServices::setUrlHandler("file", this, "slotOpenUrl");
#endif
    }


    class CLaunchThread : public QThread
    {
    public:
        explicit CLaunchThread(QString path)
        {
#ifdef Q_OS_WIN
            auto fi = QFileInfo(path);
            auto dirloc = fi.absolutePath().replace("/", "\\");
            fDir = ILCreateFromPath(reinterpret_cast<const wchar_t*>(dirloc.utf16()));

            path = path.replace("/", "\\");
            fPath = ILCreateFromPath(reinterpret_cast<const wchar_t*>(path.utf16()));
#else
            (void)path;
#endif
        }

        ~CLaunchThread()
        {
#ifdef Q_OS_WIN
            ILFree(fDir);
            ILFree(fPath);
#endif
        }
        void run() override
        {
#ifdef Q_OS_WIN
            fResult = SHOpenFolderAndSelectItems(fDir, 1, (LPCITEMIDLIST*)&fPath, 0);
#endif
        }

        bool result() const { return fResult == S_OK; }

    private:
#ifdef Q_OS_WIN
        LPITEMIDLIST fDir{ nullptr };
        LPITEMIDLIST fPath{ nullptr };
#endif
        uint64_t fResult{ S_OK };
    };

    void CSelectFileUrl::slotOpenUrl(const QUrl& url)
    {
        if (!url.isLocalFile())
            return;
        auto path = url.toLocalFile();
        auto fi = QFileInfo(path);
        if (fi.isDir())
        {
            QDesktopServices::openUrl(url);
            return;
        }

        CLaunchThread thread(url.toLocalFile());
        thread.start();
        thread.wait();
    }
}

