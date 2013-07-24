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
#ifndef CSQLEDITORFONT_H
#define CSQLEDITORFONT_H

#include <stddef.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstring.h>
#include <q3intdict.h>

class CConfig;

class CSqlEditorFont
{
public:

  enum highlight_type
  {
    STANDARD,
    KEYWORDS,
	  FIELD_TYPES,
	  TABLES_AND_FIELDS,
	  FUNCTIONS,
    VARIABLES,
    COMMENTS,
    CUSTOM,
    STRINGS,
    NUMBERS,
    PARENTHESES_MATCHING
  };

  CSqlEditorFont(CConfig *cfg, const QString & name, highlight_type highlight, const QString & defaultfont, const QString & defaultcolor);
  void setDefaultValues();
  void update(QFont *f, QColor *c, const QString &font, const QString &color);
  bool save(CConfig *cfg);
  void setDefaults();

  QString displayName() const { return displayname; }
  QString realName() const { return realname; }
  highlight_type highlightType() const { return highlighttype; }  

  QFont Font; //used for if the user cancels the dialog
  QColor Color;

  QFont m_font;  //store the font while working in the dialog
  QColor m_color;

  QFont dFont;  //default fonts
  QColor dColor;
  
  static Q3IntDict<CSqlEditorFont> * sqlEditorStyles() { return &sqleditorstyles; }  

private:
  QString realname;
  QString displayname;
  highlight_type highlighttype;
  static Q3IntDict<CSqlEditorFont> sqleditorstyles;

};

#endif
