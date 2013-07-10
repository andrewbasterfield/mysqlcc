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
#include <qtextstream.h> 
#include <qdir.h>
#include <qstringlist.h>
#include <qfile.h>

#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef HAVE_MYSQLCC_CONFIG
#include "globals.h"
#include "config.h"
#else
#include "shared.h"
#endif

#ifdef QT_OSX_BUILD
#include <CoreServices/CoreServices.h>
#endif

#ifndef EXENAME
#define EXENAME "mysqlcc"  //Name of default config file  CConfig automatically appends ".cfg" to it.
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
CConfig is a class used for reading & writing configuration files.

It is currently used for handling mysqlcc's general configuration, query history
and connections.
  
A typical usage for this class would be:
    
CConfig a("my_file.cnf")
QString value = a.readStringEntry("key", "default_value");
a.writeEntry("key", "value");
a.save();  
*/

/*
Creates a CConfig object.

If name is empty, then [EXENAME].cfg is used by default (see config.h)
  
If name is not empty, then [name] will be used as the config file name.
    
[path] is a relative path argument to be used inside the main configuration directory.
      
For example:  assuming getRootConfigPath() returns /home/jorge/.mysqlcc/
        
CConfig cfg("name.cfg", "my_path") will create a configuration class for the file
"name.cfg" which is located in "/home/jorge/.mysqlcc/my_path/name.cfg
          
NOTE:  [path] must NOT include slashes !
*/
CConfig::CConfig(const QString &name, const QString &path)
{
#ifdef DEBUG
  qDebug("CConfig::CConfig('%s', '%s')", debug_string(name), debug_string(path));
#endif
  
  setConfigName(name, path);
  reset();
}

void CConfig::setConfigName(const QString &name, const QString &path)
{
#ifdef DEBUG
  qDebug("CConfig::setConfigName('%s', '%s')", debug_string(name), debug_string(path));
#endif
  
  QString tmpFile;
  if (name.isNull())  
    tmpFile = QString(EXENAME) + ".cfg";
  else
    tmpFile = name;  
  
  QString tmpPath = QString::null;
  if (!path.isNull())    
    tmpPath = path + "/";
  
  absoluteConfigPath = getRootConfigPath() + tmpPath;
  
  if (!tmpPath.isNull())
    createDirectory(absoluteConfigPath);
  
  absoluteConfigFileName = absoluteConfigPath + charReplace(tmpFile, ':', ";");
}

CConfig::~CConfig()
{	
}

void CConfig::list(QStringList &list, const QString &path)
{
#ifdef DEBUG
  qDebug("static CConfig::list(QStringList &, '%s')", debug_string(path));
#endif
  
  QDir dir = QDir(getRootConfigPath() + path);
  if (!dir.exists())
    return;
  const QFileInfoList *filist = dir.entryInfoList( QDir::DefaultFilter, QDir::DirsFirst | QDir::Name );
  QFileInfoListIterator it( *filist );
  QFileInfo *fi;    
  while ( ( fi = it.current() ) != 0 )
  {
    ++it;
    if ( fi && fi->fileName() != ".." && fi->fileName() != ".")
      list.append(charReplace(fi->fileName(), ';', ":"));      
  }  
}

/*
This function reads all the entries of the file "configName" and stores them
in entries.

It returns true when successfull & false when the file could not be read.
*/
bool CConfig::reset()
{
#ifdef DEBUG
  qDebug("CConfig::reset()");
#endif
  
  QFile f_strm(absoluteConfigFileName);
  if (f_strm.open(IO_ReadOnly))
  {
    QTextStream t_strm(&f_strm);
    entries.clear();
    QString str;
    QString key;
    int pos;
    while ( !t_strm.atEnd() )
    {
      str = t_strm.readLine();    
      if (str.contains('=') >= 1)  //is this a valid entry ?
      {
        pos = str.find('=');
        key = str.left(pos).stripWhiteSpace();
        entries[key] = str.mid(pos + 1).stripWhiteSpace();
      }    
    }
    f_strm.close();
    return true;
  }
  else
    return false;
}


/*
This function reads all the entries of the file "configName" and stores them
in entries.

It returns true when successfull & false when the file could not be written.
*/
bool CConfig::save()
{
#ifdef DEBUG
  qDebug("CConfig::save()");
#endif

  QFile f_strm(absoluteConfigFileName);
  if (f_strm.open(IO_WriteOnly))
  {
    QTextStream t_strm(&f_strm);
    for (QMap<QString, QString>::Iterator it = entries.begin(); it != entries.end(); ++it)
      t_strm << it.key() << "\t=\t" << it.data() << "\r\n";
    f_strm.close();
#ifndef WIN32
    if (chmod(absoluteConfigFileName, S_IRUSR | S_IWUSR) != 0)
    {
#ifdef DEBUG
      qDebug("CConfig::save() - Coudn't set mode 600 for" + absoluteConfigFileName);
#endif
    }
#endif
    return true;
  }
  else
    return false;
}


bool CConfig::writeEntry(const QString & key, ulong value)
{
#ifdef DEBUG
  qDebug("CConfig::writeEntry('%s', %d)", debug_string(key), value);
#endif

  return writeEntry(key, QString::number(value));
}


bool CConfig::writeEntry(const QString & key, const QString & value)
{
#ifdef DEBUG
  qDebug("CConfig::writeEntry('%s', '%s')", debug_string(key), debug_string(value));
#endif

  if (entries.find(key) != entries.end())
    entries.remove(key);
  entries.insert(key, value);
  return true;
}

bool CConfig::removeEntry(const QString & key)
{
#ifdef DEBUG
  qDebug("CConfig::removeEntry('%s')", debug_string(key));
#endif

  if (entries.find(key) != entries.end())
  {
    entries.remove(key);
    return true;
  }
  else
    return false;
}


ulong CConfig::readNumberEntry(const QString & key, ulong def)
{
#ifdef DEBUG
  qDebug("CConfig::readNumberEntry('%s', %d)", debug_string(key), def);
#endif

  return readStringEntry(key, QString::number(def)).toULong();
}


QString CConfig::readStringEntry(const QString & key, const QString & def)
{
#ifdef DEBUG
  qDebug("CConfig::readStringEntry('%s', '%s')", debug_string(key), debug_string(def));
#endif

  if (entries.empty())
    return def;
  else
  {
    QMap<QString, QString>::Iterator i = entries.find(key);
    return i == entries.end() ? def : *i;
  }
}


bool CConfig::exists()
{
#ifdef DEBUG
  qDebug("CConfig::exists()");
#endif

  return QFile::exists(absoluteConfigFileName);
}


bool CConfig::remove()
{
#ifdef DEBUG
  qDebug("CConfig::remove()");
#endif

  return QFile::remove(absoluteConfigFileName);
}


/*
This function returns the absolute path of the user's home directory/.mysqlcc/
For example, assuming the users home directory is "/home/jorge/", this function
will return:  "/home/jorge/.mysqlcc/"

If the directory doesn't exist, it will automatically be created.
*/
QString CConfig::getRootConfigPath()
{
#ifdef DEBUG
  qDebug("static CConfig::getRootConfigPath()");
#endif

#ifdef QT_OSX_BUILD
  QString tmpstr;
  CFBundleRef appBundle = CFBundleGetMainBundle(); // No need to release this
  CFStringRef ident = CFBundleGetIdentifier(appBundle); // Do not release
  int strsize = 1024;
  char * pathstr = (char *)malloc(strsize);
  if (ident)
  {
	  while (!CFStringGetCString(ident, pathstr, strsize, kCFStringEncodingUTF8))
    {
		  strsize += 1024;
		  pathstr = (char *)realloc(pathstr, strsize);
	  }
  }
  else
  {
    free(pathstr);
  	pathstr = strdup(EXENAME);
  }
  FSRef foundref;
  OSStatus errCode = FSFindFolder(kUserDomain, kPreferencesFolderType, 1, &foundref);
  if (!errCode)
  {
	  int strsize1 = 1024;
	  char * pathstr1 = (char *)malloc(strsize);
	  while (errFSBadBuffer == FSRefMakePath(&foundref, pathstr1, strsize1))
    {
		  strsize += 1024;
		  pathstr1 = (char *)realloc(pathstr1, strsize);
	  }
	  if (pathstr1 && strlen(pathstr1))
    {
	  	tmpstr = pathstr1;
	  	tmpstr += "/";
	  }
	  else
    {
		  tmpstr = QDir::homeDirPath();
		  tmpstr += "/Library/Preferences/";	  
	  }
	  free(pathstr1);
  }
  else
  {  
	  tmpstr = QDir::homeDirPath();
	  tmpstr += "/Library/Preferences/";
  }
  if (pathstr && strlen (pathstr))
  {
  	tmpstr += pathstr;
  	free (pathstr);
  }
  else
  	tmpstr += EXENAME;
  createDirectory(tmpstr);
  tmpstr += "/";
#else
  QString tmpstr(QDir::homeDirPath() + "/." + EXENAME);
  createDirectory(tmpstr);
  tmpstr += "/";
#endif

  return tmpstr;
}

/*
This function returns the config_name without path
*/
QString CConfig::configName() const
{
#ifdef DEBUG
  qDebug("CConfig::configName()");
#endif

  return getFileName(absoluteConfigFileName);
}

/*
This function creates a directory and prints DEBUG messages through it's status
*/
bool CConfig::createDirectory(const QString &d)
{
#ifdef DEBUG
  qDebug("private static CConfig::createDirectory('%s')", debug_string(d));
#endif

  QDir dir = QDir(d);
  if (!dir.exists())
  {
#ifdef DEBUG
    qDebug("private static CConfig::createDirectory() - " + d + " does not exist ... trying to create it.");
#endif

    if (dir.mkdir(d))
    {
#ifdef DEBUG
      qDebug("private static CConfig::createDirectory() - " + d + " was created successfully.");
#endif
#ifndef WIN32
      if (chmod(d, S_IRUSR | S_IWUSR | S_IXUSR) != 0)
      {
#ifdef DEBUG
        qDebug("private static CConfig::createDirectory() - Coudn't set mode 700 for" + d);
#endif
      }
#endif
      return true;
    }
    else
    {
#ifdef DEBUG
      qDebug("private static CConfig::createDirectory() - " + d + " could not be created.");
#endif
      return false;
    }
  }
  else
  {
#ifdef DEBUG
    qDebug("private static CConfig::createDirectory() - " + d + " already exists.");
#endif
    return true;
  }
}


bool CConfig::exists(const QString &name, const QString &path)
{
#ifdef DEBUG
  qDebug("static CConfig::exists('%s', '%s')", debug_string(name), debug_string(path));
#endif
  
  QString f = getRootConfigPath();
  if (!path.isEmpty())
    f += path + "/";
  f += charReplace(name, ':', ";");
  return QFile::exists(f);
}


void CConfig::remove(const QString &name, const QString &path)
{
#ifdef DEBUG
  qDebug("static CConfig::remove('%s', '%s')", debug_string(name), debug_string(path));
#endif
  
  CConfig *c = new CConfig(name, path);
  c->remove();
  delete c;
}

bool CConfig::rename(const QString &old_name, const QString &new_name, const QString &path)
{
#ifdef DEBUG
  qDebug("static CConfig::rename('%s', '%s', '%s')", debug_string(old_name), debug_string(new_name), debug_string(path));
#endif

  if (new_name.isEmpty())
    return false;

  CConfig *c = new CConfig(old_name, path);
  c->setConfigName(new_name, path);
  int ret = c->save();
  delete c;
  if (ret)
    remove(old_name, path);
  return ret;
}
