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

#include "FileUtils.h"
#include <QDir>

namespace NSABUtils
{
    namespace NFileUtils
    {
        std::optional< QList< QFileInfo > > findAllFiles(const QDir& dir, const QStringList& nameFilters, bool recursive, bool sortByName, QString* errorMsg)
        {
            if (!dir.exists() || !dir.isReadable())
            {
                if (errorMsg)
                    *errorMsg = QString("Directory '%1' does not exist.").arg(dir.absolutePath());
                return {};
            }
            //qDebug() << dir.absolutePath();
            auto retVal = dir.entryInfoList(nameFilters, QDir::Files, sortByName ? QDir::SortFlag::Name : QDir::SortFlag::NoSort);
            if (recursive)
            {
                auto subDirs = dir.entryInfoList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot, sortByName ? QDir::SortFlag::Name : QDir::SortFlag::NoSort);
                for (auto&& ii : subDirs)
                {
                    auto curr = findAllFiles(QDir(ii.absoluteFilePath()), nameFilters, recursive, sortByName, errorMsg);
                    if (!curr.has_value())
                        return curr;
                    retVal << curr.value();
                }
            }
            return retVal;
        }
    }
}


