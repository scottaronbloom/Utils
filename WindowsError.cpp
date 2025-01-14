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

#include "WindowsError.h"
#ifdef Q_OS_WINDOWS
    #include <qt_windows.h>
#endif

namespace NSABUtils
{
#ifdef Q_OS_WINDOWS
    QString getWindowsError( int errorCode )
    {
        QString ret;
    #ifndef Q_OS_WINRT
        wchar_t *string = 0;
        FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPWSTR)&string, 0, NULL );
        ret = QString::fromWCharArray( string );
        LocalFree( (HLOCAL)string );
    #else
        wchar_t errorString[ 1024 ];
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPWSTR)&errorString, sizeof( errorString ) / sizeof( wchar_t ), NULL );
        ret = QString::fromWCharArray( errorString );
    #endif   // Q_OS_WINRT

        if ( ret.isEmpty() && errorCode == ERROR_MOD_NOT_FOUND )
            ret = QString::fromLatin1( "The specified module could not be found." );
        if ( ret.endsWith( QLatin1String( "\r\n" ) ) )
            ret.chop( 2 );
        if ( ret.isEmpty() )
            ret = QString::fromLatin1( "Unknown error 0x%1." ).arg( unsigned( errorCode ), 8, 16, QLatin1Char( '0' ) );
        return ret;
    }
#endif
}
