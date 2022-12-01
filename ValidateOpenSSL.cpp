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

#ifndef QT_NO_SSL
#include "ValidateOpenSSL.h"
#include <QApplication>
#include <QSslSocket>
#include <QLibrary>
#include <QDir>
namespace NSABUtils
{
    std::pair< bool, QString > validateOpenSSL( bool requireLocal /*= true*/ )
    {
#if defined(Q_PROCESSOR_X86_64)
#define QT_SSL_SUFFIX "-x64"
#elif defined(Q_PROCESSOR_ARM_64)
#define QT_SSL_SUFFIX "-arm64"
#elif defined(Q_PROCESSOR_ARM_32)
#define QT_SSL_SUFFIX "-arm"
#else
#define QT_SSL_SUFFIX
#endif

        if ( requireLocal )
        {
            auto libs = QStringList( { QString( "libssl-1_1" QT_SSL_SUFFIX ), QString( "libcrypto-1_1" QT_SSL_SUFFIX ) } );
            auto appDir = QDir( QApplication::applicationDirPath() );
            for ( auto && currLib : libs )
            {
                auto path = appDir.absoluteFilePath( currLib );
                QLibrary lib( path );
                if ( !lib.load() )
                {
                    return { false, QObject::tr( "Please re-install, The OpenSSL library '%1' could not be found." ).arg( currLib ) };
                }
            }
        }
        if ( !QSslSocket::supportsSsl() )
        {
            return { false, QObject::tr( "Please re-install, The OpenSSL libraries could not be found" ) };
        }

        //tryToLoadOpenSslWin32Library(QLatin1String(),
        //    QLatin1String), result);

#undef QT_SSL_SUFFIX

        return { true, QString() };
    }
}
#endif
