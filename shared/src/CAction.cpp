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
#include "CAction.h"
#include <stddef.h>
#include <qaction.h>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 4
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CAction::CAction(QObject * parent, const char * name, bool toggle)
:QAction(parent, name, toggle)
{
#ifdef DEBUG
  qDebug("CAction::CAction(QObject *, const char *, bool)");
#endif

  init();
}

CAction::CAction(const QString & text, const QIcon & icon, const QString & menuText, QKeySequence accel, QObject * parent, const char * name, bool toggle)
:QAction(text, icon, menuText, accel, parent, name, toggle)
{
#ifdef DEBUG
  qDebug("CAction::CAction(const QString &, const QIconSet &, const QString &, QKeySequence, QObject *, const char *, bool)");
#endif

  init();
}


CAction::CAction(const QString & text, const QString & menuText, QKeySequence accel, QObject * parent, const char * name, bool toggle)
:QAction(text, menuText, accel, parent, name, toggle)
{
#ifdef DEBUG
  qDebug("CAction::CAction(const QString &, const QString &, QKeySequence, QObject *, const char *, bool)");
#endif
  init();
}

void CAction::init()
{
#ifdef DEBUG
  qDebug("CAction::init()");
#endif

  parentmenutext = QString::null;
  tmpaccel = 0;
}
