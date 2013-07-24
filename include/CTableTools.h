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
#ifndef CTABLETOOLS_H
#define CTABLETOOLS_H

#include "CMyWindow.h"
//Added by qt3to4:
#include <Q3PopupMenu>
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3GridLayout>
#include <Q3PtrList>
#include <Q3HBoxLayout>

class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class Q3Frame;
class QPushButton;
class QCheckBox;
class Q3GroupBox;
class CMySQLServer;
class CTablesListBox;

class CTableTools : public CMyWindow
{ 
  Q_OBJECT
    
public:
  struct ToolOptions
  {
    QString display;
    QString value;
    QCheckBox *checkBox;
  };

  CTableTools(QWidget* parent, CMySQLServer *m, const QString dbname, int type, Q3PtrList<ToolOptions> *options, const QString &tableName = QString::null, const char* name = 0);
  ~CTableTools();

  static Q3PopupMenu *tableToolsMenu();
  static void processMenu(CMySQLServer *m, int res, const QString dbname, const QString & tableName=QString::null);
  static bool inRange(int res);

private slots:
  void execute();
  void create();
  void NoTablesLoaded();

private:
  void init();
  CMySQLServer * mysql;  
  QString m_tableName;
  QString database_name;
  Q3PtrList<ToolOptions> *Options;
  int Type;
  Q3Frame* m_pTopFrame;
  CTablesListBox* tablesListBox;
  QPushButton* m_pExecutePushButton;
  QPushButton* m_pCancelPushButton;	
  QPushButton* PushButton4;
  Q3HBoxLayout* Layout14;
  Q3VBoxLayout* CTableToolsLayout;
  Q3VBoxLayout* m_pTopFrameLayout;
  Q3GroupBox *pOptions;
  Q3GridLayout *pOptionsLayout;
  Q3GridLayout * Layout;
};

#endif
