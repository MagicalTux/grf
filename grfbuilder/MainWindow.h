#ifndef __QTWIN_H_FILE
#define __QTWIN_H_FILE

#include <QFile>
#include <QMainWindow>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	bool progress_callback(void *, int pos, int max);

private slots:
	void on_btn_open_clicked();
	void on_btn_close_clicked();
	void on_tab_sel_currentChanged(int);

private:
	Ui::MainWindow ui;
	QFile grf_file;
	void fillFilesTree(void *, QTreeWidget *);
	void fillFilesTree(void *, QTreeWidgetItem *);
	void *grf;
	bool grf_has_tree;
};

#endif
