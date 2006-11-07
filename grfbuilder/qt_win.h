#ifndef __QTWIN_H_FILE
#define __QTWIN_H_FILE

#include <QFile>

#include "ui_qt_win.h"

class MainWindow : public QWidget {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	bool MainWindow::progress_callback(void *, int pos, int max);

private slots:
	void on_btn_open_clicked();
	void on_btn_close_clicked();

private:
	Ui::main_window ui;
	QFile grf_file;
	void *grf;
};

#endif

