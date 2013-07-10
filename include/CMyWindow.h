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
#ifndef CMYWINDOW_H
#define CMYWINDOW_H

#include <qvariant.h>
#include <qmainwindow.h>
#include <qdockarea.h>
#include <qmenubar.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qpixmap.h>

#include "CAction.h"

class CMessageWindow;
class CMessagePanel;
class CConfig;

class CMyWindow : public QMainWindow
{ 
  Q_OBJECT
    
public:
  CMyWindow (QWidget * parent, const char * name=0, bool appwindow = false, WFlags f = WDestructiveClose);
  ~CMyWindow();  
  void autoPlace();

  bool isApplicationWindow() const { return isapplicationwindow; }
  void myResize (int w, int h);
  const QPoint myPos() { return QPoint(geometry().x(), geometry().y()); }
  int getWindowState();
  int windowState() const { return windowstate; }  
  CMessageWindow * messageWindow() const { return messagewindow; }  
  CMessagePanel * messagePanel() const { return messagepanel; }
  
  virtual void beforeClose();
  virtual void saveWindowSettings();
  virtual bool loadWindowSettings();
  virtual void setCaption(const QString &s);
  
  static Qt::Dock findDockWindow(CMyWindow *wnd, QDockWindow *dockWnd);

signals:
  void about_to_close();

protected:
  QPoint position;
  virtual void closeEvent(QCloseEvent * e);
  virtual void enableMessageWindow(bool e);
  virtual void saveSettings(CConfig *) {}
  
private:
  bool isapplicationwindow;
  int windowstate;
  CMessageWindow *messagewindow;
  CMessagePanel *messagepanel;
};

#endif
