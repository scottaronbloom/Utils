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
class QMenu;
class QToolBar;
class QSpinBox;

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
        std::shared_ptr< CBIFFile > setFileName( const QString & fileName );
        QString fileName() const;
        bool isValid() const;

        QSize sizeHint() const override;

        void validatePlayerActions( bool enable = true ); // enables/disables the enabled actions, however if true, only when valid, if false all false
        bool isPlaying() const;
        void setActive(bool isActive);

        void setButtonsLayout( EButtonsLayout style );
        EButtonsLayout buttonsLayout()const { return fButtonLayout; }

        int numFramesToSkip() const { return fNumFramesToSkip; } // frames to skip when stepping
        void setNumFramesToSkip(int value);

        int playerSpeedMultiplier() const;
        void setSpeedMultiplier(int ms);

        int playCount() const;
        void setPlayCount(int playCount);

        void setGIFFlipImage( bool flipImage ) { fGIFFlipImage = flipImage; }
        bool gifFlipImage() const { return fGIFFlipImage; }

        void setGIFDitherImage( bool ditherImage ) { fGIFDitherImage = ditherImage; }
        bool gifDitherImage() const { return fGIFDitherImage; }

        void setGIFLoopCount( int loopCount ) { fGIFLoopCount = loopCount; }
        int gifLoopCount() const { return fGIFLoopCount; }

        void setGIFStartFrame( int startFrame ) { fGIFStartFrame = startFrame; }
        int gifStartFrame() const { return fGIFStartFrame; }

        void setGIFEndFrame( int endFrame ) { fGIFEndFrame = endFrame; }
        int gifEndFrame() const { return fGIFEndFrame; }

        void setGIFDelay( int Delay ) { fGIFDelay = Delay; }
        int gifDelay() const { return fGIFDelay; }

        QMenu *menu();
        QToolBar *toolBar();

        QAction *actionSkipBackward();
        QAction *actionPrev();
        QAction *actionTogglePlayPause( std::optional< bool > asPlayButton = {} ); // if not set no change, if set its changed to pay or not
        QAction *actionPause();
        QAction *actionPlay();
        QAction *actionNext();
        QAction *actionSkipForward();

        QAction *actionDiscreteLayout();
        QAction *actionToggleLayout();
        QAction *actionNoLayout();

        QAction * actionSaveAsGIF();
    Q_SIGNALS:
        void sigPlayingStarted();
    public Q_SLOTS:
        void slotSkipBackard();
        void slotPrev();
        void slotTogglePlayPause();
        void slotPause();
        void slotPlay();
        void slotNext();
        void slotSkipForward();

        void slotPlayerButtonDiscrete();
        void slotPlayerButtonToggle();
        void slotPlayerButtonNone();

        void slotSaveAsGIF();
    private Q_SLOTS:
        void slotSetNumFramesToSkip( int numFrames );
        void slotSetPlayerSpeed(int playerSpeed);
        void slotSetPlayCount(int playCount);
        void slotMovieStateChanged();
        void slotFrameChanged();
    private:
        void setBIFPluginPlayCount(int playCount);
        double computeFPS() const;

        void offsetFrameBy(int offset);
        void setCurrentFrame( int frame );
        template< typename T >
        void setPlayPause( T * item, bool playPause );
        template< typename T >
        void enableItem( T * item, bool enable );
        template< typename T >
        void checkItem( T *item, bool checked );
        template< typename T >
        void setItemVisible( T *item, bool visible );
        template< typename T >
        void updateItemForLayout( T *item );

        void updateMovieSpeed();
        

        void updateToolBar();
        void updateMenu();
        void layoutButtons();

        void setInfo( QAction *item, const QString &iconPath, const QString &text, void (CBIFWidget::*slot)() );
        void setInfo( QToolButton *btn, const QString &iconPath, const QString &text, void (CBIFWidget::*slot)() );

        QAction * createAction( const QString & name, const QString &iconPath, const QString &text, void (CBIFWidget:: *slot)() );

        std::shared_ptr< NBIF::CBIFFile > fBIF;

        QAction *fActionSkipBackward{ nullptr };
        QAction *fActionPrev{ nullptr };
        QAction *fActionTogglePlayPause{ nullptr };
        QAction *fActionPlay{ nullptr };
        QAction *fActionPause{ nullptr };
        QAction *fActionNext{ nullptr };
        QAction *fActionSkipForward{ nullptr };

        QAction *fActionDiscreteLayout{ nullptr };
        QAction *fActionToggleLayout{ nullptr };
        QAction *fActionNoLayout{ nullptr };
        QAction * fActionSaveAsGIF{ nullptr };

        EButtonsLayout fButtonLayout{ EButtonsLayout::eTogglePlayPause };

        QMenu *fMenu{ nullptr };
        QToolBar *fToolBar{ nullptr };

        QSpinBox * fPlayerSpeedMultiplerSB{ nullptr };
        QSpinBox * fNumFramesToSkipSB{ nullptr };
        int fNumFramesToSkip{ 5 };
        QSpinBox * fPlayCountSB{ nullptr };

        bool fGIFFlipImage{ false };
        bool fGIFDitherImage{ true };
        int fGIFLoopCount{ 0 };
        int fGIFStartFrame{ -1 };
        int fGIFEndFrame{ -1 };
        int fGIFDelay{ -1 };

        std::shared_ptr < QMovie > fMovie;
        std::unique_ptr< Ui::CBIFWidget > fImpl;
    };



}
#endif 
