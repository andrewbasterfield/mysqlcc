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
#ifndef CQUERYWINDOWOPTIONSDIALOGTAB_H
#define CQUERYWINDOWOPTIONSDIALOGTAB_H

#include <stddef.h>
#include <qvariant.h>
#include "CConfigDialog.h"

class CMySQLServer;
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QFrame;
class QLabel;
class QSpinBox;
class QButtonGroup;
class QRadioButton;


class CQueryWindowOptionsTab : public CConfigDialogTab
{
  Q_OBJECT
    
public:
  CQueryWindowOptionsTab(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  
protected:
  void setDefaultValues(CConfig *cfg);
  bool save(CConfig *cfg);
  
protected slots:
  virtual void languageChange();
  
private:
  QGridLayout* CQueryWindowOptionsTabLayout;
  QGridLayout* multipleQueriesLayout;
  QCheckBox* followQueryTab;
  QCheckBox* queryNewTab;
  QButtonGroup* multipleQueries;
  QRadioButton* radioButton1;
  QRadioButton* radioButton3;
  QSpinBox* maxResults;
  QCheckBox* followResultsTab;
  QLabel* textLabel1;
};

class CQueryOptionsTab : public CConfigDialogTab
{ 
  Q_OBJECT
    
public:
  CQueryOptionsTab(QWidget* parent, const char* name = 0, WFlags fl = 0);  
  
protected slots:
  virtual void languageChange();
  
protected:
  void setDefaultValues(CConfig *cfg);
  bool save(CConfig *cfg);
  
private:
  QFrame* Frame5;
  QLabel* TextLabel5;
  QComboBox* autocommit;
  QLabel* TextLabel4;
  QLabel* TextLabel9;
  QLabel* TextLabel11;
  QComboBox* sql_big_selects;
  QComboBox* low_priority_updates;
  QComboBox* big_tables;
  QLabel* TextLabel6;
  QComboBox* sql_log_update;
  QLabel* TextLabel10;
  QLabel* TextLabel8;
  QComboBox* query_cache_type;
  QLabel* TextLabel2;
  QComboBox* sql_log_off;
  QComboBox* sql_buffer_result;
  QLabel* TextLabel3;
  QLabel* TextLabel1;
  QComboBox* sql_safe_updates;
  QComboBox* sql_quote_show_create;
  QComboBox* sql_auto_is_null;
  QLabel* TextLabel7;
  QLabel* TextLabel12;
  QCheckBox* force;
  QCheckBox* use_history_file;
  QCheckBox* silent;
  QGridLayout* CQueryOptionsTabLayout;
  QGridLayout* Frame5Layout;
  QHBoxLayout* Layout2;
};

class CQueryWindowOptionsDialog : public CConfigDialog
{ 
  Q_OBJECT
    
public:
  CQueryWindowOptionsDialog(QWidget* parent,  CMySQLServer *m, const char* name=0);
  ~CQueryWindowOptionsDialog();
  
  class query_config_options
  {
  public:

    query_config_options()
    {
    }

    query_config_options(const query_config_options &q)
    {
      use_history_file = q.use_history_file;
      force = q.force;
      options = q.options;
      silent = q.silent;
    }

    bool use_history_file;
    bool force;
    bool silent;
    QMap<QString, QString> options;
  };
  
  typedef QMap<QString, QString> query_config_options_map;

  static query_config_options readConfig(CConfig *m);
  
signals:
  void config_changed();
  
private:
  static void add_option(query_config_options_map *opt, CConfig *cfg, const QString &s);
  CMySQLServer *mysql;
  CConfig *config;
  void okButtonClicked();  
};

#endif // CQUERYWINDOWOPTIONSDIALOGTAB_H
