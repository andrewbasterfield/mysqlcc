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
#include "CPlugin.h"
#include "globals.h"
#include "CConfig.h"
#include <qpixmap.h>
#include <qlibrary.h>
#include <qfile.h>

CPlugin::CPlugin(const QString &file)
: m_plugin_file(file)
{
  plugin_ok = false;
  m_type = Plugin::NO_TYPE;
  plugin_place.clear();
  m_name = QString::null;
  if (file.isEmpty())
    return;  
#ifndef NO_MYSQLCC_PLUGINS

  if (!QFile::exists(myApp()->pluginsPath() + file))
    return;

  typedef Plugin::mysqlcc_plugin_type (*p_pluginType)();

  p_pluginType plugin_type = (p_pluginType) QLibrary::resolve(myApp()->pluginsPath() + file, "get_mysqlcc_plugin_type");
  if (plugin_type)
    m_type = plugin_type();

  if (m_type == Plugin::NO_TYPE)
    return;

  typedef CMySQLCCPlugin* (p_createPlugin)();

  p_createPlugin *plugin_ptr = (p_createPlugin*) QLibrary::resolve(myApp()->pluginsPath() + file, "instanciate_mysqlcc_plugin");
  if (plugin_ptr)
  {      
    CMySQLCCPlugin *plugin = (plugin_ptr)();
    if (plugin)
    {
      plugin->initPluginPlace(&plugin_place);
      if (!plugin_place.empty())
      {
        m_icon = plugin->icon();
        m_text = plugin->text();
        m_description = plugin->description();
        m_version = plugin->version();
        m_about = plugin->about();
        m_author = plugin->author();
        m_name = plugin->name();
        if (!m_name.isEmpty())
          plugin_ok = true;
      }
      delete plugin;
    }
  }

#endif
}

CPlugin::CPlugin(const CPlugin &p)
{
  plugin_ok = p.pluginOk();
  m_name = p.name();
  m_plugin_file = p.pluginFile();
  m_description = p.description();
  m_version = p.version();
  m_text = p.text();
  m_author = p.author();
  m_about = p.about();
  m_icon = p.icon();
  m_type = p.type();
  plugin_place = p.pluginPlace();
}

CMySQLCCPlugin * CPlugin::createPlugin(const CPlugin &p)
{
  if (!p.pluginOk())
    return 0;
  CMySQLCCPlugin *plugin;
  typedef CMySQLCCPlugin* (p_createPlugin)();
  p_createPlugin *plugin_ptr = (p_createPlugin*) QLibrary::resolve(myApp()->pluginsPath() + p.pluginFile(), "instanciate_mysqlcc_plugin");
  if (plugin_ptr)
  {
    plugin = (CMySQLCCWidgetPlugin *)(plugin_ptr)();
    if (plugin)
    {
      plugin->setApplication(myApp());
      plugin->setConfigPath(CConfig::getRootConfigPath());
      plugin->setLanguage(myApp()->currentLanguage());
      plugin->setConfirmCritical(myApp()->confirmCritical());
      plugin->setWorkSpace(myApp()->workSpace());
      plugin->setDefaultConnectionName(myApp()->defaultConnectionName());
      plugin->setPluginsPath(myApp()->pluginsPath());
      return plugin;
    }
  }
  return 0;
}

bool CPlugin::operator==(const CPlugin &p) const
{
  bool ret = (m_name == p.name());
  ret &= (plugin_place == p.pluginPlace());
  ret &= (m_plugin_file == p.pluginFile());
  ret &= (m_version == p.version());
  ret &= (m_type == p.type());
  ret &= (m_text == p.text());
  return ret;
}

CPlugin& CPlugin::operator=(const CPlugin &p)
{
  plugin_ok = p.pluginOk();
  m_name = p.name();
  m_plugin_file = p.pluginFile();
  m_description = p.description();
  m_version = p.version();
  m_author = p.author();
  m_about = p.about();
  m_icon = p.icon();
  m_type = p.type();
  m_text = p.text();
  plugin_place = p.pluginPlace();
  return *this;
}
