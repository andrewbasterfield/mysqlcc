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

#include <qvariant.h>
#include <qmainwindow.h>
#include "CMyWindow.h"

class QGridLayout; 
class QPopupMenu;
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
  QPopupMenu *consoleMenu;
  QPopupMenu *windowMenu;
  QPopupMenu *optionsMenu;
  QPopupMenu *helpMenu;    
  QGridLayout* CMDIWindowLayout;
  void closeEvent(QCloseEvent * e);  
};

#endif // CMDIWINDOW_H
