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
#include "CSqlEditor.h"
#include "globals.h"
#include "CApplication.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "CSqlEditorFont.h"
#include "syntaxhighliter_sql.h"
#include "sqlcompletion.h"
#include "parenmatcher.h"
#include "panels.h"
#include <stddef.h>
#include <q3filedialog.h>
#include <qsettings.h>
#include <q3popupmenu.h>
#include <qinputdialog.h>
#include <qclipboard.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <QMouseEvent>

CSqlEditor::CSqlEditor(QWidget *parent, CMySQLServer *m, const char *name)
: Editor(parent, name)
{
#ifdef DEBUG
  qDebug("CSqlEditor::CSqlEditor()");
#endif

  m_mysql = m;
  completion = 0;
  highlighter = 0;
  setUndoRedoEnabled(true);
  parenMatcher->setEnabled(myApp()->parenthesesMatching());
  if (myApp()->syntaxHighlight() && !myApp()->syntaxFile().isEmpty())
  {
    completion = new SqlEditorCompletion(this);
    highlighter = new SyntaxHighlighter_SQL(completion->completionMap());
    document()->setPreProcessor(highlighter);
    CSqlEditorFont *k = CSqlEditorFont::sqlEditorStyles()->find(CSqlEditorFont::PARENTHESES_MATCHING);
    if (k != NULL)
      document()->setSelectionColor( ParenMatcher::Match, k->Color);
    loadSyntaxFile();
  }
}

CSqlEditor::~CSqlEditor()
{
#ifdef DEBUG
  qDebug("CSqlEditor::~CSqlEditor()");
#endif

  if (completion != 0)
    delete completion;
}

void CSqlEditor::refreshFonts()
{
  if (myApp()->syntaxHighlight() && !myApp()->syntaxFile().isEmpty())
  {
    if (highlighter != 0)
      highlighter->refreshFormats();
    CSqlEditorFont *k = CSqlEditorFont::sqlEditorStyles()->find(CSqlEditorFont::PARENTHESES_MATCHING);
    if (k != NULL)
      document()->setSelectionColor( ParenMatcher::Match, k->Color);
  }
}

void CSqlEditor::refreshCompletionFields()
{
  if (!m_mysql->completion() || completion == 0)
    return;

  CMySQLQuery *qry = new CMySQLQuery(m_mysql->mysql());
  qry->setEmitMessages(false);
  if (qry->exec("SHOW TABLES"))
  {
    CMySQLQuery *qry2 = new CMySQLQuery(m_mysql->mysql());
    qry2->setEmitMessages(false);
    QString t;
    while (qry->next())
    {
      if (qry2->exec("SHOW FIELDS FROM " + m_mysql->mysql()->quote(qry->row(0))))
      {
        while (qry2->next())
        {
          SqlCompletionEntry f(qry2->row(0), SqlCompletionEntry::FIELD, CSqlEditorFont::TABLES_AND_FIELDS);
          completion->addSqlCompletionEntry(f);
          t.sprintf("%s.%s", qry->row(0), qry2->row(0));
          SqlCompletionEntry f2(t, SqlCompletionEntry::FIELD, CSqlEditorFont::TABLES_AND_FIELDS);
          completion->addSqlCompletionEntry(f2);
        }
      }
      SqlCompletionEntry entry(qry->row(0), SqlCompletionEntry::TABLE, CSqlEditorFont::TABLES_AND_FIELDS);
      completion->addSqlCompletionEntry(entry);
    }
    delete qry2;
  }
  delete qry;
  highlighter->refresh();
}

void CSqlEditor::loadSyntaxFile()
{
  if (completion == 0)
    return;

  QFile f_strm(myApp()->syntaxFile());
  if (f_strm.open(QIODevice::ReadOnly))
  {
    Q3TextStream t_strm(&f_strm);
    SqlCompletionEntry::keyword_type keyword_type = (SqlCompletionEntry::keyword_type) 0;
    CSqlEditorFont::highlight_type highlight_type = (CSqlEditorFont::highlight_type) 0;
    bool hasSection = false;
    QString keyword;
    QString str;
    QString parameters;
    QRegExp rx("^\\[(\\d+)\\s(\\d+)\\]$");
    while ( !t_strm.atEnd() )
    {
      str = t_strm.readLine().stripWhiteSpace();
      if (rx.search(str) != -1)
      {
        keyword_type = (SqlCompletionEntry::keyword_type) rx.cap(1).toUShort();
        highlight_type = (CSqlEditorFont::highlight_type) rx.cap(2).toUShort();
        hasSection = true;        
      }
      else
        if (hasSection && !str.isEmpty())
        {
          if (str.contains('-') == 1)
          {
            int pos = str.find('-');
            keyword = str.left(pos).stripWhiteSpace();
            parameters = str.mid(pos + 1).stripWhiteSpace();
          }
          else
          {
            keyword = str.stripWhiteSpace();
            parameters = "";
          }
          switch (keyword_type)
          {
            case SqlCompletionEntry::NONE:
            case SqlCompletionEntry::TYPE:
            case SqlCompletionEntry::FIELD:
            case SqlCompletionEntry::TABLE:
            case SqlCompletionEntry::SQL:
              break;

            default: parameters = " (" + parameters + ")";
              break;
          }
          SqlCompletionEntry entry(keyword, keyword_type, highlight_type, parameters);
          completion->addSqlCompletionEntry(entry);
        }
    }
    f_strm.close();
    highlighter->refresh();
  }
  else
    m_mysql->messagePanel()->critical(tr("Could not load Syntax File") + ": '" + myApp()->syntaxFile() + "'");    
}

void CSqlEditor::paste()
{
#ifdef DEBUG
  qDebug("CSqlEditor::paste()");
#endif

  Editor::paste();
  emit intervalChanged();
}

QString CSqlEditor::contents() const
{
#ifdef DEBUG
  qDebug("CSqlEditor::contents()");
#endif

  QString ret = tr("Connection") + ": " + mysql()->connectionName() + mysql()->lineTerminator(true);
  ret += tr("Host") + ": " + mysql()->hostName() + mysql()->lineTerminator(true);
  ret += tr("Saved") + ": " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + mysql()->lineTerminator(true);
  ret = myApp()->commentText(ret, mysql()->lineTerminator(true));
  ret += mysql()->lineTerminator(true) + text();
  return ret;
}

void CSqlEditor::save()
{
#ifdef DEBUG
  qDebug("CSqlEditor::save()");
#endif

  saveToFile(filename, "sql", tr("SQL Files (*.sql);;All Files(*.*)" ), contents(), m_mysql->messagePanel());
}

void CSqlEditor::openFile()
{
#ifdef DEBUG
  qDebug("CSqlEditor::openFile()");
#endif

  QString fn = Q3FileDialog::getOpenFileName( QString::null, tr("SQL Files") + " (*.sql);;" + tr("Text Files") + " (*.txt);;" + tr("All Files") + " (*.*)", this);
  if ( !fn.isEmpty() )  
    load(fn);  
}

void CSqlEditor::mouseReleaseEvent(QMouseEvent * e)
{
#ifdef DEBUG
  qDebug("CSqlEditor::mouseReleaseEvent()");
#endif

  Editor::mouseReleaseEvent(e);
#ifndef QT_NO_CLIPBOARD
  if ((e->button() == Qt::MidButton) && myApp()->linuxPaste())
    if (!QApplication::clipboard()->text().isEmpty())
      paste();  
#endif
}
