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
#ifndef CDATABASELISTVIEWITEM_H
#define CDATABASELISTVIEWITEM_H

#include <qvariant.h>
#include <qlistview.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qwidgetstack.h>
#include <qevent.h>
#include <qptrlist.h>
#include "CDatabaseListView.h"
#include "CToolBar.h"

class CMySQLServer;
class QWidget;
class CConsoleWindow;
class CMessagePanel;

class CDatabaseListViewItemMenu : public QObject
{
  Q_OBJECT

public:
  CDatabaseListViewItemMenu();
  ~CDatabaseListViewItemMenu();

  void createToolBar(CToolBar *t, int type);
  void createPopupMenu(QPopupMenu *p);
  void clear();
  void setItemEnabled(int idx, bool e);

  void insertItem(const QPixmap &pixmap, const QString &label, QPopupMenu * popup, int value);
  void insertItem(const QPixmap &pixmap, const QString &label, int value);
  void insertItem(const QString &label, int value);
  void insertSeparator();

signals:
  void clicked(int);

private slots:
  void Clicked(int);

private:

  class my_item
  {
  public:
    my_item(const QPixmap &p = 0, const QString &lbl = QString::null, int v = -1, bool sep=false)
    {
      pix = p;
      lab = lbl;
      val = v;
      separator = sep;
      ena = true;
      men = 0;
    }

    void setPopupMenu(QPopupMenu *m) { men = m; }
    QPopupMenu * popup() const { return men; }
    void setEnabled(bool b) { ena = b; }
    bool enabled() const { return ena; }
    QPixmap pixmap() const { return pix; }
    QString label() const { return lab; }
    int value() { return val; }
    bool isSeparator() { return separator; }

  private:
    QPopupMenu *men;
    QPixmap pix;
    QString lab;
    int val;
    bool separator;
    bool ena;
  };

  int cnt;
  my_item * item(int idx)
  {
    my_item *i;
    for (i = item_list.first(); i; i = item_list.next() )
      if (i->value() == idx)
        return i;
    return 0;
  }
  QPtrList<my_item> item_list;
};


class CDatabaseListViewItem : public QObject, public QListViewItem
{
  Q_OBJECT

public:
  enum item_type {NONE, SERVER, DATABASE_GROUP, DATABASE, TABLE_GROUP, TABLE, SERVER_ADMINISTRATION, USER_ADMINISTRATION, USER};

  CDatabaseListViewItem (CDatabaseListView * parent, CMySQLServer *m = 0, item_type t = NONE, const char *name=0);
  CDatabaseListViewItem (CDatabaseListViewItem * parent, CMySQLServer *m = 0, item_type t = NONE, const char *name=0);

  ~CDatabaseListViewItem();

  void setType(item_type t) { m_type = t; }
  item_type type() const { return m_type; }
  int int_type() const { return (int) m_type; }

  CDatabaseListView * databaseListView() const;
  virtual void activated() {};
  virtual void expanded() {};
  virtual void collapsed() {};
  virtual void doubleClicked() {};
  virtual void renamed(const QString &) {};
  virtual int displayMenu(const QPoint &);
  virtual void okRename(int col);
  virtual void createWindowMenu(CToolBar *);
  virtual void createPopupMenu(QPopupMenu *);
  virtual void initMenuItems() {};
  virtual void keyPressed(QKeyEvent *);
  virtual void refreshWidget(bool) {};

  QString databaseName() const { return m_dbname; }
  QString tableName() const { return m_table_name; }

  void setDatabaseName(const QString &s) { m_dbname = s; }
  void setTableName(const QString &s) { m_table_name = s; }


  void deleteChilds();
  void setOldText(const QString &s) { old_text = s; }
  QString oldText() { return old_text; }

  CConsoleWindow * consoleWindow() const;
  CMessagePanel * messagePanel() const;
  QWidgetStack * widgetStack() const;
  CMySQLServer * mysql() const { return m_mysql; }
  bool isBlocked() { return blocked; }

public slots:
  virtual void processMenu(int) {};
  void setBlocked(bool b);
  virtual void refresh();

protected:
  CMySQLServer *m_mysql;
  bool deletemysql;
  //void refreshWidget(bool) {}
  bool isBusy();
  CDatabaseListViewItemMenu *menu() { return &m_menu; }

private:
  CDatabaseListViewItemMenu m_menu;
  item_type m_type;
  QString old_text;
  QString m_dbname;
  QString m_table_name;
  bool blocked;
  void init();
};

#endif
