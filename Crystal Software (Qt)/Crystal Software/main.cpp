#include "stdafx.h"
#include "CrystalSoftware.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	CrystalSoftware w;
	w.show();
	return a.exec();
}
