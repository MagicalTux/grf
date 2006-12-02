#ifndef __QTWIN_H_FILE
#define __QTWIN_H_FILE

#define GRFBUILDER_VERSION_MAJOR 0
#define GRFBUILDER_VERSION_MINOR 1
#define GRFBUILDER_VERSION_REVISION 29

#define L_TRADITIONAL_CHINESE_NAME "\xe7\xb9\x81\xe9\xab\x94\xe4\xb8\xad\xe6\x96\x87 (Trad. Chinese)"
#define L_TRADITIONAL_CHINESE_LOC "zh_TW"
#define L_SIMPLIFIED_CHINESE_NAME "\xe7\xb0\xa1\xe9\xab\x94\xe4\xb8\xad\xe6\x96\x87 (Simp. Chinese)"
#define L_SIMPLIFIED_CHINESE_LOC "zh_CN"
#define L_ENGLISH_NAME "English"
#define L_ENGLISH_LOC "en"
#define L_FRENCH_NAME "Fran\303\247ais"
#define L_FRENCH_LOC "fr"
#define L_GERMAN_NAME "Deutsch"
#define L_GERMAN_LOC "de"
#define L_SPANISH_NAME "Espa\xc3\xb1ol"
#define L_SPANISH_LOC "es"

#include <QFile>
#include <QMainWindow>
#include <QTranslator>
#include <QProgressDialog>
#include <QSettings>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	bool progress_callback(void *, int pos, int max);
	bool repack_progress_callback(void *grf, int pos, int max, const char *filename, QProgressDialog *prog);
	bool merge_progress_callback(void *grf, int pos, int max, const char *filename, QProgressDialog *prog);
	QTranslator translator;
	void RetranslateStrings();

protected:
	void closeEvent(QCloseEvent *);

private slots:
	void on_btn_new_clicked();
	void on_btn_open_clicked();
	void on_btn_close_clicked();
	void on_btn_extract_clicked();
	void on_btn_delete_clicked();
	void on_btn_extractall_clicked();
	void on_btn_repack_clicked();
	void on_btn_mergegrf_clicked();
	void on_btn_mergedir_clicked();
	void on_listFilter_currentIndexChanged(QString);

	void on_view_allfiles_customContextMenuRequested(const QPoint);
	void on_viewSearch_customContextMenuRequested(const QPoint);

	void on_tab_sel_currentChanged(int);

	void on_view_allfiles_doubleClicked(const QModelIndex);
	void on_view_filestree_doubleClicked(const QModelIndex);
	void on_viewSearch_doubleClicked(const QModelIndex);

	// locales
	void myLocaleChange();
	// menu
	void on_action_New_triggered();
	void on_action_Open_triggered();
	void on_action_Extract_triggered();
	void on_actionDelete_triggered();
	void on_action_Extract_All_triggered();
	void on_actionRepack_triggered();
	void on_action_Merge_GRF_triggered();
	void on_actionMerge_Dir_triggered();
	void on_action_Close_triggered();
	void on_action_Quit_triggered();
	void on_actionAbout_triggered();
	// Recent files
	void myOpenRecent();
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

	void on_actionUnicode_triggered();
	void on_actionStandard_triggered();

private:
	Ui::MainWindow ui;
	QSettings *settings;
	QFile grf_file;
	QDialog *image_viewer;
	QString last_search;
	QMenu *recent_files_menu;
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
	void RefreshAfterLoad();
	bool do_recurse_dirscan(QList <struct files_list *> *, QString, QString);
	QList<QTreeWidgetItem *> myTreeViewRecuriveSearch(QList<QTreeWidgetItem *>*);
	void refreshRecentList();
	void setRecentFile(QString);
	void doLoadFile(QString);
};

#endif

