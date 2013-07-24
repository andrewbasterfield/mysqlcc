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
#include "CShowTableProperties.h"
#include "globals.h"
#include "config.h"
#include "CTableItem.h"
//Added by qt3to4:
#include <Q3PopupMenu>

CShowTableProperties::CShowTableProperties(QWidget * parent, const char *name)
: CTable(parent, name)
{
#ifdef DEBUG
  qDebug("CShowTableProperties::CShowTableProperties()");
#endif

  if (!name)
    setName("CShowTableProperties");

  verticalHeader()->hide();
  setReadOnly(true);
  setFocusStyle(Q3Table::FollowStyle);
  setSelectionMode(Q3Table::SingleRow);
  setLeftMargin(0);
  current_item = 0;  
}

void CShowTableProperties::DoubleClicked(int row, int, int button, const QPoint &)
{
#ifdef DEBUG
  qDebug("CShowTableProperties::DoubleClicked()");
#endif

  if (row > -1 && button == 1 && current_item != 0)
    current_item->doubleClicked();
};


void CShowTableProperties::ContextMenuRequested(int, int, const QPoint &pos)
{
#ifdef DEBUG
  qDebug("CShowTableProperties::ContextMenuRequested()");
#endif

  if (current_item != 0)
  {
    Q3PopupMenu *m = new Q3PopupMenu();
    m->insertItem(getPixmapIcon("designIcon"), tr("&Edit Table"), MENU_EDIT);
    m->insertSeparator();
    m->insertItem(getPixmapIcon("refreshTablesIcon"), tr("&Refresh Fields"), MENU_REFRESH);
    connect(m, SIGNAL(activated(int)), current_item, SLOT(processMenu(int)));
    m->exec(pos);
    delete m;
  }
}
