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


namespace NBIF
{
    CBIFWidget::CBIFWidget( QWidget *parent )
        : QFrame( parent ),
        fImpl( new Ui::CBIFWidget )
    {
        Q_INIT_RESOURCE( BIFPlayerResources );
        fImpl->setupUi( this );

        fMovie = std::make_shared< QMovie >();
        connect(fMovie.get(), &QMovie::frameChanged, this, &CBIFWidget::slotFrameChanged);
        connect(fMovie.get(), &QMovie::stateChanged, this, &CBIFWidget::slotMovieStateChanged);
        fImpl->imageLabel->setMovie(fMovie.get());

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
    }

    CBIFWidget::~CBIFWidget()
    {
    }

    void CBIFWidget::setNumFramesToSkip( int numFrames )
    {
        slotSetNumFramesToSkip(numFrames);
        if ( fNumFramesToSkipSB )
            fNumFramesToSkipSB->setValue( fNumFramesToSkip );
    }

    void CBIFWidget::slotSetNumFramesToSkip( int numFrames )
    {
        fNumFramesToSkip = numFrames;
    }

    void CBIFWidget::setSpeedMultiplier( int speedMultipler )
    {
        slotSetPlayerSpeed(speedMultipler);
        if (fPlayerSpeedMultiplerSB)
            fPlayerSpeedMultiplerSB->setValue(speedMultipler);
    }

    int CBIFWidget::playerSpeedMultiplier() const
    {
        return fMovie->speed() / 100;
    }

    void CBIFWidget::slotSetPlayerSpeed( int speedMultipler)
    {
        bool wasPlaying = this->isPlaying();
        int delay = fMovie->nextFrameDelay();
        fMovie->setPaused(true);
        fMovie->setSpeed(speedMultipler * 100);
        delay = fMovie->nextFrameDelay();
        if (wasPlaying)
        {
            updateMovieSpeed();
            fMovie->setPaused(false);
        }
    }

    void CBIFWidget::setPlayCount( int playCount )
    {
        slotSetPlayCount(playCount);
    }

    int CBIFWidget::playCount() const
    {
        return fPlayCountSB ? fPlayCountSB->value() : -1;
    }
    void CBIFWidget::slotSetPlayCount(int playCount)
    {
        if ( isPlaying() )
            return;
        setBIFPluginPlayCount(playCount);
        fMovie->stop();
        fMovie->jumpToFrame(0);
    }

    void CBIFWidget::slotFrameChanged()
    {
        slotMovieStateChanged();
        if (isValid())
        {
            auto labelText = tr("Frame #: %1 Time: %2").arg(fMovie->currentFrameNumber()).arg(NUtils::CTimeString(fMovie->currentFrameNumber() * std::get< 2 >(fBIF->tsMultiplier())).toString("hh:mm:ss.zzz"));
            if (isPlaying())
                labelText += tr("\nPlaying at %3 fps").arg(computeFPS(), 5, 'f', 3);

            fImpl->textLabel->setText(labelText);
            fImpl->textLabel->setVisible(true);
        }
        else
            fImpl->textLabel->setVisible(false);
    }

    void CBIFWidget::slotMovieStateChanged()
    {
        if (isPlaying())
            emit sigPlayingStarted();
        validatePlayerActions(fMovie->currentFrameNumber() != -1);
    }

    void CBIFWidget::slotSkipBackard()
    {
        slotPause();
        offsetFrameBy(-fNumFramesToSkip);
    }

    void CBIFWidget::slotPrev()
    {
        slotPause();
        offsetFrameBy(-1);
    }

    void CBIFWidget::slotTogglePlayPause()
    {
        if (isPlaying())
            slotPause();
        else
            slotPlay();
    }

    void CBIFWidget::slotPause()
    {
        fMovie->setPaused(true);
        validatePlayerActions();
        slotFrameChanged();
    }

    void CBIFWidget::slotPlay()
    {
        updateMovieSpeed();

        if (fMovie->state() == QMovie::NotRunning)
            fMovie->jumpToFrame(0);
        fMovie->setPaused(false);
        validatePlayerActions();
    }

    void CBIFWidget::slotNext()
    {
        slotPause();
        offsetFrameBy(1);
    }

    void CBIFWidget::slotSkipForward()
    {
        slotPause();
        offsetFrameBy(fNumFramesToSkip);
    }

    void CBIFWidget::setActive( bool isActive )
    {
        if ( fToolBar )
            fToolBar->setVisible( isActive );
        validatePlayerActions( isActive );
    }

    QString CBIFWidget::fileName() const
    {
        return fMovie->fileName();
    }

    bool CBIFWidget::isValid() const
    {
        return fBIF && fBIF->isValid() && fMovie->isValid();
    }

    void CBIFWidget::validatePlayerActions(bool enabled)
    {
        bool aOK = ( enabled && fBIF && fBIF->isValid() && fMovie->isValid() );

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

        enableItem(fPlayCountSB, aOK && !isPlaying() );
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

    bool CBIFWidget::isPlaying() const
    {
        return (fMovie->state() == QMovie::Running);
    }

    void CBIFWidget::offsetFrameBy(int offset)
    {
        auto curr = fMovie->currentFrameNumber();
        if (curr < 0)
            curr = 0;
        setCurrentFrame(curr + offset);
    }

    void CBIFWidget::setCurrentFrame( int frame )
    {
        int sz = fMovie->frameCount();
        if (frame < 0)
            frame = sz + frame;
        if (frame >= sz)
            frame = frame - sz;
        fMovie->jumpToFrame(frame);
    }

    void CBIFWidget::clear()
    {
        fImpl->imageLabel->setPixmap( QPixmap() );
        fImpl->textLabel->setText( QString() );
        fImpl->textLabel->setVisible( false );
    }

    std::shared_ptr< CBIFFile > CBIFWidget::setFileName( const QString & fileName )
    {
        if (fileName.isEmpty())
            fBIF.reset();
        else
            fBIF = std::make_shared< NBIF::CBIFFile >(fileName, false);

        fMovie->setFileName( fBIF ? fBIF->fileName() : QString() );
        fImpl->imageLabel->setMovie(fMovie.get());
        validatePlayerActions( fBIF && fBIF->isValid() && fMovie->isValid() );
        return fBIF;
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

    QAction * CBIFWidget::actionSaveAsGIF()
    {
        if ( !fActionSaveAsGIF )
        {
            fActionSaveAsGIF = createAction( QString::fromUtf8( "actionSaveAsGIF" ), QString(), tr( "Save As GIF..." ), &CBIFWidget::slotSaveAsGIF );
        }
        return fActionSaveAsGIF;
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
        setButtonsLayout( EButtonsLayout::eDiscretePlayPause );
    }

    void CBIFWidget::slotPlayerButtonToggle()
    {
        setButtonsLayout( EButtonsLayout::eTogglePlayPause );
    }

    void CBIFWidget::slotPlayerButtonNone()
    {
        setButtonsLayout( EButtonsLayout::eNoButtons );
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
        fMenu->addSeparator();
        fMenu->addAction( actionSaveAsGIF() );

        if ( fButtonLayout == EButtonsLayout::eNoButtons )
            return;

        updateItemForLayout( fMenu );
    }

    void CBIFWidget::updateToolBar()
    {
        if (!fToolBar)
            return;

        fToolBar->clear();

        auto label = new QLabel(tr("Player Speed Multiplier:"));
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        fToolBar->addWidget(label);

        fPlayerSpeedMultiplerSB = new QSpinBox;
        fPlayerSpeedMultiplerSB->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        fPlayerSpeedMultiplerSB->setSuffix(tr("x"));
        fPlayerSpeedMultiplerSB->setMinimum(0);
        fPlayerSpeedMultiplerSB->setMaximum(10000);
        fPlayerSpeedMultiplerSB->setSingleStep(50);
        connect(fPlayerSpeedMultiplerSB, qOverload< int >(&QSpinBox::valueChanged), this, &CBIFWidget::slotSetPlayerSpeed);

        fPlayerSpeedMultiplerSB->setValue(playerSpeedMultiplier());

        fToolBar->addWidget(fPlayerSpeedMultiplerSB); // takes ownership
        fToolBar->addSeparator();

        label = new QLabel(tr("Frames to Skip:"));
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        fToolBar->addWidget(label);

        fNumFramesToSkipSB = new QSpinBox;
        fNumFramesToSkipSB->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        fNumFramesToSkipSB->setSuffix(tr(" frames"));
        fNumFramesToSkipSB->setMinimum(0);
        fNumFramesToSkipSB->setMaximum(1000);
        fNumFramesToSkipSB->setSingleStep(50);
        fNumFramesToSkipSB->setValue(fNumFramesToSkip);
        connect(fNumFramesToSkipSB, qOverload< int >(&QSpinBox::valueChanged), this, &CBIFWidget::slotSetNumFramesToSkip);
        fToolBar->addWidget(fNumFramesToSkipSB);
        fToolBar->addSeparator();

        label = new QLabel(tr("Number of Times to Play?"));
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        fToolBar->addWidget(label);

        fPlayCountSB = new QSpinBox;
        fPlayCountSB->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        //fPlayCountSB->setSuffix(tr(""));
        fPlayCountSB->setSpecialValueText("Inifinite");
        fPlayCountSB->setMinimum(0);
        fPlayCountSB->setMaximum(std::numeric_limits< int >::max());
        fPlayCountSB->setSingleStep(1);
        fPlayCountSB->setValue(0);
        connect(fPlayCountSB, qOverload< int >(&QSpinBox::valueChanged), this, &CBIFWidget::slotSetPlayCount);
        fToolBar->addWidget(fPlayCountSB);
        fToolBar->addSeparator();

        updateItemForLayout(fToolBar);
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
    void CBIFWidget::setPlayPause( T *item, bool playPause )
    {
        if ( !item )
            return;

        if ( playPause )
            setInfo( item, ":/BIFPlayerResources/play.png", tr( "Play" ), nullptr );
        else
            setInfo( item, ":/BIFPlayerResources/pause.png", tr( "Pause" ), nullptr );
    }

    template< typename T >
    void CBIFWidget::checkItem( T *item, bool checked )
    {
        if ( !item )
            return;
        item->setChecked( checked );
    }

    template< typename T >
    void CBIFWidget::setItemVisible( T *item, bool visible )
    {
        if ( !item )
            return;
        item->setVisible( visible );
    }

    template< typename T >
    void CBIFWidget::enableItem( T *item, bool enable )
    {
        if ( !item )
            return;
        item->setEnabled( enable );
    }

    void CBIFWidget::setBIFPluginPlayCount(int playCount)
    {
        QDir pluginsDir(QCoreApplication::applicationDirPath());
        pluginsDir.cd("imageformats");
        const QStringList entries = pluginsDir.entryList(QDir::Files);
        for (const QString & fileName : entries)
        {
            if (fileName.startsWith("bif"))
            {
                auto absPath = pluginsDir.absoluteFilePath(fileName);
                QPluginLoader pluginLoader(absPath);
                auto bifPlugin = dynamic_cast<CBIFPlugin *>(pluginLoader.instance());
                if (bifPlugin)
                {
                    QLibrary lib(absPath);
                    using TSetLoopCount = void(*)(int);

                    auto setLoopCount = (TSetLoopCount)lib.resolve("setLoopCount");
                    if (setLoopCount)
                        setLoopCount(playCount);
                }
                break;
            }
        }
    }

    double CBIFWidget::computeFPS() const
    {
        if (!fBIF)
            return 0;
        auto baseMSecPerFrame = 1.0 * fBIF->imageDelay();
        auto scaledMSecPerFrame = baseMSecPerFrame * 100 / fMovie->speed();

        auto secPerFrame = scaledMSecPerFrame / 1000;
        auto fps = 1.0 / (1.0 * secPerFrame);
        return fps;
    }

    void CBIFWidget::updateMovieSpeed()
    {
        auto curr = fMovie->currentFrameNumber();
        fMovie->jumpToFrame(curr + 1);
        fMovie->jumpToFrame(curr);
    }

    QSize CBIFWidget::sizeHint() const
    {
        if (!isValid())
            return QWidget::sizeHint();

        auto retVal = fBIF->imageSize();
        retVal += QSize(6, 3);
        if (fImpl->textLabel->isVisible())
        {
            auto subSizeHint = fImpl->textLabel->sizeHint();
            retVal.setWidth(std::max(subSizeHint.width(), retVal.width()));
            retVal.setHeight(retVal.height() + subSizeHint.height());
            retVal += QSize(0, 3);
        }
        auto subSizeHint = fImpl->skipBackwardToggleBtn->sizeHint();
        retVal.setWidth(std::max(6*subSizeHint.width(), retVal.width()));
        retVal.setHeight(retVal.height() + subSizeHint.height());
        return retVal;
    }

    void CBIFWidget::slotSaveAsGIF()
    {
        if ( !fBIF || (fBIF->imageCount() == 0) )
            return;

        NUtils::CGIFWriterDlg dlg( this );
        dlg.setBIF( fBIF );
        dlg.setSpeedMultipler( fPlayerSpeedMultiplerSB->value() );
        dlg.setFlipImage( gifFlipImage() );
        dlg.setLoopCount( gifLoopCount() );
        dlg.setDither( gifDitherImage() );
        dlg.setUseNew( true );
        if ( dlg.exec() == QDialog::Accepted )
        {
            setGIFDitherImage( dlg.dither() );
            setGIFFlipImage( dlg.flipImage() );
            setGIFLoopCount( dlg.loopCount() );
        }
    }
}
