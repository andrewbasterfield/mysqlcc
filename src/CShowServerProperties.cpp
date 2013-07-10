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
#include "CShowServerProperties.h"
#include "CMySQLServer.h"
#include "globals.h"

CShowServerProperties::CShowServerProperties(QWidget* parent, CMySQLServer *m)
: CProperties(parent, m, "CShowServerProperties")
{
#ifdef DEBUG
  qDebug("CShowServerProperties::CShowServerProperties()");
#endif

  details->addColumn(tr("Property"));
  details->addColumn(tr("Value"));
  details->header()->setClickEnabled(false, 1);
  setSaveTitle(tr("Connection Properties"));
}

void CShowServerProperties::refresh()
{
#ifdef DEBUG
  qDebug("CShowServerProperties::refresh()");
#endif

  setTitle(mysql()->connectionName());
  CProperties::refresh();
}

void CShowServerProperties::insertData()
{
#ifdef DEBUG
  qDebug("CShowServerProperties::insertData()");
#endif

  insertItem(tr("Connected"), booltoyesno(mysql()->isConnected()));
  insertItem(tr("Host Name"), mysql()->hostName());
  insertItem(tr("Date Created"), mysql()->created());
  insertItem(tr("Date Modified"), mysql()->modified());
  insertItem(tr("User Name"), mysql()->userName());
  insertItem(tr("Using Blocking Queries"), booltoyesno(mysql()->blockingQueries()));
  insertItem(tr("Using Compression"), booltoyesno(mysql()->compress()));
  insertItem(tr("Using SSL"), booltoyesno(mysql()->ssl()));
  insertItem(tr("Using Prompt for Password"), booltoyesno(mysql()->promptPassword()));
  insertItem(tr("Using a Single Connection"), booltoyesno(mysql()->oneConnection()));
  insertItem(tr("Has Syntax Highlighting and Completion"), booltoyesno(mysql()->completion()));
  insertItem(tr("Fields Separated by"), mysql()->fieldSeparator());
  insertItem(tr("Fields Enclosed by"), mysql()->fieldEncloser());  
  insertItem(tr("Lines Terminated by"), mysql()->lineTerminator());
  insertItem(tr("Replace Empty fields with"), mysql()->replaceEmpty());
  if (!mysql()->socketFile().isEmpty())
    insertItem(tr("Socket File"), mysql()->socketFile());
  else
    insertItem(tr("Port"), QString::number(mysql()->port()));
  insertItem(tr("MySQL Server version"), mysql()->mysql()->mysqlGetServerInfo());
  uint i = mysql()->mysql()->mysqlGetProtoInfo();
  insertItem(tr("MySQL Protocol version"), i != 0 ? QString::number(i) : QString::null);
  insertItem(tr("MySQL Connection version"), mysql()->mysql()->mysqlGetHostInfo());
  insertItem(tr("MySQL Uptime"), mysql()->mysql()->uptime());
}
