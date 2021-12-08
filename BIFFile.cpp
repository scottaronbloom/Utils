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

CBIFFile::CBIFFile( const QString &bifFile, QObject *parent/*=nullptr */ ) :
    fBIFFile( bifFile ),
    QObject( parent )
{
    loadBIF();
}

CBIFFile::~CBIFFile()
{

}

void CBIFFile::loadBIF()
{
    if ( !openFile() )
        return;

    if ( !checkForOpen() )
        return;

    if ( !parseHeader() )
        return;

    if ( !parseIndex() )
        return;

    fAOK = true;
    fErrorString.clear();
    return;
}

bool CBIFFile::checkForOpen()
{
    if ( !fFile || !fFile->isOpen() || !fFile->isReadable() )
    {
        fAOK = false;
        fErrorString = tr( "File '%1' not open yet" ).arg( fBIFFile );
        return false;
    }
    return true;
}

bool CBIFFile::parseHeader()
{
    if ( !checkForOpen() )
        return false;

    auto header = fFile->read( 64 ); // reads 64 bytes of data, the complete header minus the index

    if ( header.length() != 64 )
    {
        fAOK = false;
        fErrorString = tr( "Could not read in header" );
        return false;
    }

    static auto sMagicNumber = QByteArray( "\x89\x42\x49\x46\x0d\x0a\x1a\x0a" );
        
    fMagicNumber = header.left( 8 );
    if ( fMagicNumber != sMagicNumber )
    {
        fAOK = false;
        fErrorString = tr( "Invalid Magic Number" );
        return false;
    }

    fVersion = getValue( header.mid( 8, 4 ), "Version");
    fNumImages = getValue( header.mid( 12, 4 ), "Number of Images" );
    fTSMultiplier = getValue( header.mid( 16, 4 ), "Timestamp Multiplier (ms/frame)" );
    fReserved = header.mid( 20, 44 );
    if ( fReserved.length() != 44 )
    {
        fAOK = false;
        fErrorString = tr( "Invalid header, reserved space isn't complete" );
        return false;
    }
    if ( fReserved != QByteArray( 44, '\0' ) )
    {
        fAOK = false;
        fErrorString = tr( "Invalid header, reserved space isn't 44 bytes of zero" );
        return false;
    }

    return true;
}

bool CBIFFile::parseIndex()
{
    if ( !checkForOpen() )
        return false;

    auto numEntries = std::get< 2 >( fNumImages ) + 1;
    auto numBytes = numEntries * 8;

    auto indexData = fFile->read( numBytes );
    if ( indexData.length() != numBytes )
    {
        fAOK = false;
        fErrorString = tr( "Index data truncated" );
        return false;
    }
    fBIFs.reserve( numEntries );
    SBIF *prev = nullptr;
    for( uint32_t ii = 0; ii < numEntries; ++ii )
    {
        auto tsPos = ( ii * 8 );
        auto offsetPos = 4 + ( ii * 8 );
        auto frameNum = getValue( indexData.mid( tsPos, 4 ), tr( "BIF Index ts# %1" ).arg( ii ) );
        auto absOffset = getValue( indexData.mid( offsetPos, 4 ), tr( "BIF Index abs offset# %1" ).arg( ii ) );

        fBIFs.push_back( SBIF( frameNum, absOffset, prev ) );
        prev = &fBIFs.at( ii );

    }

    for ( uint32_t ii = 0; ii < fBIFs.size() - 1; ++ii )
    {
        auto aOK = fBIFs[ii].loadImage( fFile, fBIFFile );
        if ( !aOK.first )
        {
            fAOK = aOK.first;
            fErrorString = aOK.second;
            return false;
        }
    }

    if ( !fBIFs.back().isLastFrame() )
    {
        fAOK = false;
        fErrorString = tr( "BIF entry #%1 in file '%2' is not the End of BIFs token" ).arg( fBIFs.size() ).arg( fBIFFile );
        return false;
    }
    fBIFs.pop_back();
    return true;
}

T32BitValue CBIFFile::getValue( const QByteArray &in, std::optional< QString > desc )
{
    if ( in.length() > 4 )
    {
        if ( desc.has_value() )
        {
            fAOK = false;
            fErrorString = tr( "Invalid '%1' field" ).arg( desc.value() );
        }
        return { QByteArray(), QString(), -1 };
    }

    uint32_t retVal = 0;
    for ( auto ii = 0; ii < in.length(); ++ii )
    {
        auto curr = static_cast< uint8_t >( in.at( ii ) ) << ( 8 * ii );
        retVal |= curr;
    }
    return { in, prettyPrint( in ), retVal };
}

QString CBIFFile::prettyPrint( const QByteArray & in ) const
{
    QString retVal;
    retVal.reserve( in.length() * 3 );
    bool first = true;
    for( auto ii : in )
    {
        if ( !first )
            retVal += " ";
        retVal += QString( "%1" ).arg( static_cast< uint8_t >( ii ), 2, 16, QChar( '0' ) ).toUpper();
        first = false;
    }
    return retVal;
}

bool CBIFFile::openFile()
{
    if ( fBIFFile.isEmpty() )
    {
        fAOK = false;
        fErrorString = tr( "Filename not set" );
        return false;
    }

    fFile = new QFile( fBIFFile, this );
    if ( !fFile->exists() )
    {
        fAOK = false;
        fErrorString = tr( "File '%1' does not exist" );
        return false;
    }

    if ( !fFile->open( QFile::ReadOnly ) )
    {
        fErrorString = tr( "Could not open '%1' for reading, please check permissions" );
        fAOK = false;
        return false;
    }

    return true;
}

SBIF::SBIF( T32BitValue num, T32BitValue offset, SBIF *prev ) :
    fBIFNum( num ),
    fOffset( offset )
{
    if ( prev )
    {
        auto prevAbsPos = std::get< 2 >( prev->fOffset );
        auto currAbsPos = std::get< 2 >( fOffset );
        prev->fSize = currAbsPos - prevAbsPos;
    }
    Q_ASSERT( std::get< 2 >( fOffset ) );
}

bool SBIF::isLastFrame() const
{
    return ( std::get< 2 >( fBIFNum ) == -1 );
}

std::pair< bool, QString > SBIF::loadImage( QFile *file, const QString &fn )
{
    if ( !file || !file->isOpen() || !file->isReadable() )
    {
        return { false, CBIFFile::tr( "File '%1' not open yet" ).arg( fn ) };
    }

    if ( !file->seek( std::get< 2 >( fOffset ) ) )
    {
        return { false, CBIFFile::tr( "Could not seek to position '%1' in file '%2' to load BIF image #%3" ).arg( std::get< 2 >( fOffset ) ).arg( fn ).arg( std::get< 2 >( fBIFNum ) ) };
    }

    fImage.first = file->read( fSize );
    if ( fImage.first.length() != fSize )
    {
        return { false, CBIFFile::tr( "Could not read '%1' of data starting at position '%2' to load BIF image #%4 from file '%3'" ).arg( fSize ).arg( std::get< 2 >( fOffset ) ).arg( fn ).arg( std::get< 2 >( fBIFNum ) ) };
    }

    fImage.second = QImage::fromData( fImage.first );
    if ( fImage.second.isNull() )
    {
        return {false, CBIFFile::tr( "Invalid JPG format for BIF #%2 in file '%2'" ).arg( std::get< 2 >( fBIFNum ) ).arg( fn ) };
    }
    return { true, QString() };
}
