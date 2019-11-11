#include <QtWidgets/QApplication>
#include "..\Framework\entry.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QScopedPointer<QMainWindow> w(GetEntry());
	w->show();
	return a.exec();
}
