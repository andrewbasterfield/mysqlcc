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
#include "CTextFieldEditor.h"
#include "CSqlTableItem.h"
#include "CAction.h"
#include "CMySQLQuery.h"
#include "editor.h"
#include "globals.h"
#include <stddef.h>  
#include <qpopupmenu.h>
#include <qclipboard.h>
#include <qfiledialog.h>

CTextFieldEditor::CTextFieldEditor(QWidget *parent, CSqlTableItem *table_item, bool ro, const char* name)
: CFieldEditorWidget(parent, ro, name), tableItem(table_item)
{
}

void CTextFieldEditor::initFieldEditorWidget()
{ 
  mainWindow()->setName("CTextFieldEditor");
  mainWindow()->statusBar();
  
  setCaption(tr("Text Field Editor") + " - " + tr("Column") + ": '" + tableItem->query()->fields(tableItem->index()).name +
    "' row" + ": " + QString::number(tableItem->row()) );

  widgetLayout = new QGridLayout( this, 1, 1, 0, 0, "widgetLayout");
  editor = new Editor(this, "CTextFieldEditor");
  widgetLayout->addWidget(editor, 0, 0 );
  editor->setTextFormat(Qt::PlainText);
  editor->setText(tableItem->query()->mysql()->codec()->toUnicode(tableItem->value()));
  editor->setReadOnly(readOnly());  
  
  fileOpenAction = new CAction(mainWindow(), "fileOpenAction");
  fileOpenAction->setText(tr("Open"));
  fileOpenAction->setIconSet(getPixmapIcon("openIcon"));
  fileOpenAction->setMenuText(tr("&Open"));
  fileOpenAction->setAccel(Qt::CTRL + Qt::Key_O);
  fileOpenAction->setParentMenuText(tr("File"));
  fileOpenAction->setEnabled(!readOnly());
  
  fileSaveAsAction = new CAction(mainWindow(), "fileSaveAsAction");
  fileSaveAsAction->setText(tr("Save As"));
  fileSaveAsAction->setIconSet(getPixmapIcon("saveIcon"));
  fileSaveAsAction->setMenuText(tr("Save &As"));
  fileSaveAsAction->setParentMenuText(tr("File"));
  fileSaveAsAction->setAccel(0);
  
  filePrintAction = new CAction(mainWindow(), "filePrintAction");
  filePrintAction->setIconSet(getPixmapIcon("printIcon"));
  filePrintAction->setText(tr("Print"));
  filePrintAction->setMenuText(tr("&Print"));
  filePrintAction->setParentMenuText(tr("File"));
  filePrintAction->setAccel(Qt::CTRL + Qt::Key_P);
  
  editUndoAction = new CAction(mainWindow(), "editUndoAction");
  editUndoAction->setIconSet(getPixmapIcon("undoIcon"));
  editUndoAction->setText(tr("Undo"));
  editUndoAction->setMenuText(tr("&Undo"));
  editUndoAction->setAccel(Qt::CTRL + Qt::Key_Z);
  editUndoAction->setParentMenuText(tr("Edit"));
  editUndoAction->setEnabled(!readOnly());
  
  editRedoAction = new CAction(mainWindow(), "editRedoAction");
  editRedoAction->setIconSet(getPixmapIcon("redoIcon"));
  editRedoAction->setText(tr("Redo"));
  editRedoAction->setMenuText(tr("&Redo"));
  editRedoAction->setAccel(Qt::CTRL + Qt::Key_Y);
  editRedoAction->setParentMenuText(tr("Edit"));
  editRedoAction->setEnabled(!readOnly());
  
  editCutAction = new CAction(mainWindow(), "editCutAction");
  editCutAction->setIconSet(getPixmapIcon("cutIcon"));
  editCutAction->setText(tr("Cut"));
  editCutAction->setMenuText(tr("&Cut"));
  editCutAction->setAccel(Qt::CTRL + Qt::Key_X);
  editCutAction->setParentMenuText(tr("Edit"));
  editCutAction->setEnabled(!readOnly());
  
  editCopyAction = new CAction(mainWindow(), "editCopyAction");
  editCopyAction->setIconSet(getPixmapIcon("copyIcon"));
  editCopyAction->setText(tr("Copy"));
  editCopyAction->setMenuText(tr("&Copy"));
  editCopyAction->setAccel(Qt::CTRL + Qt::Key_C);
  editCopyAction->setParentMenuText(tr("Edit"));
    
  editPasteAction = new CAction(mainWindow(), "editPasteAction");
  editPasteAction->setIconSet(getPixmapIcon("pasteIcon"));
  editPasteAction->setText(tr("Paste"));
  editPasteAction->setMenuText(tr("&Paste"));
  editPasteAction->setAccel(Qt::CTRL + Qt::Key_V);
  editPasteAction->setParentMenuText(tr("Edit"));
  editPasteAction->setEnabled(!readOnly());
  
  fileCloseAction = new QAction(this, "fileCloseAction");
  Q_CHECK_PTR(fileCloseAction);
  fileCloseAction->setText(tr("Close"));
  fileCloseAction->setMenuText(tr("&Close"));
  fileCloseAction->setIconSet(getPixmapIcon("closeIcon"));
  fileCloseAction->setAccel(0);
  
  toolBar = new QToolBar(tr("Tools"), mainWindow());
  
  fileOpenAction->addTo(toolBar);
  fileSaveAsAction->addTo(toolBar);
  filePrintAction->addTo(toolBar);
  toolBar->addSeparator();
  editUndoAction->addTo(toolBar);
  editRedoAction->addTo(toolBar);
  editCopyAction->addTo(toolBar);
  editCutAction->addTo(toolBar);
  editPasteAction->addTo(toolBar);
  
  fileMenu = new QPopupMenu(this);
  
  fileOpenAction->addTo(fileMenu);
  fileSaveAsAction->addTo(fileMenu);
  fileMenu->insertSeparator();
  filePrintAction->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileCloseAction->addTo(fileMenu);
  mainWindow()->menuBar()->insertItem(tr("&File"), fileMenu);
  
  editMenu = new QPopupMenu(this);
  editUndoAction->addTo(editMenu);
  editRedoAction->addTo(editMenu);
  editMenu->insertSeparator();
  editCutAction->addTo(editMenu);
  editCopyAction->addTo(editMenu);
  editPasteAction->addTo(editMenu);
  mainWindow()->menuBar()->insertItem(tr("&Edit"), editMenu);
  addHotKeyEditorMenu();
  init();
}

void CTextFieldEditor::onApplyClicked()
{
  emit contentsChanged((const char*)(tableItem->query()->mysql()->codec()->fromUnicode(editor->text())), editor->text().length(), tableItem);
}

void CTextFieldEditor::setText(const QString &txt)
{
  editor->setText(txt);
}

QString CTextFieldEditor::text()const
{
  return editor->text();
}

void CTextFieldEditor::copyAvailable(bool b)
{
#ifndef QT_NO_CLIPBOARD
  if (b && myApp()->linuxPaste())
    QApplication::clipboard()->setText(editor->selectedText());
#endif
}

void CTextFieldEditor::selectionChanged()
{
  bool s = editor->hasSelectedText();
  if (!readOnly())
    editCutAction->setEnabled(s);
  editCopyAction->setEnabled(s);  
}

void CTextFieldEditor::paste()
{
  editor->paste();
}

void CTextFieldEditor::save()
{
  saveToFile(tmpFileName, "txt", tr("Text Files (*.txt);;All Files(*.*)" ), text(), 0);
}

void CTextFieldEditor::openFile()
{
  QString fn = QFileDialog::getOpenFileName(QString::null, tr("Text Files") + " (*.txt);;" + tr("All Files") + " (*.*)", this);
  if (!fn.isEmpty())
  {
    editor->load(fn);
    editor->setTextFormat(Qt::PlainText);
  }
}

void CTextFieldEditor::init()
{
  connect(fileCloseAction, SIGNAL(activated()), this, SLOT(close()));
  connect(editor, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
  connect(editor, SIGNAL(undoAvailable(bool)), editUndoAction, SLOT(setEnabled(bool)));
  connect(editor, SIGNAL(redoAvailable(bool)), editRedoAction, SLOT(setEnabled(bool)));
  connect(editor, SIGNAL(copyAvailable(bool)), editCopyAction, SLOT(setEnabled(bool)));
  connect(editor, SIGNAL(copyAvailable(bool)), this, SLOT(copyAvailable(bool)));
  
  connect(editCopyAction, SIGNAL(activated()), editor, SLOT(copy()));
  connect(fileSaveAsAction, SIGNAL(activated()), this, SLOT(save()));
  
  if (!readOnly())
  {
    connect(fileOpenAction, SIGNAL(activated()), this, SLOT(openFile()));
    connect(editUndoAction, SIGNAL(activated()), editor, SLOT(undo()));
    connect(editRedoAction, SIGNAL(activated()), editor, SLOT(redo()));
    connect(editCutAction, SIGNAL(activated()), editor, SLOT(cut()));
    connect(editPasteAction, SIGNAL(activated()), editor, SLOT(paste()));
  }
  
#ifdef QT_NO_PRINTER
  filePrintAction->setEnabled(false);
#else
  connect(filePrintAction, SIGNAL(activated()), editor, SLOT(print()));
#endif
}
