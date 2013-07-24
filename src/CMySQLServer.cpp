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
#include "CMySQLServer.h"
#include "globals.h"
#include "config.h"
#include "CConfig.h"
#include "CTextDialog.h"
#include "CApplication.h"
#include "CConsoleWindow.h"
#include "panels.h"
#include <stddef.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qmap.h>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 4
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

static QMap<QString, QString> password_map;
static QString connections_path = QString(CONNECTIONS_PATH);

CMySQLServer::CMySQLServer(const QString & connection, CMessagePanel * msgpanel)
: QObject(0, "CMySQLServer"), p_connection_name(connection)
{
#ifdef DEBUG
  qDebug("CMySQLServer::CMySQLServer()");
#endif
  
  printconnectionname = true;
  refresh();
  p_mysql = new CMySQL();
  p_mysql->setConnectionName(connection);  
  sqlDebugPanel = myApp()->consoleWindow()->sqlDebugPanel();
  debug_connected = myApp()->enableSqlPanel();
  if (debug_connected)
    QObject::connect(p_mysql, SIGNAL(sqldebug(const QString &)), this, SLOT(SqlDebug(const QString &)));
  QObject::connect(p_mysql, SIGNAL(error(uint, const QString &)), this, SLOT(showError(uint, const QString &)));
  QObject::connect(p_mysql, SIGNAL(message(ushort, const QString &)), this, SLOT(showMessage(ushort, const QString &)));
  setMessagePanel(msgpanel);
}

void CMySQLServer::SqlDebug(const QString &s)
{
  QCustomEvent *event = new QCustomEvent(SQL_DEBUG);
  event->setData(new QString(s));
  QApplication::postEvent(this, event);
}

void CMySQLServer::customEvent(QCustomEvent *event)
{
  bool ok = false;
  QString *msg = (QString *) event->data();
  switch ((int) event->type())
  {    
  case SQL_DEBUG:
    sqlDebugPanel->SqlDebug(*msg);
    break;

  case INFORMATION:
    {
      if (messagepanel == 0)
        QMessageBox::information(0, tr("Information"), *msg);
      else
        ok = true;
    }
    break;

  case WARNING:
    {
      if (messagepanel == 0)
        QMessageBox::warning(0, tr("Warning"), *msg);
      else
        ok = true;
    }
    break;

  case CRITICAL:
    {
      if (messagepanel == 0)
        QMessageBox::critical(0, tr("Critical"), *msg);
      else
        ok = true;
    }
    break;

  default:
    {
      if (messagepanel == 0)
      {        
         QMessageBox *mb = new QMessageBox(tr("Message"), *msg, QMessageBox::NoIcon, QMessageBox::Ok, 0, 0, 0);
         mb->exec();
      }
      else
        ok = true;
    }
    break;
  }

  if (messagepanel && ok)
    messagepanel->message((int) event->type(), *msg);

  delete msg;
}

void CMySQLServer::enableSqlDebugMessages(bool b)
{
  if (myApp()->enableSqlPanel())
  {
    CSqlDebugPanel * sqlDebugPanel = myApp()->consoleWindow()->sqlDebugPanel();
    if (b)
    {
      if (!debug_connected)
        QObject::connect(p_mysql, SIGNAL(sqldebug(const QString &)), sqlDebugPanel, SLOT(SqlDebug(const QString &)));
      debug_connected = true;
    }
    else
    {
      p_mysql->QObject::disconnect(SIGNAL(sqldebug(const QString &)));
      debug_connected = false;
    }
  }
}

void CMySQLServer::showMessage(ushort type, const QString &m)
{
#ifdef DEBUG
  qDebug("CMySQLServer::showMessage()");
#endif

  QString caption;
  QString msg = QString::null;
  if (printconnectionname)
    msg = "[" + connectionName() + "] ";
  msg += m;

  QCustomEvent *event = new QCustomEvent(type);
  event->setData(new QString(m));
  QApplication::postEvent(this, event);
}

void CMySQLServer::showError(uint no, const QString &msg)
{
#ifdef DEBUG
  qDebug("CMySQLServer::showError()");
#endif

  QString errmsg = QString::null;
  if (printconnectionname)
    errmsg = "[" + connectionName() + "] ";
  errmsg += tr("ERROR") + " " + QString::number(no) + ": " + msg;

  showMessage(CRITICAL, errmsg);
}


void CMySQLServer::setMessagePanel(CMessagePanel *msgpanel)
{
#ifdef DEBUG
  qDebug("CMySQLServer::setMessagePanel()");
#endif

  messagepanel = msgpanel;
}

CMySQLServer::~CMySQLServer()
{
#ifdef DEBUG
  qDebug("CMySQLServer::~CMySQLServer()");
#endif
  
  delete p_mysql;
}

QString CMySQLServer::connectionsPath()
{
#ifdef DEBUG
  qDebug("static CMySQLServer::connectionsPath()");
#endif
  
  return connections_path;
}

/*
remove() deletes the connection name and if p_connection_name exists in password_map it's removed
*/
void CMySQLServer::remove()
{
#ifdef DEBUG
  qDebug("CMySQLServer::remove()");
#endif

  if (isConnected())
    disconnect();
  password_map.remove(connectionName());
  CConfig::remove(connectionName(), connections_path);
  emit deleted();
}

bool CMySQLServer::rename(const QString &new_name)
{
#ifdef DEBUG
  qDebug("CMySQLServer::refresh()");
#endif

  if (CConfig::exists(new_name, connections_path))
    return false;
  else
  {
    if (CConfig::rename(connectionName(), new_name, connections_path))
    {
      if (promptPassword())
        password_map.insert(new_name, p_password);
      setConnectionName(new_name);
      return true;
    }
    else
      return false;
  }
}

void CMySQLServer::refresh()
{
#ifdef DEBUG
  qDebug("CMySQLServer::refresh()");
#endif
  
  CConfig *cfg = new CConfig(p_connection_name, connections_path);
  p_user_name = cfg->readStringEntry("User");
  p_password = cfg->readStringEntry("Password");
  p_compress = strtobool(cfg->readStringEntry("Compress", "false"));
  p_reconnect = strtobool(cfg->readStringEntry("Reconnect", "true"));
  p_prompt_password = strtobool(cfg->readStringEntry("Prompt Password", "false"));
  p_blocking_queries = strtobool(cfg->readStringEntry("Blocking Queries", "false"));
  p_one_connection = strtobool(cfg->readStringEntry("One Connection"));
  p_socket_file = cfg->readStringEntry("Socket");
  p_host_name = cfg->readStringEntry("Host");
  p_port = cfg->readNumberEntry("Port");

  p_timeout = cfg->readNumberEntry("Timeout");
  p_select_limit = cfg->readNumberEntry("Limit Select Queries Number", 1000);
  p_net_buffer_length = cfg->readNumberEntry("Net Buffer Length", 16384);
  p_max_join_size = cfg->readNumberEntry("Max Join Size", 0);
  p_max_allowed_packet = cfg->readNumberEntry("Max Allowed Packet", 16*1024L*1024L);

  p_load_local_infile = strtobool(cfg->readStringEntry("Allow Local Infile", "false"));

  p_created = cfg->readStringEntry("Created");
  p_modified = cfg->readStringEntry("Modified");
  p_ssl = strtobool(cfg->readStringEntry("SSL", "false"));
  p_completion = strtobool(cfg->readStringEntry("Completion", "false"));
  p_field_separator = cfg->readStringEntry("Field Separate", ",");
  p_line_terminator = cfg->readStringEntry("Line Terminate", "\\r\\n");
  p_field_encloser = cfg->readStringEntry("Field Enclosed", "'");
  p_replace_empty = cfg->readStringEntry("Replace Empty", QString::null);
  p_retrieve_show_table_status = strtobool(cfg->readStringEntry("Retrieve Method", "true"));
  p_show_all_databases = strtobool(cfg->readStringEntry("Show All Databases", "false"));
  p_databases = QStringList::split("/", cfg->readStringEntry("Databases"));
  p_use_specific_databases = strtobool(cfg->readStringEntry("Use Specific Databases", "false"));
  p_disable_startup_connect = strtobool(cfg->readStringEntry("Disable Startup Connect", "false"));

  delete cfg;  
  emit statuschanged();
  emit refreshed();  
}

QString CMySQLServer::doEscape(const QString &str) const
{
#ifdef DEBUG
  qDebug("CMySQLServer::doEscape()");
#endif

  QString tmp = str;
  tmp = tmp.replace(QRegExp("\\\\n"), "\n");
  tmp = tmp.replace(QRegExp("\\\\r"), "\r");
  return tmp.replace(QRegExp("\\\\t"), "\t");
}

QString CMySQLServer::fieldSeparator(bool escape) const
{
#ifdef DEBUG
  qDebug("CMySQLServer::fieldSeparator()");
#endif
  
  return (!escape ? p_field_separator : doEscape(p_field_separator));
}

QString CMySQLServer::replaceEmpty(bool escape) const
{
#ifdef DEBUG
  qDebug("CMySQLServer::replaceEmpty()");
#endif
  
  return (!escape ? p_replace_empty : doEscape(p_replace_empty));                    
}

QString CMySQLServer::fieldEncloser(bool escape) const
{
#ifdef DEBUG
  qDebug("CMySQLServer::fieldEncloser()");
#endif
  
  return (!escape ? p_field_encloser : doEscape(p_field_encloser));
}

QString CMySQLServer::lineTerminator(bool escape) const
{
#ifdef DEBUG
  qDebug("CMySQLServer::lineTerminator()");
#endif
  
  return (!escape ? p_line_terminator : doEscape(p_line_terminator));  
}

void CMySQLServer::setConnectionName(const QString &s)
{
#ifdef DEBUG
  qDebug("CMySQLServer::setConnectionName()");
#endif
  p_mysql->setConnectionName(p_connection_name);
  password_map.remove(p_connection_name);
  p_connection_name = s;
  emit name_changed(p_connection_name);
}

bool CMySQLServer::connect()
{
#ifdef DEBUG
  qDebug("CMySQLServer::connect()");
#endif
    
  if (promptPassword())
  {
    QMap<QString, QString>::Iterator tmpPass = password_map.find(connectionName());
    if (password_map.empty() || tmpPass == password_map.end())
    {
      bool ok = false;
      p_password = CTextDialog::getText( tr( "Password" ),
        tr("Please enter the Password for") + ": " + userName() + "@" + hostName(),
        QLineEdit::Password, QString::null, &ok, 0);
      if (!ok)  //user didn't click Accept in the dialog     
        return false;     
      password_map.insert(connectionName(), p_password); //insert the password password_map
    }
    else
      p_password = tmpPass.data();  //password is already in password_map
  }

  p_mysql->setHostName(hostName());
  p_mysql->setUserName(userName());
  p_mysql->setPassword(password());
  p_mysql->setDatabaseName(databaseName());
  p_mysql->setPort(port());
  p_mysql->setUnixSocket(socketFile());
  p_mysql->setCompress(compress());
  p_mysql->setReconnect(reconnect());
  p_mysql->setSSL(ssl());
  p_mysql->setConnectTimeout(timeout());
  p_mysql->setSelectLimit(selectLimit());
  p_mysql->setNetBufferLength(netBufferLength());
  p_mysql->setMaxJoinSize(maxJoinSize());
  p_mysql->setMaxAllowedPacket(maxAllowedPacket());
  p_mysql->enableLoadLocalInfile(hasLocalInfile());
  p_mysql->setBlockingQueries(blockingQueries());

  if (!p_mysql->connect())
  { 
    if (promptPassword() && !password_map.empty())  //Need to remove the password from password_map
      password_map.remove(connectionName());    
    return false;
  }
  
  emit statuschanged();
  emit connected();
  return true;
}

void CMySQLServer::disconnect()
{
#ifdef DEBUG
  qDebug("CMySQLServer::disconnect()");
#endif
  
  if (p_mysql->isConnected())
  {
    p_mysql->disconnect();
    emit statuschanged();
    emit disconnected();
  }
}
