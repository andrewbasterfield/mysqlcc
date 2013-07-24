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
#ifndef CTYPETOOLBAR_H
#define CTYPETOOLBAR_H

#include <stddef.h>
#include <q3toolbar.h>
#include <qtoolbutton.h>
#include <q3intdict.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QPixmap>

class Q3MainWindow;

class CToolBarButton : public QToolButton
{
  Q_OBJECT

public:
  CToolBarButton(QWidget * parent, int v, Q3PopupMenu *p=0);
  int value() const { return val; }
  void setValue(int v) { val = v;}
  
signals:
  void button_clicked(int);
  
private slots:
  void Clicked();

private:
  int val;
};


class CToolBar : public Q3ToolBar
{
  Q_OBJECT

public:
  CToolBar (Q3MainWindow * parent = 0, const char * name = 0);
  void addButton(const QPixmap &pix, int val, Q3PopupMenu *p, bool ena, const QString &label = QString::null);
  void clear();

  void setType(int t) { m_type = t; }
  int type() const { return m_type; }
  CToolBarButton *button(int idx) const;

signals:
  void buttonClicked(int);

private slots:
  void button_clicked(int);

private:
  int cnt;
  int m_type;
  Q3IntDict<CToolBarButton> buttons_dict;
};

#endif
