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


#ifndef __BUTTONENABLER_H
#define __BUTTONENABLER_H

#include <QObject>
class QAbstractItemView;
class QAbstractButton;
class QItemSelection;
class QLineEdit;

class CButtonEnabler : public QObject
{
Q_OBJECT;
public:
    CButtonEnabler( QAbstractItemView * view, QAbstractButton * btn, QObject * parent=nullptr );
    CButtonEnabler( QLineEdit * le, QAbstractButton * btn, QObject * parent=nullptr );

    void setLineEditIsFile(){ fLineEditIsFile = true; };
public slots:
    void slotReset();

private slots:
    void slotSelectionChanged( const QItemSelection  & selected, const QItemSelection  & );
    void slotTextChanged( const QString & changed );
private:
    QAbstractButton * fButton;
    bool fLineEditIsFile{ true };
};


#endif