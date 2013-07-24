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
#ifndef CCONFIGDIALOG_H
#define CCONFIGDIALOG_H

#include <stddef.h>
#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>
#include "CMyWindow.h"

class Q3VBoxLayout; 
class Q3HBoxLayout; 
class QPushButton;
class QTabWidget;
class CConfig;

class CConfigDialogTab : public QWidget
{
  Q_OBJECT
public:
  CConfigDialogTab(QWidget* parent = 0,  const char* name = 0, Qt::WFlags fl = 0)
    : QWidget(parent, name, fl) {} ;
  ~CConfigDialogTab() {};  
  friend class CConfigDialog;

protected:
  virtual bool needRestart() { return false; }
  virtual bool save(CConfig *) { return false; }
  virtual QString validate() const { return QString::null; }
  virtual void setDefaultValues(CConfig *) {}
};

class CConfigDialog : public CMyWindow
{ 
  Q_OBJECT
    
public:  
  CConfigDialog(QWidget* parent,  const char* name);
  ~CConfigDialog();
  void insertTab (CConfigDialogTab * child, int index = -1);
  void insertTab (CConfigDialogTab * child, const QIcon & iconset, int index = -1);  
  void setCurrentPage(int t = -1);
  QTabWidget * tab() const { return p_tab; }

public slots:
  void dialogAccepted();

protected slots:
  virtual void okButtonClicked() {};
  virtual bool save(CConfig *conn);

signals:
  void okClicked();

protected:  
  void init();
  QString validate() const;
  bool needRestart();
  void setDefaultValues(CConfig *conn);
    
  QPushButton* contextHelpButton;
  QPushButton* customButton;
  QPushButton* okPushButton;

private:
  QPushButton* cancelPushButton;
  Q3VBoxLayout* CConfigDialogLayout;
  Q3HBoxLayout* Layout44;
  QTabWidget* p_tab;

};

#endif
