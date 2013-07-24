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
#include "CMDIWindow.h"
#include "globals.h"
#include "config.h"
#include "shared_menus.h"
#include "CConsoleWindow.h"
#include "CDatabaseListView.h"
#include "CHotKeyEditorDialog.h"
#include <stddef.h>  
#include <qaction.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3PopupMenu>
#include <QCloseEvent>

/*
CMDIWindow is the class used as the Main Application Widget when mysqlcc is
running in MDI mode.

It is used by main.cpp
*/

CMDIWindow::CMDIWindow(QWidget* parent)
: CMyWindow(parent, 0, true, Qt::WType_TopLevel)
{
#ifdef DEBUG
  qDebug("CMDIWindow::CMDIWindow()");
#endif
  
  setName("MainWindow");
  setMinimumSize(320, 240);
  
  setCaption(QString(APPLICATION) + " " + QString(VERSION) + QString(BRANCH));
  setIcon(getPixmapIcon("applicationIcon"));
  
  setCentralWidget( new QWidget( this, "qt_central_widget" ) );
  CMDIWindowLayout = new Q3GridLayout( centralWidget(), 1, 1, 2, 2, "CMDIWindowLayout"); 
   
  myApp()->createWorkspace(centralWidget());
  myApp()->workSpace()->setScrollBarsEnabled(true);
  
  CMDIWindowLayout->addWidget(myApp()->workSpace(), 0, 0);
  
  consoleMenu = new Q3PopupMenu(this, "ConsoleMenu");
  
  QAction * consoleExitAction = new QAction (tr("Exit"), getPixmapIcon("exitIcon"),
    tr("E&xit"), 0, this, "consoleExitAction");  //Exit should not be CAction 
  consoleExitAction->addTo(consoleMenu);
  connect(consoleExitAction, SIGNAL(activated()), this, SLOT(close()));
  
  menuBar()->insertItem(tr("&Console"), consoleMenu);  
   
  new COptionsMenu(this, menuBar(), "OptionsMenu");
#ifndef NO_MYSQLCC_PLUGINS
  new CPluginsMenu(this, menuBar(), "PluginsMenu");
#endif
  new CHotKeyEditorMenu(this, menuBar(), "HotKeyEditor");

  windowMenu = new Q3PopupMenu(this, "WindowMenu");
  windowMenu->setCheckable(true);
  menuBar()->insertItem(tr("&Window"), windowMenu);
  new CHelpMenu(this, menuBar(), "HelpMenu");

  if (!loadWindowSettings())
    setGeometry((int)(myApp()->desktop()->width() - (myApp()->desktop()->width() - (myApp()->desktop()->width() / 2)) * 1.5) / 2,
    (int)(myApp()->desktop()->height() - (myApp()->desktop()->height() -  (myApp()->desktop()->height() / 2)) * 1.5) / 2,
    (int)((myApp()->desktop()->width() - (myApp()->desktop()->width() / 2)) * 1.5),
    (int)((myApp()->desktop()->height() - (myApp()->desktop()->height() / 2)) * 1.5));
  
  consoleWindow = 0;
  connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(windowMenuAboutToShow()));
}


/*
This function creates the main Console Window.  It is not in the constructor because
QWorkspace needs a fully created workspace so that it can place correctly child windows.

When mysqlcc is in MDI mode, main.cpp will call this function.
*/
void CMDIWindow::createConsoleWindow()
{
#ifdef DEBUG
  qDebug("CMDIWindow::createConsoleWindow()");
#endif
  
  if (consoleWindow == 0)
  {
    consoleWindow = new CConsoleWindow(myApp()->workSpace(), true);
    myApp()->setMainConsoleWindow(consoleWindow);    
    if (!consoleWindow->loadWindowSettings())
      consoleWindow->setGeometry(35, 30, 600, 400);
    myShowWindow(consoleWindow);
    consoleWindow->databaseListView()->refreshServers();
  }
}


/*
This reimplemented method confirms if the user wants to exit from the application or not
*/
void CMDIWindow::closeEvent(QCloseEvent * e)
{
#ifdef DEBUG
  qDebug("CMDIWindow::closeEvent()");  
#endif
  
  if (myApp()->confirmCritical() && !myApp()->checkCritical())
    if ((QMessageBox::information(0, tr("Confirm Exit"), tr("Are you sure you want to Exit ?"),
      tr("&Yes"), tr("&No")) != 0))
    {
      e->ignore();
      return;
    }
  beforeClose();
  QWidgetList openWindows = myApp()->workSpace()->windowList();
  if (openWindows.count() > 0)
    for ( int i = 0; i < int(openWindows.count()); ++i)
      ((CMyWindow *)openWindows.at(i))->beforeClose();
  
  emit about_to_close();
  e->accept();
}


/*
This slot closes the window which currently has the focus
*/
void CMDIWindow::closeActiveWindow()
{
#ifdef DEBUG
  qDebug("CMDIWindow::closeActiveWindow()");
#endif
  
  QWidget* wnd = (QWidget*)myApp()->workSpace()->activeWindow();  
  if (wnd)
    wnd->close();
}


/*
This slot is responsible for showing an activated window via the Window menu
*/
void CMDIWindow::windowActivated(int id)
{
#ifdef DEBUG
  qDebug("CMDIWindow::windowActivated()");
#endif
  
  QWidget* wnd = myApp()->workSpace()->windowList().at(id);
  if (wnd)
    wnd->showNormal();
  wnd->setFocus();
}


/*
This slot shows the consoleWindow when it's hidden and hides it when its visible.
*/
void CMDIWindow::consoleWindowDisplay()
{
#ifdef DEBUG
  qDebug("CMDIWindow::consoleWindowDisplay()");
#endif
  
  if (consoleWindow->isHidden())
  {
    myShowWindow(consoleWindow);
    consoleWindow->setFocus();
  }
  else  
    consoleWindow->hide();  
}


/*
This slot closes ALL windows except for the Console Window
*/
void CMDIWindow::closeAllWindows()
{
#ifdef DEBUG
  qDebug("CMDIWindow::closeAllWindows()");
#endif
  
  QWidgetList lst = myApp()->workSpace()->windowList();
  Q3PtrListIterator<QWidget> it(lst);
  while (it.current())
  {
	  QWidget *w = it.current();
	  ++it;
    w->close(true);
  }
}


/*
The "Window" menu item is created dynamically;  This slot creates the menu items
for the Window menu before it's shown.
*/
void CMDIWindow::windowMenuAboutToShow()
{
#ifdef DEBUG
  qDebug("CMDIWindow::windowMenuAboutToShow()");
#endif
  
  windowMenu->clear();
  
  int consoleWindowId = windowMenu->insertItem(consoleWindow->caption(), this, SLOT(consoleWindowDisplay()));
  windowMenu->setItemParameter(consoleWindowId, 0);
  windowMenu->setItemChecked(consoleWindowId, !consoleWindow->isHidden());
  windowMenu->insertSeparator();
  
  int closeId = windowMenu->insertItem(getPixmapIcon("closeIcon"), tr("Cl&ose"), this, SLOT(closeActiveWindow()));
  int closeAllId = windowMenu->insertItem(getPixmapIcon("closeAllIcon"), tr("Close Al&l"), this, SLOT(closeAllWindows()));
  windowMenu->insertSeparator();
  int cascadeId = windowMenu->insertItem(getPixmapIcon("cascadeIcon"), tr("&Cascade"), CApplication::Application()->workSpace(), SLOT(cascade()));
  int tileId = windowMenu->insertItem(getPixmapIcon("tileIcon"), tr("&Tile"), CApplication::Application()->workSpace(), SLOT(tile()));
  QWidgetList openWindows = CApplication::Application()->workSpace()->windowList();
  
  if ( ((openWindows.count() <= 1) || (CApplication::Application()->workSpace()->windowList().isEmpty())) && (consoleWindow->isHidden()))
  {
    windowMenu->setItemEnabled( closeId, false);
    windowMenu->setItemEnabled( closeAllId, false);
    windowMenu->setItemEnabled( cascadeId, false);
    windowMenu->setItemEnabled( tileId, false);
  }
  else
    if (openWindows.count() > 1)
      windowMenu->insertSeparator();
    for ( int i = 1; i < int(openWindows.count()); ++i )
    {
      int id = windowMenu->insertItem(openWindows.at(i)->caption(), this, SLOT(windowActivated(int)));
      windowMenu->setItemParameter(id, i);
      windowMenu->setItemChecked(id, CApplication::Application()->workSpace()->activeWindow() == openWindows.at(i));
    }
}
