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
#ifndef CUSERITEM_H
#define CUSERITEM_H

#include <stddef.h>
#include <qvariant.h>
#include "CDatabaseListViewItem.h"
#include <qdict.h>

class CUserItem : public CDatabaseListViewItem
{
  Q_OBJECT

public:
  CUserItem(CDatabaseListViewItem * parent, CMySQLServer *m, const QString &u, const QString &h, const QPixmap &p);
  void keyPressed(QKeyEvent * e);
  void processMenu(int res);
  void doubleClicked();
  void activated();
  QString userName() const { return user_name; }
  QString hostName() const { return host_name; }

private:
  void initMenuItems();
  QString user_name;
  QString host_name;
};

#endif
