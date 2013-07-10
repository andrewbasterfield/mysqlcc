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

#include <qdockwindow.h>
#include <qlistview.h>

class QTable;
class CTableFieldChooser;

class CTableFieldChooserListItem : public QCheckListItem
{
public:
  CTableFieldChooserListItem(QListView * parent, const QString & text, int columnid);
  int col() { return c; }
  void keyPressed(QKeyEvent *e);

private:
  int c;
  void stateChange (bool s);  
};

class CTableFieldChooserListView : public QListView
{
  Q_OBJECT

public:
  CTableFieldChooserListView(CTableFieldChooser * parent, const char * name = 0, WFlags f = 0 );
  void checkBoxClicked(int c, bool s);
  QTable * table() const;
  void refresh();

public slots:
  void displayMenu(QListViewItem *, const QPoint &, int);

signals:
  void showColumn(int c, bool s);

private:
  void keyPressEvent(QKeyEvent * e);
};

class CTableFieldChooser : public QDockWindow
{
  Q_OBJECT

public:
  CTableFieldChooser(QWidget *parent, QTable * t, const char * name = 0, Place p = OutsideDock, WFlags f = WStyle_StaysOnTop);
  CTableFieldChooser(QWidget *parent, const char * name = 0, Place p = OutsideDock, WFlags f = WStyle_StaysOnTop);
  QTable * table() { return tbl; }
  virtual void setCaption(const QString &s);

public slots:
  void refresh();
  void show();
  void setTable(QTable *t)
  {
    tbl = t;
    refresh();
  }

private:
  void init();
  QTable *tbl;
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
