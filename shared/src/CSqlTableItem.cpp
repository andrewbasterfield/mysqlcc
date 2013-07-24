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
#include "CSqlTableItem.h"
#include "CMySQLQuery.h"
#include "CNullLineEdit.h"
#include "CMySQLQuery.h"  //for IS_PRESISION() & IS_UNSIGNED Macros

#ifdef HAVE_MYSQLCC_CONFIG

#include "globals.h"  //for debugging
#include "config.h"  //for debugging

#endif //HAVE_MYSQLCC_CONFIG

CSqlTableItem::CSqlTableItem(Q3Table * table, CMySQLQuery *q, ulong off_set, uint idx, EditType et)
: Q3TableItem(table, et, QString::null), qry(q), m_offset(off_set), m_index(idx)
{  
  init();
  is_item_inserted = true;
  is_null = !query()->row(index());
  is_new_record = false;
  field_length = query()->fieldLength(index());
  if (!is_null)
  {
    m_value = new char [field_length + 1];
    memcpy(m_value, query()->row(index()), field_length);
    *(m_value + field_length) = 0;
    setTextFromValue();
  }
  else
  {
    m_value = 0;
    setText("[NULL]");
  }
}

CSqlTableItem::CSqlTableItem(Q3Table * table, CMySQLQuery *q, uint idx, EditType et)
: Q3TableItem(table, et, QString::null), qry(q), m_index(idx)
{
  init();
  m_value = 0;
  is_item_inserted = false;
  is_new_record = true;
  is_null = true;
  field_length = 0;
}

void CSqlTableItem::init()
{
  setWordWrap(true);
  flags = query()->fields(index()).flags;
  is_number = flags & NUM_FLAG;  
  if (is_number)
  {
    is_presision = IS_PRESISION(query()->fields(index()).type);
    is_unsigned = IS_UNSIGNED(flags);
    m_decimals = query()->fields(index()).decimals;
  }
  else
  {
    is_presision = false;
    is_unsigned = false;
    m_decimals = 0;
  }

  is_binary = IS_BINARY(flags);
  is_blob = IS_BLOB(flags);
}

CSqlTableItem::~CSqlTableItem()
{
  if (m_value)
    delete [] m_value;
}

void CSqlTableItem::setNull(bool b)
{
  if (b)
    setValue(0, 0);
  else
    is_null = false;
}

void CSqlTableItem::setValue(const char *v, ulong len)
{
  if (m_value)
    delete [] m_value;

  if (v)
  {
    char *p = new char [len + 1];
    memcpy(p, v, len);
    *(p + len) = 0;
    m_value = p;
    is_null = false;
    setLength(len);
  }
  else
  {
    setLength(0);
    m_value = 0;
    is_null = true;
  }
}

void CSqlTableItem::setTextFromValue()
{
  if (isNull())
    setText("[NULL]");
  else
    if (length() <= MAX_FIELD_DISPLAY && !is_blob)
    {
      //if (length() != 0)
      Q3TableItem::setText(query()->mysql()->codec()->toUnicode(value()));
      is_big_value = false;
    }
    else
    {
      is_big_value = true;
      if (length() == 0)  //BLOB length is either too big, or a query such as SELECT REPEAT("str", 100000000) was executed.
        Q3TableItem::setText("");
      else
      {
        QString tmp(query()->mysql()->codec()->toUnicode(value()));
        if (tmp.length() > MAX_FIELD_DISPLAY)
          tmp = tmp.left(MAX_FIELD_DISPLAY-3) + "...";
        Q3TableItem::setText(tmp);
      }
    }
  table()->updateCell(row(), col());
}

QString CSqlTableItem::checkValue(const QString &str)
{
  if (str.length() <= MAX_FIELD_DISPLAY)
  {
    is_big_value = false;
    return str;
  }
  else
  {
    is_big_value = true;
    return str.left(MAX_FIELD_DISPLAY-3) + "...";
  }
}

void CSqlTableItem::setText(const QString &str)
{
  Q3TableItem::setText(checkValue(str));
}

char * CSqlTableItem::value() const
{
  return m_value ? m_value : 0;
}

int CSqlTableItem::alignment() const
{
  return is_number ? Qt::AlignRight : Qt::AlignLeft;
}

QWidget *CSqlTableItem::createEditor() const
{
#ifdef DEBUG
  qDebug("CSqlTableItem::createEditor()");
#endif

  CNullLineEdit *e = new CNullLineEdit(table()->viewport());
  e->setFrame(false);
  e->setText(text());
  return e;
}
