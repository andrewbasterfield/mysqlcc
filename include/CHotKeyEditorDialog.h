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
#ifndef CHOTKEYEDITORDIALOG_H
#define CHOTKEYEDITORDIALOG_H

#include <stddef.h>
#include <qvariant.h>
#include <qwidget.h>
#include <q3popupmenu.h>
#include <q3listview.h>
#include <qmessagebox.h>
#include <qkeysequence.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QKeyEvent>
#include "CConfigDialog.h"

class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class CConfig;

class CHotKeySetupBox : public QMessageBox
{
  Q_OBJECT

public:
  CHotKeySetupBox (QWidget * parent, QKeySequence *key, const QString &caption, const QString &text);

private:
  void keyPressEvent(QKeyEvent * e);
  void keyReleaseEvent(QKeyEvent * e);
  QKeySequence *keysequence;
  QString txt;
  int keysec;
  bool done;  
};

class CHotKeyListViewItem : public Q3ListViewItem
{
public:
  CHotKeyListViewItem ( Q3ListView * parent, CAction * a = 0 )
    : Q3ListViewItem(parent), m_action(a)
  {
    mod = false;
  }

  void setAction(CAction *a) { m_action = a; }
  void setModified(bool b) { mod = b; }
  bool modified() const { return mod; }
  CAction * action() const { return m_action; }

private:
  CAction *m_action;
  bool mod;
};

class CHotKeyEditorMenu : public Q3PopupMenu
{
  Q_OBJECT

public:
  CHotKeyEditorMenu(QWidget* parent = 0, QMenuBar *menuBar = 0, const char* name = 0);
  ~CHotKeyEditorMenu() {};

public slots:
  void openHotKeyEditor();
};

class CHotKeyEditorTab : public CConfigDialogTab
{ 
  Q_OBJECT
    
public:  
  CHotKeyEditorTab(QWidget* parent, QWidget * wnd, const char* name = 0, Qt::WFlags fl = 0);
  bool save(CConfig *cfg);
  void initHotKeys();

public slots:
  void mapKeys();

signals:
  void enableButtons(bool);

private:
  void checkDuplicates();
  QPixmap warningIcon;
  Q3GridLayout *CHotKeyEditorTabLayout;
  Q3ListView *ListView1;
  QWidget *window;
};

class CHotKeyEditorDialog : public CConfigDialog
{ 
  Q_OBJECT
    
public:
  CHotKeyEditorDialog (QWidget* parent,  QWidget * wnd);
  ~CHotKeyEditorDialog();    

private slots:
  void enableButtons(bool b);
  void customButtonClicked();

signals:
  void mapKeys();

private:
  QString parentWindowName;
  void okButtonClicked();
  CHotKeyEditorTab * hotKeyTab;
};

#endif
