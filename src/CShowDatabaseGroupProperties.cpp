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
#include "CShowDatabaseGroupProperties.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "globals.h"

CShowDatabaseGroupProperties::CShowDatabaseGroupProperties(QWidget* parent, CMySQLServer *m)
: CProperties(parent, m, "CShowDatabaseGroupProperties")
{
#ifdef DEBUG
  qDebug("CShowDatabaseGroupProperties::CShowDatabaseGroupProperties()");
#endif

  details->addColumn(tr("Property" ) );
  details->addColumn(tr("Value" ) );
  details->header()->setClickEnabled(false, 1);
  setSaveTitle(tr("Database Status"));
}

void CShowDatabaseGroupProperties::refresh()
{
#ifdef DEBUG
  qDebug("CShowDatabaseGroupProperties::refresh()");
#endif

  setTitle(mysql()->connectionName());
  CProperties::refresh();
}

void CShowDatabaseGroupProperties::insertData()
{
#ifdef DEBUG
  qDebug("CShowDatabaseGroupProperties::insertData()");
#endif

  QRegExp rx( "(.[^:]+)(:\\s)(\\d+(\\.\\d+)?)");
  int pos = 0;
  QString p;
  QString v;
  while (pos >= 0 )
  {
    pos = rx.search(mysql()->mysql()->mysqlStat(), pos);
    if ( pos > -1 )
    {
      p = rx.cap(1).stripWhiteSpace();
      v = rx.cap(3).stripWhiteSpace();
      if (p.lower() == "uptime")
        v = nice_time(v.toULong());
      insertItem(p,v);
      pos  += rx.matchedLength();
    }
  }
}
