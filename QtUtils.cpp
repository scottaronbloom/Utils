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

#include "QtUtils.h"
#include "FileUtils.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDir>
#include <QLocale>
#include <QRegularExpression>
#include <QAbstractItemModel>
#include <QSettings>
#include <QSize>
#include <QPoint>
#include <QColor>
#include <QTreeView>
#include <QTimer>
#include <QLayout>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QTableView>
#include <QHeaderView>
#include <QTreeWidget>
#include <QApplication>
#include <QDebug>
#include <QVariant>

#ifdef QT_XMLPATTERNS_LIB
    #include <QXmlQuery>
#endif

#include <cstring>

namespace NSABUtils
{
    QString allFilesFilter()
    {
        QString retVal = QObject::tr( "All Files" );
#ifdef WIN32
        retVal += " (*.*)";
#else
        retVal += " (*)";
#endif
        return retVal;
    }

    QString defaultFileDialogDir()
    {
        QString retVal;
#ifndef WIN32
        retVal += ".";
#endif
        return retVal;
    }

#ifdef QT_XMLPATTERNS_LIB
    QString getString( QXmlQuery &query, const QString &queryString, bool *aOK /*= nullptr */ )
    {
        query.setQuery( queryString );
        if ( !query.isValid() )
        {
            if ( aOK )
                *aOK = false;
            return QString();
        }
        QString retVal;
        query.evaluateTo( &retVal );
        retVal = retVal.trimmed();
        if ( aOK )
            *aOK = true;
        return retVal;
    }

    QStringList getStringList( QXmlQuery &query, const QString &queryString, bool *aOK /*= nullptr */ )
    {
        query.setQuery( queryString );
        if ( !query.isValid() )
        {
            if ( aOK )
                *aOK = false;
            return QStringList();
        }
        QStringList retVal;
        query.evaluateTo( &retVal );
        for ( auto &&ii : retVal )
            ii = ii.trimmed();
        if ( aOK )
            *aOK = true;
        return retVal;
    }

    std::set< QString > getStringSet( QXmlQuery &query, const QString &queryString, bool *aOK /*= nullptr */ )
    {
        auto tmp = getStringList( query, queryString, aOK /*= nullptr */ );
        std::set< QString > retVal = { tmp.begin(), tmp.end() };
        return retVal;
    }

    int getInt( QXmlQuery &query, const QString &queryString, bool *aOK /*= nullptr */ )
    {
        bool lclAOK = false;
        auto str = getString( query, queryString, &lclAOK );
        if ( aOK )
            *aOK = lclAOK;
        if ( !lclAOK )
            return 0;
        return getInt( str, aOK );
    }

    bool getBool( QXmlQuery &query, const QString &queryString, bool defaultVal /*=false*/ )
    {
        bool lclAOK = false;
        auto str = getString( query, queryString, &lclAOK );
        if ( !lclAOK )
            return defaultVal;
        return getBool( str, defaultVal );
    }

    double getDouble( QXmlQuery &query, const QString &queryString, bool *aOK /*= nullptr */ )
    {
        bool lclAOK = false;
        auto str = getString( query, queryString, &lclAOK );
        if ( aOK )
            *aOK = lclAOK;
        if ( !lclAOK )
            return 0.0;
        return getDouble( str, aOK );
    }

    std::list< std::pair< QString, QString > > getStringPairs( QXmlQuery &query, const QString &queryString1, const QString &queryString2, bool *aOK )
    {
        auto tmp = getStrings( query, QStringList() << queryString1 << queryString2, aOK );
        std::list< std::pair< QString, QString > > retVal;
        for ( auto &&ii : tmp )
        {
            retVal.push_back( std::make_pair( ii.front(), ii.back() ) );
        }
        return retVal;
    }

    std::list< std::list< QString > > getStrings( QXmlQuery &query, const QStringList &queries, bool *aOK )
    {
        std::list< std::list< QString > > retVal;
        size_t sz = std::string::npos;
        for ( auto &&currQuery : queries )
        {
            bool localAOK;
            auto currList = getStringList( query, currQuery, &localAOK );
            if ( !localAOK )
            {
                if ( aOK )
                    *aOK = false;
                return {};
            }
            if ( sz != std::string::npos )
            {
                if ( sz != currList.count() )
                {
                    if ( aOK )
                        *aOK = false;
                    return {};
                }
            }
            else
                sz = currList.size();

            auto curr = retVal.begin();
            for ( int ii = 0; ii < currList.count(); ++ii )
            {
                if ( curr == retVal.end() )
                {
                    retVal.push_back( { currList[ ii ] } );
                    curr = retVal.end();
                }
                else
                {
                    ( *curr ).push_back( currList[ ii ] );
                    curr++;
                }
            }
        }

        if ( aOK )
            *aOK = true;
        return retVal;
    }

    QString getFile( QXmlQuery &query, const QDir &relToDir, const QString &queryString, bool *aOK /*= nullptr */ )
    {
        bool localAOK = false;
        auto path = getString( query, queryString, &localAOK );
        if ( aOK )
            *aOK = localAOK;
        if ( !localAOK )
            return QString();

        return getFile( relToDir, path );
    }
#endif

    int getInt( const QStringRef &str, bool *aOK )
    {
        bool lclAOK;
        int retVal = str.toInt( &lclAOK );
        if ( !lclAOK )
            retVal = qRound( str.toDouble( &lclAOK ) );
        if ( !lclAOK )
            retVal = str.toInt( &lclAOK, 16 );
        if ( aOK )
            *aOK = lclAOK;
        if ( !lclAOK )
            return 0;
        return retVal;
    }

    int getInt( const QString &str, bool *aOK )
    {
        QStringRef tmp( &str );
        return getInt( tmp, aOK );
    }

    int getInt( const QString &str )
    {
        bool aOK;
        return getInt( str, &aOK );
    }

    int getInt( const QStringRef &str )
    {
        bool aOK;
        return getInt( str, &aOK );
    }

    int getInt( const QString &value, QXmlStreamReader &reader )
    {
        QStringRef tmp( &value );
        return getInt( tmp, reader );
    }

    int getInt( const QString &value, int defaultValue, QXmlStreamReader &reader )
    {
        QStringRef tmp( &value );
        return getInt( tmp, defaultValue, reader );
    }

    int getInt( const QStringRef &value, QXmlStreamReader &reader )
    {
        bool aOK;
        int retVal = getInt( value, &aOK );
        if ( !aOK )
        {
            reader.raiseError( QString( "Invalid value, expecting integer '%1'" ).arg( value.toString() ) );
            return 0;
        }
        return retVal;
    }

    int getInt( const QStringRef &value, int defaultValue, QXmlStreamReader & /*reader*/ )
    {
        bool aOK;
        int retVal = getInt( value, &aOK );
        if ( !aOK )
        {
            return defaultValue;
        }
        return retVal;
    }

    double getDouble( const QStringRef &str, bool *aOK )
    {
        bool lclAOK;
        double retVal = str.toDouble( &lclAOK );
        if ( aOK )
            *aOK = lclAOK;
        if ( !lclAOK )
            return 0.0;
        return retVal;
    }

    double getDouble( const QString &str, bool *aOK )
    {
        QStringRef tmp( &str );
        return getDouble( tmp, aOK );
    }

    double getDouble( const QString &str )
    {
        bool aOK;
        return getDouble( str, &aOK );
    }

    double getDouble( const QStringRef &str )
    {
        bool aOK;
        return getDouble( str, &aOK );
    }

    double getDouble( const QString &value, QXmlStreamReader &reader )
    {
        QStringRef tmp( &value );
        return getDouble( tmp, reader );
    }

    double getDouble( const QStringRef &value, QXmlStreamReader &reader )
    {
        bool aOK;
        double retVal = getDouble( value, &aOK );
        if ( !aOK )
        {
            reader.raiseError( QString( "Invalid value, expecting double '%1'" ).arg( value.toString() ) );
            return 0;
        }
        return retVal;
    }

    bool getBool( const QString &str, bool defaultVal )
    {
        if ( str.isEmpty() )
            return defaultVal;

        QString val = str.toLower();

        return val != "0" && val != "false" && val != "f" && val != "n" && val != "no" && val != "off";
    }

    bool getBool( const QStringRef &str, bool defaultVal )
    {
        return getBool( str.toString(), defaultVal );
    }

    QString getFile( const QString &relToDir, QXmlStreamReader &reader, QString *origFile )
    {
        return getFile( QDir( relToDir ), reader, origFile );
    }

    QString getFile( const QDir &relToDir, QXmlStreamReader &reader, QString *origFile )
    {
        QString file = reader.readElementText();
        if ( origFile )
            *origFile = file;
        return getFile( relToDir, file );
    }

    QString getFile( const QDir &relToDir, const QString &file )
    {
        QString retVal = file;
        if ( !retVal.isEmpty() && QFileInfo( retVal ).isRelative() )
        {
            retVal = relToDir.absoluteFilePath( retVal );
            if ( QFileInfo( retVal ).exists() )
                retVal = NFileUtils::canonicalFilePath( retVal );
        }
        return retVal;
    }

    QStringList getHuristicDateFormats( const QStringList &aFormats, const QStringList &bFormats, const QStringList &cFormats )
    {
        static auto separators = QStringList() << ":"
                                               << "/"
                                               << "-";
        QStringList retVal;
        for ( auto &&ii : aFormats )
        {
            for ( auto &&jj : bFormats )
            {
                for ( auto &&kk : cFormats )
                {
                    for ( auto &&ll : separators )
                    {
                        retVal << QString( "%1%4%2%4%3" ).arg( ii ).arg( jj ).arg( kk ).arg( ll );
                    }
                }
            }
        }
        return retVal;
    }

    QStringList getHuristicDateFormats( const QStringList &aFormats, const QStringList &bFormats )
    {
        static auto separators = QStringList() << ":"
                                               << "/"
                                               << "-";
        QStringList retVal;
        for ( auto &&ii : aFormats )
        {
            for ( auto &&jj : bFormats )
            {
                for ( auto &&ll : separators )
                {
                    retVal << QString( "%1%3%2%3" ).arg( ii ).arg( jj ).arg( ll );
                }
            }
        }
        return retVal;
    }

    QStringList getMonthYearFormats()
    {
        static auto yearFormats = QStringList() << "yyyy"
                                                << "yy";
        static auto monthFormats = QStringList() << "M"
                                                 << "MM";
        static QStringList sRetVal;
        if ( sRetVal.isEmpty() )
        {
            sRetVal << getHuristicDateFormats( yearFormats, monthFormats );
            sRetVal << getHuristicDateFormats( monthFormats, yearFormats );
        }
        return sRetVal;
    }

    QStringList getHuristicDateFormats()
    {
        static auto yearFormats = QStringList() << "yyyy"
                                                << "yy";
        static auto monthFormats = QStringList() << "M"
                                                 << "MM";
        static auto dateFormats = QStringList() << "dd"
                                                << "d";
        static QStringList sRetVal;
        if ( sRetVal.isEmpty() )
        {
            sRetVal << getHuristicDateFormats( yearFormats, monthFormats, dateFormats );
            sRetVal << getHuristicDateFormats( yearFormats, dateFormats, monthFormats );
            sRetVal << getHuristicDateFormats( monthFormats, yearFormats, dateFormats );
            sRetVal << getHuristicDateFormats( monthFormats, dateFormats, yearFormats );
            sRetVal << getHuristicDateFormats( dateFormats, yearFormats, monthFormats );
            sRetVal << getHuristicDateFormats( dateFormats, monthFormats, yearFormats );
        }
        return sRetVal;
    }

    QStringList getDateFormats( const SDateSearchOptions &options )
    {
        QStringList retVal;
        QLocale locale;
        retVal << "ddd MMM d yyyy" << locale.dateFormat( QLocale::ShortFormat ) << locale.dateFormat( QLocale::LongFormat )
               << "yyyy-MM-dd"
               //<< Qt::ISODateWithMs - NA on dates
               << "dd MMM yyyy"
               << "yyyy-M-d"
               << "yyyy/MM/dd";
        if ( options.fIncludeHuristics )
            retVal << getHuristicDateFormats();
        if ( options.fAllowYearOnly )
            retVal << "yyyy"
                   << "yy";
        if ( options.fAllowMonthYearOnly )
            retVal << getHuristicDateFormats();
        if ( options.fIncludeDateTimeFormats )
            retVal << getDateTimeFormats();

        return retVal;
    }

    QStringList getTimeFormats( const SDateSearchOptions &options )
    {
        QStringList retVal;
        QLocale locale;
        retVal << "HH:mm:ss" << locale.timeFormat( QLocale::ShortFormat ) << locale.timeFormat( QLocale::LongFormat ) << "h:m:s"
               << "hh:mm:ss"
               << "HH:mm:ss.zzz"
               << "H:m:s"
               << "h:m:s AP"
               << "hh:mm:ss AP"
               << "H:m:s AP"
               << "h:m:s ap"
               << "hh:mm:ss ap"
               << "H:m:s ap";
        if ( options.fIncludeDateTimeFormats )
            retVal << getDateTimeFormats();
        return retVal;
    }

    QStringList getDateTimeFormats()
    {
        QStringList retVal;
        QLocale locale;
        retVal << "ddd MMM d yyyy HH:mm:ss" << locale.dateTimeFormat( QLocale::ShortFormat ) << locale.dateTimeFormat( QLocale::LongFormat ) << "yyyy-MM-ddThh:mm:ss"
               << "yyyy-MM-ddTHH:mm:ss"
               << "yyyy-MM-ddTh:m:s"
               << "yyyy-MM-ddTH:m:s"

               << "yyyy-M-dThh:mm:ss"
               << "yyyy-M-dTHH:mm:ss"
               << "yyyy-M-dTh:m:s"
               << "yyyy-M-dTH:m:s"

               << "yyyy-MM-dd"
               << "yyyy-M-d"
               << "yyyy/MM/dd"
               << "yyyy/M/d";
        return retVal;
    }

    QDateTime getDateTime( const QString &dateString )
    {
        auto formats = getDateTimeFormats();
        QDateTime retVal;
        for ( auto &&format : formats )
        {
            retVal = QDateTime::fromString( dateString, format );
            if ( retVal.isValid() )
                return retVal;
        }
        return retVal;
    }

    QDate getDate( const QString &string, const SDateSearchOptions &options )
    {
        if ( string.isEmpty() )
            return {};
        auto formats = getDateFormats( options );
        QDate retVal;
        for ( auto &&format : formats )
        {
            if ( options.fIncludeDateTimeFormats )
                retVal = QDateTime::fromString( string, format ).date();
            else
                retVal = QDate::fromString( string, format );
            if ( retVal.isValid() )
            {
                // qDebug() << string << "=" << retVal;
                return retVal;
            }
        }
        return retVal;
    }

    QTime getTime( const QString &string, const SDateSearchOptions &options )
    {
        auto formats = getTimeFormats( options );
        QTime retVal;
        for ( auto &&format : formats )
        {
            if ( options.fIncludeDateTimeFormats )
                retVal = QDateTime::fromString( string, format ).time();
            else
                retVal = QTime::fromString( string, format );
            if ( retVal.isValid() )
                return retVal;
        }
        return retVal;
    }

    QDateTime getDateTime( const QStringRef &value, QXmlStreamReader &reader, bool optional )
    {
        QDateTime retVal = getDateTime( value.toString() );
        if ( !retVal.isValid() )
        {
            if ( !optional )
                reader.raiseError( QString( "Invalid value, expecting date '%1'" ).arg( value.toString() ) );
            return {};
        }
        return retVal;
    }

    QDateTime getDateTime( const QString &value, QXmlStreamReader &reader, bool optional )
    {
        return getDateTime( QStringRef( &value ), reader, optional );
    }

    QStringList splitLineCSV( const QString &line )
    {
        QString temp = line;
        QString field;
        QStringList field_list;

        // regex explaination
        //
        //    /(?:^|,)(\"(?:[^\"]+|\"\")*\"|[^,]*)/g
        //        (?:^|,) Non-capturing group
        //            1st Alternative: ^
        //                ^ assert position at start of the string
        //            2nd Alternative: ,
        //                , matches the character , literally
        //        1st Capturing group (\"(?:[^\"]+|\"\")*\"|[^,]*)
        //            1st Alternative: \"(?:[^\"]+|\"\")*\"
        //                \" matches the character " literally
        //                (?:[^\"]+|\"\")* Non-capturing group
        //                    Quantifier: * Between zero and unlimited times, as many times as possible, giving back as needed [greedy]
        //                    1st Alternative: [^\"]+
        //                        [^\"]+ match a single character not present in the list below
        //                            Quantifier: + Between one and unlimited times, as many times as possible, giving back as needed [greedy]
        //                            \" matches the character " literally
        //                    2nd Alternative: \"\"
        //                        \" matches the character " literally
        //                        \" matches the character " literally
        //                \" matches the character " literally
        //            2nd Alternative: [^,]*
        //                [^,]* match a single character not present in the list below
        //                    Quantifier: * Between zero and unlimited times, as many times as possible, giving back as needed [greedy]
        //                    , the literal character ,
        //        g modifier: global. All matches (don't return on first match)
        //

        QString regex = "(?:^|,)(\"(?:[^\"]+|\"\")*\"|[^,]*)";

        QRegularExpression re( regex );

        if ( temp.right( 1 ) == "\n" )
            temp.chop( 1 );

        QRegularExpressionMatchIterator it = re.globalMatch( temp );

        while ( it.hasNext() )
        {
            QRegularExpressionMatch match = it.next();
            if ( match.hasMatch() )
            {
                field = match.captured( 1 );
                if ( field.left( 1 ) == "\"" && field.right( 1 ) == "\"" )
                    field = field.mid( 1, field.length() - 2 );
                field_list.push_back( field );
            }
        }

        return field_list;
    }

    size_t SizeOf( const QDateTime & /*dt*/ )
    {
        return sizeof( QDateTime );   // NOT ACCURATE
    }

    size_t SizeOf( const QString &str )
    {
        size_t retVal = sizeof( QString );
        retVal += str.capacity() * sizeof( QChar );
        return retVal;
    }

    QString fromHtmlEscaped( const QString &str )
    {
        QString retVal = str;
        QRegularExpression regExp( "(?<lt>\\&lt\\;)|(?<gt>\\&gt\\;)|(?<amp>\\&amp\\;)|(?<quot>\\&quot\\;)", QRegularExpression::PatternOption::CaseInsensitiveOption );
        auto match = regExp.match( str, 0 );

        while ( match.hasMatch() )
        {
            if ( !match.captured( "lt" ).isEmpty() )
            {
                retVal.replace( match.capturedStart( "lt" ), match.capturedLength( "lt" ), "<" );
            }
            else if ( !match.captured( "gt" ).isEmpty() )
            {
                retVal.replace( match.capturedStart( "gt" ), match.capturedLength( "gt" ), ">" );
            }
            else if ( !match.captured( "amp" ).isEmpty() )
            {
                retVal.replace( match.capturedStart( "amp" ), match.capturedLength( "amp" ), "&" );
            }
            else if ( !match.captured( "quot" ).isEmpty() )
            {
                retVal.replace( match.capturedStart( "quot" ), match.capturedLength( "quot" ), "\"" );
            }
            match = regExp.match( retVal, match.capturedStart() + 1 );
        }

        return retVal;
    }

    void move( QSettings &settings, const QString &subGroup, const QString &key, bool overwrite )
    {
        if ( settings.contains( key ) )
        {
            auto value = settings.value( key, true );
            if ( !overwrite )
            {
                settings.beginGroup( subGroup );
                bool contains = settings.contains( key );
                settings.endGroup();
                if ( contains )
                    return;
            }

            settings.beginGroup( subGroup );
            settings.remove( key );
            settings.setValue( key, value );
            settings.endGroup();
        }
    }

    void copy( QSettings &from, QSettings &to, bool overwrite )
    {
        auto childKeys = from.childKeys();
        for ( auto &&ii : childKeys )
        {
            if ( !overwrite && to.contains( ii ) )
                continue;
            auto value = from.value( ii );
            to.setValue( ii, value );
        }
        auto childGroups = from.childGroups();
        for ( auto &&ii : childGroups )
        {
            from.beginGroup( ii );
            to.beginGroup( ii );
            copy( from, to, overwrite );
            from.endGroup();
            to.endGroup();
        }
    }

    int itemCount( const QModelIndex &idx, bool rowCountOnly, const std::pair< std::function< bool( const QVariant &value ) >, int > &excludeFunc )
    {
        bool excluded = false;
        if ( excludeFunc.first )
        {
            excluded = excludeFunc.first( idx.data( excludeFunc.second ) );
        }

        int retVal = excluded ? 0 : 1;   // 1 counts self
        for ( int ii = 0; ii < idx.model()->rowCount( idx ); ++ii )
        {
            if ( rowCountOnly )
            {
                retVal += itemCount( idx.model()->index( ii, 0, idx ), rowCountOnly, excludeFunc );
            }
            else
            {
                for ( int jj = 0; jj < idx.model()->columnCount(); ++jj )
                {
                    retVal += itemCount( idx.model()->index( ii, jj, idx ), rowCountOnly, excludeFunc );
                }
            }
        }
        return retVal;
    }

    int itemCount( QAbstractItemModel *model, bool rowCountOnly, const std::pair< std::function< bool( const QVariant &value ) >, int > &excludeFunc )
    {
        if ( !model )
            return 0;
        int retVal = 0;
        for ( int ii = 0; ii < model->rowCount(); ++ii )
        {
            if ( rowCountOnly )
            {
                retVal += itemCount( model->index( ii, 0, QModelIndex() ), rowCountOnly, excludeFunc );
            }
            else
            {
                for ( int jj = 0; jj < model->columnCount(); ++jj )
                {
                    retVal += itemCount( model->index( ii, jj, QModelIndex() ), rowCountOnly, excludeFunc );
                }
            }
        }
        return retVal;
    }

    QStringList getHeadersForModel( QAbstractItemModel *model )
    {
        QStringList retVal;
        for ( int ii = 0; ii < model->columnCount(); ++ii )
        {
            auto header = model->headerData( ii, Qt::Orientation::Horizontal ).toString();
            header = header.replace( "/", "" );
            if ( header.isEmpty() )
                header = QString( "Col%1" ).arg( ii );
            retVal << header;
        }
        return retVal;
    }

    void writeModel(
        QAbstractItemModel *model, QXmlStreamWriter &writer, const QString &keyName, const QString &plauralSuffix,
        const std::function< void( QAbstractItemModel *model, QXmlStreamWriter &writer, const QString &keyName, int rowNum ) > &writeRow )
    {
        QStringList headers = getHeadersForModel( model );

        writer.writeStartElement( keyName + plauralSuffix );   // base path is singular

        for ( int row = 0; row < model->rowCount(); ++row )
        {
            if ( writeRow )
                writeRow( model, writer, keyName, row );
            else
            {
                writer.writeStartElement( keyName );
                for ( int col = 0; col < model->columnCount(); ++col )
                {
                    auto value = model->index( row, col ).data().toString();
                    writer.writeTextElement( headers[ col ], value );
                }
                writer.writeEndElement();
            }
        }
        writer.writeEndElement();
    }

    void expandAll( const QModelIndex &index, QTreeView *view )
    {
        if ( !view || !view->model() )
            return;

        view->setExpanded( index, true );

        auto model = view->model();
        auto rowCount = model->rowCount( index );
        auto columnCount = model->columnCount( index );
        for ( auto ii = 0; ii < rowCount; ++ii )
        {
            for ( auto jj = 0; jj < columnCount; ++jj )
            {
                auto childIndex = model->index( ii, jj, index );
                expandAll( childIndex, view );
            }
        }
    }

    void expandAll( QTreeView *view )
    {
        if ( !view || !view->model() )
            return;

        return expandAll( QModelIndex(), view );
    }

    size_t CCaseInsensitiveHash::operator()( const QString &str ) const
    {
        return qHash( QDir( str ).absolutePath().toLower() );
    }

    size_t CCaseInsensitiveEqual::operator()( const QString &lhs, const QString &rhs ) const
    {
        return QDir( lhs ).absolutePath().compare( QDir( rhs ).absolutePath(), Qt::CaseInsensitive ) == 0;
    }

    void updateTimer( int delayMS, QTimer *timer )
    {
        if ( !timer )
            return;

        bool isActive = timer->isActive();
        timer->stop();
        if ( delayMS >= 0 )
        {
            timer->setInterval( delayMS );
            if ( isActive )
                timer->start();
        }
    }

    void deleteLayoutAndItems( QLayout *layout )
    {
        if ( !layout )
            return;

        QLayoutItem *child = nullptr;
        while ( ( child = layout->takeAt( 0 ) ) != nullptr )
        {
            if ( child->widget() )
                delete child->widget();
            else if ( child->layout() )
                deleteLayoutAndItems( child->layout() );
            delete child;
        }
        delete layout;
        layout = nullptr;
    }

    void appendToLog( QPlainTextEdit *textEdit, const QString &txt, std::pair< QString, bool > &previousText, QTextStream *ts )
    {
        if ( !textEdit )
            return;

        if ( ts )
        {
            *ts << txt;
            ts->flush();
        }
        auto lineStart = 0;
        if ( txt == "\n" || txt == "\r\n" )
        {
            textEdit->appendPlainText( previousText.first + txt );
            lineStart = txt.length();
        }
        else
        {
            auto regEx = QRegularExpression( "[\r\n]" );
            auto pos = txt.indexOf( regEx, lineStart );
            while ( pos != -1 )
            {
                while ( ( txt.at( pos ) == '\r' ) && ( ( ( pos + 1 ) < txt.length() ) && ( txt.at( pos + 1 ) == '\r' ) ) )
                    pos++;

                if ( previousText.second )
                {
                    previousText.second = false;
                    textEdit->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
                    textEdit->moveCursor( QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
                    textEdit->moveCursor( QTextCursor::End, QTextCursor::KeepAnchor );
                    textEdit->textCursor().removeSelectedText();
                    textEdit->textCursor().deletePreviousChar();
                    textEdit->moveCursor( QTextCursor::End );
                }

                QString tmp = txt.mid( lineStart, pos - lineStart );

                bool changeLineStart = false;
                int skip = 1;
                if ( txt.at( pos ) == '\n' )   // just reset lineStart;
                {
                    changeLineStart = true;
                }
                else if ( txt.at( pos ) == '\r' )
                {
                    // if its "\r\n" treat it just like a "\n"
                    if ( ( pos + 1 ) < txt.length() && txt.at( pos + 1 ) == '\n' )
                    {
                        changeLineStart = true;
                        skip = 2;
                    }
                    else   // just a \r throw the line out
                    {
                        // skip this line
                        changeLineStart = true;
                        previousText.second = true;
                    }
                }
                else
                {
                    changeLineStart = false;
                }

                QString subStr = previousText.first + txt.mid( lineStart, pos - lineStart );
                if ( !subStr.isEmpty() )
                {
                    // qDebug() << subStr;
                    textEdit->appendPlainText( subStr );
                    previousText.first.clear();
                }
                else
                {
                    textEdit->appendPlainText( "\n" );
                    previousText.first.clear();
                }

                if ( changeLineStart )
                    lineStart = pos + skip;

                pos = txt.indexOf( regEx, pos + skip );
            }
        }
        previousText.first += txt.mid( lineStart );
        textEdit->moveCursor( QTextCursor::End );
    }

    uint8_t *imageToPixels( const QImage &image )   // allocates the space, user is responsible for memory deletion using array delete
    {
        Q_ASSERT( sizeof( uchar ) == sizeof( uint8_t ) );

        auto imageSize = static_cast< size_t >( image.width() * image.height() * 4 * sizeof( uint8_t ) );
        auto retVal = new uint8_t[ imageSize ];
        std::memcpy( retVal, image.bits(), imageSize );
        return retVal;
    }

    QString getHexValue( intptr_t value )
    {
        auto retVal = QString( "%1" ).arg( value, 5, 16, QChar( '0' ) ).toUpper();
        retVal = "0x" + retVal;
        return retVal;
    }

    QString dumpArray( const char *title, const uint8_t *arr, const uint8_t *baseArray, int size, bool asRGB /*= false*/, int colsPerRow /*= 20*/ )
    {
        static int hitCount = 0;
        auto retVal = QString( "HitCount: %1 - %2 - Array: 0x%3\n" ).arg( hitCount++ ).arg( title ).arg( arr - baseArray, 8, 16, QChar( '0' ) );
        if ( asRGB )
            colsPerRow /= 4;
        int colCount = 0;
        int pixelMultiplier = 1;
        if ( asRGB )
            pixelMultiplier = 4;
        for ( int ii = 0; ii < size; ++ii )
        {
            auto offset = &arr[ ii ] - baseArray;
            // auto memZero = (intptr_t)&arr[ii];
            if ( colCount == 0 )
                retVal += QString( "%1-%2: " ).arg( getHexValue( offset ) );   // .arg( getHexValue( memZero ) );
            else
                retVal += " ";

            QString curr;
            if ( asRGB && ( ( ii + 3 ) < size ) )
            {
                curr = QString( "Col: %1 - Offset: %2 - " ).arg( ii / 4 ).arg( ii );

                curr += QString( "%1" ).arg( arr[ ii + 0 ], 2, 16, QChar( '0' ) ).toUpper();
                curr += QString( "%1" ).arg( arr[ ii + 1 ], 2, 16, QChar( '0' ) ).toUpper();
                curr += QString( "%1" ).arg( arr[ ii + 2 ], 2, 16, QChar( '0' ) ).toUpper();
                curr += QString( "%1" ).arg( arr[ ii + 3 ], 2, 16, QChar( '0' ) ).toUpper();
                ii += 3;
            }
            else
            {
                curr = QString( "%1" ).arg( arr[ ii ], 2, 16, QChar( '0' ) ).toUpper();
            }
            retVal += curr;

            if ( colCount == colsPerRow - 1 )
            {
                retVal += "\n";
                colCount = -1;
            }
            colCount++;
        }
        return retVal;
    }
    void dumpImage( const char *title, const uint8_t *arr, int width, int height, const uint8_t *baseArray /*= nullptr */ )
    {
        if ( !arr )
            return;

        for ( int ii = 0; ii < height; ++ii )
        {
            dumpRow( ii, title, arr, width, height, width / 4, baseArray, 0 );
        }
    }

    void fetchMore( QAbstractItemModel *model, int maxFetches )
    {
        if ( !model )
            return;
        int numFetches = 0;
        while ( numFetches < maxFetches && model->canFetchMore( QModelIndex() ) )
        {
            model->fetchMore( QModelIndex() );
            numFetches++;
        }
    }

    void setDPIAwarenessToMode( int &argc, char **&argv, const char *mode )
    {
        bool dpiAwarenessFound = false;
        for ( int ii = 1; ii < argc; ++ii )
        {
            if ( strncmp( argv[ ii ], "-platform", 9 ) == 0 )
            {
                if ( ( ii + 1 ) < argc )
                {
                    std::string arg = argv[ ii + 1 ];
                    if ( arg.find( "dpiawareness=" ) != std::string::npos )
                        dpiAwarenessFound = true;
                }
            }
        }

        if ( !dpiAwarenessFound )
        {
            char **newArgv = new char *[ argc + 2 ];
            for ( int ii = 0; ii < argc; ++ii )
            {
                newArgv[ ii ] = new char[ strlen( argv[ ii ] ) + 1 ];
                strcpy( newArgv[ ii ], argv[ ii ] );
            }
            newArgv[ argc ] = new char[ strlen( "-platform" ) + 1 ];
            strcpy( newArgv[ argc ], "-platform" );
            newArgv[ argc + 1 ] = new char[ strlen( "windows:dpiawareness=" ) + strlen( mode ) + 1 ];
            strcpy( newArgv[ argc + 1 ], "windows:dpiawareness=" );
            strcat( newArgv[ argc + 1 ], mode );
            argc += 2;
            argv = newArgv;
        }
    }

    QString getFirstString( const QVariant &value )
    {
        if ( value.type() == QVariant::StringList )
        {
            auto retVal = value.toStringList();
            if ( retVal.isEmpty() )
                return {};
            return retVal.front();
        }
        else
            return value.toString();
    }

    int autoSize( QAbstractItemView *view, QHeaderView *header, int minWidth /*=-1*/ )
    {
        if ( !view || !view->model() || !header )
            return -1;

        auto model = view->model();
        fetchMore( model, 3 );

        auto numCols = model->columnCount();
        bool stretchLastColumn = header->stretchLastSection();
        header->setStretchLastSection( false );

        header->resizeSections( QHeaderView::ResizeToContents );
        int numVisibleColumns = 0;
        for ( int ii = 0; ii < numCols; ++ii )
        {
            if ( header->isSectionHidden( ii ) )
                continue;
            numVisibleColumns++;
            if ( numVisibleColumns > 1 )
                break;
        }
        bool dontResize = ( numVisibleColumns <= 1 ) && stretchLastColumn;

        int totalWidth = 0;
        for ( int ii = 0; ii < numCols; ++ii )
        {
            if ( header->isSectionHidden( ii ) )
                continue;

            int contentSz = view->sizeHintForColumn( ii );
            int headerSz = header->sectionSizeHint( ii );

            auto newWidth = std::max( { minWidth, contentSz, headerSz } );
            totalWidth += newWidth;

            if ( !dontResize )
                header->resizeSection( ii, newWidth );
        }
        if ( stretchLastColumn )
            header->setStretchLastSection( true );
        return totalWidth;
    }

    int autoSize( QTableView *table, int minWidth /*= -1*/ )
    {
        return autoSize( table, table->horizontalHeader(), minWidth );
    }

    int autoSize( QTreeView *tree, int minWidth /*= -1*/ )
    {
        return autoSize( tree, tree->header(), minWidth );
    }

    int autoSize( QAbstractItemView *view, int minWidth /*= -1*/ )
    {
        auto treeView = dynamic_cast< QTreeView * >( view );
        if ( treeView )
            return autoSize( treeView, minWidth );

        auto tableView = dynamic_cast< QTableView * >( view );
        if ( tableView )
            return autoSize( tableView, minWidth );
        return -1;
    }

    int autoSize( QComboBox *comboBox, int minNumChars /*= -1*/ )
    {
        if ( !comboBox )
            return -1;

        comboBox->view()->setTextElideMode( Qt::ElideNone );
        auto prevPolicy = comboBox->sizeAdjustPolicy();
        if ( prevPolicy == QComboBox::AdjustToContents )
            comboBox->setSizeAdjustPolicy( QComboBox::AdjustToContentsOnFirstShow );
        comboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
        qApp->processEvents( QEventLoop::ExcludeUserInputEvents );

        if ( ( minNumChars == -1 ) && !comboBox->placeholderText().isEmpty() && ( comboBox->count() == 0 ) )
        {
            bool hasIcon = comboBox->sizeAdjustPolicy() == QComboBox::AdjustToMinimumContentsLengthWithIcon;
            const QFontMetrics &fm = comboBox->fontMetrics();
            minNumChars = comboBox->placeholderText().length();
        }

        if ( minNumChars != -1 )
            comboBox->setMinimumContentsLength( minNumChars );

        return comboBox->width();
    }

    QTreeWidgetItem *nextVisibleItem( QTreeWidgetItem *item )
    {
        if ( !item )
            return nullptr;

        auto treeWidget = item->treeWidget();
        if ( item && item->isHidden() )
        {
            QTreeWidgetItem *sibling = item;
            while ( sibling && sibling->isHidden() )
                sibling = treeWidget->topLevelItem( treeWidget->indexOfTopLevelItem( sibling ) + 1 );
            item = sibling;
        }

        if ( item && item->isHidden() )
        {
            QTreeWidgetItem *sibling = item;
            while ( sibling && sibling->isHidden() )
                sibling = treeWidget->topLevelItem( treeWidget->indexOfTopLevelItem( sibling ) - 1 );
            item = sibling;
        }
        return item;
    }

}
