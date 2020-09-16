#undef QT_NO_DEBUG_OUTPUT
#include "FlowWidget.h"

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

using TFlowWidgetItems = std::vector< std::unique_ptr< CFlowWidgetItem > >;
using TDataMap = std::map< int, QVariant >;

class CFlowWidgetHeader : public QAbstractButton
{
    Q_OBJECT
public:
    friend class CFlowWidgetImpl;
    CFlowWidgetHeader( CFlowWidgetItem* xContainer, CFlowWidget* xParent );

    CFlowWidget* mFlowWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void mSetText( const QString& text ) { return setText( text ); }
    QString mText() const { return text(); }

    void mSetIcon( const QIcon& icon ) { return setIcon( icon ); }
    QIcon mIcon() const { return icon(); }

    void mSetToolTip( const QString& tooltip ) { return setToolTip( tooltip ); }
    QString mToolTip() const { return toolTip(); }

    void mAddChild( CFlowWidgetItem* xChild ) { mInsertChild( -1, xChild ); }
    int mInsertChild( int xIndex, CFlowWidgetItem* xChild );

    int mIndexOfChild( const CFlowWidgetItem* xItem ) const;
    CFlowWidgetItem* mTakeChild( CFlowWidgetItem* xItem );
    int mChildCount() const;
    CFlowWidgetItem* mGetChild( int xIndex ) const;

    void mSetData( int xRole, const QVariant& xData );
    QVariant mData( int xRole ) const;

    void mSetVisible( bool xVisible, bool xExpandIfShow );
    bool mIsVisible() const;

    void mSetDisabled( bool xDisabled ) { return mSetEnabled( !xDisabled ); }
    bool mIsDisabled() const { return !mIsEnabled(); }

    void mSetEnabled( bool xEnabled );
    bool mIsEnabled() const { return isEnabled(); }

    bool mSelected() const;
    void mSetSelected( bool b ) { mSetExpanded( b ); }

    void mSetExpanded( bool xExpanded );
    bool mIsExpanded() const;

    void mUpdate();

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

private:
    CFlowWidgetItem* dContainer{ nullptr }; // not owned no delete
    QScrollArea* dScrollArea{ nullptr }; // child of the parent FlowWidget
    QTreeWidget* dTreeWidget{ nullptr }; // child of dScrollArea

    bool dSelected{ false };
    int dIndexInPage{ -1 };
    std::vector< CFlowWidgetItem* > dTopItems; // owned in the dContainers dChildren
    std::map< QTreeWidgetItem*, CFlowWidgetItem* > dTopItemMap; // only the top level items
    std::map< QTreeWidgetItem*, CFlowWidgetItem* > dAllChildItemMap; // the individual items own the memory
    TDataMap dData;
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

    QString mToolTip() const
    {
        return dToolTip;
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

    int mInsertChild( int xIndex, std::unique_ptr< CFlowWidgetItem > xChild )
    {
        auto lChildPtr = xChild.get();
        if ( xIndex < 0 || (xIndex > static_cast<int>(dChildren.size())) )
            xIndex = static_cast<int>(dChildren.size());
        dChildren.insert( dChildren.begin() + xIndex, std::move( xChild ) );

        Q_ASSERT( !lChildPtr->dImpl->dParent || (lChildPtr->dImpl->dParent == dContainer) );
        if ( !lChildPtr->dImpl->dParent )
            lChildPtr->dImpl->dParent = dContainer;

        auto lChildTreeWidgetItem = new QTreeWidgetItem( QStringList() << lChildPtr->mText() );
        lChildTreeWidgetItem->setIcon( 0, lChildPtr->mIcon() );
        lChildPtr->dImpl->dTreeWidgetItem = lChildTreeWidgetItem;

        if ( dHeader ) // its a top level, thus it becomes a top level to the tree
        {
            return dHeader->mInsertChild( xIndex, lChildPtr );
        }
        else if ( dTreeWidgetItem )
        {
            Q_ASSERT( xIndex >= 0 && (xIndex <= dTreeWidgetItem->childCount()) );

            dTreeWidgetItem->insertChild( xIndex, lChildTreeWidgetItem );
            if ( mHeader() )
                mHeader()->mAddToMap( lChildTreeWidgetItem, lChildPtr );
            dChildItemMap[ lChildTreeWidgetItem ] = lChildPtr;
            return xIndex;
        }
        return -1;
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

    void mSetData( int xRole, const QVariant& xData, bool xSetState = true )
    {
        if ( dHeader )
            dHeader->mSetData( xRole, xData );
        else if ( dTreeWidgetItem )
            dTreeWidgetItem->setData( 0, xRole, xData );

        dData[ xRole ] = xData;
        if ( xSetState && (xRole == CFlowWidgetItem::eStateStatusRole) )
        {
            mSetDisabled( (xData.toInt() & CFlowWidgetItem::EStates::eDisabled) != 0 );
        }
    }

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

    bool mIsStateDisabled() const
    {
        auto lData = mData( CFlowWidgetItem::eStateStatusRole ).toInt();
        return (lData & CFlowWidgetItem::EStates::eDisabled) != 0;
    }

    void mSetStateDisabled( bool xDisabled )
    {
        auto lData = mData( CFlowWidgetItem::eStateStatusRole ).toInt();
        if ( xDisabled )
            lData |= CFlowWidgetItem::EStates::eDisabled;
        else
            lData &= ~CFlowWidgetItem::EStates::eDisabled;

        mSetData( CFlowWidgetItem::eStateStatusRole, lData, false );
    }

    void mSetDisabled( bool xDisabled )
    {
        if ( dHeader )
            dHeader->mSetDisabled( xDisabled );
        else if ( dTreeWidgetItem )
            dTreeWidgetItem->setDisabled( xDisabled );

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
    void mTakeWidgets();

    CFlowWidgetItem* dContainer{ nullptr };
    int dStateID{ -1 };
    CFlowWidgetItem* dParent{ nullptr };
    CFlowWidgetHeader* dHeader{ nullptr };
    QTreeWidgetItem* dTreeWidgetItem{ nullptr };

    std::map< QTreeWidgetItem*, CFlowWidgetItem* > dChildItemMap;  // flowItems owned by dChildren

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
    }

    size_t mOpenCount() const
    {
        size_t retVal = 0;
        for ( auto&& ii : fTopLevelItems )
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
        for ( size_t ii = 0; ii < fTopLevelItems.size(); ++ii )
        {
            if ( fTopLevelItems[ ii ].get() == xItem )
                return static_cast<int>(ii);
        }
        return -1;
    }

    CFlowWidgetItem* mGetTopLevelItem( int xIndex ) const
    {
        if ( (xIndex < 0) || (xIndex >= fTopLevelItems.size()) )
            return nullptr;
        return fTopLevelItems[ xIndex ].get();
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
        return static_cast<int>(fTopLevelItems.size());
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

    void mUpdateTabs();
    void mRelayout();

    QVBoxLayout* fTopLayout{ nullptr };
    CFlowWidget* dFlowWidget{ nullptr };

    CFlowWidgetItem* dCurrentTopLevelItem{ nullptr };

    TFlowWidgetItems fTopLevelItems;
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

void CFlowWidgetItem::mSetData( int xRole, const QVariant& xVariant )
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
    return dImpl->mToolTip();
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

void CFlowWidgetItemImpl::mTakeWidgets()
{
    for ( int ii = 0; ii < mChildCount(); ++ii )
    {
        auto lCurr = mGetChild( ii );
        if ( !lCurr )
            continue;
        dChildren.push_back( std::move( std::unique_ptr< CFlowWidgetItem >( lCurr ) ) );
    }
}

bool CFlowWidgetItem::mIsTopLevelItem() const
{
    return dImpl->mBeenPlaced() && (dImpl->dHeader != nullptr) && (dImpl->dTreeWidgetItem == nullptr);
}

void CFlowWidgetItem::mSetStateStatus( EStates xStateStatus )
{
    mSetData( eStateStatusRole, xStateStatus );
}

CFlowWidgetItem::EStates CFlowWidgetItem::mStateStatus() const
{
    return static_cast<EStates>(mData( eStateStatusRole ).toInt());
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
    if ( xIndex >= 0 && xIndex < static_cast<int>(fTopLevelItems.size()) )
        return fTopLevelItems[ xIndex ].get();
    return nullptr;
}

const CFlowWidgetItem* CFlowWidgetImpl::mTopLevelItem( int xIndex ) const
{
    if ( xIndex >= 0 && xIndex < static_cast<int>(fTopLevelItems.size()) )
        return fTopLevelItems[ xIndex ].get();
    return nullptr;
}

void CFlowWidgetImpl::mUpdateTabs()
{
    CFlowWidgetHeader* lLastButton = dCurrentTopLevelItem ? dCurrentTopLevelItem->dImpl->dHeader : nullptr;
    bool lAfter = false;
    int lIndex = 0;
    for ( const auto& lCurrItem : fTopLevelItems )
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

CFlowWidgetHeader::CFlowWidgetHeader( CFlowWidgetItem* xContainer, CFlowWidget* parent ) :
    QAbstractButton( parent ),
    dContainer( xContainer )
{
    setBackgroundRole( QPalette::Window );
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
    setFocusPolicy( Qt::NoFocus );

    dTreeWidget = new QTreeWidget;
    dTreeWidget->setExpandsOnDoubleClick( false );
    dTreeWidget->header()->setHidden( true );
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

    dScrollArea = new QScrollArea( parent );
    dScrollArea->setWidget( dTreeWidget );
    dScrollArea->setWidgetResizable( true );
    dScrollArea->hide();
    dScrollArea->setFrameStyle( QFrame::NoFrame );
    dTreeWidget->installEventFilter( this );
}

bool CFlowWidgetHeader::eventFilter( QObject* xWatched, QEvent* xEvent )
{
    if ( xWatched == dTreeWidget )
    {
        if ( xEvent->type() == QEvent::ToolTip )
        {
            auto lHelpEvent = dynamic_cast<QHelpEvent*>(xEvent);
            auto xItem = dTreeWidget->itemAt( lHelpEvent->pos() );
            emit mFlowWidget()->sigFlowWidgetItemHovered( mFindFlowItem( xItem ) );
        }
    }
    return QAbstractButton::eventFilter( xWatched, xEvent );
}

bool CFlowWidgetHeader::event( QEvent* xEvent )
{
    if ( xEvent->type() == QEvent::ToolTip )
    {
        emit mFlowWidget()->sigFlowWidgetItemHovered( dContainer );
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

QSize CFlowWidgetHeader::sizeHint() const
{
    QSize iconSize( 8, 8 );
    if ( !icon().isNull() )
    {
        int icone = style()->pixelMetric( QStyle::PM_SmallIconSize, nullptr, mFlowWidget() );
        iconSize += QSize( icone + 2, icone );
    }
    QSize textSize = fontMetrics().size( Qt::TextShowMnemonic, text() ) + QSize( 0, 8 );

    QSize total( iconSize.width() + textSize.width(), qMax( iconSize.height(), textSize.height() ) );
    return total.expandedTo( QApplication::globalStrut() );
}

QSize CFlowWidgetHeader::minimumSizeHint() const
{
    if ( icon().isNull() )
        return QSize();
    int icone = style()->pixelMetric( QStyle::PM_SmallIconSize, nullptr, mFlowWidget() );
    return QSize( icone + 8, icone + 8 );
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

void CFlowWidgetHeader::mSetData( int xRole, const QVariant& xData )
{
    dData[ xRole ] = xData;
}

QVariant CFlowWidgetHeader::mData( int xRole ) const
{
    auto pos = dData.find( xRole );
    if ( pos == dData.end() )
        return QVariant();
    return (*pos).second;
}

void CFlowWidgetHeader::initStyleOption( QStyleOptionToolBox* xOption ) const
{
    if ( !xOption )
        return;
    xOption->initFrom( this );
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
        emit mFlowWidget()->sigFlowWidgetItemSelected( dContainer, dSelected );
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
    setVisible( false );
}

void CFlowWidgetHeader::mTakeFromLayout( QVBoxLayout* xLayout )
{
    xLayout->removeWidget( dScrollArea );
    xLayout->removeWidget( this );

    if ( dScrollArea )
        dScrollArea->setVisible( false );
    setVisible( false );

}

void CFlowWidgetHeader::paintEvent( QPaintEvent* )
{
    QPainter paint( this );
    QPainter* p = &paint;
    QStyleOptionToolBox opt;
    initStyleOption( &opt );
    style()->drawControl( QStyle::CE_ToolBoxTab, &opt, p, mFlowWidget() );
}

CFlowWidget::CFlowWidget( QWidget* xParent, Qt::WindowFlags xFlags )
    : QFrame( xParent, xFlags )
{
    dImpl = std::make_unique< CFlowWidgetImpl >( this );
    dImpl->fTopLayout = new QVBoxLayout( this );
    dImpl->fTopLayout->setContentsMargins( QMargins() );
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
    fTopLayout = new QVBoxLayout( dFlowWidget );
    fTopLayout->setContentsMargins( QMargins() );
    for ( const auto& page : fTopLevelItems )
    {
        page->dImpl->dHeader->mAddToLayout( fTopLayout );
    }
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
    auto first = std::find_if( fTopLevelItems.begin(), fTopLevelItems.end(), gPageEquals( xItem ) );

    std::list< CFlowWidgetItem* > lRetVal;
    if ( first != fTopLevelItems.end() )
    {
        lRetVal.push_back( (*first).get() );
        (*first).release();
        for ( auto ii = first; ++ii != fTopLevelItems.end(); )
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
    fTopLevelItems.erase( first, fTopLevelItems.end() );
    if ( lRetVal.empty() )
        return nullptr;
    auto lRetValItem = *lRetVal.begin();
    return lRetValItem;
}

CFlowWidgetItem* CFlowWidgetImpl::mTakeItem( CFlowWidgetItem* xItem )
{
    xItem->dImpl->mTakeWidgets(); // removes all GUI items from item

    auto lParent = xItem->mParentItem();
    if ( lParent )
        return lParent->mTakeChild( xItem );

    auto lTopLevelIndex = mIndexOfTopLevelItem( xItem );
    if ( lTopLevelIndex == -1 )
        return nullptr;

    xItem->dImpl->mTakeWidgets();
    xItem->dImpl->dHeader->mTakeFromLayout( fTopLayout );

    bool lRemoveCurrent = xItem == dCurrentTopLevelItem;

    auto lRetVal = mRemoveFromTopLevelItems( xItem );

    if ( fTopLevelItems.empty() )
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

    const auto it = std::find_if( dImpl->fTopLevelItems.cbegin(), dImpl->fTopLevelItems.cend(), gPageEquals( xItem ) );
    if ( it == dImpl->fTopLevelItems.cend() )
        return -1;
    return static_cast<int>(it - dImpl->fTopLevelItems.cbegin());
}

void CFlowWidgetHeader::mSetEnabled( bool xEnabled )
{
    setEnabled( xEnabled );
    if ( !xEnabled && mFlowWidget()->mCurrentTopLevelItem() == dContainer )
    {
        auto lNewIndex = mFlowWidget()->dImpl->mFindReplacementHeader( dContainer );
        mFlowWidget()->dImpl->mSetCurrentTopLevelItem( lNewIndex );
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
    if ( !xItem || !xItem->dImpl || !xItem->dImpl->dHeader )
    {
        auto dIcon = xItem->mIcon();
        auto dText = xItem->mText();
        xItem->dImpl->dHeader = new CFlowWidgetHeader( xItem.get(), dFlowWidget );
        xItem->dImpl->mSetIcon( dIcon );
        xItem->dImpl->mSetText( dText );
        xItem->dImpl->dHeader->setObjectName( QLatin1String( "flowwidgetitem_flowwidgetheader" ) );
    }

    auto lFlowItem = xItem.get();
    bool lCurrentChanged = false;
    if ( xIndex < 0 || xIndex >= static_cast<int>(fTopLevelItems.size()) )
    {
        xIndex = static_cast<int>(fTopLevelItems.size());
        fTopLevelItems.push_back( std::move( xItem ) );
        lFlowItem->dImpl->dHeader->mAddToLayout( fTopLayout );
        if ( xIndex == 0 )
        {
            mSetCurrentTopLevelItem( xIndex );
            lCurrentChanged = true;
        }
    }
    else
    {
        fTopLevelItems.insert( fTopLevelItems.cbegin() + xIndex, std::move( xItem ) );
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
    }

    QObject::connect( lFlowItem->dImpl->dHeader, &CFlowWidgetHeader::clicked,
                      [this, lFlowItem]()
    {
        auto lIndex = mIndexOfTopLevelItem( lFlowItem );
        if ( (lIndex < 0) || (lIndex >= static_cast<int>(fTopLevelItems.size())) )
            return;
        dFlowWidget->dImpl->mSetCurrentTopLevelItem( fTopLevelItems[ lIndex ].get() );
        dFlowWidget->sigFlowWidgetItemSelected( fTopLevelItems[ lIndex ].get(), true );
    } );

    QObject::connect( lFlowItem->dImpl->dHeader, &CFlowWidgetHeader::sigDoubleClicked,
                      [this, lFlowItem]()
    {
        auto lIndex = mIndexOfTopLevelItem( lFlowItem );
        if ( (lIndex < 0) || (lIndex >= static_cast<int>(fTopLevelItems.size())) )
            return;
        dFlowWidget->dImpl->mSetCurrentTopLevelItem( fTopLevelItems[ lIndex ].get() );
        dFlowWidget->sigFlowWidgetItemDoubleClicked( fTopLevelItems[ lIndex ].get() );
    } );

    lFlowItem->dImpl->dHeader->mSetVisible( true, lCurrentChanged );

    mUpdateTabs();
    emit dFlowWidget->sigFlowWidgetItemInserted( fTopLevelItems[ xIndex ].get() );
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

#include "FlowWidget.moc"
