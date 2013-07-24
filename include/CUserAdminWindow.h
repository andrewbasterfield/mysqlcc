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
#ifndef CUSERADMINWINDOW_H
#define CUSERADMINWINDOW_H

#include <stddef.h>
#include <qvariant.h>
#include <q3listview.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3ActionGroup>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <Q3PopupMenu>
#include <QCloseEvent>
#include "CMyWindow.h"

class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class QAction;
class Q3ActionGroup;
class Q3ToolBar;
class Q3PopupMenu;
class Q3ButtonGroup;
class QCheckBox;
class QLabel;
class QLineEdit;
class Q3ListBox;
class Q3ListBoxItem;
class QPushButton;
class CMySQLServer;

class CGrantItem : public Q3CheckListItem
{
public:
  enum Type { GLOBAL, DATABASE, TABLE };
  CGrantItem(Q3ListView * parent, const QString &txt, const QPixmap &pix, Type t);
  CGrantItem(Q3ListViewItem * parent, const QString &txt, const QPixmap &pix, Type t);
  
  Type type() { return m_type; }
  
  QString databaseName() const { return dbname; }
  QString tableName() const { return tblname; }
  void setDatabaseName(const QString &s) { dbname = s; }
  void setTableName(const QString &s) { tblname = s; }

  QStringList privileges;
  bool allPrivs;
  bool withGrant;    

private:
  void init(const QPixmap &pix, const Type &t);
  void stateChange(bool b);
  int compare(Q3ListViewItem * i, int col, bool ascending) const;
  Type m_type;
  QString dbname;
  QString tblname;
};

class CUserAdminWindow : public CMyWindow
{ 
  Q_OBJECT
    
public:
  CUserAdminWindow(QWidget* parent, CMySQLServer *m, const QString &u, const QString &h, bool e);
  ~CUserAdminWindow();
  static bool removeUser(CMySQLServer *m, const QString &username, const QString &hostname, bool quiet=false);

public slots:
  void refresh();

signals:
  void do_refresh();

private slots:
  void deleteClicked();
  void applyClicked();
  void refreshItems();
  void refreshPrivilegesList();
  void setCurrentItem(Q3ListViewItem *i);
  void allPrivilegesToggled(bool);
  void withGrantToggled(bool);
  void privilegeListBoxChanged();
  void checkEditing(const QString &);

private:
  void closeEvent(QCloseEvent * e);
  void setBusy(bool b);
  void getDefaultItemPrivileges(QStringList &lst, CGrantItem::Type t);
  bool grantPrivileges();
  void parseUserGrants();
  CGrantItem *findItem(const QString &db, const QString &tbl);
  void setWindowCaption();

  QString username;
  QString hostname;
  QString password;
  bool is_editing;
  bool is_busy;
  bool emit_refresh;
  bool block;
  CMySQLServer *mysql;
  QPixmap dbIcon;
  QPixmap tableIcon;
  QPixmap globalPrivsIcon;
  QLineEdit* Password;
  QLabel* hostLabel;
  QLabel* passwordLabel;
  QLineEdit* Host;
  Q3ButtonGroup* ButtonGroup1;
  QCheckBox* allPrivileges;
  QCheckBox* withGrantOption;
  Q3ListBox* privilegeListBox;
  QLineEdit* Username;
  QLabel* usernameLabel;
  QPushButton* deleteButton;
  QPushButton* applyButton;
  QPushButton* closeButton;
  Q3ListView* databaseListView;
  Q3GridLayout* CUserAdminWindowLayout;
  Q3GridLayout* ButtonGroup1Layout;
  Q3HBoxLayout* Layout2;
};

#endif
