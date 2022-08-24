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

#ifndef __DOWNLOADGITHUBASSET_H
#define __DOWNLOADGITHUBASSET_H

#include <QDialog>
#include <QFile>
#include <QTime>
#include <memory>

#include "SABUtilsExport.h"

class QNetworkReply;
class QAuthenticator;
class QNetworkProxy;
class QNetworkAccessManager;
class QSslPreSharedKeyAuthenticator;
class QSslError;

namespace Ui { class CDownloadGitHubAsset; }

namespace NSABUtils
{
    struct SGitHubAsset;
    class SABUTILS_EXPORT CDownloadGitHubAsset : public QDialog
    {
        Q_OBJECT;
    public:
        CDownloadGitHubAsset( std::shared_ptr < SGitHubAsset > asset, QWidget * parent );
        ~CDownloadGitHubAsset();

        QString getDownloadFile() const { return fDownloadFileName; }

        bool startDownload();
        bool installAfterDownload() const;
    public slots:
        void reject() override;
        void slotRequestFinished( QNetworkReply *reply );
        void slotAuthenticationRequired( QNetworkReply * reply, QAuthenticator * authenticator );
        void slotEncrypted( QNetworkReply * reply );
        void slotPreSharedKeyAuthenticationRequired( QNetworkReply * reply, QSslPreSharedKeyAuthenticator * authenticator );
        void slotProxyAuthenticationRequired( const QNetworkProxy & proxy, QAuthenticator * authenticator );
        void slotSSlErrors( QNetworkReply * reply, const QList<QSslError> & errors );


        void slotReadyRead();
        void slotUpdateProgress( qint64 progress, qint64 total );
    private:
        std::unique_ptr< Ui::CDownloadGitHubAsset > fImpl;
        QString fDownloadFileName;
        QFile fOutputFile;
        QNetworkAccessManager * fManager;
        QTime fDownloadTime;
        bool fHasError;
        std::shared_ptr < SGitHubAsset > fAssett;
    };
}
#endif

