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
#ifndef CMYSQLQUERY_H
#define CMYSQLQUERY_H

#include <qthread.h>
#include <qvariant.h>
#include "CMySQL.h"

#if !defined(max)
#define max(a, b)       ((a) > (b) ? (a) : (b))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#endif

#ifndef IS_MUL_KEY
#define IS_MUL_KEY(n)	((n) & MULTIPLE_KEY_FLAG)
#endif

#ifndef IS_UNI_KEY
#define IS_UNI_KEY(n)	((n) & UNIQUE_KEY_FLAG)
#endif

#ifndef IS_BINARY
#define IS_BINARY(n) ((n) & BINARY_FLAG)
#endif

#ifndef IS_PRESISION
#define IS_PRESISION(t)	((t) == FIELD_TYPE_DECIMAL || (t) == FIELD_TYPE_FLOAT || (t) == FIELD_TYPE_DOUBLE)
#endif

#ifndef IS_UNSIGNED
#define IS_UNSIGNED(n) ((n) & UNSIGNED_FLAG)
#endif

class CMySQLQuery : public QObject
{
  Q_OBJECT
  
public:
  CMySQLQuery(CMySQL *m);
  CMySQLQuery(CMySQL *m, bool auto_delete);
  CMySQLQuery(CMySQL *m, bool auto_delete, bool emit_error, bool emit_messages);
  ~CMySQLQuery();

  void setAutoDelete(const bool &b) { autodelete = b; }
  void setEmitErrors(const bool &b) { emiterror = b; }
  void setEmitMessages(const bool &b) { emitmessages = b; }
  
  void setBlockingQueries(const bool b) { blocking_queries = b; }
  bool blockingQueries() { return blocking_queries; }
  
  bool autoDelete() const { return autodelete; }
  bool hasEmitErrors() const { return emiterror; }
  bool hasEmitMessages() const { return emitmessages; }
  
  bool exec(const QString &qry);
  bool execStaticQuery(const QString &qry);
  bool execRealQuery(const char * qry, ulong len);
  bool execRealStaticQuery(const char * qry, ulong len);
  
  ulong fieldLength(uint index);
  char * fieldValue(ulong offset, uint index);
  void dataSeek(ulong offset);
  ulong numRows();
  uint numFields() { return num_fields; }

  QString lastQuery() const { return last_query; }
  
  void freeResult();
  bool isResultNull();
  bool returnedResults() const { return returned_results; }
  
  char * row(uint idx);
  
  MYSQL_FIELD fields(uint i) const { return mysql_fields[i]; }
  MYSQL_RES *result() const { return mysql_res; }

  CMySQL * mysql() const { return m_mysql; }
  
  bool next(bool blocking = false);
  void setRowValue(ulong offset, uint index, const char *value, ulong length);
  
  static QString strip_comments(const QString &s);
  static QString query_type(const QString &q);

public slots:
  void cancel();
  
private:

#ifdef QT_THREAD_SUPPORT
  class QueryThread : public QThread
  {
  public:  
    QueryThread (MYSQL *m, const char *q, ulong l);
    void run();
    bool getResult() { return res; }
    
  private:
    MYSQL *m_mysql;
    char *qry;
    ulong len;
    bool res;
  };
#endif

  CMySQL *m_mysql;
  MYSQL_RES *mysql_res;
  MYSQL_ROW mysql_row;
  MYSQL_FIELD *mysql_fields;
  uint num_fields;
  ushort check_pending_event;
  bool cancel_execution;
  bool blocking_queries;
  bool autodelete;
  bool emiterror;
  bool emitmessages;
  bool returned_results;
  QString last_query;
  void init(CMySQL *m, bool auto_delete, bool emit_error, bool emit_messages);
};

#endif
