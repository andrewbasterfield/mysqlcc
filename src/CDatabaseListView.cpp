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
#include "CDatabaseListView.h"
#include "CDatabaseListViewItem.h"
#include "CConsoleWindow.h"
#include "CServerItem.h"
#include "CDatabaseGroupItem.h"
#include "CDatabaseItem.h"
#include "CTableGroupItem.h"
#include "CServerAdministrationItem.h"
#include "CUserAdminItem.h"
#include "CConfig.h"
#include "panels.h"
#include "globals.h"
#include "config.h"
#include "CMySQLServer.h"
#include <stddef.h>  
#include <qstatusbar.h>


static const QString connections_path = QString(CONNECTIONS_PATH);

CDatabaseListView::CDatabaseListView(CConsoleWindow * c, QWidget * parent, const char * name, WFlags f)
: QListView(parent, name, f)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::CDatabaseListView()");
#endif

  consolewindow = c;
  addColumn(trUtf8("MySQL Servers"));  
  setRootIsDecorated(false);
  setResizeMode(QListView::AllColumns);
  setColumnWidth(0, 150);
  setMargin(0);
  setShowSortIndicator(true);
  isbusy = false;
  init();
}


void CDatabaseListView::setBusy(bool b)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::setBusy(%s)", debug_string(booltostr(b)));
#endif

  isbusy = b;
  consoleWindow()->setCursor(b ? Qt::WaitCursor : Qt::ArrowCursor);
}

void CDatabaseListView::refreshServers()
{
#ifdef DEBUG
  qDebug("CDatabaseListView::refreshServers()");
#endif

  if (isBusy())
    return;

  setBusy(true);  
  QPixmap p1 = getPixmapIcon("serverDisconnectedIcon");
  QPixmap p2 = getPixmapIcon("serverConnectedIcon");
  QStringList dbServerList;
  CConfig::list(dbServerList, connections_path);
  if (childCount() > 0)
  {
    QListViewItemIterator it(this);
    while (it.current() != 0)
    {
      if (((CDatabaseListViewItem *) it.current())->type() == CDatabaseListViewItem::SERVER)
      {
        CServerItem *i = (CServerItem *) it.current();
        ++it;
        if (dbServerList.find(i->text(0)) == dbServerList.end()) //item was deleted from hdd
        {
          i->setConnected(false);
          i->setVisible(false);
          delete i;
        }
      }
      else
        ++it;
    }
  }

  if (!dbServerList.isEmpty() && consoleWindow()->isApplicationWindow())
  {
    bool found = false;
    for (QStringList::Iterator j = dbServerList.begin(); j != dbServerList.end(); j++)
    {
      found = false;     
      QListViewItemIterator it(this);
      while (it.current() != 0)
      {
        if (((CDatabaseListViewItem *) it.current())->type() == CDatabaseListViewItem::SERVER)
          if (it.current()->text(0) == *j)
          {
            found = true;
            break;
          }      
        ++it;
      }
      if (!found)
        new CServerItem(this, *j, 0, p1, p2);
    }
  }
  setBusy(false);
  updateListView();
}

void CDatabaseListView::processCurrentItemMenu(int cmd)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::processCurrentItemMenu(%d)", cmd);
#endif

  if (isBusy())
    return;

  CDatabaseListViewItem *i = (CDatabaseListViewItem *) currentItem();
  if (i != 0)
    i->processMenu(cmd);
}

void CDatabaseListView::updateListView()
{
#ifdef DEBUG
  qDebug("CDatabaseListView::updateListView()");
#endif

  setBusy(true);
  if (childCount() > 0)
  {
    QListViewItem *i = currentItem();
    if (i == 0)
      i = firstChild();
    if (i != 0)
    {
      setCurrentItem(i);
      setSelected(currentItem(), true);
      setBusy(false);
      CurrentChanged(currentItem());
      setBusy(true);
      consoleWindow()->itemToolBar()->show();
      consoleWindow()->actionMenuAboutToShow();
    }
    else  //should never get here
    {
      consoleWindow()->itemToolBar()->hide();
      consoleWindow()->enableActionMenu(false);
    }
    emitEnableQueryButton((CDatabaseListViewItem *) i);
  }
  else
  {
    consoleWindow()->itemToolBar()->hide();
    consoleWindow()->enableActionMenu(false);
    widgetStack()->raiseWidget((int) CDatabaseListViewItem::NONE);
    emitEnableQueryButton(0);
  }
  setBusy(false);
}

void CDatabaseListView::getActionMenu(QPopupMenu *m)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::getActionMenu()");
#endif

  CDatabaseListViewItem *i = (CDatabaseListViewItem *) currentItem();
  if (i != 0)
  {
    i->createPopupMenu(m);
    consoleWindow()->enableActionMenu(m->count() > 0);
  }
  else
    consoleWindow()->enableActionMenu(false);
}


CMessagePanel * CDatabaseListView::messagePanel() const
{
#ifdef DEBUG
  qDebug("CDatabaseListView::messagePanel()");
#endif

  return consoleWindow()->messagePanel();  
}

QWidgetStack * CDatabaseListView::widgetStack() const
{
#ifdef DEBUG
  qDebug("CDatabaseListView::widgetStack()");
#endif

  return consoleWindow()->widgetStack();
}


void CDatabaseListView::ContextMenuRequested(QListViewItem *item, const QPoint & pos, int)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::ContextMenuRequested()");
#endif

  if (isBusy())
    return;
  else
    if (item == 0)  //No item was clicked
    {
      QPopupMenu *menu = new QPopupMenu();
      if (consoleWindow()->isApplicationWindow())
      {
        menu->insertItem(getPixmapIcon("registerServerIcon"), tr("New"), MENU_NEW);
        menu->insertItem(getPixmapIcon("refreshIcon"), tr("Refresh"), MENU_REFRESH);
        menu->insertSeparator();
      }
      menu->insertItem(tr("Hide"), MENU_HIDE);
      switch (menu->exec(pos))
      {
      case MENU_NEW: consoleWindow()->registerServer();
        break;

      case MENU_REFRESH: refreshServers();
        break;

      case MENU_HIDE: consoleWindow()->treePanelWindow()->hide();
        break;
      }
      delete menu;
    }
    else
      ((CDatabaseListViewItem *) item)->displayMenu(pos);
}

void CDatabaseListView::DoubleClicked(QListViewItem *item)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::DoubleClicked()");
#endif

  if(item == 0 || isBusy())
    return;

  setBusy(true);
  ((CDatabaseListViewItem *) item)->doubleClicked();
  setBusy(false);
}

void CDatabaseListView::Expanded(QListViewItem *item)
{  
#ifdef DEBUG
  qDebug("CDatabaseListView::Expanded()");
#endif

  if(item == 0)
    return;

  ((CDatabaseListViewItem *) item)->expanded();
}

void CDatabaseListView::Collapsed(QListViewItem *item)
{  
#ifdef DEBUG
  qDebug("CDatabaseListView::Collapsed()");
#endif

  if(item == 0)
    return;
  ((CDatabaseListViewItem *) item)->collapsed();
}

void CDatabaseListView::ItemRenamed(QListViewItem * item, int, const QString & text)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::ItemRenamed(QListViewItem *, int, '%s')", debug_string(text));
#endif

  if(item == 0)
    return;
  ((CDatabaseListViewItem *) item)->renamed(text);
}

void CDatabaseListView::setItemRoot(CDatabaseListViewItem *item)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::setItemRoot()");
#endif

  setBusy(true);
  QListViewItem * new_item = 0;
  CMySQLServer *m = item->mysql()->oneConnection() ? item->mysql() : 0;
  switch(item->type())
  {  
    case CDatabaseListViewItem::SERVER:
    {
      CServerItem *i = new CServerItem(this, item->text(0), m);
      i->setConnected(item->mysql()->isConnected());
      new_item = i;
    }
    break;

    case CDatabaseListViewItem::DATABASE_GROUP:
    {
      CDatabaseGroupItem *i = new CDatabaseGroupItem(this, m, item->mysql()->connectionName());
      new_item = i;
    }
    break;

    case CDatabaseListViewItem::DATABASE:
    {
      CDatabaseItem *i = new CDatabaseItem(this, item->text(0), ((CDatabaseItem *) item)->isConnected(), item->mysql()->connectionName(), m);
      new_item = i;
    }
    break;

    case CDatabaseListViewItem::TABLE_GROUP:
    {
      CTableGroupItem *i = new CTableGroupItem(this, ((CTableGroupItem *) item)->databaseName(), item->mysql()->connectionName(), m);
      new_item = i;
    }
    break;

    case CDatabaseListViewItem::SERVER_ADMINISTRATION:
    {
      CServerAdministrationItem *i = new CServerAdministrationItem(this, item->mysql()->connectionName(), m);
      new_item = i;
    }
    break;

    case CDatabaseListViewItem::USER_ADMINISTRATION:
    {      
      CUserAdminItem *i = new CUserAdminItem(this, m, item->mysql()->connectionName(), ((CUserAdminItem *)item)->isConnected());      
      new_item = i;
    }
    break;

    default:
    break;
  }
  setBusy(false);
  consoleWindow()->databaseListView()->setCurrentItem(new_item);
  consoleWindow()->databaseListView()->updateListView();
}

void CDatabaseListView::openInNewWindow(CDatabaseListViewItem *item)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::openInNewWindow()");
#endif

  if(item == 0 || item->isBlocked())
    return;

  item->setBlocked(true);
  CConsoleWindow * c = new CConsoleWindow(myApp()->workSpace());
  QString t = item->text(0);
  c->setCaption("[" + item->mysql()->connectionName() + "] " + t);
  if (item->mysql() != 0)
    c->statusBar()->message("[" + item->mysql()->connectionName() + "] " +      
      item->mysql()->connectionName() == t ? QString::null : t);
  c->databaseListView()->setColumnText(0, t);
  c->databaseListView()->setItemRoot(item);
  c->myResize(600,400);
  myShowWindow(c);
  c->setFocus();
  c->raise();
  item->setBlocked(false);
}

void CDatabaseListView::ReturnPressed(QListViewItem *item)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::ReturnPressed()");
#endif

  DoubleClicked(item);
}

void CDatabaseListView::SpacePressed(QListViewItem *item)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::SpacePressed()");
#endif

  if (item == 0 || isBusy())  
    return;
  item->startRename(0);
}

void CDatabaseListView::keyPressEvent(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CDatabaseListView::keyPressEvent()");
#endif

  if (!isBusy() && currentItem() != 0)
      ((CDatabaseListViewItem *) currentItem())->keyPressed(e);

  QListView::keyPressEvent(e);
}

void CDatabaseListView::emitEnableQueryButton(CDatabaseListViewItem *i)
{
#ifdef QDEBUG
  qDebug("CDatabaseListView::emitEnableQueryButton(CDatabaseListViewItem *)");
#endif

  emit itemSelected(i);
}

void CDatabaseListView::CurrentChanged(QListViewItem *item)
{
#ifdef QDEBUG
  qDebug("CDatabaseListView::CurrentChanged()");
#endif

  emitEnableQueryButton((CDatabaseListViewItem *) item);

  if (item == 0 || !currentItem())
    return;
  ((CDatabaseListViewItem *) currentItem())->createWindowMenu(consoleWindow()->itemToolBar());
  consoleWindow()->actionMenuAboutToShow();
  if (isBusy() || !item || !currentItem())
    return;
  setBusy(true);
  //((CDatabaseListViewItem *) item)->activated();
  ((CDatabaseListViewItem *) currentItem())->activated();
  setBusy(false);
}

void CDatabaseListView::init()
{
#ifdef DEBUG
  qDebug("CDatabaseListView::init()");
#endif

  connect(this, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)), this, SLOT(ContextMenuRequested(QListViewItem *, const QPoint &, int)));
  connect(this, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(DoubleClicked(QListViewItem *)));
  connect(this, SIGNAL(expanded(QListViewItem *)), this, SLOT(Expanded(QListViewItem *)));
  connect(this, SIGNAL(collapsed(QListViewItem *)), this, SLOT(Collapsed(QListViewItem *)));
  connect(this, SIGNAL(itemRenamed(QListViewItem *, int, const QString &)), this, SLOT(ItemRenamed(QListViewItem *, int, const QString &)));
  connect(this, SIGNAL(returnPressed(QListViewItem *)), this, SLOT(ReturnPressed(QListViewItem *)));
  connect(this, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(SpacePressed(QListViewItem *)));
  connect(this, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(CurrentChanged(QListViewItem *)));
}
