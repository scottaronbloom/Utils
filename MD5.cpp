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

#include "MD5.h"

#include <QString>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QThread>
#include <QDateTime>
#include <QDebug>
#include <QAbstractEventDispatcher>
#include <QIcon>
#include <QBuffer>

namespace NSABUtils
{
    QByteArray formatMd5( const QByteArray & digest, bool isHex )
    {
        QByteArray md5Str = digest;
        if ( !isHex )
        {
            md5Str = md5Str.toHex();
        }
        md5Str = md5Str.toUpper();
        if ( md5Str.length() < 32 )
            md5Str = QString( "%1" ).arg( QString::fromLatin1( md5Str ), 32, QLatin1Char( '0' ) ).toLatin1();
        if ( md5Str.length() == 32 )
        {
            md5Str = md5Str.mid( 0, 8 ) + "-" +
                md5Str.mid( 8, 4 ) + "-" +
                md5Str.mid( 12, 4 ) + "-" +
                md5Str.mid( 16, 4 ) + "-" +
                md5Str.mid( 20 )
                ;
        }
        else
        {
            for ( int ii = md5Str.length() - 4; ii > 0; ii -= 4 )
            {
                md5Str.insert( ii, '-' );
            }
        }
        return md5Str;
    }

    QByteArray getMd5( const QByteArray & data )
    {
        auto digest = QCryptographicHash::hash( data, QCryptographicHash::Md5 );
        return formatMd5( digest, false );
    }

    QByteArray getMd5( const QStringList & data )
    {
        QCryptographicHash hash( QCryptographicHash::Md5 );
        for ( auto && ii : data )
        {
            hash.addData( ii.toLatin1() );
        }
        return hash.result();
    }

    QString getMd5( const QString & data, bool isFileName )
    {
        if ( isFileName )
            return getMd5( QFileInfo( data ) );

        QByteArray inData = data.toLatin1();
        return QString::fromLatin1( getMd5( inData ) );
    }

    std::string getMd5( const std::string & data, bool isFileName )
    {
        return getMd5( QString::fromStdString( data ), isFileName ).toStdString();
    }

    QString getMd5( const QFileInfo & fi )
    {
        QFile file( fi.absoluteFilePath() );
        if ( !file.open( QIODevice::ReadOnly ) )
            return {};

        QCryptographicHash hash( QCryptographicHash::Md5 );
        if ( hash.addData( &file ) )
            return QString::fromLatin1( formatMd5( hash.result(), false ) );

        return {};
    }

    QByteArray getPixmapData( const QPixmap & pixMap )
    {
        QByteArray data;
        QBuffer buffer( &data );
        buffer.open( QIODevice::WriteOnly );
        pixMap.save( &buffer, "png" );
        return data;
    }

    SABUTILS_EXPORT QByteArray getMd5( const QPixmap & pixMap )
    {
        auto data = getPixmapData( pixMap );

        return getMd5( data );
    }

    SABUTILS_EXPORT QByteArray getMd5( const QIcon & icon )
    {
        auto sizes = icon.availableSizes();
        QCryptographicHash hash( QCryptographicHash::Md5 );
        for ( auto && ii : sizes )
        {
            auto data = getPixmapData( icon.pixmap( ii ) );
            hash.addData( data );
        }
        return formatMd5( hash.result(), false );
    }

    void CComputeMD5::run()
    {
        emit sigStarted( getThreadID(), QDateTime::currentDateTime(), fFileInfo.absoluteFilePath() );


        QFile file( fFileInfo.absoluteFilePath() );
        if ( !file.open( QIODevice::ReadOnly ) )
            emitFinished();


        QCryptographicHash hash( QCryptographicHash::Md5 );
        if ( !file.isReadable() )
            emitFinished();

        char buffer[ 4096 ];
        int length;

        qint64 pos = 0;
        while ( !fStopped && ( length = file.read( buffer, sizeof( buffer ) ) ) > 0 )
        {
            pos += length;
            hash.addData( buffer, length );
            emit sigReadPositionStatus( getThreadID(), QDateTime::currentDateTime(), fFileInfo.absoluteFilePath(), pos );
            if ( QThread::currentThread() && QThread::currentThread()->eventDispatcher() )
            {
                QThread::currentThread()->eventDispatcher()->processEvents( QEventLoop::AllEvents );
            }
        }

        if ( file.atEnd() )
        {
            emit sigFinishedReading( getThreadID(), QDateTime::currentDateTime(), fFileInfo.absoluteFilePath() );
            if ( fStopped )
            {
                emitFinished();
                return;
            }

            auto tmp = hash.result();
            emit sigFinishedComputing( getThreadID(), QDateTime::currentDateTime(), fFileInfo.absoluteFilePath() );
            if ( fStopped )
            {
                emitFinished();
                return;
            }

            fMD5 = QString::fromLatin1( formatMd5( tmp, false ) );
        }
        emitFinished();
    }


    unsigned long long CComputeMD5::getThreadID() const
    {
        return reinterpret_cast<unsigned long long>( QThread::currentThreadId() );
    }

    void CComputeMD5::slotStop()
    {
        fStopped = true;
        //qDebug() << "ComputeMD5 stopped";
    }

    void CComputeMD5::emitFinished()
    {
        emit sigFinished( getThreadID(), QDateTime::currentDateTime(), fFileInfo.absoluteFilePath(), fMD5 );
    }

}

