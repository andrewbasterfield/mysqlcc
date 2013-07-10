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
#include <qvariant.h>
#include <qdialog.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <qfontdatabase.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qregexp.h>
#include <qlistview.h>
#include <qheader.h>

#include "CHistoryView.h"
#include "CApplication.h"
#include "CAppConfigDialog.h"
#include "CConfig.h"
#include "globals.h"

static QString setPath(QWidget *sender, const QString &title)
{
  QString tmp = QFileDialog::getExistingDirectory("", sender, "setPath", title);
  if (!tmp.isEmpty())
  {    
    tmp = charReplace(tmp.stripWhiteSpace(),'\\', "/");
    if (!tmp.endsWith ("/"))
      tmp += "/";
    return tmp;
  }
  return QString::null;
}


CQueryConfigTab::CQueryConfigTab(QWidget* parent,  const char* name, WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
#ifdef DEBUG
  qDebug("CQueryConfigTab::CQueryConfigTab()");
#endif
  
  if (!name)
    setName("CQueryConfigTab");
  needrestart = false;
  setCaption(tr("Queries"));
  CQueryConfigTabLayout = new QGridLayout(this, 1, 1, 4, 2, "CQueryConfigTabLayout"); 
  
  CommentsBox = new QButtonGroup(this, "CommentsBox");
  CommentsBox->setTitle(tr("Default Comment Style"));
  QWhatsThis::add(CommentsBox, tr("This is the commenting style which will be used by default."));
  CommentsBox->setColumnLayout(0, Qt::Vertical);
  CommentsBox->layout()->setSpacing(6);
  CommentsBox->layout()->setMargin(11);
  CommentsBoxLayout = new QGridLayout(CommentsBox->layout());
  CommentsBoxLayout->setAlignment(Qt::AlignTop);
  
  hashComments = new QRadioButton(CommentsBox, "hashComments");
  hashComments->setText(tr("Hash [ # Comment ]"));
  QWhatsThis::add(hashComments, tr("When selected, the Hash commenting style will be used.\n"
    "\n"
    "Hash comments look as follows:\n"
    "\n"
    "# Commented text"));
  
  CommentsBoxLayout->addWidget(hashComments, 0, 0);
  
  dashComments = new QRadioButton(CommentsBox, "dashComments");
  dashComments->setText(tr("Dash [ -- Comment ]"));
  QWhatsThis::add(dashComments, tr("When selected, the Dash commenting style will be used.\n"
    "\n"
    "Dash comments look as follows:\n"
    "\n"
    " -- Commented text"));
  
  CommentsBoxLayout->addWidget(dashComments, 1, 0);
  
  cComments = new QRadioButton(CommentsBox, "cComments");
  cComments->setText(tr("C-Style [ /* Comment */ ]"));
  QWhatsThis::add(cComments, tr("When selected, the C-Style commenting style will be used.\n"
    "\n"
    "C-Style comments look as follows:\n"
    "\n"
    "/* Commented text */"));
  
  CommentsBox->insert(hashComments, CApplication::HASH_COMMENT);
  CommentsBox->insert(dashComments, CApplication::DASH_COMMENT);
  CommentsBox->insert(cComments, CApplication::C_COMMENT);
  
  CommentsBoxLayout->addWidget(cComments, 2, 0);
  
  CQueryConfigTabLayout->addMultiCellWidget(CommentsBox, 2, 2, 0, 2);
  
  OpenTablesBox = new QButtonGroup(this, "OpenTablesBox");
  OpenTablesBox->setTitle(tr("Open Table Options"));
  QWhatsThis::add(OpenTablesBox, tr("This is the action that will be taken when one double-clicks on a Table in the database tree"));
  OpenTablesBox->setColumnLayout(0, Qt::Vertical);
  OpenTablesBox->layout()->setSpacing(6);
  OpenTablesBox->layout()->setMargin(11);
  OpenTablesBoxLayout = new QGridLayout(OpenTablesBox->layout());
  OpenTablesBoxLayout->setAlignment(Qt::AlignTop);
  
  onlySqlStatement = new QRadioButton(OpenTablesBox, "onlySqlStatement");
  onlySqlStatement->setText(tr("Do not execute the query"));
  QWhatsThis::add(onlySqlStatement, tr("This option will automatically issue a SELECT * FROM Table statement but will not execute the query."));
  
  OpenTablesBoxLayout->addWidget(onlySqlStatement, 1, 0);
  
  allRecords = new QRadioButton(OpenTablesBox, "allRecords");
  allRecords->setText(tr("Retrieve all records"));
  QWhatsThis::add(allRecords, tr("This option will automatically execute a SELECT * FROM Table statement to retreive all the records in a Table."));
  
  OpenTablesBoxLayout->addWidget(allRecords, 0, 0);
  
  CQueryConfigTabLayout->addMultiCellWidget(OpenTablesBox, 1, 1, 0, 2);
  
  enableSqlDebug = new QCheckBox(this, "limitQueries");
  enableSqlDebug->setText(tr("Enable SQL Debug"));
  QWhatsThis::add(enableSqlDebug, tr("This option will append each executed query to the SQL Debug Panel."));
  
  CQueryConfigTabLayout->addWidget(enableSqlDebug, 0, 0);
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CQueryConfigTabLayout->addItem(spacer, 3, 0);
  
  QSpacerItem* spacer_2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CQueryConfigTabLayout->addItem(spacer_2, 0, 2);
  
  // tab order
  setTabOrder(enableSqlDebug, allRecords);  
  setTabOrder(allRecords, onlySqlStatement);
  setTabOrder(onlySqlStatement, hashComments);
  setTabOrder(hashComments, dashComments);
  setTabOrder(dashComments, cComments);
  setDefaultValues();
}

void CQueryConfigTab::setDefaultValues(CConfig *)
{
#ifdef DEBUG
  qDebug("CQueryConfigTab::setDefaultValues()");
#endif
  
  tmp_sqldebug = myApp()->enableSqlPanel();
  enableSqlDebug->setChecked(myApp()->enableSqlPanel());
  
  if (myApp()->retrieveAllRecordsFromTable())
    allRecords->setChecked(true);
  else
    onlySqlStatement->setChecked(true);
  ((QRadioButton *)CommentsBox->find((int) myApp()->commentStyle()))->setChecked(true);
}

bool CQueryConfigTab::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CQueryConfigTab::save()");
#endif
  
  needrestart = (tmp_sqldebug != enableSqlDebug->isChecked());
  bool ret = conn->writeEntry("Enable SQL Debug", booltostr(enableSqlDebug->isChecked()));
  ret &= conn->writeEntry("Retrieve All Records From Table", booltostr(allRecords->isChecked()));
  ret &= conn->writeEntry("Comment Style", CommentsBox->id(CommentsBox->selected()));
  return ret;
}


CSyntaxHighlightConfigTab::CSyntaxHighlightConfigTab(QWidget* parent,  const char* name, WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::CSyntaxHighlightConfigTab()");
#endif
  
  if (!name)
    setName("CSyntaxHighlightConfigTab");
  setCaption(tr("Syntax Highlighting"));
  CSyntaxHighlightConfigTabLayout = new QHBoxLayout(this, 4, 2, "CSyntaxHighlightConfigTabLayout"); 
  
  Sections = new QListBox(this, "Sections");
  Sections->setSelectionMode(QListBox::Extended);
  QWhatsThis::add(Sections, tr("Available Sections for the SQL Editor.\n  If you want to modify multiple entries at once, select all the sections you want to modify by using the CRTL key and either the mouse or the up, down and space-bar keys."));
  CSyntaxHighlightConfigTabLayout->addWidget(Sections);
  
  Layout12 = new QGridLayout(0, 1, 1, 0, 6, "Layout12"); 
  
  Preview = new QLineEdit(this, "Preview");
  Preview->setMinimumSize(QSize(0, 50));
  Preview->setText("Jorge@mysql.com");
  QWhatsThis::add(Preview, tr("Preview of this section."));
  
  Layout12->addMultiCellWidget(Preview, 3, 3, 1, 5);
  
  Underline = new QCheckBox(this, "Underline");
  Underline->setText(tr("Underline"));
  QWhatsThis::add(Underline, tr("The font for this section will be Underlined."));
  
  Layout12->addWidget(Underline, 2, 3);
  
  sizeLabel = new QLabel(this, "sizeLabel");
  sizeLabel->setText(tr("Size"));
  
  Layout12->addWidget(sizeLabel, 1, 0);
  
  Size = new QSpinBox(this, "Size");  
  
  Size->setMaxValue(30);
  Size->setMinValue(0);
  Size->setValue(8);
  Size->setSpecialValueText(" ");
  QWhatsThis::add(Size, tr("This is the font Size that will be used for this particular section."));
  
  Layout12->addWidget(Size, 1, 1);
  
  Italic = new QCheckBox(this, "Italic");
  Italic->setText(tr("Italic"));
  QWhatsThis::add(Italic, tr("The font for this section will be Italic."));
  
  Layout12->addMultiCellWidget(Italic, 2, 2, 4, 5);
  
  preveiwLabel = new QLabel(this, "preveiwLabel");
  preveiwLabel->setText(tr("Preview"));
  
  Layout12->addWidget(preveiwLabel, 3, 0);
  
  Bold = new QCheckBox(this, "Bold");
  Bold->setText(tr("Bold"));
  QWhatsThis::add(Bold, tr("The font for this section will be Bold."));
  
  Layout12->addMultiCellWidget(Bold, 2, 2, 1, 2);
  
  Font = new QComboBox(false, this, "Font");
  QWhatsThis::add(Font, tr("This is the Font that will be used for this particular section."));
  
  Layout12->addMultiCellWidget(Font, 0, 0, 1, 5);
  
  Color = new QPushButton(this, "Color");
  Color->setMinimumSize(QSize(19, 19));
  Color->setMaximumSize(QSize(19, 19));  
  Color->setText(tr(""));
  QWhatsThis::add(Color, tr("This is the font Color that will be used for this particular section."));
  
  Layout12->addWidget(Color, 1, 5);
  
  fontLabel = new QLabel(this, "fontLabel");
  fontLabel->setText(tr("Font"));
  
  Layout12->addWidget(fontLabel, 0, 0);
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
  Layout12->addItem(spacer, 5, 1);
  
  colorLabel = new QLabel(this, "colorLabel");
  colorLabel->setText(tr("Color"));
  colorLabel->setAlignment(int(QLabel::AlignVCenter));
  
  Layout12->addWidget(colorLabel, 1, 4);
  
  DefaultPushButton = new QPushButton(this, "DefaultPushButton");
  DefaultPushButton->setText(tr("Restore Defaults"));
  
  Layout12->addMultiCellWidget(DefaultPushButton, 4, 4, 4, 5);
  QSpacerItem* spacer_2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  Layout12->addMultiCell(spacer_2, 4, 4, 2, 3);
  CSyntaxHighlightConfigTabLayout->addLayout(Layout12);
  
  setTabOrder(Sections, Font);
  setTabOrder(Font, Size);
  setTabOrder(Size, Color);
  setTabOrder(Color, Bold);
  setTabOrder(Bold, Underline);
  setTabOrder(Underline, Italic);
  setTabOrder(Italic, Preview);
  setTabOrder(Preview, DefaultPushButton);
  
  init();
  setDefaultValues();  
}

void CSyntaxHighlightConfigTab::setDefaultValues(CConfig *)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::setDefaultValues()");
#endif
  
  QFontDatabase fdb;
  QStringList families = fdb.families();
  Font->insertStringList(families);  
  QIntDictIterator<CSqlEditorFont> it(*CSqlEditorFont::sqlEditorStyles());
  for (; it.current(); ++it)
  {
    Sections->insertItem(it.current()->displayName(), (int) it.current()->highlightType());    
    it.current()->setDefaults();
  }
  Sections->setSelected (0, true);
  Refresh();
}

bool CSyntaxHighlightConfigTab::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::save()");
#endif
  
  bool ret = true;
  QIntDictIterator<CSqlEditorFont> it(*CSqlEditorFont::sqlEditorStyles());
  for (; it.current(); ++it)
    ret &= it.current()->save(conn);
  return ret;
}

CSqlEditorFont *CSyntaxHighlightConfigTab::findItem(uint i)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::findItem(%d)", i);
#endif
  
  return (CSqlEditorFont *)CSqlEditorFont::sqlEditorStyles()->find(i);
}

void CSyntaxHighlightConfigTab::refreshPreview(const QFont & fnt, const QColor & clr)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::refreshPreview()");
#endif
  
  Preview->setFont(fnt);
  QPalette pal = QApplication::palette();
  QColorGroup cg = Preview->colorGroup();
  cg.setColor(QColorGroup::Text, clr);    
  pal.setActive(cg);
  Preview->setPalette(pal);  
}

void CSyntaxHighlightConfigTab::Refresh()
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::Refresh()");
#endif
  
  QString font = QString::null;
  bool bold = false;
  bool underline = false;
  bool italic = false;
  int size = 0;
  QColor color = paletteBackgroundColor();
  
  bool first = true;
  for (uint i=0; i < Sections->count(); i++)
  {
    if (Sections->isSelected(i))
    {
      CSqlEditorFont *item = findItem(i);
      if (item != NULL)
      {
        if (first)
        {
          font = item->m_font.family();
          bold = item->m_font.bold();
          underline = item->m_font.underline();
          italic = item->m_font.italic();
          size = item->m_font.pointSize();
          color = item->m_color;
          first = false;
        }
        else
        {
          if (item->m_font.family() != font)
            font = "";
          
          if (item->m_font.bold() != bold)
            bold = false;
          
          if (item->m_font.underline() != underline)
            underline = false;
          
          if (item->m_font.italic() != italic)
            italic = false;
          
          if (item->m_font.pointSize() != size)
            size = 0;
          
          if (item->m_color != color)
            color = paletteBackgroundColor();
        }
      }
    }
  }
  
  if (!first)
  {
    Font->setCurrentText(font);
    Bold->setChecked(bold);
    Underline->setChecked(underline);
    Italic->setChecked(italic);
    Size->setValue(size);
    Color->setPaletteBackgroundColor(color);
    QFont f(font, size, QFont::Normal, italic);
    f.setBold(bold);
    f.setUnderline(underline);
    refreshPreview(f, color);
  }
}

void CSyntaxHighlightConfigTab::setFontBoolValue(void (QFont::*member)(bool), bool value)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::setFontBoolValue()");
#endif
  
  for (uint i=0; i < Sections->count(); i++)
  {
    if (Sections->isSelected(i))
    {
      CSqlEditorFont *item = findItem(i);
      if (item != NULL)
        (item->m_font.*member)(value);      
    }
  }
  Refresh();
}

void CSyntaxHighlightConfigTab::BoldToggled(bool b)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::BoldToggled(%s)", debug_string(booltostr(b)));
#endif
  
  void (QFont::*member)(bool) = &QFont::setBold;
  setFontBoolValue(member, b);
}

void CSyntaxHighlightConfigTab::ItalicToggled(bool b)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::ItalicToggled(%s)", debug_string(booltostr(b)));
#endif
  
  void (QFont::*member)(bool) = &QFont::setItalic;
  setFontBoolValue(member, b);
}

void CSyntaxHighlightConfigTab::UnderlineToggled(bool b)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::UnderlineToggled(%s)", debug_string(booltostr(b)));
#endif
  
  void (QFont::*member)(bool) = &QFont::setUnderline;
  setFontBoolValue(member, b); 
}

void CSyntaxHighlightConfigTab::ValueChanged(int s)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::ValueChanged(%d)", s);
#endif
  
  for (uint i=0; i < Sections->count(); i++)
  {
    if (Sections->isSelected(i))
    {
      CSqlEditorFont *item = findItem(i);
      if (item != NULL)
        item->m_font.setPointSize(s);   
    }
  }
  Refresh();
}

void CSyntaxHighlightConfigTab::FontChanged(const QString &f)
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::FontChanged('%s')", debug_string(f));
#endif
  
  for (uint i=0; i < Sections->count(); i++)
  {
    if (Sections->isSelected(i))
    {
      CSqlEditorFont *item = findItem(i);
      if (item != NULL)
        item->m_font.setFamily(f);       
    }
  }
  Refresh();
}

void CSyntaxHighlightConfigTab::setSectionColor()
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::setSectionColor()");
#endif
  
  QColor c = QColorDialog::getColor(Color->paletteBackgroundColor(), this);
  if (c.isValid())
  {
    for (uint i=0; i < Sections->count(); i++)
    {
      if (Sections->isSelected(i))
      {
        CSqlEditorFont *item = findItem(i);
        if (item != NULL)
          item->m_color = c;    
      }
    }
    Color->setPaletteBackgroundColor(c);
    Refresh();
  }
}

void CSyntaxHighlightConfigTab::DefaultPushButtonClicked()
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::DefaultPushButtonClicked()");
#endif
  
  for (uint i=0; i < Sections->count(); i++)
  {
    if (Sections->isSelected(i))
    {
      CSqlEditorFont *item = findItem(i);
      if (item != NULL)
        item->setDefaultValues();
    }
  }  
  Refresh();
}

void CSyntaxHighlightConfigTab::init()
{
#ifdef DEBUG
  qDebug("CSyntaxHighlightConfigTab::init()");
#endif
  
  connect(Sections, SIGNAL(selectionChanged()), this, SLOT(Refresh()));  
  connect(Bold, SIGNAL(toggled(bool)), this, SLOT(BoldToggled(bool)));  
  connect(Underline, SIGNAL(toggled(bool)), this, SLOT(UnderlineToggled(bool)));
  connect(Italic, SIGNAL(toggled(bool)), this, SLOT(ItalicToggled(bool)));
  connect(Font, SIGNAL(activated(const QString &)), this, SLOT(FontChanged(const QString &)));
  connect(Size, SIGNAL(valueChanged(int)), this, SLOT(ValueChanged(int)));  
  connect(Color, SIGNAL(clicked()), this, SLOT(setSectionColor()));
  connect(DefaultPushButton, SIGNAL(clicked()), this, SLOT(DefaultPushButtonClicked()));
}

CSqlEditorConfigTab::CSqlEditorConfigTab(QWidget* parent,  const char* name, WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
#ifdef DEBUG
  qDebug("CSqlEditorConfigTab::CSqlEditorConfigTab()");
#endif
  
  if (!name)
    setName("CSqlEditorConfigTab");
  
  setCaption(tr("Sql Editor"));  
  CSqlEditorConfigTabLayout = new QGridLayout(this, 1, 1, 3, 2, "CSqlEditorConfigTabLayout");
  
  Parentheses = new QCheckBox(this, "Parentheses");
  Parentheses->setText(tr("Enable Parentheses Matching"));
  QWhatsThis::add(Parentheses, tr("When enabled, the SQL Editor will use Parentheses Matching."));
  
  CSqlEditorConfigTabLayout->addMultiCellWidget(Parentheses, 2, 2, 0, 2);
  
  LinuxPaste = new QCheckBox(this, "LinuxPaste");
  LinuxPaste->setText(tr("Enable Linux Copy / Paste Style"));
  QWhatsThis::add(LinuxPaste, tr("When enabled, the SQL Editor will emulate the way Linux works for Copy & Paste.  When text is selected, it will automatically be copied in to the buffer and when the mid-mouse button is clicked, it will be pasted at the current cursor position."));
  
  CSqlEditorConfigTabLayout->addMultiCellWidget(LinuxPaste, 3, 3, 0, 2);  
  
  SyntaxFileLabel = new QLabel(this, "SyntaxFileLabel");
  SyntaxFileLabel->setText(tr("Syntax File"));
  
  CSqlEditorConfigTabLayout->addWidget(SyntaxFileLabel, 0, 0);
  
  SyntaxFile = new QLineEdit(this, "SyntaxFile");
  SyntaxFile->setFrameShape(QLineEdit::StyledPanel);
  SyntaxFile->setFrameShadow(QLineEdit::Sunken);
  QWhatsThis::add(SyntaxFile, tr("This is the File that contains the list of keywords that will be highlighted and completed by the SQL Editor."));
  
  CSqlEditorConfigTabLayout->addWidget(SyntaxFile, 0, 1);
  
  SyntaxFileBrowse = new QPushButton(this, "SyntaxFileBrowse");
  SyntaxFileBrowse->setMinimumSize(QSize(22, 22));
  SyntaxFileBrowse->setMaximumSize(QSize(22, 22));
  SyntaxFileBrowse->setText(tr(""));
  SyntaxFileBrowse->setPixmap(getPixmapIcon("openIcon"));
  QWhatsThis::add(SyntaxFileBrowse, tr("Select the Folder that contains the Translation Files."));
  
  CSqlEditorConfigTabLayout->addWidget(SyntaxFileBrowse, 0, 2);
  
  SyntaxHighlight = new QCheckBox(this, "SyntaxHighlight");
  SyntaxHighlight->setText(tr("Enable SQL Syntax Highlighting && Completion"));
  QWhatsThis::add(SyntaxHighlight, tr("When enabled, the SQL Editor will use Syntax Highlighting for SQL Keywords and will also have support for Keyword completion."));
  
  CSqlEditorConfigTabLayout->addMultiCellWidget(SyntaxHighlight, 1, 1, 0, 2);
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);  
  CSqlEditorConfigTabLayout->addItem(spacer, 5, 1);
  setTabOrder(SyntaxFile, SyntaxFileBrowse);
  setTabOrder(SyntaxFileBrowse, SyntaxHighlight);
  setTabOrder(SyntaxHighlight, Parentheses);
  setTabOrder(Parentheses, LinuxPaste);  
  setDefaultValues();
  init();
}

void CSqlEditorConfigTab::setDefaultValues(CConfig *)
{
#ifdef DEBUG
  qDebug("CSqlEditorConfigTab::setDefaultValues()");
#endif
  
  if (myApp()->syntaxHighlight())  
    Parentheses->setChecked(myApp()->parenthesesMatching());  
  else
    Parentheses->setEnabled(false);
  LinuxPaste->setChecked(myApp()->linuxPaste());
  SyntaxFile->setText(myApp()->syntaxFile());
  SyntaxHighlight->setChecked(myApp()->syntaxHighlight());
}

bool CSqlEditorConfigTab::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CSqlEditorConfigTab::save()");
#endif
  
  bool ret = conn->writeEntry("Syntax File", SyntaxFile->text().stripWhiteSpace());  
  ret &= conn->writeEntry("Linux Paste Style", booltostr(LinuxPaste->isChecked()));
  ret &= conn->writeEntry("Syntax Highlighting", booltostr(SyntaxHighlight->isChecked()));
  ret &= conn->writeEntry("Parentheses Matching", booltostr(Parentheses->isChecked()));  
  return ret;
}

void CSqlEditorConfigTab::setSyntaxFile()
{
#ifdef DEBUG
  qDebug("CSqlEditorConfigTab::setSyntaxFile()");
#endif
  
  QString tmp = QFileDialog::getOpenFileName ("syntax.txt", tr("Text Files") + " (*.txt);;" + tr("All Files") + " (*.*)", this, "setSyntaxFile", tr("Select the Syntax File"));    
  if (!tmp.isEmpty())
  {
    tmp = charReplace(tmp.stripWhiteSpace(),'\\', "/");    
    SyntaxFile->setText(tmp);        
  }
}

void CSqlEditorConfigTab::init()
{
#ifdef DEBUG
  qDebug("CSqlEditorConfigTab::init()");
#endif
  
  connect(SyntaxFileBrowse, SIGNAL(clicked()), this, SLOT(setSyntaxFile()));
  connect(SyntaxHighlight, SIGNAL(toggled(bool)), Parentheses, SLOT(setEnabled(bool)));
}

CPluginsConfigTab::CPluginsConfigTab(QWidget* parent,  const char* name, WFlags fl)
: CConfigDialogTab(parent, name, fl)
{
#ifdef DEBUG
  qDebug("CPluginsConfigTab::CPluginsConfigTab()");
#endif
  if (!name)
    setName("CPluginsConfigTab");
  CPluginsConfigTabLayout = new QGridLayout(this, 1, 1, 4, 2, "CPluginsConfigTabLayout"); 
  
  pluginsPathBrowse = new QPushButton(this, "pluginsPathBrowse");
  pluginsPathBrowse->setMinimumSize(QSize(22, 22));
  pluginsPathBrowse->setMaximumSize(QSize(22, 22));
  pluginsPathBrowse->setPixmap(getPixmapIcon("openIcon"));
  
  CPluginsConfigTabLayout->addWidget(pluginsPathBrowse, 0, 2);
  
  pluginsPathLabel = new QLabel(this, "pluginsPathLabel");
  
  CPluginsConfigTabLayout->addWidget(pluginsPathLabel, 0, 0);
  
  pluginsPath = new QLineEdit(this, "pluginsPath");
  pluginsPath->setFrameShape(QLineEdit::StyledPanel);
  pluginsPath->setFrameShadow(QLineEdit::Sunken);
  
  CPluginsConfigTabLayout->addWidget(pluginsPath, 0, 1);
  
  textLabel1 = new QLabel(this, "textLabel1");
  QFont textLabel1_font( textLabel1->font());
  textLabel1_font.setBold(true);
  textLabel1->setFont(textLabel1_font); 
  
  CPluginsConfigTabLayout->addMultiCellWidget(textLabel1, 1, 1, 0, 2);
  
  pluginsList = new QListView(this, "pluginsList");
  pluginsList->addColumn(tr("Plugins"));
  pluginsList->setShowSortIndicator(true);
  pluginsList->setResizeMode(QListView::AllColumns);
  
  CPluginsConfigTabLayout->addMultiCellWidget(pluginsList, 2, 2, 0, 2);
  languageChange();
  clearWState(WState_Polished);
  
  // tab order
  setTabOrder(pluginsPath, pluginsPathBrowse);
  setTabOrder(pluginsPathBrowse, pluginsList);
  
  CConfig *cfg = new CConfig();
  setDefaultValues(cfg);
  delete cfg;
  connect(pluginsPathBrowse, SIGNAL(clicked()), this, SLOT(setPluginsPath()));
  connect(pluginsPath, SIGNAL(textChanged(const QString &)), this, SLOT(refreshPluginsList(const QString &)));
  needrestart = false;
}

void CPluginsConfigTab::languageChange()
{
  setCaption(tr("Plugins"));
  pluginsPathBrowse->setText(QString::null);
  QWhatsThis::add(pluginsPathBrowse, tr("Click to browse for the Plugins Path."));
  pluginsPathLabel->setText(tr("Plugins Path"));
  QWhatsThis::add(pluginsPath, tr("This is the path where MySQLCC will search for plugins."));
  textLabel1->setText(tr("The Checked plugins will be enabled to be used in MySQLCC"));
  pluginsList->header()->setLabel(0, tr("Plugins"));
  pluginsList->clear();
  QWhatsThis::add(pluginsList, tr("Available Plugins"));
}

void CPluginsConfigTab::setPluginsPath()
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::setPluginsPath()");
#endif
  
  QString tmp = setPath(this, tr("Select the Translations Path"));
  if (!tmp.isEmpty())
    pluginsPath->setText(tmp);
}

bool CPluginsConfigTab::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CPluginsConfigTab::save()");
#endif
  
  bool ret = conn->writeEntry("Plugins Path", pluginsPath->text().stripWhiteSpace());
  QStringList list;
  list.clear();

  if (pluginsList->firstChild())
    pluginsList->setCurrentItem(pluginsList->firstChild());

  QListViewItemIterator it(pluginsList);
  QStringList lst = myApp()->enabledPluginsList();
  while (it.current())
  {
    if (((QCheckListItem *)it.current())->isOn())
    {
      list.append(it.current()->text(0));
      if (!needrestart)
        if (lst.find(it.current()->text(0)) == lst.end())
          needrestart = true;
    }
    else
      if (!needrestart)
        if (lst.find(it.current()->text(0)) != lst.end())
          needrestart = true;
    ++it;
  }

  if (list.isEmpty())
    conn->removeEntry("Enabled Plugins List");
  else
    ret &= conn->writeEntry("Enabled Plugins List", list.join("/"));

  return ret;
}

void CPluginsConfigTab::setDefaultValues(CConfig *cfg)
{
#ifdef DEBUG
  qDebug("CPluginsConfigTab::setDefaultValues()");
#endif

  pluginsPath->setText(cfg->readStringEntry("Plugins Path", myApp()->pluginsPath()));
  refreshPluginsList();
}

void CPluginsConfigTab::refreshPluginsList()
{
#ifdef DEBUG
  qDebug("CPluginsConfigTab::refreshPluginsList()");
#endif
  
  refreshPluginsList(pluginsPath->text());   
}

void CPluginsConfigTab::refreshPluginsList(const QString &path)
{
#ifdef DEBUG
  qDebug("CPluginsConfigTab::refreshPluginsList(const QString &)");
#endif

  needrestart = true;
  pluginsList->clear();
  QDir dir = QDir(path, "*");
  dir.setFilter(QDir::Files);
  dir.setSorting(QDir::Name);
  
  QStringList lst = myApp()->enabledPluginsList();  
  QStringList plugin_list = dir.entryList();
  for (QStringList::Iterator it = plugin_list.begin(); it != plugin_list.end(); ++it)
  {
    QCheckListItem *c = new QCheckListItem(pluginsList, *it, QCheckListItem::CheckBox);
    c->setOn(lst.find(*it) != lst.end());
  }
}

CGeneralAppConfigTab::CGeneralAppConfigTab(QWidget* parent,  const char* name, WFlags fl)
: CConfigDialogTab(parent, name, fl),needrestart(false)
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::CGeneralAppConfigTab()");
#endif
  
  if (!name)
    setName("CGeneralAppConfigTab");
  
  CGeneralAppConfigTabLayout = new QGridLayout(this, 1, 1, 4, 2, "CGeneralAppConfigTabLayout"); 
  
  errorSoundFile = new QLineEdit(this, "errorSoundFile");
  errorSoundFile->setFrameShape(QLineEdit::StyledPanel);
  errorSoundFile->setFrameShadow(QLineEdit::Sunken);
  
  CGeneralAppConfigTabLayout->addMultiCellWidget(errorSoundFile, 1, 1, 1, 3);
  
  translationsPath = new QLineEdit(this, "translationsPath");
  translationsPath->setFrameShape(QLineEdit::StyledPanel);
  translationsPath->setFrameShadow(QLineEdit::Sunken);
  
  CGeneralAppConfigTabLayout->addMultiCellWidget(translationsPath, 0, 0, 1, 3);
  
  warningSoundLabel = new QLabel(this, "warningSoundLabel");
  
  CGeneralAppConfigTabLayout->addWidget(warningSoundLabel, 2, 0);
  
  errorSoundLabel = new QLabel(this, "errorSoundLabel");
  
  CGeneralAppConfigTabLayout->addWidget(errorSoundLabel, 1, 0);
  
  translationsPathLabel = new QLabel(this, "translationsPathLabel");
  
  CGeneralAppConfigTabLayout->addWidget(translationsPathLabel, 0, 0);
  
  warningSoundBrowse = new QPushButton(this, "warningSoundBrowse");
  warningSoundBrowse->setMinimumSize(QSize(22, 22));
  warningSoundBrowse->setMaximumSize(QSize(22, 22));
  warningSoundBrowse->setPixmap(getPixmapIcon("openIcon"));
  
  CGeneralAppConfigTabLayout->addWidget(warningSoundBrowse, 2, 4);
  
  informationSoundBrowse = new QPushButton(this, "informationSoundBrowse");
  informationSoundBrowse->setMinimumSize(QSize(22, 22));
  informationSoundBrowse->setMaximumSize(QSize(22, 22));
  informationSoundBrowse->setPixmap(getPixmapIcon("openIcon"));
  
  CGeneralAppConfigTabLayout->addWidget(informationSoundBrowse, 3, 4);
  
  errorSoundBrowse = new QPushButton(this, "errorSoundBrowse");
  errorSoundBrowse->setMinimumSize(QSize(22, 22));
  errorSoundBrowse->setMaximumSize(QSize(22, 22));
  errorSoundBrowse->setPixmap(getPixmapIcon("openIcon"));
  
  CGeneralAppConfigTabLayout->addWidget(errorSoundBrowse, 1, 4);
  
  translationsBrowse = new QPushButton(this, "translationsBrowse");
  translationsBrowse->setMinimumSize(QSize(22, 22));
  translationsBrowse->setMaximumSize(QSize(22, 22));
  translationsBrowse->setPixmap(getPixmapIcon("openIcon"));
  
  CGeneralAppConfigTabLayout->addWidget(translationsBrowse, 0, 4);
  
  informationSoundLabel = new QLabel(this, "informationSoundLabel");
  
  CGeneralAppConfigTabLayout->addWidget(informationSoundLabel, 3, 0);
  
  informationSoundFile = new QLineEdit(this, "informationSoundFile");
  informationSoundFile->setFrameShape(QLineEdit::StyledPanel);
  informationSoundFile->setFrameShadow(QLineEdit::Sunken);
  
  CGeneralAppConfigTabLayout->addMultiCellWidget(informationSoundFile, 3, 3, 1, 3);
  
  warningSoundFile = new QLineEdit(this, "warningSoundFile");
  warningSoundFile->setFrameShape(QLineEdit::StyledPanel);
  warningSoundFile->setFrameShadow(QLineEdit::Sunken);
  
  CGeneralAppConfigTabLayout->addMultiCellWidget(warningSoundFile, 2, 2, 1, 3);
  
  saveWorkspace = new QCheckBox(this, "saveWorkspace");
  
  CGeneralAppConfigTabLayout->addMultiCellWidget(saveWorkspace, 6, 6, 0, 1);
  
  language = new QComboBox(false, this, "language");
  language->setAutoCompletion(true);
  language->setDuplicatesEnabled(false);
  
  CGeneralAppConfigTabLayout->addMultiCellWidget(language, 4, 4, 1, 2);
  
  languageLabel = new QLabel(this, "languageLabel");
  
  CGeneralAppConfigTabLayout->addWidget(languageLabel, 4, 0);
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CGeneralAppConfigTabLayout->addItem(spacer, 5, 2);
  QSpacerItem* spacer_2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  CGeneralAppConfigTabLayout->addItem(spacer_2, 4, 3);
  
  historyNumberLabel = new QLabel(this, "historyNumberLabel");
  
  CGeneralAppConfigTabLayout->addWidget(historyNumberLabel, 5, 0);
  
  historySize = new QSpinBox(this, "historySize");
  historySize->setMaxValue(1000);
  historySize->setValue(100);
  
  CGeneralAppConfigTabLayout->addWidget(historySize, 5, 1);
  QSpacerItem* spacer_3 = new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CGeneralAppConfigTabLayout->addItem(spacer_3, 8, 1);
  
  confirmCritical = new QCheckBox(this, "confirmCritical");
  
  CGeneralAppConfigTabLayout->addMultiCellWidget(confirmCritical, 7, 7, 0, 1);
  languageChange();
  clearWState(WState_Polished);
  
  // tab order
  setTabOrder(translationsPath, translationsBrowse);
  setTabOrder(translationsBrowse, errorSoundFile);
  setTabOrder(errorSoundFile, errorSoundBrowse);
  setTabOrder(errorSoundBrowse, warningSoundFile);
  setTabOrder(warningSoundFile, warningSoundBrowse);
  setTabOrder(warningSoundBrowse, informationSoundFile);
  setTabOrder(informationSoundFile, informationSoundBrowse);
  setTabOrder(informationSoundBrowse, language);
  setTabOrder(language, historySize);
  setTabOrder(historySize, confirmCritical);
  setTabOrder(confirmCritical, saveWorkspace);
  
  setDefaultValues();
  init();
}

void CGeneralAppConfigTab::languageChange()
{
  setCaption(tr("General"));
  QWhatsThis::add(errorSoundFile, tr("This is a WAV file that will be played each time an Error message occures.  If you don't want to play a sound, leave this field empty."));
  QWhatsThis::add(translationsPath, tr("This is the Path which will be used by the application to find Language Files."));
  warningSoundLabel->setText(tr("Warning Sound File"));
  errorSoundLabel->setText(tr("Error Sound File"));
  translationsPathLabel->setText(tr("Translations Path"));
  warningSoundBrowse->setText(QString::null);
  QWhatsThis::add(warningSoundBrowse, tr("Click to browse for the Warning WAV File."));
  informationSoundBrowse->setText(QString::null);
  QWhatsThis::add(informationSoundBrowse, tr("Click to browse for the Information WAV File."));
  errorSoundBrowse->setText(QString::null);
  QWhatsThis::add(errorSoundBrowse, tr("Click to browse for the Error WAV File."));
  translationsBrowse->setText(QString::null);
  QWhatsThis::add(translationsBrowse, tr("Click to browse for a the Translations Path"));
  informationSoundLabel->setText(tr("Information Sound File"));
  QWhatsThis::add(informationSoundFile, tr("This is a WAV file that will be played each time a Information message occures.  If you don't want to play a sound, leave this field empty."));
  QWhatsThis::add(warningSoundFile, tr("This is a WAV file that will be played each time a Warning message occures.  If you don't want to play a sound, leave this field empty."));
  saveWorkspace->setText(tr("Restore last Workspace on startup"));
  QWhatsThis::add(saveWorkspace, tr("When enabled, the application will restore the last Workspace of the previous execution by automatically opening all windows and connections which where open prior shutdown."));
  language->clear();
  QWhatsThis::add(language, tr("This will change the display Language of the Application.  If changed, one will have to restart the application for changes to take place."));
  languageLabel->setText(tr("Language"));
  historyNumberLabel->setText(tr("History size for Queries"));
  QWhatsThis::add(historySize, tr("This number specifies how many Queries will be saved by the History Panel in  the Query Windows."));
  confirmCritical->setText(tr("Confirm critical operations"));
  QWhatsThis::add(confirmCritical, tr("This option will confirm critical operations done by the client.  Such critical operations include shutting down the application, shutting down the Server, truncate table, etc ..."));
}


static QString languageFileName(const QString &display_lang)
{
  return charReplace(display_lang + ".qm", ' ', "_");
}

static QString displayLanguageName(const QString &lang_filename)
{
  return charReplace(lang_filename.left(lang_filename.length() - 3), '_', " ");
}

bool CGeneralAppConfigTab::save(CConfig *conn)
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::save()");
#endif
  
  QString lang_tmp = languageFileName(language->currentText());
  
  bool ret = conn->writeEntry("Translations Path", translationsPath->text().stripWhiteSpace());
  
  ret &= conn->writeEntry("Language File", lang_tmp);
  ret &= conn->writeEntry("History Size", historySize->value());
  ret &= conn->writeEntry("Save Workspace", booltostr(saveWorkspace->isChecked()));
  ret &= conn->writeEntry("Confirm Critical", booltostr(confirmCritical->isChecked()));
  ret &= conn->writeEntry("Error Sound", errorSoundFile->text().stripWhiteSpace());
  ret &= conn->writeEntry("Warning Sound", warningSoundFile->text().stripWhiteSpace());
  ret &= conn->writeEntry("Information Sound", informationSoundFile->text().stripWhiteSpace());  
  
  if (myApp()->translationsPath() != translationsPath->text().stripWhiteSpace())  
    needrestart = true;  
  
  if (myApp()->currentLanguage() != lang_tmp)
    needrestart = true;  
  
  return ret;
}

void CGeneralAppConfigTab::setDefaultValues(CConfig *)
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::setDefaultValues()");
#endif
  
  translationsPath->setText(myApp()->translationsPath());
  historySize->setValue(CHistoryView::historySize());
  confirmCritical->setChecked(myApp()->confirmCritical());
  saveWorkspace->setChecked(myApp()->saveWorkspace());
  errorSoundFile->setText(myApp()->errorSoundFile());
  warningSoundFile->setText(myApp()->warningSoundFile());
  informationSoundFile->setText(myApp()->informationSoundFile());  
  refreshLanguageCombo();
}

void CGeneralAppConfigTab::refreshLanguageCombo()
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::refreshLanguageCombo()");
#endif
  
  refreshLanguageCombo(translationsPath->text());   
}

void CGeneralAppConfigTab::refreshLanguageCombo(const QString &path)
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::refreshLanguageCombo(const QString &)");
#endif
  
  language->clear();
  QDir dir = QDir(path, "*.qm");
  dir.setFilter(QDir::Files);
  dir.setSorting(QDir::Name);
  language->insertItem("English");
  
  QStringList lang_list = dir.entryList();
  for (QStringList::Iterator it = lang_list.begin(); it != lang_list.end(); ++it)
    language->insertItem(displayLanguageName(*it));
  
  if (dir.count() > 0 && dir.exists(myApp()->currentLanguage()))  
    language->setCurrentText(displayLanguageName(myApp()->currentLanguage()));
}

void CGeneralAppConfigTab::setTranslationsPath()
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::setTranslationsPath()");
#endif
  
  QString tmp = setPath(this, tr("Select the Translations Path"));
  if (!tmp.isEmpty())
  {
    translationsPath->setText(tmp);
    refreshLanguageCombo();
  }
}

void CGeneralAppConfigTab::setErrorFile()
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::setErrorFile()");
#endif
  
  QString tmp = QFileDialog::getOpenFileName(QString::null, tr("Wav Files(*.wav)"), this, "BrowseErrorFile", tr("Select Error Sound"));
  if (!tmp.isEmpty())
  {
    tmp = charReplace(tmp.stripWhiteSpace(),'\\', "/");
    errorSoundFile->setText(tmp);    
  }
}

void CGeneralAppConfigTab::setWarningFile()
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::setWarningFile()");
#endif
  
  QString tmp = QFileDialog::getOpenFileName(QString::null, tr("Wav Files(*.wav)"), this, "BrowseWarningFile", tr("Select Warning Sound"));
  if (!tmp.isEmpty())
  {    
    tmp = charReplace(tmp.stripWhiteSpace(),'\\', "/");
    warningSoundFile->setText(tmp);    
  }
}

void CGeneralAppConfigTab::setInformationFile()
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::setInformationFile()");
#endif
  
  QString tmp = QFileDialog::getOpenFileName(QString::null, tr("Wav Files(*.wav)"), this, "BrowseInformationFile", tr("Select Information Sound"));
  if (!tmp.isEmpty())
  {    
    tmp = charReplace(tmp.stripWhiteSpace(),'\\', "/");
    informationSoundFile->setText(tmp);    
  }
}

void CGeneralAppConfigTab::init()
{
#ifdef DEBUG
  qDebug("CGeneralAppConfigTab::init()");
#endif
  
  connect(translationsBrowse, SIGNAL(clicked()), this, SLOT(setTranslationsPath()));
  connect(errorSoundBrowse, SIGNAL(clicked()), this, SLOT(setErrorFile()));
  connect(warningSoundBrowse, SIGNAL(clicked()), this, SLOT(setWarningFile()));
  connect(informationSoundBrowse, SIGNAL(clicked()), this, SLOT(setInformationFile()));
  connect(translationsPath, SIGNAL(textChanged(const QString &)), this, SLOT(refreshLanguageCombo(const QString &)));  
}

CAppConfigDialog::CAppConfigDialog(QWidget* parent,  const char* name)
:CConfigDialog(parent, name)
{
#ifdef DEBUG
  qDebug("CAppConfigDialog::CAppConfigDialog()");
#endif
  
  myApp()->incCritical();
  if (!name)
    setName("CAppConfigDialog");    
  setMinimumHeight(263);  
  setCaption(tr("General Configuration Dialog"));
  generalConfigTab = new CGeneralAppConfigTab((QWidget *) tab());
  insertTab(generalConfigTab);
  insertTab(new CQueryConfigTab((QWidget *) tab()));
#ifndef NO_MYSQLCC_PLUGINS
  insertTab(new CPluginsConfigTab((QWidget *) tab()));  
#endif
  insertTab(new CSqlEditorConfigTab((QWidget *) tab()));
  insertTab(new CSyntaxHighlightConfigTab((QWidget *) tab()));
  okPushButton->setText(tr("&Apply"));
  QWhatsThis::add(okPushButton, tr("Click to Apply changes."));
  myResize(421, 263);
}

CAppConfigDialog::~CAppConfigDialog()
{
#ifdef DEBUG
  qDebug("CAppConfigDialog::~CAppConfigDialog()");
#endif
  
  myApp()->decCritical();
}

void CAppConfigDialog::okButtonClicked()
{
#ifdef DEBUG
  qDebug("CAppConfigDialog::okButtonClicked()");
#endif
  
  CConfig *conn = new CConfig();  
  bool ret = save(conn);
  ret &= conn->save();
  
  if (!ret)
  {
    QMessageBox::critical(0, tr("Error"), tr("An Error occurred while saving the Configuration."));
    return;
  }
  else
    if (needRestart())
      QMessageBox::information(0, tr("Restart"), tr("You will need to restart mysqlcc before you notice your changes."));
    myApp()->reset();
    delete conn;
    dialogAccepted();
}
