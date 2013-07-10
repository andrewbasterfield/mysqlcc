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
#include "config.h"
#include "panels.h"
#include <qpixmap.h>
#include <qregexp.h>
#include <qstringlist.h>

/*
Wrapper for CApplication::Application() (see CApplication)
*/
CApplication * myApp()
{
  return CApplication::Application();
}


/*
Wrapper for CApplication::getPixmapIcon() (see CApplication)
*/
const QPixmap getPixmapIcon(const QString &key)
{
  return myApp()->getPixmapIcon(key);
}


/*
This function loads a section of the syntax.txt file
*/
bool loadSyntaxSection(QStringList *list, int section, bool lower_case, QString syntaxFile)
{
  list->clear();  
  QFile f_strm(syntaxFile.isNull() ? myApp()->syntaxFile() : syntaxFile);
  if (f_strm.open(IO_ReadOnly))
  {
    QTextStream t_strm(&f_strm);
    bool sectionFound = false;
    QString str;
    QRegExp rx;
    while ( !t_strm.atEnd() )
    {
      str = t_strm.readLine().stripWhiteSpace();
      rx.setPattern("^\\[(\\d+)\\s(\\d+)\\]$");
      if (rx.search(str) != -1)
        sectionFound = (rx.cap(1).toInt() == section);
      else
        if (sectionFound && !str.isEmpty())
        {
          int pos = str.find(" - ");
          if (pos != -1)
            str = str.left(pos).stripWhiteSpace();
          if (lower_case)
            str = str.lower();
          list->append(str);
        }
    }
    f_strm.close();
    return !list->empty();
  }
  else
    return false;
}


/*
This function shows a window depending on its state.
*/
void myShowWindow(CMyWindow *wnd)
{ 
  switch (wnd->windowState()) {
  case Qt::WState_Maximized:
    wnd->showMaximized();
    break;
    /* Bug in Qt
    case Qt::WState_Minimized:
    wnd->showMinimized();
    break;
    */
  default:
    wnd->show();
    break;
  }
  wnd->autoPlace();
}


/*
saveToFile() saves the contents of [contents] to a file called [fileName].
*/
void saveToFile(QString &fileName, const QString &default_ext, const QString &ext_desc, const char * contents, CMessagePanel *m, bool writeBinary, uint len)
{ 
  QString fn = getSaveFileName(fileName, default_ext, ext_desc);  
  if ( !fn.isEmpty() )		
  {
    QFile file( fn );    
    if (file.exists() && myApp()->confirmCritical())
      if ((QMessageBox::warning(0, qApp->translate("SaveToFile", "Replace File"), qApp->translate("SaveToFile", "The specified file name already exists.\nDo you want to replace it ?"),
        QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes))
        return;
    if (!file.open(IO_WriteOnly))
    {
      if (m != 0)
        m->critical(qApp->translate("saveToFile", "An error occurred while saving the file"));
      else
        QMessageBox::critical(0, qApp->translate("saveToFile", "Save to File"), qApp->translate("saveToFile", "An error occurred while saving the file"));
      return;
    }
    fileName = fn;
    if (!writeBinary)
    {
      QTextStream ts(&file);
      ts << contents;
    }
    else
    {
      QDataStream ts(&file);
      ts.writeRawBytes(contents, len);
    }
    file.close();
    if (m != 0)      
      m->information(qApp->translate("saveToFile", "Successfully saved:") + " " + fn);
  }	
}

/*
plugins() returns a PluginList for all the "type" plugins
*/
PluginList myPluginsList(Plugin::mysqlcc_plugin_place place)
{
  PluginMap::Iterator it;
  PluginMap m = *myApp()->pluginsMap();
  it = m.find(place);
  if (it != m.end())
    return it.data();
  else
    return PluginList();
}
