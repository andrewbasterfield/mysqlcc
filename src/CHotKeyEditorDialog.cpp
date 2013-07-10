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
#include "CHotKeyEditorDialog.h"
#include "CConfig.h"
#include "globals.h"
#include "config.h"
#include "CAction.h"

#include <qpushbutton.h>
#include <qvariant.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qobjectlist.h>


CHotKeySetupBox::CHotKeySetupBox(QWidget *parent, QKeySequence *key, const QString &caption, const QString &text)
: QMessageBox(caption, text , NoIcon, Ok, Cancel, 0, parent, "CHotKeySetupBox", true, WDestructiveClose)
{
#ifdef DEBUG
  qDebug("CHotKeySetupBox::CHotKeySetupBox()");
#endif

  txt = text;
  txt += "\n";
  setText(txt);
  done = false;
  keysequence = key;
  keysec = 0;
}

void CHotKeySetupBox::keyPressEvent(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CHotKeySetupBox::keyPressEvent()");
#endif

  int key = e->key();  
  switch (key) {
  case Qt::Key_Control: keysec = Qt::CTRL;
    key = 0;
    break;
  case Qt::Key_Alt: keysec = Qt::ALT;
    key = 0;
    break;
  case Qt::Key_Shift: keysec = Qt::SHIFT;
    key = 0;
    break;
  default: *keysequence = QKeySequence(keysec + key);
    txt += (QString) *keysequence;
    setText(txt);
    done = true;
    break;
  }  
  e->accept();
}

void CHotKeySetupBox::keyReleaseEvent(QKeyEvent * e)
{
#ifdef DEBUG
  qDebug("CHotKeySetupBox::keyReleaseEvent()");
#endif

  e->accept();
  if (done)
    accept();
}

/*
This class takes care of displaying the hotkey menu and it also opens the hotkey editor dialog.
*/
CHotKeyEditorMenu::CHotKeyEditorMenu(QWidget * parent, QMenuBar *menuBar, const char * name)
:QPopupMenu(parent, name)
{
#ifdef DEBUG
  qDebug("CHotKeyEditorMenu::CHotKeyEditorMenu()");
#endif
  
  QAction *hotKeyEditorAction = new QAction(tr("HotKey Editor"), getPixmapIcon("keyboardIcon"),
    tr("Hot&Key Editor"), 0, this, "hotKeyEditorAction");
  connect(hotKeyEditorAction, SIGNAL(activated()), this, SLOT(openHotKeyEditor()));
  hotKeyEditorAction->addTo(this);
  menuBar->insertItem(tr("Hot&Keys"), this);
}

/*
This slot opens the hotkey editor
*/
void CHotKeyEditorMenu::openHotKeyEditor()
{
#ifdef DEBUG
  qDebug("CHotKeyEditorMenu::openHotKeyEditor()");
#endif

  CHotKeyEditorDialog *hotKeyDialog = new CHotKeyEditorDialog(myApp()->workSpace(), (QWidget *) parent());
  connect(parent(), SIGNAL(about_to_close()), hotKeyDialog, SLOT(close()));
  myShowWindow(hotKeyDialog);
}


CHotKeyEditorTab::CHotKeyEditorTab( QWidget* parent, QWidget* wnd, const char* name, WFlags fl )
: CConfigDialogTab ( parent, name, fl )
{
#ifdef DEBUG
  qDebug("CHotKeyEditorTab::CHotKeyEditorTab()");
#endif

  if ( !name )
    setName( "CHotKeyEditorTab" );
  window = wnd;
  warningIcon = getPixmapIcon("warningIcon");
  setCaption(trUtf8("HotKey Editor"));
  CHotKeyEditorTabLayout = new QGridLayout( this, 1, 1, 4, 2, "CHotKeyEditorTabLayout"); 
  
  ListView1 = new QListView( this, "ListView1" );
  ListView1->setShowSortIndicator(true);
  ListView1->addColumn(trUtf8( "Action" ) );
  ListView1->addColumn(trUtf8( "HotKey" ) );  
  ListView1->setResizeMode(QListView::AllColumns );
  QWhatsThis::add(ListView1, trUtf8( "Action is the column that contains items that can have a hot-key defined.\n"
    "\n"
    "HotKey is the column that displays the hot-key combination for the specified Action."));
  CHotKeyEditorTabLayout->addWidget( ListView1, 0, 0 );  
}

void CHotKeyEditorTab::initHotKeys()
{
#ifdef DEBUG
  qDebug("CHotKeyEditorTab::initHotKeys()");
#endif

  ListView1->clear();
  QObjectList *l = window->queryList("CAction", 0, false, false);

  if (l->isEmpty())
    emit enableButtons(false);
  else
  {
    QObjectListIt it( *l );
    CAction *action;
    while ((action = (CAction *) it.current()) != 0)
    {
      CHotKeyListViewItem * item = new CHotKeyListViewItem(ListView1, action);      
      item->setText(0, action->parentMenuText() + " | " + action->text());
      if ((int) action->accel() == 0)
        item->setText(1, QString::null);
      else
        item->setText(1, action->accel());
      ++it;    
    }
    ListView1->setSelected(ListView1->currentItem(), true);
  }
  checkDuplicates();
}

void CHotKeyEditorTab::mapKeys()
{
#ifdef DEBUG
  qDebug("CHotKeyEditorTab::mapKeys()");
#endif

  if (ListView1->currentItem() == 0)
    return;  
  QKeySequence key;
  CHotKeySetupBox *mb = new CHotKeySetupBox(0, &key, tr("HotKey Setup"), tr("Press the key-combination you want as a HotKey for the selected Action."));
  if (mb->exec() == CHotKeySetupBox::Ok)  //user accepted the dialog
  {
    CHotKeyListViewItem *item = (CHotKeyListViewItem *)ListView1->currentItem();
    item->action()->setTmpAccel(key);
    item->setModified(true);
    if ((int) key != 0)
      ListView1->currentItem()->setText(1, key);
    else
      ListView1->currentItem()->setText(1, QString::null);
  }
  checkDuplicates();
}


void CHotKeyEditorTab::checkDuplicates()
{
#ifdef DEBUG
  qDebug("CHotKeyEditorTab::checkDuplicates()");
#endif

  QPixmap pm;
  QListViewItemIterator it3(ListView1);
  while (it3.current() != 0)
  {
    it3.current()->setPixmap(0, pm);
    ++it3;
  }
  QListViewItemIterator it(ListView1);    
  while ( it.current() != 0 )
  {
    if (!it.current()->text(1).isEmpty())
    {
      QListViewItemIterator it2(ListView1);
      while (it2.current() != 0)
      {
        if (it2.current() != it.current() && it.current()->text(1) == it2.current()->text(1))
          it2.current()->setPixmap(0, warningIcon);
        ++it2;
      }
    }
    ++it;
  }
}


bool CHotKeyEditorTab::save(CConfig *cfg)
{
#ifdef DEBUG
  qDebug("CHotKeyEditorTab::save()");
#endif
  
  bool ret = true;
  QPtrList<QListViewItem> lst;
  QListViewItemIterator it(ListView1);
  while ( it.current() != 0 )
  {
    CHotKeyListViewItem *item = (CHotKeyListViewItem *)it.current();    
    if (item->modified())
    {
      item->action()->setAccel(item->action()->tmpAccel());
      ret &= cfg->writeEntry(item->action()->name(), (int) item->action()->accel());      
    }
    ++it;
  }
  return ret;  
}


CHotKeyEditorDialog::CHotKeyEditorDialog(QWidget* parent,  QWidget *wnd)
:CConfigDialog(parent, "CHotKeyEditorDialog")
{
#ifdef DEBUG
  qDebug("CHotKeyEditorDialog::CHotKeyEditorDialog()");
#endif

  myApp()->incCritical();
  setMinimumHeight(263);  
  setCaption(tr("HotKey Configuration Dialog"));
  hotKeyTab = new CHotKeyEditorTab((QWidget *) tab(), wnd);
  parentWindowName = wnd->name();
  connect(hotKeyTab, SIGNAL(enableButtons(bool)), this, SLOT(enableButtons(bool)));
  hotKeyTab->initHotKeys();
  insertTab(hotKeyTab);
  okPushButton->setText(tr("&Apply"));
  QWhatsThis::add(okPushButton, tr("Click to Apply changes."));
  customButton->setText(tr( "&Map Keys"));
  customButton->setIconSet(getPixmapIcon("keyboardIcon"));
  QWhatsThis::add(customButton, tr("Click here to map the keys for the selected Action.."));
  connect(customButton, SIGNAL(clicked()), this, SLOT(customButtonClicked()));
  connect(this, SIGNAL(mapKeys()), hotKeyTab, SLOT(mapKeys()));
  customButton->show();
  myResize(421, 263);
}

void CHotKeyEditorDialog::customButtonClicked()
{
#ifdef DEBUG
  qDebug("CHotKeyEditorDialog::customButtonClicked()");
#endif

  emit mapKeys();
}

void CHotKeyEditorDialog::enableButtons(bool b)
{
#ifdef DEBUG
  qDebug("CHotKeyEditorDialog::enableButtons()");
#endif
  
  customButton->setEnabled(b);
  okPushButton->setEnabled(b);
}

CHotKeyEditorDialog::~CHotKeyEditorDialog()
{
#ifdef DEBUG
  qDebug("CHotKeyEditorDialog::~CHotKeyEditorDialog()");
#endif

  myApp()->decCritical();
}

void CHotKeyEditorDialog::okButtonClicked()
{
#ifdef DEBUG
  qDebug("CHotKeyEditorDialog::okButtonClicked()");
#endif
  
  CConfig *conn = new CConfig(parentWindowName.lower() + ".cfg", QString(HOTKEY_PATH));
  save(conn);
  conn->save();
  delete conn;
  dialogAccepted();
}
