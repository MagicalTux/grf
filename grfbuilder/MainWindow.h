#ifndef __QTWIN_H_FILE
#define __QTWIN_H_FILE

#define GRFBUILDER_VERSION_MAJOR 0
#define GRFBUILDER_VERSION_MINOR 1
#define GRFBUILDER_VERSION_REVISION 19

#include <QFile>
#include <QMainWindow>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	bool progress_callback(void *, int pos, int max);

protected:
	void closeEvent(QCloseEvent *);

private slots:
	void on_btn_open_clicked();
	void on_btn_close_clicked();
	void on_btn_extractall_clicked();

	void on_view_allfiles_customContextMenuRequested(const QPoint);

	void on_tab_sel_currentChanged(int);

	void on_view_allfiles_doubleClicked(const QModelIndex);

	// menu
	void on_action_Open_triggered();
	void on_action_Extract_All_triggered();
	void on_action_Close_triggered();
	void on_action_Quit_triggered();
	void on_actionAbout_triggered();

	void on_actionUnicode_triggered();
	void on_actionStandard_triggered();

private:
	Ui::MainWindow ui;
	QFile grf_file;
	QDialog *image_viewer;
	unsigned int fillFilesTree(void *, QTreeWidget *);
	unsigned int fillFilesTree(void *, QTreeWidgetItem *);
	void *grf;
	bool grf_has_tree;
	void do_mkdir(QString *);
	void do_display_wav(void *);
};

#endif

