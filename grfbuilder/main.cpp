#include <QApplication>
#include "qt_win.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	MainWindow MW;

	MW.show();
	return app.exec();
}

