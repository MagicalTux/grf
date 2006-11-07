#include <QtGui>
#include "qt_win.h"
#include <libgrf.h>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
	uint32_t version=grf_version();
	uint8_t major, minor, revision;
	char win_name[256];
	major = (version >> 16) & 0xff;
	minor = (version >> 8) & 0xff;
	revision = version & 0xff;
	ui.setupUi((QDialog*)this);
	sprintf((char *)&win_name, "GrfBuilder v1.0 (libgrf v%d.%d.%d) by MagicalTux", major, minor, revision);
	((QDialog*)this)->setWindowTitle(QApplication::translate("main_window", win_name, 0, QApplication::UnicodeUTF8));
}

