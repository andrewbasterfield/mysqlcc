#include "CMySQLCCPlugin.h"

class HelloWorldPlugin : public CMySQLCCWidgetPlugin
{
public:
  HelloWorldPlugin();
  QWidget * createWidgetPlugin(QWidget *, const char *);
  void initPluginPlace(Plugin::CPluginPlaceList *);
  void init();
  QString author() const;
  QString description() const;
  QString version() const;
  QString name() const;
  void exec();
};
