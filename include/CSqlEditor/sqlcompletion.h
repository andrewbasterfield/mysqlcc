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
#ifndef SQLCOMPLETION_H
#define SQLCOMPLETION_H

#include "completion.h"
#include "CSqlEditorFont.h"
#include <stddef.h>
#include <qguardedptr.h>

class SqlCompletionEntry
{
public:

  enum keyword_type
  {
    SQL = 1,
    DATE,
    MATH,
    STRING,
    GROUP,
    MISC,
    TABLE,
    FIELD,
    TYPE,
    NONE
  };

  SqlCompletionEntry()
  {
    word = QString::null;
    keywordType = NONE;
    parameters =  QString::null;
    highlightType = CSqlEditorFont::STANDARD;
  }

  SqlCompletionEntry(const QString &w, keyword_type k, CSqlEditorFont::highlight_type h, const QString &p = QString::null)
  {
    word = w;
    keywordType = k;
    parameters =  p;
    highlightType = h;
  }

  bool operator == (SqlCompletionEntry c) const
  {
    bool ret = true;
    if (&c != this)
    {
      ret = (c.word == word);
      ret &= (c.highlightType == highlightType);
      ret &= (c.keywordType == keywordType);
      ret &= (c.parameters == parameters);
    }
    return ret;
  }

  static const QString keywordTypeToString(keyword_type);
  QString word;
  CSqlEditorFont::highlight_type highlightType;
  keyword_type keywordType;
  QString parameters;
};

class SqlEditorCompletion : public EditorCompletion
{
  Q_OBJECT
    
public:
  SqlEditorCompletion( Editor *e );
  void addSqlCompletionEntry(const SqlCompletionEntry &c);
  QValueList<CompletionEntry> completionList( const QString &s, QTextDocument *doc ) const;
  void setContext( QObjectList *toplevels, QObject *this_ );
  void addCompletionEntry(const QString &, QTextDocument *, bool) {};
  bool isEmpty() { return sqlCompletionMap.isEmpty(); }
  QMap<QString, SqlCompletionEntry> *completionMap() { return &completion_map; }

  
public slots:
  void clear();
  
private:
  typedef QValueList<SqlCompletionEntry> SqlCompletionEntryList;
  QMap<QString, SqlCompletionEntry> completion_map;
  QGuardedPtr<QObject> ths;
  QMap<QChar, SqlCompletionEntryList> sqlCompletionMap;

};

#endif
