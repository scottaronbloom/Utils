#ifndef __QTDUMPER_H
#define __QTDUMPER_H

class QWidget;
class QStandardItemModel;

void    dumpWidgetAndChildren( const QWidget* widget, QStandardItemModel* model );
#endif
