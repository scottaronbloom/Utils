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
#include <QPointer>
#include <QAbstractListModel>

#include "SABUtilsExport.h"

class QFileInfo;
class QFile;
class QDir;
namespace NSABUtils
{
    namespace NBIF
    {
        struct SABUTILS_EXPORT S32BitValue
        {
        public:
            S32BitValue();
            S32BitValue( uint32_t inValue );
            S32BitValue( const QByteArray & in, std::optional< QString > desc, QString & msg, bool & aOK );
            QString prettyPrint() const;

            static QString prettyPrint( const QByteArray & in );

            bool operator==( const S32BitValue & rhs ) const
            {
                return fByteArray == rhs.fByteArray
                    && fPrettyPrint == rhs.fPrettyPrint
                    && fValue == rhs.fValue;
            }
            bool operator!=( const S32BitValue & rhs ) const
            {
                return !operator==( rhs );
            }

            bool write( QIODevice * outFile, const std::optional< QString > & desc, QString & msg ) const;
            uint32_t size() const { return fByteArray.length(); }

            QByteArray fByteArray;
            QString fPrettyPrint;
            uint32_t fValue{ 0 };
        };

        struct SABUTILS_EXPORT SBIFImage
        {
            SBIFImage( S32BitValue bifNum, S32BitValue offset, SBIFImage * prev );
            SBIFImage( const QString & fileName, uint32_t bifNum );

            bool operator==( const SBIFImage & rhs ) const;
            bool operator!=( const SBIFImage & rhs ) const;

            QByteArray indexData() const;
            bool imageValid() const;
            bool isLastFrame() const;
            [[nodiscard]] std::pair< bool, QString > loadImage( QIODevice * device, const QString & fn );
            bool writeIndex( QIODevice * outFile, QString & msg ) const;
            bool writeImage( QIODevice * outFile, QString & msg ) const;
            S32BitValue fBIFNum{ -1 };
            S32BitValue fOffset;
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
                eReadAllImages,
                eError
            };

            CFile( const QString & bifFile, bool loadImages ); // load the file and go
            CFile( const QDir & dir, const QString & filter, uint32_t timespan, QString & msg ); // load the file and go
            CFile( const QList< QFileInfo > & images, uint32_t timespan, QString & msg ); // load the images and go
            CFile(); // used for IOHandlerStream

            virtual ~CFile();

            static bool validateMagicNumber( const QByteArray & magicNumber );
            bool save( const QString & fileName, QString & msg );

            QString fileName() const { return fBIFFile; }
            EState state() const { return fState; }
            bool isValid() const { return state() != EState::eError; }
            QString errorString() const { return fErrorString; }

            QString magicNumber() const { return S32BitValue::prettyPrint( fMagicNumber ); } // returns pretty print of the data
            const S32BitValue & version() const { return fVersion; }
            const S32BitValue & numImages() const { return fNumImages; }
            QString reserved() const { return S32BitValue::prettyPrint( fReserved ); }

            uint32_t imageDelay() const { return timePerFrame().fValue; } // number of ms to delay per image
            QSize imageSize() const;

            const S32BitValue & timePerFrame() const { return fTimePerFrame; }

            bool canLoadMoreImages()
            {
                return fLastImageLoaded < imageCount();
            }

            int lastImageLoaded() { return fLastImageLoaded; }
            std::size_t imageCount() const { return fBIFFrames.size(); }

            QImage image( size_t imageNum );
            QList< QImage > images( size_t startFrame, size_t endFrame );
            QImage imageToFrame( size_t imageNum, int * insertStart = nullptr, int * numInserted = nullptr );

            int fetchSize() const { return 8; }
            void fetchMore();

            static bool createBIF( const QDir & dir, uint32_t timespan, const QString & outFile, const QString & bifTool, QString & msg );

            std::pair< bool, QImage > read( QIODevice * device, int frameNumber ); // CFile will own the lifespace of the device
            bool readHeader( QIODevice * device );
            void setLoopCount( int loopCount ) { fLoopCount = loopCount; } // default is-1 which is infinite
            int loopCount() { return fLoopCount; }
        private:
            void init( const QList< QFileInfo > & images, uint32_t timespan, QString & msg );
            static int extractImageNum( const QString & fileName );

            std::pair< bool, QString > loadImage( size_t imageNum, bool loadUntilFrame, int * insertStart = nullptr, int * numInserted = nullptr );

            QIODevice * device() const;
            void loadBIFFromFile( bool loadImages );

            void closeIfFinished();

            void loadBIFFromIODevice( bool loadImages );

            bool checkForOpen();
            bool openFile();
            bool parseHeader( bool loadImages );

            bool parseIndex();
            bool loadImages();

            QFile * fFile{ nullptr };
            QPointer< QIODevice > fIODevice;
            QString fBIFFile;
            EState fState{ EState::eReady };
            QString fErrorString;

            QByteArray fMagicNumber;
            S32BitValue fVersion;
            S32BitValue fNumImages;
            S32BitValue fTimePerFrame;
            S32BitValue fFinalIndex;
            TBIFIndex fBIFFrames;
            QByteArray fReserved;
            int fLastImageLoaded{ 0 };
            int fLoopCount{ -1 }; // infinite = -1
        };
    }
}
#endif

