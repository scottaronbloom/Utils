#include "../FlowWidget.h"
#include <QApplication>
#include <QtTest>
inline void initResources() { Q_INIT_RESOURCE( application ); }

#ifdef QCOMPARE
#undef QCOMPARE
#endif

#define QCOMPARE(actual, expected) \
do {\
    QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__);\
    break;\
} while (false)

class CTestFlowWidget : public QObject
{
    Q_OBJECT;
private Q_SLOTS:
    void initTestCase()
    {
        initResources();
    }

    void init()
    {
        dFlowWidget = new CFlowWidget;

        int stateID = 0;

        dFlowItem1 = dFlowWidget->mAddTopLevelItem( stateID++, QIcon( ":/Entity.png" ) );
        dFlowItem1->mSetText( "FlowItem 1" );

        dFlowItem11 = new CFlowWidgetItem( stateID++, QIcon( ":/Entity.png" ), dFlowItem1 );
        dFlowItem11->mSetText( "SubFlowItem 1-1" );
        dFlowItem112 = new CFlowWidgetItem( stateID++, "SubFlowItem 1-1-2", QIcon( ":/Entity.png" ), dFlowItem11 );
        dFlowItem12 = new CFlowWidgetItem( stateID++, "SubFlowItem 1-2", dFlowItem1 );

        dFlowItem2 = new CFlowWidgetItem( stateID++, QIcon( ":/Entity.png" ), dFlowWidget );
        dFlowItem2->mSetText( "FlowItem 2" );
        dFlowItem21 = new CFlowWidgetItem( stateID++, "SubFlowItem 2-1", dFlowItem2 );
        dFlowItem22 = dFlowWidget->mAddItem( stateID++, "SubFlowItem 2-2", dFlowItem2 );
        dFlowItem221 = new CFlowWidgetItem( stateID++, "SubFlowItem 2-2-1", QIcon( ":/Entity.png" ), dFlowItem22 );
        dFlowItem222 = new CFlowWidgetItem( stateID++, "SubFlowItem 2-2-2", dFlowItem22 );
        dFlowItem222->mSetIcon( QIcon( ":/Entity.png" ) );

        dFlowItem3 = new CFlowWidgetItem( stateID++, "FlowItem 3", dFlowWidget );
        dFlowItem31 = new CFlowWidgetItem( stateID++, "SubFlowItem 3-1", dFlowItem3 );
        dFlowItem32 = new CFlowWidgetItem( stateID++, "SubFlowItem 3-2", dFlowItem3 );
        dFlowItem321 = new CFlowWidgetItem( stateID++, "SubFlowItem 3-2-1", QIcon( ":/Entity.png" ), dFlowItem32 );
    }

    void cleanupTestCase()
    {
    }
        
    void cleanup()
    {
        delete dFlowWidget;
    }

    void testIcons()
    {
        QVERIFY( !dFlowItem1->mIcon().isNull() );
        QVERIFY( !dFlowItem11->mIcon().isNull() );
        QVERIFY( !dFlowItem112->mIcon().isNull() );
        QVERIFY( dFlowItem12->mIcon().isNull() );

        QVERIFY( !dFlowItem2->mIcon().isNull() );
        QVERIFY( dFlowItem21->mIcon().isNull() );
        QVERIFY( dFlowItem22->mIcon().isNull() );
        QVERIFY( !dFlowItem221->mIcon().isNull() );
        QVERIFY( !dFlowItem222->mIcon().isNull() );

        QVERIFY( dFlowItem3->mIcon().isNull() );
        QVERIFY( dFlowItem31->mIcon().isNull() );
        QVERIFY( dFlowItem32->mIcon().isNull() );
        QVERIFY( !dFlowItem321->mIcon().isNull() );
    }

    void testText()
    {
        QCOMPARE( dFlowItem1->mText(), "FlowItem 1" );
        QCOMPARE( dFlowItem11->mText(), "SubFlowItem 1-1" );
        QCOMPARE( dFlowItem112->mText(), "SubFlowItem 1-1-2" );
        QCOMPARE( dFlowItem12->mText(), "SubFlowItem 1-2" );

        QCOMPARE( dFlowItem2->mText(), "FlowItem 2" );
        QCOMPARE( dFlowItem21->mText(), "SubFlowItem 2-1" );
        QCOMPARE( dFlowItem22->mText(), "SubFlowItem 2-2" );
        QCOMPARE( dFlowItem221->mText(), "SubFlowItem 2-2-1" );
        QCOMPARE( dFlowItem222->mText(), "SubFlowItem 2-2-2" );

        QCOMPARE( dFlowItem3->mText(), "FlowItem 3" );
        QCOMPARE( dFlowItem31->mText(), "SubFlowItem 3-1" );
        QCOMPARE( dFlowItem32->mText(), "SubFlowItem 3-2" );
        QCOMPARE( dFlowItem321->mText(), "SubFlowItem 3-2-1" );
    }

    void testFullText()
    {
        QCOMPARE( dFlowItem1->mFullText(), "FlowItem 1" );
        QCOMPARE( dFlowItem11->mFullText(), "FlowItem 1.SubFlowItem 1-1" );
        QCOMPARE( dFlowItem112->mFullText(), "FlowItem 1.SubFlowItem 1-1.SubFlowItem 1-1-2" );
        QCOMPARE( dFlowItem12->mFullText(), "FlowItem 1.SubFlowItem 1-2" );

        QCOMPARE( dFlowItem2->mFullText(), "FlowItem 2" );
        QCOMPARE( dFlowItem21->mFullText(), "FlowItem 2.SubFlowItem 2-1" );
        QCOMPARE( dFlowItem22->mFullText(), "FlowItem 2.SubFlowItem 2-2" );
        QCOMPARE( dFlowItem221->mFullText(), "FlowItem 2.SubFlowItem 2-2.SubFlowItem 2-2-1" );
        QCOMPARE( dFlowItem222->mFullText(), "FlowItem 2.SubFlowItem 2-2.SubFlowItem 2-2-2" );

        QCOMPARE( dFlowItem3->mFullText(), "FlowItem 3" );
        QCOMPARE( dFlowItem31->mFullText(), "FlowItem 3.SubFlowItem 3-1" );
        QCOMPARE( dFlowItem32->mFullText(), "FlowItem 3.SubFlowItem 3-2" );
        QCOMPARE( dFlowItem321->mFullText(), "FlowItem 3.SubFlowItem 3-2.SubFlowItem 3-2-1" );
    }

    void testToolTip()
    {
        dFlowItem1->mSetToolTip( "ToolTip - FlowItem 1" );
        dFlowItem11->mSetToolTip( "ToolTip - SubFlowItem 1-1" );
        dFlowItem112->mSetToolTip( "ToolTip - SubFlowItem 1-1-2" );
        dFlowItem12->mSetToolTip( "ToolTip - SubFlowItem 1-2" );

        dFlowItem2->mSetToolTip( "ToolTip - FlowItem 2" );
        dFlowItem21->mSetToolTip( "ToolTip - SubFlowItem 2-1" );
        dFlowItem22->mSetToolTip( "ToolTip - SubFlowItem 2-2" );
        dFlowItem221->mSetToolTip( "ToolTip - SubFlowItem 2-2-1" );
        dFlowItem222->mSetToolTip( "ToolTip - SubFlowItem 2-2-2" );

        dFlowItem3->mSetToolTip( "ToolTip - FlowItem 3" );
        dFlowItem31->mSetToolTip( "ToolTip - SubFlowItem 3-1" );
        dFlowItem32->mSetToolTip( "ToolTip - SubFlowItem 3-2" );
        dFlowItem321->mSetToolTip( "ToolTip - SubFlowItem 3-2-1" );

        QVERIFY( dFlowItem1->mToolTip() == "ToolTip - FlowItem 1" );
        QVERIFY( dFlowItem11->mToolTip() == "ToolTip - SubFlowItem 1-1" );
        QVERIFY( dFlowItem112->mToolTip() == "ToolTip - SubFlowItem 1-1-2" );
        QVERIFY( dFlowItem12->mToolTip() == "ToolTip - SubFlowItem 1-2" );

        QVERIFY( dFlowItem2->mToolTip() == "ToolTip - FlowItem 2" );
        QVERIFY( dFlowItem21->mToolTip() == "ToolTip - SubFlowItem 2-1" );
        QVERIFY( dFlowItem22->mToolTip() == "ToolTip - SubFlowItem 2-2" );
        QVERIFY( dFlowItem221->mToolTip() == "ToolTip - SubFlowItem 2-2-1" );
        QVERIFY( dFlowItem222->mToolTip() == "ToolTip - SubFlowItem 2-2-2" );

        QVERIFY( dFlowItem3->mToolTip() == "ToolTip - FlowItem 3" );
        QVERIFY( dFlowItem31->mToolTip() == "ToolTip - SubFlowItem 3-1" );
        QVERIFY( dFlowItem32->mToolTip() == "ToolTip - SubFlowItem 3-2" );
        QVERIFY( dFlowItem321->mToolTip() == "ToolTip - SubFlowItem 3-2-1" );
    }

    void testParent()
    {
        QCOMPARE( dFlowItem1->mParentItem(), nullptr );
        QCOMPARE( dFlowItem11->mParentItem(), dFlowItem1 );
        QCOMPARE( dFlowItem112->mParentItem(), dFlowItem11 );
        QCOMPARE( dFlowItem12->mParentItem(), dFlowItem1 );

        QCOMPARE( dFlowItem2->mParentItem(), nullptr );
        QCOMPARE( dFlowItem21->mParentItem(), dFlowItem2 );
        QCOMPARE( dFlowItem22->mParentItem(), dFlowItem2 );
        QCOMPARE( dFlowItem221->mParentItem(), dFlowItem22 );
        QCOMPARE( dFlowItem222->mParentItem(), dFlowItem22 );

        QCOMPARE( dFlowItem3->mParentItem(), nullptr );
        QCOMPARE( dFlowItem31->mParentItem(), dFlowItem3 );
        QCOMPARE( dFlowItem32->mParentItem(), dFlowItem3 );
        QCOMPARE( dFlowItem321->mParentItem(), dFlowItem32 );
    }

    void testFlowWidget()
    {
        QCOMPARE( dFlowItem1->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem11->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem112->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem12->mGetFlowWidget(), dFlowWidget );

        QCOMPARE( dFlowItem2->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem21->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem22->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem221->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem222->mGetFlowWidget(), dFlowWidget );

        QCOMPARE( dFlowItem3->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem31->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem32->mGetFlowWidget(), dFlowWidget );
        QCOMPARE( dFlowItem321->mGetFlowWidget(), dFlowWidget );
    }

    void testIndexOfChild()
    {
        QCOMPARE( dFlowWidget->mIndexOfTopLevelItem( dFlowItem1 ), 0 );
        QCOMPARE( dFlowWidget->mIndexOfTopLevelItem( dFlowItem2 ), 1 );
        QCOMPARE( dFlowWidget->mIndexOfTopLevelItem( dFlowItem3 ), 2 );

        QCOMPARE( dFlowWidget->mIndexOfTopLevelItem( dFlowItem11 ), -1 );
        QCOMPARE( dFlowWidget->mIndexOfTopLevelItem( nullptr ), -1 );

        QCOMPARE( dFlowItem1->mIndexOfChild( dFlowItem11 ), 0 );
        QCOMPARE( dFlowItem1->mIndexOfChild( dFlowItem112 ), -1 );
        QCOMPARE( dFlowItem11->mIndexOfChild( dFlowItem112 ), 0 );
        QCOMPARE( dFlowItem1->mIndexOfChild( dFlowItem12 ), 1 );
        QCOMPARE( dFlowItem1->mIndexOfChild( dFlowItem2 ), -1 );

        QCOMPARE( dFlowItem2->mIndexOfChild( dFlowItem21 ), 0 );
        QCOMPARE( dFlowItem2->mIndexOfChild( dFlowItem22 ), 1 );
        QCOMPARE( dFlowItem2->mIndexOfChild( dFlowItem221 ), -1 );
        QCOMPARE( dFlowItem22->mIndexOfChild( dFlowItem221 ), 0 );
        QCOMPARE( dFlowItem22->mIndexOfChild( dFlowItem222 ), 1 );
        QCOMPARE( dFlowItem2->mIndexOfChild( dFlowItem1 ), -1 );

        QCOMPARE( dFlowItem3->mIndexOfChild( dFlowItem31 ), 0 );
        QCOMPARE( dFlowItem3->mIndexOfChild( dFlowItem32 ), 1 );
        QCOMPARE( dFlowItem3->mIndexOfChild( dFlowItem221 ), -1 );
        QCOMPARE( dFlowItem32->mIndexOfChild( dFlowItem321 ), 0 );
        QCOMPARE( dFlowItem3->mIndexOfChild( dFlowItem1 ), -1 );
    }

    void testChildCount()
    {
        QCOMPARE( dFlowWidget->mTopLevelItemCount(), 3 );

        QCOMPARE( dFlowItem1->mChildCount(), 2 );
        QCOMPARE( dFlowItem11->mChildCount(), 1 );
        QCOMPARE( dFlowItem112->mChildCount(), 0 );
        QCOMPARE( dFlowItem12->mChildCount(), 0 );

        QCOMPARE( dFlowItem2->mChildCount(), 2 );
        QCOMPARE( dFlowItem21->mChildCount(), 0 );
        QCOMPARE( dFlowItem22->mChildCount(), 2 );
        QCOMPARE( dFlowItem221->mChildCount(), 0 );
        QCOMPARE( dFlowItem222->mChildCount(), 0 );

        QCOMPARE( dFlowItem3->mChildCount(), 2 );
        QCOMPARE( dFlowItem31->mChildCount(), 0 );
        QCOMPARE( dFlowItem32->mChildCount(), 1 );
        QCOMPARE( dFlowItem321->mChildCount(), 0 );
    }

    void testGetChild()
    {
        QCOMPARE( dFlowWidget->mGetTopLevelItem( 0 ), dFlowItem1 );
        QCOMPARE( dFlowWidget->mGetTopLevelItem( 1 ), dFlowItem2 );
        QCOMPARE( dFlowWidget->mGetTopLevelItem( 2 ), dFlowItem3 );
        QCOMPARE( dFlowWidget->mGetTopLevelItem( 3 ), nullptr );
        QCOMPARE( dFlowWidget->mGetTopLevelItem( -1 ), nullptr );

        QCOMPARE( dFlowItem1->mGetChild( 0 ), dFlowItem11 );
        QCOMPARE( dFlowItem1->mGetChild( 1 ), dFlowItem12 );
        QCOMPARE( dFlowItem11->mGetChild( 0 ), dFlowItem112 );
        QCOMPARE( dFlowItem112->mGetChild( 0 ), nullptr );
        QCOMPARE( dFlowItem12->mGetChild( -1 ), nullptr );

        QCOMPARE( dFlowItem2->mGetChild( 0 ), dFlowItem21 );
        QCOMPARE( dFlowItem2->mGetChild( 1 ), dFlowItem22 );
        QCOMPARE( dFlowItem22->mGetChild( 0 ), dFlowItem221 );
        QCOMPARE( dFlowItem22->mGetChild( 1 ), dFlowItem222 );
        QCOMPARE( dFlowItem221->mGetChild( 0 ), nullptr );
        QCOMPARE( dFlowItem222->mGetChild( 0 ), nullptr );
        QCOMPARE( dFlowItem21->mGetChild( -1 ), nullptr );

        QCOMPARE( dFlowItem3->mGetChild( 0 ), dFlowItem31 );
        QCOMPARE( dFlowItem3->mGetChild( 1 ), dFlowItem32 );
        QCOMPARE( dFlowItem32->mGetChild( 0 ), dFlowItem321 );
        QCOMPARE( dFlowItem321->mGetChild( 0 ), nullptr );
        QCOMPARE( dFlowItem321->mGetChild( -1 ), nullptr );
    }

    void testSetDisabled()
    {
        dFlowItem1->mSetDisabled( true );
        dFlowItem11->mSetDisabled( true );

        QVERIFY( dFlowItem1->mIsDisabled() );
        QVERIFY( dFlowItem11->mIsDisabled() );

        QVERIFY( !dFlowItem1->mIsEnabled() );
        QVERIFY( !dFlowItem11->mIsEnabled() );

        QCOMPARE( (dFlowItem1->mStateStatus() & CFlowWidgetItem::EStates::eDisabled), CFlowWidgetItem::EStates::eDisabled );
        QCOMPARE( (dFlowItem11->mStateStatus() & CFlowWidgetItem::EStates::eDisabled), CFlowWidgetItem::EStates::eDisabled );

        dFlowItem1->mSetEnabled( false );
        dFlowItem11->mSetEnabled( false );

        QVERIFY( dFlowItem1->mIsDisabled() );
        QVERIFY( dFlowItem11->mIsDisabled() );

        QVERIFY( !dFlowItem1->mIsEnabled() );
        QVERIFY( !dFlowItem11->mIsEnabled() );

        QCOMPARE( (dFlowItem1->mStateStatus() & CFlowWidgetItem::EStates::eDisabled), CFlowWidgetItem::EStates::eDisabled );
        QCOMPARE( (dFlowItem11->mStateStatus() & CFlowWidgetItem::EStates::eDisabled), CFlowWidgetItem::EStates::eDisabled );

        dFlowItem1->mSetEnabled( true );
        dFlowItem11->mSetEnabled( true );

        QVERIFY( !dFlowItem1->mIsDisabled() );
        QVERIFY( !dFlowItem11->mIsDisabled() );

        QVERIFY( dFlowItem1->mIsEnabled() );
        QVERIFY( dFlowItem11->mIsEnabled() );

        QCOMPARE( (dFlowItem1->mStateStatus() & CFlowWidgetItem::EStates::eDisabled), 0 );
        QCOMPARE( (dFlowItem11->mStateStatus() & CFlowWidgetItem::EStates::eDisabled), 0 );

        dFlowItem1->mSetDisabled( false );
        dFlowItem11->mSetDisabled( false );

        QVERIFY( !dFlowItem1->mIsDisabled() );
        QVERIFY( !dFlowItem11->mIsDisabled() );

        QVERIFY( dFlowItem1->mIsEnabled() );
        QVERIFY( dFlowItem11->mIsEnabled() );

        QCOMPARE( (dFlowItem1->mStateStatus() & CFlowWidgetItem::EStates::eDisabled), 0 );
        QCOMPARE( (dFlowItem11->mStateStatus() & CFlowWidgetItem::EStates::eDisabled), 0 );
    }

    void testSetGetData()
    {
        dFlowWidget->mGetTopLevelItem( 0 )->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 1 );
        dFlowWidget->mGetTopLevelItem( 1 )->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 2 );
        dFlowWidget->mGetTopLevelItem( 2 )->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 3 );

        dFlowItem11->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 11 );
        dFlowItem112->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 112 );
        dFlowItem12->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 12 );

        dFlowItem21->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 21 );
        dFlowItem22->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 22 );
        dFlowItem221->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 221 );
        dFlowItem222->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 222 );

        dFlowItem31->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 31 );
        dFlowItem32->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 32 );
        dFlowItem321->mSetData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole, 321 );

        QCOMPARE( dFlowItem1->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 1 );
        QCOMPARE( dFlowItem2->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 2 );
        QCOMPARE( dFlowItem3->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 3 );

        QCOMPARE( dFlowItem11->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 11 );
        QCOMPARE( dFlowItem112->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 112 );
        QCOMPARE( dFlowItem12->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 12 );

        QCOMPARE( dFlowItem21->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 21 );
        QCOMPARE( dFlowItem22->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 22 );
        QCOMPARE( dFlowItem221->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 221 );
        QCOMPARE( dFlowItem222->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 222 );

        QCOMPARE( dFlowItem31->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 31 );
        QCOMPARE( dFlowItem32->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 32 );
        QCOMPARE( dFlowItem321->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole ).toInt(), 321 );

        QCOMPARE( dFlowItem1->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole + 1 ).toInt(), 0 );
        QCOMPARE( dFlowItem31->mData( CFlowWidgetItem::EFlowItemDataRole::eStateStatusRole + 1 ).toInt(), 0 );
    }

    void testSetGetStateStatus()
    {
        dFlowWidget->mGetTopLevelItem( 0 )->mSetStateStatus( CFlowWidgetItem::EStates::eDisabled );
        dFlowWidget->mGetTopLevelItem( 1 )->mSetStateStatus( CFlowWidgetItem::EStates::eReady );
        dFlowWidget->mGetTopLevelItem( 2 )->mSetStateStatus( CFlowWidgetItem::EStates::eError );

        dFlowItem11->mSetStateStatus( CFlowWidgetItem::EStates::eWarning );
        dFlowItem112->mSetStateStatus( CFlowWidgetItem::EStates::eInfo );
        dFlowItem12->mSetStateStatus( CFlowWidgetItem::EStates::eDisabled );

        dFlowItem21->mSetStateStatus( CFlowWidgetItem::EStates::eReady );
        dFlowItem22->mSetStateStatus( CFlowWidgetItem::EStates::eError );
        dFlowItem221->mSetStateStatus( CFlowWidgetItem::EStates::eWarning );
        dFlowItem222->mSetStateStatus( CFlowWidgetItem::EStates::eInfo );

        dFlowItem31->mSetStateStatus( CFlowWidgetItem::EStates::eDisabled );
        dFlowItem32->mSetStateStatus( CFlowWidgetItem::EStates::eReady );
        dFlowItem321->mSetStateStatus( CFlowWidgetItem::EStates::eError );

        QCOMPARE( dFlowItem1->mStateStatus(), CFlowWidgetItem::EStates::eDisabled );
        QCOMPARE( dFlowItem2->mStateStatus(), CFlowWidgetItem::EStates::eReady );
        QCOMPARE( dFlowItem3->mStateStatus(), CFlowWidgetItem::EStates::eError );

        QCOMPARE( dFlowItem11->mStateStatus(), CFlowWidgetItem::EStates::eWarning );
        QCOMPARE( dFlowItem112->mStateStatus(), CFlowWidgetItem::EStates::eInfo );
        QCOMPARE( dFlowItem12->mStateStatus(), CFlowWidgetItem::EStates::eDisabled );

        QCOMPARE( dFlowItem21->mStateStatus(), CFlowWidgetItem::EStates::eReady );
        QCOMPARE( dFlowItem22->mStateStatus(), CFlowWidgetItem::EStates::eError );
        QCOMPARE( dFlowItem221->mStateStatus(), CFlowWidgetItem::EStates::eWarning );
        QCOMPARE( dFlowItem222->mStateStatus(), CFlowWidgetItem::EStates::eInfo );

        QCOMPARE( dFlowItem31->mStateStatus(), CFlowWidgetItem::EStates::eDisabled );
        QCOMPARE( dFlowItem32->mStateStatus(), CFlowWidgetItem::EStates::eReady );
        QCOMPARE( dFlowItem321->mStateStatus(), CFlowWidgetItem::EStates::eError );

        QVERIFY( dFlowItem1->mIsDisabled() );
        QVERIFY( !dFlowItem2->mIsDisabled() );
        QVERIFY( !dFlowItem3->mIsDisabled() );

        QVERIFY( !dFlowItem11->mIsDisabled() );
        QVERIFY( !dFlowItem112->mIsDisabled() );
        QVERIFY( dFlowItem12->mIsDisabled() );

        QVERIFY( !dFlowItem21->mIsDisabled() );
        QVERIFY( !dFlowItem22->mIsDisabled() );
        QVERIFY( !dFlowItem221->mIsDisabled() );
        QVERIFY( !dFlowItem222->mIsDisabled() );

        QVERIFY( dFlowItem31->mIsDisabled() );
        QVERIFY( !dFlowItem32->mIsDisabled() );
        QVERIFY( !dFlowItem321->mIsDisabled() );
    }
private:
    CFlowWidget* dFlowWidget{ nullptr };

    CFlowWidgetItem* dFlowItem1{ nullptr };
    CFlowWidgetItem* dFlowItem11{ nullptr };
    CFlowWidgetItem* dFlowItem112{ nullptr };
    CFlowWidgetItem* dFlowItem12{ nullptr };

    CFlowWidgetItem* dFlowItem2{ nullptr };
    CFlowWidgetItem* dFlowItem21{ nullptr };
    CFlowWidgetItem* dFlowItem22{ nullptr };
    CFlowWidgetItem* dFlowItem221{ nullptr };
    CFlowWidgetItem* dFlowItem222{ nullptr };

    CFlowWidgetItem* dFlowItem3{ nullptr };
    CFlowWidgetItem* dFlowItem31{ nullptr };
    CFlowWidgetItem* dFlowItem32{ nullptr };
    CFlowWidgetItem* dFlowItem321{ nullptr };
};

QTEST_MAIN( CTestFlowWidget );
#include "TestFlowWidget.moc"

