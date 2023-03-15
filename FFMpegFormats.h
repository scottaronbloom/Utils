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

#ifndef _SABUTILS_FFMPEGUTILS_H
#define _SABUTILS_FFMPEGUTILS_H

#include "SABUtilsExport.h"
#include <QStringList>
#include <QString>

#include <unordered_map>
#include <unordered_set>
#include <optionaL>
class QProgressDialog;
namespace NSABUtils
{
    enum class EFormatType
    {
        eUnknown,
        eVideo,
        eAudio,
        eSubtitle,
        eImage
    };
    SABUTILS_EXPORT QString toString( EFormatType type );
    using TFormatMap = std::unordered_map< EFormatType, std::unordered_map< QString, QStringList > >;

    class SABUTILS_EXPORT CFFMpegFormats
    {
    public:
        CFFMpegFormats();
        CFFMpegFormats( const QString &ffmpegExe );

        void initFormatsFromDefaults( const QStringList &terse, const QStringList &verbose, const TFormatMap &formatExtensions );
        void initVideoCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );
        void initAudioCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );
        void initSubtitleCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );

        void setFFMpegExecutable( const QString &ffmpegExe );
        void recompute( QProgressDialog * dlg = nullptr );
        bool loaded() const { return fLoaded; }
        TFormatMap mediaFormatExtensions() const { return fMediaFormatExtensions; }

        bool isFormat( const QString & suffix, const QString & formatName ) const;
        QStringList formats( bool verbose ) const
        {
            if ( verbose )
                return fVerboseFormats;
            else
                return fTerseFormats;
        }
        QStringList audioCodecs( bool verbose ) const
        {
            if ( verbose )
                return fAudioCodecsVerbose;
            else 
                return fAudioCodecsTerse;
        }

        QStringList videoCodecs( bool verbose ) const
        {
            if ( verbose )
                return fVideoCodecsVerbose;
            else
                return fVideoCodecsTerse;
        }

        QStringList subtitleCodecs( bool verbose ) const
        {
            if ( verbose )
                return fSubtitleCodecsVerbose;
            else
                return fSubtitleCodecsTerse;
        }

        QStringList getVideoExtensions() const;
        QStringList getSubtitleExtensions() const;
        QStringList getAudioExtensions() const;
        QStringList getImageExtensions() const;

        QStringList getExtensions( NSABUtils::EFormatType extensionType ) const;

        QString getPrimaryExtensionForFormat( const QString &formatName ) const;
        QStringList getExtensionsForFormat( const QString & formatName ) const;

        static bool validateFFMpegExe( const QString &ffmpegExe );
    private:
        bool validate() const;
        void checkLoaded();

        void clear();
        bool validateFFMpegExe() const;

        void loadCodecs( QProgressDialog *dlg );
        void loadFormats( QProgressDialog *dlg );


        bool isImageFormat( const QString &ext ) const;
        void computeReverseExtensionMap();

        QStringList computeExtensionsForFormat( const QString &formatName );
        std::optional< QStringList > formatLoaded( const QString &formatName ) const;

        bool fLoaded{ false };
        QStringList fTerseFormats;
        QStringList fVerboseFormats;
        std::unordered_map< EFormatType, std::unordered_map< QString, QStringList > > fMediaFormatExtensions;   //terse format name to list of known/common extensions
        std::unordered_map< QString, QString > fReverseMediaFormatExtensions;

        QStringList fAudioCodecsTerse;
        QStringList fAudioCodecsVerbose;
        QStringList fVideoCodecsTerse;
        QStringList fVideoCodecsVerbose;
        QStringList fSubtitleCodecsTerse;
        QStringList fSubtitleCodecsVerbose;

        QString fFFMpegExe;
        mutable std::optional< std::unordered_set< QString > > fImageFormats;
    };
}

#endif
