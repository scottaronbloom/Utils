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
#include "SABUtils/QtUtils.h"

#include <QTimer>
#include <QIcon>
#include <QAction>
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


    void CBIFWidget::validatePlayerActions( bool enabled )
    {
        bool aOK = ( enabled && fBIF && fBIF->isValid() );

        enableItem( fActionNext, aOK );
        enableItem( fActionSkipForward, aOK );
        enableItem( fActionPrev, aOK );
        enableItem( fActionSkipBackward, aOK );
        enableItem( fActionTogglePlayPause, aOK );
        enableItem( fActionPlay, aOK && !isPlaying() );
        enableItem( fActionPause, aOK && isPlaying() );
        enableItem( fImpl->playBtn, aOK && !isPlaying() );
        enableItem( fImpl->pauseBtn, aOK && isPlaying() );


        setPlayPause( fActionTogglePlayPause, !isPlaying() );
        setPlayPause( fImpl->playPauseBtn, !isPlaying() );
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
        if ( frame < 0 )
            frame = static_cast<int>( fBIF->bifs().size() ) - 1;
        else if ( frame >= fBIF->bifs().size() )
            frame = 0;
        fCurrentFrame = frame;
    }

    void CBIFWidget::offsetFrame( int offset )
    {
        if ( !fCurrentFrame.has_value() )
            setCurrentFrame( 0 );
        else
            setCurrentFrame( fCurrentFrame.value() + offset );
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

    void CBIFWidget::setButtonsLayout( EButtonsLayout style )
    {
        if ( fButtonStyle == style )
            return;

        fButtonStyle = style;

        actionTogglePlayPause()->setVisible( false );
        actionPause()->setVisible( false );
        actionPlay()->setVisible( false );

        if ( style == NBIF::EButtonsLayout::eDiscretePlayPause )
        {
            actionSkipBackward()->setVisible( true );
            actionPrev()->setVisible( true );

            actionNext()->setVisible( true );
            actionSkipForward()->setVisible( true );

            actionPause()->setVisible( true );
            actionPlay()->setVisible( true );
        }
        else if ( style == NBIF::EButtonsLayout::eTogglePlayPause )
        {
            actionSkipBackward()->setVisible( true );
            actionPrev()->setVisible( true );

            actionNext()->setVisible( true );
            actionSkipForward()->setVisible( true );

            actionTogglePlayPause()->setVisible( true );
        }
        else if ( style == NBIF::EButtonsLayout::eNoButtons )
        {
            actionSkipBackward()->setVisible( false );
            actionPrev()->setVisible( false );

            actionNext()->setVisible( false );
            actionSkipForward()->setVisible( false );
        }

        layoutButtons();
    }

    void CBIFWidget::layoutButtons()
    {
        if ( fButtonStyle == EButtonsLayout::eNoButtons )
            fImpl->stackedWidget->setVisible( false );
        else
        {
            fImpl->stackedWidget->setVisible( true );
            fImpl->stackedWidget->setCurrentIndex( static_cast<int>( fButtonStyle ) );
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
}