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

#ifndef __MOVETOTRASH_H
#define __MOVETOTRASH_H

#include "SABUtilsExport.h"

#include <string>
#include <list>
#include <set>
#include <memory>
#include <unordered_map>
#include <QStringList>
#include <QFileDevice>
#include <QList>
#include <memory>

class QFileInfo;
class QDateTime;
class QString;
class QDir;

namespace NSABUtils
{
    namespace NFileUtils
    {
        struct SABUTILS_EXPORT SRecycleOptions
        {
            SRecycleOptions() {}

            bool fDeleteOnRecycleFailure{ true };
            bool fForce{ false };
            bool fVerbose{ false };
            bool fInteractive{ false };
        };

        SABUTILS_EXPORT bool moveToTrash( const QFileInfo &info, QString *msg = nullptr, std::shared_ptr< SRecycleOptions > options = {} );
        SABUTILS_EXPORT bool moveToTrash( const QString &fileName, QString *msg = nullptr, std::shared_ptr< SRecycleOptions > options = {} );
        SABUTILS_EXPORT bool moveToTrash( const std::string &fileName, std::string *msg = nullptr, std::shared_ptr< SRecycleOptions > options = {} );
    }
}
#endif
