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
#ifndef __STAYAWAKE_H
#define __STAYAWAKE_H
#include <QRunnable>
#include <QObject>

#include "SABUtilsExport.h"

#ifdef Q_OS_WINDOWS
namespace NSABUtils
{
    // by default QRunnable has autodelete, when its finished running it automatically deletes.
    // user can override this
    class SABUTILS_EXPORT CStayAwake : public QRunnable
    {
    public:
        CStayAwake( bool keepScreenOn ) :
            fKeepScreenOn( keepScreenOn ),
            QRunnable()
        {
        };

        virtual void run() override;

        void stop();

    private:
        bool setKeepAwake( bool enable ); // only part that is OS Specific
        bool fKeepScreenOn{ false };
        bool fStopped{ false };
    };

    class SABUTILS_EXPORT CAutoStayAwake : public QObject   // allows for delete later
    {
    public:
        CAutoStayAwake( bool keepScreenOn, QObject * parent=nullptr );
        ~CAutoStayAwake();
    private:
        CStayAwake *fStayAwake{ nullptr };
    };

}
#endif

#endif
