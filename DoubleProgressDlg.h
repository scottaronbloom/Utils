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

#ifndef __DOUBLEPROGRESSDLG_H
#define __DOUBLEPROGRESSDLG_H

#include "SABUtilsExport.h"

class QProgressBar;
#include <QDialog>
#include <QElapsedTimer>
#include <memory>
class QTimer;
namespace NSABUtils
{
    class CDoubleProgressDlgImpl;
    class SABUTILS_EXPORT CDoubleProgressDlg : public QDialog
    {
        friend class CDoubleProgressDlgImpl;
        Q_OBJECT
    public:
        CDoubleProgressDlg( const QString & text, const QString & subTitle, const QString & cancelText, int min, int max, QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
        CDoubleProgressDlg( const QString & labelText, const QString & cancelText, int min, int max, QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() ); // subtitle = QString()
        CDoubleProgressDlg( const QString & text, const QString & cancelText, QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() ); // subtitle = QString(), min max = 0, 100; 
        CDoubleProgressDlg( QWidget * parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() ); // all defaults
        virtual ~CDoubleProgressDlg() override;

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

        int value() const;
        QString labelText() const { return title(); }

        // for progress bars, one step may have 4 sub events
        // set this value to for in these cases, default is 1
        // this value is used in the label and label only
        void setPrimaryEventsPerIncrement( int value );
        int  primaryEventsPerIncrement() const;

        void setSecondaryEventsPerIncrement( int value );
        int  secondaryEventsPerIncrement() const;

        void stopAutoShowTimer() const; // when you set a value, sometiems it can trigger a show you dont want

    protected:
        virtual QSize sizeHint() const override;
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
        void setPrimaryVisible( bool visible );

        void setSecondaryProgressLabel( const QString & text );
        void setSecondaryValue( int value );
        void setSecondaryMinimum( int min ) { setSecondaryRange( min, primaryMax() ); }
        void setSecondaryMaximum( int max ) { setSecondaryRange( primaryMin(), max ); }
        void setSecondaryRange( int min, int max );
        void setSecondaryFormat( const QString & format );
        void setSecondaryVisible( bool visible );

        void setSingleProgressBarMode( bool value );

        void setMinimumDuration( int msec );
        void setValue( int value ) { setPrimaryValue( value ); }
        void setRange( int min, int max ) { setPrimaryRange( min, max ); }
        void setMinimum( int min ) { setPrimaryMinimum( min ); }
        void setMaximum( int max ) { setPrimaryMaximum( max ); }
        void setLabelText( const QString & text ) { setTitle( text ); }

        void reset( bool canceled );
    protected Q_SLOTS:
        virtual void slotForceShow();

    private:
        std::unique_ptr< CDoubleProgressDlgImpl > fImpl;
    };
}
#endif 