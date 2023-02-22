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

#ifndef __UTILITYVIEWS_H
#define __UTILITYVIEWS_H

#include "SABUtilsExport.h"

#include <QTableWidget>
#include <QTableView>
#include <QItemDelegate>
#include <unordered_set>

class QAbstractButton;

namespace NSABUtils
{
    class SABUTILS_EXPORT CTableWidgetWithSelectCommand : public QTableWidget
    {
        Q_OBJECT;

    public:
        explicit CTableWidgetWithSelectCommand( QWidget *parent );
        virtual QItemSelectionModel::SelectionFlags selectionCommand( const QModelIndex &index, const QEvent *event = nullptr ) const override;
    };

    class SABUTILS_EXPORT CTableViewWithSelectCommand : public QTableView
    {
        Q_OBJECT;

    public:
        explicit CTableViewWithSelectCommand( QWidget *parent );
        virtual QItemSelectionModel::SelectionFlags selectionCommand( const QModelIndex &index, const QEvent *event = nullptr ) const override;
    };

    class SABUTILS_EXPORT CNoEditDelegate : public QItemDelegate
    {
        Q_OBJECT

    public:
        CNoEditDelegate( std::unordered_set< int > &noEditColumns, QObject *parent = nullptr );
        CNoEditDelegate( QObject *parent = nullptr );
        virtual QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    private:
        std::unordered_set< int > fNoEditColumns;
    };
}

#endif
