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
#include "CSqlEditorFont.h"
#include "CConfig.h"
#include "globals.h"
#include <stddef.h>
#include <qregexp.h>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 2
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

Q3IntDict<CSqlEditorFont> CSqlEditorFont::sqleditorstyles;

CSqlEditorFont::CSqlEditorFont(CConfig *cfg, const QString & name, highlight_type highlight, const QString & defaultfont, const QString & defaultcolor)
{
#ifdef DEBUG
  qDebug("CSqlEditorFont::CSqlEditorFont()");
#endif

  displayname = QObject::tr(name);
  realname = name;
  highlighttype = highlight;  
  update(&dFont, &dColor, defaultfont, defaultcolor);
  update(&Font, &Color, cfg->readStringEntry(realname + " Highlight", defaultfont),
         cfg->readStringEntry(realname + " Highlight Color", defaultcolor));
  if (sqleditorstyles.size() <= (sqleditorstyles.count() +1))
    sqleditorstyles.resize(sqleditorstyles.count() + 1);
  sqleditorstyles.insert((uint)highlight, this);
}


void CSqlEditorFont::setDefaultValues()
{
#ifdef DEBUG
  qDebug("CSqlEditorFont::setDefaultValuest()");
#endif

  m_font.setFamily(dFont.family());
  m_font.setBold(dFont.bold());
  m_font.setUnderline(dFont.underline());
  m_font.setItalic(dFont.italic());
  m_font.setPointSize(dFont.pointSize());  
  m_color = dColor;
}

void CSqlEditorFont::setDefaults()
{
#ifdef DEBUG
  qDebug("CSqlEditorFont::setDefaults()");
#endif

  m_font.setFamily(Font.family());
  m_font.setBold(Font.bold());
  m_font.setUnderline(Font.underline());
  m_font.setItalic(Font.italic());
  m_font.setPointSize(Font.pointSize());  
  m_color = Color;
}

void CSqlEditorFont::update(QFont *f, QColor *c, const QString &font, const QString &color)
{
#ifdef DEBUG
  qDebug("CSqlEditorFont::update(QFont *, QColor *, '%s', '%s')", debug_string(font), debug_string(color));
#endif

  f->fromString(font);
  QRegExp rx("^(\\d+),(\\d+),(\\d+)$");
  
  int r=0, g=0, b=0;
  if (rx.search(color) != -1)
  {
    r = rx.cap(1).toInt();
    g = rx.cap(2).toInt();
    b = rx.cap(3).toInt();
  }
  c->setRgb (r, g, b);
}

bool CSqlEditorFont::save(CConfig *cfg)
{
#ifdef DEBUG
  qDebug("CSqlEditorFont::save()");
#endif

  bool err;
  Font.setFamily(m_font.family());
  Font.setBold(m_font.bold());
  Font.setUnderline(m_font.underline());
  Font.setItalic(m_font.italic());
  Font.setPointSize(m_font.pointSize());
  Color = m_color;
  QString color = QString::number(Color.red()) + "," + QString::number(Color.green()) + "," + QString::number(Color.blue());
  err = cfg->writeEntry(realname + " Highlight", Font.toString());
  err &= cfg->writeEntry(realname + " Highlight Color", color);
  return err;
}
