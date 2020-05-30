#include "stdafx.h"
#include "FixedUI_DEMO.h"
#include <QtWidgets/QApplication>

#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);
	FixedUI_DEMO w;
	w.show();
	return a.exec();
}
