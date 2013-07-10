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
#include "CTableTools.h"
#include <qimage.h>
#include <qlayout.h>
#include <qptrlist.h> 
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include "CMySQLServer.h"
#include "CMySQLQuery.h"
#include "CTablesListBox.h"
#include "CQueryWindow.h"
#include "globals.h"
#include "config.h"

CTableTools::CTableTools(QWidget* parent, CMySQLServer *m, const QString dbname, int type, QPtrList<ToolOptions> *options, const QString &tableName, const char* name)
:CMyWindow(parent, name)
{
#ifdef DEBUG
  qDebug("CTableTools::CTableTools(QWidget *, CMySQLServer *, '%s', %d, QPtrList<ToolOptions> *, '%s', const char *)", debug_string(dbname), type, debug_string(tableName));
#endif

  if (!name)
    setName("CTableTools");
  setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, sizePolicy().hasHeightForWidth()));
  setMinimumSize(QSize(280, 210));
  setIcon(getPixmapIcon("applicationIcon"));
  
  setCentralWidget(new QWidget(this, "qt_central_widget"));
  CTableToolsLayout = new QVBoxLayout(centralWidget()); 
  CTableToolsLayout->setSpacing(2);
  CTableToolsLayout->setMargin(2);
  
  m_pTopFrame = new QFrame(centralWidget(), "m_pTopFrame");
  m_pTopFrame->setFrameShape(QFrame::Box);
  m_pTopFrame->setFrameShadow(QFrame::Sunken);
  mysql = m;
  database_name = dbname;
  m_tableName = tableName;
  Options = options;
  Type = type;

  create();  
  CTableToolsLayout->addWidget(m_pTopFrame);
    
  Layout14 = new QHBoxLayout();
  
  Layout14->setSpacing(6);
  Layout14->setMargin(0);  

  PushButton4 = new QPushButton(centralWidget(), "PushButton4");
  PushButton4->setPixmap(getPixmapIcon("contextHelpIcon"));
  Layout14->addWidget(PushButton4);

  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  Layout14->addItem(spacer);
  
  m_pExecutePushButton = new QPushButton(centralWidget(), "m_pExecutePushButton");
  m_pExecutePushButton->setMinimumSize(QSize(60, 0));
  m_pExecutePushButton->setMaximumSize(QSize(60, 32767));
  Layout14->addWidget(m_pExecutePushButton);
  
  QString c;
  switch (Type)
  {
  case ANALYZE_TABLE: c = tr("Analyze Table");
    m_pExecutePushButton->setText(tr("&Analyze"));
    break;
  case CHECK_TABLE: c = tr("Check Table");
	m_pExecutePushButton->setText(tr("C&heck"));
    break;
  case OPTIMIZE_TABLE: c = tr("Optimize Table");
    m_pExecutePushButton->setText(tr("&Optimize"));
	break;
  case REPAIR_TABLE: c = tr("Repair Table");
	m_pExecutePushButton->setText(tr("&Repair"));
    break;
  case SHOW_CREATE_TABLE: c = tr("Show Create Table");
	m_pExecutePushButton->setText(tr("&Execute"));
    break;
  }
  setCaption("[" + m->connectionName() + "] " + c);

  m_pCancelPushButton = new QPushButton(centralWidget(), "m_pCancelPushButton");
  m_pCancelPushButton->setMinimumSize(QSize(60, 0));
  m_pCancelPushButton->setMaximumSize(QSize(60, 32767));
  m_pCancelPushButton->setText(tr("&Cancel", ""));
  
  Layout14->addWidget(m_pCancelPushButton);
  CTableToolsLayout->addLayout(Layout14);
  init();
  myResize(280, 210);
}

CTableTools::~CTableTools()
{
#ifdef DEBUG
  qDebug("CTableTools::~CTableTools()");
#endif

  delete Options;
}

bool CTableTools::inRange(int res)
{
#ifdef DEBUG
  qDebug("static CTableTools::inRange(%d)", res);
#endif

  return ((res >= ANALYZE_TABLE) && (res <= SHOW_CREATE_TABLE));
}

void CTableTools::create()
{
#ifdef DEBUG
  qDebug("CTableTools::create()");
#endif
  
  m_pTopFrameLayout = new QVBoxLayout(m_pTopFrame);
  m_pTopFrameLayout->setSpacing(4);
  m_pTopFrameLayout->setMargin(8);  
  tablesListBox = new CTablesListBox(m_pTopFrame, mysql, database_name, m_tableName);
  m_pTopFrameLayout->addWidget(tablesListBox);  
  
  if (!Options->isEmpty())
  {
    pOptions = new QGroupBox(m_pTopFrame, "pOptions");
    pOptions->setTitle(tr("Options"));
    pOptions->setColumnLayout(0, Qt::Vertical);
    pOptions->layout()->setSpacing(0);
    pOptions->layout()->setMargin(4);
    
    pOptionsLayout = new QGridLayout(pOptions->layout());
    pOptionsLayout->setAlignment(Qt::AlignTop);
    pOptionsLayout->setSpacing(2);
    pOptionsLayout->setMargin(2);    
    Layout = new QGridLayout;    
    Layout->setSpacing(6);
    Layout->setMargin(0);
    
    ToolOptions *opt;
    int col=0, row=0;
    for (opt = Options->first(); opt; opt = Options->next())
    {
      opt->checkBox = new QCheckBox(pOptions);
      opt->checkBox->setText(opt->display);        
      Layout->addWidget(opt->checkBox, row, col++);
      if (col >= 3)
      {
        col = 0;
        row++;
      }
    }
    pOptionsLayout->addLayout(Layout, 0, 0);
    m_pTopFrameLayout->addWidget(pOptions);
  }
}

void CTableTools::processMenu(CMySQLServer *m, int res, const QString dbname, const QString & tableName)
{
#ifdef DEBUG
  qDebug("static CTableTools::processMenu(CMySQLServer *, %d, '%s', '%s')", res, debug_string(dbname), debug_string(tableName));
#endif

  QPtrList<ToolOptions> *options = new QPtrList<ToolOptions>;
  Q_CHECK_PTR(options);
  switch (res)  //SETS OPTIONS.  Currently, only CHECK and REPAIR have options.
  {
    case CHECK_TABLE:
      {
        options->setAutoDelete(true);
        ToolOptions *quick = new ToolOptions;
        quick->display = "Quick";
        quick->value = "QUICK";
        options->append(quick);
    
        ToolOptions *fast = new ToolOptions;
        fast->display = "Fast";
        fast->value = "FAST";
        options->append(fast);

        ToolOptions *medium = new ToolOptions;
        medium->display = "Medium";
        medium->value = "MEDIUM";
        options->append(medium);
    
        ToolOptions *extended = new ToolOptions;
        extended->display = "Extended";
        extended->value = "EXTENDED";
        options->append(extended);
    
        ToolOptions *changed = new ToolOptions;
        changed->display = "Changed";
        changed->value = "CHANGED";
        options->append(changed);
      }
      break;
    
  case REPAIR_TABLE:
    {
      options->setAutoDelete(true);      
      ToolOptions *quick = new ToolOptions;
      quick->display = "Quick";
      quick->value = "QUICK";
      options->append(quick);
    
      ToolOptions *extended = new ToolOptions;
      extended->display = "Extended";
      extended->value = "EXTENDED";
      options->append(extended);

      ToolOptions *use_frm = new ToolOptions;
      use_frm->display = "Use FRM";
      use_frm->value = "USE_FRM";
      options->append(use_frm);

    }
    break;
  }    
  CTableTools * c = new CTableTools(myApp()->workSpace(), m, dbname, res, options, tableName);
  myShowWindow(c);
}

void CTableTools::execute()
{
#ifdef DEBUG
  qDebug("CTableTools::execute()");
#endif
  
  QString selectedTables = tablesListBox->selectedTables();
  mysql->mysql()->mysqlSelectDb(database_name);
  if (selectedTables.isNull())
    QMessageBox::warning(0,tr("Error"), tr("You must select at least one Table."));
  else
  {
    m_pExecutePushButton->disconnect();
    QString sql = QString::null;
    int flag = -1;
    if ((Type >= ANALYZE_TABLE) && (Type <= REPAIR_TABLE))
    {
      switch (Type)
      {
      case ANALYZE_TABLE: sql = "ANALYZE TABLE";
        break;
      case CHECK_TABLE: sql = "CHECK TABLE";
        break;
      case OPTIMIZE_TABLE: sql = "OPTIMIZE TABLE";
        break;
      case REPAIR_TABLE: sql = "REPAIR TABLE";
        break;
      }
      sql += "\n" + selectedTables + "\n";
      if (!Options->isEmpty())
      {
        sql += " ";
        ToolOptions *opt;
        for (opt = Options->first(); opt; opt = Options->next())
          if (opt->checkBox->isChecked())
            sql += opt->value + " ";
      }
      flag = RESULTS_PANEL;
    }
    else
    {
      if (Type == SHOW_CREATE_TABLE)      
      {        
        QStringList tables = tablesListBox->selectedTablesList();
        CMySQLQuery *query = new CMySQLQuery(mysql->mysql());
        query->setEmitMessages(false);
	    	QString str;
        for (QStringList::Iterator i = tables.begin(); i != tables.end(); i++)
        {
          if (query->exec("SHOW CREATE TABLE " + mysql->mysql()->quote(*i)))
          {
            str = tr("Host") + ": " + mysql->hostName() + "\n" + tr("Database") + ": " + database_name + "\n" + tr("Table") + ": '" + *i + "'\n";
            sql += myApp()->commentText(str, "\n");
            query->next();
            sql += "\n";
            sql += query->row(1);
          }
          sql += "; \n\n";
        }
        delete query;
        flag = SQL_PANEL;
      }
    }

    if (flag != -1)
    {
      CQueryWindow *w = new CQueryWindow(myApp()->workSpace(), mysql, SQL_QUERY, flag);
      w->setDatabase(database_name);
      w->setQuery(sql);
      myShowWindow(w);
      w->setFocus();
      w->raise();
      if (flag != SQL_PANEL)
        w->executeQuery();
    }
    close();
  }
}

QPopupMenu * CTableTools::tableToolsMenu()
{
#ifdef DEBUG
  qDebug("static CTableTools::tableToolsMenu()");
#endif

  QPopupMenu *p_toolsMenu = new QPopupMenu();
  p_toolsMenu->insertItem(tr("Analyze Table"), ANALYZE_TABLE);
  p_toolsMenu->insertItem(tr("Check Table"), CHECK_TABLE);
  p_toolsMenu->insertItem(tr("Optimize Table"), OPTIMIZE_TABLE);
  p_toolsMenu->insertItem(tr("Repair Table"), REPAIR_TABLE);
  p_toolsMenu->insertSeparator();
  p_toolsMenu->insertItem(tr("Show Create"), SHOW_CREATE_TABLE);
  return p_toolsMenu;
}

void CTableTools::NoTablesLoaded()
{
#ifdef DEBUG
  qDebug("CTableTools::NoTablesLoaded()");
#endif

  m_pExecutePushButton->setEnabled(false);
}

void CTableTools::init()
{
#ifdef DEBUG
  qDebug("CTableTools::init()");
#endif

  QWhatsThis::add(this, tr("Select one or more Tables."));
  QWhatsThis::add(m_pExecutePushButton,tr("Click to execute."));
  QWhatsThis::add(m_pCancelPushButton, tr("Close this Dialog without executing.")); 
  connect(m_pCancelPushButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(m_pExecutePushButton, SIGNAL(clicked()), this, SLOT(execute()));
  connect(PushButton4, SIGNAL(clicked()), this, SLOT(whatsThis()));
}
