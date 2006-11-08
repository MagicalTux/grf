#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
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

void MainWindow::fillFilesTree(void *dir, QTreeWidget *parent) {
	void **list = grf_tree_list_node(dir);
	for(int i=0;list[i]!=NULL;i++) {
		QTreeWidgetItem *__f = new QTreeWidgetItem(parent);
		__f->setText(0, QString::fromUtf8(euc_kr_to_utf8(grf_tree_get_name(list[i])))); // name
		if (grf_tree_is_dir(list[i])) {
			__f->setText(1, tr("[dir]"));
			MainWindow::fillFilesTree(list[i], __f);
		} else {
			void *f = grf_tree_get_file(list[i]);
			__f->setText(1, QString("%1").arg(grf_file_get_size(f)));
		}
	}
	delete list;
}

void MainWindow::fillFilesTree(void *dir, QTreeWidgetItem *parent) {
	void **list = grf_tree_list_node(dir);
	for(int i=0;list[i]!=NULL;i++) {
		QTreeWidgetItem *__f = new QTreeWidgetItem(parent);
		__f->setText(0, QString::fromUtf8(euc_kr_to_utf8(grf_tree_get_name(list[i])))); // name
		if (grf_tree_is_dir(list[i])) {
			__f->setText(1, tr("[dir]"));
			MainWindow::fillFilesTree(list[i], __f);
		} else {
			void *f = grf_tree_get_file(list[i]);
			__f->setText(1, QString("%1").arg(grf_file_get_size(f)));
		}
	}
	delete list;
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
		f = grf_get_file_next(f);
	}
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
//		QList<QTreeWidgetItem *> list = ui.view_allfiles->findItems("", Qt::MatchContains | Qt::MatchRecursive);
//		printf("list.size()=%d\n", list.size());
//		for(int i=list.size()-1;i>=0;i--) {
//			QTreeWidgetItem *x = (QTreeWidgetItem *)list.at(i);
//			delete x;
//		}
	}
}

