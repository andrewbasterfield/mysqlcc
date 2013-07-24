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
#include "CTablesListBox.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "globals.h"
#include "config.h"
#include <stddef.h>  
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QShowEvent>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 2
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CTablesListBox::CTablesListBox(QWidget * parent, CMySQLServer *m, const QString & dbname, const QString & tableName, const char * name)
: Q3ListBox(parent, name)
{  
#ifdef DEBUG
  qDebug("CTablesListBox::CTablesListBox(QWidget *, CMySQLServer *, '%s', '%s', const char *)", debug_string(dbname), debug_string(tableName));
#endif

  if (!name)
    setName("CTablesListBox");
  setSelectionMode(Q3ListBox::Extended);
  numItems = -1;
  mysql = m;
  database_name = mysql->mysql()->quote(dbname);
  m_tableName = tableName;
  connect(this, SIGNAL(rightButtonClicked(Q3ListBoxItem *, const QPoint &)), this, SLOT(displayMenu(Q3ListBoxItem *, const QPoint &)));
}

void CTablesListBox::displayMenu(Q3ListBoxItem *, const QPoint & pos)
{
#ifdef DEBUG
  qDebug("CTablesListBox::displayMenu()");
#endif

  Q3PopupMenu *p_itemMenu = new Q3PopupMenu();
  Q_CHECK_PTR(p_itemMenu);  
  p_itemMenu->insertItem(getPixmapIcon("refreshTablesIcon"), tr("Refresh"), 1);
  if (p_itemMenu->exec(pos) == 1)
    refresh();
  delete p_itemMenu;	
}

void CTablesListBox::showEvent(QShowEvent *)
{
#ifdef DEBUG
  qDebug("CTablesListBox::showEvent()");
#endif

  refresh();
}

void CTablesListBox::refresh()
{
#ifdef DEBUG
  qDebug("CTablesListBox::refresh()");
#endif

  clear();
  int j = -1;  
  if (mysql->isConnected())
  {
    CMySQLQuery *qry = new CMySQLQuery(mysql->mysql());
    qry->setEmitMessages(false);
    if (qry->exec("SHOW TABLES FROM " + database_name))
      while (qry->next())
      {
        insertItem(getPixmapIcon("tableIcon"), qry->row(0), ++j);
        if (qry->row(0) == m_tableName)
        {
          setSelected(item(j), true);
          setCurrentItem(j);
          centerCurrentItem();
        }
      }
    delete qry;
  }
  numItems = j;
  if (count() < 0)
    emit(notablesloaded());
}

QString CTablesListBox::selectedTables() const
{
#ifdef DEBUG
  qDebug("CTablesListBox::selectedTables()");
#endif

  QString ret = QString::null;
  for (int i=0; i <= numItems; i++)
    if (isSelected (i))
      ret += mysql->mysql()->quote(text(i)) + ", ";
    if (!ret.isEmpty())
      return ret.left(ret.length() - 2);
    return QString::null;
}

QStringList CTablesListBox::selectedTablesList() const
{
#ifdef DEBUG
  qDebug("CTablesListBox::selectedTablesList()");
#endif

  QStringList list;	
  for (int i=0; i <= numItems; i++)
    if (isSelected (i))
      list.append(text(i));
    return list;
}
