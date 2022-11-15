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

#include "BIFWidget.h"
#include "BIFFile.h"
#include "utils.h"
#include "QtUtils.h"

#include "GIFWriter.h"
#include "GIFWriterDlg.h"
#include "bif/BIFPlugin.h"

#include <QTimer>
#include <QIcon>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QSpinBox>
#include <QMovie>
#include <QDir>
#include <QPluginLoader>
#include <QLibrary>
#include <QFileDialog>

#include "ui_BIFWidget.h"

namespace NSABUtils
{
    namespace NBIF
    {
        CWidget::CWidget( QWidget * parent )
            : QFrame( parent ),
            fImpl( new NSABUtils::NBIF::Ui::CWidget )
        {
            Q_INIT_RESOURCE( BIFPlayerResources );
            fImpl->setupUi( this );

            fMovie = std::make_shared< QMovie >();
            connect( fMovie.get(), &QMovie::frameChanged, this, &CWidget::slotFrameChanged );
            connect( fMovie.get(), &QMovie::stateChanged, this, &CWidget::slotMovieStateChanged );
            fImpl->imageLabel->setMovie( fMovie.get() );

            setInfo( fImpl->skipBackwardDiscreteBtn, ":/BIFPlayerResources/skipbackward.png", tr( "Skip Backward" ), &CWidget::slotSkipBackard );
            setInfo( fImpl->prevDiscreteBtn, ":/BIFPlayerResources/prev.png", tr( "Previous Frame" ), &CWidget::slotPrev );
            setInfo( fImpl->playBtn, QString(), QString(), &CWidget::slotPlay );
            setPlayPause( fImpl->playBtn, true );
            setInfo( fImpl->pauseBtn, QString(), QString(), &CWidget::slotPause );
            setPlayPause( fImpl->pauseBtn, false );
            setInfo( fImpl->nextDiscreteBtn, ":/BIFPlayerResources/next.png", tr( "Next Frame" ), &CWidget::slotNext );
            setInfo( fImpl->skipForwardDiscreteBtn, ":/BIFPlayerResources/skipforward.png", tr( "Skip Forward" ), &CWidget::slotSkipForward );

            setInfo( fImpl->skipBackwardToggleBtn, ":/BIFPlayerResources/skipbackward.png", tr( "Skip Backward" ), &CWidget::slotSkipBackard );
            setInfo( fImpl->prevToggleBtn, ":/BIFPlayerResources/prev.png", tr( "Previous Frame" ), &CWidget::slotPrev );
            setInfo( fImpl->playPauseBtn, QString(), QString(), &CWidget::slotTogglePlayPause );
            setPlayPause( fImpl->playPauseBtn, true );
            setInfo( fImpl->nextToggleBtn, ":/BIFPlayerResources/next.png", tr( "Next Frame" ), &CWidget::slotNext );
            setInfo( fImpl->skipForwardToggleBtn, ":/BIFPlayerResources/skipforward.png", tr( "Skip Forward" ), &CWidget::slotSkipForward );

            layoutButtons();

            setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
        }

        CWidget::~CWidget()
        {
        }

        void CWidget::setNumFramesToSkip( int numFrames )
        {
            slotSetNumFramesToSkip( numFrames );
            if ( fNumFramesToSkipSB )
                fNumFramesToSkipSB->setValue( fNumFramesToSkip );
        }

        void CWidget::slotSetNumFramesToSkip( int numFrames )
        {
            fNumFramesToSkip = numFrames;
        }

        void CWidget::setSpeedMultiplier( int speedMultipler )
        {
            slotSetPlayerSpeed( speedMultipler );
            if ( fPlayerSpeedMultiplerSB )
                fPlayerSpeedMultiplerSB->setValue( speedMultipler );
        }

        int CWidget::playerSpeedMultiplier() const
        {
            return fMovie->speed() / 100;
        }

        void CWidget::slotSetPlayerSpeed( int speedMultipler )
        {
            bool wasPlaying = this->isPlaying();
            int delay = fMovie->nextFrameDelay();
            fMovie->setPaused( true );
            fMovie->setSpeed( speedMultipler * 100 );
            delay = fMovie->nextFrameDelay();
            if ( wasPlaying )
            {
                updateMovieSpeed();
                fMovie->setPaused( false );
            }
        }

        void CWidget::setPlayCount( int playCount )
        {
            slotSetPlayCount( playCount );
        }

        int CWidget::playCount() const
        {
            return fPlayCountSB ? fPlayCountSB->value() : -1;
        }
        void CWidget::slotSetPlayCount( int playCount )
        {
            if ( isPlaying() )
                return;
            setBIFPluginPlayCount( playCount );
            fMovie->stop();
            fMovie->jumpToFrame( 0 );
        }

        void CWidget::slotFrameChanged()
        {
            slotMovieStateChanged();
            if ( isValid() )
            {
                auto labelText = tr( "Frame #: %1 Time: %2" ).arg( fMovie->currentFrameNumber() ).arg( NSABUtils::CTimeString( fMovie->currentFrameNumber() * fBIF->tsMultiplier().fValue ).toString( "hh:mm:ss.zzz" ) );
                if ( isPlaying() )
                    labelText += tr( "\nPlaying at %3 fps" ).arg( computeFPS(), 5, 'f', 3 );

                fImpl->textLabel->setText( labelText );
                fImpl->textLabel->setVisible( true );
            }
            else
                fImpl->textLabel->setVisible( false );
        }

        void CWidget::slotMovieStateChanged()
        {
            if ( isPlaying() )
                emit sigPlayingStarted();
            validatePlayerActions( fMovie->currentFrameNumber() != -1 );
        }

        void CWidget::slotSkipBackard()
        {
            slotPause();
            offsetFrameBy( -fNumFramesToSkip );
        }

        void CWidget::slotPrev()
        {
            slotPause();
            offsetFrameBy( -1 );
        }

        void CWidget::slotTogglePlayPause()
        {
            if ( isPlaying() )
                slotPause();
            else
                slotPlay();
        }

        void CWidget::slotPause()
        {
            fMovie->setPaused( true );
            validatePlayerActions();
            slotFrameChanged();
        }

        void CWidget::slotPlay()
        {
            updateMovieSpeed();

            if ( fMovie->state() == QMovie::NotRunning )
                fMovie->jumpToFrame( 0 );
            fMovie->setPaused( false );
            validatePlayerActions();
        }

        void CWidget::slotNext()
        {
            slotPause();
            offsetFrameBy( 1 );
        }

        void CWidget::slotSkipForward()
        {
            slotPause();
            offsetFrameBy( fNumFramesToSkip );
        }

        void CWidget::setActive( bool isActive )
        {
            validatePlayerActions( isActive );
        }

        QString CWidget::fileName() const
        {
            return fMovie->fileName();
        }

        bool CWidget::isValid() const
        {
            return fBIF && fBIF->isValid() && fMovie->isValid();
        }

        void CWidget::validatePlayerActions( bool enabled )
        {
            bool aOK = ( enabled && fBIF && fBIF->isValid() && fMovie->isValid() );

            enableItem( fActionSkipBackward, aOK );
            enableItem( fActionPrev, aOK );
            enableItem( fActionPlay, aOK && !isPlaying() );
            enableItem( fActionPause, aOK && isPlaying() );
            enableItem( fActionTogglePlayPause, aOK );
            setPlayPause( fImpl->playPauseBtn, !isPlaying() );
            setPlayPause( fActionTogglePlayPause, !isPlaying() );
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

            enableItem( fPlayCountSB, aOK && !isPlaying() );
            checkItem( fActionDiscreteLayout, false );
            checkItem( fActionToggleLayout, false );
            checkItem( fActionNoLayout, false );
            enableItem( fActionSaveAsGIF, fBIF.operator bool() );

            if ( fButtonLayout == EButtonsLayout::eDiscretePlayPause )
                checkItem( fActionDiscreteLayout, true );
            else if ( fButtonLayout == EButtonsLayout::eTogglePlayPause )
                checkItem( fActionToggleLayout, true );
            else if ( fButtonLayout == EButtonsLayout::eNoButtons )
                checkItem( fActionNoLayout, true );
        }

        bool CWidget::isPlaying() const
        {
            return ( fMovie->state() == QMovie::Running );
        }

        void CWidget::offsetFrameBy( int offset )
        {
            auto curr = fMovie->currentFrameNumber();
            if ( curr < 0 )
                curr = 0;
            setCurrentFrame( curr + offset );
        }

        void CWidget::setCurrentFrame( int frame )
        {
            int sz = fMovie->frameCount();
            if ( frame < 0 )
                frame = sz + frame;
            if ( frame >= sz )
                frame = frame - sz;
            fMovie->jumpToFrame( frame );
        }

        void CWidget::clear()
        {
            fImpl->imageLabel->setPixmap( QPixmap() );
            fImpl->textLabel->setText( QString() );
            fImpl->textLabel->setVisible( false );
        }

        std::shared_ptr< CFile > CWidget::setFileName( const QString & fileName )
        {
            if ( fileName.isEmpty() || !QFileInfo( fileName ).exists() )
                fBIF.reset();
            else
                fBIF = std::make_shared< NBIF::CFile >( fileName, false );

            fMovie->setFileName( fBIF ? fBIF->fileName() : QString() );
            fImpl->imageLabel->setMovie( fMovie.get() );
            validatePlayerActions( fBIF && fBIF->isValid() && fMovie->isValid() );
            return fBIF;
        }

        QAction * CWidget::actionSkipBackward()
        {
            if ( !fActionSkipBackward )
            {
                fActionSkipBackward = createAction( "actionSkipBackward", ":/BIFPlayerResources/skipbackward.png", tr( "Skip Backward" ), &CWidget::slotSkipBackard );
            }
            return fActionSkipBackward;
        }

        QAction * CWidget::actionPrev()
        {
            if ( !fActionPrev )
            {
                fActionPrev = createAction( "actionPrev", ":/BIFPlayerResources/prev.png", tr( "Previous Frame" ), &CWidget::slotPrev );
            }
            return fActionPrev;
        }

        QAction * CWidget::actionTogglePlayPause( std::optional< bool > asPlayButton )
        {
            if ( !fActionTogglePlayPause )
            {
                fActionTogglePlayPause = createAction( "actionTogglePlayPause", QString(), QString(), &CWidget::slotTogglePlayPause );
                if ( !asPlayButton.has_value() )
                    setPlayPause( fActionTogglePlayPause, true );
            }
            if ( asPlayButton.has_value() )
                setPlayPause( fActionTogglePlayPause, asPlayButton.value() );

            return fActionTogglePlayPause;
        }

        QAction * CWidget::actionPlay()
        {
            if ( !fActionPlay )
            {
                fActionPlay = createAction( QString::fromUtf8( "actionPlay" ), QString(), QString(), &CWidget::slotPlay );
                setPlayPause( fActionPlay, true );
            }

            return fActionPlay;
        }

        QAction * CWidget::actionPause()
        {
            if ( !fActionPause )
            {
                fActionPause = createAction( QString::fromUtf8( "actionPause" ), QString(), QString(), &CWidget::slotPause );
                setPlayPause( fActionPause, false );
            }

            return fActionPause;
        }

        QAction * CWidget::actionNext()
        {
            if ( !fActionNext )
            {
                fActionNext = createAction( QString::fromUtf8( "actionNext" ), ":/BIFPlayerResources/next.png", tr( "Next Frame" ), &CWidget::slotNext );
            }
            return fActionNext;
        }

        QAction * CWidget::actionSkipForward()
        {
            if ( !fActionSkipForward )
            {
                fActionSkipForward = createAction( QString::fromUtf8( "actionSkipForward" ), ":/BIFPlayerResources/skipforward.png", tr( "Skip Forward" ), &CWidget::slotSkipForward );
            }
            return fActionSkipForward;
        }

        QAction * CWidget::actionDiscreteLayout()
        {
            if ( !fActionDiscreteLayout )
            {
                fActionDiscreteLayout = createAction( QString::fromUtf8( "actionDiscreteLayout" ), QString(), tr( "Discrete Play/Pause Buttons" ), &CWidget::slotPlayerButtonDiscrete );
                fActionDiscreteLayout->setCheckable( true );
            }
            return fActionDiscreteLayout;
        }

        QAction * CWidget::actionToggleLayout()
        {
            if ( !fActionToggleLayout )
            {
                fActionToggleLayout = createAction( QString::fromUtf8( "actionToggleLayout" ), QString(), tr( "Single Toggleable Play/Pause Button" ), &CWidget::slotPlayerButtonToggle );
                fActionToggleLayout->setCheckable( true );
            }
            return fActionToggleLayout;
        }

        QAction * CWidget::actionNoLayout()
        {
            if ( !fActionNoLayout )
            {
                fActionNoLayout = createAction( QString::fromUtf8( "actionNoLayout" ), QString(), tr( "No Player Buttons" ), &CWidget::slotPlayerButtonNone );
                fActionNoLayout->setCheckable( true );
            }
            return fActionNoLayout;
        }

        QAction * CWidget::actionSaveAsGIF()
        {
            if ( !fActionSaveAsGIF )
            {
                fActionSaveAsGIF = createAction( QString::fromUtf8( "actionSaveAsGIF" ), QString(), tr( "Save As GIF..." ), &CWidget::slotSaveAsGIF );
            }
            return fActionSaveAsGIF;
        }



        void CWidget::setButtonsLayout( EButtonsLayout layout )
        {
            if ( fButtonLayout == layout )
                return;

            fButtonLayout = layout;

            updateMenu();
            updateToolBar();
            layoutButtons();
            validatePlayerActions();
        }

        void CWidget::layoutButtons()
        {
            if ( fButtonLayout == EButtonsLayout::eNoButtons )
                fImpl->stackedWidget->setVisible( false );
            else
            {
                fImpl->stackedWidget->setVisible( true );
                fImpl->stackedWidget->setCurrentIndex( static_cast<int>( fButtonLayout ) );
            }
        }

        void CWidget::setInfo( QAction * item, const QString & iconPath, const QString & text, void ( CWidget:: * slot )( ) )
        {
            QIcon icon;
            icon.addFile( iconPath, QSize(), QIcon::Normal, QIcon::Off );
            item->setIcon( icon );
            item->setToolTip( text );
            item->setText( text );
            if ( slot )
                connect( item, &QAction::triggered, this, slot );
        }

        void CWidget::setInfo( QToolButton * item, const QString & iconPath, const QString & text, void ( CWidget:: * slot )( ) )
        {
            QIcon icon;
            icon.addFile( iconPath, QSize(), QIcon::Normal, QIcon::Off );
            item->setIcon( icon );
            item->setToolTip( text );
            item->setText( text );
            if ( slot )
                connect( item, &QToolButton::clicked, this, slot );
        }

        QAction * CWidget::createAction( const QString & name, const QString & iconPath, const QString & text, void ( CWidget:: * slot )( ) )
        {
            auto retVal = new QAction( this );
            retVal->setObjectName( name );
            setInfo( retVal, iconPath, text, slot );
            return retVal;
        }

        void CWidget::slotPlayerButtonDiscrete()
        {
            setButtonsLayout( EButtonsLayout::eDiscretePlayPause );
        }

        void CWidget::slotPlayerButtonToggle()
        {
            setButtonsLayout( EButtonsLayout::eTogglePlayPause );
        }

        void CWidget::slotPlayerButtonNone()
        {
            setButtonsLayout( EButtonsLayout::eNoButtons );
        }

        QToolBar * CWidget::toolBar()
        {
            if ( !fToolBar )
            {
                fToolBar = new QToolBar;
                fToolBar->setObjectName( "Thumbnail Viewer Toolbar" );
                fToolBar->setWindowTitle( tr( "Thumbnail Viewer Toolbar" ) );

                updateToolBar();
                validatePlayerActions();
            }
            return fToolBar;
        }

        QMenu * CWidget::menu()
        {
            if ( !fMenu )
            {
                fMenu = new QMenu( this );
                fMenu->setObjectName( "Thumbnail Viewer Menu" );
                fMenu->setTitle( tr( "Thumbnail Viewer" ) );

                updateMenu();
                validatePlayerActions();
            }
            return fMenu;
        }

        void CWidget::updateMenu()
        {
            if ( !fMenu )
                return;

            fMenu->clear();
            auto subMenu = fMenu->addMenu( tr( "BIF Player Type" ) );
            subMenu->addAction( actionDiscreteLayout() );
            subMenu->addAction( actionToggleLayout() );
            subMenu->addAction( actionNoLayout() );
            fMenu->addSeparator();
            fMenu->addAction( actionSaveAsGIF() );

            if ( fButtonLayout == EButtonsLayout::eNoButtons )
                return;

            updateItemForLayout( fMenu );
        }

        void CWidget::updateToolBar()
        {
            if ( !fToolBar )
                return;

            fToolBar->clear();

            updateItemForLayout( fToolBar );
            fToolBar->addSeparator();

            auto label = new QLabel( tr( "Player Speed Multiplier:" ) );
            label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
            fToolBar->addWidget( label );

            fPlayerSpeedMultiplerSB = new QSpinBox;
            fPlayerSpeedMultiplerSB->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
            fPlayerSpeedMultiplerSB->setSuffix( tr( "x" ) );
            fPlayerSpeedMultiplerSB->setMinimum( 0 );
            fPlayerSpeedMultiplerSB->setMaximum( 10000 );
            fPlayerSpeedMultiplerSB->setSingleStep( 50 );
            connect( fPlayerSpeedMultiplerSB, qOverload< int >( &QSpinBox::valueChanged ), this, &CWidget::slotSetPlayerSpeed );

            fPlayerSpeedMultiplerSB->setValue( playerSpeedMultiplier() );

            fToolBar->addWidget( fPlayerSpeedMultiplerSB ); // takes ownership
            fToolBar->addSeparator();

            label = new QLabel( tr( "Frames to Skip:" ) );
            label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
            fToolBar->addWidget( label );

            fNumFramesToSkipSB = new QSpinBox;
            fNumFramesToSkipSB->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
            fNumFramesToSkipSB->setSuffix( tr( " frames" ) );
            fNumFramesToSkipSB->setMinimum( 0 );
            fNumFramesToSkipSB->setMaximum( 1000 );
            fNumFramesToSkipSB->setSingleStep( 50 );
            fNumFramesToSkipSB->setValue( fNumFramesToSkip );
            connect( fNumFramesToSkipSB, qOverload< int >( &QSpinBox::valueChanged ), this, &CWidget::slotSetNumFramesToSkip );
            fToolBar->addWidget( fNumFramesToSkipSB );
            fToolBar->addSeparator();

            label = new QLabel( tr( "Number of Times to Play?" ) );
            label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
            fToolBar->addWidget( label );

            fPlayCountSB = new QSpinBox;
            fPlayCountSB->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
            //fPlayCountSB->setSuffix(tr(""));
            fPlayCountSB->setSpecialValueText( "Inifinite" );
            fPlayCountSB->setMinimum( 0 );
            fPlayCountSB->setMaximum( std::numeric_limits< int >::max() );
            fPlayCountSB->setSingleStep( 1 );
            fPlayCountSB->setValue( 0 );
            connect( fPlayCountSB, qOverload< int >( &QSpinBox::valueChanged ), this, &CWidget::slotSetPlayCount );
            fToolBar->addWidget( fPlayCountSB );
            fToolBar->addSeparator();

        }

        template< typename T>
        void CWidget::updateItemForLayout( T * item )
        {
            if ( fButtonLayout == EButtonsLayout::eNoButtons )
                return;
            auto actions = item->actions();
            if ( !actions.isEmpty() && !actions.back()->isSeparator() )
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
        void CWidget::setPlayPause( T * item, bool playPause )
        {
            if ( !item )
                return;

            if ( playPause )
                setInfo( item, ":/BIFPlayerResources/play.png", tr( "Play" ), nullptr );
            else
                setInfo( item, ":/BIFPlayerResources/pause.png", tr( "Pause" ), nullptr );
        }

        template< typename T >
        void CWidget::checkItem( T * item, bool checked )
        {
            if ( !item )
                return;
            item->setChecked( checked );
        }

        template< typename T >
        void CWidget::setItemVisible( T * item, bool visible )
        {
            if ( !item )
                return;
            item->setVisible( visible );
        }

        template< typename T >
        void CWidget::enableItem( T * item, bool enable )
        {
            if ( !item )
                return;
            item->setEnabled( enable );
        }

        void CWidget::setBIFPluginPlayCount( int playCount )
        {
            QDir pluginsDir( QCoreApplication::applicationDirPath() );
            pluginsDir.cd( "imageformats" );
            const QStringList entries = pluginsDir.entryList( QDir::Files );
            for ( const QString & fileName : entries )
            {
                if ( fileName.startsWith( "bif" ) )
                {
                    auto absPath = pluginsDir.absoluteFilePath( fileName );
                    QPluginLoader pluginLoader( absPath );
                    auto bifPlugin = dynamic_cast<CPlugin *>( pluginLoader.instance() );
                    if ( bifPlugin )
                    {
                        QLibrary lib( absPath );
                        using TSetLoopCount = void( * )( int );

                        auto setLoopCount = (TSetLoopCount)lib.resolve( "setLoopCount" );
                        if ( setLoopCount )
                            setLoopCount( playCount );
                    }
                    break;
                }
            }
        }

        double CWidget::computeFPS() const
        {
            if ( !fBIF )
                return 0;
            auto baseMSecPerFrame = 1.0 * fBIF->imageDelay();
            auto scaledMSecPerFrame = baseMSecPerFrame * 100 / fMovie->speed();

            auto secPerFrame = scaledMSecPerFrame / 1000;
            auto fps = 1.0 / ( 1.0 * secPerFrame );
            return fps;
        }

        void CWidget::updateMovieSpeed()
        {
            auto curr = fMovie->currentFrameNumber();
            fMovie->jumpToFrame( curr + 1 );
            fMovie->jumpToFrame( curr );
        }

        QSize CWidget::sizeHint() const
        {
            if ( !isValid() )
                return QWidget::sizeHint();

            auto retVal = fBIF->imageSize();
            retVal += QSize( 6, 3 );
            if ( fImpl->textLabel->isVisible() )
            {
                auto subSizeHint = fImpl->textLabel->sizeHint();
                retVal.setWidth( std::max( subSizeHint.width(), retVal.width() ) );
                retVal.setHeight( retVal.height() + subSizeHint.height() );
                retVal += QSize( 0, 3 );
            }
            auto subSizeHint = fImpl->skipBackwardToggleBtn->sizeHint();
            retVal.setWidth( std::max( 6 * subSizeHint.width(), retVal.width() ) );
            retVal.setHeight( retVal.height() + subSizeHint.height() );
            return retVal;
        }

        void CWidget::slotSaveAsGIF()
        {
            if ( !fBIF || ( fBIF->imageCount() == 0 ) )
                return;

            CGIFWriterDlg dlg( this );
            dlg.setBIF( fBIF );
            dlg.setSpeedMultipler( fPlayerSpeedMultiplerSB->value() );
            dlg.setDelay( gifDelay() );
            dlg.setFlipImage( gifFlipImage() );
            dlg.setLoopCount( gifLoopCount() );
            dlg.setDither( gifDitherImage() );
            dlg.setStartFrame( gifStartFrame() );
            dlg.setEndFrame( gifEndFrame() );
            dlg.setUseNew( true );
            if ( dlg.exec() == QDialog::Accepted )
            {
                setGIFDitherImage( dlg.dither() );
                setGIFFlipImage( dlg.flipImage() );
                setGIFLoopCount( dlg.loopCount() );
                setGIFStartFrame( dlg.startFrame() );
                setGIFEndFrame( dlg.endFrame() );
                setGIFDelay( dlg.delay() );
            }
        }
    }
}