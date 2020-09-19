#ifndef __FLOWWIDGET_H
#define __FLOWWIDGET_H

#include <QFrame>
#include <QIcon>
#include <QAbstractButton>
#include <memory>
#include <map>

class QScrollArea;
class CFlowWidget;
class CFlowWidgetHeader;
class QVBoxLayout;
class QStyleOptionToolBox;
class QJsonObject;

class CFlowWidgetItemImpl;
class CFlowWidgetImpl;
class CFlowWidgetHeaderImpl;

// each Item can have 2 icons
// the first is defined via the xDescIcon parameter during construction, is used to represent the action describe by the flow item, ie the flow item "load files" might use the file open icon
// the second is set via the state attribute on the Item, as a state changes, that icon is update to reflect the new state
//    there are 7 predefined states with associated pre-defined icons, as part of the initialization of the CFlowWidget itself, the client can add new flow states (int value and icon) to the system

// note, you do not own the memory for the CFlowWidgetItem once its added to a parent FlowWidget or FlowWidgetItem

class CFlowWidgetItem
{
public:
    friend class CFlowWidgetItemImpl;
    friend class CFlowWidgetImpl;
    friend class CFlowWidget;
    friend class CFlowWidgetHeader;

    enum EStates
    {
        eDisabled = 0x01,
        eReady = 0x10,
        eError = 0x20,
        eWarning = 0x30,
        eInfo = 0x40,
        eLastState = eInfo
        // custom states MUST NOT use bit 0, that is used to bit wise enable "disabled"
        // so you could have "error and disable" (or any state & disable)
    };

    enum EFlowItemDataRole
    {
        eStateStatusRole = Qt::UserRole + 1
    };

    CFlowWidgetItem();
    CFlowWidgetItem( int xStateID, const QString& xFlowName, CFlowWidget* xParent );
    CFlowWidgetItem( int xStateID, const QString& xFlowName, CFlowWidgetItem* xParent );

    CFlowWidgetItem( int xStateID, const QIcon& xDescIcon, CFlowWidget* xParent );
    CFlowWidgetItem( int xStateID, const QIcon& xDescIcon, CFlowWidgetItem* xParent );

    CFlowWidgetItem( int xStateID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidget* xParent );
    CFlowWidgetItem( int xStateID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidgetItem* xParent );

    void deleteLater();

    CFlowWidgetItem* mParentItem() const;
    CFlowWidget* mGetFlowWidget() const;

    void mAddChild( CFlowWidgetItem* xChild );
    int mInsertChild( int xIndex, CFlowWidgetItem* xChild );
    int mInsertChild( CFlowWidgetItem* xPeer, CFlowWidgetItem* xChild, bool xBefore );

    int mIndexOfChild( const CFlowWidgetItem* xChild );

    int mChildCount() const;
    CFlowWidgetItem* mGetChild( int xIndex ) const;

    CFlowWidgetItem* mTakeChild( CFlowWidgetItem* xItem );

    void mSetData( int xRole, const QVariant& xVariant );
    QVariant mData( int xRole ) const; // for top level items tied to QAbstract Button, for Sub Items tree widget item

    void mSetText( const QString& xText );
    QString mText() const;
    QString mFullText( const QChar& xSeparator = QChar( '.' ) ) const;

    void mSetIcon( const QIcon& xIcon );
    QIcon mIcon() const;

    void mSetStateID( int xStateID );
    int mStateID() const;

    void mSetToolTip( const QString& xToolTip );
    QString mToolTip() const;

    void mSetStateStatus( EStates xStateStatus );
    EStates mStateStatus() const;

    void mSetHidden( bool xHidden );
    bool mIsHidden() const;

    void mSetVisible( bool xVisible, bool xExpandIfVisible );
    bool mIsVisible() const;

    void mSetDisabled( bool xDisabled );
    bool mIsDisabled() const;

    void mSetEnabled( bool xEnabled );
    bool mIsEnabled() const;

    void mSetExpanded( bool xExpanded );
    bool mIsExpanded() const;

    void mSetSelected( bool xSelected );
    bool mSelected() const;

    bool mIsTopLevelItem() const;

    int mIndexInParent() const;
    QString mDump( bool xRecursive, bool xCompacted ) const;
    void mDump( QJsonObject& xJSON, bool xRecursive ) const;

private:
    CFlowWidgetItem( int xStateID, const QString& xFlowName, const QIcon& xDescIcon );
    std::unique_ptr< CFlowWidgetItemImpl > dImpl;
};
Q_DECLARE_METATYPE( CFlowWidgetItem* );

class CFlowWidget : public QFrame
{
    friend class CFlowWidgetHeader;
    friend class CFlowWidgetImpl;
    friend class CFlowWidgetItem;

    Q_OBJECT
        Q_PROPERTY( int count READ mTopLevelItemCount )

public:
    CFlowWidget& operator=( const CFlowWidget& rhs ) = delete;
    CFlowWidget( const CFlowWidget& rhs ) = delete;

    explicit CFlowWidget( QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    // The returned CFLowWidgetItems are OWNED by the flow widget, and should not be deleted by the user without removing them from thw CFlowWidget or parent CFlowWidgetItem first
    virtual CFlowWidgetItem* mAddTopLevelItem( int xStateID, const QString& xFlowName, const QIcon& xDescIcon ) { return mInsertTopLevelItem( -1, xStateID, xFlowName, xDescIcon ); }
    virtual CFlowWidgetItem* mAddTopLevelItem( int xStateID, const QString& xFlowName ) { return mAddTopLevelItem( xStateID, xFlowName, QIcon() ); }
    virtual CFlowWidgetItem* mAddTopLevelItem( int xStateID, const QIcon& xDescIcon ) { return mAddTopLevelItem( xStateID, QString(), xDescIcon ); }

    virtual CFlowWidgetItem* mInsertTopLevelItem( int xIndex, int xStateID, const QString& xFlowName, const QIcon& xDescIcon );
    virtual CFlowWidgetItem* mInsertTopLevelItem( int xIndex, int xStateID, const QString& xFlowName ) { return mInsertTopLevelItem( xIndex, xStateID, xFlowName, QIcon() ); }
    virtual CFlowWidgetItem* mInsertTopLevelItem( int xIndex, int xStateID, const QIcon& xDescIcon ) { return mInsertTopLevelItem( xIndex, xStateID, QString(), xDescIcon ); }

    virtual std::pair< CFlowWidgetItem*, bool > mAddTopLevelItem( CFlowWidgetItem* xItem ) { return mInsertTopLevelItem( -1, xItem ); }
    virtual std::pair< CFlowWidgetItem*, bool > mInsertTopLevelItem( int xIndex, CFlowWidgetItem* xItem );
    virtual std::pair< CFlowWidgetItem*, bool > mInsertTopLevelItem( CFlowWidgetItem* xPeer, CFlowWidgetItem* xItem, bool xBefore );

    virtual CFlowWidgetItem* mAddItem( int xStateID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidgetItem* xParent ) { return mInsertItem( -1, xStateID, xFlowName, xDescIcon, xParent ); }
    virtual CFlowWidgetItem* mAddItem( int xStateID, const QString& xFlowName, CFlowWidgetItem* xParent ) { return mInsertItem( -1, xStateID, xFlowName, QIcon(), xParent ); }
    virtual CFlowWidgetItem* mAddItem( int xStateID, const QIcon& xDescIcon, CFlowWidgetItem* xParent ) { return mInsertItem( -1, xStateID, QString(), xDescIcon, xParent ); }

    virtual CFlowWidgetItem* mInsertItem( int xIndex, int xStateID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidgetItem* xParent );
    virtual CFlowWidgetItem* mInsertItem( int xIndex, int xStateID, const QString& xFlowName, CFlowWidgetItem* xParent ) { return mInsertItem( xIndex, xStateID, xFlowName, QIcon(), xParent ); }
    virtual CFlowWidgetItem* mInsertItem( int xIndex, int xStateID, const QIcon& xDescIcon, CFlowWidgetItem* xParent ) { return mInsertItem( xIndex, xStateID, QString(), xDescIcon, xParent ); }

    virtual int mTopLevelItemCount() const;
    virtual CFlowWidgetItem* mGetTopLevelItem( int xIndex ) const;
    virtual int mIndexOfTopLevelItem( const CFlowWidgetItem* xItem ) const; // returns -1 if not found as a top level widget

    virtual void mRemoveTopLevelItem( int xIndex ); // deletes the CFlowStateDefItem
    virtual CFlowWidgetItem* mTakeTopLevelItem( int xIndex );

    virtual CFlowWidgetItem* mTakeItem( CFlowWidgetItem* xItem );   // does the work of "get parent, parent->take item
    virtual void mRemoveItem( CFlowWidgetItem* xItem ); // does the work of "get parent, parent->remove item

    virtual void mSelectFlowItem( CFlowWidgetItem* xItem, bool xSelect, bool xExpand );

    virtual CFlowWidgetItem* mSelectedItem() const; // only one item is selectable at a time.

    virtual QString mDump( bool xCompacted ) const;
    virtual void mDump( QJsonObject& xTS ) const;
protected Q_SLOTS:
    virtual void slotOpenTopLevelItem( int xIndex );
    virtual void slotExpandItem( CFlowWidgetItem* xItem, bool xExpand );
Q_SIGNALS:
    void sigFlowWidgetItemInserted( CFlowWidgetItem* index );

    void sigFlowWidgetItemSelected( CFlowWidgetItem* xItem, bool xSelected ); // can be null if all items removed 
    void sigFlowWidgetItemDoubleClicked( CFlowWidgetItem* xItem );
    void sigFlowWidgetItemHovered( CFlowWidgetItem* xItem ); // can be null if hovered over a non item
protected:
    bool event( QEvent* e ) override;
    void showEvent( QShowEvent* e ) override;
    void changeEvent( QEvent* ) override;

protected:
    virtual void mSetCurrentItemExpanded( bool xExpanded );
    CFlowWidgetItem* mCurrentTopLevelItem() const;
    std::unique_ptr< CFlowWidgetImpl > dImpl;
};

#endif 
