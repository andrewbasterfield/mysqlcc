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
#ifndef CDATABASEITEM_H
#define CDATABASEITEM_H

#include "CDatabaseListViewItem.h"
#include <stddef.h>
#include <qdict.h>

class CDatabaseItem : public CDatabaseListViewItem
{
  Q_OBJECT

public:
  CDatabaseItem(CDatabaseListViewItem * parent, const QString &dbname, CMySQLServer *m, const QPixmap &p1 = 0, const QPixmap &p2 = 0);
  CDatabaseItem(CDatabaseListView * parent, const QString &dbname, bool isconnected, const QString &connection_name, CMySQLServer *m);
  void setConnected(bool b);
  bool isConnected() const { return is_connected; }
  void keyPressed(QKeyEvent * e);
  void doubleClicked();
  void activated();

public slots:
  void processMenu(int);

private:
  void refreshWidget(bool b);
  bool is_connected;
  void init(const QString &dbname);
  void initMenuItems();
  int widget_id;

  QPixmap databaseConnectIcon;
  QPixmap databaseDisconnectIcon;
  QPixmap databaseConnectedIcon;
  QPixmap databaseDisconnectedIcon;
  QPixmap sqlIcon;
  QPixmap deleteIcon;
  QPixmap newWindowIcon;
  QDict<QString> widgetData;
};

#endif
