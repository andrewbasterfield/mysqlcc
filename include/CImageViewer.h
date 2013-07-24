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
#ifndef CIMAGEVIEWER_H
#define CIMAGEVIEWER_H

#include "CFieldEditorWidget.h"
//Added by qt3to4:
#include <Q3PopupMenu>
#include <Q3GridLayout>
#include <QLabel>

class CAction;
class QAction;
class Q3ToolBar;
class CSqlTableItem;
class QLabel;
class CMySQLServer;
class QToolButton;
class Q3PopupMenu;
class QMenuBar;
class Q3GridLayout;

class CImageViewer : public CFieldEditorWidget
{
  Q_OBJECT
    
public:
  CImageViewer(QWidget *parent, CSqlTableItem *table_item, bool ro, const char* name = 0);
  ~CImageViewer();
  void initFieldEditorWidget();
  void afterInitFieldEditorWidget();

private slots:
  void openFile();
  void saveImage(int);
  void copyImageToClipboard();
  void scaleContents(bool);
  void canSave(bool);
  void onApplyClicked();

private:
  void init();
  bool loadFromData(const uchar *data, ulong len);
  char *new_data;
  ulong new_data_len;
  bool has_modified;
  Q3GridLayout* widgetLayout;
  Q3PopupMenu *fileMenu;
  Q3PopupMenu *editMenu;
  QMenuBar *menubar;

  Q3ToolBar *toolBar;

  CAction* fileOpenAction;
  CAction* fileSaveAsAction;
  QAction* fileCloseAction;
  CAction* editCopyAction;
  CAction* editScaleContentsAction;
  QToolButton *saveImageTypeButton;
  Q3PopupMenu *saveImageTypeMenu;

  CSqlTableItem *tableItem;
  QString tmpFileName;
  QLabel *image;
};

#endif
