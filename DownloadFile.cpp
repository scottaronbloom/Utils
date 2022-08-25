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

#include "DownloadFile.h"
#include "GitHubGetVersions.h"

#include "ui_DownloadFile.h"

#include <QMessageBox>
#include <QTimer>
#include <QStandardPaths>
#include <QFileDialog>

#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDebug>
#include <QSslPreSharedKeyAuthenticator>

namespace NSABUtils
{
    SDownloadFileInfo::SDownloadFileInfo( const QString & name, const QUrl & url, uint64_t size ) :
        fName( name ),
        fUrl( url ),
        fSize( size )
    {

    }

    CDownloadFile::CDownloadFile( const SDownloadFileInfo & fileInfo, QWidget * parent ) :
        QDialog( parent ),
        fImpl( new Ui::CDownloadFile ),
        fHasError( false ),
        fFileInfo( fileInfo )
    {
        fImpl->setupUi( this );
        setWindowFlags( this->windowFlags() & ~Qt::WindowContextHelpButtonHint );

        fManager = new QNetworkAccessManager( this );
        connect( fManager, &QNetworkAccessManager::finished, this, &CDownloadFile::slotRequestFinished );
        connect( fManager, &QNetworkAccessManager::authenticationRequired, this, &CDownloadFile::slotAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::encrypted, this, &CDownloadFile::slotEncrypted );
        connect( fManager, &QNetworkAccessManager::preSharedKeyAuthenticationRequired, this, &CDownloadFile::slotPreSharedKeyAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::proxyAuthenticationRequired, this, &CDownloadFile::slotProxyAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::sslErrors, this, &CDownloadFile::slotSSLErrors );

        fImpl->buttonBox->button( QDialogButtonBox::Yes )->setText( tr( "Install" ) );
        fImpl->buttonBox->button( QDialogButtonBox::No )->setText( tr( "Close" ) );
#ifndef Q_OS_WIN
        fImpl->buttonBox->button( QDialogButtonBox::Yes )->setHidden( true );
#endif
        fImpl->buttonBox->button( QDialogButtonBox::Yes )->setEnabled( false );
        fImpl->buttonBox->button( QDialogButtonBox::No )->setEnabled( false );
        fImpl->buttonBox->button( QDialogButtonBox::Cancel )->setEnabled( true );

        connect( fImpl->buttonBox->button( QDialogButtonBox::Yes ), &QPushButton::clicked, this, &CDownloadFile::slotInstallNow );
        connect( fImpl->buttonBox->button( QDialogButtonBox::No ), &QPushButton::clicked, this, &CDownloadFile::slotSaveOnly );
        connect( fImpl->buttonBox->button( QDialogButtonBox::Cancel ), &QPushButton::clicked, this, &CDownloadFile::slotCancel );
    }

    CDownloadFile::CDownloadFile( const QString & name, const QUrl & url, uint64_t size, QWidget * parent ) :
        CDownloadFile( SDownloadFileInfo( name, url, size ), parent )
    {

    }

    CDownloadFile::~CDownloadFile()
    {
    }

    void CDownloadFile::reject()
    {
        if ( !fDownloadFileName.isEmpty() )
            QFile( fDownloadFileName + ".tmp" ).remove();

        QDialog::reject();
    }

    bool CDownloadFile::startDownload()
    {
        fWasCanceled = false;
        auto downloadDirs = QStandardPaths::standardLocations( QStandardPaths::DownloadLocation );
        QDir initDir;
        QString fileName;
        if ( !downloadDirs.empty() )
        {
            initDir = QDir( downloadDirs.front() );
            fileName = initDir.absoluteFilePath( fFileInfo.fName );
        }
        else
            fileName = fFileInfo.fName;

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
        request.setUrl( fFileInfo.fUrl );
        request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );

        //qDebug() << fFileInfo.fUrl << " - " << fFileInfo.fSize << fileName;
        fReply = fManager->get( request );
        connect( fReply, &QNetworkReply::downloadProgress, this, &CDownloadFile::slotDownloadProgress );
        connect( fReply, &QNetworkReply::encrypted, this, &CDownloadFile::slotEncryptedReply );
        connect( fReply, &QNetworkReply::errorOccurred, this, &CDownloadFile::slotErrorOccurred );
        connect( fReply, &QNetworkReply::finished, this, &CDownloadFile::slotFinished );
        connect( fReply, &QNetworkReply::metaDataChanged, this, &CDownloadFile::slotMetaDataChanged );
        connect( fReply, &QNetworkReply::preSharedKeyAuthenticationRequired, this, &CDownloadFile::slotPreSharedKeyAuthenticationRequiredReply );
        connect( fReply, &QNetworkReply::redirectAllowed, this, &CDownloadFile::slotRedirectAllowed );
        connect( fReply, &QNetworkReply::redirected, this, &CDownloadFile::slotRedirected );
        connect( fReply, &QNetworkReply::sslErrors, this, &CDownloadFile::slotSSLErrorsReply );
        connect( fReply, &QNetworkReply::uploadProgress, this, &CDownloadFile::slotUploadProgress );
        connect( fReply, &QNetworkReply::aboutToClose, this, &CDownloadFile::slotAboutToClose );
        connect( fReply, &QNetworkReply::bytesWritten, this, &CDownloadFile::slotBytesWritten );
        connect( fReply, &QNetworkReply::channelBytesWritten, this, &CDownloadFile::slotChannelBytesWritten );
        connect( fReply, &QNetworkReply::channelReadyRead, this, &CDownloadFile::slotChannelReadyRead );
        connect( fReply, &QNetworkReply::readChannelFinished, this, &CDownloadFile::slotReadChannelFinished );
        connect( fReply, &QNetworkReply::readyRead, this, &CDownloadFile::slotReadyRead );
        fImpl->speed->setText( "" );
        fDownloadTime.start();
        return true;
    }

    void CDownloadFile::slotRequestFinished( QNetworkReply * reply )
    {
        if ( reply && reply->error() != QNetworkReply::NoError )
        {
            if ( !fWasCanceled )
                QMessageBox::critical( this, tr( "Error downloading latest version" ), tr( "Error downloading latest version<br>%1" ).arg( reply->errorString() ) );
            if ( fOutputFile.isOpen() )
                fOutputFile.close();
            fImpl->buttonBox->button( QDialogButtonBox::Yes )->setEnabled( false );
            fImpl->buttonBox->button( QDialogButtonBox::No )->setEnabled( true );
            fImpl->buttonBox->button( QDialogButtonBox::Cancel )->setEnabled( false );
            return;
        }

        if ( fOutputFile.isOpen() && reply )
            fOutputFile.write( reply->readAll() );

        fImpl->buttonBox->button( QDialogButtonBox::Yes )->setEnabled( true );
        fImpl->buttonBox->button( QDialogButtonBox::No )->setEnabled( true );
        fImpl->buttonBox->button( QDialogButtonBox::Cancel )->setEnabled( false );

        reply->deleteLater();
        fReply = nullptr;

        if ( !fOutputFile.isOpen() )
            return;
        else
            fOutputFile.close();

        QFile::rename( fDownloadFileName + ".tmp", fDownloadFileName );
#ifdef Q_OS_WIN
        QFile fi( fDownloadFileName );
        fi.setPermissions( fi.permissions() | QFileDevice::ExeOwner );
#endif
    }

    void CDownloadFile::slotAuthenticationRequired( QNetworkReply * reply, QAuthenticator * authenticator )
    {
        (void)reply;
        (void)authenticator;
        //qDebug() << "slotAuthenticationRequired:" << reply << reply->url().toString() << authenticator;
    }

    void CDownloadFile::slotEncrypted( QNetworkReply * reply )
    {
        (void)reply;
        //qDebug() << "slotEncrypted:" << reply << reply->url().toString();
    }

    void CDownloadFile::slotPreSharedKeyAuthenticationRequired( QNetworkReply * reply, QSslPreSharedKeyAuthenticator * authenticator )
    {
        (void)reply;
        (void)authenticator;
        //qDebug() << "slotPreSharedKeyAuthenticationRequired: 0x" << /*Qt::hex << */reply << reply->url().toString() << authenticator;
    }

    void CDownloadFile::slotProxyAuthenticationRequired( const QNetworkProxy & proxy, QAuthenticator * authenticator )
    {
        (void)proxy;
        (void)authenticator;
        //qDebug() << "slotProxyAuthenticationRequired: 0x" << /*Qt::hex << */&proxy << authenticator;
    }

    void CDownloadFile::slotSSLErrors( QNetworkReply * reply, const QList<QSslError> & errors )
    {
        (void)reply;
        (void)errors;
        //qDebug() << "slotSSLErrors: 0x" << /*Qt::hex << */reply << errors;
    }

    void CDownloadFile::slotReadyRead()
    {
        auto reply = dynamic_cast<QNetworkReply *>( sender() );
        if ( fOutputFile.isOpen() && reply )
            fOutputFile.write( reply->readAll() );
    }

    void CDownloadFile::slotInstallNow()
    {
        fInstallAfterDownload = true;
        accept();
    }

    void CDownloadFile::slotSaveOnly()
    {
        fInstallAfterDownload = false;
        reject();
    }

    void CDownloadFile::slotCancel()
    {
        fWasCanceled = true;
        fImpl->currentProgressBar->setMaximum( 100 );
        fImpl->currentProgressBar->setValue( 100 );
        fImpl->currentProgressBar->setValue( fImpl->currentProgressBar->maximum() );
        fImpl->speed->setText( "Download canceled" );
        fReply->abort();
        reject();
    }

    void CDownloadFile::slotEncryptedReply()
    {
        //qDebug() << "slotEncryptedReply";
    }

    void CDownloadFile::slotErrorOccurred( QNetworkReply::NetworkError code )
    {
        (void)code;
        //qDebug() << "slotErrorOccurred" << code;
    }

    void CDownloadFile::slotFinished()
    {
        //qDebug() << "slotFinished";
    }

    void CDownloadFile::slotMetaDataChanged()
    {
        //qDebug() << "slotMetaDataChanged";
    }

    void CDownloadFile::slotPreSharedKeyAuthenticationRequiredReply( QSslPreSharedKeyAuthenticator * authenticator )
    {
        (void)authenticator;
        //qDebug() << "slotPreSharedKeyAuthenticationRequiredReply" << authenticator;
    }

    void CDownloadFile::slotRedirectAllowed()
    {
        //qDebug() << "slotRedirectAllowed";
    }

    void CDownloadFile::slotRedirected( const QUrl & url )
    {
        (void)url;
        //qDebug() << "slotRedirected" << url;
    }

    void CDownloadFile::slotSSLErrorsReply( const QList<QSslError> & errors )
    {
        (void)errors;
        //qDebug() << "slotSSLErrorsReply" << errors;
    }

    void CDownloadFile::slotUploadProgress( qint64 bytesSent, qint64 bytesTotal )
    {
        (void)bytesSent;
        (void)bytesTotal;
        //qDebug() << "slotUploadProgress" << bytesSent << bytesTotal;
    }

    void CDownloadFile::slotAboutToClose()
    {
        //qDebug() << "slotAboutToClose";
    }

    void CDownloadFile::slotBytesWritten( qint64 bytes )
    {
        (void)bytes;
        //qDebug() << "slotBytesWritten" << bytes;
    }

    void CDownloadFile::slotChannelBytesWritten( int channel, qint64 bytes )
    {
        (void)channel;
        (void)bytes;
        //qDebug() << "slotChannelBytesWritten" << channel << bytes;
    }

    void CDownloadFile::slotChannelReadyRead( int channel )
    {
        (void)channel;
        //qDebug() << "slotChannelReadyRead" << channel;
    }

    void CDownloadFile::slotReadChannelFinished()
    {
        //qDebug() << "slotReadChannelFinished";
    }

    void CDownloadFile::slotDownloadProgress( qint64 progress, qint64 total )
    {
        if ( fWasCanceled )
            return;
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
        //layout()->activate();
        //resize( layout()->totalMinimumSize() );

        fImpl->currentProgressBar->setValue( progress );
    }
}