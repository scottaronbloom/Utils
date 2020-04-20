/*
 * (c) Copyright 2004 - 2019 Blue Pearl Software Inc.
 * All rights reserved.
 *
 * This source code belongs to Blue Pearl Software Inc.
 * It is considered trade secret and confidential, and is not to be used
 * by parties who have not received written authorization
 * from Blue Pearl Software Inc.
 *
 * Only authorized users are allowed to use, copy and modify
 * this software provided that the above copyright notice
 * remains in all copies of this software.
 *
 *
 * $Author: scott $ - $Revision: 55915 $ - $Date: 2019-11-05 18:22:16 -0800 (Tue, 05 Nov 2019) $
 * $HeadURL: http://bpsvn/svn/trunk/UI/Utils/ScrollMessageBox.cpp $
 *
 *
*/
#include "ScrollMessageBox.h"
#include "ui_ScrollMessageBox.h"

#include <QAbstractButton>
#include <QMessageBox>

CScrollMessageBox::CScrollMessageBox( QWidget *parent ) :
    QDialog( parent ),
    fImpl( new Ui::CScrollMessageBox )
{
    fImpl->setupUi( this );

    setWindowFlags( this->windowFlags() & ~Qt::WindowContextHelpButtonHint );

    QSizePolicy sizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    sizePolicy.setHorizontalStretch( 0 );
    sizePolicy.setVerticalStretch( 0 );
    sizePolicy.setHeightForWidth( fImpl->iconLabel->sizePolicy().hasHeightForWidth() );

    fImpl->iconLabel->setSizePolicy( sizePolicy );
    fImpl->iconLabel->setVisible( false );

    setIconLabel( QMessageBox::Information );

    connect( fImpl->buttonBox, &QDialogButtonBox::clicked, this, &CScrollMessageBox::slotButtonClicked );

}

CScrollMessageBox::CScrollMessageBox( const QString & title, const QString & label, QWidget *parent )
    : CScrollMessageBox( parent )
{
    setLabel( label );
    setTitle( title );
}

CScrollMessageBox::~CScrollMessageBox()
{
}

void CScrollMessageBox::setTitle( const QString & text )
{
    setWindowTitle( text );
}

void CScrollMessageBox::setLabel( const QString & text )
{
    fImpl->label->setText( text );
}

void CScrollMessageBox::setPlainText( const QString & text )
{
    fImpl->message->setPlainText( text );
}

void CScrollMessageBox::setHtmlText( const QString & text )
{
    fImpl->message->setHtml( text );
}

void CScrollMessageBox::setIconLabel( const QMessageBox::Icon & icon )
{
    fImpl->iconLabel->setVisible( true );
    auto pm = QMessageBox::standardIcon( icon );
    fImpl->iconLabel->setPixmap( pm );
    fImpl->iconLabel->setVisible( !pm.isNull() );
    if ( icon == QMessageBox::Icon::Critical )
        QApplication::beep();
}

void CScrollMessageBox::setButtons( QDialogButtonBox::StandardButtons buttons )
{
    fImpl->buttonBox->setStandardButtons( buttons );
}

void CScrollMessageBox::slotButtonClicked( QAbstractButton * btn )
{
    if ( ( fImpl->buttonBox->buttonRole( btn ) == QDialogButtonBox::ApplyRole ) || ( fImpl->buttonBox->buttonRole( btn ) == QDialogButtonBox::YesRole ))
    {
        accept();
    }
}

