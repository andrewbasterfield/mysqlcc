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
#ifndef CFIELDEDITORWIDGET_H
#define CFIELDEDITORWIDGET_H

#include <qwidget.h>

class QMainWindow;
class CSqlTableItem;

class CFieldEditorWidget : public QWidget
{
  Q_OBJECT

public:
  CFieldEditorWidget(QWidget *parent, bool readonly, const char* name = 0);
  virtual void initFieldEditorWidget() {};
  virtual void afterInitFieldEditorWidget() {};
  bool readOnly() { return is_read_only; }
  bool closeOnApply() { return close_on_apply; }
  QMainWindow *mainWindow() const { return main_window; }
  void setMainWindow(QMainWindow *w) { main_window = w; }
  void addHotKeyEditorMenu();
  void message(ushort type, const QString &message);  

signals:
  void contentsChanged(const char *, ulong, CSqlTableItem *);

  void send_message(ushort type, const QString & m);
  void set_read_only(bool);
  void set_close_on_apply(bool);
  void add_hot_key_editor_menu();
  void set_main_field_editor_widget(QWidget *);

public slots:
  virtual void onApplyClicked() {};
  virtual void onCancelClicked() {};
  virtual void onClose(QCloseEvent *e) { e->accept(); };
  virtual void setReadOnly(bool b);
  virtual void setCloseOnApply(bool b);

private:
  bool close_on_apply;
  bool is_read_only;
  QMainWindow *main_window;
  QWidget *main_field_editor_widget;
};

#endif
