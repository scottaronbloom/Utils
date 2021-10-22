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

#ifndef __QTUTILS_H
#define __QTUTILS_H

class QString;
class QStringRef;
class QDir;
class QXmlStreamReader;
class QXmlStreamWriter;
class QAbstractItemModel;
class QSettings;
#ifdef QT_XMLPATTERNS_LIB
class QXmlQuery;
#endif
#include <functional>

#include <QDateTime>
#include <QString>
#include <QSet>
#include <QList>
#include <set>
#include <QDebug>
#include <QTextStream>
#include <unordered_set>

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
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
#endif

namespace NQtUtils
{
    struct CCaseInsensitiveHash
    {
        size_t operator()( const QString& str ) const;
    };

    struct CCaseInsensitiveEqual
    {
        size_t operator()( const QString& lhs, const QString& rhs ) const;
    };

    using TCaseInsensitiveHash = std::unordered_set< QString, CCaseInsensitiveHash, CCaseInsensitiveEqual >;

    template< typename T >
    QList< T > replaceInList( const QList< T > & inList, int xFirst, int xCount, const QList< T > & values, int xNum=-1 )
    {
        auto prefix = inList.mid( 0, xFirst );
        auto mid    = values.mid( 0, xNum );
        auto suffix = inList.mid( xFirst + xCount );
        
        auto lRetVal = prefix + mid + suffix;
        return lRetVal;
    }

    QString allFilesFilter();
    QString defaultFileDialogDir();

#ifdef QT_XMLPATTERNS_LIB
    QString getString( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    QStringList getStringList( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    std::set< QString > getStringSet( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    int getInt( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    bool getBool( QXmlQuery & query, const QString & queryString, bool defaultVal=false );
    double getDouble( QXmlQuery & query, const QString & queryString, bool * aOK = nullptr );
    std::list< std::pair< QString, QString > > getStringPairs( QXmlQuery & query, const QString & queryString1, const QString & queryString2, bool * aOK = nullptr );
    std::list< std::list< QString > > getStrings( QXmlQuery & query, const QStringList & xmlPaths, bool * aOK = nullptr );
    QString getFile( QXmlQuery & query, const QDir & relToDir, const QString & queryString, bool * aOK = nullptr );
#endif

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

    QDate findDate( const QString &dateString );
}

template< std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
printToDebug( QDebug & /*retVal*/, const std::tuple< Tp... > & /*value*/ )
{
    return;
}

template< std::size_t I = 0, typename... Tp>
inline typename std::enable_if < I < sizeof...(Tp), void>::type
    printToDebug( QDebug & debug, const std::tuple< Tp... > & value )
{
    if ( I != 0 )
        debug.nospace() << ", ";

    debug << std::get< I >( value );

    printToDebug< I + 1, Tp... >( debug, value );
}

template< typename... Tp>
QDebug operator<<( QDebug & debug, const std::tuple< Tp... > & value )
{
    debug.nospace() << "std::tuple(";
    printToDebug( debug, value );
    debug.nospace() << ")";
    return debug;
}

#endif

