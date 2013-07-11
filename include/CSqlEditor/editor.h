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
/*************************************************************************
** Original code Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** Modified by Jorge del Conde <jorge@mysql.com>
**************************************************************************/

#ifndef EDITOR_H
#define EDITOR_H

#include <stddef.h>
#include <qtextedit.h>

struct Config;
class ParenMatcher;
class EditorCompletion;
class EditorBrowser;
class QTextParagraph;

class  Editor : public QTextEdit
{
  Q_OBJECT
    
public:
  enum Selection {
    Error = 3,
      Step = 4
  };
  
  Editor(QWidget *parent, const char *name );
  ~Editor();
  virtual void load( const QString &fn );
  virtual void save( const QString &fn );
  QTextDocument *document() const { return QTextEdit::document(); }
  void placeCursor( const QPoint &p, QTextCursor *c ) { QTextEdit::placeCursor( p, c ); }
  void setDocument( QTextDocument *doc ) { QTextEdit::setDocument( doc ); }
  QTextCursor *textCursor() const { return QTextEdit::textCursor(); }
  void repaintChanged() { QTextEdit::repaintChanged(); }
  
  virtual EditorCompletion *completionManager() { return 0; }
  virtual EditorBrowser *browserManager() { return 0; }
  virtual void configChanged();
  
  void setErrorSelection( int line );
  void setStepSelection( int line );
  void clearStepSelection();
  
  virtual bool supportsErrors() const { return TRUE; }
  virtual bool supportsBreakPoints() const { return TRUE; }
  virtual void makeFunctionVisible( QTextParagraph * ) {}
  
  void drawCursor( bool b ) { QTextEdit::drawCursor( b ); }

public slots:
  void print();
  virtual void clear();

signals:
  void clearErrorMarker();
  void intervalChanged();
  
private slots:
  void cursorPosChanged( QTextCursor *c );
  void doChangeInterval();
  void commentSelection();
  void uncommentSelection();
  
protected:  
  ParenMatcher *parenMatcher;
  QString filename;
  bool hasError;
};

#endif
