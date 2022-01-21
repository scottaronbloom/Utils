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

#include "BIFFile.h"
#include <QFile>
#include <QFile>
namespace NSABUtils
{
    namespace NBIF
    {
        CFile::CFile(const QString &bifFile, bool loadImages) :
            fBIFFile(bifFile)
        {
            loadBIFFromFile(loadImages);
        }

        CFile::CFile()
        {
        }

        CFile::~CFile()
        {
            if (fFile)
                delete fFile;
        }

        void CFile::loadBIFFromFile(bool loadImages)
        {
            if (!openFile())
                return;

            loadBIFFromIODevice(loadImages);
            if (fFile && loadImages)
                fFile->close();
        }

        void CFile::loadBIFFromIODevice(bool loadImages)
        {
            if (!checkForOpen())
                return;

            if (!parseHeader(loadImages))
                return;
        }

        std::pair< bool, QImage > CFile::read(QIODevice * device, int frameNum)
        {
            fIODevice = device;
            if (!device)
                return { false, {} };
            loadBIFFromIODevice(false);
            if (fState != EState::eReadingImages)
                return { false, {} };

            auto image = this->image(frameNum);
            return { !image.isNull() && (fState == EState::eReadingImages), image };
        }

        bool CFile::readHeader(QIODevice * device)
        {
            fIODevice = device;
            if (!device)
                return false;
            return parseHeader(false);
        }

        QSize CFile::imageSize() const
        {
            if (fState != EState::eReadingImages)
                return QSize();
            if (fBIFs.empty())
                return QSize();
            if (!fBIFs[0].fImage.has_value())
                return QSize();
            return fBIFs[0].fImage.value().second.size();
        }

        bool CFile::checkForOpen()
        {
            if (fState == EState::eDeviceOpen)
                return true;

            if (fState == EState::eReady)
            {
                if (fIODevice && fIODevice->isOpen() && fIODevice->isReadable())
                {
                    fState = EState::eDeviceOpen;
                }
                else
                {
                    fState = EState::eError;
                    fErrorString = QObject::tr("File '%1' not open yet").arg(fBIFFile);
                    return false;
                }
            }
            return true;
        }

        bool CFile::parseHeader(bool loadImages)
        {
            if (!checkForOpen())
                return false;

            if (fState == EState::eError)
                return false;
            if (fState != EState::eDeviceOpen)
                return true;// already been read

            fState = EState::eError;
            device()->seek(0);
            auto header = device()->read(64); // reads 64 bytes of data, the complete header minus the index

            if (header.length() != 64)
            {
                fErrorString = QObject::tr("Could not read in header");
                return false;
            }

            fMagicNumber = header.left(8);
            if (!validateMagicNumber(fMagicNumber))
            {
                fErrorString = QObject::tr("Invalid Magic Number");
                return false;
            }

            bool aOK;
            fVersion = getValue(header.mid(8, 4), "Version", aOK);
            if (!aOK)
                return false;

            fNumImages = getValue(header.mid(12, 4), "Number of Images", aOK);
            if (!aOK)
                return false;

            fTSMultiplier = getValue(header.mid(16, 4), "Timestamp Multiplier (ms/frame)", aOK);
            if (!aOK)
                return false;

            fReserved = header.mid(20, 44);
            if (fReserved.length() != 44)
            {
                fErrorString = QObject::tr("Invalid header, reserved space isn't complete");
                return false;
            }
            if (fReserved != QByteArray(44, '\0'))
            {
                fErrorString = QObject::tr("Invalid header, reserved space isn't 44 bytes of zero");
                return false;
            }

            fState = EState::eReadHeaderBase;
            if (!parseIndex())
                return false;
            if (loadImages)
            {
                return this->loadImages();
            }
            else
            {
                return loadImage(0, true).first; // always load one so size can be returned
            }
            return false;
        }

        bool CFile::validateMagicNumber(const QByteArray & magicNumber)
        {
            static auto sMagicNumber = QByteArray("\x89\x42\x49\x46\x0d\x0a\x1a\x0a");

            return (magicNumber == sMagicNumber);
        }

        bool CFile::parseIndex()
        {
            if (fState != EState::eReadHeaderBase)
                return false;

            fState = EState::eError;

            auto numEntries = std::get< 2 >(fNumImages) + 1;
            auto numBytes = numEntries * 8;

            auto indexData = device()->read(numBytes);
            if (indexData.length() != numBytes)
            {
                fErrorString = QObject::tr("Index data truncated");
                return false;
            }
            fBIFs.reserve(numEntries);
            SBIFImage * prev = nullptr;
            for (uint32_t ii = 0; ii < numEntries; ++ii)
            {
                auto tsPos = (ii * 8);
                auto offsetPos = 4 + (ii * 8);
                bool aOK;
                auto frameNum = getValue(indexData.mid(tsPos, 4), QObject::tr("BIF Index ts# %1").arg(ii), aOK);
                if (!aOK)
                    return false;

                auto absOffset = getValue(indexData.mid(offsetPos, 4), QObject::tr("BIF Index abs offset# %1").arg(ii), aOK);
                if (!aOK)
                    return false;

                fBIFs.emplace_back(frameNum, absOffset, prev);
                prev = &fBIFs.at(ii);
            }

            if (!fBIFs.back().isLastFrame())
            {
                fErrorString = QObject::tr("BIF entry #%1 in file '%2' is not the End of BIFs token").arg(fBIFs.size()).arg(fBIFFile);
                return false;
            }
            fBIFs.pop_back();
            fState = EState::eReadHeaderIndex;
            return true;
        }

        bool CFile::loadImages()
        {
            if (fState != EState::eReadHeaderIndex)
                return false;

            for (uint32_t ii = 0; ii < fBIFs.size() - 1; ++ii)
            {
                auto aOK = loadImage(ii, true);
                if (!aOK.first)
                {
                    fErrorString = aOK.second;
                    return false;
                }
            }
            return fState == EState::eReadingImages;
        }

        std::pair< bool, QString > CFile::loadImage(size_t frameNum, bool loadImageToFrame, int *insertStart, int *numInserted)
        {
            if (frameNum >= fBIFs.size())
                return{ false, "Invalid argument" };

            auto aOK = std::make_pair(true, QString());

            bool addingImages = (fLastImageLoaded <= frameNum);
            if (addingImages)
            {
                if (insertStart)
                    *insertStart = fLastImageLoaded;
                if (numInserted)
                    *numInserted = static_cast<int>(frameNum - fLastImageLoaded) + 1;
            }

            if (loadImageToFrame)
            {
                while (fLastImageLoaded <= frameNum)
                {
                    auto curr = fBIFs[fLastImageLoaded].loadImage(device(), fBIFFile);
                    if (!curr.first)
                    {
                        fState = EState::eError;
                        aOK.first = false;
                        aOK.second = curr.second;
                        return aOK;
                    }
                    fLastImageLoaded++;
                }
                fState = EState::eReadingImages;
                return aOK;
            }
            else
            {
                auto curr = fBIFs[frameNum].loadImage(device(), fBIFFile);
                return curr;
            }
        }

        QImage CFile::imageToFrame(size_t imageNum, int *insertStart, int *numInserted)
        {
            if (!loadImage(imageNum, true, insertStart, numInserted).first || !fBIFs[imageNum].fImage.has_value())
                return QImage();

            return fBIFs[imageNum].fImage.value().second;
        }

        QImage CFile::image(size_t imageNum)
        {
            if (!loadImage(imageNum, false).first || !fBIFs[imageNum].fImage.has_value())
                return QImage();

            return fBIFs[imageNum].fImage.value().second;
        }

        void CFile::fetchMore()
        {
            size_t remainder = imageCount() - fLastImageLoaded;
            int itemsToFetch = std::min(8ULL, remainder);
            if (itemsToFetch == 0)
                return;

            for (size_t ii = 0; ii < itemsToFetch; ++ii)
            {
                loadImage(fLastImageLoaded, true);
            }
        }

        T32BitValue CFile::getValue(const QByteArray & in, std::optional< QString > desc, bool & aOK)
        {
            aOK = false;
            if (in.length() > 4)
            {
                if (desc.has_value())
                {
                    fState = EState::eError;
                    fErrorString = QObject::tr("Invalid '%1' field").arg(desc.value());
                }
                return { QByteArray(), QString(), -1 };
            }

            uint32_t retVal = 0;
            for (auto ii = 0; ii < in.length(); ++ii)
            {
                auto curr = static_cast<uint8_t>(in.at(ii)) << (8 * ii);
                retVal |= curr;
            }
            aOK = true;
            return { in, prettyPrint(in), retVal };
        }

        QString CFile::prettyPrint(const QByteArray &in) const
        {
            QString retVal;
            retVal.reserve(in.length() * 3);
            bool first = true;
            for (auto ii : in)
            {
                if (!first)
                    retVal += " ";
                retVal += QString("%1").arg(static_cast<uint8_t>(ii), 2, 16, QChar('0')).toUpper();
                first = false;
            }
            return retVal;
        }

        QIODevice * CFile::device() const
        {
            if (fFile)
                return fFile;
            return fIODevice;
        }

        bool CFile::openFile()
        {
            fState = EState::eError;
            if (fBIFFile.isEmpty())
            {
                fErrorString = QObject::tr("Filename not set");
                return false;
            }

            fFile = new QFile(fBIFFile, nullptr);
            if (!fFile->exists())
            {
                fErrorString = QObject::tr("File '%1' does not exist");
                return false;
            }

            if (!fFile->open(QFile::ReadOnly))
            {
                fErrorString = QObject::tr("Could not open '%1' for reading, please check permissions");
                return false;
            }

            fIODevice = fFile;
            fState = EState::eDeviceOpen;

            return true;
        }

        SBIFImage::SBIFImage(T32BitValue num, T32BitValue offset, SBIFImage *prev) :
            fBIFNum(num),
            fOffset(offset)
        {
            if (prev)
            {
                auto prevAbsPos = std::get< 2 >(prev->fOffset);
                auto currAbsPos = std::get< 2 >(fOffset);
                prev->fSize = currAbsPos - prevAbsPos;
            }
            Q_ASSERT(std::get< 2 >(fOffset));
        }

        bool SBIFImage::isLastFrame() const
        {
            return (std::get< 2 >(fBIFNum) == -1);
        }

        std::pair< bool, QString > SBIFImage::loadImage(QIODevice *ioDevice, const QString &fn)
        {
            if (fImage.has_value())
                return { true, QString() };
            if (!ioDevice || !ioDevice->isOpen() || !ioDevice->isReadable())
            {
                return { false, QObject::tr("File '%1' not open yet").arg(fn) };
            }

            if (!ioDevice->seek(std::get< 2 >(fOffset)))
            {
                return { false, QObject::tr("Could not seek to position '%1' in file '%2' to load BIF image #%3").arg(std::get< 2 >(fOffset)).arg(fn).arg(std::get< 2 >(fBIFNum)) };
            }

            auto data = ioDevice->read(fSize);
            if (data.length() != fSize)
            {
                return { false, QObject::tr("Could not read '%1' of data starting at position '%2' to load BIF image #%4 from file '%3'").arg(fSize).arg(std::get< 2 >(fOffset)).arg(fn).arg(std::get< 2 >(fBIFNum)) };
            }

            auto image = QImage::fromData(data);
            if (image.isNull())
            {
                return { false, QObject::tr("Invalid JPG format for BIF #%2 in file '%2'").arg(std::get< 2 >(fBIFNum)).arg(fn) };
            }
            fImage = { data, image };
            return { true, QString() };
        }
    }
}