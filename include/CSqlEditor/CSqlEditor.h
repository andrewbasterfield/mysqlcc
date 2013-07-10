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
#ifndef CSQLEDITOR_H
#define CSQLEDITOR_H

#include "editor.h"

class SyntaxHighlighter_SQL;
class SqlEditorCompletion;
class CMySQLServer;

class  CSqlEditor : public Editor
{
  Q_OBJECT
    
public:
  CSqlEditor(QWidget *parent, CMySQLServer *m, const char *name);
  ~CSqlEditor();
  
  void setMySQL(CMySQLServer *m) { m_mysql = m; }
  CMySQLServer * mysql() const { return m_mysql; }
  virtual EditorCompletion *completionManager() { return (EditorCompletion *) completion; }
  bool supportsBreakPoints() const { return false; }
  void paste();
  void loadSyntaxFile();
  void refreshCompletionFields();
  
public slots:
  void save();
  void openFile();
  void refreshFonts();
  
private:
  SyntaxHighlighter_SQL *highlighter;
  void mouseReleaseEvent(QMouseEvent * e);
  QString contents() const;
  SqlEditorCompletion *completion;
  CMySQLServer *m_mysql;
};

#endif
