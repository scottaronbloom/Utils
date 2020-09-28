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

#ifndef __SPINBOX64U_H
#define __SPINBOX64U_H
#include "SpinBox64_StepType.h"
#include <QAbstractSpinBox>
#include <QString>
#include <QSize>

class CSpinBox64UImpl;
class CSpinBox64U : public QAbstractSpinBox
{
    friend class CSpinBox64UImpl;
#if (QT_VERSION < QT_VERSION_CHECK( 5, 12, 0 ))
    Q_ENUM( StepType )
#endif
Q_OBJECT;
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString cleanText READ cleanText )
    Q_PROPERTY( qulonglong minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( qulonglong maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( qulonglong singleStep READ singleStep WRITE setSingleStep )
    Q_PROPERTY( StepType stepType READ stepType WRITE setStepType )
    Q_PROPERTY( qulonglong value READ value WRITE setValue NOTIFY valueChanged USER true )
    Q_PROPERTY( qulonglong displayIntegerBase READ displayIntegerBase WRITE setDisplayIntegerBase )
public:
    explicit CSpinBox64U( QWidget* parent = nullptr );
    CSpinBox64U( const CSpinBox64U& ) = delete;
    CSpinBox64U& operator=( const CSpinBox64U& ) = delete;
    ~CSpinBox64U();

    qulonglong value() const ;

    QString prefix() const;
    void setPrefix( const QString& prefix );

    QString suffix() const;
    void setSuffix( const QString& suffix );

    bool wrapping() const;
    void setWrapping( bool wrapping );

    QString cleanText() const;

    qulonglong singleStep() const;
    void setSingleStep( qulonglong val );

    qulonglong minimum() const;
    void setMinimum( qulonglong min );

    qulonglong maximum() const;
    void setMaximum( qulonglong max );

    void setRange( qulonglong min, qulonglong max );

    StepType stepType() const;
    void setStepType( StepType stepType );

    int displayIntegerBase() const;
    void setDisplayIntegerBase( int base );

    QSize sizeHint() const override;

    virtual void stepBy( int steps ) override;

    static qulonglong maxAllowed() { return std::numeric_limits< qulonglong >::max(); }
    static qulonglong minAllowed() { return std::numeric_limits< qulonglong >::min(); }
    QValidator::State validate( QString& input, int& pos ) const override;
    virtual void fixup( QString& str ) const override;
    virtual QAbstractSpinBox::StepEnabled stepEnabled() const override;

public Q_SLOTS:
    void setValue( qulonglong val );
    void slotEditorCursorPositionChanged( int oldpos, int newpos );
    void slotEditorTextChanged( const QString& t );
Q_SIGNALS:
    void valueChanged( qulonglong v );
    void valueChanged( const QString & v );

protected:
    void connectLineEdit();

private:
    CSpinBox64UImpl * fImpl;
};
#endif
