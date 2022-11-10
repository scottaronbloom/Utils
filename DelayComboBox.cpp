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

#include "DelayComboBox.h"
#include "DelayLineEdit.h"

namespace NSABUtils
{
    CDelayComboBox::CDelayComboBox( QWidget * parent /*= nullptr */ ) :
        QComboBox( parent )
    {
        setDelayLineEdit( new CDelayLineEdit );
    }

    CDelayLineEdit * CDelayComboBox::lineEdit() const
    {
        return dynamic_cast<CDelayLineEdit *>( QComboBox::lineEdit() );
    }

    void CDelayComboBox::setDelay( int delayMS )
    {
        lineEdit()->setDelay( delayMS );
    }

    void CDelayComboBox::setIsOKFunction( std::function< bool( const QString & text ) > func, const QString & errorMsg /*= {} */ )
    {
        lineEdit()->setIsOKFunction( func, errorMsg );
    }

    bool CDelayComboBox::isOK() const
    {
        return lineEdit()->isOK();
    }

    void CDelayComboBox::setDelayLineEdit( CDelayLineEdit * le )
    {
        if ( lineEdit() )
            disconnect( lineEdit(), &CDelayLineEdit::sigTextChangedAfterDelay, this, &CDelayComboBox::sigEditTextChangedAfterDelay );
        setLineEdit( le );
        if ( le )
            connect( le, &CDelayLineEdit::sigTextChangedAfterDelay, this, &CDelayComboBox::sigEditTextChangedAfterDelay );
    }

    QStringList CDelayComboBox::getAllText() const
    {
        auto retVal = QStringList() << currentText();
        for ( int ii = 0; ii < this->count(); ++ii )
        {
            retVal << itemText( ii );
        }
        return retVal;
    }

    void CDelayComboBox::addCurrentItem()
    {
        auto currText = currentText();
        if ( currText.isEmpty() )
            return;

        disconnect( lineEdit(), &CDelayLineEdit::sigTextChangedAfterDelay, this, &CDelayComboBox::sigEditTextChangedAfterDelay );

        int index = -1;
        switch ( insertPolicy() )
        {
            case QComboBox::InsertAtTop:
                index = 0;
                break;
            case QComboBox::InsertAtBottom:
                index = count();
                break;
            case QComboBox::InsertAtCurrent:
            case QComboBox::InsertAfterCurrent:
            case QComboBox::InsertBeforeCurrent:
                if ( !count() || ( currentIndex() != -1 ) )
                    index = 0;
                else if ( insertPolicy() == QComboBox::InsertAtCurrent )
                    setItemText( currentIndex(), currText );
                else if ( insertPolicy() == QComboBox::InsertAfterCurrent )
                    index = currentIndex() + 1;
                else if ( insertPolicy() == QComboBox::InsertBeforeCurrent )
                    index = currentIndex();
                break;
            case QComboBox::InsertAlphabetically:
                index = 0;
                for ( int i = 0; i < count(); i++, index++ )
                {
                    if ( currText.toLower() < itemText( i ).toLower() )
                        break;
                }
                break;
            default:
                break;
        }
        if ( index >= 0 )
        {
            bool add = true;
            if ( !duplicatesEnabled() )
            {
                for ( int ii = 0; ii < count(); ++ii )
                {
                    if ( itemText( ii ) == currText )
                    {
                        if ( ii == index )
                        {
                            add = false;
                            break;
                        }
                        removeItem( ii );
                        ii--;
                    }
                }
            }

            if ( add )
            {
                insertItem( index, currText );
                setCurrentIndex( index );
            }
        }

        connect( lineEdit(), &CDelayLineEdit::sigTextChangedAfterDelay, this, &CDelayComboBox::sigEditTextChangedAfterDelay );
    }

}

