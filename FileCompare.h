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

#ifndef __FILECOMPARE_H
#define __FILECOMPARE_H

#include "SABUtilsExport.h"

#include <string>
#include <QFileDevice>
class QFileInfo;
class QString;

namespace NSABUtils
{
    namespace NFileUtils
    {
        // order of comparison
        // size
        // timestamp
        // attribute bits
        //   archive // windows only
        //   system  // windows only
        //   hidden  // windows only
        //   readonly
        // md5
        class CFileCompareImpl;
        class SABUTILS_EXPORT CFileCompare
        {
        public:
            CFileCompare( const std::string & lhs, const std::string & rhs );
            CFileCompare( const QString & lhs, const QString & rhs );
            CFileCompare( const QFileInfo & lhs, const QFileInfo & rhs );
            virtual ~CFileCompare();

            void setCheckSize( bool value ); // default true
            bool checkSize() const;

            void disableCheckTimeStamps();
            void checkTimeStamp( std::list< QFileDevice::FileTime > timeStampsToCheck ); // default modtime only
            std::list< QFileDevice::FileTime > timeStampsChecked() const;

            void setTimeStampTolerance( int seconds ); // default 2 seconds
            int timeStampTolerance() const;

            void compareArchiveBit( bool value ); // default false
            bool archiveBit() const;

            void compareSystemBit( bool value ); // default false
            bool systemBit() const;

            void compareHiddenBit( bool value ); // default false
            bool hiddenBit() const;

            void compareReadOnlyBit( bool value ); // default false
            bool readOnlyBit() const;

            void compareMD5( bool value ); // default true
            bool md5() const;

            bool compare() const;
        private:
            CFileCompareImpl * fImpl;
        };
    }
}
#endif
