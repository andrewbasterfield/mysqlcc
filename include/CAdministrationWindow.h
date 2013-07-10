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
#ifndef CADMINISTRATIONWINDOW_H
#define CADMINISTRATIONWINDOW_H

#include "CMyWindow.h"
#include "CProperties.h"
#include "editor.h"
#include "CQueryTable.h"

class CMySQLServer;
class QGridLayout; 
class QTabWidget;
class CMySQL;
class CAction;
class CTableFieldChooser;
class QTimer;
class QSpinBox;

class CProcessListTable : public CQueryTable
{
  Q_OBJECT

public:
  CProcessListTable(QWidget * parent, CMySQLServer *m);
  ~CProcessListTable();

public slots:
  void refresh();

private slots:  
  void ContextMenuRequested(int row, int col, const QPoint &pos);
  void DoubleClicked(int row, int col, int button, const QPoint &);

protected:
  void copy_data_func(QString *cpy, CMySQLQuery *qry, QTableSelection *sel, QMap<uint, ulong> *max_length_map);

private:
  class CCheckTableItem : public QCheckTableItem
  {
  public:
    CCheckTableItem(QTable * table, const QString & txt)
      : QCheckTableItem(table, txt) {}
    int alignment() const { return Qt::AlignLeft; }
  };

  bool is_first;
  CTableFieldChooser * columnsWindow;
  CMySQLQuery *qry;
  QString sql;
};

class CServerStatusTable : public CQueryTable
{
  Q_OBJECT

public:
  CServerStatusTable(QWidget * parent, CMySQLServer *m);
  ~CServerStatusTable();
  void setTraditionalMode(bool b) { is_traditional = b; }
  bool isTraditionalMode() { return is_traditional; }

public slots:
  void refresh();
  void copy(int row, int col);

private:
  void ContextMenuRequested(int row, int col, const QPoint &pos);
  bool is_first;
  bool is_traditional;
  CMySQLQuery *qry;
  CTableFieldChooser * columnsWindow;
};

class CInnoDBStatus : public Editor
{
  Q_OBJECT

public:
  CInnoDBStatus(QWidget *parent, CMySQLServer *m);
  static bool hasInnoDB(CMySQLServer *mysql);

public slots:
  void refresh();
  void save();

private:
  QString contents() const;
  QPopupMenu * createPopupMenu(const QPoint &p);
  CMySQLServer *mysql;
};

class CAdministrationWindow : public CMyWindow
{ 
  Q_OBJECT
    
public:
  enum tabs { SHOW_PROCESSLIST, SHOW_STATUS, SHOW_VARIABLES, SHOW_INNODB_STATUS };
  CAdministrationWindow(QWidget* parent,  CMySQLServer *m);
  ~CAdministrationWindow();
  CMySQLServer * mysql() const { return m_mysql; }
  void setCurrentTabPage(int);
  static QPopupMenu * flushMenu(const CMySQL *mysql);
  static void ping(CMySQLServer *m);
  static void shutdown(CMySQLServer *m);
  static void flush(CMySQLServer *m, int flush_type);

public slots:
  void refresh();

private slots:
  void tabChanged(QWidget *);
  void showMessages(bool);
  void viewMenuAboutToShow();
  void fileTimerActionToggled(bool b);
  void killProcesses();
  void ping();
  void shutdown();
  void flush(int);
  void save(int);

private:

  class CShowServerVariables : public CProperties
  {
  public:
    CShowServerVariables(QWidget *parent, CMySQLServer *m);    

  public slots:
    void refresh();

  private:  
    void insertData();
  };

  void saveSettings(CConfig *);
  void setBlocked(bool b);
  bool isBlocked() { return blocked; }
  bool blocked;
  QSpinBox * delay;
  QTimer * refreshTimer;
  CAction *viewShowMessagesAction;
  CAction *fileRefreshAction;
  CAction * fileTimerAction;
  CAction * actionKillProcessAction;
  CShowServerVariables *variables;
  CInnoDBStatus *innoDBStatus;
  CProcessListTable *processList;
  CServerStatusTable *status;
  QPopupMenu * flush_menu;
  QPopupMenu * save_menu;
  QGridLayout* CAdministrationWindowLayout;
  QTabWidget* tabWidget;
  CMySQLServer *m_mysql;
  bool has_innodb;
  bool delete_mysql;
};

#endif
