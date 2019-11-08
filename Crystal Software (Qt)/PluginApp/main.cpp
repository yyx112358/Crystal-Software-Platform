#include "PluginApp.h"
#include <QtWidgets/QApplication>
#include <QtPlugin>
//Q_IMPORT_PLUGIN(PluginStaticTool)

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	PluginApp w;
	w.show();
	return a.exec();
}
