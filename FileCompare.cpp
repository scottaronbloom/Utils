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


#include "FileCompare.h"
#include "FileUtils.h"
#include "MD5.h"

#include <QString>
#include <QFileInfo>

namespace NSABUtils
{
    namespace NFileUtils
    {
        class CFileCompareImpl
        {
        public:
            CFileCompareImpl( const QFileInfo & lhs, const QFileInfo & rhs ) :
                fLHS( lhs ),
                fRHS( rhs )
            {
            }

            bool compare() const;

            QFileInfo fLHS;
            QFileInfo fRHS;

            bool fCheckSize{ true };
            std::list< QFileDevice::FileTime > fTimeStampsToCheck{ QFileDevice::FileTime::FileModificationTime };
            int fTolerance{ 2 };
            bool fArchiveBit{ false };
            bool fSystemBit{ false };
            bool fHiddenBit{ false };
            bool fReadOnlyBit{ false };
            bool fMD5{ true };
        };

        CFileCompare::CFileCompare( const std::string & lhs, const std::string & rhs ) :
            CFileCompare( QString::fromStdString( lhs ), QString::fromStdString( rhs ) )
        {

        }

        CFileCompare::CFileCompare( const QString & lhs, const QString & rhs ) :
            CFileCompare( QFileInfo( lhs ), QFileInfo( rhs ) )
        {

        }

        CFileCompare::CFileCompare( const QFileInfo & lhs, const QFileInfo & rhs ) :
            fImpl( new CFileCompareImpl( lhs, rhs ) )
        {
        }

        CFileCompare::~CFileCompare()
        {
            delete fImpl;
        }

        void CFileCompare::setCheckSize( bool value )
        {
            fImpl->fCheckSize = value;
        }

        bool CFileCompare::checkSize() const
        {
            return fImpl->fCheckSize;
        }

        void CFileCompare::disableCheckTimeStamps()
        {
            checkTimeStamp( {} );
        }

        void CFileCompare::checkTimeStamp( std::list< QFileDevice::FileTime > value ) // default modtime only
        {
            fImpl->fTimeStampsToCheck = value;
        }

        std::list< QFileDevice::FileTime > CFileCompare::timeStampsChecked() const
        {
            return fImpl->fTimeStampsToCheck;
        }

        void CFileCompare::setTimeStampTolerance( int seconds ) // default 2 seconds
        {
            fImpl->fTolerance = seconds;
        }

        int CFileCompare::timeStampTolerance() const
        {
            return fImpl->fTolerance;
        }

        void CFileCompare::compareArchiveBit( bool value ) // default false
        {
            fImpl->fArchiveBit = value;
        }

        bool CFileCompare::archiveBit() const
        {
            return fImpl->fArchiveBit;
        }

        void CFileCompare::compareSystemBit( bool value ) // default false
        {
            fImpl->fSystemBit = value;
        }

        bool CFileCompare::systemBit() const
        {
            return fImpl->fSystemBit;
        }

        void CFileCompare::compareHiddenBit( bool value ) // default false
        {
            fImpl->fHiddenBit = value;
        }

        bool CFileCompare::hiddenBit() const
        {
            return fImpl->fHiddenBit;
        }

        void CFileCompare::compareReadOnlyBit( bool value ) // default false
        {
            fImpl->fReadOnlyBit = value;
        }

        bool CFileCompare::readOnlyBit() const
        {
            return fImpl->fReadOnlyBit;
        }

        void CFileCompare::compareMD5( bool value ) // default true
        {
            fImpl->fMD5 = value;
        }

        bool CFileCompare::md5() const
        {
            return fImpl->fMD5;
        }

        bool CFileCompare::compare() const
        {
            return fImpl->compare();
        }

        // order of comparison
        // size
        // timestamp
        // attribute bits
        //   archive
        //   system
        //   hidden
        //   readonly
        // md5

        bool CFileCompareImpl::compare() const
        {
            if ( fCheckSize )
            {
                if ( fLHS.size() != fRHS.size() )
                    return false;
            }

            if ( !compareTimeStamp( fLHS, fRHS, fTolerance, fTimeStampsToCheck ) )
            {
                return false;
            }

            if ( fArchiveBit )
            {
                if ( isArchiveFile( fLHS ) != isArchiveFile( fRHS ) )
                    return false;
            }

            if ( fSystemBit )
            {
                if ( isSystemFile( fLHS ) != isSystemFile( fRHS ) )
                    return false;
            }

            if ( fHiddenBit )
            {
                if ( isHiddenFile( fLHS ) != isHiddenFile( fRHS ) )
                    return false;
            }

            if ( fReadOnlyBit )
            {
                if ( isReadOnlyFile( fLHS ) != isReadOnlyFile( fRHS ) )
                    return false;
            }

            return NSABUtils::getMd5( fLHS ) == NSABUtils::getMd5( fRHS );
        }

    }
}
