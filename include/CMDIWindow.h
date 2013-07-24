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
#ifndef CMDIWINDOW_H
#define CMDIWINDOW_H

#include <stddef.h>
#include <qvariant.h>
#include <q3mainwindow.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3PopupMenu>
#include "CMyWindow.h"

class Q3GridLayout; 
class Q3PopupMenu;
class CConsoleWindow;
class QWorkspace;

class CMDIWindow : public CMyWindow
{ 
  Q_OBJECT
    
public:
  CMDIWindow(QWidget* parent = 0);
  void createConsoleWindow();
  
private slots:
  void windowMenuAboutToShow();
  void closeAllWindows();
  void closeActiveWindow();
  void consoleWindowDisplay();
  void windowActivated(int id);
  
private:
  CConsoleWindow*	consoleWindow;
  QMenuBar *menubar;
  Q3PopupMenu *consoleMenu;
  Q3PopupMenu *windowMenu;
  Q3PopupMenu *optionsMenu;
  Q3PopupMenu *helpMenu;    
  Q3GridLayout* CMDIWindowLayout;
  void closeEvent(QCloseEvent * e);  
};

#endif // CMDIWINDOW_H
