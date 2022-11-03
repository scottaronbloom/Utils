#ifndef __DELAYCOMBOBOX_H
#define __DELAYCOMBOBOX_H
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

#include "SABUtilsExport.h"

#include <QComboBox>
namespace NSABUtils
{
    class CDelayLineEdit;
    class SABUTILS_EXPORT CDelayComboBox : public QComboBox
    {
        Q_OBJECT;
    Q_SIGNALS:
        void sigEditTextChangedAfterDelay(const QString &text);
    public:
        CDelayComboBox(QWidget * parent = nullptr);

        CDelayLineEdit * lineEdit() const;
        void setDelay(int delayMS);
        void setIsOKFunction(std::function< bool(const QString &text) > func, const QString &errorMsg = {});
        bool isOK() const;

        void setDelayLineEdit( CDelayLineEdit * le );
        QStringList getAllText() const;
        inline void addItems(const QStringList &texts, bool selectFirstItem = false)
        {
            insertItems(count(), texts);
            if (selectFirstItem)
                setCurrentIndex(0);
        }

#if QT_VERSION < QT_VERSION_CHECK( 5,15, 0 )
        QString placeholderText() const { return QString(); }
        void setPlaceholderText( const QString & /*placeholderText*/ ) {}
#endif


        void addCurrentItem();
    };
}
#endif

