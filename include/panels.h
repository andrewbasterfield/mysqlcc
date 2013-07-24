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

#include <stddef.h>
#include <qvariant.h>
#include <q3listview.h>
#include <qtabwidget.h>
#include <qsize.h>
#include <q3intdict.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include "CDockWindow.h"

class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class QLabel;

class CMessagePanel;
class CPanel;

class CPanel : public Q3ListView
{
  Q_OBJECT
    
public:
  CPanel(QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0);
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
  void setBottomItem(Q3ListViewItem *i=0);

protected slots:
  virtual void displayMenu(Q3ListViewItem *, const QPoint &, int) {} ;
  
protected:
  virtual void save();
  virtual void copy(Q3ListViewItem *);
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
  Q3IntDict<QString> realtext;
};

class CPanelItem : public Q3ListViewItem
{
public:
  CPanelItem(Q3ListView * parent, const QString &t=QString::null, const QPixmap &p=0);
  CPanelItem(Q3ListView * parent, Q3ListViewItem * after, const QString &t=QString::null, const QPixmap &p=0);
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
  CMessagePanel(const QString &caption=QString::null, QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0);
  
public slots:
  void warning(const QString & m);
  void critical(const QString & m);
  void information(const QString & m);
  void message(ushort type, const QString & m);
  
private slots:  
  void displayMenu(Q3ListViewItem *, const QPoint &, int);

private:
  QPixmap warningIcon;
  QPixmap criticalIcon;
  QPixmap informationIcon;
};


class CHistoryPanel : public CPanel
{
  Q_OBJECT
    
public:
  CHistoryPanel(const QString &caption=QString::null, QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0);  
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
  void setCurrentHint(Q3ListViewItem * i);
  void DoubleClicked(Q3ListViewItem * i);
  void displayMenu(Q3ListViewItem *, const QPoint &, int);
  void mouseClicked(int, Q3ListViewItem * i, const QPoint &, int);
  
private:
  QPixmap historyScriptIcon;
  bool itemsEnabled;
};


class CSqlDebugPanel : public CPanel
{
  Q_OBJECT
    
public:
  CSqlDebugPanel(const QString &caption=QString::null, QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0);
  QString selectedHintText();
  
public slots:
  void SqlDebug(const QString & m);
  
private slots:
  void displayMenu(Q3ListViewItem *, const QPoint &, int);
  void setCurrentHint(Q3ListViewItem * i);
  void mouseClicked(int, Q3ListViewItem * i, const QPoint &, int);
  
private:
  void copy(Q3ListViewItem *);
  QPixmap historyScriptIcon;
};


class CMessageWindow : public CDockWindow
{
  Q_OBJECT
    
public:
  CMessageWindow (Place p = InDock, QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0) ;
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
