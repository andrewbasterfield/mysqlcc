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
#ifndef CTEXTDIALOG_H
#define CTEXTDIALOG_H

#include <stddef.h>
#include <qvariant.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

class QLabel;
class QLineEdit;
class QPushButton;

class CTextDialog : public QDialog
{ 
  Q_OBJECT
    
public:
  CTextDialog( QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags fl = 0 );
  ~CTextDialog();
  QLabel* textLabel;
  QLineEdit* Text;
  QPushButton* CancelButton;
  QPushButton* OkButton;
  
  static QString getText (const QString &caption, const QString &label, QLineEdit::EchoMode echo = QLineEdit::Normal,
    const QString &text = QString::null, bool *ok = 0, QWidget *parent = 0, const char *name = 0 );
  
};

#endif // CTEXTDIALOG_H
