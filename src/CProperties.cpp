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
#include "CProperties.h"
#include "CMySQLServer.h"
#include "globals.h"
#include "config.h"
#include <qclipboard.h>
#include <qdatetime.h>

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 2
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

CProperties::CProperties(QWidget* parent,  CMySQLServer *m, const char *name)
: QWidget(parent, name)
{
#ifdef DEBUG
  qDebug("CProperties::CProperties()");
#endif

  if (!name)
    setName("CProperties");
  tmpFileName = QString::null;
  CPropertiesLayout = new QVBoxLayout( this, 0, 1, "CPropertiesLayout"); 
  title = new QLabel( this, "connectionName" );
  QFont title_font(title->font());
  title_font.setPointSize(9);  
  title->setFont(title_font);
  title->setMinimumSize(0, 20);
  CPropertiesLayout->addWidget(title);
  details = new QListView(this, "details");

  details->setFrameShape(QListView::Box);
  details->setLineWidth(1);
  details->setSelectionMode(QListView::NoSelection);
  details->setShowSortIndicator(true);
  details->setShowToolTips(false);
  details->header()->setClickEnabled(true, 0);
  details->header()->setClickEnabled(false, 1);
  details->header()->setStretchEnabled(true, 1);
  details->header()->setCellSize (0, 150);  
  
  refreshAction = new QAction(this, "refreshAction" );
  refreshAction->setText(tr("Refresh"));
  refreshAction->setIconSet( QIconSet(getPixmapIcon("refreshIcon")) );
  refreshAction->setMenuText(tr("&Refresh"));
  refreshAction->setAccel( 0 );
  
  connect(details, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)), this, SLOT(RightButtonClicked( QListViewItem *, const QPoint &, int )));
  
  CPropertiesLayout->addWidget(details);
  p_mysql = m;
  hasSaveTitle = false;
  saveTitle = QString::null;
}

void CProperties::refresh()
{
#ifdef DEBUG
  qDebug("CProperties::refresh()");
#endif

  details->clear();
  insertData();
}

void CProperties::setSaveTitle(const QString &title)
{
#ifdef DEBUG
  qDebug("CProperties::setSaveTitle('%s')", debug_string(title));
#endif

  saveTitle = title;
  hasSaveTitle = true;
}

QString CProperties::getSaveTitle()
{
#ifdef DEBUG
  qDebug("CProperties::getSaveTitle()");
#endif

  QString ret = QString::null;
  if (hasSaveTitle)
  {
    ret = saveTitle + mysql()->lineTerminator(true);
    ret += tr("Connection") + ": " + mysql()->connectionName() + mysql()->lineTerminator(true);
    ret += tr("Host") + ": " + mysql()->hostName() + mysql()->lineTerminator(true);
    ret += tr("Saved") + ": " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + mysql()->lineTerminator(true);    
    ret = CApplication::commentText(ret, mysql()->lineTerminator(true)) + mysql()->lineTerminator(true);
  }
  return ret;
}

QString CProperties::getSaveContents()
{
#ifdef DEBUG
  qDebug("CProperties::getSaveContents()");
#endif

  QListViewItemIterator it(details);
  QString contents = getSaveTitle();
  for (int i = 0; i < details->columns(); i++)
    contents += mysql()->fieldEncloser(true) + details->columnText(i) + mysql()->fieldEncloser(true) 
    + ((i < details->columns() - 1) ? mysql()->fieldSeparator(true) : mysql()->lineTerminator(true));
  QString line;
  QString txt;
  for ( ; it.current(); ++it )
  {
    line = QString::null;
    for (int i = 0; i < details->columns(); i++)
    {
      txt = ((it.current()->text(i).isEmpty()) ? mysql()->replaceEmpty(true) : it.current()->text(i));      
      line += mysql()->fieldEncloser(true) + txt + mysql()->fieldEncloser(true) 
      + ((i < details->columns() - 1) ? mysql()->fieldSeparator(true) : mysql()->lineTerminator(true));
    }
    contents += line;
  }
  return contents;
}

void CProperties::save()
{
#ifdef DEBUG
  qDebug("CProperties::save()");
#endif

  saveToFile(tmpFileName, "txt", tr("Text Files (*.txt);;All Files(*.*)" ), getSaveContents(), mysql()->messagePanel());
}

void CProperties::RightButtonClicked( QListViewItem *, const QPoint & pos, int c)
{
#ifdef DEBUG
  qDebug("CProperties::RightButtonClicked()");
#endif

  QPopupMenu p_itemMenu;
  p_itemMenu.insertItem(getPixmapIcon("copyIcon"), tr("Copy"), MENU_COPY);
  p_itemMenu.setItemEnabled(MENU_COPY, (details->childCount() > 0));
  p_itemMenu.insertSeparator();
  p_itemMenu.insertItem(getPixmapIcon("saveIcon"), tr("Save"), MENU_SAVE);
  p_itemMenu.insertSeparator();
  p_itemMenu.insertItem(refreshAction->iconSet(), refreshAction->text(), MENU_REFRESH);
  switch (p_itemMenu.exec(pos))
  {
  case MENU_COPY:
    {
#ifndef QT_NO_CLIPBOARD      
      QListViewItem * item = details->currentItem();
      if (item != 0)
        QApplication::clipboard()->setText(item->text(c));
#endif      
    }
    break;
  case MENU_SAVE:
    save();
    break;
  case MENU_REFRESH:
    refresh();
    break;
  }  
}

void CProperties::setTitle(const QString &s)
{
#ifdef DEBUG
  qDebug("CProperties::setTitle('%s')", debug_string(s));
#endif

  title->setText(" " + s);
}

void CProperties::saveData(QDict<QString> *data)
{
#ifdef DEBUG
  qDebug("CProperties::saveData()");
#endif

  data->clear();
  QListViewItemIterator it(details);
  while ( it.current() != 0 )
  {
    data->insert(it.current()->text(0), new QString(it.current()->text(1)));
    ++it;
  }
}

void CProperties::loadData(const QDict<QString> &data)
{
#ifdef DEBUG
  qDebug("CProperties::loadData()");
#endif

  details->clear();
  if (data.count() > 0)
  {
    QDictIterator<QString> it(data);
    for( ; it.current(); ++it )
      insertItem(it.currentKey(), *it.current());
  }
}

void CProperties::insertItem(const QString & property, const QString & value)
{
#ifdef DEBUG
  qDebug("CProperties::insertItem('%s', '%s')", debug_string(property), debug_string(value));
#endif

  if (property.isEmpty() || value.isEmpty())
    return;
  QListViewItem * item = new QListViewItem(details);
  item->setText( 0, property);
  item->setText( 1, value);
  details->insertItem(item);
}
