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
#include "CPluginWindow.h"
#include "CConfig.h"
#include "globals.h"
#include "config.h"
#include "panels.h"
#include "CHotKeyEditorDialog.h"
#include <stddef.h>
#include <qmessagebox.h>
#include <qvariant.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qlibrary.h>

CPluginWindow::CPluginWindow(QWidget * parent, const CPlugin &p)
: CMyWindow(parent), pluginWidget(0), plugin(0)
{
  myApp()->incCritical();
#ifndef NO_MYSQLCC_PLUGINS
  if (p.name().isEmpty())
    setName("CPluginWindow");
  else
    setName(p.name());
  setCaption(p.name());
  setIcon(p.icon().isNull() ? getPixmapIcon("applicationIcon") : p.icon());
  setCentralWidget(new QWidget(this, "qt_central_widget"));
  CPluginWindowLayout = new QGridLayout(centralWidget(), 1, 1, 4, 2, "CPluginWindowLayout");

  plugin = (CMySQLCCWidgetPlugin *) CPlugin::createPlugin(p);
  if (plugin)
  {
    plugin->setPluginWindow(this);
    connect (plugin, SIGNAL(p_message(const QString &, p_message_type)), this, SLOT(p_message(const QString &, p_message_type)));
    connect (plugin, SIGNAL(my_message(const QPixmap &, const QString &)), this, SLOT(my_message(const QPixmap &, const QString &)));
    connect (plugin, SIGNAL(message_panel()), this, SLOT(message_panel()));
    connect (plugin, SIGNAL(show_message_panel()), this, SLOT(show_message_panel()));
    connect (plugin, SIGNAL(hide_message_panel()), this, SLOT(hide_message_panel()));
    connect (plugin, SIGNAL(enable_hotkey_editor()), this, SLOT(enable_hotkey_editor()));

    plugin->init();

    pluginWidget = plugin->createWidgetPlugin(centralWidget(), p.pluginFile());
    if (pluginWidget)
      CPluginWindowLayout->addWidget(pluginWidget, 0, 0);
  }
#endif
  clearWState( WState_Polished );
  myResize(600, 400);
}

CPluginWindow::~CPluginWindow()
{
  if (pluginWidget)
    delete pluginWidget;

  if (plugin)
    delete plugin;

  myApp()->decCritical();
}

void CPluginWindow::setPluginWidget(QWidget *w)
{
  CPluginWindowLayout->addWidget(w, 0, 0 );
  pluginWidget = w;
}

void CPluginWindow::p_message(const QString &s, CMySQLCCWidgetPlugin::p_message_type type)
{
  QString icn;
  switch (type)
  {
  case CMySQLCCWidgetPlugin::CRITICAL_MSG:
    if (!messagePanel())
      QMessageBox::critical(0, tr("Critical"), s);
    else
      icn = "criticalIcon";
    break;


  case CMySQLCCWidgetPlugin::WARNING_MSG:
    if (!messagePanel())
      QMessageBox::warning(0, tr("Warning"), s);
    else
      icn = "warningIcon";
    break;

  case CMySQLCCWidgetPlugin::INFORMATION_MSG:
    if (!messagePanel())
      QMessageBox::warning(0, tr("Information"), s);
    else
      icn = "informationIcon";
    break;
  }

  if (messagePanel())
    my_message(getPixmapIcon(icn), s);
}

void CPluginWindow::customEvent(QCustomEvent *event)
{
  switch ((int) event->type())
  {
  case MESSAGE:
    {
      CPluginMessage *evt = (CPluginMessage *) event->data();
      if (messagePanel())
      {
        if (!evt->pixmap.isNull())
          messagePanel()->showMessage(evt->pixmap, evt->message);
        else
          messagePanel()->showMessage(evt->message);
      }
      else
      {        
        QMessageBox m;
        m.setText(evt->message);
        m.setCaption(tr("Message"));
        if (!evt->pixmap.isNull())
          m.setIcon(evt->pixmap);
        m.exec();
      }
      delete evt;
    }
    break;
  }
}

void CPluginWindow::my_message(const QPixmap &p, const QString &s)
{
  QCustomEvent *event = new QCustomEvent(MESSAGE);
  event->setData(new CPluginMessage(p, s));
  QApplication::postEvent(this, event);
}

void CPluginWindow::message_panel()
{
  if (!messagePanel())
  {
    enableMessageWindow(true);
    plugin->setHasMessagePanel(true);
    plugin->setMessagePanelVisible(true);  //Message panel is always shown when created
    plugin->setMessageTabWidget(messageWindow()->tabWidget());
  }
}

void CPluginWindow::enable_hotkey_editor()
{
  if (menuBar())
    new CHotKeyEditorMenu(this, menuBar(), "HotKeyEditor");
}

void CPluginWindow::display_message_panel(bool b)
{
  if (messagePanel())
  {
    if (b)
      messagePanel()->show();
    else
      messagePanel()->hide();
    plugin->setMessagePanelVisible(b);
  }
}

void CPluginWindow::show_message_panel()
{
  display_message_panel(true);
}

void CPluginWindow::hide_message_panel()
{
  display_message_panel(false);
}
