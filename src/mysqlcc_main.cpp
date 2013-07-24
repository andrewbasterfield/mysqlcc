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
#include "CMDIWindow.h"
#include "CDatabaseListView.h"
#include "CDatabaseListViewItem.h"
#include "CConsoleWindow.h"
#include "CMySQLServer.h"
#include "CConfig.h"
#include "config.h"
#include "globals.h"
#include "panels.h"
#include <stddef.h>  
#include <q3textstream.h>
#include <qapplication.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qmap.h>
//Added by qt3to4:
#include <QPixmap>
#ifndef WIN32
#include <unistd.h>
#endif

static QMap<int, QString> argumentMap;
static const QString connections_path = QString(CONNECTIONS_PATH);

static bool set_argument(CConfig *cfg, int id, const QString &key)
{
  if (!argumentMap[id].isNull())
  {
    if (argumentMap[id].length() <= 0)
      return cfg->writeEntry(key, "true");
    else
      return cfg->writeEntry(key, argumentMap[id]);
  }
  return true;
}


static bool modify_global_options()
{
#ifdef DEBUG
  qDebug("static bool modify_global_options()");
#endif

  CConfig cfg;
  bool ret = true;

  if (!argumentMap['Y'].isNull())
    ret &= cfg.writeEntry("Syntax File", argumentMap['Y']);

  if (!argumentMap['H'].isNull())
    ret &= cfg.writeEntry("History Size", argumentMap['H']);

  if (!argumentMap['T'].isNull())
    ret &= cfg.writeEntry("Translations Path", argumentMap['T']);

#ifndef NO_MYSQLCC_PLUGINS
  if (!argumentMap['g'].isNull())
    ret &= cfg.writeEntry("Plugins Path", argumentMap['g']);
#endif

  ret &= cfg.save();
  return ret;
}

static bool check_startup_arguments(const char *arguments)
{
  const char *arg = arguments;
  while (*arg)
    if (!argumentMap[*arg++].isNull())
      return true;
  return false;
}


void add_argument(int optid, char *argument)
{
  argumentMap.insert(optid, argument);
}

static QString unquote(const QString &str)
{
  if (str.startsWith("'") && str.endsWith("'"))
    return str.mid(1, str.length() - 2);
  else
    return str;
}

static Q3ListViewItem * connect_to_server(const QString &arg)
{
  QStringList databases;
  QString server;
  int p;
  Q3ListViewItem *ret = 0;
  bool db_ok = false;
  bool server_ok = false;
  bool done = false;

  if ((p = arg.find('/')) == -1)
    server = unquote(arg);
  else
  {
    server = unquote(arg.left(p));
    databases = QStringList::split("/", arg.mid(p+1));
    for (QStringList::Iterator it = databases.begin(); it != databases.end(); ++it)
    {
      QString &s = *it;
      s = unquote(*it);
    }
  }
 
  if (!server.isEmpty())
  {
    Q3ListViewItemIterator it(myApp()->consoleWindow()->databaseListView());
    while (it.current() != 0)
    {
      switch (((CDatabaseListViewItem *) it.current())->type())
      {
      case CDatabaseListViewItem::SERVER:
        if (it.current()->text(0) == server)
        {
          uint tmp = ((CDatabaseListViewItem *) it.current())->mysql()->timeout();
          ((CDatabaseListViewItem *) it.current())->mysql()->setTimeout(5);
          ((CDatabaseListViewItem *) it.current())->doubleClicked();
          ((CDatabaseListViewItem *) it.current())->mysql()->setTimeout(tmp);
          server_ok = ((CDatabaseListViewItem *) it.current())->mysql()->isConnected();
          ret = it.current();
        }
        break;
        
      case CDatabaseListViewItem::DATABASE_GROUP:
        if (server_ok)
          ((CDatabaseListViewItem *) it.current())->setOpen(true);
        break;
        
      case CDatabaseListViewItem::DATABASE:
        if (server_ok)
        {
          QStringList::Iterator db_it = databases.find(it.current()->text(0));
          if (db_it != databases.end())
          {
            ((CDatabaseListViewItem *) it.current())->doubleClicked();
            ret = it.current();
            databases.remove(db_it);
            if (databases.empty())
              done = true;
          }
        }
        break;
        
      default:
        break;
      }
      
      if (done)
      {
        db_ok = true;
        break;
      }
      ++it;
    }
  }
  
  if (!server_ok)
    myApp()->consoleWindow()->messagePanel()->warning(QObject::tr("Could not connect to the server:") + " " + server);

  if (!db_ok && server_ok)
    for (QStringList::Iterator db_it = databases.begin(); db_it != databases.end(); ++db_it )
      myApp()->consoleWindow()->messagePanel()->warning(QObject::tr("The database:") + " " + *db_it + " "
      + QObject::tr("in server:") + " " + server + " " + QObject::tr("was not found."));

  return ret;
}

static void handle_connect_to_servers(const QString &connection, CConfig *cfg)
{
  bool conn = !connection.isEmpty();

  if (conn)
  {
    Q3ListViewItem *item = connect_to_server(connection);
    if (item && !argumentMap['q'].isNull())
    {
      myApp()->consoleWindow()->prepareQueryWindow((CDatabaseListViewItem *) item);
      myApp()->consoleWindow()->openQueryWindow();
    }
  }
  
  if (myApp()->saveWorkspace())
  {
    QString restore = cfg->readStringEntry("Open Items", QString::null);
    if (!restore.isEmpty())
    {
      conn = true;
      QStringList restore_lst = QStringList::split("\t", restore);
      for (QStringList::Iterator it_restore = restore_lst.begin(); it_restore != restore_lst.end(); ++it_restore)
        connect_to_server(*it_restore);
    }
  }

  if (conn)
  {
    Q3ListViewItem *item = myApp()->consoleWindow()->databaseListView()->firstChild();
    if (item)
    {
      myApp()->consoleWindow()->databaseListView()->setSelected(item, true);
      myApp()->consoleWindow()->databaseListView()->CurrentChanged(item);
    }
  }
}

static void showLoadedPlugins()
{
#ifndef NO_MYSQLCC_PLUGINS
  QMap<QString, QPixmap>::Iterator it;
  for (it = myApp()->loadedPlugins()->begin(); it != myApp()->loadedPlugins()->end(); ++it)
  {
    QPixmap pix = it.data();
    if (pix.isNull())
      pix = getPixmapIcon("informationIcon");
    myApp()->consoleWindow()->messagePanel()->showMessage(pix, it.key());
  }
#endif
}

static void doRegisterServer()
{
  if (!argumentMap['r'].isNull() || myApp()->consoleWindow()->databaseListView()->childCount() == 0)
    myApp()->consoleWindow()->registerServer();
}

static void fix_config_bugs(CConfig * main_config)
{
  ulong v = main_config->readNumberEntry("MySQLCC Version", 0);
  if (v != MYSQLCC_VERSION)
  {
    main_config->writeEntry("MySQLCC Version", MYSQLCC_VERSION);

    if (v < 0x000903)  //Fix a bug in versions before 0.9.3 with 'Max Join Size'
    {
      QStringList dbServerList;
      CConfig::list(dbServerList, CONNECTIONS_PATH);
      CConfig *cfg = new CConfig();
      for (QStringList::Iterator j = dbServerList.begin(); j != dbServerList.end(); j++)
      {
        cfg->setConfigName(*j, CONNECTIONS_PATH);
        cfg->reset();
        cfg->writeEntry("Max Join Size", 0);
        cfg->save();
        myApp()->consoleWindow()->messagePanel()->warning(QObject::tr("Setting 'Max Join Size' to Unlimited in:") + *j);
      }
      delete cfg;
    }
    main_config->save();
  }
}

int mysqlcc_main(int argc, char** argv)
{
#ifdef DEBUG
  qDebug("main()");
#endif

  QString err, connection;  

  if (check_startup_arguments("YHTg"))
    if (!modify_global_options())
      err = "An Error occurred while saving the Configuration.";  

  CApplication app(argc, argv);

  if (*argv)
    argumentMap.insert('d', *argv);

  if (!argumentMap['c'].isNull() || !argumentMap['s'].isNull())
    if (argumentMap['s'].isNull())
      argumentMap['s'] = argumentMap['c'];

  if (check_startup_arguments("CbydhupPStcsLANlJq"))
  {
    if (argumentMap['s'].isNull())
      argumentMap['s'] = myApp()->defaultConnectionName();

    if (argumentMap['s'].isEmpty())
      err = QObject::tr("There is no default connection defined.  Create and edit a MySQL Connection and make sure the 'Default' checkbox is checked.");
    else
    {      
      CConfig *cfg = new CConfig(argumentMap['s'], connections_path);
      if (cfg->exists())
      {        
        set_argument(cfg, 'C', "Compress");
        set_argument(cfg, 'b', "Blocking Queries");
        set_argument(cfg, 'y', "Completion");
        set_argument(cfg, 'h', "Host");
        set_argument(cfg, 'u', "User");
        set_argument(cfg, 'p', "Password");
        set_argument(cfg, 'P', "Port");
#ifndef WIN32
        set_argument(cfg, 'S', "Socket");
#endif
        set_argument(cfg, 't', "Timeout");
        set_argument(cfg, 'L', "Allow Local Infile");
        set_argument(cfg, 'A', "Max Allowed Packet");
        set_argument(cfg, 'J', "Max Join Size");
        set_argument(cfg, 'N', "Net Buffer Length");
        set_argument(cfg, 'l', "Limit Select Queries Number");

        cfg->save();
        connection = argumentMap['s'];
        if (!argumentMap['d'].isNull())
        {
          argumentMap['q'] = "true";
          connection += "/" + argumentMap['d'];
        }
      }
      else
        err = QObject::tr("The server:") + " " + cfg->configName() + " " + QObject::tr("was not found.");

      delete cfg;
    }
  }
  
  app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
  int res;
  if (app.isMDI())
  {
    CMDIWindow mainwindow;
    app.setMainWidget(&mainwindow);
    myShowWindow(&mainwindow);
    mainwindow.createConsoleWindow();    
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    myApp()->consoleWindow()->databaseListView()->setBusy(true);
    CConfig *cfg = new CConfig();   //Hack to reload the mainwindow settings after the consoleWindow is created.
    fix_config_bugs(cfg);
    QString s = cfg->readStringEntry("Window " + QString(mainwindow.name()), QString::null);
    if (!s.isNull())
    {
      s = charReplace(s, '|', "\n");
      Q3TextStream ts( &s, QIODevice::ReadOnly);
      ts >> mainwindow;
    }
    if (!err.isEmpty())
      myApp()->consoleWindow()->messagePanel()->critical(err);

    handle_connect_to_servers(connection, cfg);

    delete cfg;
    myApp()->consoleWindow()->databaseListView()->setBusy(false);
    showLoadedPlugins();
    QApplication::restoreOverrideCursor();
    doRegisterServer();
    res = app.exec();
  }
  else
  {
    CConsoleWindow mainwindow(0, true, Qt::WType_TopLevel);
    app.setMainWidget(&mainwindow);
    app.setMainConsoleWindow(&mainwindow);
    if (!mainwindow.loadWindowSettings())
      mainwindow.setGeometry( (int)(qApp->desktop()->width() -  (qApp->desktop()->width() - (qApp->desktop()->width() / 2)) * 1.5) / 2,
      (int)(qApp->desktop()->height() - (qApp->desktop()->height() -  (qApp->desktop()->height() / 2)) * 1.5) / 2,
      (int)((qApp->desktop()->width() - (qApp->desktop()->width() / 2)) * 1.5),
      (int)((qApp->desktop()->height() - (qApp->desktop()->height() / 2)) * 1.5));
    myShowWindow(&mainwindow);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    mainwindow.databaseListView()->refreshServers();
    myApp()->consoleWindow()->databaseListView()->setBusy(true);    
    if (!err.isEmpty())
      mainwindow.messagePanel()->critical(err);
    CConfig *cfg = new CConfig();
    handle_connect_to_servers(connection, cfg);
    fix_config_bugs(cfg);
    delete cfg;
    myApp()->consoleWindow()->databaseListView()->setBusy(false);
    showLoadedPlugins();
    QApplication::restoreOverrideCursor();
    doRegisterServer();    
    res = app.exec();
  }

  return res;
}
