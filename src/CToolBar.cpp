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
#include "CToolBar.h"
#include "config.h"
//Added by qt3to4:
#include <Q3PopupMenu>
#include <QPixmap>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 4
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CToolBarButton::CToolBarButton(QWidget * parent, int v, Q3PopupMenu *p)
: QToolButton(parent, "my_ToolButton")
{
#ifdef DEBUG
  qDebug("CToolBarButton::CToolBarButton(QWidget *, int, QPopupMenu *)");
#endif

  val = v;
  if (p != 0)
  {
    setPopup (p);
    setPopupDelay(0);
  }
  connect(this, SIGNAL(clicked()), this, SLOT(Clicked()));
}


void CToolBarButton::Clicked()
{
#ifdef DEBUG
  qDebug("CToolBarButton::Clicked()");
#endif

  emit button_clicked(value());
}

CToolBar::CToolBar(Q3MainWindow * parent, const char * name)
: Q3ToolBar(parent, name)
{
#ifdef DEBUG
  qDebug("CToolBar::CToolBar(QMainWindow *, const char *)");
#endif

  m_type = -2;
  cnt = 0;
}

void CToolBar::clear()
{
#ifdef DEBUG
  qDebug("CToolBar::clear");
#endif

  cnt = 0;
  Q3ToolBar::clear();
  buttons_dict.clear();
}

void CToolBar::addButton(const QPixmap &pix, int val, Q3PopupMenu *p, bool ena, const QString &label)
{
#ifdef DEBUG
  qDebug("CToolBar::addButton(const QPixmap &, %d, QPopupMenu *, %s)", val, debug_string(booltostr(ena)));
#endif

  CToolBarButton * b = new CToolBarButton(this, val, p);
  b->setPixmap(pix);
  b->setEnabled(ena);
  if (!label.isEmpty())
    b->setTextLabel(label, true);
  connect(b, SIGNAL(button_clicked(int)), this, SLOT(button_clicked(int)));
  buttons_dict.insert(cnt++, b);
}

void CToolBar::button_clicked(int v)
{
#ifdef DEBUG
  qDebug("CToolBar::button_clicked(%d)", val);
#endif

  emit buttonClicked(v);
}

CToolBarButton * CToolBar::button(int idx) const
{
#ifdef DEBUG
  qDebug("CToolBar::button(%d)", idx);
#endif

  return buttons_dict.find(idx);
}
