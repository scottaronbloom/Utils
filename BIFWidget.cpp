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

#include "BIFWidget.h"
#include "BIFFile.h"
#include "utils.h"
#include "QtUtils.h"

#include <QTimer>
#include <QIcon>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QSpinBox>

#include "ui_BIFWidget.h"

namespace NBIF
{
    CBIFWidget::CBIFWidget( QWidget *parent )
        : QFrame( parent ),
        fImpl( new Ui::CBIFWidget )
    {
        Q_INIT_RESOURCE( BIFPlayerResources );
        fImpl->setupUi( this );

        setInfo( fImpl->skipBackwardDiscreteBtn, ":/BIFPlayerResources/skipbackward.png", tr( "Skip Backward" ), &CBIFWidget::slotSkipBackard );
        setInfo( fImpl->prevDiscreteBtn, ":/BIFPlayerResources/prev.png", tr( "Previous Frame" ), &CBIFWidget::slotPrev );
        setInfo( fImpl->playBtn, QString(), QString(), &CBIFWidget::slotPlay );
        setPlayPause( fImpl->playBtn, true );
        setInfo( fImpl->pauseBtn, QString(), QString(), &CBIFWidget::slotPause );
        setPlayPause( fImpl->pauseBtn, false );
        setInfo( fImpl->nextDiscreteBtn, ":/BIFPlayerResources/next.png", tr( "Next Frame" ), &CBIFWidget::slotNext );
        setInfo( fImpl->skipForwardDiscreteBtn, ":/BIFPlayerResources/skipforward.png", tr( "Skip Forward" ), &CBIFWidget::slotSkipForward );

        setInfo( fImpl->skipBackwardToggleBtn, ":/BIFPlayerResources/skipbackward.png", tr( "Skip Backward" ), &CBIFWidget::slotSkipBackard );
        setInfo( fImpl->prevToggleBtn, ":/BIFPlayerResources/prev.png", tr( "Previous Frame" ), &CBIFWidget::slotPrev );
        setInfo( fImpl->playPauseBtn, QString(), QString(), &CBIFWidget::slotTogglePlayPause );
        setPlayPause( fImpl->playPauseBtn, true );
        setInfo( fImpl->nextToggleBtn, ":/BIFPlayerResources/next.png", tr( "Next Frame" ), &CBIFWidget::slotNext );
        setInfo( fImpl->skipForwardToggleBtn, ":/BIFPlayerResources/skipforward.png", tr( "Skip Forward" ), &CBIFWidget::slotSkipForward );

        layoutButtons();

        setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
        
        slotSetFrameInterval( 50 );
    }

    CBIFWidget::~CBIFWidget()
    {
    }

    void CBIFWidget::setSkipInterval( int msec )
    {
        slotSetSkipInterval( msec );
        if ( fSkipIntervalSB )
            fSkipIntervalSB->setValue( fSkipInterval );
    }

    void CBIFWidget::slotSetSkipInterval( int numFrames )
    {
        fSkipInterval = numFrames;
    }

    void CBIFWidget::setFrameInterval( int msec )
    {
        slotSetFrameInterval( msec );
    }

    int CBIFWidget::frameInterval() const
    {
        if ( !fFrameTimer )
            return 0;
        return fFrameTimer->interval();
    }

    void CBIFWidget::slotSetFrameInterval( int msec )
    {
        if ( !fFrameTimer )
        {
            fFrameTimer = new QTimer( this );
            fFrameTimer->setSingleShot( false );
            connect( fFrameTimer, &QTimer::timeout, this, &CBIFWidget::slotTimerExpired );
        }
        bool wasRunning = isPlaying();
        fFrameTimer->stop();
        fFrameTimer->setInterval( msec );
        if ( wasRunning )
            fFrameTimer->start();
    }

    void CBIFWidget::showCurrentFrame()
    {
        if ( !fBIF )
            return;

        if ( !fCurrentFrame.has_value() )
            return;

        auto &&image = fBIF->image( fCurrentFrame.value() );
        fImpl->imageLabel->setPixmap( QPixmap::fromImage( image ) );
        fImpl->imageLabel->setMinimumSize( image.size() );
        setMinimumWidth( image.width() + layout()->contentsMargins().left() + layout()->contentsMargins().right() );
        emit sigShowingFrame( fCurrentFrame.value() );
        fImpl->textLabel->setText( tr( "BIF #: %1 Time: %2" ).arg( fCurrentFrame.value() ).arg( NUtils::CTimeString( fCurrentFrame.value() * std::get< 2 >( fBIF->tsMultiplier() ) ).toString( "hh:mm:ss.zzz" ) ) );
        fImpl->textLabel->setVisible( true );
    }

    void CBIFWidget::slotPause()
    {
        fFrameTimer->stop();
        emit sigPaused();

        validatePlayerActions();
    }

    void CBIFWidget::slotPlay()
    {
        fFrameTimer->start();
        emit sigStarted();

        validatePlayerActions();
    }


    void CBIFWidget::setActive( bool isActive )
    {
        fToolBar->setVisible( isActive );
        validatePlayerActions( isActive );
    }

    void CBIFWidget::validatePlayerActions( bool enabled )
    {
        bool aOK = ( enabled && fBIF && fBIF->isValid() );

        enableItem( fActionSkipBackward, aOK );
        enableItem( fActionPrev, aOK );
        enableItem( fActionPlay, aOK && !isPlaying() );
        enableItem( fActionPause, aOK && isPlaying() );
        enableItem( fActionTogglePlayPause, aOK );
        setPlayPause(fImpl->playPauseBtn, !isPlaying());
        setPlayPause(fActionTogglePlayPause, !isPlaying());
        enableItem( fActionNext, aOK );
        enableItem( fActionSkipForward, aOK );


        enableItem( fImpl->skipBackwardDiscreteBtn, aOK );
        enableItem( fImpl->prevDiscreteBtn, aOK );
        enableItem( fImpl->playBtn, aOK && !isPlaying() );
        enableItem( fImpl->pauseBtn, aOK && isPlaying() );
        enableItem( fImpl->skipForwardDiscreteBtn, aOK );
        enableItem( fImpl->nextDiscreteBtn, aOK );

        enableItem( fImpl->skipBackwardToggleBtn, aOK );
        enableItem( fImpl->prevToggleBtn, aOK );
        enableItem( fImpl->playPauseBtn, aOK );
        enableItem( fImpl->skipForwardToggleBtn, aOK );
        enableItem( fImpl->nextToggleBtn, aOK );

        checkItem( fActionDiscreteLayout, false );
        checkItem( fActionToggleLayout, false );
        checkItem( fActionNoLayout, false );

        if ( fButtonLayout == NBIF::EButtonsLayout::eDiscretePlayPause )
            checkItem( fActionDiscreteLayout, true );
        else if ( fButtonLayout == NBIF::EButtonsLayout::eTogglePlayPause )
            checkItem( fActionToggleLayout, true );
        else if ( fButtonLayout == NBIF::EButtonsLayout::eNoButtons )
            checkItem( fActionNoLayout, true );
    }

    bool CBIFWidget::isPlaying() const
    {
        return fFrameTimer && fFrameTimer->isActive();
    }

    void CBIFWidget::slotPrev()
    {
        slotPause();
        offsetFrame( -1 );
        showCurrentFrame();
    }

    void CBIFWidget::slotNext()
    {
        slotPause();
        offsetFrame( 1 );
        showCurrentFrame();
    }

    void CBIFWidget::slotSkipBackard()
    {
        slotPause();
        offsetFrame( -fSkipInterval );
        showCurrentFrame();
    }

    void CBIFWidget::slotSkipForward()
    {
        slotPause();
        offsetFrame( fSkipInterval );
        showCurrentFrame();
    }

    void CBIFWidget::slotTimerExpired()
    {
        if ( !fCurrentFrame.has_value() )
            setCurrentFrame( 0 );
        showCurrentFrame();
        offsetFrame( 1 );
    }


    void CBIFWidget::slotTogglePlayPause()
    {
        if ( isPlaying() )
            slotPause();
        else
            slotPlay();
    }

    void CBIFWidget::setCurrentFrame( int frame )
    {
        int sz = fBIF ? static_cast<int>( fBIF->bifs().size() ) : 0;
        if ( frame < 0 )
            frame = sz + frame;
        if ( frame >= sz )
            frame = frame - sz;
        fCurrentFrame = frame;
    }

    void CBIFWidget::offsetFrame( int offset )
    {
        if ( !fCurrentFrame.has_value() )
        {
            if ( offset < 0 )
                setCurrentFrame( offset + 1 );
            else
                setCurrentFrame( offset );
        }
        else
        {
            setCurrentFrame( fCurrentFrame.value() + offset );
        }
    }

    void CBIFWidget::clear()
    {
        fImpl->imageLabel->setPixmap( QPixmap() );
        fImpl->textLabel->setText( QString() );
        fImpl->textLabel->setVisible( false );
        fCurrentFrame.reset();
        fFrameTimer->stop();
    }

    void CBIFWidget::setBIFFile( std::shared_ptr< CBIFFile > bif )
    {
        fBIF = bif;
        validatePlayerActions( fBIF && fBIF->isValid() );
    }

    QAction *CBIFWidget::actionSkipBackward()
    {
        if ( !fActionSkipBackward )
        {
            fActionSkipBackward = createAction( "actionSkipBackward", ":/BIFPlayerResources/skipbackward.png", tr( "Skip Backward" ), &CBIFWidget::slotSkipBackard );
        }
        return fActionSkipBackward;
    }

    QAction *CBIFWidget::actionPrev()
    {
        if ( !fActionPrev )
        {
            fActionPrev = createAction( "actionPrev", ":/BIFPlayerResources/prev.png", tr( "Previous Frame" ), &CBIFWidget::slotPrev );
        }
        return fActionPrev;
    }

    QAction *CBIFWidget::actionTogglePlayPause( std::optional< bool > asPlayButton )
    {
        if ( !fActionTogglePlayPause )
        {
            fActionTogglePlayPause = createAction( "actionTogglePlayPause", QString(), QString(), &CBIFWidget::slotTogglePlayPause );
            if ( !asPlayButton.has_value() )
                setPlayPause( fActionTogglePlayPause, true );
        }
        if ( asPlayButton.has_value() )
            setPlayPause( fActionTogglePlayPause, asPlayButton.value() );

        return fActionTogglePlayPause;
    }

    QAction *CBIFWidget::actionPlay()
    {
        if ( !fActionPlay )
        {
            fActionPlay = createAction( QString::fromUtf8( "actionPlay" ), QString(), QString(), &CBIFWidget::slotPlay );
            setPlayPause( fActionPlay, true );
        }

        return fActionPlay;
    }

    QAction *CBIFWidget::actionPause()
    {
        if ( !fActionPause )
        {
            fActionPause = createAction( QString::fromUtf8( "actionPause" ), QString(), QString(), &CBIFWidget::slotPause );
            setPlayPause( fActionPause, false );
        }

        return fActionPause;
    }

    QAction *CBIFWidget::actionNext()
    {
        if ( !fActionNext )
        {
            fActionNext = createAction( QString::fromUtf8( "actionNext" ), ":/BIFPlayerResources/next.png", tr( "Next Frame" ), &CBIFWidget::slotNext );
        }
        return fActionNext;
    }

    QAction *CBIFWidget::actionSkipForward()
    {
        if ( !fActionSkipForward )
        {
            fActionSkipForward = createAction( QString::fromUtf8( "actionSkipForward" ), ":/BIFPlayerResources/skipforward.png", tr( "Skip Forward" ), &CBIFWidget::slotSkipForward );
        }
        return fActionSkipForward;
    }

    QAction *CBIFWidget::actionDiscreteLayout()
    {
        if ( !fActionDiscreteLayout )
        {
            fActionDiscreteLayout = createAction( QString::fromUtf8( "actionDiscreteLayout" ), QString(), tr( "Discrete Play/Pause Buttons" ), &CBIFWidget::slotPlayerButtonDiscrete );
            fActionDiscreteLayout->setCheckable( true );
        }
        return fActionDiscreteLayout;
    }

    QAction *CBIFWidget::actionToggleLayout()
    {
        if ( !fActionToggleLayout )
        {
            fActionToggleLayout = createAction( QString::fromUtf8( "actionToggleLayout" ), QString(), tr( "Single Toggleable Play/Pause Button" ), &CBIFWidget::slotPlayerButtonToggle );
            fActionToggleLayout->setCheckable( true );
        }
        return fActionToggleLayout;
    }

    QAction *CBIFWidget::actionNoLayout()
    {
        if ( !fActionNoLayout )
        {
            fActionNoLayout = createAction( QString::fromUtf8( "actionNoLayout" ), QString(), tr( "No Player Buttons" ), &CBIFWidget::slotPlayerButtonNone );
            fActionNoLayout->setCheckable( true );
        }
        return fActionNoLayout;
    }

    void CBIFWidget::setButtonsLayout( EButtonsLayout layout )
    {
        if ( fButtonLayout == layout )
            return;

        fButtonLayout = layout;

        updateMenu();
        updateToolBar();
        layoutButtons();
        validatePlayerActions();
    }

    void CBIFWidget::layoutButtons()
    {
        if ( fButtonLayout == EButtonsLayout::eNoButtons )
            fImpl->stackedWidget->setVisible( false );
        else
        {
            fImpl->stackedWidget->setVisible( true );
            fImpl->stackedWidget->setCurrentIndex( static_cast<int>( fButtonLayout ) );
        }
    }

    void CBIFWidget::setInfo( QAction *item, const QString &iconPath, const QString &text, void (CBIFWidget::*slot)() )
    {
        QIcon icon;
        icon.addFile( iconPath, QSize(), QIcon::Normal, QIcon::Off );
        item->setIcon( icon );
        item->setToolTip( text );
        item->setText( text );
        if ( slot )
            connect( item, &QAction::triggered, this, slot );
    }

    void CBIFWidget::setInfo( QToolButton *item, const QString &iconPath, const QString &text, void (CBIFWidget::*slot)() )
    {
        QIcon icon;
        icon.addFile( iconPath, QSize(), QIcon::Normal, QIcon::Off );
        item->setIcon( icon );
        item->setToolTip( text );
        item->setText( text );
        if ( slot )
            connect( item, &QToolButton::clicked, this, slot );
    }

    QAction *CBIFWidget::createAction( const QString & name, const QString &iconPath, const QString &text, void (CBIFWidget::*slot)() )
    {
        auto retVal = new QAction( this );
        retVal->setObjectName( name );
        setInfo( retVal, iconPath, text, slot );
        return retVal;
    }

    void CBIFWidget::slotPlayerButtonDiscrete()
    {
        setButtonsLayout( NBIF::EButtonsLayout::eDiscretePlayPause );
    }

    void CBIFWidget::slotPlayerButtonToggle()
    {
        setButtonsLayout( NBIF::EButtonsLayout::eTogglePlayPause );
    }

    void CBIFWidget::slotPlayerButtonNone()
    {
        setButtonsLayout( NBIF::EButtonsLayout::eNoButtons );
    }

    QToolBar *CBIFWidget::toolBar()
    {
        if ( !fToolBar )
        {
            fToolBar = new QToolBar;
            fToolBar->setObjectName( "BIF Viewer Toolbar" );
            fToolBar->setWindowTitle( tr( "BIF Viewer Toolbar" ) );

            updateToolBar();
            validatePlayerActions();
        }
        return fToolBar;
    }

    QMenu *CBIFWidget::menu()
    {
        if ( !fMenu )
        {
            fMenu = new QMenu( this );
            fMenu->setObjectName( "BIF Viewer Menu" );
            fMenu->setTitle( tr( "BIF Viewer" ) );

            updateMenu();
            validatePlayerActions();
        }
        return fMenu;
    }

    void CBIFWidget::updateMenu()
    {
        if ( !fMenu )
            return;

        fMenu->clear();
        auto subMenu = fMenu->addMenu( tr( "BIF Player Type" ) );
        subMenu->addAction( actionDiscreteLayout() );
        subMenu->addAction( actionToggleLayout() );
        subMenu->addAction( actionNoLayout() );

        if ( fButtonLayout == EButtonsLayout::eNoButtons )
            return;

        updateItemForLayout( fMenu );
    }

    void CBIFWidget::updateToolBar()
    {
        if ( !fToolBar )
            return;

        fToolBar->clear();

        auto label = new QLabel( tr( "Seconds per Frame:" ) );
        label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
        fToolBar->addWidget( label );

        auto frameIntervalSB = new QSpinBox;
        frameIntervalSB->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        frameIntervalSB->setSuffix( tr( "ms" ) );
        frameIntervalSB->setMinimum( 0 );
        frameIntervalSB->setMaximum( std::numeric_limits< int >::max() );
        frameIntervalSB->setSingleStep( 50 );
        connect( frameIntervalSB, qOverload< int >( &QSpinBox::valueChanged ), this, &CBIFWidget::slotSetFrameInterval );

        if ( fFrameTimer )
            frameIntervalSB->setValue( fFrameTimer->interval() );

        fToolBar->addWidget( frameIntervalSB );
        fToolBar->addSeparator();

        label = new QLabel( tr( "Frames to Skip:" ) );
        label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
        fToolBar->addWidget( label );

        fSkipIntervalSB = new QSpinBox;
        fSkipIntervalSB->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        fSkipIntervalSB->setSuffix( tr( "frames" ) );
        fSkipIntervalSB->setMinimum( 0 );
        fSkipIntervalSB->setMaximum( std::numeric_limits< int >::max() );
        fSkipIntervalSB->setSingleStep( 50 );
        fSkipIntervalSB->setValue( fSkipInterval );
        connect( fSkipIntervalSB, qOverload< int >( &QSpinBox::valueChanged ), this, &CBIFWidget::slotSetSkipInterval );

        fToolBar->addWidget( fSkipIntervalSB );

        updateItemForLayout( fToolBar );
    }

    template< typename T>
    void CBIFWidget::updateItemForLayout( T * item )
    {
        if ( fButtonLayout == EButtonsLayout::eNoButtons )
            return;
        item->addSeparator();

        item->addAction( actionSkipBackward() );
        item->addAction( actionPrev() );
        item->addSeparator();
        if ( fButtonLayout == EButtonsLayout::eDiscretePlayPause )
        {
            item->addAction( actionPause() );
            item->addAction( actionPlay() );
        }
        else if ( fButtonLayout == EButtonsLayout::eTogglePlayPause )
        {
            item->addAction( actionTogglePlayPause() );
        }
        item->addSeparator();
        item->addAction( actionNext() );
        item->addAction( actionSkipForward() );
    }


    template< typename T >
    void NBIF::CBIFWidget::setPlayPause( T *item, bool playPause )
    {
        if ( !item )
            return;

        if ( playPause )
            setInfo( item, ":/BIFPlayerResources/play.png", tr( "Play" ), nullptr );
        else
            setInfo( item, ":/BIFPlayerResources/pause.png", tr( "Pause" ), nullptr );
    }

    template< typename T >
    void NBIF::CBIFWidget::checkItem( T *item, bool checked )
    {
        if ( !item )
            return;
        item->setChecked( checked );
    }

    template< typename T >
    void NBIF::CBIFWidget::setItemVisible( T *item, bool visible )
    {
        if ( !item )
            return;
        item->setVisible( visible );
    }

    template< typename T >
    void NBIF::CBIFWidget::enableItem( T *item, bool enable )
    {
        if ( !item )
            return;
        item->setEnabled( enable );
    }
}
