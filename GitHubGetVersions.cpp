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

#include "GitHubGetVersions.h"
#include "FileUtils.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QDebug>
#include <QAuthenticator>
#include <QUrlQuery>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCoreApplication>

namespace NSABUtils
{

    CGitHubGetVersions::CGitHubGetVersions( const QByteArray &githubToken, QObject *parent /*= nullptr */ ) :
        CGitHubGetVersions( QString(), githubToken, parent )
    {
        fURLPath = determineReleasesPath();
    }

    CGitHubGetVersions::CGitHubGetVersions( const QString &urlPath, const QByteArray &githubToken, QObject *parent /*= nullptr */ ) :
        QObject( parent ),
        fCurrentVersion(),
        fManager( nullptr ),
        fHasError( false ),
        fURLPath( urlPath ),
        fGitHubToken( githubToken )
    {
        fManager = new QNetworkAccessManager( this );
        connect( fManager, &QNetworkAccessManager::authenticationRequired, this, &CGitHubGetVersions::slotAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::finished, this, &CGitHubGetVersions::slotFinished );

        connect( fManager, &QNetworkAccessManager::encrypted, this, &CGitHubGetVersions::slotEncrypted );
        connect( fManager, &QNetworkAccessManager::preSharedKeyAuthenticationRequired, this, &CGitHubGetVersions::slotPreSharedKeyAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::proxyAuthenticationRequired, this, &CGitHubGetVersions::slotProxyAuthenticationRequired );
        connect( fManager, &QNetworkAccessManager::sslErrors, this, &CGitHubGetVersions::slotSSlErrors );
    }

    CGitHubGetVersions::~CGitHubGetVersions()
    {
    }

    void CGitHubGetVersions::setCurrentVersion( int major, int minor, const QDateTime &dt )
    {
        fCurrentVersion.fMajor = major;
        fCurrentVersion.fMinor = minor;
        fCurrentVersion.fReleaseDate = dt;
    }

    void CGitHubGetVersions::slotAuthenticationRequired( QNetworkReply * /*reply*/, QAuthenticator * /*authenticator*/ )
    {
    }

    void CGitHubGetVersions::slotEncrypted( QNetworkReply * /*reply*/ )
    {
        // qDebug() << "slotEncrypted:" << reply << reply->url().toString();
    }

    void CGitHubGetVersions::slotPreSharedKeyAuthenticationRequired( QNetworkReply * /*reply*/, QSslPreSharedKeyAuthenticator * /*authenticator*/ )
    {
        // qDebug() << "slotPreSharedKeyAuthenticationRequired: 0x" << Qt::hex << reply << reply->url().toString() << authenticator;
    }

    void CGitHubGetVersions::slotProxyAuthenticationRequired( const QNetworkProxy & /*proxy*/, QAuthenticator * /*authenticator*/ )
    {
        // qDebug() << "slotProxyAuthenticationRequired: 0x" << Qt::hex << &proxy << authenticator;
    }

    void CGitHubGetVersions::slotSSlErrors( QNetworkReply * /*reply*/, const QList< QSslError > & /*errors*/ )
    {
        // qDebug() << "slotSSlErrors: 0x" << Qt::hex << reply << errors;
    }

    void CGitHubGetVersions::slotFinished( QNetworkReply *reply )
    {
        if ( !fErrorString.isEmpty() )
        {
            fErrorString = reply->errorString();
            fHasError = ( reply->error() != QNetworkReply::NoError );
        }

        auto data = reply->readAll();
        if ( data.isEmpty() )
        {
            fErrorString = QObject::tr( "Error in reply: Empty Response" );
            fHasError = true;
        }

        if ( !fHasError )
        {
            QJsonParseError error;
            auto json = QJsonDocument::fromJson( data, &error );

            if ( error.error != QJsonParseError::NoError )
            {
                fErrorString += QObject::tr( "Error in Reply from server %1: '%2' @ %3" ).arg( fURLPath ).arg( error.errorString() ).arg( error.offset );
                fHasError = true;
            }
            if ( !json.isArray() )
            {
                fErrorString += QObject::tr( "Error in Reply: '%1' @ %2" ).arg( error.errorString() ).arg( error.offset );
                fHasError = true;
            }

            if ( !fHasError )
                loadResults( json.array() );
        }

        if ( fHasError )
            emit sigLogMessage( tr( "Versions finished downloading from github with error - %1" ).arg( fErrorString ) );

        reply->deleteLater();
        emit sigVersionsDownloaded();
    }

    void CGitHubGetVersions::requestLatestVersion()
    {
        QUrl url( fURLPath );

        fErrorString = "";
        fHasError = false;

        QNetworkRequest request;
        request.setUrl( url );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
        request.setTransferTimeout( getTimeOutDelay() );
#endif

        emit sigLogMessage( tr( "Requesting version info from - %1" ).arg( fURLPath ) );

        request.setRawHeader( QByteArray( "Accept" ), QByteArray( "application/vnd.github+json" ) );
        // request.setRawHeader( QByteArray( "Authorization" ), QByteArray( "token " ) + fGitHubToken );
        fManager->get( request );
    }

    int CGitHubGetVersions::getTimeOutDelay() const
    {
        int delay = 10000;
        QString value = qgetenv( "TOWEL42_TIMEOUT" );
        if ( !value.isEmpty() )
        {
            bool aOK;
            int val = value.toInt( &aOK );
            if ( aOK && ( val > 0 ) )
                delay = val;
        }
        return delay;
    }

    void CGitHubGetVersions::loadResults( const QJsonArray &versions )
    {
        if ( fHasError )
            return;

        emit sigLogMessage( tr( "Versions finished downloading from github - %1 versions found" ).arg( versions.count() ) );

        fReleases.clear();
        fLatestUpdate.reset();
        for ( auto &&version : versions )
        {
            auto curr = std::make_shared< SGitHubRelease >( version.toObject() );
            if ( !curr->fAOK )
                continue;
            fReleases.push_back( curr );
        }
        fReleases.sort(
            []( std::shared_ptr< SGitHubRelease > lhs, std::shared_ptr< SGitHubRelease > rhs ) -> bool
            {
                if ( !lhs || !rhs )
                    return false;

                SGitHubRelease lhsVal = *lhs;
                SGitHubRelease rhsVal = *rhs;
                return lhsVal > rhsVal;
            } );

        emit sigLogMessage( tr( "Searching for a newer version than '%1'" ).arg( fCurrentVersion.toString( true ) ) );
        for ( auto &&curr : fReleases )
        {
            if ( fLatestUpdate.has_value() )
                break;

            if ( curr->supportsOS() && ( curr->fVersion > fCurrentVersion ) )
            {
                emit sigLogMessage( tr( "Newer version found '%1'" ).arg( curr->fVersion.toString( true ) ) );
                auto text = curr->fVersion.toString( true ) + "\n";
                for ( auto &&asset : curr->fAssets )
                {
                    text += "\tDownload: " + asset->fUrl.first + " (" + asset->getSize() + ")\n";
                }
                fLatestUpdate = { text, curr };
            }
        }

        if ( !fLatestUpdate.has_value() )
        {
            emit sigLogMessage( tr( "Newer version not found" ) );
            fLatestUpdate = { "You are running the latest version of the release: " + fCurrentVersion.toString( true ), nullptr };
        }
    }

    bool CGitHubGetVersions::hasUpdate() const
    {
        return fLatestUpdate.has_value() && fLatestUpdate.value().second;
    }

    QString CGitHubGetVersions::updateVersion() const
    {
        if ( fHasError )
            return fErrorString;
        if ( !fLatestUpdate.has_value() )
            return {};
        if ( fLatestUpdate.value().second )
            return fLatestUpdate.value().second->getTitle();
        return fLatestUpdate.value().first;
    }

    std::shared_ptr< SGitHubRelease > CGitHubGetVersions::updateRelease() const
    {
        if ( fHasError )
            return {};
        if ( !fLatestUpdate.has_value() )
            return {};
        return fLatestUpdate.value().second;
    }

    QString CGitHubGetVersions::determineReleasesPath()
    {
        QString retVal = qApp->organizationDomain();
        if ( !retVal.startsWith( "https://" ) )
            retVal = "https://" + retVal;

        if ( !retVal.endsWith( "/" ) )
            retVal += "/";
        if ( !retVal.endsWith( "releases" ) )
            retVal += "releases";
        auto url = QUrl( retVal );

        auto path = url.path();
        path = "/repos" + path;
        url.setPath( path );

        auto host = url.host();
        if ( !host.startsWith( "api." ) )
            host = "api." + host;

        url.setHost( host );

        return url.toString();
    }

    bool SVersion::setVersionInfo( const QString &tagName, const QString &createdDate )
    {
        fReleaseDate = QDateTime::fromString( createdDate, Qt::ISODate );
        if ( !fReleaseDate.isValid() )
            return false;
        if ( !tagName.startsWith( "v" ) )
            return false;
        auto pos = tagName.indexOf( '.' );
        if ( pos == -1 )
            return false;
        bool aOK = false;
        fMajor = tagName.mid( 1, pos - 1 ).toInt( &aOK );
        if ( !aOK )
            return false;
        fMinor = tagName.mid( pos + 1 ).toInt( &aOK );
        if ( !aOK )
            return false;

        return aOK;
    }

    bool operator>( const SVersion &lhs, const SVersion &rhs )
    {
        if ( lhs.fMajor != rhs.fMajor )
            return lhs.fMajor > rhs.fMajor;
        if ( lhs.fMinor != rhs.fMinor )
            return lhs.fMinor > rhs.fMinor;
        return lhs.fReleaseDate > rhs.fReleaseDate;
    }

    bool operator<( const SVersion &lhs, const SVersion &rhs )
    {
        return !operator==( lhs, rhs ) && !operator>( lhs, rhs );
    }

    bool operator==( const SVersion &lhs, const SVersion &rhs )
    {
        return ( lhs.fMajor == rhs.fMajor ) && ( lhs.fMinor != rhs.fMinor ) && ( lhs.fReleaseDate == rhs.fReleaseDate );
    }

    QString SVersion::toString( bool verbose ) const
    {
        QString retVal;
        if ( verbose )
            retVal = QString( "Version: %1.%2 - Release Date: %3" );
        else
            retVal = QString( "%1.%2" );

        retVal = retVal.arg( fMajor ).arg( fMinor );
        if ( verbose )
            retVal = retVal.arg( fReleaseDate.toString() );
        return retVal;
    }

    SGitHubRelease::SGitHubRelease( const QJsonObject &version )
    {
        fTagName = version[ "tag_name" ].toString();
        fDescription = version[ "name" ].toString();
        fPreRelease = version[ "prerelease" ].toBool();
        auto createdAt = version[ "created_at" ].toString();

        if ( !fVersion.setVersionInfo( fTagName, createdAt ) )
            return;

        auto assets = version[ "assets" ].toArray();
        for ( auto &&ii : assets )
        {
            auto asset = ii.toObject();
            auto assetInfo = std::make_shared< SGitHubAsset >( asset );
            if ( !assetInfo->fAOK )
                return;
            fAssets.push_back( assetInfo );
        }
        fAOK = !fAssets.empty();
    }

    bool SGitHubRelease::operator>( const SGitHubRelease &rhs ) const
    {
        return fVersion > rhs.fVersion;
    }

    bool SGitHubRelease::operator<( const SGitHubRelease &rhs ) const
    {
        return fVersion < rhs.fVersion;
    }

    bool SGitHubRelease::operator==( const SGitHubRelease &rhs ) const
    {
        return fVersion == rhs.fVersion;
    }

    bool SGitHubRelease::supportsOS() const
    {
        for ( auto &&ii : fAssets )
        {
            if ( ii->supportsOS() )
                return true;
        }
        return false;
    }

    std::list< std::shared_ptr< SGitHubRelease > > SGitHubRelease::splitAssets() const
    {
        std::list< std::shared_ptr< SGitHubRelease > > retVal;
        if ( fAssets.empty() )
            return {};
        for ( auto &&ii : fAssets )
        {
            auto curr = std::make_shared< SGitHubRelease >( *this );
            curr->fAssets.push_back( ii );
            retVal.push_back( curr );
        }
        return std::move( retVal );
    }

    std::shared_ptr< NSABUtils::SGitHubAsset > SGitHubRelease::getAssetForOS() const
    {
        for ( auto &&ii : fAssets )
        {
            if ( ii->supportsOS() )
                return ii;
        }
        return {};
    }

    SGitHubAsset::SGitHubAsset( const QJsonObject &asset )
    {
        auto state = asset[ "state" ].toString();
        if ( state != "uploaded" )
            return;
        fUrl.first = asset[ "browser_download_url" ].toString();
        fUrl.second = QUrl( fUrl.first );
        fName = asset[ "name" ].toString();
        fSize = asset[ "size" ].toInt();
        fContentType = asset[ "content_type" ].toString();
        fAOK = true;
    }

    QString SGitHubAsset::getSize() const
    {
        return NFileUtils::byteSizeString( fSize );
    }

    bool SGitHubAsset::supportsOS() const
    {
#ifdef Q_OS_WIN
        return fContentType == "application/x-msdownload";
#endif
        return false;
    }

}
