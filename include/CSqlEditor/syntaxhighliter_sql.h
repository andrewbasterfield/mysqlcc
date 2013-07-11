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
#ifndef SQLSYNTAXHIGHLIGHTER_H
#define SQLSYNTAXHIGHLIGHTER_H

#include <stddef.h>
#include <qintdict.h>
#include <qmap.h>
#include <private/qrichtext_p.h>

class SqlCompletionEntry;

class SyntaxHighlighter_SQL : public QTextPreProcessor
{
public:
      enum CppIds {
	Comment = 1,
	Number,
	String,
	Type,
	Keyword,
	PreProcessor,
	Label
    };

  SyntaxHighlighter_SQL(QMap<QString, SqlCompletionEntry> *c);
  ~SyntaxHighlighter_SQL();
  void process( QTextDocument *doc, QTextParagraph *string, int start, bool invalidate = true); 
  QTextFormat *format( int id );
  void refresh();
  void refreshFormats();
  
private:
  QMap<int, QMap<QString, int> > *wordMap;
  void addFormat( int id, QTextFormat *f );
  void removeFormat( int id );
  QTextFormat *lastFormat;
  int lastFormatId;
  QIntDict<QTextFormat> formats; 
  QMap<QString, SqlCompletionEntry> *highlight_map;
};

#endif
