#ifndef __QTDUMPER_H
#define __QTDUMPER_H

#include <QString>
class QWidget;
class QStandardItemModel;

QString dumpWidgetAndChildren(const QWidget* w); // calls above function with a new text stream, on w at level 0
void    dumpWidgetAndChildren( const QWidget* widget, QStandardItemModel* model );
#endif
