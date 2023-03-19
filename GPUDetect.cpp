#include "GPUDetect.h"

#define _WIN32_DCOM
#include <qt_windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment( lib, "wbemuuid.lib" )

namespace NSABUtils
{
    std::list< std::shared_ptr< CGPUInfo > > detectGPUs( QString *errorMsg /*= nullptr*/ )
    {
        // Step 1: --------------------------------------------------
        // Initialize COM. ------------------------------------------

        // Step 2: --------------------------------------------------
        // Set general COM security levels --------------------------

        auto hres = CoInitializeSecurity(
            nullptr,
            -1,   // COM authentication
            nullptr,   // Authentication services
            nullptr,   // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
            RPC_C_IMP_LEVEL_IMPERSONATE,   // Default Impersonation
            nullptr,   // Authentication info
            EOAC_NONE,   // Additional capabilities
            nullptr   // Reserved
        );

        bool needCoUnit = false;
        if ( hres == CO_E_NOTINITIALIZED )
        {
            hres = CoInitializeEx( 0, COINIT_MULTITHREADED );
            if ( FAILED( hres ) )
            {
                if ( errorMsg )
                    *errorMsg = QString( "Failed to initialize COM library. Error code = 0x" ).arg( hres, 0, 16 );
                return {};
            }
            needCoUnit = true;
            hres = CoInitializeSecurity(
                nullptr,
                -1,   // COM authentication
                nullptr,   // Authentication services
                nullptr,   // Reserved
                RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
                RPC_C_IMP_LEVEL_IMPERSONATE,   // Default Impersonation
                nullptr,   // Authentication info
                EOAC_NONE,   // Additional capabilities
                nullptr   // Reserved
            );
        }

        if ( ( hres != RPC_E_TOO_LATE) && FAILED( hres ) )
        {
            if ( errorMsg )
                *errorMsg = QString( "Failed to initialize security. Error code = 0x" ).arg( hres, 0, 16 );
            if ( needCoUnit )
                CoUninitialize();
            return {};
        }

        // Step 3: ---------------------------------------------------
        // Obtain the initial locator to WMI -------------------------

        IWbemLocator *pLoc = nullptr;
        hres = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc );

        if ( FAILED( hres ) )
        {
            if ( errorMsg )
                *errorMsg = QString( "Failed to create IWbemLocator object. Error code = 0x" ).arg( hres, 0, 16 );
            if ( needCoUnit )
                CoUninitialize();
            return {};   // Program has failed.
        }

        // Step 4: -----------------------------------------------------
        // Connect to WMI through the IWbemLocator::ConnectServer method
        IWbemServices *pSvc = nullptr;

        // Connect to the root\cimv2 namespace with
        // the current user and obtain pointer pSvc
        // to make IWbemServices calls.
        hres = pLoc->ConnectServer(
            _bstr_t( L"ROOT\\CIMV2" ),   // Object path of WMI namespace
            nullptr,   // User name. nullptr = current user
            nullptr,   // User password. nullptr = current
            0,   // Locale. nullptr indicates current
            0,   // Security flags.
            0,   // Authority (for example, Kerberos)
            0,   // Context object
            &pSvc   // pointer to IWbemServices proxy
        );

        if ( FAILED( hres ) )
        {
            if ( errorMsg )
                *errorMsg = QString( "Failed to create connect. Error code = 0x" ).arg( hres, 0, 16 );
            pLoc->Release();
            if ( needCoUnit )
                CoUninitialize();
            return {};   // Program has failed.
        }

        //cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;

        // Step 5: --------------------------------------------------
        // Set security levels on the proxy -------------------------

        hres = CoSetProxyBlanket(
            pSvc,   // Indicates the proxy to set
            RPC_C_AUTHN_WINNT,   // RPC_C_AUTHN_xxx
            RPC_C_AUTHZ_NONE,   // RPC_C_AUTHZ_xxx
            nullptr,   // Server principal name
            RPC_C_AUTHN_LEVEL_CALL,   // RPC_C_AUTHN_LEVEL_xxx
            RPC_C_IMP_LEVEL_IMPERSONATE,   // RPC_C_IMP_LEVEL_xxx
            nullptr,   // client identity
            EOAC_NONE   // proxy capabilities
        );

        if ( FAILED( hres ) )
        {
            if ( errorMsg )
                *errorMsg = QString( "Failed to set proxy blanket. Error code = 0x" ).arg( hres, 0, 16 );
            pSvc->Release();
            pLoc->Release();
            if ( needCoUnit )
                CoUninitialize();
            return {};   // Program has failed.
        }

        // Step 6: --------------------------------------------------
        // Use the IWbemServices pointer to make requests of WMI ----

        // For example, get the name of the operating system
        IEnumWbemClassObject *pEnumerator = nullptr;
        hres = pSvc->ExecQuery( bstr_t( "WQL" ), bstr_t( "SELECT * FROM Win32_VideoController" ), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator );

        if ( FAILED( hres ) )
        {
            if ( errorMsg )
                *errorMsg = QString( "Failed to query for video controllers. Error code = 0x" ).arg( hres, 0, 16 );
            pSvc->Release();
            pLoc->Release();
            if ( needCoUnit )
                CoUninitialize();
            return {};   // Program has failed.
        }

        // Step 7: -------------------------------------------------
        // Get the data from the query in step 6 -------------------

        IWbemClassObject *gpuInfo = nullptr;
        ULONG uReturn = 0;

        std::list< std::shared_ptr< CGPUInfo > > retVal;
        while ( pEnumerator )
        {
            HRESULT hr = pEnumerator->Next( WBEM_INFINITE, 1, &gpuInfo, &uReturn );
            if ( FAILED( hr ) )
                break;

            if ( 0 == uReturn )
                break;

            retVal.push_back( std::make_shared< CGPUInfo >( gpuInfo ) );
            gpuInfo->Release();
        }

        // Cleanup
        // ========

        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        if ( needCoUnit )
            CoUninitialize();

        return retVal;   // Program successfully completed.
    }

    //std::wstring dumpVariant( VARIANT vtProp )
    //{
    //    switch ( vtProp.vt )
    //    {
    //        case VT_I4:
    //            return std::to_wstring( vtProp.lVal );
    //        case VT_LPSTR:
    //        case VT_LPWSTR:
    //        case VT_BSTR:
    //            return vtProp.bstrVal;
    //        case VT_BOOL:
    //            return vtProp.boolVal ? L"true" : L"false";
    //        case VT_EMPTY:
    //        case VT_NULL:
    //            return {};

    //        case VT_I2:
    //        case VT_I8:
    //        case VT_R4:
    //        case VT_R8:
    //        case VT_CY:
    //        case VT_DATE:
    //        case VT_DISPATCH:
    //        case VT_ERROR:
    //        case VT_VARIANT:
    //        case VT_UNKNOWN:
    //        case VT_DECIMAL:
    //        case VT_I1:
    //        case VT_UI1:
    //        case VT_UI2:
    //        case VT_UI4:
    //        case VT_UI8:
    //        case VT_INT:
    //        case VT_UINT:
    //        case VT_VOID:
    //        case VT_HRESULT:
    //        case VT_PTR:
    //        case VT_SAFEARRAY:
    //        case VT_CARRAY:
    //        case VT_USERDEFINED:
    //        case VT_LPSTR:
    //        case VT_LPWSTR:
    //        case VT_RECORD:
    //        case VT_INT_PTR:
    //        case VT_UINT_PTR:
    //        case VT_FILETIME:
    //        case VT_BLOB:
    //        case VT_STREAM:
    //        case VT_STORAGE:
    //        case VT_STREAMED_OBJECT:
    //        case VT_STORED_OBJECT:
    //        case VT_BLOB_OBJECT:
    //        case VT_CF:
    //        case VT_CLSID:
    //        case VT_VERSIONED_STREAM:
    //        case VT_BSTR_BLOB:
    //        case VT_VECTOR:
    //        case VT_ARRAY:
    //        case VT_BYREF:
    //        case VT_RESERVED:
    //        default:
    //            return L"<UNKNOWN>";
    //    }
    //}

    void dumpValue( IWbemClassObject *gpuInfo, wchar_t * /*desc*/, wchar_t *key )
    {
        VARIANT vtProp;

        VariantInit( &vtProp );
        // Get the value of the Name property
        auto hr = gpuInfo->Get( key, 0, &vtProp, 0, 0 );
        if ( FAILED( hr ) )
        {
            //wcerr << "Error getting key: " << key << endl;
            return;
        }

        //wcout << desc << dumpVariant( vtProp ) << endl;
        VariantClear( &vtProp );
    }

    

    void loadValue( IWbemClassObject *gpuInfo, wchar_t *key, bool &value )
    {
        value = false;

        VARIANT vtProp;

        VariantInit( &vtProp );
        // Get the value of the Name property
        auto hr = gpuInfo->Get( key, 0, &vtProp, 0, 0 );
        if ( FAILED( hr ) )
        {
            //wcerr << "Error getting key: " << key << endl;
            return;
        }

        if ( ( vtProp.vt == VT_NULL ) || ( vtProp.vt == VT_EMPTY ) )
        {
            return;
        }
        Q_ASSERT( vtProp.vt == VT_BOOL );
        value = vtProp.boolVal;

        VariantClear( &vtProp );
    }

    void loadValue( IWbemClassObject *gpuInfo, wchar_t *key, uint32_t &value )
    {
        value = 0;

        VARIANT vtProp;

        VariantInit( &vtProp );
        // Get the value of the Name property
        auto hr = gpuInfo->Get( key, 0, &vtProp, 0, 0 );
        if ( FAILED( hr ) )
        {
            //wcerr << "Error getting key: " << key << endl;
            return;
        }

        if ( ( vtProp.vt == VT_NULL ) || ( vtProp.vt == VT_EMPTY ) )
        {
            return;
        }
        Q_ASSERT( vtProp.vt == VT_I4 );
        value = vtProp.lVal;

        VariantClear( &vtProp );
    }

    void loadValue( IWbemClassObject *gpuInfo, wchar_t *key, QString &value )
    {
        value.clear();
        VARIANT vtProp;

        VariantInit( &vtProp );
        // Get the value of the Name property
        auto hr = gpuInfo->Get( key, 0, &vtProp, 0, 0 );
        if ( FAILED( hr ) )
        {
            //wcerr << "Error getting key: " << key << endl;
            return;
        }

        if ( ( vtProp.vt == VT_NULL ) || ( vtProp.vt == VT_EMPTY ) )
        {
            return;
        }
        value = QString::fromUtf16( reinterpret_cast< ushort * >( vtProp.bstrVal ) );

        VariantClear( &vtProp );
    }

    CGPUInfo::CGPUInfo( IWbemClassObject *gpuInfo )
    {
        if ( !gpuInfo )
            return;

        loadValue( gpuInfo, L"AcceleratorCapabilities", fAcceleratorCapabilities );
        loadValue( gpuInfo, L"AdapterCompatibility", fAdapterCompatibility );
        loadValue( gpuInfo, L"AdapterDACType", fAdapterDACType );
        loadValue( gpuInfo, L"AdapterRAM", fAdapterRAM );
        loadValue( gpuInfo, L"Availability", fAvailability );
        loadValue( gpuInfo, L"CapabilityDescriptions", fCapabilityDescriptions );
        loadValue( gpuInfo, L"Caption", fCaption );
        loadValue( gpuInfo, L"ColorTableEntries", fColorTableEntries );
        loadValue( gpuInfo, L"ConfigManagerErrorCode", fConfigManagerErrorCode );
        loadValue( gpuInfo, L"ConfigManagerUserConfig", fConfigManagerUserConfig );
        loadValue( gpuInfo, L"CreationClassName", fCreationClassName );
        loadValue( gpuInfo, L"CurrentBitsPerPixel", fCurrentBitsPerPixel );
        loadValue( gpuInfo, L"CurrentHorizontalResolution", fCurrentHorizontalResolution );
        loadValue( gpuInfo, L"CurrentNumberOfColors", fCurrentNumberOfColors );
        loadValue( gpuInfo, L"CurrentNumberOfColumns", fCurrentNumberOfColumns );
        loadValue( gpuInfo, L"CurrentNumberOfRows", fCurrentNumberOfRows );
        loadValue( gpuInfo, L"CurrentRefreshRate", fCurrentRefreshRate );
        loadValue( gpuInfo, L"CurrentScanMode", fCurrentScanMode );
        loadValue( gpuInfo, L"CurrentVerticalResolution", fCurrentVerticalResolution );
        loadValue( gpuInfo, L"Description", fDescription );
        loadValue( gpuInfo, L"DeviceID", fDeviceID );
        loadValue( gpuInfo, L"DeviceSpecificPens", fDeviceSpecificPens );
        loadValue( gpuInfo, L"DitherType", fDitherType );
        loadValue( gpuInfo, L"DriverDate", fDriverDate );
        loadValue( gpuInfo, L"DriverVersion", fDriverVersion );
        loadValue( gpuInfo, L"ErrorCleared", fErrorCleared );
        loadValue( gpuInfo, L"ErrorDescription", fErrorDescription );
        loadValue( gpuInfo, L"ICMIntent", fICMIntent );
        loadValue( gpuInfo, L"ICMMethod", fICMMethod );
        loadValue( gpuInfo, L"InfFilename", fInfFilename );
        loadValue( gpuInfo, L"InfSection", fInfSection );
        loadValue( gpuInfo, L"InstallDate", fInstallDate );
        loadValue( gpuInfo, L"InstalledDisplayDrivers", fInstalledDisplayDrivers );
        loadValue( gpuInfo, L"LastErrorCode", fLastErrorCode );
        loadValue( gpuInfo, L"MaxMemorySupported", fMaxMemorySupported );
        loadValue( gpuInfo, L"MaxNumberControlled", fMaxNumberControlled );
        loadValue( gpuInfo, L"MaxRefreshRate", fMaxRefreshRate );
        loadValue( gpuInfo, L"MinRefreshRate", fMinRefreshRate );
        loadValue( gpuInfo, L"Monochrome", fMonochrome );
        loadValue( gpuInfo, L"Name", fName );
        loadValue( gpuInfo, L"NumberOfColorPlanes", fNumberOfColorPlanes );
        loadValue( gpuInfo, L"NumberOfVideoPages", fNumberOfVideoPages );
        loadValue( gpuInfo, L"PNPDeviceID", fPNPDeviceID );
        loadValue( gpuInfo, L"PowerManagementCapabilities", fPowerManagementCapabilities );
        loadValue( gpuInfo, L"PowerManagementSupported", fPowerManagementSupported );
        loadValue( gpuInfo, L"ProtocolSupported", fProtocolSupported );
        loadValue( gpuInfo, L"ReservedSystemPaletteEntries", fReservedSystemPaletteEntries );
        loadValue( gpuInfo, L"SpecificationVersion", fSpecificationVersion );
        loadValue( gpuInfo, L"Status", fStatus );
        loadValue( gpuInfo, L"StatusInfo", fStatusInfo );
        loadValue( gpuInfo, L"SystemCreationClassName", fSystemCreationClassName );
        loadValue( gpuInfo, L"SystemName", fSystemName );
        loadValue( gpuInfo, L"SystemPaletteEntries", fSystemPaletteEntries );
        loadValue( gpuInfo, L"TimeOfLastReset", fTimeOfLastReset );
        loadValue( gpuInfo, L"VideoArchitecture", fVideoArchitecture );
        loadValue( gpuInfo, L"VideoMemoryType", fVideoMemoryType );
        loadValue( gpuInfo, L"VideoMode", fVideoMode );
        loadValue( gpuInfo, L"VideoModeDescription", fVideoModeDescription );
        loadValue( gpuInfo, L"VideoProcessor", fVideoProcessor );
    }

    bool CGPUInfo::isIntelGPU() const
    {
        return 
            fVideoProcessor.contains( "Intel", Qt::CaseSensitivity::CaseInsensitive )   //
               || fName.contains( "Intel", Qt::CaseSensitivity::CaseInsensitive )   //
               || fDescription.contains( "Intel", Qt::CaseSensitivity::CaseInsensitive )   //
               || fCaption.contains( "Intel", Qt::CaseSensitivity::CaseInsensitive )   //
               || fAdapterCompatibility.contains( "Intel", Qt::CaseSensitivity::CaseInsensitive );
    }

    bool CGPUInfo::isNVidiaGPU() const
    {
        return fVideoProcessor.contains( "NVIDIA", Qt::CaseSensitivity::CaseInsensitive )   //
               || fName.contains( "NVIDIA", Qt::CaseSensitivity::CaseInsensitive )   //
               || fDescription.contains( "NVIDIA", Qt::CaseSensitivity::CaseInsensitive )   //
               || fCaption.contains( "NVIDIA", Qt::CaseSensitivity::CaseInsensitive )   //
               || fAdapterCompatibility.contains( "NVIDIA", Qt::CaseSensitivity::CaseInsensitive );
    }

    bool CGPUInfo::isAMDGPU() const
    {
        return fVideoProcessor.contains( "AMD", Qt::CaseSensitivity::CaseInsensitive )   //
               || fName.contains( "AMD", Qt::CaseSensitivity::CaseInsensitive )   //
               || fDescription.contains( "AMD", Qt::CaseSensitivity::CaseInsensitive )   //
               || fCaption.contains( "AMD", Qt::CaseSensitivity::CaseInsensitive )   //
               || fAdapterCompatibility.contains( "AMD", Qt::CaseSensitivity::CaseInsensitive );
    }

    SGPUInfo::SGPUInfo( QString *errorMsg /*= nullptr */ )
    {
        auto gpus = NSABUtils::detectGPUs( errorMsg );
        bool hasIntel = false;
        bool hasAMD = false;
        bool hasNVidia = false;
        for ( auto &&ii : gpus )
        {
            if ( ii->isIntelGPU() )
            {
                hasIntel = true;
            }
            else if ( ii->isNVidiaGPU() )
            {
                hasNVidia = true;
            }
            else if ( ii->isAMDGPU() )
            {
                hasAMD = true;
            }
        }
    }
}
