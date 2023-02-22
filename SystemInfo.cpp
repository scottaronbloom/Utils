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

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <string>
#include <sstream>

namespace NSABUtils
{
    std::string CSystemInfo::sBaseApplicationMemory;
    CSystemInfo::CSystemInfo( bool baseInfoOnly )
    {
#if BPS_64BIT
        fSystemInformation.push_back( std::pair< std::string, std::string >( "Application Pointer Size", "64 bits" ) );
#else
        fSystemInformation.push_back( std::pair< std::string, std::string >( "Application Pointer Size", "32 bits" ) );
#endif
        if ( baseInfoOnly )
            LoadMemoryInfo( true );
        else
            Load();
    }

    void CSystemInfo::Load()
    {
        LoadSystemInfo();
        LoadMemoryInfo( false );
        LoadNICInfo();
    }

    std::string CSystemInfo::getPrimaryHostID() const
    {
        std::string retVal;
        if ( hasSystemInfo() && !fNics.empty() )
        {
            retVal = fNics.front().fMacAddr;
        }
        return retVal;
    }

    std::string CSystemInfo::getText( EFormat format, bool memoryOnly, bool showSystemMemory ) const
    {
        if ( !hasSystemInfo() )
            return {};

        std::ostringstream oss;

        QJsonDocument jsonDoc;
        QJsonArray dataArray;

        if ( !memoryOnly && format == EFormat::eText )
            oss << "-------------------------\n";

        if ( !memoryOnly )
        {
            oss << dumpTable( fSystemInformation, "System Information", {}, format, dataArray );
        }

        oss << dumpTable( fApplicationMemoryData, "Application Memory", { "Base Application Memory Size", sBaseApplicationMemory }, format, dataArray );

        if ( showSystemMemory )
        {
            oss << dumpTable( fSystemMemoryData, "System Memory", {}, format, dataArray );
        }

        if ( !memoryOnly )
        {
            dumpNics( fNics, "Network Interface", format, dataArray );
        }
        oss << "------------------"
            << "\n";

        if ( format == EFormat::eJSON )
        {
            jsonDoc.setArray( dataArray );
            return jsonDoc.toJson( QJsonDocument::Indented ).toStdString();
        }
        return oss.str();
    }

    std::string CSystemInfo::dumpTable( const std::list< std::pair< std::string, std::string > > &table, const std::string &title, const std::pair< std::string, std::string > &subTitle, EFormat format, QJsonArray &jsonParentArray ) const
    {
        std::ostringstream oss;
        if ( !table.empty() )
        {
            QJsonObject jsonObj;
            jsonObj.insert( "name", QString::fromStdString( title ) );

            if ( format == EFormat::eText )
            {
                oss << "\n"
                    << title << "\n"
                    << "------------------"
                    << "\n";
                if ( !subTitle.first.empty() )
                    oss << subTitle.first << ": " << subTitle.second << "\n";
            }
            else if ( format == EFormat::eHtml )
            {
                oss << "\n"
                    << R"(<table width="100%" border="1">\n)"
                    << R"(<tr><th align="left" colspan="2">)" << title << "</th></tr>\n";

                if ( !subTitle.first.empty() )
                    oss << R"(<tr><th align="left">)" << subTitle.first << ":</th><td>" << sBaseApplicationMemory << "</td></tr>\n";
            }

            QJsonObject data;
            if ( !subTitle.first.empty() )
                data.insert( QString::fromStdString( subTitle.first ), QString::fromStdString( subTitle.second ) );

            for ( auto &&ii : table )
            {
                if ( format == EFormat::eText )
                    oss << ii.first << ": " << ii.second << "\n";
                else if ( format == EFormat::eHtml )
                    oss << R"(<tr><th align="left">)" << ii.first << ":</th><td>" << ii.second << "</td></tr>\n";

                data.insert( QString::fromStdString( ii.first ), QString::fromStdString( ii.second ) );
            }

            jsonObj.insert( "data", data );

            if ( format == EFormat::eHtml )
                oss << "</table>\n";
            jsonParentArray.push_back( jsonObj );
        }
        return oss.str();
    }

    std::string CSystemInfo::dumpNics( const std::list< SNicInfo > &nics, const std::string &title, EFormat format, QJsonArray &jsonParentArray ) const
    {
        std::ostringstream oss;
        if ( !nics.empty() )
        {
            QJsonObject jsonObj;
            jsonObj.insert( "name", QString::fromStdString( title + "s" ) );
            if ( format == EFormat::eText )
            {
                oss << "\n"
                    << title << "s"
                    << "\n"
                    << "------------------"
                    << "\n";
            }
            else if ( format == EFormat::eHtml )
            {
                oss << "\n"
                    << R"(<table width="100%" border="1">\n)"
                    << R"(<tr><th align="left" colspan="2">)" << title << "s"
                    << "</th></tr>\n";
            }

            QJsonArray jsonNics;

            int ii = 1;
            for ( auto &&currNic : nics )
            {
                if ( format == EFormat::eText )
                {
                    oss << title << ": " << ii << "\n"
                        << "Name: " << currNic.fName << "\n"
                        << "MAC: " << currNic.fMacAddr << "\n"
                        << "IP: " << currNic.fIPAddr << "\n";
                }
                else if ( format == EFormat::eHtml )
                {
                    oss << R"(<tr><th align="center"colspan="2">)" << title << ": " << std::to_string( ii ) << "</th></tr>\n"
                        << R"(<tr><th>Name:</th><td>)" << currNic.fName << "</td></tr>\n"
                        << R"(<tr><th>MAC:</th><td>)" << currNic.fMacAddr << "</td></tr>\n"
                        << R"(<tr><th>IP:</th><td>)" << currNic.fIPAddr << "</td></tr>\n";
                }
                QJsonObject jsonNic;
                jsonNic.insert( QString::fromStdString( title ), QString::number( ii++ ) );
                jsonNic.insert( "Name", QString::fromStdString( currNic.fName ) );
                jsonNic.insert( "MAC", QString::fromStdString( currNic.fMacAddr ) );
                jsonNic.insert( "IP", QString::fromStdString( currNic.fIPAddr ) );

                jsonNics.push_back( jsonNic );
            }

            if ( format == EFormat::eHtml )
                oss << "</table>\n";

            jsonObj.insert( "nics", jsonNics );
            jsonParentArray.push_back( jsonObj );
        }
        return oss.str();
    }
}
