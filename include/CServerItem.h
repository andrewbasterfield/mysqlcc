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
#ifndef CSERVERITEM_H
#define CSERVERITEM_H

#include "CDatabaseListViewItem.h"
#include <stddef.h>
#include <q3dict.h>
#include <qtoolbutton.h>
#include <q3toolbar.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QPixmap>

class CServerItem : public CDatabaseListViewItem
{
  Q_OBJECT

public:
  CServerItem(CDatabaseListView * parent, const QString &server_name, CMySQLServer *m, const QPixmap &p1=0, const QPixmap &p2=0);
  void doubleClicked();
  void activated();
  void renamed(const QString &);
  void setConnected(bool b);
  void keyPressed(QKeyEvent * e);

public slots:
  void processMenu(int);

private slots:
  void connectionEdited(const QString &s);

private:
  void initMenuItems();
  QPixmap serverDisconnectedIcon;
  QPixmap serverConnectedIcon;
  QPixmap sqlIcon;
  QPixmap editIcon;
  QPixmap deleteIcon;
  QPixmap newWindowIcon;
  void refreshWidget(bool b);
  Q3Dict<QString> widgetData;
};

#endif
