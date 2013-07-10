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
#include "CImageViewer.h"
#include "CSqlTableItem.h"
#include "CAction.h"
#include "CMySQLQuery.h"
#include "globals.h"

#include <qlabel.h>
#include <qclipboard.h>
#include <qfiledialog.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <ctype.h>

QString g_img_fname_prompt;

static inline void append_img_ext(QString& s, const char* ext)
{
  const char *p;
  char c;
  for (p=ext; (c=tolower(*p)); p++)
  {
    if (c == 'e' && p == ext+2 && tolower(*ext) == 'j' &&
	    tolower(ext[1]) == 'p')
      continue;
    s += c;
  }
}

void init_img_fname_prompt()
{
  g_img_fname_prompt = QObject::tr("Image Files");
  g_img_fname_prompt += " (";
  QStrList fmt = QImage::outputFormats();  
  for (const char* f = fmt.first(); f; f = fmt.next())
  {
    g_img_fname_prompt += '*';
    g_img_fname_prompt += '.';
    append_img_ext(g_img_fname_prompt,f);
    g_img_fname_prompt += ' ';
  }  
  g_img_fname_prompt += ");;";
  g_img_fname_prompt += QObject::tr("All Files");
  g_img_fname_prompt += " (*.*)";
}

CImageViewer::CImageViewer(QWidget *parent, CSqlTableItem *table_item, bool ro, const char *name)
: CFieldEditorWidget(parent, ro, name), tableItem(table_item)
{
  new_data = 0;
  new_data_len = 0;
  has_modified = false;
  tmpFileName = QString::null;
}

void CImageViewer::initFieldEditorWidget()
{
  mainWindow()->statusBar();
  mainWindow()->setName("CImageViewer");
  init_img_fname_prompt();
  mainWindow()->setCaption(tr("Image Viewer") + " - " + tr("Column") + ": '" + tableItem->query()->fields(tableItem->index()).name +
    "' Row" + ": " + QString::number(tableItem->row()) );
  
  widgetLayout = new QGridLayout( this, 1, 1, 0, 0, "widgetLayout"); 

  image = new QLabel(this, "image");
  image->setAlignment(int( QLabel::AlignCenter));
  image->setFrameShape(QLabel::Panel);
  image->setFrameShadow(QLabel::Sunken);
  image->setScaledContents(false);

  widgetLayout->addWidget(image, 0, 0 );
  
  fileOpenAction = new CAction (tr("Open"), getPixmapIcon("openIcon"),
    tr("&Open"), Qt::CTRL + Qt::Key_O, mainWindow(), "fileOpenAction");
  fileOpenAction->setParentMenuText(tr("File"));
  fileOpenAction->setEnabled(!readOnly());

  editScaleContentsAction = new CAction (tr("Scale Contents"), getPixmapIcon("scalePictureIcon"),
    tr("Sc&ale Contents"), Qt::CTRL + Qt::Key_A, mainWindow(), "editScaleContentsAction", true);
  editScaleContentsAction->setParentMenuText(tr("Edit"));

  editCopyAction = new CAction (tr("Copy"), getPixmapIcon("copyIcon"),
    tr("&Copy"), Qt::CTRL + Qt::Key_C, mainWindow(), "editCopyAction");
  editCopyAction->setParentMenuText(tr("Edit"));

  fileCloseAction = new QAction (tr("Close"), getPixmapIcon("closeIcon"),
    tr("&Close"), 0, this, "fileCloseAction");

  saveImageTypeMenu = new QPopupMenu(this);
  QStrList fmt = QImage::outputFormats();  
  for (const char* f = fmt.first(); f; f = fmt.next())
    saveImageTypeMenu->insertItem(f);

  toolBar = new QToolBar(tr("Tools"), mainWindow());
  
  fileOpenAction->addTo(toolBar);

  saveImageTypeButton = new QToolButton(toolBar);
  saveImageTypeButton->setPopup(saveImageTypeMenu);
  saveImageTypeButton->setPixmap(getPixmapIcon("saveIcon"));
  saveImageTypeButton->setTextLabel("Save As", true);
  saveImageTypeButton->setPopupDelay (0);

  toolBar->addSeparator();
  editCopyAction->addTo(toolBar);
  toolBar->addSeparator();
  editScaleContentsAction->addTo(toolBar);

  fileMenu = new QPopupMenu(this);
  
  fileOpenAction->addTo(fileMenu);

  fileMenu->insertItem(getPixmapIcon("saveIcon"), tr("Save &As"), saveImageTypeMenu, 1);
  
  fileMenu->insertSeparator();
  fileCloseAction->addTo(fileMenu);
  mainWindow()->menuBar()->insertItem(tr("&File"), fileMenu);
  
  editMenu = new QPopupMenu(this);
  editScaleContentsAction->addTo(editMenu);
  editMenu->insertSeparator();
  editCopyAction->addTo(editMenu);
  mainWindow()->menuBar()->insertItem(tr("&Edit"), editMenu);

  connect(fileCloseAction, SIGNAL(activated()), mainWindow(), SLOT(close()));
  connect(editCopyAction, SIGNAL(activated()), this, SLOT(copyImageToClipboard()));
  connect(saveImageTypeMenu, SIGNAL(activated(int)), this, SLOT(saveImage(int)));
  connect(editScaleContentsAction, SIGNAL(toggled(bool)), this, SLOT(scaleContents(bool)));
  
  if (!readOnly())
    connect(fileOpenAction, SIGNAL(activated()), this, SLOT(openFile()));

  addHotKeyEditorMenu();
}

void CImageViewer::afterInitFieldEditorWidget()
{
  /*
  This goes in "afterInitFieldEditorWidget()" because it resizes the mainWindow() to the size of the widget.
  This means this function is called after all the resizing and placing routines are called in mainWindow().
  */
  loadFromData((const uchar *) tableItem->value(), tableItem->length());
  has_modified = false;
}

CImageViewer::~CImageViewer()
{
  if (new_data)
    delete [] new_data;
}

void CImageViewer::copyImageToClipboard()
{
#ifndef QT_NO_CLIPBOARD
  if (!image->pixmap()->isNull())
    QApplication::clipboard()->setPixmap(*image->pixmap());
#endif
}

void CImageViewer::canSave(bool b)
{
  saveImageTypeButton->setEnabled(b);
  saveImageTypeMenu->setEnabled(b);
}

void CImageViewer::saveImage(int id)
{
  QString fmt = saveImageTypeMenu->text(id);
  if (fmt.lower() == "jpg")
    fmt = "JPEG";

  QString ext;
  append_img_ext(ext,(const char*)fmt);
  tmpFileName = getSaveFileName("", ext, fmt + "(*." + ext + ")");
  if (tmpFileName.isEmpty())
    return;

  if (myApp()->confirmCritical())
  {
    QFile file(tmpFileName);
    if (file.exists())
      if ((QMessageBox::warning(0, tr("Replace File"), tr("The specified file name already exists.\nDo you want to replace it ?"),
        QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes))              
        return;
    file.close();
  }

  if (!image->pixmap()->isNull())
  {
    image->pixmap()->save(tmpFileName, fmt);
    message(INFORMATION, tr("Successfully saved") + ": " + tmpFileName);
  }
  else
    message(CRITICAL, tr("An error occurred while saving the file"));
}


bool CImageViewer::loadFromData(const uchar *data, ulong len)
{
  bool b = false;
  if (data && len > 0)
  {
    QPixmap p;    
    if (b = p.loadFromData(data, len))
    {
      image->setPixmap(p);
      mainWindow()->resize(p.width(), p.height());
      if (new_data)
        delete [] new_data;
      new_data = new char [len + 1];
      memcpy(new_data, data, len);
      *(new_data + len) = 0;
      new_data_len = len;
      has_modified = true;
    }
    else
      message(CRITICAL, tr("An error occurred while loading the image."));
  }
  else
  {
    if (new_data)
      delete [] new_data;
    new_data = 0;
    new_data_len = 0;
    has_modified = true;
    image->setPixmap(QPixmap(0));
  }

  canSave(b);
  editCopyAction->setEnabled(b);  
  return b;
}

void CImageViewer::openFile()
{ 
  tmpFileName = QFileDialog::getOpenFileName(tmpFileName, g_img_fname_prompt);
  bool ok = false;
  QPixmap p;
  if (!tmpFileName.isEmpty())
    ok = p.load(tmpFileName);
  else
    return;

  QFile file(tmpFileName);
  if (!file.open(IO_ReadOnly))
  {
    message(CRITICAL, tr("An error occured while trying to load") + " " + tmpFileName);
    return;
  }
  else
  {
    QDataStream ts(&file);
    QFileInfo fi(tmpFileName);
    ulong len = fi.size();      
    char *data;      
    if ((data = new char [len +1]))
    {
      ts.readRawBytes(data, len);
      data[len] = 0;
      loadFromData((const uchar *) data, len);
      delete [] data;
    }
    else
      message(CRITICAL, tr("There is not enough memory to open the file"));
    file.close();
  }
}

void CImageViewer::onApplyClicked()
{
  if (has_modified)
    emit contentsChanged(new_data, new_data_len, tableItem);
}

void CImageViewer::scaleContents(bool b)
{
  image->setScaledContents(b);
}
