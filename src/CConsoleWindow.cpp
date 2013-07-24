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
#include "CConsoleWindow.h"
#include "globals.h"
#include "CConfig.h"
#include "config.h"
#include "CDockWindow.h"
#include "panels.h"
#include "CDatabaseListView.h"
#include "CDatabaseListViewItem.h"
#include "CDatabaseItem.h"
#include "shared_menus.h"
#include "CRegisterServerDialog.h"
#include "CHotKeyEditorDialog.h"
#include <stddef.h>
#include <qregexp.h>
#include <qlayout.h>
#include <q3widgetstack.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <Q3PopupMenu>
#include "CMySQLServer.h"
#include "CShowServerProperties.h"
#include "CShowDatabaseGroupProperties.h"
#include "CShowTableGroupProperties.h"
#include "CShowTableProperties.h"
#include "CTableSpecific.h"

/*
CConsoleWindow() is the main Console Window in mysqlcc.
*/
CConsoleWindow::CConsoleWindow(QWidget * parent, bool appwindow, Qt::WFlags f)
: CMyWindow(parent, 0, appwindow, f)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::CConsoleWindow()");
#endif

  current_query_item = 0;
  (void)statusBar();
  
  setIcon(getPixmapIcon("applicationIcon"));
  setCentralWidget( new QWidget( this, "qt_central_widget"));

  enableMessageWindow(true);

  messageWindow()->setCaption(tr("Message Panel"));

  CConsoleWindowLayout = new Q3GridLayout( centralWidget(), 1, 1, 2, 2, "CConsoleWindowLayout"); 

  widgetstack = new Q3WidgetStack(centralWidget(), "widgetstack");
  widgetstack->setFrameShape( Q3Frame::StyledPanel );
  widgetstack->setFrameShadow( Q3Frame::Sunken );
  CConsoleWindowLayout->addWidget(widgetstack, 0, 0);

  defaultwidget = new QWidget(widgetstack);
  widgetstack->addWidget(defaultwidget, (int) CDatabaseListViewItem::NONE);

  widgetstack->addWidget(new CShowServerProperties(widgetstack), (int) CDatabaseListViewItem::SERVER);
  widgetstack->addWidget(new CShowDatabaseGroupProperties(widgetstack), (int) CDatabaseListViewItem::DATABASE_GROUP);

  //TODO .. This should be changed to contain a DATABASE specific widget
  widgetstack->addWidget(new CShowDatabaseGroupProperties(widgetstack), (int) CDatabaseListViewItem::DATABASE);

  widgetstack->addWidget(new CShowTableGroupProperties(widgetstack), (int) CDatabaseListViewItem::TABLE_GROUP);
  widgetstack->addWidget(new CShowTableProperties(widgetstack), (int) CDatabaseListViewItem::TABLE);
  widgetstack->addWidget(new CShowDatabaseGroupProperties(widgetstack), (int) CDatabaseListViewItem::SERVER_ADMINISTRATION);
  widgetstack->addWidget(new CShowDatabaseGroupProperties(widgetstack), (int) CDatabaseListViewItem::USER_ADMINISTRATION);
    
  treepanelwindow = new CDockWindow(CDockWindow::InDock, this, isApplicationWindow() ? "TreePanelWindow" : "TreePanelWindow2");
  treepanelwindow->setCaption((const QString&)(isApplicationWindow() ? tr("MySQL Servers") : tr("Tree Panel")));
  dbListView = new CDatabaseListView(this, treepanelwindow, "DatabaseListView");
    
  treepanelwindow->setWidget(dbListView);
  addDockWindow(treepanelwindow, Qt::DockLeft);
    
  Q3PopupMenu *fileMenu = new Q3PopupMenu(this, "FileMenu");
  menuBar()->insertItem(tr("&File"), fileMenu);
  sqldebugpanel = 0;

  Q3PopupMenu *viewMenu = new Q3PopupMenu(this, "ViewMenu");
  connect(viewMenu, SIGNAL(aboutToShow()), this, SLOT(viewMenuAboutToShow()));

  consoletoolbar = new Q3ToolBar(this);
  consoletoolbar->setLabel(tr("Console"));

  if (isApplicationWindow())
  {
    setCaption(tr("Console Manager"));
    
    sqldebugpanel = new CSqlDebugPanel(tr("SQL Debug"));
    messageWindow()->addPanel(sqldebugpanel);
    messageWindow()->setTabEnabled(sqldebugpanel, myApp()->enableSqlPanel());

    CAction * fileRegisterServerAction = new CAction (tr("New"), getPixmapIcon("registerServerIcon"),
      tr("&New"), Qt::CTRL + Qt::Key_N, this, "fileRegisterServerAction");
    fileRegisterServerAction->setParentMenuText(tr("File"));
    fileRegisterServerAction->addTo(fileMenu);
    
    connect(fileRegisterServerAction, SIGNAL(activated()), this, SLOT(registerServer()));

    CAction * fileRefreshServersAction = new CAction (tr("Refresh"), getPixmapIcon("refreshIcon"),
      tr("&Refresh"), Qt::Key_F5, this, "fileRefreshServersAction");
    fileRefreshServersAction->setParentMenuText(tr("File"));
    fileRefreshServersAction->addTo(fileMenu);
    
    connect(fileRefreshServersAction, SIGNAL(activated()), dbListView, SLOT(refreshServers()));

    fileMenu->insertSeparator();
    
    fileRegisterServerAction->addTo(consoletoolbar);
    consoletoolbar->addSeparator();
    fileRefreshServersAction->addTo(consoletoolbar);
    consoletoolbar->addSeparator();
  }
  itemtoolbar = new CToolBar(this);
  itemtoolbar->setLabel(tr("Action"));
  
  fileQueryAction = new CAction (tr("Query"), getPixmapIcon("sqlIcon"),
    tr("&Query"), Qt::CTRL + Qt::Key_Q, this, "fileQueryAction");
  connect(fileQueryAction, SIGNAL(activated()), this, SLOT(openQueryWindow()));
  connect(dbListView, SIGNAL(itemSelected(CDatabaseListViewItem *)), this, SLOT(prepareQueryWindow(CDatabaseListViewItem *)));
  fileQueryAction->setEnabled(false);

  fileQueryAction->setParentMenuText(tr("File"));
  fileQueryAction->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileQueryAction->addTo(consoletoolbar);

  viewShowTreeAction = new CAction (tr("Show Tree"), tr("&Show Tree"), Qt::CTRL + Qt::Key_T,
    this, "viewShowTreeAction", true);
  viewShowTreeAction->setParentMenuText(tr("View"));
  connect(viewShowTreeAction, SIGNAL(toggled(bool)), this, SLOT(showTree(bool)));  
  viewShowTreeAction->addTo(viewMenu);

  viewShowMessagesAction = new CAction (tr("Show Messages"), tr("Show &Messages"), Qt::CTRL + Qt::Key_M,
    this, "fileShowMessagesAction", true);
  viewShowMessagesAction->setParentMenuText(tr("View"));
  connect(viewShowMessagesAction, SIGNAL(toggled(bool)), this, SLOT(showMessages(bool)));
  viewShowMessagesAction->addTo(viewMenu);
  
  menuBar()->insertItem(tr("&View"), viewMenu);

  if (isApplicationWindow() && !myApp()->isMDI())
  {
    setName("MainWindow");
    QAction * fileExitAction = new QAction (tr("Exit"), getPixmapIcon("exitIcon"),
      tr("E&xit"), 0, this, "fileExitAction");  //Exit should not be CAction
    connect(fileExitAction, SIGNAL(activated()), this, SLOT(close()));      
    fileExitAction->addTo(fileMenu);
    
    new COptionsMenu(this, menuBar(), "OptionsMenu");

    actionmenu = new Q3PopupMenu(this, "ActionMenu");
    action_menu_id = menuBar()->insertItem(tr("&Action"), actionmenu);
#ifndef NO_MYSQLCC_PLUGINS
    new CPluginsMenu(this, menuBar(), "PluginsMenu");
#endif
    new CHotKeyEditorMenu(this, menuBar(), "HotKeyEditor");
    new CHelpMenu(this, menuBar(), "HelpMenu");
  }
  else
  {
    CAction * fileCloseAction = new CAction (tr("Close"), getPixmapIcon("closeIcon"),
      tr("&Close"), 0, this, "fileCloseAction");
    fileCloseAction->setParentMenuText(tr("File"));        
    connect(fileCloseAction, SIGNAL(activated()), this, SLOT(close()));
    fileCloseAction->addTo(fileMenu);
    
    actionmenu = new Q3PopupMenu(this, "ActionMenu");
    action_menu_id = menuBar()->insertItem(tr("&Action"), actionmenu);
    new CHotKeyEditorMenu(this, menuBar(), "HotKeyEditor");
    
    if (myApp()->isMDI() && isApplicationWindow())
    {
      setName("CConsoleWindow");
      ((CMyWindow *)myApp()->mainWidget())->setDockEnabled(treepanelwindow, Qt::DockLeft, true);
      ((CMyWindow *)myApp()->mainWidget())->setDockEnabled(messageWindow(), Qt::DockLeft, true);
    }
    else
    {
      setName("CConsoleWindow2");
      myResize(600,400);
    }

  }

  connect(actionmenu, SIGNAL(aboutToShow()), this, SLOT(actionMenuAboutToShow()));
  connect(actionmenu, SIGNAL(activated(int)), databaseListView(), SLOT(processCurrentItemMenu(int)));
  connect(itemtoolbar, SIGNAL(buttonClicked(int)), databaseListView(), SLOT(processCurrentItemMenu(int)));
}

void CConsoleWindow::prepareQueryWindow(CDatabaseListViewItem *i)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::prepareQueryWindow()");
#endif

  if (i == 0)
  {
    fileQueryAction->setEnabled(false);
    current_query_item = 0;
    return;
  }

  if (!i->mysql()->isConnected())
  {
    current_query_item = 0;
    fileQueryAction->setEnabled(false);
  }
  else
  {
    current_query_item = i;
    fileQueryAction->setEnabled(true);
  }
}


void CConsoleWindow::openQueryWindow()
{
#ifdef DEBUG
  qDebug("CConsoleWindow::openQueryWindow()");
#endif

  if (current_query_item == 0 || current_query_item->isBlocked())
    return;

  QString tmp_db = current_query_item->databaseName();
  QString tmp_table = current_query_item->tableName();

  if (current_query_item->type() == CDatabaseListViewItem::DATABASE)
    if (!((CDatabaseItem *) current_query_item)->isConnected())
      tmp_db = QString::null;

  CTableSpecific::QueryWindow(current_query_item->mysql(), tmp_db, current_query_item->tableName(), SQL_QUERY);
}


void CConsoleWindow::enableActionMenu(bool b)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::enableActionMenu(%s)", debug_string(booltostr(b)));
#endif

  menuBar()->setItemEnabled(action_menu_id, b);
}

void CConsoleWindow::actionMenuAboutToShow()
{
#ifdef DEBUG
  qDebug("CConsoleWindow::actionMenuAboutToShow()");
#endif

  databaseListView()->getActionMenu(actionMenu());
}

void CConsoleWindow::setCursor (const QCursor &c)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::setCursor()");
#endif

  CMyWindow::setCursor(c);
  widgetStack()->setCursor(c);
  QWidget *v = widgetStack()->visibleWidget();
  if (v != 0)
    v->setCursor(c);
  databaseListView()->setCursor(c);
}

void CConsoleWindow::removeFromWidgetStack(int id)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::removeFromWidgetStack(%d)", id);
#endif

  QWidget *w = widgetstack->widget(id);
  if (w != 0)
  {
    widgetstack->removeWidget(w);
    delete w;
  }
}


void CConsoleWindow::viewMenuAboutToShow()
{
#ifdef DEBUG
  qDebug("CConsoleWindow::viewMenuAboutToShow()");
#endif

  viewShowTreeAction->setOn(treepanelwindow->isVisible());
  viewShowMessagesAction->setOn(messageWindow()->isVisible());
}

void CConsoleWindow::showTree(bool b)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::showTree(%s)", debug_string(booltostr(b)));
#endif

  if (b)
    treepanelwindow->show();
  else
    treepanelwindow->hide();  
}

void CConsoleWindow::showMessages(bool b)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::showMessages(%s)", debug_string(booltostr(b)));
#endif

  if (b)
    messageWindow()->show();
  else
    messageWindow()->hide();  
}  

void CConsoleWindow::registerServer()
{
#ifdef DEBUG
  qDebug("CConsoleWindow::registerServer()");
#endif

  CRegisterServerDialog *c = new CRegisterServerDialog(messagePanel(), myApp()->workSpace());
  connect(c, SIGNAL(newConnection()), dbListView, SLOT(refreshServers()));
  myShowWindow(c);
}


/*
reimplemented beforeClose() takes care of calling the window position/size saving routines.
*/
void CConsoleWindow::beforeClose()
{
#ifdef DEBUG
  qDebug("CConsoleWindow::beforeClose()");
#endif
  
  if (isApplicationWindow() && myApp()->isMDI())
  {
    position = parentWidget()->pos();
    saveWindowSettings();
  }
  else
    CMyWindow::beforeClose();
}


void CConsoleWindow::closeEvent(QCloseEvent * e)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::closeEvent()");
#endif
  
  beforeClose();  
  if (!myApp()->isMDI() && isApplicationWindow())
  {      
    if (myApp()->confirmCritical() && !myApp()->checkCritical())
      if ((QMessageBox::information(0, tr("Confirm Exit"), tr("Are you sure you want to Exit ?"),
        QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes))
      {
        e->ignore();
        return;
      }
      emit about_to_close();
      e->accept();
      return;
  }
  else
  {
    if (isApplicationWindow())
    {
      e->ignore();
      hide();
    }
    else
    {
      treepanelwindow->close();
      messageWindow()->close();
      emit about_to_close();
      e->accept();
    }
  }  
}

void CConsoleWindow::saveDockWindowSettings(Q3DockWindow *dw, const QString &c, CConfig *cfg)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::saveDockWindowSettings()");
#endif
  
  Qt::ToolBarDock d = findDockWindow((CMyWindow *)qApp->mainWidget(), dw);
  QString s;
  if ((int) d != 0)
  {
    int m = 0;
    if ((d == Qt::DockTop) || (d == Qt::DockBottom))
      m = dw->height();
    else
      m = dw->width();
    s.sprintf("%d,%d,%d,%s", (dw->isVisible() ? 1 : 0), (int) d, m, dw->area() != 0 ? dw->area()->parent()->name() : "null");
  }
  else
    s.sprintf("%d,%d,%d,%s",1, 0, 0, "null");
  cfg->writeEntry(c, s);
}

/*
  saveSettings() is the function responsible for saving connected servers and databases
  so that they can be restored in the next mysqlcc execution.
*/
void CConsoleWindow::saveSettings(CConfig *cfg)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::saveSettings()");
#endif

  if (!isApplicationWindow() || !myApp()->saveWorkspace())
    return;

  Q3ListViewItemIterator it(databaseListView());
  QString server;
  bool server_done = false;
  while (it.current() != 0)
  {
    switch (((CDatabaseListViewItem *) it.current())->type())
    {
      case CDatabaseListViewItem::SERVER:
        if (it.current()->childCount() > 0)
        {
          ((CDatabaseListViewItem *) it.current())->mysql()->refresh();
          server_done = !((CDatabaseListViewItem *) it.current())->mysql()->disableStartupConnect();
          if (server_done)
            server += "\t'" + it.current()->text(0) + "'";
        }
        else
          server_done = false;
        break;
  
      case CDatabaseListViewItem::DATABASE:
        if (((CDatabaseItem *) it.current())->isConnected() && server_done)
          server += "/'" + it.current()->text(0) + "'";
        break;
      
      default:
        break;
    }    
    ++it; 
  }
  server = server.stripWhiteSpace();
  if (server.isEmpty())
    cfg->removeEntry("Open Items");
  else
    cfg->writeEntry("Open Items", server);
}

void CConsoleWindow::saveWindowSettings()
{
#ifdef DEBUG
  qDebug("CConsoleWindow::saveWindowSettings()");
#endif
  
  CMyWindow::saveWindowSettings();
  if (!myApp()->isMDI() || !isApplicationWindow())
    return;
  CConfig *cfg = new CConfig();
  saveDockWindowSettings(treepanelwindow, "TreePanelWindow", cfg);
  saveDockWindowSettings(messageWindow(), "MessagePanel", cfg);
  cfg->save();
  delete cfg;
}

bool CConsoleWindow::loadDockWindowSettings(bool r, Q3DockWindow *dw, const QString &c, CConfig *cfg)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::loadDockWindowSettings()");
#endif
  
 bool ret = r;
  QString s = cfg->readStringEntry(c, QString::null);
  if (!s.isNull())
  {
    QRegExp rx("^(\\d+),(\\d+),(\\d+),(.+)");
    if (rx.search(s) != -1)
    {
      if (rx.cap(4) == "MainWindow")
      {
        Qt::ToolBarDock d = (Qt::ToolBarDock) rx.cap(2).toInt();
        if ((int) d == 0)
          d = Qt::DockLeft;
        ((CMyWindow *)qApp->mainWidget())->addDockWindow(dw, d);
        int m = rx.cap(3).toInt();
        if (((d == Qt::DockTop) || (d == Qt::DockBottom)) && (m != 0))
          dw->setFixedExtentHeight(m);
        else
          dw->setFixedExtentWidth(m);
        if (rx.cap(1) != "1")
          dw->hide();
        ret = true;
      }
    }
  }  
  return ret;
}

bool CConsoleWindow::loadWindowSettings()
{
#ifdef DEBUG
  qDebug("CConsoleWindow::loadWindowSettings()");
#endif
    
  bool ret = CMyWindow::loadWindowSettings();
  if (!myApp()->isMDI() || !isApplicationWindow())
    return ret;
  CConfig *cfg = new CConfig();  
  ret = loadDockWindowSettings(ret, treepanelwindow, "TreePanelWindow", cfg);
  ret = loadDockWindowSettings(ret, messageWindow(), "MessagePanel", cfg);
  delete cfg;
  return ret;
}
