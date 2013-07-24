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
#include "CQueryWindow.h"
#include "globals.h"
#include "CConfig.h"
#include "CHotKeyEditorDialog.h"
#include "CApplication.h"
#include "CAction.h"
#include "shared_menus.h"
#include "panels.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "CSqlEditor.h"
#include "CSqlTable.h"
#include "CAction.h"
#include "CAppConfigDialog.h"
#include "CHistoryView.h"
#include "CConsoleWindow.h"
#include "CTableFieldChooser.h"

#include <stddef.h>
#include <q3accel.h>
#include <qtimer.h>
#include <qsplitter.h>
#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qcursor.h>
#include <qtoolbutton.h>
#include <qmenubar.h>
#include <q3popupmenu.h>
#include <q3toolbar.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <QCustomEvent>

#define PROCESS_EVENT 35

CExplainQueryPanel::CExplainQueryPanel(QWidget * parent)
: CTable(parent, "CExplainQueryPanel")
{
#ifndef QT_THREAD_SUPPORT
  enableProcessEvents(true);
#else
  enableProcessEvents(false);
#endif
  verticalHeader()->hide();
  setFocusStyle(Q3Table::FollowStyle);
  setSelectionMode(Q3Table::Single);
  setLeftMargin(0);
  setCaption(tr("Explain Panel"));
  disconnect(SIGNAL(contextMenuRequested(int, int, const QPoint &)));
}

void CExplainQueryPanel::setData(uint rows, uint cols, const QMap<uint, QString> &data)
{
  reset();
  if ((rows == 0 && cols == 0) || data.isEmpty())
    return;
  else
  {
    setNumRows(rows);
    setNumCols(cols);
    for (uint i = 0; i <= rows; i++)
      for (uint j = 0; j < cols; j++)
      {
        if (i == 0)
          horizontalHeader()->setLabel(j, data[j]);
        else
          setText(i - 1, j, data[(i * cols) + j]);
      }
  }
}

void CExplainQueryPanel::contextMenuEvent (QContextMenuEvent * e)
{
  if (isBlocked())
    return;
  Q3PopupMenu *menu = new Q3PopupMenu();
  menu->insertItem(getPixmapIcon("copyIcon"), tr("&Copy"), MENU_COPY);
  int res = menu->exec(e->globalPos());
  delete menu;
  
  switch (res)
  {   
  case MENU_COPY:
    copy(rowAt(e->pos().y()), columnAt(e->pos().x()));
    break;
  }
  e->accept();
}


#ifdef QT_THREAD_SUPPORT
CThreadedQuery::CThreadedQuery(CQuerySet *q, QMutex *m)
: query_set(q), mutex(m)
{
  qry = QString::null;
}

void CThreadedQuery::run()
{
  query_set->execQuery(qry);
}
#endif


CQuerySet::CQuerySet(CQueryWindow *p, uint id, int querytype, const QString &db, const QString &t)
: QObject(p, "CQuerySet"), query_set_id(id), query_type(querytype), is_busy(false)
{
  use_history_file = true;
  is_force = true;
  my_parent = p;
  do_cancel = false;
  is_first = true;
  default_database = QString::null;
  explain_rows = 0;
  explain_cols = 0;
  
  mysql_server = new CMySQLServer(my_parent->mysql()->connectionName(), my_parent->messagePanel());
  mysql_server->connect();

  m_query = new CMySQLQuery(mysql_server->mysql());
  m_query->setAutoDelete(false);

  explain_query = new CMySQLQuery(mysql_server->mysql());
  explain_query->setEmitMessages(false);

  query_editor = new CSqlEditor(my_parent->queryTab(), mysql_server, "query_editor");
  query_editor->setWordWrap(Q3TextEdit::WidgetWidth);
  query_editor->setWrapPolicy(Q3TextEdit::AtWhiteSpace);

  if (!db.isEmpty())
    setDefaultDatabase(db);

  default_table = t;

  QString number = " " + QString::number(query_set_id);
  qry_label = tr("Query") + number;
  res_label = tr("Result") + number;
  my_parent->queryTab()->insertTab(query_editor, qry_label);
  
  results_table = new CSqlTable(my_parent->resultsTab(), m_query, mysql_server);
  results_table->setRealParent(my_parent);
  results_table->setHistoryPanel(my_parent->historyPanel());
  results_table->reset();
  connect(results_table, SIGNAL(canDelete(bool)), this, SLOT(enableDeleteAction(bool)));
  
  connect (my_parent, SIGNAL(about_to_close()), results_table, SLOT(aboutToClose()));
    
  my_parent->resultsTab()->insertTab(results_table, res_label);

  connect(query_editor, SIGNAL(undoAvailable(bool)), this, SLOT(undoAvailable()));
  connect(query_editor, SIGNAL(redoAvailable(bool)), this, SLOT(redoAvailable()));
  connect(query_editor, SIGNAL(copyAvailable(bool)), this, SLOT(cutCopyAvailable()));
  connect(query_editor, SIGNAL(textChanged()), this, SLOT(textAvailable()));
  connect(results_table, SIGNAL(selectionChanged()), this, SLOT(cutCopyAvailable()));

  textAvailable();

  refreshQueryOptions();

#ifdef QT_THREAD_SUPPORT
  thread = new CThreadedQuery(this, &mutex);
#endif
}

void CQuerySet::enableDeleteAction(bool b)
{
  if (my_parent->resultsTab()->currentPage() == results_table)
    my_parent->queryDeleteRecordAction->setEnabled(b);
}

void CQuerySet::setDefaultDatabase(const QString &s)
{
  default_db = s;
  if (default_database != s)
  {
    default_table = QString::null;
    if (mysql()->mysql()->mysqlSelectDb(s))
    {
      default_database = s;
      if (!do_cancel)
        query_editor->refreshCompletionFields();
    }
  }
}

void CQuerySet::textAvailable()
{
  if (my_parent->queryTab()->currentPage() == query_editor)
  {
    bool tmp = !query_editor->text().isEmpty() && my_parent->queryTab()->isVisible();
    my_parent->editEraseAction->setEnabled(tmp);
    my_parent->queryExecuteAction->setEnabled(tmp && !isBusy());
  }
}

void CQuerySet::undoAvailable()
{
  if (my_parent->queryTab()->currentPage() == query_editor)
    my_parent->editUndoAction->setEnabled(query_editor->isUndoAvailable() && my_parent->queryTab()->isVisible());
}

void CQuerySet::redoAvailable()
{
  if (my_parent->queryTab()->currentPage() == query_editor)
    my_parent->editRedoAction->setEnabled(query_editor->isRedoAvailable() && my_parent->queryTab()->isVisible());
}

void CQuerySet::cutCopyAvailable()
{
#ifdef DEBUG
  qDebug("CQuerySet::cutCopyAvailable()");
#endif

  bool cut_copy = false;
   if (my_parent->queryTab()->currentPage() == query_editor)
     cut_copy = my_parent->queryTab()->currentPage() == query_editor && 
                my_parent->queryTab()->isVisible() && queryEditor()->hasFocus() &&
                queryEditor()->hasSelectedText();

  if (!cut_copy)
  {
    cut_copy = my_parent->resultsTab()->currentPage() == results_table && 
               my_parent->resultsTab()->isVisible() && resultsTable()->hasFocus() &&
               resultsTable()->numCols() > 0 && resultsTable()->numRows() > 0;
    my_parent->editCutAction->setEnabled(false);
  }
  else
    my_parent->editCutAction->setEnabled(cut_copy);

#ifdef QT_NO_CLIPBOARD
    my_parent->editCopyAction->setEnabled(false);
#else
    my_parent->editCopyAction->setEnabled(cut_copy);
#endif    
}

CQuerySet::~CQuerySet()
{
  my_parent->queryTab()->removePage(query_editor);
  my_parent->resultsTab()->removePage(results_table);
  delete query_editor;
  delete results_table;
  delete m_query;
  delete mysql_server;
  delete explain_query;
#ifdef QT_THREAD_SUPPORT
  delete thread;
#endif
}

void CQuerySet::killThread()
{
#ifdef QT_THREAD_SUPPORT
#if QT_VERSION >= 0x030100
  if (thread->running())
    thread->terminate();
#endif
#endif
}

bool CQuerySet::execQuery(const QString &query_str)
{
#ifdef DEBUG
  qDebug("CQueryWindow::execQuery()");
#endif  
  
#ifndef QT_THREAD_SUPPORT
  int check_pending_event = 0;
#endif
  
  finished_event * finished_evt = new finished_event;
  finished_evt->query_type = QString::null;
  finished_evt->read_only = true;
  finished_evt->ret = false;

  if (!mysql_server->isConnected())
  {
    bool e = mysql_server->mysql()->hasEmitErrors();
    if (is_first)
      mysql_server->mysql()->setEmitErrors(false);
    mysql_server->connect();
    if (is_first)
      mysql_server->mysql()->setEmitErrors(e);
    if (!mysql_server->isConnected())
    {
      QCustomEvent *event = new QCustomEvent(FINISHED);
      event->setData(finished_evt);
      QApplication::postEvent(this, event);
      is_first = false;
      return false;
    }
    else
      setDefaultDatabase(default_db);
  }

  is_first = false;

  QString qry = query_str;
  bool query_ok = false;
  bool isquoted = false;
  bool new_tab = false;
  bool multiple_queries = false;

  explain_rows = 0;
  explain_cols = 0;
 
  do_cancel = false;
  explain_data.clear();  
  
  QString original_database = default_database;
  QString original_table = default_table;
  QString queryType;
  QString q;
  QString last_query;
  
  QChar startquote;
  QChar c;
 
  if (!qry.stripWhiteSpace().isEmpty())
  {
    qry = CMySQLQuery::strip_comments(qry);
    
    if (qry.at(qry.length() -1) != ';')
      qry += ";";
    ulong slash = 0;
    for (ulong i = 0; i < qry.length(); i++)
    {
      if (do_cancel)
        break;
      c = qry.at(i);
      
      if (!isquoted && (c == '\"' || c == '\''))
      {   
        isquoted = true;
        startquote = c;
        slash = 0;
      }
      else
        if (isquoted)
        {
          if (c == '\\')
            slash++;
          else if (c == startquote && (slash % 2 == 0)) // 'Foo' or '\\Foo\\'
          {
            isquoted = false;
            slash = 0;
          }
          else
            if (slash % 2 > 0 && (c == '\"' || c == '\'' || qry.at(i + 1) != '\\')) //  'Foo\'' or 'Foo\?'
              slash = 0;
        }
        
        if (!isquoted && c == ';')
        {
          q = q.stripWhiteSpace();
          if (!q.isEmpty())
          {
            last_query = q;
            if (!q.isEmpty())
            {
              query_ok = true;
              
              if (use_history_file)
              {
                QCustomEvent *event = new QCustomEvent(HISTORY);
                event->setData(new QString(last_query));
                QApplication::postEvent(this, event);
              }
              
              queryType = CMySQLQuery::query_type(q.lower());
              
              if (queryType == "use")
              {
                bool emit_messages = query()->hasEmitMessages();
                query()->setEmitMessages(false);
                query_ok = query()->exec(mysql()->mysql()->codec()->fromUnicode(q));
                query()->setEmitMessages(emit_messages);
                if (query_ok)
                {
                  mysql()->mysql()->emitMessage(INFORMATION, tr("Database Changed"));
                  QString db = mysql()->mysql()->realDatabaseName();
                  if (!db.isEmpty() && db != default_database)
                    setDefaultDatabase(db);
                }
                query()->setEmitMessages(emit_messages);
              }
              else
              {
                query_ok = query()->exec(mysql()->mysql()->codec()->fromUnicode(q));
                if (i < qry.length() - 1)
                  multiple_queries = true;
                if (query_ok && query()->returnedResults())
                {
                  if (!new_tab)
                    new_tab = true;
                  if (my_parent->multipleQueriesOpt() == 1 && multiple_queries)
                  {
                    QCustomEvent *event = new QCustomEvent(NEW_QUERY_WINDOW);
                    event->setData(new CQueryEvent(mysql()->connectionName(),
                      mysql()->mysql()->codec()->fromUnicode(q), default_database, default_table));
                    QApplication::postEvent(this, event);
                  }
                }
              }
              if (!query_ok)
              {
                QCustomEvent *event = new QCustomEvent(SET_ACTIVE_PANEL);
                event->setData(new int(0));
                QApplication::postEvent(this, event);                
                if (!is_force)
                  break;
              }
            }
          }
          q = QString::null;
        }
        else
          q += c;
#ifndef QT_THREAD_SUPPORT
        if (++check_pending_event >= PROCESS_EVENT && qApp->hasPendingEvents())
        {
          qApp->processEvents();
          check_pending_event = 0;
        }
#endif
    }
    if (isquoted)
      mysql()->mysql()->emitMessage(WARNING, tr("Error: You have mismatched") + " `" + startquote + "`");
   
    query_type = SQL_QUERY;    
    
    if (query_ok && !do_cancel)
    {
      bool exp_ok = false;

      finished_evt->query_type = queryType;
      finished_evt->read_only = true;

      if (queryType == "select")
      {
        if (explain_query->exec("EXPLAIN " + last_query))
        {
          explain_rows = explain_query->numRows();
          explain_cols = explain_query->numFields();
          exp_ok = true;
          if (explain_rows == 1)
          {
            explain_query->next(true);
//            default_table = explain_query->row(mysql()->mysql()->version().major >= 4 && 
//              mysql()->mysql()->version().minor >= 1 ? 2 : 0);
                               // NetVicious modification
                               if ( mysql()->mysql()->version().major == 5 )
                               {
                                       default_table = explain_query->row(2);
                               }
                               else
                               {
               default_table = explain_query->row(mysql()->mysql()->version().major >= 4 &&
                       mysql()->mysql()->version().minor >= 1 ? 2 : 0);
                               }
                               // End NetVicious modification


            if (!default_table.isEmpty())
            {
              finished_evt->read_only = default_database.isEmpty() || 
                !last_query.simplifyWhiteSpace().lower().startsWith("select * from");
              query_type = SQL_ALL_ROWS;
            }
            else
              default_table = QString::null;
          }

          explain_query->dataSeek(0);
          uint k = 0;
          for (int i = 0; i < explain_cols; i++)
            explain_data[k++] = explain_query->fields(i).name;
      
          while (explain_query->next())
          {
            for (int i = 0; i < explain_cols; i++)
              explain_data[k++] = explain_query->row(i) ? explain_query->row(i) : NULL_TEXT;
          }
          explain_query->freeResult();
        }     
      }

      if (new_tab && my_parent->queryNewTab())
      {
        QCustomEvent *event = new QCustomEvent(NEW_QUERY_TAB);
        event->setData(new CQueryEvent(mysql()->connectionName(),
        mysql()->mysql()->codec()->fromUnicode(queryEditor()->text()), original_database, original_table));
        QApplication::postEvent(this, event);
      }      
    }    

    finished_evt->ret = !do_cancel;
    if (do_cancel)
    {
      cancelQuery();
      mysql()->mysql()->emitMessage(CRITICAL, tr("Query canceled by user"));
    }
  }
  else
    mysql()->mysql()->emitMessage(CRITICAL, tr("No query specified"));

  QCustomEvent *event = new QCustomEvent(FINISHED);
  event->setData(finished_evt);
  QApplication::postEvent(this, event);
  return !do_cancel;
}


void CQuerySet::customEvent(QCustomEvent *event)
{
  switch ((int) event->type())
  {
  case HISTORY:
    {
      QString *s = (QString *) event->data();
      my_parent->historyPanel()->History(*s);      
      delete s;
    }
    break;

  case NEW_QUERY_WINDOW:
    {
      CQueryEvent *evt = (CQueryEvent *) event->data();
      my_parent->newQueryWindow(evt->query, evt->database, evt->table);
      delete evt;
      my_parent->freeResults();
    }
    break;

  case NEW_QUERY_TAB:
    {
      CQueryEvent *evt = (CQueryEvent *) event->data();
      CQuerySet *q = my_parent->createNewQuerySet(query_type, evt->database, evt->table);
      if (q)
      {
        q->queryEditor()->setText(evt->query);
        my_parent->setTabFollowDisabled(true);
        my_parent->resultsTab()->mySetCurrentPage(my_parent->resultsTab()->indexOf(results_table));
        my_parent->queryTab()->mySetCurrentPage(my_parent->queryTab()->indexOf(q->queryEditor()));
        my_parent->setTabFollowDisabled(false);
      }
      delete evt;
      my_parent->freeResults();
    }
    break;

  case SET_ACTIVE_PANEL:
    {
      int *id = (int *) event->data();
      my_parent->messageWindow()->setActivePanel(*id);
      delete id;
    }
    break;

  case FINISHED:
    {
      finished_event * evt = (finished_event *) event->data();

      if (evt->ret)
      {
        results_table->setQuery(m_query);
        results_table->refresh();
        if (!default_table.isEmpty())
          results_table->setTableName(default_table);
        if (!default_database.isEmpty())
          results_table->setDatabaseName(default_database);
        if (evt->query_type == "select")
          results_table->setReadOnly(evt->read_only);
      }

      if (my_parent->resultsTab()->currentPage() == results_table)
      {
        my_parent->columnsWindow()->setTable(results_table);
        my_parent->read_only_label->setEnabled(results_table->isReadOnly());
        my_parent->explain_panel->setData(explain_rows, explain_cols, explain_data);
        my_parent->messageWindow()->setTabEnabled(my_parent->explain_panel, explain_cols > 0 && !explain_data.isEmpty());
      }

      if (my_parent->queryTab()->currentPage() == query_editor)
      {
        my_parent->queryDeleteRecordAction->setEnabled(!results_table->isReadOnly() && query()->numRows() > 0);
        my_parent->queryInsertRecordAction->setEnabled(!results_table->isReadOnly());
        my_parent->queryExecuteAction->setEnabled(true);
        my_parent->executing_query_label->setEnabled(false);
        my_parent->queryTypeButton->setEnabled(!defaultTable().isEmpty() && query_type != SQL_QUERY);
        my_parent->queryMenu->setItemEnabled(my_parent->query_type_item_id, !defaultTable().isEmpty() && query_type != SQL_QUERY);        
      }

      QString tip = "\n\n";
      if (defaultDatabase().isEmpty() && query_type == SQL_QUERY)
        tip += tr("Query");
      else if (query_type == SQL_QUERY)
        tip += tr("Query in Database") + " '" + mysql()->mysql()->databaseName() + "'";
      else
      {
        tip += tr("Data in Table") + ": '" + default_table + "'";
        if (!mysql()->mysql()->databaseName().isEmpty())
          tip += " " + tr("in") + " '" + mysql()->mysql()->databaseName() + "'";
      }

      tip += "\n\n" + query_editor->text().left(150);
      if (query_editor->text().length() > 150)
        tip += "...";

      my_parent->queryTab()->setTabToolTip(query_editor, my_parent->queryTab()->tabLabel(query_editor) + tip);
      my_parent->resultsTab()->setTabToolTip(results_table, my_parent->resultsTab()->tabLabel(results_table) + tip);

      my_parent->historyPanel()->setBottomItem();
      my_parent->messagePanel()->setBottomItem();
      myApp()->consoleWindow()->sqlDebugPanel()->setBottomItem();

      if (evt->ret && results_table->numRows() > 0 && results_table->numCols() > 0)
        my_parent->fifo.append(this);
      else
        my_parent->fifo.removeRef(this);
      queryEditor()->setReadOnly(false);
      setBusy(false);
      delete evt;
    }
    break;

  case PROCESS_EVENTS:
    qApp->processEvents();
    break;

  default:
    break;
  }
}

void CQuerySet::executeQuery()
{
  if (isBusy())
    return;

  setBusy(true);

  my_parent->fifo.removeRef(this);

  if (my_parent->resultsTab()->currentPage() == results_table)
  {
    my_parent->queryDeleteRecordAction->setEnabled(false);
    my_parent->queryInsertRecordAction->setEnabled(false);
    my_parent->read_only_label->setEnabled(results_table->isReadOnly());
    my_parent->messageWindow()->setTabEnabled(my_parent->explain_panel, false);
  }

  if (my_parent->queryTab()->currentPage() == query_editor)
  {
    my_parent->queryCancelAction->setEnabled(true);
    my_parent->executing_query_label->setEnabled(true);
    my_parent->queryExecuteAction->setEnabled(false);
  }

  do_cancel = false;

  results_table->reset();
  my_parent->showResultsPanel(true);

#ifndef QT_THREAD_SUPPORT    
  execQuery(query_editor->hasSelectedText() ? query_editor->selectedText() : query_editor->text());    
#else
  if (!thread->running())
  {
    mutex.lock();
    thread->setQuery(query_editor->hasSelectedText() ? query_editor->selectedText() : query_editor->text());
    thread->start();
    mutex.unlock();
  }
#endif
}

void CQuerySet::cancelQuery()
{
#ifdef DEBUG
  qDebug("CQuerySet::cancel()");
#endif

  if (!isBusy() || do_cancel)
  {
    setBusy(true);
    results_table->reset();
    query()->cancel();
    query()->freeResult();
    if (my_parent->queryTab()->currentPage() == query_editor)
    {
      my_parent->queryDeleteRecordAction->setEnabled(false);
      my_parent->queryInsertRecordAction->setEnabled(false);
      my_parent->queryCancelAction->setEnabled(false);
      my_parent->executing_query_label->setEnabled(false);      
    }
    if (my_parent->resultsTab()->currentPage() == results_table)
    {
      my_parent->read_only_label->setEnabled(results_table->isReadOnly());
      explain_data.clear();
      my_parent->messageWindow()->setTabEnabled(my_parent->explain_panel, false);
    }

    do_cancel = false;
    setBusy(false);
  }
  else
  {
    if (myApp()->confirmCritical() && !my_parent->cancelAllQueries())
      if ((QMessageBox::information(0, tr("Confirm Cancel"), tr("Are you sure you want to cancel Query") + QString::number(query_set_id) + " ?", tr("&Yes"),tr("&No")) != 0))
        return;
    do_cancel = true;
    if (my_parent->queryTab()->currentPage() == query_editor)
      my_parent->queryCancelAction->setEnabled(false);
    results_table->cancel();
  }
}


bool CQuerySet::refreshQueryOptions(CConfig *c, CQueryWindowOptionsDialog::query_config_options *o)
{
  CConfig *cfg;
  if (!c)
    cfg = new CConfig(mysql()->connectionName(), mysql()->connectionsPath());
  else
    cfg = c;

  CQueryWindowOptionsDialog::query_config_options *opt;

  if (!o)
    opt = new CQueryWindowOptionsDialog::query_config_options(CQueryWindowOptionsDialog::readConfig(cfg));
  else
    opt = o;

  bool ret = true;

  setUseHistoryFile(opt->use_history_file);
  resultsTable()->addToHistoryPanel(useHistoryFile());
  query()->setEmitMessages(opt->silent);
  resultsTable()->displayMessages(opt->silent);
  mysql()->enableSqlDebugMessages(opt->silent);

  setForce(opt->force);

  CMySQLQuery *qry = new CMySQLQuery(mysql()->mysql());
  qry->setEmitErrors(false);
  qry->setEmitMessages(false);
  for (CQueryWindowOptionsDialog::query_config_options_map::Iterator it = opt->options.begin(); it != opt->options.end(); ++it)
    if (!qry->execStaticQuery("SET " + it.key().upper() + "=" + it.data()))
    {
      ret = false;
      cfg->writeEntry(it.key(), "0");
    }
  delete qry;

  if (!c)
  {
    if (!ret)
      cfg->save();
    delete cfg;
  }

  if (!o)
    delete opt;

  return ret;
}

void CQuerySet::setBusy(bool b)
{
  is_busy = b;

  QCursor c = b ? Qt::WaitCursor : Qt::ArrowCursor;
  queryEditor()->setCursor(c);
  resultsTable()->setCursor(c);

  my_parent->messagePanel()->setEnableBottomItem(!is_busy);
  my_parent->historyPanel()->setEnableBottomItem(!is_busy);
  myApp()->consoleWindow()->sqlDebugPanel()->setEnableBottomItem(!is_busy);

  QCustomEvent *event = new QCustomEvent(PROCESS_EVENTS);
  QApplication::postEvent(this, event);
}

CQueryWindow::CQueryWindow(QWidget * parent, const QString &connection_name, int querytype, ushort display, const char *name, Qt::WFlags f)
: CMyWindow(parent, name, false, f), tab_number(0)
{  
  init(connection_name, querytype, display, name);
}

CQueryWindow::CQueryWindow(QWidget * parent, CMySQLServer *m, int querytype, ushort display, const char *name, Qt::WFlags f)
: CMyWindow(parent, name, false, f), tab_number(0)
{
  init (m->connectionName(), querytype, display, name);
}

void CQueryWindow::init(const QString &connection_name, int querytype, ushort display, const char *name)
{
  if ( !name )
    setName( "CQueryWindow" );

  myApp()->incCritical();
  enableMessageWindow(true);
  mysql_server = new CMySQLServer(connection_name, messagePanel());
  querySetList.setAutoDelete(true);  
  setIcon(getPixmapIcon("applicationIcon"));
  doing_close = false;
  cancel_all_queries = false;
  close_timer = false;
  block_query_tab_changed = false;
  block_results_tab_changed = false;
  disable_follow = false;
  default_database = QString::null;
  default_table = QString::null;
  query_type = querytype;
  
  CConfig *cfg = new CConfig(mysql()->connectionName(), mysql()->connectionsPath());

  query_new_tab = strtobool(cfg->readStringEntry("Query New Tab", "false"));
  follow_query_tab = strtobool(cfg->readStringEntry("Follow Query Tab", "true"));
  follow_results_tab = strtobool(cfg->readStringEntry("Follow Results Tab", "false"));
  max_results_save = cfg->readNumberEntry("Max Results to Save", 10);
  multiple_queries_opt = cfg->readNumberEntry("Multiple Query Options", 0);  
  delete cfg;

  (void)statusBar();

  read_only_label = new QLabel(statusBar());
  read_only_label->setText(tr("Read Only"));
  read_only_label->setEnabled(true);
  read_only_label->setAlignment(Qt::AlignHCenter);

  executing_query_label = new QLabel(statusBar());
  executing_query_label->setText(tr("Executing Query"));
  executing_query_label->setEnabled(false);
  executing_query_label->setAlignment(Qt::AlignHCenter);

  statusBar()->addWidget(executing_query_label, 0, true);
  statusBar()->addWidget(read_only_label, 0, true);
  
  setCentralWidget( new QWidget( this, "qt_central_widget" ) );
  CQueryWindowLayout = new Q3GridLayout( centralWidget(), 1, 1, 4, 0, "CQueryWindowLayout"); 
  
  QSplitter* splitter = new QSplitter(Qt::Vertical, centralWidget());
  
  query_tab = new CQueryWindowTab( splitter, "query_tab" );
  query_tab->setTabPosition( QTabWidget::Bottom );
  query_tab->setTabShape (QTabWidget::Triangular);
  
  results_tab = new CQueryWindowTab( splitter, "results_tab" );
  results_tab->setTabPosition( QTabWidget::Bottom);
  results_tab->setTabShape (QTabWidget::Triangular);
  
  CQueryWindowLayout->addWidget(splitter, 0, 0 );
  
  splitter->setOpaqueResize(true);
  splitter->setResizeMode (query_tab, QSplitter::KeepSize);
  splitter->setResizeMode (results_tab, QSplitter::KeepSize);    
  
  query_tab->setMinimumSize(10, 50);  
  results_tab->setMinimumSize(10, 50);
  
  columns_window = new CTableFieldChooser(0, "CQueryTableColumnsWindow", Q3DockWindow::InDock);
  columns_window->setCaption("[" + mysql()->connectionName() + "] " + tr("Query Columns"));
  
  setDockEnabled(columns_window, Qt::DockRight, true);
  moveDockWindow(columns_window, Qt::DockRight);
  
  sqldebugpanel = 0;
  
  fileOpenAction = new CAction(tr("Open"), getPixmapIcon("openIcon"), tr("&Open"),
    Qt::CTRL + Qt::Key_O, this, "fileOpenAction");
  fileOpenAction->setParentMenuText(tr("File"));
  connect(fileOpenAction, SIGNAL(activated()), this, SLOT(openFile()));  

  saveTypesMenu = new Q3PopupMenu(this);

  CAction *fileSaveToClipboardAction = new CAction(tr("Save to Clipboard"), getPixmapIcon("saveToClipboardIcon"),
    tr("Save to &Clipboard"), Qt::ALT + Qt::Key_S, this, "fileSaveToClipboardAction");
  fileSaveToClipboardAction->setParentMenuText(tr("File | Save"));
  connect(fileSaveToClipboardAction, SIGNAL(activated()), this, SLOT(saveToClipboard()));
  fileSaveToClipboardAction->addTo(saveTypesMenu);

  saveTypesMenu->insertSeparator();

  CAction *fileSaveAction = new CAction(tr("Save Query"), getPixmapIcon("saveIcon"), tr("&Save Query"),
    Qt::CTRL + Qt::Key_S, this, "fileSaveAction");
  fileSaveAction->setParentMenuText(tr("File | Save"));
  connect(fileSaveAction, SIGNAL(activated()), this, SLOT(saveQuery()));
  fileSaveAction->addTo(saveTypesMenu);
  
  fileSaveResultsAction = new CAction(tr("Save Results"), getPixmapIcon("saveGridResultsIcon"), tr("Save &Results"),
    Qt::CTRL + Qt::Key_R, this, "fileSaveAction");
  fileSaveResultsAction->setParentMenuText(tr("File | Save"));
  connect(fileSaveResultsAction, SIGNAL(activated()), this, SLOT(saveResults()));
  fileSaveResultsAction->addTo(saveTypesMenu);
  
  fileNewTabAction = new CAction(tr("New Tab"), getPixmapIcon("newTabIcon"),
    tr("&New Tab"), Qt::ALT + Qt::Key_N, this, "fileNewTabAction");
  fileNewTabAction->setParentMenuText(tr("File"));
  connect(fileNewTabAction, SIGNAL(activated()), this, SLOT(newTab()));

  fileDeleteTabAction = new CAction(tr("Delete Tab"), getPixmapIcon("deleteTabIcon"),
    tr("&Delete Tab"), Qt::ALT + Qt::Key_D, this, "fileDeleteTabAction");
  fileDeleteTabAction->setParentMenuText(tr("File"));
  connect(fileDeleteTabAction, SIGNAL(activated()), this, SLOT(deleteTab()));

  fileNewWindowFromTabAction = new CAction(tr("New Window From Tab"), getPixmapIcon("newWindowIcon"),
    tr("New &Window From Tab"), Qt::ALT + Qt::Key_W, this, "fileNewWindowFromTabAction");
  fileNewWindowFromTabAction->setParentMenuText(tr("File"));
  connect(fileNewWindowFromTabAction, SIGNAL(activated()), this, SLOT(newWindowFromTab()));

  filePrintAction = new CAction(tr("Print"), getPixmapIcon("printIcon"), tr("&Print"),
    Qt::CTRL + Qt::Key_P, this, "filePrintAction");
  filePrintAction->setParentMenuText(tr("File"));
#ifdef QT_NO_PRINTER
  filePrintAction->setEnabled(false);
#else
  connect(filePrintAction, SIGNAL(activated()), this, SLOT(printQuery()));
#endif
  
  fileCloseAction = new QAction (tr("Close"), getPixmapIcon("closeIcon"),
    tr("&Close"), 0, this, "fileCloseAction");     
  connect(fileCloseAction, SIGNAL(activated()), this, SLOT(close()));
    
  editEraseAction = new CAction(tr("Clear Query"), getPixmapIcon("eraserIcon"),
    tr("C&lear Query"), Qt::CTRL + Qt::Key_L, this, "editEraseAction");
  editEraseAction->setParentMenuText(tr("Edit"));
  connect(editEraseAction, SIGNAL(activated()), this, SLOT(clearQuery()));
  
  editUndoAction = new CAction(tr("Undo"), getPixmapIcon("undoIcon"),
    tr("&Undo"), Qt::CTRL + Qt::Key_Z, this, "editUndoAction");
  editUndoAction->setParentMenuText(tr("Edit"));
  connect(editUndoAction, SIGNAL(activated()), this, SLOT(undoQuery()));
  
  editRedoAction = new CAction(tr("Redo"), getPixmapIcon("redoIcon"),
    tr("&Redo"), Qt::CTRL + Qt::Key_Y, this, "editRedoAction");
  editRedoAction->setParentMenuText(tr("Edit"));
  connect(editRedoAction, SIGNAL(activated()), this, SLOT(redoQuery()));
  
  editCutAction = new CAction(tr("Cut"), getPixmapIcon("cutIcon"),
    tr("Cu&t"), Qt::CTRL + Qt::Key_X, this, "editCutAction");
  editCutAction->setParentMenuText(tr("Edit"));
  connect(editCutAction, SIGNAL(activated()), this, SLOT(cutQuery()));
  
  editCopyAction = new CAction(tr("Copy"), getPixmapIcon("copyIcon"),
    tr("&Copy"), Qt::CTRL + Qt::Key_C, this, "editCopyAction");
  editCopyAction->setParentMenuText(tr("Edit"));
  connect(editCopyAction, SIGNAL(activated()), this, SLOT(copy()));
  
  editPasteAction = new CAction(tr("Paste"), getPixmapIcon("pasteIcon"),
    tr("&Paste"), Qt::CTRL + Qt::Key_V, this, "editPasteAction");
  editPasteAction->setParentMenuText(tr("Edit"));
  connect(editPasteAction, SIGNAL(activated()), this, SLOT(pasteQuery()));

  viewQueryTabsMenu = new Q3PopupMenu(this);

  CAction *viewQueryTabFirstAction = new CAction(tr("First"), getPixmapIcon("firstIcon"), tr("&First"),
    Qt::CTRL + Qt::Key_1, this, "viewQueryTabFirstAction");
  viewQueryTabFirstAction->setParentMenuText(tr("View | Query Tabs"));
  connect(viewQueryTabFirstAction, SIGNAL(activated()), query_tab, SLOT(first()));
  viewQueryTabFirstAction->addTo(viewQueryTabsMenu);

  CAction *viewQueryTabBackAction = new CAction(tr("Back"), getPixmapIcon("backIcon"), tr("&Back"),
    Qt::CTRL + Qt::Key_2, this, "viewQueryTabBackAction");
  viewQueryTabBackAction->setParentMenuText(tr("View | Query Tabs"));
  connect(viewQueryTabBackAction, SIGNAL(activated()), query_tab, SLOT(back()));
  viewQueryTabBackAction->addTo(viewQueryTabsMenu);

  CAction *viewQueryTabPreviousAction = new CAction(tr("Toggle Previous"), getPixmapIcon("toggleIcon"), tr("&Toggle Previous"),
    Qt::CTRL + Qt::Key_3, this, "viewQueryTabPreviousAction");
  viewQueryTabPreviousAction->setParentMenuText(tr("View | Query Tabs"));
  connect(viewQueryTabPreviousAction, SIGNAL(activated()), query_tab, SLOT(toggle()));
  viewQueryTabPreviousAction->addTo(viewQueryTabsMenu);
  Q3Accel *queryTabPreviousAccel = new Q3Accel(this);
  queryTabPreviousAccel->connectItem(queryTabPreviousAccel->insertItem(Qt::CTRL + Qt::Key_Space), query_tab, SLOT(toggle()));

  CAction *viewQueryTabNextAction = new CAction(tr("Next"), getPixmapIcon("nextIcon"), tr("&Next"),
    Qt::CTRL + Qt::Key_4, this, "viewQueryTabNextAction");
  viewQueryTabNextAction->setParentMenuText(tr("View | Query Tabs"));
  connect(viewQueryTabNextAction, SIGNAL(activated()), query_tab, SLOT(next()));
  viewQueryTabNextAction->addTo(viewQueryTabsMenu);

  CAction *viewQueryTabLastAction = new CAction(tr("Last"), getPixmapIcon("lastIcon"), tr("&Last"),
    Qt::CTRL + Qt::Key_5, this, "viewQueryTabLastAction");
  viewQueryTabLastAction->setParentMenuText(tr("View | Query Tabs"));
  connect(viewQueryTabLastAction, SIGNAL(activated()), query_tab, SLOT(last()));
  viewQueryTabLastAction->addTo(viewQueryTabsMenu);

  viewResultTabsMenu = new Q3PopupMenu(this);

  CAction *viewResultTabFirstAction = new CAction(tr("First"), getPixmapIcon("firstIcon"), tr("&First"),
    Qt::CTRL + Qt::Key_F7, this, "viewResultTabFirstAction");
  viewResultTabFirstAction->setParentMenuText(tr("View | Result Tabs"));
  connect(viewResultTabFirstAction, SIGNAL(activated()), results_tab, SLOT(first()));
  viewResultTabFirstAction->addTo(viewResultTabsMenu);

  CAction *viewResultTabBackAction = new CAction(tr("Back"), getPixmapIcon("backIcon"), tr("&Back"),
    Qt::CTRL + Qt::Key_F8, this, "viewResultTabBackAction");
  viewResultTabBackAction->setParentMenuText(tr("View | Result Tabs"));
  connect(viewResultTabBackAction, SIGNAL(activated()), results_tab, SLOT(back()));
  viewResultTabBackAction->addTo(viewResultTabsMenu);

  CAction *viewResultTabPreviousAction = new CAction(tr("Toggle Previous"), getPixmapIcon("toggleIcon"), tr("&Toggle Previous"),
    Qt::CTRL + Qt::Key_F9, this, "viewResultTabPreviousAction");
  viewResultTabPreviousAction->setParentMenuText(tr("View | Result Tabs"));
  connect(viewResultTabPreviousAction, SIGNAL(activated()), results_tab, SLOT(toggle()));
  viewResultTabPreviousAction->addTo(viewResultTabsMenu);
  /*
  QAccel *resultsTabPreviousAccel = new QAccel(this);
  resultsTabPreviousAccel->connectItem(resultsTabPreviousAccel->insertItem(Qt::SHIFT + Qt::Key_Space), results_tab, SLOT(toggle()));
  */

  CAction *viewResultTabNextAction = new CAction(tr("Next"), getPixmapIcon("nextIcon"), tr("&Next"),
    Qt::CTRL + Qt::Key_F10, this, "viewResultTabNextAction");
  viewResultTabNextAction->setParentMenuText(tr("View | Result Tabs"));
  connect(viewResultTabNextAction, SIGNAL(activated()), results_tab, SLOT(next()));
  viewResultTabNextAction->addTo(viewResultTabsMenu);

  CAction *viewResultTabLastAction = new CAction(tr("Last"), getPixmapIcon("lastIcon"), tr("&Last"),
    Qt::CTRL + Qt::Key_F11, this, "viewResultTabLastAction");
  viewResultTabLastAction->setParentMenuText(tr("View | Result Tabs"));
  connect(viewResultTabLastAction, SIGNAL(activated()), results_tab, SLOT(last()));
  viewResultTabLastAction->addTo(viewResultTabsMenu);

  viewSqlPanelAction = new CAction(tr("SQL Pane"), getPixmapIcon("sqlIcon"),
    tr("&SQL Pane"), Qt::Key_F2, this, "viewSqlPanelAction", true);
  viewSqlPanelAction->setParentMenuText(tr("View"));
  connect(viewSqlPanelAction, SIGNAL(activated()), this, SLOT(sqlPanelAction()));
  
  viewResultsPanelAction = new CAction(tr("Results Pane"), getPixmapIcon("gridIcon"),
    tr("&Results Pane"), Qt::Key_F3, this, "viewResultsPanelAction", true);
  viewResultsPanelAction->setParentMenuText(tr("View"));
  connect(viewResultsPanelAction, SIGNAL(activated()), this, SLOT(resultsPanelAction()));
  
  viewShowColumnsWindowAction = new CAction (tr("Show Columns Window"), getPixmapIcon("chooseFieldsIcon"),
    tr("Show Columns &Window"), Qt::CTRL + Qt::Key_W, this, "viewShowColumnsWindowAction", true);
  viewShowColumnsWindowAction->setParentMenuText(tr("View"));
  connect(viewShowColumnsWindowAction, SIGNAL(toggled(bool)), this, SLOT(showColumnsWindow(bool)));
  connect(columns_window, SIGNAL(visibilityChanged(bool)), viewShowColumnsWindowAction, SLOT(setOn(bool)));
  
  viewShowMessagesAction = new CAction (tr("Show Messages"), tr("Show &Messages"),
    Qt::CTRL + Qt::Key_M, this, "viewShowMessagesAction", true);
  viewShowMessagesAction->setParentMenuText(tr("View"));
  connect(viewShowMessagesAction, SIGNAL(toggled(bool)), this, SLOT(showMessages(bool)));
  
  queryExecuteAction = new CAction(tr("Execute"), getPixmapIcon("executeQueryIcon"),
    tr("&Execute"), Qt::CTRL + Qt::Key_E, this, "queryExecuteAction");
  queryExecuteAction->setParentMenuText(tr("Query"));
  connect(queryExecuteAction, SIGNAL(activated()), this, SLOT(executeQuery()));
  Q3Accel *a = new Q3Accel(this);
  a->connectItem(a->insertItem(Qt::CTRL + Qt::Key_Return), this, SLOT(executeQuery()));
  
  queryCancelAction = new CAction(tr("Cancel Execution and Clear Results"), getPixmapIcon("cancelQueryIcon"),
    tr("&Cancel"), Qt::CTRL + Qt::Key_Escape, this, "queryCancelAction");
  queryCancelAction->setParentMenuText(tr("Query"));
  connect(queryCancelAction, SIGNAL(activated()), this, SLOT(cancelQuery()));
  
  queryInsertRecordAction = new CAction(tr("Insert Record"), getPixmapIcon("insertRowIcon"),
    tr("&Insert Record"), Qt::CTRL + Qt::Key_I, this, "queryInsertRecordAction");
  queryInsertRecordAction->setParentMenuText(tr("Query"));
  connect(queryInsertRecordAction, SIGNAL(activated()), this, SLOT(insertRecord()));
  Q3Accel *insertRecordAccel = new Q3Accel(this);
  insertRecordAccel->connectItem(insertRecordAccel->insertItem(Qt::Key_Insert), this, SLOT(insertRecord()));
  
  queryDeleteRecordAction = new CAction(tr("Delete Record"), getPixmapIcon("deleteRowIcon"),
    tr("&Delete Record"), Qt::CTRL + Qt::Key_D, this, "queryDeleteRowAction");
  queryDeleteRecordAction->setParentMenuText(tr("Query"));
  connect(queryDeleteRecordAction, SIGNAL(activated()), this, SLOT(deleteRecord()));
  Q3Accel *deleteRecordAccel = new Q3Accel(this);
  deleteRecordAccel->connectItem(deleteRecordAccel->insertItem(Qt::Key_Delete), this, SLOT(deleteRecord()));
  
  optionsQueryWindowAction = new CAction(tr("General options for the Query Window"),
    tr("&Query Window Options"), Qt::CTRL + Qt::Key_Q, this, "optionsQueryWindowAction");
  optionsQueryWindowAction->setParentMenuText(tr("Options"));
  connect(optionsQueryWindowAction, SIGNAL(activated()), this, SLOT(queryWindowOptions()));
  
  optionsGeneralAction = new CAction(tr("General"), getPixmapIcon("hammerIcon"),
    tr("&General"), Qt::CTRL + Qt::Key_G, this, "optionsGeneralAction");
  optionsGeneralAction->setParentMenuText(tr("Options"));
  connect(optionsGeneralAction, SIGNAL(activated()), this, SLOT(generalOptions()));
  
  
  Q3PopupMenu * fileMenu = new Q3PopupMenu(this, "fileMenu");
  menuBar()->insertItem(tr("&File"), fileMenu);
  
  Q3PopupMenu * editMenu = new Q3PopupMenu(this, "editMenu");
  menuBar()->insertItem(tr("&Edit"), editMenu);
  
  Q3PopupMenu * viewMenu = new Q3PopupMenu(this, "viewMenu");
  connect(viewMenu, SIGNAL(aboutToShow()), this, SLOT(viewMenuAboutToShow()));
  menuBar()->insertItem(tr("&View"), viewMenu);
  
  queryMenu = new Q3PopupMenu(this, "queryMenu");
  menuBar()->insertItem(tr("&Query"), queryMenu);
  
  Q3PopupMenu * optionsMenu = new Q3PopupMenu(this, "optionsMenu");
  menuBar()->insertItem(tr("&Options"), optionsMenu);
  
  Q3ToolBar * fileToolBar = new Q3ToolBar(tr("File Bar"), this);
  
  queryTypesMenu = new Q3PopupMenu(this, "queryTypesMenu");
  
  queryTypesMenu->insertItem(getPixmapIcon("selectQueryIcon"), tr("Select Query"), 1);
  queryTypesMenu->insertItem(getPixmapIcon("updateQueryIcon"), tr("Update Query"), 2);
  queryTypesMenu->insertItem(getPixmapIcon("appendQueryIcon"), tr("Insert Query"), 3);
  queryTypesMenu->insertItem(getPixmapIcon("deleteQueryIcon"), tr("Delete Query"), 4);
  
  queryTypesMenu->connectItem(1,this, SLOT(selectQuery()));
  queryTypesMenu->connectItem(2,this, SLOT(updateQuery()));
  queryTypesMenu->connectItem(3,this, SLOT(insertQuery()));
  queryTypesMenu->connectItem(4,this, SLOT(deleteQuery()));
  
  
  fileOpenAction->addTo(fileMenu);
  fileOpenAction->addTo(fileToolBar);
  
  fileMenu->insertItem(getPixmapIcon("saveIcon"), tr("Save"), saveTypesMenu);
  
  saveTypeButton = new QToolButton(fileToolBar);
  saveTypeButton->setPopup(saveTypesMenu);
  saveTypeButton->setPixmap(getPixmapIcon("saveIcon"));
  saveTypeButton->setTextLabel(tr("Save"), true);
  saveTypeButton->setPopupDelay(0);

  fileMenu->insertSeparator();
  fileToolBar->addSeparator();

  fileNewTabAction->addTo(fileMenu);
  fileDeleteTabAction->addTo(fileMenu);
  fileNewWindowFromTabAction->addTo(fileMenu);

  fileNewTabAction->addTo(fileToolBar);
  fileDeleteTabAction->addTo(fileToolBar);
  fileNewWindowFromTabAction->addTo(fileToolBar);

  fileMenu->insertSeparator();
  fileToolBar->addSeparator();
  filePrintAction->addTo(fileMenu);
  filePrintAction->addTo(fileToolBar);
  fileMenu->insertSeparator();
  fileCloseAction->addTo(fileMenu);
  
  Q3ToolBar * editToolBar = new Q3ToolBar(tr("Edit Bar"), this);

  editEraseAction->addTo(editMenu);
  editEraseAction->addTo(editToolBar);
  editMenu->insertSeparator();
  editToolBar->addSeparator();
  editUndoAction->addTo(editMenu);
  editUndoAction->addTo(editToolBar);
  editRedoAction->addTo(editMenu);
  editRedoAction->addTo(editToolBar);
  editMenu->insertSeparator();
  editToolBar->addSeparator();
  editCutAction->addTo(editMenu);
  editCutAction->addTo(editToolBar);
  editCopyAction->addTo(editMenu);
  editCopyAction->addTo(editToolBar);
  editPasteAction->addTo(editMenu);
  editPasteAction->addTo(editToolBar);
  
  Q3ToolBar * viewToolBar = new Q3ToolBar(tr("View Bar"), this);

  viewMenu->insertItem(tr("Query Tabs"), viewQueryTabsMenu);
  viewMenu->insertItem(tr("Result Tabs"), viewResultTabsMenu);
  viewMenu->insertSeparator();
  viewShowColumnsWindowAction->addTo(viewMenu);
  viewMenu->insertSeparator();
  viewSqlPanelAction->addTo(viewMenu);
  viewSqlPanelAction->addTo(viewToolBar);
  viewResultsPanelAction->addTo(viewMenu);
  viewResultsPanelAction->addTo(viewToolBar);
  
  viewToolBar->addSeparator();
  viewShowColumnsWindowAction->addTo(viewToolBar);
  viewMenu->insertSeparator();
  viewShowMessagesAction->addTo(viewMenu);
  
  Q3ToolBar * queryToolBar = new Q3ToolBar(tr("Query Bar"), this);
  queryExecuteAction->addTo(queryMenu);
  queryExecuteAction->addTo(queryToolBar);
  queryCancelAction->addTo(queryMenu);
  queryCancelAction->addTo(queryToolBar);
  
  queryMenu->insertSeparator();
  queryToolBar->addSeparator();
  
  query_type_item_id = queryMenu->insertItem(getPixmapIcon("queryIcon"), tr("Query Type"), queryTypesMenu);
  queryTypeButton = new QToolButton(queryToolBar);
  queryTypeButton->setPopup(queryTypesMenu);
  queryTypeButton->setPixmap(getPixmapIcon("queryIcon"));
  queryTypeButton->setTextLabel(tr("Query Type"), true);
  queryTypeButton->setPopupDelay(0);
  
  queryToolBar->addSeparator();
  queryMenu->insertSeparator();
  
  queryInsertRecordAction->addTo(queryToolBar);
  queryInsertRecordAction->addTo(queryMenu);
  
  queryDeleteRecordAction->addTo(queryToolBar);  
  queryDeleteRecordAction->addTo(queryMenu);  

  optionsQueryWindowAction->addTo(optionsMenu);
  optionsMenu->insertSeparator();
  optionsGeneralAction->addTo(optionsMenu);
  
  editUndoAction->setEnabled(false);
  editRedoAction->setEnabled(false);
  queryDeleteRecordAction->setEnabled(false);
  queryInsertRecordAction->setEnabled(false);  
  
  history_panel = new CHistoryPanel(tr("History"));
  messageWindow()->addPanel(history_panel);
  history_panel->setMaxDisplaySize(259);
  history_panel->setMessagePanel(messagePanel());  
  reloadHistoryList();
  
  connect(history_panel, SIGNAL(executeQuery(const QString &)), this, SLOT(doExecuteQuery(const QString &)));
  connect(history_panel, SIGNAL(doubleClicked(const QString &)), this, SLOT(setQuery(const QString &)));
  connect(history_panel, SIGNAL(reloadHistoryList()), this, SLOT(reloadHistoryList()));
  
  setCaption("[" + mysql()->connectionName() + "] " + tr("Query Window"));
  new CHotKeyEditorMenu(this, menuBar(), "HotKeyEditor");
  
  myResize(600, 400);
  
  explain_panel = new CExplainQueryPanel(0);
  explain_panel->setCaption(tr("Explain"));
  messageWindow()->addPanel(explain_panel);
  messageWindow()->setTabEnabled(explain_panel, false);
  
  showQueryPanel((display & SQL_PANEL) == SQL_PANEL);
  showResultsPanel((display & RESULTS_PANEL) == RESULTS_PANEL);
  
  closeTimer = new QTimer(this);
  connect(closeTimer, SIGNAL(timeout()), this, SLOT(tryClose()));  

  CQuerySet * s = createNewQuerySet(query_type);
  if (s)
    s->queryEditor()->setFocus();
  qApp->processEvents();

  connect(query_tab, SIGNAL(currentChanged(QWidget *)), this, SLOT(queryTabChanged(QWidget *)));
  connect(results_tab, SIGNAL(currentChanged(QWidget *)), this, SLOT(resultsTabChanged(QWidget *)));
}

CQueryWindow::~CQueryWindow()
{
  myApp()->decCritical();
  delete mysql_server;
}

void CQueryWindow::doExecuteQuery(const QString &qry)
{
#ifdef DEBUG
  qDebug("CQueryWindow::setDatabase('%s')", debug_string(qry));
#endif

  setQuery(qry);  
  executeQuery();
}

void CQueryWindow::setQuery(const QString &s)
{
#ifdef DEBUG
  qDebug("CQueryWindow::setQuery('%s')", debug_string(s));
#endif

  CQuerySet *q = currentQuerySet();
  if (q)
  {
    q->queryEditor()->setText(s);
    if (!s.isEmpty())
      queryExecuteAction->setEnabled(true);
  }
}

void CQueryWindow::setDatabase(const QString &s)
{
  default_database = s;

  if (querySetList.isEmpty())
    return;

  for (CQuerySet *q = querySetList.first(); q; q = querySetList.next())
    q->setDefaultDatabase(s);
}

void CQueryWindow::setTable(const QString &s)
{
  default_table = s;

  if (querySetList.isEmpty())
    return;

  for (CQuerySet *q = querySetList.first(); q; q = querySetList.next())
    q->setDefaultTable(s);
}


void CQueryWindow::newWindowFromTab()
{
  CQuerySet *q = currentQuerySet();
  if (q)
    newQueryWindow(q->queryEditor()->text(), q->defaultDatabase(), q->defaultTable());
}

void CQueryWindow::reloadHistoryList()
{
#ifdef DEBUG
  qDebug("CQueryWindow::reloadHistoryList()");
#endif

  history_panel->clear();
  CHistoryView::historyView()->loadTo(history_panel);
}

CQuerySet * CQueryWindow::createNewQuerySet(int q, const QString &d, const QString &t)
{
  CQuerySet *qry_set = new CQuerySet(this, ++tab_number, q ? q : query_type, d.isNull() ? default_database : d, t.isNull() ? default_table : t);
  if (qry_set)
  {
    querySetList.append(qry_set);
    return qry_set;
  }
  return 0;
}

CQuerySet * CQueryWindow::currentQuerySet()
{
  if (querySetList.isEmpty())
    return 0;
  CQuerySet *q;
  for (q = querySetList.first(); q; q = querySetList.next())
    if (queryTab()->currentPage() == q->queryEditor())
      return q;
  return 0;
}

CQuerySet * CQueryWindow::currentResultSet()
{
  if (querySetList.isEmpty())
    return 0;
  CQuerySet *q;
  for (q = querySetList.first(); q; q = querySetList.next())
    if (resultsTab()->currentPage() == q->resultsTable())
      return q;
  return 0;
}

void CQueryWindow::newTab()
{
  bool b = querySetList.isEmpty();
  CQuerySet *q = createNewQuerySet();
  if (q)
  {
    queryTab()->mySetCurrentPage(queryTab()->indexOf(q->queryEditor()));
    fileDeleteTabAction->setEnabled(true);
    if (b)
      queryTabChanged(q->queryEditor());

    if (resultsTab()->currentPage() == q->resultsTable())
    {
      read_only_label->setEnabled(q->resultsTable()->isReadOnly());      
      messageWindow()->setTabEnabled(explain_panel, false);
    }
  }
}

void CQueryWindow::setMenuItemsEnabled(bool b)
{
  fileDeleteTabAction->setEnabled(b);
  fileNewWindowFromTabAction->setEnabled(b);
#ifdef QT_NO_PRINTER
  filePrintAction->setEnabled(false);
#else
  filePrintAction->setEnabled(b);
#endif
  saveTypesMenu->setEnabled(b);
  saveTypeButton->setEnabled(b);
  viewQueryTabsMenu->setEnabled(b);
  viewResultTabsMenu->setEnabled(b);
  queryExecuteAction->setEnabled(b);
  enableQueryTypes(b);
  editPasteAction->setEnabled(b);
  editRedoAction->setEnabled(b);
  editUndoAction->setEnabled(b);
  editEraseAction->setEnabled(b);
  editCutAction->setEnabled(b);
  editCutAction->setEnabled(b);
}

void CQueryWindow::saveToClipboard()
{
#ifndef QT_NO_CLIPBOARD
  CQuerySet *q = currentQuerySet();
  if (q)
  {
    QString str = q->queryEditor()->text().stripWhiteSpace();    
    if (q->resultsTable()->numRows() > 0 && q->resultsTable()->numCols() > 0)
    {
      str += "\n\n";
      q->resultsTable()->setForceCopyAll(true);
      str += q->resultsTable()->copy_data(0, 0);
      q->resultsTable()->setForceCopyAll(false);
    }
    QApplication::clipboard()->setText(str);
  }
#endif
}

void CQueryWindow::deleteTab()
{
  CQuerySet *q = currentQuerySet();
  if (q)
    if (!q->isBusy() && !q->isCanceling())
    {
      if (myApp()->confirmCritical())
        if ((QMessageBox::information(0, tr("Confirm Close"), tr("Are you sure you want to close this Query Tab ?"), tr("&Yes"),tr("&No")) != 0))
          return;
      cancelQuery();
      resultsTab()->removePage(q->resultsTable());
      queryTab()->removePage(q->queryEditor());
      fifo.remove(q);
      querySetList.remove(q);  //AutoDelete
      if (querySetList.isEmpty())
      {
        setMenuItemsEnabled(false);
        messageWindow()->setTabEnabled(explain_panel, false);        
      }
    }
    else
      messagePanel()->information("Can't close Query Tab;  Query Tab is busy.");
}

CQuerySet * CQueryWindow::callEditorSlot(void (CSqlEditor::*slot)(void))
{
#ifdef DEBUG
  qDebug("CQueryWindow::callEditorMember(void (CSqlEditor::*)(void))");
#endif

  CQuerySet *q = currentQuerySet();
  if (q)
  {
    (q->queryEditor()->*slot)();
    return q;
  }
  return 0;
}

void CQueryWindow::saveQuery()
{
  callEditorSlot(&CSqlEditor::save);
}

void CQueryWindow::saveResults()
{
  CQuerySet *q = currentResultSet();
  if (q)
    q->resultsTable()->save();
}

void CQueryWindow::printQuery()
{
  callEditorSlot(&CSqlEditor::print);
}

void CQueryWindow::clearQuery()
{
  callEditorSlot(&CSqlEditor::clear);
}

void CQueryWindow::openFile()
{
  if (querySetList.isEmpty())
    newTab();
  callEditorSlot(&CSqlEditor::openFile);
}

void CQueryWindow::undoQuery()
{
  CQuerySet *q = callEditorSlot(&CSqlEditor::undo);
  if (q)
    q->undoAvailable();
}

void CQueryWindow::redoQuery()
{
  CQuerySet *q = callEditorSlot(&CSqlEditor::redo);
  if (q)
    q->redoAvailable();
}

void CQueryWindow::cutQuery()
{
  CQuerySet *q = currentQuerySet();
  if (!q)
    return;

  if (queryTab()->isVisible() && q->queryEditor()->hasFocus() && q->queryEditor()->hasSelectedText())
  {
    q->queryEditor()->cut();
    q->cutCopyAvailable();
  }
}

void CQueryWindow::copy()
{
  CQuerySet *q = currentQuerySet();
  if (!q)
    return;

  if (queryTab()->isVisible() && q->queryEditor()->hasFocus() && q->queryEditor()->hasSelectedText())
  {
    q->queryEditor()->copy();
    q->cutCopyAvailable();
    return;
  }

  q = currentResultSet();
  if (!q)
    return;
  else
    if (resultsTab()->isVisible() && q->resultsTable()->hasFocus() && q->resultsTable()->numSelections() > 0
      && q->resultsTable()->numCols() > 0)
    {
      q->resultsTable()->copy(q->resultsTable()->currentRow(), q->resultsTable()->currentColumn());
      q->cutCopyAvailable();
    }
}

void CQueryWindow::pasteQuery()
{
  callEditorSlot(&CSqlEditor::paste);
}

void CQueryWindow::executeQuery()
{
  CQuerySet *q = currentQuerySet();    
  if (q && queryExecuteAction->isEnabled())
    if (!q->isBusy())
      q->executeQuery();
}

void CQueryWindow::cancelQuery()
{
  CQuerySet *q = currentQuerySet();
  if (q)
    q->cancelQuery();
}

void CQueryWindow::insertRecord()
{
  CQuerySet *q = currentResultSet();
  if (q)
    q->resultsTable()->insertRecord();
}

void CQueryWindow::deleteRecord()
{
  CQuerySet *r = currentResultSet();
  if (r)
    r->resultsTable()->deleteRecord();
  else
  {
    CQuerySet *q = currentQuerySet();
    if (q)
      if (q->queryEditor()->hasFocus())
        q->queryEditor()->doKeyboardAction(Q3TextEdit::ActionDelete);
  }
}

void CQueryWindow::queryWindowOptions()
{
  CQueryWindowOptionsDialog *w = new CQueryWindowOptionsDialog(myApp()->workSpace(), mysql());
  connect(w, SIGNAL(config_changed()), this, SLOT(refreshOptions()));  
  connect(this, SIGNAL(about_to_close()), w, SLOT(close()));
  myShowWindow(w);
}

void CQueryWindow::refreshOptions()
{
  CConfig *cfg = new CConfig(mysql()->connectionName(), mysql()->connectionsPath());

  query_new_tab = strtobool(cfg->readStringEntry("Query New Tab", "false"));
  follow_query_tab = strtobool(cfg->readStringEntry("Follow Query Tab", "false"));
  follow_results_tab = strtobool(cfg->readStringEntry("Follow Results Tab", "true"));
  max_results_save = cfg->readNumberEntry("Max Results to Save", 10);
  multiple_queries_opt = cfg->readNumberEntry("Multiple Query Options", 0);

  CQueryWindowOptionsDialog::query_config_options opt(CQueryWindowOptionsDialog::readConfig(cfg));
  bool ret = true;
  CQuerySet *qry_set;
  for (qry_set = querySetList.first(); qry_set; qry_set = querySetList.next())
    ret &= qry_set->refreshQueryOptions(cfg, &opt);
  if (!ret)
    cfg->save();
  delete cfg;
}


void CQueryWindow::enableQueryTypes(bool b)
{
#ifdef DEBUG
  qDebug("CQueryWindow::enableQueryTypes(%s)", debug_string(booltostr(b)));
#endif

  queryTypeButton->setEnabled(b);
  queryMenu->setItemEnabled(query_type_item_id, b);
}

void CQueryWindow::showQueryPanel(bool s)
{
#ifdef DEBUG
  qDebug("CQueryWindow::showQueryPanel(%s)", debug_string(booltostr(s)));
#endif

  if (s)
    query_tab->show();
  else
    query_tab->hide();
  viewSqlPanelAction->setOn(s);
  viewResultsPanelAction->setEnabled(s);
}

void CQueryWindow::showResultsPanel(bool s)
{
#ifdef DEBUG
  qDebug("CQueryWindow::showResultsPanel(%s)", debug_string(booltostr(s)));
#endif

  if (s)
    results_tab->show();  
  else  
    results_tab->hide();      
  viewResultsPanelAction->setOn(s);
  viewSqlPanelAction->setEnabled(s);
}

void CQueryWindow::sqlPanelAction()
{
#ifdef DEBUG
  qDebug("CQueryWindow::sqlPanelAction()");
#endif
  
  CQuerySet *q = currentQuerySet();
  if (!q)
    return;

  showQueryPanel(!queryTab()->isVisible());
  queryTabChanged(q->queryEditor());
}


void CQueryWindow::resultsPanelAction()
{
#ifdef DEBUG
  qDebug("CQueryWindow::resultsPanelAction()");
#endif  

  showResultsPanel(!resultsTab()->isVisible());
  if (resultsTab()->isVisible())
  {
    CQuerySet *r = currentResultSet();
    if (!r)
      return;
    queryDeleteRecordAction->setEnabled(!r->resultsTable()->isReadOnly());
    queryInsertRecordAction->setEnabled(!r->resultsTable()->isReadOnly());
  }
  else
  {
    queryDeleteRecordAction->setEnabled(false);
    queryInsertRecordAction->setEnabled(false);
  }
}

void CQueryWindow::showColumnsWindow(bool b)
{
#ifdef DEBUG
  qDebug("CQueryWindow::showColumnsWindow(%s)", debug_string(booltostr(b)));
#endif

  if (b)
    columns_window->show();
  else
    columns_window->hide();
  viewShowColumnsWindowAction->setOn(b);
}

void CQueryWindow::showMessages(bool b)
{
#ifdef DEBUG
  qDebug("CQueryWindow::showMessages(%s)", debug_string(booltostr(b)));
#endif

  if (b)
    messageWindow()->show();
  else
    messageWindow()->hide();  
}

void CQueryWindow::generalOptions()
{
  CAppConfigDialog *config = new CAppConfigDialog(myApp()->workSpace());

  CQuerySet *q;
  for (q = querySetList.first(); q; q = querySetList.next())
  {
    CSqlEditor *e = q->queryEditor();
    connect(config, SIGNAL(okClicked()), e, SLOT(refreshFonts()));
  }
  connect(this, SIGNAL(about_to_close()), config, SLOT(close()));
  config->setCurrentPage(3);
  myShowWindow(config);
}


void CQueryWindow::resultsTabChanged(QWidget *w)
{  
  CQuerySet *q = currentResultSet();
  if (!q)
    return;

  if (q->resultsTable() == w)
  {
    read_only_label->setEnabled(q->resultsTable()->isReadOnly());
    columnsWindow()->setTable((Q3Table *) q->resultsTable());
    explain_panel->setData(q->explainRows(), q->explainCols(), q->explainData());
    messageWindow()->setTabEnabled(explain_panel, q->explainCols() > 0 && !q->explainData().isEmpty());
  }

  if (block_results_tab_changed)
    return;  

  if (follow_query_tab && !disable_follow)
  {
    block_query_tab_changed = true;
    queryTab()->mySetCurrentPage(resultsTab()->indexOf(w));
    block_query_tab_changed = false;
  }  
}

void CQueryWindow::queryTabChanged(QWidget *w)
{
  CQuerySet *q = currentQuerySet();
  if (!q)
    return;

  bool ena = !q->isBusy();

  if (q->queryEditor() == w)
  {
    executing_query_label->setEnabled(!ena);
    queryExecuteAction->setEnabled(ena && !q->queryEditor()->text().isEmpty());
  }

  if (block_query_tab_changed)
    return;  

  if (follow_results_tab && !disable_follow)
  {
    block_results_tab_changed = true;
    resultsTab()->mySetCurrentPage(queryTab()->indexOf(w));    
    block_results_tab_changed = false;
  }  

#ifdef QT_NO_PRINTER
  filePrintAction->setEnabled(false);
#else
  filePrintAction->setEnabled(ena);
#endif  
  
  fileSaveResultsAction->setEnabled(ena);
  saveTypesMenu->setEnabled(ena);
  fileOpenAction->setEnabled(ena);
  editPasteAction->setEnabled(ena && queryTab()->isVisible());  
  editRedoAction->setEnabled(ena && queryTab()->isVisible() && q->queryEditor()->isRedoAvailable());
  editUndoAction->setEnabled(ena && queryTab()->isVisible() && q->queryEditor()->isUndoAvailable());
  queryCancelAction->setEnabled(!q->isCanceling() && (!ena || q->resultsTable()->numCols() > 0));
  
  editEraseAction->setEnabled(ena && queryTab()->isVisible() && !q->queryEditor()->text().isEmpty());
  fileCloseAction->setEnabled(ena);

  editCutAction->setEnabled(ena && queryTab()->isVisible() && q->queryEditor()->hasSelectedText());
#ifdef QT_NO_CLIPBOARD
  editCopyAction->setEnabled(false);
#else
  editCopyAction->setEnabled(ena && queryTab()->isVisible() && q->queryEditor()->hasSelectedText());
#endif

  queryTypeButton->setEnabled(ena && queryTab()->isVisible() && !q->defaultTable().isEmpty());
  queryMenu->setItemEnabled(query_type_item_id, ena && queryTab()->isVisible() && !q->defaultTable().isEmpty());

  saveTypeButton->setEnabled(ena);

  q->queryEditor()->setReadOnly(!ena);
  
  messagePanel()->setEnableBottomItem(ena);
  historyPanel()->setEnableBottomItem(ena);
  myApp()->consoleWindow()->sqlDebugPanel()->setEnableBottomItem(ena);
}

void CQueryWindow::viewMenuAboutToShow()
{
#ifdef DEBUG
  qDebug("CQueryWindow::viewMenuAboutToShow()");
#endif

  viewShowColumnsWindowAction->setOn(columns_window->isVisible());
  viewShowMessagesAction->setOn(messageWindow()->isVisible());
}

void CQueryWindow::tryClose()
{
  close_timer = true;
  messagePanel()->information(tr("Trying to close window."));
  messagePanel()->setBottomItem();
  close();
}

void CQueryWindow::freeResults()
{
  if (maxResultsSave() <= 0 || fifo.isEmpty() || fifo.count() < (uint) maxResultsSave())  //unlimited tabs
    return;

  int del = fifo.count() - maxResultsSave() + 1;

  if (del <= 0)
    return;

  int cnt = 0;  

  for (CQuerySet *t = fifo.first(); t; t = fifo.next())
    if (!t->isBusy() && !t->isCanceling())
    {
       t->query()->freeResult();
       t->resultsTable()->reset();
       messagePanel()->information(tr("Automatically freed results for") + " " + t->resultsLabel());
       fifo.removeRef(t);
       if (++cnt >= del)
         break;
    }
}

void CQueryWindow::closeEvent(QCloseEvent * e)
{
#ifdef DEBUG
  qDebug("CQueryWindow::closeEvent()");
#endif  

  if (doing_close && !close_timer)
  {
#ifdef QT_THREAD_SUPPORT
    if (myApp()->confirmCritical())
      if ((QMessageBox::warning(0, tr("Confirm Close"), tr("Are you sure you want to kill all Query threads ?"), tr("&Yes"),tr("&No")) != 0))
      {
        e->ignore();
        return;
      }

    for (CQuerySet *t = querySetList.first(); t; t = querySetList.next())
      if (t->isBusy() || t->isCanceling())
        t->killThread();
#endif
    
    beforeClose();
    messageWindow()->close();
    emit about_to_close();
    e->accept();
    return;
  }

  doing_close = true;
  bool is_busy = false;
  bool is_first = true;

  CQuerySet *t;
  for (t = querySetList.first(); t; t = querySetList.next())
  {
    if (t->isBusy() && !t->isCanceling() && !close_timer)
    {
      if (is_first)
      {
        if (myApp()->confirmCritical())
          if ((QMessageBox::information(0, tr("Confirm Cancel"), tr("Are you sure you want to cancel all Queries ?"), tr("&Yes"),tr("&No")) != 0))
          {            
            doing_close = false;
            e->ignore();
            return;
          }
        cancel_all_queries = true;
        is_busy = true;
        is_first = false;
      }
      t->cancelQuery();
    }
    else
      if (t->isBusy() || t->isCanceling() && close_timer)
        is_busy = true;
  }
  close_timer = false;
  if (!is_busy)
  {
    emit about_to_close();
    beforeClose();
    messageWindow()->close();
    e->accept();
  }
  else
  {
    messagePanel()->information(tr("Waiting for Query Threads to terminate."));
    messagePanel()->setBottomItem();
    closeTimer->start(5000);
    e->ignore();
  }
}

void CQueryWindow::newQueryWindow(const QString &query_str, const QString &database, const QString &table)
{
  CQueryWindow *w = new CQueryWindow(myApp()->workSpace(), mysql()->connectionName(), SQL_QUERY, SQL_PANEL | RESULTS_PANEL);
  myShowWindow(w);
  if (!database.isEmpty())
    w->setDatabase(database);
  if (!table.isEmpty())
    w->setTable(table);
  if (!query_str.isEmpty())
    w->doExecuteQuery(query_str);  
}

void CQueryWindow::selectQuery()
{  
  CQuerySet *q = currentQuerySet();
  if (q)
  {
    showQueryPanel(true);
    q->queryEditor()->setText("SELECT *\nFROM " + q->defaultTable());
  }
}

void CQueryWindow::updateQuery()
{
  CQuerySet *q = currentQuerySet();
  if (q)
  {
    showQueryPanel(true);
    q->queryEditor()->setText("UPDATE " + q->defaultTable() + "\nSET [" + tr("Field1") + " = " + tr("Value1") + ",...N],");
  }
}

void CQueryWindow::insertQuery()
{
  CQuerySet *q = currentQuerySet();
  if (q)
  {
    showQueryPanel(true);
    q->queryEditor()->setText("INSERT INTO " + q->defaultTable() + "\n([" + tr("Field1") + ",...N])\nVALUES([" + tr("Value1") + ",...N])");
  }
}

void CQueryWindow::deleteQuery()
{
  CQuerySet *q = currentQuerySet();
  if (q)
  {
    showQueryPanel(true);
    q->queryEditor()->setText("DELETE\nFROM " + q->defaultTable() + " WHERE ");
  }
}

CQueryWindowTab::CQueryWindowTab(QWidget * parent, const char * name, Qt::WFlags f)
: QTabWidget(parent, name, f), tab_toggle(false)
{
  last_tab_idx = 0;
  current_idx = 0;
  connect(this, SIGNAL(currentChanged(QWidget *)), this, SLOT(CurrentChanged(QWidget *)));
}

void CQueryWindowTab::CurrentChanged(QWidget *)
{
  if (!tab_toggle)
  {
    last_tab_idx = current_idx;
    current_idx = currentPageIndex();
  }
}

void CQueryWindowTab::mySetCurrentPage(int t)
{
  int tmp = currentPageIndex();
  current_idx = t;
  tab_toggle = true;
  setCurrentPage(t);
  if (currentPage())
    currentPage()->setFocus();
  last_tab_idx = tmp;
  tab_toggle = false;
}

void CQueryWindowTab::first()
{
  mySetCurrentPage(0);
}

void CQueryWindowTab::back()
{
  if (currentPageIndex() - 1 >= 0)
    mySetCurrentPage(currentPageIndex() - 1);  
  else
    last();
}

void CQueryWindowTab::toggle()
{
  mySetCurrentPage(last_tab_idx);
}

void CQueryWindowTab::next()
{
  if (currentPageIndex() + 1 > count() - 1)
    first();
  else
    mySetCurrentPage(currentPageIndex() + 1);
}

void CQueryWindowTab::last()
{
  mySetCurrentPage(count() - 1);
}
