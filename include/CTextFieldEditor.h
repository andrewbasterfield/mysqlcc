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
#ifndef CTEXTFIELDEDITOR_H
#define CTEXTFIELDEDITOR_H

#include "CFieldEditorWidget.h"

class CAction;
class Editor;
class QToolBar;
class CSqlTableItem;
class CMySQLServer;
class QPopupMenu;
class QGridLayout;
class QAction;

class CTextFieldEditor : public CFieldEditorWidget
{
  Q_OBJECT
    
public:
  CTextFieldEditor(QWidget *parent, CSqlTableItem *table_item, bool ro, const char* name = 0);
  void setText(const QString &txt);
  QString text() const;
  void initFieldEditorWidget();

private slots:
  void onApplyClicked();
  void selectionChanged();
  void copyAvailable(bool);
  void openFile();
  void save();
  void paste();

private:
  void init();
  QGridLayout* widgetLayout;
  QPopupMenu *fileMenu;
  QPopupMenu *editMenu;
  QToolBar *toolBar;
  CAction* fileOpenAction;
  CAction* fileSaveAsAction;
  CAction* filePrintAction;
  QAction* fileCloseAction;
  CAction* editUndoAction;
  CAction* editRedoAction;
  CAction* editCutAction;
  CAction* editCopyAction;
  CAction* editPasteAction;
  CSqlTableItem *tableItem;

  QString tmpFileName;
  Editor * editor;
};

#endif
