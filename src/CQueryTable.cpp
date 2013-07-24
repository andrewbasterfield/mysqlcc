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
#include "CQueryTable.h"
#include "CMySQLServer.h"
#include "CMySQL.h"
#include "CMySQLQuery.h"
#include "config.h"
#include "globals.h"
#include "panels.h"
#include <stddef.h>
#include <qfile.h>
#include <qdatetime.h>
#include <q3popupmenu.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <QPixmap>


CQueryTable::CQueryTable(QWidget * parent, CMySQLQuery *q, CMySQLServer *m, const char * name)
: CTable(parent, name), m_cancel(false)
{
#ifdef DEBUG
  qDebug("CQueryTable::CQueryTable()");
#endif

  if (!name)
    setName("CQueryTable");

  if (q != 0)
    setQuery(q);
  else
    qry = 0;

  setSelectionMode(Q3Table::Single);
  setReadOnly(true);
  pkIcon = getPixmapIcon("pkIcon");
  mulIcon = getPixmapIcon("mulIcon");
  uniIcon = getPixmapIcon("uniIcon");
  nothingIcon = getPixmapIcon("nothingIcon");
  m_mysql = m;
}

void CQueryTable::cancel()
{
  m_cancel = true;
}

void CQueryTable::reset()
{
  CTable::reset();
  emit refreshed();
}

void CQueryTable::refresh()
{
  if (isBlocked())
    return;
  reset();
  m_cancel = false;
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
    if (columns_ok)
      for (uint i = 0; i < num_fields; i++)
        columns_ok &= (previous_columns_map[i].label == query()->fields(i).name);

    if (!columns_ok && keepColumnWidth())
      previous_columns_map.clear();

    for (uint i = 0; i < num_fields; i++)
    {
      if (IS_PRI_KEY(query()->fields(i).flags))
        icon = pkIcon;
      else if (IS_MUL_KEY(query()->fields(i).flags))
        icon = mulIcon;
      else if (IS_UNI_KEY(query()->fields(i).flags))
        icon = uniIcon;
      else     
        icon = nothingIcon;
      if (columns_ok && previous_columns_map[i].label == query()->fields(i).name)
        horizontalHeader()->setLabel(i, icon, query()->fields(i).name, previous_columns_map[i].size);
      else
        horizontalHeader()->setLabel(i, icon, query()->fields(i).name);
    }
    if (hasProcessEvents())
      qApp->processEvents();
    else
      updateScrollBars();

    ulong j = 0;
    while (query()->next(!hasProcessEvents()))
    {
      if (m_cancel)
        break;
      for (uint i = 0; i < num_fields; i++)
        setText(j, i, query()->row(i));
      j++;
    }
    setBlocked(false);
    emit refreshed();
  }
  else
    if (!previous_columns_map.empty())
      previous_columns_map.clear();
}

void CQueryTable::save()
{
  if (isBlocked())
    return;  
  tmpFileName = getSaveFileName(tmpFileName, "txt", tr("Text Files (*.txt);;All Files(*.*)"));  

  if (tmpFileName.isEmpty())
    return;
  
  QFile file( tmpFileName );    
  if (file.exists() && myApp()->confirmCritical())
    if ((QMessageBox::warning(0, tr("Replace File"), tr("The specified file name already exists.\nDo you want to replace it ?"),
      QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes))
      return;

  if ( !file.open( QIODevice::WriteOnly ) )
  {
    if (mysql()->messagePanel())
      mysql()->messagePanel()->critical(tr("An error occurred while saving the file"));
    return;
  }

  setBlocked(true);

  QString line_terminator = mysql()->lineTerminator(true);

  QString title = caption() + line_terminator;
  title += tr("Connection") + ": " + mysql()->connectionName() + line_terminator;
  title += tr("Host") + ": " + mysql()->hostName() + line_terminator;  
  title += tr("Saved") + ": " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + line_terminator;  
  title = CApplication::commentText(title, line_terminator) + line_terminator;

  Q3TextStream ts( &file );
  ts << title;
  
  QString tmp = tr("Query") + ":" + line_terminator;
  tmp += query()->lastQuery().replace(QRegExp("\n"), line_terminator) + line_terminator;
  ts << CApplication::commentText(tmp, line_terminator) << line_terminator;
 
  query()->dataSeek(0);
  
  uint num_fields = query()->numFields();
  QString encl = mysql()->fieldEncloser(true);
  QString sep = mysql()->fieldSeparator(true);
  uint j;
  bool add_sep = false;
  for (j = 0; j < num_fields; j++)
  {
    if (horizontalHeader()->sectionSize(j) != 0)
    {
      if (add_sep)
        ts << sep;
      ts << encl << query()->fields(j).name << encl;
      if (!add_sep)
        add_sep = true;
    }
  }
  ts << line_terminator;


  if (numRows() <= 0 || numCols() <= 0)
    return;

  QString repl_empty = mysql()->replaceEmpty(true);
  
  while (query()->next(!hasProcessEvents()))
  {
    add_sep = false;
    for (j = 0; j < num_fields; j++)
      if (horizontalHeader()->sectionSize(j) != 0)
      {
        const char* r = query()->row(j);
        if (!r)
          r = "[NULL]";
        if (!*r)
          r = (const char*)repl_empty;
        if (add_sep)
          ts << sep;

        ts << encl << r << encl;
        if (!add_sep)
          add_sep = true;
      }
    ts << line_terminator;
  }

  file.close();
  if (mysql()->messagePanel()) 
    mysql()->messagePanel()->information(tr("Successfully saved") + ": " + tmpFileName);

  setBlocked(false);
}

QString CQueryTable::copy_data(int row, int col)
{
#ifdef DEBUG
  qDebug("CQueryTable::copy_data(int, int)");
#endif

  if (!query())
    return QString::null;

  if (query()->isResultNull() || isBlocked())
    return QString::null;

  if (currentSelection() == -1 && !forceCopyAll())
    return copy_current_selection_func(row, col);
  else
  {
    Q3TableSelection sel;
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
      length = strlen(query()->fields(current_col).name);
      length = max(length, query()->fields(current_col).max_length);
      if (length < strlen(NULL_TEXT) && !IS_NOT_NULL(query()->fields(current_col).flags))
        length = strlen(NULL_TEXT);
      max_length_map.insert(current_col, length + 1);
      for (uint i = 0; i < min(max_length_map[current_col] - 1, MAX_COLUMN_LENGTH) + 2; i++)
        separator += "-";
      separator += "+";
    }

    separator += "\n";
    cpy = separator + "|";
    
    for (current_col = sel.leftCol(); current_col <= sel.rightCol(); current_col++)
    {
      if (horizontalHeader()->sectionSize(current_col) <= 0)
        continue;
      tmp.sprintf(" %-*s|",min((int) max_length_map[current_col], MAX_COLUMN_LENGTH), query()->fields(current_col).name);
      cpy += tmp;
    }
    cpy += "\n" + separator;

    copy_data_func(&cpy, query(), &sel, &max_length_map);

    setBlocked(false);
    return cpy + separator;
  }
}

void CQueryTable::copy_data_func(QString *cpy, CMySQLQuery *qry, Q3TableSelection *sel, QMap<uint, ulong> *max_length_map)
{
  uint length;
  QString tmp;
  qry->dataSeek(0);
  for (int current_row = sel->topRow(); current_row <= sel->bottomRow(); current_row++)
  {
    qry->next();
    *cpy += "|";
    for (int current_col = sel->leftCol(); current_col <= sel->rightCol(); current_col++)
    {
      if (horizontalHeader()->sectionSize(current_col) <= 0)
        continue;
      const char *str= query()->row(current_col) ? query()->row(current_col) : NULL_TEXT;
      length = (*max_length_map)[current_col];
      
      if (length > MAX_COLUMN_LENGTH)
      {
        *cpy += str;
        *cpy += "|";
      }
      else
      {
        tmp.sprintf(IS_NUM(qry->fields(current_col).type) ? "%*s |" : " %-*s|", length, str);
        *cpy += tmp;
      }
    }
    *cpy += "\n";
  }
}

void CQueryTable::sortColumn(int col, bool ascending, bool wholeRows)
{
#ifdef DEBUG
  qDebug("CQueryTable::sortColumn(%d, %s, %s)", col, debug_string(booltostr(ascending)), debug_string(booltostr(wholeRows)));
#endif
  
  if (!query()->isResultNull())
    mySortColumn(col, IS_NUM(query()->fields(col).type) ? cmpIntTableItems : cmpTableItems, ascending, wholeRows);
  else
    CTable::sortColumn(col, ascending, wholeRows);
}
