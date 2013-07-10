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
#ifndef PANELS_H
#define PANELS_H

#include <qvariant.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qsize.h>
#include <qintdict.h>
#include "CDockWindow.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;

class CMessagePanel;
class CPanel;

class CPanel : public QListView
{
  Q_OBJECT
    
public:
  CPanel(QWidget * parent = 0, const char * name = 0, WFlags f = 0);
  void setCarriageReturn(const QString &s);
  QString realText(int idx);
  void setMaxDisplaySize(int len) { maxdisplay = len; };
  int maxDisplay() { return maxdisplay; }
  void setMessagePanel (CMessagePanel *m);
  void setEnableBottomItem(bool b) { enable_bottom_item = b; }
  bool enableBottomItem() const { return enable_bottom_item; }
  
  void showMessage(const QString & m);
  void showMessage(const QPixmap & w, const QString & m);  
 
public slots:
  void setBottomItem(QListViewItem *i=0);

protected slots:
  virtual void displayMenu(QListViewItem *, const QPoint &, int) {} ;
  
protected:
  virtual void save();
  virtual void copy(QListViewItem *);
  virtual QSize minimumSizeHint() const { return QSize(1, 1); }
  virtual QSize sizeHint() const { return QSize(1, 1); }
  QString tmpFile;
  
private:
  void keyPressEvent(QKeyEvent * e);
  int maxdisplay;
  bool enable_bottom_item;
  bool ctrl;
  CMessagePanel *msgPanel;
  QString enter;
  QString insertRealText(const QString &s);
  QIntDict<QString> realtext;
};

class CPanelItem : public QListViewItem
{
public:
  CPanelItem(QListView * parent, const QString &t=QString::null, const QPixmap &p=0);
  CPanelItem(QListView * parent, QListViewItem * after, const QString &t=QString::null, const QPixmap &p=0);
  QString realText() const { return real_text; }
  void setText(int, const QString &s);
  int maxDisplay() { return ((CPanel *)listView())->maxDisplay(); }

private:
  void init(const QString &t, const QPixmap &p);
  QString real_text;
};

class CMessagePanel : public CPanel
{
  Q_OBJECT
    
public:
  CMessagePanel(const QString &caption=QString::null, QWidget * parent = 0, const char * name = 0, WFlags f = 0);
  
public slots:
  void warning(const QString & m);
  void critical(const QString & m);
  void information(const QString & m);
  void message(ushort type, const QString & m);
  
private slots:  
  void displayMenu(QListViewItem *, const QPoint &, int);

private:
  QPixmap warningIcon;
  QPixmap criticalIcon;
  QPixmap informationIcon;
};


class CHistoryPanel : public CPanel
{
  Q_OBJECT
    
public:
  CHistoryPanel(const QString &caption=QString::null, QWidget * parent = 0, const char * name = 0, WFlags f = 0);  
  QString getSelectedHintText();
  
public slots:  
  void enableMenuItems(bool b) { itemsEnabled = b; };
  void History(const QString & m, bool append=true);
  
signals:
  void reloadHistoryList();
  void executeQuery(const QString &);
  void itemSelected(const QString &);
  void doubleClicked(const QString &);
  
private slots:
  void setCurrentHint(QListViewItem * i);
  void DoubleClicked(QListViewItem * i);
  void displayMenu(QListViewItem *, const QPoint &, int);
  void mouseClicked(int, QListViewItem * i, const QPoint &, int);
  
private:
  QPixmap historyScriptIcon;
  bool itemsEnabled;
};


class CSqlDebugPanel : public CPanel
{
  Q_OBJECT
    
public:
  CSqlDebugPanel(const QString &caption=QString::null, QWidget * parent = 0, const char * name = 0, WFlags f = 0);
  QString selectedHintText();
  
public slots:
  void SqlDebug(const QString & m);
  
private slots:
  void displayMenu(QListViewItem *, const QPoint &, int);
  void setCurrentHint(QListViewItem * i);
  void mouseClicked(int, QListViewItem * i, const QPoint &, int);
  
private:
  void copy(QListViewItem *);
  QPixmap historyScriptIcon;
};


class CMessageWindow : public CDockWindow
{
  Q_OBJECT
    
public:
  CMessageWindow (Place p = InDock, QWidget * parent = 0, const char * name = 0, WFlags f = 0) ;
  void setCarriageReturn(const QString &s);
  void addPanel(QWidget *panel);
  void setTabEnabled(QWidget * w, bool enable);
  void setActivePanel(int t);
  QTabWidget *tabWidget() const { return Tabs; }

private slots:
  void tab_changed(QWidget *);
  
private:
  QTabWidget* Tabs;
  QString cr;
};

#endif
