// The MIT License( MIT )
//
// Copyright( c ) 2022 Scott Aron Bloom
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

#include "FileUtils.h"

#include <windows.h>
#include <shellapi.h>

#include <QString>
#include <QFileInfo>

namespace NSABUtils
{
    namespace NFileUtils
    {
        bool moveToTrashImpl( const QString & fileName, std::shared_ptr< SRecycleOptions > options )
        {
            (void)options;
            QFileInfo fi( fileName );
            if ( !fi.exists() )
                return false;

            auto path = fileName.toStdWString();
            path.append( 1, L'\0' );

            SHFILEOPSTRUCTW shfos = {};
            shfos.hwnd = nullptr;       // handle to window that will own generated windows, if applicable
            shfos.wFunc = FO_DELETE;
            shfos.pFrom = path.c_str();
            shfos.pTo = nullptr;       // not used for deletion operations
            shfos.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_NO_UI;
            const int retVal = SHFileOperationW( &shfos );
            return retVal == 0;
        }
    }
}


