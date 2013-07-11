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
#ifndef CTABLEWINDOW_H
#define CTABLEWINDOW_H

#include <stddef.h>
#include <qvariant.h>
#include "CMyWindow.h"
#include <qtable.h>
#include <qwidget.h>
#include <qvalidator.h>
#include <qdict.h>

class CMessagePanel;
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QTabWidget;
class CMySQLServer;
class QListView;
class QListViewItem;
class QCheckBox;
class QLabel;
class QLineEdit;
class CAction;
class QPushButton;
class QRadioButton;
class QButtonGroup;
class CMySQL;
class CNullLineEdit;
class CTableWindow;

class CTableWindowField
{
public:
  CTableWindowField()
  {    
    Type = QString::null;
    Default = "";
    Value = QString::null;
    FieldName = QString::null;
    Length = QString::null;
    OriginalFieldName = QString::null;
    Unsigned = false;
    Binary = false;
    ZeroFill = false;
    AutoIncrement = false;
    Null = false;
    isNewField = false;
  }
  
  CTableWindowField(const CTableWindowField &f)
  {
    Type = f.Type;
    Default = f.Default;
    Value = f.Value;
    FieldName = f.FieldName;
    Length = f.Length;
    OriginalFieldName = QString::null;
    Unsigned = f.Unsigned;
    Binary = f.Binary;
    ZeroFill = f.ZeroFill;
    AutoIncrement = f.AutoIncrement;    
    Null = f.Null;
    isNewField = false;
  }  

  QString Type;
  QString Default;
  QString Value;
  QString Length;
  QString FieldName;
  QString OriginalFieldName;  //ALTER TABLE CHANGE OriginalFieldName FieldName
  bool Unsigned;
  bool Binary;
  bool ZeroFill;
  bool AutoIncrement;
  bool Null;
  bool isNewField;
  
  bool operator == (CTableWindowField f) const
  {
    bool ret = true;
    if (&f != this)
    {
      ret = (f.AutoIncrement == AutoIncrement);
      ret &= (f.Type == Type);
      ret &= (f.Default == Default);
      ret &= (f.Value == Value);
      ret &= (f.Length == Length);
      ret &= (f.FieldName == FieldName);
      ret &= (f.Unsigned == Unsigned);
      ret &= (f.Binary == Binary);
      ret &= (f.ZeroFill == ZeroFill);
      ret &= (f.Null == Null);
    }
    return ret;
  }
};

class CTableWindowComboItem : public QComboTableItem
{
public:
  CTableWindowComboItem(QTable * table, const QStringList & list, bool editable = false)
  : QComboTableItem(table, list, editable)
  {
    Field = new CTableWindowField();
  }
  CTableWindowComboItem(QTable * table, CTableWindowField *f, const QStringList & list, bool editable = false)
  : QComboTableItem(table, list, editable)
  {
    Field = new CTableWindowField(*f);
  }

  ~CTableWindowComboItem()
  {
    delete Field;
  }

  CTableWindowField *Field;
};

class CAlterTableOptions : public QWidget
{ 
  Q_OBJECT
    
public:
  CAlterTableOptions(QWidget* parent, QTable *f, CMySQLServer *m, const char* name = 0, WFlags fl = 0);
  QString getAlterTableOptions() const;
  QCheckBox* ignore;
  QComboBox* orderBy;

public slots:
  void refresh();
  void reset();
  
private:
  CMySQLServer *mysql;
  QTable *table;
  QLabel* TextLabel1;
  QGridLayout* CAlterTableOptionsLayout;
};

class CTblProperties : public QWidget
{ 
  Q_OBJECT
    
public:
  CTblProperties(QWidget* parent, CMySQL *m, const char* name = 0, WFlags fl = 0);
  QString getTableProperties() const;
  QLineEdit* tableName;
  QLineEdit* comment;
  QLineEdit* minRows;
  QLineEdit* maxRows;
  QLineEdit* autoIncrement;
  QComboBox* tableType;
  QComboBox* rowFormat;
  QComboBox* packKeys;
  QComboBox* delayKeyWrite;
  QComboBox* checksum;
  QLineEdit* avgRowLength;

  
private:
  CMySQL *mysql;
  QLabel* TextLabel1;
  QLabel* TextLabel5;
  QLabel* TextLabel10;
  QLabel* TextLabel11;
  QLabel* TextLabel8;
  QLabel* TextLabel9;
  QLabel* TextLabel2;
  QLabel* TextLabel3;
  QLabel* TextLabel4;
  QLabel* TextLabel6;
  QLabel* TextLabel7;
  QGridLayout* CTblPropertiesLayout;
};

class CIndexField
{
public:
  CIndexField(const QString &field_name=0, const QString &field_type=0, int field_length=-1)
  : fn(field_name), ft(field_type), len(field_length)
  {
  }

  bool operator == (CIndexField f) const
  {
    return (f.fieldName() == fieldName());
  }

  QString fieldName() const { return fn; }
  QString fieldType() const { return ft.lower(); }
  int length() { return len; }

  void setFieldName(const QString &field_name) { fn = field_name; }
  void setLength(int field_length) { len = field_length; }
  void setFieldType(const QString &field_type) { ft = field_type; }
   
private:
  QString fn;
  QString ft;
  int len;
};

typedef QValueList<CIndexField> IndexFieldList;
enum IndexType { PRIMARY, INDEX, UNIQUE, FULLTEXT }; 

class CIndex
{
public:
  CIndex(const QString &name, IndexType t = INDEX)      
  {
    Name = name;
    Type = t;
    isNewIndex = false;
  }
  IndexFieldList index_fields;
  IndexType Type;
  bool UniFullIndex;
  bool isNewIndex;
  QString Name;
};

class CTableIndexes : public QWidget
{ 
  Q_OBJECT
    
public: 
  CTableIndexes(QWidget* parent, QTable *table, CMySQL *m, CMessagePanel *p, const char* name = 0, WFlags fl = 0);
  void setIsAlterTable(bool b) { isAlterTable = b; }
  QButtonGroup* indexTypes;
  QRadioButton* uniqueIndex;
  QRadioButton* indexIndex;
  QRadioButton* fullTextIndex;
  QComboBox* indexName;
  void add(const QString &index_name, IndexType type, IndexFieldList fields);
  void singleRemove(const QString &index, const QString &field);
  void singleAdd(const QString &index, const QString &field);
  QString getIndexes(bool alter=false) const;
  QString getDropIndexes();

public slots:
  void refresh();
  void clear();
  void changeType(IndexType t);

private slots:
  void RemoveField();
  void AddField();
  void AddIndex();
  void DeleteIndex();  
  void IndexChanged(const QString &s);
  void UniToggled(bool);
  void IdxToggled(bool);
  void FtToggled(bool);  
  void setIdxLength(QListViewItem *);
  void showFieldsUsedMenu(QListViewItem *, const QPoint &, int);  

signals:
  void primaryKey(int row, bool isPk);
  void do_click(int);

private: 
  void insertListViewItem(QListView *, const QString &);
  bool canSetIndexLength(QListViewItem *, IndexFieldList::Iterator &);
  void setIndexLength(QListViewItem *, IndexFieldList::Iterator &);
  QString getIndexFields(CIndex *index) const;
  QDict<CIndex> indexDict;
  QTable *fields;
  CMySQL *mysql;
  CMessagePanel *messagePanel;
  QLabel* TextLabel1;
  QPushButton* deleteIndex;
  QPushButton* removeField;
  QPushButton* addField;
  QPushButton* addIndex;
  QLabel* PixmapLabel2;
  QLabel* PixmapLabel3;
  QLabel* PixmapLabel4;
  QListView* fieldsUsed;
  QListView* availableFields;
  QGridLayout* CTableIndexesLayout;
  QGridLayout* indexTypesLayout;
  QStringList deletedIndexes;
  bool isAlterTable;
};

class CFieldProperties : public QWidget
{ 
  Q_OBJECT
    
public:
  CFieldProperties(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  void resetLength(CTableWindowField *f, const QString &type);
  void setTableWindow(CTableWindow *w) { tableWindow = w; }

public slots:
  void setField(CTableWindowField *f);
  void setMessagePanel(CMessagePanel *m) { messagePanel = m; }

private slots:
  void setFieldType(const QString &t);
  void setDefault(const QString &s);
  void setValue(const QString &s);
  void setLength(const QString &s);
  void setUnsigned(bool b);
  void setBinary(bool b);
  void setZeroFill(bool b);
  void setAutoIncrement(bool b);
  void setHasModified(bool b);

private:
  bool skip;
  CTableWindow *tableWindow;
  CMessagePanel *messagePanel;
  CTableWindowField *currentField;
  CTableWindowField *autoIncField;
  QIntValidator *intValidator;
  QRegExpValidator *floatValidator;
  QLabel* TextLabel1;
  QLabel* TextLabel2;
  QLabel* TextLabel3;
  CNullLineEdit* Default;
  QLineEdit* Value;
  QLineEdit* Length;
  QCheckBox* Unsigned;
  QCheckBox* Binary;
  QCheckBox* ZeroFill;
  QCheckBox* AutoIncrement;
  QGridLayout* CFieldPropertiesLayout;
  bool isNumberField(const QString &type);
  bool isCharField(const QString &type);
};

class CTableWindow : public CMyWindow
{ 
  Q_OBJECT
    
public:
  CTableWindow(QWidget* parent, CMySQLServer *m, const QString &dbname, const QString &table_name);
  ~CTableWindow();
  QStringList fieldTypes() const { return field_types; }
  QStringList deletedFields;  
  
signals:
  void do_refresh();
  void valueChanged();

public slots:
  void setHasModified(bool b) { hasModified = b; }

private slots:
  void viewMenuAboutToShow();
  void showMessages(bool);
  void currentChanged(int row, int);
  void fieldsValueChanged(int row, int col);
  void showContextMenu(int, int, const QPoint &pos);
  void insertRow();
  void deleteRow();
  void setPrimaryKey(int row, bool add);
  void setPk();
  bool save();
  void saveAs();
  void reset();
  void copy();
  void paste();
  void emitValueChanged(QWidget *w = 0);
  void addPrimaryIcon(int row, bool add);
  void doClick(int row);
  void verticalHeaderReleased(int);

private:
  class MyTable : public QTable
  {
  public:
    MyTable(QWidget * parent = 0, const char * name = 0) : QTable(parent, name) {}
    bool is_editing() { return isEditing(); }
    void finishEdit()
    {
      if (isEditing())
        endEdit (currentRow(), currentColumn(), true, false);
    }
  };
  
  void closeEvent(QCloseEvent * e);
  bool exec(const QString &qry);
  bool createTable(const QString &tblName);
  QString getFieldSpecs(CTableWindowField *field, QStringList &dups, bool &ok) const;
  bool alterTable();
  void setWindowCaption();
  bool checkDuplicateNames(const QString &txt, int row, int col);
  void insert(CTableWindowField *f=0);
  QPixmap pkIcon;
  QPixmap emptyPkIcon;
  MyTable* fields;
  QTabWidget* tab;
  QPixmap null_pixmap;
  CFieldProperties* fieldPropertiesTab;
  CTableIndexes* indexesTab;
  CTblProperties* tablePropertiesTab;
  CAlterTableOptions * alterTableOptionsTab;
  QGridLayout* CTableWindowLayout;
  CAction *tablePrimaryKeyAction;
  CAction *viewShowMessagesAction;
  CAction *fileReloadAction;
  CAction *fileSaveAsAction;
  CAction *editCopyAction;
  CAction *editPasteAction;
  bool isAlterTable;
  bool hasModified;
  bool hasPaste;
  bool skipValueChanged;
  QString tableName;
  QString databaseName;
  CTableWindowField *pasteField;
  CMySQLServer *mysql;
  QStringList field_types;

  QString defaultFieldLength;
  int defaultFieldType;
};

#endif // CTABLEWINDOW_H
