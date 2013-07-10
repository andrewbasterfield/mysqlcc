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
#include "CMyWindow.h"
#include "globals.h"
#include "panels.h"
#include "CConfig.h"
#include "config.h"
#include <qapplication.h>
#include <qregexp.h>
#include <qobjectlist.h>

#ifdef QT_OSX_BUILD
#include <Carbon/Carbon.h>  
#endif 

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 2
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

/*
CMyWindow is the base class for handling windows.  Basically, its main purpose is to provide functionality
for loading & saving window sizes & coordenates.
*/
CMyWindow::CMyWindow(QWidget * parent, const char * name, bool appwindow, WFlags f)
: QMainWindow(parent, name, f), isapplicationwindow(appwindow) 
{
#ifdef DEBUG
  qDebug("CMyWindow::CMyWindow()");
#endif
  
  if (!myApp()->isMDI() && !isapplicationwindow)
    myApp()->childWindows()->append(this);
  
  windowstate = 0;
  messagewindow = 0;
  messagepanel = 0;
}

void CMyWindow::enableMessageWindow(bool e)
{
#ifdef DEBUG
  qDebug("CMyWindow::enableMessageWindow(%s)", debug_string(booltostr(e)));
#endif
  
  if (messagewindow != 0 && !e)
  {
    delete messagewindow;
    messagewindow = 0;
  }
  else
    if (e && messagewindow == 0)
    {
      messagewindow = new CMessageWindow(QDockWindow::InDock, this, "MessagePanel");
      messagepanel = new CMessagePanel(tr("Messages"));    
      messagewindow->addPanel(messagepanel);
      messagewindow->setFixedExtentHeight(90);
      moveDockWindow(messagewindow, DockBottom);
    }
}

/*
This function returns the current window state
*/
int CMyWindow::getWindowState()
{
#ifdef DEBUG
  qDebug("CMyWindow::getWindowState()");
#endif
  
  if (isMinimized())  
    return WState_Minimized;
  else
    if (isMaximized())
      return WState_Maximized;
    else
      return 0;      
}


/*
This function first tries to load the current window settings (sizes & coordenates) and if it fails
it resizes the windown to w, h.

For conveniance, this function also "places" the window automatically.
*/
void CMyWindow::myResize(int w, int h)
{
#ifdef DEBUG
  qDebug("CMyWindow::myResize()");
#endif
  
  if (!loadWindowSettings())
    resize(w, h);
  
  if (!myApp()->isMDI() && !isapplicationwindow)
    autoPlace();
}


/*
Reimplemented member that handles saving positions when closing a window.
*/
void CMyWindow::closeEvent(QCloseEvent *e)
{
#ifdef DEBUG
  qDebug("CMyWindow::closeEvent()");
#endif
  
  beforeClose();
  emit about_to_close();
  e->accept();
}


/*
This function gets the current coordenates of the window and then saves them to the general config file
*/
void CMyWindow::beforeClose()
{
#ifdef DEBUG
  qDebug("CMyWindow::beforeClose()");
#endif
  
  position = (myApp()->isMDI() && !isapplicationwindow) ? parentWidget()->pos() : myPos();
  saveWindowSettings();
}


/*
This function loads the window's settings from the general config file and resizes the window accordingly.
*/
bool CMyWindow::loadWindowSettings()
{
#ifdef DEBUG
  qDebug("CMyWindow::loadWindowSettings()");
#endif

  CConfig *cfg = new CConfig();
  bool ret = false;
  QString s = cfg->readStringEntry("Window " + QString(name()), QString::null);  
  if (!s.isNull())  
  {
    s = charReplace(s, '|', "\n");
    QTextStream ts( &s, IO_ReadOnly);
    ts >> *this;    
  }

  s = cfg->readStringEntry(name(), QString::null);
  if (!s.isNull())  
  {
    QRegExp rx("^(\\d+),(\\d+|-\\d+),(\\d+|-\\d+),(\\d+),(\\d+)$");
    if (rx.search(s) != -1)
    {
      windowstate = rx.cap(1).toInt();
      if (windowstate == 0 || windowstate == WState_Minimized || !isapplicationwindow)
      {
        if (isapplicationwindow)
        {
          int y = rx.cap(3).toInt();
          if (y < 0)
            y = 0;          
          setGeometry(rx.cap(2).toInt(), y, rx.cap(4).toInt(), rx.cap(5).toInt());
        }
        else
          resize(rx.cap(4).toInt(), rx.cap(5).toInt());
      }
      else
        showMaximized();
      
      ret = true;
    }    
  }
  delete cfg;
  
  QString f = QString(name()).lower() + ".cfg";
  QString p =  QString(HOTKEY_PATH);
  if (CConfig::exists(f, p)) //Read the Hotkey configuration for this window  
  {    
    QObjectList *l = queryList("QAction", 0, true, false);
    if (!l->isEmpty())
    {
      QObjectListIt it(*l);
      CAction *action;
      CConfig *cfg_hotkeys = new CConfig(f, p);
      while ((action = (CAction *) it.current()) != 0)
      {
        ulong accel = cfg_hotkeys->readNumberEntry(action->name(), 0xFFFFFF);
        if (accel != 0xFFFFFF)
          action->setAccel(accel);
        ++it;
      }
      delete cfg_hotkeys;
    }
  }
  return ret;
}

/*
This function writes the current window settings to the general config file.
*/
void CMyWindow::saveWindowSettings()
{
#ifdef DEBUG
  qDebug("CMyWindow::saveWindowSettings()");
#endif
  
  QString s;
  if (position.y() < 0)
    position.setY(0);
  s.sprintf("%d,%d,%d,%d,%d", getWindowState(), position.x(), position.y(), width(), height());
  CConfig *cfg = new CConfig();
  saveSettings(cfg);
  cfg->writeEntry(name(), s);
  s = QString::null;
  QTextStream ts( &s, IO_WriteOnly );
  ts << *this;
  s = charReplace(s, '\n', "|");
  cfg->writeEntry("Window " + QString(name()), s);  
  cfg->save();
  delete cfg;  
}

Qt::Dock CMyWindow::findDockWindow(CMyWindow *wnd, QDockWindow *dockWnd)
{
#ifdef DEBUG
  qDebug("static CMyWindow::findDockWindow()");
#endif

  if (wnd->leftDock()->hasDockWindow(dockWnd))
    return Qt::DockLeft;
  else
    if (wnd->rightDock()->hasDockWindow(dockWnd))
      return Qt::DockRight;
    else
      if (wnd->topDock()->hasDockWindow(dockWnd))
        return Qt::DockTop;
      else
        if (wnd->bottomDock()->hasDockWindow(dockWnd))
          return Qt::DockBottom;
       else
          return (Qt::Dock)0;
}

CMyWindow::~CMyWindow()
{
#ifdef DEBUG
  qDebug("CMyWindow::~CMyWindow()");
#endif  
  
  if (!isapplicationwindow)
    myApp()->childWindows()->remove(this);
}

/*
This function is used only when mysqlcc is NOT running in MDI mode ... what it does is it handles the
placement of the windows in a "smart" manner.
*/
void CMyWindow::autoPlace()
{
#ifdef DEBUG
  qDebug("CMyWindow::autoPlace()");
#endif

  if (myApp()->isMDI() || isapplicationwindow)
    return;
  
  int overlap, minOverlap = 0;
  int possible;
  
  QRect r1(0, 0, 0, 0);
  QRect r2(0, 0, 0, 0);

  QRect maxRect = qApp->desktop()->rect();
  int x = maxRect.left(), y = maxRect.top();
  QPoint wpos(maxRect.left(), maxRect.top());
#ifdef QT_OSX_BUILD
  OSStatus macx_retcode;
  SInt16 macx_mbarh;
  if (0 == GetThemeMenuBarHeight(&macx_mbarh))
  	y += macx_mbarh;
#endif 
  
  bool firstPass = true;  
  do
  {
    if (y + height() > maxRect.bottom())
      overlap = -1;
    else
      if (x + width() > maxRect.right())
        overlap = -2;
      else
      {
        overlap = 0;
        
        r1.setRect(x, y, width(), height());
        
        CMyWindow *l;
        for (l = myApp()->childWindows()->first(); l; l = myApp()->childWindows()->next())
        {
          if (!l->isHidden() && l != this)
          {
            r2.setRect(l->x(), l->y(), l->width(), l->height());
            
            if (r2.intersects(r1))
            {
              r2.setCoords(QMAX(r1.left(), r2.left()),
                QMAX(r1.top(), r2.top()),
                QMIN(r1.right(), r2.right()),
                QMIN(r1.bottom(), r2.bottom())
                );
              
              overlap += (r2.right() - r2.left()) * (r2.bottom() - r2.top());
            }
          }
        }
      }
      
    if (overlap == 0)    
    {
      wpos = QPoint(x, y);
      break;
    }
      
    if (firstPass)
    {
      firstPass = false;
      minOverlap = overlap;
    }
    else
      if ( overlap >= 0 && overlap < minOverlap)
      {
        minOverlap = overlap;
        wpos = QPoint(x, y);
      }

    if (overlap > 0)
    {
      possible = maxRect.right();
      if (possible - width() > x)
        possible -= width();         
      CMyWindow *l;
      for (l = myApp()->childWindows()->first(); l; l = myApp()->childWindows()->next())
      {
        if (!l->isHidden() && l != this)
        {
          r2.setRect(l->x(), l->y(), l->width(), l->height());             
          if( ( y < r2.bottom() ) && ( r2.top() < height() + y ))
          {
            if( r2.right() > x )
              possible = possible < r2.right() ? possible : r2.right();               
            if( r2.left() - width() > x)
              possible = possible < r2.left() - width() ? possible : r2.left() - width();
          }
        }
      }
          
      x = possible;
    }
    else
      if ( overlap == -2 )
      {
        x = maxRect.left();
        possible = maxRect.bottom();
            
        if ( possible - height() > y )
          possible -= height();
            
        CMyWindow *l;
        for (l = myApp()->childWindows()->first(); l; l = myApp()->childWindows()->next())
        {
          if (l != this)
          {
            r2.setRect(l->x(), l->y(), l->width(), l->height());                
            if (r2.bottom() > y)
              possible = possible < r2.bottom() ? possible : r2.bottom();
                
            if (r2.top() - height() > y )
              possible = possible < r2.top() - height() ? possible : r2.top() - height();
          }
        }      
        y = possible;
      }
  }
  while (overlap != 0 && overlap != -1);
  move(wpos);
}

void CMyWindow::setCaption(const QString &s)
{
  if (QMainWindow::caption() == s)
    return;

  if (myApp()->isMDI())
    QMainWindow::setCaption(s);
  else
  {
    QString app_name(SHORT_NAME);
    app_name += " - ";
    if (s.startsWith(app_name))
      QMainWindow::setCaption(s);
    else
      QMainWindow::setCaption(app_name + s);
  }
}
