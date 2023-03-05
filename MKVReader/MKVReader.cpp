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

#include "MKVReader.h"
#include "EBML.h"
#include "ids.h"

namespace NSABUtils
{
    namespace NMKVReader
    {
        CMKVFile::CMKVFile()
        {
        }

        CMKVFile::CMKVFile( const QString &fileName ) :
            CMKVFile()
        {
            open( fileName );
        }

        CMKVFile::~CMKVFile()
        {
        }

        bool CMKVFile::open( const QString &fileName )
        {
            if ( fFile.isOpen() )
                fFile.close();

            fFile.setFileName( fileName );
            if ( !fFile.exists() )
            {
                fErrorMsg = QString( "File: '%1' does not exist" ).arg( fileName );
                return false;
            }

            fFile.open( QFile::ReadOnly );
            if ( !fFile.isOpen() )
            {
                fErrorMsg = QString( "File: '%1' could not be open for reading" ).arg( fileName );
                return false;
            }

            uint32_t id;
            uint64_t idSize;
            uint64_t elementSize;
            std::tie( id, idSize, elementSize ) = CEBML::readElementIDSize( fFile );
            while ( id != EIDs::kSEGMENT )
            {
                if ( !fFile.seek( idSize ) )
                {
                    fErrorMsg = QString( "Could not find id at location '%1'" ).arg( idSize );
                    return false;
                }
                std::tie( id, idSize, elementSize ) = CEBML::readElementIDSize( fFile );
            }

            return true;
        }
    }
}