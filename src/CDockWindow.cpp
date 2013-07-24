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
#include "CDockWindow.h"
#include "config.h"
#include "globals.h"
//Added by qt3to4:
#include <QCloseEvent>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 2
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CDockWindow::CDockWindow(Place p, QWidget * parent, const char * name, Qt::WFlags f)
: Q3DockWindow(p, parent, name, f)
{
#ifdef DEBUG
  qDebug("CDockWindow::CDockWindow()");
#endif
  
  if (!name)
    setName("CDockWindow");
  setMovingEnabled (true);
  setResizeEnabled (true);
  setHorizontallyStretchable (true);
  setVerticallyStretchable (true);
  setOpaqueMoving (false);
  setCloseMode(Always);    
  setNewLine(true);
}  

void CDockWindow::closeEvent(QCloseEvent *e)
{
#ifdef DEBUG
  qDebug("CDockWindow::closeEvent()");
#endif

  e->ignore();
  hide();
}

void CDockWindow::setCaption(const QString &s)
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

