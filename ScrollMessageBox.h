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
 * $Author: scott $ - $Revision: 52243 $ - $Date: 2019-04-10 14:52:19 -0700 (Wed, 10 Apr 2019) $
 * $HeadURL: http://bpsvn/svn/trunk/UI/Utils/ScrollMessageBox.h $
 *
 *
*/

#ifndef _SCROLLMESSAGEBOX_H
#define _SCROLLMESSAGEBOX_H

#include <QDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <memory>
class QAbstractButton;

namespace Ui {class CScrollMessageBox;};

class CScrollMessageBox : public QDialog
{
    Q_OBJECT

public:
    CScrollMessageBox( QWidget *parent = 0 );
    CScrollMessageBox( const QString & title, const QString & label, QWidget *parent = 0 );
    ~CScrollMessageBox();

    void setTitle( const QString & title );
    void setLabel( const QString & label );

    void setPlainText( const QString & text );
    void setHtmlText( const QString & text );
    void setIconLabel( const QMessageBox::Icon & icon );
    void setButtons( QDialogButtonBox::StandardButtons buttons );
private slots:
    void slotButtonClicked( QAbstractButton * btn );

private:
    std::unique_ptr< Ui::CScrollMessageBox > fImpl;
};

#endif 
