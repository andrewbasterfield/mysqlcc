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
#include "CApplication.h"
#include "CServerItem.h"
#include "CMySQLServer.h"
#include "CConfig.h"
#include "config.h"
#include "globals.h"
#include "CRegisterServerDialog.h"
#include "CShowServerProperties.h"
#include "CConsoleWindow.h"
#include "CDatabaseGroupItem.h"
#include "CServerAdministrationItem.h"
#include "CUserAdminItem.h"
//Added by qt3to4:
#include <QPixmap>
#include <QKeyEvent>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 1
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CServerItem::CServerItem(CDatabaseListView * parent, const QString &server_name, CMySQLServer *m, const QPixmap &p1, const QPixmap &p2)
: CDatabaseListViewItem(parent, m, SERVER, "CServerItem")
{
#ifdef DEBUG
  qDebug("CServerItem::CServerItem(CDatabaseListView *, '%s', CMySQLServer *, const QPixmap &, const QPixmap &)", debug_string(server_name));
#endif

  serverDisconnectedIcon = p1.isNull() ? getPixmapIcon("serverDisconnectedIcon") : p1;
  serverConnectedIcon = p2.isNull() ? getPixmapIcon("serverConnectedIcon") : p2;

  sqlIcon = getPixmapIcon("sqlIcon");
  editIcon = getPixmapIcon("editIcon");
  deleteIcon = getPixmapIcon("deleteIcon");
  newWindowIcon = getPixmapIcon("newWindowIcon");

  setText(0, server_name);
  widgetData.resize(25);
  widgetData.setAutoDelete(true);
  setRenameEnabled(0, true);
  if (m == 0)
  {
    setPixmap(0, serverDisconnectedIcon);
    m_mysql = new CMySQLServer(server_name, messagePanel());
    deletemysql = true;
  }
  else
  {
    if (m->isConnected())
    {
      setPixmap(0, serverConnectedIcon);
      setOpen(true);
    }
    else
    {
      setPixmap(0, serverDisconnectedIcon);
      setOpen(false);
    }
  }
}


void CServerItem::keyPressed(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CServerItem::keyPressed()");
#endif

  if (isBlocked())
    return;

  if (e->key() == Qt::Key_Delete && !isBlocked())
    processMenu(MENU_DELETE);
  else
    CDatabaseListViewItem::keyPressed(e);
}

void CServerItem::refreshWidget(bool b)
{
#ifdef DEBUG
  qDebug("CServerItem::refreshWidget(%s)", debug_string(booltostr(b)));
#endif

  if (isBlocked())
    return;
  setBlocked(true);
  CShowServerProperties * w = (CShowServerProperties *) widgetStack()->widget(int_type());
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

void CServerItem::activated()
{
#ifdef DEBUG
  qDebug("CServerItem::activated()");
#endif

  if (isBlocked())
    return;

  refreshWidget(false);
}

void CServerItem::doubleClicked()
{
#ifdef DEBUG
  qDebug("CServerItem::doubleClicked()");
#endif

  if (isBlocked())
    return;

  if (!mysql()->isConnected())
    setConnected(true);
}

void CServerItem::setConnected(bool b)
{
#ifdef DEBUG
  qDebug("CServerItem::setConnected(%s)", debug_string(booltostr(b)));
#endif

  if (isBlocked())
    return;
  setBlocked(true);

  bool c = false;
  if (b)
    c = mysql()->connect();
  else
    mysql()->disconnect();
  createWindowMenu(consoleWindow()->itemToolBar());
  if (c)
  {
    setPixmap(0, serverConnectedIcon);    
    new CDatabaseGroupItem(this, mysql());    
    new CServerAdministrationItem(this, mysql());
    new CUserAdminItem(this, mysql());
  }
  else
  {
    setPixmap(0, serverDisconnectedIcon);
    deleteChilds();
  }
  setOpen(c);  
  refreshWidget(true);
  databaseListView()->emitEnableQueryButton(this);
  setBlocked(false);
}

void CServerItem::connectionEdited(const QString &s)
{
#ifdef DEBUG
  qDebug("CServerItem::connectionEdited('%s')", debug_string(s));
#endif

  mysql()->setConnectionName(s);
  mysql()->refresh();
  setText(0, mysql()->connectionName());
  createWindowMenu(consoleWindow()->itemToolBar());
}


void CServerItem::renamed(const QString &newtext)
{
#ifdef DEBUG
  qDebug("CServerItem::renamed('%s')", debug_string(newtext));
#endif

  if (!mysql()->rename(newtext))
    setText(0, oldText());
  else
  {
    if (oldText() == myApp()->defaultConnectionName() && !myApp()->defaultConnectionName().isEmpty())
    {
      CConfig *c = new CConfig();
      c->writeEntry("Default Connection Name", newtext);
      c->save();
      delete c;
      myApp()->setDefaultConnectionName(newtext);
    }
  }
}


void CServerItem::processMenu(int res)
{
#ifdef DEBUG
  qDebug("CServerItem::processMenu(%d)", res);
#endif

  if (isBlocked())
    return;
  bool do_refresh = false;

  switch (res)
  {
		case MENU_CONNECT: setConnected(true);
      break;
      
    case MENU_DISCONNECT: setConnected(false);
      break;

    case MENU_RENAME: startRename(0);
      break;

    case MENU_EDIT:
      {
        setBlocked(true);
        CRegisterServerDialog * c = new CRegisterServerDialog(mysql()->connectionName(), messagePanel(), myApp()->workSpace());
        connect(c, SIGNAL(connectionEdited(const QString &)), this, SLOT(connectionEdited(const QString &)));
        myShowWindow(c);
        setBlocked(false);
      }
      break;

    case MENU_DELETE:
      {
        if (isBlocked())
          return;        
        if (myApp()->confirmCritical())
          if (QMessageBox::information(0, tr("Delete Server"), tr("Are you sure you want to Delete Server") + 
            ": '" + text(0) + "' ?", tr("&Yes"), tr("&No")) != 0)
            return;
        if (mysql()->isConnected())
          setConnected(false);
        if (text(0) == myApp()->defaultConnectionName() && !myApp()->defaultConnectionName().isEmpty())
        {
          CConfig *c = new CConfig();
          c->removeEntry("Default Connection Name");
          c->save();
          delete c;
          myApp()->setDefaultConnectionName(QString::null);
        }
        setBlocked(true);
        mysql()->remove();
        setVisible(false);
        do_refresh = true;
        setBlocked(false);
      }
      break;

    case MENU_NEW_WINDOW_FROM_HERE: databaseListView()->openInNewWindow(this);
      break;
  }

  if (do_refresh && !isBlocked())
    databaseListView()->refreshServers();
}

void CServerItem::initMenuItems()
{
#ifdef DEBUG
  qDebug("CServerItem::initMenuItems()");
#endif

  menu()->clear();
  if (!mysql()->isConnected())
    menu()->insertItem(serverConnectedIcon, tr("&Connect"), MENU_CONNECT);
  else    
    menu()->insertItem(serverDisconnectedIcon, tr("&Disconnect"), MENU_DISCONNECT);

  menu()->insertSeparator();
  menu()->insertItem(editIcon, tr("&Edit"), MENU_EDIT);  
  menu()->insertItem(deleteIcon, tr("De&lete"), MENU_DELETE);
  menu()->insertItem(tr("&Rename"), MENU_RENAME);
  menu()->insertSeparator();
  menu()->insertItem(newWindowIcon, tr("New &Window from Here"), MENU_NEW_WINDOW_FROM_HERE);

  menu()->setItemEnabled(MENU_NEW_WINDOW_FROM_HERE, !mysql()->oneConnection());
}
