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
#ifndef CDATABASEGROUPITEM_H
#define CDATABASEGROUPITEM_H

#include "CDatabaseListViewItem.h"
#include <qdict.h>

class CDatabaseGroupItem : public CDatabaseListViewItem
{
  Q_OBJECT

public:
  CDatabaseGroupItem(CDatabaseListView * parent, CMySQLServer *m, const QString &connection_name = QString::null);
  CDatabaseGroupItem(CDatabaseListViewItem * parent, CMySQLServer *m);
  void activated();
  void setOpen(bool b);
  void refreshWidget(bool b);

public slots:
  void processMenu(int);

private:
  void addDatabases(bool is_refresh);
  QPixmap newDatabaseIcon;
  QPixmap refreshIcon;
  QPixmap newWindowIcon;
  QDict<QString> widgetData;
  QPixmap openFolderIcon;
  QPixmap closedFolderIcon;
  void init();
  void initMenuItems();
};

#endif
