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
#ifndef CHISTORYVIEW_H
#define CHISTORYVIEW_H

#include <stddef.h>
#include <qvariant.h>
#include <qintdict.h>

#include "panels.h"

class CHistoryView
{
public:
  CHistoryView();
  ~CHistoryView();
  void load();
  void loadTo(CHistoryPanel *h);
  void write();
  void truncate();
  bool append(const QString &);
  static QString historyFile();
  static uint historySize();
  static void setHistorySize(uint size);  
  static CHistoryView * historyView() { return &history_view; }
  
private:
  QStringList historyList;
  static CHistoryView history_view;
  static uint history_size;
};

#endif
