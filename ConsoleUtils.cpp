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

#include "ConsoleUtils.h"
#include "utils.h"
#include <QString>
#include <io.h>
#include <cstdio>
#include <iostream>

#ifdef Q_OS_WINDOWS
#define UNICODE
#include <qt_windows.h>
#endif

namespace NSABUtils
{
#ifdef Q_OS_WINDOWS
    ESubSystem getSubSystemForHandle(void * handle)
    {
        auto offsetValue = NSABUtils::MarshalRead<uint32_t>(handle, 0x3C);
        auto actualValue = NSABUtils::MarshalRead<uint16_t>(handle, offsetValue + 0x5c);
        return static_cast<ESubSystem>(actualValue);
    }

    const char * toString(ESubSystem subsystem)
    {
        switch (subsystem)
        {
        case ESubSystem::eIMAGE_SUBSYSTEM_NATIVE: return "Device drivers and native Windows processes";
        case ESubSystem::eIMAGE_SUBSYSTEM_WINDOWS_GUI: return "The Windows graphical user interface(GUI) subsystem";
        case ESubSystem::eIMAGE_SUBSYSTEM_WINDOWS_CUI: return "The Windows character subsystem";
        case ESubSystem::eIMAGE_SUBSYSTEM_OS2_CUI: return "The OS/2 character subsystem";
        case ESubSystem::eIMAGE_SUBSYSTEM_POSIX_CUI: return "The Posix character subsystem";
        case ESubSystem::eIMAGE_SUBSYSTEM_NATIVE_WINDOWS: return "Native Win9x driver";
        case ESubSystem::eIMAGE_SUBSYSTEM_WINDOWS_CE_GUI: return "Windows CE";
        case ESubSystem::eIMAGE_SUBSYSTEM_EFI_APPLICATION: return "An Extensible Firmware Interface(EFI) application";
        case ESubSystem::eIMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER: return "An EFI driver with boot services";
        case ESubSystem::eIMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER: return "An EFI driver with run - time services";
        case ESubSystem::eIMAGE_SUBSYSTEM_EFI_ROM: return "An EFI ROM image";
        case ESubSystem::eIMAGE_SUBSYSTEM_XBOX: return "XBOX";
        case ESubSystem::eIMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION: return "Windows boot application.";
        case ESubSystem::eIMAGE_SUBSYSTEM_UNKNOWN:
        default:
            return "An unknown subsystem";
        };
    }

    ESubSystem getSubSystemForCurrentHandle()
    {
        return getSubSystemForHandle(::GetModuleHandle(nullptr));
    }

    bool runningAsConsole()
    {
        auto subsystem = getSubSystemForCurrentHandle();
        if (subsystem != ESubSystem::eIMAGE_SUBSYSTEM_WINDOWS_CUI)
            return false;
        return ::GetConsoleWindow() != nullptr;

    }

    bool attachConsoleInt(QString * msg, bool tryToAlloc)
    {
        if (msg)
            msg->clear();
        auto aOK = ::AttachConsole(ATTACH_PARENT_PROCESS);
        // aOK == 0 if error
        // non-zero if successful
        if (aOK == 0)
        {
            auto errorID = ::GetLastError();
            // known errors are 
            // If the calling process is already attached to a console, the error code returned is ERROR_ACCESS_DENIED.
            // If the specified process does not have a console, the error code returned is ERROR_INVALID_HANDLE.
            // If the specified process does not exist, the error code returned is ERROR_INVALID_PARAMETER.

            if ((errorID == ERROR_ACCESS_DENIED)
                || (errorID == ERROR_INVALID_HANDLE)
                || (errorID == ERROR_INVALID_PARAMETER)
                )
            {
                auto currProcessID = ::GetCurrentProcessId();
                aOK = ::AttachConsole(currProcessID);
                if (tryToAlloc && (aOK == 0))
                {
                    aOK = ::AllocConsole();
                    if (aOK != 0)
                    {
                        return attachConsoleInt(msg, false);
                    }
                }
            }
        }

        if (::GetConsoleWindow() == nullptr)
        {
            if (msg)
                *msg = getLastError();
            return false;
        }
        else
        {
            fprintf(stdout, "STDOUT 1\n");
            fprintf(stderr, "STDERR 1\n");

            HANDLE hConIn = GetStdHandle(STD_INPUT_HANDLE);
            if (hConIn != INVALID_HANDLE_VALUE)
            {
                int fd0 = _open_osfhandle((intptr_t)hConIn, 0);
                if (fd0 != -1)
                {
                    _dup2(fd0, 0);
                }
            }

            HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConOut != INVALID_HANDLE_VALUE)
            {
                int fd1 = _open_osfhandle((intptr_t)hConOut, 0);
                if (fd1 != -1)
                {
                    _dup2(fd1, 1);
                }
                freopen("CONOUT$", "w", stdout);
            }

            fprintf(stdout, "STDOUT 2\n");
            fprintf(stderr, "STDERR 2\n");

            HANDLE hConErr = GetStdHandle(STD_ERROR_HANDLE);
            if (hConErr != INVALID_HANDLE_VALUE)
            {
                int fd2 = _open_osfhandle((intptr_t)hConErr, 0);
                if (fd2 != -1)
                {
                    _dup2(fd2, 2);
                }
                freopen("CONOUT$", "w", stderr);
            }
        }

        fprintf(stdout, "STDOUT 3\n");
        fprintf(stderr,"STDERR 3\n");

        return true;
    }

    bool attachConsole(QString * msg)
    {
        auto handle = ::GetConsoleWindow();
        auto retVal = attachConsoleInt(msg, true);
        handle = ::GetConsoleWindow();
        return retVal || ( handle != nullptr );
    }

    bool attachConsole(std::string * msg)
    {
        QString tmp;
        auto retVal = attachConsole(&tmp);
        if (msg)
            *msg = tmp.toStdString();
        return retVal;
    }
    

#else
bool runningAsConsole() { return true; }
void attachConsole() {}
#endif
}
