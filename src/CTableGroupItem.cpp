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
#include "CTableGroupItem.h"
#include "CTableItem.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "globals.h"
#include "config.h"
#include "CTableTools.h"
#include "CShowTableGroupProperties.h"
#include "CTableSpecific.h"
#include "CTableWindow.h"
#include "panels.h"
#include <qdict.h>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 1
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif


CTableGroupItem::CTableGroupItem(CDatabaseListViewItem * parent, const QString &dbname, CMySQLServer *m)
: CDatabaseListViewItem(parent, m, TABLE_GROUP, "CTableGroupItem")
{
#ifdef DEBUG
  qDebug("CTableGroupItem::CTableGroupItem(CDatabaseListViewItem *, const QString &, CMySQLServer *)");
#endif

  init(dbname);
  dont_raise_widget = true;
  refreshWidget(true);
  dont_raise_widget = false;
}


CTableGroupItem::CTableGroupItem(CDatabaseListView * parent, const QString &dbname, const QString &connection_name, CMySQLServer *m)
: CDatabaseListViewItem(parent, m, TABLE_GROUP, "CTableGroupItem")
{
#ifdef DEBUG
  qDebug("CTableGroupItem::CTableGroupItem(CDatabaseListView *, const QString &, const QString &, CMySQLServer *)");
#endif

  dont_raise_widget = false;
  if (m == 0)
  {
    m_mysql = new CMySQLServer(connection_name, messagePanel());
    mysql()->connect();
    deletemysql = true;
  }
  init(dbname);
  setOpen(true);
}

CTableGroupItem::~CTableGroupItem()
{
#ifdef DEBUG
  qDebug("CTableGroupItem::~CTableGroupItem()");
#endif

  delete tools_menu;
  delete open_table_menu;
}

void CTableGroupItem::init(const QString &dbname)
{
#ifdef DEBUG
  qDebug("CTableGroupItem::init('%s')", debug_string(dbname));
#endif

  refresh_on_activate = false;
  setDatabaseName(dbname);
  tools_menu = CTableTools::tableToolsMenu();
  connect(tools_menu, SIGNAL(activated(int)), this, SLOT(processMenu(int)));

  is_old_mysql = mysql()->mysql()->version().major <= 3 && mysql()->mysql()->version().minor <= 22;
  if (is_old_mysql)
    tools_menu->setEnabled(false);

  retrieveShowTableStatus = mysql()->retrieveShowTableStatus();
  database_name = dbname;
  newTableIcon = getPixmapIcon("newTableIcon");
  tools_icon = getPixmapIcon("toolsIcon");
  importTableIcon = getPixmapIcon("importTableIcon");
  refresh_tables_icon = getPixmapIcon("refreshTablesIcon");
  newWindowIcon = getPixmapIcon("newWindowIcon");
  table_icon = getPixmapIcon("tableIcon");

  open_table_icon = getPixmapIcon("openTableIcon");
  design_icon = getPixmapIcon("designIcon");
  properties_icon = getPixmapIcon("propertiesIcon");
  export_table_icon = getPixmapIcon("exportTableIcon");
  trash_icon = getPixmapIcon("trashIcon");
  delete_icon = getPixmapIcon("deleteIcon");
  rename_table_icon = getPixmapIcon("renameTableIcon");

  open_table_menu = new QPopupMenu();
  open_table_menu->insertItem(tr("Return &all rows"), SQL_ALL_ROWS);
  open_table_menu->insertItem(tr("Return &Limit"), SQL_LIMIT);
  connect(open_table_menu, SIGNAL(activated(int)), this, SLOT(processMenu(int)));

  setPixmap(0, getPixmapIcon("tablesIcon"));
  setText(0, tr("Tables"));
  real_numRows = 0;
  real_numCols = 0;
}

void CTableGroupItem::insertWidgetData(QWidget *w, int id, int row, int col, const QString &str)
{
#ifdef DEBUG
  qDebug("CTableGroupItem::insertWidgetData()");
#endif
  
  widgetData.insert(id, str);
  ((CShowTableGroupProperties *) w)->setText(row, col, str);
}

void CTableGroupItem::refreshWidget(bool b)
{
#ifdef DEBUG
  qDebug("CTableGroupItem::refresh(%s)", debug_string(booltostr(b)));
#endif

  if (isBlocked())
    return;

  setBlocked(true);

  CShowTableGroupProperties * w = (CShowTableGroupProperties *) widgetStack()->widget(int_type());
  w->setCurrentItem(this);

  if (b || (real_numCols == 0 &&  real_numRows == 0) || retrieveShowTableStatus != mysql()->retrieveShowTableStatus())
  {
    retrieveShowTableStatus = mysql()->retrieveShowTableStatus();    
     
    CMySQLQuery *qry = new CMySQLQuery(mysql()->mysql(), true);
    qry->setEmitMessages(false);

    bool notnew = is_old_mysql || !retrieveShowTableStatus;

    QString sql = notnew ? "SHOW TABLES" : "SHOW TABLE STATUS";
    sql += " FROM " + mysql()->mysql()->quote(database_name);
    mysql()->messagePanel()->information("[" + mysql()->connectionName() + "] " + tr("Querying MySQL Server for Table information in database:") + " " + database_name);
    qApp->processEvents();
    if (qry->exec(sql))
    {
      while (this->childCount() > 0)
        deleteChilds();      

      real_numRows = qry->numRows();
      real_numCols = notnew ? 1 : 6;
      widgetData.clear();
      w->setNumRows(real_numRows);
      w->setNumCols(real_numCols);
      w->horizontalHeader()->setLabel(0, tr("Table"));
      if (!notnew)
      {
        w->horizontalHeader()->setLabel(1, tr("Records"));
        w->horizontalHeader()->setLabel(2, tr("Size (bytes)"));
        w->horizontalHeader()->setLabel(3, tr("Created"));
        w->horizontalHeader()->setLabel(4, tr("Type"));
        w->horizontalHeader()->setLabel(5, tr("Comments"));
      }    
      int j = 0;
      int k = 0;
      QString str0;      
      while (qry->next())
      {
        str0 = qry->row(0);
        new CTableItem (this, str0, mysql());
        widgetData.insert(k++, str0);
        w->setPixmap(j,0, table_icon);
        w->setText(j,0, str0);
        if (!notnew)
        {
          insertWidgetData(w, k++, j, 1, qry->row(3));
          insertWidgetData(w, k++, j, 2, qry->row(5));
          insertWidgetData(w, k++, j, 3, qry->row(10));
          insertWidgetData(w, k++, j, 4, qry->row(1));
          insertWidgetData(w, k++, j, 5, qry->row(14));          
        }
        j++;
      }
    }
    delete qry;
  }
  else
  {
    w->setNumRows(real_numRows);
    w->setNumCols(real_numCols);
    w->horizontalHeader()->setLabel(0, tr("Table"));
    if (real_numCols > 1)
    {
      w->horizontalHeader()->setLabel(1, tr("Records"));
      w->horizontalHeader()->setLabel(2, tr("Size (bytes)"));
      w->horizontalHeader()->setLabel(3, tr("Created"));
      w->horizontalHeader()->setLabel(4, tr("Type"));
      w->horizontalHeader()->setLabel(5, tr("Comments"));
    }
    for (int i = 0; i < real_numRows; i++)
      for (int j = 0; j < real_numCols; j++)
      {
        if (j == 0)
          w->setPixmap(i, 0, table_icon);

        w->setText(i, j, *widgetData.find((i * real_numCols) + j));
      }
  }

  if (real_numCols > 0)
    for (int j = 0; j < real_numCols; j++)
      w->adjustColumn(j);

  if (!dont_raise_widget)
    widgetStack()->raiseWidget(int_type());

  setBlocked(false);
}

void CTableGroupItem::activated()
{
#ifdef DEBUG
  qDebug("CTableGroupItem::activated()");
#endif

  if (isBlocked())
    return;

  if (refresh_on_activate)
  {
    refreshWidget(true);
    refresh_on_activate = false;
  }

  refreshWidget(false);
}

void CTableGroupItem::processMenu(int res)
{
#ifdef DEBUG
  qDebug("CTableGroupItem::processMenu(%d)", res);
#endif

  if (isBlocked())
    return;
  
  mysql()->mysql()->mysqlSelectDb(databaseName());
  if (CTableTools::inRange(res))
    CTableTools::processMenu(mysql(), res, databaseName(), current_table_name);
  else
    if (res == SQL_LIMIT || res == SQL_ALL_ROWS)
    {
      setBlocked(true);      
      CTableSpecific::QueryWindow(mysql(), databaseName(), current_table_name, res);
      setBlocked(false);
    }
    else
    {     
      switch (res)
      { 
        case MENU_NEW:
          {
            CTableWindow *w = new CTableWindow(myApp()->workSpace(), mysql(), databaseName(), QString::null);
            connect(w, SIGNAL(do_refresh()), this, SLOT(refresh()));
            w->setFocus();
            w->raise();
            myShowWindow(w);
          }
          break;
          
        case MENU_IMPORT: CTableSpecific::importTable(mysql());
          break;
        
        case MENU_REFRESH:
          {
            if (childCount() == 0)
            {
              refreshWidget(true);
              setOpen(true);
            }
            else
              refreshWidget(true);
          }
          break;
        
        case MENU_NEW_WINDOW_FROM_HERE: databaseListView()->openInNewWindow(this);
          break;
      }
    }
}

void CTableGroupItem::initMenuItems()
{
#ifdef DEBUG
  qDebug("CTableGroupItem::initMenuItems()");
#endif

  current_table_name = QString::null;
  menu()->clear();

  menu()->clear();
  menu()->insertItem(newTableIcon, tr("&New Table"), MENU_NEW);
  menu()->insertSeparator();  

  menu()->insertItem(tools_icon, tr("&Tools"), tools_menu, MENU_TOOLS);
  menu()->insertItem(importTableIcon, tr("&Import"), MENU_IMPORT);

  menu()->insertSeparator();  
  menu()->insertItem(refresh_tables_icon, tr("&Refresh"), MENU_REFRESH);
  menu()->insertSeparator();  
  menu()->insertItem(newWindowIcon, tr("New &Window from Here"), MENU_NEW_WINDOW_FROM_HERE);
  
  //Disable features that aren't currently working
  menu()->setItemEnabled(MENU_IMPORT, false);


  if (is_old_mysql)  //Disable unsupported features for mysql <= 3.22
    menu()->setItemEnabled(MENU_TOOLS, false);
}
