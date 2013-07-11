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
#include "CTableWindow.h"
#include "CMySQLServer.h"
#include "panels.h"
#include "config.h"
#include "globals.h"
#include "CAction.h"
#include "CMySQLQuery.h"
#include "CMySQL.h"
#include "CHotKeyEditorDialog.h"
#include "CNullLineEdit.h"
#include <stddef.h>
#include <qregexp.h>
#include <qvariant.h>
#include <qtable.h>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qinputdialog.h>
#include <qaccel.h>

static bool has_primary_index = false;

CAlterTableOptions::CAlterTableOptions(QWidget* parent, QTable *f, CMySQLServer *m, const char* name, WFlags fl)
: QWidget(parent, name, fl)
{
  if (!name)
    setName("CAlterTableOptions");
  setCaption(trUtf8("Alter Table Options"));
  mysql = m;
  table = f;
  CAlterTableOptionsLayout = new QGridLayout(this, 1, 1, 4, 2, "CAlterTableOptionsLayout"); 
  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CAlterTableOptionsLayout->addItem(spacer, 2, 2);
  
  TextLabel1 = new QLabel(this, "TextLabel1");
  TextLabel1->setText(trUtf8("Order By"));
  
  CAlterTableOptionsLayout->addWidget(TextLabel1, 1, 0);
  
  ignore = new QCheckBox(this, "ignore");
  ignore->setText(trUtf8("IGNORE"));
  
  CAlterTableOptionsLayout->addMultiCellWidget(ignore, 0, 0, 0, 1);
  
  orderBy = new QComboBox(false, this, "orderBy");
  
  CAlterTableOptionsLayout->addWidget(orderBy, 1, 1);
  QSpacerItem* spacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CAlterTableOptionsLayout->addItem(spacer_2, 0, 3);
  reset();
}

QString CAlterTableOptions::getAlterTableOptions() const
{
  QString ret = QString::null;
  if (orderBy->currentText() != "[DEFAULT]")
    ret = ", ORDER BY " + mysql->mysql()->quote(orderBy->currentText());
  return ret;
}

void CAlterTableOptions::reset()
{  
  orderBy->clear();
  orderBy->insertItem("[DEFAULT]");
  orderBy->setCurrentItem(0);
  ignore->setChecked(false);
}

void CAlterTableOptions::refresh()
{
  QString current = orderBy->currentText();
  orderBy->clear();
  orderBy->insertItem("[DEFAULT]");
  QString opt;
  for (int i = 0; i < table->numRows(); i++)
  {
    opt = table->text(i, 0);
    if (!opt.isEmpty())
      orderBy->insertItem(opt);
  }
  orderBy->setCurrentText(current);
}

CTblProperties::CTblProperties(QWidget* parent, CMySQL *m, const char* name, WFlags fl)
: QWidget(parent, name, fl), mysql(m)
{
  if (!name)
    setName("CTblProperties");
  setCaption(trUtf8("Table Properties"));
  CTblPropertiesLayout = new QGridLayout(this, 1, 1, 4, 2, "CTblPropertiesLayout"); 
  
  TextLabel1 = new QLabel(this, "TextLabel1");
  TextLabel1->setText(trUtf8("Table Name"));
  
  CTblPropertiesLayout->addWidget(TextLabel1, 0, 0);
  
  TextLabel5 = new QLabel(this, "TextLabel5");
  TextLabel5->setText(trUtf8("Avg. Row Length"));
  
  CTblPropertiesLayout->addMultiCellWidget(TextLabel5, 4, 5, 0, 0);
  
  TextLabel10 = new QLabel(this, "TextLabel10");
  TextLabel10->setText(trUtf8("Comment"));
  
  CTblPropertiesLayout->addWidget(TextLabel10, 1, 0);
  
  TextLabel11 = new QLabel(this, "TextLabel11");
  TextLabel11->setText(trUtf8("Auto Increment"));
  
  CTblPropertiesLayout->addWidget(TextLabel11, 6, 0);
  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CTblPropertiesLayout->addMultiCell(spacer, 4, 5, 2, 2);
  QSpacerItem* spacer_2 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CTblPropertiesLayout->addItem(spacer_2, 7, 1);
  
  TextLabel8 = new QLabel(this, "TextLabel8");
  TextLabel8->setText(trUtf8("Delay Key Write"));
  TextLabel8->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));
  
  CTblPropertiesLayout->addMultiCellWidget(TextLabel8, 3, 3, 4, 5);
  
  TextLabel9 = new QLabel(this, "TextLabel9");
  TextLabel9->setText(trUtf8("Row Format"));
  TextLabel9->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));
  
  CTblPropertiesLayout->addWidget(TextLabel9, 1, 5);
  
  TextLabel2 = new QLabel(this, "TextLabel2");
  TextLabel2->setText(trUtf8("Table Type"));
  TextLabel2->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));
  
  CTblPropertiesLayout->addWidget(TextLabel2, 0, 5);
  
  TextLabel3 = new QLabel(this, "TextLabel3");
  TextLabel3->setText(trUtf8("Min Rows"));
  
  CTblPropertiesLayout->addWidget(TextLabel3, 2, 0);
  
  TextLabel4 = new QLabel(this, "TextLabel4");
  TextLabel4->setText(trUtf8("Max Rows"));
  
  CTblPropertiesLayout->addWidget(TextLabel4, 3, 0);
  
  TextLabel6 = new QLabel(this, "TextLabel6");
  TextLabel6->setText(trUtf8("Pack Keys"));
  TextLabel6->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));
  
  CTblPropertiesLayout->addMultiCellWidget(TextLabel6, 2, 2, 4, 5);
  
  TextLabel7 = new QLabel(this, "TextLabel7");
  TextLabel7->setText(trUtf8("Checksum"));
  TextLabel7->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));
  
  CTblPropertiesLayout->addMultiCellWidget(TextLabel7, 4, 4, 3, 5);
  
  tableName = new QLineEdit(this, "tableName");
  
  CTblPropertiesLayout->addMultiCellWidget(tableName, 0, 0, 1, 4);
  
  comment = new QLineEdit(this, "comment");
  
  CTblPropertiesLayout->addMultiCellWidget(comment, 1, 1, 1, 4);
  
  minRows = new QLineEdit(this, "minRows");
  
  CTblPropertiesLayout->addWidget(minRows, 2, 1);
  
  maxRows = new QLineEdit(this, "maxRows");
  
  CTblPropertiesLayout->addWidget(maxRows, 3, 1);
  
  autoIncrement = new QLineEdit(this, "autoIncrement");
  
  CTblPropertiesLayout->addMultiCellWidget(autoIncrement, 5, 6, 1, 1);
  
  tableType = new QComboBox(false, this, "tableType");
  tableType->insertItem(trUtf8("MyISAM"));
  tableType->insertItem(trUtf8("InnoDB"));
  tableType->insertItem(trUtf8("HEAP"));
  tableType->insertItem(trUtf8("BDB"));
  tableType->insertItem(trUtf8("ISAM"));
  
  CTblPropertiesLayout->addWidget(tableType, 0, 6);
  
  rowFormat = new QComboBox(false, this, "rowFormat");
  rowFormat->insertItem(trUtf8("DEFAULT"));
  rowFormat->insertItem(trUtf8("DYNAMIC"));
  rowFormat->insertItem(trUtf8("FIXED"));
  rowFormat->insertItem(trUtf8("COMPRESSED"));
  rowFormat->setMinimumSize(QSize(100, 0));
  
  CTblPropertiesLayout->addWidget(rowFormat, 1, 6);
  
  packKeys = new QComboBox(false, this, "packKeys");
  packKeys->insertItem(trUtf8("DEFAULT"));
  packKeys->insertItem(trUtf8("0"));
  packKeys->insertItem(trUtf8("1"));
  
  CTblPropertiesLayout->addWidget(packKeys, 2, 6);
  
  delayKeyWrite = new QComboBox(false, this, "delayKeyWrite");
  delayKeyWrite->insertItem(trUtf8("[DEFAULT]"));
  delayKeyWrite->insertItem(trUtf8("0"));
  delayKeyWrite->insertItem(trUtf8("1"));
  
  CTblPropertiesLayout->addWidget(delayKeyWrite, 3, 6);
  
  checksum = new QComboBox(false, this, "checksum");
  checksum->insertItem(trUtf8("[DEFAULT]"));
  checksum->insertItem(trUtf8("0"));
  checksum->insertItem(trUtf8("1"));
  checksum->setMinimumSize(QSize(100, 0));
  
  CTblPropertiesLayout->addWidget(checksum, 4, 6);
  
  avgRowLength = new QLineEdit(this, "avgRowLength");
  
  CTblPropertiesLayout->addWidget(avgRowLength, 4, 1);
  
  // tab order
  setTabOrder(tableName, tableType);
  setTabOrder(tableType, comment);
  setTabOrder(comment, rowFormat);
  setTabOrder(rowFormat, minRows);
  setTabOrder(minRows, packKeys);
  setTabOrder(packKeys, maxRows);
  setTabOrder(maxRows, delayKeyWrite);
  setTabOrder(delayKeyWrite, avgRowLength);
  setTabOrder(avgRowLength, checksum);
  setTabOrder(checksum, autoIncrement);
}

QString CTblProperties::getTableProperties() const
{
  QString ret = "TYPE=" + tableType->currentText().upper() + " ";
  //bool notnew = (m->mysql()->version().major <= 3 && m->mysql()->version().minor <= 22);

  if (!minRows->text().stripWhiteSpace().isEmpty())
    ret += "MIN_ROWS=" + minRows->text().stripWhiteSpace() + " ";

  if (!maxRows->text().stripWhiteSpace().isEmpty())
    ret += "MAX_ROWS=" + maxRows->text().stripWhiteSpace() + " ";

  if (!autoIncrement->text().stripWhiteSpace().isEmpty())
    ret += "AUTO_INCREMENT=" + autoIncrement->text().stripWhiteSpace() + " ";

  if (!avgRowLength->text().stripWhiteSpace().isEmpty())
    ret += "AVG_ROW_LENGTH=" + avgRowLength->text().stripWhiteSpace() + " ";
   
  if (packKeys->currentText() != "DEFAULT" || mysql->version().major > 3)
  ret += "PACK_KEYS=" + packKeys->currentText() + " ";

  if (checksum->currentText() != "[DEFAULT]")
    ret += "CHECKSUM=" + checksum->currentText() + " ";

  if (delayKeyWrite->currentText() != "[DEFAULT]")
    ret += "DELAY_KEY_WRITE=" + delayKeyWrite->currentText() + " ";

  ret += "ROW_FORMAT=" + rowFormat->currentText() + " ";

  QString c = comment->text().stripWhiteSpace();
  if (!c.isEmpty())
    ret += "COMMENT='" + mysql->escape(c) + "' ";

  return ret.left(ret.length() - 1);
}

CTableIndexes::CTableIndexes(QWidget* parent, QTable *table, CMySQL *m, CMessagePanel *p, const char* name, WFlags fl)
: QWidget(parent, name, fl), fields(table), mysql(m), messagePanel(p)
{
  if (!name)
    setName("CTableIndexes");
  
  setCaption(trUtf8("Indexes"));
  isAlterTable = false;
  indexDict.setAutoDelete(true);
  CTableIndexesLayout = new QGridLayout(this, 1, 1, 4, 2, "CTableIndexesLayout"); 
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);
  CTableIndexesLayout->addItem(spacer, 3, 1);
  
  TextLabel1 = new QLabel(this, "TextLabel1");
  TextLabel1->setText(trUtf8("Index Name"));
  
  CTableIndexesLayout->addWidget(TextLabel1, 0, 3);
  
  deleteIndex = new QPushButton(this, "deleteIndex");
  deleteIndex->setMinimumSize(QSize(20, 20));
  deleteIndex->setMaximumSize(QSize(20, 20));
  deleteIndex->setText(trUtf8(""));
  deleteIndex->setPixmap(getPixmapIcon("minusIcon"));
  QWhatsThis::add(deleteIndex, trUtf8("Delete selected Index"));
  
  CTableIndexesLayout->addWidget(deleteIndex, 1, 7);
  
  indexName = new QComboBox(false, this, "indexName");
  indexName->insertItem(getPixmapIcon("pkIcon"), "PRIMARY");
  indexName->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, indexName->sizePolicy().hasHeightForWidth()));
  QWhatsThis::add(indexName, trUtf8("Available Index Names"));
  
  CTableIndexesLayout->addMultiCellWidget(indexName, 0, 0, 4, 7);
  
  removeField = new QPushButton(this, "removeField");
  removeField->setMinimumSize(QSize(30, 30));
  removeField->setMaximumSize(QSize(30, 30));
  removeField->setText(trUtf8(""));
  removeField->setPixmap(getPixmapIcon("leftArrowIcon"));
  
  CTableIndexesLayout->addWidget(removeField, 4, 1);
  
  addField = new QPushButton(this, "addField");
  addField->setMinimumSize(QSize(30, 30));
  addField->setMaximumSize(QSize(30, 30));
  addField->setText(trUtf8(""));
  addField->setPixmap(getPixmapIcon("rightArrowIcon"));
  
  CTableIndexesLayout->addWidget(addField, 2, 1);
  QSpacerItem* spacer_2 = new QSpacerItem(64, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CTableIndexesLayout->addItem(spacer_2, 1, 4);
  QSpacerItem* spacer_3 = new QSpacerItem(16, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
  CTableIndexesLayout->addItem(spacer_3, 1, 6);
  
  addIndex = new QPushButton(this, "addIndex");
  addIndex->setMinimumSize(QSize(20, 20));
  addIndex->setMaximumSize(QSize(20, 20));
  addIndex->setText(trUtf8(""));
  addIndex->setPixmap(getPixmapIcon("plusIcon"));
  QWhatsThis::add(addIndex, trUtf8("Add new Index"));
  
  CTableIndexesLayout->addWidget(addIndex, 1, 5);
  
  indexTypes = new QButtonGroup(this, "indexTypes");
  indexTypes->setTitle(trUtf8("Index Type"));
  QWhatsThis::add(indexTypes, trUtf8("Type of Index for 'Index Name'"));
  indexTypes->setColumnLayout(0, Qt::Vertical);
  indexTypes->layout()->setSpacing(2);
  indexTypes->layout()->setMargin(4);
  indexTypesLayout = new QGridLayout(indexTypes->layout());
  indexTypesLayout->setAlignment(Qt::AlignTop);
  QSpacerItem* spacer_4 = new QSpacerItem(0, 53, QSizePolicy::Minimum, QSizePolicy::Expanding);
  indexTypesLayout->addItem(spacer_4, 3, 1);
  
  indexIndex = new QRadioButton(indexTypes, "indexIndex");
  indexIndex->setText(trUtf8("INDEX"));
  
  indexTypesLayout->addWidget(indexIndex, 0, 1);
  
  PixmapLabel2 = new QLabel(indexTypes, "PixmapLabel2");
  PixmapLabel2->setPixmap(getPixmapIcon("mulIcon"));
  
  indexTypesLayout->addWidget(PixmapLabel2, 0, 0);
  
  uniqueIndex = new QRadioButton(indexTypes, "uniqueIndex");
  uniqueIndex->setText(trUtf8("UNIQUE"));
  
  indexTypesLayout->addWidget(uniqueIndex, 1, 1);
  
  PixmapLabel3 = new QLabel(indexTypes, "PixmapLabel3");
  PixmapLabel3->setPixmap(getPixmapIcon("uniIcon"));
  
  indexTypesLayout->addWidget(PixmapLabel3, 1, 0);
  
  fullTextIndex = new QRadioButton(indexTypes, "fullTextIndex");
  fullTextIndex->setText(trUtf8("FULLTEXT"));
  
  indexTypesLayout->addWidget(fullTextIndex, 2, 1);
  
  PixmapLabel4 = new QLabel(indexTypes, "PixmapLabel4");
  PixmapLabel4->setPixmap(getPixmapIcon("ftIcon"));
  
  indexTypesLayout->addWidget(PixmapLabel4, 2, 0);
  QSpacerItem* spacer_5 = new QSpacerItem(31, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  indexTypesLayout->addItem(spacer_5, 1, 2);
  
  CTableIndexesLayout->addMultiCellWidget(indexTypes, 2, 4, 3, 7);
  
  fieldsUsed = new QListView(this, "fieldsUsed");
  fieldsUsed->addColumn(trUtf8("Fields Used"));
  fieldsUsed->header()->setClickEnabled(false, 0);
  fieldsUsed->header()->setResizeEnabled(false, 0);
  fieldsUsed->setSelectionMode(QListView::Extended);
  fieldsUsed->setSorting(-1);
  fieldsUsed->setResizeMode(QListView::AllColumns);

  QWhatsThis::add(fieldsUsed, trUtf8("Fields used in this Index"));
  
  CTableIndexesLayout->addMultiCellWidget(fieldsUsed, 0, 4, 2, 2);
  
  availableFields = new QListView(this, "availableFields");
  availableFields->addColumn(trUtf8("Available Fields"));
  availableFields->header()->setClickEnabled(false, 0);
  availableFields->header()->setResizeEnabled(false, 0);
  availableFields->setSelectionMode(QListView::Extended);  
  availableFields->setSorting(-1);
  
  availableFields->setResizeMode(QListView::AllColumns);
  QWhatsThis::add(availableFields, trUtf8("Available Fields that can be used as an Index"));
  
  CTableIndexesLayout->addMultiCellWidget(availableFields, 0, 4, 0, 0);
  
  connect(removeField, SIGNAL(clicked()), this, SLOT(RemoveField()));
  connect(addField, SIGNAL(clicked()), this, SLOT(AddField()));
  connect(addIndex, SIGNAL(clicked()), this, SLOT(AddIndex()));
  connect(deleteIndex, SIGNAL(clicked()), this, SLOT(DeleteIndex()));
  connect(indexName, SIGNAL(activated(const QString &)), this, SLOT(IndexChanged(const QString &)));
  connect(uniqueIndex, SIGNAL(toggled(bool)), this, SLOT(UniToggled(bool)));
  connect(indexIndex, SIGNAL(toggled(bool)), this, SLOT(IdxToggled(bool)));
  connect(fullTextIndex, SIGNAL(toggled(bool)), this, SLOT(FtToggled(bool)));
  connect(fieldsUsed, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)), this, SLOT(showFieldsUsedMenu(QListViewItem *, const QPoint &, int)));
  connect(fieldsUsed, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(setIdxLength(QListViewItem *)));
  
  // tab order
  setTabOrder(indexName, availableFields);
  setTabOrder(availableFields, addField);
  setTabOrder(addField, fieldsUsed);
  setTabOrder(fieldsUsed, removeField);
  setTabOrder(removeField, addIndex);
  setTabOrder(addIndex, deleteIndex);
  setTabOrder(deleteIndex, indexIndex);
  setTabOrder(indexIndex, uniqueIndex);
  setTabOrder(uniqueIndex, fullTextIndex);

  indexDict.insert("PRIMARY", new CIndex("PRIMARY", PRIMARY));
  indexName->setCurrentItem(0);
}

void CTableIndexes::clear()
{
  indexDict.clear();
  fieldsUsed->clear();
  availableFields->clear();
  indexName->clear();
  deletedIndexes.clear();
  indexDict.insert("PRIMARY", new CIndex("PRIMARY", PRIMARY));
  indexName->insertItem(getPixmapIcon("pkIcon"), "PRIMARY");
  indexName->setCurrentItem(0);
}

void CTableIndexes::insertListViewItem(QListView *lv, const QString &txt)
{
  QListViewItem *item;
  if (lv->lastItem() != 0)
    item = new QListViewItem(lv, lv->lastItem());
  else
    item = new QListViewItem(lv);
  item->setText(0, txt);
}

bool CTableIndexes::canSetIndexLength(QListViewItem *item, IndexFieldList::Iterator &iterator)
{
  if (!item)
    return false;
  bool ok = false;
  QString type;
  QString txt = item->text(0);
  txt = txt.left(txt.find(" "));
  if (txt.isEmpty())
    return false;

  for (int i = 0; i < fields->numRows(); i++)
  {
    if (!fields->text(i, 0).isEmpty())
    {
      CTableWindowComboItem *ft = (CTableWindowComboItem *)fields->item(i, 2);
      if (ft->Field->FieldName == txt)
      {
        type = ft->Field->Type;
        ok = true;
        break;
      }
    }
  }

  if (ok && (type.find("char") != -1 || type.find("blob") != -1 || type.find("text") != -1))
  {
    CIndex *currentIndex = indexDict.find(indexName->currentText());
    IndexFieldList::Iterator it = currentIndex->index_fields.find(txt);
    if (it != currentIndex->index_fields.end())
    {
      iterator = it;
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

void CTableIndexes::setIndexLength(QListViewItem *item, IndexFieldList::Iterator &iterator)
{
  bool ok = false;
  int res = QInputDialog::getInteger(item->text(0), tr("Enter the Index Length"), 
    (*iterator).length() > 0 ? (*iterator).length() : 10, 1, 500, 2, &ok, this);
  if (ok)
  {
    (*iterator).setLength(res);
    refresh();
  }
}

void CTableIndexes::setIdxLength(QListViewItem *item)
{
  IndexFieldList::Iterator iterator;
  if (canSetIndexLength(item, iterator))
    setIndexLength(item, iterator);
}

void CTableIndexes::showFieldsUsedMenu(QListViewItem *item, const QPoint &pos, int)
{
  if (!item)
    return;
  IndexFieldList::Iterator iterator;
  if (canSetIndexLength(item, iterator))
  {
    QPopupMenu *menu = new QPopupMenu();
    menu->insertItem(tr("Index Length"), 1);
    if (menu->exec(pos) == 1)
      setIndexLength(item, iterator);
    delete menu;
  }
}

void CTableIndexes::refresh()
{
  availableFields->clear();
  fieldsUsed->clear();

  CIndex *currentIndex = indexDict.find(indexName->currentText());

  if (currentIndex->Type == PRIMARY)
  {
    uniqueIndex->setEnabled(false);
    fullTextIndex->setEnabled(false);
    indexIndex->setEnabled(false);
    uniqueIndex->setChecked(false);
    fullTextIndex->setChecked(false);
    indexIndex->setChecked(false);
  }
  else
  {
    uniqueIndex->setEnabled(true);
    fullTextIndex->setEnabled(true);
    indexIndex->setEnabled(true);
    switch (currentIndex->Type)
    {
    case UNIQUE:
      uniqueIndex->setChecked(true);
      break;
    case FULLTEXT:
      fullTextIndex->setChecked(true);
      break;
    default:  //index->Type can't be PRIMARY ... needs to be INDEX
      indexIndex->setChecked(true);
      break;
    }
  }

  QString t;
  QStringList lst;
  int i;
  for (i = 0; i < fields->numRows(); i++)
  {
    t = fields->text(i, 0);    
    if (!t.isEmpty())
    {
      lst.append(t);
      if (currentIndex->index_fields.find(t) == currentIndex->index_fields.end())
        insertListViewItem(availableFields, t);
    }
  }
 
  QDictIterator<CIndex> it(indexDict);
  IndexFieldList del;
  IndexFieldList::Iterator del_it;
  IndexFieldList::iterator it2;
  for(; it.current(); ++it)
  {
    del.clear();
    for (it2 = it.current()->index_fields.begin(); it2 != it.current()->index_fields.end(); ++it2)
      if (lst.find((*it2).fieldName()) == lst.end())
        del.append(*it2);

    for (del_it = del.begin(); del_it != del.end(); ++del_it)
      it.current()->index_fields.remove(*del_it);

    if (it.current()->Type == PRIMARY)
    {
      for (i = 0; i < fields->numRows(); i++)
      {
        t = fields->text(i, 0);
        if (!t.isEmpty())
          emit primaryKey(i, it.current()->index_fields.find(t) != it.current()->index_fields.end());
        else
          emit primaryKey(i, false);
      }
    }
  }

  for (IndexFieldList::iterator it3 = currentIndex->index_fields.begin(); it3 != currentIndex->index_fields.end(); ++it3)
  {
    t = (*it3).fieldName();
    if ((*it3).length() > 0)
      t += " (" + QString::number((*it3).length()) + ")";
    insertListViewItem(fieldsUsed, t);
  }
  emit do_click(fields->currentRow());
  addField->setEnabled(availableFields->childCount() > 0);
  removeField->setEnabled(fieldsUsed->childCount() > 0);
  deleteIndex->setEnabled(indexName->currentText().lower() != "primary");
}

void CTableIndexes::add(const QString &index_name, IndexType type, IndexFieldList fields)
{
  CIndex *index = 0;
  if (type != PRIMARY)
  {
    indexName->insertItem(index_name);
    indexName->setCurrentText(index_name);
    indexDict.resize(indexName->count());
    index = new CIndex(index_name);
    index->Type = type;
    indexDict.insert(index_name, index);
  }
  else
    index = indexDict.find("PRIMARY");

  for (IndexFieldList::Iterator it = fields.begin(); it != fields.end(); ++it)
    index->index_fields.append(*it);

  if (type != PRIMARY)
    changeType(type);
}

void CTableIndexes::changeType(IndexType t)
{
  QString current = indexName->currentText();
  CIndex *index = indexDict.find(current);
  index->Type = t;    
  int i = indexName->currentItem();
  QPixmap im;
  switch (t)
  {
  case INDEX:
    im = getPixmapIcon("mulIcon");
    break;
  case UNIQUE:      
    im = getPixmapIcon("uniIcon");
    break;
  case FULLTEXT:      
    im = getPixmapIcon("ftIcon");
    break;
  case PRIMARY:
    break;
  }
  indexName->changeItem(im, current, i);
}


void CTableIndexes::singleRemove(const QString &index, const QString &field)
{
  if (field.isEmpty() || index.isEmpty())
   return;

  CIndex *idx = indexDict.find(index);
  idx->index_fields.remove(field);
  refresh();
}

void CTableIndexes::singleAdd(const QString &index, const QString &field)
{
  if (field.isEmpty() || index.isEmpty())
   return;
  
  CIndex *idx = indexDict.find(index);
  if (idx->index_fields.find(field) == idx->index_fields.end())
    idx->index_fields.append(field);    
  refresh();
}

void CTableIndexes::RemoveField()
{
  if (fieldsUsed->childCount() == 0)
    return;
  CIndex *index = indexDict.find(indexName->currentText());

  QListViewItemIterator it(fieldsUsed);
  IndexFieldList::iterator f;
  QString idx_name;
  while (it.current() != 0)
  {
    if (it.current()->isSelected())
    {
      idx_name = it.current()->text(0);
      idx_name = idx_name.left(idx_name.find(" "));
      f = index->index_fields.find(idx_name);
      if (f != index->index_fields.end())
        index->index_fields.remove(f);
    }
    ++it;
  }
  refresh();
}

void CTableIndexes::AddField()
{
  if (availableFields->childCount() == 0)
    return;
  CIndex *index = indexDict.find(indexName->currentText());

  QListViewItemIterator it(availableFields);
  while (it.current() != 0)
  {
    if (it.current()->isSelected())
      index->index_fields.append(it.current()->text(0));
    ++it;
  }
  refresh();
}

void CTableIndexes::AddIndex()
{ 
  bool ok = false;
  QString idx = QInputDialog::getText(tr("New Index"), tr("Please enter the Index Name"), QLineEdit::Normal, QString::null, &ok);
  if (ok && !idx.isEmpty())
  {
    if (idx.lower() == "primary")
    {
      messagePanel->warning(tr("You can not have an Index named 'PRIMARY'"));
      return;
    }
    for (int i = 0; i < indexName->count(); i++)
    {
      if (indexName->text(i).lower() == idx.lower())
      {
        messagePanel->warning(tr("You already have an index called") + ": " + idx);
        return;
      }
    }

    indexName->insertItem(idx);
    indexName->setCurrentText(idx);
    indexDict.resize(indexName->count());
    CIndex *index = new CIndex(idx);
    index->isNewIndex = isAlterTable;
    indexDict.insert(idx, index);
    changeType(INDEX);
    refresh();
  }
}

void CTableIndexes::DeleteIndex()
{
  if (myApp()->confirmCritical())
    if ((QMessageBox::information(0, tr("Confirm Delete"), tr("Are you sure you want to Delete Index") +
		      ": \"" + indexName->currentText() + "\"?", tr("&Yes"),tr("&No")) != 0))
      return;  
  CIndex *index = indexDict.find(indexName->currentText());
  if (!index->isNewIndex)
    deletedIndexes.append("DROP INDEX " + mysql->quote(index->Name) + ", ");
  indexDict.remove(indexName->currentText());
  int i = indexName->currentItem();
  indexName->removeItem(i);
  indexName->setCurrentItem(i - 1);
  refresh();
}

void CTableIndexes::IndexChanged(const QString &)
{
  refresh();
}

void CTableIndexes::UniToggled(bool b)
{
  if (b)
    changeType(UNIQUE);  
}

void CTableIndexes::IdxToggled(bool b)
{
  if (b)
    changeType(INDEX);  
}

void CTableIndexes::FtToggled(bool b)
{
  if (b)
    changeType(FULLTEXT);  
}

QString CTableIndexes::getIndexFields(CIndex *index) const
{
  QString ret = QString::null;
  if (!index->index_fields.isEmpty())
  {
    for (IndexFieldList::Iterator it = index->index_fields.begin(); it != index->index_fields.end(); ++it)    
    {
      ret += mysql->quote((*it).fieldName());
      if ((*it).length() > 0)
        ret += "(" + QString::number((*it).length()) + ")";
      ret += ", ";
    }
    ret = ret.left(ret.length() - 2);
  }
  return ret;
}

QString CTableIndexes::getIndexes(bool alter) const
{
  QString ret = QString::null;
  QDictIterator<CIndex> it(indexDict);
  QString txt;
  QString p;
  for(; it.current(); ++it)
  {
    p = getIndexFields(*it);
    txt = mysql->quote((*it)->Name);
    if (!p.isNull())
    {
      if (alter)
        ret += "ADD ";
      switch ((*it)->Type)
      {
      case PRIMARY: ret += "PRIMARY KEY ";
        break;
      case INDEX: ret += "INDEX " + txt;
        break;
      case UNIQUE: ret += "UNIQUE " + txt;
        break;
      case FULLTEXT: ret += "FULLTEXT " + txt;
        break;
      }
      ret += "(" + p + "), ";
    }
  }
  return ret;
}


QString CTableIndexes::getDropIndexes()
{
  QString ret = QString::null;
  QDictIterator<CIndex> it(indexDict);
  for(; it.current(); ++it)
  {
    if (!it.current()->isNewIndex)
    {
      switch (it.current()->Type)
      {
      case PRIMARY:
        if (has_primary_index)
          ret += "DROP PRIMARY KEY, ";
        break;

      default: ret += "DROP INDEX " + mysql->quote(it.current()->Name) + ", ";
        break;
      }
    }
  }
    
  if (deletedIndexes.count() > 0)
    for (QStringList::Iterator it2 = deletedIndexes.begin(); it2 != deletedIndexes.end(); ++it2)
      ret += *it2;

  return ret;
}


CFieldProperties::CFieldProperties(QWidget* parent,  const char* name, WFlags fl)
: QWidget(parent, name, fl)
{
  if (!name)
    setName("CFieldProperties");
  currentField = 0;
  autoIncField = 0;
  tableWindow = 0;
  intValidator = new QIntValidator(this);
  intValidator->setBottom(1);  
  floatValidator = new QRegExpValidator(QRegExp("\\d+\\s*,\\s*\\d+"), this);

  setCaption(trUtf8("Field Properties"));
  CFieldPropertiesLayout = new QGridLayout(this, 1, 1, 4, 2, "CFieldPropertiesLayout"); 
  
  TextLabel1 = new QLabel(this, "TextLabel1");
  TextLabel1->setText(trUtf8("Default"));
  
  CFieldPropertiesLayout->addWidget(TextLabel1, 1, 0);
  
  TextLabel2 = new QLabel(this, "TextLabel2");
  TextLabel2->setText(trUtf8("Length"));
  
  CFieldPropertiesLayout->addWidget(TextLabel2, 0, 0);
  
  TextLabel3 = new QLabel(this, "TextLabel3");
  TextLabel3->setText(trUtf8("Value"));
  
  CFieldPropertiesLayout->addWidget(TextLabel3, 2, 0);
  
  Default = new CNullLineEdit(this, "Default");
  QWhatsThis::add(Default, trUtf8("Default value for the Field."));
  
  CFieldPropertiesLayout->addMultiCellWidget(Default, 1, 1, 1, 4);
  
  Value = new QLineEdit(this, "Value");
  QWhatsThis::add(Value, trUtf8("This stores the values for SET & ENUM Fields.  Separate each field with commas and when using text values, enclose them with single-quotes."));
  
  CFieldPropertiesLayout->addMultiCellWidget(Value, 2, 2, 1, 4);
  
  Length = new QLineEdit(this, "Length");
  QWhatsThis::add(Length, trUtf8("Maximum Length of the Field.  When empty, the Default Length will be used."));
  
  CFieldPropertiesLayout->addMultiCellWidget(Length, 0, 0, 1, 4);
  
  Unsigned = new QCheckBox(this, "Unsigned");
  Unsigned->setText(trUtf8("UNSIGNED"));
  QWhatsThis::add(Unsigned, trUtf8("Set the UNSIGNED property for this field.  For more information refere to the MySQL manual."));
  
  CFieldPropertiesLayout->addMultiCellWidget(Unsigned, 3, 3, 0, 1);
  
  Binary = new QCheckBox(this, "Binary");
  Binary->setText(trUtf8("BINARY"));
  QWhatsThis::add(Binary, trUtf8("Set the BINARY property for this field.  For more information refere to the MySQL manual."));
  
  CFieldPropertiesLayout->addMultiCellWidget(Binary, 4, 4, 0, 1);
  
  ZeroFill = new QCheckBox(this, "ZeroFill");
  ZeroFill->setText(trUtf8("ZEROFILL"));
  QWhatsThis::add(ZeroFill, trUtf8("Set the ZEROFILL property for this field.  For more information refere to the MySQL manual."));
  
  CFieldPropertiesLayout->addWidget(ZeroFill, 4, 3);
  
  AutoIncrement = new QCheckBox(this, "AutoIncrement");
  AutoIncrement->setText(trUtf8("AUTO_INCREMENT"));
  QWhatsThis::add(AutoIncrement, trUtf8("Set the AUTO_INCREMENT property for this field.  For more information refere to the MySQL manual."));
  
  CFieldPropertiesLayout->addWidget(AutoIncrement, 3, 3);
  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CFieldPropertiesLayout->addItem(spacer, 4, 4);
  QSpacerItem* spacer_2 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CFieldPropertiesLayout->addItem(spacer_2, 5, 3);
  QSpacerItem* spacer_3 = new QSpacerItem(50, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
  CFieldPropertiesLayout->addItem(spacer_3, 3, 2);
  
  skip = true;
  connect(Default, SIGNAL(textChanged(const QString &)), this, SLOT(setDefault(const QString &)));
  connect(Value, SIGNAL(textChanged(const QString &)), this, SLOT(setValue(const QString &)));
  connect(Length, SIGNAL(textChanged(const QString &)), this, SLOT(setLength(const QString &)));
  connect(Unsigned, SIGNAL(toggled(bool)), this, SLOT(setUnsigned(bool)));
  connect(Binary, SIGNAL(toggled(bool)), this, SLOT(setBinary(bool)));
  connect(ZeroFill, SIGNAL(toggled(bool)), this, SLOT(setZeroFill(bool)));
  connect(AutoIncrement, SIGNAL(toggled(bool)), this, SLOT(setAutoIncrement(bool)));
  skip = false;


  // tab order
  setTabOrder(Length, Default);
  setTabOrder(Default, Value);
  setTabOrder(Value, Unsigned);
  setTabOrder(Unsigned, AutoIncrement);
  setTabOrder(AutoIncrement, Binary);
  setTabOrder(Binary, ZeroFill);
}

bool CFieldProperties::isNumberField(const QString &type)
{
  return (type == "bit" || type == "bool" || (type.find ("int") != -1) || type == "real" || 
    type == "double" || type == "float" || type == "decimal" || type == "numeric");
}

bool CFieldProperties::isCharField(const QString &type)
{
  return (type.find("char") != -1);
}

void CFieldProperties::resetLength(CTableWindowField *f, const QString &t)
{  
  if (currentField == 0 || !currentField)
  {
    f->Length = QString::null;
    return;
  }

  QString type = t.lower().stripWhiteSpace();
  QString current_type = currentField->Type.lower().stripWhiteSpace();

  bool ok = false;

  if (isNumberField(type) && isNumberField(current_type))
    ok = true;
  else
    if (isCharField(type) && isCharField(current_type))
      ok = true;

  f->Length = ok ? currentField->Length : QString::null;
}

void CFieldProperties::setHasModified(bool b)
{
  if (tableWindow != 0)
    tableWindow->setHasModified(b);
}

void CFieldProperties::setFieldType(const QString &t)
{
  skip = true;
  QString type = t.lower().stripWhiteSpace();
  if (type == "real" || type == "double" || type == "float" || type == "decimal" || type == "numeric")
    Length->setValidator(floatValidator);    
  else
    Length->setValidator(intValidator);
  
  if (isNumberField(type)) //AUTO_INCREMENT, ZEROFILL, UNSIGNED
  {    
    Value->setEnabled(false);
    Binary->setEnabled(false);
    
    currentField->Value = QString::null;
    currentField->Binary = false;
    
    Length->setEnabled(true);
    Unsigned->setEnabled(true);
    ZeroFill->setEnabled(true);
    AutoIncrement->setEnabled(true);
    Default->setEnabled(true);
  }
  else if (isCharField(type)) //BINARY
    {
      Value->setEnabled(false);
      Unsigned->setEnabled(false);
      ZeroFill->setEnabled(false);
      AutoIncrement->setEnabled(false);
      
      currentField->Value = QString::null;
      currentField->Unsigned = false;
      currentField->ZeroFill = false;
      currentField->AutoIncrement = false;
      
      Length->setEnabled(true);
      Binary->setEnabled(true);
      Default->setEnabled(true);
    }   
  else if (type == "enum" || type == "set") //(value1, value2, ...)
    {
      Length->setEnabled(false);
      Unsigned->setEnabled(false);
      Binary->setEnabled(false);
      ZeroFill->setEnabled(false);
      AutoIncrement->setEnabled(false);
      
      currentField->Length = QString::null;
      currentField->Unsigned = false;
      currentField->Binary = false;
      currentField->ZeroFill = false;
      currentField->AutoIncrement = false;
        
      Value->setEnabled(true);
      Default->setEnabled(true);
    }
  else if ((type.find("blob") != -1) || (type.find("text") != -1))
    {          
      Value->setEnabled(false);          
      Default->setEnabled(false);
      Unsigned->setEnabled(false);
      Binary->setEnabled(false);
      ZeroFill->setEnabled(false);
      AutoIncrement->setEnabled(false);
      Length->setEnabled(false);
          
      currentField->Length = QString::null;
      currentField->Unsigned = false;
      currentField->Binary = false;
      currentField->ZeroFill = false;
      currentField->AutoIncrement = false;
      currentField->Default = "";
      currentField->Value = QString::null;
    }
  else
    {
      Value->setEnabled(false);
      Length->setEnabled(false);
      Unsigned->setEnabled(false);
      Binary->setEnabled(false);
      ZeroFill->setEnabled(false);
      AutoIncrement->setEnabled(false);
         
      currentField->Unsigned = false;
      currentField->Binary = false;
      currentField->ZeroFill = false;
      currentField->AutoIncrement = false;
      currentField->Value = QString::null;
      currentField->Length = QString::null;
          
      bool b = (type != "timestamp");
      Default->setEnabled(b);
      if (!b)
        currentField->Default = "";
    }
  skip = false;
  setHasModified(true);
}

void CFieldProperties::setDefault(const QString &s)
{
  if (currentField == 0 || skip)
    return;

  currentField->Default = Default->isNull() ? QString::null : s;
  setHasModified(true);
}

void CFieldProperties::setValue(const QString &s)
{
  if (currentField == 0 || skip)
    return;
  currentField->Value = s;
  setHasModified(true);  
}

void CFieldProperties::setLength(const QString &s)
{
  if (currentField == 0 || skip)
    return;
  currentField->Length = s;
  setHasModified(true);
}

void CFieldProperties::setUnsigned(bool b)
{
  if (currentField == 0 || skip)
    return;
  currentField->Unsigned = b;
  setHasModified(true);
}

void CFieldProperties::setBinary(bool b)
{
  if (currentField == 0 || skip)
    return;
  currentField->Binary = b;
  setHasModified(true);
}

void CFieldProperties::setZeroFill(bool b)
{
  if (currentField == 0 || skip)
    return;
  currentField->ZeroFill = b;
  setHasModified(true);
}

void CFieldProperties::setAutoIncrement(bool b)
{
  if (currentField == 0 || skip)
    return;  
  currentField->AutoIncrement = b;  
  if (b)
  {
    if (autoIncField != 0)
    {
      messagePanel->warning(tr("You can only have one AUTO_INCREMENT field in a Table.  The AUTO_INCREMENT property of field '" + autoIncField->FieldName + "' was removed."));
      autoIncField->AutoIncrement = false;
    }
    autoIncField = currentField;
  }
  else  
    autoIncField = 0;
  setHasModified(true);
}

void CFieldProperties::setField(CTableWindowField *f)
{
  currentField = f;
  setFieldType(currentField->Type);
  skip = true;
  if (currentField->Default.isNull())
    Default->setNull(true);
  else
  {
    Default->setNull(false);
    Default->setText(currentField->Default);
  }
  Value->setText(currentField->Value);
  Length->setText(currentField->Length);
  Unsigned->setChecked(currentField->Unsigned);
  Binary->setChecked(currentField->Binary);
  ZeroFill->setChecked(currentField->ZeroFill);
  AutoIncrement->setChecked(currentField->AutoIncrement);
  skip = false;
}


CTableWindow::CTableWindow(QWidget* parent, CMySQLServer *m, const QString &dbname, const QString &table_name)
: CMyWindow(parent, "CTableWindow")
{
  myApp()->incCritical();
  (void)statusBar();
  setIcon(getPixmapIcon("applicationIcon"));
  pkIcon = getPixmapIcon("pkIcon");
  emptyPkIcon = getPixmapIcon("emptyPkIcon");
  enableMessageWindow(true);
  hasModified = false;
  hasPaste = false;
  skipValueChanged = true;
  mysql = new CMySQLServer(m->connectionName(), messagePanel());
  mysql->setDatabaseName(dbname);
  mysql->connect();
  isAlterTable = !table_name.isNull();
  if (isAlterTable)
    tableName = table_name;

  setCentralWidget(new QWidget(this, "qt_central_widget"));
  CTableWindowLayout = new QGridLayout(centralWidget(), 1, 1, 4, 2, "CTableWindowLayout"); 
  
  fields = new MyTable(centralWidget(), "fields");  
  fields->setNumCols(3);
  fields->setNumRows(0);
  fields->horizontalHeader()->setLabel(0, tr("Field Name"), 210);
  fields->horizontalHeader()->setLabel(1, tr("Allow NULL"));
  fields->horizontalHeader()->setLabel(2, tr("Data Type"));
  
  CTableWindowLayout->addWidget(fields, 0, 0);
  
  tab = new QTabWidget(centralWidget(), "tab");
  tab->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1, 0, 0, tab->sizePolicy().hasHeightForWidth()));
  
  fieldPropertiesTab = new CFieldProperties(tab, "fieldPropertiesTab");  
  fieldPropertiesTab->setMessagePanel(messagePanel());
  tab->insertTab(fieldPropertiesTab, trUtf8("Field Properties"));
  
  indexesTab = new CTableIndexes(tab, fields, mysql->mysql(), messagePanel(), "indexesTab");
  indexesTab->setIsAlterTable(isAlterTable);
  tab->insertTab(indexesTab, trUtf8("Indexes"));
  
  tablePropertiesTab = new CTblProperties(tab, mysql->mysql(), "tablePropertiesTab");
  tab->insertTab(tablePropertiesTab, trUtf8("Table Properties"));
  
  alterTableOptionsTab = new CAlterTableOptions(tab, fields, mysql, "alterTableOptionsTab");
  tab->insertTab(alterTableOptionsTab, trUtf8("Alter Table Options"));
  
  CTableWindowLayout->addWidget(tab, 1, 0);
   
  CAction *fileSaveAction = new CAction (tr("Save"), getPixmapIcon("saveIcon"),
    tr("&Save"), Qt::CTRL + Qt::Key_S, this, "fileSaveAction");
  fileSaveAction->setParentMenuText(tr("File"));
  
  fileSaveAsAction = new CAction (tr("Save As"), tr("Save &As"), 0, this, "fileSaveAsAction");
  fileSaveAsAction->setParentMenuText(tr("File"));
  
  CAction *fileCloseAction = new CAction (tr("Close"), getPixmapIcon("closeIcon"),
    tr("&Close"), 0, this, "fileCloseAction");
  connect(fileCloseAction, SIGNAL(activated()), this, SLOT(close()));
  fileCloseAction->setParentMenuText(tr("File"));
  
  fileReloadAction = new CAction (tr("Reload"), getPixmapIcon("refreshTablesIcon"),
    tr("&Reload"), Qt::CTRL + Qt::Key_R, this, "fileReloadAction");
  fileReloadAction->setParentMenuText(tr("File"));
  
  viewShowMessagesAction = new CAction (tr("Show Messages"), tr("Show &Messages"), Qt::CTRL + Qt::Key_M,
    this, "fileShowMessagesAction", true);
  connect(viewShowMessagesAction, SIGNAL(toggled(bool)), this, SLOT(showMessages(bool)));
  viewShowMessagesAction->setParentMenuText(tr("View"));
  
  CAction *tableInsertAction = new CAction (tr("Insert Row"), getPixmapIcon("insertRowIcon"),
    tr("&Insert Row"), Qt::CTRL + Qt::Key_I, this, "tableInsertAction");
  tableInsertAction->setParentMenuText(tr("Table"));
  QAccel *tableInsertAccel = new QAccel(this);  
  
  CAction *tableDeleteAction = new CAction (tr("Delete Row"), getPixmapIcon("deleteRowIcon"),
    tr("&Delete Row"), Qt::CTRL + Qt::Key_D, this, "tableDeleteAction");
  tableDeleteAction->setParentMenuText(tr("Table"));
  QAccel *tableDeleteAccel = new QAccel(this);    
  
  tablePrimaryKeyAction = new CAction (tr("Add Primary Key"), pkIcon,
    tr("Add Primary &Key"), Qt::CTRL + Qt::Key_K, this, "tablePrimaryKeyAction");
  tablePrimaryKeyAction->setParentMenuText(tr("Table"));
  
  editCopyAction = new CAction (tr("Copy"), getPixmapIcon("copyIcon"),
    tr("&Copy"), Qt::CTRL + Qt::Key_C, this, "editCopyAction");  
  editCopyAction->setParentMenuText(tr("Edit"));
  
  editPasteAction = new CAction (tr("Paste"), getPixmapIcon("pasteIcon"),
    tr("&Paste"), Qt::CTRL + Qt::Key_V, this, "editPasteAction");
  editPasteAction->setEnabled(false);
  editPasteAction->setParentMenuText(tr("Edit"));
  
  QToolBar * fileBar = new QToolBar(tr("File"), this, DockTop);  
  fileSaveAction->addTo(fileBar);  
  fileReloadAction->addTo(fileBar);
  fileBar->addSeparator();
  editCopyAction->addTo(fileBar);
  editPasteAction->addTo(fileBar);
  
  QToolBar * tableBar = new QToolBar(tr("Table"), this, DockTop);
  tablePrimaryKeyAction->addTo(tableBar);
  tableBar->addSeparator();
  tableInsertAction->addTo(tableBar);
  tableDeleteAction->addTo(tableBar);
  
  QPopupMenu *fileMenu = new QPopupMenu(this);
  fileSaveAction->addTo(fileMenu);
  fileSaveAsAction->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileReloadAction->addTo(fileMenu); 
  fileMenu->insertSeparator();
  fileCloseAction->addTo(fileMenu);
  menuBar()->insertItem(trUtf8("&File"), fileMenu);
  
  QPopupMenu *editMenu = new QPopupMenu(this);  
  
  editCopyAction->addTo(editMenu);
  editPasteAction->addTo(editMenu);
  menuBar()->insertItem(trUtf8("&Edit"), editMenu);
  
  QPopupMenu *viewMenu = new QPopupMenu(this, "ViewMenu");
  connect(viewMenu, SIGNAL(aboutToShow()), this, SLOT(viewMenuAboutToShow()));
  
  viewShowMessagesAction->addTo(viewMenu);
  menuBar()->insertItem(tr("&View"), viewMenu);
  
  QPopupMenu * tableMenu = new QPopupMenu(this);
  tablePrimaryKeyAction->addTo(tableMenu);
  tableMenu->insertSeparator();
  tableInsertAction->addTo(tableMenu);
  tableDeleteAction->addTo(tableMenu);
  menuBar()->insertItem(trUtf8("&Table"), tableMenu);
  new CHotKeyEditorMenu(this, menuBar(), "HotKeyEditor");
  skipValueChanged = false;
  bool is_new = !(mysql->mysql()->version().major <= 3 && mysql->mysql()->version().minor <= 22);
  
  if (loadSyntaxSection(&field_types, 9, true) && !field_types.empty() && is_new)
  {
    defaultFieldType = field_types.findIndex("varchar");
    if (defaultFieldType == -1)
      defaultFieldType = 0;
    defaultFieldLength = "100";
    messagePanel()->information(tr("Field Types loaded correctly."));

    connect(tableInsertAction, SIGNAL(activated()), this, SLOT(insertRow()));
    connect(tableDeleteAction, SIGNAL(activated()), this, SLOT(deleteRow()));
    tableInsertAccel->connectItem(tableInsertAccel->insertItem(Key_Insert), this, SLOT(insertRow()));
    tableDeleteAccel->connectItem(tableDeleteAccel->insertItem(Qt::Key_Delete), this, SLOT(deleteRow()));

    connect(tablePrimaryKeyAction, SIGNAL(activated()), this, SLOT(setPk()));
    connect(fileSaveAction, SIGNAL(activated()), this, SLOT(save()));
    connect(fileSaveAsAction, SIGNAL(activated()), this, SLOT(saveAs()));
    connect(fileReloadAction, SIGNAL(activated()), this, SLOT(reset()));
    connect(editCopyAction, SIGNAL(activated()), this, SLOT(copy()));
    connect(editPasteAction, SIGNAL(activated()), this, SLOT(paste()));
    
    connect(fields, SIGNAL(currentChanged(int, int)), this, SLOT(currentChanged(int, int)));    
    connect(fields, SIGNAL(valueChanged(int, int)), this, SLOT(fieldsValueChanged(int, int)));
    connect(fields, SIGNAL(contextMenuRequested(int, int, const QPoint &)), this, SLOT(showContextMenu(int, int, const QPoint &)));

    connect(tab, SIGNAL(currentChanged(QWidget *)), this, SLOT(emitValueChanged(QWidget *)));
    connect(this, SIGNAL(valueChanged()), indexesTab, SLOT(refresh()));
    connect(this, SIGNAL(valueChanged()), alterTableOptionsTab, SLOT(refresh()));
    connect(indexesTab, SIGNAL(primaryKey(int, bool)), this, SLOT(addPrimaryIcon(int, bool)));
    connect(indexesTab, SIGNAL(do_click(int)), this, SLOT(doClick(int)));
    connect(fields->verticalHeader(), SIGNAL(released(int)), this, SLOT (verticalHeaderReleased(int)));

    reset();
  }
  else
  {
    if (!is_new)
      messagePanel()->critical(tr("This feature is not supported for your mysqld server version.  You should upgrade to a newer version ASAP !!!"));
    else
      messagePanel()->critical(tr("Could not load Field Types.  Please select the location of the Syntax File in 'Options->General->Sql Editor' and re-open this dialog."));
    fileSaveAction->setEnabled(false);
    fileSaveAsAction->setEnabled(false);
    fileReloadAction->setEnabled(false);
    tableInsertAction->setEnabled(false);
    tableDeleteAction->setEnabled(false);
    tablePrimaryKeyAction->setEnabled(false);
    editCopyAction->setEnabled(false);
    editPasteAction->setEnabled(false);
    tab->setTabEnabled(indexesTab, false);
    tab->setTabEnabled(tablePropertiesTab, false);
    tab->setTabEnabled(alterTableOptionsTab, false);
    tab->setTabEnabled(fieldPropertiesTab, false);
    setWindowCaption();
  }
  skipValueChanged = false;
  myResize(550, 500);
  fieldPropertiesTab->setTableWindow(this);
}


CTableWindow::~CTableWindow()
{
  delete mysql;
  if (hasPaste)
    delete pasteField;
  myApp()->decCritical();
}

void CTableWindow::addPrimaryIcon(int row, bool add)
{
  if (add)
    fields->verticalHeader()->setLabel(row, pkIcon, QString::null);
  else
    fields->verticalHeader()->setLabel(row, null_pixmap, " ");
}

void CTableWindow::emitValueChanged(QWidget *w)
{
  if (skipValueChanged)
    return;

  if (w != 0)
  {
    if (w->caption() == indexesTab->caption() || w->caption() == alterTableOptionsTab->caption())
      emit valueChanged();
  }
  else
    emit valueChanged();
}

bool CTableWindow::checkDuplicateNames(const QString &txt, int row, int col)
{
  bool err = false;
  for (int i = 0; i < (int) fields->numRows(); i++)    
    if ((fields->text(i,0) == txt) && (fields->item(i, 2)->row() != row))
    {
      messagePanel()->warning(tr("You already have a Field Name called") + " '" + txt + "'");
      fields->editCell(row, col);
      err = true;
      break;
    }
  return err;
}

void CTableWindow::fieldsValueChanged(int row, int col)
{
  CTableWindowField *f = ((CTableWindowComboItem *)fields->item(row, 2))->Field;
  switch (col)
  {
  case 0: f->FieldName = fields->text(row,col);
          checkDuplicateNames(f->FieldName, row, col);
    break;
  case 1: f->Null = ((QCheckTableItem *) fields->item(row, col))->isChecked();
    break;
  case 2: 
    {
      QString t = fields->text(row, col);
      if (f->Type != t)
      {
        fieldPropertiesTab->resetLength(f, t);
        f->Type = t;
      }
      fieldPropertiesTab->setField(f);
    }
    break;
  }
  hasModified = true;
  emitValueChanged();
}

void CTableWindow::insert(CTableWindowField *f)
{
  if (fields->is_editing())
    fields->finishEdit();
  int currentRow = (mysql->mysql()->version().major > 3 || !isAlterTable) ? fields->currentRow() : fields->numRows();
  int currentColumn = fields->currentColumn();
  if (currentRow == -1 || currentColumn == -1)
    return;
  
  fields->insertRows(currentRow);
  fields->verticalHeader()->setLabel(currentRow, " ");

  CTableWindowComboItem *combo;  
  QCheckTableItem *check = new QCheckTableItem(fields, QString::null);
  if (f != 0)  //paste insert
  {
    hasModified = true;
    combo = new CTableWindowComboItem(fields, f, field_types);
    check->setChecked(f->Null);
    combo->setCurrentItem(f->Type);
    fields->setText(currentRow, 0, f->FieldName);
  }
  else  //plain insert
  {
    combo = new CTableWindowComboItem(fields, field_types);
    combo->setCurrentItem(defaultFieldType);
    combo->Field->Length = defaultFieldLength;
  }
  combo->Field->isNewField = true;
  fields->setItem(currentRow, 1, check);
	fields->setItem(currentRow, 2, combo);
  fields->setCurrentCell(currentRow, currentColumn);  
  currentChanged(currentRow, 0);
  if (f != 0)
    emitValueChanged();
}

void CTableWindow::insertRow()
{ 
  insert(0);  
}

void CTableWindow::verticalHeaderReleased(int r)
{
  //This is needed because Qt 3.0.5 has a bug with isSelected()
  fields->setCurrentCell(r, 0);
}

void CTableWindow::deleteRow()
{
  int currentRow = fields->currentRow();
  int currentColumn = fields->currentColumn();

  if (currentRow == -1 || currentColumn == -1)
    return;
  int numRows = fields->numRows();
  int min = numRows - 1;
  QMap<int, bool> selectedMap;
 
  for (int j = 0; j < numRows; j++)
    if (fields->isRowSelected(j))
      selectedMap.insert(j, true);

  fields->clearSelection();
  if (numRows - selectedMap.count() < 1)
    messagePanel()->warning(tr("You must have at least one field in the table"));
  else
  {
    hasModified = true;
    int factor = 0;
    int row;
  
    for (QMap<int, bool>::Iterator it = selectedMap.begin(); it != selectedMap.end(); ++it)
    {
      row = it.key() - factor++;
      if (isAlterTable)
      {
        CTableWindowField *f = ((CTableWindowComboItem *)fields->item(row, 2))->Field;
        if (!f->isNewField)
          deletedFields.append(f->OriginalFieldName);
      }
      if (row < min)
        min = row;
      fields->removeRow(row);  
    }
  }  
  min = min < 1 ? 0 : min - 1;  
  fields->setCurrentCell(min, currentColumn);
  currentChanged(min, 0);
  fields->adjustRow(0);
  emitValueChanged();
}

void CTableWindow::setPk()
{
  setPrimaryKey(fields->currentRow(), fields->verticalHeader()->label(fields->currentRow()).isNull());
}

void CTableWindow::setPrimaryKey(int row, bool add)
{
  QString txt = fields->text(row, 0);
  if (!txt.isEmpty())
  {
    if (!add)
      indexesTab->singleAdd("PRIMARY", txt);
    else
      indexesTab->singleRemove("PRIMARY", txt);
    currentChanged(row, 0);
  }
}

bool CTableWindow::exec(const QString &qry)
{
  CMySQLQuery *query = new CMySQLQuery(mysql->mysql());
  query->setEmitMessages(false);
  bool ret = query->execStaticQuery(qry);	
  delete query;
  return ret;
}

QString CTableWindow::getFieldSpecs(CTableWindowField *field, QStringList &dups, bool &ok) const
{
  QString sql = QString::null;  
  if (!field->FieldName.isEmpty() && !field->FieldName.isNull())
  {    
    if (dups.findIndex(field->FieldName) != -1)  //check for duplicate names
    {
      messagePanel()->critical(tr("You already have a Field Name called") + " '" + field->FieldName + "'");
      ok = false;
      return QString::null;      
    }
    else
      dups.append(field->FieldName);
    QString type = field->Type.lower();
    sql += mysql->mysql()->quote(field->FieldName) + " ";
    sql += field->Type;      
    if (type == "varchar" && field->Length.isEmpty())  //varchar requires length to be defined
    {
      messagePanel()->warning(tr("All VARCHAR fields must have Length defined...Using the default field length."));
      field->Length = defaultFieldLength;
    }

    if (!field->Length.isEmpty())
    {
        field->Length = field->Length.stripWhiteSpace().simplifyWhiteSpace();        
        if ((type == "double" || type == "real") && field->Length.find(',') == -1)
          field->Length += ",0";
        sql += "(" + field->Length + ")";
    }
    else
    {
      if (type == "enum" || type == "set")
      {
        if (!field->Value.isEmpty())
        {          
          QStringList valueList(QStringList::split(",", field->Value, true));
          field->Value = QString::null;
          QString value;
          QChar c;
          for (QStringList::Iterator it = valueList.begin(); it != valueList.end(); ++it)
          {
            value = (*it).stripWhiteSpace();
            if (!value.isEmpty())
            {
              c = value.at(0);
              if ((c != '\'' && c != '\"') || ((c == '\'' || c == '\"') && value.at(value.length() - 1) != c))                
                value = "'" + mysql->mysql()->codec()->fromUnicode(mysql->mysql()->escape(value)) + "'";
              field->Value += value + ",";
            }
            else
              field->Value += "'',";
          }            
          field->Value = field->Value.left(field->Value.length() - 1);
          sql += "(" + field->Value + ")";            
        }
        else
        {
          messagePanel()->critical(tr("All ENUM and SET fields must have a Value separated by commas."));
          ok = false;
          return QString::null;
        }
      }        
    }
    sql += " ";
    
    if (field->Unsigned)
      sql += "UNSIGNED ";
    
    if (field->ZeroFill)
      sql += "ZEROFILL ";
    
    if (field->Binary)
      sql += "BINARY ";
    
    if (!field->Null)
      sql += "NOT NULL ";
    
    bool skipDefault = false;

    if (field->Type == "enum" || field->Type == "set")
    {
      if (field->Default.isEmpty())
        skipDefault = true;
    }
    else
      if ( field->Type.find("blob") != -1 || field->Type.find("text") != -1 || field->Type == "timestamp" || field->AutoIncrement)
        skipDefault = true;

    if (!skipDefault)
    {
      sql += "DEFAULT ";

      if (field->Default.isNull())
        sql += "NULL ";
      else
      {
        sql += "'";
        if (!field->Default.isEmpty())
          sql += mysql->mysql()->codec()->fromUnicode(mysql->mysql()->escape(field->Default));
        sql += "' ";
      }
    }

    if (field->AutoIncrement)
      sql += "AUTO_INCREMENT ";    
  }
  ok = true;
  return sql;
}

bool CTableWindow::alterTable()
{
  QString sql = "ALTER TABLE ";
  if (alterTableOptionsTab->ignore->isChecked())
    sql += "IGNORE ";
  sql += mysql->mysql()->quote(tableName) + " ";
  bool ok = false;
  bool hasFields = false;
  bool isFirst = true;
  QStringList dups;
  QString pos;
  QString aboveFieldName = QString::null;
  int ver = mysql->mysql()->version().major;
  
  sql += indexesTab->getDropIndexes();
  for (QStringList::Iterator it = deletedFields.begin(); it != deletedFields.end(); ++it)
    sql += "DROP " + mysql->mysql()->quote(*it) + ", ";  

  QString s;
  for (int i = 0; i < fields->numRows(); i++)
  {    
    CTableWindowField *field = ((CTableWindowComboItem *)fields->item(i, 2))->Field;
    s = getFieldSpecs(field, dups, ok);   
    if (!ok)
      break;
    else
      if (!s.isNull())
      {
        hasFields = true;
        if (field->isNewField)
          sql += "ADD ";
        else        
          sql += "CHANGE " + mysql->mysql()->quote(field->OriginalFieldName) + " ";

        sql += s;

        if (ver > 3)
        {
          if (isFirst)
          {
            sql += "FIRST";
            isFirst = false;
          }
          else
            sql += "AFTER " + aboveFieldName;
        }        
        sql += ",\n";
        aboveFieldName = mysql->mysql()->quote(field->FieldName);
      }
  }

  if (ok)
  {
    if (hasFields)
    {
      sql += indexesTab->getIndexes(true);      
      sql += tablePropertiesTab->getTableProperties();
      sql += alterTableOptionsTab->getAlterTableOptions();
      QString t = tablePropertiesTab->tableName->text().stripWhiteSpace();
      if (t.isEmpty())
      {
        messagePanel()->warning(tr("The original Table Name will be used"));
        t = tableName;
      }
      else
        if (t != tableName) 
          sql += ", RENAME " + mysql->mysql()->quote(t);        
      if (ok = exec(sql))
      {
        tableName = t;
        tablePropertiesTab->tableName->setText(tableName);
      }
    }
    else   
    {
      messagePanel()->critical(tr("You must have at least one field in the table"));
      ok = false;
    }
  }  
  return ok;
}

bool CTableWindow::createTable(const QString &tblName)
{
  tableName = tblName;
  QString sql = "CREATE TABLE " + mysql->mysql()->quote(tblName) + " (";  
  bool ok = false;
  bool hasFields = false;
  QStringList dups;
  dups.clear();
  QString s;
  for (int i = 0; i < fields->numRows(); i++)
  {
    CTableWindowField *field = ((CTableWindowComboItem *)fields->item(i, 2))->Field;
    s = getFieldSpecs(field, dups, ok);
    if (!ok)
      break;
    else    
      if (!s.isNull())
      {
        hasFields = true;
        sql += s + ", ";
      }
  }
  if (ok)
  {
    if (hasFields)
    {
      sql += indexesTab->getIndexes();
      sql = sql.left(sql.length() - 2);
      sql += ") " + tablePropertiesTab->getTableProperties();
      ok = exec(sql);
    }
    else   
    {
      messagePanel()->critical(tr("You must have at least one field in the table"));
      ok = false;
    }
  }
  return ok;
}

bool CTableWindow::save()
{
  bool ok = false;
  if (fields->is_editing())
    fields->finishEdit();
  if (!isAlterTable)
  {
    QString tblName = tablePropertiesTab->tableName->text().stripWhiteSpace();
    if (tblName.isEmpty())
    {
      tblName = QInputDialog::getText(tr("Table Name"), tr("Please enter the Table Name"), QLineEdit::Normal, QString::null, &ok);
      tblName = tblName.stripWhiteSpace();
      if (!ok || tblName.isEmpty())
        return false;
      else
        tablePropertiesTab->tableName->setText(tblName);
    }
    ok = createTable(tblName);
    if (ok)
    {
      tableName = tblName;
      messagePanel()->information(tr("Table created successfully."));
    }
  }
  else
  {
    ok = alterTable();
    if (ok)
    {
      messagePanel()->information(mysql->mysql()->mysqlInfo());
      messagePanel()->information(tr("Table modified successfully."));
    }
  }
  if (ok)
  {
    isAlterTable = true;
    hasModified = false;
    emit do_refresh();
    reset();
  }
  return ok;
}

void CTableWindow::saveAs()
{
  if (fields->is_editing())
    fields->finishEdit();
  hasModified = false;
  bool ok = false;  
  QString tblName = QInputDialog::getText(tr("Save Table As"), tr("Please enter the new Table Name"), QLineEdit::Normal, QString::null, &ok);
  tblName = tblName.stripWhiteSpace();
  if (ok && createTable(tblName))
  {
    messagePanel()->information(tr("Table created successfully."));
    hasModified = false;
    isAlterTable = true;
    emit do_refresh();
    reset();
  }
}


void CTableWindow::currentChanged(int row, int)
{
  if( !fields->item(row,2)) /* currentChanged() is called when new, null row is inserted */
    return;
  
  CTableWindowField *f = ((CTableWindowComboItem *)fields->item(row, 2))->Field;
  if (f->Type.isNull())
    f->Type = fields->text(row, 2);
  fieldPropertiesTab->setField(f);

  if (fields->verticalHeader()->label(row).isNull())
  {    
    tablePrimaryKeyAction->setText(trUtf8("Drop Primary Key"));
    tablePrimaryKeyAction->setMenuText(trUtf8("Dr&op Primary Key"));
    tablePrimaryKeyAction->setIconSet(emptyPkIcon);
  }
  else
  {   
    tablePrimaryKeyAction->setText(trUtf8("Add Primary Key"));
    tablePrimaryKeyAction->setMenuText(trUtf8("Add Primary &Key"));
    tablePrimaryKeyAction->setIconSet(pkIcon);
  }
}


void CTableWindow::reset()
{
  fields->setRowMovingEnabled(mysql->mysql()->version().major > 3 || !isAlterTable);
  fields->setNumRows(0);
  setWindowCaption();
  fileReloadAction->setEnabled(isAlterTable);
  fileSaveAsAction->setEnabled(isAlterTable);
  tab->setTabEnabled(alterTableOptionsTab, isAlterTable);
  alterTableOptionsTab->ignore->setEnabled(mysql->mysql()->version().major >= 4);
  if (hasPaste)
  {
    delete pasteField;
    hasPaste = false;
  }
  QStringList keys;
  QIntDict<CTableWindowField> *fieldsDict = 0;
  deletedFields.clear();
  indexesTab->clear();
  if (isAlterTable)
  {
    keys.clear();
    CMySQLQuery query(mysql->mysql());
    query.setEmitMessages(false);
    query.execStaticQuery("SET SQL_QUOTE_SHOW_CREATE = 1");
    QString createQuery = QString::null;
    if (query.exec("SHOW CREATE TABLE " + mysql->mysql()->quote(tableName)))
    {     
      query.next();
      createQuery = query.row(1);
    }
    else
      return;
    
    tablePropertiesTab->tableName->setText(tableName);
    QStringList lines = QStringList::split("\n", createQuery);
    uint cnt = 0;
    bool hasquote = false;
    bool hasparen = false;
    QRegExp dblQuoteRegExp = QRegExp("''");
    int p;
    QChar c;
    QString line;
    fieldsDict = new QIntDict<CTableWindowField>(lines.count());
    fieldsDict->setAutoDelete(true);
    for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)  //parse the query
    {
      if (it == lines.begin())  //create definition
        continue;
      else
        if (it == lines.fromLast())  //table options
        {
          QStringList opt;
          line = QString::null;
          c = 0;
          int sep;
          QString option;
          QString value;
          QString str = (*it).stripWhiteSpace();
          hasquote = false;
          for (uint i = 0; i < str.length(); i++)
          {
            c = str.at(i);            
            if (c == '\'' || c == '\"')
              hasquote = !hasquote;

            if ((!hasquote && c == ' ' || i == str.length() - 1) && !line.isEmpty())
            {
              if (i == str.length() - 1)
                line += c;
              sep = line.find('=');
              option = line.left(sep).stripWhiteSpace().lower();
              value = line.mid(sep+1).stripWhiteSpace();
              if (value.isEmpty() || option.isEmpty())  //This should never happen
                continue;
              if (option == "min_rows")
                tablePropertiesTab->minRows->setText(value);
              else if (option == "max_rows")
                tablePropertiesTab->maxRows->setText(value);
              else if (option == "avg_row_length")
                tablePropertiesTab->avgRowLength->setText(value);
              else if (option == "pack_keys")
                tablePropertiesTab->packKeys->setCurrentText(value);
              else if (option == "checksum")
                tablePropertiesTab->checksum->setCurrentText(value);
              else if (option == "delay_key_write")
                tablePropertiesTab->delayKeyWrite->setCurrentText(value);
              else if (option == "row_format")
                tablePropertiesTab->rowFormat->setCurrentText(value);
              else if (option == "auto_increment")
                tablePropertiesTab->autoIncrement->setText(value);
              else if (option == "comment")
              {
                QString comments = value.mid(1, value.length() - 2);
                comments = comments.replace(dblQuoteRegExp, "'");
                tablePropertiesTab->comment->setText(comments);
              }
              else if (option == "type")
                for (int x = 0; x < tablePropertiesTab->tableType->count(); x++)
                  if (tablePropertiesTab->tableType->text(x).lower() == value.lower())
                  {
                    tablePropertiesTab->tableType->setCurrentItem(x);
                    break;
                  }
              line = QString::null;
            }
            else
              line += c;
          }          
        } //table options
        else  //fields or indexes
        {
          line = (*it).stripWhiteSpace();          
          if (line.right(1) == ",")
            line = line.left(line.length() - 1);
          if (line.at(0) == '`')  //its a field
          {
            CTableWindowField *f = new CTableWindowField();
            line = line.mid(1);
            p = line.find('`');
            f->FieldName = line.left(p);
            line = line.mid(p + 1).stripWhiteSpace();
            hasquote = false;
            hasparen = false;
            for (uint x = 0; x < line.length(); x++)
            {
              c = line.at(x);
              if (c == '\'')
                hasquote = !hasquote;
              else if (c == '(' && !hasquote)
                hasparen = true;
              else if (c == ')' && !hasquote)
                hasparen = false;

              if (x == line.length() - 1)  //No options besides the field-type where found.
              {
                p = x;
                break;
              }
              else
                if (!hasquote && !hasparen && c == ' ')
                {
                  p = x - 1;
                  break;
                }
            }            
            f->Type = line.left(p + 1);
            line = line.mid(p + 1).stripWhiteSpace();
            p = f->Type.find('(');
            if (p != -1) // type has length or value;
            {
              f->Length = f->Type.mid(p + 1);
              f->Length = f->Length.left(f->Length.findRev(')'));
              f->Type = f->Type.left(p);
              if (f->Type.lower() == "enum" || f->Type.lower() == "set")
              {
                f->Value = f->Length;
                f->Length = QString::null;
              }
            }
            p = line.find("'");
            if (p != -1) //has default ... auto_increment fields can't have default !
            {
              f->Default = line.mid(p + 1);
              line = line.left(p);
              f->Default = f->Default.left(f->Default.findRev('\''));
              f->Default = f->Default.replace(dblQuoteRegExp, "'");
            }
            else
              if (line.find("default NULL", 0, false) != -1)
                f->Default = QString::null;
              else
                f->Default = "";
            line = line.lower();
            f->Unsigned = (line.find("unsigned") != -1);
            f->ZeroFill = (line.find("zerofill") != -1);
            f->AutoIncrement = (line.find("auto_increment") != -1);
            f->Binary = (line.find("binary") != -1);
            f->Null = (line.find("not null") == -1);
            fieldsDict->insert(cnt, f);
            cnt++;
          }
          else  //must be an index
            keys.append(*it);          
        }

    }  //end parse query
    fields->setNumRows(cnt);
    fileReloadAction->setEnabled(true);
    fileSaveAsAction->setEnabled(true);
  }  //isAlterTable
  else  // is New Table
    fields->setNumRows(50);

  for (int i = 0; i < fields->numRows(); ++i)
  {
    fields->verticalHeader()->setLabel(i, " ");
    CTableWindowComboItem * combo = 0;    
    QCheckTableItem *check = new QCheckTableItem(fields, QString::null);
    if (isAlterTable)
    {
      combo = new CTableWindowComboItem(fields, fieldsDict->find(i), field_types);
      combo->setCurrentItem(combo->Field->Type);      
      check->setChecked(combo->Field->Null);
      fields->setText(i, 0, combo->Field->FieldName);
      combo->Field->OriginalFieldName = combo->Field->FieldName;  //ALTER TABLE CHANGE originalFieldName FieldName ...      
    }
    else
    {
      combo = new CTableWindowComboItem(fields, field_types);
	    combo->setCurrentItem(defaultFieldType);
      combo->Field->Length = defaultFieldLength;
    }
    fields->setItem(i, 1, check);
	  fields->setItem(i, 2, combo);
  }

  if (isAlterTable)
  {
    alterTableOptionsTab->refresh();
    QString t;
    QString k;
    QString f;
    QString len;
    QString name;
    IndexType type;
    QStringList flds;
    IndexFieldList index_fields;
    CTableWindowComboItem *ft;
    QChar c;
    uint n;

    has_primary_index=false;
    for (QStringList::Iterator idx = keys.begin(); idx != keys.end(); ++idx)
    {
      k = (*idx).stripWhiteSpace();      
      t = k.left(k.find(' ')).lower();
      if (t == "primary")
      {
        has_primary_index=true;
        type = PRIMARY;
        name = "PRIMARY";
        k = k.mid(k.find('(') + 1);
      }
      else
      {
        if (t == "unique")
          type = UNIQUE;
        else if (t == "key")
          type = INDEX;
        else
          type = FULLTEXT;
        k = k.mid(k.find('`') + 1);        
        name = k.left(k.find('`'));
        k = k.mid(k.find('(') + 1);
      }      
      k = k.left(k.findRev(')'));
      flds = QStringList::split(",", k);
      index_fields.clear();
      for (QStringList::Iterator fld = flds.begin(); fld != flds.end(); ++fld)
      {
        f = QString::null;
        len = QString::null;
        int cnt = 0;
        for (n = 0; n < (*fld).length(); n++)
        {
          c = (*fld).at(n);
          if (c != '`')
          {
            if (cnt < 2)
              f += c;
            else
              if (c != '(' && c != ')')
                len += c;
          }
          else
            cnt++;
        }
        CIndexField idx_field(f);
        if (!len.isEmpty())
          idx_field.setLength(len.toInt());

        for (n = 0; n < (uint) fields->numRows(); n++)
        {
          ft = (CTableWindowComboItem *)fields->item(n, 2);
          if (ft->Field->FieldName == f)
          {
            idx_field.setFieldType(ft->Field->Type);
            break;
          }
        }
        index_fields.append(idx_field);
      }
      indexesTab->add(name, type, index_fields);
    }
    indexesTab->indexName->setCurrentItem(0);
    delete fieldsDict;
  }
  fields->setCurrentCell(0, 0);
  fields->adjustRow(0);
  currentChanged(0, 0);
  emitValueChanged();
}

void CTableWindow::doClick(int row)
{
  currentChanged(row, 0);
}

void CTableWindow::copy()
{
  if (hasPaste)
    delete pasteField;

  if (fields->currentRow() < 0)
  {
    hasPaste = false;
    editPasteAction->setEnabled(false);
    return;
  }

  pasteField = new CTableWindowField(*((CTableWindowComboItem *)fields->item(fields->currentRow(), 2))->Field);
  if (pasteField->AutoIncrement)
    pasteField->AutoIncrement = false;
  hasPaste = true;
  editPasteAction->setEnabled(true);
}

void CTableWindow::paste()
{
  if (hasPaste)
  {
    insert(pasteField);
    emitValueChanged();
  }
}

void CTableWindow::setWindowCaption()
{
  QString c = "[" + mysql->connectionName() + "] ";
  if (isAlterTable)
    c += tr("Editing Table") + " '" + tableName + "' ";
  else
    c += tr("Creating Table") + " ";
  c += tr("in database") + " '" + mysql->databaseName() + "'"; 
  setCaption(c);
}

void CTableWindow::viewMenuAboutToShow()
{
#ifdef DEBUG
  qDebug("CTableWindow::viewMenuAboutToShow()");
#endif
  
  viewShowMessagesAction->setOn(messageWindow()->isVisible());
}

void CTableWindow::showMessages(bool b)
{
#ifdef DEBUG
  qDebug("CConsoleWindow::showMessages(%s)", debug_string(booltostr(b)));
#endif
  
  if (b)
    messageWindow()->show();
  else
    messageWindow()->hide();  
}

void CTableWindow::showContextMenu(int row, int, const QPoint &pos)
{
  QPopupMenu *menu = new QPopupMenu();
  bool isPrimaryKey = fields->verticalHeader()->label(row).isNull();
  if (isPrimaryKey)
    menu->insertItem(emptyPkIcon, tr("Dr&op Primary Key"), 0);
  else
    menu->insertItem(pkIcon, tr("Add Primary &Key"), 0);

  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("insertRowIcon"), tr("&Insert Row"), 1);
  menu->insertItem(getPixmapIcon("deleteRowIcon"), tr("&Delete Row"), 2);
  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("copyIcon"), tr("&Copy"), 3);
  menu->insertItem(getPixmapIcon("pasteIcon"), tr("&Paste"), 4);
  menu->insertSeparator();
  menu->insertItem(getPixmapIcon("refreshTablesIcon"), tr("&Reload"), 5);
  
  if (row < 0)
  {
    menu->setItemEnabled(0, false);
    menu->setItemEnabled(2, false);
    menu->setItemEnabled(3, false);
  }
  menu->setItemEnabled(4, hasPaste);
  menu->setItemEnabled(5, isAlterTable);

  int res = menu->exec(pos);
  delete menu;
  switch (res)
  {
  case 0: setPrimaryKey(row, isPrimaryKey);
    break;
  case 1: insertRow();
    break;
  case 2: deleteRow();
    break;
  case 3: copy();
    break;
  case 4: paste();
    break;
  case 5: reset();
    break;
  }  
}

void CTableWindow::closeEvent(QCloseEvent * e)
{
  if (hasModified && myApp()->confirmCritical()) 
  {
    if ((QMessageBox::information(0, tr("Save Changes"), tr("Do you want to save your changes ?"),
      tr("&Yes"), tr("&No")) == 0))
      if (!save())        
      {
        e->ignore();
        return;
      }
  }  
  e->accept();
  CMyWindow::closeEvent(e);
}
