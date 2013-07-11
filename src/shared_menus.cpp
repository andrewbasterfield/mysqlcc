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
#include "shared_menus.h"
#include "config.h"
#include "globals.h"
#include "CAboutBox.h"
#include "CCreditsBox.h"
#include "CConfig.h"
#include "CAppConfigDialog.h"
#include "CApplication.h"
#include "CPluginWindow.h"
#include <stddef.h>  
#include <qfontdialog.h>

/*
This file contains shared Menus which are used by CConsoleWindow & CMDIWindow
*/

/*
Created a "Plugins" menu item"
*/
CPluginsMenu::CPluginsMenu(QWidget * parent, QMenuBar *menuBar, const char * name)
:QPopupMenu(parent, name)
{
#ifdef DEBUG
  qDebug("CPluginsMenu::CPluginsMenu()");
#endif
  
  PluginList pl = myPluginsList(Plugin::MAIN);
  if (!pl.empty())
  {
    for (PluginList::iterator it = pl.begin(); it != pl.end(); ++it)
    {
      CPluginAction *action = new CPluginAction(parent, (*it).name() + "Action");
      action->setPlugin(*it);
      action->setText((*it).name());
      action->setMenuText((*it).name());
      action->setParentMenuText(tr("Plugins"));
      if (!(*it).icon().isNull())
        action->setIconSet((*it).icon());
      action->addTo(this);
      connect(action, SIGNAL(my_activated(const CPlugin &)), this, SLOT(Activated(const CPlugin &)));
    }
    menuBar->insertItem(tr("&Plugins"), this);
  }
}

void CPluginsMenu::Activated(const CPlugin &p)
{
#ifndef NO_MYSQLCC_PLUGINS
  switch (p.type())
  {
  case Plugin::WIDGET:
      myShowWindow(new CPluginWindow(myApp()->workSpace(), p));
      break;

  case Plugin::NO_WIDGET:
    {
      CMySQLCCPlugin *plugin = CPlugin::createPlugin(p);
      plugin->init();
      plugin->exec();
      delete plugin;
    }
    break;
  }
#endif
} 

/*
Creates a "Help" menu item
*/
CHelpMenu::CHelpMenu(QWidget * parent, QMenuBar *menuBar, const char * name)
:QPopupMenu(parent, name)
{
#ifdef DEBUG
  qDebug("CHelpMenu::CHelpMenu()");
#endif
      
  helpContentsAction = new CAction(parent, "helpContentsAction");
  helpContentsAction->setText(tr("Contents"));
  helpContentsAction->setMenuText(tr("&Contents"));
  helpContentsAction->setParentMenuText(tr("Help"));
  helpContentsAction->setAccel(Qt::Key_F1);
  //TODO Remove the below line when the help file is implemented
  helpContentsAction->setEnabled(false);
  
  helpWhatIsThisAction = new CAction(parent, "helpWhatIsThisAction");
  helpWhatIsThisAction->setText(tr("What's This ?"));
  helpWhatIsThisAction->setMenuText(tr("&What's This ?"));
  helpWhatIsThisAction->setIconSet(getPixmapIcon("contextHelpIcon"));
  helpWhatIsThisAction->setParentMenuText(tr("Help"));
  helpWhatIsThisAction->setAccel(Qt::Key_F10);
  
  helpCreditsAction = new CAction(parent, "helpCreditsAction");
  helpCreditsAction->setText(tr("Credits"));
  helpCreditsAction->setMenuText(tr("&Credits"));
  helpCreditsAction->setParentMenuText(tr("Help"));
  helpCreditsAction->setAccel(Qt::Key_F11);
  
  helpAboutAction = new CAction(parent, "helpAboutAction");
  helpAboutAction->setText(tr("About") + " " + QString(APPLICATION));
  helpAboutAction->setMenuText(tr("&About") + " " + QString(APPLICATION));
  helpAboutAction->setIconSet(getPixmapIcon("aboutIcon"));
  helpAboutAction->setParentMenuText(tr("Help"));
  helpAboutAction->setAccel(Qt::Key_F12);
  
  helpContentsAction->addTo(this);
  helpWhatIsThisAction->addTo(this);
  insertSeparator();
  helpCreditsAction->addTo(this);
  helpAboutAction->addTo(this);
  
  menuBar->insertItem(tr( "&Help"), this);
  connect(helpAboutAction, SIGNAL(activated()), this, SLOT(aboutAction()));
  connect(helpCreditsAction, SIGNAL(activated()), this, SLOT(creditsAction()));
  connect(helpWhatIsThisAction, SIGNAL(activated()), parent, SLOT(whatsThis()));
  connect(helpContentsAction, SIGNAL(activated()), this, SLOT(contentsAction()));  
}

/*
TODO
This function displays the Help Dialog
*/
void CHelpMenu::contentsAction()
{
#ifdef DEBUG
  qDebug("CHelpMenu::contentsAction()");
#endif
  
  QMessageBox::information(0, "Missing", "CHelpMenu::ContentsAction(): Not implemented yet!" );
}

/*
Displays the About Dialog
*/
void CHelpMenu::aboutAction()
{
#ifdef DEBUG
  qDebug("CHelpMenu::aboutAction()");
#endif
  
  CAboutBox aboutBox;
  aboutBox.exec();
}

/*
Displays the Credits Dialog
*/
void CHelpMenu::creditsAction()
{
#ifdef DEBUG
  qDebug("CHelpMenu::creditsAction()");
#endif
  
  CCreditsBox creditsBox;
  creditsBox.exec();
}


/*
This class handles all the "Options" menu items
*/
COptionsMenu::COptionsMenu(QWidget * parent, QMenuBar *menuBar, const char * name)
:QPopupMenu(parent, name)
{
#ifdef DEBUG
  qDebug("COptionsMenu::COptionsMenu()");
#endif
  
  mdiAction = new CAction(parent, "mdiAction", true);  
  mdiAction->setMenuText(tr("&MDI"));
  mdiAction->setText(tr("MDI"));
  mdiAction->setOn(myApp()->isMDI());
  mdiAction->setParentMenuText(tr("Options"));
  
  generalAction = new CAction(parent, "generalAction");
  generalAction->setMenuText(tr("&General"));
  generalAction->setText(tr("General"));
  generalAction->setIconSet(getPixmapIcon("hammerIcon"));
  generalAction->setParentMenuText(tr("Options"));
  generalAction->setAccel(Qt::CTRL + Qt::Key_O);
  
  mdiAction->addTo(this);
  
#ifndef QT_NO_STYLE  
  themesMenu = new QPopupMenu(this);  
  themesMenu->setCheckable(true);
  insertItem(getPixmapIcon("stylesIcon"), tr("&Themes"), themesMenu);
  connect(themesMenu, SIGNAL(aboutToShow()), this, SLOT(themesMenuAboutToShow()));
#endif
  
  fontsMenu = new QPopupMenu(this);
  applicationFontAction = new CAction(parent, "applicationFontAction");
  applicationFontAction->setMenuText(tr("&Application Font"));
  applicationFontAction->setText(tr("Application Font"));
  applicationFontAction->setIconSet(getPixmapIcon("fontsIcon"));
  applicationFontAction->addTo(fontsMenu);
  applicationFontAction->setParentMenuText(tr("Options | Fonts"));
  connect(applicationFontAction, SIGNAL(activated()), this, SLOT(applicationFontSelection()));

  
  printerFontAction = new CAction(parent, "printerFontAction");
  printerFontAction->setMenuText(tr("&Printer Font"));
  printerFontAction->setText(tr("Printer Font"));
  printerFontAction->setIconSet(getPixmapIcon("printerFontIcon"));
  printerFontAction->addTo(fontsMenu);
  printerFontAction->setParentMenuText(tr("Options | Fonts"));
  connect(printerFontAction, SIGNAL(activated()), this, SLOT(printerFontSelection()));
  
  fontsMenu->insertSeparator();

  queryEditorFontAction = new CAction(parent, "queryEditorFontAction");
  queryEditorFontAction->setMenuText(tr("&Query Editor Font"));
  queryEditorFontAction->setText(tr("Query Editor Font"));
  queryEditorFontAction->addTo(fontsMenu);
  queryEditorFontAction->setParentMenuText(tr("Options | Fonts"));
  connect(queryEditorFontAction, SIGNAL(activated()), this, SLOT(queryEditorFontSelection()));
 
  insertItem(getPixmapIcon("fontsIcon"), tr("&Fonts"), fontsMenu);
  
  insertSeparator();
  
  generalAction->addTo( this );
  
  menuBar->insertItem(tr("&Options"), this);
  connect(mdiAction, SIGNAL(activated()), this, SLOT(toggleMDI()));  
  connect(generalAction, SIGNAL(activated()), this, SLOT(showGeneralConfig()));
}

/*
This class toggles the MDI option and writes its status to the main config file
*/
void COptionsMenu::toggleMDI()
{
#ifdef DEBUG
  qDebug("COptionsMenu::toggleMDI()");
#endif
  
  CConfig *cfg = new CConfig();  
  cfg->writeEntry("MDI", booltostr(mdiAction->isOn()));
  cfg->save();
  delete cfg;  
  QMessageBox::information(0, tr("Restart"), tr("You will need to restart mysqlcc before you notice your changes."));
}

/*
The Themes menu is created dynamically.  This function creates the Themes menu before
its shown.
*/
void COptionsMenu::themesMenuAboutToShow()
{
#ifndef QT_NO_STYLE
#ifdef DEBUG
  qDebug("COptionsMenu::themesMenuAboutToShow()");
#endif
  
  themesMenu->clear();
  QStringList list = QStyleFactory::keys();
  list.sort();
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
    int id = themesMenu->insertItem(*it, this, SLOT(styleSelected(int)));
    themesMenu->setItemChecked(id, myApp()->currentStyle() == *it);
  }
#endif
}


/*
This function sets the Application Style and writes the selected style to the configuration
file.
*/
void COptionsMenu::styleSelected(int id)
{
#ifndef QT_NO_STYLE
#ifdef DEBUG
  qDebug("COptionsMenu::styleSelected()");
#endif
  
  myApp()->setCurrentStyle(themesMenu->text(id));
  CConfig *cfg = new CConfig();  
  cfg->writeEntry("Style", myApp()->currentStyle());
  cfg->save();  
  myApp()->setStyle(myApp()->currentStyle());
  myApp()->setStylePalette(myApp()->currentStyle());
  delete cfg;
#endif
}

/*
This function sets the Application Font and saves the configuration to the general config file
*/
void COptionsMenu::applicationFontSelection()
{
#ifdef DEBUG
  qDebug("COptionsMenu::applicationFontSelection()");
#endif
  
  bool ok;
  QFont font = QFontDialog::getFont( &ok, myApp()->font());
  if (ok)
  {
    myApp()->setFont(font, true);
    CConfig *cfg = new CConfig();    
    cfg->writeEntry("Font", font.toString());
    cfg->save();
    delete cfg;    
  }
}

/*
This function sets the Printer Font and saves the configuration to the general config file.
*/
void COptionsMenu::printerFontSelection()
{
#ifdef DEBUG
  qDebug("COptionsMenu::printerFontSelection()");
#endif
  
  bool ok;
  QFont font = QFontDialog::getFont( &ok, myApp()->printerFont());
  if (ok)
  {
    CConfig *cfg = new CConfig();
    cfg->writeEntry("Printer Font", font.toString());
    myApp()->setPrinterFont(font);
    cfg->save();
    delete cfg;    
  }
}

void COptionsMenu::openConfigScreen(int tab)
{
#ifdef DEBUG
  qDebug("static COptionsMenu::openConfigScreen()");
#endif
  
  CAppConfigDialog *config = new CAppConfigDialog(myApp()->workSpace());
  config->setCurrentPage(tab);
  myShowWindow(config);
}

void COptionsMenu::queryEditorFontSelection()
{
#ifdef DEBUG
  qDebug("COptionsMenu::queryEditorFontSelection()");
#endif
  
  openConfigScreen(3);
}

void COptionsMenu::showGeneralConfig()
{
#ifdef DEBUG
  qDebug("COptionsMenu::showGeneralConfig()");
#endif
  
  openConfigScreen();
}
