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
#include "CDatabaseListViewItem.h"
#include "CDatabaseListView.h"
#include "CMySQLServer.h"
#include "CConsoleWindow.h"
#include "CAction.h"
#include "panels.h"
#include "globals.h"
#include <stddef.h>  
#include <q3listview.h>
#include <q3widgetstack.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PopupMenu>
#include <QKeyEvent>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 3
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CDatabaseListViewItemMenu::CDatabaseListViewItemMenu()
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::CDatabaseListViewMenu()");
#endif

  item_list.setAutoDelete(true);
  cnt = 0;
}

CDatabaseListViewItemMenu::~CDatabaseListViewItemMenu()
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::~CDatabaseListViewItemMenu()");
#endif

  clear();
}

void CDatabaseListViewItemMenu::Clicked(int i)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::Clicked(%d)", i);
#endif

  emit clicked(i);
}

static QString strip_amp(const QString &str)
{
  return charReplace(str, '&', "");
}

void CDatabaseListViewItemMenu::createToolBar(CToolBar *t, int type)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::createToolBar()");
#endif

  bool istype = false;
  if (t->type() != type)
  {
    t->setType(type);
    t->clear();
    istype = true;
  }
  my_item *i;
  int n = 0;
  for (i = item_list.first(); i; i = item_list.next() )
  {
    if (istype)
    {
      if (!i->pixmap().isNull() && i->value() != -1)
        t->addButton(i->pixmap(), i->value(), i->popup(), i->enabled(), strip_amp(i->label()));
      else
        if (i->isSeparator())
          t->addSeparator();
    }
    else
      if (!i->pixmap().isNull() && i->value() != -1)
      {
        CToolBarButton *b = t->button(n++);
        if (!i->label().isEmpty())
          b->setTextLabel(strip_amp(i->label()), true);
        if (i->value() != b->value())
        {
          b->setPixmap(i->pixmap());
          b->setValue(i->value());
          b->setPopup(i->popup());
        }
        b->setEnabled(i->enabled());
      }
  }
}

void CDatabaseListViewItemMenu::createPopupMenu(Q3PopupMenu *p)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::createPopupMenu()");
#endif

  p->clear();
  my_item *i;
  for (i = item_list.first(); i; i = item_list.next() )
  {
    if (i->value() != -1 && i->popup() == 0)
    {
      if (!i->pixmap().isNull())
        p->insertItem(i->pixmap(), i->label(), i->value());
      else
        p->insertItem(i->label(), i->value());
      p->setItemEnabled(i->value(), i->enabled());
    }
    else
      if (i->popup() != 0)
      {
        p->insertItem(i->pixmap(), i->label(), i->popup());
        p->setItemEnabled(i->value(), i->enabled());
      }
      else
        if (i->isSeparator())
          p->insertSeparator();
  }
}

void CDatabaseListViewItemMenu::clear()
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::clear()");
#endif

  cnt = 0;
  item_list.clear();
}

void CDatabaseListViewItemMenu::insertItem(const QPixmap &pixmap, const QString &label, Q3PopupMenu * popup, int value)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::insertItem(const QPixmap &, '%s', QPopupMenu *, %d)", debug_string(label), value);
#endif

  my_item *i = new my_item(pixmap, label, value, false);
  i->setPopupMenu(popup);
  item_list.insert(cnt++, i);
}

void CDatabaseListViewItemMenu::insertItem(const QPixmap &pixmap, const QString &label, int value)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::insertItem(const QPixmap &, '%s', %d)", debug_string(label), value);
#endif

  my_item *i = new my_item(pixmap, label, value, false);
  item_list.insert(cnt++, i);
}

void CDatabaseListViewItemMenu::insertItem(const QString &label, int value)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::insertItem('%s', %d)",debug_string(label), value);
#endif

  my_item *i = new my_item(0, label, value, false);
  item_list.insert(cnt++, i);
}

void CDatabaseListViewItemMenu::insertSeparator()
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::insertSeparator()");
#endif

  my_item *i = new my_item(0, QString::null, -1, true);
  item_list.insert(cnt++, i);
}

void CDatabaseListViewItemMenu::setItemEnabled(int idx, bool e)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItemMenu::setItemEnabled(%d, %s)", idx, debug_string(booltostr(e)));
#endif

  my_item *i = item(idx);
  if (i != 0)
    i->setEnabled(e);
}


CDatabaseListViewItem::CDatabaseListViewItem(CDatabaseListView * parent, CMySQLServer *m, item_type t, const char *name)
: QObject(parent, name), Q3ListViewItem(parent), m_mysql(m), m_type(t)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::CDatabaseListViewItem(CDatabaseListView *, CMySQLServer *, item_type, const char *)");
#endif

  init();
}

CDatabaseListViewItem::CDatabaseListViewItem(CDatabaseListViewItem * parent, CMySQLServer *m, item_type t, const char *name)
: QObject(parent, name), Q3ListViewItem(parent), m_mysql(m), m_type(t)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::CDatabaseListViewItem(CDatabaseListViewItem *, CMySQLServer *, item_type, const char *)");
#endif

  init();
}


CDatabaseListViewItem::~CDatabaseListViewItem()
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::~CDatabaseListViewItem()");
#endif

  if (deletemysql)
    delete m_mysql;
}

void CDatabaseListViewItem::refresh()
{
  listView()->setCurrentItem((Q3ListViewItem *) this);
  listView()->setSelected((Q3ListViewItem *) this, true);
  refreshWidget(true);
}

void CDatabaseListViewItem::init()
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::init()");
#endif

  blocked = false;
  m_dbname = QString::null;
  m_table_name = QString::null;
  connect(menu(), SIGNAL(clicked(int)), this, SLOT(processMenu(int)));
  deletemysql = false;
  if (!name())
    setName("CDatabaseListViewItem");
}

void CDatabaseListViewItem::setBlocked(bool b)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::setBlocked(%s)", debug_string(booltostr(b)));
#endif

  consoleWindow()->setCursor(b ? Qt::WaitCursor : Qt::ArrowCursor);
  blocked = b;
  qApp->processEvents();
}

void CDatabaseListViewItem::okRename(int col)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::okRename(%d)", col);
#endif

  old_text = text(0);
  Q3ListViewItem::okRename(col);
}

CConsoleWindow * CDatabaseListViewItem::consoleWindow() const
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::consoleWindow()");
#endif

  return databaseListView()->consoleWindow();
}

CMessagePanel * CDatabaseListViewItem::messagePanel() const
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::messagePanel()");
#endif

  return databaseListView()->messagePanel();
}

Q3WidgetStack * CDatabaseListViewItem::widgetStack() const
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::widgetStack()");
#endif

  return databaseListView()->widgetStack();
}

CDatabaseListView * CDatabaseListViewItem::databaseListView() const
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::databaseListView()");
#endif

  return (CDatabaseListView *)listView();
}

bool CDatabaseListViewItem::isBusy()
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::isBusy()");
#endif

  return databaseListView()->isBusy();
}

void CDatabaseListViewItem::keyPressed(QKeyEvent *e)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::keyPressed()");
#endif

  if (e->key() == Qt::Key_Shift)
  {
    QPoint p(listView()->itemRect(this).bottomLeft());
    p.setY(p.y() + listView()->itemRect(this).bottom() - listView()->itemRect(this).top() + 7);
    p.setX(p.x() + 20);
    databaseListView()->ContextMenuRequested(this, databaseListView()->mapToGlobal(p) , 0);
  }
}

void CDatabaseListViewItem::createWindowMenu(CToolBar *t)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::createWindowMenu()");
#endif

  initMenuItems();
  menu()->createToolBar(t, int_type());
  qApp->processEvents();
}

void CDatabaseListViewItem::createPopupMenu(Q3PopupMenu *m)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::createPopupMenu()");
#endif

  initMenuItems();
  menu()->createPopupMenu(m);
  qApp->processEvents();
}

int CDatabaseListViewItem::displayMenu(const QPoint &pos)
{
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::displayMenu()");
#endif

  Q3PopupMenu *m = new Q3PopupMenu();
  connect(m, SIGNAL(activated(int)), this, SLOT(processMenu(int)));
  createPopupMenu(m);
  int res = m->exec(pos);
  delete m;
  return res;
}

void CDatabaseListViewItem::deleteChilds()
{  
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::deleteChilds()");
#endif

  Q3ListViewItemIterator c( firstChild() );
  for ( ; c.current(); ++c )
  {    
    if (c.current()->parent() != Q3ListViewItem::parent())
    {   
      Q3ListViewItem *p = c.current();     
      takeItem(p);      
      delete(p);
      p = NULL;
      if (!this->nextSibling() && ((CDatabaseListViewItem *)this)->type() != SERVER)
        break;
    }
    else
      break;
  }
}
