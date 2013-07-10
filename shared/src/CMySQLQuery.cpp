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
#include "CMySQLQuery.h"
#include <qdatetime.h>
#include <qapplication.h>

#ifdef HAVE_MYSQLCC_CONFIG

#include "config.h"  //Used for DEBUG & DEBUG_LEVEL

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 2
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

#endif //HAVE_MYSQLCC_CONFIG

#define PROCESS_EVENT 35

CMySQLQuery::QueryThread::QueryThread(MYSQL *m, const char *q, ulong l)
: QThread(), m_mysql(m), qry((char *) q), len(l)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::QueryThread::QueryThread()");
#endif
  
  res = false;  
}

void CMySQLQuery::QueryThread::run()
{
#ifdef DEBUG
  qDebug("CMySQLQuery::QueryThread::run()");
#endif
  
  res = (mysql_real_query(m_mysql, qry, len) == 0);
}

CMySQLQuery::CMySQLQuery(CMySQL *m)
: QObject(0, "CMySQLQuery")
{
#ifdef DEBUG
  qDebug("CMySQLQuery::CMySQLQuery(CMySQL *)");
#endif
  
  init(m, false, m->hasEmitErrors(), m->hasEmitMessages());
}


CMySQLQuery::CMySQLQuery(CMySQL *m, bool auto_delete)
: QObject(0, "CMySQLQuery")
{
#ifdef DEBUG
  qDebug("CMySQLQuery::CMySQLQuery(CMySQL *, bool)");
#endif
  
  init(m, auto_delete,  m->hasEmitErrors(), m->hasEmitMessages());
}


CMySQLQuery::CMySQLQuery(CMySQL *m, bool auto_delete, bool emit_error, bool emit_messages)
:QObject(0, "CMySQLQuery")
{
#ifdef DEBUG
  qDebug("CMySQLQuery::CMySQLQuery(CMySQL *, bool, bool, bool)");
#endif

  init(m, auto_delete, emit_error, emit_messages);
}


CMySQLQuery::~CMySQLQuery()
{
#ifdef DEBUG
  qDebug("CMySQLQuery::~CMySQLQuery()");
#endif
  
  freeResult();
}

void CMySQLQuery::init(CMySQL *m, bool auto_delete, bool emit_error, bool emit_messages)
{
  m_mysql = m;
  autodelete = auto_delete;
  emiterror = emit_error;
  emitmessages = emit_messages;
  check_pending_event = 0;
  cancel_execution = false;
  num_fields = 0;
  mysql_res = NULL;
  mysql_fields = NULL;
  blocking_queries = m_mysql->blockingQueries();
  last_query = QString::null;
  returned_results = false;
}

bool CMySQLQuery::exec(const QString &qry)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::exec()");
#endif
  
  return execRealQuery(qry, qry.length());
}

bool CMySQLQuery::execStaticQuery(const QString &qry)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::execStaticQuery()");
#endif
  
  QString q = qry.stripWhiteSpace();
  return execRealStaticQuery(q, q.length());
}

bool CMySQLQuery::execRealStaticQuery(const char * qry, ulong len)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::execRealStaticQuery()");
#endif
  
  check_pending_event = 0;  
  cancel_execution = false;
  freeResult();
  returned_results = false;
  QString c = QString::null;
  if (!m_mysql->connectionName().isEmpty())
    c = "[" + m_mysql->connectionName() + "] ";
  emit m_mysql->sqldebug(m_mysql->codec()->fromUnicode(c + qry));
  if (!m_mysql->isConnected())
  {  
    if (emiterror)
      m_mysql->emitError();
    return false;	
  }  
  QTime tm;
  tm.start();

#ifdef QT_THREAD_SUPPORT
  QueryThread queryThread(m_mysql->mysql, qry, len);
  queryThread.start();
  queryThread.wait();
  if (!queryThread.getResult())  
  {
    if (emiterror)
      m_mysql->emitError();
    return false;
  }
#else
  if (mysql_real_query(m_mysql->mysql, qry, len) != 0)
  {
    if (emiterror)
      m_mysql->emitError();
    return false;
  }
#endif

  mysql_res = NULL;
  num_fields = 0;
  last_query = qry;
  
  if (emitmessages)
  {
    QString time_buff = " (" + QString::number((double)tm.elapsed() / 1000L, 'f', 2) + ") " + tr("sec");
    QString buff;
    if (mysql_affected_rows(m_mysql->mysql) == ~(ulong) 0)
      buff = tr("Query OK");
    else
    {
      buff = tr("Query OK,") + " ";
      buff += QString::number((ulong) mysql_affected_rows(m_mysql->mysql)) + " ";
      buff += mysql_affected_rows(m_mysql->mysql) == 1 ? tr("row") : tr("rows");
      buff += " " + tr("affected");
    }
    buff += time_buff;
    m_mysql->emitMessage(WARNING, buff);
  }
  return true;
}

bool CMySQLQuery::execRealQuery(const char * qry, ulong len)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::execRealQuery()");
#endif
  
  check_pending_event = 0;
  cancel_execution = false;
  freeResult();
  returned_results = false;
  QString c = QString::null;
  if (!m_mysql->connectionName().isEmpty())
    c = "[" + m_mysql->connectionName() + "] ";
  emit m_mysql->sqldebug(m_mysql->codec()->fromUnicode(c + qry));
  if (!m_mysql->isConnected())
  {
    if (emiterror)
      m_mysql->emitError();
    return false;
  }  
  
  QTime tm;
  tm.start();

#ifdef QT_THREAD_SUPPORT
  QueryThread queryThread(m_mysql->mysql, qry, len);
  queryThread.start();
  queryThread.wait();
  if (!queryThread.getResult())
  {    
    if (emiterror)
      m_mysql->emitError();
    return false;
  }
#else
  if (mysql_real_query(m_mysql->mysql, qry, len) != 0)
  {
    if (emiterror)
      m_mysql->emitError();
    return false;
  }
#endif

  if (!(mysql_res=mysql_store_result(m_mysql->mysql)))
  {
    mysql_res = NULL;
    if (mysql_error(m_mysql->mysql)[0])
    {
      if (emiterror)
        m_mysql->emitError();
      return false;
    }
  }
  last_query = qry;
  if (mysql_res)
  {
    mysql_fields = mysql_fetch_fields(mysql_res);
    num_fields = mysql_num_fields(mysql_res);
    returned_results = true;
  }
  else
  {
    mysql_fields = 0;
    num_fields = 0;
  }
 
  if (emitmessages)
  {
    QString time_buff = " (" + QString::number((double)tm.elapsed() / 1000L, 'f', 2) + ") " + tr("sec");
    QString buff;
    uint flag;
    if (mysql_res)
    {
      flag = INFORMATION;
      if (!mysql_num_rows(mysql_res))
        buff = tr("Empty set");
      else
      {
        buff = QString::number(numRows()) + " ";
        buff += numRows() == 1 ? tr("row") : tr("rows");
        buff += " " + tr("in set");
      }
    }
    else
    {
      flag = WARNING;
      if (mysql_affected_rows(m_mysql->mysql) == ~(ulong) 0)
        buff = tr("Query OK");
      else
      {
        buff = tr("Query OK,") + " ";
        buff += QString::number((ulong) mysql_affected_rows(m_mysql->mysql)) + " ";
        buff += mysql_affected_rows(m_mysql->mysql) == 1 ? tr("row") : tr("rows");
        buff += " " + tr("affected");
      }
    }
    buff += time_buff;
    m_mysql->emitMessage(flag, buff);
    if (mysql_info(m_mysql->mysql))
      m_mysql->emitMessage(INFORMATION, mysql_info(m_mysql->mysql));
  }

  return true;
}

char * CMySQLQuery::row(uint index)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::row()");
#endif
  
  return mysql_row[index];
}

void CMySQLQuery::freeResult()
{
#ifdef DEBUG
  qDebug("CMySQLQuery::freeResult()");
#endif
  
  check_pending_event = 0;
  if (!isResultNull())
  {
    
    num_fields = 0;
    mysql_free_result(mysql_res);
    mysql_res = NULL;
  }  
}

bool CMySQLQuery::isResultNull()
{
#ifdef DEBUG
  qDebug("CMySQLQuery::isResultNull()");
#endif
  
  return (mysql_res == NULL);
}

ulong CMySQLQuery::fieldLength(uint index)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::fieldLength()");
#endif
  
  return !isResultNull() ? *(mysql_fetch_lengths(mysql_res) + index) : 0;  
}

char * CMySQLQuery::fieldValue(ulong offset, uint index)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::fieldValue()");
#endif
  
  dataSeek(offset);
  return ((mysql_row=mysql_fetch_row(mysql_res)) != NULL) ? mysql_row[index] : 0;
}

void CMySQLQuery::dataSeek(ulong offset)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::dataSeek()");
#endif
  
  check_pending_event = 0;
  if (!isResultNull())
    mysql_data_seek(mysql_res, offset);
}

ulong CMySQLQuery::numRows()
{
#ifdef DEBUG
  qDebug("CMySQLQuery::numRows()");
#endif
  
  return (!isResultNull() ? mysql_num_rows(mysql_res) : 0);
}

void CMySQLQuery::cancel()
{
#ifdef DEBUG
  qDebug("CMySQLQuery::cancel()");
#endif
  
  cancel_execution = true;
}

bool CMySQLQuery::next(bool blocking)
{
#ifdef DEBUG
  qDebug("CMySQLQuery::next()");
#endif

  bool r = ((mysql_row=mysql_fetch_row(mysql_res)) != NULL);
  if (check_pending_event++ >= PROCESS_EVENT)
  {
    if (!blocking && !blocking_queries)
      if (qApp->hasPendingEvents())
        qApp->processEvents();

    if (cancel_execution)
      r = false;
    check_pending_event = 0;
  }

  if (!r && autodelete )
    freeResult();
  return r;
}

QString CMySQLQuery::strip_comments(const QString &s)
{  
#ifdef DEBUG
  qDebug("static CMySQLQuery::strip_comments(const QString &)");
#endif

  if (s.isEmpty())
    return s;
  QChar startquote;
  QString ret = QString::null;
  bool isquoted = false;
  bool c_comment = false;
  bool line_comment = false;
  QChar c;
  for (ulong i = 0; i < s.length(); i++)
  {
    c = s.at(i);
    if (!isquoted && !line_comment && !c_comment && (c == '\"' || c == '\''))
    {
      isquoted = true;
      startquote = c;
    }
    else
      if (isquoted && !line_comment && !c_comment && startquote == c)
        isquoted = (s.at(i - 1) == '\\');

    if (!isquoted)
    {
      if (!c_comment && line_comment && c == '\n')
        line_comment = false;
      else
        if (!c_comment && (i == 0 || s.at(i-1).isSpace()) && 
          (c == '#' || (c == "-" && s.at(i+1) == '-') || 
                       (c == ' ' && s.at(i+1) == '-' && s.at(i+2) == '-')))  //Line comments
          line_comment = true;

       if (!line_comment && c_comment &&  c == '*' && s.at(i+1) == '/')
       {
         c_comment = false;
         if (i == s.length() - 1)
           break;
         else
         {
           i++;
           continue;
         }
       }
       else
         if (!line_comment && !c_comment && c == '/' && s.at(i+1) == '*')
           c_comment = true;

      if (!line_comment && !c_comment)
        ret += c;
    }
    else
      ret += c;
  }

  return ret.isEmpty() ? s : ret.stripWhiteSpace();
}

QString CMySQLQuery::query_type(const QString &q)
{
#ifdef DEBUG
  qDebug("static CMySQLQuery::query_type(const QString &)");
#endif

  if (q.isEmpty())
    return QString::null;

  QString qry = strip_comments(q).simplifyWhiteSpace();
  if (qry.isEmpty())
    return QString::null;

  int p = qry.find(" ");
  return p != -1 ? qry.left(p).lower() : QString::null;
}
