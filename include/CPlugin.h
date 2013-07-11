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
#ifndef CPLUGIN_H
#define CPLUGIN_H

#include "CMySQLCCPlugin.h"
#include <stddef.h>
#include <qmap.h>
#include <qvaluelist.h>

class QPixmap;
class QString;
class CPlugin;

typedef QValueList<CPlugin> PluginList;
typedef QMap<Plugin::mysqlcc_plugin_place, PluginList> PluginMap;

class CPlugin
{
public:
  CPlugin(const QString &file=QString::null);
  CPlugin(const CPlugin &p);
  QString name() const { return m_name; }
  QString text() const { return m_text; }
  QPixmap icon() const { return m_icon; }
  QString pluginFile() const { return m_plugin_file; }
  QString version() const { return m_version; }
  QString author() const { return m_author; }
  QString about() const { return m_about; }
  QString description() const { return m_description; }
  Plugin::CPluginPlaceList pluginPlace() const { return plugin_place; }
  bool pluginOk() const { return plugin_ok; }
  Plugin::mysqlcc_plugin_type type() const { return m_type; }

  static CMySQLCCPlugin * createPlugin(const CPlugin &);

  bool operator == (const CPlugin &) const;
  CPlugin & operator=(const CPlugin &);

private:
  bool plugin_ok;
  QString m_name;
  QString m_text;
  QString m_plugin_file;
  QString m_description;
  QString m_version;
  QString m_author;
  QString m_about;
  QPixmap m_icon;
  Plugin::CPluginPlaceList plugin_place;
  Plugin::mysqlcc_plugin_type m_type;
};

#endif

