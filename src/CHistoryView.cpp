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
#include <qtextstream.h>
#include <qfile.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include "CApplication.h"
#include "CHistoryView.h"
#include "globals.h"
#include "config.h"
#include "CConfig.h"
#include <qtextcodec.h>
#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif
/*
CHistoryView() is the class responsible for handling the queries history.
*/
CHistoryView::CHistoryView()
{
#ifdef DEBUG
  qDebug("CHistoryView::CHistoryView()");
#endif

  load(); 
}

CHistoryView::~CHistoryView()
{
#ifdef DEBUG
  qDebug("CHistoryView::~CHistoryView()");
#endif
  
  write();
}


CHistoryView CHistoryView::history_view;


/*
setHistorySize() sets the MAX number of elements that can be stored
*/
void CHistoryView::setHistorySize(uint size)
{
#ifdef DEBUG
  qDebug("static CHistoryView::setHistorySize(%d)", size);
#endif
  
  history_size = size;
}


uint CHistoryView::history_size = 100;

/*
historySize() returns the MAX number of elements that should be stored in the history.
*/
uint CHistoryView::historySize()
{
#ifdef DEBUG
  qDebug("static CHistoryView::historySize()");
#endif
  
  return history_size;
}

/*
historyFile() returns the complete path to the history file.
*/
QString CHistoryView::historyFile()
{
#ifdef DEBUG
  qDebug("static CHistoryView::historyFile()");
#endif
  
  return CConfig::getRootConfigPath() + QString(HISTORY_FILE_NAME);
}


/*
load() reads the complete history file and loads each query to QStringList historyList.
*/
void CHistoryView::load()
{
#ifdef DEBUG
  qDebug("CHistoryView::load()");
#endif
  QFileInfo fi(historyFile());
  uint size = fi.size();
  if (size > 0)
  {    
    QFile f_strm(historyFile());
    if (f_strm.open(IO_ReadOnly) && size > 0)
    {
      if (!historyList.empty())
        historyList.clear();
      char *buff = new char [size];
      f_strm.readBlock (buff, size);
      f_strm.close();
      buff[size - 1] = 0;
      QString buffer = QTextCodec::codecForLocale()->toUnicode(buff);
      delete [] buff;
      bool quote = false;
      QString line = QString::null;
      if (!buffer.isEmpty())
      {                
        for (uint i = 0; i < buffer.length(); i++)
        {
          if (!quote && (buffer.at(i) == '\"'))
            quote = true;
          else
            if (quote)
            {
              if ((buffer.at(i) == '\\') && (buffer.at(i+1) == '\"'))
              {
                line += '\"';
                i++;
              }
              else
                if (buffer.at(i) == '\"')
                {
                  quote = false;
                  historyList.append(line);
                  line = QString::null;
                }
                else
                  line += buffer.at(i);
            }            
        }
        truncate();
      }
    }
  }
}


/*
loadTo() places the contents of QStringList historyList to a CHistoryPanel *[h]
*/
void CHistoryView::loadTo(CHistoryPanel *h)
{
#ifdef DEBUG
  qDebug("CHistoryView::loadTo()");
#endif
  
  if (historyList.isEmpty())
    return;  
  truncate();  
  for ( QStringList::Iterator it = historyList.begin(); it != historyList.end(); ++it )
    h->History(*it, false);
  h->setBottomItem();  
}


/*
truncate() makes sure QStringList historyList has at the most historySize() elements.
if historyList has more elements, it deletes the first N elements to satisfy the above.
*/
void CHistoryView::truncate()
{
#ifdef DEBUG
  qDebug("CHistoryView::truncate()");
#endif
  
  if (historyList.empty())
    return;
  int start = historyList.count() - historySize();
  if (start > 0)  
    for (int i = 0; i < start; i++)
      historyList.remove(historyList.at(0));
}


/*
write() writes the contents of historyList to the file called historyFile()
*/
void CHistoryView::write()
{
#ifdef DEBUG
  qDebug("CHistoryView::write()");
#endif
  
  truncate();
  if (historyList.empty())
    return;
  QFile file(historyFile());
  if (!file.open( IO_WriteOnly ))
  {    
    QMessageBox::critical(0, qApp->translate("CHistoryView", "Error"), qApp->translate("CHistoryView", "An error occurred while saving the history list."));
    return;
  }    
  QTextStream ts( &file );
  QString line;
  for ( QStringList::Iterator it = historyList.begin(); it != historyList.end(); ++it )
  {
    if (!(*it).stripWhiteSpace().isEmpty())
    {
      line = *it;
      ts << "\"" << charReplace(line, '"', "\\\"") << "\"\n";
    }
  }
  file.close();
#ifndef WIN32
    if (chmod(historyFile(), S_IRUSR | S_IWUSR) != 0)
    {
#ifdef DEBUG
      qDebug("CHistoryView::write() - Coudn't set mode 600 for" + historyFile());
#endif
    }
#endif
}


/*
append() inserts the string [s] to the bottom of the history list.
*/
bool CHistoryView::append(const QString &s)
{
#ifdef DEBUG
  qDebug("CHistoryView::append('%s')", debug_string(s));
#endif

  bool ret = false;
  if (!s.stripWhiteSpace().isEmpty())
  {
    if (historyList.empty())
      ret = true;
    else
      if (*(historyList.end()) != s)  //Apend only if the last item is not the same as "s"
        ret = true;
    if (ret)
      historyList.append(s);
  }  
  return ret;
}
