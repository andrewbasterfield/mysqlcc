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
#ifndef CPLUGINWINDOW_H
#define CPLUGINWINDOW_H

#include "CMyWindow.h"
#include "CPlugin.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;

class CPluginMessage
{
public:
  CPluginMessage(const QPixmap &p, const QString &s)
  : message(s), pixmap(p) {}

  QString message;
  QPixmap pixmap;
};

class CPluginWindow : public CMyWindow
{
    Q_OBJECT

public:
  CPluginWindow(QWidget * parent, const CPlugin &p);
  ~CPluginWindow();
  void setPluginWidget(QWidget *w);

private slots:
  void p_message(const QString &, CMySQLCCWidgetPlugin::p_message_type);
  void my_message(const QPixmap &, const QString &);
  void message_panel();
  void show_message_panel();
  void hide_message_panel();
  void enable_hotkey_editor();

protected:
  void customEvent(QCustomEvent *);

private:
  QWidget* pluginWidget;
  CMySQLCCWidgetPlugin *plugin;
  QGridLayout* CPluginWindowLayout;

  void display_message_panel(bool);
};

#endif

