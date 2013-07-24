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
#ifndef CTABLESPECIFIC_H
#define CTABLESPECIFIC_H

#include <stddef.h>
#include <qvariant.h>
#include <qdialog.h>
#include <q3popupmenu.h>
#include <qinputdialog.h>

class CMySQLServer;
class CDatabaseListViewItem;

class CTableSpecific
{
  
public:

  static bool importTable(CMySQLServer *m);
  static bool exportTable(CMySQLServer *m, const QString &dbname, const QString & tableName);
  static void tableProperties(CMySQLServer *m, const QString &dbname, const QString & tableName);
  static bool deleteTable(CMySQLServer *m, const QString &dbname, const QString & tableName);  
  static bool emptyTable(CMySQLServer *m, const QString &dbname, const QString & tableName);
  static bool renameTable(CMySQLServer *m, const QString &dbname, const QString & from, const QString & to);
  static void QueryWindow(CMySQLServer *m, const QString &dbname, const QString & tableName, int q);
};

#endif
