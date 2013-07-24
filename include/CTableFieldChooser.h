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
#ifndef CTABLEFIELDCHOOSER_H
#define CTABLEFIELDCHOOSER_H

#include <stddef.h>
#include <q3dockwindow.h>
#include <q3listview.h>
//Added by qt3to4:
#include <QKeyEvent>

class Q3Table;
class CTableFieldChooser;

class CTableFieldChooserListItem : public Q3CheckListItem
{
public:
  CTableFieldChooserListItem(Q3ListView * parent, const QString & text, int columnid);
  int col() { return c; }
  void keyPressed(QKeyEvent *e);

private:
  int c;
  void stateChange (bool s);  
};

class CTableFieldChooserListView : public Q3ListView
{
  Q_OBJECT

public:
  CTableFieldChooserListView(CTableFieldChooser * parent, const char * name = 0, Qt::WFlags f = 0 );
  void checkBoxClicked(int c, bool s);
  Q3Table * table() const;
  void refresh();

public slots:
  void displayMenu(Q3ListViewItem *, const QPoint &, int);

signals:
  void showColumn(int c, bool s);

private:
  void keyPressEvent(QKeyEvent * e);
};

class CTableFieldChooser : public Q3DockWindow
{
  Q_OBJECT

public:
  CTableFieldChooser(QWidget *parent, Q3Table * t, const char * name = 0, Place p = OutsideDock, Qt::WFlags f = Qt::WStyle_StaysOnTop);
  CTableFieldChooser(QWidget *parent, const char * name = 0, Place p = OutsideDock, Qt::WFlags f = Qt::WStyle_StaysOnTop);
  Q3Table * table() { return tbl; }
  virtual void setCaption(const QString &s);

public slots:
  void refresh();
  void show();
  void setTable(Q3Table *t)
  {
    tbl = t;
    refresh();
  }

private:
  void init();
  Q3Table *tbl;
  bool is_first;
  CTableFieldChooserListView * columnsListView;

  QSize minimumSizeHint() const
  {
    return QSize(1, 1);
  }

  QSize sizeHint() const
  {  
    return QSize(1, 1);
  }  
};

#endif
