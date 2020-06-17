#ifndef __WORDEXP_H
#define __WORDEXP_H

#include <tuple>
#include <QString>
#include <QStringList>
#include <QHash>

class CWordExp
{
    friend class CWordExpImpl;
public:
    CWordExp( const QString & pathName );
    bool status() const{ return fAOK; } // when used via constructor, set to false if invalid user request
    QStringList getAbsoluteFilePaths( bool * aOK = nullptr ) const; // aOK set to false if windows and invalid user request
    bool beenThere( const QString & path );
    static QStringList getAbsoluteFilePaths( const QString & pathName, bool * aOK = nullptr ); // aOK set to false if windows and invalid user request

    static QString getHomeDir( const QString & userName, bool * aOK = nullptr ); // on windows can only return for current user, aOK set to false if windows and invalid user request
    static QString expandTildePath( const QString & fileName, bool * aOK = nullptr ); // On linux, if it starts with ~user or ~/ on windows, only ~/ is supported, returns ~user (no expansion) otherwise, aOK set to false if windows and invalid user request

    static QString getUserName();
    static QString getHostName();
    static QString getUserInfo();
private:
    static std::tuple< bool, QString, QString > isValidTilde( const QString & fileName );
    void expandPaths();
    void addResult( const QString & path );
    bool fAOK{ false };
    QString fOrigPathName;
    QStringList fExpandedPaths; // on linux, the wordexp expansion can return multiple paths
    QHash< QString, bool > fBeenThere; // due to links and other reasons
};

#endif



