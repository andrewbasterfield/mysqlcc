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
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stddef.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include "shared.h"
#include "CApplication.h"
#include "CPlugin.h"

class CMyWindow;
class CMessagePanel;
class QStringList;

extern void myShowWindow(CMyWindow *wnd);
extern bool loadSyntaxSection(QStringList *list, int section, bool lower_case=false, QString syntaxFile=QString::null);
extern CApplication * myApp();
extern const QPixmap getPixmapIcon(const QString &key);
extern void saveToFile(QString &fileName, const QString &default_ext, const QString &ext_desc, const char * contents, CMessagePanel *m=0, bool writeBinary = false, uint len=0);
extern PluginList myPluginsList(Plugin::mysqlcc_plugin_place place);
#endif
