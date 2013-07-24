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
#ifndef CCONSOLEWINDOW_H
#define CCONSOLEWINDOW_H

#include "CMyWindow.h"
#include "CToolBar.h"
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3PopupMenu>
#include <QCloseEvent>

class CDockWindow;
class CConfig;
class CDatabaseListView;
class CDatabaseListViewItem;
class CSqlDebugPanel;
class CAction;
class Q3GridLayout;
class Q3WidgetStack;
class Q3ToolBar;
class CMySQLServer;

class CConsoleWindow : public CMyWindow
{
  Q_OBJECT
public:
  CConsoleWindow (QWidget * parent, bool appwindow = false, Qt::WFlags f = Qt::WDestructiveClose);

  void saveWindowSettings();
  bool loadWindowSettings();

  CDatabaseListView * databaseListView() const { return dbListView; }

  CDockWindow * treePanelWindow() const { return treepanelwindow; }

  CSqlDebugPanel *sqlDebugPanel() const { return sqldebugpanel; }
  Q3WidgetStack *widgetStack() const { return widgetstack; }
  QWidget * defaultWidget() const { return defaultwidget; }
  void removeFromWidgetStack(int);
  CToolBar * itemToolBar() const { return itemtoolbar; }
  Q3PopupMenu * actionMenu() { return actionmenu; }
  void setCursor ( const QCursor & );

public slots:
  void actionMenuAboutToShow();
  void enableActionMenu(bool);
  void registerServer();
  void showTree(bool);
  void showMessages(bool);
  void prepareQueryWindow(CDatabaseListViewItem *);
  void openQueryWindow();


private slots:
  void viewMenuAboutToShow();

protected:
  void saveSettings(CConfig *cfg);

private:
  CToolBar * itemtoolbar;
  Q3ToolBar * consoletoolbar;
  Q3GridLayout* CConsoleWindowLayout;
  Q3WidgetStack * widgetstack;
  QWidget * defaultwidget;
  CDockWindow *treepanelwindow;
  CDatabaseListView *dbListView;
  int action_menu_id;
  Q3PopupMenu * actionmenu;
  CDatabaseListViewItem *current_query_item;
  CSqlDebugPanel *sqldebugpanel;
  CAction *viewShowTreeAction;
  CAction *viewShowMessagesAction;
  CAction * fileQueryAction;
  void beforeClose();
  void closeEvent(QCloseEvent * e);
  void saveDockWindowSettings(Q3DockWindow *dw, const QString &c, CConfig *cfg);
  bool loadDockWindowSettings(bool r, Q3DockWindow *dw, const QString &c, CConfig *cfg);  
};

#endif
