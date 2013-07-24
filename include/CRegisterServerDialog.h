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
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qfile.h>
#include "CConfigDialog.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class CConfig;
class CMessagePanel;
class QListBox;
class QListBoxItem;
class QPixmap;


class CMySQLOptionsTab : public CConfigDialogTab
{
  Q_OBJECT

public:
  CMySQLOptionsTab(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
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
  QGridLayout* CMySQLOptionsTabLayout;
};

class CSaveServerOptionsTab : public CConfigDialogTab
{ 
  Q_OBJECT
      
public:
  CSaveServerOptionsTab (QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
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
  QVBoxLayout* CSaveServerOptionsTabLayout;
  QGridLayout* Layout20;  
};

class CDatabaseServerOptionsTab : public CConfigDialogTab
{
  Q_OBJECT

public:
  CDatabaseServerOptionsTab (QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  void setDefaultValues(CConfig *conn=0);
  bool save(CConfig *conn);
  QListBox* databases;

private slots:
  void radioButton2Toggled(bool);
  void AddDatabase();
  void DeleteDatabase();

private:
  QCheckBox* showAllDatabasesCheckBox;
  QButtonGroup* databaseRetrievalGroup;
  QPushButton* addDatabase;
  QPushButton* deleteDatabase;
  QRadioButton* radioButton2;
  QRadioButton* radioButton1;  
  QGridLayout* CDatabaseServerOptionsTabLayout;
  QGridLayout* databaseRetrievalGroupLayout;
  QPixmap databaseIcon;
};

class CGeneralServerOptionsTab : public CConfigDialogTab
{ 
  Q_OBJECT
    
public:
  CGeneralServerOptionsTab (bool isediting, QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  void setDefaultValues(CConfig *Settings);
  bool save(CConfig *conn);  
  QString validate() const;
  
  QLineEdit* HostNameBox;
  QButtonGroup* tableRetrievalGroup;
  QRadioButton* retrieveShowTableStatus;
  QRadioButton* retrieveShowTables;
  QLineEdit* PasswordBox;
  QLineEdit* UserNameBox;
  QSpinBox* PortBox;
  QLineEdit* ConnectionNameBox;
  QGroupBox* Options;
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
  QGridLayout* CGeneralServerOptionsTabLayout;
  QGridLayout* tableRetrievalGroupLayout;
  QGridLayout* OptionsLayout;
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
