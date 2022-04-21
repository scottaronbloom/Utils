/*
 * (c) Copyright 2004 - 2019 Blue Pearl Software Inc.
 * All rights reserved.
 *
 * This source code belongs to Blue Pearl Software Inc.
 * It is considered trade secret and confidential, and is not to be used
 * by parties who have not received written authorization
 * from Blue Pearl Software Inc.
 *
 * Only authorized users are allowed to use, copy and modify
 * this software provided that the above copyright notice
 * remains in all copies of this software.
 *
 *
 * $Author: scott $ - $Revision: 51872 $ - $Date: 2019-03-25 10:59:32 -0700 (Mon, 25 Mar 2019) $
 * $HeadURL: http://bpsvn/svn/trunk/Shared/SystemInfo/SystemInfo_linux.cpp $
 *
 *
*/
#include "SystemInfo.h"
#include <sys/sysinfo.h>
#include <iostream>
#include <ios>
#include <fstream>
#include <string>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/utsname.h>

#include "qt_prolog.h"
#include <QLocale>
#include "qt_epilog.h"

namespace NSystemInfo
{
    bool SSystemInfo::hasSystemInfo()
    {
        return true;
    }

    void SSystemInfo::LoadMemoryInfo( bool baseInfo )
    {
        QLocale locale;
        struct sysinfo info;
        if ( sysinfo( &info ) != 0 )
            return;
        
        fSystemMemoryData.push_back( QPair< QString, QString >( "Total Physical", locale.toString( (qlonglong)info.totalram * info.mem_unit) ) );
        fSystemMemoryData.push_back( QPair< QString, QString >( "Available Physical", locale.toString( (qlonglong)info.freeram * info.mem_unit) ) );
        fSystemMemoryData.push_back( QPair< QString, QString >( "Shared Ram", locale.toString( (qlonglong)info.sharedram * info.mem_unit) ) );
        fSystemMemoryData.push_back( QPair< QString, QString >( "Buffer Ram", locale.toString( (qlonglong)info.bufferram * info.mem_unit) ) );
        fSystemMemoryData.push_back( QPair< QString, QString >( "Total high memory size", locale.toString( (qlonglong)info.totalhigh * info.mem_unit) ) );
        fSystemMemoryData.push_back( QPair< QString, QString >( "Available high memory size", locale.toString( (qlonglong)info.freehigh * info.mem_unit) ) );

        struct rusage rUsage;
        bool aOK = getrusage( RUSAGE_SELF, &rUsage ) == 0;
        if ( aOK )
        {
            fApplicationMemoryData.push_back( QPair< QString, QString >( "Maximum resident set size", locale.toString( (qlonglong)rUsage.ru_maxrss ) ) );
            if ( baseInfo )
                sBaseApplicationMemory = locale.toString( (qlonglong)rUsage.ru_maxrss );

            fApplicationMemoryData.push_back( QPair< QString, QString >( "Integral shared memory size", locale.toString( (qlonglong)rUsage.ru_ixrss ) ) );
            fApplicationMemoryData.push_back( QPair< QString, QString >( "Integral unshared data size", locale.toString( (qlonglong)rUsage.ru_idrss ) ) );
            fApplicationMemoryData.push_back( QPair< QString, QString >( "Integral unshared stack size", locale.toString( (qlonglong)rUsage.ru_isrss ) ) );
        }
    }

    void getOSInfo( std::string & osName, std::string & osVersion )
    {
        struct utsname uts;
        if ( uname( &uts) != 0 )
            return;

        osName = uts.sysname ? uts.sysname : "";
        osVersion  = uts.machine ? uts.machine : "";
        osVersion += " ";
        osVersion += uts.release ? uts.release : "";
        osVersion += " ";
        osVersion += uts.version ? uts.version : "";
    }

    std::string SSystemInfo::getOSVersion()
    {
        std::string version;
        std::string name;
        getOSInfo( name, version );
        return version;
    }

    std::string SSystemInfo::getOSName()
    {
        std::string version;
        std::string name;
        getOSInfo( name, version );
        return name;
    }

    void SSystemInfo::LoadSystemInfo()
    {
        QLocale locale;
        struct sysinfo info;
        if ( sysinfo( &info ) != 0 )
            return;

        std::string version;
        std::string name;
        getOSInfo( name, version );
        fSystemInformation.push_back( QPair< QString, QString >( "OS", QString::fromStdString( name ) ) );
        fSystemInformation.push_back( QPair< QString, QString >( "OS Version", QString::fromStdString( version ) ) );

        int days = info.uptime / 86400;
        int hours = ( info.uptime / 3600 ) - ( days * 24 );
        int mins = ( info.uptime / 60 ) - ( days * 1440 ) - ( hours * 60 );
        fSystemInformation.push_back( QPair< QString, QString >( "Uptime", QString( "%1 days, %2 hours, %3 minutes, %4 seconds" )
                    .arg( days )
                    .arg( hours )
                    .arg( mins )
                    .arg( info.uptime % 60 ) ) )
                    ;
        fSystemInformation.push_back( QPair< QString, QString >( "Load Avgs", QString( "1min(%1) 5min(%2) 15min(%3)" )
                    .arg( locale.toString( info.loads[ 0 ]/65536.0, 'g', 3 ) )
                    .arg( locale.toString( info.loads[ 1 ]/65536.0, 'g', 3 ) )
                    .arg( locale.toString( info.loads[ 2 ]/65536.0, 'g', 3 ) ) ) )
                    ;
        fSystemInformation.push_back( QPair< QString, QString >( "Total Swap", locale.toString( (qlonglong)info.totalswap ) ) );
        fSystemInformation.push_back( QPair< QString, QString >( "Free Swap", locale.toString( (qlonglong)info.freeswap ) ) );
        fSystemInformation.push_back( QPair< QString, QString >( "Number of Processes", locale.toString( (qlonglong)info.procs ) ) );

    
    }

    void SSystemInfo::LoadNICInfo()
    {
    }
}
