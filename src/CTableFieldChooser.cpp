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
#include "CTableFieldChooser.h"
#include "config.h"
#include "globals.h"
#include <stddef.h>  
#include <q3table.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QKeyEvent>

CTableFieldChooserListItem::CTableFieldChooserListItem(Q3ListView * parent, const QString & text, int columnid)
: Q3CheckListItem(parent, text, CheckBox)
{
  c = columnid;
}

void CTableFieldChooserListItem::stateChange(bool s)
{  
  ((CTableFieldChooserListView *)listView())->checkBoxClicked(col(), s);
}

void CTableFieldChooserListItem::keyPressed(QKeyEvent *e)
{
#ifdef DEBUG
  qDebug("CTableFieldChooserListItem::keyPressed()");
#endif

  if (e->key() == Qt::Key_Shift)
  {
    QPoint p(listView()->itemRect(this).bottomLeft());
    p.setY(p.y() + listView()->itemRect(this).bottom() - listView()->itemRect(this).top() + 7);
    p.setX(p.x() + 20);
    ((CTableFieldChooserListView *) listView())->displayMenu(this, listView()->mapToGlobal(p) , 0);
  }
}

CTableFieldChooserListView::CTableFieldChooserListView(CTableFieldChooser * parent, const char * name, Qt::WFlags f)
:Q3ListView(parent, name, f)
{
  connect(this, SIGNAL(contextMenuRequested(Q3ListViewItem *, const QPoint &, int)), this, SLOT(displayMenu(Q3ListViewItem *, const QPoint &, int)));
}

void CTableFieldChooserListView::displayMenu( Q3ListViewItem *, const QPoint &pos, int )
{
  Q3PopupMenu *menu = new Q3PopupMenu();
  menu->insertItem(getPixmapIcon("checkedIcon"), tr("Check All"), 1);
  menu->insertItem(getPixmapIcon("uncheckedIcon"), tr("Clear All"), 2);
  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("refreshTablesIcon"), "Refresh", MENU_REFRESH);
  int res = menu->exec(pos);
  delete menu;
  bool b = false;
  switch (res)
  {
  case 1: b = true;
    break;
  case 2: b = false;
    break;
  case MENU_REFRESH:
    refresh();
    break;
  }

  if (res != MENU_REFRESH)
  {
    Q3ListViewItemIterator it(this);
    for ( ; it.current(); ++it)
      ((CTableFieldChooserListItem *)it.current())->setOn(b);
  }  
}

Q3Table * CTableFieldChooserListView::table() const
{ 
  return ((CTableFieldChooser *) parent())->table();
}

void CTableFieldChooserListView::keyPressEvent(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CTableFieldChooserListView::keyPressEvent()");
#endif

  if (currentItem() != 0)
      ((CTableFieldChooserListItem *) currentItem())->keyPressed(e);

  Q3ListView::keyPressEvent(e);
}

void CTableFieldChooserListView::checkBoxClicked(int c, bool s)
{
  if (!table())
    return;

  if (s)
    table()->showColumn(c);
  else
    table()->hideColumn(c);
  table()->updateScrollBars();
}

void CTableFieldChooserListView::refresh()
{
  clear();
  if (table())
  {
    for (int i = 0; i < table()->numCols(); i++)
    {
      CTableFieldChooserListItem *x = new CTableFieldChooserListItem(this, table()->horizontalHeader()->label(i), i);
      x->setOn(table()->horizontalHeader()->sectionSize(i) != 0);
    }
  }
}


CTableFieldChooser::CTableFieldChooser(QWidget *parent, Q3Table * t, const char * name, Place p, Qt::WFlags f)
:Q3DockWindow(p, parent, name, f), tbl(t)
{
  init();
}

CTableFieldChooser::CTableFieldChooser(QWidget *parent, const char * name, Place p, Qt::WFlags f)
:Q3DockWindow(p, parent, name, f), tbl(0)
{
  init();
}

void CTableFieldChooser::init()
{
  setMovingEnabled (true);
  setResizeEnabled (true);
  setHorizontallyStretchable (true);
  setVerticallyStretchable (true);
  setOpaqueMoving (false);
  setCloseMode(Q3DockWindow::Always);

  setCaption(tr("Table Columns" ));

  columnsListView = new CTableFieldChooserListView(this, "Columns");
  columnsListView->addColumn(tr("Columns"));
  columnsListView->header()->setResizeEnabled(false, columnsListView->header()->count() - 1);
  columnsListView->setFrameShape(Q3ListView::Box);
  columnsListView->setLineWidth(1);
  columnsListView->setShowSortIndicator(true);
  columnsListView->setResizeMode(Q3ListView::AllColumns);

  Q3WhatsThis::add(columnsListView, tr("Select the columns that you want visible in the grid."));
  setWidget(columnsListView);
  is_first = true;
}

void CTableFieldChooser::show()
{
  if (is_first)
    move(QCursor::pos().x(), QCursor::pos().y());
  is_first = false;
  columnsListView->refresh();
  Q3DockWindow::show();
}

void CTableFieldChooser::refresh()
{
  columnsListView->refresh();
}

void CTableFieldChooser::setCaption(const QString &s)
{
  if (Q3DockWindow::caption() == s)
    return;

  if (myApp()->isMDI())
    Q3DockWindow::setCaption(s);
  else
  {
    QString app_name(SHORT_NAME);
    app_name += " - ";
    if (s.startsWith(app_name))
      Q3DockWindow::setCaption(s);
    else
      Q3DockWindow::setCaption(app_name + s);
  }
}
