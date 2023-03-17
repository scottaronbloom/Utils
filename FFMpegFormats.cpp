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

#include "FFMpegFormats.h"
#include "GPUDetect.h"

#include <QProcess>
#include <QFileInfo>
#include <QProgressDialog>
#include <QRegularExpression>
#include <QImageReader>

namespace NSABUtils
{
    void removeFromlist( QStringList &list, const QString &regex )
    {
        auto regExp = QRegularExpression( regex );
        auto pos = list.indexOf( regExp );
        while ( pos != -1 )
        {
            list.removeAt( pos );
            pos = list.indexOf( regExp, pos );
        }
    }

    void cleanLists( const QString &intel, const QString &nvidia, const QString &amd, QStringList &terse, QStringList &verbose )
    {
        SGPUInfo info;
        QStringList regExs;
        if ( !info.fIntel )
        {
            regExs << intel;
        }

        if ( !info.fNVidia )
        {
            regExs << nvidia;
        }

        if ( !info.fAMD )
        {
            regExs << amd;
        }
        auto regEx = regExs.join( "|" );
        removeFromlist( terse, regEx );
        removeFromlist( verbose, regEx );
    }

    CFFMpegFormats::CFFMpegFormats()
    {
    }

    CFFMpegFormats::CFFMpegFormats( const QString &ffmpegExe )
    {
        setFFMpegExecutable( ffmpegExe );
    }

    void CFFMpegFormats::initEncoderFormatsFromDefaults( const QStringList &terse, const QStringList &verbose, const TFormatMap &formatExtensions )
    {
        fTerseEncoderFormats = terse;
        fVerboseEncoderFormats = verbose;
        fMediaEncoderFormatExtensions = formatExtensions;
        computeReverseExtensionMap( true );
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initDecoderFormatsFromDefaults( const QStringList &terse, const QStringList &verbose, const TFormatMap &formatExtensions )
    {
        fTerseDecoderFormats = terse;
        fVerboseDecoderFormats = verbose;
        fMediaDecoderFormatExtensions = formatExtensions;
        computeReverseExtensionMap( false );
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initVideoEncoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fVideoEncoderCodecsTerse = terse;
        fVideoEncoderCodecsVerbose = verbose;

        //cleanLists( R"((.*_qsv.*))", R"((.*_nvenc.*))", R"((.*_amf.*))", fVideoEncoderCodecsTerse, fVideoEncoderCodecsVerbose );

        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initVideoDecoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fVideoDecoderCodecsTerse = terse;
        fVideoDecoderCodecsVerbose = verbose;

        //cleanLists( R"((.*_qsv.*))", R"((.*_nvenc.*))", R"((.*_amf.*))", fVideoDecoderCodecsTerse, fVideoDecoderCodecsVerbose );

        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initHWAccelsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fHWAccelsTerse = terse;
        fHWAccelsVerbose = verbose;

        //cleanLists( R"((.*qsv.*))", R"((.*nvenc.*))", R"((.*amf.*))", fHWAccelsTerse, fHWAccelsVerbose );

        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initAudioEncoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fAudioEncoderCodecsTerse = terse;
        fAudioEncoderCodecsVerbose = verbose;
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initAudioDecoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fAudioDecoderCodecsTerse = terse;
        fAudioDecoderCodecsVerbose = verbose;
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initSubtitleEncoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fSubtitleEncoderCodecsTerse = terse;
        fSubtitleEncoderCodecsVerbose = verbose;
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initSubtitleDecoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fSubtitleDecoderCodecsTerse = terse;
        fSubtitleDecoderCodecsVerbose = verbose;
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::setFFMpegExecutable( const QString &ffmpegExe )
    {
        fFFMpegExe = ffmpegExe;
        Q_ASSERT( validateFFMpegExe() );
        clear();
    }

    void CFFMpegFormats::recompute( QProgressDialog *dlg )
    {
        clear();
        Q_ASSERT( validateFFMpegExe() );
        if ( !validateFFMpegExe() )
            return;

        loadFormats( dlg );
        computeReverseExtensionMap( true );
        computeReverseExtensionMap( false );
        loadCodecs( dlg );
        loadHWAccels( dlg );
        fLoaded = true;
        Q_ASSERT( validate() );
    }

    bool CFFMpegFormats::isEncoderFormat( const QString &suffix, const QString &formatName ) const
    {
        Q_ASSERT( loaded() );
        auto realSuffix = suffix;
        if ( !realSuffix.startsWith( "*." ) )
            realSuffix = "*." + realSuffix;
        return getEncoderExtensionsForFormat( formatName ).contains( realSuffix );
    }

    bool CFFMpegFormats::isDecoderFormat( const QString &suffix, const QString &formatName ) const
    {
        Q_ASSERT( loaded() );
        auto realSuffix = suffix;
        if ( !realSuffix.startsWith( "*." ) )
            realSuffix = "*." + realSuffix;
        return getDecoderExtensionsForFormat( formatName ).contains( realSuffix );
    }

    QStringList CFFMpegFormats::encoderFormats( bool verbose ) const
    {
        if ( verbose )
            return fVerboseEncoderFormats;
        else
            return fTerseEncoderFormats;
    }

    QStringList CFFMpegFormats::decoderFormats( bool verbose ) const
    {
        if ( verbose )
            return fVerboseDecoderFormats;
        else
            return fTerseDecoderFormats;
    }

    QStringList CFFMpegFormats::audioEncoderCodecs( bool verbose ) const
    {
        if ( verbose )
            return fAudioEncoderCodecsVerbose;
        else
            return fAudioEncoderCodecsTerse;
    }

    QStringList CFFMpegFormats::audioDecoderCodecs( bool verbose ) const
    {
        if ( verbose )
            return fAudioDecoderCodecsVerbose;
        else
            return fAudioDecoderCodecsTerse;
    }

    QStringList CFFMpegFormats::videoEncoderCodecs( bool verbose ) const
    {
        if ( verbose )
            return fVideoEncoderCodecsVerbose;
        else
            return fVideoEncoderCodecsTerse;
    }

    QStringList CFFMpegFormats::videoDecoderCodecs( bool verbose ) const
    {
        if ( verbose )
            return fVideoDecoderCodecsVerbose;
        else
            return fVideoDecoderCodecsTerse;
    }

    QStringList CFFMpegFormats::subtitleEncoderCodecs( bool verbose ) const
    {
        if ( verbose )
            return fSubtitleEncoderCodecsVerbose;
        else
            return fSubtitleEncoderCodecsTerse;
    }

    QStringList CFFMpegFormats::subtitleDecoderCodecs( bool verbose ) const
    {
        if ( verbose )
            return fSubtitleDecoderCodecsVerbose;
        else
            return fSubtitleDecoderCodecsTerse;
    }

    QStringList CFFMpegFormats::hwAccels( bool verbose ) const
    {
        if ( verbose )
            return fHWAccelsVerbose;
        else
            return fHWAccelsTerse;
    }

    void CFFMpegFormats::checkLoaded()
    {
        bool loaded =      !fTerseEncoderFormats.isEmpty() && !fVerboseEncoderFormats.isEmpty() /* && !fMediaFormatExtensions.empty() && !fReverseMediaFormatExtensions.empty() */;
        loaded = loaded && !fTerseDecoderFormats.isEmpty() && !fVerboseDecoderFormats.isEmpty() /* && !fMediaFormatExtensions.empty() && !fReverseMediaFormatExtensions.empty() */;

        loaded = loaded && !fAudioEncoderCodecsTerse.isEmpty() && !fAudioEncoderCodecsVerbose.isEmpty();
        loaded = loaded && !fAudioDecoderCodecsTerse.isEmpty() && !fAudioDecoderCodecsVerbose.isEmpty();

        loaded = loaded && !fVideoEncoderCodecsTerse.isEmpty() && !fVideoEncoderCodecsVerbose.isEmpty();
        loaded = loaded && !fVideoDecoderCodecsTerse.isEmpty() && !fVideoDecoderCodecsVerbose.isEmpty();

        loaded = loaded && !fSubtitleEncoderCodecsTerse.isEmpty() && !fSubtitleEncoderCodecsVerbose.isEmpty();
        loaded = loaded && !fSubtitleDecoderCodecsTerse.isEmpty() && !fSubtitleDecoderCodecsVerbose.isEmpty();

        loaded = loaded && !fSubtitleEncoderCodecsTerse.isEmpty() && !fSubtitleEncoderCodecsVerbose.isEmpty();
        loaded = loaded && !fSubtitleDecoderCodecsTerse.isEmpty() && !fSubtitleDecoderCodecsVerbose.isEmpty();

        loaded = loaded && !fHWAccelsTerse.isEmpty() && !fHWAccelsVerbose.isEmpty();
        fLoaded = loaded;
    }

    void CFFMpegFormats::clear()
    {
        fLoaded = false;

        fTerseEncoderFormats.clear();
        fVerboseEncoderFormats.clear();
        fTerseDecoderFormats.clear();
        fVerboseDecoderFormats.clear();

        fMediaEncoderFormatExtensions.clear();
        fReverseMediaEncoderFormatExtensions.clear();

        fMediaDecoderFormatExtensions.clear();
        fReverseMediaDecoderFormatExtensions.clear();

        fAudioEncoderCodecsTerse.clear();
        fAudioEncoderCodecsVerbose.clear();
        fVideoEncoderCodecsTerse.clear();
        fVideoEncoderCodecsVerbose.clear();
        fSubtitleEncoderCodecsTerse.clear();
        fSubtitleEncoderCodecsVerbose.clear();

        fAudioDecoderCodecsTerse.clear();
        fAudioDecoderCodecsVerbose.clear();
        fVideoDecoderCodecsTerse.clear();
        fVideoDecoderCodecsVerbose.clear();
        fSubtitleDecoderCodecsTerse.clear();
        fSubtitleDecoderCodecsVerbose.clear();

        fHWAccelsTerse.clear();
        fHWAccelsVerbose.clear();

        fImageFormats.reset();
    }

    bool CFFMpegFormats::validateFFMpegExe() const
    {
        return validateFFMpegExe( fFFMpegExe );
    }

    bool CFFMpegFormats::validateFFMpegExe( const QString &ffmpegExe )
    {
        if ( ffmpegExe.isEmpty() )
            return false;
        auto fi = QFileInfo( ffmpegExe );
        bool aOK = fi.exists() && fi.isExecutable();
        return aOK;
    }

    bool CFFMpegFormats::validate() const
    {
        bool aOK = loaded();
        aOK = ( fTerseEncoderFormats.count() == fVerboseEncoderFormats.count() ) && aOK;
        aOK = ( fTerseDecoderFormats.count() == fVerboseDecoderFormats.count() ) && aOK;
        //aOK = ( fTerseFormats.count() == fMediaFormatExtensions.size() ) && aOK; // not every format has known extensions
        
        aOK = ( fAudioEncoderCodecsTerse.count() == fAudioEncoderCodecsVerbose.count() ) && aOK;
        aOK = ( fAudioDecoderCodecsTerse.count() == fAudioDecoderCodecsVerbose.count() ) && aOK;
        
        aOK = ( fVideoEncoderCodecsTerse.count() == fVideoEncoderCodecsVerbose.count() ) && aOK;
        aOK = ( fVideoDecoderCodecsTerse.count() == fVideoDecoderCodecsVerbose.count() ) && aOK;
        
        aOK = ( fSubtitleEncoderCodecsTerse.count() == fSubtitleEncoderCodecsVerbose.count() ) && aOK;
        aOK = ( fSubtitleDecoderCodecsTerse.count() == fSubtitleDecoderCodecsVerbose.count() ) && aOK;

        aOK = ( fHWAccelsTerse.count() == fHWAccelsVerbose.count() ) && aOK;
        return aOK;
    }

    QStringList CFFMpegFormats::getEncoderExtensions( NSABUtils::EFormatType extensionType ) const
    {
        auto pos = fMediaEncoderFormatExtensions.find( extensionType );
        if ( pos == fMediaEncoderFormatExtensions.end() )
            return {};
        QStringList retVal;
        for ( auto &&ii : ( *pos ).second )
        {
            retVal << ii.second;
        }
        return retVal;
    }

    QStringList CFFMpegFormats::getDecoderExtensions( NSABUtils::EFormatType extensionType ) const
    {
        auto pos = fMediaDecoderFormatExtensions.find( extensionType );
        if ( pos == fMediaDecoderFormatExtensions.end() )
            return {};
        QStringList retVal;
        for ( auto &&ii : ( *pos ).second )
        {
            retVal << ii.second;
        }
        return retVal;
    }

    QStringList CFFMpegFormats::getVideoEncoderExtensions() const
    {
        return getEncoderExtensions( EFormatType::eVideo );
    }

    QStringList CFFMpegFormats::getAudioEncoderExtensions() const
    {
        return getEncoderExtensions( EFormatType::eAudio );
    }

    QStringList CFFMpegFormats::getSubtitleEncoderExtensions() const
    {
        return getEncoderExtensions( EFormatType::eSubtitle );
    }

    QStringList CFFMpegFormats::getVideoDecoderExtensions() const
    {
        return getDecoderExtensions( EFormatType::eVideo );
    }

    QStringList CFFMpegFormats::getAudioDecoderExtensions() const
    {
        return getDecoderExtensions( EFormatType::eAudio );
    }

    QStringList CFFMpegFormats::getSubtitleDecoderExtensions() const
    {
        return getDecoderExtensions( EFormatType::eSubtitle );
    }

    QStringList CFFMpegFormats::getImageEncoderExtensions() const
    {
        return getEncoderExtensions( EFormatType::eImage );
    }

    QStringList CFFMpegFormats::getImageDecoderExtensions() const
    {
        return getDecoderExtensions( EFormatType::eImage );
    }

    QString CFFMpegFormats::getPrimaryEncoderExtensionForFormat( const QString &formatName ) const
    {
        auto exts = getEncoderExtensionsForFormat( formatName );
        if ( exts.isEmpty() )
            return {};
        auto retVal = exts.front();
        retVal.remove( "*." );
        return retVal;
    }

    QString CFFMpegFormats::getPrimaryDecoderExtensionForFormat( const QString &formatName ) const
    {
        auto exts = getDecoderExtensionsForFormat( formatName );
        if ( exts.isEmpty() )
            return {};
        auto retVal = exts.front();
        retVal.remove( "*." );
        return retVal;
    }

    std::optional< QStringList > CFFMpegFormats::encoderFormatLoaded( const QString &formatName ) const
    {
        for ( auto &&ii : fMediaEncoderFormatExtensions )
        {
            auto pos = ii.second.find( formatName );
            if ( pos == ii.second.end() )
                return {};
            return ( *pos ).second;
        }
        return {};
    }

    std::optional< QStringList > CFFMpegFormats::decoderFormatLoaded( const QString &formatName ) const
    {
        for ( auto &&ii : fMediaDecoderFormatExtensions )
        {
            auto pos = ii.second.find( formatName );
            if ( pos == ii.second.end() )
                return {};
            return ( *pos ).second;
        }
        return {};
    }

    QStringList CFFMpegFormats::getEncoderExtensionsForFormat( const QString &formatName ) const
    {
        Q_ASSERT( loaded() );
        for ( auto &&ii : fMediaEncoderFormatExtensions )
        {
            auto pos = ii.second.find( formatName );
            if ( pos == ii.second.end() )
                return {};
            return ( *pos ).second;
        }
        return {};
    }

    QStringList CFFMpegFormats::getDecoderExtensionsForFormat( const QString &formatName ) const
    {
        Q_ASSERT( loaded() );
        for ( auto &&ii : fMediaDecoderFormatExtensions )
        {
            auto pos = ii.second.find( formatName );
            if ( pos == ii.second.end() )
                return {};
            return ( *pos ).second;
        }
        return {};
    }

    QString CFFMpegFormats::getTranscodeHWAccel( const QString &formatName ) const
    {
        if ( formatName.contains( "_qsv" ) )
            return "qsv";
        if ( formatName.contains( "_nvenc" ) )
            return "cuda";
        if ( formatName.contains( "_amf" ) )
            return "amd";
        return {};
    }

    QString CFFMpegFormats::getCodecForHWAccel( const QString &hwAccel ) const
    {
        if ( hwAccel.contains( "cuda" ) )
            return "hevc_nvenc";
        if ( hwAccel.contains( "qsv" ) )
            return "hevc_qsv";
        if ( hwAccel.contains( "amd" ) )
            return "hevc_amf";
        return {};
    }

    void CFFMpegFormats::loadCodecs( QProgressDialog *dlg )
    {
        loadCodecs( true, dlg );
        loadCodecs( false, dlg );
    }

    void CFFMpegFormats::loadCodecs( bool isEncoding, QProgressDialog *dlg )
    {
        if ( !validateFFMpegExe() )
            return;

        if ( dlg )
            dlg->setLabelText( QString( "Loading %1 Codecs" ).arg( isEncoding ? "Decoder" : "Decoder" ) );

        QProcess process;
        process.start( fFFMpegExe, QStringList() << "-hide_banner" << ( isEncoding ? "-encoders" : "-decoders" ) );
        process.waitForFinished();
        auto codecs = process.readAllStandardOutput();

        auto pos = codecs.indexOf( "------" );
        if ( pos == -1 )
            return;

        codecs = codecs.mid( pos + 6 );
        /*
        *Encoders/Decoders:
            V..... = Video
            A..... = Audio
            S..... = Subtitle
        .   F.... = Frame-level multithreading
            .S... = Slice-level multithreading
            ...  X.. = Codec is experimental
        ....   B. = Supports draw_horiz_band
        .....   D = Supports direct rendering method 1
                    
                    
            V....D a64multi             Multicolor charset for Commodore 64 (codec a64_multi)
    */
        auto regEx = QRegularExpression( R"((?<type>[VAS][FSXBD\.]{5})\s+(?<name>\S+)\s+(?<desc>.*))" );
        auto ii = regEx.globalMatch( codecs );
        if ( dlg )
        {
            int count = 0;
            while ( ii.hasNext() )
            {
                count++;
                ii.next();
            }
            ii = regEx.globalMatch( codecs );
            dlg->setRange( 0, count );
            dlg->setValue( 0 );
        }

        while ( ii.hasNext() )
        {
            if ( dlg )
                dlg->setValue( dlg->value() + 1 );
            auto match = ii.next();
            auto type = match.captured( "type" );
            if ( type.length() != 6 )
                continue;
            if ( type[ 3 ] == 'X' )
                continue;
            if ( ( type[ 0 ] != 'V' ) && ( type[ 0 ] != 'A' ) && ( type[ 0 ] != 'S' ) )
                continue;
            auto name = match.captured( "name" ).trimmed();
            auto desc = match.captured( "desc" ).trimmed();
            switch ( type[ 0 ].toLatin1() )
            {
                case 'A':
                    if ( isEncoding )
                    {
                        fAudioEncoderCodecsTerse.push_back( name );
                        fAudioEncoderCodecsVerbose.push_back( name + " - " + desc );
                    }
                    else
                    {
                        fAudioDecoderCodecsTerse.push_back( name );
                        fAudioDecoderCodecsVerbose.push_back( name + " - " + desc );
                    }
                    break;
                case 'V':
                    if ( isEncoding )
                    {
                        fVideoEncoderCodecsTerse.push_back( name );
                        fVideoEncoderCodecsVerbose.push_back( name + " - " + desc );
                    }
                    else
                    {
                        fVideoDecoderCodecsTerse.push_back( name );
                        fVideoDecoderCodecsVerbose.push_back( name + " - " + desc );
                    }
                    break;
                case 'S':
                    if ( isEncoding )
                    {
                        fSubtitleEncoderCodecsTerse.push_back( name );
                        fSubtitleEncoderCodecsVerbose.push_back( name + " - " + desc );
                    }
                    else
                    {
                        fSubtitleDecoderCodecsTerse.push_back( name );
                        fSubtitleDecoderCodecsVerbose.push_back( name + " - " + desc );
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void CFFMpegFormats::loadHWAccels( QProgressDialog *dlg )
    {
        if ( !validateFFMpegExe() )
            return;

        if ( dlg )
            dlg->setLabelText( "Loading HW Accelerators" );

        QProcess process;
        process.start(
            fFFMpegExe, QStringList() << "-hide_banner"
                                      << "-hwaccels" );
        process.waitForFinished();
        auto hwaccels = process.readAllStandardOutput();

        auto pos = hwaccels.indexOf( ":" );
        if ( pos == -1 )
            return;

        hwaccels = hwaccels.mid( pos + 1 );
        /*
            * Hardware acceleration methods:
            cuda
            dxva2
            qsv
            d3d11va
        */
        auto regEx = QRegularExpression( R"(\s*(?<hwaccel>\S+))" );
        auto ii = regEx.globalMatch( hwaccels );
        if ( dlg )
        {
            int count = 0;
            while ( ii.hasNext() )
            {
                count++;
                ii.next();
            }
            ii = regEx.globalMatch( hwaccels );
            dlg->setRange( 0, count );
            dlg->setValue( 0 );
        }

        while ( ii.hasNext() )
        {
            if ( dlg )
                dlg->setValue( dlg->value() + 1 );
            auto match = ii.next();
            auto type = match.captured( "hwaccel" ).trimmed().toLower();
            if ( type == "none" )
                fHWAccelsVerbose.push_back( type + " - Do not use any hardware acceleration (the default)." );
            else if ( type == "auto" )
                fHWAccelsVerbose.push_back( type + " - Automatically select the hardware acceleration method." );
            else if ( type == "vdpau" )
                fHWAccelsVerbose.push_back( type + " - Use VDPAU (Video Decode and Presentation API for Unix) hardware acceleration." );
            else if ( type == "dxva2" )
                fHWAccelsVerbose.push_back( type + " - Use DXVA2 (DirectX Video Acceleration) API hardware acceleration." );
            else if ( type == "d3d11va" )
                fHWAccelsVerbose.push_back( type + " - Use D3D11VA (DirectX Video Acceleration) API hardware acceleration." );
            else if ( type == "vaapi" )
                fHWAccelsVerbose.push_back( type + " - Use VAAPI (Video Acceleration API) hardware acceleration." );
            else if ( type == "qsv" )
                fHWAccelsVerbose.push_back( type + " - Use the Intel QuickSync Video acceleration for video transcoding." );
            else if ( type == "cuda" )
                fHWAccelsVerbose.push_back( type + " - Use the nVidia Cuda Video acceleration for video transcoding." );
            else if ( type == "amf" )
                fHWAccelsVerbose.push_back( type + " - Use the AMD Advanced Media Framework Video acceleration for video transcoding." );
            else if ( type == "opencl" )
                fHWAccelsVerbose.push_back( type + " - Use OpenCL hardware acceleration for API video transcoding.)" );   //
            else if ( type == "vulkan" )
                fHWAccelsVerbose.push_back( type + " - Use the Vulkan hardware acceleration API for video transcoding.)" );   //
            else
                continue;
            fHWAccelsTerse.push_back( type );
        }
    }

    void CFFMpegFormats::loadFormats( QProgressDialog *dlg )
    {
        if ( !validateFFMpegExe() )
            return;

        if ( dlg )
            dlg->setLabelText( "Loading Formats" );

        QProcess process;
        process.start(
            fFFMpegExe, QStringList() << "-hide_banner"
                                      << "-formats" );
        process.waitForFinished();
        auto formats = process.readAllStandardOutput();

        auto pos = formats.indexOf( "--" );
        if ( pos == -1 )
            return;

        formats = formats.mid( pos + 2 );
        /*
        * Encoers/Decoders:
            D. = Demuxing supported
            .E = Muxing supported
                        
            D  3dostr          3DO STR
            E 3g2             3GP2 (3GPP2 file format)
        */

        auto regEx = QRegularExpression( R"((?<type>[DE]{1,2})\s+(?<name>\S+)\s+(?<desc>.*))" );
        auto ii = regEx.globalMatch( formats );
        if ( dlg )
        {
            int count = 0;
            while ( ii.hasNext() )
            {
                count++;
                ii.next();
            }
            ii = regEx.globalMatch( formats );
            dlg->setRange( 0, count );
            dlg->setValue( 0 );
        }

        while ( ii.hasNext() )
        {
            if ( dlg )
                dlg->setValue( dlg->value() + 1 );
            auto match = ii.next();
            auto type = match.captured( "type" ).trimmed();
            bool isEncoder = ( ( type.length() == 1 ) && ( type[ 0 ] == 'E' ) ) || ( ( type.length() == 2 ) && ( type[ 1 ] == 'E' ) );
            bool isDecoder = ( ( type.length() == 1 ) && ( type[ 0 ] == 'D' ) ) || ( ( type.length() == 2 ) && ( type[ 0 ] == 'E' ) );
            if ( !isEncoder && !isDecoder )
                continue;
            auto names = match.captured( "name" ).trimmed().split( "," );
            for ( auto && ii : names )
                ii = ii.trimmed();
            auto desc = match.captured( "desc" ).trimmed();

            if ( isEncoder )
                computeExtensionsForFormat( names, desc, true );
            if ( isDecoder )
                computeExtensionsForFormat( names, desc, false );
        }
    }

    void CFFMpegFormats::computeExtensionsForFormat( const QStringList & names, const QString & desc, bool isEncoder )
    {
        for ( auto &&name : names )
        {
            auto exts = computeExtensionsForFormat( name, isEncoder );
            if ( exts.empty() )
                continue;

            if ( isEncoder )
            {
                fTerseEncoderFormats.push_back( name );
                fVerboseEncoderFormats.push_back( name + " - " + desc + " (" + exts.join( ";" ) + ")" );
            }
            else
            {
                fTerseDecoderFormats.push_back( name );
                fVerboseDecoderFormats.push_back( name + " - " + desc + " (" + exts.join( ";" ) + ")" );
            }
        }
    }

    QStringList CFFMpegFormats::computeExtensionsForFormat( const QString &formatName, bool encoders )
    {
        if ( !validateFFMpegExe() )
            return {};

        auto retVal = encoders ? encoderFormatLoaded( formatName ) : decoderFormatLoaded( formatName );
        if ( !retVal.has_value() )
        {
            retVal = QStringList();
            QProcess process;
            process.start(
                fFFMpegExe, QStringList() << "-hide_banner"
                                          << "-h"
                                          << ( encoders ? "muxer=" : "demuxer=" ) + formatName );
            process.waitForFinished();
            auto formatHelp = process.readAllStandardOutput();

            // Common extensions: 3g2.
            auto regEx = QRegularExpression( R"(Common extensions\:\s*(?<exts>.*)\.)" );
            auto match = regEx.match( formatHelp );
            if ( match.hasMatch() )
            {
                auto tmp = match.captured( "exts" ).trimmed();
                retVal = match.captured( "exts" ).trimmed().split( "," );
                for ( auto &&ii : retVal.value() )
                    ii = "*." + ii.trimmed().toLower();
                retVal.value().removeDuplicates();
            }

            std::list< EFormatType > formats;
            auto lc = formatName.toLower();
            if ( ( lc == "image2" ) || ( lc == "webp" ) )
                formats.push_back( EFormatType::eImage );
            else
            {
                // Mime type: video/x-matroska.
                regEx = QRegularExpression( R"(Mime type\:\s*(?<mimetype>.*)\.)" );
                match = regEx.match( formatHelp );
                if ( match.hasMatch() )
                {
                    auto mime = match.captured( "mimetype" ).trimmed().toLower();
                    if ( mime.contains( "video" ) )
                        formats.push_back( EFormatType::eVideo );
                    if ( mime.contains( "audio" ) )
                        formats.push_back( EFormatType::eAudio );
                    if ( mime.contains( "subtitle" ) )
                        formats.push_back( EFormatType::eSubtitle );
                    if ( mime.contains( "image" ) )
                        formats.push_back( EFormatType::eImage );
                }

                if ( formats.empty() )
                {
                    regEx = QRegularExpression( R"(Default\s+(?<codec>.*)\s+codec\:)" );
                    auto ii = regEx.globalMatch( formatHelp );

                    while( ii.hasNext() )
                    {
                        auto match = ii.next();

                        auto codec = match.captured( "codec" ).toLower().trimmed();
                        if ( codec == "video" )
                            formats.push_back( EFormatType::eVideo );
                        else if ( codec == "audio" )
                            formats.push_back( EFormatType::eAudio );
                        else if ( codec == "subtitle" )
                            formats.push_back( EFormatType::eSubtitle );
                        // images will return video codec
                    }
                }
            }
            if ( !encoders && formats.empty() )
                formats.push_back( EFormatType::eVideo );
            for ( auto &&format : formats )
            {
                if ( encoders )
                    fMediaEncoderFormatExtensions[ format ][ formatName ] = retVal.value();
                else
                    fMediaDecoderFormatExtensions[ format ][ formatName ] = retVal.value();
            }
        }
        return retVal.value();
    }
   

    bool CFFMpegFormats::isImageFormat( const QString &ext ) const
    {
        if ( !fImageFormats.has_value() )
        {
            fImageFormats = std::unordered_set< QString >();
            auto tmp = QImageReader::supportedImageFormats();
            for ( auto &&ii : tmp )
            {
                fImageFormats.value().insert( "*." + ii );
                fImageFormats.value().insert( ii );
            }
        }

        return ( fImageFormats.value().find( ext ) != fImageFormats.value().end() );
    }

    void CFFMpegFormats::computeReverseExtensionMap( bool encoders )
    {
        auto &&whichReversMap = encoders ? fReverseMediaEncoderFormatExtensions : fReverseMediaDecoderFormatExtensions;
        auto &&whichExtensions = encoders ? fMediaEncoderFormatExtensions : fMediaDecoderFormatExtensions;

        whichReversMap.clear();

        for ( auto &&ii : whichExtensions )
        {
            for ( auto &&jj : ii.second )
            {
                for ( auto &&kk : jj.second )
                {
                    if ( whichReversMap.find( kk ) == whichReversMap.end() )
                        whichReversMap[ kk ] = jj.first;
                }
            }
        }
    }

    QString toString( EFormatType type )
    {
        switch ( type )
        {
            case EFormatType::eUnknown:
                return "NSABUtils::EFormatType::eUnknown";
            case EFormatType::eVideo:
                return "NSABUtils::EFormatType::eVideo";
            case EFormatType::eAudio:
                return "NSABUtils::EFormatType::eAudio";
            case EFormatType::eSubtitle:
                return "NSABUtils::EFormatType::eSubtitle";
            case EFormatType::eImage:
                return "NSABUtils::EFormatType::eImage";
        }
        return toString( EFormatType::eUnknown );
    }

}
