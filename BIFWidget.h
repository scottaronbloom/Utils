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

#ifndef _BIFWIDGET_H
#define _BIFWIDGET_H

#include <QFrame>
#include <optional>
#include <memory>

class QAction;
class QLabel;
class QSpacerItem;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;

namespace Ui { class CBIFWidget; }
namespace NBIF
{
    class CBIFFile;
    enum class EButtonsLayout
    {
        eTogglePlayPause,
        eDiscretePlayPause,
        eNoButtons,
    };
    class CBIFWidget : public QFrame
    {
        Q_OBJECT
    public:
        CBIFWidget( QWidget *parent = 0 );
        ~CBIFWidget();

        void clear();
        void setBIFFile( std::shared_ptr< CBIFFile > bif );

        void validatePlayerActions( bool enable = true ); // enables/disables the enabled actions, however if true, only when valid, if false all false
        bool isPlaying() const;

        void setButtonsLayout( EButtonsLayout style );
        EButtonsLayout buttonsLayout()const { return fButtonStyle; }

        QAction *actionSkipBackward();
        QAction *actionPrev();
        QAction *actionTogglePlayPause( std::optional< bool > asPlayButton = {} ); // if not set no change, if set its changed to pay or not
        QAction *actionPause();
        QAction *actionPlay();
        QAction *actionNext();
        QAction *actionSkipForward();

        int skipInterval() const { return fSkipInterval; }
    Q_SIGNALS:
        void sigPaused();
        void sigStarted();
        void sigShowingFrame( uint32_t frameNum );
    public Q_SLOTS:
        void slotTimerExpired();
        void slotSetFrameInterval( int msec );
        void slotSetSkipInterval( int numFrames ) { fSkipInterval = numFrames; }

        void slotSkipBackard();
        void slotPrev();
        void slotTogglePlayPause();
        void slotPause();
        void slotPlay();
        void slotNext();
        void slotSkipForward();
    private:
        void showCurrentFrame();
        void setCurrentFrame( int frame );
        void offsetFrame( int offset );
        template< typename T >
        void setPlayPause( T * item, bool playPause )
        {
            if ( !item )
                return;

            if ( playPause )
                setInfo( item, ":/BIFPlayerResources/play.png", tr( "Play" ), nullptr );
            else
                setInfo( item, ":/BIFPlayerResources/pause.png", tr( "Pause" ), nullptr );
        }
        template< typename T >
        void enableItem( T * item, bool enable )
        {
            if ( !item )
                return;
            item->setEnabled( enable );
        }

        void layoutButtons();

        void setInfo( QAction *item, const QString &iconPath, const QString &text, void (CBIFWidget::*slot)() );
        void setInfo( QToolButton *btn, const QString &iconPath, const QString &text, void (CBIFWidget::*slot)() );

        QToolButton * createToolButton( const QString & name, const QString & iconPath, const QString & text, void (CBIFWidget::*slot)() );
        QAction * createAction( const QString & name, const QString &iconPath, const QString &text, void (CBIFWidget:: *slot)() );

        QTimer *fFrameTimer{ nullptr };
        std::optional< uint32_t > fCurrentFrame;
        std::shared_ptr< NBIF::CBIFFile > fBIF;

        QAction *fActionSkipBackward{ nullptr };
        QAction *fActionPrev{ nullptr };
        QAction *fActionTogglePlayPause{ nullptr };
        QAction *fActionPlay{ nullptr };
        QAction *fActionPause{ nullptr };
        QAction *fActionNext{ nullptr };
        QAction *fActionSkipForward{ nullptr };

        int fSkipInterval{ 5 };
        EButtonsLayout fButtonStyle{ EButtonsLayout::eTogglePlayPause };

        std::unique_ptr< Ui::CBIFWidget > fImpl;
    };
}
#endif 
