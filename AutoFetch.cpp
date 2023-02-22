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

#include "AutoFetch.h"

#include <QTreeView>
#include <QScrollBar>
#include <tuple>

namespace NSABUtils
{
    // 0 = OK view && idx.isValid
    // second return value is visible (idx intersetcs or is contained by the view)
    //
    // third is a tuple of ( isAbove, isLeft, isRight, isBelow ), invalid IF its visible
    // isAbove = if idx.rect.top < view.rect.top,
    // isLeft = if idxRect.right < view.rect.left
    // isRight = if idxRect.left > view.rect.right
    // isBelow = if idxRect.top > view.rect.bottom
    std::tuple< bool, bool, std::tuple< bool, bool, bool, bool > > isVisible( QAbstractItemView *view, const QModelIndex &idx )
    {
        auto retVal = std::make_tuple( false, false, std::make_tuple( false, false, false, false ) );
        bool aOK = std::get< 0 >( retVal ) = view && idx.isValid();
        if ( !aOK )
            return retVal;

        auto vpRegion = view->viewport()->visibleRegion();
        auto vpRect = view->viewport()->rect();
        auto idxRect = view->visualRect( idx );
        // qDebug() << "vpRegion:" << vpRegion << "vpRect:" << vpRect << " IDX Rect:" << idxRect;
        bool isVisible = std::get< 1 >( retVal ) = vpRegion.intersects( idxRect ) || vpRegion.contains( idxRect );
        if ( !isVisible )
        {
            std::get< 0 >( std::get< 2 >( retVal ) ) = idxRect.top() < 0;   // above
            std::get< 1 >( std::get< 2 >( retVal ) ) = idxRect.right() < vpRect.left();   // left
            std::get< 2 >( std::get< 2 >( retVal ) ) = idxRect.left() > vpRect.right();   // right
            std::get< 3 >( std::get< 2 >( retVal ) ) = idxRect.top() > vpRect.bottom();   // below
        }
        return retVal;
    }

    QModelIndex findFirstVisible( QTreeView *view, QModelIndex idx )
    {
        // qDebug() << "findFirstVisible:" << idx << idx.data();

        bool aOK = false;
        bool isVisible = false;
        std::tuple< bool, bool, bool, bool > whereIsIt;
        std::tie( aOK, isVisible, whereIsIt ) = NSABUtils::isVisible( view, idx );
        if ( !aOK || isVisible )
            return idx;

        // dont bother if its not expanded..
        if ( !aOK || !view->isExpanded( idx ) )
        {
            return idx;
        }

        // dont bother if its not visible and below the screen
        if ( !isVisible && std::get< 3 >( whereIsIt ) )
        {
            return idx;
        }

        while ( aOK && !isVisible && !std::get< 3 >( whereIsIt ) )   // must be above or off to the left
        {
            idx = view->indexBelow( idx );
            // qDebug() << "findFirstVisible: indexBelow" << idx << idx.data();
            std::tie( aOK, isVisible, whereIsIt ) = NSABUtils::isVisible( view, idx );
        }
        return idx;
    }

    // initIndex is the item being expanded (or is expanded)
    bool autoFetchMore( QTreeView *view, const QModelIndex &initIndex )
    {
        if ( !view || !view->model() )
            return false;

        auto model = view->model();

        if ( !initIndex.isValid() )
        {
            // top level only
            bool retVal = false;
            for ( int ii = 0; ii < view->model()->rowCount( QModelIndex() ); ++ii )
            {
                auto idx = model->index( ii, 0, QModelIndex() );
                retVal = autoFetchMore( view, idx ) || retVal;
            }
            return retVal;
        }

        bool aOK = false;
        bool isVisible = false;
        std::tuple< bool, bool, bool, bool > whereIsIt;
        std::tie( aOK, isVisible, whereIsIt ) = NSABUtils::isVisible( view, initIndex );
        // qDebug() << "initIndex:" << initIndex << initIndex.data() << "isExpanded:" << view->isExpanded( initIndex ) << "Num Children:" << model->rowCount( initIndex ) << "Can FetchMore:" << model->canFetchMore( initIndex )
        //     << " aOK:" << aOK
        //     << " isVisible:" << isVisible
        //     << " whereIsIt:" << whereIsIt
        //     ;

        // dont bother if its not expanded..
        if ( !aOK || !view->isExpanded( initIndex ) )
        {
            return false;
        }

        // dont bother if its not visible and below the screen
        if ( !isVisible && std::get< 3 >( whereIsIt ) )
        {
            return false;
        }

        Q_ASSERT( isVisible || ( std::get< 0 >( whereIsIt ) || std::get< 1 >( whereIsIt ) || std::get< 2 >( whereIsIt ) ) );

        QModelIndex expandingIndex = initIndex;
        // qDebug() << "expandingIndex:" << expandingIndex << expandingIndex.data();

        QModelIndex firstVisible = findFirstVisible( view, expandingIndex );
        auto idxRect = view->visualRect( expandingIndex );
        auto itemHeight = idxRect.height();

        auto viewHeight = view->viewport()->height();
        auto remainingHeight = viewHeight - idxRect.bottom();
        auto viewCount = 1 + ( remainingHeight / itemHeight );

        bool retVal = false;
        while ( ( model->rowCount( expandingIndex ) < viewCount ) && model->canFetchMore( expandingIndex ) )
        {
            model->fetchMore( expandingIndex );
            retVal = true;
        }

        if ( ( model->rowCount( expandingIndex ) < viewCount ) && !model->canFetchMore( expandingIndex ) )
        {
            for ( int ii = 0; ii < view->model()->rowCount( expandingIndex ); ++ii )
            {
                auto idx = model->index( ii, 0, expandingIndex );
                retVal = autoFetchMore( view, idx ) || retVal;
            }
        }
        return retVal;
    }

    CAutoFetchMore::CAutoFetchMore( QTreeView *view ) :
        QObject( view )
    {
        if ( !view )
            return;
        connect( view->verticalScrollBar(), &QScrollBar::valueChanged, [ view ]( int /*value*/ ) { autoFetchMore( view, QModelIndex() ); } );
        connect( view->verticalScrollBar(), &QScrollBar::rangeChanged, [ view ]( int /*min*/, int /*max*/ ) { autoFetchMore( view, QModelIndex() ); } );
        connect( view, &QTreeView::expanded, [ view ]( const QModelIndex &idx ) { autoFetchMore( view, idx ); } );
        connect( view, &QTreeView::collapsed, [ view ]( const QModelIndex & /*idx*/ ) { autoFetchMore( view, QModelIndex() ); } );
    }
}
