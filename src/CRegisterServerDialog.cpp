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
#include "CRegisterServerDialog.h"
#include "globals.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "CConsoleWindow.h"
#include "CDatabaseListView.h"
#include "CConfig.h"
#include "config.h"
#include "panels.h"
#include <stddef.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#ifndef WIN32
#include <unistd.h>
#endif

static const QString connections_path = QString(CONNECTIONS_PATH);


CMySQLOptionsTab::CMySQLOptionsTab(QWidget* parent, const char* name, WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
  
  if (!name)
    setName("CMySQLOptionsTab");
  CMySQLOptionsTabLayout = new QGridLayout(this, 1, 1, 4, 2, "CMySQLOptionsTabLayout"); 
  
  textLabel1 = new QLabel(this, "textLabel1");
  
  CMySQLOptionsTabLayout->addWidget(textLabel1, 0, 0);
  
  textLabel2 = new QLabel(this, "textLabel2");
  
  CMySQLOptionsTabLayout->addWidget(textLabel2, 1, 0);
  
  textLabel3 = new QLabel(this, "textLabel3");
  
  CMySQLOptionsTabLayout->addWidget(textLabel3, 2, 0);
  
  textLabel4 = new QLabel(this, "textLabel4");
  
  CMySQLOptionsTabLayout->addWidget(textLabel4, 3, 0);
  
  textLabel5 = new QLabel(this, "textLabel5");
  
  CMySQLOptionsTabLayout->addWidget(textLabel5, 4, 0);
  
  connectTimeout = new QSpinBox(this, "connectTimeout");
  connectTimeout->setMaxValue(500);
  connectTimeout->setValue(5);
  
  CMySQLOptionsTabLayout->addWidget(connectTimeout, 0, 1);
  
  selectLimit = new QSpinBox(this, "selectLimit");
  selectLimit->setMaxValue(0xFFFFFF);
  selectLimit->setMinValue(0);
  selectLimit->setSpecialValueText(tr("Unlimited"));
  selectLimit->setValue(0);
  
  CMySQLOptionsTabLayout->addMultiCellWidget(selectLimit, 1, 1, 1, 2);
  
  netBufferLength = new QSpinBox(this, "netBufferLength");
  netBufferLength->setMaxValue(512*1024*1024L);
  netBufferLength->setMinValue(1024);
  netBufferLength->setValue(16384);
  
  CMySQLOptionsTabLayout->addMultiCellWidget(netBufferLength, 2, 2, 1, 2);
  QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CMySQLOptionsTabLayout->addMultiCell(spacer, 0, 0, 2, 3);
  QSpacerItem* spacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CMySQLOptionsTabLayout->addItem(spacer_2, 1, 3);
  
  maxJoinSize = new QSpinBox(this, "maxJoinSize");
  maxJoinSize->setMaxValue(0xFFFFFF);
  maxJoinSize->setMinValue(0);
  maxJoinSize->setSpecialValueText(tr("Unlimited"));
  maxJoinSize->setValue(0);  
  
  CMySQLOptionsTabLayout->addMultiCellWidget(maxJoinSize, 4, 4, 1, 2);
  
  maxAllowedPacket = new QSpinBox(this, "maxAllowedPacket");
  maxAllowedPacket->setMaxValue(512*1024L*1024L);
  maxAllowedPacket->setMinValue(4096);
  maxAllowedPacket->setValue(16*1024L*1024L);
  
  CMySQLOptionsTabLayout->addMultiCellWidget(maxAllowedPacket, 3, 3, 1, 2);
  
  localInFile = new QCheckBox(this, "localInFile");
  
  CMySQLOptionsTabLayout->addMultiCellWidget(localInFile, 5, 5, 0, 3);
  QSpacerItem* spacer_3 = new QSpacerItem(20, 130, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CMySQLOptionsTabLayout->addItem(spacer_3, 6, 1);
  languageChange();
  clearWState(WState_Polished);
  
  // tab order
  setTabOrder(connectTimeout, selectLimit);
  setTabOrder(selectLimit, netBufferLength);
  setTabOrder(netBufferLength, maxAllowedPacket);
  setTabOrder(maxAllowedPacket, maxJoinSize);
  setTabOrder(maxJoinSize, localInFile);
}


void CMySQLOptionsTab::languageChange()
{
  setCaption(tr("MySQL Options"));
  textLabel1->setText(tr("Connect Timeout (sec)"));
  textLabel2->setText(tr("Automatically limit SELECT queries to"));
  textLabel3->setText(tr("Net Buffer Length"));
  textLabel4->setText(tr("Maximum Allowed Packet"));
  textLabel5->setText(tr("Maximum Join Size"));
  localInFile->setText(tr("Allow the use of LOAD DATA LOCAL INFILE"));
  QWhatsThis::add(connectTimeout, tr("Specifies the amount of time the client should wait to receive a response from the server."));
  QWhatsThis::add(selectLimit, tr("This option will automatically limit the number of results returned by a SELECT query if the LIMIT clause wasn't specified."));
  QWhatsThis::add(netBufferLength, tr("Buffer length for TCP/IP and socket communication between the client and the server."));
  QWhatsThis::add(maxJoinSize, tr("Automatic limit rows in a join."));
  QWhatsThis::add(maxAllowedPacket, tr("Maximum packet-length to send and receive from to server."));
  QWhatsThis::add(maxAllowedPacket, tr("Maximum packet-length to send and receive from to server."));
  QWhatsThis::add(localInFile, tr("When enabled, the use of LOAD DATA LOCAL INFILE will be available."));
}

bool CMySQLOptionsTab::save(CConfig *conn)
{ 
  bool ret = conn->writeEntry("Timeout", connectTimeout->value());
  ret &= conn->writeEntry("Limit Select Queries Number", selectLimit->value());
  ret &= conn->writeEntry("Net Buffer Length", netBufferLength->value());
  ret &= conn->writeEntry("Max Join Size", maxJoinSize->value());
  ret &= conn->writeEntry("Max Allowed Packet", maxAllowedPacket->value());
  ret &= conn->writeEntry("Allow Local Infile", booltostr(localInFile->isChecked()));
  return ret;
}

void CMySQLOptionsTab::setDefaultValues(CConfig *Settings)
{
  connectTimeout->setValue(Settings->readNumberEntry("Timeout", 0));
  selectLimit->setValue(Settings->readNumberEntry("Limit Select Queries Number", 0));
  netBufferLength->setValue(Settings->readNumberEntry("Net Buffer Length", 16384));
  maxJoinSize->setValue(Settings->readNumberEntry("Max Join Size", 0));
  maxAllowedPacket->setValue(Settings->readNumberEntry("Max Allowed Packet", 16*1024L*1024L));
  localInFile->setChecked(strtobool(Settings->readStringEntry("Allow Local Infile")));
}

CSaveServerOptionsTab::CSaveServerOptionsTab(QWidget * parent, const char *name, WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
#ifdef DEBUG
  qDebug("CSaveServerOptionsTab::CSaveServerOptionsTab()");
#endif
  
  if (!name)
    setName("CSaveServerOptionsTab");
  
  setCaption(tr("Save Options"));
  CSaveServerOptionsTabLayout = new QVBoxLayout(this, 4, 2, "CSaveServerOptionsTabLayout");   
  
  Layout20 = new QGridLayout(0, 1, 1, 0, 2, "Layout20"); 
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);  
  Layout20->addItem(spacer, 2, 2);
  QSpacerItem* spacer_2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);  
  Layout20->addItem(spacer_2, 0, 2);
  
  Separated = new QLineEdit(this, "Separated");  
  Separated->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)4, (QSizePolicy::SizeType)0, 0, 0, Separated->sizePolicy().hasHeightForWidth()));
  QWhatsThis::add(Separated,tr("The specified character will be used to Separate fields whenever you save a text file."));
  
  Layout20->addWidget(Separated, 0, 1);  
  QSpacerItem* spacer_3 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  Layout20->addItem(spacer_3, 1, 2);
  
  Terminated = new QLineEdit(this, "Terminated");  
  Terminated->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, Terminated->sizePolicy().hasHeightForWidth()));
  QWhatsThis::add(Terminated,tr("The specified character will be appended to all the lines whenever you save a text file."));
  
  Layout20->addWidget(Terminated, 2, 1);
  
  separateLable = new QLabel(this, "separateLable");  
  separateLable->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, separateLable->sizePolicy().hasHeightForWidth()));
  separateLable->setText(tr("Fields Separated by"));
  
  Layout20->addWidget(separateLable, 0, 0);
  
  enclosedLabel = new QLabel(this, "enclosedLabel");  
  enclosedLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, enclosedLabel->sizePolicy().hasHeightForWidth()));
  enclosedLabel->setText(tr("Fields enclosed by"));
  
  Layout20->addWidget(enclosedLabel, 1, 0);
  
  terminatedLabel = new QLabel(this, "terminatedLabel");
  terminatedLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, terminatedLabel->sizePolicy().hasHeightForWidth()));
  terminatedLabel->setText(tr("Lines terminated by"));
  
  Layout20->addWidget(terminatedLabel, 2, 0);
  
  Enclosed = new QLineEdit(this, "Enclosed");
  Enclosed->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, Enclosed->sizePolicy().hasHeightForWidth()));
  QWhatsThis::add(Enclosed,tr("The specified character will be used to Enclose each field whenever you save a text file."));
  
  Layout20->addWidget(Enclosed, 1, 1);
  CSaveServerOptionsTabLayout->addLayout(Layout20);
  
  
  replaceEmptyLabel = new QLabel(this, "replaceEmptyLabel");
  replaceEmptyLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, replaceEmptyLabel->sizePolicy().hasHeightForWidth()));
  replaceEmptyLabel->setText(tr("Replace emtpy fields with"));
  
  
  Layout20->addWidget(replaceEmptyLabel, 3, 0);
  
  ReplaceEmpty = new QLineEdit(this, "ReplaceEmpty");
  ReplaceEmpty->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ReplaceEmpty->sizePolicy().hasHeightForWidth()));
  QWhatsThis::add(ReplaceEmpty,tr("All Empty fields will be replaced with what you type in this box for whenever you save a text file."));
  Layout20->addWidget(ReplaceEmpty, 3, 1);
  
  TextLabel4 = new QLabel(this, "TextLabel4");  
  TextLabel4->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, TextLabel4->sizePolicy().hasHeightForWidth()));
  TextLabel4->setMargin(3);
  TextLabel4->setText(tr("You can use the following escaped symbols to represent certain escaped characters:<br>\n"
    "<b>\\n</b>    New Line<br>\n"
    "<b>\\r</b>     Carriage Return<br>\n"
    "<b>\\t</b>     Tab"));
  TextLabel4->setAlignment(int(QLabel::WordBreak | QLabel::AlignTop));
  CSaveServerOptionsTabLayout->addWidget(TextLabel4);
  QSpacerItem* spacer_4 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CSaveServerOptionsTabLayout->addItem(spacer_4);
  
  Separated->setText(",");
  Terminated->setText("\\r\\n");
  Enclosed->setText("'");  
  
  setTabOrder(Separated, Enclosed);
  setTabOrder(Enclosed, Terminated);
  setTabOrder(Terminated, ReplaceEmpty);
}

bool CSaveServerOptionsTab::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CSaveServerOptionsTab::save()");
#endif
  
  bool ret = conn->writeEntry("Field Separate", Separated->text().stripWhiteSpace());  
  ret &= conn->writeEntry("Line Terminate", Terminated->text().stripWhiteSpace());
  ret &= conn->writeEntry("Field Enclosed", Enclosed->text().stripWhiteSpace());  
  ret &= conn->writeEntry("Replace Empty", ReplaceEmpty->text().stripWhiteSpace());
  return ret;
}

void CSaveServerOptionsTab::setDefaultValues(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CSaveServerOptionsTab::setDefaultValues()");
#endif
  
  Separated->setText(conn->readStringEntry("Field Separate", ","));
  Terminated->setText(conn->readStringEntry("Line Terminate", "\\r\\n"));
  Enclosed->setText(conn->readStringEntry("Field Enclosed", "'"));
  ReplaceEmpty->setText(conn->readStringEntry("Replace Emtpy", QString::null));
}


CDatabaseServerOptionsTab::CDatabaseServerOptionsTab(QWidget * parent, const char *name, WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
#ifdef DEBUG
  qDebug("CDatabaseServerOptionsTab::CDatabaseServerOptionsTab()");
#endif
  
  if (!name)
    setName("CDatabaseServerOptionsTab");
  setCaption(tr("Databases"));
  databaseIcon = getPixmapIcon("databaseDisconnectedIcon"); 
  CDatabaseServerOptionsTabLayout = new QGridLayout(this, 1, 1, 2, 4, "CDatabaseServerOptionsTabLayout"); 
  
  showAllDatabasesCheckBox = new QCheckBox(this, "showAllDatabasesCheckBox");
  showAllDatabasesCheckBox->setChecked(true);
  showAllDatabasesCheckBox->setText(tr("Show all Databases"));
  QWhatsThis::add(showAllDatabasesCheckBox, tr("When enabled, all the databases in the server will be displayed.  If unchecked, only the databases to which you have access to will be shown.\\n\\nNOTE:  This option calls mysql_select_db() for each database to check access.  If unchecked, it will take longer to connect to a server."));
  
  CDatabaseServerOptionsTabLayout->addWidget(showAllDatabasesCheckBox, 0, 0);
  
  databaseRetrievalGroup = new QButtonGroup(this, "databaseRetrievalGroup");
  databaseRetrievalGroup->setColumnLayout(0, Qt::Vertical);
  databaseRetrievalGroup->layout()->setSpacing(2);
  databaseRetrievalGroup->layout()->setMargin(4);
  databaseRetrievalGroupLayout = new QGridLayout(databaseRetrievalGroup->layout());
  databaseRetrievalGroupLayout->setAlignment(Qt::AlignTop);
  databaseRetrievalGroup->setTitle(tr("Database Retrieval Method"));  
  
  addDatabase = new QPushButton(databaseRetrievalGroup, "addDatabase");
  addDatabase->setMinimumSize(QSize(20, 20));
  addDatabase->setMaximumSize(QSize(20, 20));
  addDatabase->setPixmap(getPixmapIcon("plusIcon"));
  addDatabase->setAutoDefault(false);
  addDatabase->setText(QString::null);
  connect(addDatabase, SIGNAL(clicked()), this, SLOT(AddDatabase()));
  QWhatsThis::add(addDatabase, tr("Add new Database"));
  
  databaseRetrievalGroupLayout->addWidget(addDatabase, 2, 1);
  
  deleteDatabase = new QPushButton(databaseRetrievalGroup, "deleteDatabase");
  deleteDatabase->setMinimumSize(QSize(20, 20));
  deleteDatabase->setMaximumSize(QSize(20, 20));
  deleteDatabase->setPixmap(getPixmapIcon("minusIcon"));
  deleteDatabase->setAutoDefault(false);
  deleteDatabase->setText(QString::null);
  connect(deleteDatabase, SIGNAL(clicked()), this, SLOT(DeleteDatabase()));
  QWhatsThis::add(deleteDatabase, tr("Delete selected Database"));
  
  databaseRetrievalGroupLayout->addWidget(deleteDatabase, 3, 1);
  QSpacerItem* spacer = new QSpacerItem(297, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  databaseRetrievalGroupLayout->addItem(spacer, 2, 2);
  
  radioButton2 = new QRadioButton(databaseRetrievalGroup, "radioButton2");
  radioButton2->setText(tr("Use the following Databases:"));
  QWhatsThis::add(radioButton2, tr("Only the following Databases will be displayed"));
  connect(radioButton2, SIGNAL(toggled(bool)), this, SLOT(radioButton2Toggled(bool)));
  
  databaseRetrievalGroupLayout->addMultiCellWidget(radioButton2, 1, 1, 0, 2);
  
  radioButton1 = new QRadioButton(databaseRetrievalGroup, "radioButton1");
  radioButton1->setText(tr("Use SHOW DATABASES"));
  QWhatsThis::add(radioButton1, tr("The SHOW DATABASES command will be used to retrieve all the Databases in the server."));
  
  databaseRetrievalGroupLayout->addMultiCellWidget(radioButton1, 0, 0, 0, 2);
  
  databases = new QListBox(databaseRetrievalGroup, "databases");
  databases->setSelectionMode(QListBox::Extended);
  databases->clear();  
  QWhatsThis::add(databases, tr("Databases that will be shown in the Database Tree"));
  
  databaseRetrievalGroupLayout->addMultiCellWidget(databases, 2, 4, 0, 0);
  QSpacerItem* spacer_2 = new QSpacerItem(20, 445, QSizePolicy::Minimum, QSizePolicy::Expanding);
  databaseRetrievalGroupLayout->addItem(spacer_2, 4, 1);
  
  CDatabaseServerOptionsTabLayout->addWidget(databaseRetrievalGroup, 1, 0);
  
  // tab order
  setTabOrder(showAllDatabasesCheckBox, radioButton1);
  setTabOrder(radioButton1, radioButton2);
  setTabOrder(radioButton2, addDatabase);
  setTabOrder(addDatabase, deleteDatabase);
  setTabOrder(deleteDatabase, databases);
  radioButton1->setChecked(true);
  radioButton2Toggled(false);
}

void CDatabaseServerOptionsTab::radioButton2Toggled(bool b)
{
#ifdef DEBUG
  qDebug("CDatabaseServerOptionsTab::radioButton2Toggled()");
#endif

  databases->setEnabled(b);
  addDatabase->setEnabled(b);
  deleteDatabase->setEnabled(b);
}

void CDatabaseServerOptionsTab::DeleteDatabase()
{
#ifdef DEBUG
  qDebug("CDatabaseServerOptionsTab::DeleteDatabase()");
#endif

  for (uint i = 0; i < databases->count(); i++)
    if (databases->isSelected(i))
      databases->removeItem(i--);      
  if (databases->count() <= 0)
    deleteDatabase->setEnabled(false);
}

void CDatabaseServerOptionsTab::AddDatabase()
{
#ifdef DEBUG
  qDebug("CDatabaseServerOptionsTab::AddDatabase()");
#endif

  bool ok = false;
  QString db = QInputDialog::getText(tr("Database"), tr("Please enter the Database Name"), QLineEdit::Normal, QString::null, &ok);
  if (ok && !db.isEmpty())
  {
    if (databases->findItem(db, Qt::ExactMatch
#ifndef WIN32
      | Qt::CaseSensitive  //Windows is case sensitive for databases.
#endif
     ) == 0)
    {
        databases->insertItem(databaseIcon, db);
        deleteDatabase->setEnabled(true);
    }
    else
      QMessageBox::warning(0,tr("Warning"), tr("The database is already in the list."));
  }
}

bool CDatabaseServerOptionsTab::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CDatabaseServerOptionsTab::save()");
#endif

  bool ret = conn->writeEntry("Show All Databases", booltostr(showAllDatabasesCheckBox->isChecked()));  
  ret &= conn->writeEntry("Use Specific Databases", booltostr(radioButton2->isChecked()));
  QString databaseItems = QString::null;
  for (uint i = 0; i < databases->count(); i++)
    databaseItems += databases->text(i) + "/";
  ret &= conn->writeEntry("Databases", databaseItems);
  return ret;  
}

void CDatabaseServerOptionsTab::setDefaultValues(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CDatabaseServerOptionsTab::setDefaultValues()");
#endif

  showAllDatabasesCheckBox->setChecked(strtobool(conn->readStringEntry("Show All Databases")));
  bool b = strtobool(conn->readStringEntry("Use Specific Databases", "false"));
  radioButton1->setChecked(!b);
  radioButton2->setChecked(b);
  radioButton2Toggled(b);
  QString databaseItems = conn->readStringEntry("Databases", QString::null);
  if (!databaseItems.isNull())
  {
    QStringList dblist = QStringList::split("/", databaseItems);
    for (QStringList::Iterator it = dblist.begin(); it != dblist.end(); ++it)
      databases->insertItem(databaseIcon, *it);
  }
}


CGeneralServerOptionsTab::CGeneralServerOptionsTab(bool isediting, QWidget * parent, const char *name, WFlags fl)
: CConfigDialogTab(parent, name, fl), isEditing(isediting)
{
#ifdef DEBUG
  qDebug("CGeneralServerOptionsTab::CGeneralServerOptionsTab()");
#endif
  
  if (!name)
    setName("CGeneralServerOptionsTab");
  setCaption(tr("General"));
  CGeneralServerOptionsTabLayout = new QGridLayout(this, 1, 1, 4, 2, "CGeneralServerOptionsTabLayout"); 
  
  HostNameBox = new QLineEdit(this, "HostNameBox");
  
  CGeneralServerOptionsTabLayout->addMultiCellWidget(HostNameBox, 1, 1, 1, 3);
  
  tableRetrievalGroup = new QButtonGroup(this, "tableRetrievalGroup");
  tableRetrievalGroup->setColumnLayout(0, Qt::Vertical);
  tableRetrievalGroup->layout()->setSpacing(2);
  tableRetrievalGroup->layout()->setMargin(4);
  tableRetrievalGroupLayout = new QGridLayout(tableRetrievalGroup->layout());
  tableRetrievalGroupLayout->setAlignment(Qt::AlignTop);
  
  retrieveShowTableStatus = new QRadioButton(tableRetrievalGroup, "retrieveShowTableStatus");
  retrieveShowTableStatus->setChecked(true);
  
  tableRetrievalGroupLayout->addWidget(retrieveShowTableStatus, 0, 0);
  
  retrieveShowTables = new QRadioButton(tableRetrievalGroup, "retrieveShowTables");
  if (!isEditing)
    retrieveShowTableStatus->setChecked(true);
  
  tableRetrievalGroupLayout->addWidget(retrieveShowTables, 0, 2);
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  tableRetrievalGroupLayout->addItem(spacer, 0, 1);
  
  CGeneralServerOptionsTabLayout->addMultiCellWidget(tableRetrievalGroup, 5, 5, 0, 3);
  
  portLabel = new QLabel(this, "portLabel");
  portLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, portLabel->sizePolicy().hasHeightForWidth()));
  
  CGeneralServerOptionsTabLayout->addWidget(portLabel, 4, 0);
  
  connectionLabel = new QLabel(this, "connectionLabel");
  connectionLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, connectionLabel->sizePolicy().hasHeightForWidth()));
  
  CGeneralServerOptionsTabLayout->addWidget(connectionLabel, 0, 0);
  
  hostLabel = new QLabel(this, "hostLabel");
  hostLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, hostLabel->sizePolicy().hasHeightForWidth()));
  
  CGeneralServerOptionsTabLayout->addWidget(hostLabel, 1, 0);
  
  PasswordBox = new QLineEdit(this, "PasswordBox");
  PasswordBox->setEchoMode(QLineEdit::Password);
  
  CGeneralServerOptionsTabLayout->addMultiCellWidget(PasswordBox, 3, 3, 1, 3);
  
  UserNameBox = new QLineEdit(this, "UserNameBox");
#ifndef WIN32
  UserNameBox->setText(getlogin());
#else
  UserNameBox->setText("root");
#endif
  
  CGeneralServerOptionsTabLayout->addMultiCellWidget(UserNameBox, 2, 2, 1, 3);
  
  passwordLabel = new QLabel(this, "passwordLabel");
  passwordLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, passwordLabel->sizePolicy().hasHeightForWidth()));
  
  CGeneralServerOptionsTabLayout->addWidget(passwordLabel, 3, 0);
  
  PortBox = new QSpinBox(this, "PortBox");
  PortBox->setMaxValue(100000);
  PortBox->setMinValue(100);
  PortBox->setValue(3306);
  
  CGeneralServerOptionsTabLayout->addWidget(PortBox, 4, 1);
  
  ConnectionNameBox = new QLineEdit(this, "ConnectionNameBox");
  
  CGeneralServerOptionsTabLayout->addMultiCellWidget(ConnectionNameBox, 0, 0, 1, 3);
  
  userNameLabel = new QLabel(this, "userNameLabel");
  userNameLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, userNameLabel->sizePolicy().hasHeightForWidth()));
  
  CGeneralServerOptionsTabLayout->addWidget(userNameLabel, 2, 0);
  QSpacerItem* spacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CGeneralServerOptionsTabLayout->addItem(spacer_2, 4, 2);
  
  Options = new QGroupBox(this, "Options");
  Options->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, Options->sizePolicy().hasHeightForWidth()));
  Options->setColumnLayout(0, Qt::Vertical);
  Options->layout()->setSpacing(2);
  Options->layout()->setMargin(4);
  OptionsLayout = new QGridLayout(Options->layout());
  OptionsLayout->setAlignment(Qt::AlignTop);
  
  socketLabel = new QLabel(Options, "socketLabel");
  socketLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, socketLabel->sizePolicy().hasHeightForWidth()));
  
  OptionsLayout->addWidget(socketLabel, 4, 0);
  
  socketFile = new QLineEdit(Options, "socketFile");
  socketFile->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, socketFile->sizePolicy().hasHeightForWidth()));
  
  OptionsLayout->addMultiCellWidget(socketFile, 4, 4, 1, 2);
  
  socketBrowse = new QPushButton(Options, "socketBrowse");
  socketBrowse->setMinimumSize(QSize(22, 22));
  socketBrowse->setMaximumSize(QSize(22, 22));
  socketBrowse->setPixmap(getPixmapIcon("openIcon"));
  
#ifdef WIN32
  socketLabel->setEnabled(false);
  socketBrowse->setEnabled(false);
  socketFile->setEnabled(false);
#endif    
  
  OptionsLayout->addWidget(socketBrowse, 4, 4);
  
  oneConnectionCheckBox = new QCheckBox(Options, "oneConnectionCheckBox");
  oneConnectionCheckBox->setEnabled(false);  //TODO ... REMOVE WHEN FULLY SUPPORTED
  
  OptionsLayout->addMultiCellWidget(oneConnectionCheckBox, 1, 1, 2, 3);
  
  compressCheckBox = new QCheckBox(Options, "compressCheckBox");
  compressCheckBox->setChecked(true);
  
  OptionsLayout->addMultiCellWidget(compressCheckBox, 0, 0, 0, 1);
  
  reconnectCheckBox = new QCheckBox(Options, "reconnectCheckBox");
  reconnectCheckBox->setChecked(true);
  
  OptionsLayout->addMultiCellWidget(reconnectCheckBox, 1, 1, 0, 1);
  
  blockingCheckBox = new QCheckBox(Options, "blockingCheckBox");
  
  OptionsLayout->addMultiCellWidget(blockingCheckBox, 2, 2, 0, 1);
  
  enableCompletionCheckBox = new QCheckBox(Options, "enableCompletionCheckBox");
  enableCompletionCheckBox->setChecked(true);
  
  OptionsLayout->addMultiCellWidget(enableCompletionCheckBox, 3, 3, 0, 1);
  
  SSLCheckBox = new QCheckBox(Options, "SSLCheckBox");
  SSLCheckBox->setEnabled(false);  //TODO ... REMOVE WHEN FULLY SUPPORTED
  
  OptionsLayout->addWidget(SSLCheckBox, 2, 2);
  
  promptPasswordCheckBox = new QCheckBox(Options, "promptPasswordCheckBox");
  
  OptionsLayout->addMultiCellWidget(promptPasswordCheckBox, 0, 0, 2, 3);
  
  CGeneralServerOptionsTabLayout->addMultiCellWidget(Options, 6, 6, 0, 3);
  
  defaultServer = new QCheckBox(this, "defaultServer");
  defaultServer->setChecked(myApp()->consoleWindow()->databaseListView()->childCount() == 0);
  
  CGeneralServerOptionsTabLayout->addMultiCellWidget(defaultServer, 7, 7, 0, 3);

  disableStartupConnect = new QCheckBox(this, "disableStartupConnect");
  disableStartupConnect->setChecked(false);
  CGeneralServerOptionsTabLayout->addMultiCellWidget(disableStartupConnect, 8, 8, 0, 3);

  QSpacerItem* spacer_3 = new QSpacerItem(20, 160, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CGeneralServerOptionsTabLayout->addItem(spacer_3, 9, 3);
  languageChange();
  clearWState(WState_Polished);

  setTabOrder(ConnectionNameBox, HostNameBox);
  setTabOrder(HostNameBox, UserNameBox);
  setTabOrder(UserNameBox, PasswordBox);
  setTabOrder(PasswordBox, PortBox);
  setTabOrder(PortBox, retrieveShowTableStatus);
  setTabOrder(retrieveShowTableStatus, retrieveShowTables);
  setTabOrder(retrieveShowTables, compressCheckBox);
  setTabOrder(compressCheckBox, reconnectCheckBox);
  setTabOrder(reconnectCheckBox, blockingCheckBox);
  setTabOrder(blockingCheckBox, enableCompletionCheckBox);
  setTabOrder(enableCompletionCheckBox, promptPasswordCheckBox);
  setTabOrder(promptPasswordCheckBox, oneConnectionCheckBox);
  setTabOrder(oneConnectionCheckBox, SSLCheckBox);
  setTabOrder(SSLCheckBox, socketFile);
  setTabOrder(socketFile, socketBrowse);
  setTabOrder(socketBrowse, defaultServer);
  setTabOrder(defaultServer, disableStartupConnect);
  init();
}

void CGeneralServerOptionsTab::languageChange()
{
  setCaption(tr("General"));
  QWhatsThis::add(HostNameBox, tr("Specifies the Database Server you'll be connecting to.  A valid Host Name or IP address is required to establish a connection."));
  tableRetrievalGroup->setTitle(tr("Table retrieval method"));
  retrieveShowTableStatus->setText(tr("SHOW TABLE STATUS"));
  QWhatsThis::add(retrieveShowTableStatus, tr("This method will make the GUI better looking but it will be slower than SHOW TABLES.  This option is recommended if the server you'll be connecting to doesn't have databases with many tables (<= 300)."));
  retrieveShowTables->setText(tr("SHOW TABLES"));
  QWhatsThis::add(retrieveShowTables, tr("This method is recommended when you're connecting to a server where the databases have many tables (> 300).  It's less appealing in terms of visual effects as opposed to SHOW TABLE STATUS, but it will retrieve tables much faster.  This method will be used automatically when connecting to MySQL Servers < 3.23."));
  portLabel->setText(tr("Port"));
  connectionLabel->setText(tr("Name"));
  hostLabel->setText(tr("Host Name"));
  QWhatsThis::add(PasswordBox, tr("Specifies the Password associated with the User Name which will be used to connect to the Server."));
  QWhatsThis::add(UserNameBox, tr("Specifies the User Name which will be used to connect to the Server."));
  passwordLabel->setText(tr("Password"));
  QWhatsThis::add(PortBox, tr("TCP/IP Port Number to which the Server is listening to."));
  QWhatsThis::add(ConnectionNameBox, tr("This is an alias to your connection.  Whatever you type here it will be displayed in the Left Tree."));
  userNameLabel->setText(tr("User Name"));
  Options->setTitle(tr("Options"));
  socketLabel->setText(tr("Socket File"));
  QWhatsThis::add(socketFile, tr("Use a Unix Socket File instead of a TCP/IP connection to the Server.  This option will override the Host Name and Port."));
  socketBrowse->setText(QString::null);
  QWhatsThis::add(socketBrowse, tr("Click to browse for a Socket File"));
  oneConnectionCheckBox->setText(tr("Use a single connection"));
  QWhatsThis::add(oneConnectionCheckBox, tr("Enable this option if you only want to use One connection rather than having MySQL Control Center use as many as required.  This option is recommended for when connecting to a high-load server."));
  compressCheckBox->setText(tr("Use Compression"));
  QWhatsThis::add(compressCheckBox, tr("Use the compressed client/server protocol."));
  reconnectCheckBox->setText(tr("Automatically Reconnect"));
  QWhatsThis::add(reconnectCheckBox, tr("Automatically connect when database connection is disconnected."));
  blockingCheckBox->setText(tr("Blocking Queries"));
  QWhatsThis::add(blockingCheckBox, tr("Use Blocking Queries when enabled.  This option will allow the user to keep doing things while processing a query.  If disabled, the application will block until the query is concluded. If you're unsure about this option, leave it disabled."));
  enableCompletionCheckBox->setText(tr("Completion and Syntax Highlighting"));
  QWhatsThis::add(enableCompletionCheckBox, tr("Enable Completion and Syntax Highlighting for the Database, Tables and Fields in this connection."));
  SSLCheckBox->setText(tr("Enable SSL"));
  QWhatsThis::add(SSLCheckBox, tr("Enable SSL between client & server."));
  promptPasswordCheckBox->setText(tr("Prompt for Password"));
  QWhatsThis::add(promptPasswordCheckBox, tr("When enabled, a Password Dialog will appear before a connection to the Server is attempted.  This is useful for when you don't want to store your password in the settings file."));
  defaultServer->setText(tr("Make this server the Default Connection"));
  QWhatsThis::add(defaultServer, tr("When checked, this server will be used as the default server for command line parameters."));
  disableStartupConnect->setText(tr("Never automatically connect to this server on startup"));
  QWhatsThis::add(disableStartupConnect, tr("When checked, MySQLCC will not automatically connect to this server on startup."));
}


void CGeneralServerOptionsTab::setDefaultValues(CConfig *Settings)
{
#ifdef DEBUG
  qDebug("CGeneralServerOptionsTab::setDefaultValues()");
#endif

  ConnectionNameBox->setText(charReplace(Settings->configName(), ';', ":"));
  defaultServer->setChecked(ConnectionNameBox->text() == myApp()->defaultConnectionName() && !myApp()->defaultConnectionName().isEmpty());
  HostNameBox->setText(Settings->readStringEntry("Host"));
  UserNameBox->setText(Settings->readStringEntry("User"));  
  PortBox->setValue(Settings->readNumberEntry("Port"));  
  compressCheckBox->setChecked(strtobool(Settings->readStringEntry("Compress", "false")));
  reconnectCheckBox->setChecked(strtobool(Settings->readStringEntry("Reconnect", "true")));
  promptPasswordCheckBox->setChecked(strtobool(Settings->readStringEntry("Prompt Password", "false")));
  if (!promptPasswordCheckBox->isChecked())
    PasswordBox->setText(Settings->readStringEntry("Password"));
  blockingCheckBox->setChecked(strtobool(Settings->readStringEntry("Blocking Queries")));
  oneConnectionCheckBox->setChecked(strtobool(Settings->readStringEntry("One Connection")));
  enableCompletionCheckBox->setChecked(strtobool(Settings->readStringEntry("Completion")));
  SSLCheckBox->setChecked(strtobool(Settings->readStringEntry("SSL")));
  disableStartupConnect->setChecked(strtobool(Settings->readStringEntry("Disable Startup Connect", "false")));

  bool r = strtobool(Settings->readStringEntry("Retrieve Method", "true"));
  retrieveShowTableStatus->setChecked(r);
  retrieveShowTables->setChecked(!r);
  
#ifndef WIN32  
  socketFile->setText(Settings->readStringEntry("Socket"));
#endif
}

QString CGeneralServerOptionsTab::validate() const
{
#ifdef DEBUG
  qDebug("CGeneralServerOptionsTab::validate()");
#endif
  
  QString Err = QString::null;
  if (socketFile->text().stripWhiteSpace().isEmpty() && HostNameBox->text().stripWhiteSpace().isEmpty())
    HostNameBox->setText("localhost");
  else
  {
    if (socketFile->text().isEmpty() && !validateAlphaNum(HostNameBox->text()))
      Err = hostLabel->text() + ", ";	
    if (!validateAlphaNum(UserNameBox->text()))
      Err += userNameLabel->text() + ", ";
    if (!Err.isEmpty())          
      Err = tr("The following fields contain invalid characters") + ": " + Err.left(Err.length()-2);    
  }
  
#ifndef WIN32  //Detect if mysql.sock exists if "localhost" is used.
  if (Err.isEmpty() && (HostNameBox->text().lower().stripWhiteSpace() == "localhost" && PortBox->value() == 3306) && socketFile->text().stripWhiteSpace().isEmpty())
  {
    QStringList sock_path;
    sock_path.append("/tmp/mysql.sock");
    sock_path.append("/var/lib/mysql/mysql.sock");
    for (QStringList::Iterator it = sock_path.begin(); it != sock_path.end(); ++it)
      if (QFile::exists(*it))
      {
        socketFile->setText(*it);
        break;
      }
  }
#endif  
  
  return Err;
}

bool CGeneralServerOptionsTab::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CGeneralServerOptionsTab::save()");
#endif
  
  bool ret = conn->writeEntry("Host", HostNameBox->text().stripWhiteSpace());  
  ret &= conn->writeEntry("Port", PortBox->value());
  //ret &= conn->writeEntry("Timeout", connectTimeout->value());
  ret &= conn->writeEntry("User", UserNameBox->text().stripWhiteSpace());
  if (!promptPasswordCheckBox->isChecked())
    ret &= conn->writeEntry("Password", PasswordBox->text());
  ret &= conn->writeEntry("Compress", booltostr(compressCheckBox->isChecked()));
  ret &= conn->writeEntry("Reconnect", booltostr(reconnectCheckBox->isChecked()));
  ret &= conn->writeEntry("Prompt Password", booltostr(promptPasswordCheckBox->isChecked()));
  ret &= conn->writeEntry("Blocking Queries", booltostr(blockingCheckBox->isChecked()));
  ret &= conn->writeEntry("One Connection", booltostr(oneConnectionCheckBox->isChecked()));
  ret &= conn->writeEntry("Completion", booltostr(enableCompletionCheckBox->isChecked()));
  ret &= conn->writeEntry("SSL", booltostr(SSLCheckBox->isChecked()));
  ret &= conn->writeEntry("Retrieve Method", booltostr(retrieveShowTableStatus->isChecked()));
  ret &= conn->writeEntry("Disable Startup Connect", booltostr(disableStartupConnect->isChecked()));

#ifndef WIN32
  ret &= conn->writeEntry("Socket", socketFile->text().stripWhiteSpace());
#endif
  return ret;
}

void CGeneralServerOptionsTab::setSocketFile()
{
#ifdef DEBUG
  qDebug("CGeneralServerOptionsTab::setSocketFile()");
#endif
  
  socketFile->setText(QFileDialog::getOpenFileName("", tr("All Files (*)"), this, "setSocketFile", tr("Select the MySQL Socket file")));
}

void CGeneralServerOptionsTab::init()
{
#ifdef DEBUG
  qDebug("CGeneralServerOptionsTab::init()");
#endif
  
  connect(socketBrowse, SIGNAL(clicked()), this, SLOT(setSocketFile())); 
}

CRegisterServerDialog::CRegisterServerDialog(CMessagePanel * messagepanel, QWidget* parent)
:CConfigDialog(parent, "CRegisterServerDialog"), isEditing(false)
{
#ifdef DEBUG
  qDebug("CRegisterServerDialog::CRegisterServerDialog(CMessagePanel *, QWidget *)");
#endif
  
  setCaption(tr("Register Server"));
  initConnectionDialog(messagepanel);
}

CRegisterServerDialog::CRegisterServerDialog(const QString &cname, CMessagePanel * messagepanel, QWidget* parent)
:CConfigDialog(parent, "CRegisterServerDialogEdit"), isEditing(true)
{
#ifdef DEBUG
  qDebug("CRegisterServerDialog::CRegisterServerDialog(const QString &, CMessagePanel *, QWidget *)");
#endif
  
  cfgname = cname;
  setCaption("[" + cfgname + "] " + tr("Register Server"));  
  initConnectionDialog(messagepanel);  
}


void CRegisterServerDialog::initConnectionDialog(CMessagePanel *messagepanel)
{
#ifdef DEBUG
  qDebug("CRegisterServerDialog::initConnectionDialog()");
#endif

  myApp()->incCritical();
  messagePanel = messagepanel;
  GeneralServerOptionsTab = new CGeneralServerOptionsTab(isEditing, tab(), "General");
  insertTab(GeneralServerOptionsTab);
  insertTab(new CMySQLOptionsTab(tab()));
  CDatabaseServerOptionsTab *databaseServerOptionsTab = new CDatabaseServerOptionsTab(tab());
  insertTab(databaseServerOptionsTab);
  insertTab(new CSaveServerOptionsTab(tab()));
  customButton->setText(tr("&Test"));
  customButton->setIconSet(getPixmapIcon("pingIcon"));
  QWhatsThis::add(customButton, tr("Click here to test the connection."));
  connect(customButton, SIGNAL(clicked()), this, SLOT(testConnection()));
  customButton->show();
  
  if (isEditing)
  {
    CConfig *Settings = new CConfig(cfgname, connections_path);
    setDefaultValues(Settings);
    delete Settings;    
    okPushButton->setText(tr("&Apply"));
    QWhatsThis::add(okPushButton, tr("Click to Apply changes to this connection."));
    if (databaseServerOptionsTab->databases->count() == 0)
    {
      CMySQLServer tmp(cfgname);
      if (tmp.connect())
      {
        CMySQLQuery q(tmp.mysql());
        q.setEmitErrors(false);
        q.setEmitMessages(false);
        QPixmap databaseIcon = getPixmapIcon("databaseDisconnectedIcon"); 
        if (q.exec("SHOW databases"))
        {
          while (q.next())
            databaseServerOptionsTab->databases->insertItem(databaseIcon, q.row(0));
        }
      }
    }
  }
  else
  {
    okPushButton->setText(tr("&Add"));
    QWhatsThis::add(okPushButton, tr("Click to Add a new connection."));
  }
  is_default_connection_name = GeneralServerOptionsTab->HostNameBox->text() && !GeneralServerOptionsTab->HostNameBox->text().isEmpty();
  setMinimumSize(QSize(0, 349));
  myResize(421, 349);
}

void CRegisterServerDialog::testConnection()
{
#ifdef DEBUG
  qDebug("CRegisterServerDialog::testConnection()");
#endif
  
  if (validate())
  {
    QString tmpConnectionName = "mysqlcctmp";
    CConfig *connection = new CConfig(tmpConnectionName);
    
    for (unsigned int i = 1; i; i++)
    {
      connection->setConfigName(tmpConnectionName + "_" + QString::number(i), connections_path);        
      if (!connection->exists())
      {
        tmpConnectionName = connection->configName();        
        break;
      }
    }
    
    if (!save(connection))
    {
      messagePanel->critical(tr("An error occured while saving the temporary connection."));
      return;
    }
    delete connection;
    messagePanel->information(tr("Testing connection to MySQL Server ..."));
    messagePanel->information(tr("connecting ..."));
    CMySQLServer *testConn = new CMySQLServer(tmpConnectionName);
    testConn->setPrintConnectionName(false);
    if (testConn->connect())
      testConn->showMessage(INFORMATION, tr("Connection Successful."));
    this->show();
    qApp->processEvents();
    testConn->remove();
    delete testConn;    
  }
}

CRegisterServerDialog::~CRegisterServerDialog()
{
#ifdef DEBUG
  qDebug("CRegisterServerDialog::~CRegisterServerDialog()");
#endif

  myApp()->decCritical();
}

void CRegisterServerDialog::okButtonClicked()
{
#ifdef DEBUG
  qDebug("CRegisterServerDialog::okButtonClicked()");
#endif
  
  if (validate())
  {	    
    QString tmpConnectionName = GeneralServerOptionsTab->ConnectionNameBox->text().stripWhiteSpace();
    
    if (tmpConnectionName.isEmpty())    
    {
      if (GeneralServerOptionsTab->HostNameBox->text().isEmpty() && !GeneralServerOptionsTab->socketFile->text().isEmpty())  //socket connection
        GeneralServerOptionsTab->HostNameBox->setText("localhost");
      
      tmpConnectionName = GeneralServerOptionsTab->UserNameBox->text() + "@" + GeneralServerOptionsTab->HostNameBox->text() + ";";
      tmpConnectionName += (GeneralServerOptionsTab->socketFile->text().isEmpty()) ? QString::number(GeneralServerOptionsTab->PortBox->value()) : getFileName(GeneralServerOptionsTab->socketFile->text());
    }
    
    CConfig connection(tmpConnectionName, connections_path);
    
    if (!isEditing && connection.exists())
    {
      
      for (unsigned int i = 1; i; i++)
      {
        connection.setConfigName(tmpConnectionName + "_" + QString::number(i), connections_path);        
        if (!connection.exists())
        {
          tmpConnectionName = connection.configName();
          if (!GeneralServerOptionsTab->ConnectionNameBox->text().stripWhiteSpace().isEmpty())
            messagePanel->warning(tr("A Connection with that name already exists.  The following connection name was used") + ": " + getFileName(tmpConnectionName));
          break;
        }
      }
    }
    else
      if (isEditing && cfgname != tmpConnectionName && connection.exists())
      {
        messagePanel->critical(tr("A Connection with that name already exists."));        
        return;     
      }    
      
    if (!save(&connection))
    {
      messagePanel->critical(tr("An Error occurred while saving the Connection."));
      return;
    }

    if (GeneralServerOptionsTab->defaultServer->isChecked())
    {
      CConfig *c = new CConfig();
      c->writeEntry("Default Connection Name", tmpConnectionName);
      c->save();
      myApp()->setDefaultConnectionName(tmpConnectionName);
      delete c;
    }
    else
      if (is_default_connection_name)  //Default Connection was unchecked
      {
        CConfig *c = new CConfig();
        c->removeEntry("Default Connection Name");
        c->save();
        myApp()->setDefaultConnectionName(QString::null);
        delete c;
      }
      
    if (isEditing)
    {
      if (cfgname != tmpConnectionName)
        CConfig::remove(cfgname, connections_path);        
      emit connectionEdited(tmpConnectionName);
      messagePanel->information(tr("Connection edited successfully."));
    }
    else
    {
      messagePanel->information(tr("Connection added successfully."));
      emit newConnection();
    }
    dialogAccepted();
  }
}

bool CRegisterServerDialog::validate()
{
#ifdef DEBUG
  qDebug("CRegisterServerDialog::validate()");
#endif
  
  QString err = CConfigDialog::validate();
  if (!err.isEmpty())
  {    
    messagePanel->warning(err);
    return false;
  }
  return true;
}

bool CRegisterServerDialog::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CRegisterServerDialog::save()");
#endif
  
  bool ret = CConfigDialog::save(conn);
  ret &= conn->writeEntry("Modified", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
  if (!isEditing)
    ret &= conn->writeEntry("Created", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
  ret &= conn->save();  
  return ret;
}
