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
#ifndef CTABLEITEM_H
#define CTABLEITEM_H

#include "CDatabaseListViewItem.h"
//Added by qt3to4:
#include <QKeyEvent>
#include <QPixmap>

class CTableGroupItem;

class CTableItem : public CDatabaseListViewItem
{
  Q_OBJECT

public:
  CTableItem (CDatabaseListViewItem * parent, const QString &table_name, CMySQLServer *m);
  ~CTableItem();
  void activated();
  void renamed(const QString &);
  void doubleClicked();
  void keyPressed(QKeyEvent * e);
  
public slots:
  void processMenu(int res);

private:

  class header
  {
  public:
    header(const QString &c, int s)
    {
      m_caption = c;
      m_size = s;
    }
    int size() const { return m_size; }
    QString caption() const { return m_caption; }

  private:
    int m_size;
    QString m_caption;
  };

  Q3IntDict<header> headers;
  Q3IntDict<QPixmap> pix;
  CTableGroupItem *my_parent;
  Q3IntDict<QString> widgetData;
  void refreshWidget(bool b);
  void initMenuItems();
  uint real_numRows;
  uint real_numCols;
};

#endif
