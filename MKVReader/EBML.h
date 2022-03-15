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

#ifndef __MKVREADER_EBML_H
#define __MKVREADER_EBML_H

#include <tuple>
#include <cstdint>

class QFile;
namespace NSABUtils
{
    namespace NMKVReader
    {
        enum class EElementType
        {
            eMaster,
            eInt,
            eUInt,
            eString,
            eUTF8,
            eBinary,
            eFloat,
            eDate
        };

        //class CElement
        //{
        //public:
        //    CElement() = default;
        //private:
        //    uint32_t fID;
        //    uint64_t fSize;
        //};

        class CEBML
        {
        public:
            static std::tuple< uint32_t, uint64_t, uint64_t > readElementIDSize( QFile & file );
            static std::tuple< uint32_t, uint64_t > readElementID( QFile & file );
        };
    }

}


#endif
