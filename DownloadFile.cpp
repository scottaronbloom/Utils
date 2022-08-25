// The MIT License( MIT )
//
// Copyright( c ) 2022 Scott Aron Bloom
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software iRHS
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

#include "DownloadGitHubAsset.h"
#include "GitHubGetVersions.h"

#include "ui_DownloadGitHubAsset.h"

#include <QMessageBox>
#include <QTimer>
#include <QStandardPaths>
#include <QFileDialog>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDebug>

namespace NSABUtils
{
    CDownloadGitHubAsset::CDownloadGitHubAsset( std::shared_ptr < SGitHubAsset > asset, QWidget * parent ) :
        QDialog( parent ),
        fImpl( new Ui::CDownloadGitHubAsset ),
        fManager( nullptr ),
        fHasError( false ),
        fAssett( asset )
    {
        fImpl->setupUi( this );
        setWindowFlags( this->windowFlags() & ~Qt::WindowContextHelpButtonHint );

        fManager = new QNetworkAccessManager( this );
        connect( fManager, &QNetworkAccessManager::finished, this, &CDownloadGitHubAsset::slotRequestFinished );
        connect( fManager, &QNetworkAccessManager::authenticationRequired, this, &CDownloadGitHubAsset::slotAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::encrypted, this, &CDownloadGitHubAsset::slotEncrypted );
        connect( fManager, &QNetworkAccessManager::preSharedKeyAuthenticationRequired, this, &CDownloadGitHubAsset::slotPreSharedKeyAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::proxyAuthenticationRequired, this, &CDownloadGitHubAsset::slotProxyAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::sslErrors, this, &CDownloadGitHubAsset::slotSSlErrors );

        fImpl->installAfterDownload->setChecked( true );
#ifndef Q_OS_WIN
        fImpl->installAfterDownload->setHidden( true );
        fImpl->installAfterDownload->setChecked( false );
#endif
    }

    CDownloadGitHubAsset::~CDownloadGitHubAsset()
    {
    }

    void CDownloadGitHubAsset::slotAuthenticationRequired( QNetworkReply * reply, QAuthenticator * authenticator )
    {
        qDebug() << "slotAuthenticationRequired:" << reply << reply->url().toString() << authenticator;
    }

    void CDownloadGitHubAsset::slotEncrypted( QNetworkReply * reply )
    {
        qDebug() << "slotEncrypted:" << reply << reply->url().toString();
    }

    void CDownloadGitHubAsset::slotPreSharedKeyAuthenticationRequired( QNetworkReply * reply, QSslPreSharedKeyAuthenticator * authenticator )
    {
        qDebug() << "slotPreSharedKeyAuthenticationRequired: 0x" << /*Qt::hex << */reply << reply->url().toString() << authenticator;
    }

    void CDownloadGitHubAsset::slotProxyAuthenticationRequired( const QNetworkProxy & proxy, QAuthenticator * authenticator )
    {
        qDebug() << "slotProxyAuthenticationRequired: 0x" << /*Qt::hex << */&proxy << authenticator;
    }

    void CDownloadGitHubAsset::slotSSlErrors( QNetworkReply * reply, const QList<QSslError> & errors )
    {
        qDebug() << "slotSSlErrors: 0x" << /*Qt::hex << */reply << errors;
    }

    void CDownloadGitHubAsset::slotRequestFinished( QNetworkReply * reply )
    {
        if ( reply && reply->error() != QNetworkReply::NoError )
        {
            QMessageBox::critical( this, tr( "Error downloading latest version" ), tr( "Error downloading latest version<br>%1" ).arg( reply->errorString() ) );
            if ( fOutputFile.isOpen() )
                fOutputFile.close();
            reject();
            return;
        }

        if ( fOutputFile.isOpen() && reply )
            fOutputFile.write( reply->readAll() );

        if ( !fOutputFile.isOpen() )
            return;
        else
            fOutputFile.close();

        QFile::rename( fDownloadFileName + ".tmp", fDownloadFileName );
#ifdef Q_OS_WIN
        QFile fi( fDownloadFileName );
        fi.setPermissions( fi.permissions() | QFileDevice::ExeOwner );
#endif

        accept();
    }

    bool CDownloadGitHubAsset::installAfterDownload() const
    {
        return fImpl->installAfterDownload->isChecked();
    }

    void CDownloadGitHubAsset::reject()
    {
        if ( fOutputFile.isOpen() )
            fOutputFile.close();

        if ( !fDownloadFileName.isEmpty() )
            QFile( fDownloadFileName + ".tmp" ).remove();

        QDialog::reject();
    }

    bool CDownloadGitHubAsset::startDownload()
    {
        if ( !fAssett )
            return false;

        auto downloadDirs = QStandardPaths::standardLocations( QStandardPaths::DownloadLocation );
        QDir initDir;
        QString fileName;
        if ( !downloadDirs.empty() )
        {
            initDir = QDir( downloadDirs.front() );
            fileName = initDir.absoluteFilePath( fAssett->fName );
        }
        else
            fileName = fAssett->fName;

        fDownloadFileName = QFileDialog::getSaveFileName( this, "Save Filename:", fileName );
        if ( fDownloadFileName.isEmpty() )
        {
            return false;
        }
        if ( QFile::exists( fDownloadFileName ) )
        {
            QFile::remove( fDownloadFileName + ".bak" );
            QFile::rename( fDownloadFileName, fDownloadFileName + ".bak" );
        }
        fOutputFile.setFileName( fDownloadFileName + ".tmp" );
        if ( !fOutputFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        {
            QMessageBox::warning( this, "Could not open", QString( "Could not open file '%1' for writing." ).arg( fDownloadFileName + ".tmp" ) );
            return false;
        }

        QNetworkRequest request;
        request.setUrl( fAssett->fUrl.second );

        auto reply = fManager->get( request );
        connect( fImpl->cancel, &QAbstractButton::clicked, reply, &QNetworkReply::abort );
        //connect( reply, &QNetworkReply::readyRead, this, &CDownloadGitHubAsset::slotReadyRead );
        fImpl->speed->setText( "" );
        fDownloadTime.start();
        return true;
    }


    void CDownloadGitHubAsset::slotReadyRead()
    {
        auto reply = dynamic_cast<QNetworkReply *>( sender() );
        if ( fOutputFile.isOpen() && reply )
            fOutputFile.write( reply->readAll() );
    }

    void CDownloadGitHubAsset::slotUpdateProgress( qint64 progress, qint64 total )
    {
        //fProgress = progress;
        if ( total != fImpl->currentProgressBar->maximum() )
            fImpl->currentProgressBar->setMaximum( total );

        double speed = progress * 1000.0 / fDownloadTime.elapsed();
        QString unit;
        if ( speed < 1024 )
        {
            unit = tr( "bytes/sec" );
        }
        else if ( speed < 1024 * 1024 )
        {
            speed /= 1024;
            unit = tr( "kB/s" );
        }
        else if ( speed < 1024 * 1024 * 1024 )
        {
            speed /= 1024 * 1024;
            unit = tr( "MB/s" );
        }
        else
        {
            speed /= 1024 * 1024 * 1024;
            unit = tr( "GB/s" );
        }

        fImpl->speed->setText( QString( "%1 %2" ).arg( speed, 3, 'f', 1 ).arg( unit ) );
        layout()->activate();
        resize( layout()->totalMinimumSize() );

        fImpl->currentProgressBar->setValue( progress );
    }
}