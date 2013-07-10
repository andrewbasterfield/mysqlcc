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
#ifndef CUSERADMINITEM_H
#define CUSERADMINITEM_H

#include <qvariant.h>
#include "CDatabaseListViewItem.h"
#include <qdict.h>

class CUserAdminItem : public CDatabaseListViewItem
{
  Q_OBJECT

public:
  CUserAdminItem(CDatabaseListViewItem * parent, CMySQLServer *m);
  CUserAdminItem(CDatabaseListView * parent, CMySQLServer *m, const QString &connection_name, bool connected);  
  void doubleClicked();
  void activated();
  bool isConnected() { return is_connected; }
  void refreshWidget(bool b);

public slots:
  void doCreateUser();
  void refresh();
  void processMenu(int res);

private:
  QPixmap userIcon;
  void initMenuItems();
  void setConnected(bool, bool refresh_toolbar = true);
  void init(bool c, bool refresh_toolbar);
  bool displayUsers();
  void printAccessError();
  bool is_connected;
  QDict<QString> widgetData;
};

#endif
