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

class QAction;
class QLabel;

namespace NBIF
{
    class CBIFFile;
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
        void setPlayPause( QAction *actin, bool playPause );

        void setActionInfo( QAction *action, const QString & iconPath, const QString & text );

        QTimer *fFrameTimer{ nullptr };
        std::optional< uint32_t > fCurrentFrame;
        std::shared_ptr< NBIF::CBIFFile > fBIF;

        QLabel *fImageLabel{ nullptr };
        QLabel *fTextLabel{ nullptr };

        QAction *fActionSkipBackward{ nullptr };
        QAction *fActionPrev{ nullptr };
        QAction *fActionTogglePlayPause{ nullptr };
        QAction *fActionPlay{ nullptr };
        QAction *fActionPause{ nullptr };
        QAction *fActionNext{ nullptr };
        QAction *fActionSkipForward{ nullptr };

        void enableAction( QAction * action, bool enable );
        int fSkipInterval{ 5 };
    };
}
#endif 
