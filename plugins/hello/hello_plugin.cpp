#include "hello_plugin.h"
#include "hello.h"
#include "CAction.h"
#include <qpopupmenu.h>
#include <qmenubar.h>

static const char* const icon_data[] = { 
"16 16 16 1",
". c None",
"j c #65361b",
"m c #94521c",
"n c #b56321",
"f c #c8ab49",
"k c #c99029",
"i c #d4ad32",
"h c #d5bf7d",
"l c #e7ad21",
"g c #ecba2f",
"# c #f0e1c0",
"a c #f3d987",
"d c #f5cc3a",
"b c #f9e04e",
"c c #fde03f",
"e c #fff352",
".....#abba#.....",
"...#abbbbcda#...",
"..#bbeeeebcdda..",
".#bbffbeebffdd#.",
".abf##feef##fgh.",
"#cef##febf##fdi#",
"abefjjfebkjjfglh",
"bbbimjibbijmiglf",
"bcbbiibbcdiigglf",
"acccbcccccddgglh",
"#dccnidcdgknllk#",
".hddinkiiknkllf.",
".#ddgknnnnkllf#.",
"..agglllllllfh..",
"...#hillllkf#...",
".....#hffh#....."};


HelloWorldPlugin::HelloWorldPlugin()
: CMySQLCCWidgetPlugin()
{
  setText("Hello World Plugin");
  setIcon((const char **) icon_data);
}

QString HelloWorldPlugin::author() const
{
  return "Jorge del Conde - jorge@mysql.com";
}

QString HelloWorldPlugin::version() const
{
  return "1.0";
}

QString HelloWorldPlugin::name() const
{
  return "Hello World";
}

QString HelloWorldPlugin::description() const
{
  return "This is a very simple plugin based in the \"hello\" example provided by Trolltech AS.";
}

void HelloWorldPlugin::initPluginPlace(Plugin::CPluginPlaceList *plugin_place)
{
  plugin_place->append(Plugin::MAIN);
}

void HelloWorldPlugin::init()
{
  if (pluginWindow())
  {
    messagePanel();
    pluginWindow()->statusBar();
    pluginWindow()->menuBar();
    CAction * fileCloseAction = new CAction(tr("Close"), icon(), tr("&Close"),
      Qt::CTRL + Qt::Key_C, pluginWindow(), "fileCloseAction");
    fileCloseAction->setParentMenuText(tr("File"));
    connect(fileCloseAction, SIGNAL(activated()), pluginWindow(), SLOT(close()));
    QPopupMenu * fileMenu = new QPopupMenu(pluginWindow(), "fileMenu");
    pluginWindow()->menuBar()->insertItem(tr("&File"), fileMenu);
    fileCloseAction->addTo(fileMenu);
    enableHotKeyEditor();
    message(icon(), "Creating Hello World Plugin");
  }
}

QWidget * HelloWorldPlugin::createWidgetPlugin(QWidget *parent, const char *name)
{
  Hello *hello = new Hello(pluginWindow()->caption(), parent, name);
  hello->setFont(QFont("times",14,QFont::Bold) );
  hello->setBackgroundColor(Qt::black);
  return hello;
}

#include <qmessagebox.h>
void HelloWorldPlugin::exec()
{
  QMessageBox::warning(0,"Hello World", "Hello World Plugin saying Hello");
}

MYSQLCC_EXPORT_PLUGIN(HelloWorldPlugin, Plugin::WIDGET);

