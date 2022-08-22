#ifndef __DOWNLOADLATESTVERSION_H
#define __DOWNLOADLATESTVERSION_H


#include "SABUtilsExport.h"
#include <QObject>
#include <QUrl>
#include <QDate>

class QEventLoop;
class QNetworkAccessManager;
class QNetworkReply;
class QAuthenticator;

namespace NSABUtils
{
struct SABUTILS_EXPORT SVersion
{
    SVersion() :
        fMajor( -1 ),
        fMinor( -1 ),
        fGitSHA( -1 )
    {
    }

    SVersion( const SVersion & rhs ) :
        fMajor( rhs.fMajor ),
        fMinor( rhs.fMinor ),
        fGitSHA( rhs.fGitSHA )
    {
    }

    SVersion & operator=( const SVersion & rhs )
    {
        fMajor = rhs.fMajor;
        fMinor = rhs.fMinor;
        fGitSHA = rhs.fGitSHA;
        return *this;
    }

    QString getVersion() const;
    int fMajor;
    int fMinor;
    QString fGitSHA; // the unique sha on the build
};


struct SABUTILS_EXPORT SDownloadInfo
{
    SDownloadInfo() :
        fSize( 0 )
    {
    }

    QUrl fUrl;
    //QString fPlatform;
    int fSize{0};
    //QString fMD5;

    //QString getSize() const;
};

struct SABUTILS_EXPORT SReleaseInfo
{
    SReleaseInfo() :
        fPatchRelease( false ),
        fDownloadable( false ),
        fInMaintenance( false )
    {
    }

    QList< SReleaseInfo > splitDownloads() const;
    QString getKey() const;

    SVersion fVersion;
    QDate fReleaseDate;
    bool fPatchRelease;
    bool fDownloadable;
    bool fInMaintenance;
    QString fReleaseInfo;
    QString fMD5;
    QList< SDownloadInfo > fDownloads;
};

class SABUTILS_EXPORT CDownloadLatestVersion : public QObject
{
Q_OBJECT;
public:
    CDownloadLatestVersion( const QByteArray & githubToken, QObject * parent = nullptr );
    CDownloadLatestVersion( const QString & urlPath, const QByteArray & githubToken, QObject * parent = nullptr );

    static QString determineReleasesPath(); // uses QApplication::organizationDomain

    ~CDownloadLatestVersion();

    void setCurrentVersion( int major, int minor );
    void setVerbose( bool verbose );

    void requestLatestVersion();

    QString latestVersion() const;
    QList< SReleaseInfo > getLatestVersionInfo() const{ return fReleases; }

    static bool getVersionOverride( int & major, int & minor, int & buildNumber, int & month, int & year );
    static QString getOverrideOS();

    bool hasError()const { return fHasError; }
    QString errorString() const{ return fErrorString; }

    //static bool isActive(){ return fReply != nullptr; }
private Q_SLOTS:
    void slotAuthenticationRequired( QNetworkReply * reply, QAuthenticator * authenticator );
    void slotFinished( QNetworkReply * reply );
    void slotKillCurrentReply();

Q_SIGNALS:
    void sigKillLoop();
    void sigConnectionChanged();

private:
    int getTimeOutDelay() const;
    void loadResults( const QByteArray & results );

    SVersion fCurrentVersion;
    QString fURLPath;
    QByteArray fGitHubToken;

    bool fVerbose;

    QNetworkAccessManager * fManager{ nullptr };
    QNetworkReply * fReply{ nullptr };
    bool fHasError{ false };
    QString fErrorString;
    QString fRetVal;
    QList< SReleaseInfo > fReleases;
};
}

#endif
