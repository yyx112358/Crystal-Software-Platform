#include "TestAnything.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TestAnything w;
	w.show();
	return a.exec();
}
