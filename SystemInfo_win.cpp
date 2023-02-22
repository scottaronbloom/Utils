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
#include "SystemInfo.h"
#include "nodiscard.h"

#include <QDateTime>
#include <QTimeZone>
#include <QDebug>
#include <QSettings>
#include <QLocale>

#include "StringUtils.h"
#include <unordered_map>
#include <iomanip>
#include <bitset>

#pragma warning( disable:4996 )
#define WINVER 0x0501
#include <windows.h>
#include <comdef.h>
#include <psapi.h>
#include <pdh.h>
#include <pdhmsg.h>

#include <iphlpapi.h>
#include <ipifcons.h>
#pragma comment( lib, "IPHLPAPI.lib" )
#pragma comment( lib, "PDH.lib" )

/////////////////////////////////////////////////////////////////////////////
// Win32 libraries

namespace NSABUtils
{
    bool CSystemInfo::hasSystemInfo()
    {
        return true;
    }

#define ALLOCATE_FROM_PROCESS_HEAP( bytes ) ::HeapAlloc( ::GetProcessHeap(), HEAP_ZERO_MEMORY, bytes )
#define DEALLOCATE_FROM_PROCESS_HEAP( ptr ) \
    if ( ptr ) \
    ::HeapFree( ::GetProcessHeap(), 0, ptr )
#define REALLOC_FROM_PROCESS_HEAP( ptr, bytes ) ::HeapReAlloc( ::GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, bytes )

    std::list< std::unordered_map< std::string, std::string > > GetAllNICDesc()
    {
        std::list< std::unordered_map< std::string, std::string > > retVal;

        IP_ADAPTER_INFO *pAdptInfo = nullptr;
        IP_ADAPTER_INFO *pNextAd = nullptr;
        ULONG ulLen = 0;

        DWORD rc = ::GetAdaptersInfo( pAdptInfo, &ulLen );
        if ( rc == ERROR_BUFFER_OVERFLOW )
        {
            pAdptInfo = (IP_ADAPTER_INFO *)ALLOCATE_FROM_PROCESS_HEAP( ulLen );
            rc = ::GetAdaptersInfo( pAdptInfo, &ulLen );
        }

        if ( rc != NO_ERROR )
            return retVal;

        // acquire a count of how many network adapters
        // we are going to need to work on so that
        // we can make sure we have adequate buffer space.
        pNextAd = pAdptInfo;
        for ( pNextAd = pAdptInfo; pNextAd; pNextAd = pNextAd->Next )
        {
            std::string desc = pNextAd->Description;

            __int64 macAddr = 0;
            for ( unsigned int ii = 0; ii < pNextAd->AddressLength; ++ii )
            {
                macAddr <<= 8;
                macAddr |= pNextAd->Address[ ii ];
            }

            std::ostringstream oss;
            oss << std::setw( pNextAd->AddressLength * 2 ) << std::uppercase << std::hex << macAddr << std::dec;

            std::unordered_map< std::string, std::string > currMap;
            currMap[ "Description" ] = pNextAd->Description;
            currMap[ "MAC" ] = oss.str();

            PIP_ADDR_STRING pAddressList = &( pNextAd->IpAddressList );
            int ipNum = 0;
            do
            {
                std::string ipaddr = pAddressList->IpAddress.String;
                currMap[ "IP" + std::to_string( ipNum ) ] = ipaddr;
                ipNum++;
                pAddressList = pAddressList->Next;
            }
            while ( pAddressList != nullptr );

            retVal.push_back( currMap );
        }

        // free any memory we allocated from the heap before
        // exit.  we wouldn't wanna leave memory leaks now would we? ;p
        DEALLOCATE_FROM_PROCESS_HEAP( pAdptInfo );
        return retVal;
    }

    std::wstring getOSName( OSVERSIONINFOEX &osvi )
    {
        std::wstring retVal;
        switch ( osvi.dwPlatformId )
        {
                // Test for the Windows NT product family.
            case VER_PLATFORM_WIN32_NT:
                {
                    typedef void( WINAPI * PGNSI )( LPSYSTEM_INFO );
                    SYSTEM_INFO si;
                    ZeroMemory( &si, sizeof( SYSTEM_INFO ) );
                    PGNSI pGNSI = (PGNSI)GetProcAddress( GetModuleHandle( L"kernel32.dll" ), "GetNativeSystemInfo" );
                    if ( nullptr != pGNSI )
                        pGNSI( &si );
                    else
                        GetSystemInfo( &si );

                    retVal += L"Microsoft ";

                    // Test for the specific product.

                    if ( osvi.dwMajorVersion == 6 )
                    {
                        if ( osvi.dwMinorVersion == 0 )
                        {
                            if ( osvi.wProductType == VER_NT_WORKSTATION )
                                retVal += L"Windows Vista ";
                            else
                                retVal += L"Windows Server 2008 ";
                        }

                        if ( osvi.dwMinorVersion == 1 )
                        {
                            if ( osvi.wProductType == VER_NT_WORKSTATION )
                                retVal += L"Windows 7 ";
                            else
                                retVal += L"Windows Server 2008 R2 ";
                        }

                        typedef BOOL( WINAPI * PGPI )( DWORD, DWORD, DWORD, DWORD, PDWORD );
                        PGPI pGPI;
                        pGPI = (PGPI)GetProcAddress( GetModuleHandle( TEXT( "kernel32.dll" ) ), "GetProductInfo" );

                        DWORD dwType;
                        pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType );

                        switch ( dwType )
                        {
                            case PRODUCT_ULTIMATE:
                            case 0x0000001C:   // PRODUCT_ULTIMATE_N:
                                retVal += L"Ultimate Edition";
                                break;
                            case 0x00000030:   // PRODUCT_PROFESSIONAL:
                            case 0x00000045:   // PRODUCT_PROFESSIONAL_2:
                            case 0x00000031:   // PRODUCT_PROFESSIONAL_N:
                                retVal += L"Professional";
                                break;
                            case PRODUCT_HOME_PREMIUM:
                                retVal += L"Home Premium Edition";
                                break;
                            case PRODUCT_HOME_BASIC:
                                retVal += L"Home Basic Edition";
                                break;
                            case PRODUCT_ENTERPRISE:
                                retVal += L"Enterprise Edition";
                                break;
                            case PRODUCT_BUSINESS:
                                retVal += L"Business Edition";
                                break;
                            case PRODUCT_STARTER:
                                retVal += L"Starter Edition";
                                break;
                            case PRODUCT_CLUSTER_SERVER:
                                retVal += L"Cluster Server Edition";
                                break;
                            case PRODUCT_DATACENTER_SERVER:
                                retVal += L"Datacenter Edition";
                                break;
                            case PRODUCT_DATACENTER_SERVER_CORE:
                                retVal += L"Datacenter Edition (core installation)";
                                break;
                            case PRODUCT_ENTERPRISE_SERVER:
                                retVal += L"Enterprise Edition";
                                break;
                            case PRODUCT_ENTERPRISE_SERVER_CORE:
                                retVal += L"Enterprise Edition (core installation)";
                                break;
                            case PRODUCT_ENTERPRISE_SERVER_IA64:
                                retVal += L"Enterprise Edition for Itanium-based Systems";
                                break;
                            case PRODUCT_SMALLBUSINESS_SERVER:
                                retVal += L"Small Business Server";
                                break;
                            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                                retVal += L"Small Business Server Premium Edition";
                                break;
                            case PRODUCT_STANDARD_SERVER:
                                retVal += L"Standard Edition";
                                break;
                            case PRODUCT_STANDARD_SERVER_CORE:
                                retVal += L"Standard Edition (core installation)";
                                break;
                            case PRODUCT_WEB_SERVER:
                                retVal += L"Web Server Edition";
                                break;
                        }
                    }

                    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
                    {
                        if ( GetSystemMetrics( SM_SERVERR2 ) )
                            retVal += L"Windows Server 2003 R2, ";
                        else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
                            retVal += L"Windows Storage Server 2003";
                        else if ( osvi.wSuiteMask & 0x00008000 )   // VER_SUITE_WH_SERVER )
                            retVal += L"Windows Home Server";
                        else if ( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
                        {
                            retVal += L"Windows XP Professional x64 Edition";
                        }
                        else
                            retVal += L"Windows Server 2003, ";

                        // Test for the server type.
                        if ( osvi.wProductType != VER_NT_WORKSTATION )
                        {
                            if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
                            {
                                if ( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                                    retVal += L"Datacenter Edition for Itanium-based Systems";
                                else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                                    retVal += L"Enterprise Edition for Itanium-based Systems";
                            }

                            else if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
                            {
                                if ( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                                    retVal += L"Datacenter x64 Edition";
                                else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                                    retVal += L"Enterprise x64 Edition";
                                else
                                    retVal += L"Standard x64 Edition";
                            }

                            else
                            {
                                if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                                    retVal += L"Compute Cluster Edition";
                                else if ( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                                    retVal += L"Datacenter Edition";
                                else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                                    retVal += L"Enterprise Edition";
                                else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                                    retVal += L"Web Edition";
                                else
                                    retVal += L"Standard Edition";
                            }
                        }
                    }

                    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
                    {
                        retVal += L"Windows XP ";
                        if ( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                            retVal += L"Home Edition";
                        else
                            retVal += L"Professional";
                    }

                    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                    {
                        retVal += L"Windows 2000 ";

                        if ( osvi.wProductType == VER_NT_WORKSTATION )
                        {
                            retVal += L"Professional";
                        }
                        else
                        {
                            if ( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                                retVal += L"Datacenter Server";
                            else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                                retVal += L"Advanced Server";
                            else
                                retVal += L"Server";
                        }
                    }

                    // Include service pack (if any) and build number.
                    if ( *osvi.szCSDVersion )
                    {
                        retVal += L" ";
                        retVal += std::wstring( osvi.szCSDVersion );
                    }

                    BOOL isWow64 = false;
                    typedef BOOL( WINAPI * LPFN_ISWOW64PROCESS )( HANDLE, PBOOL );
                    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle( TEXT( "kernel32" ) ), "IsWow64Process" );

                    if ( fnIsWow64Process == nullptr )
                    {
                        // 32 bit os
                        isWow64 = false;
                    }
                    else
                    {
                        if ( !fnIsWow64Process( GetCurrentProcess(), &isWow64 ) )
                        {
                            isWow64 = false;
                        }
                    }

                    if ( isWow64 )
                        retVal += L"(x64) ";
                }

                // Display service pack (if any) and build number.

                if ( osvi.dwMajorVersion == 4 && QString::fromWCharArray( osvi.szCSDVersion ).toUpper() == "SERVICE PACK 6" )
                {
                    QString key = R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Hotfix)";
                    QSettings settings( key, QSettings::NativeFormat );
                    settings.setFallbacksEnabled( false );
                    if ( settings.contains( "Q246009" ) )
                        retVal += L"Service Pack 6a (Build " + std::to_wstring( osvi.dwBuildNumber & 0xFFFF ) + L")";
                    else   // Windows NT 4.0 prior to SP6a
                    {
                        retVal += std::wstring( osvi.szCSDVersion ) + L" (Build " + std::to_wstring( osvi.dwBuildNumber & 0xFFFF ) + L")";
                    }
                }
                else   // Windows NT 3.51 and earlier or Windows 2000 and later
                {
                    retVal += std::wstring( osvi.szCSDVersion ) + L" (Build " + std::to_wstring( osvi.dwBuildNumber & 0xFFFF ) + L")";
                }

                break;

            // Test for the Windows 95 product family.
            case VER_PLATFORM_WIN32_WINDOWS:

                if ( osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0 )
                {
                    retVal += L"Microsoft Windows 95 ";
                    if ( osvi.szCSDVersion[ 1 ] == 'C' || osvi.szCSDVersion[ 1 ] == 'B' )
                        retVal += L"OSR2 ";
                }

                if ( osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10 )
                {
                    retVal += L"Microsoft Windows 98 ";
                    if ( osvi.szCSDVersion[ 1 ] == 'A' )
                        retVal += L"SE ";
                }

                if ( osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90 )
                {
                    retVal += L"Microsoft Windows Millennium Edition";
                }
                break;

            case VER_PLATFORM_WIN32s:

                retVal += L"Microsoft Win32s";
                break;
        }

        return NSABUtils::NStringUtils::trim( retVal );
    }

    // returns version, name
    std::pair< std::wstring, std::wstring > getOSInfo()
    {
        OSVERSIONINFOEX osvi;
        ZeroMemory( &osvi, sizeof( OSVERSIONINFOEX ) );
        osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );

        bool versionOK = true;
        if ( !GetVersionEx( (OSVERSIONINFO *)&osvi ) )
        {
            osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
            if ( !GetVersionEx( (OSVERSIONINFO *)&osvi ) )
                versionOK = false;
        }

        std::wstring version;
        std::wstring osName;
        if ( versionOK )
        {
            version = std::to_wstring( osvi.dwMajorVersion ) + L"." + std::to_wstring( osvi.dwMinorVersion ) + L"." + std::to_wstring( osvi.dwBuildNumber );
            osName = getOSName( osvi );
        }
        return { version, osName };
    }

    int getCPUSpeed( size_t logicalProcessorID )
    {
        QString key = R"(HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor)";
        QSettings settings( key, QSettings::NativeFormat );
        settings.setFallbacksEnabled( false );
        QStringList cpus = settings.childGroups();
        int speed = 0;
        if ( cpus.contains( QString::number( logicalProcessorID ) ) )
        {
            settings.beginGroup( QString::number( logicalProcessorID ) );
            speed = settings.value( "~MHz" ).toInt();
        }
        return speed;
    }

    namespace NCPUUtilization
    {
        std::optional< std::pair< void *, void * > > initQuery()
        {
            void *query;
            auto status = PdhOpenQuery( nullptr, NULL, &query );
            if ( status != ERROR_SUCCESS )
                return {};

            void *cpuTotal;
            status = PdhAddCounter( query, L"\\Processor(*)\\% Processor Time", 0, &cpuTotal );
            if ( status != ERROR_SUCCESS )
            {
                qDebug() << "Error (PdhAddCounter): " << status;
                return {};
            }

            status = PdhCollectQueryData( query );
            if ( status != ERROR_SUCCESS )
            {
                qDebug() << "Error (PdhCollectQueryData): " << status;
                return {};
            }

            auto retVal = std::make_pair( query, cpuTotal );

            return retVal;
        }

        void freeQuery( std::pair< void *, void * > &query )
        {
            if ( query.first )
                PdhCloseQuery( query.first );
        }

        std::unordered_map< size_t, double > getCPUCoreUtilizations( const std::pair< void *, void * > &query )
        {
            auto status = PdhCollectQueryData( query.first );
            if ( status != ERROR_SUCCESS )
            {
                qDebug() << "Error (PdhCollectQueryData): " << status;
                return {};
            }

            std::unordered_map< size_t, double > retVal;
            DWORD dwBufferSize = 0;
            DWORD dwItemCount = 0;
            PDH_FMT_COUNTERVALUE_ITEM *pItems = nullptr;
            status = PdhGetFormattedCounterArray( query.second, PDH_FMT_DOUBLE, &dwBufferSize, &dwItemCount, pItems );
            if ( PDH_MORE_DATA != status )
            {
                qDebug() << "Error (PdhGetFormattedCounterArray): " << status;
                return retVal;
            }

            pItems = (PDH_FMT_COUNTERVALUE_ITEM *)new uint8_t[ dwBufferSize ];
            if ( !pItems )
            {
                qDebug() << "Error (new):";
                return retVal;
            }

            if ( pItems )
            {
                status = PdhGetFormattedCounterArray( query.second, PDH_FMT_DOUBLE, &dwBufferSize, &dwItemCount, pItems );
                if ( ERROR_SUCCESS != status )
                {
                    qDebug() << "Error (PdhGetFormattedCounterArray): " << status;
                    return {};
                }

                // Loop through the array and print the instance name and counter value.
                for ( DWORD ii = 0; ii < dwItemCount; ii++ )
                {
                    std::wstring nm = pItems[ ii ].szName;
                    if ( nm == L"_Total" )
                    {
                        retVal[ -1 ] = pItems[ ii ].FmtValue.doubleValue;
                    }
                    else
                    {
                        try
                        {
                            auto cpuID = std::stoll( nm );
                            retVal[ cpuID ] = pItems[ ii ].FmtValue.doubleValue;
                            // wprintf( L"counter: %s, value %.20g\n", pItems[ i ].szName, );
                        }
                        catch ( ... )
                        {
                        }
                    }
                }

                delete pItems;
                pItems = nullptr;
                dwBufferSize = dwItemCount = 0;
            }

            return retVal;
        }

        std::unordered_map< size_t, double > getCPUCoreUtilizations( uint64_t sampleTime )
        {
            auto query = initQuery();

            if ( !query.has_value() || !query.value().first )
                return {};

            Sleep( sampleTime );

            auto cpuUtilization = getCPUCoreUtilizations( query.value() );
            freeQuery( query.value() );
            return cpuUtilization;
        }
    }

    struct SCPUInfo
    {
        void getCPUInfo( size_t &procNum )
        {
            fProcNum = procNum;
            procNum += fNumLogicalCores;

            fUtilization.resize( fNumLogicalCores );
        }

        void updateCPUInfo( const std::unordered_map< size_t, double > &cpuUtilizations )
        {
            for ( size_t ii = fProcNum; ii < fProcNum + fNumLogicalCores; ++ii )
            {
                auto pos = cpuUtilizations.find( ii );
                double utilizationPercentage = 0.0;
                if ( pos != cpuUtilizations.end() )
                    utilizationPercentage = ( *pos ).second;
                fUtilization[ ii - fProcNum ] = utilizationPercentage;
            }
        }

        void dump() const
        {
            std::cout << "Processor ID: 0x" << std::setfill( '0' ) << std::hex << std::setw( 16 ) << fProcID << std::dec << "\n";
            std::cout << "Number of Cores: " << fNumLogicalCores << "\n";
            std::cout << "Has L1: " << ( fHasL1Cache ? "Yes" : "No" ) << "\n";
            std::cout << "Has L2: " << ( fHasL2Cache ? "Yes" : "No" ) << "\n";
            std::cout << "Has L3: " << ( fHasL3Cache ? "Yes" : "No" ) << "\n";
            for ( size_t ii = fProcNum; ii < fProcNum + fNumLogicalCores; ++ii )
            {
                std::cout << "Logical Core Number: " << ii << "\n";
                std::cout << "% Utilization: " << NStringUtils::getPercentageAsString( fUtilization[ ii - fProcNum ] ) << "%\n";
            }
        }
        uint64_t fProcID{ 0 };
        size_t fNumLogicalCores{ 0 };
        DWORD fNumaNodeNumber{ static_cast< DWORD >( -1 ) };
        bool fHasL1Cache{ false };
        bool fHasL2Cache{ false };
        bool fHasL3Cache{ false };
        bool fIsPackage{ false };

        size_t fProcNum{ 0 };
        std::vector< double > fUtilization;
    };

    struct SPackageInfo
    {
        SPackageInfo( SCPUInfo *info ) :
            fProcMask( info->fProcID ),
            fNumaNodeNumber( info->fNumaNodeNumber ),
            fHasL1Cache( info->fHasL1Cache ),
            fHasL2Cache( info->fHasL2Cache ),
            fHasL3Cache( info->fHasL3Cache )
        {
        }

        NODISCARD std::list< uint64_t > findCPUsForPackage( const std::unordered_map< uint64_t, std::shared_ptr< SCPUInfo > > &cpus )
        {
            std::list< uint64_t > retVal;
            for ( auto &&ii : cpus )
            {
                if ( ii.second->fIsPackage )
                    continue;
                if ( ( fProcMask & ii.second->fProcID ) != 0 )
                {
                    fProcessors.push_back( ii.second );
                    retVal.push_back( ii.second->fProcID );
                }
            }
            fProcessors.sort( []( const std::shared_ptr< SCPUInfo > &lhs, const std::shared_ptr< SCPUInfo > &rhs ) { return lhs->fProcID < rhs->fProcID; } );

            return retVal;
        }

        void getCPUInfo( size_t &procNum )
        {
            fSpeed = getCPUSpeed( procNum );

            for ( auto &&ii : fProcessors )
                ii->getCPUInfo( procNum );
        }

        void updateCPUInfo( const std::unordered_map< size_t, double > &cpuUtilizations )
        {
            for ( auto &&ii : fProcessors )
            {
                ii->updateCPUInfo( cpuUtilizations );
            }
            auto pos = cpuUtilizations.find( -1 );
            if ( pos == cpuUtilizations.end() )
                fAvgUtilization = 0.0;
            else
                fAvgUtilization = ( *pos ).second;
        }

        void dump()
        {
            std::cout << "Package: 0x" << std::hex << std::setfill( '0' ) << std::setw( 16 ) << fProcMask << std::dec << "\n";
            std::cout << "Numa Node Number: " << fNumaNodeNumber << "\n";
            std::cout << "Has L1: " << ( fHasL1Cache ? "Yes" : "No" ) << "\n";
            std::cout << "Has L2: " << ( fHasL2Cache ? "Yes" : "No" ) << "\n";
            std::cout << "Has L3: " << ( fHasL3Cache ? "Yes" : "No" ) << "\n";

            std::cout << "Speed: " << fSpeed << "\n";

            for ( auto &&ii : fProcessors )
                ii->dump();
        }

        uint64_t fProcMask{ 0 };
        DWORD fNumaNodeNumber{ static_cast< DWORD >( -1 ) };
        bool fHasL1Cache{ false };
        bool fHasL2Cache{ false };
        bool fHasL3Cache{ false };

        int fSpeed{ 0 };
        double fAvgUtilization{ 0.0 };

        std::list< std::shared_ptr< SCPUInfo > > fProcessors;   // only valid when it is a package
    };

    NODISCARD std::list< std::shared_ptr< SPackageInfo > > getCPUs()
    {
        DWORD size = 0;
        auto rc = GetLogicalProcessorInformation( nullptr, &size );
        if ( !rc )
        {
            if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
                return {};
        }

        auto buffer = ( PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ) new uint8_t[ size ];
        if ( nullptr == buffer )
            return {};

        rc = GetLogicalProcessorInformation( buffer, &size );
        if ( !rc )
            return {};

        std::unordered_map< uint64_t, std::shared_ptr< SCPUInfo > > cpus;

        std::list< std::shared_ptr< SPackageInfo > > packages;
        int numRecs = size / sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION );
        for ( int ii = 0; ii < numRecs; ++ii )
        {
            auto &&ptr = buffer[ ii ];

            std::shared_ptr< SCPUInfo > currCPU;
            auto pos = cpus.find( ptr.ProcessorMask );
            if ( pos == cpus.end() )
            {
                currCPU = std::make_shared< SCPUInfo >();
                currCPU->fProcID = ptr.ProcessorMask;
                cpus[ ptr.ProcessorMask ] = currCPU;
            }
            else
                currCPU = ( *pos ).second;

            switch ( ptr.Relationship )
            {
                case RelationProcessorCore:
                    {
                        // A hyperthreaded core supplies more than one logical processor.
                        std::bitset< sizeof( ULONG_PTR ) * 8 > bitset( ptr.ProcessorMask );
                        currCPU->fNumLogicalCores = bitset.count();
                        break;
                    }
                case RelationNumaNode:
                    // Non-NUMA systems report a single record of this type.
                    currCPU->fNumaNodeNumber = ptr.NumaNode.NodeNumber;
                    break;

                case RelationCache:
                    {
                        // Cache data is in ptr.Cache, one CACHE_DESCRIPTOR structure for each cache.
                        auto &&cache = ptr.Cache;
                        if ( cache.Level == 1 )
                        {
                            currCPU->fHasL1Cache = true;
                        }
                        else if ( cache.Level == 2 )
                        {
                            currCPU->fHasL2Cache = true;
                        }
                        else if ( cache.Level == 3 )
                        {
                            currCPU->fHasL3Cache = true;
                        }
                    }
                    break;

                case RelationProcessorPackage:
                    // Logical processors share a physical package.
                    currCPU->fIsPackage = true;
                    break;

                default:
                    break;
            }
        }
        std::list< uint64_t > toRemove;
        for ( auto &&ii : cpus )
        {
            if ( ii.second->fIsPackage )
            {
                auto package = std::make_shared< SPackageInfo >( ii.second.get() );
                toRemove.push_back( ii.first );

                packages.push_back( package );
                auto usedCPUs = package->findCPUsForPackage( cpus );
                toRemove.insert( toRemove.end(), usedCPUs.begin(), usedCPUs.end() );
            }
        }
        for ( auto jj : toRemove )
        {
            auto pos = cpus.find( jj );
            if ( pos != cpus.end() )
                cpus.erase( pos );
        }
        Q_ASSERT( cpus.empty() );

        packages.sort( []( const std::shared_ptr< SPackageInfo > &lhs, const std::shared_ptr< SPackageInfo > &rhs ) { return lhs->fProcMask < rhs->fProcMask; } );

        delete[] buffer;

        size_t currProcNum = 0;
        for ( auto &&ii : packages )
        {
            ii->getCPUInfo( currProcNum );
        }
        return packages;
    }

    std::unordered_map< std::string, std::list< std::unordered_map< std::string, std::string > > > GetSystemInfo()
    {
        std::unordered_map< std::string, std::list< std::unordered_map< std::string, std::string > > > retVal;

        auto cpuUtilizations = NCPUUtilization::getCPUCoreUtilizations();
        auto cpus = getCPUs();
        for ( auto &&ii : cpus )
            ii->updateCPUInfo( cpuUtilizations );

        std::list< std::unordered_map< std::string, std::string > > maps;
        int cpuNum = 0;
        for ( auto &&ii : cpus )
        {
            QLocale locale;

            std::unordered_map< std::string, std::string > currMap;
            currMap[ "NumberOfProcessors" ] = std::to_string( ii->fProcessors.size() );
            currMap[ "NumaNode" ] = std::to_string( ii->fNumaNodeNumber );
            currMap[ "HasL1Cache" ] = std::to_string( ii->fHasL1Cache );
            currMap[ "HasL2Cache" ] = std::to_string( ii->fHasL2Cache );
            currMap[ "HasL3Cache" ] = std::to_string( ii->fHasL3Cache );
            currMap[ "Frequency" ] = std::to_string( ii->fSpeed );
            currMap[ "%Utilization" ] = NStringUtils::getPercentageAsString( ii->fAvgUtilization );
            size_t totalCores = 0;
            for ( auto &&jj : ii->fProcessors )
            {
                auto keyBase = "Proc" + std::to_string( jj->fProcNum );

                totalCores += jj->fNumLogicalCores;
                currMap[ keyBase + "NumberOfCores" ] = std::to_string( jj->fNumLogicalCores );
                currMap[ keyBase + "HasL1Cache" ] = std::to_string( jj->fHasL1Cache );
                currMap[ keyBase + "HasL2Cache" ] = std::to_string( jj->fHasL2Cache );
                currMap[ keyBase + "HasL3Cache" ] = std::to_string( jj->fHasL3Cache );
                for ( size_t kk = jj->fProcNum; kk < jj->fProcNum + jj->fNumLogicalCores; ++kk )
                {
                    currMap[ keyBase + "Core" + std::to_string( kk ) + "%Util" ] = NStringUtils::getPercentageAsString( jj->fUtilization[ kk - jj->fProcNum ] );
                }
            }
            currMap[ "NumberOfCores" ] = std::to_string( totalCores );
            cpuNum++;
            maps.push_back( currMap );
        }

        retVal[ "CPU" ] = maps;

        std::unordered_map< std::string, std::string > currMap;
        maps.clear();

        std::wstring version;
        std::wstring osName;
        std::tie( version, osName ) = getOSInfo();

        currMap[ "Version" ] = std::string( { version.begin(), version.end() } );
        currMap[ "Name" ] = std::string( { osName.begin(), osName.end() } );

        quint64 tc = GetTickCount();
        QDateTime tcTime = QDateTime::currentDateTime();
        tcTime = tcTime.addMSecs( -1 * tc );

        currMap[ "LastBootUpTime" ] = tcTime.toString().toStdString();

        auto tz = QTimeZone::systemTimeZone();
        QDateTime local = QDateTime::currentDateTime();

        auto tzMins = ( tz.offsetFromUtc( local ) / 60 );
        bool neg = tzMins < 0;
        if ( neg )
            tzMins *= -1;
        auto tzHours = tzMins / 60;
        tzMins = tzMins - ( tzHours * 60 );
        std::ostringstream oss;
        oss << tz.id().toStdString() << " ";
        if ( neg )
            oss << "-";
        oss << std::fixed << std::setw( 2 ) << std::setfill( '0' ) << tzHours << ":" << std::fixed << std::setw( 2 ) << std::setfill( '0' ) << tzMins;

        currMap[ "CurrentTimeZone" ] = oss.str();
        maps.push_back( currMap );
        retVal[ "OS" ] = maps;
        return retVal;
    }

    std::string CSystemInfo::getOSName()
    {
        auto retVal = getOSInfo().second;
        return std::string( retVal.begin(), retVal.end() );
    }

    std::string CSystemInfo::getOSVersion()
    {
        auto retVal = getOSInfo().first;
        return std::string( retVal.begin(), retVal.end() );
    }

    void CSystemInfo::LoadSystemInfo()
    {
        // std::unordered_map< std::string, std::list< std::unordered_map< std::string, std::string > > > GetSystemInfo()
        auto data = GetSystemInfo();

        auto pos = data.find( "OS" );
        if ( ( pos != data.end() ) && !( *pos ).second.empty() )
        {
            auto osName = ( *pos ).second.front()[ "Name" ];
            auto pipeIdx = osName.find( '|' );
            if ( pipeIdx != std::string::npos )
                osName = osName.substr( 0, pipeIdx );
            fSystemInformation.push_back( { std::string( "OS" ), osName } );
            fSystemInformation.push_back( { "OS Version", data[ "OS" ].front()[ "Version" ] } );
            fSystemInformation.push_back( { "Boot Time", data[ "OS" ].front()[ "LastBootUpTime" ] } );
            fSystemInformation.push_back( { "Time Zone", data[ "OS" ].front()[ "CurrentTimeZone" ] } );
        }

        pos = data.find( "CPU" );
        if ( ( pos != data.end() ) && !( *pos ).second.empty() )
        {
            int cpuNum = 0;
            for ( auto &&ii : ( *pos ).second )
            {
                std::ostringstream oss;
                oss << ii[ "NumberOfCores" ] << " Core(s) - " << ii[ "Frequency" ] << " Frequency (Mhz) - " << ii[ "%Utilization" ] << "% Utilization";
                fSystemInformation.push_back( { "CPU" + std::to_string( cpuNum++ ), oss.str() } );
            }
        }
    }

    void CSystemInfo::LoadMemoryInfo( bool baseInfo )
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof( statex );
        QLocale locale;

        fSystemMemoryData.clear();
        if ( GlobalMemoryStatusEx( &statex ) )
        {
            fSystemMemoryData.push_back( { "Total Physical", locale.toString( statex.ullTotalPhys ).toStdString() } );
            fSystemMemoryData.push_back( { "Available Physical", locale.toString( statex.ullAvailPhys ).toStdString() } );
            fSystemMemoryData.push_back( { "Total Page File", locale.toString( statex.ullTotalPageFile ).toStdString() } );
            fSystemMemoryData.push_back( { "Available Page File", locale.toString( statex.ullAvailPageFile ).toStdString() } );
            fSystemMemoryData.push_back( { "Total Virtual", locale.toString( statex.ullTotalVirtual ).toStdString() } );
            fSystemMemoryData.push_back( { "Available Virtual", locale.toString( statex.ullAvailVirtual ).toStdString() } );
            fSystemMemoryData.push_back( { "Available Extended Virtual", locale.toString( statex.ullAvailExtendedVirtual ).toStdString() } );
        }

        fApplicationMemoryData.clear();
        PROCESS_MEMORY_COUNTERS pmc;
        if ( GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof( pmc ) ) )
        {
            fApplicationMemoryData.push_back( { "Application Memory Size", locale.toString( (qlonglong)pmc.WorkingSetSize ).toStdString() } );
            if ( baseInfo )
                sBaseApplicationMemory = locale.toString( (qlonglong)pmc.WorkingSetSize ).toStdString();

            fApplicationMemoryData.push_back( { "Peak Application Memory Size", locale.toString( (qlonglong)pmc.PeakWorkingSetSize ).toStdString() } );
            fApplicationMemoryData.push_back( { "Page Fault Count", locale.toString( (qlonglong)pmc.PageFaultCount ).toStdString() } );
            fApplicationMemoryData.push_back( { "Quota Peak Paged Pool Usage", locale.toString( (qlonglong)pmc.QuotaPeakPagedPoolUsage ).toStdString() } );
            fApplicationMemoryData.push_back( { "Quota Paged Pool Usage", locale.toString( (qlonglong)pmc.QuotaPagedPoolUsage ).toStdString() } );
            fApplicationMemoryData.push_back( { "Quota Peak Non Paged Pool Usage", locale.toString( (qlonglong)pmc.QuotaPeakNonPagedPoolUsage ).toStdString() } );
            fApplicationMemoryData.push_back( { "Quota Non Paged Pool Usage", locale.toString( (qlonglong)pmc.QuotaNonPagedPoolUsage ).toStdString() } );
            fApplicationMemoryData.push_back( { "Pagefile Usage", locale.toString( (qlonglong)pmc.PagefileUsage ).toStdString() } );
            fApplicationMemoryData.push_back( { "Peak Pagefile Usage", locale.toString( (qlonglong)pmc.PeakPagefileUsage ).toStdString() } );
        }
    }

    void CSystemInfo::LoadNICInfo()
    {
        auto nics = GetAllNICDesc();
        for ( auto &&nic : nics )
        {
            SNicInfo curr;
            curr.fIPAddr = nic[ "IP0" ];   // note might have more then 1 IP
            curr.fMacAddr = nic[ "MAC" ];
            curr.fName = nic[ "Description" ];

            if ( curr.fIPAddr.empty() || curr.fMacAddr.empty() )
                continue;
            fNics.emplace_back( curr );
        }
    }

}
