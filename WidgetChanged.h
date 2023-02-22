// The MIT License( MIT )
//
// Copyright( c ) 2022 Scott Aron Bloom
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

#include "SABUtilsExport.h"

class QWidget;
class QObject;
class QMetaMethod;
class QAbstractItemModel;

#include <set>
namespace NSABUtils
{
    // for a given model, when it changes at all call/connect to the given member on the reciever object
    // if excluded then it only disconnects, and doesnt connect to the model and reciever
    SABUTILS_EXPORT void setupModelChanged( const QAbstractItemModel *model, const QObject *reciever, const char *member, bool isExcluded = false );
    SABUTILS_EXPORT void setupModelChanged( const QAbstractItemModel *model, const QObject *reciever, const QMetaMethod &member, bool isExcluded = false );

    // for a given parent widget call/connect to the given member on the parent widget for all the child widgets of the parent
    // if the child widget is in the excluded list OR its parent is in the excluded list, then it is not connected
    // if exclude all is set, then it disconnects from all previous connections
    SABUTILS_EXPORT void setupWidgetChanged( const QWidget *parentWidget, const char *member, const std::set< QWidget * > &excluded = {}, bool excludeAll = false );
    SABUTILS_EXPORT void setupWidgetChanged( const QWidget *parentWidget, const QMetaMethod &member, const std::set< QWidget * > &excluded = {}, bool excludeAll = false );
}
