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
#include "panels.h"
#include "config.h"
#include "globals.h"
#include "CHistoryView.h"
#include "CDockWindow.h"
#include "CMySQLQuery.h"
#include <stddef.h>  
#include <qlayout.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qclipboard.h>
#include <qheader.h>
#include <qtooltip.h>

#define MAX_MESSAGE_LENGTH CHistoryView::querySize()

CPanelItem::CPanelItem(QListView * parent, const QString &t, const QPixmap &p)
: QListViewItem(parent)
{
  init(t, p);  
}

CPanelItem::CPanelItem(QListView * parent, QListViewItem * after, const QString &t, const QPixmap &p)
: QListViewItem(parent, after)
{
  init(t, p);  
}

void CPanelItem::init(const QString &t, const QPixmap &p)
{
  if (!t.isEmpty())
    setText(0, t);

  if (t.length() > MAX_MESSAGE_LENGTH)   
    setPixmap(0, getPixmapIcon("warningIcon"));
  else if (!p.isNull())
    setPixmap(0, p);
}

void CPanelItem::setText(int col, const QString &s)
{
  if (s.length() > MAX_MESSAGE_LENGTH)
    real_text = s.left(MAX_MESSAGE_LENGTH);
  else
    real_text = s;

  if (maxDisplay() > -1 && s.length() >= (uint) maxDisplay())
  {
     if (s.length() > MAX_MESSAGE_LENGTH)
       QListViewItem::setText(col, QObject::tr("[BIG QUERY] - Query not stored completely in memory."));
     else
       QListViewItem::setText(col, s.simplifyWhiteSpace().left(maxDisplay() - 3) + "...");
  }
  else
    QListViewItem::setText(col, s.simplifyWhiteSpace());
}


CPanel::CPanel(QWidget * parent, const char * name, WFlags f)
: QListView(parent, name, f), maxdisplay(-1), enter("\r\n")
{
#ifdef DEBUG
  qDebug("CPanel::CPanel()");
#endif
  
  setLineWidth(1);
  setSelectionMode(QListView::Single);
  setSorting(-1);
  addColumn("");
  setMaxDisplaySize(150);
  header()->hide();
  header()->setStretchEnabled(true, 0);
  msgPanel = 0;
  realtext.setAutoDelete(true);
  enable_bottom_item = true;
  ctrl = false;
  connect(this, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)), this, SLOT(displayMenu(QListViewItem *, const QPoint &, int)));
}

void CPanel::setCarriageReturn(const QString &s)
{
#ifdef DEBUG
  qDebug("CPanel::setCarriageReturn()");
#endif
  
  enter = s;
}

void CPanel::setBottomItem(QListViewItem *i)
{
#ifdef DEBUG
  qDebug("CPanel::setBottomItem()");
#endif
  
  QListViewItem *x = i != 0 ? i : lastItem();
  if (x != 0)
    ensureItemVisible(x);
}

void CPanel::setMessagePanel(CMessagePanel *m)
{
#ifdef DEBUG
  qDebug("CPanel::setMessagePanel()");
#endif
  
  msgPanel = m;
}

void CPanel::showMessage(const QString & m)
{
#ifdef DEBUG
  qDebug("CPanel::showMessage(const QString &)");
#endif

  if (m.isEmpty())
    return;  
    
  QListViewItem *p = lastItem();

  if (p != 0)
  {
    if (enable_bottom_item)
      setBottomItem(new CPanelItem(this, p, m));
    else
      new CPanelItem(this, p, m);
  }
  else
  {
    if (enable_bottom_item)
      setBottomItem(new CPanelItem(this, m));
    else
      new CPanelItem(this, m);
  }

}

void CPanel::showMessage(const QPixmap & w, const QString & m)
{
#ifdef DEBUG
  qDebug("CPanel::showMessage(const QPixmap &, const QString &)");
#endif
  
  if (m.isEmpty())
    return;

  QListViewItem *p = lastItem();
  if (p != 0)
  {
    if (enable_bottom_item)
      setBottomItem(new CPanelItem(this, p, m, w));
    else
      new CPanelItem(this, p, m, w);
  }
  else
  {
    if (enable_bottom_item)
      setBottomItem(new CPanelItem(this, m, w));
    else
      new CPanelItem(this, m, w);
  }
}

void CPanel::save()
{
#ifdef DEBUG
  qDebug("CPanel::save()");
#endif
  
  QString txt = QString::null;   
  QListViewItemIterator it(this);
  while ( it.current() != 0 )
  {
    txt += ((CPanelItem *) it.current())->realText() + enter;
    ++it;
  }

  saveToFile(tmpFile, "txt", tr("Text Files (*.txt);;All Files(*.*)" ), txt, msgPanel);
}

void CPanel::copy(QListViewItem *item)
{
#ifdef DEBUG
  qDebug("CPanel::copy()");
#endif
  
#ifndef QT_NO_CLIPBOARD
  if (item)
    QApplication::clipboard()->setText(((CPanelItem *)item)->realText());  
#endif  
}

void CPanel::keyPressEvent(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CPanel::keyPressEvent()");
#endif

  if (e->key() == Qt::Key_Control && !ctrl)
    ctrl = true;
  else
  {
    if (ctrl && e->key() == Qt::Key_C || e->key() == Qt::Key_Insert)
      copy(currentItem());
    ctrl = false;
  }

  QListView::keyPressEvent(e);
}

CMessagePanel::CMessagePanel(const QString &caption, QWidget * parent, const char * name, WFlags f)
: CPanel(parent, name, f)
{
#ifdef DEBUG
  qDebug("CMessagePanel::CMessagePanel()");
#endif
  
  if (!name)
    setName("CMessagePanel");
  if (caption.isEmpty())
    setCaption(tr("Messages"));
  else    
    setCaption(caption);
  warningIcon = getPixmapIcon("warningIcon");
  criticalIcon = getPixmapIcon("criticalIcon");
  informationIcon = getPixmapIcon("informationIcon");
}

void CMessagePanel::displayMenu(QListViewItem *item, const QPoint &pos, int)
{
#ifdef DEBUG
  qDebug("CMessagePanel::displayMenu()");
#endif
  
  QPopupMenu *p_itemMenu = new QPopupMenu();
  Q_CHECK_PTR(p_itemMenu);  
  p_itemMenu->insertItem(getPixmapIcon("copyIcon"), tr("Copy"), 1);
#ifdef QT_NO_CLIPBOARD
  p_itemMenu->setItemEnabled (1, false);
#else
  p_itemMenu->setItemEnabled (1, (item != 0));
#endif 
  p_itemMenu->insertSeparator();
  p_itemMenu->insertItem(getPixmapIcon("saveIcon"), tr("Save"), 2);
  p_itemMenu->insertSeparator();
  p_itemMenu->insertItem(tr("Clear"), 3);
  if (childCount() == 0)
  {    
    p_itemMenu->setItemEnabled (2, false);
    p_itemMenu->setItemEnabled (3, false);
  } 
  int res = p_itemMenu->exec(pos);
  delete p_itemMenu;
  switch (res)
  {
  case 1: copy(item);
    break;
  case 2: save();
    break;
  case 3: clear();
    break;
  }    
}

void CMessagePanel::warning(const QString & m)
{
#ifdef DEBUG
  qDebug("CMessagePanel::warning()");
#endif
  
  showMessage(warningIcon, m);
  playSound(myApp()->warningSoundFile());
}

void CMessagePanel::critical(const QString & m)
{
#ifdef DEBUG
  qDebug("CMessagePanel::critical()");
#endif
  
  showMessage(criticalIcon, m);
  playSound(myApp()->errorSoundFile());
}

void CMessagePanel::information(const QString & m)
{
#ifdef DEBUG
  qDebug("CMessagePanel::information()");
#endif
  
  showMessage(informationIcon, m);
  playSound(myApp()->informationSoundFile());
}

void CMessagePanel::message(ushort type, const QString & m)
{
#ifdef DEBUG
  qDebug("CMessagePanel::message()");
#endif
  
  switch (type)
  {
    case INFORMATION: information(m);
      break;
    case WARNING: warning(m);
      break;
    case CRITICAL: critical(m);
      break;
    default: showMessage(m);
      break;
  }  
}


CHistoryPanel::CHistoryPanel(const QString &caption, QWidget * parent, const char * name, WFlags f)
: CPanel(parent, name, f)
{
#ifdef DEBUG
  qDebug("CHistoryPanel::CHistoryPanel()");
#endif
  
  if (!name)
    setName("CHistoryPanel");
  if (caption.isEmpty())
    setCaption(tr("Messages"));
  else    
    setCaption(caption);
  historyScriptIcon = getPixmapIcon("historyScriptIcon");
  
  itemsEnabled = true;
  connect(this, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(DoubleClicked(QListViewItem *)));
  connect(this, SIGNAL(onItem(QListViewItem *)), this, SLOT(setCurrentHint(QListViewItem * )));  
  connect(this, SIGNAL(mouseButtonClicked(int, QListViewItem *, const QPoint &, int)), this, SLOT(mouseClicked(int, QListViewItem *, const QPoint &, int)));
}

void CHistoryPanel::displayMenu(QListViewItem *item, const QPoint &pos, int)
{
#ifdef DEBUG
  qDebug("CHistoryPanel::displayMenu()");
#endif
  
  QPopupMenu *p_itemMenu = new QPopupMenu();

  QString qry = ((CPanelItem *)item)->realText();

  p_itemMenu->insertItem(getPixmapIcon("executeQueryIcon"), tr("Execute Query"), 1);

  if (CMySQLQuery::query_type(qry.lower()) == "select")
    p_itemMenu->insertItem(tr("Explain Query"), 2);

  p_itemMenu->insertSeparator();
  p_itemMenu->insertItem(getPixmapIcon("copyIcon"), tr("Copy"), 3);
  
#ifdef QT_NO_CLIPBOARD
  p_itemMenu->setItemEnabled (3, false);
#else
  p_itemMenu->setItemEnabled (3, (item != 0));
#endif  
  
  p_itemMenu->insertSeparator();
  p_itemMenu->insertItem(getPixmapIcon("saveIcon"), tr("Save"), 4);
  p_itemMenu->insertSeparator();
  p_itemMenu->insertItem(getPixmapIcon("refreshIcon"), tr("Refresh"), 5);
  
  if (childCount() == 0)  
    p_itemMenu->setItemEnabled(4, false);
  
  p_itemMenu->setItemEnabled(1, itemsEnabled);
  p_itemMenu->setItemEnabled(5, itemsEnabled);
  
  int res = p_itemMenu->exec(pos);
  delete p_itemMenu;
  switch (res) {
  case 1: emit executeQuery(qry);
    break;
  case 2: emit executeQuery("EXPLAIN\n" + qry);
    break;
  case 3: copy(item);
    break;
  case 4: save();
    break;
  case 5: emit reloadHistoryList();
    break;
  }    
}

void CHistoryPanel::DoubleClicked(QListViewItem * i)
{
#ifdef DEBUG
  qDebug("CHistoryPanel::DoubleClicked()");
#endif
  
  setCurrentHint(i);
  QString hint = ((CPanelItem *) i)->realText();
  if (!hint.isEmpty())
    emit itemSelected(hint);
  emit doubleClicked(hint);
}

void CHistoryPanel::History(const QString & m, bool append)
{
#ifdef DEBUG
  qDebug("CHistoryPanel::History()");
#endif
  
  bool b = true;
  if (append)  
    b = CHistoryView::historyView()->append(m);

  if (b)
  {
    if (lastItem() != 0)
      if (((CPanelItem *) lastItem())->realText() == m)
        return;
    showMessage(historyScriptIcon, m);
  }

}

void CHistoryPanel::mouseClicked(int, QListViewItem * i, const QPoint &, int)
{
#ifdef DEBUG
  qDebug("CHistoryPanel::mouseClicked()");
#endif
  
  setCurrentHint(i); 
}

QString CHistoryPanel::getSelectedHintText()
{
#ifdef DEBUG
  qDebug("CHistoryPanel::getSelectedHintText()");
#endif
  
  return currentItem() != 0 ? ((CPanelItem *)currentItem())->realText() : QString::null;
}

void CHistoryPanel::setCurrentHint(QListViewItem * i)
{
#ifdef DEBUG
  qDebug("CHistoryPanel::setCurrentHint()");
#endif
  
  if (i == 0)
    return;
  QToolTip::remove(this);
  QString tmp = ((CPanelItem *) i)->realText();
  if (!tmp.isEmpty())  
  QToolTip::add(this, tmp);
}


CSqlDebugPanel::CSqlDebugPanel(const QString &caption, QWidget * parent, const char * name, WFlags f)
: CPanel(parent, name, f)
{
#ifdef DEBUG
  qDebug("CSqlDebugPanel::CSqlDebugPanel()");
#endif
  
  if (!name)
    setName("CSqlDebugPanel");
  
  if (caption.isEmpty())  
    setCaption(tr("SQL Debug"));
  else
    setCaption(caption);
  historyScriptIcon = getPixmapIcon("historyScriptIcon");
  
  connect (this, SIGNAL(onItem(QListViewItem *)), this, SLOT(setCurrentHint(QListViewItem * )));
  connect (this, SIGNAL(mouseButtonClicked(int, QListViewItem *, const QPoint &, int)), this, SLOT(mouseClicked(int, QListViewItem *, const QPoint &, int)));
}

void CSqlDebugPanel::copy(QListViewItem *item)
{
#ifdef DEBUG
  qDebug("CSqlDebugPanel::copy()");
#endif  
  
#ifndef QT_NO_CLIPBOARD
  if (item == 0)
    return;

  QString tmp = ((CPanelItem *)item)->realText();  
  if (!tmp.isEmpty())
  {    
    int p = tmp.find("] ");
    if (p > -1)
    {
      p += 2;
      QApplication::clipboard()->setText( tmp.mid(p, tmp.length() - p) );
    }
  }
#endif  
}

void CSqlDebugPanel::displayMenu(QListViewItem *item, const QPoint &pos, int)
{
#ifdef DEBUG
  qDebug("CSqlDebugPanel::displayMenu()");
#endif
  
  QPopupMenu *p_itemMenu = new QPopupMenu();
  Q_CHECK_PTR(p_itemMenu);
  p_itemMenu->insertItem(getPixmapIcon("copyIcon"), tr("Copy"), 1);
  
#ifdef QT_NO_CLIPBOARD
  p_itemMenu->setItemEnabled (1, false);
#else
  p_itemMenu->setItemEnabled (1, (item != 0));
#endif
  
  p_itemMenu->insertItem(getPixmapIcon("saveIcon"), tr("Save"), 2);
  p_itemMenu->insertSeparator();
  p_itemMenu->insertItem(tr("Clear"), 3);  
  
  if (childCount() == 0)
  {
    p_itemMenu->setItemEnabled (2, false);
    p_itemMenu->setItemEnabled (3, false);
  }  
  
  int res = p_itemMenu->exec(pos);
  delete p_itemMenu;
  switch (res) {
  case 1: copy(item);
    break;
  case 2: save();
    break;
  case 3: clear();
    break;
  }    
}

void CSqlDebugPanel::SqlDebug(const QString & m)
{
#ifdef DEBUG
  qDebug("CSqlDebugPanel::SqlDebug()");
#endif
  
  showMessage(historyScriptIcon, m);  
}

void CSqlDebugPanel::mouseClicked(int, QListViewItem * i, const QPoint &, int)
{
#ifdef DEBUG
  qDebug("CSqlDebugPanel::mouseClicked()");
#endif
  
  setCurrentHint(i); 
}

QString CSqlDebugPanel::selectedHintText()
{
#ifdef DEBUG
  qDebug("CSqlDebugPanel::selectedHintText()");
#endif
  
  return currentItem() != 0 ? ((CPanelItem *)currentItem())->realText() : QString::null;  
}

void CSqlDebugPanel::setCurrentHint(QListViewItem * i)
{
#ifdef DEBUG
  qDebug("CSqlDebugPanel::setCurrentHint()");
#endif
  
  if (i == 0)
    return;
  QToolTip::remove(this);
  QString tmp = ((CPanelItem *) i)->realText();
  if (!tmp.isEmpty())  
  QToolTip::add(this, tmp);
}


CMessageWindow::CMessageWindow(Place p, QWidget * parent, const char * name, WFlags f)
:CDockWindow (p, parent, name, f)
{
#ifdef DEBUG
  qDebug("CMessageWindow::CMessageWindow()");
#endif
  
  if ( !name )
    setName("CMessageWindow");
  cr = QString::null;
  setCaption(tr("Message Panel"));
  Tabs = new QTabWidget(this, "Tabs" );  
  Tabs->setTabPosition( QTabWidget::Bottom );
  Tabs->setTabShape( QTabWidget::Triangular );
  connect(Tabs, SIGNAL(currentChanged(QWidget *)), this, SLOT(tab_changed(QWidget *)));
  setWidget(Tabs);  
}

void CMessageWindow::setCarriageReturn(const QString &s)
{
#ifdef DEBUG
  qDebug("CMessageWindow::setCarriageReturn()");
#endif
  
  cr = s;
  for (int i = 0; i < Tabs->count(); i++)
    ((CPanel *)Tabs->page(i))->setCarriageReturn(s);
}

void CMessageWindow::addPanel(QWidget *panel)
{
#ifdef DEBUG
  qDebug("CMessageWindow::addPanel()");
#endif
  
  if (panel != 0)
  {
    panel->reparent(Tabs, 0, QPoint(0,0));
    if (!cr.isEmpty() && panel->inherits("CPanel"))
      ((CPanel *) panel)->setCarriageReturn(cr);
    Tabs->insertTab(panel, panel->caption());
  }
}

void CMessageWindow::tab_changed(QWidget *w)
{
#ifdef DEBUG
  qDebug("CMessageWindow::tab_changed()");
#endif

  if (w->inherits("CPanel"))
    ((CPanel *) w)->setBottomItem();
}

void CMessageWindow::setTabEnabled(QWidget * w, bool enable)
{
#ifdef DEBUG
  qDebug("CMessageWindow::enablePanel(QWidget, bool)");
#endif
  
  if (w != 0)
    Tabs->setTabEnabled(w, enable);
}

void CMessageWindow::setActivePanel(int t)
{
#ifdef DEBUG
  qDebug("CMessageWindow::setActivePanel(int)");
#endif

  if (Tabs->page(t) != 0 && Tabs->currentPageIndex() != t)
    Tabs->setCurrentPage(t);
}
