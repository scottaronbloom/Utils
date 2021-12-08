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
#include <QTimer>
#include <QLabel>
#include <QIcon>
#include <QAction>
#include <QVBoxLayout>
#include <QSpacerItem>

namespace NBIF
{
    CBIFWidget::CBIFWidget( QWidget *parent )
        : QFrame( parent )
    {
        Q_INIT_RESOURCE( BIFPlayerResources );
        auto vlayout = new QVBoxLayout( this );
        vlayout->setObjectName( "vlayout" );

        fImageLabel = new QLabel( this );
        fImageLabel->setObjectName( QString::fromUtf8( "ImageLabel" ) );

        QSizePolicy sizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
        sizePolicy.setHorizontalStretch( 0 );
        sizePolicy.setVerticalStretch( 0 );
        sizePolicy.setHeightForWidth( fImageLabel->sizePolicy().hasHeightForWidth() );
        fImageLabel->setSizePolicy( sizePolicy );

        fImageLabel->setMinimumSize( QSize( 0, 200 ) );
        fImageLabel->setBaseSize( QSize( 0, 0 ) );
        fImageLabel->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
        vlayout->addWidget( fImageLabel );

        fTextLabel = new QLabel( this );
        fTextLabel->setObjectName( QString::fromUtf8( "TextLabel" ) );
        vlayout->addWidget( fTextLabel );

        auto verticalSpacer = new QSpacerItem( 17, 336, QSizePolicy::Minimum, QSizePolicy::Expanding );

        vlayout->addItem( verticalSpacer );

        setFrameShape( QFrame::Panel );
        setFrameShadow( QFrame::Sunken );

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
        fImageLabel->setPixmap( QPixmap::fromImage( image ) );
        emit sigShowingFrame( fCurrentFrame.value() );
        fTextLabel->setText( tr( "BIF #: %1 Time: %2" ).arg( fCurrentFrame.value() ).arg( NUtils::CTimeString( fCurrentFrame.value() * std::get< 2 >( fBIF->tsMultiplier() ) ).toString( "hh:mm:ss.zzz" ) ) );
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

        enableAction( fActionNext, aOK );
        enableAction( fActionSkipForward, aOK );
        enableAction( fActionPrev, aOK );
        enableAction( fActionSkipBackward, aOK );
        enableAction( fActionTogglePlayPause, aOK );
        enableAction( fActionPlay, aOK && !isPlaying() );
        enableAction( fActionPause, aOK && isPlaying() );

        setPlayPause( fActionTogglePlayPause, !isPlaying() );
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

    void CBIFWidget::setActionInfo( QAction *action, const QString & iconPath, const QString & text )
    {
        QIcon icon;
        icon.addFile( iconPath, QSize(), QIcon::Normal, QIcon::Off );
        action->setIcon( icon );
        action->setToolTip( text );
        action->setText( text );
    }

    void CBIFWidget::enableAction( QAction *action, bool enable )
    {
        if ( action )
            action->setEnabled( enable );
    }

    void CBIFWidget::clear()
    {
        fImageLabel->setPixmap( QPixmap() );
        fTextLabel->setText( QString() );
    }

    void CBIFWidget::setBIFFile( std::shared_ptr< CBIFFile > bif )
    {
        fBIF = bif;
        validatePlayerActions( fBIF && fBIF->isValid() );
    }

    void CBIFWidget::setPlayPause( QAction * action, bool playPause )
    {
        if ( !action )
            return;

        if ( playPause )
            setActionInfo( action, ":/BIFPlayerResources/play.png", tr( "Play" ) );
        else
            setActionInfo( action, ":/BIFPlayerResources/pause.png", tr( "Pause" ) );
    }

    QAction *CBIFWidget::actionSkipBackward()
    {
        if ( !fActionSkipBackward )
        {
            fActionSkipBackward = new QAction( this );
            fActionSkipBackward->setObjectName( QString::fromUtf8( "actionSkipBackward" ) );
            setActionInfo( fActionSkipBackward, ":/BIFPlayerResources/skipbackward.png", tr( "Skip Backward" ) );
            connect( fActionSkipBackward, &QAction::triggered, this, &CBIFWidget::slotSkipBackard );
        }
        return fActionSkipBackward;
    }

    QAction *CBIFWidget::actionPrev()
    {
        if ( !fActionPrev )
        {
            fActionPrev = new QAction( this );
            fActionPrev->setObjectName( QString::fromUtf8( "actionPrev" ) );
            setActionInfo( fActionPrev, ":/BIFPlayerResources/prev.png", tr( "Previous Frame" ) );
            connect( fActionPrev, &QAction::triggered, this, &CBIFWidget::slotPrev );
        }
        return fActionPrev;
    }

    QAction *CBIFWidget::actionTogglePlayPause( std::optional< bool > asPlayButton )
    {
        if ( !fActionTogglePlayPause )
        {
            fActionTogglePlayPause = new QAction( this );
            fActionTogglePlayPause->setObjectName( QString::fromUtf8( "actionTogglePlayPause" ) );
            connect( fActionTogglePlayPause, &QAction::triggered, this, &CBIFWidget::slotTogglePlayPause );
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
            fActionPlay = new QAction( this );
            fActionPlay->setObjectName( QString::fromUtf8( "actionPlay" ) );
            connect( fActionPlay, &QAction::triggered, this, &CBIFWidget::slotPlay );
            setPlayPause( fActionPlay, true );
        }

        return fActionPlay;
    }

    QAction *CBIFWidget::actionPause()
    {
        if ( !fActionPause )
        {
            fActionPause = new QAction( this );
            fActionPause->setObjectName( QString::fromUtf8( "actionPause" ) );
            connect( fActionPause, &QAction::triggered, this, &CBIFWidget::slotPause );
            setPlayPause( fActionPause, false );
        }

        return fActionPause;
    }

    QAction *CBIFWidget::actionNext()
    {
        if ( !fActionNext )
        {
            fActionNext = new QAction( this );
            fActionNext->setObjectName( QString::fromUtf8( "actionNext" ) );
            setActionInfo( fActionNext, ":/BIFPlayerResources/next.png", tr( "Next Frame" ) );
            connect( fActionNext, &QAction::triggered, this, &CBIFWidget::slotNext );
        }
        return fActionNext;
    }

    QAction *CBIFWidget::actionSkipForward()
    {
        if ( !fActionSkipForward )
        {
            fActionSkipForward = new QAction( this );
            fActionSkipForward->setObjectName( QString::fromUtf8( "actionSkipForward" ) );
            setActionInfo( fActionSkipForward, ":/BIFPlayerResources/skipforward.png", tr( "Skip Forward" ) );
            connect( fActionSkipForward, &QAction::triggered, this, &CBIFWidget::slotSkipForward );
        }
        return fActionSkipForward;
    }

}