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
#include "CSqlTable.h"
#include "CMySQLServer.h"
#include "CMySQL.h"
#include "CMySQLQuery.h"
#include "CTextFieldEditor.h"
#include "CFieldEditorWindow.h"
#include "CFieldEditorWidget.h"
#include "CImageViewer.h"
#include "CSqlTableItem.h"
#include "config.h"
#include "globals.h"
#include "panels.h"
#include "CNullLineEdit.h"
#include <qfile.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qfiledialog.h>
#include <stdlib.h>
#include <math.h>
#include <qtextcodec.h>
#include <qcstring.h>

CSqlTable::CSqlTable(QWidget * parent, CMySQLQuery *q, CMySQLServer *m, const char * name)
: CQueryTable(parent, q, m, name)
{
#ifdef DEBUG
  qDebug("CSqlTable::CSqlTable()");
#endif

  if (!name)
    setName("CSqlTable");
  enableProcessEvents(true);
  
#ifndef QT_THREAD_SUPPORT
  enableProcessEvents(true);
#else
  enableProcessEvents(false);
#endif

  setKeepColumnWidth(true);
  add_to_history = true;
  display_messages = true;
  database_name = QString::null;
  table_name = QString::null;
  messagePanel = m->messagePanel();
  historyPanel = 0;
  connect(this, SIGNAL(currentChanged(int, int)), this, SLOT(currentChanged(int, int)));
  setFocusStyle(QTable::FollowStyle);
  inserting_row = -1;
  force_insert = false;
}

QWidget *CSqlTable::createEditor(int row, int col, bool initFromCell) const
{
#ifdef DEBUG
  qDebug("CSqlTable::createEditor(int, int, bool) const");
#endif

  if (isReadOnly() || isRowReadOnly(row) || isColumnReadOnly(col))
	  return 0;

  CNullLineEdit *e = 0;

  CSqlTableItem *i = (CSqlTableItem *) item(row, col);
  if (initFromCell || i && !i->isReplaceable())
  {
	  if (i)
    {
	    e = (CNullLineEdit *) i->createEditor();
      e->setMaxLength(maxColumnLength(col));
      if (i->isNull())
        e->setNull(true);
	    if (!e || i->editType() == QTableItem::Never)
		    return 0;
    }
  }

 if (!e)
 {
	 e = new CNullLineEdit(viewport(), "CNullLineEdit");
   if (i->isNull())
     e->setNull(true);
   e->setMaxLength(maxColumnLength(col));
	 e->setFrame(false);
 }

 return e;
}

QWidget *CSqlTable::beginEdit(int row, int col, bool replace)
{
#ifdef DEBUG
  qDebug("CSqlTable::beginEdit(int, int, bool)");
#endif

  if (isReadOnly() || isRowReadOnly(row) || isColumnReadOnly(col))
	  return 0;
  CSqlTableItem *itm = (CSqlTableItem *) item(row, col);
  if (itm && !itm->isEnabled())
	  return 0;
  if (itm && cellWidget(itm->row(), itm->col()))
	  return 0;
  ensureCellVisible(row, col);
  CNullLineEdit *e = (CNullLineEdit *) createEditor(row, col, !replace);
  if (!e)
	  return 0;

  setCellWidget(row, col, e);
  e->setMaxLength(maxColumnLength(col));
  e->setActiveWindow();
  e->setFocus();
  updateCell(row, col);
  return e;
}

void CSqlTable::endEdit(int row, int col, bool accept, bool)
{
#ifdef DEBUG
  qDebug("CSqlTable::endEdit(int, int, bool, bool)");
#endif

  CNullLineEdit *editor = (CNullLineEdit *) cellWidget(row, col);  
  if (!editor)
	  return;
  editor->setMaxLength(maxColumnLength(col));

  if (!accept)
  {
	  if (row == currEditRow() && col == currEditCol())
	    setEditMode(NotEditing, -1, -1);
	  clearCellWidget(row, col);
	  updateCell(row, col);
	  viewport()->setFocus();
	  updateCell(row, col);
	  return;
  }

  CSqlTableItem *i = (CSqlTableItem *) item(row, col);
  
  bool edit_ok = false;
  QString txt = text(row, col);
  bool text_different = false;

  if (txt != editor->text() && !(i->isNull() || editor->isNull()) )
  {
    if (!(txt.isEmpty() && editor->text().isEmpty()))
      text_different = true;
  }
    
  if (text_different || i->isNull() != editor->isNull())
  {
    if (edit_ok = doUpdateRecord(editor, i))
      i->setContentFromEditor(editor);
  }

  if (!edit_ok)
  {
    if (row == currEditRow() && col == currEditCol())
	    setEditMode(NotEditing, -1, -1);
	  clearCellWidget(row, col);
	  updateCell(row, col);
	  viewport()->setFocus();
	  updateCell(row, col);
	  return;
  }

  if (row == currEditRow() && col == currEditCol())
	  setEditMode(NotEditing, -1, -1 );

  viewport()->setFocus();
  updateCell(row, col);

  if (i->isNewRecord() && !i->isItemInserted())
    doInsertRecord(row);

  clearCellWidget(row, col);
}

bool CSqlTable::exec(const char *sql, ulong length)
{
#ifdef DEBUG
  qDebug("CSqlTable::exec()");
#endif

  CMySQLQuery *qry = new CMySQLQuery(mysql()->mysql());
  qry->setEmitMessages(display_messages);
  bool b = qry->execRealStaticQuery(sql, length);
  if (add_to_history)
    historyPanel->History(mysql()->mysql()->codec()->toUnicode(sql));
  delete qry;
  return b;
}

bool CSqlTable::doUpdateRecord(CNullLineEdit *editor, CSqlTableItem *field)
{
#ifdef DEBUG
  qDebug("CSqlTable::doUpdateRecord()");
#endif

  if (isBlocked())
    return false;  

  setBlocked(true);
  bool b = false;
  if (!field->isNewRecord() || (field->isNewRecord() && field->isItemInserted()))
  {
    ulong query_length;
    char *sql;
    if(editor->isNull())
      sql = getUpdateQuery(0, 0, field, &query_length);
    else
      sql = getUpdateQuery((const char *)(mysql()->mysql()->codec()->fromUnicode(editor->text())), qstrlen(editor->text().local8Bit()), field, &query_length);
//      sql = getUpdateQuery((const char *)(mysql()->mysql()->codec()->fromUnicode(editor->text())), editor->text().length(), field, &query_length);
    b = exec(sql, query_length);
    delete [] sql;
  }
  else  //Item hasn't been inserted into the database
    b = true;

  if (b)
  {
    if (editor->isNull())
      field->setNull(true);
    else
      field->setValue((const char *)(mysql()->mysql()->codec()->fromUnicode(editor->text())), editor->text().length());
  }
  setBlocked(false);
  
  return b;
}


void CSqlTable::doInsertRecord(int row)
{
#ifdef DEBUG
  qDebug("CSqlTable::doInsertRecord()");
#endif

  if (isBlocked())
    return;  

  setBlocked(true);
  if ((inserting_row != -1 && row != inserting_row) || force_insert)
  {
    ulong query_length;
    char *sql = getInsertQuery(inserting_row, &query_length);
    bool b = exec(sql, query_length);
    delete [] sql;
    if (b)
    {
      CSqlTableItem *field;
      for (uint i = 0; i < query()->numFields(); i++)
      {
        field = (CSqlTableItem *) item(inserting_row, i);
        field->setItemInserted(true);
        if (default_fields_map[i].auto_increment)
        {
          ulong auto_inc = mysql()->mysql()->mysqlInsertID();
          if (auto_inc != 0)
          {
            QString a = QString::number(auto_inc);
            field->setValue(a, a.length());
            field->setTextFromValue();
          }
        }
      }
      inserting_row = -1;
      force_insert = false;
    }
  }
  setBlocked(false);
}

void CSqlTable::insertRecord(int r)
{
#ifdef DEBUG
  qDebug("CSqlTable::insertRecord()");
#endif

  if (!isVisible() || isReadOnly())
    return;

  if (isBlocked())
    return;  

  setBlocked(true);

  if (inserting_row != -1)
  {
    removeRow(inserting_row);
    inserting_row = -1;
  }
  int row = r == -1 ? currentRow() : r;
  if (row < 0)
    row = 0;

  insertRows(row);

  CDefaultFieldSpecs field_info;
  for (uint i = 0; i < query()->numFields(); i++)
  {
    CSqlTableItem *item = new CSqlTableItem(this, query(), i);
    if (!default_fields_map.empty())
    {
      field_info = default_fields_map[i];
      if ((field_info.is_null || field_info.auto_increment) && field_info.default_value.isEmpty())
        item->setValue(0, 0);
      else
        item->setValue(field_info.default_value, field_info.default_value.length());
    }
    else
      item->setValue(0, 0);
    item->setTextFromValue();
    setItem(row, i, item);
  }  
  ensureCellVisible(row, 0);
  setCurrentCell(row, 0);  
  inserting_row = row;
  emit canDelete(true);
  force_insert = (numRows() == 1);
  setBlocked(false);
}

void CSqlTable::deleteRecord()
{
#ifdef DEBUG
  qDebug("CSqlTable::deleteRecord()");
#endif

  if (!isVisible() || isReadOnly() || numRows() < 1)
  {
    emit canDelete(false);
    return;
  }

  if (isBlocked())
    return;  

  if (myApp()->confirmCritical())
    if ((QMessageBox::information(0, tr("Confirm Delete"), tr("Are you sure you want to Delete the selected records ?"),
      QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes))
      return;

  setBlocked(true);
  QMemArray<int> delete_rows(0);
  CSqlTableItem *field;
  int cnt = 0;
  bool ok;
  for (int i = 0; i < numRows(); i++)
  {
    if (isRowSelected(i))
    {
      field = (CSqlTableItem *) item(i, 0);
      ok = false;
      if (!field->isNewRecord() || (field->isNewRecord() && field->isItemInserted()))
      {
        ulong sql_length;
        char *sql = getDeleteQuery(field, &sql_length);
        ok = exec(sql, sql_length);        
        delete [] sql;
      }
      else
        ok = true;
      if (ok)
      {
        if (field->isNewRecord() && !field->isItemInserted() && inserting_row == i)
        {
          force_insert = false;
          inserting_row = -1;
        }
        delete_rows.resize(++cnt);
        delete_rows[cnt - 1] = i;
      }
    }
  }

  if (!delete_rows.isEmpty())
    removeRows(delete_rows);
  if (numRows() == 0)
    emit canDelete(false);
  setBlocked(false);
}

void CSqlTable::doUpdateFromFieldEditor(const char *v, ulong l, CSqlTableItem *table_item)
{
#ifdef DEBUG
  qDebug("CSqlTable::doUpdateFromFieldEditor()");
#endif

  if (isBlocked())
    return;  

  setBlocked(true);
  bool ok = false;  
  if (!table_item->isNewRecord() || (table_item->isNewRecord() && table_item->isItemInserted()))
  {
    ulong query_length;
    char *sql = getUpdateQuery(v, l, table_item, &query_length);
    if (exec(sql, query_length))
      ok = true;
    delete [] sql;
  }
  else
    ok = true;

  setBlocked(false);
  if (ok)
  {
    ulong len = query()->fields(table_item->col()).length;
    table_item->setValue(v, l > len ? len : l);
    table_item->setTextFromValue();
    if (table_item->isNewRecord() && !table_item->isItemInserted())
    {
      force_insert = true;
      doInsertRecord(table_item->row());
    }
  }
}

void CSqlTable::doLoadFromFile(CSqlTableItem *field)
{
#ifdef DEBUG
  qDebug("CSqlTable::doLoadFromFile()");
#endif

  if (isBlocked())
    return;  

  setBlocked(true);
  bool unblocked = false;
  QString s = QFileDialog::getOpenFileName(QString::null, tr("All Files(*.*)"), 0, tr("Load from File"), tr("Choose a file"));
  if (!s.isNull())
  {
    QFile file(s);
    if (!file.open(IO_ReadOnly))
    {
      mysql()->showMessage(CRITICAL, tr("An error occurred while opening the file."));
      return;
    }
    else
    {
      QDataStream ts(&file);
      QFileInfo fi(s);
      ulong len = fi.size();      
      char *data;      
      if ((data = new char [len +1]))
      {
        ts.readRawBytes(data, len);
        data[len] = 0;
        bool ok = false;
        if (!field->isNewRecord() || (field->isNewRecord() && field->isItemInserted()))
        {
          ulong query_length;
          char *sql = getUpdateQuery(data, len, field, &query_length);
          
          bool a = add_to_history;
          add_to_history = false;  //Don't add the query to the history file
          
          if (exec(sql, query_length))
            ok = true;
          add_to_history = a;  //restore the original "Add to History" settings.
          delete [] sql;
        }
        else
          ok = true;

        if (ok)
        {
          ulong l = query()->fields(field->col()).length;
          field->setValue(data, len > l ? l : len);
          field->setTextFromValue();
          if (field->isNewRecord() && !field->isItemInserted())
          {
            setBlocked(false);
            unblocked = true;
            force_insert = true;
            doInsertRecord(field->row());
          }
        }

        delete [] data;
      }
      else
        mysql()->showMessage(CRITICAL, tr("There is not enough memory to open the file"));
      file.close();
    }
  }
  if (!unblocked)
    setBlocked(false);
}

char * CSqlTable::getInsertQuery(int row, ulong *length)
{
#ifdef DEBUG
  qDebug("CSqlTable::getInsertQuery()");
#endif

  QString s = "INSERT INTO " + mysql()->mysql()->quote(table_name) + "\n (";
  uint i;
  ulong len = 0;
  CSqlTableItem *field;
  for (i = 0; i < query()->numFields(); i++)
  {
    s += mysql()->mysql()->quote(query()->fields(i).name);
    field = (CSqlTableItem *) item(row, i);

    if (field->isNull())
      len += 4;
    else
      if (field->isNumber())
        len += field->length();
      else
        len += field->length() * 2 + 3;

    if (i < query()->numFields() - 1)
    {
      s += ", ";
      len += 2;
    }
  }
  s += ")\nVALUES (";

  len+=s.length() + 2;

  char *insert = new char[len + 1];
  char *end = insert;
  end = strmov(end, (const char *)(mysql()->mysql()->codec()->fromUnicode(s)));

  for (i = 0; i < query()->numFields(); i++)
  {
    field = (CSqlTableItem *) item(row, i);
    if (field->isNull())
      end = strmov(end, "NULL");
    else
      if (field->isNumber())
      {
        memcpy(end, field->value(), field->length());
        end += field->length();
      }
      else
      {
        char * value = new char[field->length() * 2 + 3];
        char *v = value;
        *v++ = '\'';
        ulong value_length = mysql()->mysql()->mysqlRealEscapeString(field->value(), v, field->length());
        v += value_length;
        *v++ = '\'';
        value_length += 2;
        memcpy(end, value, value_length);
        end += value_length;
        delete [] value;
      }

    if (i < query()->numFields() - 1)
    {
      *end++ = ',';
      *end++ = ' ';
    }
  }
  *end++ = ')';
  *end = 0;
  *length = end - insert;
  return insert;
}

char * CSqlTable::getUpdateQuery(const char *new_value, ulong len, CSqlTableItem *field, ulong *length)
{
#ifdef DEBUG
  qDebug("CSqlTable::getUpdateQuery()");
#endif

  QString s = "UPDATE " + mysql()->mysql()->quote(table_name) + "\nSET " + 
    mysql()->mysql()->quote(query()->fields(field->index()).name) + "=";

  char *value;
  char *v = NULL;
  ulong value_length = 0;
  if (!new_value)
  {
    value_length = 4;
    value = new char[5];
    v = value;
    v = strmov(v, "NULL");
  }
  else
  {
    if (field->isNumber())
    {
      value_length = len;
      value = new char[len + 1];
      v = value;
      v = strmov(v, new_value);
    }
    else
    {
      value = new char[len * 2 + 3];
      v = value;
      *v++ = '\'';
      value_length = mysql()->mysql()->mysqlRealEscapeString(new_value, v, len);
      v += value_length;
      *v++ = '\'';
      value_length += 2;
    }
  }

  ulong where_length;
  char *where = getWhereClause(field->row(), &where_length);
  char *sql = new char[s.length() + where_length + value_length + 1];
  char *end = sql;
  end = strmov(end, (const char *)(mysql()->mysql()->codec()->fromUnicode(s)));
  memcpy(end, value, value_length);
  end += value_length;
  delete [] value;  
  memcpy(end, where, where_length);
  delete [] where;
  end += where_length;
  *end = 0;
  *length = end - sql;
  return sql;
}

char * CSqlTable::getDeleteQuery(CSqlTableItem *field, ulong *length)
{
#ifdef DEBUG
  qDebug("CSqlTable::getDeleteQuery()");
#endif

  QString s = "DELETE FROM " + mysql()->mysql()->quote(table_name);
  ulong where_length;
  char *where = getWhereClause(field->row(), &where_length);  
  char *delete_sql = new char[s.length() + where_length + 1];
  char *end = delete_sql;
  memcpy(end, (const char *)(mysql()->mysql()->codec()->fromUnicode(s)), s.length());
  end += s.length();
  memcpy(end, where, where_length);
  end += where_length;
  delete [] where;
  *end = 0;
  *length = end - delete_sql;  
  return delete_sql;
}

char * CSqlTable::getWhereClause(int row, ulong *length)
{
#ifdef DEBUG
  qDebug("CSqlTable::getWhereClause()");
#endif

  ulong len_tmp = 0;
  CSqlTableItem *field;
  QMap<uint, QString>::Iterator it;

  char where_text[] = "\nWHERE ";
  char is_null_text[] = " IS NULL ";
  char and_text[] = " AND ";
  char limit_text[] = "\nLIMIT 1";

  ulong len=strlen(where_text);

  for (it = update_columns.begin(); it != update_columns.end(); ++it)
  {
    len += it.data().length();
    field = (CSqlTableItem *) item(row, it.key());

    if (field->isNull())
      len += strlen(is_null_text);
    else
      if (field->isNumber())
      {
        len += field->length() + 1;
        if (field->isPresision())
          len += 10 + QString::number(pow(0.1, field->decimals())).length();
      }
      else
        len_tmp += field->length() * 2 + 4;
  }
  len += (update_columns.count() - 1) * strlen(and_text);
  if (update_type == ALL_FIELDS)
    len += strlen(limit_text);


  char *where = new char[len + len_tmp + 1];
  char *end_ptr;

  end_ptr = strmov(where, where_text);

  uint count = 0;
  for (it = update_columns.begin(); it != update_columns.end(); ++it)
  {
    count++;
    field = (CSqlTableItem *) item(row, it.key());   
    
    if (field->isPresision())
    {
      QString tmp = "ABS(" + it.data() + " - " + field->value() + ") < " + QString::number(pow(0.1, field->decimals()));
      end_ptr = strmov(end_ptr, (const char *)(mysql()->mysql()->codec()->fromUnicode(tmp)));      
    }
    else
    {
      end_ptr = strmov(end_ptr, it.data());
      if (field->isNull())
        end_ptr = strmov(end_ptr, is_null_text);
      else
        if (field->isNumber())  //presision numbers handled above.
        {
          *end_ptr++ = '=';
          memcpy(end_ptr, field->value(), field->length());
          end_ptr += field->length();       
        }
        else
        {
          char * value = new char[field->length() * 2 + 4];
          char *v = value;
          *v++ = '=';
          *v++ = '\'';
          ulong value_length = mysql()->mysql()->mysqlRealEscapeString(field->value(), v, field->length());
          v += value_length;
          *v++ = '\'';
          *v = 0;
          value_length += 3;
          memcpy(end_ptr, value, value_length);
          end_ptr += value_length;
          len += value_length;
          delete [] value;
        }
    }
    
    if (count < update_columns.count())
      end_ptr = strmov(end_ptr, and_text);
  }

  if (update_type == ALL_FIELDS)
    end_ptr = strmov(end_ptr, limit_text);

  *length = end_ptr - where;
  return where;
}
  
void CSqlTable::refresh()
{
#ifdef DEBUG
  qDebug("CSqlTable::refresh()");
#endif
    
  if (isBlocked())
    return;

  m_cancel = false;
  reset();
  setCaption(realParent()->caption());  
  update_columns.clear();
  default_fields_map.clear();
  bool is_pk = false;
  bool is_unique = false;
  if (m_cancel)
    return;

  if (!query()->isResultNull())
  {
    setBlocked(true);
    query()->dataSeek(0);
    uint num_fields = query()->numFields();
    ulong num_rows = query()->numRows();
    setNumRows(num_rows);
    setNumCols(num_fields);    
    QPixmap icon;
    bool columns_ok = (keepColumnWidth() && previous_columns_map.count() == num_fields);

    for (uint j = 0; j < num_fields; j++)
    {
      if (IS_PRI_KEY(query()->fields(j).flags))
      {
        is_pk = true;
        if (!columns_ok)
          break;
      }
      else
        if (!is_pk && IS_UNI_KEY(query()->fields(j).flags))
          is_unique = true;

      if (columns_ok)
        columns_ok &= (previous_columns_map[j].label == query()->fields(j).name);
    }

    if (!columns_ok && keepColumnWidth())
      previous_columns_map.clear();

    update_type = is_pk ? PRIMARY : is_unique ? UNIQUE : ALL_FIELDS;

    for (uint i = 0; i < num_fields; i++)
    {
      if (IS_PRI_KEY(query()->fields(i).flags))
      {
        icon = pkIcon;
        if (update_type == PRIMARY)
          update_columns.insert(i, mysql()->mysql()->quote(query()->fields(i).name));
      }
      else if (IS_UNI_KEY(query()->fields(i).flags))
      {
        icon = uniIcon;
        if (update_type == UNIQUE)
          update_columns.insert(i, mysql()->mysql()->quote(query()->fields(i).name));
      }
      else
      {
        icon = IS_MUL_KEY(query()->fields(i).flags) ? mulIcon : nothingIcon;
        if (update_type == ALL_FIELDS)
          update_columns.insert(i, mysql()->mysql()->quote(query()->fields(i).name));
      }

      if (columns_ok && previous_columns_map[i].label == query()->fields(i).name)
        horizontalHeader()->setLabel(i, icon, query()->fields(i).name, previous_columns_map[i].size);
      else
        horizontalHeader()->setLabel(i, icon, query()->fields(i).name);

      if (IS_BLOB(query()->fields(i).flags))
        setColumnReadOnly(i, true);
      else
        setColumnReadOnly(i, is_read_only);      
    }

    qApp->processEvents();  //Refresh should NEVER be called inside a thread other than the main GUI thread.
    
    if (!m_cancel)
    { 
      ulong z = 0;     
      while (query()->next() && !m_cancel)
      {
        if (m_cancel)
          break;
        for (uint i = 0; i < num_fields; i++)
          setItem(z, i, new CSqlTableItem(this, query(), z, i));        
        z++;
      }
    }
    setBlocked(false);
    emit refreshed();
  }
  else
    if (!previous_columns_map.empty())
      previous_columns_map.clear();
}

void CSqlTable::openFieldEditor(CFieldEditorWidget *editor)
{
  connect(editor, SIGNAL(send_message(ushort, const QString &)), mysql()->messagePanel(), SLOT(message(ushort, const QString &)));
  connect(editor, SIGNAL(contentsChanged(const char *, ulong, CSqlTableItem *)), this, SLOT(doUpdateFromFieldEditor(const char *, ulong, CSqlTableItem *)));
  CFieldEditorWindow *wnd = new CFieldEditorWindow(editor);
  connect(this, SIGNAL(about_to_close()), wnd, SLOT(close()));
  myShowWindow(wnd);
}

void CSqlTable::openTextEditor(CSqlTableItem *table_item)
{
#ifdef DEBUG
  qDebug("CSqlTable::openTextEditor()");
#endif

  CTextFieldEditor *t = new CTextFieldEditor(this, table_item, isReadOnly());
  openFieldEditor(t);
}

void CSqlTable::openImageViewer(CSqlTableItem *table_item)
{
#ifdef DEBUG
  qDebug("CSqlTable::openImageViewer()");
#endif

  CImageViewer *editor = new CImageViewer(this, table_item, isReadOnly());
  openFieldEditor(editor);
}

void CSqlTable::DoubleClicked(int row, int col, int button, const QPoint &)
{
#ifdef DEBUG
  qDebug("CSqlTable::DoubleClicked()");
#endif

  if (numCols() == 0 && numRows() == 0)
    return;

  if (button == Qt::LeftButton)
  {
    if (IS_BLOB(query()->fields(col).flags))
    {
      CSqlTableItem *table_item = (CSqlTableItem *) item(row, col);
      if (table_item)
      {
        if (table_item->isBinary())
          openImageViewer(table_item);
        else
          openTextEditor(table_item);
      }
    }
  }
}

void CSqlTable::ContextMenuRequested(int row, int col, const QPoint &pos)
{
#ifdef DEBUG
  qDebug("CSqlTable::ContextMenuRequested()");
#endif

  if (isBlocked())
    return;

  int num_cols = numCols();
  int num_rows = numRows();

  QPopupMenu *menu = new QPopupMenu();

  menu->insertItem(getPixmapIcon("insertRowIcon"), tr("&Insert Record"), MENU_INSERT);
  menu->setItemEnabled (MENU_INSERT, !isReadOnly());

  menu->insertItem(getPixmapIcon("deleteRowIcon"), tr("&Delete Record"), MENU_DELETE);
  menu->setItemEnabled (MENU_DELETE, !isReadOnly() && num_rows > 0 && num_cols > 0);

  menu->insertSeparator();
  
  QPopupMenu *open_menu = new QPopupMenu();

  CSqlTableItem *table_item = (CSqlTableItem *) item(currentRow(), currentColumn());

  if (table_item != 0)
  {
    if (table_item->isBinary())
      open_menu->insertItem(getPixmapIcon("pictureIcon"), tr("Image &Viewer"), MENU_OPEN_IMAGE);
    open_menu->insertItem(getPixmapIcon("textEditorIcon"), tr("&Text Editor"), MENU_OPEN_TEXT);
  }
  
  int id = menu->insertItem(getPixmapIcon("openIcon"), tr("Open in"), open_menu);
  if (table_item == 0)
    menu->setItemEnabled(id, false);

  menu->insertSeparator();

  menu->insertItem(getPixmapIcon("loadIcon"), tr("&Load from File"), MENU_LOAD);
  menu->setItemEnabled (MENU_LOAD, !isReadOnly());  //TODO... Need to change

  menu->insertItem(getPixmapIcon("saveIcon"), tr("&Save to File"), MENU_SAVE_TO_FILE);
  menu->setItemEnabled(MENU_SAVE_TO_FILE, table_item != 0 && !table_item->isNull());

  menu->insertSeparator();

  menu->insertItem(getPixmapIcon("copyIcon"), tr("&Copy"), MENU_COPY);
  menu->setItemEnabled(MENU_COPY, num_cols > 0 && num_rows > 0);

  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("saveGridResultsIcon"), tr("Save &Results"), MENU_SAVE);
  menu->setItemEnabled(MENU_SAVE, num_cols > 0);

  menu->insertSeparator();

  menu->insertItem(getPixmapIcon("clearGridIcon"), tr("C&lear Grid"), MENU_CLEAR_GRID);
  menu->setItemEnabled(MENU_CLEAR_GRID, num_cols > 0);

  int res = menu->exec(pos);
  delete open_menu;
  delete menu;  
  
  switch (res)
  {
    case MENU_INSERT:
      insertRecord(row);
      break;

    case MENU_DELETE:
      deleteRecord();
      break;

    case MENU_OPEN_TEXT:
      openTextEditor(table_item);        
      break;

    case MENU_OPEN_IMAGE:
      openImageViewer(table_item);
      break;

    case MENU_SAVE_TO_FILE:
      saveCellToFile(table_item);
      break;

    case MENU_COPY:
      copy(row, col);
      break;

    case MENU_LOAD:
      doLoadFromFile(table_item);
      break;

    case MENU_SAVE:
      save();
      break;

    case MENU_CLEAR_GRID:
      reset();
      break;
  }
}

QString CSqlTable::copy_current_selection_func(int row, int col)
{
  CSqlTableItem * field = (CSqlTableItem *) item(row, col);
  if (field)
    return field->value();
  else
    return QString::null;
}

void CSqlTable::copy_data_func(QString *cpy, CMySQLQuery *, QTableSelection *sel, QMap<uint, ulong> *max_length_map)
{
  uint length;
  QString tmp;
  CSqlTableItem * field;
  for (int current_row = sel->topRow(); current_row <= sel->bottomRow(); current_row++)
  {
    *cpy += "|";
    for (int current_col = sel->leftCol(); current_col <= sel->rightCol(); current_col++)
    {
      if (horizontalHeader()->sectionSize(current_col) <= 0)
        continue;
      field = (CSqlTableItem *) item(current_row, current_col);
      const char *str= field->value() ? field->value() : NULL_TEXT;
      length = (*max_length_map)[current_col];
      if (length > MAX_COLUMN_LENGTH)
      {
        *cpy += str;
        *cpy += "|";
      }
      else
      {
        tmp.sprintf(field->isNumber() ? "%*s |" : " %-*s|", length, str);
        *cpy += tmp;
      }
    }
    *cpy += "\n";
  }
}


void CSqlTable::saveCellToFile(CSqlTableItem *table_item)
{
#ifdef DEBUG
  qDebug("CSqlTable::saveCellToFile()");
#endif

  if (table_item == 0)
    return ;
  QString tmp, ext = QString::null;
  QString ext_desc = tr("All Files(*.*)");
  bool writeBinary = table_item->isBinary();
  if (!writeBinary)
  {
    ext = "txt";
    ext_desc = tr("Text Files (*.txt)") + ";;" + ext_desc;
  }
  saveToFile(tmp, ext, ext_desc, table_item->value(), messagePanel, writeBinary, table_item->length());
}

void CSqlTable::reset()
{
#ifdef DEBUG
  qDebug("CSqlTable::reset()");
#endif

  CQueryTable::reset();
  database_name = QString::null;
  table_name = QString::null;
  force_insert = false;
  inserting_row = -1;
  setReadOnly(true);
}

void CSqlTable::setReadOnly(bool b)
{
#ifdef DEBUG
  qDebug("CSqlTable::setReadOnly()");
#endif

  is_read_only = b;
  bool ro = true;
  default_fields_map.clear();
  if (query())
  {
    if (!b && !query()->isResultNull() && query()->numFields() > 0)
    {
      ro = false;
      for (uint i = 0; i < query()->numFields(); i++)
      {
        if (IS_BLOB(query()->fields(i).flags))
          setColumnReadOnly(i, true);
        else
          setColumnReadOnly(i, is_read_only);
      }
      
      if (!table_name.isEmpty())
      {        
        CMySQLQuery *qry = new CMySQLQuery(mysql()->mysql());
        qry->setEmitMessages(false);
        uint i = 0;
        if (qry->exec("SHOW FIELDS FROM " + mysql()->mysql()->quote(table_name)))
          while (qry->next(!hasProcessEvents()))
          {
            CDefaultFieldSpecs field;
            field.default_value = qry->row(4);
            field.is_null = !QString(qry->row(2)).isEmpty();
            field.auto_increment = !QString(qry->row(5)).isEmpty();
            default_fields_map[i] = field;
            i++;
          }
        delete qry;
      }
    }
  }
  CQueryTable::setReadOnly(ro);
}

int CSqlTable::maxColumnLength(uint col) const
{
#ifdef DEBUG
  qDebug("CSqlTable::maxColumnLength()");
#endif

  uint max = query()->fields(col).length;
  if (max > MAX_FIELD_DISPLAY)
    max = MAX_FIELD_DISPLAY;
  return max;
}

void CSqlTable::currentChanged(int row, int)
{
#ifdef DEBUG
  qDebug("CSqlTable::currentChanged()");
#endif

  if ((inserting_row != -1 && row != inserting_row) || force_insert)
    doInsertRecord(row);
}
