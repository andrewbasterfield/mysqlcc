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
#include "CTableProperties.h"
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "globals.h"
#include <stddef.h>  
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3GridLayout>

CTablePropertiesWidget::CTablePropertiesWidget(QWidget* parent,  CMySQLServer *m, const QString & t, const QString & d)
: CProperties(parent, m, "properties")
{
#ifdef DEBUG
  qDebug("CTablePropertiesWidget::CTablePropertiesWidget()");
#endif

  tableName = t;
  dbname = d;
  details->addColumn(tr("Property"));
  details->addColumn(tr("Value"));
  details->header()->setClickEnabled(false, 1);
  setSaveTitle(tr("Table Properties") + ": " + tableName);
  refresh();
}


void CTablePropertiesWidget::refresh()
{
#ifdef DEBUG
  qDebug("CTablePropertiesWidget::refresh()");
#endif
  
  setTitle(tr("Table") + ": '<b>" + tableName + "</b>'<br> " + tr("Database") + ": '" + dbname + "'");
  CProperties::refresh();
}


void CTablePropertiesWidget::insertData()
{
#ifdef DEBUG
  qDebug("CTablePropertiesWidget::insertData()");
#endif
  
  CMySQLQuery *query = new CMySQLQuery(mysql()->mysql());  
  query->setEmitMessages(false);
  query->setEmitErrors(false);
  if (query->exec("SHOW TABLE STATUS LIKE '" + tableName + "'"))
  {
    query->next();
    for (unsigned int i = 0; i < query->numFields(); i++)
      insertItem(query->fields(i).name, query->row(i));
  }
  delete query;
}


CTableProperties::CTableProperties(QWidget* parent, CMySQLServer *m , const QString &tablename, const QString &dbname, const char* name)
:CMyWindow(parent, name)
{
#ifdef DEBUG
  qDebug("CTableProperties::CTableProperties()");
#endif

  if (!name)
	  setName("CTableProperties");  
  setIcon(getPixmapIcon("applicationIcon"));  
  setCaption("[" + m->connectionName() + "] " + tr("Table Properties"));

  setCentralWidget(new QWidget(this, "qt_central_widget"));
  Form1Layout = new Q3GridLayout(centralWidget(), 1, 1, 2, 4, "Form1Layout"); 
  Form1Layout->addMultiCellWidget(new CTablePropertiesWidget(this, m, tablename, dbname), 0, 0, 0, 1);
  QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  Form1Layout->addItem(spacer, 1, 0);

  closeButton = new QPushButton(centralWidget(), "closeButton");
  closeButton->setMinimumSize(QSize(60, 0));
  closeButton->setMaximumSize(QSize(60, 32767));
  closeButton->setText(tr("&Close"));

  Form1Layout->addWidget(closeButton, 1, 1);
  Q3WhatsThis::add((QWidget *) closeButton, tr("Close this Dialog."));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
  myResize(300, 350);
}
