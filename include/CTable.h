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
#ifndef CTABLE_H
#define CTABLE_H

#include <stddef.h>
#include <qvariant.h>
#include <q3table.h>
#include <q3intdict.h>
#include <qclipboard.h>
#include <qmap.h>
//Added by qt3to4:
#include <QPixmap>

class QPixmap;
class CMySQLServer;

class CTable : public Q3Table
{
  Q_OBJECT
public:
  CTable (QWidget * parent = 0, const char * name = 0);
  virtual bool isBlocked() { return blocked; }
  static void saveTableContentsToFile(Q3Table *table, CMySQLServer *m, QString &fname, const QString &ext, const QString &txt, const QString &title);
  QWidget *realParent() const { return real_parent; }
  void setRealParent(QWidget *w) { real_parent = w; }
  bool hasProcessEvents() { return enable_process_events; }
  bool keepColumnWidth() const { return keep_column_width; }
  bool forceCopyAll() const { return force_copy_all; }
  virtual QString copy_data(int row, int col);
  void applyLastSort();

signals:
  void about_to_close();

public slots:
  void setKeepColumnWidth(bool b) { keep_column_width = b; }
  void setBlocked(bool);
  void enableProcessEvents(bool b) { enable_process_events = b; }
  virtual void copy(int row, int col);
  virtual void sort(int c);
  virtual void reset();
  virtual void save() {}
  virtual void setNumCols(int r);
  virtual void aboutToClose();
  void setForceCopyAll(bool b) { force_copy_all = b; }

protected slots:
  virtual void Clicked(int, int, int, const QPoint &) {};
  virtual void DoubleClicked(int, int, int, const QPoint &) {};
  virtual void ContextMenuRequested(int, int, const QPoint &) {};
  void select_all();

protected:
  struct SortableTableItem
  {
    Q3TableItem *item;
  };

  class OldColumn
  {
  public:
    OldColumn(int s=0, const QString &l=QString::null)
    : size(s), label(l)
    {
    }

    OldColumn(const OldColumn &c)
    {
      size = c.size;
      label = c.label;
    }

    bool operator == (const OldColumn &c) const
    {
      return (c.size == size && c.label == label);
    }

    int size;
    QString label;
  };

  virtual QString copy_current_selection_func(int row, int col);
  static int cmpIntTableItems(const void *n1, const void *n2);
  static int cmpTableItems(const void *n1, const void *n2);
  void mySortColumn(int col, int (*sort_func)(const void *, const void *), bool ascending=true, bool wholeRows=false);
  QWidget *real_parent;
  QMap<uint, OldColumn> previous_columns_map; 

private:
  bool blocked;
  bool force_copy_all;
  bool enable_process_events;
  bool keep_column_width;
  QMap<int, bool> header_sort;
  QPixmap nothingIcon;  
  int last_sort;
  bool last_sort_dir;
};

#endif
