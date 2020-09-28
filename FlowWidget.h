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

class CFlowWidgetItem;
class CFlowWidgetItemImpl;
class CFlowWidgetImpl;
class CFlowWidgetHeaderImpl;

class CFlowWidget : public QFrame
{
    friend class CFlowWidgetHeader;
    friend class CFlowWidgetImpl;
    friend class CFlowWidgetItem;
    friend class CFlowWidgetItemImpl;

    Q_OBJECT;
    Q_PROPERTY( int count READ mTopLevelItemCount )

public:
    enum EStates
    {
        eNone,
        eDisabled,
        eReadyToRun, // ready to run
        eRunning,
        eRunCompletedWithWarning,
        eRunCompletedWithError,
        eRunCompletedWithInfo,
        eRunPassed,
        eLastState = eRunPassed
    };

    CFlowWidget& operator=( const CFlowWidget& rhs ) = delete;
    CFlowWidget( const CFlowWidget& rhs ) = delete;
    ~CFlowWidget();

    explicit CFlowWidget( QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    virtual std::pair< bool, QString > mLoadFromXML( const QString & xFileName );
    // The returned CFLowWidgetItems are OWNED by the flow widget, and should not be deleted by the user without removing them from thw CFlowWidget or parent CFlowWidgetItem first
    virtual CFlowWidgetItem* mAddTopLevelItem( const QString & xStepID, const QString& xFlowName, const QIcon& xDescIcon ) { return mInsertTopLevelItem( -1, xStepID, xFlowName, xDescIcon ); }
    virtual CFlowWidgetItem* mAddTopLevelItem( const QString & xStepID, const QString& xFlowName ) { return mAddTopLevelItem( xStepID, xFlowName, QIcon() ); }
    virtual CFlowWidgetItem* mAddTopLevelItem( const QString & xStepID, const QIcon& xDescIcon ) { return mAddTopLevelItem( xStepID, QString(), xDescIcon ); }

    virtual CFlowWidgetItem* mInsertTopLevelItem( int xIndex, const QString & xStepID, const QString& xFlowName, const QIcon& xDescIcon );
    virtual CFlowWidgetItem* mInsertTopLevelItem( int xIndex, const QString & xStepID, const QString& xFlowName ) { return mInsertTopLevelItem( xIndex, xStepID, xFlowName, QIcon() ); }
    virtual CFlowWidgetItem* mInsertTopLevelItem( int xIndex, const QString & xStepID, const QIcon& xDescIcon ) { return mInsertTopLevelItem( xIndex, xStepID, QString(), xDescIcon ); }

    virtual std::pair< CFlowWidgetItem*, bool > mAddTopLevelItem( CFlowWidgetItem* xItem ) { return mInsertTopLevelItem( -1, xItem ); }
    virtual std::pair< CFlowWidgetItem*, bool > mInsertTopLevelItem( int xIndex, CFlowWidgetItem* xItem );
    virtual std::pair< CFlowWidgetItem*, bool > mInsertTopLevelItem( CFlowWidgetItem* xPeer, CFlowWidgetItem* xItem, bool xBefore );

    virtual CFlowWidgetItem* mAddItem( const QString & xStepID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidgetItem* xParent ) { return mInsertItem( -1, xStepID, xFlowName, xDescIcon, xParent ); }
    virtual CFlowWidgetItem* mAddItem( const QString & xStepID, const QString& xFlowName, CFlowWidgetItem* xParent ) { return mInsertItem( -1, xStepID, xFlowName, QIcon(), xParent ); }
    virtual CFlowWidgetItem* mAddItem( const QString & xStepID, const QIcon& xDescIcon, CFlowWidgetItem* xParent ) { return mInsertItem( -1, xStepID, QString(), xDescIcon, xParent ); }

    virtual CFlowWidgetItem* mInsertItem( int xIndex, const QString & xStepID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidgetItem* xParent );
    virtual CFlowWidgetItem* mInsertItem( int xIndex, const QString & xStepID, const QString& xFlowName, CFlowWidgetItem* xParent ) { return mInsertItem( xIndex, xStepID, xFlowName, QIcon(), xParent ); }
    virtual CFlowWidgetItem* mInsertItem( int xIndex, const QString & xStepID, const QIcon& xDescIcon, CFlowWidgetItem* xParent ) { return mInsertItem( xIndex, xStepID, QString(), xDescIcon, xParent ); }

    virtual int mTopLevelItemCount() const;
    virtual CFlowWidgetItem* mGetTopLevelItem( int xIndex ) const;
    virtual int mIndexOfTopLevelItem( const CFlowWidgetItem* xItem ) const; // returns -1 if not found as a top level widget

    virtual void mRemoveTopLevelItem( int xIndex ); // deletes the CFlowStateDefItem
    virtual CFlowWidgetItem* mTakeTopLevelItem( int xIndex );

    virtual void mClear();
    virtual CFlowWidgetItem* mTakeItem( CFlowWidgetItem* xItem );   // does the work of "get parent, parent->take item
    virtual void mRemoveItem( CFlowWidgetItem* xItem ); // does the work of "get parent, parent->remove item

    virtual void mSelectFlowItem( CFlowWidgetItem* xItem, bool xSelect, bool xExpand );

    virtual CFlowWidgetItem* mSelectedItem() const; // only one item is selectable at a time.

    void mRegisterStateStatus( int xState, const QString & xDescription, const QIcon& xIcon );  // xIcon can be null if so, nothing painted
    int mGetNextStatusID() const;
    QList< std::tuple< int, QString, QIcon > > mGetRegisteredStatuses() const;
    std::pair< QString, QIcon > mGetStateStatus( int xState ) const;

    void mSetElideText( bool xElide );
    bool mElideText() const;

    virtual QString mDump( bool xCompacted ) const;
    virtual void mDump( QJsonObject& xTS ) const;

    void mSetFindIconFunc( const std::function< std::pair< bool, QString >( const QString& fileName ) > & lFindIcon );
protected Q_SLOTS:
    virtual void slotOpenTopLevelItem( int xIndex );
    virtual void slotExpandItem( CFlowWidgetItem* xItem, bool xExpand );
Q_SIGNALS:
    void sigFlowWidgetItemChanged( CFlowWidgetItem * xItem ); // data changed
    void sigFlowWidgetItemInserted( CFlowWidgetItem* xItem );
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
    CFlowWidgetImpl * dImpl{nullptr};
};
Q_DECLARE_METATYPE( CFlowWidget::EStates );

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

    enum ERoles
    {
        eStateStatusRole = Qt::UserRole + 1,  // returns the states
        eStateIconsRole,                       // returns he icons
        eUIClassNameRole,
        eTclProcNameRole
    };

    CFlowWidgetItem();
    CFlowWidgetItem( CFlowWidgetItem * xParent );
    CFlowWidgetItem( CFlowWidget * xParent );

    CFlowWidgetItem( const QString & xStepID, const QString& xFlowName, CFlowWidget* xParent );
    CFlowWidgetItem( const QString& xStepID, const QString& xFlowName, CFlowWidgetItem* xParent );

    CFlowWidgetItem( const QString & xStepID, const QIcon& xDescIcon, CFlowWidget* xParent );
    CFlowWidgetItem( const QString & xStepID, const QIcon& xDescIcon, CFlowWidgetItem* xParent );

    CFlowWidgetItem( const QString & xStepID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidget* xParent );
    CFlowWidgetItem( const QString & xStepID, const QString& xFlowName, const QIcon& xDescIcon, CFlowWidgetItem* xParent );
    ~CFlowWidgetItem();

    void deleteLater();

    CFlowWidgetItem* mParentItem() const;
    CFlowWidget* mGetFlowWidget() const;

    void mAddChild( CFlowWidgetItem* xChild );
    int mInsertChild( int xIndex, CFlowWidgetItem* xChild );
    int mInsertChild( CFlowWidgetItem* xPeer, CFlowWidgetItem* xChild, bool xBefore );

    int mIndexOfChild( const CFlowWidgetItem* xChild );

    int mChildCount() const;
    CFlowWidgetItem* mGetChild( int xIndex ) const;

    CFlowWidgetItem* mTakeChild( CFlowWidgetItem* xItem ); // ownership of the returned item is now the client

    bool mSetData( int xRole, const QVariant& xVariant ); // if the value for the role changes return true
    QVariant mData( int xRole ) const; // for top level items tied to QAbstract Button, for Sub Items tree widget item

    void mSetText( const QString& xText );
    QString mText() const;
    QString mFullText( const QChar& xSeparator = QChar( '.' ) ) const;

    void mSetIcon( const QIcon& xIcon );
    QIcon mIcon() const;

    void mSetStepID( const QString & xStepID );
    QString mStepID() const;

    void mSetToolTip( const QString& xToolTip );
    QString mToolTip() const;

    void mSetUIClassName( const QString& xUIClassName );
    QString mUIClassName() const;

    void mSetTclProcName( const QString& xTclProcName );
    QString mTclProcName() const;

    // order is kept of statuses, if you set via a list and only the order is changed, the item is considered changed
    bool mSetStateStatus( int xStateStatus ); // returns true if state changed if none is sent it, it clears all
    bool mSetStateStatus( const QList< int > & xStateStatuses ); // returns true if state changed, if none is the only item sent in, it clears, if individual it is ignored
    bool mAddStateStatus( int xStateStatus ); // returns true if state add, false if already set, none is sent in, its ignored
    bool mRemoveStateStatus( int xStateStatus ); // returns true if state removed, false if already set
    QList< int > mStateStatuses() const; // guaranteed to never be empty, eNone is always returned if needed

    void mSetHidden( bool xHidden );
    bool mIsHidden() const;

    void mSetVisible( bool xVisible, bool xExpandIfVisible );
    bool mIsVisible() const;

    void mSetDisabled( bool xDisabled );
    bool mIsDisabled() const;

    void mSetEnabled( bool xEnabled );
    bool mIsEnabled() const;

    void mExpandAll();
    void mSetExpanded( bool xExpanded );
    bool mIsExpanded() const;

    void mSetSelected( bool xSelected );
    bool mSelected() const;

    bool mIsTopLevelItem() const;

    int mIndexInParent() const;

    QString mDump( bool xRecursive, bool xCompacted ) const;
    void mDump( QJsonObject& xJSON, bool xRecursive ) const;

private:
    CFlowWidgetItem( const QString & xStepID, const QString& xFlowName, const QIcon& xDescIcon );
    CFlowWidgetItemImpl * dImpl{nullptr};
};
Q_DECLARE_METATYPE( CFlowWidgetItem* );

#endif 
