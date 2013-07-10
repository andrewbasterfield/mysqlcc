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
#ifndef SHARED_MENUS_H
#define SHARED_MENUS_H

#include <qmenubar.h>
#include <qfontdialog.h>
#include <qstylefactory.h>
#include "CAction.h"
#include "CPlugin.h"

class CActionGroup;
class QPopupMenu;
class QFontDialog;
class QStyleFactory;
class CPlugin;


class CPluginAction : public CAction
{
  Q_OBJECT
public:
  CPluginAction(QObject * parent, const char *name=0)
  : CAction(parent, name)
  {
    connect(this, SIGNAL(activated()), this, SLOT(Activated()));
  }
  void setPlugin(const CPlugin &p) { pl = p; }  

private slots:
  void Activated()
  {
    emit my_activated(pl);
  }

signals:
  void my_activated(const CPlugin &);

private:
  CPlugin pl;
};

class CPluginsMenu : public QPopupMenu
{ 
  Q_OBJECT
    
public:
  CPluginsMenu(QWidget* parent = 0, QMenuBar *menuBar = 0, const char* name = 0);

private slots:  
  void Activated(const CPlugin &);
};


class CHelpMenu : public QPopupMenu
{ 
  Q_OBJECT
    
public:
  CHelpMenu(QWidget* parent = 0, QMenuBar *menuBar = 0, const char* name = 0);

public slots:  
  void contentsAction();
  void aboutAction();
  void creditsAction();
  
private:
  CAction*	  	helpContentsAction;
  CAction*	  	helpAboutAction;
  CAction*	  	helpCreditsAction;
  CAction*	  	helpWhatIsThisAction;
};


class COptionsMenu : public QPopupMenu
{ 
  Q_OBJECT
    
public:
  COptionsMenu( QWidget* parent = 0, QMenuBar *menuBar = 0, const char* name = 0);
  ~COptionsMenu() {};
  static void openConfigScreen(int tab=-1);
 
public slots:  
  void toggleMDI();
  void showGeneralConfig();
  void applicationFontSelection();
  void printerFontSelection();  
  void queryEditorFontSelection();
  
private slots:
  void themesMenuAboutToShow();
  void styleSelected(int id);  
  
private:
  CAction*  		mdiAction;
  CAction*      applicationFontAction;
  CAction*      printerFontAction;
  CAction*      queryEditorFontAction;
  QPopupMenu*   themesMenu;
  QPopupMenu*   fontsMenu;  
  CAction*  		generalAction;
};

#endif
