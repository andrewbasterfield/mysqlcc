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
#include "CTableSpecific.h"
#include "CApplication.h"
#include "config.h"
#include "CMySQLServer.h"
#include "CMySQL.h"
#include "CMySQLQuery.h"
#include "panels.h"
#include "CQueryWindow.h"
#include "globals.h"
#include "CTableProperties.h"
#include "CDatabaseListViewItem.h"
#include <qmessagebox.h>


static QString table_name(CMySQLServer *m, const QString &db, const QString &t)
{
  return m->mysql()->quote(db) + "." + m->mysql()->quote(t);
}

bool CTableSpecific::renameTable(CMySQLServer *m, const QString &dbname, const QString & from, const QString & to)
{
#ifdef DEBUG
  qDebug("static CTableSpecific::renameTable(CMySQLServer *, '%s', '%s')", debug_string(from), debug_string(to));
#endif

  CMySQLQuery *qry = new CMySQLQuery(m->mysql());
  qry->setEmitMessages(false);
  bool ret = false;
  QString sql;
  bool notnew = m->mysql()->version().major <= 3 && m->mysql()->version().minor <= 22;
  if (notnew)
    sql = "ALTER TABLE " + table_name(m, dbname, from) + " RENAME " + table_name(m, dbname, to);
  else
    sql = "RENAME TABLE " + table_name(m, dbname, from) + " TO " + table_name(m, dbname, to);

  if (qry->execStaticQuery(sql))
  {
    m->messagePanel()->warning(QObject::tr("Successfully Renamed Table") + ": " + from + " -> " + to);
    ret = true;
  }
  delete qry;
  return ret;
}

bool CTableSpecific::importTable(CMySQLServer *)
{
#ifdef DEBUG
  qDebug("static CTableSpecific::importTable()");
#endif

 // QMessageBox::warning(0,"Import Table", "Import Table");
  return false;
}

bool CTableSpecific::exportTable(CMySQLServer *, const QString &/*dbname*/, const QString & /*tableName*/)
{
#ifdef DEBUG
  qDebug("static CTableSpecific::exportTable()");
#endif

//  QMessageBox::warning(0,"Export Table", "Export Table: " + tableName);
  return false;
}

bool CTableSpecific::deleteTable(CMySQLServer *m, const QString &dbname, const QString & tableName)
{
#ifdef DEBUG
  qDebug("static CTableSpecific::deleteTable(CMySQLServer *, '%s')", debug_string(tableName));
#endif

  if (myApp()->confirmCritical())
    if ((QMessageBox::information(0, QObject::tr("Confirm Delete Table"), 
      QObject::tr("Are you sure you want to Delete Table") + ": \"" + tableName + "\" ?",
      QObject::tr("&Yes"), QObject::tr("&No")) != 0))
    return false;
  
  CMySQLQuery *qry = new CMySQLQuery(m->mysql());
  qry->setEmitMessages(false);
  bool ret = false;

  QString sql = "DROP TABLE " + table_name(m, dbname, tableName);
  if (qry->execStaticQuery(sql))
  {
    m->messagePanel()->warning(QObject::tr("Successfully Deleted Table") + ": " + tableName);
    ret = true;
  }
  delete qry;
  return ret;
}

bool CTableSpecific::emptyTable(CMySQLServer *m, const QString &dbname, const QString & tableName)
{
#ifdef DEBUG
  qDebug("static CTableSpecific::emptyTable(CMySQLServer *, '%s')", debug_string(tableName));
#endif

  if (myApp()->confirmCritical())
    if (QMessageBox::information(0, QObject::tr("Confirm Empty Table"), 
      QObject::tr("Are you sure you want to Empty Table") + ": \"" + tableName + "\" ?",
      QObject::tr("&Yes"), QObject::tr("&No")) != 0)
    return false;

  CMySQLQuery *qry = new CMySQLQuery(m->mysql());
  qry->setEmitMessages(false);
  bool ret = false;
  QString sql;
  bool notnew = (m->mysql()->version().major <= 3 && m->mysql()->version().minor <= 22);
  sql = notnew ? "DELETE FROM " : "TRUNCATE TABLE ";  
  sql += table_name(m, dbname, tableName);
  if (qry->execStaticQuery(sql))
  {
    m->messagePanel()->warning(QObject::tr("Successfully Emptied Table") + ": " + tableName);
    ret = true;
  }
  delete qry; 
  return ret;
}


void CTableSpecific::tableProperties(CMySQLServer *m, const QString &dbname, const QString & tableName)
{
#ifdef DEBUG
  qDebug("static CTableSpecific::tableProperties(CMySQLServer *, '%s')", debug_string(tableName));
#endif
  
  CTableProperties *w = new CTableProperties(myApp()->workSpace(), m , tableName, dbname);  
  w->setFocus();
  w->raise();
  myShowWindow(w);  
}


void CTableSpecific::QueryWindow(CMySQLServer *m, const QString &dbname, const QString & tableName, int query_type)
{
#ifdef DEBUG
  qDebug("static CTableSpecific::QueryWindow(CMySQLServer *, '%s', '%s', %d)", debug_string(dbname), debug_string(tableName), query_type);
#endif

  int limit = 0;
  if (query_type == SQL_LIMIT)
  {
    bool ok = false;
    limit = QInputDialog::getInteger(
      QObject::tr("Number of Rows"),
      QObject::tr("Maximum number of Rows to fetch") + ":", 1000, 0, 2147483647, 1, &ok, 0);

    if (!ok)
      return;		
  }

  ushort flags = query_type != SQL_QUERY ?
    (myApp()->retrieveAllRecordsFromTable() ?
      RESULTS_PANEL :
      SQL_PANEL | RESULTS_PANEL)
    : SQL_PANEL;

  CQueryWindow *w = new CQueryWindow(myApp()->workSpace(), m, query_type, flags);

  if (!dbname.isEmpty())
  {
    w->setDatabase(dbname);

    if (!tableName.isEmpty())
    {
      w->setTable(tableName);
      QString sql = "SELECT *\nFROM " + m->mysql()->quote(tableName);
      if (query_type == SQL_LIMIT)
        sql += "\nLIMIT " + QString::number(limit);
      w->setQuery(sql);
    }
  }
  w->setFocus();
  w->raise();  
  myShowWindow(w);
  qApp->processEvents();
  if (query_type != SQL_QUERY && myApp()->retrieveAllRecordsFromTable())
    w->executeQuery();
}
