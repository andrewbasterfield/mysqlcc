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
#ifndef CSQLTABLE_H
#define CSQLTABLE_H

#include "CQueryTable.h"
#include <stddef.h>
#include <qmap.h>

class CMySQLQuery;
class CSqlTableItem;
class CNullLineEdit;
class CMessagePanel;
class QLineEdit;
class CHistoryPanel;
class CFieldEditorWidget;

class CSqlTable : public CQueryTable
{
  Q_OBJECT

public:
  CSqlTable(QWidget * parent, CMySQLQuery *q, CMySQLServer *m, const char * name = 0);
  bool isReadOnly() const { return is_read_only; }
  void setHistoryPanel(CHistoryPanel *h) { historyPanel = h; }
  void addToHistoryPanel(bool b) { add_to_history = b; }
  void displayMessages(bool b) { display_messages = b; }

signals:
  void progress(int);
  void canDelete(bool);

public slots:
  void refresh();
  void reset();
  void setReadOnly(bool b);
  void setDatabaseName(const QString &db_name) { database_name = db_name; }
  void setTableName(const QString &tbl_name) { table_name = tbl_name; }
  bool exec(const char *, ulong);  
  void insertRecord(int row=-1);
  void deleteRecord();
  void openFieldEditor(CFieldEditorWidget *editor);

protected:
  void endEdit(int row, int col, bool accept, bool replace);
  QWidget * beginEdit(int row, int col, bool replace);
  QWidget * createEditor(int row, int col, bool initFromCell) const;
  void copy_data_func(QString *cpy, CMySQLQuery *qry, Q3TableSelection *sel, QMap<uint, ulong> *max_length_map);
  QString copy_current_selection_func(int row, int col);

protected slots:
  void ContextMenuRequested(int row, int col, const QPoint &pos);
  void openTextEditor(CSqlTableItem *table_item);
  void openImageViewer(CSqlTableItem *table_item);
  void DoubleClicked(int, int, int, const QPoint &);

private slots:
  void saveCellToFile(CSqlTableItem *table_item);
  void doLoadFromFile(CSqlTableItem *current_item);
  void doUpdateFromFieldEditor(const char * /*value*/, ulong /*length*/, CSqlTableItem * /*table_item*/);  
  bool doUpdateRecord(CNullLineEdit *editor, CSqlTableItem *current_item);
  void doInsertRecord(int row);
  void currentChanged(int, int);

private:
  class CDefaultFieldSpecs
  {
  public:
    CDefaultFieldSpecs()
    {
      default_value = QString::null;
      is_null = false;      
      auto_increment = false;
    }

    CDefaultFieldSpecs(const QString &def, bool n, bool a)
    : default_value(def), is_null(n), auto_increment(a)
    {
    }

    QString default_value;
    bool is_null;
    bool auto_increment;
  };  

  enum update_Type { PRIMARY, UNIQUE, ALL_FIELDS };
  CMessagePanel *messagePanel;
  CHistoryPanel *historyPanel;
  int maxColumnLength(uint col) const;    
  char * getUpdateQuery(const char *new_value, ulong len, CSqlTableItem *current_item, ulong *length);  
  char * getInsertQuery(int row, ulong *length);
  char * getWhereClause(int row, ulong *length);
  char * getDeleteQuery(CSqlTableItem *current_item, ulong *length);
  update_Type update_type;
  QMap<uint, QString> update_columns;
  QMap<uint, CDefaultFieldSpecs> default_fields_map;  
  bool is_read_only;
  bool add_to_history;
  bool display_messages;
  int inserting_row;
  QString database_name;
  QString table_name;
  bool force_insert;
};

#endif

