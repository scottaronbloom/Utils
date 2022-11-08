#ifndef __QTDUMPER_H
#define __QTDUMPER_H

#include "SABUtilsExport.h"

class QWidget;
class QStandardItemModel;
namespace NSABUtils
{
    SABUTILS_EXPORT void dumpWidgetAndChildren( const QWidget * widget, QStandardItemModel * model );
}
#endif
