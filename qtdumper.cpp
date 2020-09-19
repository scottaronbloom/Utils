#include "QtDumper.h"

#include <QTextStream>
#include <QLayout>
#include <QWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QScrollBar>
#include <QScrollArea>
#include <QFormLayout>
#include <QGridLayout>
#include <QStackedLayout>

QString toString( const QSizePolicy::Policy& policy )
{
    switch ( policy )
    {
        case QSizePolicy::Fixed: return "Fixed";
        case QSizePolicy::Minimum: return "Minimum";
        case QSizePolicy::Maximum: return "Maximum";
        case QSizePolicy::Preferred: return "Preferred";
        case QSizePolicy::MinimumExpanding: return "MinimumExpanding";
        case QSizePolicy::Expanding: return "Expanding";
        case QSizePolicy::Ignored: return "Ignored";
    }
    return "unknown";
}

QString toString( const QSizePolicy& policy )
{
    return "(" + toString( policy.horizontalPolicy() ) + ", " + toString( policy.verticalPolicy() ) + ")";
}

QString toString( const QStackedLayout::StackingMode & mode )
{
    switch ( mode )
    {
        case QStackedLayout::StackingMode::StackOne: return "StackOne";
        case QStackedLayout::StackingMode::StackAll: return "StackAll";
    }
    return "unknown";
}

QString toString( QLayout::SizeConstraint constraint )
{
    switch ( constraint )
    {
        case QLayout::SetDefaultConstraint: return "SetDefaultConstraint";
        case QLayout::SetNoConstraint: return "SetNoConstraint";
        case QLayout::SetMinimumSize: return "SetMinimumSize";
        case QLayout::SetFixedSize: return "SetFixedSize";
        case QLayout::SetMaximumSize: return "SetMaximumSize";
        case QLayout::SetMinAndMaxSize: return "SetMinAndMaxSize";
    }
    return "unknown";
}

void dumpClassName( const QMetaObject* metaObject, QStandardItem * parent )
{
    if ( !metaObject )
        return;

    auto item = new QStandardItem( metaObject->className() );
    parent->appendRow( item );
    if ( metaObject->superClass() )
        dumpClassName( metaObject->superClass(), item );
}

QString dumpClassName( const QMetaObject * metaObject, int level )
{
    if ( !metaObject )
        return QString();

    QString retVal;
    QTextStream ts( &retVal );

    ts << QString( 2*level, QChar( ' ' ) ) << "ClsName: " << metaObject->className() << "\n";
    if ( metaObject->superClass() )
        ts << dumpClassName( metaObject->superClass(), level + 1 );

    return retVal;
}

QStandardItem * getWidgetInfo( const QWidget& w, QStandardItem * parent, bool skipScrollBar )
{
    if ( !parent )
        return nullptr;
    if ( skipScrollBar && dynamic_cast< const QScrollBar * >( &w ) )
        return nullptr;

    auto widgetItem = new QStandardItem( "Widget" );
    QString widgetClassName = w.metaObject()->className();
    parent->appendRow( QList< QStandardItem* >() << widgetItem << new QStandardItem( widgetClassName ) );
    dumpClassName( w.metaObject(), widgetItem );

    const QRect& geom = w.geometry();
    QSize hint = w.sizeHint();

    widgetItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Object Name" ) << new QStandardItem( w.objectName().isEmpty() ? "***UNNAMED***" : w.objectName() ) );
    widgetItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Address" ) << new QStandardItem( QString( "0x%1" ).arg( (uintptr_t)&w, QT_POINTER_SIZE * 2, 16, QChar( '0' ) ) ) );
    widgetItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Position" ) << new QStandardItem( QString::number( geom.x() ) ) << new QStandardItem( QString::number( geom.y() ) ) );
    widgetItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Size" ) << new QStandardItem( QString::number( geom.width() ) ) << new QStandardItem( QString::number( geom.height() ) ) );
    widgetItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "SizeHint" ) << new QStandardItem( QString::number( hint.width() ) ) << new QStandardItem( QString::number( hint.height() ) ));
    widgetItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Policy" ) << new QStandardItem( toString( w.sizePolicy() ) ) );
    widgetItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Visible" ) << new QStandardItem( (w.isVisible() ? "" : "**HIDDEN**") ) );
    return widgetItem;
}

QString getWidgetInfo( const QWidget& w, int level )
{
    const QRect& geom = w.geometry();
    QSize hint = w.sizeHint();

    QString retVal;
    QTextStream ts( &retVal );

    ts <<  dumpClassName( w.metaObject(), level )
        << QString( 2*level + 1, QChar( ' ' ) )
        << "objName: " << ( w.objectName().isEmpty() ? "***UNNAMED***" : w.objectName() )
        << QString( ", Address(0x%1)" ).arg( (uintptr_t)&w, QT_POINTER_SIZE * 2, 16, QChar( '0' ) )
        << " , pos(" << geom.x() << ", " << geom.y() << ")"
        << " , size (" << geom.width() << " x " << geom.height() << ")"
        << " , szhint (" << hint.width() << " x " << hint.height() << ")"
        << " , policy " << toString( w.sizePolicy() )
        << " " << ( w.isVisible() ? "" : "**HIDDEN**")
        << "\n"
        ;

    return retVal;
}

QStandardItem * getLayoutItemInfo( QLayoutItem * layoutItem, QStandardItem * parent, bool skipScrollBar )
{
    if ( dynamic_cast<QWidgetItem*>(layoutItem) )
    {
        QWidgetItem* wi = dynamic_cast<QWidgetItem*>(layoutItem);
        if ( wi->widget() )
        {
            return getWidgetInfo( *wi->widget(), parent, skipScrollBar );
        }
    }
    else if ( dynamic_cast<QSpacerItem*>(layoutItem) )
    {
        QSpacerItem* si = dynamic_cast<QSpacerItem*>(layoutItem);
        QSize hint = si->sizeHint();

        parent->appendRow( QList< QStandardItem* >() << new QStandardItem( "SpacerItem Hint" ) << new QStandardItem( QString::number( hint.width() ) ) << new QStandardItem( QString::number( hint.height() ) ) );
        parent->appendRow( QList< QStandardItem* >() << new QStandardItem( "Size Policy" ) << new QStandardItem( toString( si->sizePolicy() ) ) );
        parent->appendRow( QList< QStandardItem* >() << new QStandardItem( "Size Constraint" ) << new QStandardItem( toString( si->layout()->sizeConstraint() ) ) );

        return nullptr;
    }
    return nullptr;
}

QString getLayoutItemInfo( QLayoutItem* item )
{
    if ( dynamic_cast<QWidgetItem*>(item) )
    {
        QWidgetItem* wi = dynamic_cast<QWidgetItem*>(item);
        if ( wi->widget() )
        {
            return getWidgetInfo( *wi->widget(), 0 );
        }

    }
    else if ( dynamic_cast<QSpacerItem*>(item) )
    {
        QSpacerItem* si = dynamic_cast<QSpacerItem*>(item);
        QSize hint = si->sizeHint();
        QString retVal = 
            QString( " SpacerItem hint (%1 x %2) policy: %3 constraint: %4\n" )
            .arg( hint.width() )
            .arg( hint.height() )
            .arg( toString( si->sizePolicy() ) )
            .arg( toString( si->layout()->sizeConstraint() ) )
            ;
        return retVal;
    }
    return "";
}

//------------------------------------------------------------------------
void dumpWidgetAndChildren( QTextStream& os, const QWidget* w, int level )
{
    QString padding = QString( 2 * level, QChar( ' ' ) );

    QLayout* layout = w->layout();
    QList<QWidget*> dumpedChildren;
    if ( layout && layout->isEmpty() == false )
    {
        os << padding << "Layout ";
        QMargins margins = layout->contentsMargins();
        os << " margin: (" << margins.left() << "," << margins.top()
            << "," << margins.right() << "," << margins.bottom() << "), constraint: "
            << toString( layout->sizeConstraint() );

        if ( dynamic_cast<QBoxLayout*>(layout) )
        {
            QBoxLayout* boxLayout = dynamic_cast<QBoxLayout*>(layout);
            os << " spacing: " << boxLayout->spacing();
        }
        os << ":\n";

        int numItems = layout->count();
        for ( int ii = 0; ii < numItems; ii++ )
        {
            QLayoutItem* layoutItem = layout->itemAt( ii );
            QString itemInfo = getLayoutItemInfo( layoutItem );

            os << padding << " " << itemInfo;

            QWidgetItem* wi = dynamic_cast<QWidgetItem*>(layoutItem);
            if ( wi && wi->widget() )
            {
                dumpWidgetAndChildren( os, wi->widget(), level + 1 );
                dumpedChildren.push_back( wi->widget() );
            }
        }
    }

    // now output any child widgets that weren't dumped as part of the layout
    QList<QWidget*> widgets = w->findChildren<QWidget*>( QString(), Qt::FindDirectChildrenOnly );
    QList<QWidget*> undumpedChildren;
    foreach( QWidget * child, widgets )
    {
        if ( dumpedChildren.indexOf( child ) == -1 )
        {
            undumpedChildren.push_back( child );
        }
    }

    if ( undumpedChildren.empty() == false )
    {
        os << padding << " non-layout children:\n";
        foreach( QWidget * child, undumpedChildren )
        {
            dumpWidgetAndChildren( os, child, level + 1 );
        }
    }
}

//------------------------------------------------------------------------
QString dumpWidgetAndChildren( const QWidget* w )
{
    QString retVal;
    QTextStream ts( &retVal );
    if ( w )
    {
        ts << getWidgetInfo( *w, 0 );
        dumpWidgetAndChildren( ts, w, 0 );
    }
    else
        ts << "<nullptr>";
    return retVal;
}

void addItem( QStandardItemModel* model, QStandardItem* parent, QStandardItem* item )
{
    if ( parent )
        parent->appendRow( item );
    else
        model->appendRow( item );
}

QStandardItem * dumpLayoutInfo( QStandardItemModel* model, QStandardItem* parent, QLayout* layout )
{
    if ( !layout || ( !parent && !model ) )
        return nullptr;

    auto layoutItem = new QStandardItem( "Layout" );
    if ( parent )
        parent->appendRow( QList< QStandardItem* >() << layoutItem << new QStandardItem( layout->metaObject()->className() ) );
    else
        model->appendRow( QList< QStandardItem* >() << layoutItem << new QStandardItem( layout->metaObject()->className() ) );


    auto marginItem = new QStandardItem( "Margin" );
    layoutItem->appendRow( marginItem );

    QMargins margins = layout->contentsMargins();
    marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Left" ) << new QStandardItem( QString::number( margins.left() ) ) );
    marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Top" ) << new QStandardItem( QString::number( margins.top() ) ) );
    marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Right" ) << new QStandardItem( QString::number( margins.right() ) ) );
    marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Bottom" ) << new QStandardItem( QString::number( margins.bottom() ) ) );
    marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Size Constraint" ) << new QStandardItem( toString( layout->sizeConstraint() ) ) );

    if ( dynamic_cast<QBoxLayout*>(layout) )
    {
        marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Spacing" ) << new QStandardItem( QString::number( dynamic_cast<QBoxLayout*>(layout)->spacing() ) ) );
    }
    else if ( dynamic_cast< QFormLayout * >( layout ) )
    {
        marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "HSpacing" ) << new QStandardItem( QString::number( dynamic_cast<QFormLayout*>(layout)->horizontalSpacing() ) ) );
        marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "VSpacing" ) << new QStandardItem( QString::number( dynamic_cast<QFormLayout*>(layout)->verticalSpacing() ) ) );
    }
    else if ( dynamic_cast<QGridLayout*>(layout) )
    {
        marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "HSpacing" ) << new QStandardItem( QString::number( dynamic_cast<QGridLayout*>(layout)->horizontalSpacing() ) ) );
        marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "VSpacing" ) << new QStandardItem( QString::number( dynamic_cast<QGridLayout*>(layout)->verticalSpacing() ) ) );
    }
    else if ( dynamic_cast<QStackedLayout*>(layout) )
    {
        marginItem->appendRow( QList< QStandardItem* >() << new QStandardItem( "Stacking Mode" ) << new QStandardItem( toString( dynamic_cast<QStackedLayout*>(layout)->stackingMode() ) ) );
    }
    return layoutItem;
}

void dumpWidgetAndChildren( const QWidget* widget, QStandardItemModel* model, QStandardItem* parent, bool skipScrollBar )
{
    Q_ASSERT( parent || ( !parent && model ) );

    QList<QWidget*> allWidgets = widget->findChildren<QWidget*>();
    bool foundNonScrollBar = false;
    if ( skipScrollBar )
    {
        for ( auto ii : allWidgets )
        {
            if ( dynamic_cast<QScrollBar*>(ii) )
                continue;
            foundNonScrollBar = true;
            break;
        }
    }
    else
        foundNonScrollBar = true;
    if ( !foundNonScrollBar )
        return;

    QLayout* layout = widget->layout();
    QList<QWidget*> dumpedChildren;
    //QStandardItem* childItems = new QStandardItem( "Children" );
    //addItem( model, parent, childItems );

    auto widgets = widget->findChildren<QWidget*>( QString(), Qt::FindDirectChildrenOnly );

    if ( layout && !layout->isEmpty() )
    {
        auto layoutInfoItem = dumpLayoutInfo( model, parent, layout );

        int numItems = layout->count();
        if ( numItems )
        {
            for ( int ii = 0; ii < numItems; ii++ )
            {
                QLayoutItem* layoutItem = layout->itemAt( ii );
                auto layoutWidgetItem = getLayoutItemInfo( layoutItem, layoutInfoItem, skipScrollBar );

                QWidgetItem* wi = dynamic_cast<QWidgetItem*>(layoutItem);
                if ( wi && wi->widget() )
                {
                    dumpedChildren.push_back( wi->widget() );
                    auto subParent = layoutWidgetItem ? layoutWidgetItem : parent;
                    if ( dynamic_cast<const QScrollArea*>(wi->widget()) ) // is a container and needs to be treated AS a layed out item
                    {
                        auto scrollAreaItem = new QStandardItem( "QScrollArea" );
                        if ( subParent )
                            subParent->appendRow( QList< QStandardItem* >() << scrollAreaItem << new QStandardItem( widget->objectName() ) );
                        else 
                            model->appendRow( QList< QStandardItem* >() << scrollAreaItem << new QStandardItem( widget->objectName() ) );

                        auto scrollArea = dynamic_cast<const QScrollArea*>(wi->widget());
                        dumpedChildren.push_back( wi->widget() );
                        dumpWidgetAndChildren( scrollArea->widget(), model, subParent, skipScrollBar );
                    }
                    else
                    {
                        dumpWidgetAndChildren( wi->widget(), model, subParent, skipScrollBar );
                    }
                }
            }
        }
    }
    else if ( !widgets.isEmpty() )
    {
        QStandardItem* childItems = new QStandardItem( "Children" );
        addItem( model, parent, childItems );
        getWidgetInfo( *widget, childItems, skipScrollBar );
    }

    //// now output any child widgets that weren't dumped as part of the layout
    //QList<QWidget*> undumpedChildren;
    //QList<QWidget*> widgets = widget->findChildren<QWidget*>( QString(), Qt::FindDirectChildrenOnly );
    //foreach( QWidget * child, widgets )
    //{
    //    if ( dumpedChildren.indexOf( child ) == -1 )
    //    {
    //        undumpedChildren.push_back( child );
    //    }
    //}
    //if ( undumpedChildren.empty() == false )
    //{
    //    auto nonLayoutChildren = new QStandardItem( "Non-Layedout Children" );

    //    foreach( QWidget * child, undumpedChildren )
    //    {
    //        dumpWidgetAndChildren( child, model, nonLayoutChildren, true );
    //    }
    //    if ( nonLayoutChildren->rowCount() )
    //        addItem( model, parent, nonLayoutChildren );
    //    else
    //        delete nonLayoutChildren;
    //}
}

void dumpWidgetAndChildren( const QWidget* widget, QStandardItemModel* model )
{
    model->clear();
    model->setColumnCount( 2 );
    model->setHeaderData( 0, Qt::Horizontal, "Item Name" );
    model->setHeaderData( 1, Qt::Horizontal, "Value" );
    dumpWidgetAndChildren( widget, model, nullptr, false );
}

