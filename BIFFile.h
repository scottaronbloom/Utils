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

#ifndef __BIFFILE_H
#define __BIFFILE_H

#include "SABUtilsExport.h"

#include <QObject>
#include <QString>
#include <QImage>
#include <utility>
#include <optional>
#include <QAbstractListModel>

#include "SABUtilsExport.h"

class QFile;
class QDir;
namespace NSABUtils
{
    namespace NBIF
    {
        using T32BitValue = std::tuple< QByteArray, QString, uint32_t >;
        struct SABUTILS_EXPORT SBIFImage
        {
            SBIFImage(T32BitValue ts, T32BitValue offset, SBIFImage *prev);

            bool isLastFrame() const;
            [[nodiscard]] std::pair< bool, QString > loadImage(QIODevice * device, const QString &fn);
            T32BitValue fBIFNum;
            T32BitValue fOffset;
            uint64_t fSize{ 0 };
            std::optional< std::pair< QByteArray, QImage > > fImage;
        };

        using TBIFIndex = std::vector< SBIFImage >; // data read in of ts, pos then a pair of pos, size
        class SABUTILS_EXPORT CFile
        {
        public:
            enum class EState
            {
                eReady, // start
                eDeviceOpen, // after file open, and iodevice set
                eReadHeaderBase,
                eReadHeaderIndex,
                eReadingImages,
                eError
            };

            CFile(const QString &bifFile, bool loadImages); // load the file and go
            CFile(); // used for IOHandlerStream
            virtual ~CFile();

            EState state() const { return fState; }
            bool isValid() const { return state() != EState::eError; }
            QString errorString() const { return fErrorString; }

            static bool createBIF( const QDir & dir, uint32_t timespan, const QString & outFile, const QString & bifTool, QString & msg );

            bool save( const QString & fileName );
            static bool validateMagicNumber(const QByteArray & magicNumber);

            std::pair< bool, QImage > read(QIODevice * device, int frameNumber);
            bool readHeader(QIODevice * device);

            QString magicNumber() const { return prettyPrint(fMagicNumber); } // returns pretty print of the data
            const T32BitValue &version() const { return fVersion; }
            const T32BitValue &numImages() const { return fNumImages; }
            const T32BitValue & tsMultiplier() const { return fTSMultiplier; }
            QString reserved() const { return prettyPrint(fReserved); }

            const TBIFIndex &bifFrames() const { return fBIFFrames; }

            QString fileName() const { return fBIFFile; }

            std::size_t imageCount() const { return fBIFFrames.size(); }
            uint32_t imageDelay() const { return std::get< 2 >(tsMultiplier()); } // number of ms to delay per image
            QSize imageSize() const;

            std::pair< bool, QString > loadImage(size_t imageNum, bool loadUntilFrame, int *insertStart = nullptr, int *numInserted = nullptr);
            QImage imageToFrame(size_t imageNum, int *insertStart = nullptr, int *numInserted = nullptr);
            QImage image(size_t imageNum);

            int lastImageLoaded() { return fLastImageLoaded; }
            bool canLoadMoreImages()
            {
                return fLastImageLoaded < imageCount();
            }
            int fetchSize() const { return 8; }
            void fetchMore();

            void setLoopCount(int loopCount) { fLoopCount = loopCount; } // default is-1 which is infinite
            int loopCount() { return fLoopCount; }
        private:
            static int extractImageNum( const QString & fileName );
            QIODevice * device() const;
            void loadBIFFromFile(bool loadImages);
            void loadBIFFromIODevice(bool loadImages);

            bool checkForOpen();
            bool openFile();
            bool parseHeader(bool loadImages);

            bool parseIndex();
            bool loadImages();

            QString prettyPrint(const QByteArray &in) const;

            T32BitValue getValue(const QByteArray &in, std::optional< QString > desc, bool & aOK);
            QFile *fFile{ nullptr };
            QIODevice * fIODevice{ nullptr };
            QString fBIFFile;
            EState fState{ EState::eReady };
            QString fErrorString;
            QByteArray fMagicNumber;
            T32BitValue fVersion;
            T32BitValue fNumImages;
            T32BitValue fTSMultiplier;
            TBIFIndex fBIFFrames;
            QByteArray fReserved;
            int fLastImageLoaded{ 0 };
            int fLoopCount{ -1 }; // infinite = -1
        };
    }
}
#endif

