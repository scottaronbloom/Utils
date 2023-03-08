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

#include "ForceUnbufferedProcessModifier.h"
#include "FileUtils.h"

#include <QProcess>
#include <qt_windows.h>

namespace NSABUtils
{
    QProcess::CreateProcessArgumentModifier getForceUnbufferedProcessModifier()
    {
        return []( QProcess::CreateProcessArguments *args )
        {
            __pragma( pack( push, 1 ) ) struct HackedHandlePasser
            {
                using HANDLE = void *;

                DWORD NumberOfHandles{ 3 };   // 4 Byte
                BYTE FlagsPerHandle[ 3 ]{ 0 };   // 3 * 1 Byte
                HANDLE Handles[ 3 ]{ 0 };   // 3 * 8 Byte
            };
            __pragma( pack( pop ) )

            static_assert( sizeof( DWORD ) == 4 );
            static_assert( sizeof( BYTE ) == 1 );
            static_assert( sizeof( HANDLE ) == 8 );

            static_assert( sizeof( HackedHandlePasser ) == ( 4 + ( 3 * 1 ) + ( 3 * 8 ) ) );
            static_assert( sizeof( HackedHandlePasser ) == 31 );

#ifndef FOPEN
    #define FOPEN 0x01
#endif
#ifndef FDEV
    #define FDEV 0x40
#endif
            HackedHandlePasser handles;
            handles.FlagsPerHandle[ 0 ] = 0;
            std::memset( &handles.FlagsPerHandle[ 1 ], FOPEN | FDEV, 2 );

            const HANDLE invalid = INVALID_HANDLE_VALUE;
            std::memcpy( &handles.Handles[ 0 ], &args->startupInfo->hStdInput, sizeof( HANDLE ) );
            std::memcpy( &handles.Handles[ 1 ], &args->startupInfo->hStdOutput, sizeof( HANDLE ) );
            std::memcpy( &handles.Handles[ 2 ], &args->startupInfo->hStdError, sizeof( HANDLE ) );

            args->startupInfo->cbReserved2 = sizeof( HackedHandlePasser );
            args->startupInfo->lpReserved2 = reinterpret_cast< LPBYTE >( &handles );
        };
    }
}
