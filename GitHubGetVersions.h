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

#ifndef __GITHUBGETVERSIONS_H
#define __GITHUBGETVERSIONS_H


#include "SABUtilsExport.h"
#include <QObject>
#include <QUrl>
#include <QDateTime>
#include <optional>

class QEventLoop;
class QNetworkAccessManager;
class QNetworkReply;
class QAuthenticator;
class QJsonArray;

namespace NSABUtils
{
struct SABUTILS_EXPORT SVersion
{
    SVersion() {};
    bool setVersionInfo( const QString & tagName, const QString & createdDate );

    bool operator>( const SVersion & rhs ) const
    {
        if ( fMajor != rhs.fMajor )
            return fMajor > rhs.fMajor;
        if ( fMinor != rhs.fMinor )
            return fMinor > rhs.fMinor;
        return fReleaseDate > rhs.fReleaseDate;
    }

    bool operator<( const SVersion & rhs ) const
    {
        if ( fMajor != rhs.fMajor )
            return fMajor < rhs.fMajor;
        if ( fMinor != rhs.fMinor )
            return fMinor < rhs.fMinor;
        return fReleaseDate < rhs.fReleaseDate;
    }

    bool operator==( const SVersion & rhs ) const
    {
        if ( fMajor == rhs.fMajor )
            return false;
        if ( fMinor != rhs.fMinor )
            return false;
        return fReleaseDate == rhs.fReleaseDate;
    }

    QString getVersion( bool verbose ) const;
    int fMajor{ -1 };
    int fMinor{ -1 };
    QDateTime fReleaseDate;
};

struct SABUTILS_EXPORT SGitHubAsset
{
    SGitHubAsset( const QJsonObject & assetInfo );

    QString getSize() const;

    bool supportsOS() const;
    std::pair< QString, QUrl > fUrl;
    QString fName;
    QString fContentType;
    int fSize{0};
    bool fAOK{ false };
};

struct SABUTILS_EXPORT SGitHubRelease
{
    SGitHubRelease( const SGitHubRelease & rhs ) :
        fTagName( rhs.fTagName ),
        fDescription( rhs.fDescription ),
        fPreRelease( rhs.fPreRelease ),
        fVersion( rhs.fVersion ),
        fAOK( rhs.fAOK )
    {}
    SGitHubRelease( const QJsonObject & version );
    QString getTitle() const
    {
        return QString( "%1 - %2" ).arg( fTagName ).arg( fDescription );
    }

    bool operator>( const SGitHubRelease & rhs ) const
    {
        return fVersion > rhs.fVersion;
    }

    bool operator<( const SGitHubRelease & rhs ) const
    {
        return fVersion < rhs.fVersion;
    }

    bool operator==( const SGitHubRelease & rhs ) const
    {
        return fVersion == rhs.fVersion;
    }
    bool supportsOS() const;
    std::list< std::shared_ptr< SGitHubRelease > > splitAssets() const;

    std::shared_ptr< SGitHubAsset > getAssetForOS() const;

    QString fTagName; // vMajor.Minor
    QString fDescription;
    bool fPreRelease{ false };
    SVersion fVersion;

    bool fAOK{ false };
    std::list< std::shared_ptr< SGitHubAsset > > fAssets;
};

class SABUTILS_EXPORT CGitHubGetVersions : public QObject
{
Q_OBJECT;
public:
    CGitHubGetVersions( const QByteArray & githubToken, QObject * parent = nullptr );
    CGitHubGetVersions( const QString & urlPath, const QByteArray & githubToken, QObject * parent = nullptr );

    static QString determineReleasesPath(); // uses QApplication::organizationDomain

    ~CGitHubGetVersions();

    void setCurrentVersion( int major, int minor, const QDateTime & buildDT );
    void requestLatestVersion();

    bool hasUpdate() const;
    QString updateVersion() const;
    std::shared_ptr< SGitHubRelease > updateRelease() const;

    bool hasError()const { return fHasError; }
    QString errorString() const{ return fErrorString; }
private Q_SLOTS:
    void slotAuthenticationRequired( QNetworkReply * reply, QAuthenticator * authenticator );
    void slotFinished( QNetworkReply * reply );
    void slotKillCurrentReply();

Q_SIGNALS:
    void sigKillLoop();

private:
    int getTimeOutDelay() const;
    void loadResults( const QJsonArray & results );

    SVersion fCurrentVersion;
    QString fURLPath;
    QByteArray fGitHubToken;

    QNetworkAccessManager * fManager{ nullptr };
    QNetworkReply * fReply{ nullptr };
    bool fHasError{ false };
    QString fErrorString;

    std::optional< std::pair< QString, std::shared_ptr< SGitHubRelease > > > fLatestUpdate;
    std::list< std::shared_ptr< SGitHubRelease > > fReleases;
};
}

#endif
