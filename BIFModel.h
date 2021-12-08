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

<<<<<<< Updated upstream
#ifndef __BIFFILE_H
#define __BIFFILE_H

#include <QObject>
#include <QString>
#include <QImage>
#include <utility>
#include <optional>
#include <QAbstractListModel>

class QFile;
using T32BitValue = std::tuple< QByteArray, QString, uint32_t >;
struct SBIF
{
    SBIF( T32BitValue ts, T32BitValue offset, SBIF * prev );

    bool isLastFrame() const;
    [[nodiscard]] std::pair< bool, QString > loadImage( QFile *file, const QString & fn );
    T32BitValue fBIFNum;
    T32BitValue fOffset;
    uint64_t fSize{ 0 };
    std::optional< std::pair< QByteArray, QImage > > fImage;
};

using TBIFIndex = std::vector< SBIF >; // data read in of ts, pos then a pair of pos, size
class CBIFFile : public QObject
{
    Q_OBJECT;
public:
    CBIFFile( const QString & bifFile, bool loadImages, QObject * parent=nullptr );
    virtual ~CBIFFile();

    bool isValid() const { return fAOK; }
    QString errorString() const { return fErrorString; }

    QString magicNumber() const { return prettyPrint( fMagicNumber ); } // returns pretty print of the data
    const T32BitValue & version() const { return fVersion; }
    const T32BitValue & numImages() const { return fNumImages; }
    const T32BitValue & tsMultiplier() const { return fTSMultiplier; }
    QString reserved() const { return prettyPrint( fReserved ); }

    const TBIFIndex & bifs() const { return fBIFs; }

    QString fileName() const { return fBIFFile; }
    std::size_t size() const { return fBIFs.size(); }

    std::pair< bool, QString > loadImage( size_t imageNum, int *insertStart=nullptr, int *numInserted = nullptr );
    QImage image( size_t imageNum, int *insertStart = nullptr, int *numInserted = nullptr );

    int lastImageLoaded() { return fLastImageLoaded; }
    bool canLoadMoreImages()
    {
        return fLastImageLoaded < size();
    }
    int fetchSize() const { return 8; }
    void fetchMore();
private:
    void loadBIF( bool loadImages );

    bool checkForOpen();
    bool openFile();
    bool parseHeader();
    bool parseIndex( bool loadImages );

    QString prettyPrint( const QByteArray &in ) const;

    T32BitValue getValue( const QByteArray & in, std::optional< QString > desc );
    QFile *fFile{ nullptr };
    QString fBIFFile;
    bool fAOK{ false };
    QString fErrorString;
    QByteArray fMagicNumber;
    T32BitValue fVersion;
    T32BitValue fNumImages;
    T32BitValue fTSMultiplier;
    TBIFIndex fBIFs;
    QByteArray fReserved;
    int fLastImageLoaded{ 0 };
};

class CBIFModel : public QAbstractListModel
{
    Q_OBJECT;
public:
    CBIFModel( QObject * parent  = nullptr );

    void setBIFFile( CBIFFile *bifFile );
        
    int rowCount( const QModelIndex & parent ) const { return ( parent.isValid() || !fBIFFile ) ? 0 : fBIFFile->lastImageLoaded(); }
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    QImage image( size_t imageNum );
protected:
    virtual bool canFetchMore( const QModelIndex &parent ) const override;
    virtual void fetchMore( const QModelIndex &parent ) override;
private:
    CBIFFile *fBIFFile{ nullptr };
};

=======
#ifndef __BIFMODEL_H
#define __BIFMODEL_H

#include <QAbstractListModel>
#include <memory>

class QFile;
namespace NBIF
{
    class CBIFFile;
    class CBIFModel : public QAbstractListModel
    {
        Q_OBJECT;
    public:
        CBIFModel( QObject *parent = nullptr );

        void setBIFFile( std::shared_ptr< CBIFFile > bifFile );

        virtual int rowCount( const QModelIndex &parent ) const override;
        virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

        QImage image( size_t imageNum );
    protected:
        virtual bool canFetchMore( const QModelIndex &parent ) const override;
        virtual void fetchMore( const QModelIndex &parent ) override;
    private:
        std::shared_ptr< CBIFFile > fBIFFile;
    };
}
>>>>>>> Stashed changes
#endif

