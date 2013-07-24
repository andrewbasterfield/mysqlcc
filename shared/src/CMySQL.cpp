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
#include "CMySQL.h"
#include "CMySQLQuery.h"
#include "shared.h"
#include <qregexp.h>
#include <qapplication.h>

#ifdef HAVE_MYSQLCC_CONFIG

#include "config.h"   //Used for DEBUG & DEBUG_LEVEL

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 4
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

#endif //HAVE_MYSQLCC_CONFIG

/*
CMySQL is the base class for handling mysql conectivity and administration.  It does not handle things having to
do with queries.
*/

CMySQL::CMySQL()
: QObject(0, "CMySQL")
{
#ifdef DEBUG
  qDebug("CMySQL::CMySQL()");
#endif
  
  init();
}

CMySQL::CMySQL(const QString & h, const QString &u, const QString &pass, const QString &d, uint p, const QString &u_s)
: QObject(0, "CMySQL"), mysql_host(h), mysql_user(u), mysql_passwd(pass), mysql_db(d), mysql_port(p), mysql_unix_socket(u_s)
{
#ifdef DEBUG
  qDebug("CMySQL::CMySQL()");
#endif
  
  init();
}

void CMySQL::init()
{
#ifdef DEBUG
  qDebug("CMySQL::init()");
#endif

  mysql = 0;
  textcodec = 0;
  mysql_client_flags = 0;
  mysql_client_ssl = false;
  mysql_opt_compress = false;
  mysql_opt_reconnect = true;
  mysql_opt_timeout = 0;
  mysql_opt_select_limit = 1000;
  mysql_opt_net_buffer_length = 16384;
  mysql_opt_max_join_size = 1000000L;
  mysql_opt_max_allowed_packet = 16*1024L*1024L;
  mysql_opt_load_local_infile = false;
  emiterror = true;
  emitmessages = true;
  blocking_queries = false;
  isconnected = false;
  initServerVersion();
}



CMySQL::~CMySQL()
{
#ifdef DEBUG
  qDebug("CMySQL::~CMySQL()");
#endif
  
  disconnect();
}


bool CMySQL::isConnected() const
{
  return isconnected && (mysql != 0);
}


/*
setCompress() handles the client/server compression protocol.
*/
void CMySQL::setCompress(bool b)
{
#ifdef DEBUG
  qDebug("CMySQL::setCompress()");
#endif
  
  if (isConnected())
    return;
  
  mysql_opt_compress = b;
}


/*
setSSL() handles the use of SSL (encrypted) protocol.
*/
void CMySQL::setSSL(bool b)
{
#ifdef DEBUG
  qDebug("CMySQL::setSSL()");
#endif
  
  if (isConnected())
    return;
  
  mysql_client_ssl = b;
}

/*
diconnect() closes a mysql connection if it has previously been connected.
*/
void CMySQL::disconnect(bool e)
{
#ifdef DEBUG
  qDebug("CMySQL::disconnect()");
#endif
  
  if (isconnected)
  {
    if (mysql != 0)
      mysql_close(mysql);
    mysql = 0;
    isconnected = false;
    initServerVersion();
    if (e)
      emit disconnected();
  }
}

/*
connect() creates a connection to a mysql server.
*/
bool CMySQL::connect()
{

#ifdef DEBUG
		qDebug("CMySQL::connect()");
#endif  

  char init_command[100];
  if (isConnected())
    disconnect();
  
  mysql = mysql_init(0);

  mysql_options(mysql, MYSQL_OPT_RECONNECT, &mysql_opt_reconnect);

  if (mysql_opt_compress)
  {
    mysql_options(mysql, MYSQL_OPT_COMPRESS,0);
    mysql_client_flags |= CLIENT_COMPRESS;
  }
  
  if (mysql_opt_timeout)
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char*) &mysql_opt_timeout);

  if (mysql_opt_load_local_infile)
    mysql_options(mysql, MYSQL_OPT_LOCAL_INFILE, mysql_opt_load_local_infile ? 0 : (char*) &mysql_opt_load_local_infile);

  if (mysql_opt_max_join_size > 0)
    sprintf(init_command, "SET SQL_MAX_JOIN_SIZE=%lu", mysql_opt_max_join_size);
  else
    sprintf(init_command, "SET SQL_BIG_SELECTS=1");

  mysql_options(mysql, MYSQL_INIT_COMMAND, init_command);

/*
  mysql->options.max_allowed_packet = mysql_opt_max_allowed_packet;
  net_buffer_length = mysql_opt_net_buffer_length;
*/
  
  if (mysql_client_ssl)
    mysql_client_flags |= CLIENT_SSL;

#ifndef WIN32
  if (!mysql_unix_socket.isEmpty())  //Set options to use a Unix Socket file instead of TCP/IP
  {
    mysql_port = 0;
    mysql_host = QString::null;
  }
  else
#endif  
    mysql_unix_socket = QString::null;
//    mysql_init(mysql);
  if (!mysql_real_connect(mysql, mysql_host, mysql_user, mysql_passwd, mysql_db, mysql_port, mysql_unix_socket, mysql_client_flags))
  {
    disconnect(false);
    if (emiterror)
      emitError();
  }
  else
  {
    isconnected = true;
    textcodec = QTextCodec::codecForName(mysqlGetCharsetName(), 2);
    initServerVersion();
    emit connected();
  }
  
  return isconnected;
}


void CMySQL::emitMessage(ushort type, const QString &msg)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::emitError()");
#endif
  
  emit message(type, codec()->fromUnicode(msg));
}


void CMySQL::emitError()
{
#ifdef DEBUG
  qDebug("CMySQLQuery::emitError()");
#endif

  emit error(mysqlErrno(), codec()->fromUnicode(mysqlError()));
}

/*
quote() adds single quotes at the begining and end of [s].  If the server is older than 3.22 then the
original string is returned (without quotes).  The same happens if the server is not connected.
*/
QString CMySQL::quote(const QString &s) const
{
#ifdef DEBUG
  qDebug("CMySQL::quote()");
#endif
  
  if (!isConnected())
    return s;
  return (version().major <= 3 && version().minor <= 22 ? s : "`" + s + "`");    
}

/*
mysqlAffectedRows() is a wrapper for mysql_affected_rows().  See the MySQL API documentation for further information.
*/
ulong CMySQL::mysqlAffectedRows() const
{
  if (!isConnected())
    return 0;
  else
    return mysql_affected_rows(mysql);
}

/*
mysqlInsertID() is a wrapper for mysql_insert_id().  See the MySQL API documentation for further information.
*/
ulong CMySQL::mysqlInsertID() const
{
  if (!isConnected())
    return 0;
  else
    return mysql_insert_id(mysql);
}


/*
mysqlSelectDb() is a wrapper for mysql_select_db().  See the MySQL API documentation for further information.
*/
bool CMySQL::mysqlSelectDb(const QString & dbname)
{
#ifdef DEBUG
  qDebug("CMySQL::quote()");
#endif
  
  if (!isConnected())
    return false;
  
  if (mysql_select_db(mysql, dbname) != 0)
  {
    if (emiterror)
      emitError();
    return false;
  }
  else
    setDatabaseName(dbname);
  return true;
}


/*
uptime() returns how long the mysql server has been running for since the last shutdown.
*/
QString CMySQL::uptime()
{
#ifdef DEBUG
  qDebug("CMySQL::uptime()");
#endif
  
  
  if (!isConnected() || (version().major <= 3 && version().minor <= 22))
    return QString::null;

  QString ret = QString::null;
  CMySQLQuery *qry = new CMySQLQuery(this);
  qry->setEmitMessages(false);
  
  if (qry->exec("SHOW STATUS LIKE 'Uptime'"))
    if (qry->next())
      ret = nice_time(QString(qry->row(1)).toULong());
  delete qry;
  return ret;
}


/*
getRealDatabaseName() returns the current MySQL Database or QString::null if there isn't one.
The difference between this function and databaseName() is that this one queries mysql for the
current database.
*/
QString CMySQL::realDatabaseName()
{
#ifdef DEBUG
  qDebug("CMySQL::realDatabaseName())");
#endif
  
  if (!isConnected())
    return QString::null;  
  
  QString ret = QString::null;
  CMySQLQuery *qry = new CMySQLQuery(this);
  qry->setEmitMessages(false);
  if (qry->exec("SELECT database()"))
    if (qry->next())
      ret = qry->row(0);
  delete qry;
  return ret;
}

/*
mysqlErrno() is a wrapper for mysql_errno().  See the MySQL API documentation for further information.
*/
uint CMySQL::mysqlErrno() const
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlErrno()");
#endif
  
  if (mysql != 0)
    return mysql_errno(mysql);
  else
    return 0;
}


/*
mysqlError() is a wrapper for mysql_error().  See the MySQL API documentation for further information.
*/
QString CMySQL::mysqlError() const
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlError()");
#endif
  
  if (mysql != 0)
    return mysql_error(mysql);
  else
    return tr("Unspecified MySQL Error");
}


/*
mysqlPing() is a wrapper for mysql_ping().  See the MySQL API documentation for further information.
*/
bool CMySQL::mysqlPing()
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlPing()");
#endif
  
  if (!isConnected())
    return false;
  
  if (mysql_ping(mysql) != 0)
  {
    if (emiterror)
      emitError();
    return false;
  }
  return true;
}


/*
mysqlErrno() is a wrapper for mysql_kill().  See the MySQL API documentation for further information.
*/
bool CMySQL::mysqlKill(ulong pid)
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlKill()");
#endif
  
  if (!isConnected())
    return false;
  
  if (mysql_kill(mysql, pid) != 0)
  {
    if (emiterror)
      emitError();      
    return false;
  }
  return true;
}


/*
mysqlShutdown() is a wrapper for mysql_shutdown().  See the MySQL API documentation for further information.
*/
bool CMySQL::mysqlShutdown()
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlShutdown()");
#endif
  
	if (!isConnected()){
		return false;
	}
	
#ifdef SHUTDOWN_DEFAULT
	if (mysql_shutdown(mysql,SHUTDOWN_DEFAULT) != 0)
	{
	if (emiterror)
		emitError();
		return false;
	}
#else
	if (mysql_shutdown(mysql,SHUTDOWN_DEFAULT) != 0)
	{
	if (emiterror)
		emitError();
		return false;
	}
#endif
	return true;
}

/*
mysqlInfo() is a wrapper for mysql_info().  See the MySQL API documentation for further information.
*/
QString CMySQL::mysqlInfo() const
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlInfo()");
#endif
  
  if (isConnected())
    return mysql_info(mysql);
  else
    return QString::null;
}

/*
mysqlGetClientInfo() is a wrapper for mysql_get_client_info().  See the MySQL API documentation for further information.
*/
QString CMySQL::mysqlGetClientInfo() const
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlGetClientInfo()");
#endif
  
  return mysql_get_client_info();
}


/*
mysqlGetHostInfo() is a wrapper for mysql_get_host_info().  See the MySQL API documentation for further information.
*/
QString CMySQL::mysqlGetHostInfo() const
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlGetHostInfo()");
#endif
  
  if (isConnected())
    return mysql_get_host_info(mysql);
  else
    return QString::null;
}

/*
mysqlGetProtoInfo() is a wrapper for mysql_get_proto_info().  See the MySQL API documentation for further information.
*/
uint CMySQL::mysqlGetProtoInfo() const
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlGetProtoInfo()");
#endif
  
  if (isConnected())
    return mysql_get_proto_info(mysql);
  else
    return 0;
}


/*
mysqlGetServerInfo() is a wrapper for mysql_get_server_info().  See the MySQL API documentation for further information.
*/
QString CMySQL::mysqlGetServerInfo() const
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlGetServerInfo()");
#endif
  
  if (isConnected())
    return mysql_get_server_info(mysql);
  else
    return QString::null;
}


/*
mysqlStat() is a wrapper for mysql_stat().  See the MySQL API documentation for further information.
*/
QString CMySQL::mysqlStat()
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlStat()");
#endif
  
  if (!isConnected())
    return QString::null;
  
  QString tmp = mysql_stat(mysql);
  
  if (tmp.isEmpty())
  {
    if (emiterror)
      emitError();
    return QString::null;
  }
  return tmp;
}

/*
mysqlGetCharsetName() is a wrapper for mysql_character_set_name().  See the MySQL API documentation for further information.
*/
QString CMySQL::mysqlGetCharsetName() const
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlGetCharsetName()");
#endif
  
  if (isConnected())
    return  mysql_character_set_name(mysql);
  else
    return  QString::null;
}


QTextCodec *CMySQL::codec() const
{
#ifdef DEBUG
  qDebug("CMySQL::codec()");
#endif
  
  if (textcodec)
    return textcodec;
  return QTextCodec::codecForLocale();
}


void CMySQL::initServerVersion()
{
#ifdef DEBUG
  qDebug("CMySQL::initServerVersion()");
#endif  

  QRegExp rx("^(\\d+)\\.(\\d+)\\.(.+)$");
  QString str = mysqlGetServerInfo();
  if (!isConnected() || str.isEmpty() || rx.search(str) == -1)
  {
    ver.major = 0;
    ver.minor = 0;
    ver.release = "0";
    ver.relnum = -1;
  }
  else
  {    
    ver.major = rx.cap(1).toInt();
    ver.minor = rx.cap(2).toInt();
    ver.release = rx.cap(3);
    QString t=QString::null;
    for (uint i = 0; i < ver.release.length(); i++)
      if (ver.release.at(i).isNumber())
        t += ver.release.at(i);
      else
        break;
    
    bool ok;
    ver.relnum = t.toInt(&ok);
    if (!ok)
      ver.relnum = -1;    
  }
}


/*
mysqlRealEscapeString() is a wrapper for mysql_real_escape_string().  See the MySQL API documentation for further information.
*/
ulong CMySQL::mysqlRealEscapeString(const char *from, char *to, uint from_length)
{
#ifdef DEBUG
  qDebug("CMySQL::mysqlRealEscapeString()");
#endif
  
  if (isConnected())
    return mysql_real_escape_string(mysql, to, from, from_length);
  else
    return 0;
}


/*
escape() is a method that escapes a string [s] using mysql_real_escape_string().
*/
QString CMySQL::escape(const QString &s) const
{
#ifdef DEBUG
  qDebug("CMySQL::escape()");
#endif
  
  if (s.isEmpty())
    return s;
  else
    if (char *tmp = new char[s.length()*2+1])
    {      
      mysql_real_escape_string(mysql, tmp, (const char *)codec()->fromUnicode(s), s.length());
      QString ret(codec()->toUnicode(tmp));
      delete [] tmp;
      return ret;
    }
    else
      return s;
}
