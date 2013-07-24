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
#ifndef CFIELDEDITOR_H
#define CFIELDEDITOR_H

#include "CMyWindow.h"
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3GridLayout>
#include <QCloseEvent>
#include <Q3HBoxLayout>

class CDefaultTableItem;
class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class Q3Frame;
class QPushButton;
class CFieldEditorWidget;

class CFieldEditorWindow : public CMyWindow
{ 
  Q_OBJECT
    
public:
  CFieldEditorWindow(CFieldEditorWidget *editor, const char* name = 0);
  ~CFieldEditorWindow();
  bool closeOnApply() { return close_on_apply; }

public slots:
  void setCloseOnApply(bool b) { close_on_apply = b; }
  void addHotKeyEditorMenu();

signals:
  void apply_clicked();
  void cancel_clicked();
  void close_event(QCloseEvent *);

private slots:
  void applyButtonClicked();
  void cancelButtonClicked();

private:
  void closeEvent(QCloseEvent * e);
  Q3GridLayout* CFieldEditorWindowLayout;
  Q3HBoxLayout* Frame5Layout;
  Q3Frame* Frame5;
  QPushButton* applyPushButton;
  QPushButton* cancelPushButton;
  bool close_on_apply;
};

#endif
