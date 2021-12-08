#ifndef __DELAYLINEEDIT_H
#define __DELAYLINEEDIT_H
// The MIT License( MIT )
//
// Copyright( c ) 2021 Scott Aron Bloom
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

#include <QLineEdit>
class QTimer;

class CDelayLineEdit : public QLineEdit
{
    Q_OBJECT;
public:
    enum class ELineEditStatus
    {
        ePending,
        eOK,
        eNotOK
    };

    explicit CDelayLineEdit( QWidget* parent = nullptr );
    explicit CDelayLineEdit( const QString& text, QWidget* parent = nullptr );
    explicit CDelayLineEdit( const QString& text, int delayMS, QWidget* parent = nullptr );

    virtual ~CDelayLineEdit();

    void setText( const QString &text );
    void setDelay( int delayMS );

    void setLineEditColor( ELineEditStatus status );
    void setLineEditColor( bool aOK );
    void setIsOKFunction( std::function< bool( const QString &text ) > func, const QString &errorMsg = {} )
    {
        fIsOK.first = func;
        fIsOK.second = errorMsg;
    }
Q_SIGNALS:
    void sigTextChangedAfterDelay( const QString& text );
    void sigTextEditedAfterDelay( const QString& text );
    void sigFinishedEditingAfterDelay();

public Q_SLOTS:
    void slotTextChanged();
    void slotTextEdited();
    void slotEditingFinished();
    void slotChangedTimerTimeout();
    void slotEditTimerTimeout();

private:
    void connectToEditor( bool connectOrDisconnect );

    int fDelayMS{ 500 };
    QTimer* fChangedTimer{ nullptr };
    QTimer* fEditedTimer{ nullptr };
    bool fEditingFinished{ false };
    std::pair< std::function< bool( const QString &text ) >, QString > fIsOK;
    ELineEditStatus fStatus{ ELineEditStatus::ePending };
};

#endif

