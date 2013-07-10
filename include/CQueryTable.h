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
#ifndef CQUERYTABLE_H
#define CQUERYTABLE_H

#include "CTable.h"

class CMySQLQuery;

class CQueryTable : public CTable
{
  Q_OBJECT

public:
  CQueryTable(QWidget * parent, CMySQLQuery *q, CMySQLServer *m, const char * name = 0);
  CMySQLQuery *query() const { return qry; }
  void setQuery(CMySQLQuery *q) { qry = q; }
  virtual void sortColumn(int col, bool ascending, bool wholeRows);
  void setMySQL(CMySQLServer *m) { m_mysql = m; }
  CMySQLServer *mysql() const { return m_mysql; }
  virtual QString copy_data(int row, int col);

signals:
  void refreshed();  

public slots:  
  virtual void refresh();
  virtual void cancel();
  virtual void save();
  virtual void reset();

protected:
  bool m_cancel;
  QPixmap pkIcon;
  QPixmap mulIcon;
  QPixmap uniIcon;
  QPixmap nothingIcon;
  QString tmpFileName;
  virtual void copy_data_func(QString *cpy, CMySQLQuery *qry, QTableSelection *sel, QMap<uint, ulong> *max_length_map);  

private:
  CMySQLQuery *qry;
  CMySQLServer *m_mysql;  
};

#endif
