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
#include "CServerAdministrationItem.h"
#include "CMySQLServer.h"
#include "CShowDatabaseGroupProperties.h"
#include "CAdministrationWindow.h"
#include "config.h"
#include "globals.h"

CServerAdministrationItem::CServerAdministrationItem(CDatabaseListViewItem * parent, CMySQLServer *m)
: CDatabaseListViewItem(parent, m, SERVER_ADMINISTRATION, "CServerAdministrationItem")
{
#ifdef DEBUG
  qDebug("CServerAdministrationItem::CServerAdministrationItem(CDatabaseListViewItem *, CMySQLServer *)");
#endif

  init();
}

CServerAdministrationItem::CServerAdministrationItem(CDatabaseListView * parent, const QString &connection_name, CMySQLServer *m)
: CDatabaseListViewItem(parent, m, SERVER_ADMINISTRATION, "CServerAdministrationItem")
{
#ifdef DEBUG
  qDebug("CServerAdministrationItem::CServerAdministrationItem(CDatabaseListView *, const QString &, CMySQLServer *)");
#endif

  if (m == 0)
  {
    m_mysql = new CMySQLServer(connection_name, messagePanel());
    mysql()->connect();
    deletemysql = true;
  }
  init();
}

CServerAdministrationItem::~CServerAdministrationItem()
{
  delete flush_menu;
}

void CServerAdministrationItem::refreshWidget(bool b)
{
#ifdef DEBUG
  qDebug("CServerAdministrationItem::refreshWidget(%s)", debug_string(booltostr(b)));
#endif

  if (isBlocked())
    return;
  setBlocked(true);

  CShowDatabaseGroupProperties * w = (CShowDatabaseGroupProperties *) widgetStack()->widget(int_type());
  w->setMySQL(mysql());
  w->setTitle(mysql()->connectionName());
  if (b || widgetData.isEmpty())
  {
    w->refresh();
    w->saveData(&widgetData);
  }
  else
    w->loadData(widgetData);
  widgetStack()->raiseWidget(int_type());

  setBlocked(false);
}

void CServerAdministrationItem::activated()
{
#ifdef DEBUG
  qDebug("CServerAdministrationItem::activated()");
#endif

  if (isBlocked())
    return;

  refreshWidget(false);
}

void CServerAdministrationItem::init()
{
  widgetData.resize(25);
  widgetData.setAutoDelete(true);
  setText( 0, tr("Server Administration"));  
  setPixmap(0, getPixmapIcon("serverAdministrationIcon"));
  flush_menu = CAdministrationWindow::flushMenu(mysql()->mysql());
  connect(flush_menu, SIGNAL(activated(int)), this, SLOT(processMenu(int)));
}

void CServerAdministrationItem::doubleClicked()
{
  if (isBlocked())
    return;

  setBlocked(true);
  myShowWindow(new CAdministrationWindow(myApp()->workSpace(), mysql()));
  setBlocked(false);
}

void CServerAdministrationItem::processMenu(int res)
{
  if (isBlocked())
    return;
  
  int t = -1;
  if (res >= FLUSH_HOSTS && res <= FLUSH_USER_RESOURCES)
  {
    CAdministrationWindow::flush(mysql(), res);
    return;
  }
  else
    switch (res)
    {
      case MENU_PING: CAdministrationWindow::ping(mysql());
        break;

      case MENU_SHUTDOWN: CAdministrationWindow::shutdown(mysql());
        break;

      case MENU_SHOW_PROCESSLIST: t = CAdministrationWindow::SHOW_PROCESSLIST;
        break;

      case MENU_SHOW_STATUS: t = CAdministrationWindow::SHOW_STATUS;
        break;

      case MENU_SHOW_VARIABLES: t = CAdministrationWindow::SHOW_VARIABLES;
        break;

      case MENU_NEW_WINDOW_FROM_HERE: databaseListView()->openInNewWindow(this);
        break;
    }

  if (t > -1)
  {
    CAdministrationWindow *w = new CAdministrationWindow(myApp()->workSpace(), mysql());
    w->setCurrentTabPage(t);      
    myShowWindow(w);
  }
}

void CServerAdministrationItem::initMenuItems()
{
#ifdef DEBUG
  qDebug("CServerAdministrationItem::initMenuItems()");
#endif

  menu()->clear();

  menu()->insertItem(getPixmapIcon("showProcessListIcon"), tr("Show Process List"), MENU_SHOW_PROCESSLIST);
  menu()->insertItem(getPixmapIcon("showStatusIcon"), tr("Show Status"), MENU_SHOW_STATUS);
  menu()->insertItem(getPixmapIcon("showVariablesIcon"), tr("Show Variables"), MENU_SHOW_VARIABLES);

  menu()->insertSeparator();

  menu()->insertItem(getPixmapIcon("flushIcon"), tr("Flush"), flush_menu, 0);

  menu()->insertItem(getPixmapIcon("pingIcon"), tr("Ping"), MENU_PING);
  menu()->insertItem(getPixmapIcon("serverShutdownIcon"), tr("Shutdown"), MENU_SHUTDOWN);
  menu()->insertSeparator();
  menu()->insertItem(getPixmapIcon("newWindowIcon"), tr("New &Window from Here"), MENU_NEW_WINDOW_FROM_HERE);
  menu()->setItemEnabled(MENU_NEW_WINDOW_FROM_HERE, !mysql()->oneConnection());
}
