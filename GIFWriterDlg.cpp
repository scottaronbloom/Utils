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

#include "GIFWriterDlg.h"
#include "BIFFile.h"
#include "GIFWriter.h"
#include "gif/gif-h/gif.h"
#include "QtUtils.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QProgressDialog>

#include "ui_GIFWriterDlg.h"

namespace NUtils
{
    CGIFWriterDlg::CGIFWriterDlg( std::shared_ptr< NBIF::CBIFFile > bifFile, int delayInMSec, QWidget * parent ) :
        QDialog( parent ),
        fImpl( new Ui::CGIFWriterDlg )
    {
        Q_INIT_RESOURCE( BIFPlayerResources );
        fImpl->setupUi( this );

        connect( fImpl->selectGIFFile, &QToolButton::clicked, this, &CGIFWriterDlg::slotSelectGIFFile );
        connect( fImpl->startFrame, qOverload< int >( &QSpinBox::valueChanged ), this, &CGIFWriterDlg::slotStartFrameChanged );
        connect( fImpl->endFrame, qOverload< int >( &QSpinBox::valueChanged ), this, &CGIFWriterDlg::slotEndFrameChanged );
        setBIF( bifFile );
        setDelay( delayInMSec );
        setDither( true );
        setFlipImage( false );
        setLoopCount( 0 );

        connect( fImpl->useNew, &QCheckBox::clicked, this, &CGIFWriterDlg::slotUpdateFileName );
#ifndef _DEBUG
        fImpl->useNew->setVisible( false );
#endif
    }

    CGIFWriterDlg::CGIFWriterDlg( std::shared_ptr< NBIF::CBIFFile > bifFile, QWidget * parent ) :
        CGIFWriterDlg( bifFile, 2, parent )
    {
    }

    CGIFWriterDlg::CGIFWriterDlg( QWidget * parent ) :
        CGIFWriterDlg( {}, parent )
    {
    }

    CGIFWriterDlg::~CGIFWriterDlg()
    {
    }

    void CGIFWriterDlg::setBIF( std::shared_ptr< NBIF::CBIFFile > bifFile )
    {
        fBIF = bifFile;
        fImpl->startFrame->setValue( 1 );
        fImpl->startFrame->setMinimum( 1 );
        fImpl->startFrame->setMaximum( 1 );
        fImpl->endFrame->setMinimum( 1 );
        fImpl->endFrame->setValue( 1 );
        fImpl->endFrame->setMaximum( 1 );
        if ( fBIF )
        {
            fImpl->endFrame->setMaximum( (int)fBIF->imageCount() );
            fImpl->endFrame->setValue( (int)fBIF->imageCount() );
            slotUpdateFileName();
            updateDelay();
        }
    }

    void CGIFWriterDlg::slotUpdateFileName()
    {
        QString fn;
        if ( fBIF )
        {
            auto fi = QFileInfo( fBIF->fileName() );
            fn = QFileInfo( fi.absolutePath() + "/" + fi.completeBaseName() ).absoluteFilePath();
            if ( !useNew() )
                fn += ".old";
            fn += ".gif";
        }

        fImpl->fileName->setText( fn );
    }

    void CGIFWriterDlg::slotStartFrameChanged()
    {
        fImpl->endFrame->setMinimum( fImpl->startFrame->value() );
        if ( fImpl->startFrame->value() >= fImpl->endFrame->value() )
            fImpl->endFrame->setValue( fImpl->startFrame->value() );
    }

    void CGIFWriterDlg::slotEndFrameChanged()
    {
        fImpl->startFrame->setMaximum( fImpl->endFrame->value() );
        if ( fImpl->endFrame->value() <= fImpl->startFrame->value() )
            fImpl->endFrame->setValue( fImpl->startFrame->value() );
    }

    void CGIFWriterDlg::setSpeedMultipler( int multiplier )
    {
        fMultipler = multiplier;
        updateDelay();
    }

    void CGIFWriterDlg::setDelay( int delay )
    {
        if ( delay != -1 )
            fImpl->delay->setValue( delay );
    }

    int CGIFWriterDlg::delay() const
    {
        return fImpl->delay->value();
    }

    void CGIFWriterDlg::updateDelay()
    {
        if ( !fBIF )
            return;
        auto delay = fBIF->imageDelay(); // delay per frame;
        delay /= fMultipler;
        setDelay( delay );
    }

    void CGIFWriterDlg::setDither( bool dither )
    {
        fImpl->dither->setChecked( dither );
    }

    bool CGIFWriterDlg::dither() const
    {
        return fImpl->dither->isChecked();
    }

    void CGIFWriterDlg::setFlipImage( bool flipImage )
    {
        fImpl->flipImage->setChecked( flipImage );
    }

    bool CGIFWriterDlg::flipImage() const
    {
        return fImpl->flipImage->isChecked();
    }

    void CGIFWriterDlg::setLoopCount( int loopCount )
    {
        fImpl->loopCount->setValue( loopCount );
    }

    int CGIFWriterDlg::loopCount() const
    {
        return fImpl->loopCount->value();
    }

    void CGIFWriterDlg::slotSelectGIFFile()
    {
        auto fn = QFileDialog::getSaveFileName( this, tr( "GIF File Name" ), fImpl->fileName->text(), tr( "GIF Files (*.gif);;All Files (*.*)" ) );
        if ( fn.isEmpty() )
            return;
        fImpl->fileName->setText( fn );
    }

    void CGIFWriterDlg::accept()
    {
        if ( saveToGIF() )
            QDialog::accept();
    }

    int CGIFWriterDlg::numFramesToSave() const
    {
        return endFrame() - startFrame() + 1;
    }

    void CGIFWriterDlg::setStartFrame( int startFrame )
    {
        if ( startFrame == -1 )
            return;
        return fImpl->startFrame->setValue( startFrame );
    }

    int CGIFWriterDlg::startFrame() const
    {
        return fImpl->startFrame->value();
    }

    void CGIFWriterDlg::setEndFrame( int endFrame )
    {
        if ( endFrame == -1 )
            return;
        return fImpl->endFrame->setValue( endFrame );
    }

    int CGIFWriterDlg::endFrame() const
    {
        return fImpl->endFrame->value();
    }

    bool  CGIFWriterDlg::useNew() const
    {
        return fImpl->useNew->isChecked();
    }

    void CGIFWriterDlg::setUseNew( bool useNew )
    {
        fImpl->useNew->setChecked( useNew );
        slotUpdateFileName();
    }

    bool CGIFWriterDlg::saveToGIF()
    {
        auto fn = fImpl->fileName->text();
        if ( fn.isEmpty() )
            return true;

        if ( !fBIF )
            return false;

        if ( QFileInfo( fn ).exists() )
        {
            if ( QMessageBox::warning( this, tr( "File already exists" ), tr( "'%1' already exists, are you sure?" ).arg( QFileInfo( fn ).fileName() ), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
            {
                return false;
            }
        }

        std::unique_ptr< CGIFWriter > newWriter;
        std::unique_ptr< GifWriter > oldWriter;
        if ( fImpl->useNew->isChecked() )
        {
            newWriter = std::make_unique< CGIFWriter >( fn );
            newWriter->setDither( dither() );
            newWriter->setFlipImage( flipImage() );
            newWriter->setBitDepth( 8 );
            newWriter->setLoopCount( loopCount() );
            newWriter->setDelay( delay() );
        }
        else
        {
            oldWriter = std::make_unique< GifWriter >();
            oldWriter->firstFrame = false;
            oldWriter->f = nullptr;
            oldWriter->oldImage = nullptr;
            GifBegin( oldWriter.get(), qPrintable( fn ), fBIF->imageSize().width(), fBIF->imageSize().height(), delay(), 8, dither(), loopCount() );
        }

        bool wasCancelled = false;
        QProgressDialog dlg( this );
        dlg.setLabelText( tr( "Generating GIF" ) );
        dlg.setMinimum( startFrame() );
        dlg.setMaximum( endFrame() );
        dlg.setWindowModality( Qt::WindowModal );
        dlg.show();

        for ( int ii = startFrame() - 1; !wasCancelled && ii <= endFrame() - 1; ++ii )
        {
            dlg.setValue( ii );
            QApplication::processEvents();

            wasCancelled = dlg.wasCanceled();

            auto image = fBIF->image( ii );
            if ( image.isNull() )
            {
                if ( QMessageBox::warning( this, tr( "BIF File has empty image" ), tr( "Image #%1 is null, skipped.  Continue?" ).arg( ii ), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
                    return false;
            }

            wasCancelled = dlg.wasCanceled();
            if ( newWriter )
            {
                if ( !newWriter->writeImage( image, false ) )
                {
                    if ( QMessageBox::warning( this, tr( "Problem writing frame" ), tr( "Image #%1 was not written.  Continue?" ).arg( ii ), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
                        return false;
                }
            }
            else
            {
                auto imageData = NQtUtils::imageToPixels( image );
                GifWriteFrame( oldWriter.get(), imageData, image.width(), image.height(), delay(), flipImage(), 8, dither() );
                delete[] imageData;
            }
        }

        if ( newWriter )
            newWriter->writeEnd();
        else
            GifEnd( oldWriter.get() );
        if ( wasCancelled )
        {
            QFile::remove( fn );
        }
        return !wasCancelled;
    }
}

