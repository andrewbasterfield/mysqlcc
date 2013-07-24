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
#ifndef CCONNECTIONDIALOG_H
#define CCONNECTIONDIALOG_H

#include <stddef.h>
#include <qvariant.h>
#include <qwidget.h>
#include <q3groupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qfile.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include "CConfigDialog.h"

class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class QCheckBox;
class Q3GroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class CConfig;
class CMessagePanel;
class Q3ListBox;
class Q3ListBoxItem;
class QPixmap;


class CMySQLOptionsTab : public CConfigDialogTab
{
  Q_OBJECT

public:
  CMySQLOptionsTab(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
  void setDefaultValues(CConfig *conn=0);
  bool save(CConfig *conn);
  
protected slots:
  virtual void languageChange();
  
private:
  QLabel* textLabel1;
  QLabel* textLabel2;
  QLabel* textLabel3;
  QLabel* textLabel4;
  QLabel* textLabel5;
  QSpinBox* connectTimeout;
  QSpinBox* selectLimit;
  QSpinBox* netBufferLength;
  QSpinBox* maxJoinSize;
  QSpinBox* maxAllowedPacket;
  QCheckBox* localInFile;
  Q3GridLayout* CMySQLOptionsTabLayout;
};

class CSaveServerOptionsTab : public CConfigDialogTab
{ 
  Q_OBJECT
      
public:
  CSaveServerOptionsTab (QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
  void setDefaultValues(CConfig *conn=0);
  bool save(CConfig *conn);  
  
private:
  QLineEdit* Separated;
  QLineEdit* Terminated;
  QLabel* separateLable;
  QLabel* enclosedLabel;
  QLabel* terminatedLabel;
  QLabel* replaceEmptyLabel;
  QLineEdit* ReplaceEmpty;
  QLineEdit* Enclosed;
  QLabel* TextLabel4;
  Q3VBoxLayout* CSaveServerOptionsTabLayout;
  Q3GridLayout* Layout20;  
};

class CDatabaseServerOptionsTab : public CConfigDialogTab
{
  Q_OBJECT

public:
  CDatabaseServerOptionsTab (QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
  void setDefaultValues(CConfig *conn=0);
  bool save(CConfig *conn);
  Q3ListBox* databases;

private slots:
  void radioButton2Toggled(bool);
  void AddDatabase();
  void DeleteDatabase();

private:
  QCheckBox* showAllDatabasesCheckBox;
  Q3ButtonGroup* databaseRetrievalGroup;
  QPushButton* addDatabase;
  QPushButton* deleteDatabase;
  QRadioButton* radioButton2;
  QRadioButton* radioButton1;  
  Q3GridLayout* CDatabaseServerOptionsTabLayout;
  Q3GridLayout* databaseRetrievalGroupLayout;
  QPixmap databaseIcon;
};

class CGeneralServerOptionsTab : public CConfigDialogTab
{ 
  Q_OBJECT
    
public:
  CGeneralServerOptionsTab (bool isediting, QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
  void setDefaultValues(CConfig *Settings);
  bool save(CConfig *conn);  
  QString validate() const;
  
  QLineEdit* HostNameBox;
  Q3ButtonGroup* tableRetrievalGroup;
  QRadioButton* retrieveShowTableStatus;
  QRadioButton* retrieveShowTables;
  QLineEdit* PasswordBox;
  QLineEdit* UserNameBox;
  QSpinBox* PortBox;
  QLineEdit* ConnectionNameBox;
  Q3GroupBox* Options;
  QLineEdit* socketFile;
  QPushButton* socketBrowse;
  QCheckBox* oneConnectionCheckBox;
  QCheckBox* compressCheckBox;
  QCheckBox* reconnectCheckBox;
  QCheckBox* blockingCheckBox;
  QCheckBox* enableCompletionCheckBox;
  QCheckBox* SSLCheckBox;
  QCheckBox* promptPasswordCheckBox;
  QCheckBox* defaultServer;
  QCheckBox* disableStartupConnect;

protected slots:
  virtual void languageChange();
  
private slots:
  void setSocketFile();
  
private:
  void init();
  bool isEditing;
  QLabel* socketLabel;
  QLabel* userNameLabel;
  Q3GridLayout* CGeneralServerOptionsTabLayout;
  Q3GridLayout* tableRetrievalGroupLayout;
  Q3GridLayout* OptionsLayout;
  QLabel* portLabel;
  QLabel* connectionLabel;
  QLabel* hostLabel;
  QLabel* passwordLabel;
};

class CRegisterServerDialog : public CConfigDialog
{ 
  Q_OBJECT
    
public:
  CRegisterServerDialog(CMessagePanel * messagepanel, QWidget* parent);
  CRegisterServerDialog(const QString &cname, CMessagePanel * messagepanel, QWidget* parent);
  ~CRegisterServerDialog();
  
signals:
  void connectionEdited(const QString &);
  void newConnection();

private slots:
  void testConnection();

private:
  void initConnectionDialog(CMessagePanel * messagepanel);
  void okButtonClicked();
  bool validate();
  bool save(CConfig *conn);
  bool is_default_connection_name;
  CGeneralServerOptionsTab * GeneralServerOptionsTab;
  CMessagePanel * messagePanel;
  bool isEditing;
  QString cfgname;
};

#endif
