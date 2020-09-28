// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#ifndef __SPINBOX64_H
#define __SPINBOX64_H
#include <QAbstractSpinBox>
#include "SpinBox64_StepType.h"

class CSpinBox64Impl;
class CSpinBox64 : public QAbstractSpinBox
{
    friend class CSpinBox64Impl;
    Q_ENUM(StepType)
Q_OBJECT;
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString cleanText READ cleanText )
    Q_PROPERTY( qlonglong minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( qlonglong maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( qlonglong singleStep READ singleStep WRITE setSingleStep )
    Q_PROPERTY( StepType stepType READ stepType WRITE setStepType )
    Q_PROPERTY( qlonglong value READ value WRITE setValue NOTIFY valueChanged USER true )
    Q_PROPERTY( qlonglong displayIntegerBase READ displayIntegerBase WRITE setDisplayIntegerBase )
public:
    explicit CSpinBox64( QWidget* parent = nullptr );
    CSpinBox64( const CSpinBox64& ) = delete;
    CSpinBox64& operator=( const CSpinBox64& ) = delete;
    ~CSpinBox64();

    qlonglong value() const ;

    QString prefix() const;
    void setPrefix( const QString& prefix );

    QString suffix() const;
    void setSuffix( const QString& suffix );

    bool wrapping() const;
    void setWrapping( bool wrapping );

    QString cleanText() const;

    qlonglong singleStep() const;
    void setSingleStep( qlonglong val );

    qlonglong minimum() const;
    void setMinimum( qlonglong min );

    qlonglong maximum() const;
    void setMaximum( qlonglong max );

    void setRange( qlonglong min, qlonglong max );

    StepType stepType() const;
    void setStepType( StepType stepType );

    int displayIntegerBase() const;
    void setDisplayIntegerBase( int base );

    QSize sizeHint() const override;

    virtual void stepBy( int steps ) override;

    static qlonglong maxAllowed() { return std::numeric_limits< qlonglong >::max(); }
    static qlonglong minAllowed() { return std::numeric_limits< qlonglong >::min(); }
    QValidator::State validate( QString& input, int& pos ) const override;
    virtual void fixup( QString& str ) const override;
    virtual QAbstractSpinBox::StepEnabled stepEnabled() const override;

public Q_SLOTS:
    void setValue( qlonglong val );
    void slotEditorCursorPositionChanged( int oldpos, int newpos );
    void slotEditorTextChanged( const QString& t );
Q_SIGNALS:
    void valueChanged( qlonglong v );
    void valueChanged( const QString & v );

protected:
    void connectLineEdit();

private:
    CSpinBox64Impl * fImpl;
};
#endif
