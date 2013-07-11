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
#include "CDatabaseGroupItem.h"
#include "globals.h"
#include "config.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "CShowDatabaseGroupProperties.h"
#include "CDatabaseItem.h"
#include "panels.h"
#include <stddef.h>  
#include <qinputdialog.h>

CDatabaseGroupItem::CDatabaseGroupItem(CDatabaseListViewItem * parent, CMySQLServer *m)
: CDatabaseListViewItem(parent, m, DATABASE_GROUP, "CDatabaseGroupItem")
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::CDatabaseGroupItem(CDatabaseListViewItem *, CMySQLServer *)");
#endif

  init();
  setOpen(false);
}

CDatabaseGroupItem::CDatabaseGroupItem(CDatabaseListView * parent, CMySQLServer *m, const QString &connection_name)
: CDatabaseListViewItem(parent, m, DATABASE_GROUP, "CDatabaseGroupItem")
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::CDatabaseGroupItem(CDatabaseListView *, CMySQLServer *, const QString &)");
#endif

  if (m == 0)
  {
    m_mysql = new CMySQLServer(connection_name, messagePanel());
    mysql()->connect();
    deletemysql = true;
  }
  init();
  if (mysql()->isConnected())
    refreshWidget(true);
  setOpen(true);
}


void CDatabaseGroupItem::refreshWidget(bool b)
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::refreshWidget(%s)", debug_string(booltostr(b)));
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

void CDatabaseGroupItem::setOpen(bool b)
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::setOpen(%s)", debug_string(booltostr(b)));
#endif

  if (isBlocked())
    return;

  setPixmap(0, b ? openFolderIcon : closedFolderIcon);
  QListViewItem::setOpen(b);
}

void CDatabaseGroupItem::activated()
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::activated()");
#endif

  if (isBlocked())
    return;
  refreshWidget(false);
}

void CDatabaseGroupItem::init()
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::init()");
#endif

  widgetData.resize(25);
  widgetData.setAutoDelete(true);
  openFolderIcon = getPixmapIcon("openFolderIcon");
  closedFolderIcon = getPixmapIcon("closedFolderIcon");
  newDatabaseIcon = getPixmapIcon("newDatabaseIcon");
  refreshIcon = getPixmapIcon("refreshIcon");
  newWindowIcon = getPixmapIcon("newWindowIcon");
  setText(0, tr("Databases"));
  addDatabases(false);
}

void CDatabaseGroupItem::addDatabases(bool is_refresh)
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::addDatabases(%s)", debug_string(booltostr(is_refresh)));
#endif

  if (!mysql()->isConnected() || isBlocked())
    return;

  setBlocked(true);

  QStringList database_list;  
  
  if (mysql()->hasDatabases())
  {
    if (!mysql()->showAllDatabases())
    {
      mysql()->mysql()->setEmitErrors(false);
      QStringList tmpList = mysql()->databases();
      for (QStringList::Iterator it = tmpList.begin(); it != tmpList.end(); ++it)
         if (mysql()->mysql()->mysqlSelectDb(*it))
          database_list.append(*it);
    }
    else
      database_list = mysql()->databases();
  }
  else
  {
    CMySQLQuery *qry = new CMySQLQuery(mysql()->mysql(), true);
    qry->setEmitMessages(false);
    qry->setEmitErrors(false);  //Avoid an "Access denied" error when --skip-show-database is used in the server.
    mysql()->messagePanel()->information("[" + mysql()->connectionName() + "] " + tr("Querying MySQL Server for Database Information."));
    qApp->processEvents();
    if (!qry->exec("SHOW DATABASES"))
    {
      setBlocked(false);
      return;
    }
    qry->setEmitErrors(true);    
    while (qry->next(true))
    {
      if (!mysql()->showAllDatabases())
      {
        mysql()->mysql()->setEmitErrors(false);
        if (mysql()->mysql()->mysqlSelectDb(qry->row(0)))
          database_list.append(qry->row(0));
      }
      else
        database_list.append(qry->row(0));
    }
    delete qry;
  }

  if (!mysql()->showAllDatabases())
    mysql()->mysql()->setEmitErrors(true);

  if (is_refresh)
  {
    QPtrList<CDatabaseListViewItem> removed_databases_list; 
    QListViewItemIterator c( firstChild() );
    for ( ; c.current(); ++c )
    {
      if (c.current()->parent() == this)
      {
        if (database_list.findIndex(c.current()->text(0)) != -1)
          database_list.remove (c.current()->text(0));
        else
        {
          CDatabaseListViewItem *p = (CDatabaseListViewItem *) c.current();
          p->setVisible(false);
          removed_databases_list.append(p);
        }
      }
    }
    if (!removed_databases_list.isEmpty())
    {
      CDatabaseListViewItem *p;
      for (p = removed_databases_list.first(); p; p = removed_databases_list.next())
        delete p;
    }
  }
  if (!database_list.isEmpty())
  {
  
    QPixmap p1 = getPixmapIcon("databaseDisconnectedIcon");
    QPixmap p2 = getPixmapIcon("databaseConnectedIcon");
    QStringList::Iterator i = database_list.begin();
    while ( i != database_list.end() )
    {
      new CDatabaseItem(this, *i, mysql(), p1, p2);
      i++;
    }
  }
  setBlocked(false);
}

void CDatabaseGroupItem::processMenu(int res)
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::processMenu(%d)", res);
#endif

  if (isBlocked())
    return;

  switch (res) {
		case MENU_NEW:
      {
        bool ok = false;
        QString dbname = QInputDialog::getText(tr("Create Database"), tr("Enter new Database name") +
          ":", QLineEdit::Normal, QString::null, &ok);
        if ( ok && !dbname.isEmpty())
        {
          setBlocked(true);
          CMySQLQuery query(mysql()->mysql());
          query.setEmitMessages(false);
          QString qry;
          if (mysql()->mysql()->version().major <= 3 && mysql()->mysql()->version().minor <= 22)
            qry = "CREATE DATABASE " + dbname;
          else
            qry = "CREATE DATABASE IF NOT EXISTS " + mysql()->mysql()->quote(dbname);
          if (query.execStaticQuery(qry))
            new CDatabaseItem(this, dbname, mysql());
          setBlocked(false);
        }
      }
      break;

    case MENU_REFRESH:
      {
        addDatabases(true);
        refreshWidget(true);
      }
      break;

    case MENU_NEW_WINDOW_FROM_HERE: databaseListView()->openInNewWindow(this);
      break;
  }
}

void CDatabaseGroupItem::initMenuItems()
{
#ifdef DEBUG
  qDebug("CDatabaseGroupItem::initMenuItems()");
#endif

  menu()->clear();
  menu()->insertItem(newDatabaseIcon, tr("&New Database"), MENU_NEW);  
  menu()->insertSeparator();    
  menu()->insertItem(refreshIcon, tr("&Refresh"), MENU_REFRESH);  
  menu()->insertSeparator();  
  menu()->insertItem(newWindowIcon, tr("New &Window from Here"), MENU_NEW_WINDOW_FROM_HERE);
  menu()->setItemEnabled(MENU_NEW_WINDOW_FROM_HERE, !mysql()->oneConnection());
}
