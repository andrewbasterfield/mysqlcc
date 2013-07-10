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
#ifndef CMYSQL_H
#define CMYSQL_H

#ifdef WIN32
#include <winsock.h>
#endif

#include <mysql.h>
#include <mysqld_error.h>
#include <qvariant.h>
#include <qobject.h>
#include <qtextcodec.h>

class CMySQLQuery;

class CMySQL : public QObject
{
  Q_OBJECT
    
public:
  CMySQL();
  CMySQL(const QString & h, const QString &u, const QString &pass = QString::null, const QString &d = QString::null, uint p = 3306, const QString &u_s = QString::null);
  ~CMySQL();
  
  struct mysql_server_version
  {
    uint major;
    uint minor;
    int relnum;
    QString release;
  };
  
  
  void setConnectionName(const QString &s) { connection_name = s; }  //hack so that CMySQLServer::connectionName() is always available to CMySQLQuery()
  QString connectionName() const { return connection_name; }
  QString hostName() const { return mysql_host; }
  QString userName() const { return mysql_user; }
  QString password() const { return mysql_passwd; }
  QString databaseName() const { return mysql_db; }
  QString unixSocket() const { return mysql_unix_socket; }
  uint port() const { return mysql_port; }
  ulong timeout() const { return mysql_opt_timeout; }

  mysql_server_version version() const { return ver; }
  
  void setEmitErrors(bool b) { emiterror = b; }
  void setEmitMessages(bool b) { emitmessages = b; }
  
  bool hasEmitErrors() const { return emiterror; }
  bool hasEmitMessages() const { return emitmessages; }
  
  void emitError();
  void emitMessage(ushort type, const QString &msg);
  
  bool blockingQueries() { return blocking_queries; }
  void setBlockingQueries(bool b) { blocking_queries = b; }
  
  QString realDatabaseName();
  QString uptime();
  
  QTextCodec *codec() const;
  
  bool isConnected() const;
  
  void setHostName(const QString &s) { mysql_host = s; }
  void setUserName(const QString &s) { mysql_user = s; }
  void setPassword(const QString &s) { mysql_passwd = s; }
  void setDatabaseName(const QString &s) { mysql_db = s; }
  void setPort(const uint &p) { mysql_port = p; }
  void setUnixSocket(const QString &s) { mysql_unix_socket = s; }
  void setCompress(bool b);
  void setSSL(bool b);
  void setConnectTimeout(ulong t) { mysql_opt_timeout = t; }
  void setSelectLimit(ulong i) { mysql_opt_select_limit = i; }
  void setNetBufferLength(ulong i) { mysql_opt_net_buffer_length = i; }
  void setMaxJoinSize(ulong i) { mysql_opt_max_join_size = i; }
  void setMaxAllowedPacket(ulong i) { mysql_opt_max_allowed_packet = i; }
  void enableLoadLocalInfile(bool b) { mysql_opt_load_local_infile =  b; }

  bool connect();
  void disconnect(bool e = true);
  QString escape(const QString &s) const;
  QString quote(const QString &s) const;
  
  uint mysqlErrno() const;
  QString mysqlError() const;
  bool mysqlKill(ulong pid);  
  bool mysqlPing();
  bool mysqlShutdown();
  QString mysqlInfo() const;
  QString mysqlGetClientInfo() const;
  QString mysqlGetHostInfo() const;
  uint mysqlGetProtoInfo() const;
  QString mysqlGetServerInfo() const;
  QString mysqlStat();
  QString mysqlGetCharsetName() const;
  ulong mysqlRealEscapeString(const char *from, char *to, uint from_length);
  ulong mysqlAffectedRows() const;
  ulong mysqlInsertID() const;
  bool mysqlSelectDb(const QString & dbname);
  
  friend class CMySQLQuery;
  
signals:
  void connected();  //connected() is emited when a successful connection is made.
  void disconnected();  //disconnected() is emited when the connection is closed.
  void error(uint, const QString &);  //error() is emmited when an error occures.
  void message(ushort, const QString &);
  void sqldebug(const QString &);
  
private:
  MYSQL *mysql;
  bool isconnected;
  mysql_server_version ver;
  QTextCodec *textcodec;
  QString connection_name;

  QString mysql_host;
  QString mysql_user;
  QString mysql_passwd;
  QString mysql_db;  
  uint mysql_port;
  QString mysql_unix_socket;
  uint mysql_client_flags;
  bool mysql_client_ssl;
  ulong mysql_opt_timeout;
  ulong mysql_opt_select_limit;
  ulong mysql_opt_net_buffer_length;
  ulong mysql_opt_max_join_size;
  ulong mysql_opt_max_allowed_packet;
  bool mysql_opt_load_local_infile;
  bool mysql_opt_compress;
  bool emiterror;
  bool emitmessages;
  bool blocking_queries;    
  void initServerVersion();
  void init();
    
};

#endif
