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
#include "CDatabaseItem.h"
#include "config.h"
#include "globals.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "CConsoleWindow.h"
#include "CApplication.h"
#include "CShowDatabaseGroupProperties.h"
#include "CTableGroupItem.h"
#include <qmessagebox.h>

CDatabaseItem::CDatabaseItem(CDatabaseListViewItem * parent, const QString &dbname, CMySQLServer *m, const QPixmap &p1, const QPixmap &p2)
: CDatabaseListViewItem(parent, m, DATABASE, "CDatabaseItem")
{
#ifdef DEBUG
  qDebug("CDatabaseItem::CDatabaseItem(CDatabaseListViewItem *, '%s', CMySQLServer *, const QPixmap &, const QPixmap &)", debug_string(dbname));
#endif

  databaseDisconnectedIcon = p1;
  databaseConnectedIcon = p2;
  is_connected = false;
  init(dbname);
  widget_id = parent->int_type();
}

CDatabaseItem::CDatabaseItem(CDatabaseListView * parent, const QString &dbname, bool isconnected, const QString &connection_name, CMySQLServer *m)
: CDatabaseListViewItem(parent, m, DATABASE, "CDatabaseItem")
{
#ifdef DEBUG
  qDebug("CDatabaseItem::CDatabaseItem(CDatabaseListView *, '%s', bool, const QString &, CMySQLServer *)", debug_string(dbname));
#endif

  if (m == 0)
  {
    m_mysql = new CMySQLServer(connection_name, messagePanel());
    mysql()->connect();
    deletemysql = true;
  }
  is_connected = isconnected;
  init(dbname);
  widget_id = int_type();
}

void CDatabaseItem::init(const QString &dbname)
{
#ifdef DEBUG
  qDebug("CDatabaseItem::init('%s')", debug_string(dbname));
#endif

  setText(0,dbname);
  setDatabaseName(dbname);
  widgetData.setAutoDelete(true);
  if (databaseDisconnectedIcon.isNull())
    databaseDisconnectedIcon = getPixmapIcon("databaseDisconnectedIcon");

  if (databaseConnectedIcon.isNull())
    databaseConnectedIcon = getPixmapIcon("databaseConnectedIcon");

  databaseConnectIcon = getPixmapIcon("databaseConnectIcon");
  databaseDisconnectIcon = getPixmapIcon("databaseDisconnectIcon");
  sqlIcon = getPixmapIcon("sqlIcon");
  deleteIcon = getPixmapIcon("deleteIcon");
  newWindowIcon = getPixmapIcon("newWindowIcon");
  if (is_connected)
    setConnected(is_connected);
  else
    setPixmap(0, databaseDisconnectedIcon);
}

void CDatabaseItem::refreshWidget(bool b)
{
#ifdef DEBUG
  qDebug("CDatabaseItem::refreshWidget(%s)", debug_string(booltostr(b)));
#endif

  if (isBlocked())
    return;
  setBlocked(true);

  if (widget_id == int_type())
  {
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
  }
  else
    ((CDatabaseListViewItem *) QListViewItem::parent())->refreshWidget(b);
  widgetStack()->raiseWidget(widget_id);
  setBlocked(false);
}

void CDatabaseItem::activated()
{
#ifdef DEBUG
  qDebug("CDatabaseItem::activated()");
#endif

  if (isBlocked())
    return;

  refreshWidget(false);
}

void CDatabaseItem::doubleClicked()
{
#ifdef DEBUG
  qDebug("CDatabaseItem::doubleClicked()");
#endif


  if (!is_connected && !isBlocked())
    setConnected(true);
}

void CDatabaseItem::setConnected(bool isconnected)
{
#ifdef DEBUG
  qDebug("CDatabaseItem::setConnected(%s)", debug_string(booltostr(isconnected)));
#endif

  if (isBlocked())
    return;

  setBlocked(true);  

  if (isconnected)
    is_connected = isconnected && mysql()->mysql()->mysqlSelectDb(text(0));
  else
    is_connected = false;

  setPixmap(0, is_connected ? databaseConnectedIcon : databaseDisconnectedIcon);
  if (is_connected)
  {
    new CTableGroupItem(this, text(0), mysql());
    setOpen(true);
  }
  else
    deleteChilds();

  createWindowMenu(consoleWindow()->itemToolBar());
  setBlocked(false);
}

void CDatabaseItem::processMenu(int res)
{
#ifdef DEBUG
  qDebug("CDatabaseItem::processMenu(%d)", res);
#endif

  if (isBlocked())
    return;

  switch (res)
  {
		case MENU_CONNECT: setConnected(true);
      break;

    case MENU_DISCONNECT: setConnected(false);
      break;

    case MENU_DELETE:
      {
        if (myApp()->confirmCritical())
          if ((QMessageBox::information(0, tr("Confirm Delete Database"), 
            tr("Are you sure you want to Delete Database") + ":\'" + text(0) + "\' ?",
            tr("&Yes"), tr("&No")) != 0))
            return;

        setBlocked(true);
        CMySQLQuery qry(mysql()->mysql());
        qry.setEmitMessages(false);
        if (qry.execStaticQuery("DROP DATABASE IF EXISTS " + mysql()->mysql()->quote(text(0))))
        {
          setVisible(false);
          setBlocked(false);
          delete this;
        }
        else
          setBlocked(false);
      }
      break;

    case MENU_NEW_WINDOW_FROM_HERE: databaseListView()->openInNewWindow(this);
      break;
  }
}

void CDatabaseItem::keyPressed(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CDatabaseItem::keyPressed()");
#endif

  if (isBlocked())
    return;

  if (e->key() == QListViewItem::Key_Delete)
    processMenu(MENU_DELETE);
  else
    CDatabaseListViewItem::keyPressed(e);
}

void CDatabaseItem::initMenuItems()
{
#ifdef DEBUG
  qDebug("CDatabaseItem::initMenuItems()");
#endif

  menu()->clear();
  if (!is_connected)
    menu()->insertItem(databaseConnectIcon, tr("&Connect"), MENU_CONNECT);
  else
    menu()->insertItem(databaseDisconnectIcon, tr("&Disconnect"), MENU_DISCONNECT);

  menu()->insertSeparator();
  menu()->insertItem(deleteIcon, tr("D&rop Database"), MENU_DELETE);
  menu()->insertSeparator();
  menu()->insertItem(newWindowIcon, tr("New &Window from Here"), MENU_NEW_WINDOW_FROM_HERE);
  menu()->setItemEnabled(MENU_NEW_WINDOW_FROM_HERE, !mysql()->oneConnection());
}
