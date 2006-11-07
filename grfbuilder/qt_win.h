#ifndef __QTWIN_H_FILE
#define __QTWIN_H_FILE

#include "ui_qt_win.h"

class MainWindow : public QWidget {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);

private slots:

private:
	Ui::main_window ui;
};

#endif

