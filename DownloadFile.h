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

#ifndef __DOWNLOADFILE_H
#define __DOWNLOADFILE_H

#include <QDialog>
#include <QFile>
#include <QElapsedTimer>
#include <memory>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "SABUtilsExport.h"

class QNetworkReply;
class QAuthenticator;
class QNetworkProxy;
class QNetworkAccessManager;
class QSslPreSharedKeyAuthenticator;
class QSslError;

namespace Ui { class CDownloadFile; }

namespace NSABUtils
{
    struct SABUTILS_EXPORT SDownloadFileInfo
    {
        SDownloadFileInfo( const QString & name, const QUrl & url, uint64_t size );
        QString fName;
        QUrl fUrl;
        uint64_t fSize{ 0 };
    };

    class SABUTILS_EXPORT CDownloadFile : public QDialog
    {
        Q_OBJECT;
    public:
        CDownloadFile( const SDownloadFileInfo & fileInfo, QWidget * parent );
        CDownloadFile( const QString & name, const QUrl & url, uint64_t size, QWidget * parent );

        ~CDownloadFile();

        QString getDownloadFile() const { return fDownloadFileName; }

        bool startDownload();
        bool installAfterDownload() const { return fInstallAfterDownload; }
    public slots:
        void reject() override;
        void slotRequestFinished( QNetworkReply * reply );
        void slotAuthenticationRequired( QNetworkReply * reply, QAuthenticator * authenticator );
        void slotEncrypted( QNetworkReply * reply );
        void slotPreSharedKeyAuthenticationRequired( QNetworkReply * reply, QSslPreSharedKeyAuthenticator * authenticator );
        void slotProxyAuthenticationRequired( const QNetworkProxy & proxy, QAuthenticator * authenticator );
        void slotSSLErrors( QNetworkReply * reply, const QList<QSslError> & errors );

        void slotEncryptedReply();
        void slotErrorOccurred( QNetworkReply::NetworkError code );
        void slotFinished();
        void slotMetaDataChanged();
        void slotPreSharedKeyAuthenticationRequiredReply( QSslPreSharedKeyAuthenticator * authenticator );
        void slotRedirectAllowed();
        void slotRedirected( const QUrl & url );
        void slotSSLErrorsReply( const QList<QSslError> & errors );
        void slotUploadProgress( qint64 bytesSent, qint64 bytesTotal );
        void slotDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );

        void slotAboutToClose();
        void slotBytesWritten( qint64 bytes );
        void slotChannelBytesWritten( int channel, qint64 bytes );
        void slotChannelReadyRead( int channel );
        void slotReadChannelFinished();
        void slotReadyRead();

        void slotInstallNow();
        void slotSaveOnly();
        void slotCancel();

    private:
        std::unique_ptr< Ui::CDownloadFile > fImpl;
        QString fDownloadFileName;
        QFile fOutputFile;
        QNetworkAccessManager * fManager{ nullptr };
        QNetworkReply * fReply{ nullptr };
        QElapsedTimer fDownloadTime;
        bool fHasError;
        SDownloadFileInfo fFileInfo;
        bool fInstallAfterDownload{ true };
        bool fWasCanceled{ false };
    };
}
#endif

