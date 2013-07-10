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

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QFrame;
class QPushButton;
class QCheckBox;
class QGroupBox;
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

  CTableTools(QWidget* parent, CMySQLServer *m, const QString dbname, int type, QPtrList<ToolOptions> *options, const QString &tableName = QString::null, const char* name = 0);
  ~CTableTools();

  static QPopupMenu *tableToolsMenu();
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
  QPtrList<ToolOptions> *Options;
  int Type;
  QFrame* m_pTopFrame;
  CTablesListBox* tablesListBox;
  QPushButton* m_pExecutePushButton;
  QPushButton* m_pCancelPushButton;	
  QPushButton* PushButton4;
  QHBoxLayout* Layout14;
  QVBoxLayout* CTableToolsLayout;
  QVBoxLayout* m_pTopFrameLayout;
  QGroupBox *pOptions;
  QGridLayout *pOptionsLayout;
  QGridLayout * Layout;
};

#endif
