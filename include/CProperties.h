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

#include <qvariant.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qaction.h>
#include <qdict.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QListView;
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
  void saveData(QDict<QString> *data);
  void loadData(const QDict<QString> &data);
  void setTitle(const QString &s);
  CMySQLServer *mysql() const { return p_mysql; }

public slots:
  void RightButtonClicked( QListViewItem *, const QPoint & pos, int);
  void refresh();
  void save();
  
protected slots:
  virtual void insertData(){};

protected:  
  void insertItem(const QString & property, const QString & value);  
  QListView * details;  

private:
  QLabel* title;
  QVBoxLayout* CPropertiesLayout;
  QAction* refreshAction;
  QString tmpFileName;
  bool hasSaveTitle;
  QString saveTitle;
  CMySQLServer *p_mysql;
};

#endif
