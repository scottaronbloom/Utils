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

#ifndef __GPUDETECT_H
#define __GPUDETECT_H

#include "SABUtilsExport.h"

#include <QString>
#include <list>
#include <memory>
#include <cinttypes>

class IWbemClassObject;
namespace NSABUtils
{
    class SABUTILS_EXPORT CGPUInfo
    {
    public:
        CGPUInfo(){};
        CGPUInfo( IWbemClassObject *gpuInfo );

        bool isIntelGPU() const;
        bool isNVidiaGPU() const;

    private:
        QString fAcceleratorCapabilities;
        QString fAdapterCompatibility;
        QString fAdapterDACType;
        uint32_t fAdapterRAM{ 0 };
        uint32_t fAvailability{ 0 };
        QString fCapabilityDescriptions;
        QString fCaption;
        QString fColorTableEntries;
        uint32_t fConfigManagerErrorCode{ 0 };
        bool fConfigManagerUserConfig{ false };
        QString fCreationClassName;
        uint32_t fCurrentBitsPerPixel{ 0 };
        uint32_t fCurrentHorizontalResolution{ 0 };
        QString fCurrentNumberOfColors;
        uint32_t fCurrentNumberOfColumns{ 0 };
        uint32_t fCurrentNumberOfRows{ 0 };
        uint32_t fCurrentRefreshRate{ 0 };
        uint32_t fCurrentScanMode{ 0 };
        uint32_t fCurrentVerticalResolution{ 0 };
        QString fDescription;
        QString fDeviceID;
        QString fDeviceSpecificPens;
        uint32_t fDitherType{ 0 };
        QString fDriverDate;
        QString fDriverVersion;
        QString fErrorCleared;
        QString fErrorDescription;
        QString fICMIntent;
        QString fICMMethod;
        QString fInfFilename;
        QString fInfSection;
        QString fInstallDate;
        QString fInstalledDisplayDrivers;
        QString fLastErrorCode;
        QString fMaxMemorySupported;
        QString fMaxNumberControlled;
        uint32_t fMaxRefreshRate{ 0 };
        uint32_t fMinRefreshRate{ 0 };
        bool fMonochrome{ false };
        QString fName;
        QString fNumberOfColorPlanes;
        QString fNumberOfVideoPages;
        QString fPNPDeviceID;
        QString fPowerManagementCapabilities;
        QString fPowerManagementSupported;
        QString fProtocolSupported;
        QString fReservedSystemPaletteEntries;
        QString fSpecificationVersion;
        QString fStatus;
        QString fStatusInfo;
        QString fSystemCreationClassName;
        QString fSystemName;
        QString fSystemPaletteEntries;
        QString fTimeOfLastReset;
        uint32_t fVideoArchitecture{ 0 };
        uint32_t fVideoMemoryType{ 0 };
        QString fVideoMode;
        QString fVideoModeDescription;
        QString fVideoProcessor;
    };

    SABUTILS_EXPORT std::list< std::shared_ptr< CGPUInfo > > detectGPUs( QString *errorMsg = nullptr );
};

#endif
