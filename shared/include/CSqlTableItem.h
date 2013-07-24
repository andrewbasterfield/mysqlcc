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
#ifndef CQUERYTABLEITEM_H
#define CQUERYTABLEITEM_H

#include <stddef.h>
#include <q3table.h>

class CMySQLQuery;

#define MAX_FIELD_DISPLAY 384

class CSqlTableItem : public Q3TableItem
{
public:
  CSqlTableItem (Q3Table * table, CMySQLQuery *q, ulong off_set, uint idx, EditType et=OnTyping);
  CSqlTableItem (Q3Table * table, CMySQLQuery *q, uint idx, EditType et=OnTyping);

  ~CSqlTableItem();
  bool isBinary() { return is_binary; }
  bool isNumber() { return is_number; }
  bool isNull() { return is_null; }
  bool isBlob() { return is_blob; }
  bool isBigValue() { return is_big_value; }
  bool isNewRecord() { return is_new_record; }
  bool isItemInserted() { return is_item_inserted; }
  bool isPresision() { return is_presision; }
  uint decimals() { return m_decimals; }
  bool isUnsigned() { return is_unsigned; }

  ulong offset() const { return m_offset; }
  uint index() const { return m_index; }
  ulong length() const { return field_length; }

  void setLength(ulong len) { field_length = len; }
  void setNull(bool b);
  void setValue(const char *, ulong len);
  void setItemInserted(bool b) { is_item_inserted = b; }
  char * value() const;

  CMySQLQuery *query() const { return qry; };

  virtual void setText(const QString &str);
  void setTextFromValue();
  virtual int alignment() const;
  QWidget *createEditor() const;

protected:  
  uint flags;
  virtual QString checkValue(const QString &str);  

private:
  void init();
  CMySQLQuery *qry;
  ulong m_offset;  
  uint m_index;
  ulong field_length;
  char *m_value;
  uint m_decimals;
  bool is_big_value;
  bool is_binary;
  bool is_null;
  bool is_number;
  bool is_blob;
  bool is_new_record;
  bool is_item_inserted;
  bool is_presision;
  bool is_unsigned;
};

#endif
