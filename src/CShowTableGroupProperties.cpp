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
#include "CShowTableGroupProperties.h"
#include "config.h"
//Added by qt3to4:
#include <Q3PopupMenu>

CShowTableGroupProperties::CShowTableGroupProperties(QWidget * parent, const char *name)
: CTable(parent, name), current_item(0)
{
#ifdef DEBUG
  qDebug("CShowTableGroupProperties::CShowTableGroupProperties()");
#endif

  if (!name)
    setName("CShowTableGroupProperties");  
  verticalHeader()->hide();
  setReadOnly(true);
  setFocusStyle(Q3Table::FollowStyle);
  setSelectionMode(Q3Table::SingleRow);
  setLeftMargin(0);
}

CTableItem * CShowTableGroupProperties::findItem(const QString &str)
{  
#ifdef DEBUG
  qDebug("CDatabaseListViewItem::findItem()");
#endif

  if (current_item == 0)
    return 0;

  Q3ListViewItemIterator c(current_item->firstChild());
  for ( ; c.current(); ++c )
  {    
    if ((CTableGroupItem *)c.current()->parent() == current_item)
    {
      if (c.current()->text(0) == str)
        return (CTableItem *) c.current();      
    }
    else
      break;
  }
  return 0;
}

void CShowTableGroupProperties::DoubleClicked(int row, int, int button, const QPoint &)
{
#ifdef DEBUG
  qDebug("CShowTableGroupProperties::DoubleClicked()");
#endif

  if (currentItem() == 0)
      return;

  if (row > -1 && button == 1)
  {
    CTableItem * item = findItem(text(row, 0));
    if (item != 0)
      item->doubleClicked();
  }
};


void CShowTableGroupProperties::sortColumn(int col, bool ascending, bool wholeRows)
{
#ifdef DEBUG
  qDebug("CShowTableGroupProperties::sortColumn()");
#endif
  
  if (col == 1 || col == 2)
    mySortColumn(col, cmpIntTableItems, ascending, wholeRows);
  else  
    CTable::sortColumn(col, ascending, wholeRows);
}

void CShowTableGroupProperties::ContextMenuRequested(int row, int, const QPoint &pos)
{
#ifdef DEBUG
  qDebug("CShowTableGroupProperties::ContextMenuRequested()");
#endif

  if (row > -1)
  {
    CTableItem * item = findItem(text(row, 0));
    if (item == 0)
      return;
    Q3PopupMenu *m = new Q3PopupMenu();
    connect(m, SIGNAL(activated(int)), item, SLOT(processMenu(int)));
    item->createPopupMenu(m);
    m->removeItem(MENU_RENAME);
    m->removeItem(MENU_REFRESH);
    m->removeItemAt(m->count() - 1);
    m->exec(pos);
    delete m;
  }
}
