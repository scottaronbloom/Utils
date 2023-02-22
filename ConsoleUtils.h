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
#ifndef __CONSOLEUTILS_H
#define __CONSOLEUTILS_H

#include "SABUtilsExport.h"


#include <string>
#include <QString>
namespace NSABUtils
{
    enum class ESubSystem
    {
        eIMAGE_SUBSYSTEM_UNKNOWN = 0
        , eIMAGE_SUBSYSTEM_NATIVE = 1
        , eIMAGE_SUBSYSTEM_WINDOWS_GUI = 2
        , eIMAGE_SUBSYSTEM_WINDOWS_CUI = 3
        , eIMAGE_SUBSYSTEM_OS2_CUI = 5
        , eIMAGE_SUBSYSTEM_POSIX_CUI = 7
        , eIMAGE_SUBSYSTEM_NATIVE_WINDOWS = 8
        , eIMAGE_SUBSYSTEM_WINDOWS_CE_GUI = 9
        , eIMAGE_SUBSYSTEM_EFI_APPLICATION = 10
        , eIMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER = 11
        , eIMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER = 12
        , eIMAGE_SUBSYSTEM_EFI_ROM = 13
        , eIMAGE_SUBSYSTEM_XBOX = 14
        , eIMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION = 16
    };

    // windows only, not defined for linux
    SABUTILS_EXPORT const char* toString(ESubSystem subsystem);
    SABUTILS_EXPORT ESubSystem getSubSystemForCurrentHandle();
    SABUTILS_EXPORT ESubSystem getSubSystemForHandle(void* handle); // return from GetModuleHandle

    // returns true for linux
    SABUTILS_EXPORT bool runningAsConsole();  // getSubSystemForCurrentHandle() == eIMAGE_SUBSYSTEM_WINDOWS_CUI

    // does nothing for linux
    SABUTILS_EXPORT bool attachConsole(std::string* msg = nullptr); // returns true if successful
    SABUTILS_EXPORT bool attachConsole(QString* msg); // returns true if successful
}

#endif

