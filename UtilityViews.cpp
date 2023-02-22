// The MIT License( MIT )
//
// Copyright( c ) 2020-2021 Scott Aron Bloom
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

#include "UtilityViews.h"

#include <QFileInfo>
#include <QKeyEvent>
#include <QLineEdit>
#include <QAbstractButton>
#include <QTimer>

namespace NSABUtils
{
    CTableWidgetWithSelectCommand::CTableWidgetWithSelectCommand(QWidget* parent) :
        QTableWidget(parent)
    {
    }

    QItemSelectionModel::SelectionFlags CTableWidgetWithSelectCommand::selectionCommand(const QModelIndex& index, const QEvent* event /*= 0*/) const
    {
        return QTableWidget::selectionCommand(index, event);
    }


    CTableViewWithSelectCommand::CTableViewWithSelectCommand(QWidget* parent) :
        QTableView(parent)
    {
    }

    QItemSelectionModel::SelectionFlags CTableViewWithSelectCommand::selectionCommand(const QModelIndex& index, const QEvent* event /*= 0*/) const
    {
        return QTableView::selectionCommand(index, event);
    }

    CNoEditDelegate::CNoEditDelegate(QObject* parent) :
        QItemDelegate(parent)
    {
    }

    CNoEditDelegate::CNoEditDelegate(std::unordered_set< int >& noEditColumns, QObject* parent) :
        QItemDelegate(parent)
    {
        fNoEditColumns = noEditColumns;
    }

    QWidget* CNoEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if (fNoEditColumns.empty() || (fNoEditColumns.find(index.column()) != fNoEditColumns.end()))
            return nullptr;
        return QItemDelegate::createEditor(parent, option, index);
    }

}
