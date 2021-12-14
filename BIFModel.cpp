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

#include "BIFModel.h"
#include "BIFFile.h"

#include <QPixmap>
#include <QIcon>

namespace NBIF
{
    CBIFModel::CBIFModel( QObject *parent /*= nullptr */ ) :
        QAbstractListModel( parent )
    {

    }

    void CBIFModel::setBIFFile( std::shared_ptr< CBIFFile > bifFile )
    {
        beginResetModel();
        fBIFFile = bifFile;
        endResetModel();
    }

    int CBIFModel::rowCount( const QModelIndex &parent ) const
    {
        return ( parent.isValid() || !fBIFFile ) ? 0 : fBIFFile->lastImageLoaded();
    }

    QVariant CBIFModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole */ ) const
    {
        if ( !index.isValid() )
            return QVariant();
        if ( index.row() > fBIFFile->imageCount() || index.row() < 0 )
            return QVariant();
        if ( role == Qt::DisplayRole )
            return QString( "BIF #%1" ).arg( index.row() );
        else if ( role == Qt::DecorationRole )
        {
            return QIcon( QPixmap::fromImage( fBIFFile->image( index.row() ) ) );
        }
        return QVariant();
    }

    QImage CBIFModel::image( size_t imageNum )
    {
        if ( !fBIFFile )
            return QImage();
        int insertStart = -1;
        int insertNum = -1;
        auto retVal = fBIFFile->imageToFrame( imageNum, &insertStart, &insertNum );
        if ( ( insertStart != -1 ) && ( insertNum != -1 ) )
        {
            beginInsertRows( QModelIndex(), insertStart, insertStart + insertNum - 1 );
            endInsertRows();
        }
        return retVal;
    }

    bool CBIFModel::canFetchMore( const QModelIndex &parent ) const
    {
        if ( parent.isValid() )
            return false; // its a list
        if ( !fBIFFile )
            return false;
        return fBIFFile->canLoadMoreImages();
    }

    void CBIFModel::fetchMore( const QModelIndex &parent )
    {
        if ( parent.isValid() )
            return; // its a list
        if ( !fBIFFile )
            return;

        beginInsertRows( QModelIndex(), fBIFFile->lastImageLoaded(), fBIFFile->lastImageLoaded() + fBIFFile->fetchSize() - 1 );
        fBIFFile->fetchMore();
        endInsertRows();
    }
}
