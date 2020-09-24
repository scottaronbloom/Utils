#undef QT_NO_DEBUG_OUTPUT
#include "FlowWidget.h"

#include <unordered_map>
#include <QAbstractButton>
#include <QStyleOptionToolBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QApplication>
#include <QPainter>
#include <QTreeWidget>
#include <QHeaderView>
#include <QHelpEvent>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QItemDelegate>
#include <QTextOption>
#include <QTextLayout>
#include <QSizeF>
#include <QToolTip>
#include <QHelpEvent>

using TFlowWidgetItems = std::vector< std::unique_ptr< CFlowWidgetItem > >;
using TDataMap = std::map< int, QVariant >;

// from QItemDelegatePrivate::textLayoutBounds
static QRect textLayoutBounds( const QRect& xRect )
{
    auto lRetVal = xRect;
    lRetVal.setWidth( INT_MAX / 256 );
    return lRetVal;
}

static QSizeF doTextLayout( QTextLayout& lTextLayout, int lLineWidth )
{
    qreal height = 0;
    qreal widthUsed = 0;
    lTextLayout.beginLayout();
    while ( true )
    {
        QTextLine line = lTextLayout.createLine();
        if ( !line.isValid() )
            break;
        line.setLineWidth( lLineWidth );
        line.setPosition( QPointF( 0, height ) );
        height += line.height();
        widthUsed = qMax( widthUsed, line.naturalTextWidth() );
    }
    lTextLayout.endLayout();
    return QSizeF( widthUsed, height );
}

static QRect calcDisplayRect( const QFont & lFont, QString lText, const QRect & xRect )
{
    QTextOption lTextOption;
    lTextOption.setWrapMode( QTextOption::WordWrap );

    QTextLayout lTextLayout;
    lTextLayout.setTextOption( lTextOption );
    lTextLayout.setFont( lFont );
    lTextLayout.setText( lText.replace( '\n', QChar::LineSeparator ) );
    auto lFPSize = doTextLayout( lTextLayout, textLayoutBounds( xRect ).width() );
    const QSize lSize = QSize( std::ceil( lFPSize.width() ), std::ceil( lFPSize.height() ) );
    const int lTextMargin = QApplication::style()->pixelMetric( QStyle::PM_FocusFrameHMargin, nullptr ) + 1;
    return QRect( 0, 0, lSize.width() + 2 * lTextMargin, lSize.height() );
}

class CFlowWidgetHeader : public QAbstractButton
{
    Q_OBJECT
public:
    friend class CFlowWidgetImpl;
    CFlowWidgetHeader( CFlowWidgetItem* xContainer, CFlowWidget* xParent );
    ~CFlowWidgetHeader();

    CFlowWidget* mFlowWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void mSetText( const QString& text ) 
    { 
        setText( text ); 
        mSetObjectName();
    }
    QString mText() const { return text(); }

    void mSetIcon( const QIcon& icon ) { return setIcon( icon ); }
    QIcon mIcon() const { return icon(); }

    void mSetToolTip( const QString& tooltip ) { return setToolTip( tooltip ); }

    void mAddChild( CFlowWidgetItem* xChild ) { mInsertChild( -1, xChild ); }
    int mInsertChild( int xIndex, CFlowWidgetItem* xChild );
    void mAddChildren( const TFlowWidgetItems & xChildren );

    int mIndexOfChild( const CFlowWidgetItem* xItem ) const;
    CFlowWidgetItem* mTakeChild( CFlowWidgetItem* xItem );
    int mChildCount() const;
    CFlowWidgetItem* mGetChild( int xIndex ) const;

    void mSetVisible( bool xVisible, bool xExpandIfShow );
    bool mIsVisible() const;

    void mSetDisabled( bool xDisabled ) { return mSetEnabled( !xDisabled ); }
    bool mIsDisabled() const { return !mIsEnabled(); }

    void mSetEnabled( bool xEnabled );
    bool mIsEnabled() const { return dEnabled.first && isEnabled(); }

    bool mSelected() const;
    void mSetSelected( bool b ) { mSetExpanded( b ); }

    void mSetExpanded( bool xExpanded );
    bool mIsExpanded() const;

    void mUpdate();

    void mSetObjectName()
    {
        QAbstractButton::setObjectName( QLatin1String( "flowwidgetitem_flowwidgetheader" ) + "_" + mText() );
        dTreeWidget->setObjectName( QLatin1String( "flowwidgetheader_treewidget" ) + "_" + mText() );
    }

    CFlowWidgetItem* mSelectedItem() const
    {
        if ( !dTreeWidget )
            return nullptr;
        auto lSelectedItems = dTreeWidget->selectedItems();
        if ( lSelectedItems.isEmpty() )
            return nullptr;
        return mFindFlowItem( lSelectedItems.front() );
    }

    CFlowWidgetItem* mFindFlowItem( QTreeWidgetItem* xItem ) const;
    void mAddToMap( QTreeWidgetItem* xItem, CFlowWidgetItem* xFlowItem )
    {
        dAllChildItemMap[ xItem ] = xFlowItem;
    }

    virtual void mouseDoubleClickEvent( QMouseEvent* /*xEvent*/ ) override
    {
        emit sigDoubleClicked();
    }

Q_SIGNALS:
    void sigDoubleClicked();
protected:
    bool event( QEvent* xEvent ) override;
    bool eventFilter( QObject* xWatched, QEvent* xEvent ) override;

    void mSetIndex( int newIndex );

    void mAddToLayout( QVBoxLayout* xLayout );
    void mTakeFromLayout( QVBoxLayout* xLayout );

    void initStyleOption( QStyleOptionToolBox* opt ) const;
    void paintEvent( QPaintEvent* ) override;

    int mComputeTextWidth( QStyleOptionToolBox& lOption, const std::pair< bool, int > & xMaxWidth ) const;

    void mCollectDisabledTreeWidgetItems( QTreeWidgetItem* xItem );
    void mCollectDisabledTreeWidgetItems();
    void mSetTreeWidgetItemsEnabled( QTreeWidgetItem* xItem, bool xEnabled );
    void mSetTreeWidgetItemsEnabled( bool xEnabled );

private:
    CFlowWidgetItem* dContainer{ nullptr }; // not owned no delete
    QScrollArea* dScrollArea{ nullptr }; // child of the parent FlowWidget
    QTreeWidget* dTreeWidget{ nullptr }; // child of dScrollArea

    bool dSelected{ false };
    std::pair< bool, std::unordered_map< QTreeWidgetItem *, bool > > dEnabled{ true, {} };
    int dIndexInPage{ -1 };
    std::vector< CFlowWidgetItem* > dTopItems; // owned in the dContainers dChildren
    std::map< QTreeWidgetItem*, CFlowWidgetItem* > dTopItemMap; // only the top level items
    std::map< QTreeWidgetItem*, CFlowWidgetItem* > dAllChildItemMap; // the individual items own the memory
};

QIcon::Mode iconMode( QStyle::State xState ) 
{
    if ( !(xState & QStyle::State_Enabled) ) 
        return QIcon::Disabled;
    if ( xState & QStyle::State_Selected ) 
        return QIcon::Selected;
    return QIcon::Normal;
}

QIcon::State iconState( QStyle::State xState )
{ 
    return xState & QStyle::State_Open ? QIcon::On : QIcon::Off; 
}

class CFlowWidgetItemDelegate : public QItemDelegate
{
public:
    CFlowWidgetItemDelegate( QObject * xParent ) :
        QItemDelegate( xParent )
    {
    }

    QRect mIconSizeHint( const QStyleOptionViewItem& xOption, const QModelIndex& xIndex ) const
    {
        auto lMode = iconMode( xOption.state );
        auto lState = iconState( xOption.state );
        auto lIcon = xIndex.data( Qt::DecorationRole ).value< QIcon >();
        auto lSize = lIcon.actualSize( xOption.decorationSize, lMode, lState );
        return QRect( QPoint( 0, 0 ), lSize );
    }

    QSize sizeHint( const QStyleOptionViewItem& xOption, const QModelIndex& xIndex ) const override
    {
        QVariant lValue = xIndex.data( Qt::SizeHintRole );
        if ( lValue.isValid() )
            return qvariant_cast<QSize>(lValue);

        if ( !xIndex.isValid() )
            return QItemDelegate::sizeHint( xOption, xIndex );

        auto xDesc = xIndex.data( Qt::DisplayRole ).toString();
        auto lTreeWidgetItem = static_cast<QTreeWidgetItem*>(xIndex.internalPointer());
        if ( !lTreeWidgetItem )
            return QItemDelegate::sizeHint( xOption, xIndex );

        auto xIdentityIcon = xIndex.data( Qt::DecorationRole ).value< QIcon >();
        auto xStateIcons = lTreeWidgetItem->data( 0, CFlowWidgetItem::ERoles::eStateIconsRole ).value< QList< QIcon > >();

        bool lEnabled = xOption.state & QStyle::State_Enabled;

        auto xIdentityPixmap = xIdentityIcon.pixmap( xOption.decorationSize, lEnabled ? QIcon::Normal : QIcon::Disabled );
        QList< QPixmap > lPixmaps;
        for ( auto&& ii : xStateIcons )
        {
            if ( ii.isNull() )
                continue;
            auto lPixMap = ii.pixmap( xOption.decorationSize, lEnabled ? QIcon::Normal : QIcon::Disabled );
            if ( lPixMap.isNull() )
                continue;
            lPixmaps << lPixMap;
        }

        auto lIdentityRect = mIconSizeHint( xOption, xIndex );

        auto lDescRect = calcDisplayRect( xIndex, xOption );
        QList< QRect > lStatusRects;
        for ( int ii = 0; ii < lPixmaps.count(); ++ii )
        {
            auto lRect = calcDecorationRect( xOption );
            lStatusRects << lRect;
        }

        doLayout( xOption, lDescRect, lIdentityRect, lStatusRects, true );

        QRect lOverAllRect = lIdentityRect | lDescRect;
        for( auto && ii : lStatusRects )
            lOverAllRect = lOverAllRect | ii;

        return lOverAllRect.size();
    }

    virtual void paint( QPainter * xPainter, const QStyleOptionViewItem & xOption, const QModelIndex & xIndex ) const override
    {
        if ( !xIndex.isValid() )
        {
            QItemDelegate::paint( xPainter, xOption, xIndex );
            return;
        }

        auto lDescription = xIndex.data( Qt::DisplayRole ).toString();
        auto lTreeWidgetItem = static_cast<QTreeWidgetItem*>(xIndex.internalPointer());
        if ( !lTreeWidgetItem )
        {
            QItemDelegate::paint( xPainter, xOption, xIndex );
            return;
        }
        auto lIdentityIcon = xIndex.data( Qt::DecorationRole ).value< QIcon >();
        auto lStateIcons = lTreeWidgetItem->data( 0, CFlowWidgetItem::ERoles::eStateIconsRole ).value< QList< QIcon > >();

        bool lEnabled = xOption.state & QStyle::State_Enabled;
        auto xIdentityPixmap = lIdentityIcon.pixmap( xOption.decorationSize, lEnabled ? QIcon::Normal : QIcon::Disabled );
        QList< QPixmap > lPixmaps;
        for( auto && ii : lStateIcons )
        {
            if ( ii.isNull() )
                continue;
            auto lPixMap = ii.pixmap( xOption.decorationSize, lEnabled ? QIcon::Normal : QIcon::Disabled );
            if ( lPixMap.isNull() )
                continue;
            lPixmaps << lPixMap;
        }

        // order is IdentityIcon Text StatusIcons
        QRect lIdentityRect = xIdentityPixmap.isNull() ? QRect() : calcDecorationRect( xOption );

        QRect lDescRect = calcDisplayRect( xIndex, xOption );
        QList< QRect > lStatusRects;
        for( int ii = 0; ii < lPixmaps.count(); ++ii )
        {
            auto lRect = calcDecorationRect( xOption );
            lStatusRects << lRect;
        }

        doLayout( xOption, lDescRect, lIdentityRect, lStatusRects, false );

        xPainter->save();
        drawBackground( xPainter, xOption, xIndex );
        drawDecoration( xPainter, xOption, lIdentityRect, xIdentityPixmap );
        drawDisplay( xPainter, xOption, lDescRect, lDescription );
        for( int ii = 0; ii < lStatusRects.count(); ++ii )
        {
            drawDecoration( xPainter, xOption, lStatusRects[ ii ], lPixmaps[ ii ] );
        }
        drawFocus( xPainter, xOption, xOption.rect );
        xPainter->restore();
    }

    void doLayout( const QStyleOptionViewItem& xOption, QRect & xTextRect, QRect & xIdentityRect, QList< QRect > & xStatusRects, bool xHint ) const
    {
        const auto lOrigTextWidth = xTextRect.width();

        QRect lCheckRect; // unused
        QItemDelegate::doLayout( xOption, &lCheckRect, &xIdentityRect, &xTextRect, xHint );
        xTextRect.setWidth( lOrigTextWidth );

        if ( !xIdentityRect.isValid() )
        {
            auto lHeight = std::max( xTextRect.height(), xOption.decorationSize.height() );
            for( auto && ii : xStatusRects )
            {
                lHeight = std::max( lHeight, ii.height() );
            }
            if ( lHeight != xTextRect.height() )
                xTextRect.setHeight( lHeight );
        }

        const QWidget* widget = xOption.widget;
        QStyle* style = widget ? widget->style() : QApplication::style();
        const int lMargin = style->pixelMetric( QStyle::PM_FocusFrameHMargin, nullptr, widget ) + 1;
        for( int ii = 0; ii < xStatusRects.count(); ++ii )
        {
            auto lCurr = xStatusRects[ ii ]; // x,y = 0;
            int x1 = 0;
            if ( ii == 0 )
                x1 = xTextRect.right() + 2*lMargin;
            else
                x1 = xStatusRects[ ii - 1 ].right() + 2*lMargin;
            int y1 = xIdentityRect.isValid() ? xIdentityRect.top() : ( xTextRect.isValid() ? xTextRect.top() : 0 );

            lCurr.setRect( x1, y1, lCurr.width(), lCurr.height() );

            xStatusRects[ ii ] = lCurr;
        }
    }

    QRect calcDecorationRect( const QStyleOptionViewItem& xOption ) const
    {
        QRect rect;

        rect.setX( xOption.rect.x() + 2 );
        rect.setWidth( xOption.decorationSize.width() );

        rect.setY( xOption.rect.y() + (xOption.rect.height() - xOption.decorationSize.height() )/2 );
        rect.setHeight( xOption.decorationSize.height() );
        return rect;
    }

    QRect calcDisplayRect( const QModelIndex & xIndex, const QStyleOptionViewItem& xOption ) const
    {
        auto xText = xIndex.data( Qt::DisplayRole );
        if ( !xIndex.isValid() || xText.isNull() )
            return QRect();

        QString lText = xText.toString();
        const QVariant lFontVal = xIndex.data( Qt::FontRole );
        const QFont lFont = qvariant_cast<QFont>(lFontVal).resolve( xOption.font );

        return ::calcDisplayRect( lFont, lText, xOption.rect );
    }

};

class CFlowWidgetItemImpl
{
public:
    CFlowWidgetItemImpl( CFlowWidgetItem* xContainer ) :
        dContainer( xContainer )
    {
    }

    friend class CFlowWidget;
    void deleteLater()
    {
        if ( dHeader )
            dHeader->deleteLater();
        else if ( dTreeWidgetItem )
            delete dTreeWidgetItem;
        delete this;
    }

    void mSetStateID( int xStateID )
    {
        dStateID = xStateID;
    }
    int mStateID() const { return dStateID; }
    void mSetIcon( const QIcon& icon )
    {
        dIcon = icon;

        if ( dHeader )
            dHeader->mSetIcon( icon );
        else if ( dTreeWidgetItem )
            dTreeWidgetItem->setIcon( 0, icon );
    }

    QIcon mIcon() const
    {
        return dIcon;
    }

    void mSetToolTip( const QString& tip )
    {
        dToolTip = tip;
        if ( dHeader )
            dHeader->mSetToolTip( tip );
        else if ( dTreeWidgetItem )
            dTreeWidgetItem->setToolTip( 0, tip );
    }

    QString mToolTip( bool xIncludeStateInfo ) const
    {
        QStringList lRetVal;
        if ( !dToolTip.isEmpty() )
            lRetVal << dToolTip;
        if ( xIncludeStateInfo )
        {
            auto lStates = mData( CFlowWidgetItem::eStateStatusRole ).value< QList< int > >();
            for ( auto&& ii : lStates )
            {
                lRetVal << mFlowWidget()->mGetStateStatus( ii ).first;
            }
        }
        return lRetVal.join( "\n" );
    }

    bool mIsExpanded() const
    {
        if ( dHeader )
            return dHeader->mIsExpanded();
        else if ( dTreeWidgetItem )
            return dTreeWidgetItem->isExpanded();
        return false;
    }

    inline bool operator==( const CFlowWidgetItemImpl& xOther ) const
    {
        return (dHeader == xOther.dHeader) && (dTreeWidgetItem == xOther.dTreeWidgetItem);
    }

    CFlowWidget* mFlowWidget() const
    {
        if ( dHeader )
            return dynamic_cast<CFlowWidget*>(dHeader->mFlowWidget());
        else if ( dParent )
            return dParent->mGetFlowWidget();
        return nullptr;
    }

    CFlowWidgetItem* mSelectedItem() const
    {
        if ( mHeader() )
            return mHeader()->mSelectedItem();
        return nullptr;
    }
    bool mBeenPlaced() const;

    int mIndexOfChild( const CFlowWidgetItem* xChild )
    {
        if ( !xChild )
            return -1;
        if ( dHeader )
        {
            auto lRetVal = dHeader->mIndexOfChild( xChild );
            if ( lRetVal != -1 )
                return lRetVal;
        }
        else if ( dTreeWidgetItem )
        {
            return dTreeWidgetItem->indexOfChild( xChild->dImpl->dTreeWidgetItem );
        }
        return -1;
    }

    CFlowWidgetItem* mTakeChild( CFlowWidgetItem* xChild )
    {
        if ( !xChild )
            return nullptr;

        if ( dHeader )
        {
            return dHeader->mTakeChild( xChild );
        }
        else if ( dTreeWidgetItem )
        {
            dTreeWidgetItem->takeChild( dTreeWidgetItem->indexOfChild( xChild->dImpl->dTreeWidgetItem ) );
            return xChild;
        }
        return nullptr;
    }

    int mCreateTreeWidgetItem( int xIndex )
    {
        if ( !dParent || !dParent->dImpl )
            return -1;

        auto lTreeWidgetItem = new QTreeWidgetItem( QStringList() << mText() );
        lTreeWidgetItem->setIcon( 0, mIcon() );
        dTreeWidgetItem = lTreeWidgetItem;

        int lRetVal = -1;
        if ( dParent->dImpl->dHeader ) // its a top level, thus it becomes a top level to the tree
        {
            lRetVal = dParent->dImpl->dHeader->mInsertChild( xIndex, dContainer );
        }
        else if ( dParent->dImpl->dTreeWidgetItem )
        {
            Q_ASSERT( xIndex >= 0 && (xIndex <= dParent->dImpl->dTreeWidgetItem->childCount()) );

            dParent->dImpl->dTreeWidgetItem->insertChild( xIndex, dTreeWidgetItem );
            if ( dParent->dImpl->mHeader() )
                dParent->dImpl->mHeader()->mAddToMap( dTreeWidgetItem, dContainer );
            dChildItemMap[ dTreeWidgetItem ] = dContainer;
            lRetVal = xIndex;
        }
        for ( auto ii = 0; ii < static_cast<int>(dChildren.size()); ++ii )
        {
            auto && lChild = dChildren[ ii ];
            lChild->dImpl->mCreateTreeWidgetItem( ii );
        }
        return lRetVal;
    }

    int mInsertChild( int xIndex, std::unique_ptr< CFlowWidgetItem > xChild )
    {
        auto lChildPtr = xChild.get();
        if ( xIndex < 0 || (xIndex > static_cast<int>(dChildren.size())) )
            xIndex = static_cast<int>(dChildren.size());
        dChildren.insert( dChildren.begin() + xIndex, std::move( xChild ) );

        Q_ASSERT( !lChildPtr->dImpl->dParent || (lChildPtr->dImpl->dParent == dContainer) );
        if ( !lChildPtr->dImpl->dParent )
            lChildPtr->dImpl->dParent = dContainer;

        return lChildPtr->dImpl->mCreateTreeWidgetItem( xIndex );
    }

    int mInsertChild( int xIndex, CFlowWidgetItem* xChild )
    {
        auto lUniquePtr = std::unique_ptr< CFlowWidgetItem >( xChild );
        return mInsertChild( xIndex, std::move( lUniquePtr ) );
    }

    int mChildCount() const
    {
        return static_cast<int>(dChildren.size());
    }

    CFlowWidgetItem* mGetChild( int xIndex ) const
    {
        if ( xIndex < 0 || (xIndex >= mChildCount()) )
            return nullptr;

        return dChildren[ xIndex ].get();
    }

    bool mSetData( int xRole, const QVariant& xData, bool xSetState = true );

    QVariant mData( int xRole ) const
    {
        auto pos = dData.find( xRole );
        if ( pos == dData.end() )
            return QVariant();
        return (*pos).second;
    }

    void mSetText( const QString& text )
    {
        dText = text;
        if ( dHeader )
            dHeader->mSetText( text );
        else if ( dTreeWidgetItem )
            dTreeWidgetItem->setText( 0, text );
    }

    QString mText() const
    {
        return dText;
    }

    QString mFullText( const QChar& xSeparator ) const
    {
        QString lMyText = dText;

        QString lParentText;
        if ( dParent )
        {
            lParentText = dParent->mFullText( xSeparator ) + xSeparator;
        }
        lParentText += lMyText;
        return lParentText;
    }

    void mSetVisible( bool xVisible, bool xExpandIfShow ) const
    {
        if ( dHeader )
            dHeader->mSetVisible( xVisible, xExpandIfShow );
        else if ( dTreeWidgetItem )
        {
            dTreeWidgetItem->setHidden( !xVisible );
            if ( xVisible && xExpandIfShow )
                dTreeWidgetItem->setExpanded( true );
        }
    }

    bool mIsVisible() const
    {
        if ( dHeader )
            return dHeader->mIsVisible();
        else if ( dTreeWidgetItem )
            return !dTreeWidgetItem->isHidden();
        return false;
    }

    bool mSetStateStatus( QList< int > xStateStatus );

    bool mAddStateStatus( int xStateStatus )
    {
        if ( xStateStatus == CFlowWidget::EStates::eNone )
            return false;
        auto lStates = mData( CFlowWidgetItem::eStateStatusRole ).value< QList< int > >();
        if ( lStates.indexOf( xStateStatus ) == -1 )
        {
            lStates.push_back( xStateStatus );
            return mSetStateStatus( lStates );
        }
        else
            return false;
    }

    bool mRemoveStateStatus( int xStateStatus )
    {
        if ( xStateStatus == CFlowWidget::EStates::eNone )
            return false;

        auto lStates = mData( CFlowWidgetItem::eStateStatusRole ).value< QList< int > >();
        if ( lStates.removeAll( xStateStatus ) )
            return mSetStateStatus( lStates );
        else
            return false;
    }

    QList< int > mStateStatuses() const
    {
        auto lStates = mData( CFlowWidgetItem::eStateStatusRole ).value< QList< int > >();
        if ( lStates.isEmpty() )
        {
            lStates.push_back( CFlowWidget::eNone );
        }
        return lStates;
    }

    static bool mIsStateDisabled( const QList< int > & xStates )
    {
        return xStates.indexOf( CFlowWidget::EStates::eDisabled ) != -1;
    }

    static bool mIsStateDisabled( const QVariant & xStates )
    {
        return mIsStateDisabled( xStates.value< QList< int > >() );
    }

    bool mIsStateDisabled() const
    {
        return mIsStateDisabled( mData( CFlowWidgetItem::eStateStatusRole ) );
    }

    void mSetStateDisabled( bool xDisabled )
    {
        if ( xDisabled )
            mAddStateStatus( CFlowWidget::EStates::eDisabled );
        else
            mRemoveStateStatus( CFlowWidget::EStates::eDisabled );
    }

    void mSetDisabled( bool xDisabled )
    {
        if ( dHeader )
            dHeader->mSetDisabled( xDisabled );
        else if ( dTreeWidgetItem )
        {
            dTreeWidgetItem->setDisabled( xDisabled );
            if ( xDisabled )
                dTreeWidgetItem->setSelected( false );
        }

        if ( xDisabled != mIsStateDisabled() )
        {
            mSetStateDisabled( xDisabled );
        }
    }

    bool mIsDisabled() const
    {
        if ( dHeader )
            return dHeader->mIsDisabled();
        else if ( dTreeWidgetItem )
            return dTreeWidgetItem->isDisabled();
        return false;
    }

    void mSetSelected( bool xSelected )
    {
        if ( dHeader )
        {
            dHeader->mSetSelected( xSelected ); // for top leave headers, selected opens it always
        }
        else if ( dTreeWidgetItem )
        {
            dTreeWidgetItem->setSelected( xSelected );
        }
    }

    bool mSelected() const
    {
        if ( dHeader )
        {
            dHeader->mSelected();
        }
        else if ( dTreeWidgetItem )
        {
            dTreeWidgetItem->isSelected();
        }
        return false;
    }
    
    CFlowWidgetHeader* mHeader() const
    {
        if ( dHeader )
            return dHeader;
        if ( dParent )
            return dParent->dImpl->mHeader();
        return nullptr;
    }

    void mSetExpanded( bool xExpanded )
    {
        if ( dHeader )
            dHeader->mSetExpanded( xExpanded );
        else if ( dTreeWidgetItem )
            dTreeWidgetItem->setExpanded( xExpanded );
    }
    void mRemoveWidgets();
    void mClearWidgets( bool xClearCurrent );

    void mCreateFlowWidget( CFlowWidget * xFlowWidget )
    {
        dHeader = new CFlowWidgetHeader( dContainer, xFlowWidget );        
        mSetIcon( mIcon() );
        mSetText( mText() );
        dHeader->mSetObjectName();
        dHeader->mAddChildren( dChildren );
    }

    void mDump( QJsonObject& xJSON, bool xRecursive ) const
    {
        xJSON[ "StateID" ] = dStateID;
        xJSON[ "Text" ] = dText;
        xJSON[ "ToolTip" ] = dToolTip;
        xJSON[ "HasIcon" ] = !dIcon.isNull();
        xJSON[ "Disabled" ] = mIsDisabled();
        xJSON[ "Visible" ] = mIsVisible();
        QJsonArray lDataArray;
        for( auto && ii : dData )
        {
            QJsonObject lCurr;
            lCurr[ QString::number( ii.first ) ] = ii.second.toString();
            lDataArray.push_back( lCurr );
        }
        if ( lDataArray.size() )
            xJSON[ "Data" ] = lDataArray;

        if ( xRecursive )
        {
            QJsonArray lChildren;
            int childNum = 0;
            for ( auto&& ii : dChildren )
            {
                QJsonObject lCurr;
                ii->mDump( lCurr, true );
                lChildren.push_back( lCurr );
            }
            if ( lChildren.size() )
                xJSON[ "Children" ] = lChildren;
        }
    }

    CFlowWidgetItem* dContainer{ nullptr };
    CFlowWidgetItem* dParent{ nullptr };
    CFlowWidgetHeader* dHeader{ nullptr };
    QTreeWidgetItem* dTreeWidgetItem{ nullptr };

    std::map< QTreeWidgetItem*, CFlowWidgetItem* > dChildItemMap;  // flowItems owned by dChildren

    int dStateID{ -1 };
    QString dText;
    QString dToolTip;
    QIcon dIcon;
    TDataMap dData;
    TFlowWidgetItems dChildren;
};

class CFlowWidgetImpl
{
public:
    inline CFlowWidgetImpl( CFlowWidget* parent )
        : dCurrentTopLevelItem( nullptr ),
        dFlowWidget( parent )
    {
        mInitDefaultMap();
    }

    void mInitDefaultMap()
    {
        Q_INIT_RESOURCE( FlowWidget ); // make sure its initialized
        mRegisterStateStatus( CFlowWidget::EStates::eDisabled, QObject::tr( "Disable" ), QIcon( ":/FlowWidgetResources/Disabled.png" ), true );
        mRegisterStateStatus( CFlowWidget::EStates::eReadyToRun, QObject::tr( "Ready To Run" ), QIcon( ":/FlowWidgetResources/ReadyToRun.png" ), true );
        mRegisterStateStatus( CFlowWidget::EStates::eRunning, QObject::tr( "Running" ), QIcon( ":/FlowWidgetResources/Running.png" ), true );
        mRegisterStateStatus( CFlowWidget::EStates::eRunCompletedWithWarning, QObject::tr( "Run Completed With Warning" ), QIcon( ":/FlowWidgetResources/RunCompletedWithWarning.png" ), true );
        mRegisterStateStatus( CFlowWidget::EStates::eRunCompletedWithError, QObject::tr( "Run Completed With Error" ), QIcon( ":/FlowWidgetResources/RunCompletedWithError.png" ), true );
        mRegisterStateStatus( CFlowWidget::EStates::eRunCompletedWithInfo, QObject::tr( "Run Completed With Info" ), QIcon( ":/FlowWidgetResources/RunCompletedWithInfo.png" ), true );
        mRegisterStateStatus( CFlowWidget::EStates::eRunPassed, QObject::tr( "Run Passed" ), QIcon( ":/FlowWidgetResources/RunPassed.png" ), true );
    }

    void mClear()
    {
        dTopLevelItems.clear();
        dCurrentTopLevelItem = nullptr;
        mRelayout();
    }

    size_t mOpenCount() const
    {
        size_t retVal = 0;
        for ( auto&& ii : dTopLevelItems )
        {
            if ( ii->mIsVisible() )
                retVal++;
        }
        return retVal;
    }
    int mAddTopLevelItem( CFlowWidgetItem* xItem ) { return mInsertTopLevelItem( -1, xItem ); }
    int mAddTopLevelItem( std::unique_ptr< CFlowWidgetItem > xItem ) { return mInsertTopLevelItem( -1, std::move( xItem ) ); }

    int mAddItem( CFlowWidgetItem* xItem, CFlowWidgetItem* xParent ) { return mInsertItem( -1, xItem, xParent ); }
    int mAddItem( std::unique_ptr< CFlowWidgetItem > xItem, CFlowWidgetItem* xParent ) { return mInsertItem( -1, std::move( xItem ), xParent ); }

    int mInsertTopLevelItem( int xIndex, CFlowWidgetItem* xItem ); // the parent is "this"
    int mInsertTopLevelItem( int xIndex, std::unique_ptr< CFlowWidgetItem > xItem ); // the parent is "this"

    int mInsertItem( int xIndex, CFlowWidgetItem* xItem, CFlowWidgetItem* xParent );
    int mInsertItem( int xIndex, std::unique_ptr< CFlowWidgetItem > xItem, CFlowWidgetItem* xParent );

    void mSetCurrentTopLevelItem( int xIndex, bool xMakeVisible = true )
    {
        auto lTopLevelItem = mTopLevelItem( xIndex );
        mSetCurrentTopLevelItem( lTopLevelItem, xMakeVisible );
    }

    void mSetCurrentTopLevelItem( CFlowWidgetItem* xItem, bool xMakeVisible = true )
    {
        if ( dCurrentTopLevelItem )
            dCurrentTopLevelItem->mSetSelected( false );

        if ( xItem )
        {
            xItem->mSetSelected( true );
            if ( xMakeVisible )
                xItem->mSetVisible( true, true );
        }
        if ( xItem != dCurrentTopLevelItem )
        {
            dCurrentTopLevelItem = xItem;
            dFlowWidget->dImpl->mUpdateTabs();
            emit dFlowWidget->sigFlowWidgetItemSelected( xItem, true );
        }
    }

    int mIndexOfTopLevelItem( CFlowWidgetItem* xItem )
    {
        for ( size_t ii = 0; ii < dTopLevelItems.size(); ++ii )
        {
            if ( dTopLevelItems[ ii ].get() == xItem )
                return static_cast<int>(ii);
        }
        return -1;
    }

    CFlowWidgetItem* mGetTopLevelItem( int xIndex ) const
    {
        if ( (xIndex < 0) || (xIndex >= dTopLevelItems.size()) )
            return nullptr;
        return dTopLevelItems[ xIndex ].get();
    }

    CFlowWidgetItem* mCurrentTopLevelItem() const
    {
        return dCurrentTopLevelItem;
    }

    void mSetCurrentItemExpanded( bool xExpanded )
    {
        if ( dCurrentTopLevelItem )
        {
            dCurrentTopLevelItem->mSetExpanded( xExpanded );
        }
    }

    int mTopLevelItemCount() const
    {
        return static_cast<int>(dTopLevelItems.size());
    }

    int mFindReplacementHeader( CFlowWidgetItem* xItem )
    {
        if ( !xItem )
            return -1;
        if ( !xItem->dImpl->dHeader )
            return -1;


        auto lCurrentIndex = mIndexOfTopLevelItem( xItem );
        return mFindReplacementHeader( lCurrentIndex, false );
    }

    int mFindReplacementHeader( int xCurrentIndex, bool xPreIndexed )
    {
        int lRetVal = xCurrentIndex;
        int lCurIndexUp = xCurrentIndex;
        int lCurIndexDown = lCurIndexUp;
        const int lCount = mTopLevelItemCount();
        while ( lCurIndexUp > 0 || lCurIndexDown < lCount - 1 )
        {
            if ( lCurIndexDown < lCount - 1 )
            {
                auto lDownItem = mGetTopLevelItem( xPreIndexed ? lCurIndexDown : ++lCurIndexDown );
                if ( lDownItem->mIsEnabled() && lDownItem->mIsVisible() )
                {
                    lRetVal = lCurIndexDown;
                    break;
                }
                if ( xPreIndexed )
                    lCurIndexDown++;
            }
            if ( lCurIndexUp > 0 )
            {
                auto lUpItem = mGetTopLevelItem( xPreIndexed ? lCurIndexUp : (--lCurIndexUp) );
                if ( lUpItem->mIsEnabled() && lUpItem->mIsVisible() )
                {
                    lRetVal = lCurIndexUp;
                    break;
                }
                if ( xPreIndexed )
                    lCurIndexUp--;
            }
        }
        return lRetVal;
    }

    const CFlowWidgetItem* mTopLevelItem( int xIndex ) const;
    CFlowWidgetItem* mTopLevelItem( int xIndex );

    CFlowWidgetItem* mTakeItem( CFlowWidgetItem* xItem );
    CFlowWidgetItem* mRemoveFromTopLevelItems( CFlowWidgetItem* xItem );
    void mRemoveTopLevelItem( int xIndex );

    void mToggleTopLevelItem( int index ); // if already open, close it, if closed open it
    void mDump( QJsonObject& xJSON ) const
    {
        if ( dCurrentTopLevelItem )
        {
            QJsonObject lCurrentObject;
            dCurrentTopLevelItem->mDump( lCurrentObject, false );
            xJSON[ "Current" ] = lCurrentObject;
        }
        else
        {
            xJSON[ "Current" ] = "nullptr";
        }

        QJsonArray lChildren;
        for ( auto && ii : dTopLevelItems )
        {
            QJsonObject lCurr;
            ii->mDump( lCurr, true );
            lChildren.append( lCurr );
        }
        if ( lChildren.size() )
            xJSON[ "Children" ] = lChildren;
    }

    void mUpdateTabs();
    void mRelayout();

    QList< std::tuple< int, QString, QIcon > > mGetRegisteredStatuses() const
    {
        QList< std::tuple< int, QString, QIcon > > lRetVal;
        for( auto && ii : dStateStatusMap )
        {
            lRetVal.push_back( std::make_tuple( ii.first, ii.second.first, ii.second.second ) );
        }
        return lRetVal;
    }

    void mRegisterStateStatus( int xState, const QString& xDescription, const QIcon& xIcon, bool xCheckForNullIcon )
    {
        Q_ASSERT( !xCheckForNullIcon || ( !xIcon.isNull() && !xIcon.pixmap( 16, 16 ).isNull() ) );
        dStateStatusMap[ xState ] = std::make_pair( xDescription, xIcon );
    }

    int mGetNextStatusID() const
    {
        auto lRetVal = CFlowWidget::EStates::eLastState + 1;
        for( auto && ii : dStateStatusMap )
        {
            lRetVal = std::max( lRetVal, ii.first );
        }
        return lRetVal;
    }

    std::pair< QString, QIcon > mGetStateStatus( int xState ) const
    {
        auto pos = dStateStatusMap.find( xState );
        if ( pos == dStateStatusMap.end() )
            return std::make_pair( QString(), QIcon() );
        return (*pos).second;
    }

    bool mIsRegistered( int xState )
    {
        return dStateStatusMap.find( xState ) != dStateStatusMap.end();
    }

    void mSetElideText( bool xElideText )
    {
        dElideText = xElideText;
        dFlowWidget->repaint();
    }
    bool mElideText() const
    {
        return dElideText;
    }

    QVBoxLayout* fTopLayout{ nullptr };
    CFlowWidget* dFlowWidget{ nullptr };

    CFlowWidgetItem * dCurrentTopLevelItem{ nullptr };

    TFlowWidgetItems dTopLevelItems;

    std::unordered_map< int, std::pair< QString, QIcon > > dStateStatusMap;
    bool dElideText{ false };
};

CFlowWidgetItem::CFlowWidgetItem( int xStateID, const QString& xFlowName, const QIcon& xDescIcon )
{
    dImpl = std::make_unique< CFlowWidgetItemImpl >( this );
    dImpl->mSetStateID( xStateID );
    dImpl->mSetText( xFlowName );
    dImpl->mSetIcon( xDescIcon );
}

CFlowWidgetItem::CFlowWidgetItem() :
    CFlowWidgetItem( -1, QString(), QIcon() )
{
}

CFlowWidgetItem::CFlowWidgetItem( int xStateID, const QString& xFlowName, CFlowWidget* xParent ) :
    CFlowWidgetItem( xStateID, xFlowName, QIcon(), xParent )
{
}

CFlowWidgetItem::CFlowWidgetItem( int xStateID, const QString& xFlowName, CFlowWidgetItem* xParent ) :
    CFlowWidgetItem( xStateID, xFlowName, QIcon(), xParent )
{
}

CFlowWidgetItem::CFlowWidgetItem( int xStateID, const QIcon& xDescIcon, CFlowWidget* xParent ) :
    CFlowWidgetItem( xStateID, QString(), xDescIcon, xParent )
{
}

CFlowWidgetItem::CFlowWidgetItem( int xStateID, const QIcon& xDescIcon, CFlowWidgetItem* xParent ) :
    CFlowWidgetItem( xStateID, QString(), xDescIcon, xParent )
{
}

CFlowWidgetItem::CFlowWidgetItem( int xStateID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidget* xParent ) :
    CFlowWidgetItem( xStateID, xFlowName, xDescIcon )
{
    if ( xParent )
        xParent->dImpl->mAddTopLevelItem( this );
}

CFlowWidgetItem::CFlowWidgetItem( int xStateID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidgetItem* xParent ) :
    CFlowWidgetItem( xStateID, xFlowName, xDescIcon )
{
    dImpl->dParent = xParent;
    if ( xParent )
        xParent->mAddChild( this );
}

void CFlowWidgetItem::deleteLater()
{
    dImpl->deleteLater();
    delete this;
}

CFlowWidgetItem* CFlowWidgetItem::mParentItem() const
{
    return dImpl->dParent;
}

CFlowWidget* CFlowWidgetItem::mGetFlowWidget() const
{
    return dImpl->mFlowWidget();
}

void CFlowWidgetItem::mAddChild( CFlowWidgetItem* xChild )
{
    mInsertChild( -1, xChild );
}

int CFlowWidgetItem::mInsertChild( CFlowWidgetItem* xPeer, CFlowWidgetItem* xItem, bool xBefore )
{
    if ( !xPeer || !xItem )
        return -1;

    auto lIndex = xPeer->mIndexInParent();
    if ( !xBefore )
        lIndex++;
    return mInsertChild( lIndex, xItem );
}

int CFlowWidgetItem::mInsertChild( int xIndex, CFlowWidgetItem* xChild )
{
    return dImpl->mInsertChild( xIndex, xChild );
}

int CFlowWidgetItem::mIndexOfChild( const CFlowWidgetItem* xChild )
{
    return dImpl->mIndexOfChild( xChild );
}

int CFlowWidgetItem::mChildCount() const
{
    return dImpl->mChildCount();
}

CFlowWidgetItem* CFlowWidgetItem::mGetChild( int xIndex ) const
{
    return dImpl->mGetChild( xIndex );
}

bool CFlowWidgetItem::mSetData( int xRole, const QVariant& xVariant )
{
    return dImpl->mSetData( xRole, xVariant );
}

QVariant CFlowWidgetItem::mData( int xRole ) const
{
    return dImpl->mData( xRole );
}

void CFlowWidgetItem::mSetText( const QString& xText )
{
    return dImpl->mSetText( xText );
}

QString CFlowWidgetItem::mText() const
{
    return dImpl->mText();
}

QString CFlowWidgetItem::mFullText( const QChar& xSeparator ) const
{
    return dImpl->mFullText( xSeparator );
}

void CFlowWidgetItem::mSetToolTip( const QString& xToolTip )
{
    return dImpl->mSetToolTip( xToolTip );
}

QString CFlowWidgetItem::mToolTip() const
{
    return dImpl->mToolTip( false );
}

void CFlowWidgetItem::mSetIcon( const QIcon& xIcon )
{
    return dImpl->mSetIcon( xIcon );
}

QIcon CFlowWidgetItem::mIcon() const
{
    return dImpl->mIcon();
}

void CFlowWidgetItem::mSetStateID( int xStateID )
{
    return dImpl->mSetStateID( xStateID );
}

int CFlowWidgetItem::mStateID() const
{
    return dImpl->mStateID();
}

void CFlowWidgetItem::mSetHidden( bool xHidden )
{
    return mSetVisible( !xHidden, false );
}

bool CFlowWidgetItem::mIsHidden() const
{
    return !mIsVisible();
}

void CFlowWidgetItem::mSetVisible( bool xVisible, bool xExpandIfShow )
{
    return dImpl->mSetVisible( xVisible, xExpandIfShow );
}

bool CFlowWidgetItem::mIsVisible() const
{
    return dImpl->mIsVisible();
}

void CFlowWidgetItem::mSetDisabled( bool xDisabled )
{
    return dImpl->mSetDisabled( xDisabled );
}

bool CFlowWidgetItem::mIsDisabled() const
{
    return dImpl->mIsDisabled();
}

void CFlowWidgetItem::mSetEnabled( bool xEnabled )
{
    return mSetDisabled( !xEnabled );
}

bool CFlowWidgetItem::mIsEnabled() const
{
    return !mIsDisabled();
}

void CFlowWidgetItem::mSetSelected( bool xSelected )
{
    dImpl->mSetSelected( xSelected );
}

bool CFlowWidgetItem::mSelected() const
{
    return dImpl->mSelected();
}

bool CFlowWidgetItemImpl::mBeenPlaced() const
{
    return (dHeader != nullptr) || (dTreeWidgetItem != nullptr);
}

void CFlowWidgetItemImpl::mClearWidgets( bool xClearCurrent )
{
    if ( xClearCurrent )
    {
        dHeader = nullptr;
        dTreeWidgetItem = nullptr;
    }

    dChildItemMap.clear();
    for ( int ii = 0; ii < mChildCount(); ++ii )
    {
        auto lCurr = mGetChild( ii );
        if ( !lCurr )
            continue;
        lCurr->dImpl->mClearWidgets( true ); // clears all tree widget items
    }
}

bool CFlowWidgetItemImpl::mSetData( int xRole, const QVariant& xData, bool xSetState /*= true */ )
{
    // header data comes from here
    auto lData = mData( xRole );
    if ( lData == xData )
        return false;

    if ( dTreeWidgetItem )
        dTreeWidgetItem->setData( 0, xRole, xData );

    dData[ xRole ] = xData;
    if ( !dTreeWidgetItem )
        emit mFlowWidget()->sigFlowWidgetItemChanged( dContainer );

    if ( xSetState && (xRole == CFlowWidgetItem::eStateStatusRole) )
    {
        auto lStates = xData.value< QList< int > >();
        mSetDisabled( mIsStateDisabled( xData ) );
    }

    if ( xRole == CFlowWidgetItem::eStateStatusRole )
    {
        QList< QIcon > lIcons;
        auto lStates = xData.value< QList< int > >();
        for ( auto ii : lStates )
        {
            // what to do about the tool tips???
            auto lIcon = mFlowWidget()->dImpl->mGetStateStatus( ii ).second;
            if ( !lIcon.isNull() )
                lIcons.push_back( lIcon );
        }
        mSetData( CFlowWidgetItem::eStateIconsRole, QVariant::fromValue< QList< QIcon > >( lIcons ) );
    }

    if ( !dTreeWidgetItem )
        dHeader->repaint();
    return true;
}

bool CFlowWidgetItemImpl::mSetStateStatus( QList< int > xStateStatuses )
{
    xStateStatuses.removeAll( CFlowWidget::EStates::eNone );
    auto lCurrData = mStateStatuses();
    for ( auto&& ii : xStateStatuses )
    {
        if ( !mFlowWidget()->dImpl->mIsRegistered( ii ) )
            return false;
    }
    if ( xStateStatuses != lCurrData )
        return mSetData( CFlowWidgetItem::eStateStatusRole, QVariant::fromValue< QList< int > >( xStateStatuses ) );
    else
        return false;
}

void CFlowWidgetItemImpl::mRemoveWidgets()
{
    mClearWidgets( false );
    if ( dHeader )
    {
        delete dHeader;
        dHeader = nullptr;
    }
    else if ( dTreeWidgetItem )
    {
        delete dTreeWidgetItem;
        dTreeWidgetItem = nullptr;
    }
}

bool CFlowWidgetItem::mIsTopLevelItem() const
{
    return dImpl->mBeenPlaced() && (dImpl->dHeader != nullptr) && (dImpl->dTreeWidgetItem == nullptr);
}

bool CFlowWidgetItem::mSetStateStatus( int xStateStatus )
{
    return mSetStateStatus( QList< int >( { xStateStatus } ) );
}

bool CFlowWidgetItem::mSetStateStatus( const QList< int > & xStateStatus )
{
    return dImpl->mSetStateStatus( xStateStatus );
}

bool CFlowWidgetItem::mAddStateStatus( int xStateStatus )
{
    return dImpl->mAddStateStatus( xStateStatus );
}

bool CFlowWidgetItem::mRemoveStateStatus( int xStateStatus )
{
    return dImpl->mRemoveStateStatus( xStateStatus );
}

QList< int > CFlowWidgetItem::mStateStatuses() const
{
    return dImpl->mStateStatuses();
}

void CFlowWidgetItem::mSetExpanded( bool xExpanded )
{
    return dImpl->mSetExpanded( xExpanded );
}

bool CFlowWidgetItem::mIsExpanded() const
{
    return dImpl->mIsExpanded();
}

int CFlowWidgetItem::mIndexInParent() const
{
    if ( !mParentItem() )
    {
        auto lFlowWidget = dImpl->mFlowWidget();
        if ( !lFlowWidget )
            return -1;

        return lFlowWidget->mIndexOfTopLevelItem( this );
    }
    else
        return mParentItem()->mIndexOfChild( this );
}
/*========================================================================================================*/

CFlowWidgetItem* CFlowWidgetImpl::mTopLevelItem( int xIndex )
{
    if ( xIndex >= 0 && xIndex < static_cast<int>(dTopLevelItems.size()) )
        return dTopLevelItems[ xIndex ].get();
    return nullptr;
}

const CFlowWidgetItem* CFlowWidgetImpl::mTopLevelItem( int xIndex ) const
{
    if ( xIndex >= 0 && xIndex < static_cast<int>(dTopLevelItems.size()) )
        return dTopLevelItems[ xIndex ].get();
    return nullptr;
}

void CFlowWidgetImpl::mUpdateTabs()
{
    CFlowWidgetHeader* lLastButton = dCurrentTopLevelItem ? dCurrentTopLevelItem->dImpl->dHeader : nullptr;
    bool lAfter = false;
    int lIndex = 0;
    for ( const auto& lCurrItem : dTopLevelItems )
    {
        auto lHeader = lCurrItem->dImpl->dHeader;
        // update indexes, since the updates are delayed, the indexes will be correct
        // when we actually paint.
        lHeader->mSetIndex( lIndex );
        lHeader->mUpdate();
        lAfter = lHeader == lLastButton;
        ++lIndex;
    }
}

CFlowWidgetHeader::CFlowWidgetHeader( CFlowWidgetItem* xContainer, CFlowWidget* xParent ) :
    QAbstractButton( xParent ),
    dContainer( xContainer )
{
    setBackgroundRole( QPalette::Window );
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
    setFocusPolicy( Qt::NoFocus );

    dTreeWidget = new QTreeWidget;
    dTreeWidget->setItemDelegate( new CFlowWidgetItemDelegate( this ) );
    dTreeWidget->setObjectName( "flowwidgetheader_treewidget" );
    dTreeWidget->setExpandsOnDoubleClick( false );
    dTreeWidget->header()->setHidden( true );
    connect( dTreeWidget, &QTreeWidget::itemChanged,
             [this, xParent]( QTreeWidgetItem * xItem, int /*xColumn*/ )
    {
        auto pos = dAllChildItemMap.find( xItem );
        if ( pos != dAllChildItemMap.end() )
            emit xParent->sigFlowWidgetItemChanged( (*pos).second );
    }
    );
    connect( dTreeWidget, &QTreeWidget::itemDoubleClicked,
             [this]()
    {
        auto lSelected = mSelectedItem();
        mFlowWidget()->sigFlowWidgetItemDoubleClicked( lSelected );
    }
    );
    connect( dTreeWidget, &QTreeWidget::itemSelectionChanged,
             [this]()
    {
        auto lSelected = mSelectedItem();
        mFlowWidget()->sigFlowWidgetItemSelected( lSelected, true );
    }
    );

    dScrollArea = new QScrollArea( xParent );
    dScrollArea->setObjectName( "flowwidgetheader_scrollarea" );
    dScrollArea->setWidget( dTreeWidget );
    dScrollArea->setWidgetResizable( true );
    dScrollArea->hide();
    dScrollArea->setFrameStyle( QFrame::NoFrame );
    dTreeWidget->installEventFilter( this );
    dTreeWidget->viewport()->installEventFilter( this );
}

CFlowWidgetHeader::~CFlowWidgetHeader()
{
    delete dScrollArea;
}

bool CFlowWidgetHeader::eventFilter( QObject* xWatched, QEvent* xEvent )
{
    if ( ( xWatched == dTreeWidget ) || ( xWatched == dTreeWidget->viewport() ) )
    {
        if ( xEvent->type() == QEvent::ToolTip )
        {
            auto lHelpEvent = dynamic_cast< QHelpEvent * >( xEvent );
            auto pos = lHelpEvent->pos();
            if ( xWatched == dTreeWidget->viewport() )
            {
                pos = dTreeWidget->mapFromGlobal( lHelpEvent->globalPos() );
            }
            auto xItem = dTreeWidget->itemAt( lHelpEvent->pos() );
            auto xFlowItem = mFindFlowItem( xItem );

            emit mFlowWidget()->sigFlowWidgetItemHovered( xFlowItem );

            if ( xFlowItem )
            {
                auto lText = xFlowItem->dImpl->mToolTip( true );
                if ( !lText.isEmpty() )
                {
                    QToolTip::showText( static_cast<QHelpEvent*>(xEvent)->globalPos(), lText, this, QRect(), toolTipDuration() );
                    return true;
                }
            }
        }
    }
    return QAbstractButton::eventFilter( xWatched, xEvent );
}

bool CFlowWidgetHeader::event( QEvent* xEvent )
{
    if ( xEvent->type() == QEvent::ToolTip )
    {
        emit mFlowWidget()->sigFlowWidgetItemHovered( dContainer );
        auto lText = dContainer->dImpl->mToolTip( true );
        if ( lText.isEmpty() )
            xEvent->ignore();
        else
            QToolTip::showText( static_cast<QHelpEvent*>(xEvent)->globalPos(), lText, this, QRect(), toolTipDuration() );
    }
    return QAbstractButton::event( xEvent );
}

CFlowWidgetItem* CFlowWidgetHeader::mFindFlowItem( QTreeWidgetItem* xItem ) const
{
    if ( !xItem )
        return nullptr;
    auto pos = dTopItemMap.find( xItem );
    if ( pos != dTopItemMap.end() )
        return (*pos).second;

    auto pos2 = dAllChildItemMap.find( xItem );
    if ( pos2 != dAllChildItemMap.end() )
        return (*pos2).second;

    return nullptr;
}

CFlowWidget* CFlowWidgetHeader::mFlowWidget() const
{
    return dynamic_cast<CFlowWidget*>(parentWidget());
}

int CFlowWidgetHeader::mIndexOfChild( const CFlowWidgetItem* xItem ) const
{
    if ( !xItem )
        return -1;

    for ( auto ii = 0; ii < dTreeWidget->topLevelItemCount(); ++ii )
    {
        auto lCurrItem = dTreeWidget->topLevelItem( ii );
        if ( !lCurrItem )
            continue;
        if ( lCurrItem == xItem->dImpl->dTreeWidgetItem )
            return ii;
    }
    return -1;
}

int CFlowWidgetHeader::mInsertChild( int xIndex, CFlowWidgetItem* xItem )
{
    Q_ASSERT( xItem->dImpl->dTreeWidgetItem );
    if ( xIndex < 0 || xIndex > dTreeWidget->topLevelItemCount() )
        xIndex = dTreeWidget->topLevelItemCount();
    dTreeWidget->insertTopLevelItem( xIndex, xItem->dImpl->dTreeWidgetItem );

    dTopItems.insert( dTopItems.begin() + xIndex, xItem );
    dTopItemMap[ xItem->dImpl->dTreeWidgetItem ] = xItem;
    return xIndex;
}

void CFlowWidgetHeader::mAddChildren( const TFlowWidgetItems& xChildren )
{
    for ( auto ii = 0; ii < static_cast<int>(xChildren.size()); ++ii )
    {
        auto && lChild = xChildren[ ii ];
        lChild->dImpl->dParent = dContainer;
        lChild->dImpl->mCreateTreeWidgetItem( ii );
    }
}

QString CFlowWidgetItem::mDump( bool xRecursive, bool xCompacted ) const
{
    QJsonObject lObject;
    mDump( lObject, xRecursive );
    auto lRetVal = QJsonDocument( lObject ).toJson( xCompacted ? QJsonDocument::Compact : QJsonDocument::Indented );
    return lRetVal;
}

void CFlowWidgetItem::mDump( QJsonObject& xJSON, bool xRecursive ) const
{
    dImpl->mDump( xJSON, xRecursive );
}

CFlowWidgetItem* CFlowWidgetItem::mTakeChild( CFlowWidgetItem* xItem )
{
    return dImpl->mTakeChild( xItem );
}

CFlowWidgetItem* CFlowWidgetHeader::mTakeChild( CFlowWidgetItem* xItem )
{
    if ( !dTreeWidget )
        return nullptr;
    auto lIndex = dTreeWidget->indexOfTopLevelItem( xItem->dImpl->dTreeWidgetItem );
    if ( lIndex != -1 )
    {
        dTreeWidget->takeTopLevelItem( lIndex );
        auto pos = dTopItemMap.find( xItem->dImpl->dTreeWidgetItem );
        if ( pos != dTopItemMap.end() )
            dTopItemMap.erase( pos );
        auto lItem = std::move( dTopItems[ lIndex ] );

        return xItem;
    }
    else
    {
        auto lParent = xItem->mParentItem();
        if ( !lParent )
            return nullptr;
        return lParent->dImpl->mTakeChild( xItem );
    }
}

int CFlowWidgetHeader::mChildCount() const
{
    if ( dTreeWidget )
        return dTreeWidget->topLevelItemCount();
    return 0;
}

CFlowWidgetItem* CFlowWidgetHeader::mGetChild( int xIndex ) const
{
    if ( (xIndex < 0) || (xIndex > static_cast<int>(dTopItems.size())) )
        return nullptr;
    return dTopItems[ xIndex ];
}

void CFlowWidgetHeader::initStyleOption( QStyleOptionToolBox* xOption ) const
{
    if ( !xOption )
        return;
    xOption->initFrom( this );
    if ( !mIsEnabled() )
    {
        xOption->state &= ~QStyle::State_Enabled;
        xOption->palette.setCurrentColorGroup( QPalette::Disabled );
    }
    if ( dSelected )
        xOption->state |= QStyle::State_Selected;

    if ( isDown() )
        xOption->state |= QStyle::State_Sunken;
    xOption->text = text();
    xOption->icon = icon();

    auto lFlowWidget = mFlowWidget();
    const auto lTopLevelCount = lFlowWidget->mTopLevelItemCount();
    const auto lCurrIndex = lFlowWidget->mIndexOfTopLevelItem( lFlowWidget->mCurrentTopLevelItem() );
    if ( lTopLevelCount == 1 )
    {
        xOption->position = QStyleOptionToolBox::OnlyOneTab;
    }
    else if ( dIndexInPage == 0 )
    {
        xOption->position = QStyleOptionToolBox::Beginning;
    }
    else if ( dIndexInPage == lTopLevelCount - 1 )
    {
        xOption->position = QStyleOptionToolBox::End;
    }
    else
    {
        xOption->position = QStyleOptionToolBox::Middle;
    }
    if ( lCurrIndex == dIndexInPage - 1 )
    {
        xOption->selectedPosition = QStyleOptionToolBox::PreviousIsSelected;
    }
    else if ( lCurrIndex == dIndexInPage + 1 )
    {
        xOption->selectedPosition = QStyleOptionToolBox::NextIsSelected;
    }
    else
    {
        xOption->selectedPosition = QStyleOptionToolBox::NotAdjacent;
    }
}

void CFlowWidgetHeader::mSetVisible( bool xVisible, bool xExpandIfShow )
{
    if ( isVisible() != xVisible )
    {
        if ( dScrollArea )
        {
            if ( xVisible && !dScrollArea->isVisible() )
                dScrollArea->setVisible( xVisible && xExpandIfShow );
            else if ( !xVisible )
                dScrollArea->setVisible( false );
        }
        setVisible( xVisible );

        if ( !xVisible && mFlowWidget()->mCurrentTopLevelItem() == dContainer )
        {
            auto lNewIndex = mFlowWidget()->dImpl->mFindReplacementHeader( dContainer );
            mFlowWidget()->dImpl->mSetCurrentTopLevelItem( lNewIndex );
        }

        if ( xVisible && mFlowWidget()->mCurrentTopLevelItem() != dContainer )
        {
            mFlowWidget()->dImpl->mSetCurrentTopLevelItem( dContainer, false );
        }
    }
}

bool CFlowWidgetHeader::mIsVisible() const
{
    return isVisible() /*&& ( dScrollArea && dScrollArea->isVisible() )*/;
}

void CFlowWidgetHeader::mSetExpanded( bool xExpanded )
{
    if ( xExpanded )
        mFlowWidget()->mSetCurrentItemExpanded( false );

    dScrollArea->setVisible( xExpanded );
    dSelected = xExpanded;
    mFlowWidget()->dImpl->mUpdateTabs();
    if ( dSelected )
    {
        auto lCurrSelected = dTreeWidget->selectedItems();
        for( auto && ii : lCurrSelected )
            ii->setSelected( false );
        emit mFlowWidget()->sigFlowWidgetItemSelected( dContainer, dSelected );
    }
}

bool CFlowWidgetHeader::mIsExpanded() const
{
    return (dScrollArea && dScrollArea->isVisible());
}

void CFlowWidgetHeader::mUpdate()
{
    auto lTW = dTreeWidget;
    if ( dIndexInPage )
    {
        QPalette lPalette = palette();
        lPalette.setColor( backgroundRole(), lTW->palette().color( lTW->backgroundRole() ) );
        setPalette( lPalette );
    }
    else if ( backgroundRole() != QPalette::Window )
    {
        setBackgroundRole( QPalette::Window );
    }
    update();
}

void CFlowWidgetHeader::mSetIndex( int newIndex )
{
    dIndexInPage = newIndex;
}

void CFlowWidgetHeader::mAddToLayout( QVBoxLayout* xLayout )
{
    xLayout->addWidget( this );
    xLayout->addWidget( dScrollArea );

    if ( dScrollArea )
        dScrollArea->setVisible( false );
}

void CFlowWidgetHeader::mTakeFromLayout( QVBoxLayout* xLayout )
{
    xLayout->removeWidget( dScrollArea );
    xLayout->removeWidget( this );

    if ( dScrollArea )
        dScrollArea->setVisible( false );
    setVisible( false );

}


QSize CFlowWidgetHeader::sizeHint() const
{
    QSize iconSize( 8, 8 );
    int icone = style()->pixelMetric( QStyle::PM_SmallIconSize, nullptr, mFlowWidget() );
    if ( !icon().isNull() )
    {
        iconSize += QSize( icone + 2, icone );
    }
    QSize textSize = fontMetrics().size( Qt::TextShowMnemonic, text() ) + QSize( 0, 8 );

    auto xStates = dContainer->mData( CFlowWidgetItem::ERoles::eStateStatusRole ).value< QList< int > >();
    QSize total( iconSize.width() + textSize.width(), qMax( iconSize.height(), textSize.height() ) );

    total.setWidth( total.width() + xStates.count() * (icone + 2) );

    return total.expandedTo( QApplication::globalStrut() );
}

QSize CFlowWidgetHeader::minimumSizeHint() const
{
    auto xStates = dContainer->mData( CFlowWidgetItem::ERoles::eStateStatusRole ).value< QList< int > >();
    if ( icon().isNull() && xStates.isEmpty() )
        return QSize();
    int icone = style()->pixelMetric( QStyle::PM_SmallIconSize, nullptr, mFlowWidget() );
    int lCount = icon().isNull() ? 0 : 1;
    lCount += xStates.count();
    return QSize( lCount * (icone + 8), icone + 8 );
}

void CFlowWidgetHeader::paintEvent( QPaintEvent* )
{
    QPainter lPaint( this );
    QPainter* lPainter = &lPaint;
    QStyleOptionToolBox lOption;
    initStyleOption( &lOption );
   
    lPainter->save();
    style()->drawControl( QStyle::CE_ToolBoxTabShape, &lOption, lPainter, mFlowWidget() );

    bool lEnabled = lOption.state & QStyle::State_Enabled;
    bool lSelected = lOption.state & QStyle::State_Selected;
    int lIconExtent = style()->proxy()->pixelMetric( QStyle::PM_SmallIconSize, &lOption, this );
    QPixmap lIdentityPixmap = lOption.icon.pixmap( this->window()->windowHandle(), QSize( lIconExtent, lIconExtent ), lEnabled ? QIcon::Normal : QIcon::Disabled );

    auto xStateIcons = dContainer->mData( CFlowWidgetItem::ERoles::eStateIconsRole ).value< QList< QIcon > >();
    QList< QPixmap > lPixMaps;
    for( auto && ii : xStateIcons )
    {
        auto lPixmap = ii.pixmap( this->window()->windowHandle(), QSize( lIconExtent, lIconExtent ), lEnabled ? QIcon::Normal : QIcon::Disabled );
        if ( !lPixmap.isNull() )
            lPixMaps.push_back( lPixmap );
    }


    QRect tr, ir;
    int ih = 0;
    if ( lSelected && style()->proxy()->styleHint( QStyle::SH_ToolBox_SelectedPageTitleBold, &lOption, this ) )
    {
        QFont f( lPainter->font() );
        f.setBold( true );
        lPainter->setFont( f );
    }
    QRect cr = style()->subElementRect( QStyle::SE_ToolBoxTabContents, &lOption, this );
    int lMaxWidth = cr.width();
    if ( !lIdentityPixmap.isNull() )
        lMaxWidth -= (lIdentityPixmap.width() / lIdentityPixmap.devicePixelRatio()) + 4;
    for( int ii = 0; ii < lPixMaps.count(); ++ii )
        lMaxWidth -= (lPixMaps[ ii ].width() / lIdentityPixmap.devicePixelRatio()) + 4;

    auto lTextBoundWidth = mComputeTextWidth( lOption, std::make_pair( mFlowWidget()->dImpl->mElideText(), lMaxWidth - 4 ) );

    if ( lIdentityPixmap.isNull() )
    {
        tr = QRect( cr.left() + 4, cr.top(), lTextBoundWidth, cr.height() );
    }
    else
    {
        int iw = ( lIdentityPixmap.width() / lIdentityPixmap.devicePixelRatio() + 4 );
        ih = ( lIdentityPixmap.height() / lIdentityPixmap.devicePixelRatio() );

        ir = QRect( cr.left() + 4, cr.top(), iw + 2, ih );
        tr = QRect( ir.right(), cr.top(), lTextBoundWidth, cr.height() );
    }

    QString lElidedText = lOption.fontMetrics.elidedText( lOption.text, Qt::ElideRight, lTextBoundWidth );

    if ( ih )
        lPainter->drawPixmap( ir.left(), (lOption.rect.height() - ih) / 2, lIdentityPixmap );

    int alignment = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic;
    if ( !style()->proxy()->styleHint( QStyle::SH_UnderlineShortcut, &lOption, this ) )
        alignment |= Qt::TextHideMnemonic;
    style()->proxy()->drawItemText( lPainter, tr, alignment, lOption.palette, lEnabled, lElidedText, QPalette::ButtonText );

    auto x1 = std::max( tr.right(), ir.right() ) + 4;
    for ( int ii = 0; ii < lPixMaps.count(); ++ii )
    {
        auto y1 = lOption.rect.height();
        auto ih = lPixMaps[ ii ].height()/lIdentityPixmap.devicePixelRatio();

        lPainter->drawPixmap( x1, (y1 - ih) / 2, lPixMaps[ ii ] );
        x1 += lPixMaps[ ii ].width() / lIdentityPixmap.devicePixelRatio() + 4;
    }

    if ( !lElidedText.isEmpty() && lOption.state & QStyle::State_HasFocus )
    {
        QStyleOptionFocusRect opt;
        opt.rect = tr;
        opt.palette = lOption.palette;
        opt.state = QStyle::State_None;
        style()->proxy()->drawPrimitive( QStyle::PE_FrameFocusRect, &opt, lPainter, this );
    }

    lPainter->restore();
}

int CFlowWidgetHeader::mComputeTextWidth( QStyleOptionToolBox& lOption, const std::pair< bool, int > & xMaxWidth ) const
{
    auto lRetVal = lOption.fontMetrics.boundingRect( lOption.text );
    if ( xMaxWidth.first && ( lRetVal.width() > xMaxWidth.second ) )
        lRetVal.setWidth( xMaxWidth.second );
    auto lElidedText = lOption.fontMetrics.elidedText( lOption.text, Qt::ElideRight, lRetVal.width() );
    while ( ( lElidedText != lOption.text ) && ( !xMaxWidth.first || ( lRetVal.width() < xMaxWidth.second ) ) )
    {
        lRetVal.setWidth( lRetVal.width() * 1.1 );
        lElidedText = lOption.fontMetrics.elidedText( lOption.text, Qt::ElideRight, lRetVal.width() );
    }
    return lRetVal.width();
}

CFlowWidget::CFlowWidget( QWidget* xParent, Qt::WindowFlags xFlags )
    : QFrame( xParent, xFlags )
{
    dImpl = std::make_unique< CFlowWidgetImpl >( this );
    dImpl->mRelayout();
    setBackgroundRole( QPalette::Button );
}

int CFlowWidget::mTopLevelItemCount() const
{
    return dImpl->mTopLevelItemCount();
}

CFlowWidgetItem* CFlowWidget::mGetTopLevelItem( int xIndex ) const
{
    return dImpl->mGetTopLevelItem( xIndex );
}

void CFlowWidgetImpl::mToggleTopLevelItem( int index )
{
    auto lTopLevelItem = mTopLevelItem( index );
    if ( !lTopLevelItem )
        return;

    lTopLevelItem->mSetExpanded( !lTopLevelItem->mIsExpanded() );
}

void CFlowWidget::slotOpenTopLevelItem( int index )
{
    auto lTopLevelItem = dImpl->mTopLevelItem( index );
    if ( !lTopLevelItem )
        return;
    slotExpandItem( lTopLevelItem, true );
}

void CFlowWidget::slotExpandItem( CFlowWidgetItem* xItem, bool xExpand )
{
    if ( !xItem )
        return;

    xItem->mSetExpanded( xExpand );
}

void CFlowWidget::mSelectFlowItem( CFlowWidgetItem* xItem, bool xSelect, bool xExpand )
{
    auto lOrigItem = xItem;
    while ( xItem )
    {
        if ( xExpand )
            xItem->mSetExpanded( true );
        xItem->mSetSelected( xSelect );
        xItem = xItem->mParentItem();
    }
}

void CFlowWidget::mSetCurrentItemExpanded( bool xExpanded )
{
    dImpl->mSetCurrentItemExpanded( xExpanded );
}

void CFlowWidgetImpl::mRelayout()
{
    delete fTopLayout;
    fTopLayout = new QVBoxLayout;
    fTopLayout->setContentsMargins( QMargins() );
    for ( const auto& page : dTopLevelItems )
    {
        page->dImpl->dHeader->mAddToLayout( fTopLayout );
    }
    dFlowWidget->setLayout( fTopLayout );
}

auto gPageEquals = []( const CFlowWidgetItem* page )
{
    return [page]( const std::unique_ptr<CFlowWidgetItem>& ptr )
    {
        return ptr.get() == page;
    };
};

CFlowWidgetItem* CFlowWidgetImpl::mRemoveFromTopLevelItems( CFlowWidgetItem* xItem )
{
    auto first = std::find_if( dTopLevelItems.begin(), dTopLevelItems.end(), gPageEquals( xItem ) );

    std::list< CFlowWidgetItem* > lRetVal;
    if ( first != dTopLevelItems.end() )
    {
        lRetVal.push_back( (*first).get() );
        (*first).release();
        for ( auto ii = first; ++ii != dTopLevelItems.end(); )
        {
            if ( (*ii).get() != xItem )
            {
                *first++ = std::move( *ii );
            }
            else
            {
                lRetVal.push_back( (*ii).get() );
                (*first).release();
            }
        }
    }
    dTopLevelItems.erase( first, dTopLevelItems.end() );
    if ( lRetVal.empty() )
        return nullptr;
    auto lRetValItem = *lRetVal.begin();
    return lRetValItem;
}

CFlowWidgetItem* CFlowWidgetImpl::mTakeItem( CFlowWidgetItem* xItem )
{
    xItem->dImpl->mRemoveWidgets(); // removes all GUI items from item

    auto lParent = xItem->mParentItem();
    if ( lParent )
        return lParent->mTakeChild( xItem );

    auto lTopLevelIndex = mIndexOfTopLevelItem( xItem );
    if ( lTopLevelIndex == -1 )
        return nullptr;

    bool lRemoveCurrent = xItem == dCurrentTopLevelItem;

    auto lRetVal = mRemoveFromTopLevelItems( xItem );

    if ( dTopLevelItems.empty() )
    {
        dCurrentTopLevelItem = nullptr;
        emit dFlowWidget->sigFlowWidgetItemSelected( nullptr, false );
    }
    else if ( lRemoveCurrent )
    {
        auto lNewIndex = mFindReplacementHeader( lTopLevelIndex ? (lTopLevelIndex - 1) : 0, true );
        mSetCurrentTopLevelItem( lNewIndex );
        emit dFlowWidget->sigFlowWidgetItemSelected( xItem, false );
    }

    return lRetVal;
}

CFlowWidgetItem* CFlowWidget::mTakeTopLevelItem( int xIndex )
{
    auto lTopLevelItem = mGetTopLevelItem( xIndex );
    if ( !lTopLevelItem )
        return nullptr;

    return mTakeItem( lTopLevelItem );
}

void CFlowWidget::mClear()
{
    dImpl->mClear();
}

CFlowWidgetItem* CFlowWidget::mTakeItem( CFlowWidgetItem* xItem )
{
    if ( !xItem )
        return nullptr;

    return dImpl->mTakeItem( xItem );
}

void CFlowWidgetImpl::mRemoveTopLevelItem( int xIndex )
{
    auto lTopLevelItem = mTopLevelItem( xIndex );
    if ( !lTopLevelItem )
        return;

    return dFlowWidget->mRemoveItem( lTopLevelItem );
}

void CFlowWidget::mRemoveTopLevelItem( int xIndex )
{
    dImpl->mRemoveTopLevelItem( xIndex );
}
void CFlowWidget::mRemoveItem( CFlowWidgetItem* xItem )
{
    auto lTakenItem = mTakeItem( xItem );
    if ( lTakenItem )
        lTakenItem->deleteLater();
}

int CFlowWidget::mIndexOfTopLevelItem( const CFlowWidgetItem* xItem ) const
{
    if ( !xItem )
        return -1;
    // item can be a tree widget item or a parent item
    if ( xItem->mParentItem() )
        return -1;

    const auto it = std::find_if( dImpl->dTopLevelItems.cbegin(), dImpl->dTopLevelItems.cend(), gPageEquals( xItem ) );
    if ( it == dImpl->dTopLevelItems.cend() )
        return -1;
    return static_cast<int>(it - dImpl->dTopLevelItems.cbegin());
}

void CFlowWidgetHeader::mCollectDisabledTreeWidgetItems( QTreeWidgetItem * xItem )
{
    if ( !xItem )
        return;
    dEnabled.second[ xItem ] = xItem->isDisabled();
    for( auto ii = 0; ii < xItem->childCount(); ++ii )
    {
        mCollectDisabledTreeWidgetItems( xItem->child( ii ) );
    }
}

void CFlowWidgetHeader::mCollectDisabledTreeWidgetItems()
{
    for( auto ii = 0; ii < dTreeWidget->topLevelItemCount(); ++ii )
    {
        mCollectDisabledTreeWidgetItems( dTreeWidget->topLevelItem( ii ) );
    }
}

void CFlowWidgetHeader::mSetTreeWidgetItemsEnabled( QTreeWidgetItem* xItem, bool xEnabled )
{
    if ( !xItem )
        return;
    xItem->setDisabled( !xEnabled );
    for ( auto ii = 0; ii < xItem->childCount(); ++ii )
    {
        mSetTreeWidgetItemsEnabled( xItem->child( ii ), xEnabled );
    }
}

void CFlowWidgetHeader::mSetTreeWidgetItemsEnabled( bool xEnabled )
{
    for ( auto ii = 0; ii < dTreeWidget->topLevelItemCount(); ++ii )
    {
        mSetTreeWidgetItemsEnabled( dTreeWidget->topLevelItem( ii ), xEnabled );
    }
}

void CFlowWidgetHeader::mSetEnabled( bool xEnabled )
{
    if ( dEnabled.first != xEnabled )
    {
        dEnabled.first = xEnabled;
        // if we are disabling collect all items that are already disabled
        if ( !xEnabled )
        {
            dEnabled.second.clear();
            mCollectDisabledTreeWidgetItems();
        }
        mSetTreeWidgetItemsEnabled( xEnabled ); // set all items

        // if we are enabling, disable all that were previously disabled
        if ( xEnabled )
        {
            for ( auto&& ii : dEnabled.second )
            {
                ii.first->setDisabled( ii.second );
            }
        }
        repaint();
    }
}

bool CFlowWidgetHeader::mSelected() const
{
    return dSelected;
}

void CFlowWidget::showEvent( QShowEvent* e )
{
    QFrame::showEvent( e );
}

void CFlowWidget::changeEvent( QEvent* ev )
{
    if ( ev->type() == QEvent::StyleChange )
        dImpl->mUpdateTabs();
    QFrame::changeEvent( ev );
}

bool CFlowWidget::event( QEvent* e )
{
    return QFrame::event( e );
}

int CFlowWidgetImpl::mInsertTopLevelItem( int xIndex, std::unique_ptr< CFlowWidgetItem > xItem )
{
    if ( !xItem || !xItem->dImpl )
        return -1;
    if ( !xItem->dImpl->mBeenPlaced() )
    {
        xItem->dImpl->mCreateFlowWidget( dFlowWidget );
    }

    auto lFlowItem = xItem.get();
    bool lCurrentChanged = false;
    //if ( xIndex < 0 || xIndex >= static_cast<int>(dTopLevelItems.size()) )
    //{
    //    xIndex = static_cast<int>(dTopLevelItems.size());
    //    dTopLevelItems.insert( dTopLevelItems.cbegin() + xIndex, std::move( xItem ) );
    //    lFlowItem->dImpl->dHeader->mAddToLayout( fTopLayout );
    //    mRelayout();
    //    if ( xIndex == 0 )
    //    {
    //        mSetCurrentTopLevelItem( xIndex );
    //        lCurrentChanged = true;
    //    }
    //}
    //else
    //{
    //    dTopLevelItems.insert( dTopLevelItems.cbegin() + xIndex, std::move( xItem ) );
    //    mRelayout();
    //    if ( dCurrentTopLevelItem )
    //    {
    //        int oldindex = mIndexOfTopLevelItem( dCurrentTopLevelItem );
    //        if ( xIndex <= oldindex )
    //        {
    //            dCurrentTopLevelItem = nullptr; // trigger change
    //            lCurrentChanged = true;
    //            mSetCurrentTopLevelItem( oldindex );
    //        }
    //    }
    //}

    if ( xIndex < 0 || xIndex >= static_cast<int>(dTopLevelItems.size()) )
        xIndex = static_cast<int>(dTopLevelItems.size());

    dTopLevelItems.insert( dTopLevelItems.cbegin() + xIndex, std::move( xItem ) );
    mRelayout();
    if ( dCurrentTopLevelItem )
    {
        int oldindex = mIndexOfTopLevelItem( dCurrentTopLevelItem );
        if ( xIndex <= oldindex )
        {
            dCurrentTopLevelItem = nullptr; // trigger change
            lCurrentChanged = true;
            mSetCurrentTopLevelItem( oldindex );
        }
    }
    else
    {
        mSetCurrentTopLevelItem( xIndex );
        lCurrentChanged = true;
    }

    QObject::connect( lFlowItem->dImpl->dHeader, &CFlowWidgetHeader::clicked,
                      [this, lFlowItem]()
    {
        auto lIndex = mIndexOfTopLevelItem( lFlowItem );
        if ( (lIndex < 0) || (lIndex >= static_cast<int>(dTopLevelItems.size())) )
            return;
        dFlowWidget->dImpl->mSetCurrentTopLevelItem( dTopLevelItems[ lIndex ].get() );
        dFlowWidget->sigFlowWidgetItemSelected( dTopLevelItems[ lIndex ].get(), true );
    } );

    QObject::connect( lFlowItem->dImpl->dHeader, &CFlowWidgetHeader::sigDoubleClicked,
                      [this, lFlowItem]()
    {
        auto lIndex = mIndexOfTopLevelItem( lFlowItem );
        if ( (lIndex < 0) || (lIndex >= static_cast<int>(dTopLevelItems.size())) )
            return;
        dFlowWidget->dImpl->mSetCurrentTopLevelItem( dTopLevelItems[ lIndex ].get() );
        dFlowWidget->sigFlowWidgetItemDoubleClicked( dTopLevelItems[ lIndex ].get() );
    } );

    lFlowItem->dImpl->dHeader->mSetVisible( true, lCurrentChanged );

    mUpdateTabs();
    emit dFlowWidget->sigFlowWidgetItemInserted( dTopLevelItems[ xIndex ].get() );
    return xIndex;
}

int CFlowWidgetImpl::mInsertItem( int xIndex, std::unique_ptr< CFlowWidgetItem > xItem, CFlowWidgetItem* xParent )
{
    Q_ASSERT( xParent );
    if ( !xParent )
        return -1;

    auto lPtr = xItem.release();
    return xParent->mInsertChild( xIndex, lPtr );
}

int CFlowWidgetImpl::mInsertTopLevelItem( int xIndex, CFlowWidgetItem* xItem )
{
    auto lUniquePtr = std::unique_ptr< CFlowWidgetItem >( xItem );
    return mInsertTopLevelItem( xIndex, std::move( lUniquePtr ) );
}

CFlowWidgetItem* CFlowWidget::mInsertTopLevelItem( int xIndex, int xStateID, const QString& xFlowName, const QIcon& xDescIcon )
{
    auto lItem = new CFlowWidgetItem( xStateID, xFlowName, xDescIcon );
    auto lRetVal = mInsertTopLevelItem( xIndex, lItem );
    return lRetVal.second ? lRetVal.first : nullptr;
}

std::pair< CFlowWidgetItem*, bool > CFlowWidget::mInsertTopLevelItem( CFlowWidgetItem * xPeer, CFlowWidgetItem* xItem, bool xBefore )
{
    if ( !xPeer || !xItem )
        return std::make_pair( xItem, false );

    auto lIndex = xPeer->mIndexInParent();
    if ( !xBefore )
        lIndex++;
    return mInsertTopLevelItem( lIndex, xItem );
}

std::pair< CFlowWidgetItem*, bool > CFlowWidget::mInsertTopLevelItem( int xIndex, CFlowWidgetItem* xItem )
{
    if ( xItem->dImpl->mBeenPlaced() && !xItem->mIsTopLevelItem() )
        return std::make_pair( xItem, false );

    dImpl->mInsertTopLevelItem( xIndex, xItem );
    return std::make_pair( xItem, true );
}

int CFlowWidgetImpl::mInsertItem( int xIndex, CFlowWidgetItem* xItem, CFlowWidgetItem* xParent )
{
    auto lUniquePtr = std::unique_ptr< CFlowWidgetItem >( xItem );
    return mInsertItem( xIndex, std::move( lUniquePtr ), xParent );
}

CFlowWidgetItem* CFlowWidget::mInsertItem( int xIndex, int xStateID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidgetItem* xParent )
{
    auto lItem = new CFlowWidgetItem( xStateID, xFlowName, xDescIcon ); // no parent the mInsertItem sets the parent
    dImpl->mInsertItem( xIndex, lItem, xParent );
    return lItem;
}

CFlowWidgetItem* CFlowWidget::mCurrentTopLevelItem() const
{
    return dImpl->mCurrentTopLevelItem();
}

CFlowWidgetItem* CFlowWidget::mSelectedItem() const
{
    auto lCurrentTop = mCurrentTopLevelItem();
    if ( !lCurrentTop )
        return nullptr;

    auto lItemSelected = lCurrentTop->dImpl->mSelectedItem();
    return lItemSelected ? lItemSelected : lCurrentTop;
}

void CFlowWidget::mRegisterStateStatus( int xState, const QString & xDescription, const QIcon& xIcon )
{
    return dImpl->mRegisterStateStatus( xState, xDescription, xIcon, false );
}

int CFlowWidget::mGetNextStatusID() const
{
    return dImpl->mGetNextStatusID();
}

QList< std::tuple< int, QString, QIcon > > CFlowWidget::mGetRegisteredStatuses() const
{
    return dImpl->mGetRegisteredStatuses();
}

std::pair< QString, QIcon > CFlowWidget::mGetStateStatus( int xState ) const
{
    return dImpl->mGetStateStatus( xState );
}

void CFlowWidget::mSetElideText( bool xElide )
{
    return dImpl->mSetElideText( xElide );
}

bool CFlowWidget::mElideText() const
{
    return dImpl->mElideText();
}

void CFlowWidget::mDump( QJsonObject & xJSON ) const
{
    dImpl->mDump( xJSON );
}

QString CFlowWidget::mDump( bool xCompacted ) const
{
    QJsonObject lObject;
    mDump( lObject );
    auto lRetVal = QJsonDocument( lObject ).toJson( xCompacted ? QJsonDocument::Compact : QJsonDocument::Indented );
    return lRetVal;
}

#include "FlowWidget.moc"
