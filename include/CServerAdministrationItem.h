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
#ifndef CSERVERADMINISTRATIONITEM_H
#define CSERVERADMINISTRATIONITEM_H

#include "CDatabaseListViewItem.h"
#include <stddef.h>
#include <q3dict.h>
//Added by qt3to4:
#include <Q3PopupMenu>

class CServerAdministrationItem : public CDatabaseListViewItem
{
  Q_OBJECT

public:
  CServerAdministrationItem(CDatabaseListViewItem * parent, CMySQLServer *m);
  CServerAdministrationItem(CDatabaseListView * parent, const QString &connection_name, CMySQLServer *m); 
  ~CServerAdministrationItem();
  void doubleClicked();
  void activated();
  
public slots:
  void processMenu(int);

private:
  void init();
  void initMenuItems();
  void refreshWidget(bool b);
  Q3PopupMenu * flush_menu;
  Q3Dict<QString> widgetData;
};

#endif
