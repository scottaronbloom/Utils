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

        void initEncoderFormatsFromDefaults( const QStringList &terse, const QStringList &verbose, const TFormatMap &formatExtensions );
        void initDecoderFormatsFromDefaults( const QStringList &terse, const QStringList &verbose, const TFormatMap &formatExtensions );
        void initVideoEncoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );
        void initAudioEncoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );
        void initSubtitleEncoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );
        void initVideoDecoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );
        void initAudioDecoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );
        void initSubtitleDecoderCodecsFromDefaults( const QStringList &terse, const QStringList &verbose );

        void initHWAccelsFromDefaults( const QStringList &terse, const QStringList &verbose );

        void setFFMpegExecutable( const QString &ffmpegExe );
        void recompute( QProgressDialog * dlg = nullptr );
        bool loaded() const { return fLoaded; }
        TFormatMap mediaEncoderFormatExtensions() const { return fMediaEncoderFormatExtensions; }
        TFormatMap mediaDecoderFormatExtensions() const { return fMediaDecoderFormatExtensions; }

        bool isEncoderFormat( const QString & suffix, const QString & formatName ) const;
        bool isDecoderFormat( const QString &suffix, const QString &formatName ) const;
        QStringList encoderFormats( bool verbose ) const;
        QStringList decoderFormats( bool verbose ) const;

        QStringList audioEncoderCodecs( bool verbose ) const;
        QStringList videoEncoderCodecs( bool verbose ) const;
        QStringList subtitleEncoderCodecs( bool verbose ) const;
        QStringList audioDecoderCodecs( bool verbose ) const;
        QStringList videoDecoderCodecs( bool verbose ) const;
        QStringList subtitleDecoderCodecs( bool verbose ) const;
        QStringList hwAccels( bool verbose ) const;

        QStringList getVideoEncoderExtensions() const;
        QStringList getSubtitleEncoderExtensions() const;
        QStringList getAudioEncoderExtensions() const;

        QStringList getVideoDecoderExtensions() const;
        QStringList getSubtitleDecoderExtensions() const;
        QStringList getAudioDecoderExtensions() const;

        QStringList getImageEncoderExtensions() const;
        QStringList getImageDecoderExtensions() const;

        QStringList getEncoderExtensions( NSABUtils::EFormatType extensionType ) const;
        QStringList getDecoderExtensions( NSABUtils::EFormatType extensionType ) const;

        QString getPrimaryEncoderExtensionForFormat( const QString &formatName ) const;
        QStringList getEncoderExtensionsForFormat( const QString &formatName ) const;

        QString getPrimaryDecoderExtensionForFormat( const QString &formatName ) const;
        QStringList getDecoderExtensionsForFormat( const QString &formatName ) const;

        QString getTranscodeHWAccel( const QString &formatName ) const;
        QString getCodecForHWAccel( const QString & hwAccel ) const;

        static bool validateFFMpegExe( const QString &ffmpegExe );
    private:
        bool validate() const;
        void checkLoaded();

        void clear();
        bool validateFFMpegExe() const;

        void loadCodecs( QProgressDialog *dlg );
        void loadCodecs( bool isEncoding, QProgressDialog *dlg );

        void loadFormats( QProgressDialog *dlg );

        void loadHWAccels( QProgressDialog *dlg );

        bool isImageFormat( const QString &ext ) const;
        void computeReverseExtensionMap( bool encoders );

        void computeExtensionsForFormat( const QStringList &names, const QString &desc, bool isEncoder );
        QStringList computeExtensionsForFormat( const QString &formatName, bool encoders );
        std::optional< QStringList > encoderFormatLoaded( const QString &formatName ) const;
        std::optional< QStringList > decoderFormatLoaded( const QString &formatName ) const;

        bool fLoaded{ false };
        QStringList fTerseEncoderFormats;
        QStringList fVerboseEncoderFormats;
        QStringList fTerseDecoderFormats;
        QStringList fVerboseDecoderFormats;
        std::unordered_map< EFormatType, std::unordered_map< QString, QStringList > > fMediaEncoderFormatExtensions;   //terse format name to list of known/common extensions
        std::unordered_map< QString, QString > fReverseMediaEncoderFormatExtensions;
        std::unordered_map< EFormatType, std::unordered_map< QString, QStringList > > fMediaDecoderFormatExtensions;   //terse format name to list of known/common extensions
        std::unordered_map< QString, QString > fReverseMediaDecoderFormatExtensions;

        QStringList fAudioEncoderCodecsTerse;
        QStringList fAudioEncoderCodecsVerbose;
        QStringList fVideoEncoderCodecsTerse;
        QStringList fVideoEncoderCodecsVerbose;
        QStringList fSubtitleEncoderCodecsTerse;
        QStringList fSubtitleEncoderCodecsVerbose;

        QStringList fAudioDecoderCodecsTerse;
        QStringList fAudioDecoderCodecsVerbose;
        QStringList fVideoDecoderCodecsTerse;
        QStringList fVideoDecoderCodecsVerbose;
        QStringList fSubtitleDecoderCodecsTerse;
        QStringList fSubtitleDecoderCodecsVerbose;

        QStringList fHWAccelsTerse;
        QStringList fHWAccelsVerbose;

        QString fFFMpegExe;
        mutable std::optional< std::unordered_set< QString > > fImageFormats;
    };
}

#endif
