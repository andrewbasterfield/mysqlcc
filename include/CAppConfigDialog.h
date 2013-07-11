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
#ifndef CGENERALCONFIG_H
#define CGENERALCONFIG_H

#include <stddef.h>
#include <qvariant.h>
#include "CConfigDialog.h"
#include "CSqlEditorFont.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QListBox;
class QListBoxItem;
class privateTabConfig;
class QButtonGroup;
class QRadioButton;
class QListView;
class CConfig;

class CQueryConfigTab : public CConfigDialogTab
{ 
  Q_OBJECT
    
public:
  CQueryConfigTab (QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  void setDefaultValues(CConfig * conn = 0);
  bool save(CConfig *conn);
  bool needRestart() { return needrestart; }
  
private:  
  bool needrestart;
  bool tmp_sqldebug;
  QButtonGroup* CommentsBox;
  QRadioButton* hashComments;
  QRadioButton* dashComments;
  QRadioButton* cComments;
  QButtonGroup* OpenTablesBox;
  QRadioButton* onlySqlStatement;
  QRadioButton* allRecords;
  QCheckBox* enableSqlDebug;
  QGridLayout* CQueryConfigTabLayout;
  QGridLayout* CommentsBoxLayout;
  QGridLayout* OpenTablesBoxLayout;
};

class CSyntaxHighlightConfigTab : public CConfigDialogTab
{ 
  Q_OBJECT
    
public:
  CSyntaxHighlightConfigTab (QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  void setDefaultValues(CConfig * conn = 0);
  bool save(CConfig *conn);

private slots:
  void Refresh();
  void BoldToggled(bool b);
  void ItalicToggled(bool b);
  void UnderlineToggled(bool b);
  void FontChanged(const QString &);
  void ValueChanged(int);
  void setSectionColor();
  void DefaultPushButtonClicked();

private:
  void init();
  void refreshPreview(const QFont & fnt, const QColor & clr);
  void setFontBoolValue(void (QFont::*member)(bool), bool value);  
  CSqlEditorFont *findItem(uint i);
  QHBoxLayout* CSyntaxHighlightConfigTabLayout;
  QGridLayout* Layout12;
  QListBox* Sections;
  QLineEdit* Preview;
  QCheckBox* Underline;
  QLabel* sizeLabel;
  QSpinBox* Size;
  QCheckBox* Italic;
  QLabel* preveiwLabel;
  QCheckBox* Bold;
  QComboBox* Font;
  QPushButton* Color;
  QLabel* fontLabel;
  QLabel* colorLabel;
  QPushButton* DefaultPushButton;
};


class CPluginsConfigTab : public CConfigDialogTab
{
  Q_OBJECT
    
public:
  CPluginsConfigTab (QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  void setDefaultValues(CConfig * conn = 0);
  bool save(CConfig *conn);
  bool needRestart() { return needrestart; }
  
protected slots:
  virtual void languageChange();
  
private slots:
  void setPluginsPath();
  void refreshPluginsList();
  void refreshPluginsList(const QString &path);

private:
  QPushButton* pluginsPathBrowse;
  QLabel* pluginsPathLabel;
  QLineEdit* pluginsPath;
  QLabel* textLabel1;
  QListView* pluginsList;
  QGridLayout* CPluginsConfigTabLayout;
  bool needrestart;
};

class CSqlEditorConfigTab : public CConfigDialogTab
{ 
  Q_OBJECT
    
public:
  CSqlEditorConfigTab (QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  void setDefaultValues(CConfig * conn = 0);
  bool save(CConfig *conn);  

private slots:
  void setSyntaxFile();
  
private:  
  void init();
  QGridLayout* CSqlEditorConfigTabLayout;
  QCheckBox* Parentheses;
  QLabel* SyntaxFileLabel;
  QLineEdit* SyntaxFile;
  QPushButton* SyntaxFileBrowse;
  QCheckBox* SyntaxHighlight;
  QCheckBox* LinuxPaste;
};

class CGeneralAppConfigTab : public CConfigDialogTab
{ 
  Q_OBJECT

public:
  CGeneralAppConfigTab (QWidget* parent = 0, const char* name = 0, WFlags fl = 0); 
  void setDefaultValues(CConfig * conn = 0);
  bool save(CConfig *conn);
  bool needRestart() { return needrestart; }

protected slots:
  virtual void languageChange();

private slots:  
  void setTranslationsPath();  
  void refreshLanguageCombo();
  void refreshLanguageCombo(const QString &path);
  void setErrorFile();
  void setWarningFile();
  void setInformationFile();  

private:  
  void init();
  bool needrestart;
  
  QLineEdit* errorSoundFile;
  QLineEdit* translationsPath;
  QLabel* warningSoundLabel;
  QLabel* errorSoundLabel;
  QLabel* translationsPathLabel;
  QPushButton* warningSoundBrowse;
  QPushButton* informationSoundBrowse;
  QPushButton* errorSoundBrowse;
  QPushButton* translationsBrowse;
  QLabel* informationSoundLabel;
  QLineEdit* informationSoundFile;
  QLineEdit* warningSoundFile;
  QCheckBox* saveWorkspace;
  QComboBox* language;
  QLabel* languageLabel;
  QLabel* historyNumberLabel;
  QSpinBox* historySize;
  QCheckBox* confirmCritical;
  QGridLayout* CGeneralAppConfigTabLayout;
};

class CAppConfigDialog : public CConfigDialog
{ 
  Q_OBJECT
    
public:
  CAppConfigDialog( QWidget* parent = 0,  const char* name=0);
  ~CAppConfigDialog();  

private:
  void okButtonClicked();
  CGeneralAppConfigTab * generalConfigTab;
};

#endif
