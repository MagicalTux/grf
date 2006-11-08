#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QProgressDialog>
#include <QDir>
#include "MainWindow.h"

/* libgrf */
#include <libgrf.h>

#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	uint32_t version=grf_version();
	uint8_t major, minor, revision;
	major = (version >> 16) & 0xff;
	minor = (version >> 8) & 0xff;
	revision = version & 0xff;
	this->grf = NULL;
	ui.setupUi(this);
	((QDialog*)this)->setWindowTitle(tr("GrfBuilder v1.0 (libgrf v%1.%2.%3) by MagicalTux").arg(major).arg(minor).arg(revision));
}

bool MainWindow::progress_callback(void *grf, int pos, int max) {
	ui.progbar->setValue(pos * 100 / max);
	return true;
}

static bool grf_callback_caller(void *MW_, void *grf, int pos, int max) {
	MainWindow *MW = (MainWindow *)MW_;
	return MW->progress_callback(grf, pos, max);
}

unsigned int MainWindow::fillFilesTree(void *dir, QTreeWidget *parent) {
	void **list = grf_tree_list_node(dir);
	unsigned int total_size = 0;
	for(int i=0;list[i]!=NULL;i++) {
		unsigned int s;
		QTreeWidgetItem *__f = new QTreeWidgetItem(parent);
		__f->setText(0, QString::fromUtf8(euc_kr_to_utf8(grf_tree_get_name(list[i])))); // name
		if (grf_tree_is_dir(list[i])) {
			s=MainWindow::fillFilesTree(list[i], __f);
			total_size += s;
			__f->setText(1, QString("[%1]").arg(s));
		} else {
			void *f = grf_tree_get_file(list[i]);
			s = grf_file_get_size(f);
			total_size += s;
			__f->setText(1, QString("%1").arg(s));
		}
	}
	delete list; 
	return total_size;
}

unsigned int MainWindow::fillFilesTree(void *dir, QTreeWidgetItem *parent) {
	void **list = grf_tree_list_node(dir);
	unsigned int total_size = 0;
	for(int i=0;list[i]!=NULL;i++) {
		unsigned int s;
		QTreeWidgetItem *__f = new QTreeWidgetItem(parent);
		__f->setText(0, QString::fromUtf8(euc_kr_to_utf8(grf_tree_get_name(list[i])))); // name
		if (grf_tree_is_dir(list[i])) {
			s=MainWindow::fillFilesTree(list[i], __f);
			total_size += s;
			__f->setText(1, QString("[%1]").arg(s));
		} else {
			void *f = grf_tree_get_file(list[i]);
			s = grf_file_get_size(f);
			total_size += s;
			__f->setText(1, QString("%1").arg(s));
		}
	}
	delete list;
	return total_size;
}

void MainWindow::on_tab_sel_currentChanged(int idx) {
	if (idx == 0) return;
	if (this->grf == NULL) {
		ui.tab_sel->setCurrentIndex(0);
		return;
	}
	if (this->grf_has_tree) return;
	this->grf_has_tree = true;
	grf_create_tree(this->grf);
	// fill ui.view_filestree recursively
	MainWindow::fillFilesTree(grf_tree_get_root(this->grf), ui.view_filestree);
	// sort the total
	ui.view_filestree->sortItems(0, Qt::AscendingOrder);
}

void MainWindow::on_action_Open_triggered() {
	this->on_btn_open_clicked();
}

void MainWindow::on_btn_open_clicked() {
	QString str = QFileDialog::getOpenFileName(this, tr("Open File"),
			NULL, tr("GRF Files (*.grf *.gpf)"));
	void *f;
	int i;

	if (str.size() == 0) return;

	this->on_btn_close_clicked();

	this->grf_file.setFileName(str);
	if (!this->grf_file.open(QIODevice::ReadWrite)) {
		QMessageBox::warning(this, tr("GrfBuilder"), tr("Could not load this file in read/write mode."), QMessageBox::Cancel, QMessageBox::Cancel);
		return;
	}
	this->grf = grf_new_by_fd(this->grf_file.handle(), true);
	this->grf_has_tree = false;
	grf_set_callback(this->grf, grf_callback_caller, (void *)this);
	this->grf = grf_load_from_new(this->grf);
	if (this->grf == NULL) {
		QMessageBox::warning(this, tr("GrfBuilder"), tr("The selected file doesn't look like a valid GRF file."), QMessageBox::Cancel, QMessageBox::Cancel);
		return;
	}
	ui.tab_sel->setCurrentIndex(0);
	f = grf_get_file_first(this->grf);
	i=0;
	while(f != NULL) {
		QTreeWidgetItem *__item = new QTreeWidgetItem(ui.view_allfiles);
		__item->setText(0, QString("%1").arg(i++)); // idx
		__item->setText(1, QString("%1").arg(grf_file_get_storage_size(f))); // compsize
		__item->setText(2, QString("%1").arg(grf_file_get_size(f))); // realsize
		__item->setText(3, QString("%1").arg(grf_file_get_storage_pos(f))); // pos
		__item->setText(4, QString::fromUtf8(euc_kr_to_utf8(grf_file_get_filename(f)))); // name
//		__item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
		f = grf_get_file_next(f);
	}
	// enable buttons
	ui.btn_extract->setEnabled(false);
	ui.btn_extractall->setEnabled(true);
	ui.btn_repack->setEnabled(false);
	ui.btn_close->setEnabled(true);
	ui.btn_mergegrf->setEnabled(false);
	ui.btn_mergedir->setEnabled(false);
	// menuitems
	ui.action_Extract->setEnabled(false);
	ui.action_Extract_All->setEnabled(true);
	ui.actionMove_files->setEnabled(false);
	ui.actionDecrypt->setEnabled(false);
	ui.actionRecompress->setEnabled(false);
	ui.action_Close->setEnabled(true);
	ui.action_Merge_GRF->setEnabled(false);
	ui.actionMerge_Dir->setEnabled(false);
}

void MainWindow::on_action_Close_triggered() {
	this->on_btn_close_clicked();
}

void MainWindow::on_btn_close_clicked() {
	if (this->grf != NULL) {
		grf_free(this->grf);
		this->grf_file.close();
		this->grf = NULL;
		this->grf_has_tree = false;
		ui.progbar->setValue(0);
		ui.view_allfiles->clear();
		ui.view_filestree->clear();
		ui.tab_sel->setCurrentIndex(0);
		// disable buttons
		ui.btn_extract->setEnabled(false);
		ui.btn_extractall->setEnabled(false);
		ui.btn_repack->setEnabled(false);
		ui.btn_close->setEnabled(false);
		ui.btn_mergegrf->setEnabled(false);
		ui.btn_mergedir->setEnabled(false);
		// menuitems
		ui.action_Extract->setEnabled(false);
		ui.action_Extract_All->setEnabled(false);
		ui.actionMove_files->setEnabled(false);
		ui.actionDecrypt->setEnabled(false);
		ui.actionRecompress->setEnabled(false);
		ui.action_Close->setEnabled(false);
		ui.action_Merge_GRF->setEnabled(false);
		ui.actionMerge_Dir->setEnabled(false);
	}
}

void MainWindow::on_actionUnicode_triggered() {
	ui.actionUnicode->setChecked(true);
	ui.actionStandard->setChecked(false);
}

void MainWindow::on_action_Quit_triggered() {
//	QCoreApplication::exit();
	this->close();
}

void MainWindow::on_actionStandard_triggered() {
	ui.actionUnicode->setChecked(false);
	ui.actionStandard->setChecked(true);
}

void MainWindow::on_action_Extract_All_triggered() {
	this->on_btn_extractall_clicked();
}

void MainWindow::on_btn_extractall_clicked() {
	void *cur_file;
	int c=0;
	int n = 1;
	int x;
	if (this->grf == NULL) return;
	QProgressDialog prog(tr("Extraction in progress..."), tr("Cancel"), 0, grf_filecount(this->grf), this);
	prog.setWindowModality(Qt::WindowModal);
	x = grf_filecount(this->grf)/100;
	if (x<5) x=5;
	if (x>150) x=150;
	/* get files list */
	cur_file = grf_get_file_first(this->grf);
	while(cur_file != NULL) {
		c++;
		prog.setValue(c);
		if (prog.wasCanceled()) break;
		QString name(QString::fromUtf8(euc_kr_to_utf8(grf_file_get_filename(cur_file))));
		if (--n<=0) {
			n = x;
			prog.setLabelText(tr("Extracting file %1...").arg(name));
			QCoreApplication::processEvents();
		}
		if (ui.actionUnicode->isChecked()) {
			size_t size;
			size = grf_file_get_size(cur_file);
#ifdef __WIN32
			name.replace("/", "\\");
			QString dirname(name);
			dirname.resize(name.lastIndexOf("\\"));
#else
			name.replace("\\", "/");
			QString dirname(name);
			dirname.resize(name.lastIndexOf("/"));
#endif
			QDir foo(dirname);
			foo.mkpath(".");
			QFile output(name);
			if (!output.open(QIODevice::WriteOnly)) {
				cur_file = grf_get_file_next(cur_file);
				continue;
			}
			if (grf_file_put_contents_to_fd(cur_file, output.handle()) != size) {
				output.close();
				cur_file = grf_get_file_next(cur_file);
				continue;
			}
		} else {
			grf_put_contents_to_file(cur_file, grf_file_get_filename(cur_file));
		}
		cur_file = grf_get_file_next(cur_file);
	}
	prog.reset();
	prog.close();
}


