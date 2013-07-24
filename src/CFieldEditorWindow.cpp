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
#include <stddef.h>
#include <qvariant.h>
#include <q3frame.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include "CHotKeyEditorDialog.h"
#include "CFieldEditorWindow.h"
#include "CFieldEditorWidget.h"
#include "CApplication.h"
#include "CMyWindow.h"
#include "globals.h"

CFieldEditorWindow::CFieldEditorWindow(CFieldEditorWidget *editor, const char* name)
:CMyWindow(myApp()->workSpace(), name), close_on_apply(true)
{
#ifdef DEBUG
  qDebug("CFieldEditorWindow()");
#endif

  if (!name)
    setName("CFieldEditorWindow");
  setIcon(getPixmapIcon("applicationIcon"));
  setCaption(trUtf8("Field Editor"));
  setCentralWidget(new QWidget( this, "qt_central_widget"));
  CFieldEditorWindowLayout = new Q3GridLayout(centralWidget(), 1, 1, 2, 2, "CFieldEditorWindowLayout");

  editor->reparent(centralWidget(), 0, QPoint());
  editor->setMainWindow(this);

  connect(editor, SIGNAL(set_close_on_apply(bool)), this, SLOT(setCloseOnApply(bool)));
  connect(editor, SIGNAL(add_hot_key_editor_menu()), this, SLOT(addHotKeyEditorMenu()));

  connect(this, SIGNAL(apply_clicked()), editor, SLOT(onApplyClicked()));
  connect(this, SIGNAL(cancel_clicked()), editor, SLOT(onCancelClicked()));
  connect(this, SIGNAL(close_event(QCloseEvent *)), editor, SLOT(onClose(QCloseEvent *)));

  editor->initFieldEditorWidget();
    
  editor->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, 0, 0, editor->sizePolicy().hasHeightForWidth()));
  CFieldEditorWindowLayout->addWidget(editor, 0, 0);

  Frame5 = new Q3Frame(centralWidget(), "Frame5");
  Frame5->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, Frame5->sizePolicy().hasHeightForWidth()));
  Frame5->setFrameShape(Q3Frame::Box);
  Frame5->setFrameShadow(Q3Frame::Sunken);
  Frame5Layout = new Q3HBoxLayout(Frame5, 4, 6, "Frame5Layout");
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  Frame5Layout->addItem(spacer);
 
  applyPushButton = new QPushButton(Frame5, "applyPushButton");
  applyPushButton->setMinimumSize(QSize(70, 0));
  applyPushButton->setText(tr("&Apply"));
  applyPushButton->setEnabled(!editor->readOnly());
  connect(editor, SIGNAL(set_read_only(bool)), applyPushButton, SLOT(setDisabled(bool)));
  Frame5Layout->addWidget(applyPushButton);
    
  cancelPushButton = new QPushButton(Frame5, "cancelPushButton");
  cancelPushButton->setMinimumSize(QSize(70, 0));
  cancelPushButton->setText(tr("&Cancel")); 
  Frame5Layout->addWidget(cancelPushButton);
  CFieldEditorWindowLayout->addWidget(Frame5, 1, 0);

  connect(applyPushButton, SIGNAL(clicked()), this, SLOT(applyButtonClicked()));
  connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(close()));

  myResize(520, 350);
  editor->afterInitFieldEditorWidget();

  myApp()->incCritical();
}

CFieldEditorWindow::~CFieldEditorWindow()
{
  myApp()->decCritical();
}

void CFieldEditorWindow::applyButtonClicked()
{
  emit apply_clicked();
  if (close_on_apply)
    close();
}

void CFieldEditorWindow::cancelButtonClicked()
{
  emit cancel_clicked();
  close();
}

void CFieldEditorWindow::closeEvent(QCloseEvent * e)
{
  emit close_event(e);
  CMyWindow::closeEvent(e);
}

void CFieldEditorWindow::addHotKeyEditorMenu()
{
  new CHotKeyEditorMenu(this, menuBar(), "HotKeyEditor");
}
