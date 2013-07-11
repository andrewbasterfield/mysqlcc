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
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qintdict.h>
#include <qpopupmenu.h>

class QMainWindow;

class CToolBarButton : public QToolButton
{
  Q_OBJECT

public:
  CToolBarButton(QWidget * parent, int v, QPopupMenu *p=0);
  int value() const { return val; }
  void setValue(int v) { val = v;}
  
signals:
  void button_clicked(int);
  
private slots:
  void Clicked();

private:
  int val;
};


class CToolBar : public QToolBar
{
  Q_OBJECT

public:
  CToolBar (QMainWindow * parent = 0, const char * name = 0);
  void addButton(const QPixmap &pix, int val, QPopupMenu *p, bool ena, const QString &label = QString::null);
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
  QIntDict<CToolBarButton> buttons_dict;
};

#endif
