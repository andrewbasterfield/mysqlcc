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
#ifndef CQUERYWINDOW_H
#define CQUERYWINDOW_H

#include "CMyWindow.h"
#include "config.h"
#include "CQueryWindowOptionsDialog.h"
#include "CTable.h"
#include <stddef.h>
#include <qmap.h>
#include <qtabwidget.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <QLabel>
#include <QCustomEvent>
#include <Q3GridLayout>
#include <Q3PopupMenu>
#include <QCloseEvent>

class Q3GridLayout;
class QLabel;
class CMySQLServer;
class CSqlEditor;
class CSqlTable;
class CMySQLQuery;
class CSqlDebugPanel;
class CHistoryPanel;
class CAction;
class CTableFieldChooser;
class CQueryWindow;
class CQuerySet;

class CExplainQueryPanel : public CTable
{
  Q_OBJECT
public:
  CExplainQueryPanel(QWidget * parent);
  void setData(uint rows, uint cols, const QMap<uint, QString> &data);
protected:
  void contextMenuEvent (QContextMenuEvent * e);
};


class CQueryEvent
{
public:
  CQueryEvent(const QString &conn, const QString &qry, const QString &db, const QString &tbl)
    : connection_name(conn), query(qry), database(db), table(tbl)
  {
  }

  QString connection_name;
  QString query;
  QString database;
  QString table;
};

class CQueryWindowTab : public QTabWidget
{
  Q_OBJECT
public:
  CQueryWindowTab (QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0);

public slots:
  void first();
  void back();
  void toggle();
  void next();
  void last();
  void mySetCurrentPage(int t);

private slots:
  void CurrentChanged(QWidget *);  

private:
  int last_tab_idx;
  int current_idx;
  bool tab_toggle;
};

#ifdef QT_THREAD_SUPPORT
#include <qthread.h>

class CThreadedQuery : public QThread
{
public:
  CThreadedQuery(CQuerySet *q, QMutex *m);
  void run();
  void setQuery(const QString &str) { qry = str; }

private:
  QString qry;
  CQuerySet *query_set;
  QMutex *mutex;
};
#endif

class CQuerySet : public QObject
{
  Q_OBJECT

public:
  CQuerySet (CQueryWindow *p, uint id, int querytype = SQL_QUERY, const QString &db=QString::null, const QString &t=QString::null);
  ~CQuerySet();

  CMySQLServer *mysql() const { return mysql_server; }
  uint id() const { return query_set_id; }
  CSqlTable * resultsTable() const { return results_table; }
  CSqlEditor * queryEditor() const { return query_editor; }
  CMySQLQuery * query() const { return m_query; }
  CMySQLQuery * explainQuery() const { return explain_query; }
  bool isBusy() const { return is_busy; }
  bool isCanceling() const { return do_cancel; }
  bool useHistoryFile() const { return use_history_file; }
  bool force() const { return is_force; }  
  QString defaultTable() const { return default_table; }
  QString defaultDatabase() const { return default_database; }
  QString queryLabel() const { return qry_label; }
  QString resultsLabel() const { return res_label; }

  QMap<uint, QString> explainData() { return explain_data; }
  int explainRows() { return explain_rows; }
  int explainCols() { return explain_cols; }
  
  bool refreshQueryOptions(CConfig *c = 0, CQueryWindowOptionsDialog::query_config_options *o = 0);
  bool execQuery(const QString &query_str);
  void customEvent(QCustomEvent *);

public slots:
  void setBusy(bool);
  void executeQuery();
  void cancelQuery();
  void setUseHistoryFile(bool b) { use_history_file = b; }
  void setForce(bool b) { is_force = b; }
  void undoAvailable();
  void redoAvailable();
  void cutCopyAvailable();
  void textAvailable();
  void killThread();
  void setDefaultTable(const QString &s) { default_table = s; };
  void setDefaultDatabase(const QString &s);

private slots:
  void enableDeleteAction(bool b);

private:
#ifdef QT_THREAD_SUPPORT
  QMutex mutex;
  CThreadedQuery *thread;
#endif

  struct finished_event
  {
    bool ret;
    bool read_only;
    QString query_type;
  };

  QMap<uint, QString> explain_data;
  CSqlTable * results_table;
  CSqlEditor * query_editor;

  int explain_rows;
  int explain_cols;

  CMySQLServer *mysql_server;
  uint query_set_id;  
  CMySQLQuery *m_query;
  CMySQLQuery *explain_query;
  CQueryWindow *my_parent;

  QString default_table;
  QString default_database;
  QString default_db;
  QString qry_label;
  QString res_label;
  int query_type;

  bool is_first;
  bool is_busy;
  bool is_force;
  bool use_history_file;
  bool do_cancel;
};


class CQueryWindow : public CMyWindow
{
    Q_OBJECT

public:
  enum query_type { OPEN_TABLE, QUERY, OPEN_TABLE_LIMIT };
  CQueryWindow(QWidget * parent, CMySQLServer *m, int querytype = SQL_QUERY, ushort display=0xFFFF, const char *name = 0, Qt::WFlags f = Qt::WDestructiveClose);
  CQueryWindow(QWidget * parent, const QString &connection_name, int querytype = SQL_QUERY, ushort display=0xFFFF, const char *name = 0, Qt::WFlags f = Qt::WDestructiveClose);
  ~CQueryWindow();

  CQueryWindowTab * resultsTab() const { return results_tab; }
  CQueryWindowTab * queryTab() const { return query_tab; }

  CMySQLServer *mysql() { return mysql_server; }
  CQuerySet * currentQuerySet();
  CQuerySet * currentResultSet();
  CHistoryPanel * historyPanel() const { return history_panel; }
  CTableFieldChooser *columnsWindow() { return columns_window; }
  bool cancelAllQueries() const { return cancel_all_queries; }
  bool queryNewTab() const { return query_new_tab; }
  bool followQueryTab() const { return follow_query_tab; }
  bool followResultsTab() const { return follow_results_tab; }
  int maxResultsSave() const { return max_results_save; }
  int multipleQueriesOpt() const { return multiple_queries_opt; }

  void newQueryWindow(const QString &query_str=QString::null, const QString &database=QString::null, const QString &table=QString::null);

  friend class CQuerySet;

public slots:
  CQuerySet * createNewQuerySet(int q=0, const QString &d=QString::null, const QString &t=QString::null);
  void showQueryPanel(bool);
  void showResultsPanel(bool);
  void openFile();
  void saveQuery();
  void saveResults();
  void printQuery();
  void clearQuery();
  void undoQuery();
  void redoQuery();
  void cutQuery();
  void pasteQuery();
  void showMessages(bool);
  void executeQuery();
  void cancelQuery();
  void reloadHistoryList();
  void setQuery(const QString &s);
  void doExecuteQuery(const QString &);
  void showColumnsWindow(bool b);
  void setTable(const QString &s);
  void setDatabase(const QString &s);
  void setTabFollowDisabled(bool b) { disable_follow = b; }
  void freeResults();
  void newTab();
  void deleteTab();
  void saveToClipboard();
  void newWindowFromTab();

protected:  

  CExplainQueryPanel * explain_panel;

  CAction *fileSaveResultsAction;
  CAction *filePrintAction;
  CAction *fileOpenAction;
  CAction *fileSaveToClipboardAction;
  CAction *fileNewWindowFromTabAction;
  CAction *queryCancelAction;
  CAction *queryExecuteAction;
  CAction *fileNewTabAction;
  CAction *fileDeleteTabAction;
  CAction *editPasteAction;
  CAction *editRedoAction;
  CAction *editUndoAction;
  CAction *editEraseAction;
  QAction *fileCloseAction;
  CAction *viewShowMessagesAction;
  CAction *viewShowColumnsWindowAction;
  CAction *optionsQueryWindowAction;
  CAction *optionsGeneralAction;
  CAction *viewSqlPanelAction;
  CAction *viewResultsPanelAction;  

  CAction *viewQueryTabAction;

  CAction *editCopyAction;
  CAction *editCutAction;
  CAction *queryDeleteRecordAction;
  CAction *queryInsertRecordAction;
  Q3PopupMenu *queryTypesMenu;
  Q3PopupMenu *saveTypesMenu;

  Q3PopupMenu *viewQueryTabsMenu;
  Q3PopupMenu *viewResultTabsMenu;

  QToolButton *queryTypeButton;
  Q3PopupMenu * queryMenu;
  int query_type_item_id;

  QLabel *read_only_label;
  QLabel *executing_query_label;

  Q3PtrList<CQuerySet> fifo;

protected slots:
  void refreshOptions();

private slots:  
  void enableQueryTypes(bool);
  void copy();
  void sqlPanelAction();
  void resultsPanelAction();
  void generalOptions();
  void queryTabChanged(QWidget *);
  void resultsTabChanged(QWidget *w);
  void insertRecord();
  void deleteRecord();
  void viewMenuAboutToShow();
  void queryWindowOptions();
  void tryClose();
  void selectQuery();
  void updateQuery();
  void insertQuery();
  void deleteQuery();
  void setMenuItemsEnabled(bool b);

private:
  void init(const QString &connection_name, int querytype, ushort display, const char *name);
  QTimer *closeTimer;
  Q3PtrList<CQuerySet> querySetList;    

  QToolButton *saveTypeButton;
  CQueryWindowTab* results_tab;
  CQueryWindowTab* query_tab;
  Q3GridLayout* CQueryWindowLayout;
  CSqlDebugPanel *sqldebugpanel;
  CMySQLServer *mysql_server;  
  CHistoryPanel *history_panel;
  CTableFieldChooser *columns_window;
  QString default_table;
  QString default_database;
  int query_type;
  uint tab_number;  
  bool cancel_all_queries;
  bool doing_close;
  bool close_timer;
  bool block_query_tab_changed;
  bool block_results_tab_changed;

  bool query_new_tab;
  bool follow_query_tab;
  bool follow_results_tab;
  bool disable_follow;
  int max_results_save;
  int multiple_queries_opt;

  CQuerySet * callEditorSlot(void (CSqlEditor::*)(void));
  void closeEvent(QCloseEvent * e);
};

#endif // CQUERYWINDOW_H
