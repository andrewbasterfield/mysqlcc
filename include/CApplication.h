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
#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include <stddef.h>
#include <qapplication.h>
#include <qmap.h>
#include <qworkspace.h>
#include <qptrlist.h>
#include "CMyWindow.h"
#include "CPlugin.h"
#include "config.h"

class QFont;
class CConsoleWindow;
class QPixmap;
class QTranslator;

class CApplication : public QApplication
{
  Q_OBJECT
    
public:
  enum commenting_style { NO_COMMENT, HASH_COMMENT, C_COMMENT, DASH_COMMENT };

  CApplication (int & argc, char ** argv);
  ~CApplication();

  QWorkspace *workSpace() const { return workspace; }
  void createWorkspace(QWidget *parent);

  void setMainConsoleWindow(CConsoleWindow *w) { console_window = w; }
  CConsoleWindow * consoleWindow() const { return console_window; }

  bool isMDI() const { return ismdi; }
  void setMDI(bool b) { ismdi = b; }

  void setConfirmCritical(bool b) { confirm_critical = b; }
  bool confirmCritical() const { return confirm_critical; }

  bool checkCritical() { return (check_critical == 0); }
  void incCritical() { check_critical++; }
  void decCritical() { check_critical--; }

  QString defaultConnectionName() { return default_connection_name; }
  void setDefaultConnectionName(const QString &s) { default_connection_name = s; }

  QFont printerFont() const { return printer_font; }
  void setPrinterFont(const QFont &font) { printer_font = font; }

  QString currentStyle() const { return current_style; }
  void setCurrentStyle(const QString &s) { current_style = s; }

  QString warningSoundFile() const { return warning_sound_file; }
  void setWarningSoundFile(const QString &s) { warning_sound_file = s; }
  
  QString errorSoundFile() const { return error_sound_file; }
  void setErrorSoundFile(const QString &s) { error_sound_file = s; }
  
  QString informationSoundFile() const { return information_sound_file; }
  void setInformationSoundFile(const QString &s) { information_sound_file = s; }
  
  QString translationsPath() const { return translations_path; }
  void setTranslationsPath(const QString &s) { translations_path = s; }
  
  QString pluginsPath() const { return plugins_path; }
  void setPluginsPath(const QString &s) { plugins_path = s; }

  QStringList enabledPluginsList() const { return enabled_plugins_list; }
  void setEnabledPluginsList(const QStringList &s) { enabled_plugins_list = s; }
  
  QString currentLanguage() const { return current_language; }
  void setCurrentLanguage(const QString &s) { current_language = s; }

  bool saveWorkspace() const { return save_workspace; }
  void setSaveWorkspace(bool b) { save_workspace = b; }
  
  QString syntaxFile() const { return syntax_file; }
  void setSyntaxFile(const QString &s) { syntax_file = s; }

  bool syntaxHighlight() const { return syntax_highlight; }
  void setSyntaxHighlight(bool b) { syntax_highlight = b; }

  bool parenthesesMatching() const { return parentheses_matching; }
  void setParenthesesMatching(bool b) { parentheses_matching = b; }

  bool enableSqlPanel() const { return enable_sql_panel; }
  void setEnableSqlPanel(bool b) { enable_sql_panel = b; }

  bool retrieveAllRecordsFromTable() const { return retrive_all_records_from_table; }
  void setRetrieveAllRecordsFromTable(bool b) { retrive_all_records_from_table = b; }

  commenting_style commentStyle() const { return comment_style; }
  void setCommentingStyle(commenting_style c) { comment_style = c; }

  bool linuxPaste() const { return linux_paste; }
  void setLinuxPaste(bool b) { linux_paste = b; }

  QPtrList<CMyWindow> *childWindows() { return &child_windows; }

  QMap<QString, QPixmap> * loadedPlugins() { return &loaded_plugins; }
  
  void reset();
  QPixmap getPixmapIcon(const QString &key) const;

  static void setStylePalette(const QString &style);  
  static CApplication * Application() { return (CApplication *) qApp; }
  static QString commentText(const QString &text, const QString &eol, commenting_style t = NO_COMMENT);
 
  PluginMap * pluginsMap() { return &plugin_map; }
 
private:
  void initIcons();
  void initGlobals(bool startup = false);

  PluginMap plugin_map;

  QTranslator *translator;
  QMap<QString, QPixmap> icons_map;
  
  CConsoleWindow *console_window;
  QPtrList<CMyWindow> child_windows;
  QFont printer_font;
  QWorkspace *workspace;

  QString current_style;
  QString warning_sound_file;
  QString error_sound_file;
  QString information_sound_file;
  QString translations_path;  
  QString current_language;
  QString syntax_file;
  QString plugins_path;
  QString default_connection_name;

  bool save_workspace;
  bool confirm_critical;
  bool ismdi;
  bool syntax_highlight;
  bool parentheses_matching;
  bool enable_sql_panel;
  bool retrive_all_records_from_table;
  bool linux_paste;
  QMap<QString, QPixmap> loaded_plugins;
  QStringList enabled_plugins_list;

  ulong check_critical;

  commenting_style comment_style;
};

#endif
