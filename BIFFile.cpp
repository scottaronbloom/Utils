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
#include "BackupFile.h"
#include "FileUtils.h"

#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QProcess>
#include <QBuffer>

namespace NSABUtils
{
    namespace NBIF
    {
        S32BitValue::S32BitValue() :
            S32BitValue( 0 )
        {

        }

        S32BitValue::S32BitValue( uint32_t value )
        {
            fByteArray = QByteArray( reinterpret_cast<const char *>( &value ), 4 );
            fPrettyPrint = prettyPrint();
            fValue = value;
        }

        S32BitValue::S32BitValue( const QByteArray & in, std::optional< QString > desc, QString & msg, bool & aOK )
        {
            aOK = false;
            if ( in.length() > 4 )
            {
                if ( desc.has_value() )
                {
                    msg = QObject::tr( "Invalid '%1' field" ).arg( desc.value() );
                }
                return;
            }

            fValue = 0;
            for ( auto ii = 0; ii < in.length(); ++ii )
            {
                auto curr = static_cast<uint8_t>( in.at( ii ) ) << ( 8 * ii );
                fValue |= curr;
            }
            aOK = true;
            fByteArray = in;
            fPrettyPrint = prettyPrint();
        }

        QString S32BitValue::prettyPrint( const QByteArray & in )
        {
            QString retVal;
            retVal.reserve( in.length() * 3 );
            bool first = true;
            for ( auto ii : in )
            {
                if ( !first )
                    retVal += " ";
                retVal += QString( "%1" ).arg( static_cast<uint8_t>( ii ), 2, 16, QChar( '0' ) ).toUpper();
                first = false;
            }
            return retVal;
        }

        QString S32BitValue::prettyPrint() const
        {
            return prettyPrint( fByteArray );
        }

        bool S32BitValue::write( QIODevice * outFile, const std::optional< QString > & desc, QString & msg ) const
        {
            if ( !outFile || !outFile->isOpen() || !outFile->isWritable() )
            {
                msg = QString( "Outfile is not open" );
                if ( desc )
                    msg += QString( " to write '%1'" ).arg( desc.value() );
                return false;
            }
            auto value = outFile->write( fByteArray );
            if ( value != fByteArray.length() )
            {
                if ( desc.has_value() )
                    msg = QString( "Could not write out '%1'" ).arg( desc.value() );
                return false;
            }
            return true;
        }

        static auto sMagicNumber = QByteArray( "\x89\x42\x49\x46\x0d\x0a\x1a\x0a" );

        CFile::CFile( const QString & bifFile, bool loadImages ) :
            fBIFFile( bifFile )
        {
            loadBIFFromFile( loadImages );
        }

        CFile::CFile()
        {
        }

        CFile::CFile( const QList< QFileInfo > & allFiles, uint32_t timespan, QString & msg )
        {
            init( allFiles, timespan, msg );
        }

        CFile::CFile( const QDir & dir, const QString & filter, uint32_t timespan, QString & msg )
        {
            fState = EState::eError;

            auto allFiles = NFileUtils::findAllFiles( dir, { filter }, false, true, &msg );
            if ( !allFiles.has_value() || allFiles.value().isEmpty() )
            {
                msg = QString( "No images exists in dir '%1' of the format 'img_*.jpg'." ).arg( dir.absolutePath() );
                return;
            }

            init( allFiles.value(), timespan, msg );
        }

        void CFile::init( const QList< QFileInfo > & images, uint32_t timespan, QString & msg )
        {
            fTimePerFrame = S32BitValue( timespan );

            fBIFFrames.reserve( images.count() );
            uint32_t imageNum = 0;
            for ( auto && ii : images )
            {
                auto currFrame = SBIFImage( ii.absoluteFilePath(), imageNum );
                if ( !currFrame.imageValid() )
                {
                    msg = QString( "Could not read JPG file '%1'" ).arg( ii.absoluteFilePath() );
                    return;
                }

                imageNum++;
                fBIFFrames.push_back( currFrame );
            }
            fMagicNumber = sMagicNumber;
            fVersion = S32BitValue( 0 );
            fNumImages = S32BitValue( static_cast<uint32_t>( fBIFFrames.size() ) );
            fReserved = QByteArray( 44, '\0' );
            fState = EState::eReady;
            uint32_t offset = static_cast<uint32_t>( 8 * ( fBIFFrames.size() + 1 ) ) + fMagicNumber.size() + fVersion.size() + fNumImages.size() + fTimePerFrame.size() + fReserved.size();
            for ( auto && ii : fBIFFrames )
            {
                ii.fOffset = S32BitValue( offset );
                offset += ii.fSize;
            }
            fFinalIndex = offset;
        }

        CFile::~CFile()
        {
            if ( fFile )
                delete fFile;
            if ( fIODevice && fIODevice.data() != fFile )
                fIODevice->close();
        }

        void CFile::loadBIFFromFile( bool loadImages )
        {
            if ( !openFile() )
                return;

            loadBIFFromIODevice( loadImages );
            closeIfFinished();
        }

        void CFile::closeIfFinished()
        {
            if ( ( fState == EState::eReadAllImages ) || ( fState == EState::eError ) )
            {
                if ( fFile )
                    fFile->close();
                if ( fIODevice && fIODevice.data() != fFile )
                    fIODevice->close();
            }
        }

        void CFile::loadBIFFromIODevice( bool loadImages )
        {
            if ( !checkForOpen() )
                return;

            if ( !parseHeader( loadImages ) )
                return;
        }

        std::pair< bool, QImage > CFile::read( QIODevice * device, int frameNum )
        {
            fIODevice = device;
            if ( !device )
                return { false, {} };
            loadBIFFromIODevice( false );
            if ( fState != EState::eReadingImages )
                return { false, {} };

            auto image = this->image( frameNum );
            return { !image.isNull() && ( fState == EState::eReadingImages ), image };
        }

        bool CFile::readHeader( QIODevice * device )
        {
            fIODevice = device;
            if ( !device )
                return false;
            return parseHeader( false );
        }

        QSize CFile::imageSize() const
        {
            if ( fState != EState::eReadingImages )
                return QSize();
            if ( fBIFFrames.empty() )
                return QSize();
            if ( !fBIFFrames[ 0 ].fImage.has_value() )
                return QSize();
            return fBIFFrames[ 0 ].fImage.value().second.size();
        }

        bool CFile::checkForOpen()
        {
            if ( fState == EState::eDeviceOpen )
                return true;

            if ( fState == EState::eReady )
            {
                if ( fIODevice && fIODevice->isOpen() && fIODevice->isReadable() )
                {
                    fState = EState::eDeviceOpen;
                }
                else
                {
                    fState = EState::eError;
                    fErrorString = QObject::tr( "File '%1' not open yet" ).arg( fBIFFile );
                    return false;
                }
            }
            return true;
        }

        bool CFile::save( const QString & fileName, QString & msg )
        {
            QFile outFile( fileName );
            if ( !outFile.open( QFile::WriteOnly | QFile::Truncate ) )
            {
                msg = QString( "Could not open '%1' for writing" ).arg( fileName );
                return false;
            }

            outFile.write( fMagicNumber );
            if ( !fVersion.write( &outFile, "Version", msg ) )
                return false;
            if ( !fNumImages.write( &outFile, "Number of Images", msg ) )
                return false;
            if ( !fTimePerFrame.write( &outFile, "Timestamp Multiplier (ms/frame)", msg ) )
                return false;
            outFile.write( fReserved );

            for ( auto && ii : fBIFFrames )
            {
                if ( !ii.writeIndex( &outFile, msg ) )
                    return false;
            }
            auto sentinel = S32BitValue( static_cast<uint32_t>( -1 ) );
            if ( !sentinel.write( &outFile, "Sentinel", msg ) )
                return false;
            if ( !fFinalIndex.write( &outFile, "Final Offset", msg ) )
                return false;

            for ( auto && ii : fBIFFrames )
            {
                if ( !ii.writeImage( &outFile, msg ) )
                    return false;
            }

            return true;
        }

        bool CFile::parseHeader( bool loadImages )
        {
            if ( !checkForOpen() )
                return false;

            if ( fState == EState::eError )
                return false;
            if ( fState != EState::eDeviceOpen )
                return true;// already been read

            fState = EState::eError;
            device()->seek( 0 );
            auto header = device()->read( 64 ); // reads 64 bytes of data, the complete header minus the index

            if ( header.length() != 64 )
            {
                fErrorString = QObject::tr( "Could not read in header" );
                return false;
            }

            fMagicNumber = header.left( 8 );
            if ( !validateMagicNumber( fMagicNumber ) )
            {
                fErrorString = QObject::tr( "Invalid Magic Number" );
                return false;
            }

            bool aOK = false;
            QString msg;
            fVersion = S32BitValue( header.mid( 8, 4 ), "Version", msg, aOK );
            if ( !aOK )
            {
                fState = EState::eError;
                fErrorString = msg;
                return false;
            }

            fNumImages = S32BitValue( header.mid( 12, 4 ), "Number of Images", msg, aOK );
            if ( !aOK )
            {
                fState = EState::eError;
                fErrorString = msg;
                return false;
            }

            fTimePerFrame = S32BitValue( header.mid( 16, 4 ), "Timestamp Multiplier (ms/frame)", msg, aOK );
            if ( !aOK )
            {
                fState = EState::eError;
                fErrorString = msg;
                return false;
            }

            fReserved = header.mid( 20, 44 );
            if ( fReserved.length() != 44 )
            {
                fErrorString = QObject::tr( "Invalid header, reserved space isn't complete" );
                return false;
            }
            if ( fReserved != QByteArray( 44, '\0' ) )
            {
                fErrorString = QObject::tr( "Invalid header, reserved space isn't 44 bytes of zero" );
                return false;
            }

            fState = EState::eReadHeaderBase;
            if ( !parseIndex() )
                return false;
            if ( loadImages )
            {
                return this->loadImages();
            }
            else
            {
                return loadImage( 0, true ).first; // always load one so size can be returned
            }
            return false;
        }

        bool CFile::createBIF( const QDir &dir, uint32_t timespan, const QString &outFile, const QString &bifTool, QString &msg )
        {
            if ( !dir.exists() || !dir.isReadable() )
            {
                msg = QString( "Directory '%1' does not exist." ).arg( dir.absolutePath() );
                return false;
            }

            QFileInfo fi( bifTool );
            if ( !fi.exists() || !fi.isExecutable() )
            {
                msg = QString( "biftool '%1' does not exist." ).arg( bifTool );
                return false;
            }

            auto files = dir.entryList( { "img_*.jpg" }, QDir::Files, QDir::SortFlag::Name );
            if ( files.isEmpty() )
            {
                msg = QString( "No images exists in dir '%1' of the format 'img_*.jpg'." ).arg( dir.absolutePath() );
                return false;
            }

            auto num = extractImageNum( files.front() );
            if ( num == -1 )
            {
                msg = QString( "Image number could not be determined on file '%1'" ).arg( files.front() );
                return false;
            }
            if ( num == 1 )
            {
                for ( auto &&ii : files )
                {
                    auto num = extractImageNum( ii );
                    if ( num <= 0 )
                    {
                        msg = QString( "Image number could not be determined on file '%1'" ).arg( ii );
                        return false;
                    }
                    auto absPath = dir.absoluteFilePath( ii );
                    auto newName = dir.absoluteFilePath( QString( "%1.jpg" ).arg( num - 1, 5, 10, QChar( '0' ) ) );

                    if ( !NFileUtils::backup( newName ) )
                    {
                        msg = QString( "Could not backup '%1'" ).arg( newName );
                        return false;
                    }
                    if ( !QFile::rename( absPath, newName ) )
                    {
                        msg = QString( "Could not rename image file from '%1' to '%2'" ).arg( absPath ).arg( newName );
                        return false;
                    }
                }
            }

            auto args = QStringList() << "-v"
                                      << "-t" << QString::number( timespan ) << dir.absolutePath();
            QProcess process;
            process.start( bifTool, args );
            if ( !process.waitForFinished( -1 ) || ( process.exitStatus() != QProcess::NormalExit ) || ( process.exitCode() != 0 ) )
            {
                msg = QString( "Error running biftool '%1' - " ).arg( bifTool ).arg( QString( process.readAllStandardError() ) );
                return false;
            }
            msg = process.readAll();

            auto tmpFile = QFileInfo( dir.dirName() + ".bif" );
            if ( !tmpFile.exists() )
            {
                msg = QString( "Generated BIF file '%1' does not exist." ).arg( tmpFile.absoluteFilePath() );
                return false;
            }
            if ( !NFileUtils::backup( outFile ) )
            {
                msg = QString( "Could not backup file '%1'." ).arg( outFile );
                return false;
            }

            if ( !QFile::rename( tmpFile.absoluteFilePath(), outFile ) )
            {
                msg = QString( "Could not move generated BIF file '%1' to '%2' does not exist." ).arg( tmpFile.absoluteFilePath() ).arg( outFile );
                return false;
            }
            return true;
        }

        int CFile::extractImageNum( const QString & file )
        {
            static auto regExp = QRegularExpression( R"(img_(\d+)\.jpg)" );
            auto match = regExp.match( file );
            int num = -1;
            if ( match.hasMatch() )
            {
                bool aOK;
                num = match.captured( 1 ).toInt( &aOK );
                if ( !aOK )
                    num = -1;
            }
            return num;
        }

        bool CFile::validateMagicNumber( const QByteArray & magicNumber )
        {
            return ( magicNumber == sMagicNumber );
        }

        bool CFile::parseIndex()
        {
            if ( fState != EState::eReadHeaderBase )
                return false;

            fState = EState::eError;

            auto numEntries = fNumImages.fValue + 1;
            auto numBytes = numEntries * 8;

            auto indexData = device()->read( numBytes );
            if ( indexData.length() != numBytes )
            {
                fErrorString = QObject::tr( "Index data truncated" );
                return false;
            }
            fBIFFrames.reserve( numEntries );
            SBIFImage * prev = nullptr;
            for ( uint32_t ii = 0; ii < numEntries; ++ii )
            {
                auto tsPos = ( ii * 8 );
                auto offsetPos = 4 + ( ii * 8 );
                bool aOK = false;
                QString msg;
                auto frameNum = S32BitValue( indexData.mid( tsPos, 4 ), QObject::tr( "BIF Index ts# %1" ).arg( ii ), msg, aOK );
                if ( !aOK )
                {
                    fState = EState::eError;
                    fErrorString = msg;
                    return false;
                }

                auto absOffset = S32BitValue( indexData.mid( offsetPos, 4 ), QObject::tr( "BIF Index abs offset# %1" ).arg( ii ), msg, aOK );
                if ( !aOK )
                {
                    fState = EState::eError;
                    fErrorString = msg;
                    return false;
                }

                fBIFFrames.emplace_back( frameNum, absOffset, prev );
                prev = &fBIFFrames.at( ii );
            }

            if ( !fBIFFrames.back().isLastFrame() )
            {
                fErrorString = QObject::tr( "BIF entry #%1 in file '%2' is not the End of BIFs token" ).arg( fBIFFrames.size() ).arg( fBIFFile );
                return false;
            }
            auto lastFrame = fBIFFrames.back();
            fBIFFrames.pop_back();
            fFinalIndex = lastFrame.fOffset;
            fState = EState::eReadHeaderIndex;
            return true;
        }

        bool SBIFImage::operator==( const SBIFImage & rhs ) const
        {
            if ( fImage.has_value() != rhs.fImage.has_value() )
                return false;
            if ( !fImage.has_value() )
                return true;

            return fImage.value() == rhs.fImage.value();
        }

        bool SBIFImage::operator!=( const SBIFImage & rhs ) const
        {
            return !operator==( rhs );
        }

        QByteArray SBIFImage::indexData() const
        {
            QByteArray retVal;
            retVal += fBIFNum.fByteArray;
            retVal += fOffset.fByteArray;
            return retVal;
        }


        bool CFile::loadImages()
        {
            if ( fState != EState::eReadHeaderIndex )
                return false;

            for ( uint32_t ii = 0; ii < fBIFFrames.size() - 1; ++ii )
            {
                auto aOK = loadImage( ii, true );
                if ( !aOK.first )
                {
                    fErrorString = aOK.second;
                    return false;
                }
            }
            return isValid();
        }

        std::pair< bool, QString > CFile::loadImage( size_t frameNum, bool loadImageToFrame, int * insertStart, int * numInserted )
        {
            if ( frameNum >= fBIFFrames.size() )
                return{ false, "Invalid argument" };

            auto retVal = std::make_pair( true, QString() );

            bool addingImages = ( fLastImageLoaded <= frameNum );
            if ( addingImages )
            {
                if ( insertStart )
                    *insertStart = fLastImageLoaded;
                if ( numInserted )
                    *numInserted = static_cast<int>( frameNum - fLastImageLoaded ) + 1;
            }

            if ( frameNum >= ( fBIFFrames.size() - 1 ) )
                int xyz = 0;
            if ( loadImageToFrame )
            {
                while ( fLastImageLoaded <= frameNum )
                {
                    auto curr = fBIFFrames[ fLastImageLoaded ].loadImage( device(), fBIFFile );
                    if ( !curr.first )
                    {
                        fState = EState::eError;
                        retVal.first = false;
                        retVal.second = curr.second;
                        return retVal;
                    }
                    fLastImageLoaded++;
                }
                fState = fLastImageLoaded >= fBIFFrames.size() ? EState::eReadAllImages : EState::eReadingImages;
                closeIfFinished();
                return retVal;
            }
            else
            {
                auto curr = fBIFFrames[ frameNum ].loadImage( device(), fBIFFile );
                return curr;
            }
        }

        QImage CFile::imageToFrame( size_t imageNum, int * insertStart, int * numInserted )
        {
            if ( !loadImage( imageNum, true, insertStart, numInserted ).first || !fBIFFrames[ imageNum ].fImage.has_value() )
                return QImage();

            return fBIFFrames[ imageNum ].fImage.value().second;
        }

        QImage CFile::image( size_t imageNum )
        {
            if ( !loadImage( imageNum, false ).first || !fBIFFrames[ imageNum ].fImage.has_value() )
                return QImage();

            return fBIFFrames[ imageNum ].fImage.value().second;
        }

        QList< QImage > CFile::images( size_t startFrame, size_t endFrame )
        {
            QList< QImage > retVal;
            for ( auto ii = startFrame; ii <= endFrame; ++ii )
                retVal.push_back( image( ii ) );
            return retVal;
        }

        void CFile::fetchMore()
        {
            size_t remainder = imageCount() - fLastImageLoaded;
            int itemsToFetch = std::min( 8ULL, remainder );
            if ( itemsToFetch == 0 )
                return;

            for ( size_t ii = 0; ii < itemsToFetch; ++ii )
            {
                loadImage( fLastImageLoaded, true );
            }
        }

        QIODevice * CFile::device() const
        {
            if ( fFile )
                return fFile;
            return fIODevice;
        }

        bool CFile::openFile()
        {
            fState = EState::eError;
            if ( fBIFFile.isEmpty() )
            {
                fErrorString = QObject::tr( "Filename not set" );
                return false;
            }

            fFile = new QFile( fBIFFile, nullptr );
            if ( !fFile->exists() )
            {
                fErrorString = QObject::tr( "File '%1' does not exist" );
                return false;
            }

            if ( !fFile->open( QFile::ReadOnly ) )
            {
                fErrorString = QObject::tr( "Could not open '%1' for reading, please check permissions" );
                return false;
            }

            fIODevice = fFile;
            fState = EState::eDeviceOpen;

            return true;
        }

        SBIFImage::SBIFImage( S32BitValue num, S32BitValue offset, SBIFImage * prev ) :
            fBIFNum( num ),
            fOffset( offset )
        {
            if ( prev )
            {
                auto prevAbsPos = prev->fOffset.fValue;
                auto currAbsPos = fOffset.fValue;
                prev->fSize = currAbsPos - prevAbsPos;
            }
            Q_ASSERT( fOffset.fValue );
        }

        SBIFImage::SBIFImage( const QString & fileName, uint32_t bifNum ) :
            fBIFNum( bifNum )
        {
            auto fi = QFile( fileName );
            if ( !fi.open( QFile::ReadOnly ) )
                return;

            auto ba = fi.readAll();
            auto image = QImage::fromData( ba );

            fImage = { ba, image };
            fSize = ba.size();
        }

        bool SBIFImage::imageValid() const
        {
            if ( !fImage.has_value() )
                return false;
            return !fImage.value().second.isNull();
        }

        bool SBIFImage::isLastFrame() const
        {
            return ( fBIFNum.fValue == -1 );
        }

        std::pair< bool, QString > SBIFImage::loadImage( QIODevice * ioDevice, const QString & fn )
        {
            if ( fImage.has_value() )
                return { true, QString() };
            if ( !ioDevice || !ioDevice->isOpen() || !ioDevice->isReadable() )
            {
                return { false, QObject::tr( "File '%1' not open yet" ).arg( fn ) };
            }

            if ( !ioDevice->seek( fOffset.fValue ) )
            {
                return { false, QObject::tr( "Could not seek to position '%1' in file '%2' to load BIF image #%3" ).arg( fOffset.fValue ).arg( fn ).arg( fBIFNum.fValue ) };
            }

            auto data = ioDevice->read( fSize );
            if ( data.length() != fSize )
            {
                return { false, QObject::tr( "Could not read '%1' of data starting at position '%2' to load BIF image #%4 from file '%3'" ).arg( fSize ).arg( fOffset.fValue ).arg( fn ).arg( fBIFNum.fValue ) };
            }

            auto image = QImage::fromData( data );
            if ( image.isNull() )
            {
                return { false, QObject::tr( "Invalid JPG format for BIF #%2 in file '%2'" ).arg( fBIFNum.fValue ).arg( fn ) };
            }
            fImage = { data, image };
            return { true, QString() };
        }

        bool SBIFImage::writeIndex( QIODevice * outFile, QString & msg ) const
        {
            if ( !outFile || !outFile->isOpen() || !outFile->isWritable() )
            {
                msg = QString( "Outfile is not open to write index for image #%1" ).arg( fBIFNum.fValue );
                return false;
            }

            auto len = indexData().length();
            auto num = outFile->write( indexData() );
            if ( num != len )
            {
                msg = QString( "Problem writing out image index '%1'" ).arg( fBIFNum.fValue );
            }
            return num == len;
        }

        bool SBIFImage::writeImage( QIODevice * outFile, QString & msg ) const
        {
            if ( !outFile || !outFile->isOpen() || !outFile->isWritable() )
            {
                msg = QString( "Outfile is not open to write image index '%1'" ).arg( fBIFNum.fValue );
                return false;
            }

            if ( !fImage.has_value() || fImage.value().second.isNull() )
            {
                msg = QString( "Image %1 not loaded or invalid" ).arg( fBIFNum.fValue );
                return false;
            }

            auto len = fImage.value().first.length();
            auto num = outFile->write( fImage.value().first );
            if ( num != len )
            {
                msg = QString( "Problem writing out image index '%1'" ).arg( fBIFNum.fValue );
            }
            return num == len;
        }

    }
}