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
#include "syntaxhighliter_sql.h"
#include "CSqlEditorFont.h"
#include "sqlcompletion.h"
#include "paragdata.h"
#include <stddef.h>
#include <qstring.h>
#include <qdict.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qapplication.h>
#include <qregexp.h>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 5
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

SyntaxHighlighter_SQL::SyntaxHighlighter_SQL(QMap<QString, SqlCompletionEntry> *c)
: QTextPreProcessor(), lastFormat(0), lastFormatId(-1)
{
#ifdef DEBUG
  qDebug("SyntaxHighlighter_SQL::SyntaxHighlighter_SQL()");
#endif
  
  formats.setAutoDelete(true);
  refreshFormats();
  highlight_map = c;
  wordMap = new QMap<int, QMap<QString, int> >;
}

SyntaxHighlighter_SQL::~SyntaxHighlighter_SQL()
{
#ifdef DEBUG
  qDebug("SyntaxHighlighter_SQL::~SyntaxHighlighter_SQL()");
#endif
  
  delete wordMap;
}

void SyntaxHighlighter_SQL::refreshFormats()
{
#ifdef DEBUG
  qDebug("SyntaxHighlighter_SQL::refreshFormats()");
#endif
  
  bool empty = formats.isEmpty();
  
  QIntDictIterator<CSqlEditorFont> it(*CSqlEditorFont::sqlEditorStyles());
  for(; it.current(); ++it)
    if(it.current()->highlightType() != CSqlEditorFont::PARENTHESES_MATCHING)
      if (empty)
        addFormat(it.current()->highlightType(), new QTextFormat(it.current()->Font, it.current()->Color));
      else
      {
        formats[it.current()->highlightType()]->setFont(it.current()->Font);
        formats[it.current()->highlightType()]->setColor(it.current()->Color);
      }
}

void SyntaxHighlighter_SQL::refresh()
{
#ifdef DEBUG
  qDebug("SyntaxHighlighter_SQL::refresh()");
#endif
  
  wordMap->clear();
  int len;
  QMap<QString, SqlCompletionEntry>::Iterator it;
  for (it = highlight_map->begin(); it != highlight_map->end(); ++it)
  {
    len = it.data().word.length();
    if(!wordMap->contains(len))
      wordMap->insert(len, QMap<QString, int >());
    
    QMap<QString, int> &map = wordMap->operator[](len);
    map[it.data().word.lower()] = it.data().highlightType;
  }
}

void SyntaxHighlighter_SQL::process(QTextDocument *doc, QTextParagraph *string, int, bool invalidate)
{
#ifdef DEBUG
  qDebug("SyntaxHighlighter_SQL::process()");
#endif
  
  QTextFormat *formatStandard = format(CSqlEditorFont::STANDARD);
  QTextFormat *formatString = format(CSqlEditorFont::STRINGS);
  QTextFormat *formatNumber = format(CSqlEditorFont::NUMBERS);
  QTextFormat *formatComment = format(CSqlEditorFont::COMMENTS);
  QTextFormat *formatVariable = format(CSqlEditorFont::VARIABLES);
  QTextFormat *formatTablesFields = format(CSqlEditorFont::TABLES_AND_FIELDS);
  
  
  const int StateStandard = 0;
  const int StateCommentStart1 = 1;
  const int StateCCommentStart2 = 2;
  const int StateCComment = 3;
  const int StateCCommentEnd1 = 4;
  const int StateCCommentEnd2 = 5;
  const int StateStringStart = 6;
  const int StateString = 7;
  const int StateStringEnd = 8;
  const int StateString2Start = 9;
  const int StateString2 = 10;
  const int StateString2End = 11;
  const int StateHashComment = 12;
  const int StateEscStart = 13;
  const int StateEsc = 14;
  const int StateEscEnd = 15;  
  const int StateDashComment = 16;
  const int StateDashComment2 = 17;
  
  const int InputAlpha = 0;
  const int InputAsterix = 1;
  const int InputSlash = 2;
  const int InputParen = 3;
  const int InputSpace = 4;
  const int InputHash = 5;
  const int InputQuotation = 6;
  const int InputApostrophe = 7;
  const int InputSep = 8;
  const int InputEsc = 9;
  const int InputDash = 10;
  
  static uchar table[ 18 ][ 11 ] = {
    { StateStandard,     StateStandard,       StateCommentStart1, StateStandard,     StateStandard,     StateHashComment,  StateStringStart,  StateString2Start, StateStandard,     StateEscStart,     StateDashComment },  // StateStandard
    { StateStandard,     StateCCommentStart2, StateStandard,      StateStandard,     StateStandard,     StateHashComment,  StateStringStart,  StateString2Start, StateStandard,     StateEscStart,     StateDashComment },  // StateCommentStart1
    { StateCComment,     StateCCommentEnd1,   StateCComment,      StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment },     // StateCCommentStart2
    { StateCComment,     StateCCommentEnd1,   StateCComment,      StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment },     // StateCComment
    { StateCComment,     StateCCommentEnd1,   StateCCommentEnd2,  StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment,     StateCComment },     // StateCCommentEnd1
    { StateStandard,     StateStandard,       StateCommentStart1, StateStandard,     StateStandard,     StateHashComment,  StateStringStart,  StateString2Start, StateStandard,     StateEscStart,     StateDashComment },  // StateCCommentEnd2
    { StateString,       StateString,         StateString,        StateString,       StateString,       StateString,       StateStringEnd,    StateString,       StateString,       StateString,       StateDashComment2 }, // StateStringStart
    { StateString,       StateString,         StateString,        StateString,       StateString,       StateString,       StateStringEnd,    StateString,       StateString,       StateString,       StateString },       // StateString
    { StateStandard,     StateStandard,       StateCommentStart1, StateStandard,     StateStandard,     StateHashComment,  StateStringStart,  StateString2Start, StateStandard,     StateEscStart,     StateDashComment },  // StateStringEnd
    { StateString2,      StateString2,        StateString2,       StateString2,      StateString2,      StateString2,      StateString2,      StateString2End,   StateString2,      StateString2,      StateDashComment2 }, // StateString2Start
    { StateString2,      StateString2,        StateString2,       StateString2,      StateString2,      StateString2,      StateString2,      StateString2End,   StateString2,      StateString2,      StateString2 },      // StateString2
    { StateStandard,     StateStandard,       StateCommentStart1, StateStandard,     StateStandard,     StateHashComment,  StateStringStart,  StateString2Start, StateStandard,     StateEscStart,     StateDashComment },  // StateString2End
    { StateHashComment,  StateHashComment,    StateHashComment,   StateHashComment,  StateHashComment,  StateHashComment,  StateHashComment,  StateHashComment,  StateHashComment,  StateHashComment,  StateHashComment },  // StateHashComment
    { StateEsc,          StateEsc,            StateEsc,           StateEsc,          StateEsc,          StateHashComment,  StateEsc,          StateEsc,          StateEsc,          StateStandard,     StateDashComment },  // StateEscStart
    { StateEsc,          StateEsc,            StateEsc,           StateEsc,          StateEsc,          StateHashComment,  StateEsc,          StateEsc,          StateEsc,          StateStandard,     StateDashComment },  // StateEsc
    { StateStandard,     StateStandard,       StateCommentStart1, StateStandard,     StateStandard,     StateHashComment,  StateStringStart,  StateString2Start, StateStandard,     StateStandard,     StateDashComment },  // StateEscEnd
    { StateDashComment,  StateDashComment,    StateDashComment,   StateDashComment,  StateDashComment,  StateDashComment,  StateDashComment,  StateDashComment,  StateDashComment,  StateDashComment,  StateDashComment },  // StateDashComment ' -- '
    { StateDashComment2, StateDashComment2,   StateDashComment2,  StateDashComment2, StateDashComment2, StateDashComment2, StateDashComment2, StateDashComment2, StateDashComment2, StateDashComment2, StateDashComment2 }  // StateDashComment2 '--'
    
  };
  
  QString buffer;
  
  int state = StateStandard;
  if(string->prev())
  {
    if(string->prev()->endState() == -1)
      process(doc, string->prev(), 0, false);
    state = string->prev()->endState();
  }
  int input = 0;
  int i = 0;
  bool lastWasBackSlash = false;
  bool isDashComment = false;
  bool makeLastStandard = false;
  
  ParagData *paragData =(ParagData*)string->extraData();
  if(paragData)
    paragData->parenList.clear();
  else
    paragData = new ParagData;
  
  QString alphabeth = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  QString mathChars = "xXeE";
  QString numbers = "0123456789";
  QChar lastChar;
  for(;;)
  {
    QChar c = string->at(i)->c;   
    if(lastWasBackSlash)
      input = InputSep;
    else
    {
      switch(c)
      {
      case '*':
        input = InputAsterix;
        break;
        
      case '`':
        input = InputEsc;      
        break;
      case '/':
        input = InputSlash;
        break;
      case '(':
        input = InputParen;        
        if(state == StateStandard ||       
          state == StateHashComment ||
          state == StateCCommentEnd2 ||
          state == StateCCommentEnd1 ||
          state == StateString2End ||
          state == StateStringEnd)
          paragData->parenList << Paren(Paren::Open, c, i);
        break;
      case ')':
        input = InputParen;        
        if(state == StateStandard ||        
          state == StateHashComment ||
          state == StateCCommentEnd2 ||
          state == StateCCommentEnd1 ||
          state == StateString2End ||
          state == StateStringEnd)
          paragData->parenList << Paren(Paren::Closed, c, i);
        break;
      case '"':
        input = InputQuotation;
        break;        
      case '\'':
        input = InputApostrophe;
        break;
      case '#':
        input = InputHash;
        break;        
      case '-':
        input = InputAlpha;
        if(!isDashComment)        
          isDashComment = (lastChar == ' ' || i == 0);
        else
          if(lastChar == '-')
            input = InputDash;  
          break;        
      case ' ':
        input = InputSpace;
        break;
      default:        
        input =(c.isNumber() || c.isLetter() || c == '_' || c == '@' || c == '.') ? InputAlpha : InputSep;
        break;
      }
    }
    
    lastWasBackSlash = !lastWasBackSlash && c == '\\';
    
    if(input == InputAlpha)
      buffer += c;
    
    state = table[ state ][ input ];
    
    switch(state)
    {
    case StateStandard:
      {
        int len = buffer.length();
        string->setFormat(i, 1, formatStandard, false);
        
        if(makeLastStandard)
          string->setFormat(i - 1, 1, formatStandard, false);
        
        makeLastStandard = false;
        
        if(buffer.length() > 0 && input != InputAlpha)
        {
          QRegExp rx("^(\\d+(\\.)?(\\.\\d+)?)$");
          if(rx.search(buffer) != -1)         
            string->setFormat(i - buffer.length(), buffer.length(), formatNumber, false);
          else         
            if(buffer[0] == '@')
              string->setFormat(i - buffer.length(), buffer.length(), formatVariable, false);
            else
            {
              QMap<int, QMap<QString, int > >::Iterator it = wordMap->find(len);
              if(it != wordMap->end())
              {
                QMap<QString, int >::Iterator it2 =(*it).find(buffer.lower());
                if(it2 !=(*it).end())
                  string->setFormat(i - buffer.length(), buffer.length(), format((*it2)), false);
              }
            }
            buffer = QString::null;
        }
      }
      break;
      
    case StateCommentStart1:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = true;
      buffer = QString::null;
      break;
    case StateCCommentStart2:
      string->setFormat(i - 1, 2, formatComment, false);
      makeLastStandard = false;
      buffer = QString::null;
      break;
    case StateCComment:    
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatComment, false);
      buffer = QString::null;
      break;      
    case StateCCommentEnd1:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatComment, false);
      buffer = QString::null;
      break;
    case StateCCommentEnd2:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatComment, false);
      buffer = QString::null;
      break;
    case StateStringStart:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatStandard, false);
      buffer = QString::null;
      break;
    case StateString:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatString, false);
      buffer = QString::null;
      break;
    case StateStringEnd:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatStandard, false);
      buffer = QString::null;
      break;
    case StateString2Start:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatStandard, false);
      buffer = QString::null;
      break;
    case StateString2:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatString, false);
      buffer = QString::null;
      break;
    case StateString2End:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatStandard, false);
      buffer = QString::null;
      break;
      
    case StateHashComment:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatComment, false);
      buffer = QString::null;
      break;
      
    case StateDashComment:
      makeLastStandard = false;
      string->setFormat(i - 1, 1, formatComment, false);
      buffer = QString::null;
      break;
      
    case StateDashComment2:
      makeLastStandard = false;
      string->setFormat(i, 1, formatComment, false);
      buffer = QString::null;
      break;
      
    case StateEscStart:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatStandard, false);
      buffer = QString::null;      
      break;
      
    case StateEsc:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatTablesFields, false);
      buffer = QString::null;    
      break;
      
    case StateEscEnd:
      if(makeLastStandard)
        string->setFormat(i - 1, 1, formatStandard, false);
      makeLastStandard = false;
      string->setFormat(i, 1, formatStandard, false);
      buffer = QString::null;      
      break;
    }
    
    lastChar = c;
    i++;
    if(i >= string->length())
      break;
  }
  
  string->setExtraData(paragData);
  
  if(state == StateCComment || state == StateCCommentEnd1)
    string->setEndState(StateCComment);
  else
    if(state == StateString)
      string->setEndState(StateString);
    else
      if(state == StateString2)
        string->setEndState(StateString2);
      else      
        string->setEndState(StateStandard);
      
      string->setFirstPreProcess(false);
      
      if(invalidate && string->next() && !string->next()->firstPreProcess() && string->next()->endState() != -1)
      {
        QTextParagraph *p = string->next();
        while(p)
        {
          if(p->endState() == -1)
            return;
          p->setEndState(-1);
          p = p->next();
        }
      }
}

QTextFormat *SyntaxHighlighter_SQL::format(int id)
{
#ifdef DEBUG
  qDebug("SyntaxHighlighter_SQL::format(%d) - count: %d", id, formats.count());
#endif
  
  if(lastFormatId == id  && lastFormat)
    return lastFormat;
  
  QTextFormat *f = formats[ id ];
  lastFormat = f ? f : formats[ 0 ];
  lastFormatId = id;
  return lastFormat;
}

void SyntaxHighlighter_SQL::addFormat(int id, QTextFormat *f)
{
#ifdef DEBUG
  qDebug("SyntaxHighlighter_SQL::addFormat()");
#endif
  
  formats.insert(id, f);
}

void SyntaxHighlighter_SQL::removeFormat(int id)
{
#ifdef DEBUG
  qDebug("SyntaxHighlighter_SQL::removeFormat()");
#endif
  
  formats.remove(id);
}
