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

#include "SetMKVTags.h"
#include "ui_SetMKVTags.h"

#include "MediaInfo.h"
#include "MKVUtils.h"
#include "AutoWaitCursor.h"
#include "UtilityViews.h"
#include "QtUtils.h"

#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>

namespace NSABUtils
{
    CSetMKVTags::CSetMKVTags( const QString & fileName, const QString & mkvPropEdit, QWidget * parent )
        : QDialog( parent ),
        fImpl( new Ui::CSetMKVTags ),
        fMKVPropEditExe( mkvPropEdit )
    {
        auto fi = QFileInfo( mkvPropEdit );
        Q_ASSERT( !mkvPropEdit.isEmpty() && fi.isExecutable() && fi.isFile() );

        fImpl->setupUi( this );
        fImpl->fileName->setText( fileName );
        fImpl->fileName->setReadOnly( true );
        fImpl->tags->setItemDelegateForColumn( 0, new CNoEditDelegate( this ) );
        QTimer::singleShot( 0, this, &CSetMKVTags::slotLoadTags );
    }

    CSetMKVTags::~CSetMKVTags()
    {
    }

    void CSetMKVTags::slotLoadTags()
    {
        NSABUtils::CAutoWaitCursor awc;
        
        auto mediaInfo = CMediaInfo( fImpl->fileName->text() );
        auto tags = mediaInfo.getMediaTags();
        
        fImpl->tags->setHeaderLabels( QStringList() << tr( "Tag Name" ) << tr( "Value" ) );
        for ( auto && ii : tags )
        {
            bool isEditable = isSettableTag( ii.first );
            auto item = new QTreeWidgetItem( QStringList() << displayName( ii.first ) + ":" << NSABUtils::getFirstString( ii.second ), static_cast<int>( ii.first ) );
            fImpl->tags->addTopLevelItem( item );
            if ( isEditable )
            {
                item->setFlags( item->flags() | Qt::ItemIsEditable );
            }
        }
    }

    void CSetMKVTags::accept()
    {
        TMediaTagMap tags;
        int rowCount = fImpl->tags->topLevelItemCount();
        for ( auto ii = 0; ii < rowCount; ++ii )
        {
            auto item = fImpl->tags->topLevelItem( ii );
            if ( !item )
                continue;
            auto type = static_cast<EMediaTags>( item->type() );
            if ( !isSettableTag( type ) )
                continue;
            tags[ type ] = item->text( 1 );
        }

        QString msg;
        CAutoWaitCursor awc;
        if ( !NSABUtils::setMediaTags( fImpl->fileName->text(), tags, fMKVPropEditExe, &msg ) )
        {
            QMessageBox::critical( this, tr( "Problem setting tags" ), msg, QMessageBox::Ok );
            return;
        }
        QDialog::accept();
    }
}
