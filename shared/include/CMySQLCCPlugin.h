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
#ifndef CMYSQLCCPLUGIN_H
#define CMYSQLCCPLUGIN_H

#include <qwidget.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qworkspace.h>
#include <qvaluelist.h>
#include <qtabwidget.h>
#include <qtabwidget.h>
#include <qmainwindow.h>
#include "CMySQL.h"

namespace Plugin
{
  enum mysqlcc_plugin_place { NONE, SERVER_ITEM, DATABASEGROUP_ITEM,
       DATABASE_ITEM, TABLEGROUP_ITEM, TABLE_ITEM, SERVER_ADMIN_ITEM,
       USER_ADMIN_ITEM, USER_ITEM, MAIN, CONSOLE_WINDOW, QUERY_WINDOW };

  enum mysqlcc_plugin_type { NO_TYPE, WIDGET, NO_WIDGET, QUERY_PARSER };

typedef QValueList<mysqlcc_plugin_place> CPluginPlaceList;
}


#ifdef MYSQLCC_PLUGIN
#  ifdef __cplusplus
#    define MYSQLCC_EXTERN_C extern "C"
#  else
#    define MYSQLCC_EXTERN_C extern
#  endif
#  ifdef WIN32
#    if defined(__BORLANDC__) || defined(__TURBOC__)
#      define MYSQLCC_STDCALL __stdcall
#    else
#      define MYSQLCC_STDCALL
#    endif
#    define MYSQLCC_EXPORT __declspec(dllexport)
#  else
#    define MYSQLCC_STDCALL
#    define MYSQLCC_EXPORT
#  endif
#  define MYSQLCC_EXPORT_PLUGIN(PLUGIN, TYPE) \
    MYSQLCC_EXTERN_C MYSQLCC_EXPORT \
    CMySQLCCPlugin * MYSQLCC_STDCALL instanciate_mysqlcc_plugin() \
    { return new PLUGIN; } \
    MYSQLCC_EXTERN_C MYSQLCC_EXPORT \
    Plugin::mysqlcc_plugin_type MYSQLCC_STDCALL get_mysqlcc_plugin_type() \
    { return TYPE; }
#else
#  define MYSQLCC_EXPORT
#endif

class MYSQLCC_EXPORT CMySQLCCPlugin
{
public:
  CMySQLCCPlugin()
  {
    p_mysql = 0;
    app = 0;
    workspace = 0;
    confirm_critical = false;
  }

  virtual ~CMySQLCCPlugin() {} 
  virtual void initPluginPlace(Plugin::CPluginPlaceList *plugin_place) { plugin_place->clear(); }
  virtual QString author() const { return QString::null; }
  virtual QString description() const { return QString::null; }
  virtual QString about() const { return QString::null; }
  virtual QString version() const { return QString::null; }
  virtual QString name() const { return QString::null; }
  virtual void init() {}
  virtual void exec() {}  //Used in NO_WIDGET Plugins.

  QString text() const { return m_text; }
  QPixmap icon() const { return m_icon; }

  void setMySQL(CMySQL *m) { p_mysql = m; }
  void setTable(const QString &s) { m_table = s; }
  void setConnectionName(const QString &s) { connection_name = s; }
  void setApplication(QApplication *a) { app = a; }
  void setConfigPath(const QString &s) { config_path = s; }
  void setLanguage(const QString &s) { lang = s; }
  void setConfirmCritical(bool b) { confirm_critical = b; }
  void setWorkSpace(QWorkspace *w) { workspace = w; }
  void setDefaultConnectionName(const QString &s) { default_connection_name = s; }
  void setPluginsPath(const QString &s) { plugins_path = s; }


protected:
  CMySQL * mysql() const { return p_mysql; }
  QString table() const { return m_table; }
  QString connectionName() const { return lang; }
  QApplication *application() { return app; }
  QString configPath() const { return config_path; }
  QString language() const { return lang; }
  bool confirmCritical() const { return confirm_critical; }
  QWorkspace *workSpace() const { return workspace; }
  QString defaultConnectionName() const { return default_connection_name; }
  QString pluginsPath() const { return plugins_path; }

  void setText(const QString &s) { m_text = s; }
  void setIcon(const QPixmap &i) { m_icon = i; }

private:
  CMySQL *p_mysql;
  QApplication *app;
  QWorkspace *workspace;
  bool confirm_critical;
  QString lang;
  QString connection_name;
  QString default_connection_name;
  QString config_path;
  QString m_table;
  QString plugins_path;
  QPixmap m_icon;
  QString m_text;
};

class MYSQLCC_EXPORT CMySQLCCQueryParserPlugin : public QObject, public CMySQLCCPlugin
{
  Q_OBJECT

public:
  CMySQLCCQueryParserPlugin() : CMySQLCCPlugin() {}
  enum p_message_type { CRITICAL_MSG, WARNING_MSG, INFORMATION_MSG };

  virtual QString parse(const QString &) { return QString::null; }  //Used in QUERY_PARSER Plugins

signals:
  void p_message(const QString &, p_message_type);
  void my_message(const QPixmap &, const QString &);

protected slots:
  void critical(const QString &s) { emit p_message(s, CRITICAL_MSG); }
  void warning(const QString &s) { emit p_message(s, WARNING_MSG); }
  void information(const QString &s) { emit p_message(s, INFORMATION_MSG); }
  void message(const QString &s) { message(QPixmap(), s); }
  void message(const QPixmap &p, const QString &s) { emit my_message(p, s); }
};


class MYSQLCC_EXPORT CMySQLCCWidgetPlugin : public CMySQLCCQueryParserPlugin
{
  Q_OBJECT

public:  

  CMySQLCCWidgetPlugin() : CMySQLCCQueryParserPlugin()
  {
    has_message_panel = false;
    plugin_window = 0;
    messages_tab_widget = 0;
    message_panel_visible = false;
  }

  virtual QWidget * createWidgetPlugin(QWidget *, const char *) { return 0; }  //Used in WIDGET Plugins

  void setPluginWindow(QMainWindow *w) { plugin_window = w; }
  void setHasMessagePanel(bool b) { has_message_panel = b; }
  void setMessagePanelVisible(bool b) { message_panel_visible = b; }
  void setMessageTabWidget(QTabWidget *w) { messages_tab_widget = w; }

signals:
  void message_panel();
  void show_message_panel();
  void hide_message_panel();
  void enable_hotkey_editor();

protected:
  QMainWindow *pluginWindow() const { return plugin_window; }
  bool hasMessagePanel() const { return has_message_panel; }  
  void messagePanel() { emit message_panel(); }
  void showMessagePanel() { emit show_message_panel(); }
  void hideMessagePanel() { emit hide_message_panel(); }
  void enableHotKeyEditor() { emit enable_hotkey_editor(); }
  bool messagePanelVisible() { return message_panel_visible; }
  QTabWidget * messagesTabWidget() const { return messages_tab_widget; }
  
private:
  QMainWindow *plugin_window;
  bool has_message_panel;
  bool message_panel_visible;
  QTabWidget *messages_tab_widget;
};

#endif

