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
#include "CAdministrationWindow.h"
#include "CConfig.h"
#include "config.h"
#include "globals.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "CMySQL.h"
#include "panels.h"
#include "CTableFieldChooser.h"
#include "CHotKeyEditorDialog.h"
#include "CQueryWindow.h"
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qdatetime.h>

CProcessListTable::CProcessListTable (QWidget * parent, CMySQLServer *m)
: CQueryTable(parent, 0, m, "CProcessListTable")
{
  bool notnew = m->mysql()->version().major <= 3 && m->mysql()->version().minor <= 22;

  sql = "SHOW ";
  if (!notnew)
    sql += "FULL ";
  sql += "PROCESSLIST";

  setKeepColumnWidth(true);
  qry = new CMySQLQuery(m->mysql());
  qry->setEmitMessages(false);
  setQuery(qry);

  columnsWindow = new CTableFieldChooser(0, this, "CProcessListTableColumnsWindow");
  setCaption(tr("Process List"));
  columnsWindow->setCaption("[" + m->connectionName() + "] " + tr("Process List Columns"));
  verticalHeader()->hide();
  setFocusStyle(QTable::FollowStyle);
  setSelectionMode(QTable::MultiRow);
  setLeftMargin(0);
  is_first = true;
  setReadOnly(false);
  refresh();
}

CProcessListTable::~CProcessListTable()
{
  delete qry;
  delete columnsWindow;
}

void CProcessListTable::ContextMenuRequested(int row, int col, const QPoint &pos)
{
  if (isBlocked())
    return;

  QPopupMenu *menu = new QPopupMenu();
  
    
  menu->insertItem(getPixmapIcon("chooseFieldsIcon"), tr("Choose Fields"), MENU_COLUMNS_WINDOW);  
  menu->insertSeparator();

  menu->insertItem(getPixmapIcon("copyIcon"), tr("Copy"), MENU_COPY);

  menu->insertSeparator();
  menu->setItemEnabled(MENU_COPY, numRows() > 0 && numCols() > 0);

  menu->insertItem(getPixmapIcon("killProcessIcon"), "Kill Process", MENU_KILL_PROCESS);
  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("saveIcon"), "Save", MENU_SAVE);
  menu->insertSeparator();  
  menu->insertItem(getPixmapIcon("refreshTablesIcon"), "Refresh", MENU_REFRESH);
  
  int res = menu->exec(pos);
  delete menu;
  
  switch (res)
  {
    case MENU_COLUMNS_WINDOW:
      columnsWindow->show();
      columnsWindow->raise();
      columnsWindow->setFocus();
    break;

    case MENU_COPY:
      copy(row, col);
      break;

    case MENU_REFRESH:
      refresh();
      break;
    
    case MENU_SAVE:
      save();
      break;
    
    case MENU_KILL_PROCESS:
      {
        bool ok = false;
        for (int current_row = 0; current_row < numRows(); current_row++)
        {
          if (!isRowSelected(current_row))
            continue;
          if (mysql()->mysql()->mysqlKill(text(current_row,0).toLong()))
          {
            mysql()->messagePanel()->information(tr("Process killed successfully") + " :" +  text(current_row,0));
            ok = true;
          }
        }
        if (ok)
          refresh();
      }
      break;
  }
}


void CProcessListTable::copy_data_func(QString *cpy, CMySQLQuery *qry, QTableSelection *sel, QMap<uint, ulong> *max_length_map)
{
  uint length;
  QString tmp;
  qry->dataSeek(0);
  for (int current_row = 0; current_row < numRows(); current_row++)
  {
    qry->next();
    if (!isRowSelected(current_row))
      continue;    
    *cpy += "|";
    for (int current_col = sel->leftCol(); current_col <= sel->rightCol(); current_col++)
    {
      if (horizontalHeader()->sectionSize(current_col) <= 0)
        continue;
      const char *str= query()->row(current_col) ? query()->row(current_col) : NULL_TEXT;
      length = (*max_length_map)[current_col];
      
      if (length > MAX_COLUMN_LENGTH)
      {
        *cpy += str;
        *cpy += "|";
      }
      else
      {
        tmp.sprintf(IS_NUM(qry->fields(current_col).type) ? "%*s |" : " %-*s|", length, str);
        *cpy += tmp;
      }
    }
    *cpy += "\n";
  }
}

void CProcessListTable::DoubleClicked(int row, int col, int button, const QPoint &)
{
  if (isBlocked() || button != 1 || col < 1)
    return;
  QString txt = text(row, 7);
  if (txt.isEmpty() || txt.isNull())
    return;

  CQueryWindow *w = new CQueryWindow(myApp()->workSpace(), mysql(), CQueryWindow::QUERY, SQL_PANEL | RESULTS_PANEL);
  w->setQuery(txt);
  w->setFocus();
  w->raise();
  myShowWindow(w);
}

void CProcessListTable::refresh()
{
  if (isBlocked())
    return;

  setBlocked(true);

  bool max_scroll = false;
  int scroll = is_first ? 0 : verticalScrollBar()->value();
  if (scroll >= verticalScrollBar()->maxValue() - 1 && scroll != 0)
    max_scroll = true;    

  if (keepColumnWidth() && numCols() > 0)
    for (int i = 0; i < numCols(); i++)
      previous_columns_map.insert(i, CTable::OldColumn(horizontalHeader()->sectionSize(i), horizontalHeader()->label(i)));

  clearCellWidget(currentRow(), currentColumn());
  if (is_first)
    ensureVisible(0, 0);  
  query()->freeResult();
  setNumRows(0);
  horizontalHeader()->setSortIndicator( -1 );  
  if (query()->exec(sql))
  {
    uint num_rows = qry->numRows();
    uint num_fields = qry->numFields();

    bool columns_ok = (keepColumnWidth() && previous_columns_map.count() == num_fields);
    if (columns_ok)
      for (uint i = 0; i < num_fields; i++)
        columns_ok &= (previous_columns_map[i].label == query()->fields(i).name);

    if (!columns_ok && keepColumnWidth())
      previous_columns_map.clear();

    setNumRows(num_rows);
    setNumCols(num_fields);
    if (is_first)
    {      
      for (uint i = 0; i < num_fields; i++)
      {
        if (columns_ok && previous_columns_map[i].label == query()->fields(i).name)
          horizontalHeader()->setLabel(i, qry->fields(i).name, previous_columns_map[i].size);
        else
          horizontalHeader()->setLabel(i, qry->fields(i).name);

        setColumnReadOnly (i, (i != 0));
      }
      is_first = false;
    }

    if (hasProcessEvents())
      qApp->processEvents();
    else
      updateScrollBars();

    uint j = 0;
    while (query()->next())
    {
      for (uint i = 0; i < num_fields; i++)
      {
        const char *val = query()->row(i) ? query()->row(i) : NULL_TEXT;
        if (i == 0)
          setItem (j, i, new CCheckTableItem(this, val));
        else        
          setText(j, i, val);
      }
      j++;
    }
    if (!columns_ok)
      for (j = 0; j < num_fields; j++)
        adjustColumn(j);

    if (max_scroll)
      scroll = verticalScrollBar()->maxValue();
    verticalScrollBar()->setValue(scroll);
  }  
  setBlocked(false);
  applyLastSort();
}


CServerStatusTable::CServerStatusTable(QWidget * parent, CMySQLServer *m)
: CQueryTable(parent, 0, m, "CServerStatusTable")
{
  verticalHeader()->hide();
  setFocusStyle(QTable::FollowStyle);
  setSelectionMode(QTable::Single);
  setLeftMargin(0);
  setKeepColumnWidth(true);
  qry = new CMySQLQuery(m->mysql());
  qry->setEmitMessages(false);
  setQuery(qry);
  setReadOnly(true);
  is_first = true;
  is_traditional = false;
  columnsWindow = new CTableFieldChooser(0, this, "CServerStatusTableColumnsWindow");
  columnsWindow->setCaption("[" + mysql()->connectionName() + "] " + tr("Server Status Columns"));
  setCaption(tr("Server Status"));
}

CServerStatusTable::~CServerStatusTable()
{
  delete columnsWindow;
  delete qry;
}

void CServerStatusTable::ContextMenuRequested(int row, int col, const QPoint &pos)
{
  if (isBlocked())
    return;

  QPopupMenu *menu = new QPopupMenu();
  
  menu->insertItem(getPixmapIcon("chooseFieldsIcon"), tr("Choose Fields"), MENU_COLUMNS_WINDOW);  
  menu->insertSeparator();

  menu->insertItem(getPixmapIcon("copyIcon"), tr("Copy"), MENU_COPY);
  menu->setItemEnabled(MENU_COPY, numRows() > 0 && numCols() > 0);

  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("saveIcon"), "Save", MENU_SAVE);
  menu->insertSeparator();  
  menu->insertItem(getPixmapIcon("refreshTablesIcon"), "Refresh", MENU_REFRESH);
  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("clearGridIcon"), tr("Clear Grid"), MENU_CLEAR_GRID);
  menu->setItemEnabled (MENU_CLEAR_GRID, !isBlocked());
  menu->insertSeparator();
  menu->insertItem((const QString&)((is_traditional) ? tr("Horizontal Mode") : tr("Vertical Mode")), MENU_TRADITIONAL);
  
  int res = menu->exec(pos);
  delete menu;
  
  switch (res)
  {
    case MENU_COLUMNS_WINDOW:
      columnsWindow->show();
      columnsWindow->raise();
      columnsWindow->setFocus();
    break;

    case MENU_COPY:
      copy(row, col);      
      break;

    case MENU_REFRESH:
      refresh();
      break;
    
    case MENU_SAVE:
      save();
      break; 

    case MENU_CLEAR_GRID:
      is_first = true;
      reset();
      break;

    case MENU_TRADITIONAL:
      {
        is_first = true;
        is_traditional = !is_traditional;
        reset();
        refresh();
      }
      break;
  }
}

void CServerStatusTable::copy(int row, int col)
{
#ifdef DEBUG
  qDebug("CSqlTable::copy(int, int)");
#endif

#ifndef QT_NO_CLIPBOARD
  if (is_traditional)
    QApplication::clipboard()->setText(CQueryTable::copy_data(row, col));
  else
    QApplication::clipboard()->setText(CTable::copy_data(row, col));        
#endif
}

void CServerStatusTable::refresh()
{
  if (isBlocked())
    return;

  setBlocked(true);
  
  bool max_scroll = false;
  int scroll = is_first ? 0 : verticalScrollBar()->value();
  if (scroll >= verticalScrollBar()->maxValue() - 1 && scroll != 0)
    max_scroll = true;    

  if (is_traditional)
  {
    if (keepColumnWidth() && numCols() > 0)
      for (int i = 0; i < numCols(); i++)
        previous_columns_map.insert(i, CTable::OldColumn(horizontalHeader()->sectionSize(i), horizontalHeader()->label(i)));

    clearCellWidget(currentRow(), currentColumn());
    if (is_first)
      ensureVisible(0, 0);
    setNumRows(0);
    if (sorting())
      horizontalHeader()->setSortIndicator( -1 );
  }
  if (query()->exec("SHOW STATUS"))
  {
    uint num_rows = query()->numRows();
    uint num_fields = query()->numFields();
    bool adjust = false;

    if (is_traditional)
    {
      bool columns_ok = (keepColumnWidth() && previous_columns_map.count() == num_fields);
      if (columns_ok)
        for (uint i = 0; i < num_fields; i++)
          columns_ok &= (previous_columns_map[i].label == query()->fields(i).name);

      if (!columns_ok)
      {
        adjust = true;
        if (keepColumnWidth())
          previous_columns_map.clear();
      }

      setNumRows(num_rows);
      setNumCols(num_fields);
      if (is_first)
      {
        for (uint i = 0; i < num_fields; i++)
          if (columns_ok && previous_columns_map[i].label == query()->fields(i).name)
            horizontalHeader()->setLabel(i, query()->fields(i).name, previous_columns_map[i].size);
          else
            horizontalHeader()->setLabel(i, query()->fields(i).name);

        if (columnsWindow->isVisible())
          columnsWindow->refresh();
        is_first = false;
        adjust = true;
      }
      uint j = 0;
      while (query()->next())
      {
        for (uint i = 0; i < num_fields; i++)
        {
          const char * val = query()->row(i) ? query()->row(i) : NULL_TEXT;
          setText(j, i, val);
        }
        j++;
      }
    }
    else
    {      
      if (is_first)
      {
        setNumRows(0);
        setNumCols(num_rows);
        int i = 0;
        while (query()->next())
          horizontalHeader()->setLabel(i++, query()->row(0));
        if (columnsWindow->isVisible())
          columnsWindow->refresh();
        is_first = false;
        adjust = true;
        query()->dataSeek(0);
      }
      setNumRows(numRows() + 1);      
      ulong i = 0;    
      while (query()->next())
      {
        const char * val = query()->row(1) ? query()->row(1) : NULL_TEXT;
        setText(numRows() - 1, i++, val);
      }
    }
    if (adjust)
      for (int j = 0; j < numCols(); j++)
        adjustColumn(j);

    if (max_scroll)
      scroll = verticalScrollBar()->maxValue();
    verticalScrollBar()->setValue(scroll);
  }
  setBlocked(false);
  applyLastSort();
}


CInnoDBStatus::CInnoDBStatus(QWidget *parent, CMySQLServer *m)
: Editor(parent, "CInnoDBStatus")
{
  setReadOnly(true);
  mysql = m;
  refresh();
}

bool CInnoDBStatus::hasInnoDB(CMySQLServer *mysql)
{
  bool has_innodb = false;

  CMySQLQuery *qry = new CMySQLQuery(mysql->mysql());
  bool notnew = mysql->mysql()->version().major <= 3;
  if (!notnew)
    notnew = mysql->mysql()->version().major == 4 && mysql->mysql()->version().minor == 0 && mysql->mysql()->version().relnum <= 2;
  
  qry->setEmitMessages(false);
  QString sql = "SHOW ";
  if (!notnew)
    sql += "GLOBAL ";
  sql += "VARIABLES LIKE 'have_innodb'";
  if (qry->exec(sql))
  {
    if (qry->numRows() >= 1)
    {
      qry->next();
      QString tmp = qry->row(1);
      if (tmp.lower() == "yes")
      {
        if (mysql->mysql()->version().major >= 3)
        {
          if (mysql->mysql()->version().major == 3 && mysql->mysql()->version().minor >= 23 && mysql->mysql()->version().relnum >= 53)
            has_innodb = true;
          else
            if (mysql->mysql()->version().major == 4 && mysql->mysql()->version().minor == 0)
            {
              if (mysql->mysql()->version().relnum >= 4)
                has_innodb = true;
            }
            else
              has_innodb = true;
        }
      }
    }
  }
  delete qry;
  return has_innodb;
}

void CInnoDBStatus::refresh()
{
  clear();
  CMySQLQuery *qry = new CMySQLQuery(mysql->mysql());
  qry->setEmitMessages(false);
  qry->setEmitErrors(false);
  if (qry->exec("SHOW INNODB STATUS"))
  {
    qry->next();
    setText(qry->row(0));
  }
  delete qry;
}

QString CInnoDBStatus::contents() const
{
  QString ret = tr("Connection") + ": " + mysql->connectionName() + mysql->lineTerminator(true);
  ret += tr("Host") + ": " + mysql->hostName() + mysql->lineTerminator(true);
  ret += tr("Saved") + ": " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + mysql->lineTerminator(true);
  ret += mysql->lineTerminator(true);
  ret += mysql->lineTerminator(true);
  ret = myApp()->commentText(ret, mysql->lineTerminator(true));
  ret += text();
  return ret;
}

void CInnoDBStatus::save()
{
  saveToFile(filename, "txt", tr("Text Files (*.txt);;All Files(*.*)" ), contents(), mysql->messagePanel());
}


QPopupMenu *CInnoDBStatus::createPopupMenu(const QPoint &)
{
  QPopupMenu *menu = new QPopupMenu(this);
  int copy_id = menu->insertItem(getPixmapIcon("copyIcon"), tr("&Copy"), this, SLOT(copy()));
  menu->setItemEnabled(copy_id, hasSelectedText());
  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("saveIcon"), tr("&Save"), this, SLOT(save()));
  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("refreshIcon"), tr("&Refresh"), this, SLOT(refresh()));
  return menu;
}

CAdministrationWindow::CShowServerVariables::CShowServerVariables(QWidget *parent, CMySQLServer *m)
: CProperties(parent, m, "CShowServerVariables")
{
  details->addColumn(tr("Property" ) );
  details->addColumn(tr("Value" ) );
  details->header()->setClickEnabled(false, 1);
  setSaveTitle(tr("Server Variables"));
  refresh();
}

void CAdministrationWindow::CShowServerVariables::refresh()
{
  setTitle(tr("Server Variables"));
  CProperties::refresh();
}

void CAdministrationWindow::CShowServerVariables::insertData()
{
  CMySQLQuery *qry = new CMySQLQuery(mysql()->mysql());

  bool notnew = mysql()->mysql()->version().major <= 3;
  if (!notnew)
    notnew = mysql()->mysql()->version().major == 4 && mysql()->mysql()->version().minor == 0 && mysql()->mysql()->version().relnum <= 2;
  
  qry->setEmitMessages(false);
  QString sql = "SHOW ";
  if (!notnew)
    sql += "GLOBAL ";
  sql += "VARIABLES";
  if (qry->exec(sql))
    while (qry->next())
      insertItem(qry->row(0), qry->row(1));
  delete qry;
}

CAdministrationWindow::CAdministrationWindow(QWidget* parent,  CMySQLServer *m)
: CMyWindow(parent, "CAdministrationWindow")
{
  (void)statusBar();

  blocked = false;
  myApp()->incCritical();
  setCaption("[" + m->connectionName() + "] " + trUtf8("Administration Panel"));
  setIcon(getPixmapIcon("applicationIcon"));
  enableMessageWindow(true);  

  if (!m->oneConnection())
  {
    m_mysql = new CMySQLServer(m->connectionName(), messagePanel());
    m_mysql->connect();
    delete_mysql = true;
  }
  else
  {
    m_mysql = m;  //POSIBLE BUG ... need to make m_mysql redirect messages & errors to this->messagePanel() also.  Currently they are in consoleWindow()
    //probably need to disable the timer !
    delete_mysql = false;
  }

  setCentralWidget( new QWidget( this, "qt_central_widget"));
  CAdministrationWindowLayout = new QGridLayout( centralWidget(), 1, 1, 4, 2, "CAdministrationWindowLayout"); 
  
  tabWidget = new QTabWidget( centralWidget(), "tabWidget");  
  
  processList = new CProcessListTable(tabWidget, m_mysql);
  tabWidget->insertTab(processList, getPixmapIcon("showProcessListIcon"), tr("Process List"), SHOW_PROCESSLIST);

  status = new CServerStatusTable(tabWidget, m_mysql);
  tabWidget->insertTab(status, getPixmapIcon("showStatusIcon"), tr("Status"), SHOW_STATUS);

  variables = new CShowServerVariables(tabWidget, m_mysql);
  tabWidget->insertTab(variables, getPixmapIcon("showVariablesIcon"), tr("Variables"), SHOW_VARIABLES);

  save_menu = new QPopupMenu(this);
  connect(save_menu, SIGNAL(activated(int)), this, SLOT(save(int)));
  save_menu->insertItem(getPixmapIcon("showProcessListIcon"), tr("&Process List"), MENU_SAVE_PROCESSLIST);
  save_menu->insertItem(getPixmapIcon("showStatusIcon"), tr("&Status"), MENU_SAVE_STATUS);
  save_menu->insertItem(getPixmapIcon("showVariablesIcon"), tr("&Variables"), MENU_SAVE_VARIABLES);
  save_menu->insertItem(tr("&InnoDB Status"), MENU_SAVE_INNODB_STATUS);

  has_innodb = CInnoDBStatus::hasInnoDB(m_mysql);
  if (has_innodb)
  {
    innoDBStatus = new CInnoDBStatus(tabWidget, m_mysql);
    tabWidget->insertTab(innoDBStatus, tr("InnoDB Status"), SHOW_INNODB_STATUS);    
  }
  else
    save_menu->setItemEnabled(MENU_SAVE_INNODB_STATUS, false);
  
  CAdministrationWindowLayout->addWidget( tabWidget, 0, 0 );

  viewShowMessagesAction = new CAction (tr("Show Messages"), tr("Show &Messages"), Qt::CTRL + Qt::Key_M,
    this, "fileShowMessagesAction", true);
  viewShowMessagesAction->setParentMenuText(tr("View"));
  connect(viewShowMessagesAction, SIGNAL(toggled(bool)), this, SLOT(showMessages(bool)));
  
  CAction * fileCloseAction = new CAction (tr("Close"), getPixmapIcon("closeIcon"),
    tr("&Close"), 0, this, "fileCloseAction");
  fileCloseAction->setParentMenuText(tr("File"));        
  connect(fileCloseAction, SIGNAL(activated()), this, SLOT(close()));


  fileRefreshAction = new CAction (tr("Refresh"), getPixmapIcon("refreshIcon"),
    tr("&Refresh"), Qt::Key_F5, this, "fileRefreshAction");
  fileRefreshAction->setParentMenuText(tr("File"));
  connect(fileRefreshAction, SIGNAL(activated()), this, SLOT(refresh()));


  actionKillProcessAction = new CAction (tr("Kill Process"), getPixmapIcon("killProcessIcon"),
    tr("&Kill Process"), Qt::CTRL + Qt::Key_K, this, "actionKillProcessAction");
  actionKillProcessAction->setParentMenuText(tr("Action"));
  connect(actionKillProcessAction, SIGNAL(activated()), this, SLOT(killProcesses()));

  CAction * actionPingAction = new CAction (tr("Ping"), getPixmapIcon("pingIcon"),
    tr("&Ping"), Qt::CTRL + Qt::Key_P, this, "actionPingAction");
  actionPingAction->setParentMenuText(tr("Action"));
  connect(actionPingAction, SIGNAL(activated()), this, SLOT(ping()));

  CAction * actionShutdownAction = new CAction (tr("Shutdown"), getPixmapIcon("serverShutdownIcon"),
    tr("&Shutdown"), 0, this, "actionShutdownAction");
  actionShutdownAction->setParentMenuText(tr("Action"));
  connect(actionShutdownAction, SIGNAL(activated()), this, SLOT(shutdown()));

  fileTimerAction = new CAction (tr("Start Refresh Timer"), getPixmapIcon("timerIcon"),
    tr("Start Refresh &Timer"), Qt::CTRL + Qt::Key_T, this, "fileTimerAction", true);
  fileTimerAction->setParentMenuText(tr("File"));
  connect(fileTimerAction, SIGNAL(toggled(bool)), this, SLOT(fileTimerActionToggled(bool)));

  QPopupMenu *fileMenu = new QPopupMenu(this);
  QPopupMenu *actionMenu = new QPopupMenu(this);
  QPopupMenu *viewMenu = new QPopupMenu(this);
  connect(viewMenu, SIGNAL(aboutToShow()), this, SLOT(viewMenuAboutToShow()));

  fileMenu->insertItem(getPixmapIcon("saveIcon"), tr("Save"), save_menu);
  fileMenu->insertSeparator();
  fileRefreshAction->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileTimerAction->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileCloseAction->addTo(fileMenu);
  menuBar()->insertItem(tr("&File"), fileMenu);

  viewShowMessagesAction->addTo(viewMenu);
  menuBar()->insertItem(tr("&View"), viewMenu);

  actionKillProcessAction->addTo(actionMenu);

  flush_menu = flushMenu(m_mysql->mysql());
  connect(flush_menu, SIGNAL(activated(int)), this, SLOT(flush(int)));

  actionMenu->insertItem(getPixmapIcon("flushIcon"),tr("Flush"), flush_menu);

  actionPingAction->addTo(actionMenu);
  actionMenu->insertSeparator();
  actionShutdownAction->addTo(actionMenu);
  menuBar()->insertItem(tr("&Action"), actionMenu);

  new CHotKeyEditorMenu(this, menuBar(), "HotKeyEditor");

  QToolBar * actionToolBar = new QToolBar(tr("Action Bar"), this, Top);

  QToolButton * saveTypeButton = new QToolButton(actionToolBar);
  saveTypeButton->setPopup(save_menu);
  saveTypeButton->setPixmap(getPixmapIcon("saveIcon"));
  saveTypeButton->setTextLabel(tr("Save"), true);
  saveTypeButton->setPopupDelay(0);
  actionToolBar->addSeparator();

  actionKillProcessAction->addTo(actionToolBar);
  
  QToolButton * flushTypeButton = new QToolButton(actionToolBar);  
  flushTypeButton->setPopup(flush_menu);
  flushTypeButton->setPixmap(getPixmapIcon("flushIcon"));
  flushTypeButton->setTextLabel(tr("Flush"), true);
  flushTypeButton->setPopupDelay(0);    

  actionPingAction->addTo(actionToolBar);
  actionToolBar->addSeparator();
  actionShutdownAction->addTo(actionToolBar);

  QToolBar * refreshToolBar = new QToolBar(tr("Refresh Bar"), this, Top );  
  fileRefreshAction->addTo(refreshToolBar);  
  refreshToolBar->addSeparator();
 
  QLabel * delayLabel = new QLabel(refreshToolBar, "delayLabel" ); 
  delayLabel->setMinimumSize(QSize(65, 0));
  delayLabel->setText(tr("Refresh Rate (sec)"));
  
  delay = new QSpinBox(refreshToolBar, "delayBox");  
  delay->setMinimumSize(QSize( 50, 0)); 
  delay->setMaxValue(7200);
  delay->setMinValue(1);

  CConfig *cfg = new CConfig();  
  delay->setValue(cfg->readNumberEntry("Refresh Rate", 30));
  status->setTraditionalMode(strtobool(cfg->readStringEntry("Status Traditional Mode", "true")));
  delete cfg;
  status->refresh();  

  fileTimerAction->addTo(refreshToolBar);

  refreshTimer = new QTimer(this);
  connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));

  connect(tabWidget, SIGNAL(currentChanged (QWidget *)), this, SLOT(tabChanged(QWidget *)));
  myResize(600, 400);
}

void CAdministrationWindow::save(int t)
{
  switch (t)
  {
  case MENU_SAVE_PROCESSLIST: processList->save();
    break;
  case MENU_SAVE_STATUS: status->save();
    break;
  case MENU_SAVE_VARIABLES: variables->save();
    break;
  case MENU_SAVE_INNODB_STATUS: 
    if (has_innodb)
      innoDBStatus->save();
    break;
  }
}

void CAdministrationWindow::killProcesses()
{
  if (isBlocked())
    return;
  setBlocked(true);
  bool found = false;
  for (int i = 0; i < processList->numRows(); i++)
  {     
    if (processList->item(i,0) != 0 && ((QCheckTableItem *) processList->item(i,0))->isChecked())
      if (mysql()->mysql()->mysqlKill(processList->text(i,0).toLong()))
      {
        messagePanel()->information(tr("Process killed successfully") + " :" +  processList->text(i,0));
        if (!found)
          found = true;
      }
  }
  if (found)
    processList->refresh();
  setBlocked(false);
}

void CAdministrationWindow::setBlocked(bool b)
{
  fileRefreshAction->setEnabled(!b);
  blocked = b;
  QCursor c = b ? Qt::WaitCursor : Qt::ArrowCursor;
  setCursor(c);
  variables->setCursor(c);
  qApp->processEvents();
}

void CAdministrationWindow::fileTimerActionToggled(bool b)
{      
  delay->setEnabled(!b);
  if (b)
  {
    fileTimerAction->setText(tr( "Stop Refresh &Timer"));  
    fileTimerAction->setMenuText(tr( "Stop Refresh &Timer"));
    refreshTimer->start(delay->value() * 1000);    
  }
  else
  {
    fileTimerAction->setText(tr( "Start Refresh Timer"));  
    fileTimerAction->setMenuText(tr( "Start Refresh &Timer"));
    refreshTimer->stop();
  }
}

void CAdministrationWindow::refresh()
{
  if (isBlocked())
    return;
  setBlocked(true); 
  processList->refresh();
  status->refresh();
  if (has_innodb)
    innoDBStatus->refresh();
  messagePanel()->information(tr("Refresh successful"));
  setBlocked(false);
}

CAdministrationWindow::~CAdministrationWindow()
{
  myApp()->decCritical();
  if (delete_mysql)
    delete m_mysql;
  delete flush_menu;
}

QPopupMenu * CAdministrationWindow::flushMenu(const CMySQL *mysql)
{
#ifdef DEBUG
  qDebug("static CTableTools::tableToolsMenu()");
#endif
    
  QPopupMenu *m = new QPopupMenu();
  m->insertItem(tr("&Hosts"), FLUSH_HOSTS);
  m->insertItem(tr("&Logs"), FLUSH_LOGS);
  m->insertItem(tr("&Privileges"), FLUSH_PRIVILEGES);
  m->insertItem(tr("&Tables"), FLUSH_TABLES);
  m->insertItem(tr("&Status"), FLUSH_STATUS);
  if (!(mysql->version().major <= 3 && mysql->version().minor <= 22))
    m->insertItem(tr("Tables with &Read Lock"), FLUSH_TABLES_RL);
  if (mysql->version().major >= 4)
  {
    m->insertItem(tr("&DES Keys"), FLUSH_DES_KEY_FILE);
    m->insertItem(tr("&Query Cache"), FLUSH_QUERY_CACHE);
    m->insertItem(tr("&User Resources"), FLUSH_USER_RESOURCES);
  }
  return m;
}

void CAdministrationWindow::tabChanged(QWidget *)
{
  actionKillProcessAction->setEnabled(tabWidget->currentPageIndex() == SHOW_PROCESSLIST);  
}

void CAdministrationWindow::saveSettings(CConfig *cfg)
{
  cfg->writeEntry("Refresh Rate", delay->value());
  cfg->writeEntry("Status Traditional Mode", booltostr(status->isTraditionalMode()));
}

void CAdministrationWindow::viewMenuAboutToShow()
{
#ifdef DEBUG
  qDebug("CAdministrationWindow::viewMenuAboutToShow()");
#endif
  
  viewShowMessagesAction->setOn(messageWindow()->isVisible());
}

void CAdministrationWindow::showMessages(bool b)
{
#ifdef DEBUG
  qDebug("CAdministrationWindow::showMessages(%s)", debug_string(booltostr(b)));
#endif

  if (b)
    messageWindow()->show();
  else
    messageWindow()->hide();  
}

void CAdministrationWindow::ping()
{
  ping(mysql());
}

void CAdministrationWindow::ping(CMySQLServer *m)
{
  if (m->mysql()->mysqlPing())
    m->messagePanel()->information("[" + m->connectionName() + "] " + tr("mysqld is alive"));    
}

void CAdministrationWindow::shutdown()
{
  shutdown(mysql());
}

void CAdministrationWindow::shutdown(CMySQLServer *m)
{
  if (myApp()->confirmCritical())
    if ((QMessageBox::information(0, tr("Confirm Shutdown"), tr("Are you sure you want to Shutdown mysqld ?"),
      tr("&Yes"), tr("&No")) != 0))
      return;

  if (m->mysql()->mysqlShutdown())
    m->messagePanel()->information("[" + m->connectionName() + "] " + tr("mysqld was shutdown successfully"));
}

void CAdministrationWindow::flush(int flush_type)
{
  flush(mysql(), flush_type);
}

void CAdministrationWindow::flush(CMySQLServer *m, int flush_type)
{
  QString f = QString::null;
  switch (flush_type)
  {
  case FLUSH_HOSTS: f = "HOSTS";
    break;
  case FLUSH_LOGS: f = "LOGS";
    break;
  case FLUSH_PRIVILEGES: f = "PRIVILEGES";
    break;
  case FLUSH_TABLES: f = "TABLES";
    break;
  case FLUSH_STATUS: f = "STATUS";
    break;
  case FLUSH_TABLES_RL: f = "TABLES WITH READ LOCK";
    break;
  case FLUSH_DES_KEY_FILE: f = "DES_KEY_FILE";
    break;
  case FLUSH_QUERY_CACHE: f = "QUERY CACHE";
    break;
  case FLUSH_USER_RESOURCES: f = "USER_RESOURCES";
    break;
  }

  CMySQLQuery *qry = new CMySQLQuery(m->mysql());
  qry->setEmitMessages(false);
  if (qry->execStaticQuery("FLUSH " + f))
    m->messagePanel()->information("Flush " + capitalize(f) + " " + tr("successful"));
  delete qry;
}

void CAdministrationWindow::setCurrentTabPage(int t)
{
  tabWidget->setCurrentPage(t);
}
