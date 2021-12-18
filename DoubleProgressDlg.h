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

#ifndef __DOUBLEPROGRESSDLG_H
#define __DOUBLEPROGRESSDLG_H

class QProgressBar;
#include <QDialog>
#include <QElapsedTimer>
#include <memory>
class QTimer;

class CDoubleProgressDlgImpl;
class CDoubleProgressDlg : public QDialog
{
    friend class CDoubleProgressDlgImpl;
    Q_OBJECT
public:
    CDoubleProgressDlg( const QString & text, const QString & subTitle, const QString & cancelText, int min, int max, QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
    CDoubleProgressDlg( const QString & labelText, const QString & cancelText, int min, int max, QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() ); // subtitle = QString()
    CDoubleProgressDlg( const QString & text, const QString & cancelText, QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() ); // subtitle = QString(), min max = 0, 100; 
    CDoubleProgressDlg( QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() ); // all defaults
    ~CDoubleProgressDlg();

    int primaryValue() const;
    int primaryMin() const;
    int primaryMax() const;
    QString primaryFormat() const;

    int secondaryValue() const;
    int secondaryMin() const;
    int secondaryMax() const;
    QString secondaryFormat() const;

    QString title() const;
    QString subTitle() const;

    void setCancelButton( QPushButton * button );
    QPushButton * cancelButton() const;
    QString cancelText() const;

    bool wasCanceled() const;

    bool singleProgressBarMode() const; // default false

    int minumumDuration() const;
    void setAutoClose( bool autoClose ); // tied to primary value
    bool autoClose() const;

    void setAutoReset( bool autoReset ); // primary value
    bool autoReset() const;

    int value() const { return primaryValue(); }
    QString labelText() const { return title(); }

    virtual QSize sizeHint() const override;
protected:
    virtual void closeEvent( QCloseEvent * e ) override;
    virtual void resizeEvent( QResizeEvent * ) override;
    virtual void changeEvent( QEvent * ev ) override;
    virtual void showEvent( QShowEvent * e ) override;
Q_SIGNALS:
    void canceled(); // 
public Q_SLOTS:
    virtual void slotCanceled();
    virtual void cancel();

    void setTitle( const QString & text );
    void setSubTitle( const QString & text );
    void setCancelButtonText( const QString & cancelText );

    void setPrimaryProgressLabel( const QString & text );

    void setPrimaryValue( int value );
    void setPrimaryRange( int min, int max );
    void setPrimaryMinimum( int min ) { setPrimaryRange( min, primaryMax() ); }
    void setPrimaryMaximum( int max ) { setPrimaryRange( primaryMin(), max ); }
    void setPrimaryFormat( const QString & format );

    void setSecondaryProgressLabel( const QString & text );
    void setSecondaryValue( int value );
    void setSecondaryMinimum( int min ) { setSecondaryRange( min, primaryMax() ); }
    void setSecondaryMaximum( int max ) { setSecondaryRange( primaryMin(), max ); }
    void setSecondaryRange( int min, int max );
    void setSecondaryFormat( const QString & format );

    void setSingleProgressBarMode( bool value );

    void setMinimumDuration( int msec );
    void setValue( int value ) { setPrimaryValue( value ); }
    void setRange( int min, int max ) { setPrimaryRange( min, max ); }
    void setMinimum( int min ) { setPrimaryMinimum( min ); }
    void setMaximum( int max ) { setPrimaryMaximum( max ); }
    void setLabelText( const QString & text ) { setTitle( text ); }

    void reset();
protected Q_SLOTS:
    virtual void slotForceShow();

private:
    std::unique_ptr< CDoubleProgressDlgImpl > fImpl;
};
#endif 