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
#include "CUserAdminWindow.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "globals.h"
#include "config.h"
#include "panels.h"
#include <stddef.h>
#include <qvariant.h>
#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3listbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <q3popupmenu.h>
#include <q3toolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <QCloseEvent>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 2
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CGrantItem::CGrantItem(Q3ListView * parent, const QString &txt, const QPixmap &pix, Type t)
: Q3CheckListItem(parent, txt, Q3CheckListItem::CheckBox)
{
#ifdef DEBUG
  qDebug("CGrantItem::CGrantItem(QListView *, const QString &, const QPixmap &, Type)");
#endif

  init(pix, t);
}

CGrantItem::CGrantItem(Q3ListViewItem * parent, const QString &txt, const QPixmap &pix, Type t)
: Q3CheckListItem(parent, txt, Q3CheckListItem::CheckBox)
{
#ifdef DEBUG
  qDebug("CGrantItem::CGrantItem(QListViewItem *, const QString &, const QPixmap &, Type)");
#endif

  init(pix, t);
}

void CGrantItem::init(const QPixmap &pix, const Type &t)
{
#ifdef DEBUG
  qDebug("CGrantItem::init(const QPixmap &, const Type &)");
#endif

  m_type = t;
  setPixmap(0, pix);
  dbname = QString::null;
  tblname = QString::null;
  allPrivs = false;
  withGrant = false;
  privileges.clear();
}

int CGrantItem::compare(Q3ListViewItem * i, int col, bool ascending) const
{
#ifdef DEBUG
  qDebug("CGrantItem::compare(QListViewItem *, int, bool)");
#endif

  return ((m_type == GLOBAL) ? (ascending ? -1 : 1) :
         ((((CGrantItem *)i)->type() == GLOBAL) ? (ascending ? 1 : -1) :
         key( col, ascending ).compare( i->key(col, ascending))));
}

void CGrantItem::stateChange(bool b)
{
#ifdef DEBUG
  qDebug("CGrantItem::stateChange(bool)");
#endif

  if (type() == DATABASE)
  {
    Q3CheckListItem *d = (Q3CheckListItem *)firstChild();
    listView()->setCursor(Qt::WaitCursor);
    while (d != NULL)
    {    
      d->setEnabled(!b);
      if (d->isOn() && b)
        d->setOn(false);
      d = (Q3CheckListItem *)d->nextSibling();
    }
    listView()->setCursor(Qt::ArrowCursor);
  }
}

CUserAdminWindow::CUserAdminWindow(QWidget *parent, CMySQLServer *m, const QString &u, const QString &h, bool e)
: CMyWindow(parent, "CUserAdminWindow"), username(u), hostname(h), is_editing(e)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::CUserAdminWindow(QWidget *, CMySQLServer *, const QString &, const QString &, bool)");
#endif

  myApp()->incCritical();
  is_busy = false;
  block = false;
  setIcon(getPixmapIcon("applicationIcon"));
  dbIcon = getPixmapIcon("databaseConnectedIcon");
  tableIcon = getPixmapIcon("tableIcon");
  globalPrivsIcon = getPixmapIcon("globalPrivsIcon");
  emit_refresh = false;
  mysql = new CMySQLServer(m->connectionName(), m->messagePanel());
  mysql->connect();
  
  setCentralWidget(new QWidget(this, "qt_central_widget"));
  CUserAdminWindowLayout = new Q3GridLayout(centralWidget(), 1, 1, 4, 2, "CUserAdminWindowLayout"); 
  
  Password = new QLineEdit(centralWidget(), "Password");
  Password->setFrameShape(QFrame::StyledPanel);
  Password->setFrameShadow(QLineEdit::Sunken);
  Password->setEchoMode(QLineEdit::Password);
  Q3WhatsThis::add(Password, trUtf8("This is the Password for Username."));
  
  CUserAdminWindowLayout->addWidget(Password, 2, 1);
  
  hostLabel = new QLabel(centralWidget(), "hostLabel");
  hostLabel->setText(trUtf8("Host"));
  
  CUserAdminWindowLayout->addWidget(hostLabel, 1, 0);
  
  passwordLabel = new QLabel(centralWidget(), "passwordLabel");
  passwordLabel->setText(trUtf8("Password"));  
  
  CUserAdminWindowLayout->addWidget(passwordLabel, 2, 0);
  
  Host = new QLineEdit(centralWidget(), "Host");
  Host->setFrameShape(QFrame::StyledPanel);
  Host->setFrameShadow(QLineEdit::Sunken);
  Host->setText(hostname);
  Q3WhatsThis::add(Host, trUtf8("This is the Hostname the Username will be connecting from.  Use '%' for referring to Any."));
  
  CUserAdminWindowLayout->addWidget(Host, 1, 1);
  
  ButtonGroup1 = new Q3ButtonGroup(centralWidget(), "ButtonGroup1");
  ButtonGroup1->setTitle(trUtf8("Privileges"));
  ButtonGroup1->setColumnLayout(0, Qt::Vertical);
  ButtonGroup1->layout()->setSpacing(6);
  ButtonGroup1->layout()->setMargin(6);
  ButtonGroup1Layout = new Q3GridLayout(ButtonGroup1->layout());
  ButtonGroup1Layout->setAlignment(Qt::AlignTop);
  
  allPrivileges = new QCheckBox(ButtonGroup1, "allPrivileges");
  allPrivileges->setText(trUtf8("All Privileges"));
  Q3WhatsThis::add(allPrivileges, trUtf8("This option will GRANT ALL PRIVILEGES to Username"));
  
  ButtonGroup1Layout->addWidget(allPrivileges, 0, 0);
  
  withGrantOption = new QCheckBox(ButtonGroup1, "withGrantOption");
  withGrantOption->setText(trUtf8("With GRANT option"));
  Q3WhatsThis::add(withGrantOption, trUtf8("This option will grant Username privileges for GRANT."));
  
  ButtonGroup1Layout->addWidget(withGrantOption, 2, 0);
  
  privilegeListBox = new Q3ListBox(ButtonGroup1, "privilegeListBox");
  privilegeListBox->setSelectionMode(Q3ListBox::Multi);
  Q3WhatsThis::add(privilegeListBox, trUtf8("Select the desired Privileges for Username"));
  
  ButtonGroup1Layout->addWidget(privilegeListBox, 1, 0);
  
  CUserAdminWindowLayout->addMultiCellWidget(ButtonGroup1, 3, 3, 0, 1);
  
  Username = new QLineEdit(centralWidget(), "Username");
  Username->setText(username);
  Q3WhatsThis::add(Username, trUtf8("This is the Username you will be Granting / Editing privileges to."));
  
  CUserAdminWindowLayout->addWidget(Username, 0, 1);
  
  usernameLabel = new QLabel(centralWidget(), "usernameLabel");
  usernameLabel->setText(trUtf8("Username"));
  
  CUserAdminWindowLayout->addWidget(usernameLabel, 0, 0);
  
  Layout2 = new Q3HBoxLayout(0, 0, 6, "Layout2"); 
  
  deleteButton = new QPushButton(centralWidget(), "deleteButton");
  deleteButton->setText(trUtf8("&Delete User"));
  deleteButton->setIconSet(getPixmapIcon("deleteUserIcon"));
  deleteButton->setEnabled(is_editing);
  Layout2->addWidget(deleteButton);
  QSpacerItem* spacer = new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  Layout2->addItem(spacer);
  
  applyButton = new QPushButton(centralWidget(), "applyButton");
  applyButton->setMinimumSize(QSize(60, 0));
  applyButton->setText((const QString&)(is_editing ? trUtf8("&Apply") : trUtf8("&Add")));
  applyButton->setDefault(true);
  Q3WhatsThis::add(applyButton, trUtf8("Click here to apply changes any you have made."));
  Layout2->addWidget(applyButton);
  
  closeButton = new QPushButton(centralWidget(), "closeButton");
  closeButton->setMinimumSize(QSize(60, 0));
  closeButton->setText(trUtf8("&Close"));
  Q3WhatsThis::add(closeButton, trUtf8("Close this Dialog without saving any changes you have made."));
  Layout2->addWidget(closeButton);
  
  CUserAdminWindowLayout->addMultiCellLayout(Layout2, 4, 4, 0, 2);
  
  databaseListView = new Q3ListView(centralWidget(), "databaseListView");
  databaseListView->addColumn(tr("Allow access to"));
  databaseListView->header()->setResizeEnabled(false, databaseListView->header()->count() - 1);  
  databaseListView->setShowSortIndicator(true);
  databaseListView->setResizeMode(Q3ListView::AllColumns);
  databaseListView->setRootIsDecorated (true);    
  Q3WhatsThis::add(databaseListView, trUtf8("These are the Databases/Tables username will have access to."));
  
  CUserAdminWindowLayout->addMultiCellWidget(databaseListView, 0, 3, 2, 2);  
  
  setTabOrder(Username, Host);
  setTabOrder(Host, Password);
  setTabOrder(Password, allPrivileges);
  setTabOrder(allPrivileges, privilegeListBox);
  setTabOrder(privilegeListBox, withGrantOption);
  setTabOrder(withGrantOption, databaseListView);
  setTabOrder(databaseListView, applyButton);
  setTabOrder(applyButton, closeButton);
  setTabOrder(closeButton, deleteButton);
  myResize(437, 321);

  connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(applyButton, SIGNAL(clicked()), this, SLOT(applyClicked()));
  connect(databaseListView, SIGNAL(currentChanged(Q3ListViewItem *)), this, SLOT(setCurrentItem(Q3ListViewItem *)));
  connect(privilegeListBox, SIGNAL(selectionChanged()), this, SLOT(privilegeListBoxChanged()));
  connect(allPrivileges, SIGNAL(toggled(bool)), this, SLOT(allPrivilegesToggled(bool)));
  connect(withGrantOption, SIGNAL(toggled(bool)), this, SLOT(withGrantToggled(bool)));
  connect(Username, SIGNAL(textChanged(const QString &)), this, SLOT(checkEditing(const QString &)));
  connect(Host, SIGNAL(textChanged(const QString &)), this, SLOT(checkEditing(const QString &)));  
  refresh();
}

CUserAdminWindow::~CUserAdminWindow()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::~CUserAdminWindow()");
#endif

  delete mysql;
  myApp()->decCritical();
}

void CUserAdminWindow::setBusy(bool b)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::setBusy(bool)");
#endif

  is_busy = b;
  QCursor c = b ? Qt::WaitCursor : Qt::ArrowCursor;
  setCursor(c);
  databaseListView->setCursor(c);
  closeButton->setEnabled(!b);
  applyButton->setEnabled(!b);
  qApp->processEvents();
}

void CUserAdminWindow::checkEditing(const QString &)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::checkEditing(const QString &)");
#endif

  bool tmp = is_editing;
  is_editing = (Username->text() == username && Host->text() == hostname);

  if (tmp != is_editing)
  {
    setWindowCaption();
    deleteButton->setEnabled(is_editing);
    applyButton->setText((const QString&)(is_editing ? trUtf8("&Apply") : trUtf8("&Add")));
  }
}

void CUserAdminWindow::setWindowCaption()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::setWindowCaption()");
#endif

  setCaption("[" + mysql->connectionName() + "] " + (!is_editing ? tr("Add User") : tr("Edit User") + " - '" + username + "@" + hostname + "'"));
}

void CUserAdminWindow::refresh()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::refresh()");
#endif

  setWindowCaption(); 
  refreshPrivilegesList();

  Q3ListViewItemIterator it(databaseListView);
  for (; it.current(); ++it)
  {
    CGrantItem *i = (CGrantItem *) it.current();
    i->allPrivs = false;
    i->withGrant = false;
    i->privileges.clear();    
  }

  refreshItems();
  if (is_editing)
    parseUserGrants();
  databaseListView->setSelected(databaseListView->firstChild(), true);
  if (databaseListView->childCount() > 0)
  {
    setCurrentItem(databaseListView->firstChild());
    applyButton->setEnabled(true);
  }
  else
  {
    mysql->messagePanel()->critical(tr("An error occurred while refreshing the databases or tables."));
    applyButton->setEnabled(false);
  }
}

void CUserAdminWindow::refreshItems()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::refreshItems()");
#endif

  if (is_busy)
    return;  
  setBusy(true);

  databaseListView->clear();
  CMySQLQuery *q = new CMySQLQuery(mysql->mysql());
  CMySQLQuery *q2 = new CMySQLQuery(mysql->mysql());
  q->setEmitErrors(false);
  q->setEmitMessages(false);
  q2->setEmitErrors(false);
  q2->setEmitMessages(false);
  if (q->exec("SHOW DATABASES"))
  {
    CGrantItem * global = new CGrantItem(databaseListView, tr("Global Privileges"), globalPrivsIcon, CGrantItem::GLOBAL);
    getDefaultItemPrivileges(global->privileges, CGrantItem::GLOBAL);
    global->setDatabaseName("*");
    global->setTableName("*");
    QString row;
    QStringList default_db_privs;
    QStringList default_table_privs;
    getDefaultItemPrivileges(default_db_privs, CGrantItem::DATABASE);
    getDefaultItemPrivileges(default_table_privs, CGrantItem::TABLE);
    while (q->next())
    {
      row = q->row(0);
      if (mysql->mysql()->mysqlSelectDb(row))  //Must be used to check access to the database.  Else show tables from could be used.
      {
        CGrantItem * database = new CGrantItem(databaseListView, row, dbIcon, CGrantItem::DATABASE);
        database->setDatabaseName(row);
        database->setTableName("*");
        database->privileges = default_db_privs;
        if (q2->exec("SHOW TABLES"))
        {
          while (q2->next())
          {
            CGrantItem * table = new CGrantItem(database, q2->row(0), tableIcon, CGrantItem::TABLE);
            table->setDatabaseName(row);
            table->setTableName(q2->row(0));
            table->privileges = default_table_privs;
          }
        }
      }
      qApp->processEvents();
    }
  }
  delete q;
  delete q2;
  setBusy(false);
}

void CUserAdminWindow::refreshPrivilegesList()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::refreshPrivilegesList()");
#endif

  int version = mysql->mysql()->version().major;
  privilegeListBox->clear();
  
  privilegeListBox->insertItem("Select");
  privilegeListBox->insertItem("Insert");
  privilegeListBox->insertItem("Update");
  privilegeListBox->insertItem("Delete");
  privilegeListBox->insertItem("Create");
  privilegeListBox->insertItem("Drop");
  privilegeListBox->insertItem("Index");
  privilegeListBox->insertItem("Alter");
  privilegeListBox->insertItem("References");
  
  if (version >= 4)
  {
    privilegeListBox->insertItem("Create Temporary Tables");
    privilegeListBox->insertItem("Lock Tables");    
  }

  privilegeListBox->insertItem("Reload");
  privilegeListBox->insertItem("Shutdown");
  privilegeListBox->insertItem("Process");
  privilegeListBox->insertItem("File");

  if (version >= 4)
  {
    privilegeListBox->insertItem("Show Databases");
    privilegeListBox->insertItem("Execute");
    privilegeListBox->insertItem("Replication Client");
    privilegeListBox->insertItem("Replication Slave");
    privilegeListBox->insertItem("Super");
  }
}

void CUserAdminWindow::getDefaultItemPrivileges(QStringList &lst, CGrantItem::Type t)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::getDefaultItemPrivileges(QStringList &, CGrantItem::Type)");
#endif

  lst.clear();
  if (is_editing)
    return;
  if (privilegeListBox->count() > 0)
  {
    for (int i = 0; i <= 8; i++)
      lst.append(privilegeListBox->text(i).lower());
  }

  if (mysql->mysql()->version().major >= 4 && (t == CGrantItem::GLOBAL || t == CGrantItem::DATABASE))
  {
    lst.append(privilegeListBox->text(9).lower());  //Create Temporary Tables
    lst.append(privilegeListBox->text(10).lower()); //Lock Tables
    if (t == CGrantItem::GLOBAL)
      lst.append(privilegeListBox->text(15).lower()); //Show Databases
  }
}

void CUserAdminWindow::setCurrentItem(Q3ListViewItem *i)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::setCurrentItem(QListViewItem *)");
#endif

  if (privilegeListBox->count() < 1 || !i)
  {
    applyButton->setEnabled(false);
    return;
  }

  block = true;
  CGrantItem *item = (CGrantItem *) i;
  uint j;
  for (j = 0; j < privilegeListBox->count(); j++)
  {
    privilegeListBox->item(j)->setSelectable(true);
    privilegeListBox->setSelected(j, item->privileges.find(privilegeListBox->text(j).lower()) != item->privileges.end());
  }

  allPrivileges->setChecked(item->allPrivs);
  if (item->allPrivs)
    privilegeListBox->setEnabled(false);
 
  if (item->type() == CGrantItem::DATABASE || item->type() == CGrantItem::GLOBAL)
  {
    withGrantOption->setEnabled(true);
    withGrantOption->setChecked(item->withGrant);

    if (mysql->mysql()->version().major >= 4)
    {
      privilegeListBox->item(9)->setSelectable(true);  //Create Temporary Tables
      privilegeListBox->item(10)->setSelectable(true); //Lock Tables

      for (j = 11; j <= 19; j++)  //Reload, Shutdown, Process, File, Show db, Execute, Rep Client, Rep Slave, Super
        privilegeListBox->item(j)->setSelectable(item->type() == CGrantItem::GLOBAL);
    }
    else
      for (j = 9; j <= 12; j++)  //Reload, Shutdown, Process, File
        privilegeListBox->item(j)->setSelectable(item->type() == CGrantItem::GLOBAL);
  }
  else
  {
    withGrantOption->setEnabled(false);
    withGrantOption->setChecked(false);
    for (j = mysql->mysql()->version().major >= 4 ? 11 : 9; j < privilegeListBox->count(); j++)
      privilegeListBox->item(j)->setSelectable(false);
  }
  block = false;
}


void CUserAdminWindow::closeEvent(QCloseEvent * e)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::closeEvent(QCloseEvent *)");
#endif

  if (is_busy)
    e->ignore();
  else
  {
    if (emit_refresh)
      emit do_refresh();
    e->accept();
    CMyWindow::closeEvent(e);
  }
}

void CUserAdminWindow::deleteClicked()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::deleteClicked()");
#endif

  if (removeUser(mysql, Username->text(), Host->text()))
  {
    emit_refresh = true;
    close();
  }
}

static const QString strip_char(const QString &str, QChar c)
{
#ifdef DEBUG
  qDebug("static CUserAdminWindow::strip_char(const QString &, QChar)");
#endif

  QString tmp = str;
  if (str.find(c) != -1)
  {
    tmp = QString::null;
    for (uint i = 0; i < str.length(); i++)
      if (str.at(i) != c)
        tmp += str.at(i);
  }
  return tmp;
}

CGrantItem *CUserAdminWindow::findItem(const QString &db, const QString &tbl)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::findItem(const QString &, const QString &)");
#endif

  QString tmp_db;
  QString tmp_tbl;
  bool db_wild = false;
  bool tbl_wild = false;

  if (db.endsWith("%"))
  {
    tmp_db = db.left(db.length() - 2);
    db_wild = true;
  }
  else
    tmp_db = db;

  if (tbl.endsWith("%"))
  {
    tmp_tbl = tbl.left(tbl.length() - 2);
    tbl_wild = true;
  }
  else
    tmp_tbl = tbl;

  Q3ListViewItemIterator it(databaseListView);
  bool ok = false;
  for (; it.current(); ++it)
  {
    CGrantItem *i = (CGrantItem *) it.current();
    ok = db_wild ? i->databaseName().startsWith(tmp_db) : i->databaseName() == db;
    ok &= tbl_wild ? i->tableName().startsWith(tmp_tbl) : i->tableName() == tbl;

    if (ok)
      return i;
  }
  return 0;
}

void CUserAdminWindow::parseUserGrants()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::parseUserGrants()");
#endif

  CMySQLQuery *qry = new CMySQLQuery(mysql->mysql(), true);
  qry->setEmitMessages(false);
  if (qry->exec("SHOW GRANTS FOR '" + Username->text() + "'@'" + Host->text() + "'"))
  {
    QString line;
    QString privs;
    QString dbname;
    QString table;
    QString passwd;
    bool withGrant;
    int p;
    int p2;
    CGrantItem *item;
    if (qry->numRows() == 0)  // has USAGE ... check the "Global Privileges" option ...
    {
      item = findItem("*", "*");
      if (item != 0)
        item->setOn(true);
    }
    else
    while (qry->next())
    { 
      privs = QString::null;
      line = QString::null;
      dbname = QString::null;
      passwd = QString::null;
      table = QString::null;

      withGrant = false;
      line = qry->row(0);
      p = line.find(" ") + 1;
      p2 = line.find(" ON ");

      privs = line.mid(p, p2++ - p);
      line = line.mid(p2, line.length() - p2);
      line = line.mid(line.find(" ") + 1);
      p = line.find(".");
      dbname = strip_char(line.left(p), '`');
      line = line.mid(p+1);
      p = line.find(" ");
      table = strip_char(line.left(p), '`');
      p = line.find("'");
      line = line.mid(p);
      line = line.mid(line.find(" "));
      p = line.find("IDENTIFIED BY");
      if (p != -1)
      {
        p = line.find("'") + 1;
        line = line.mid(p);
        passwd = line.left(line.find("'"));
      }
      if (line.find("WITH GRANT OPTION") != -1)
        withGrant = true;

      item = findItem(dbname, table);
      if (item != 0)
      {
        item->setOn(true);
        item->withGrant = withGrant;
        privs = privs.stripWhiteSpace().lower();
        if (!passwd.isEmpty())
        {
          Password->setText(passwd);
          password = passwd;
        }
        item->privileges.clear();
        item->allPrivs = privs == "all privileges";
        if (!item->allPrivs && privs != "usage")
          item->privileges = QStringList::split(", ", privs);

        if (item->type() == CGrantItem::TABLE)
          item->parent()->setOpen(true);
      }
    }
  }
  delete qry;
}

bool CUserAdminWindow::grantPrivileges()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::grantPrivileges()");
#endif

  setBusy(true);
  bool ret = true;
  if (is_editing)
  {
    if (Username->text() != username)  
      is_editing = false;
    if (password != Password->text())
      password = Password->text();
    else
      password = QString::null;
  }
  else
    password = Password->text();
 
  if (Host->text().isEmpty())
    Host->setText("%");

  if (is_editing)
    ret &= removeUser(mysql, username, hostname, true);

  Q3ListViewItemIterator it(databaseListView);
  QString sql;
  CMySQLQuery *query = new CMySQLQuery(mysql->mysql());
  query->setEmitMessages(false);
  while (it.current() != 0)
  {
    CGrantItem *item = (CGrantItem *) it.current();
    sql = QString::null;
    if (item->isOn())
    {
      sql = "GRANT ";
      if (item->allPrivs)
        sql += "ALL PRIVILEGES";
      else if (item->privileges.empty())
        sql += "USAGE";
      else
        sql += item->privileges.join(",");

      if (item->databaseName() != "*")
        sql += " ON " + mysql->mysql()->quote(item->databaseName()) + ".";
      else
        sql += " ON " + item->databaseName() + ".";

      if (item->tableName() != "*")
        sql += mysql->mysql()->quote(item->tableName());
      else
        sql += item->tableName();

      sql += " TO '" + Username->text() + "'@'" + Host->text() + "'";
      
      if (!password.isNull())
        sql += " IDENTIFIED BY '" + password + "'";

      if (item->withGrant)
        sql += " WITH GRANT OPTION";

      ret &= query->execStaticQuery(sql);      
    }
    ++it;
  }
  delete query;
  setBusy(false);
  return ret;
}

void CUserAdminWindow::applyClicked()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::applyClicked()");
#endif

  emit_refresh = true;
  bool ver = false;
  if (grantPrivileges())
  {
    if (!is_editing)
    {
      mysql->messagePanel()->information(tr("User created successfully."));
      if (mysql->mysql()->version().major <= 3 && mysql->mysql()->version().minor <= 22)
        ver = true;        
      else
      {
        hostname = Host->text();
        username = Username->text();
        applyButton->setText(trUtf8("&Apply"));
        deleteButton->setEnabled(true);
        is_editing = true;
      }
    }
    else
      mysql->messagePanel()->information(tr("User modified successfully."));
    if (!ver)
      refresh();
  }
  else
    mysql->messagePanel()->critical(tr("An error occured while applying the Grants."));

  if (ver)
    close();
}

bool CUserAdminWindow::removeUser(CMySQLServer *m, const QString &username, const QString &hostname, bool quiet)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::removeUser(CMySQLServer *, const QString &, const QString &, bool)");
#endif

  if (myApp()->confirmCritical() && !quiet)
    if ((QMessageBox::information(0, tr("Confirm Delete User"), 
      tr("Are you sure you want to Delete User") + ": '" + username + "@" + hostname + "' ?",
      tr("&Yes"), QObject::tr("&No")) != 0))
      return false;
  bool ret = true;
  CMySQLQuery *q = new CMySQLQuery(m->mysql());
  q->setEmitMessages(false);
  QStringList tables;
  if (!quiet)
    tables += "user";
  else
  {
    ret &= q->execStaticQuery("REVOKE ALL PRIVILEGES ON *.* FROM '" + username + "'@'" + (hostname.isEmpty() ? (const QString)"%" : hostname) + "'");
    ret &= q->execStaticQuery("REVOKE GRANT OPTION ON *.* FROM '" + username + "'@'" + (hostname.isEmpty() ? (const QString)"%" : hostname) + "'");
  }
  tables += "db";
  tables += "tables_priv";
  tables += "columns_priv";
  for (QStringList::Iterator it = tables.begin(); it != tables.end(); ++it)  
    ret &= q->execStaticQuery("DELETE FROM mysql." + *it + " WHERE User='" + username + "' AND Host = '" + hostname + "'");  
  if (ret)
  {
    q->execStaticQuery("FLUSH PRIVILEGES");
    if (!quiet)
      m->messagePanel()->information(tr("User deleted successfully"));
  }
  delete q;
  return ret;
}

void CUserAdminWindow::allPrivilegesToggled(bool b)
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::allPrivilegesToggled(bool)");
#endif

  CGrantItem *p = (CGrantItem *)databaseListView->currentItem();
  if (p == 0)
    return;
  p->allPrivs = b;  
  privilegeListBox->setEnabled(!b);
}

void CUserAdminWindow::withGrantToggled(bool b)
{
  CGrantItem *p = (CGrantItem *)databaseListView->currentItem();
  if (!p)
    return;
  p->withGrant = b;
}

void CUserAdminWindow::privilegeListBoxChanged()
{
#ifdef DEBUG
  qDebug("CUserAdminWindow::privilegeListBoxChanged()");
#endif

  if (block || is_busy)
    return;
  block = true;
  CGrantItem *p = (CGrantItem *)databaseListView->currentItem();
  if (p == 0)
    return;

  for (uint i = 0; i < privilegeListBox->count(); i++)
  {
    if (privilegeListBox->isSelected(i))
    {
      if (p->privileges.find(privilegeListBox->text(i).lower()) == p->privileges.end())
        p->privileges.append(privilegeListBox->text(i).lower());
    }
    else
      p->privileges.remove(privilegeListBox->text(i).lower());
  }

 block = false;
}
