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

#include "StayAwake.h"
#include <QThread>
#include <QThreadPool>

#ifdef Q_OS_WINDOWS
    #include <qt_windows.h>
#endif

namespace NSABUtils
{
    void CStayAwake::run()
    {
        fStopped = false;
        bool success = false;
        while ( !fStopped )
        {
            try
            {
                success = setKeepAwake( true );
            }
            catch ( ... )
            {
                success = false;
            }
            if ( !success )
                break;
            QThread::sleep( 2 );
        }
        try
        {
            if ( success )
                setKeepAwake( false );
        }
        catch ( ... )
        {
        }
    }

#ifdef Q_OS_WINDOWS
    bool CStayAwake::setKeepAwake( bool enable )
    {
        bool retVal = false;
        if ( enable )
        {
            if ( fKeepScreenOn )
                retVal = SetThreadExecutionState( ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED );
            else
                retVal = SetThreadExecutionState( ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED );
        }
        else
            retVal = SetThreadExecutionState( ES_CONTINUOUS );

        Q_ASSERT( retVal );
        return retVal;
    }
#else
#endif

    void CStayAwake::stop()
    {
        fStopped = true;
    }

    CAutoStayAwake::CAutoStayAwake( bool keepScreenOn, QObject *parent ) :
        QObject( parent )
    {
        if ( !fStayAwake )
            fStayAwake = new NSABUtils::CStayAwake( true );

        QThreadPool::globalInstance()->start( fStayAwake );
    }

    CAutoStayAwake::~CAutoStayAwake()
    {
        if ( fStayAwake )
            fStayAwake->stop();
    }
}
