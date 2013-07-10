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
#include "CUserAdminItem.h"
#include "CShowDatabaseGroupProperties.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "globals.h"
#include "config.h"
#include "panels.h"
#include "CConsoleWindow.h"
#include "CUserItem.h"
#include "CUserAdminWindow.h"


#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 1
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CUserAdminItem::CUserAdminItem(CDatabaseListViewItem * parent, CMySQLServer *m)
: CDatabaseListViewItem(parent, m, USER_ADMINISTRATION, "CUserAdminItem")
{
#ifdef DEBUG
  qDebug("CUserAdminItem::CUserAdminItem(CDatabaseListViewItem *, CMySQLServer *)");  
#endif    

  is_connected = false;
  init(false, false);
}

CUserAdminItem::CUserAdminItem(CDatabaseListView * parent, CMySQLServer *m, const QString &connection_name, bool connected)
: CDatabaseListViewItem(parent, m, USER_ADMINISTRATION, "CUserAdminItem")
{
#ifdef DEBUG
  qDebug("CUserAdminItem::CUserAdminItem(CDatabaseListView *, CMySQLServer *, const QString &, bool)");  
#endif    

  is_connected = false;
  if (m == 0)
  {
    m_mysql = new CMySQLServer(connection_name, messagePanel());
    mysql()->connect();
    deletemysql = true;
  }
  init(connected, true);
}

void CUserAdminItem::init(bool c, bool refresh_toolbar)
{
#ifdef DEBUG
  qDebug("CUserAdminItem::init(%s)", debug_string(booltostr(c)));
#endif

  widgetData.setAutoDelete(true);
  userIcon = getPixmapIcon("userIcon");
  setText( 0, tr("User Administration"));
  setConnected(c, refresh_toolbar);
}

void CUserAdminItem::setConnected(bool connected, bool refresh_toolbar)
{
#ifdef DEBUG
  qDebug("CUserAdminItem::setConnected(%s)", debug_string(booltostr(connected)));
#endif

  if (isBlocked())
    return;
  
  setBlocked(true);  

  if (connected && !is_connected)
  {
    mysql()->mysql()->setEmitErrors(false);
    if (mysql()->mysql()->mysqlSelectDb("mysql"))
      is_connected = displayUsers();
    else
      printAccessError();
    mysql()->mysql()->setEmitErrors(true);
  }
  else
    is_connected = false;
  
  if (!is_connected && !connected)
  {
    while (this->childCount() > 0)
      deleteChilds();
  }
  else
    if (is_connected)
      setOpen(true);

  setPixmap(0, getPixmapIcon(!is_connected ? "databaseUsersDisconnectedIcon" : "databaseUsersIcon"));
  if (refresh_toolbar)
    createWindowMenu(consoleWindow()->itemToolBar());
  setBlocked(false);
}


void CUserAdminItem::doubleClicked()
{
#ifdef DEBUG
  qDebug("CUserAdminItem::doubleClicked()");
#endif

  if (isBlocked())
    return;

  if (!is_connected && !isBlocked())
    setConnected(true);
}

bool CUserAdminItem::displayUsers()
{
#ifdef DEBUG
  qDebug("CUserAdminItem::displayUsers()");
#endif

  //TODO:  This needs to be modified to NOT use mysql.user for the users list.  SHOW GRANTS FOR % is needed in the server.  
  while (this->childCount() > 0)
    deleteChilds();

  bool hasAccess = false;
  CMySQLQuery *q = new CMySQLQuery(mysql()->mysql());
  q->setEmitMessages(false);
  q->setEmitErrors(false);
  if (q->exec("SELECT User, Host FROM mysql.user ORDER BY User"))
  {
    hasAccess = true;
    while (q->next())
      new CUserItem(this, mysql(), q->row(0), q->row(1), userIcon);
  }
  delete q;
  return hasAccess;
}

void CUserAdminItem::printAccessError()
{
#ifdef DEBUG
  qDebug("CUserAdminItem::printAccessError()");
#endif

  messagePanel()->critical(tr("You don't have enough privileges to access this section"));  
}

void CUserAdminItem::doCreateUser()
{
#ifdef DEBUG
  qDebug("CUserAdminItem::doCreateUser()");
#endif

  CUserAdminWindow *p = new CUserAdminWindow(myApp()->workSpace(), mysql(), QString::null, QString::null, false);
  connect(p, SIGNAL(do_refresh()), this, SLOT(refresh()));
  myShowWindow(p);
}

void CUserAdminItem::refreshWidget(bool b)
{
#ifdef DEBUG
  qDebug("CUserAdminItem::refreshWidget(%s)", debug_string(booltostr(b)));
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

void CUserAdminItem::refresh()
{
#ifdef DEBUG
  qDebug("CUserAdminItem::refresh()");
#endif

  if (isBlocked() || !is_connected)
    return;

  setBlocked(true);
  if (!displayUsers())
    printAccessError();
  setBlocked(false);
  refreshWidget(true);
}

void CUserAdminItem::activated()
{
#ifdef DEBUG
  qDebug("CUserAdminItem::activated()");
#endif

  if (isBlocked())
    return;

  refreshWidget(false);
}

void CUserAdminItem::processMenu(int res)
{
#ifdef DEBUG
  qDebug("CUserAdminItem::processMenu(%d)", res);
#endif

  if (isBlocked())
    return;
  switch (res)
  {
  case MENU_NEW: doCreateUser();
    break;
  case MENU_REFRESH: refresh();
    break;
  case MENU_DISCONNECT: setConnected(false);
    break;
  case MENU_CONNECT: setConnected(true);
    break;
  case MENU_NEW_WINDOW_FROM_HERE: databaseListView()->openInNewWindow(this);
    break;
  }
}

void CUserAdminItem::initMenuItems()
{
#ifdef DEBUG
  qDebug("CUserAdminItem::initMenuItems()");
#endif

  menu()->clear();  
  if (is_connected)
    menu()->insertItem(getPixmapIcon("databaseUsersDisconnectedIcon"), tr("&Disconnect"), MENU_DISCONNECT);  
  else      
    menu()->insertItem(getPixmapIcon("databaseUsersIcon"), tr("&Connect"), MENU_CONNECT);
  menu()->insertSeparator();
  menu()->insertItem(getPixmapIcon("newUserIcon"), QObject::tr("&New User"), MENU_NEW);
  menu()->insertSeparator();
  menu()->insertItem(getPixmapIcon("refreshIcon"), QObject::tr("&Refresh"), MENU_REFRESH);
  menu()->setItemEnabled(MENU_REFRESH, is_connected); 
  menu()->insertSeparator();
  menu()->insertItem(getPixmapIcon("newWindowIcon"), tr("New &Window from Here"), MENU_NEW_WINDOW_FROM_HERE);
  menu()->setItemEnabled(MENU_NEW_WINDOW_FROM_HERE, !mysql()->oneConnection());
}
