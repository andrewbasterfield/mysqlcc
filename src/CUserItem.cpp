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
#include "CUserItem.h"
#include "CShowDatabaseGroupProperties.h"
#include "CMySQLServer.h"
#include "globals.h"
#include "config.h"
#include "panels.h"
#include "CUserAdminItem.h"
#include "CUserAdminWindow.h"
//Added by qt3to4:
#include <QPixmap>
#include <QKeyEvent>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 1
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CUserItem::CUserItem(CDatabaseListViewItem * parent, CMySQLServer *m, const QString &username, const QString &hostname, const QPixmap &p)
: CDatabaseListViewItem(parent, m, USER, "CUserItem"), user_name(username), host_name(hostname)
{
#ifdef DEBUG
  qDebug("CUserItem::CUserItem()");
#endif

  setText(0, user_name + "@" + host_name);
  setPixmap(0, p);  
}

void CUserItem::activated()
{
#ifdef DEBUG
  qDebug("CUserItem::activated()");
#endif

  if (isBlocked())
    return;

  ((CUserAdminItem *) Q3ListViewItem::parent())->refreshWidget(false);
}

void CUserItem::processMenu(int res)
{
#ifdef DEBUG
  qDebug("CUserItem::processMenu(%d)", res);
#endif

  switch (res)
  {
    case MENU_DELETE:      
    {
      if (CUserAdminWindow::removeUser(mysql(), user_name, host_name))
      {
        setVisible(false);
        delete this;
      }
    }
    break;

    case MENU_NEW:
    {
      CUserAdminWindow *p = new CUserAdminWindow(myApp()->workSpace(), mysql(), QString::null, QString::null, false);
      connect(p, SIGNAL(do_refresh()), (CUserAdminItem *) Q3ListViewItem::parent(), SLOT(refresh()));
      myShowWindow(p);
    }
    break;

  case MENU_EDIT:
    {
      bool notnew = mysql()->mysql()->version().major <= 3 && mysql()->mysql()->version().minor <= 22;        
      if (notnew)
        messagePanel()->warning(tr("Edit User only works for mysql >= 3.23"));        
      else
      {
        CUserAdminWindow *p = new CUserAdminWindow(myApp()->workSpace(), mysql(), user_name, host_name, true);
        connect(p, SIGNAL(do_refresh()), (CUserAdminItem *) Q3ListViewItem::parent(), SLOT(refresh()));
        myShowWindow(p);
      }
    }
    break;
  }
}

void CUserItem::doubleClicked()
{
#ifdef DEBUG
  qDebug("CUserItem::doubleClicked()");
#endif

  processMenu(MENU_EDIT);
}

void CUserItem::keyPressed(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CUserItem::keyPressed()");
#endif

  if (isBlocked())
    return;

  if (e->key() == Qt::Key_Delete)
    processMenu(MENU_DELETE);
  else
    CDatabaseListViewItem::keyPressed(e);
}

void CUserItem::initMenuItems()
{
#ifdef DEBUG
  qDebug("CUserItem::initMenuItems()");
#endif

  menu()->clear();  
  menu()->insertItem(getPixmapIcon("editUserIcon"), tr("&Edit User"), MENU_EDIT);
  menu()->insertSeparator();
  menu()->insertItem(getPixmapIcon("deleteUserIcon"), tr("&Delete User"), MENU_DELETE);
  menu()->insertSeparator();
  menu()->insertItem(getPixmapIcon("newUserIcon"), tr("&New User"), MENU_NEW);  
}
