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
#ifndef CINFODIALOG_H
#define CINFODIALOG_H

#include <stddef.h>
#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class QLabel;
class QPushButton;

class CInfoDialog : public QDialog
{ 
  Q_OBJECT
    
public:
  CInfoDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~CInfoDialog();
  virtual void setCaption(const QString &);
  
protected:
  void addMainWidget(QWidget *widget);
  QLabel* Copyright;
  QLabel* Title;
  QLabel* PixmapLabel1;  
  QPushButton* m_pClosePushButton;
  QLabel* TextLabel4;
  Q3GridLayout* CInfoDialogLayout;
};

#endif // CINFODIALOG_H
