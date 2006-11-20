#ifndef __QTWIN_H_FILE
#define __QTWIN_H_FILE

#define GRFBUILDER_VERSION_MAJOR 0
#define GRFBUILDER_VERSION_MINOR 1
#define GRFBUILDER_VERSION_REVISION 22

#include <QFile>
#include <QMainWindow>
#include <QTranslator>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	bool progress_callback(void *, int pos, int max);
	QTranslator translator;
	void RetranslateStrings();

protected:
	void closeEvent(QCloseEvent *);

private slots:
	void on_btn_open_clicked();
	void on_btn_close_clicked();
	void on_btn_extractall_clicked();
	void on_btn_repack_clicked();
	void on_listFilter_currentIndexChanged(QString);

	void on_view_allfiles_customContextMenuRequested(const QPoint);
	void on_viewSearch_customContextMenuRequested(const QPoint);

	void on_tab_sel_currentChanged(int);

	void on_view_allfiles_doubleClicked(const QModelIndex);
	void on_view_filestree_doubleClicked(const QModelIndex);
	void on_viewSearch_doubleClicked(const QModelIndex);

	// menu
	void on_action_Open_triggered();
	void on_action_Extract_All_triggered();
	void on_action_Close_triggered();
	void on_action_Quit_triggered();
	void on_actionAbout_triggered();
	// compression
	void on_actionC0_triggered();
	void on_actionC1_triggered();
	void on_actionC2_triggered();
	void on_actionC3_triggered();
	void on_actionC4_triggered();
	void on_actionC5_triggered();
	void on_actionC6_triggered();
	void on_actionC7_triggered();
	void on_actionC8_triggered();
	void on_actionC9_triggered();

	void on_actionMove_files_triggered();
	void on_actionDecrypt_triggered();
	void on_actionRecompress_triggered();

	void on_actionEn_triggered();
	void on_actionFr_triggered();
	void on_actionDe_triggered();
	void on_actionEs_triggered();

	void on_actionUnicode_triggered();
	void on_actionStandard_triggered();

private:
	Ui::MainWindow ui;
	QFile grf_file;
	QDialog *image_viewer;
	QString last_search;
	int repack_type;
	int compression_level;
	unsigned int fillFilesTree(void *, QTreeWidget *);
	unsigned int fillFilesTree(void *, QTreeWidgetItem *);
	void *grf;
	bool grf_has_tree;
	void do_mkdir(QString *);
	void do_display_wav(void *);
	void DoUpdateFilter(QString);
	void doOpenFileById(int);
	QString showSizeAsString(unsigned int);
	void setCompressionLevel(int);
	void setRepackType(int);
};

#endif

