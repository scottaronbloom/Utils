// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#include "UtilityModels.h"
#include "QtUtils.h"

#include <QFileInfo>
#include <QKeyEvent>
#include <QLineEdit>
#include <QAbstractButton>
#include <QTimer>
#include <QDir>

CMoveStringListModel::CMoveStringListModel( QObject * parent ) :
    QStringListModel( parent )
{
}

bool CMoveStringListModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
    QModelIndex pidx = parent;
    if ( parent.isValid() && row == -1 && column == -1 )
    {
        row = parent.row();
        column = parent.column();
        pidx = parent.parent();
    }
    return QStringListModel::dropMimeData( data, action, row, column, pidx );
}

bool CMoveStringListModel::setData(const QModelIndex &index, const QVariant &value, int role )
{
    if ( index.data( role ) == value.toString() )
        return true;
    return QStringListModel::setData( index, value, role );
}

void CMoveStringListModel::addRow( const QString & value )
{
    insertRows( rowCount(), 1 );
    setData( index( rowCount() - 1, 0 ), value );
}

QVariant CMoveStringListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return tr( "Name" );
    return QStringListModel::headerData( section, orientation, role );
}

CStringListModel::CStringListModel( QObject * parent ) :
    QStringListModel( parent )
{
}

void CStringListModel::addRow()
{
    addRow( QString() );
}

void CStringListModel::addRow( const QString & value )
{
    insertRows( rowCount(), 1 );
    setData( index( rowCount() - 1, 0 ), value );
}

void CStringListModel::copy( const CStringListModel * rhs )
{
    setStringList( rhs ? rhs->stringList() : QStringList() );
}

QString CStringListModel::at( int ii ) const
{
    return index( ii ).data().toString();
}

bool CStringListModel::operator==( const CStringListModel & rhs ) const
{
    return stringList() == rhs.stringList();
}

CCheckableStringListModel::CCheckableStringListModel( QObject * parent ) :
    QStringListModel( parent )
{
    QObject::disconnect( this, &CCheckableStringListModel::dataChanged, this, &CCheckableStringListModel::sigChanged );
    QObject::connect   ( this, &CCheckableStringListModel::dataChanged, this, &CCheckableStringListModel::sigChanged );

    QObject::disconnect( this, &CCheckableStringListModel::modelReset, this, &CCheckableStringListModel::sigChanged );
    QObject::connect   ( this, &CCheckableStringListModel::modelReset, this, &CCheckableStringListModel::sigChanged );

    QObject::disconnect( this, &CCheckableStringListModel::rowsInserted, this, &CCheckableStringListModel::sigChanged );
    QObject::connect   ( this, &CCheckableStringListModel::rowsInserted, this, &CCheckableStringListModel::sigChanged );

    QObject::disconnect( this, &CCheckableStringListModel::rowsRemoved, this, &CCheckableStringListModel::sigChanged );
    QObject::connect   ( this, &CCheckableStringListModel::rowsRemoved, this, &CCheckableStringListModel::sigChanged );

    QObject::disconnect( this, &CCheckableStringListModel::rowsMoved, this, &CCheckableStringListModel::sigChanged );
    QObject::connect   ( this, &CCheckableStringListModel::rowsMoved, this, &CCheckableStringListModel::sigChanged );

    QObject::disconnect( this, &CCheckableStringListModel::columnsInserted, this, &CCheckableStringListModel::sigChanged );
    QObject::connect   ( this, &CCheckableStringListModel::columnsInserted, this, &CCheckableStringListModel::sigChanged );

    QObject::disconnect( this, &CCheckableStringListModel::columnsRemoved, this, &CCheckableStringListModel::sigChanged );
    QObject::connect   ( this, &CCheckableStringListModel::columnsRemoved, this, &CCheckableStringListModel::sigChanged );

    QObject::disconnect( this, &CCheckableStringListModel::columnsMoved, this, &CCheckableStringListModel::sigChanged );
    QObject::connect   ( this, &CCheckableStringListModel::columnsMoved, this, &CCheckableStringListModel::sigChanged );
}

bool CCheckableStringListModel::isChecked( int rowNum ) const
{
    if ( rowNum > rowCount() )
        return false;

    auto key = stringList()[ rowNum ].toUpper();
    auto pos = fEnabled.find( key );
    if ( pos == fEnabled.end() )
        return false;

    return (*pos).second.second;
}

void CCheckableStringListModel::setStringList( const std::list< std::pair< QString, bool > > & values )
{
    beginResetModel();
    QStringList stringValues;
    for ( auto && ii : values )
    {
        stringValues.push_back( ii.first );
    }
    QStringListModel::setStringList( stringValues );
    for ( auto && ii : values )
    {
        setChecked( ii.first, ii.second, false );
    }
    endResetModel();
}

void CCheckableStringListModel::setAliasMap( const std::map< QString, QString > & map )
{
    fAliasMap = map;
    for( auto && ii : map )
    {
        fReverseAliasMap[ ii.second ] = ii.first;
    }
}

QString CCheckableStringListModel::getAlias( const QString & key ) const
{
    auto pos = fAliasMap.find( key );
    if ( pos == fAliasMap.end() )
        return key;
    return ( *pos ).second;
}

bool CCheckableStringListModel::setData(const QModelIndex &index, const QVariant &value, int role )
{
    if ( role != Qt::CheckStateRole )
        return QStringListModel::setData( index, value, role );

    bool toCheck = ( value.toInt() == Qt::Checked );
    if ( isChecked( index.row() ) != toCheck )
    {
        QString orig = index.data().toString();
        QString key = orig.toUpper();
        fEnabled[ key ] = std::make_pair( orig, toCheck );
        emit dataChanged( index, index );
    }
    return true;
}

QVariant CCheckableStringListModel::data( const QModelIndex & index, int role ) const
{
    if ( role != Qt::CheckStateRole )
        return QStringListModel::data( index, role );

    return isChecked( index.row() ) ? Qt::Checked : Qt::Unchecked;
}

Qt::ItemFlags CCheckableStringListModel::flags( const QModelIndex & index ) const
{
    Qt::ItemFlags retVal = QAbstractItemModel::flags( index );
    retVal &= ~Qt::ItemIsDragEnabled;
    retVal &= ~Qt::ItemIsEditable;
    retVal |= Qt::ItemIsUserCheckable;
    return retVal;
}

QStringList CCheckableStringListModel::getCheckedStrings( bool & allChecked ) const
{
    QStringList retVal;
    for( auto ii = fEnabled.begin(); ii != fEnabled.end(); ++ii )
    {
        if ( (*ii).second.second )
            retVal << getAlias( (*ii).second.first );
    }
    allChecked = retVal.size() == rowCount();
    return retVal;
}

QStringList CCheckableStringListModel::getCheckedStrings() const
{
    bool allChecked;
    return getCheckedStrings( allChecked );
}

bool CCheckableStringListModel::operator==( const CCheckableStringListModel & rhs ) const
{
    bool allChecked;
    auto lhs = getCheckedStrings( allChecked );
    QSet< QString > lhsSet( lhs.begin(), lhs.end() );
    auto rhsList = rhs.getCheckedStrings( allChecked );
    QSet< QString > rhsSet( rhsList.begin(), rhsList.end() );
    return lhsSet == rhsSet;
}

void CCheckableStringListModel::copy( const CCheckableStringListModel & rhs )
{
    QStringListModel::setStringList( rhs.stringList() );
    fEnabled = rhs.fEnabled;
}

bool CCheckableStringListModel::setChecked( QString item, bool checked, bool update )
{
    auto pos = stringList().indexOf( item );
    if ( pos == -1 )
    {
        pos = stringList().indexOf( item.toUpper() );
    }

    if ( pos == -1 )
    {
        auto ii = fReverseAliasMap.find( item );
        if ( ii == fReverseAliasMap.find( item ) )
            ii = fReverseAliasMap.find( item.toUpper() );

        if ( ii != fReverseAliasMap.end() )
            item = (*ii).second;

        pos = stringList().indexOf( item );
    }

    if ( ( pos ==-1 ) && update )
    {
        beginResetModel();
        endResetModel();
        return false;
    }

    auto key = item.toUpper();
    fEnabled[ key ] = std::make_pair( item, checked );
    if ( update )
        emit sigBlockFilterUpdates( true );

    bool found = false;
    for( int ii = 0; ii < rowCount(); ++ii )
    {
        if ( stringList()[ ii ] == item )
        {
            if ( update )
                emit dataChanged( index( ii, 0 ), index( ii, 0 ) );
            found = true;
            break;
        }
    }
    if ( !found && update )
    {
        beginResetModel();
        endResetModel();
    }
    if ( update )
        emit sigBlockFilterUpdates( false );
    return found;
}

void CCheckableStringListModel::setChecked( const QStringList & strings, bool checked, bool update )
{
    if ( update )
        beginResetModel();

    fEnabled.clear();
    for( int ii = 0; ii < strings.count(); ++ii )
    {
        setChecked( strings[ ii ], checked, false );
    }

    if ( update )
        endResetModel();
}

void CCheckableStringListModel::uncheckAll( bool update )
{
    fEnabled.clear();
    if ( update )
        emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) );
}

void CCheckableStringListModel::checkAll( bool update )
{
    fEnabled.clear();
    auto strings = stringList();

    if ( update )
        beginResetModel();

    for( int ii = 0; ii < strings.size(); ++ii )
    {
        fEnabled[ strings[ ii ].toUpper() ] = std::make_pair( strings[ ii ], true );
    }

    if ( update )
        endResetModel();
}

QVariant CCheckableStringListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return tr( "Name" );
    return QStringListModel::headerData( section, orientation, role );
}

CStringFilterModel::CStringFilterModel( QAbstractItemModel * sourceModel, QLineEdit * filter, QObject * parent ) :
    QSortFilterProxyModel( parent ),
    fFilter( filter )
{
    setFilterCaseSensitivity( Qt::CaseInsensitive );
    setSourceModel( sourceModel );
    setDynamicSortFilter( true );

    fTimer = new QTimer( this );
    fTimer->setInterval( 500 );
    fTimer->setSingleShot( true );
    connect( fTimer, &QTimer::timeout, this, &CStringFilterModel::slotFilterChanged );
    if( filter )
        connect( filter, &QLineEdit::textChanged, fTimer, static_cast<void (QTimer::*)()>(&QTimer::start) );
}

CStringFilterModel::CStringFilterModel( QAbstractItemModel * sourceModel, QLineEdit * filter, QAbstractButton * btn, QObject * parent ) :
    QSortFilterProxyModel( parent ),
    fFilter( filter )
{
    setFilterCaseSensitivity( Qt::CaseInsensitive );
    connect( btn, &QAbstractButton::clicked, this, &CStringFilterModel::slotFilterChanged );
    setSourceModel( sourceModel );
    setDynamicSortFilter( true );

    fTimer = new QTimer( this );
    fTimer->setInterval( 500 );
    fTimer->setSingleShot( true );
    connect( fTimer, &QTimer::timeout, this, &CStringFilterModel::slotFilterChanged );
    if( filter )
        connect( filter, &QLineEdit::textChanged, fTimer, static_cast<void (QTimer::*)()>(&QTimer::start) );
}

void CStringFilterModel::slotFilterChanged()
{
    QString filter = fFilter->text();
    if ( filter.isEmpty() )
        setFilterWildcard( QString() );
    else
        setFilterWildcard( filter );
}


CCheckableListView::CCheckableListView( QWidget * parent ) :
    QListView( parent )
{
}

void CCheckableListView::keyPressEvent(QKeyEvent *event)
{
    switch ( event->key() )
    {
    case Qt::Key_Space:
        if ( selectionModel() )
        {
            QModelIndexList indexes = selectionModel()->selectedRows();
            emit sigBlockFilterUpdates( true );
            for( const QModelIndex & idx : indexes )
            {
                if ( !edit(idx, AnyKeyPressed, event) )
                {
                    event->ignore();
                    return;
                }
            }
            emit sigBlockFilterUpdates( false );
            event->accept();
            return;
        }
        break;
    }
    QListView::keyPressEvent( event );
}

CCheckableTableView::CCheckableTableView( QWidget * parent ) :
    QTableView( parent )
{
}

void CCheckableTableView::keyPressEvent(QKeyEvent *event)
{
    switch ( event->key() )
    {
    case Qt::Key_Space:
        if ( selectionModel() )
        {
            QModelIndexList indexes = selectionModel()->selectedRows();
            emit sigBlockFilterUpdates( true );
            for( const QModelIndex & idx : indexes )
            {
                if ( !edit(idx, AnyKeyPressed, event) )
                {
                    event->ignore();
                    return;
                }
            }
            emit sigBlockFilterUpdates( false );
            event->accept();
            return;
        }
        break;
    }
    QTableView::keyPressEvent( event );
}

CCheckableTreeView::CCheckableTreeView( QWidget * parent ) :
    QTreeView( parent )
{
}

void CCheckableTreeView::keyPressEvent(QKeyEvent *event)
{
    switch ( event->key() )
    {
    case Qt::Key_Space:
        if ( selectionModel() )
        {
            QModelIndexList indexes = selectionModel()->selectedRows();
            emit sigBlockFilterUpdates( true );
            for( const QModelIndex & idx : indexes )
            {
                if ( !edit(idx, AnyKeyPressed, event) )
                {
                    event->ignore();
                    return;
                }
            }
            emit sigBlockFilterUpdates( false );
            event->accept();
            return;
        }
        break;
    }
    QTreeView::keyPressEvent( event );
}

CStringTupleModel::CStringTupleModel( const QStringList & columnNames, QObject * parent ) :
    QAbstractTableModel( parent ),
    fColumnNames( columnNames ),
    fIsKeyEditable( true )
{
}

CStringTupleModel::CStringTupleModel( int numColumns, QObject * parent ) :
    CStringTupleModel( QStringList( "Key" ), parent )
{
    for( int ii = 1; ii <= numColumns; ++ii )
        fColumnNames << QString( "Value%1" ).arg( ii );
}


QVariant CStringTupleModel::data( const QModelIndex &index, int role ) const
{
    if ( index.row() <= -1 || index.row() >= rowCount( index.parent() ) || index.column() <= -1 || index.column() >= columnCount( index.parent() ) )
        return QVariant();

    if ( role != Qt::DisplayRole && role != Qt::EditRole )
        return QVariant();

    return fData[ index.row() ][ index.column() ];
}

bool CStringTupleModel::setData(const QModelIndex &index, const QVariant &value, int role )
{
    if ( index.row() <= -1 || index.row() >= rowCount() || index.column() <= -1 || index.column() >= columnCount() )
        return false;

    if ( role != Qt::DisplayRole && role != Qt::EditRole )
        return false;

    if ( index.column() == 0 && ( fColumnNames.count() == 2 ) )
    {
        QString tmp = value.toString();
        int pos = value.toString().indexOf( '=' );

        if ( pos > 0 )
        {
            fData[ index.row() ][ 0 ] = tmp.left( pos );
            fData[ index.row() ][ 1 ] = tmp.mid( pos + 1 );
            emit dataChanged( createIndex( index.row(), 0 ), createIndex( index.row(), 1 ) );
        }
        else
        {
            fData[ index.row() ][ 0 ] = tmp;
            emit dataChanged( index, index );
        }
    }
    else
    {
        fData[ index.row() ][ index.column() ] = value.toString();
        emit dataChanged( index, index );
    }
    return true;
}

QVariant CStringTupleModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( section >= columnCount() || orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QAbstractTableModel::headerData( section, orientation, role );
    return fColumnNames[ section ];
}

bool CStringTupleModel::removeRows(int row, int count, const QModelIndex & parent )
{
    if ((count < 1) || (row < 0) || ((row + count) > rowCount( parent )))
        return false;

    beginRemoveRows( parent, row, row+count -1 );
    for ( int ii = 0; ii < count; ++ii )
    {
        fData.removeAt(row);
    }
    endRemoveRows();
    return true;
}

bool CStringTupleModel::removeRow(int row, const QModelIndex & parent )
{
    return removeRows( row, 1, parent );
}

void CStringTupleModel::clear()
{
    beginResetModel();
    fData.clear();
    endResetModel();
}

void CStringTupleModel::setValues( const QList< QStringList > & values )
{
    beginResetModel();
    fData = values;
    endResetModel();
}

bool CStringTupleModel::operator==( const CStringTupleModel & rhs ) const
{
    return fData == rhs.fData;
}

void CStringTupleModel::copy( const CStringTupleModel * rhs )
{
    beginResetModel();
    fColumnNames = rhs->fColumnNames;
    fIsKeyEditable = rhs->fIsKeyEditable;
    fData = rhs->fData;
    endResetModel();
}

void CStringTupleModel::addRow()
{
    QStringList tmp;
    for( int ii = 0; ii < fColumnNames.count(); ++ii )
        tmp << QString();
    addRow( tmp );
}

void CStringTupleModel::addRow( const QStringList & data)
{
    beginInsertRows( QModelIndex(), fData.count(), fData.count() );
    fData << data;
    endInsertRows();
}

void CStringTupleModel::setRow( const QString & key, const QStringList & values )
{
    for( int ii = 0; ii < fData.count(); ++ii )
    {
        if ( key == fData[ ii ][ 0 ] )
        {
            fData[ ii ] = QStringList() << key << values;
            emit dataChanged( index( ii, 1 ), index( ii, columnCount() ) );
            return;
        }
    }
    addRow( QStringList() << key << values );
}

Qt::ItemFlags CStringTupleModel::flags( const QModelIndex & index ) const
{
    auto retVal = QAbstractTableModel::flags( index );
    if ( !fIsKeyEditable && ( index.column() == 0 ) )
        retVal &= ~Qt::ItemIsEditable;
    else
        retVal |= Qt::ItemIsEditable;
    return retVal;
}

bool CStringTupleModel::loadXML( QXmlQuery & query, const std::list< std::pair< QString, bool > > & queries, const QDir & relToDir, QString & msg, bool clear )
{
#ifdef QT_XMLPATTERNS_LIB
    beginResetModel();
    if( clear )
        fData.clear();

    QStringList queryStrings;
    for( auto && ii : queries )
        queryStrings << ii.first;
    if( queryStrings.count() != fColumnNames.count() )
    {
        msg = "Invalid Query Setup";
        return false;
    }
    auto data = NQtUtils::getStrings( query, queryStrings );
    for( auto && ii : data )
    {
        auto jj = ii.begin();
        auto kk = queries.begin();
        QStringList currRow;
        for( ; jj != ii.end() && kk != queries.end(); ++jj, ++kk )
        {
            if ( (*kk).second )
                (*jj) = NQtUtils::getFile( relToDir, *jj );
            currRow.push_back( *jj );
        }
        if ( clear )
            fData << currRow;
        else
        {
            int idx = -1;
            for( int ii = 0; ii < fData.count(); ++ii )
            {
                if ( fData[ ii ][ 0 ] == currRow[ 0 ] )
                {
                    idx = ii;
                    break;
                }
            }
            if ( idx == -1 )
                fData << currRow;
            else
                fData[ idx ] = currRow;
        }
    }
    endResetModel();
#else
    (void)clear;
    (void)msg;
    (void)relToDir;
    (void)queries;
    (void)query;
#endif
    return true;
}

CKeyValuePairModel::CKeyValuePairModel( const QString & keyName, const QString & valueName, QObject * parent ) :
    CStringTupleModel( QStringList() << keyName << valueName, parent )
{
}

CKeyValuePairModel::CKeyValuePairModel( QObject * parent ) :
    CKeyValuePairModel( "Key", "Value", parent )
{
}

void CKeyValuePairModel::setValues( const QList< QPair< QString, QString > > & values )
{
    QList< QStringList > newValues;
    for( auto && ii : values )
        newValues << ( QStringList() << ii.first << ii.second );
    CStringTupleModel::setValues( newValues );
}


void CKeyValuePairModel::addRow( const QString & key, const QString & value )
{
    CStringTupleModel::addRow( QStringList() << key << value );
}

void CKeyValuePairModel::addRow( const QString & string )
{
    int pos = string.indexOf( "=" );
    QString key = string;
    QString value;
    if ( pos >= 0 )
    {
        key = string.left( pos );
        value = string.mid( pos + 1 );
    }
    addRow( key, value );
}

void CKeyValuePairModel::setRow( const QString & key, const QString & value )
{
    CStringTupleModel::setRow( key, QStringList() << value );
}

QString CKeyValuePairModel::getPair( int pos ) const
{
    if ( pos < 0 || pos >= rowCount() )
        return QString();
    QString retVal = fData[ pos ][ 0 ];
    if ( !fData[ pos ][ 1 ].isEmpty() )
        retVal += "=" + fData[ pos ][ 1 ];
    return retVal;
}

QPair< QString, QString > CKeyValuePairModel::at( int ii ) const
{
    return qMakePair( fData.at( ii )[ 0 ], fData.at( ii )[ 1 ] );
}

QList< QPair< QString, QString > > CKeyValuePairModel::data() const
{
    QList< QPair< QString, QString > > retVal;
    auto tmp = CStringTupleModel::data();
    for ( auto && curr : tmp )
        retVal << qMakePair( curr[ 0 ], curr[ 1 ] );
    return retVal;
}

