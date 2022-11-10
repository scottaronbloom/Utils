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

#ifndef __BIFPLUGIN_H
#define __BIFPLUGIN_H
#include <QImageIOHandler>

#ifndef BIF_PLUGIN_DECLSPEC
#if (defined(Q_OS_WINDOWS))
#  if defined(BUILD_BIF_PLUGIN)
#    define BIF_PLUGIN_DECLSPEC Q_DECL_EXPORT
#  else
#    define BIF_PLUGIN_DECLSPEC Q_DECL_IMPORT
#  endif
#else
#  define BIF_PLUGIN_DECLSPEC
#endif
#endif

namespace NSABUtils
{
    namespace NBIF
    {
        class BIF_PLUGIN_DECLSPEC CPlugin : public QImageIOPlugin
        {
            Q_OBJECT
                Q_PLUGIN_METADATA( IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "bif.json" )
        public:
            CPlugin();
            ~CPlugin();

            Capabilities capabilities( QIODevice * device, const QByteArray & format ) const override;
            QImageIOHandler * create( QIODevice * device, const QByteArray & format = QByteArray() ) const override;
        };
    }
}

namespace NBIFPlugin
{
    extern "C" void BIF_PLUGIN_DECLSPEC setLoopCount( int loopCount );
}

#endif
