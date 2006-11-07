#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include "qt_win.h"

/* libgrf */
#include <libgrf.h>

#include <stdio.h>

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

bool MainWindow::progress_callback(void *, int pos, int max) {
	ui.progbar->setValue(pos * 100 / max);
	return true;
}

static bool grf_callback_caller(void *MW_, void *grf, int pos, int max) {
	MainWindow *MW = (MainWindow *)MW_;
	return MW->progress_callback(grf, pos, max);
}

void MainWindow::on_btn_open_clicked() {
	QString str = QFileDialog::getOpenFileName(this, tr("Open File"),
			NULL, tr("GRF Files (*.grf *.gpf)"));

	if (str.size() == 0) return;

	this->on_btn_close_clicked();

	this->grf_file.setFileName(str);
	if (!this->grf_file.open(QIODevice::ReadWrite)) {
		QMessageBox::warning(this, tr("GrfBuilder"), tr("Could not load this file in read/write mode."), QMessageBox::Cancel, QMessageBox::Cancel);
		return;
	}
	this->grf = grf_new_by_fd(this->grf_file.handle(), true);
	grf_set_callback(this->grf, grf_callback_caller, (void *)this);
	this->grf = grf_load_from_new(this->grf);
	printf("RES=%p\n", this->grf);
}

void MainWindow::on_btn_close_clicked() {
	if (this->grf != NULL) {
		grf_free(this->grf);
		this->grf_file.close();
		this->grf = NULL;
		printf("close()\n");
	}
}

