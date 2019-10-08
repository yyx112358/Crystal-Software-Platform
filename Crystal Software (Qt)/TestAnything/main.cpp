#include "TestAnything.h"
#include <QtWidgets/QApplication>
#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TestAnything w;
	w.show();
	return a.exec();
}
