/*
 * (c) Copyright 2004 - 2019 Blue Pearl Software Inc.
 * All rights reserved.
 *
 * This source code belongs to Blue Pearl Software Inc.
 * It is considered trade secret and confidential, and is not to be used
 * by parties who have not received written authorization
 * from Blue Pearl Software Inc.
 *
 * Only authorized users are allowed to use, copy and modify
 * this software provided that the above copyright notice
 * remains in all copies of this software.
 *
 *
 * $Author: scott $ - $Revision: 53890 $ - $Date: 2019-07-08 17:09:58 -0700 (Mon, 08 Jul 2019) $
 * $HeadURL: http://bpsvn/svn/trunk/Shared/GenUtils/QtUtils.h $
 *
 *
*/
#ifndef __SHARED_GENUTILS_QTUTILS_H
#define __SHARED_GENUTILS_QTUTILS_H

class QString;
class QStringRef;
class QDir;
class QXmlStreamReader;
class QXmlStreamWriter;
class QAbstractItemModel;
class QSettings;
class QXmlQuery;
#include <functional>

#include "qt_prolog.h"
#include <QDateTime>
#include <QString>
#include <QSet>
#include <QList>
#include "qt_epilog.h"
#include <set>

namespace std
{
    template <>
    struct hash<QString>
    {
        std::size_t operator()( const QString& k ) const
        {
            return qHash( k );
        }
    };
}


namespace NQtUtils
{
    QString getString( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    QStringList getStringList( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    std::set< QString > getStringSet( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    int getInt( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    bool getBool( QXmlQuery & query, const QString & queryString, bool defaultVal=false );
    double getDouble( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    std::list< std::pair< QString, QString > > getStringPairs( QXmlQuery & query, const QString & queryString1, const QString & queryString2, bool * aOK = nullptr );
    std::list< std::list< QString > > getStrings( QXmlQuery & query, const QStringList & xmlPaths, bool * aOK = nullptr );
    QString getFile( QXmlQuery & query, const QDir & relToDir, const QString & queryString, bool * aOK = nullptr );


    int getInt( const QString & str, bool * aOK );
    int getInt( const QStringRef & str, bool * aOK );
    int getInt( const QString & str );
    int getInt( const QStringRef & str );
    int getInt( const QString & str, QXmlStreamReader & reader );
    int getInt( const QStringRef & str, QXmlStreamReader & reader );
    int getInt( const QString & str, int defaultValue, QXmlStreamReader & reader );
    int getInt( const QStringRef & str, int defaultValue, QXmlStreamReader & reader );

    double getDouble( const QString & str, bool * aOK );
    double getDouble( const QString & str );
    double getDouble( const QStringRef & str, bool * aOK );
    double getDouble( const QStringRef & str );
    double getDouble( const QString & str, QXmlStreamReader & reader );
    double getDouble( const QStringRef & str, QXmlStreamReader & reader );

    bool getBool( const QString & str, bool defaultVal=false ); // default returned when string is empty
    bool getBool( const QStringRef & str, bool defaultVal=false );

    QString getFile( const QString & relToDir, QXmlStreamReader & reader, QString * origFile=nullptr );
    QString getFile( const QDir & relToDir, QXmlStreamReader & reader, QString * origFile=nullptr );
    QString getFile( const QDir & relToDir, const QString & file );

    QDateTime getDateTime( const QStringRef & str, QXmlStreamReader & reader, bool optional );
    QDateTime getDateTime( const QString & str, QXmlStreamReader & reader, bool optional );
    QDateTime getDateTime( const QString & str );

    template< typename T >
    std::set< T > toSet( const QSet< T > & values )
    {
        std::set< T > retVal;
        for( auto val : values )
            retVal.insert( val );
        return retVal;
    }

    template< typename T >
    std::set< T > toSet( const QList< T > & values )
    {
        std::set< T > retVal;
        for( auto val : values )
            retVal.insert( val );
        return retVal;
    }

    class noCaseQStringCmp
    {
    public:
        bool operator() (const QString & s1, const QString & s2) const
        {
            return s1.compare( s2, Qt::CaseInsensitive ) < 0;
        }
    };

    QStringList splitLineCSV( const QString & line );
    size_t SizeOf( const QString & str );
    size_t SizeOf( const QDateTime & str );
    QString fromHtmlEscaped( const QString & str );

    void move( QSettings & settings, const QString & subGroup, const QString & key, bool overwrite );
    void copy( QSettings & from, QSettings & to, bool overwrite );

    QStringList getHeadersForModel( QAbstractItemModel * model );
    void writeModel( QAbstractItemModel * model, 
                     QXmlStreamWriter & writer, 
                     const QString & keyName, 
                     const QString & plauralSuffix, 
                     const std::function<void(QAbstractItemModel * model, QXmlStreamWriter &writer, const QString & keyName, int rowNum ) > & writeRow = 
                           std::function<void(QAbstractItemModel * model, QXmlStreamWriter &writer, const QString & keyName, int rowNum ) >() );
}
#endif

