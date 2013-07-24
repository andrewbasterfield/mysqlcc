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
#ifndef CTABLEGROUPITEM_H
#define CTABLEGROUPITEM_H

#include "CDatabaseListViewItem.h"
#include <stddef.h>
#include <qmap.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PopupMenu>


class CTableGroupItem : public CDatabaseListViewItem
{
  Q_OBJECT

public:
  CTableGroupItem(CDatabaseListViewItem * parent, const QString &dbname, CMySQLServer *m);
  CTableGroupItem(CDatabaseListView * parent, const QString &dbname, const QString &connection_name, CMySQLServer *m);
  ~CTableGroupItem();
  void activated();
  void refreshWidget(bool b);
  void refreshOnActivate() { refresh_on_activate = true; }

  bool isOldMySQL() const { return is_old_mysql; }

  void setCurrentTableName(const QString &s) { current_table_name = s; }
  QString databaseName() const { return database_name; }

  //The below declarations are used to save memory in CTableItem

  Q3PopupMenu *toolsMenu() const { return tools_menu; }
  Q3PopupMenu *openTableMenu() const { return open_table_menu; }
  QPixmap toolsIcon() const { return tools_icon; }
  QPixmap tableIcon() const { return table_icon; }
  QPixmap refreshTablesIcon() const { return refresh_tables_icon; }
  QPixmap openTableIcon() const { return open_table_icon; }
  QPixmap designIcon() const { return design_icon; }
  QPixmap propertiesIcon() const { return properties_icon; }
  QPixmap exportTableIcon() const { return export_table_icon; }
  QPixmap trashIcon() const { return trash_icon; }
  QPixmap deleteIcon() const { return delete_icon; }
  QPixmap renameTableIcon() const { return rename_table_icon; }  
  
public slots:
  void processMenu(int res);

private:
  bool refresh_on_activate;
  void insertWidgetData(QWidget *w, int id, int row, int col, const QString &str);
  Q3PopupMenu * tools_menu;
  Q3PopupMenu * open_table_menu;
  void initMenuItems();
  void init(const QString &dbname);
  QString database_name;
  QPixmap newTableIcon;
  QPixmap tools_icon;
  QPixmap importTableIcon;
  QPixmap refresh_tables_icon;
  QPixmap newWindowIcon;
  QPixmap table_icon;

  QPixmap open_table_icon;
  QPixmap design_icon;
  QPixmap properties_icon;
  QPixmap export_table_icon;
  QPixmap trash_icon;
  QPixmap delete_icon;
  QPixmap rename_table_icon;

  QMap<int, QString> widgetData;
  bool retrieveShowTableStatus;
  bool dont_raise_widget;
  bool is_old_mysql;
  QString current_table_name;
  int real_numRows;
  int real_numCols;
};

#endif
