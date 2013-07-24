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
#include "CQueryWindowOptionsDialog.h"
#include "CMySQLServer.h"
#include "CConfig.h"
#include "config.h"
#include "globals.h"

#include <stddef.h>
#include <qvariant.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <q3frame.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qmessagebox.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>

CQueryWindowOptionsTab::CQueryWindowOptionsTab(QWidget* parent, const char* name, Qt::WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
  if (!name)
    setName("CQueryWindowOptionsTab");
  CQueryWindowOptionsTabLayout = new Q3GridLayout(this, 1, 1, 4, 6, "CQueryWindowOptionsTabLayout"); 
  
  followQueryTab = new QCheckBox(this, "followQueryTab");
  
  CQueryWindowOptionsTabLayout->addMultiCellWidget(followQueryTab, 2, 2, 0, 2);
  
  queryNewTab = new QCheckBox(this, "queryNewTab");
  
  CQueryWindowOptionsTabLayout->addMultiCellWidget(queryNewTab, 3, 3, 0, 2);
  
  multipleQueries = new Q3ButtonGroup(this, "multipleQueries");
  multipleQueries->setColumnLayout(0, Qt::Vertical);
  multipleQueries->layout()->setSpacing(4);
  multipleQueries->layout()->setMargin(6);
  multipleQueries->setExclusive(true);
  multipleQueriesLayout = new Q3GridLayout(multipleQueries->layout());
  multipleQueriesLayout->setAlignment(Qt::AlignTop);
  
  radioButton1 = new QRadioButton(multipleQueries, "radioButton1");
  radioButton1->setChecked(true);
  
  multipleQueriesLayout->addWidget(radioButton1, 0, 0);

  radioButton3 = new QRadioButton(multipleQueries, "radioButton3");
  
  multipleQueriesLayout->addWidget(radioButton3, 1, 0);
  
  CQueryWindowOptionsTabLayout->addMultiCellWidget(multipleQueries, 5, 5, 0, 2);
  
  maxResults = new QSpinBox(this, "maxResults");
  maxResults->setMaxValue(100);
  maxResults->setMinValue(0);
  maxResults->setValue(10);
  
  CQueryWindowOptionsTabLayout->addWidget(maxResults, 0, 1);
  
  followResultsTab = new QCheckBox(this, "followResultsTab");
  followResultsTab->setChecked(true);
  
  CQueryWindowOptionsTabLayout->addMultiCellWidget(followResultsTab, 1, 1, 0, 2);
  
  textLabel1 = new QLabel(this, "textLabel1");
  
  CQueryWindowOptionsTabLayout->addWidget(textLabel1, 0, 0);
  QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CQueryWindowOptionsTabLayout->addItem(spacer, 0, 2);
  QSpacerItem* spacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CQueryWindowOptionsTabLayout->addMultiCell(spacer_2, 6, 6, 0, 1);
  languageChange();
  clearWState(WState_Polished);
  
  // tab order
  setTabOrder(maxResults, followResultsTab);
  setTabOrder(followResultsTab, followQueryTab);
  setTabOrder(followQueryTab, queryNewTab);
  setTabOrder(queryNewTab, radioButton1);
  setTabOrder(radioButton1, radioButton3);
}

void CQueryWindowOptionsTab::languageChange()
{
  setCaption(tr("Query Window Options"));
  followQueryTab->setText(tr("When changing a Result Tab, automatically set the corresponding Query Tab"));
  queryNewTab->setText(tr("When executing a Query, automatically create a new Tab"));
  multipleQueries->setTitle(tr("When executing Multiple Queries that return a Result Set"));
  radioButton1->setText(tr("Execute everything in the same Query Tab"));
  radioButton3->setText(tr("Create a new Query Window for each Query"));
  maxResults->setSpecialValueText(tr("Unlimited"));
  followResultsTab->setText(tr("When changing a Query Tab, automatically set the corresponding Result Tab"));
  textLabel1->setText(tr("Maximum number of Results to Save:"));
}

void CQueryWindowOptionsTab::setDefaultValues(CConfig *cfg)
{
  queryNewTab->setChecked(strtobool(cfg->readStringEntry("Query New Tab", "true")));
  followQueryTab->setChecked(strtobool(cfg->readStringEntry("Follow Query Tab", "true")));
  followResultsTab->setChecked(strtobool(cfg->readStringEntry("Follow Results Tab", "false")));
  maxResults->setValue(cfg->readNumberEntry("Max Results to Save", 10));
  ((QRadioButton *)multipleQueries->find(cfg->readNumberEntry("Multiple Query Options", 1)))->setChecked(true);
}

bool CQueryWindowOptionsTab::save(CConfig *cfg)
{
  bool ret = cfg->writeEntry("Query New Tab", booltostr(queryNewTab->isChecked()));
  ret &= cfg->writeEntry("Follow Query Tab", booltostr(followQueryTab->isChecked()));
  ret &= cfg->writeEntry("Follow Results Tab", booltostr(followResultsTab->isChecked()));
  ret &= cfg->writeEntry("Max Results to Save", maxResults->value());
  ret &= cfg->writeEntry("Multiple Query Options", multipleQueries->id(multipleQueries->selected()));
  return ret;
}

CQueryOptionsTab::CQueryOptionsTab(QWidget* parent, const char* name, Qt::WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
  if (!name)
    setName("CQueryOptionsTab");
  
  setCaption(tr("Query Configuration Dialog"));
  
  CQueryOptionsTabLayout = new Q3GridLayout(this, 1, 1, 4, 6, "CQueryOptionsTabLayout"); 
  
  Frame5 = new Q3Frame(this, "Frame5");
  Frame5->setFrameShape(Q3Frame::Box);
  Frame5->setFrameShadow(Q3Frame::Sunken);
  Frame5Layout = new Q3GridLayout(Frame5, 1, 1, 6, 4, "Frame5Layout"); 
  
  TextLabel5 = new QLabel(Frame5, "TextLabel5");
  
  Frame5Layout->addWidget(TextLabel5, 3, 0);
  
  autocommit = new QComboBox(false, Frame5, "autocommit");
  
  Frame5Layout->addWidget(autocommit, 1, 4);
  
  TextLabel4 = new QLabel(Frame5, "TextLabel4");
  
  Frame5Layout->addWidget(TextLabel4, 2, 3);
  
  TextLabel9 = new QLabel(Frame5, "TextLabel9");
  
  Frame5Layout->addWidget(TextLabel9, 5, 0);
  
  TextLabel11 = new QLabel(Frame5, "TextLabel11");
  
  Frame5Layout->addWidget(TextLabel11, 6, 0);
  
  sql_big_selects = new QComboBox(false, Frame5, "sql_big_selects");
  
  Frame5Layout->addWidget(sql_big_selects, 2, 4);
  
  low_priority_updates = new QComboBox(false, Frame5, "low_priority_updates");
  
  Frame5Layout->addWidget(low_priority_updates, 3, 4);
  QSpacerItem* spacer = new QSpacerItem(16, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
  Frame5Layout->addItem(spacer, 3, 2);
  
  big_tables = new QComboBox(false, Frame5, "big_tables");
  
  Frame5Layout->addWidget(big_tables, 2, 1);
  
  TextLabel6 = new QLabel(Frame5, "TextLabel6");
  
  Frame5Layout->addWidget(TextLabel6, 3, 3);
  
  sql_log_update = new QComboBox(false, Frame5, "sql_log_update");
  
  Frame5Layout->addWidget(sql_log_update, 5, 4);
  
  TextLabel10 = new QLabel(Frame5, "TextLabel10");
  
  Frame5Layout->addWidget(TextLabel10, 5, 3);
  
  TextLabel8 = new QLabel(Frame5, "TextLabel8");
  
  Frame5Layout->addWidget(TextLabel8, 4, 3);
  
  query_cache_type = new QComboBox(false, Frame5, "query_cache_type");
  
  Frame5Layout->addWidget(query_cache_type, 4, 1);
  
  TextLabel2 = new QLabel(Frame5, "TextLabel2");
  
  Frame5Layout->addWidget(TextLabel2, 1, 3);
  
  sql_log_off = new QComboBox(false, Frame5, "sql_log_off");
  
  Frame5Layout->addWidget(sql_log_off, 5, 1);
  
  sql_buffer_result = new QComboBox(false, Frame5, "sql_buffer_result");
  
  Frame5Layout->addWidget(sql_buffer_result, 3, 1);
  
  TextLabel3 = new QLabel(Frame5, "TextLabel3");
  
  Frame5Layout->addWidget(TextLabel3, 2, 0);
  
  TextLabel1 = new QLabel(Frame5, "TextLabel1");
  
  Frame5Layout->addWidget(TextLabel1, 1, 0);
  
  sql_safe_updates = new QComboBox(false, Frame5, "sql_safe_updates");
  
  Frame5Layout->addWidget(sql_safe_updates, 4, 4);
  
  sql_quote_show_create = new QComboBox(false, Frame5, "sql_quote_show_create");
  
  Frame5Layout->addWidget(sql_quote_show_create, 6, 1);
  
  sql_auto_is_null = new QComboBox(false, Frame5, "sql_auto_is_null");
  
  Frame5Layout->addWidget(sql_auto_is_null, 1, 1);
  
  TextLabel7 = new QLabel(Frame5, "TextLabel7");
  
  Frame5Layout->addWidget(TextLabel7, 4, 0);
  
  TextLabel12 = new QLabel(Frame5, "TextLabel12");
  TextLabel12->setAlignment(int(Qt::TextWordWrap | Qt::AlignVCenter | Qt::AlignLeft));
  
  Frame5Layout->addMultiCellWidget(TextLabel12, 0, 0, 0, 4);
  
  CQueryOptionsTabLayout->addWidget(Frame5, 1, 0);
  
  Layout2 = new Q3HBoxLayout(0, 0, 25, "Layout2"); 
  
  force = new QCheckBox(this, "force");
  force->setChecked(true);
  Layout2->addWidget(force);
  
  use_history_file = new QCheckBox(this, "use_history_file");
  use_history_file->setChecked(true);
  Layout2->addWidget(use_history_file);
  
  silent = new QCheckBox(this, "silent");
  silent->setChecked(false);
  Layout2->addWidget(silent);
  QSpacerItem* spacer_2 = new QSpacerItem(31, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  Layout2->addItem(spacer_2);
  
  CQueryOptionsTabLayout->addLayout(Layout2, 0, 0);
  QSpacerItem* spacer_3 = new QSpacerItem(16, 288, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CQueryOptionsTabLayout->addItem(spacer_3, 2, 0);
  languageChange();
  clearWState(WState_Polished);
  
  // tab order
  setTabOrder(force, use_history_file);
  setTabOrder(use_history_file, silent);
  setTabOrder(silent, sql_auto_is_null);
  setTabOrder(sql_auto_is_null, big_tables);
  setTabOrder(big_tables, sql_buffer_result);
  setTabOrder(sql_buffer_result, query_cache_type);
  setTabOrder(query_cache_type, sql_log_off);
  setTabOrder(sql_log_off, sql_quote_show_create);
  setTabOrder(sql_quote_show_create, autocommit);
  setTabOrder(autocommit, sql_big_selects);
  setTabOrder(sql_big_selects, low_priority_updates);
  setTabOrder(low_priority_updates, sql_safe_updates);
  setTabOrder(sql_safe_updates, sql_log_update);
}

void CQueryOptionsTab::languageChange()
{
  setCaption(tr("Query Options"));
  TextLabel5->setText(tr("SQL_BUFFER_RESULT"));
  autocommit->clear();
  autocommit->insertItem(tr("[DEFAULT]"));
  autocommit->insertItem(tr("0"));
  autocommit->insertItem(tr("1"));
  Q3WhatsThis::add(autocommit, tr("If set to 1 all changes to a table will be done at once. To start a multi-command transaction, you have to use the BEGIN statement."));
  TextLabel4->setText(tr("SQL_BIG_SELECTS"));
  TextLabel9->setText(tr("SQL_LOG_OFF"));
  TextLabel11->setText(tr("SQL_QUOTE_SHOW_CREATE"));
  sql_big_selects->clear();
  sql_big_selects->insertItem(tr("[DEFAULT]"));
  sql_big_selects->insertItem(tr("0"));
  sql_big_selects->insertItem(tr("1"));
  Q3WhatsThis::add(sql_big_selects, tr("If set to 0, MySQL will abort if a SELECT is attempted that probably will take a very long time. This is useful when an inadvisable WHERE statement has been issued."));
  low_priority_updates->clear();
  low_priority_updates->insertItem(tr("[DEFAULT]"));
  low_priority_updates->insertItem(tr("0"));
  low_priority_updates->insertItem(tr("1"));
  Q3WhatsThis::add(low_priority_updates, tr("If set to 1, all INSERT, UPDATE, DELETE, and and LOCK TABLE WRITE statements wait until there is no pending SELECT or LOCK TABLE READ on the affected table."));
  big_tables->clear();
  big_tables->insertItem(tr("[DEFAULT]"));
  big_tables->insertItem(tr("0"));
  big_tables->insertItem(tr("1"));
  Q3WhatsThis::add(big_tables, tr("If set to 1, all temporary tables are stored on disk rather than in memory."));
  TextLabel6->setText(tr("LOW_PRIORITY_UPDATES"));
  sql_log_update->clear();
  sql_log_update->insertItem(tr("[DEFAULT]"));
  sql_log_update->insertItem(tr("0"));
  sql_log_update->insertItem(tr("1"));
  Q3WhatsThis::add(sql_log_update, tr("If set to 0, no logging will be done to the update log for the client, if the client has the SUPER privilege. This does not affect the standard log!"));
  TextLabel10->setText(tr("SQL_LOG_UPDATE"));
  TextLabel8->setText(tr("SQL_SAFE_UPDATES"));
  query_cache_type->clear();
  query_cache_type->insertItem(tr("[DEFAULT]"));
  query_cache_type->insertItem(tr("0"));
  query_cache_type->insertItem(tr("1"));
  query_cache_type->insertItem(tr("2"));
  Q3WhatsThis::add(query_cache_type, tr("Set query cache setting for this thread."));
  TextLabel2->setText(tr("AUTOCOMMIT"));
  sql_log_off->clear();
  sql_log_off->insertItem(tr("[DEFAULT]"));
  sql_log_off->insertItem(tr("0"));
  sql_log_off->insertItem(tr("1"));
  Q3WhatsThis::add(sql_log_off, tr("If set to 1, no logging will be done to the standard log for this client, if the client has the SUPER privilege. This does not affect the update log!"));
  sql_buffer_result->clear();
  sql_buffer_result->insertItem(tr("[DEFAULT]"));
  sql_buffer_result->insertItem(tr("0"));
  sql_buffer_result->insertItem(tr("1"));
  Q3WhatsThis::add(sql_buffer_result, tr("SQL_BUFFER_RESULT will force the result from SELECTs to be put into a temporary table. This will help MySQL free the table locks early and will help in cases where it takes a long time to send the result set to the client."));
  TextLabel3->setText(tr("BIG_TABLES"));
  TextLabel1->setText(tr("SQL_AUTO_IS_NULL"));
  sql_safe_updates->clear();
  sql_safe_updates->insertItem(tr("[DEFAULT]"));
  sql_safe_updates->insertItem(tr("0"));
  sql_safe_updates->insertItem(tr("1"));
  Q3WhatsThis::add(sql_safe_updates, tr("If set to 1, MySQL will abort if an UPDATE or DELETE is attempted that doesn't use a key or LIMIT in the WHERE clause. This makes it possible to catch wrong updates when creating SQL commands by hand."));
  sql_quote_show_create->clear();
  sql_quote_show_create->insertItem(tr("[DEFAULT]"));
  sql_quote_show_create->insertItem(tr("0"));
  sql_quote_show_create->insertItem(tr("1"));
  Q3WhatsThis::add(sql_quote_show_create, tr("If set to 1, SHOW CREATE TABLE will quote table and column names."));
  sql_auto_is_null->clear();
  sql_auto_is_null->insertItem(tr("[DEFAULT]"));
  sql_auto_is_null->insertItem(tr("0"));
  sql_auto_is_null->insertItem(tr("1"));
  Q3WhatsThis::add(sql_auto_is_null, tr("If set to 1 then one can find the last inserted row for a table with an AUTO_INCREMENT column with the following construct: WHERE auto_increment_column IS NULL."));
  TextLabel7->setText(tr("QUERY_CACHE_TYPE"));
  TextLabel12->setText(tr("<B>NOTE:</B> The below options apply to <b>this</b> session only;  the server's configuration will not be affected by these settings.  All options set to '[DEFAULT]' will remain untouched."));
  force->setText(tr("Force"));
  Q3WhatsThis::add(force, tr("Continue even if we get an sql error"));
  use_history_file->setText(tr("Append to History View"));
  Q3WhatsThis::add(use_history_file, tr("When checked, each executed query will be appended to the History Panel."));
  silent->setText(tr("Silent"));
  Q3WhatsThis::add(silent, tr("When checked, no messages will be printed in the Messages Panel with the exception of errors."));
}


void CQueryOptionsTab::setDefaultValues(CConfig *cfg)
{
#ifdef DEBUG
  qDebug("CQueryOptionsTab::setDefaultValues()");
#endif

  force->setChecked(strtobool(cfg->readStringEntry("Force Queries", "true")));
  silent->setChecked(strtobool(cfg->readStringEntry("Silent", "false")));
  use_history_file->setChecked(strtobool(cfg->readStringEntry("Append Queries to History", "true")));
  sql_auto_is_null->setCurrentItem(cfg->readNumberEntry("sql_auto_is_null"));
  big_tables->setCurrentItem(cfg->readNumberEntry("big_tables"));
  sql_buffer_result->setCurrentItem(cfg->readNumberEntry("sql_buffer_result"));
  query_cache_type->setCurrentItem(cfg->readNumberEntry("query_cache_type"));
  sql_log_off->setCurrentItem(cfg->readNumberEntry("sql_log_off"));
  sql_quote_show_create->setCurrentItem(cfg->readNumberEntry("sql_quote_show_create"));
  autocommit->setCurrentItem(cfg->readNumberEntry("autocommit"));
  sql_big_selects->setCurrentItem(cfg->readNumberEntry("sql_big_selects"));
  low_priority_updates->setCurrentItem(cfg->readNumberEntry("low_priority_updates"));
  sql_safe_updates->setCurrentItem(cfg->readNumberEntry("sql_safe_updates"));
  sql_log_update->setCurrentItem(cfg->readNumberEntry("sql_log_update"));
}

bool CQueryOptionsTab::save(CConfig *cfg)
{
#ifdef DEBUG
  qDebug("CQueryOptionsTab::save()");
#endif

  bool ret = cfg->writeEntry("Force Queries", booltostr(force->isChecked()));
  ret &= cfg->writeEntry("Silent", booltostr(silent->isChecked()));
  ret &= cfg->writeEntry("Append Queries to History", booltostr(use_history_file->isChecked()));
  ret &= cfg->writeEntry("sql_auto_is_null", sql_auto_is_null->currentItem());
  ret &= cfg->writeEntry("big_tables", big_tables->currentItem());
  ret &= cfg->writeEntry("sql_buffer_result", sql_buffer_result->currentItem());
  ret &= cfg->writeEntry("query_cache_type", query_cache_type->currentItem());
  ret &= cfg->writeEntry("sql_log_off", sql_log_off->currentItem());
  ret &= cfg->writeEntry("sql_quote_show_create", sql_quote_show_create->currentItem());
  ret &= cfg->writeEntry("autocommit", autocommit->currentItem());
  ret &= cfg->writeEntry("sql_big_selects", sql_big_selects->currentItem());
  ret &= cfg->writeEntry("low_priority_updates", low_priority_updates->currentItem());
  ret &= cfg->writeEntry("sql_safe_updates", sql_safe_updates->currentItem());
  ret &= cfg->writeEntry("sql_log_update", sql_log_update->currentItem());

  return ret;
}

CQueryWindowOptionsDialog::CQueryWindowOptionsDialog(QWidget* parent, CMySQLServer *m, const char* name)
:CConfigDialog(parent, name), mysql(m)
{
#ifdef DEBUG
  qDebug("CQueryWindowOptionsDialog::CQueryWindowOptionsDialog()");
#endif

  if (!name)
    setName("CQueryWindowOptionsDialog");    
  setMinimumHeight(308);  
  setCaption(tr("Query Configuration Dialog"));  
  insertTab(new CQueryWindowOptionsTab((QWidget *) tab()));
  insertTab(new CQueryOptionsTab((QWidget *) tab()));
  okPushButton->setText(tr("&Apply"));
  Q3WhatsThis::add(okPushButton, tr("Click to Apply changes."));
  myResize(460, 308);
  config = new CConfig(m->connectionName(), m->connectionsPath());
  setDefaultValues(config);
}

CQueryWindowOptionsDialog::~CQueryWindowOptionsDialog()
{
  delete config;
}

void CQueryWindowOptionsDialog::okButtonClicked()
{
#ifdef DEBUG
  qDebug("CQueryWindowOptionsDialog::okButtonClicked()");
#endif

  bool ret = save(config);
  ret &= config->save();
  if (!ret)
  {
    mysql->showMessage(CRITICAL, tr("An error occurred while saving the Query configuration"));
    return;
  }  
  emit config_changed();
  mysql->showMessage(INFORMATION, tr("Query configuration updated successfully"));
  dialogAccepted();
}

void CQueryWindowOptionsDialog::add_option(query_config_options_map *opt, CConfig *cfg, const QString &s)
{
#ifdef DEBUG
  qDebug("static CQueryWindowOptionsDialog::add_option(QStringList *, CConfig *, const QString &)");
#endif

  int v = cfg->readNumberEntry(s);
  if (v > 0)
    opt->insert(s, QString::number(v - 1));
}


CQueryWindowOptionsDialog::query_config_options CQueryWindowOptionsDialog::readConfig(CConfig *cfg)
{
#ifdef DEBUG
  qDebug("static query_config_options::readConfig(CMySQLServer *)");
#endif

  query_config_options opt;
  
  opt.force = strtobool(cfg->readStringEntry("Force Queries", "true"));
  opt.silent = !strtobool(cfg->readStringEntry("Silent", "false"));
  opt.use_history_file = strtobool(cfg->readStringEntry("Append Queries to History", "true"));
  add_option(&opt.options, cfg, "sql_auto_is_null");
  add_option(&opt.options, cfg, "big_tables");
  add_option(&opt.options, cfg, "sql_buffer_result");
  add_option(&opt.options, cfg, "query_cache_type");
  add_option(&opt.options, cfg, "sql_log_off");
  add_option(&opt.options, cfg, "sql_quote_show_create");
  add_option(&opt.options, cfg, "autocommit");
  add_option(&opt.options, cfg, "sql_big_selects");
  add_option(&opt.options, cfg, "low_priority_updates");
  add_option(&opt.options, cfg, "sql_safe_updates");
  add_option(&opt.options, cfg, "sql_log_update");

  return opt;
}
