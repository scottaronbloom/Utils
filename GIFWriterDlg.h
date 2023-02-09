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

#ifndef _GIFWRITERDLG_H
#define _GIFWRITERDLG_H

#include "SABUtilsExport.h"

class QProgressDialog;
class QFileInfo;

#include <QDialog>
#include <memory>

namespace NSABUtils
{
    namespace NBIF
    {
        class CFile;
    }

    namespace Ui { class CGIFWriterDlg; }
    class SABUTILS_EXPORT CGIFWriterDlg : public QDialog
    {
        Q_OBJECT
    public:
        CGIFWriterDlg( QWidget * parent = nullptr );
        CGIFWriterDlg::CGIFWriterDlg( std::shared_ptr< NBIF::CFile > bifFile, QWidget * parent = nullptr );
        CGIFWriterDlg::CGIFWriterDlg( std::shared_ptr< NBIF::CFile > bifFile, int delayInMSec, QWidget * parent = nullptr );

        ~CGIFWriterDlg();

        static bool saveToGIF( QWidget * parent, const QString & fileName, const QList< QFileInfo > images, int startFrame, int endFrame, bool dither, bool flipImage, int loopCount, int delay, std::function< void( size_t min, size_t max ) > setRange, std::function< void( size_t curr ) > setCurr, std::function< bool() > wasCancelled );
        static bool saveToGIF( QWidget * parent, const QString & fileName, const QList< QFileInfo > images, bool dither, bool flipImage, int loopCount, int delay, std::function< void( size_t min, size_t max ) > setRange, std::function< void( size_t curr ) > setCurr, std::function< bool() > wasCancelled );
        static bool saveToGIF( QWidget * parent, const QString & fileName, const QList< QImage > images, int startFrame, int endFrame, bool dither, bool flipImage, int loopCount, int delay, std::function< void( size_t min, size_t max ) > setRange, std::function< void( size_t curr ) > setCurr, std::function< bool() > wasCancelled );
        static bool saveToGIF( QWidget * parent, const QString & fileName, const QList< QImage > images, bool dither, bool flipImage, int loopCount, int delay, std::function< void( size_t min, size_t max ) > setRange, std::function< void( size_t curr ) > setCurr, std::function< bool() > wasCancelled );

        void setBIF( std::shared_ptr< NBIF::CFile > bifFile );
        std::shared_ptr< NBIF::CFile > bifFile() const { return fBIF; }

        // speed multiplier uses the delay built into the BIF file
        void setSpeedMultipler( int multiplier ); // delay or multiplier (setting the bif as well) override each other, last one set wins
        void setDelay( int msec );
        int delay() const;

        void setDither( bool dither );
        bool dither() const;

        void setFlipImage( bool flipImage );
        bool flipImage() const;

        void setLoopCount( int loopCount );
        int loopCount() const;

        void setStartFrame( int startFrame );
        int startFrame() const;

        void setEndFrame( int endFrame );
        int endFrame() const;

        virtual void accept() override;
        int numFramesToSave() const;

        void setUseNew( bool useNew );
        bool useNew() const;
    Q_SIGNALS:
    public Q_SLOTS:
        void slotSelectGIFFile();
        void slotStartFrameChanged();
        void slotEndFrameChanged();
    private Q_SLOTS:
        void slotUpdateFileName();
    private:
        void updateDelay();
        bool saveToGIF();

        int fMultipler{ 1 };

        std::shared_ptr< NBIF::CFile > fBIF;
        std::unique_ptr< Ui::CGIFWriterDlg > fImpl;
    };
}
#endif 
