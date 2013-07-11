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
#include "sqlcompletion.h"
#include "editor.h"
#include "config.h"
#include <stddef.h>
#include <qobject.h>
#include <qmetaobject.h>
#include <qobjectlist.h>
#include <qregexp.h>

#include <private/qrichtext_p.h>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 4
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

const QString SqlCompletionEntry::keywordTypeToString(keyword_type t)
{
#ifdef DEBUG
  qDebug("SqlCompletionEntry::keywordTypeToString()");
#endif

  switch (t)
  {
    case SQL: return "SQL";
      break;
    case DATE: return "Date";
      break;
    case MATH: return "Math";
      break;
    case STRING: return "String";
      break;
    case GROUP: return "Group";
      break;
    case MISC: return "Misc";
      break;
    case TABLE: return "Table";
      break;
    case FIELD: return "Field";
      break;
    case TYPE: return "Type";
      break;
    default: return "None";
      break;    
  }
}

SqlEditorCompletion::SqlEditorCompletion( Editor *e )
    : EditorCompletion( e )
{
#ifdef DEBUG
  qDebug("SqlEditorCompletion::SqlEditorCompletion()");
#endif

  setCaseSensitive(false);
}


void SqlEditorCompletion::addSqlCompletionEntry(const SqlCompletionEntry &c)
{
#ifdef DEBUG
  qDebug("EditorCompletion::addSqlCompletionEntry()");
#endif

  QChar key = c.word.lower()[ 0 ];
  QMap<QChar, SqlCompletionEntryList>::Iterator it = sqlCompletionMap.find(key);

  if ( it == sqlCompletionMap.end() )
  {
    SqlCompletionEntryList x;
    SqlCompletionEntryList::Iterator it2 = x.append(c);
    completion_map.insert((*it2).word, *it2);
    sqlCompletionMap.insert(key, x);
  }
  else
  {
    SqlCompletionEntryList::Iterator it2 = (*it).append(c);
    completion_map.insert((*it2).word, *it2);
  }
}

void SqlEditorCompletion::clear()
{
#ifdef DEBUG
  qDebug("SqlEditorCompletion::clear()");
#endif

  if (!completion_map.empty())
    completion_map.clear();

  if (!isEmpty())  
    sqlCompletionMap.clear();
}

QValueList<CompletionEntry> SqlEditorCompletion::completionList( const QString &s, QTextDocument *) const
{
#ifdef DEBUG
  qDebug("SqlEditorCompletion::completionList()");
#endif
  
  QChar key = s.lower()[ 0 ];

  QMap<QChar, SqlCompletionEntryList>::ConstIterator it = sqlCompletionMap.find( key );
  if ( it == sqlCompletionMap.end() )
    return QValueList<CompletionEntry>();
  SqlCompletionEntryList::ConstIterator it2 = (*it).begin();
  QValueList<CompletionEntry> lst;
  int len = s.length();
  for ( ; it2 != (*it).end(); ++it2 )
  {
    SqlCompletionEntry x = *it2;
    CompletionEntry c;
    c.type = SqlCompletionEntry::keywordTypeToString(x.keywordType);
    c.postfix2 = x.parameters;    
    c.text = x.word;
    c.postfix = "";
    c.prefix = "";
    if ((int)x.word.length() > len && x.word.left(len).lower() == s.lower() && lst.find( c ) == lst.end())
      lst << c;
  }
  return lst;
}


void SqlEditorCompletion::setContext( QObjectList *, QObject *this_ )
{
#ifdef DEBUG
  qDebug("SqlEditorCompletion::setContext()");
#endif

    ths = this_;
}
