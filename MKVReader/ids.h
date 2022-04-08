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

#ifndef __MKVREADER_IDS_H
#define __MKVREADER_IDS_H
#include <cstdint>

namespace NSABUtils
{
    namespace NMKVReader
    {
        enum EIDs
        {
            kSEGMENT = 0x1853'8067u,
            kSEEKHEAD = 0x114D'9B74u,
            kSEEK = 0x4DBBu,
            kSEEKID = 0x53ABu,
            kSEEKPOSITION = 0x53ACu,
            kINFO = 0x1549'A966u,
            kSEGMENTUID = 0x73A4u,
            kPREVUID = 0x3C'B923u,
            kNEXTUID = 0x3E'B923u,
            kSEGMENTFAMILY = 0x4444u,
            kTITLE = 0x7BA9u,
            kMUXINGAPP = 0x4D80u,
            kWRITINGAPP = 0x5741u,
            kDATEUTC = 0x4461u,
            kTIMECODESCALE = 0x2A'D7B1u,
            kDURATION = 0x4489u,
            kTRACKS = 0x1654'AE6Bu,
            kTRACKENTRY = 0xAEu,
            kTRACKNUMBER = 0xD7u,
            kTRACKUID = 0x73C5u,
            kTRACKTYPE = 0x83u,
            kFLAGENABLED = 0xB9u,
            kFLAGDEFAULT = 0x88u,
            kFLAGFORCED = 0x55AAu,
            kFLAGLACING = 0x9Cu,
            kDEFAULTDURATION = 0x23'E383u,
            kNAME = 0x536Eu,
            kLANGUAGE = 0x22'B59Cu,
            kLANGUAGE_IETF = 0x22'B59Du,
            kCODEC_ID = 0x86u,
            kCODEC_NAME = 0x25'8688u,
            kVIDEO = 0xE0u,
            kPIXELWIDTH = 0xB0u,
            kPIXELHEIGHT = 0xBAu,
            kDISPLAYWIDTH = 0x54B0u,
            kDISPLAYHEIGHT = 0x54BAu,
            kINTERLACED = 0x9Au,
            kSTEREOMODE = 0x53B8u,
            kAUDIO = 0xE1u,
            kSAMPLINGFREQUENCY = 0xB5u,
            kCHANNELS = 0x9Fu,
            kBITDEPTH = 0x6264u,
            kATTACHMENTS = 0x1941'A469u,
            kATTACHEDFILE = 0x61A7u,
            kFILEDESCRIPTION = 0x467Eu,
            kFILENAME = 0x466Eu,
            kFILEMIMETYPE = 0x4660u,
            kFILEDATA = 0x465Cu,
            kCHAPTERS = 0x1043'A770u,
            kEDITIONENTRY = 0x45B9u,
            kEDITIONUID = 0x45BCu,
            kEDITIONFLAGHIDDEN = 0x45BDu,
            kEDITIONFLAGDEFAULT = 0x45DBu,
            kEDITIONFLAGORDERED = 0x45DDu,
            kCHAPTERATOM = 0xB6u,
            kCHAPTERUID = 0x73C4u,
            kCHAPTERTIMESTART = 0x91u,
            kCHAPTERTIMEEND = 0x92u,
            kCHAPTERFLAGHIDDEN = 0x98u,
            kCHAPTERFLAGENABLED = 0x4598u,
            kCHAPTERSEGMENTUID = 0x6E67u,
            kCHAPTERSEGMENTEDITIONUID = 0x6EBCu,
            kCHAPTERDISPLAY = 0x80u,
            kCHAPSTRING = 0x85u,
            kCHAPLANGUAGE = 0x437Cu,
            kCHAPLANGUAGE_IETF = 0x437Du,
            kTAGS = 0x1254'C367u,
            kTAG = 0x7373u,
            kTARGETS = 0x63C0u,
            kTARGETTYPEVALUE = 0x68CAu,
            kTARGETTYPE = 0x63CAu,
            kTAG_TRACK_UID = 0x63C5u,
            kTAG_EDITION_UID = 0x63C9u,
            kTAG_CHAPTER_UID = 0x63C4u,
            kTAG_ATTACHMENT_UID = 0x63C6u,
            kSIMPLETAG = 0x67C8u,
            kTAGNAME = 0x45A3u,
            kTAGLANGUAGE = 0x447Au,
            kTAGLANGUAGE_IETF = 0x447Bu,
            kTAGDEFAULT = 0x4484u,
            kTAGSTRING = 0x4487u,
            kTAGBINARY = 0x4485u
        };
    }

}


#endif
