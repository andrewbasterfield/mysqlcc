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
#include <qlistview.h>

class CConsoleWindow;
class CMessagePanel;
class QWidgetStack;
class QPopupMenu;
class CMySQLServer;
class CDatabaseListViewItem;

class CDatabaseListView : public QListView
{
  Q_OBJECT
    
public:
  CDatabaseListView(CConsoleWindow * c, QWidget * parent, const char * name = 0, WFlags f = 0);
  CConsoleWindow * consoleWindow() const { return consolewindow; }
  CMessagePanel * messagePanel() const;
  QWidgetStack * widgetStack() const;
  void updateListView();

  bool isBusy() const { return isbusy; }
  void setBusy(bool b);
  void setItemRoot(CDatabaseListViewItem *item);
  void getActionMenu(QPopupMenu *);
  void emitEnableQueryButton(CDatabaseListViewItem *);

signals:
  void itemSelected(CDatabaseListViewItem *);

public slots:
  void processCurrentItemMenu(int);
  void ContextMenuRequested(QListViewItem *item, const QPoint & pos, int c);
  void CurrentChanged(QListViewItem *item);
  void refreshServers();
  void openInNewWindow(CDatabaseListViewItem *item);

private slots:
  void DoubleClicked( QListViewItem *item);
  void Expanded(QListViewItem *item);
  void Collapsed(QListViewItem *item);
  void ItemRenamed(QListViewItem * item, int col, const QString & text);
  void ReturnPressed( QListViewItem *item);
  void SpacePressed( QListViewItem *item);
  
private:
  void init();
  CConsoleWindow * consolewindow;
  bool isbusy;
  void keyPressEvent(QKeyEvent * e);
};

#endif
