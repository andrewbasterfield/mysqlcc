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
#include "CTableItem.h"
#include "CTableGroupItem.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "config.h"
#include "globals.h"
#include "CShowTableProperties.h"
#include "CTableSpecific.h"
#include "CTableTools.h"
#include "CTableWindow.h"
#include "CQueryWindow.h"
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

CTableItem::CTableItem(CDatabaseListViewItem * parent, const QString &table_name, CMySQLServer *m)
: CDatabaseListViewItem(parent, m, TABLE, "CTableItem")
{
#ifdef DEBUG
  qDebug("CTableItem::CTableItem(CDatabaseListViewItem *, '%s', CMySQLServer *)", debug_string(table_name));
#endif

  my_parent = (CTableGroupItem *)parent;
  setDatabaseName(my_parent->databaseName());
  widgetData.setAutoDelete(true);
  headers.setAutoDelete(true);
  pix.setAutoDelete(true);
  setPixmap(0, my_parent->tableIcon());
  setText(0, table_name);
  setTableName(table_name);
  setRenameEnabled(0, true);
  real_numCols = 0;
  real_numRows = 0;
}

CTableItem::~CTableItem()
{
#ifdef DEBUG
  qDebug("CTableItem::~CTableItem()");
#endif

}

void CTableItem::refreshWidget(bool b)
{
#ifdef DEBUG
  qDebug("CTableItem::refreshWidget()");
#endif

  if (isBlocked())
    return;

  setBlocked(true);
  CShowTableProperties * w = (CShowTableProperties *) widgetStack()->widget(int_type());
  w->setCurrentItem(this);

  if (real_numCols == 0 && real_numRows == 0 || b)
  {  
    CMySQLQuery *qry = new CMySQLQuery(mysql()->mysql(), true);
    qry->setEmitMessages(false);
    QString sql = "SHOW COLUMNS FROM " + mysql()->mysql()->quote(databaseName()) + "." + mysql()->mysql()->quote(text(0));
    if (qry->exec(sql))
    {
      real_numRows = qry->numRows();
      real_numCols = qry->numFields();
      widgetData.clear();
      widgetData.resize(real_numRows * real_numCols);
      w->setNumRows(real_numRows);
      w->setNumCols(real_numCols);
      for (uint i = 0; i < real_numCols; i++)
        w->horizontalHeader()->setLabel(i, qry->fields(i).name);
      ulong j = 0;
      ulong k = 0;
      QString icon;
      pix.clear();
      pix.resize(real_numRows + 1);
      QPixmap icn;
      QString str;
      while (qry->next())
      {
        for (uint i = 0; i < real_numCols; i++)
        {
          str = qry->row(i);
          widgetData.insert(k++, new QString(str));
          w->setText(j, i, str);
          if (i == 3)
          {
            if (str == "PRI")
              icon = "pkIcon";            
            else  if (str == "MUL")
              icon = "mulIcon";              
            else if (str == "UNI")
              icon = "uniIcon";                
            else
              icon = str.isEmpty() ? "nothingIcon" : "otherIcon";
            icn = getPixmapIcon(icon);
            w->setPixmap(j, 0, icn);
            pix.insert(j, new QPixmap(icn));
          }
        }
        j++;
      }  
    }
    delete qry;
    headers.clear();
    headers.resize(real_numCols + 1);
    for (uint i = 0; i < real_numCols; i++)
      headers.insert(i, new header(w->horizontalHeader()->label(i), w->horizontalHeader()->sectionSize(i)));
  }
  else
  {
    w->reset();
    w->setNumRows(real_numRows);
    w->setNumCols(real_numCols);
    for (uint i = 0; i < real_numRows; i++)
      for (uint j = 0; j < real_numCols; j++)
      {
        if (i == 0)
        {
          header *h = headers.find(j);
          w->horizontalHeader()->setLabel(j, h->caption());
          w->horizontalHeader()->resizeSection(j, h->size());
        }
        if (j == 0)
        {
          QPixmap *p = pix.find(i);
          w->setPixmap(i, 0, *p);
        }
        w->setText(i, j, *widgetData.find((i * real_numCols) + j));
      }
  }
  widgetStack()->raiseWidget(int_type());
  setBlocked(false);
}

void CTableItem::keyPressed(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CTableItem::keyPressed()");
#endif

  if (isBlocked())
    return;

  if (e->key() == Qt::Key_Delete)
    processMenu(MENU_DELETE);
  else
    CDatabaseListViewItem::keyPressed(e);
}

void CTableItem::activated()
{
#ifdef DEBUG
  qDebug("CTableItem::activated()");
#endif

  if (isBlocked())
    return;

  refreshWidget(false);
}

void CTableItem::renamed(const QString &newtext)
{
#ifdef DEBUG
  qDebug("CTableItem::renamed('%s')", debug_string(newtext));
#endif

  if (!CTableSpecific::renameTable(mysql(), databaseName(), oldText(), newtext))
    setText(0, oldText());
  else
  {
    setText(0, newtext);
    setTableName(newtext);
    my_parent->setCurrentTableName(newtext);
    my_parent->refreshWidget(true);
  }
}

void CTableItem::doubleClicked()
{
#ifdef DEBUG
  qDebug("CTableItem::doubleClicked()");
#endif

  if (isBlocked())
    return;

  setBlocked(true);
  CTableSpecific::QueryWindow(mysql(), databaseName(), text(0), SQL_ALL_ROWS);
  setBlocked(false);
}

void CTableItem::processMenu(int res)
{
#ifdef DEBUG
  qDebug("CTableItem::processMenu(%d)", res);
#endif
  
  if (isBlocked())
    return;
  mysql()->mysql()->mysqlSelectDb(databaseName()); 
  switch (res)
  {
    //Tools & Open Table are handled by CTableGroupItem.cpp

    case MENU_EDIT:
      {
        CTableWindow *w = new CTableWindow(myApp()->workSpace(), mysql(), databaseName(), text(0));
        connect(w, SIGNAL(do_refresh()), my_parent, SLOT(refresh()));
        w->setFocus();
        w->raise();
        myShowWindow(w);
      }
      break;
    
    case MENU_PROPERTIES: CTableSpecific::tableProperties(mysql(), databaseName(), text(0));
      break;
    
    case MENU_EXPORT: CTableSpecific::exportTable(mysql(), databaseName(), text(0));
      break;
    
    case MENU_EMPTY: CTableSpecific::emptyTable(mysql(), databaseName(), text(0));
      break;
    
    case MENU_DELETE:
      if (CTableSpecific::deleteTable(mysql(), databaseName(), text(0)))
      {
		setVisible(false);
		my_parent->refresh();        
      }
      break;
    
    case MENU_RENAME: startRename(0);
      break;
    
    case MENU_REFRESH: refreshWidget(true);
      break;
  }
}

void CTableItem::initMenuItems()
{
#ifdef DEBUG
  qDebug("CTableItem::initMenuItems()");
#endif

  // All the icons and menus are created in CTableGroupItem to save some memory and make things faster.

  my_parent->setCurrentTableName(text(0));
  
  menu()->clear();

  menu()->insertItem(my_parent->openTableIcon(), tr("&Open Table"), my_parent->openTableMenu(), MENU_OPEN);

  menu()->insertItem(my_parent->designIcon(), tr("&Edit Table"), MENU_EDIT);
  menu()->insertItem(my_parent->propertiesIcon(), tr("&Properties"), MENU_PROPERTIES);
  menu()->insertItem(my_parent->exportTableIcon(), tr("E&xport Table"), MENU_EXPORT);
  menu()->insertSeparator();
  
  menu()->insertItem(my_parent->toolsIcon(), tr("&Tools"), my_parent->toolsMenu(), MENU_TOOLS);
  menu()->insertSeparator();
  menu()->insertItem(my_parent->trashIcon(), tr("E&mpty Table"), MENU_EMPTY);
  menu()->insertItem(my_parent->deleteIcon(), tr("D&rop Table"), MENU_DELETE);
  menu()->insertItem(my_parent->renameTableIcon(), tr("Re&name"), MENU_RENAME);
  menu()->insertSeparator();
  menu()->insertItem(my_parent->refreshTablesIcon(), tr("&Refresh Fields"), MENU_REFRESH);
  
  //Disable features that aren't currently working
  menu()->setItemEnabled(MENU_EXPORT, false);
  
  if (my_parent->isOldMySQL())  //Disable unsupported features for mysql <= 3.22
  {
    menu()->setItemEnabled(MENU_TOOLS, false);
    menu()->setItemEnabled(MENU_PROPERTIES, false);
  }
}
