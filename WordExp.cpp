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

#include "WordExp.h"
#include <QDir>
#include <QDebug>
#include <iostream>
#ifdef WIN32 
#include <QProcessEnvironment>
#define UNICODE
#include <userenv.h>
#include <lmcons.h>
#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Advapi32.lib")

#else
#include <pwd.h>
#include <unistd.h>
#include <wordexp.h>
#endif

namespace NSABUtils
{
    CWordExp::CWordExp(const QString & pathName) :
        fOrigPathName(pathName)
    {
        expandPaths();
    }

    QStringList CWordExp::getAbsoluteFilePaths(const QString & pathName, bool * aOK)
    {
        CWordExp wordExp(pathName);
        return wordExp.getAbsoluteFilePaths(aOK);
    }

    QStringList CWordExp::getAbsoluteFilePaths(bool * aOK) const
    {
        if (aOK)
            *aOK = fAOK;
        return fExpandedPaths;
    }

    bool CWordExp::beenThere(const QString & path)
    {
        if (fBeenThere.find(path) == fBeenThere.end())
        {
            fBeenThere.insert(path, true);
            return false;
        }
        else
            return true;
    }

#ifdef WIN32
    void DisplayError(LPWSTR pszAPI)
    {
        LPVOID lpvMessageBuffer;

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpvMessageBuffer, 0, NULL);

        //
        //... now display this string
        //
        wprintf(L"ERROR: API        = %s.\n", pszAPI);
        wprintf(L"       error code = %d.\n", GetLastError());
        wprintf(L"       message    = %s.\n", (LPWSTR)lpvMessageBuffer);

        //
        // Free the buffer allocated by the system
        //
        LocalFree(lpvMessageBuffer);
    }
#endif

    QString CWordExp::getUserName()
    {
        QString retVal = "unknown";
#ifdef _WIN32
        DWORD sz = UNLEN + 1;
        wchar_t * wcharUserName = new wchar_t[sz + 1];
        *wcharUserName = 0;
        if (!GetUserName(wcharUserName, &sz))
        {
            DisplayError(L"GetUserName");
            return QString();
        }

        retVal = QString::fromWCharArray(wcharUserName);
        delete[] wcharUserName;
#else
        int uid = geteuid();
        struct passwd * p = getpwuid(uid);
        if (p && p->pw_name)
            retVal = p->pw_name;
#endif
        return retVal;
    }

    QString CWordExp::getHostName()
    {
        QString retVal = "unknown";
#ifdef _WIN32
        WSADATA WSAData;

        // Initialize winsock dll
        if (::WSAStartup(MAKEWORD(1, 0), &WSAData) == 0)
        {
            char szHostName[128] = "";
            if (gethostname(szHostName, sizeof(szHostName) - 1) == 0)
                retVal = szHostName;

            WSACleanup();
        }
#else
        char hostName[256];
        *hostName = 0;
        if (gethostname(hostName, 256) == 0)
            retVal = hostName;
#endif
        return retVal;
        }

    QString CWordExp::getUserInfo()
    {
        return getUserName() + "@" + getHostName();
    }

    QString CWordExp::getHomeDir(const QString & userName, bool * aOK)
    {
        if (aOK)
            *aOK = false;
#ifdef WIN32
        if (userName.compare(getUserName(), Qt::CaseInsensitive) != 0)
        {
            return QString();
        }

        HANDLE hToken;
        QString retVal;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return retVal;

        DWORD dwSize = 0;
        if (!GetUserProfileDirectory(hToken, nullptr, &dwSize) && (dwSize != 0))
        {
            wchar_t * tmp = new wchar_t[dwSize + 1];
            bool aOK = GetUserProfileDirectory(hToken, tmp, &dwSize);
            if (aOK)
                retVal = QString::fromWCharArray(tmp);
            delete[] tmp;
        }
        ::CloseHandle(hToken);
        if (aOK)
            *aOK = true;
        return retVal;
#else
        size_t bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
        if (bufSize == (size_t)-1) // undetermined , just use a large number
            bufSize = 16384;

        char * buf = static_cast<char *>(malloc(bufSize));
        if (!buf)
            return QString();

        struct passwd pwd;
        struct passwd * result = &pwd;
        int s = getpwnam_r(qPrintable(userName), &pwd, buf, bufSize, &result);
        if ((s != 0) || (result == nullptr))
        {
            free(buf);
            perror("getpwnam_r");
            return QString();
        }
        QString retVal = pwd.pw_dir;
        free(buf);
        if (aOK)
            *aOK = true;
        return retVal;
#endif
    }

    std::tuple< bool, QString, QString > CWordExp::isValidTilde(const QString & fileName)
    {
        bool aOK = false;
        QString userName;
        QString remainder;

        if (fileName.startsWith("~"))
        {
            userName = fileName.mid(1);
            userName.replace('\\', '/');
            auto pos = userName.indexOf('/');
            if (pos != -1)
            {
                remainder = userName.mid(pos + 1);
                if (remainder.isEmpty())
                    remainder = userName[pos];
                userName = userName.left(pos);
            }

#ifdef WIN32
            if (userName.isEmpty())
                userName = getUserName();

            aOK = (userName == getUserName());
            if (!aOK)
            {
                remainder.clear();
                userName.clear();
            }
#else
            aOK = true;
#endif
            }
        else
        {
            aOK = true;
        }

        return std::make_tuple(aOK, userName, remainder);
        }

    QString CWordExp::expandTildePath(const QString & fileName, bool * aOK)
    {
        if (aOK)
            *aOK = false;
        bool localAOK = false;
        QString userName;
        QString remainder;
        std::tie(localAOK, userName, remainder) = isValidTilde(fileName);
        if (!localAOK)
        {
            return fileName;
        }

        if (userName.isEmpty())
        {
            if (aOK)
                *aOK = true;
#ifdef WIN32
            return QDir::toNativeSeparators(fileName);
#else
            return QDir::homePath() + remainder;
#endif
        }

        auto homePath = getHomeDir(userName);
        if (homePath.isEmpty())
            return fileName;

        QString retVal;
        if ((remainder == '/') || (remainder == '\\'))
            retVal = homePath + remainder;
        else
            retVal = QDir(homePath).absoluteFilePath(remainder);

        if (aOK)
            *aOK = true;
        return QDir::toNativeSeparators(retVal);
    }

    QString getEnvValue(const QString & envVar)
    {
        auto envVal = qgetenv(qPrintable(envVar));
        if (envVal.isEmpty())
            return envVar;
        else
            return envVal;
    }

    QString replaceEnvVars(const QString & curr)
    {
        QString retVal;
        QString currEnvVar;
        bool inDollar = false;
        bool inPercent = false;
        for (int ii = 0; ii < curr.length(); ++ii)
        {
            if (curr[ii] == '$')
            {
                if (inDollar)
                {
                    retVal += getEnvValue(currEnvVar);
                    currEnvVar.clear();
                }
                inDollar = true;
            }
            else if (curr[ii] == '%')
            {
                if (inPercent)
                {
                    retVal += getEnvValue(currEnvVar);
                    currEnvVar.clear();
                    inPercent = false;
                }
                else
                    inPercent = true;
            }
            else
            {
                if (inPercent || inDollar)
                    currEnvVar += curr[ii];
                else
                    retVal += curr[ii];
            }
        }
        if (inDollar)
            retVal += getEnvValue(currEnvVar);
        else if (inPercent)
            retVal += currEnvVar;
        return retVal;
    }

    class CWordExpImpl
    {
    public:
        CWordExpImpl(const QString & path, CWordExp * wordExp, CWordExpImpl * parent) :
            fPath(path),
            fParent(parent),
            fWordExp(wordExp)
        {
        }
        ~CWordExpImpl()
        {
            for (auto ii : fChildren)
                delete ii;
        }
        void findMatches(const QStringList & children)
        {
            bool aOK = false;
            findMatches(children, aOK, 0);
        }
    private:
        QString getMyPath() const
        {
            QString retVal = fPath;
            if (fParent)
                retVal = fParent->getMyPath() + "\\" + retVal;
            return retVal;
        }
        void findMatches(QStringList children, bool & aOK, int indent);
        QString fPath;
        CWordExpImpl * fParent{ nullptr };
        CWordExp * fWordExp{ nullptr };
        std::list< CWordExpImpl * > fChildren;
    };

    void CWordExpImpl::findMatches(QStringList children, bool & aOK, int indent)
    {
        if (children.empty())
            return;

        if (children.empty() || fWordExp->beenThere(getMyPath()))
        {
            aOK = true;
            return;
        }

        auto child = children.front();
        children.pop_front();

        aOK = false;
        QStringList matches;
        auto myPath = getMyPath();
        //std::cout << std::string( indent, ' ' ) << qPrintable( myPath ) << "\n";
        if ((child.indexOf("*") == -1) && (child.indexOf('?') == -1))
        {
            myPath += "\\" + child;
            if (QFileInfo::exists(myPath))
            {
                matches << child;
                aOK = true;
            }
        }
        else
        {
            QFileInfo fi(myPath);
            if (!fi.exists() || !fi.isDir())
                return;

            QDir dir(myPath);
            auto filters = QDir::Dirs | QDir::NoDotAndDotDot;
            if (children.isEmpty())
                filters |= QDir::Files;
            matches = dir.entryList(QStringList() << child, filters);
            aOK = !matches.isEmpty();
        }
        if (aOK)
        {
            for (auto && curr : matches)
            {
                //std::cout << std::string( indent + 1, ' ' ) << "=== FOUND: " << qPrintable( curr ) << "\n";
                auto child = new CWordExpImpl(curr, fWordExp, this);
                fChildren.push_back(child);
                child->findMatches(children, aOK, indent + 1);
                if (!aOK) // this branch is dead
                {
                    fChildren.pop_back();
                    delete child;
                    continue;
                }
                if (aOK && children.isEmpty())
                {
                    for (auto && ii : fChildren)
                        fWordExp->addResult(ii->getMyPath());
                }
            }
        }
    }

    void CWordExp::expandPaths()
    {
        // path no longer has ~ in it
#ifdef WIN32 
        auto nonTildePath = expandTildePath(fOrigPathName, &fAOK);
        if (!fAOK)
            return;

        auto dirs = nonTildePath.split("\\");
        if (dirs.isEmpty())
        {
            fAOK = false;
            return;
        }
        for (auto && ii : dirs)
        {
            ii = replaceEnvVars(ii);
        }

        nonTildePath = dirs.join("\\");

        // path now has no ~ and is using native separators "\\";
        QFileInfo fi(nonTildePath);
        if (fi.isRelative())
        {
            nonTildePath = QDir::toNativeSeparators(fi.absoluteFilePath());
        }

        fAOK = false;
        dirs = nonTildePath.split("\\");
        if (dirs.isEmpty())
            return;

        // will start with a drive;
        CWordExpImpl top(dirs.front(), this, nullptr);
        dirs.pop_front();
        top.findMatches(dirs);
        fAOK = !fExpandedPaths.isEmpty();


#else
        wordexp_t results;
        int status = wordexp(qPrintable(fOrigPathName), &results, 0);
        fAOK = status == 0;
        for (int ii = 0; ii < results.we_wordc; ++ii)
        {
            fExpandedPaths.push_back(results.we_wordv[ii]);
    }
        wordfree(&results);
#endif
    }

    void CWordExp::addResult(const QString & path)
    {
        if (beenThere(path))
            return;
        fExpandedPaths << path;
    }
}

