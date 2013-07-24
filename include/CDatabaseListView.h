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
#ifndef CDATABASELISTVIEW_H
#define CDATABASELISTVIEW_H

#include <stddef.h>
#include <q3listview.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include <QKeyEvent>

class CConsoleWindow;
class CMessagePanel;
class Q3WidgetStack;
class Q3PopupMenu;
class CMySQLServer;
class CDatabaseListViewItem;

class CDatabaseListView : public Q3ListView
{
  Q_OBJECT
    
public:
  CDatabaseListView(CConsoleWindow * c, QWidget * parent, const char * name = 0, Qt::WFlags f = 0);
  CConsoleWindow * consoleWindow() const { return consolewindow; }
  CMessagePanel * messagePanel() const;
  Q3WidgetStack * widgetStack() const;
  void updateListView();

  bool isBusy() const { return isbusy; }
  void setBusy(bool b);
  void setItemRoot(CDatabaseListViewItem *item);
  void getActionMenu(Q3PopupMenu *);
  void emitEnableQueryButton(CDatabaseListViewItem *);

signals:
  void itemSelected(CDatabaseListViewItem *);

public slots:
  void processCurrentItemMenu(int);
  void ContextMenuRequested(Q3ListViewItem *item, const QPoint & pos, int c);
  void CurrentChanged(Q3ListViewItem *item);
  void refreshServers();
  void openInNewWindow(CDatabaseListViewItem *item);

private slots:
  void DoubleClicked( Q3ListViewItem *item);
  void Expanded(Q3ListViewItem *item);
  void Collapsed(Q3ListViewItem *item);
  void ItemRenamed(Q3ListViewItem * item, int col, const QString & text);
  void ReturnPressed( Q3ListViewItem *item);
  void SpacePressed( Q3ListViewItem *item);
  
private:
  void init();
  CConsoleWindow * consolewindow;
  bool isbusy;
  void keyPressEvent(QKeyEvent * e);
};

#endif
