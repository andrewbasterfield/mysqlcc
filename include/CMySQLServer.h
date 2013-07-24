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
#ifndef CMYSQLSERVER_H
#define CMYSQLSERVER_H

#include <stddef.h>
#include <qvariant.h>
#include <qobject.h>
//Added by qt3to4:
#include <QCustomEvent>
#include "CMySQL.h"

class CMessagePanel;
class CSqlDebugPanel;


class CMySQLServer : public QObject
{
  Q_OBJECT
    
public:
  CMySQLServer(const QString & connection, CMessagePanel * msgpanel=0);
  ~CMySQLServer();  
  
  void setConnectionName(const QString &s);
  void setUserName(const QString &s) { p_user_name = s; }
  void setPassword(const QString &s) { p_password = s; }
  void setSocketFile(const QString &s) { p_socket_file = s; }
  void setHostName(const QString &s) { p_host_name = s; }
  void setCreated(const QString &s) { p_created = s; }
  void setModified(const QString &s) { p_modified = s; }
  void setFieldSeparator(const QString &s) { p_field_separator = s; }
  void setFieldTerminator(const QString &s) { p_line_terminator = s; }
  void setFieldEncloser(const QString &s) { p_field_encloser = s; }
  void setReplaceEmpty(const QString &s) { p_replace_empty = s; }
  void setCompress(bool b) { p_compress = b; }
  void setReconnect(bool b) { p_reconnect = b; }
  void setPromptPassword(bool b) { p_prompt_password = b; }
  void setBlockingQueries(bool b) { p_blocking_queries = b; }
  void setOneConnection(bool b) { p_one_connection = b; }
  void setSSL(bool b) { p_ssl = b; }
  void setCompletion(bool b) { p_completion = b; }
  void setRetrieveShowTableStatus(bool b) { p_retrieve_show_table_status = b; }
  void setPort(uint i) { p_port = i; }
  void setTimeout(ulong i) { p_timeout = i; }

  void setSelectLimit(ulong i) { p_select_limit = i; }
  void setNetBufferLength(ulong i) { p_net_buffer_length = i; }
  void setMaxJoinSize(ulong i) { p_max_join_size = i; }
  void setMaxAllowedPacket(ulong i) { p_max_allowed_packet = i; }

  void setMessagePanel(CMessagePanel *msgpanel = 0);
  void setDatabaseName(const QString &s) { p_database_name = s; }
  void setShowAllDatabases(bool b) { p_show_all_databases = b; }
  void setPrintConnectionName(bool b) { printconnectionname = b; }
  
  QString connectionName() const { return p_connection_name; }  
  QString userName() const { return p_user_name; }
  QString password() const { return p_password; }
  QString socketFile() const { return p_socket_file; }
  QString hostName() const { return p_host_name; }
  QString created() const { return p_created; }
  QString modified() const { return p_modified; }
  QString databaseName() const { return p_database_name; }  
  QStringList databases() const { return p_databases; }
  CMessagePanel * messagePanel() { return messagepanel; }
  
  QString fieldSeparator(bool escape = false) const;
  QString fieldEncloser(bool escape = false) const;
  QString lineTerminator(bool escape = false) const;
  QString replaceEmpty(bool escape = false) const;
  
  bool isConnected() const { return mysql()->isConnected(); }
  bool compress() const { return p_compress; }
  bool reconnect() const { return p_reconnect; }
  bool promptPassword() const { return p_prompt_password; }
  bool blockingQueries() const { return p_blocking_queries; }
  bool oneConnection() const { return p_one_connection; }
  bool ssl() const { return p_ssl; }
  bool completion() const { return p_completion; }
  bool showAllDatabases() const { return p_show_all_databases; }
  bool retrieveShowTableStatus() const { return p_retrieve_show_table_status; }
  bool hasDatabases() { return p_use_specific_databases; }
  bool hasLocalInfile() const { return p_load_local_infile; }
  bool disableStartupConnect() const { return p_disable_startup_connect; }

  uint port() const { return p_port; }
  ulong timeout() const { return p_timeout; }
  ulong selectLimit() const { return p_select_limit; }
  ulong netBufferLength() const { return p_net_buffer_length; }
  ulong maxJoinSize() const { return p_max_join_size; }
  ulong maxAllowedPacket() const { return p_max_allowed_packet; }
  
  bool connect();
  void disconnect();
  bool rename(const QString &);
  CMySQL * mysql() const { return p_mysql; }
    
  static QString connectionsPath();
  void customEvent(QCustomEvent *event);
  
  
public slots:
  void SqlDebug(const QString &s);
  void refresh();
  void remove();
  void showMessage(ushort, const QString &);
  void showError(uint, const QString &);
  void enableSqlDebugMessages(bool);
  
signals:
  void refreshed();
  void statuschanged();
  void connected();
  void disconnected();
  void name_changed(const QString &);
  void deleted();
  
private:
  QString p_connection_name;
  QString p_user_name;
  QString p_password;
  QString p_socket_file;
  QString p_host_name;
  QString p_created;
  QString p_modified;
  QString p_field_separator;
  QString p_line_terminator;
  QString p_field_encloser;
  QString p_replace_empty;
  QString p_database_name;
  QStringList p_databases;
  bool p_use_specific_databases;
  bool p_compress;
  bool p_reconnect;
  bool p_show_all_databases;
  bool p_prompt_password;
  bool p_blocking_queries;
  bool p_one_connection;
  bool p_ssl;
  bool p_completion;
  bool p_retrieve_show_table_status;
  bool printconnectionname;
  bool debug_connected;
  bool p_load_local_infile;
  bool p_disable_startup_connect;
  int p_port;
  ulong p_timeout;
  ulong p_select_limit;
  ulong p_net_buffer_length;
  ulong p_max_join_size;
  ulong p_max_allowed_packet;

  CMessagePanel *messagepanel;
  CSqlDebugPanel *sqlDebugPanel;
  
  CMySQL *p_mysql;
  
  QString doEscape(const QString &str) const;
};

#endif
