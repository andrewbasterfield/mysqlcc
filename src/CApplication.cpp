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
#include "globals.h"
#include "CMyWindow.h"
#include "icons.h"
#include "CConfig.h"
#include "CSqlEditorFont.h"
#include "CHistoryView.h"
#include <qpixmap.h>
#include <qptrlist.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtranslator.h>

#ifdef QT_OSX_BUILD
#include <CoreServices/CoreServices.h>
#endif

#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 4
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

/*
This is a inherited QApplication class.  It is used by main.cpp and the purpose
of this class is to store global variables.
*/
CApplication::CApplication (int & argc, char ** argv)
: QApplication(argc, argv), check_critical(0)
{
#ifdef DEBUG
  qDebug("CApplication::CApplication()");
#endif

  CSqlEditorFont::sqlEditorStyles()->setAutoDelete(true);
  translator = new QTranslator(0);
  initIcons();
  initGlobals(true);
}


/*
reset() calls initGlobals with a "false" startup parameter.
reset() is public while initGlobals is private to this class.
*/
void CApplication::reset()
{
#ifdef DEBUG
  qDebug("CApplication::reset()");
#endif

  initGlobals(false);
}

/*
This function initializes all the global variables
The startup parameter is used because there are a couple of settings that must only be stored in their
corresponding variable once (for example ismdi)

One should NEVER use this function except for when reading the initial configuration of mysqlcc.
One should use reset() instead.
*/
void CApplication::initGlobals(bool startup)
{
#ifdef DEBUG
  qDebug("CApplication::initGlobals(%s)", debug_string(booltostr(startup)));
#endif  
  
  CConfig *cfg = new CConfig();

#ifdef QT_OSX_BUILD
  CFBundleRef appBundle = CFBundleGetMainBundle (); // No need to release this
  CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL (appBundle); // must release this
  CFURLRef absoluteURL = CFURLCopyAbsoluteURL (resourcesURL); // must release
  CFStringRef pathref = CFURLCopyFileSystemPath ( absoluteURL , kCFURLPOSIXPathStyle ); // must release
  int strsize = 1024;
  char * pathstr = (char *)malloc(strsize);
  //CFShow(pathref);
  while (!CFStringGetCString(pathref, pathstr, strsize, kCFStringEncodingUTF8))
  {
  	strsize += 1024;
	  pathstr = (char *)realloc(pathstr, strsize);
  }
  QString mydir = pathstr;
  free(pathstr);
  CFRelease(pathref);
  CFRelease(resourcesURL);
  CFRelease(absoluteURL);
  mydir += "/";
#else
  //Directory where mysqlcc.exe (or mysqlcc) is.
  //QString mydir = QDir::currentDirPath() + "/";
  QString mydir = "/usr/share/mysqlcc/";  // This needs to be done with a PREFIX define ..
#endif

  //Translations path
  translations_path = cfg->readStringEntry("Translations Path", mydir + "translations/");  

  //Enabled Plugins
  enabled_plugins_list = QStringList::split('/', cfg->readStringEntry("Enabled Plugins List", QString::null));
  
  if (startup)
  {
    console_window = 0;  
    workspace = 0;
  
#ifdef WIN32
    ismdi = true;
    linux_paste = false;
#else
    ismdi = false;
    linux_paste = true;
#endif

    //MDI
    ismdi = strtobool(cfg->readStringEntry("MDI", booltostr(ismdi)));

    //Current Language
    current_language = cfg->readStringEntry("Language File", "English");
  
    translator->load(translations_path + "/" + current_language, "." );
    installTranslator(translator);

    //Plugins Path
    plugins_path = cfg->readStringEntry("Plugins Path", mydir + "plugins/");
#ifndef NO_MYSQLCC_PLUGINS

    if (!enabled_plugins_list.isEmpty() && QFile::exists(plugins_path))
    {
      QDir dir = QDir(plugins_path, "*");
      dir.setFilter(QDir::Files);
      dir.setSorting(QDir::Name);
      QStringList plugin_list = dir.entryList();
      for (QStringList::Iterator it = plugin_list.begin(); it != plugin_list.end(); ++it)
        if (enabled_plugins_list.find(*it) != enabled_plugins_list.end())
        {
          CPlugin p(*it);
          if (p.pluginOk())
          {
            bool ok = false;
            Plugin::CPluginPlaceList::iterator place;
            Plugin::CPluginPlaceList list = p.pluginPlace();
            for (place = list.begin(); place != list.end(); ++place)
            {
              PluginMap::Iterator it;
              it = plugin_map.find(*place);
              if (it != plugin_map.end())  //type found in plugin_map
              {
                it.data().append(p);
//loaded_plugins.insert("appended plugin: " + p.pluginFile() + " type:" + QString::number((int) *place), p.icon());
                ok = true;
              }
              else  //insert type in pluginmap
              {
                PluginList l;
                l.append(p);
                plugin_map.insert(*place, l);
//loaded_plugins.insert("inserted plugin: " + p.pluginFile() + " type:" + QString::number((int) *place), p.icon());
                ok = true;
              }
            }
            if (ok)
              loaded_plugins.insert(tr("Plugin") + " '" + p.name() + " ( " + p.pluginFile() + " )' " + tr("successfully loaded"), p.icon());
          }
          else
            loaded_plugins.insert(tr("Plugin") + " '( " + *it + " )' " + tr("failed to load"), getPixmapIcon("criticalIcon"));
        }
    }
#endif
  }

  //Default Connection Name
  default_connection_name = cfg->readStringEntry("Default Connection Name", QString::null);

  //Style
  current_style = cfg->readStringEntry("Style", "MotifPlus");
  setStyle(current_style);
  setStylePalette(current_style);
  
  //Application Font
  QFont font;
  font.fromString(cfg->readStringEntry("Font", QFont().toString()));
  setFont(font, true);
  
  //Printer Font
  printer_font.fromString(cfg->readStringEntry("Printer Font", QFont("Times", 10).toString()));  
  
  //Confirm Critical operations
  confirm_critical = strtobool(cfg->readStringEntry("Confirm Critical", "true"));

  //Save Workspace
  save_workspace = strtobool(cfg->readStringEntry("Save Workspace", "true"));  

  //History File Size
  uint history_size = cfg->readNumberEntry("History Size", 100);
  if (history_size > 1000)
    history_size = 1000;

  CHistoryView::setHistorySize(history_size);

  //Syntax File
  syntax_file = cfg->readStringEntry("Syntax File", mydir + "syntax.txt");

  //Audio files
  error_sound_file = cfg->readStringEntry("Error Sound", mydir + "error.wav");
  warning_sound_file = cfg->readStringEntry("Warning Sound", mydir + "warning.wav");
  information_sound_file = cfg->readStringEntry("Information Sound", mydir + "information.wav");

  //Query Syntax Highlighting
  syntax_highlight = strtobool(cfg->readStringEntry("Syntax Highlighting", "true"));

  //Query Parentheses Matching
  parentheses_matching = strtobool(cfg->readStringEntry("Parentheses Matching", "true"));

  //Auto - Limit SELECT Queries
  enable_sql_panel = strtobool(cfg->readStringEntry("Enable SQL Debug", "true"));

   //Retrieve All Records from Table
  retrive_all_records_from_table = strtobool(cfg->readStringEntry("Retrieve All Records From Table", "true"));

  //Default Commenting Style
  comment_style = (commenting_style) cfg->readNumberEntry("Comment Style", int(HASH_COMMENT));

    //Linux style Paste (middle button)
  linux_paste = strtobool(cfg->readStringEntry("Linux Paste Style", booltostr(linux_paste)));


  CSqlEditorFont::sqlEditorStyles()->clear();
  
  QString f, c;

#ifndef WIN32
  font = QFont(qApp->font().family(), 12, QFont::Normal);
#else
  font = QFont(qApp->font().family(), 8, QFont::Normal);
#endif

  font.setBold(false);
  //Standard should ALWAYS be the first.
  new CSqlEditorFont(cfg, "Standard", CSqlEditorFont::STANDARD, font.toString(), "0,0,0");  
  font.setBold(true);  
  new CSqlEditorFont(cfg, "Keywords", CSqlEditorFont::KEYWORDS, font.toString(), "0,0,255");
  new CSqlEditorFont(cfg, "Field Types", CSqlEditorFont::FIELD_TYPES, font.toString(), "128,0,128");
  new CSqlEditorFont(cfg, "Tables and Fields", CSqlEditorFont::TABLES_AND_FIELDS, font.toString(), "0,0,0");  

  font.setBold(false);
  new CSqlEditorFont(cfg, "Functions", CSqlEditorFont::FUNCTIONS, font.toString(), "0,0,192");  
  new CSqlEditorFont(cfg, "Variables", CSqlEditorFont::VARIABLES, font.toString(), "255,0,0");
  font.setItalic(true);  
  new CSqlEditorFont(cfg, "Comments", CSqlEditorFont::COMMENTS, font.toString(), "192,0,0");
  font.setItalic(false);
  new CSqlEditorFont(cfg, "Custom", CSqlEditorFont::CUSTOM, font.toString(), "0,0,0");  
  new CSqlEditorFont(cfg, "Strings", CSqlEditorFont::STRINGS, font.toString(), "0,192,0");;
  new CSqlEditorFont(cfg, "Numbers", CSqlEditorFont::NUMBERS, font.toString(), "128,0,255");  
  new CSqlEditorFont(cfg, "Parentheses Matching", CSqlEditorFont::PARENTHESES_MATCHING, font.toString(), "255,255,192");  

  delete cfg;
}


/*
workspace should not be deleted by ~CApplication().  it's deleted
by the "parent" parameter sent to "createWorkspace"
*/
CApplication::~CApplication()
{
#ifdef DEBUG
  qDebug("CApplication::~CApplication()");
#endif
  
  if (!ismdi)  //Close all child windows
  {
    CMyWindow *w;
    QPtrListIterator<CMyWindow> it(*childWindows());
    while ((w = it.current()) != 0 )
    {
      ++it;      
      w->close(true);
    }
    childWindows()->clear();
  }

  delete translator;
}

/*
This function creates the Workspace

if workspace == 0 mysqlcc is running in SDI mode
if workspace != 0 mysqlcc is running in MDI mode
*/
void CApplication::createWorkspace(QWidget *parent)
{
#ifdef DEBUG
  qDebug("CApplication::createWorkspace()");  
#endif
  
  if (workspace == 0)      
    workspace = new QWorkspace(parent, "workspace");  
}

/*
This function returns a cached pixmap stored in icons_map
*/
QPixmap CApplication::getPixmapIcon(const QString &key) const
{
#ifdef DEBUG
  qDebug("static CApplication::getPixmapIcon('%s')", debug_string(key));  
#endif

  QMap<QString, QPixmap>::ConstIterator i = icons_map.find(key);
  return i == icons_map.end() ? QPixmap() : *i;
}


/*
This member sets the style palette (colors)
*/
void CApplication::setStylePalette(const QString &style)
{
#ifdef DEBUG
  qDebug("static CApplication::setStylePalette('%s')", debug_string(style));  
#endif
  
  if (style== "Platinum")
  {
    QPalette p(QColor(239, 239, 239));
    qApp->setPalette(p, true);
  }
  else
    if (style == "CDE")
    {
      QPalette p(QColor(75, 123, 130));
      p.setColor(QPalette::Active, QColorGroup::Base, QColor(55, 77, 78));
      p.setColor(QPalette::Inactive, QColorGroup::Base, QColor(55, 77, 78));
      p.setColor(QPalette::Disabled, QColorGroup::Base, QColor(55, 77, 78));
      p.setColor(QPalette::Active, QColorGroup::Highlight, Qt::white);
      p.setColor(QPalette::Active, QColorGroup::HighlightedText, QColor(55, 77, 78));
      p.setColor(QPalette::Inactive, QColorGroup::Highlight, Qt::white);
      p.setColor(QPalette::Inactive, QColorGroup::HighlightedText, QColor(55, 77, 78));
      p.setColor(QPalette::Disabled, QColorGroup::Highlight, Qt::white);
      p.setColor(QPalette::Disabled, QColorGroup::HighlightedText, QColor(55, 77, 78));
      p.setColor(QPalette::Active, QColorGroup::Foreground, Qt::white);
      p.setColor(QPalette::Active, QColorGroup::Text, Qt::white);
      p.setColor(QPalette::Active, QColorGroup::ButtonText, Qt::white);
      p.setColor(QPalette::Inactive, QColorGroup::Foreground, Qt::white);
      p.setColor(QPalette::Inactive, QColorGroup::Text, Qt::white);
      p.setColor(QPalette::Inactive, QColorGroup::ButtonText, Qt::white);
      p.setColor(QPalette::Disabled, QColorGroup::Foreground, Qt::lightGray);
      p.setColor(QPalette::Disabled, QColorGroup::Text, Qt::lightGray);
      p.setColor(QPalette::Disabled, QColorGroup::ButtonText, Qt::lightGray);
      qApp->setPalette(p, true);
    }
    else
      if (style == "Motif")
      {
        QPalette p(QColor(192, 192, 192));
        qApp->setPalette(p, true);
      }
      else
        if (style == "MotifPlus")
        {
          QPalette p(QColor(192, 192, 192));
          
          qApp->setPalette(p, true);
        }
}


/*
Returns a commented string with [comment_type] comments
*/
QString CApplication::commentText(const QString &text, const QString &eol, commenting_style t)
{
#ifdef DEBUG
  qDebug("static CApplication::commentText('%s', '%s', %d)",debug_string(text), debug_string(eol), t);
#endif

  commenting_style type = t == NO_COMMENT ? myApp()->commentStyle() : t;
  QString ret(text);
  switch (type) {
  case C_COMMENT:
    ret = "/*" + eol + ret + eol + "*/" + eol;
    break;

  case HASH_COMMENT:
    ret = "# " + ret.replace(QRegExp(eol), eol + "# ");
    break;

  case DASH_COMMENT:    
    ret = " -- " + ret.replace(QRegExp(eol), eol + " -- ");
    break;

  default:
    break;
  }

  return ret;
}


/*
This function reads all the icon definitions from ../include/icons.h
and inserts them into icons_map!

All icons in mysqlcc are cached !
*/
void CApplication::initIcons()
{
#ifdef DEBUG
  qDebug("CApplication::initIcons()");  
#endif

#ifdef Q_WS_WIN
  QPixmap::setDefaultOptimization(QPixmap::MemoryOptim);
#endif

  icons_map.insert("aboutIcon", QPixmap((const char**)aboutIcon_data));
  icons_map.insert("applicationIcon", QPixmap((const char**)applicationIcon_data));
  icons_map.insert("copyIcon", QPixmap((const char**)copyIcon_data));
  icons_map.insert("cutIcon", QPixmap((const char**)cutIcon_data));
  icons_map.insert("findIcon", QPixmap((const char**)findIcon_data));
  icons_map.insert("openIcon", QPixmap((const char**)openIcon_data));
  icons_map.insert("pasteIcon", QPixmap((const char**)pasteIcon_data));
  icons_map.insert("printIcon", QPixmap((const char**)printIcon_data));
  icons_map.insert("redoIcon", QPixmap((const char**)redoIcon_data));
  icons_map.insert("saveIcon", QPixmap((const char**)saveIcon_data));
  icons_map.insert("undoIcon", QPixmap((const char**)undoIcon_data));
  icons_map.insert("newWindowIcon", QPixmap((const char**)newWindowIcon_data));
  icons_map.insert("pkIcon", QPixmap((const char**)pkIcon_data));
  icons_map.insert("mulIcon", QPixmap((const char**)mulIcon_data));
  icons_map.insert("uniIcon", QPixmap((const char**)uniIcon_data));
  icons_map.insert("nothingIcon", QPixmap((const char**)nothingIcon_data));
  icons_map.insert("otherIcon", QPixmap((const char**)otherIcon_data));
  icons_map.insert("refreshIcon", QPixmap((const char**)refreshIcon_data));
  icons_map.insert("closedFolderIcon", QPixmap((const char**)closedFolderIcon_data));
  icons_map.insert("databaseConnectedIcon", QPixmap((const char**)databaseConnectedIcon_data));
  icons_map.insert("databaseConnectIcon", QPixmap((const char**)databaseConnectIcon_data));
  icons_map.insert("databaseDisconnectedIcon", QPixmap((const char**)databaseDisconnectedIcon_data));
  icons_map.insert("databaseDisconnectIcon", QPixmap((const char**)databaseDisconnectIcon_data));
  icons_map.insert("deleteIcon", QPixmap((const char**)deleteIcon_data));
  icons_map.insert("designIcon", QPixmap((const char**)designIcon_data));
  icons_map.insert("exportTableIcon", QPixmap((const char**)exportTableIcon_data));
  icons_map.insert("importTableIcon", QPixmap((const char**)importTableIcon_data));
  icons_map.insert("newTableIcon", QPixmap((const char**)newTableIcon_data));
  icons_map.insert("openFolderIcon", QPixmap((const char**)openFolderIcon_data));
  icons_map.insert("openTableIcon", QPixmap((const char**)openTableIcon_data));
  icons_map.insert("refreshTablesIcon", QPixmap((const char**)refreshTablesIcon_data));
  icons_map.insert("registerServerIcon", QPixmap((const char**)registerServerIcon_data));
  icons_map.insert("serverConnectedIcon", QPixmap((const char**)serverConnectedIcon_data));
  icons_map.insert("serverDisconnectedIcon", QPixmap((const char**)serverDisconnectedIcon_data));
  icons_map.insert("tableIcon", QPixmap((const char**)tableIcon_data));
  icons_map.insert("toolsIcon", QPixmap((const char**)toolsIcon_data));
  icons_map.insert("editIcon", QPixmap((const char**)editIcon_data));
  icons_map.insert("newDatabaseIcon", QPixmap((const char**)newDatabaseIcon_data));
  icons_map.insert("trashIcon", QPixmap((const char**)trashIcon_data));
  icons_map.insert("saveAllIcon", QPixmap((const char**)saveAllIcon_data));
  icons_map.insert("cascadeIcon", QPixmap((const char**)cascadeIcon_data));
  icons_map.insert("exitIcon", QPixmap((const char**)exitIcon_data));
  icons_map.insert("closeIcon", QPixmap((const char**)closeIcon_data));
  icons_map.insert("closeAllIcon", QPixmap((const char**)closeAllIcon_data));
  icons_map.insert("tileIcon", QPixmap((const char**)tileIcon_data));
  icons_map.insert("windowIcon", QPixmap((const char**)windowIcon_data));
  icons_map.insert("selectQueryIcon", QPixmap((const char**)selectQueryIcon_data));
  icons_map.insert("tablesIcon", QPixmap((const char**)tablesIcon_data));
  icons_map.insert("appendQueryIcon", QPixmap((const char**)appendQueryIcon_data));
  icons_map.insert("cancelQueryIcon", QPixmap((const char**)cancelQueryIcon_data));
  icons_map.insert("deleteQueryIcon", QPixmap((const char**)deleteQueryIcon_data));
  icons_map.insert("executeQueryIcon", QPixmap((const char**)executeQueryIcon_data));
  icons_map.insert("gridIcon", QPixmap((const char**)gridIcon_data));
  icons_map.insert("newTableQueryIcon", QPixmap((const char**)newTableQueryIcon_data));
  icons_map.insert("queryTypesIcon", QPixmap((const char**)queryTypesIcon_data));
  icons_map.insert("sqlIcon", QPixmap((const char**)sqlIcon_data));
  icons_map.insert("updateQueryIcon", QPixmap((const char**)updateQueryIcon_data));
  icons_map.insert("contextHelpIcon", QPixmap((const char**)contextHelpIcon_data));
  icons_map.insert("fontsIcon", QPixmap((const char**)fontsIcon_data));
  icons_map.insert("stylesIcon", QPixmap((const char**)stylesIcon_data));
  icons_map.insert("hammerIcon", QPixmap((const char**)hammerIcon_data));
  icons_map.insert("renameTableIcon", QPixmap((const char**)renameTableIcon_data));
  icons_map.insert("serverAdministrationIcon", QPixmap((const char**)serverAdministrationIcon_data));
  icons_map.insert("showVariablesIcon", QPixmap((const char**)showVariablesIcon_data));
  icons_map.insert("showStatusIcon", QPixmap((const char**)showStatusIcon_data));
  icons_map.insert("showProcessListIcon", QPixmap((const char**)showProcessListIcon_data));
  icons_map.insert("pingIcon", QPixmap((const char**)pingIcon_data));
  icons_map.insert("serverShutdownIcon", QPixmap((const char**)serverShutdownIcon_data));
  icons_map.insert("flushIcon", QPixmap((const char**)flushIcon_data));
  icons_map.insert("timerIcon", QPixmap((const char**)timerIcon_data));
  icons_map.insert("killProcessIcon", QPixmap((const char**)killProcessIcon_data));
  icons_map.insert("warningIcon", QPixmap((const char**)warningIcon_data));
  icons_map.insert("criticalIcon", QPixmap((const char**)criticalIcon_data));
  icons_map.insert("informationIcon", QPixmap((const char**)informationIcon_data));
  icons_map.insert("propertiesIcon", QPixmap((const char**)propertiesIcon_data));
  icons_map.insert("chooseFieldsIcon", QPixmap((const char**)chooseFieldsIcon_data));
  icons_map.insert("clearGridIcon", QPixmap((const char**)clearGridIcon_data));
  icons_map.insert("checkedIcon", QPixmap((const char**)checkedIcon_data));
  icons_map.insert("uncheckedIcon", QPixmap((const char**)uncheckedIcon_data));
  icons_map.insert("loadIcon", QPixmap((const char**)loadIcon_data));
  icons_map.insert("savePixmapIcon", QPixmap((const char**)savePixmapIcon_data));
  icons_map.insert("flipHIcon", QPixmap((const char**)flipHIcon_data));
  icons_map.insert("flipVIcon", QPixmap((const char**)flipVIcon_data));
  icons_map.insert("rotateIcon", QPixmap((const char**)rotateIcon_data));
  icons_map.insert("bits32Icon", QPixmap((const char**)bits32Icon_data));
  icons_map.insert("bits8Icon", QPixmap((const char**)bits8Icon_data));
  icons_map.insert("bit1Icon", QPixmap((const char**)bit1Icon_data));
  icons_map.insert("historyScriptIcon", QPixmap((const char**)historyScriptIcon_data));
  icons_map.insert("saveGridResultsIcon", QPixmap((const char**)saveGridResultsIcon_data));
  icons_map.insert("userIcon", QPixmap((const char**)userIcon_data));
  icons_map.insert("databaseUsersIcon", QPixmap((const char**)databaseUsersIcon_data));
  icons_map.insert("newUserIcon", QPixmap((const char**)newUserIcon_data));
  icons_map.insert("editUserIcon", QPixmap((const char**)editUserIcon_data));
  icons_map.insert("deleteUserIcon", QPixmap((const char**)deleteUserIcon_data));
  icons_map.insert("databaseUsersDisconnectedIcon", QPixmap((const char**)databaseUsersDisconnectedIcon_data));
  icons_map.insert("globalPrivsIcon", QPixmap((const char**)globalPrivsIcon_data));
  icons_map.insert("printerFontIcon", QPixmap((const char**)printerFontIcon_data));
  icons_map.insert("ftIcon", QPixmap((const char**)ftIcon_data));
  icons_map.insert("insertRowIcon", QPixmap((const char**)insertRowIcon_data));
  icons_map.insert("deleteRowIcon", QPixmap((const char**)deleteRowIcon_data));
  icons_map.insert("plusIcon", QPixmap((const char**)plusIcon_data));
  icons_map.insert("minusIcon", QPixmap((const char**)minusIcon_data));
  icons_map.insert("leftArrowIcon", QPixmap((const char**)leftArrowIcon_data));
  icons_map.insert("rightArrowIcon", QPixmap((const char**)rightArrowIcon_data));
  icons_map.insert("eraserIcon", QPixmap((const char**)eraserIcon_data));
  icons_map.insert("emptyPkIcon", QPixmap((const char**)emptyPkIcon_data));
  icons_map.insert("keyboardIcon", QPixmap((const char**)keyboardIcon_data));
  icons_map.insert("pictureIcon", QPixmap((const char**)pictureIcon_data));
  icons_map.insert("textEditorIcon", QPixmap((const char**)textEditorIcon_data));
  icons_map.insert("scalePictureIcon", QPixmap((const char**)scalePictureIcon_data));
  icons_map.insert("queryIcon", QPixmap((const char**)queryIcon_data));
  icons_map.insert("firstIcon", QPixmap((const char**)firstIcon_data));
  icons_map.insert("backIcon", QPixmap((const char**)backIcon_data));
  icons_map.insert("toggleIcon", QPixmap((const char**)toggleIcon_data));
  icons_map.insert("nextIcon", QPixmap((const char**)nextIcon_data));
  icons_map.insert("lastIcon", QPixmap((const char**)lastIcon_data));
  icons_map.insert("newTabIcon", QPixmap((const char**)newTabIcon_data));
  icons_map.insert("deleteTabIcon", QPixmap((const char**)deleteTabIcon_data));
  icons_map.insert("saveToClipboardIcon", QPixmap((const char**)saveToClipboardIcon_data));
}

