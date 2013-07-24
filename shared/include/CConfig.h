//Added by qt3to4:
#include <Q3TextStream>
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
#ifndef CCONFIG_H
#define CCONFIG_H

class Q3TextStream;

#include <stddef.h>
#include <qmap.h>
#include <qstring.h>

class QStringList;

class CConfig
{
public:
  CConfig(const QString &name = QString::null, const QString &path = QString::null);
  ~CConfig();
  bool reset();
  bool save();
  bool exists();
  bool remove();
  
  bool writeEntry(const QString & key, const QString & value=QString::null);
  bool writeEntry(const QString & key, ulong value);
  bool removeEntry(const QString & key);
  QString readStringEntry(const QString & key, const QString &def="");
  ulong readNumberEntry(const QString & key, ulong def=0);
  
  QString configName() const;
  
  void setConfigName(const QString &name = QString::null, const QString &path = QString::null);

  static QString getRootConfigPath();
  static void list(QStringList &list, const QString & path);
  static bool exists(const QString &name, const QString &path = QString::null);
  static void remove(const QString &name, const QString &path = QString::null);
  static bool rename(const QString &old_name, const QString &new_name, const QString &path = QString::null);
  
private:
  QMap<QString, QString> entries;
  QString absoluteConfigFileName;
  QString absoluteConfigPath;
  static bool createDirectory(const QString &d);
};

#endif
