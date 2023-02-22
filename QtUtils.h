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

#include "SABUtilsExport.h"
#include "nodiscard.h"

class QString;
class QStringRef;
class QDir;
class QXmlStreamReader;
class QXmlStreamWriter;
class QAbstractItemModel;
class QModelIndex;
class QTreeView;
class QSettings;
class QPlainTextEdit;
class QTextStream;
class QTimer;
class QLayout;
class QAbstractItemView;
class QTreeWidgetItem;

class QTableView;
class QTreeView;
class QHeaderView;
class QComboBox;

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
#include <QFileInfo>
#include <QTextStream>
#include <unordered_set>

namespace NSABUtils
{
    struct SABUTILS_EXPORT CCaseInsensitiveHash
    {
        size_t operator()( const QString &str ) const;
    };

    struct SABUTILS_EXPORT CCaseInsensitiveEqual
    {
        size_t operator()( const QString &lhs, const QString &rhs ) const;
    };

    using TCaseInsensitiveHash = std::unordered_set< QString, CCaseInsensitiveHash, CCaseInsensitiveEqual >;

    template< typename T >
    QList< T > replaceInList( const QList< T > &inList, int xFirst, int xCount, const QList< T > &values, int xNum = -1 )
    {
        auto prefix = inList.mid( 0, xFirst );
        auto mid = values.mid( 0, xNum );
        auto suffix = inList.mid( xFirst + xCount );

        auto lRetVal = prefix + mid + suffix;
        return lRetVal;
    }

    SABUTILS_EXPORT QString allFilesFilter();
    SABUTILS_EXPORT QString defaultFileDialogDir();

#ifdef QT_XMLPATTERNS_LIB
    SABUTILS_EXPORT QString getString( QXmlQuery &query, const QString &queryString, bool *aOK = nullptr );
    SABUTILS_EXPORT QStringList getStringList( QXmlQuery &query, const QString &queryString, bool *aOK = nullptr );
    SABUTILS_EXPORT std::set< QString > getStringSet( QXmlQuery &query, const QString &queryString, bool *aOK = nullptr );
    SABUTILS_EXPORT int getInt( QXmlQuery &query, const QString &queryString, bool *aOK = nullptr );
    SABUTILS_EXPORT bool getBool( QXmlQuery &query, const QString &queryString, bool defaultVal = false );
    SABUTILS_EXPORT double getDouble( QXmlQuery &query, const QString &queryString, bool *aOK = nullptr );
    SABUTILS_EXPORT std::list< std::pair< QString, QString > > getStringPairs( QXmlQuery &query, const QString &queryString1, const QString &queryString2, bool *aOK = nullptr );
    SABUTILS_EXPORT std::list< std::list< QString > > getStrings( QXmlQuery &query, const QStringList &xmlPaths, bool *aOK = nullptr );
    SABUTILS_EXPORT QString getFile( QXmlQuery &query, const QDir &relToDir, const QString &queryString, bool *aOK = nullptr );
#endif

    SABUTILS_EXPORT int getInt( const QString &str, bool *aOK );
    SABUTILS_EXPORT int getInt( const QStringRef &str, bool *aOK );
    SABUTILS_EXPORT int getInt( const QString &str );
    SABUTILS_EXPORT int getInt( const QStringRef &str );
    SABUTILS_EXPORT int getInt( const QString &str, QXmlStreamReader &reader );
    SABUTILS_EXPORT int getInt( const QStringRef &str, QXmlStreamReader &reader );
    SABUTILS_EXPORT int getInt( const QString &str, int defaultValue, QXmlStreamReader &reader );
    SABUTILS_EXPORT int getInt( const QStringRef &str, int defaultValue, QXmlStreamReader &reader );

    SABUTILS_EXPORT double getDouble( const QString &str, bool *aOK );
    SABUTILS_EXPORT double getDouble( const QString &str );
    SABUTILS_EXPORT double getDouble( const QStringRef &str, bool *aOK );
    SABUTILS_EXPORT double getDouble( const QStringRef &str );
    SABUTILS_EXPORT double getDouble( const QString &str, QXmlStreamReader &reader );
    SABUTILS_EXPORT double getDouble( const QStringRef &str, QXmlStreamReader &reader );

    SABUTILS_EXPORT bool getBool( const QString &str, bool defaultVal = false );   // default returned when string is empty
    SABUTILS_EXPORT bool getBool( const QStringRef &str, bool defaultVal = false );

    SABUTILS_EXPORT QString getFile( const QString &relToDir, QXmlStreamReader &reader, QString *origFile = nullptr );
    SABUTILS_EXPORT QString getFile( const QDir &relToDir, QXmlStreamReader &reader, QString *origFile = nullptr );
    SABUTILS_EXPORT QString getFile( const QDir &relToDir, const QString &file );

    struct SABUTILS_EXPORT SDateSearchOptions
    {
        SDateSearchOptions() {}
        SDateSearchOptions( bool includeHuristics, bool includeDateTimeFormat ) :
            fIncludeDateTimeFormats( includeDateTimeFormat ),
            fIncludeHuristics( includeHuristics )
        {
        }
        bool fIncludeDateTimeFormats{ true };
        bool fIncludeHuristics{ true };
        bool fAllowYearOnly{ false };
        bool fAllowMonthYearOnly{ false };
    };

    SABUTILS_EXPORT QDateTime getDateTime( const QStringRef &str, QXmlStreamReader &reader, bool optional );
    SABUTILS_EXPORT QDateTime getDateTime( const QString &str, QXmlStreamReader &reader, bool optional );
    SABUTILS_EXPORT QDateTime getDateTime( const QString &str );

    SABUTILS_EXPORT QDate getDate( const QString &str, const SDateSearchOptions &options = {} );
    SABUTILS_EXPORT QTime getTime( const QString &str, const SDateSearchOptions &options = {} );

    SABUTILS_EXPORT QStringList getDateTimeFormats();
    SABUTILS_EXPORT QStringList getTimeFormats( const SDateSearchOptions &options = {} );
    SABUTILS_EXPORT QStringList getHuristicDateFormats();
    SABUTILS_EXPORT QStringList getDateFormats( const SDateSearchOptions &options = {} );

    SABUTILS_EXPORT int autoSize( QTableView *table, int minWidth = -1 );   // autoSize( table, table->horizontalHeader )
    SABUTILS_EXPORT int autoSize( QTreeView *treeView, int minWidth = -1 );   // autoSize( table, table->header )
    SABUTILS_EXPORT int autoSize( QAbstractItemView *view, int minWidth = -1 );   // autoSize( table, table->horizontalHeader )
    SABUTILS_EXPORT int autoSize( QAbstractItemView *view, QHeaderView *header, int minWidth = -1 );
    SABUTILS_EXPORT int autoSize( QComboBox *comboBox, int minNumChars = -1 );

    SABUTILS_EXPORT QTreeWidgetItem *nextVisibleItem( QTreeWidgetItem *item );
    SABUTILS_EXPORT void fetchMore( QAbstractItemModel *model, int maxFetches = 3 );

    template< typename T >
    std::set< T > toSet( const QSet< T > &values )
    {
        std::set< T > retVal;
        for ( auto val : values )
            retVal.insert( val );
        return retVal;
    }

    template< typename T >
    std::set< T > toSet( const QList< T > &values )
    {
        std::set< T > retVal;
        for ( auto val : values )
            retVal.insert( val );
        return retVal;
    }

    class SABUTILS_EXPORT noCaseQStringCmp
    {
    public:
        bool operator()( const QString &s1, const QString &s2 ) const { return s1.compare( s2, Qt::CaseInsensitive ) < 0; }
    };

    SABUTILS_EXPORT QStringList splitLineCSV( const QString &line );
    SABUTILS_EXPORT size_t SizeOf( const QString &str );
    SABUTILS_EXPORT size_t SizeOf( const QDateTime &str );
    SABUTILS_EXPORT QString fromHtmlEscaped( const QString &str );

    struct CFileInfoCaseInsensitiveHash
    {
        size_t operator()( const QString &str ) const { return qHash( QFileInfo( str ).absoluteFilePath().toLower() ); }
    };

    struct CFileInfoCaseInsensitiveLessThan
    {
        bool operator()( const QString &lhs, const QString &rhs ) const { return QFileInfo( lhs ).absoluteFilePath().compare( QFileInfo( rhs ).absoluteFilePath(), Qt::CaseInsensitive ) < 0; }
    };

    struct CFileInfoCaseInsensitiveEqual
    {
        size_t operator()( const QString &lhs, const QString &rhs ) const { return QFileInfo( lhs ).absoluteFilePath().compare( QFileInfo( rhs ).absoluteFilePath(), Qt::CaseInsensitive ) == 0; }
    };

    SABUTILS_EXPORT void move( QSettings &settings, const QString &subGroup, const QString &key, bool overwrite );
    SABUTILS_EXPORT void copy( QSettings &from, QSettings &to, bool overwrite );

    SABUTILS_EXPORT int itemCount( QAbstractItemModel *model, bool rowCountOnly );

    SABUTILS_EXPORT QStringList getHeadersForModel( QAbstractItemModel *model );
    SABUTILS_EXPORT void writeModel( QAbstractItemModel *model, QXmlStreamWriter &writer, const QString &keyName, const QString &plauralSuffix,
                                     const std::function< void( QAbstractItemModel *model, QXmlStreamWriter &writer, const QString &keyName, int rowNum ) > &writeRow =
                                         std::function< void( QAbstractItemModel *model, QXmlStreamWriter &writer, const QString &keyName, int rowNum ) >() );

    SABUTILS_EXPORT void expandAll( QTreeView *view );

    SABUTILS_EXPORT void updateTimer( int delayMS, QTimer *timer );

    SABUTILS_EXPORT void deleteLayoutAndItems( QLayout *layout );

    SABUTILS_EXPORT void appendToLog( QPlainTextEdit *te, const QString &txt, std::pair< QString, bool > &previousText, QTextStream *ts = nullptr );

    SABUTILS_EXPORT NODISCARD uint8_t *imageToPixels( const QImage &image );   // allocates the space, user is responsible for memory deletion using array delete

    SABUTILS_EXPORT QString getHexValue( intptr_t value );
    SABUTILS_EXPORT QString dumpArray( const char *title, const uint8_t *arr, const uint8_t *baseArray, int size, bool asRGB = false, int colsPerRow = 20 );

    template< typename T >
    void dumpRow( int currRow, const char *title, const T *array, int width, int height, int colsPerRow, const T *baseArray, int rowOffset )
    {
        if ( currRow < 0 )
            return;
        if ( currRow >= height )
            return;

        if ( baseArray == nullptr )
            baseArray = array;

        auto rowBytes = width * 4;
        auto numBytes = rowBytes * height;
        auto offset = currRow * rowBytes + rowOffset;
        if ( ( offset + rowBytes ) > numBytes )
            return;

        qDebug().noquote().nospace() << "Row: " << currRow << " : Offset: " << offset << ":\n" << dumpArray( title, (const uint8_t *)array + offset, (const uint8_t *)baseArray, rowBytes, true, colsPerRow );
    }

    SABUTILS_EXPORT void dumpImage( const char *title, const uint8_t *arr, int width, int height, const uint8_t *baseArray = nullptr );

    template< template< typename > class T1, typename T2 >
    std::unordered_set< T2 > hashFromList( const T1< T2 > &values )
    {
        return std::unordered_set< T2 >( { values.begin(), values.end() } );
    }

    SABUTILS_EXPORT void setDPIAwarenessToMode( int &argc, char **&argv, const char *mode );
}

template< std::size_t I = 0, typename... Tp >
inline typename std::enable_if< I == sizeof...( Tp ), void >::type printToDebug( QDebug & /*retVal*/, const std::tuple< Tp... > & /*value*/ )
{
    return;
}

template< std::size_t I = 0, typename... Tp >
    inline typename std::enable_if < I< sizeof...( Tp ), void >::type printToDebug( QDebug &debug, const std::tuple< Tp... > &value )
{
    if ( I != 0 )
        debug.nospace() << ", ";

    debug << std::get< I >( value );

    printToDebug< I + 1, Tp... >( debug, value );
}

template< typename... Tp >
QDebug operator<<( QDebug &debug, const std::tuple< Tp... > &value )
{
    debug.nospace() << "std::tuple(";
    printToDebug( debug, value );
    debug.nospace() << ")";
    return debug;
}

#endif
