#include "PluginApp.h"
#include <QPluginLoader>
#include <Interface.h>
#include <QDebug>

PluginApp::PluginApp(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QPluginLoader loader("F:\\Github\\Crystal-Software-Platform\\Crystal Software (Qt)\\x64\\Debug\\PluginTool.dll");
	QObject *plugin = loader.instance();
	if (plugin) {
		qDebug() << plugin->objectName();
		qobject_cast<Interface_Plugin*>(plugin)->Print();
	}
}
