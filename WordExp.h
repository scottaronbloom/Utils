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

#ifndef __WORDEXP_H
#define __WORDEXP_H

#include <tuple>
#include <QString>
#include <QStringList>
#include <QHash>

// convert a path with wildcards into absolute paths

namespace NSABUtils
{
    class CWordExp
    {
        friend class CWordExpImpl;
    public:
        CWordExp(const QString & pathName);
        bool status() const { return fAOK; } // when used via constructor, set to false if invalid user request
        QStringList getAbsoluteFilePaths(bool * aOK = nullptr) const; // aOK set to false if windows and invalid user request
        bool beenThere(const QString & path);
        static QStringList getAbsoluteFilePaths(const QString & pathName, bool * aOK = nullptr); // aOK set to false if windows and invalid user request

        static QString getHomeDir(const QString & userName, bool * aOK = nullptr); // on windows can only return for current user, aOK set to false if windows and invalid user request
        static QString expandTildePath(const QString & fileName, bool * aOK = nullptr); // On linux, if it starts with ~user or ~/ on windows, only ~/ is supported, returns ~user (no expansion) otherwise, aOK set to false if windows and invalid user request

        static QString getUserName();
        static QString getHostName();
        static QString getUserInfo();
    private:
        static std::tuple< bool, QString, QString > isValidTilde(const QString & fileName);
        void expandPaths();
        void addResult(const QString & path);
        bool fAOK{ false };
        QString fOrigPathName;
        QStringList fExpandedPaths; // on linux, the wordexp expansion can return multiple paths
        QHash< QString, bool > fBeenThere; // due to links and other reasons
    };
}
#endif



