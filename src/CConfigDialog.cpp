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
#include "CConfig.h"
#include "globals.h"
#include "CConfigDialog.h"
#include <stddef.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

CConfigDialog::CConfigDialog(QWidget* parent,  const char* name)
:CMyWindow(parent, name)
{
#ifdef DEBUG
  qDebug("CConfigDialog::CConfigDialog()");
#endif

  if (!name)
    setName("CConfigDialog");  
  setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth()));  
  setMaximumSize(QSize(32767, 32767));  
  setIcon(getPixmapIcon("applicationIcon"));
  setCentralWidget(new QWidget(this, "qt_central_widget"));  
  CConfigDialogLayout = new QVBoxLayout(centralWidget(), 4, 2, "CConfigDialogLayout");   
  
  p_tab = new QTabWidget(centralWidget(), "p_tab");  
  QWhatsThis::add(p_tab, tr("This is the Configuration Dialog."));
  
  CConfigDialogLayout->addWidget(p_tab);
  
  Layout44 = new QHBoxLayout(0, 0, 6, "Layout44");

  contextHelpButton = new QPushButton(centralWidget(), "contextHelpButton");  
  contextHelpButton->setPixmap(getPixmapIcon("contextHelpIcon"));
  Layout44->addWidget(contextHelpButton);

  customButton = new QPushButton(centralWidget(), "customButton");
  Layout44->addWidget(customButton);
  customButton->hide();

  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  Layout44->addItem(spacer);

  okPushButton = new QPushButton(centralWidget(), "okPushButton");
  Q_CHECK_PTR(okPushButton);
  okPushButton->setMinimumSize(QSize(60, 0));
  okPushButton->setText(tr("&Ok"));
  okPushButton->setDefault(true);  
  Layout44->addWidget(okPushButton);

  cancelPushButton = new QPushButton(centralWidget(), "cancelPushButton");
  Q_CHECK_PTR(cancelPushButton);
  cancelPushButton->setMinimumSize(QSize(70, 0));
  cancelPushButton->setText(tr("&Cancel"));
  QWhatsThis::add(cancelPushButton, tr("Close this Dialog without saving any changes you have made."));
  Layout44->addWidget(cancelPushButton);
  CConfigDialogLayout->addLayout(Layout44);
  
  init();
}

void CConfigDialog::setCurrentPage(int t)
{
#ifdef DEBUG
  qDebug("CConfigDialog::setCurrentPage(%d)", t);
#endif

  if (t != -1)
    p_tab->setCurrentPage(t);
}

void CConfigDialog::insertTab (CConfigDialogTab * child, int index)
{
#ifdef DEBUG
  qDebug("CConfigDialog::insertTab(CConfigDialogTab *, %d)", index);
#endif

  p_tab->insertTab(child, child->caption(), index);
}

void CConfigDialog::insertTab (CConfigDialogTab * child, const QIconSet & iconset, int index)
{
#ifdef DEBUG
  qDebug("CConfigDialog::insertTab(CConfigDialogTab *, const QIconSet &, %d)", index);
#endif

  p_tab->insertTab(child, iconset, child->caption(), index);
}

CConfigDialog::~CConfigDialog()
{
#ifdef DEBUG
  qDebug("CConfigDialog::~CConfigDialog()");
#endif

}

bool CConfigDialog::needRestart()
{
#ifdef DEBUG
  qDebug("CConfigDialog::setDefaultValues()");
#endif
  
  for (int i = 0; i < p_tab->count(); i++)
    if (((CConfigDialogTab *)p_tab->page(i))->needRestart())
      return true;

  return false;
}

void CConfigDialog::setDefaultValues(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CConfigDialog::setDefaultValues()");
#endif

  for (int i = 0; i < p_tab->count(); i++)
    ((CConfigDialogTab *)p_tab->page(i))->setDefaultValues(conn);  
}

QString CConfigDialog::validate() const
{
#ifdef DEBUG
  qDebug("CConfigDialog::validate()");
#endif

  QString err = QString::null;
  for (int i = 0; i < p_tab->count(); i++)
    err += ((CConfigDialogTab *)p_tab->page(i))->validate();
  return err;  
}

bool CConfigDialog::save(CConfig *conn)
{	
#ifdef DEBUG
  qDebug("CConfigDialog::save()");
#endif

  bool ret = true; 
  for (int i = 0; i < p_tab->count(); i++)
    ret &= ((CConfigDialogTab *)p_tab->page(i))->save(conn);  
  return ret;
}

void CConfigDialog::dialogAccepted()
{
  emit okClicked();
  close();
}

void CConfigDialog::init()
{
#ifdef DEBUG
  qDebug("CConfigDialog::init()");
#endif

  connect(okPushButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));  
  connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(contextHelpButton, SIGNAL(clicked()), this, SLOT(whatsThis()));
}
