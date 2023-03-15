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
#include <QProcess>
#include <QFileInfo>
#include <QImageReader>
#include <QProgressDialog>

namespace NSABUtils
{
    CFFMpegFormats::CFFMpegFormats()
    {
    }

    CFFMpegFormats::CFFMpegFormats( const QString &ffmpegExe )
    {
        setFFMpegExecutable( ffmpegExe );
    }

    void CFFMpegFormats::initFormatsFromDefaults( const QStringList &terse, const QStringList &verbose, const TFormatMap &formatExtensions )
    {
        fTerseFormats = terse;
        fVerboseFormats = verbose;
        fMediaFormatExtensions = formatExtensions;
        computeReverseExtensionMap();
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initVideoCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fVideoCodecsTerse = terse;
        fVideoCodecsVerbose = verbose;
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initAudioCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fAudioCodecsTerse = terse;
        fAudioCodecsVerbose = verbose;
        checkLoaded();
        if ( loaded() )
        {
            Q_ASSERT( validate() );
        }
    }

    void CFFMpegFormats::initSubtitleCodecsFromDefaults( const QStringList &terse, const QStringList &verbose )
    {
        fSubtitleCodecsTerse = terse;
        fSubtitleCodecsVerbose = verbose;
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

    void CFFMpegFormats::recompute( QProgressDialog * dlg )
    {
        clear();
        Q_ASSERT( validateFFMpegExe() );
        if ( !validateFFMpegExe() )
            return;

        loadFormats( dlg );
        computeReverseExtensionMap();
        loadCodecs( dlg );
        fLoaded = true;
        Q_ASSERT( validate() );
    }

    void CFFMpegFormats::checkLoaded()
    {
        bool loaded = !fTerseFormats.isEmpty() && !fVerboseFormats.isEmpty() /* && !fMediaFormatExtensions.empty() && !fReverseMediaFormatExtensions.empty() */;
        loaded = loaded && !fAudioCodecsTerse.isEmpty() && !fAudioCodecsVerbose.isEmpty();
        loaded = loaded && !fVideoCodecsTerse.isEmpty() && !fVideoCodecsVerbose.isEmpty();
        loaded = loaded && !fSubtitleCodecsTerse.isEmpty() && !fSubtitleCodecsVerbose.isEmpty();
        fLoaded = loaded;
    }

    void CFFMpegFormats::clear()
    {
        fLoaded = false;
        fTerseFormats.clear();
        fVerboseFormats.clear();
        fMediaFormatExtensions.clear();
        fReverseMediaFormatExtensions.clear();
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
        aOK = ( fTerseFormats.count() == fVerboseFormats.count() ) && aOK;
        //aOK = ( fTerseFormats.count() == fMediaFormatExtensions.size() ) && aOK; // not every format has known extensions
        aOK = ( fAudioCodecsTerse.count() == fAudioCodecsVerbose.count() ) && aOK;
        aOK = ( fVideoCodecsTerse.count() == fVideoCodecsVerbose.count() ) && aOK;
        aOK = ( fSubtitleCodecsTerse.count() == fSubtitleCodecsVerbose.count() ) && aOK;
        return aOK;
    }

    QStringList CFFMpegFormats::getExtensions( NSABUtils::EFormatType extensionType ) const
    {
        if ( !loaded() )
            return {};
        auto pos = fMediaFormatExtensions.find( extensionType );
        if ( pos == fMediaFormatExtensions.end() )
            return {};
        QStringList retVal;
        for ( auto &&ii : (*pos).second )
        {
            retVal << ii.second;
        }
        return retVal;
    }

    QStringList CFFMpegFormats::getVideoExtensions() const
    {
        return getExtensions( EFormatType::eVideo );
    }

    QStringList CFFMpegFormats::getAudioExtensions() const
    {
        return getExtensions( EFormatType::eAudio );
    }

    QStringList CFFMpegFormats::getSubtitleExtensions() const
    {
        return getExtensions( EFormatType::eSubtitle );
    }

    QStringList CFFMpegFormats::getImageExtensions() const
    {
        return getExtensions( EFormatType::eImage );
    }

    QString CFFMpegFormats::getPrimaryExtensionForFormat( const QString &formatName ) const
    {
        auto exts = getExtensionsForFormat( formatName );
        if ( exts.isEmpty() )
            return {};
        auto retVal = exts.front();
        retVal.remove( "*." );
        return retVal;
    }

    std::optional< QStringList > CFFMpegFormats::formatLoaded( const QString &formatName ) const
    {
        for ( auto &&ii : fMediaFormatExtensions )
        {
            auto pos = ii.second.find( formatName );
            if ( pos == ii.second.end() )
                return {};
            return ( *pos ).second;
        }
        return {};
    }

    QStringList CFFMpegFormats::getExtensionsForFormat( const QString &formatName ) const
    {
        Q_ASSERT( loaded() );
        for ( auto &&ii : fMediaFormatExtensions )
        {
            auto pos = ii.second.find( formatName );
            if ( pos == ii.second.end() )
                return {};
            return ( *pos ).second;
        }
        return {};
    }

    void CFFMpegFormats::loadCodecs( QProgressDialog *dlg )
    {
        if ( !validateFFMpegExe() )
            return;

        if ( dlg )
            dlg->setLabelText( "Loading Codecs" );

        QProcess process;
        process.start(
            fFFMpegExe, QStringList() << "-hide_banner"
                                      << "-encoders" );
        process.waitForFinished();
        auto codecs = process.readAllStandardOutput();

        auto pos = codecs.indexOf( "------" );
        if ( pos == -1 )
            return;

        codecs = codecs.mid( pos + 6 );
        /*
            * Encoders:
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
                    fAudioCodecsTerse.push_back( name );
                    fAudioCodecsVerbose.push_back( name + " - " + desc );
                    break;
                case 'V':
                    fVideoCodecsTerse.push_back( name );
                    fVideoCodecsVerbose.push_back( name + " - " + desc );
                    break;
                case 'S':
                    fSubtitleCodecsTerse.push_back( name );
                    fSubtitleCodecsVerbose.push_back( name + " - " + desc );
                    break;
                default:
                    break;
            }
        }
    }

    void CFFMpegFormats::loadFormats( QProgressDialog * dlg )
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
        * Encoders:
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
            auto type = match.captured( "type" );
            if ( ( type.length() == 1 ) && ( type[ 0 ] != 'E' ) )
                continue;
            if ( ( type.length() == 2 ) && ( type[ 1 ] != 'E' ) )
                continue;
            auto names = match.captured( "name" ).trimmed().split( "," );
            for ( auto &&ii : names )
                ii = ii.trimmed();
            auto desc = match.captured( "desc" ).trimmed();

            for ( auto &&name : names )
            {
                auto exts = computeExtensionsForFormat( name );
                if ( exts.empty() )
                    continue;

                fTerseFormats.push_back( name );
                fVerboseFormats.push_back( name + " - " + desc + " (" + exts.join( ";" ) + ")" );
            }
        }
    }

    QStringList CFFMpegFormats::computeExtensionsForFormat( const QString &formatName )
    {
        if ( !validateFFMpegExe() )
            return {};

        auto retVal = formatLoaded( formatName );
        if ( !retVal.has_value() )
        {
            retVal = QStringList();
            QProcess process;
            process.start(
                fFFMpegExe, QStringList() << "-hide_banner"
                                          << "-h"
                                          << "muxer=" + formatName );
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

            auto formatType = EFormatType::eUnknown;
            auto lc = formatName.toLower();
            if ( ( lc == "image2" ) || ( lc == "webp" ) )
                formatType = EFormatType::eImage;
            else
            {
                // Mime type: video/x-matroska.
                regEx = QRegularExpression( R"(Mime type\:\s*(?<mimetype>.*)\.)" );
                match = regEx.match( formatHelp );
                if ( match.hasMatch() )
                {
                    auto mime = match.captured( "mimetype" ).trimmed().toLower();
                    if ( mime.contains( "video" ) )
                        formatType = EFormatType::eVideo;
                    else if ( mime.contains( "audio" ) )
                        formatType = EFormatType::eAudio;
                    else if ( mime.contains( "subtitle" ) )
                        formatType = EFormatType::eSubtitle;
                    else if ( mime.contains( "image" ) )
                        formatType = EFormatType::eImage;
                }

                if ( formatType == EFormatType::eUnknown )
                {
                    regEx = QRegularExpression( R"(Default\s+(?<codec>.*)\s+codec\:)" );
                    match = regEx.match( formatHelp );

                    if ( match.hasMatch() )
                    {
                        auto codec = match.captured( "codec" ).toLower().trimmed();
                        if ( codec == "video" )
                            formatType = EFormatType::eVideo;
                        else if ( codec == "audio" )
                            formatType = EFormatType::eAudio;
                        else if ( codec == "subtitle" )
                            formatType = EFormatType::eSubtitle;
                        // images will return video codec
                    }
                }
            }
            if ( formatType == EFormatType::eUnknown )
                return {};
            fMediaFormatExtensions[ formatType ][ formatName ] = retVal.value();
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

    void CFFMpegFormats::computeReverseExtensionMap()
    {
        fReverseMediaFormatExtensions.clear();
        for ( auto &&ii : fMediaFormatExtensions )
        {
            for ( auto &&jj : ii.second )
            {
                for ( auto &&kk : jj.second )
                {
                    if ( fReverseMediaFormatExtensions.find( kk ) == fReverseMediaFormatExtensions.end() )
                        fReverseMediaFormatExtensions[ kk ] = jj.first;
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

