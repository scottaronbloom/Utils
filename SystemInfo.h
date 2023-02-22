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

#ifndef __SYSTEMINFO_H
#define __SYSTEMINFO_H

#include "SABUtilsExport.h"

#include <unordered_map>
#include <string>
#include <optional>
#include <list>
class QJsonArray;

namespace NSABUtils
{
    struct SNicInfo
    {
        std::string fName;
        std::string fMacAddr;
        std::string fIPAddr;
    };

    namespace NCPUUtilization
    {
        std::optional< std::pair< void *, void * > > SABUTILS_EXPORT initQuery();   // returns the handle to the query and the specific counter, the first needs to have free query called on it
        void SABUTILS_EXPORT freeQuery( std::pair< void *, void * > &query );

        std::unordered_map< size_t, double > SABUTILS_EXPORT getCPUCoreUtilizations( uint64_t sampleTime = 1000 );   // returns a map of logical processor id to %utilization over a 1000 msec sample
        std::unordered_map< size_t, double > SABUTILS_EXPORT getCPUCoreUtilizations( const std::pair< void *, void * > &query );
    }

    class SABUTILS_EXPORT CSystemInfo
    {
    public:
        CSystemInfo( bool baseSettings = false );
        enum class EFormat
        {
            eText,
            eHtml,
            eJSON
        };
        std::string getText( EFormat format = EFormat::eText, bool memoryOnly = false, bool showSystemMemory = true ) const;

        static std::string getOSName();
        static std::string getOSVersion();
        std::string getPrimaryHostID() const;

        static bool hasSystemInfo();

    private:
        std::list< std::pair< std::string, std::string > > fSystemMemoryData;
        std::list< std::pair< std::string, std::string > > fApplicationMemoryData;
        std::list< std::pair< std::string, std::string > > fSystemInformation;

        std::list< SNicInfo > fNics;

    private:
        std::string dumpTable( const std::list< std::pair< std::string, std::string > > &table, const std::string &title, const std::pair< std::string, std::string > &subTitle, EFormat format, QJsonArray &jsonParentArray ) const;
        std::string dumpNics( const std::list< SNicInfo > &nics, const std::string &title, EFormat format, QJsonArray &jsonParentArray ) const;
        void Load();
        void LoadSystemInfo();
        void LoadMemoryInfo( bool baseInfo );
        void LoadNICInfo();

        static std::string sBaseApplicationMemory;
    };
}

#endif
