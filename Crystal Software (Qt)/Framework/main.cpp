#include "stdafx.h"
#include "Controller.h"
#include <QtWidgets/QApplication>

#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Controller w;
	w.show();
	return a.exec();
}
