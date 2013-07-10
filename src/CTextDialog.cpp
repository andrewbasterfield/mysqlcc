/* Copyright (C) 2003 MySQL AB & Jorge del Conde

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
    
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA 02111-1307, USA 
*/
#include "CTextDialog.h"
#include "globals.h"

/*
This class is needed because QInputDialog::getText() forces the user to write
text in the QLineEdit.  In other words, it doesn's support empty strings.
*/

CTextDialog::CTextDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
: QDialog( parent, name, modal, fl )
{  
#ifdef DEBUG
  qDebug("CTextDialog::CTextDialog()");
#endif
  
  setIcon(getPixmapIcon("applicationIcon"));
  if ( !name )
    setName( "CTextDialog" );
  resize( 401, 81 ); 
  setMinimumSize( QSize( 401, 81 ) );
  setMaximumSize( QSize( 401, 81 ) );
  setCaption(tr("Text", "" ) );
  
  textLabel = new QLabel( this, "textLabel" );
  textLabel->setGeometry( QRect( 5, 5, 390, 16 ) ); 
  textLabel->setText(tr("Enter Text", "" ) );
  
  Text = new QLineEdit( this, "Text" );
  Text->setGeometry( QRect( 5, 25, 390, 19 ) ); 
  
  CancelButton = new QPushButton( this, "CancelButton" );
  CancelButton->setGeometry( QRect( 315, 50, 80, 25 ) ); 
  CancelButton->setText(tr("&Cancel", "" ) );
  QWhatsThis::add( CancelButton,tr("Close this dialog", "" ) );
  
  OkButton = new QPushButton( this, "OkButton" );
  OkButton->setGeometry( QRect( 230, 50, 80, 25 ) ); 
  OkButton->setText(tr("&OK", "" ) );
  OkButton->setDefault( TRUE );
  QWhatsThis::add( OkButton,tr("Click to Accept changes", "" ) );
  
  connect(Text, SIGNAL( returnPressed() ), this, SLOT( accept() ) );
  connect(OkButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect(CancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

CTextDialog::~CTextDialog()
{
#ifdef DEBUG
  qDebug("CTextDialog::~CTextDialog()");
#endif

}

QString CTextDialog::getText(const QString &caption, const QString &label, QLineEdit::EchoMode mode,
                             const QString &text, bool *ok, QWidget *parent, const char *name )
{
#ifdef DEBUG
  qDebug("static CTextDialog::getText()");
#endif
  
  CTextDialog *dlg = new CTextDialog(parent, name, true);
  dlg->textLabel->setText(label);
  dlg->Text->setText(text);
  dlg->setCaption(caption);
  dlg->Text->setEchoMode(mode);
  bool ok_ = false;
  QString result;
  ok_ = dlg->exec() == QDialog::Accepted;
  if (ok)
    *ok = ok_;
  if ( ok_ )
    result = dlg->Text->text();
  delete dlg;
  return result; 
}
