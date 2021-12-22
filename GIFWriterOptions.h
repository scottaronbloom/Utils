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

#ifndef _GIFWRITEROPTIONS_H
#define _GIFWRITEROPTIONS_H

#include <QDialog>
#include <memory>

namespace NBIF
{
    class CBIFFile;
}

namespace Ui { class CGIFWriterOptions; }
namespace NUtils
{
    class CGIFWriterOptions : public QDialog
    {
        Q_OBJECT
    public:
        CGIFWriterOptions( QWidget * parent = nullptr);
        CGIFWriterOptions::CGIFWriterOptions( std::shared_ptr< NBIF::CBIFFile > bifFile, QWidget * parent=nullptr );
        CGIFWriterOptions::CGIFWriterOptions( std::shared_ptr< NBIF::CBIFFile > bifFile, int delayInMSec, QWidget * parent=nullptr );

        ~CGIFWriterOptions();

        void setSpeedMultipler( int multiplier );

        void setBIF( std::shared_ptr< NBIF::CBIFFile > bifFile );
        std::shared_ptr< NBIF::CBIFFile > bifFile() const { return fBIF; }

        void setDelay( int msec );
        int delay() const;

        void setDither( bool dither );
        bool dither() const;

        void setFlipImage( bool flipImage );
        bool flipImage() const;

        void setLoopCount( int loopCount );
        int loopCount() const;

        virtual void accept() override;

        int numFramesToSave() const;

        int startFrame() const;
        int endFrame() const;
    Q_SIGNALS:
    public Q_SLOTS:
        void slotSelectGIFFile();
        void slotStartFrameChanged();
        void slotEndFrameChanged();
    private Q_SLOTS:
    private:
        void updateDelay();
        bool saveToGIF();

        int fMultipler{ 1 };

        std::shared_ptr< NBIF::CBIFFile > fBIF;
        std::unique_ptr< Ui::CGIFWriterOptions > fImpl;
    };
}
#endif 
