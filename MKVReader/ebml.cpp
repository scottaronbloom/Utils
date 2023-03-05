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

#include "EBML.h"

#include <QFile>

namespace NSABUtils
{
    namespace NMKVReader
    {
        std::tuple< uint32_t, uint64_t, uint64_t > CEBML::readElementIDSize( QFile &file )
        {
            uint32_t id{ 0 };
            uint64_t idLen{ 0 };
            std::tie( id, idLen ) = readElementID( file );
            return {};
        }

        std::tuple< uint32_t, uint64_t > CEBML::readElementID( QFile &file )
        {
            uint32_t id{ 0 };
            uint8_t tmp{ 0 };
            file.read( reinterpret_cast< char * >( &tmp ), sizeof( tmp ) );
            id = ( 0x00FF & tmp ) << 24;

            file.read( reinterpret_cast< char * >( &tmp ), sizeof( tmp ) );
            uint32_t id2 = ( 0x00FF & tmp ) << 16;

            file.read( reinterpret_cast< char * >( &tmp ), sizeof( tmp ) );
            uint32_t id3 = ( 0x00FF & tmp ) << 8;

            file.read( reinterpret_cast< char * >( &tmp ), sizeof( tmp ) );
            uint32_t id4 = ( 0x00FF & tmp ) << 0;
            id = id | id2 | id3 | id4;

            file.seek( 0 );
            file.read( reinterpret_cast< char * >( &id ), sizeof( id ) );

            uint64_t idLen{ 0 };
            return { id, idLen };
        }

    }
}