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
#include "CTable.h"
#include "globals.h"
#include "CMySQLServer.h"
#include "config.h"
#include "CApplication.h"
#include <qdatetime.h>
#include <stdlib.h>
#include <qaccel.h>

#ifdef __BCPLUSPLUS__
#include <search.h>
#endif

CTable::CTable(QWidget * parent, const char * name)
: QTable(parent, name)
{
#ifdef DEBUG
  qDebug("CTable::CTable()");
#endif

  if (!name)
    setName("CTable");
  nothingIcon = getPixmapIcon("nothingIcon");
  setMargin(0);
  setLineWidth(1);
  real_parent = 0;
  blocked = false;
  force_copy_all = false;
  keep_column_width = false;
  enable_process_events = true;
  connect(horizontalHeader(), SIGNAL(clicked(int)), this, SLOT (sort(int)));
  connect(this, SIGNAL(clicked(int, int, int, const QPoint &)), this, SLOT(Clicked(int, int, int, const QPoint &)));
  connect(this, SIGNAL(doubleClicked(int, int, int, const QPoint &)), this, SLOT(DoubleClicked(int, int, int, const QPoint &)));
  connect(this, SIGNAL(contextMenuRequested(int, int, const QPoint &)), this, SLOT(ContextMenuRequested(int, int, const QPoint &)));
  last_sort = -1;
  last_sort_dir = false;
  
  QAccel *accel = new QAccel(this);
  accel->connectItem(accel->insertItem(Qt::CTRL + Qt::Key_A), this, SLOT(select_all()));
}

void CTable::select_all()
{
/*  if (numRows() > 0 && numCols() > 0)
  {
    clearSelection();
    QTableSelection sel;    
    sel.init(0,0);
    sel.expandTo(numRows() -1 , numCols() - 1);
    updateHeaderStates();    
  }
*/
}

void CTable::setBlocked(bool b)
{
#ifdef DEBUG
  qDebug("CTable::setBlocked(%s)", debug_string(booltostr(b)));
#endif

  setCursor(b ? Qt::WaitCursor : Qt::ArrowCursor);
  blocked = b;
  if (enable_process_events)
    qApp->processEvents();
}

void CTable::aboutToClose()
{
  emit about_to_close();
}

QString CTable::copy_current_selection_func(int row, int col)
{
  return text(row, col);
}

void CTable::copy(int row, int col)
{
#ifdef DEBUG
  qDebug("CTable::copy(int, int)");
#endif

#ifndef QT_NO_CLIPBOARD
  QApplication::clipboard()->setText(copy_data(row, col));
#endif
}

QString CTable::copy_data(int row, int col)
{
  if (currentSelection() == -1 && !forceCopyAll())
    return copy_current_selection_func(row, col);
  else
  {
    QTableSelection sel;
    if (currentSelection() == -1 || forceCopyAll())
    {
      sel.init(0, 0);
      sel.expandTo(numRows() -1, numCols() - 1);
    }
    else
      sel = selection(currentSelection());

    if (sel.topRow() == sel.bottomRow() && sel.leftCol() == sel.rightCol() && !forceCopyAll())
      return copy_current_selection_func(row, col);
   
    setBlocked(true);
    QString cpy;
    QString separator = "+";
    int current_col;
    uint length;
    QMap<uint, ulong> max_length_map; 
    QString tmp;
    
    for (current_col = sel.leftCol(); current_col <= sel.rightCol(); current_col++)
    {
      if (horizontalHeader()->sectionSize(current_col) <= 0)
        continue;
      length = max(strlen(NULL_TEXT), horizontalHeader()->label(current_col).length());
      max_length_map.insert(current_col, length + 1);
      for (uint i = 0; i < min(max_length_map[current_col] - 1,MAX_COLUMN_LENGTH) + 2; i++)
        separator += "-";
      separator += "+";
    }
    
    separator += "\n";
    cpy = separator + "|";
    
    for (current_col = sel.leftCol(); current_col <= sel.rightCol(); current_col++)
    {
      if (horizontalHeader()->sectionSize(current_col) <= 0)
        continue;
      tmp.sprintf(" %-*s|",min((int) max_length_map[current_col], MAX_COLUMN_LENGTH), horizontalHeader()->label(current_col).ascii());
      cpy += tmp;
    }
    cpy += "\n" + separator;  
    
    for (int current_row = sel.topRow(); current_row <= sel.bottomRow(); current_row++)
    {
      cpy += "|";
      for (current_col = sel.leftCol(); current_col <= sel.rightCol(); current_col++)
      {
        if (horizontalHeader()->sectionSize(current_col) <= 0)
          continue;
        tmp = text(current_row, current_col);
        const char *str = tmp.isNull() ? NULL_TEXT : tmp.ascii();
        length = max_length_map[current_col];
        if (length > MAX_COLUMN_LENGTH)
        {
          cpy += str;
          cpy += "|";
        }
        else
        {
          tmp.sprintf(" %-*s|", length, str);
          cpy += tmp;
        }
      }
      cpy += "\n";
    }
    setBlocked(false);
    return cpy + separator;
  }
}

void CTable::reset()
{
#ifdef DEBUG
  qDebug("CTable::reset()");
#endif

  bool b = isBlocked();
  if (!b)
    setBlocked(true);

  if (keep_column_width && numCols() > 0)
    for (int i = 0; i < numCols(); i++)
      previous_columns_map.insert(i, OldColumn(horizontalHeader()->sectionSize(i), horizontalHeader()->label(i)));

  clearCellWidget(currentRow(), currentColumn());
  ensureVisible(0, 0);
  verticalScrollBar()->setValue(0);
  setNumRows(0);
  setNumCols(0);
  for (int i = 0; i <= horizontalHeader()->count(); i++)  
    horizontalHeader()->setLabel(i, QString::null);

  if (sorting())
    horizontalHeader()->setSortIndicator( -1 );
  updateScrollBars();
  if (!b)
    setBlocked(false);
}

void CTable::saveTableContentsToFile(QTable *table, CMySQLServer *m, QString &fname, const QString &ext, const QString &file_txt, const QString &title)
{
  QString contents = title + m->lineTerminator(true);
  contents += tr("Connection") + ": " + m->connectionName() + m->lineTerminator(true);
  contents += tr("Host") + ": " + m->hostName() + m->lineTerminator(true);
  contents += tr("Saved") + ": " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + m->lineTerminator(true);
  contents = myApp()->commentText(contents, m->lineTerminator(true)) + m->lineTerminator(true);

  QString line = QString::null;
  QString txt = QString::null;
  int nrows = table->numRows();
  int ncols = table->numCols();
  
  if (nrows > 0 && ncols > 0)
  {
    for (int c = 0; c <= ncols -1; c++)
    {
      if (table->horizontalHeader()->sectionSize(c) > 0)
      {
        line += m->fieldEncloser(true) + table->horizontalHeader()->label(c) + 
          m->fieldEncloser(true) + m->fieldSeparator(true);
      }
    }
    line = line.mid(0,line.length() - m->fieldSeparator(true).length());
    contents += line + m->lineTerminator(true);
    for (int r = 0; r <= nrows - 1; r++)
    {
      line = QString::null;      
      for (int c = 0; c <= ncols - 1 ; c++)
      {      
        if (table->horizontalHeader()->sectionSize(c) > 0)
        {
          txt = table->text(r,c);
          if (txt.isEmpty())
            txt = m->replaceEmpty(true);
          line += m->fieldEncloser(true) + txt + m->fieldEncloser(true) + m->fieldSeparator(true);          
        }
      }
      line = line.mid(0,line.length() - m->fieldSeparator(true).length());
      contents += line + m->lineTerminator(true);
    }
  }
  saveToFile(fname, ext, file_txt, contents, m->messagePanel());
}

void CTable::setNumCols(int r)
{
#ifdef DEBUG
  qDebug("CTable::setNumCols(%d)", r);
#endif

  header_sort.clear();
  for (int i = 0; i < r; i++)
    header_sort.insert(i, false);

  QTable::setNumCols(r);
}

void CTable::sort(int col)
{
#ifdef DEBUG
  qDebug("CTable::sort(%d)", col);
#endif

  if (isBlocked())
    return;
  setBlocked(true);
  bool b = header_sort[col];
  last_sort = col;
  last_sort_dir = b;
  horizontalHeader()->setSortIndicator(col, b);
  sortColumn(col, b, true);
  header_sort[col] = !b;
  setBlocked(false);
}

void CTable::applyLastSort()
{
  if (last_sort == -1)
    return;
  header_sort[last_sort] = last_sort_dir;
  sort(last_sort);
}


int CTable::cmpTableItems(const void *n1, const void *n2)
{
#ifdef DEBUG
  qDebug("int CQueryTable::cmpTableItems()");
#endif

  if (!n1 || !n2)
    return 0;    
  SortableTableItem *i1 = (SortableTableItem *)n1;
  SortableTableItem *i2 = (SortableTableItem *)n2;    
  return i1->item->key().localeAwareCompare(i2->item->key());
}

int CTable::cmpIntTableItems(const void *n1, const void *n2)
{
#ifdef DEBUG
  qDebug("int CQueryTable::cmpIntTableItems()");
#endif

  if (!n1 || !n2)
    return 0;
  
  SortableTableItem *i1 = (SortableTableItem *)n1;
  SortableTableItem *i2 = (SortableTableItem *)n2;
  bool ok;
  double v1 = i1->item->key().toDouble(&ok);
  if (!ok)
    return cmpTableItems(n1, n2);
  double v2 = i2->item->key().toDouble(&ok);
  if (!ok)
    return cmpTableItems(n1, n2);
  return (v1 < v2) ? -1 : (v1 > v2) ? 1 : 0;
}

void CTable::mySortColumn(int col, int (*sort_func)(const void *, const void *), bool ascending, bool wholeRows)
{
#ifdef DEBUG
  qDebug("CTable::mySortColumn(%d, int (*sort_func)(const void *, const void *), %s, %s)", col, debug_string(booltostr(ascending)), debug_string(booltostr(wholeRows)));
#endif

  int filledRows = 0, i;
  for (i = 0; i < numRows(); ++i)
  {
    QTableItem *itm = item(i, col);
    if (itm)
      filledRows++;
  }
  
  if (!filledRows)
    return;
  
  SortableTableItem *items = new SortableTableItem[filledRows];
  int j = 0;
  for (i = 0; i < numRows(); ++i)
  {
    QTableItem *itm = item(i, col);
    if (!itm)
      continue;
    items[j++].item = itm;
  }
  
  qsort(items, filledRows, sizeof(SortableTableItem), sort_func);
  
  bool updatesEnabled = isUpdatesEnabled();
  bool viewportUpdatesEnabled = viewport()->isUpdatesEnabled();
  setUpdatesEnabled(false);
  viewport()->setUpdatesEnabled(false);
  for (i = 0; i < numRows(); ++i)
  {
    if (i < filledRows)
    {
      if (ascending)
      {
        if (items[i].item->row() == i)
          continue;
        if (wholeRows)
          swapRows(items[i].item->row(), i);
        else
          swapCells(items[i].item->row(), col, i, col);
      }
      else
      {
        if (items[i].item->row() == filledRows - i - 1)
          continue;
        if (wholeRows)
          swapRows(items[i].item->row(), filledRows - i - 1);
        else
          swapCells(items[i].item->row(), col, filledRows - i - 1, col);
      }
    }
  }
  setUpdatesEnabled(updatesEnabled);
  viewport()->setUpdatesEnabled(viewportUpdatesEnabled);
  
  if (!wholeRows)
    repaintContents(columnPos(col), contentsY(), columnWidth(col), visibleHeight(), false);
  else
    repaintContents(contentsX(), contentsY(), visibleWidth(), visibleHeight(), false);
  
  delete [] items;
}
