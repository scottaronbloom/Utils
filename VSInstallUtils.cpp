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

#include "VSInstallUtils.h"
#include "AutoWaitCursor.h"
#include "StringUtils.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace NSABUtils
{
    namespace NVSInstallUtils
    {
        std::tuple< bool, QString, TInstalledVisualStudios > getInstalledVisualStudios( QProcess * process, bool * retry )
        {
            if ( retry )
                *retry = false;
            QString errorMsg;
            std::map< QString, QString > retVal;
            if ( !process || ( process->state() != QProcess::ProcessState::NotRunning ) )
            {
                if ( retry )
                    *retry = true;
                return { false, QString(), { retVal, QStringList() } };
            }

            auto programFiles = qgetenv( "PROGRAMFILES(x86)" );
            if ( programFiles.isEmpty() )
                programFiles = qgetenv( "PROGRAMFILES" );

            if ( programFiles.isEmpty() )
                return { false, QString(), { retVal, QStringList() } };
            if ( !QFileInfo( programFiles ).exists() )
                return { false, QString(), { retVal, QStringList() } };

            auto vsWhere = QDir( programFiles ).absoluteFilePath( "Microsoft Visual Studio/Installer/vswhere.exe" );
            if ( !QFileInfo( programFiles ).exists() )
                return { false, QString(), { retVal, QStringList() } };

            if ( !process )
                return { false, QString(), { retVal, QStringList() } };

            CAutoWaitCursor awc;
            QStringList args = QStringList()
                << "-requires"
                << "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
                << "-format"
                << "json";

            //process.setProcessChannelMode(QProcess::MergedChannels);
            process->start( vsWhere, args );
            if ( !process->waitForFinished( -1 ) || ( process->exitStatus() != QProcess::NormalExit ) || ( process->exitCode() != 0 ) )
            {
                return { false, QString( "Error: '%1' Could not run vswhere and determine VS installations" ).arg( QString( process->readAllStandardError() ) ), { retVal, QStringList() } };
            }
            auto data = process->readAll();

            auto vsInstallsDoc = QJsonDocument::fromJson( data );

            if ( !vsInstallsDoc.isArray() )
                return { false, QString( "Error: Invalid format from vswhere" ), { retVal, QStringList() } };

            std::map< QString, QString > installedVS;
            QStringList displayNames;

            QJsonArray vsInstalls = vsInstallsDoc.array();
            for ( int ii = 0; ii < vsInstalls.size(); ++ii )
            {
                auto vsInstall = vsInstalls[ ii ].toObject();
                if ( !vsInstall.contains( "displayName" ) || !vsInstall[ "displayName" ].isString() )
                    continue;

                if ( !vsInstall.contains( "installationPath" ) || !vsInstall[ "installationPath" ].isString() )
                    continue;

                auto dispName = vsInstall[ "displayName" ].toString();
                auto path = vsInstall[ "installationPath" ].toString();

                installedVS[ dispName ] = path;
                displayNames << dispName;
            }

            return { true, QString(), { installedVS, displayNames } };
        }
    }
}
