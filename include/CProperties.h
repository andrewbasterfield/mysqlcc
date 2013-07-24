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
#ifndef CPROPERTIES_H
#define CPROPERTIES_H

#include <stddef.h>
#include <qvariant.h>
#include <qwidget.h>
#include <qlabel.h>
#include <q3listview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <q3header.h>
#include <q3popupmenu.h>
#include <qpixmap.h>
#include <qaction.h>
#include <q3dict.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>

class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class QLabel;
class Q3ListView;
class CMySQLServer;

class CProperties : public QWidget
{ 
  Q_OBJECT
    
public:
  CProperties(QWidget* parent,  CMySQLServer *m=0, const char *name=0);
  QString getSaveContents();
  virtual QString getSaveTitle();
  void setSaveTitle(const QString &title);
  void setMySQL(CMySQLServer *m) { p_mysql = m; }
  void saveData(Q3Dict<QString> *data);
  void loadData(const Q3Dict<QString> &data);
  void setTitle(const QString &s);
  CMySQLServer *mysql() const { return p_mysql; }

public slots:
  void RightButtonClicked( Q3ListViewItem *, const QPoint & pos, int);
  void refresh();
  void save();
  
protected slots:
  virtual void insertData(){};

protected:  
  void insertItem(const QString & property, const QString & value);  
  Q3ListView * details;  

private:
  QLabel* title;
  Q3VBoxLayout* CPropertiesLayout;
  QAction* refreshAction;
  QString tmpFileName;
  bool hasSaveTitle;
  QString saveTitle;
  CMySQLServer *p_mysql;
};

#endif
