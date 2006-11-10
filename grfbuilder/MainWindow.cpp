#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QProgressDialog>
#include <QDir>
#include <QImage>
#include "MainWindow.h"
#ifdef __WIN32
#include <windows.h> /* Sleep() */
#else
#include <unistd.h> /* usleep() */
#endif

/* libgrf */
#include <libgrf.h>

#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	this->grf = NULL;
	this->image_viewer = NULL;
	ui.setupUi(this);
	ui.view_allfiles->setColumnHidden(0, true);
}

void MainWindow::RetranslateStrings() {
	uint32_t version=grf_version();
	uint8_t major, minor, revision;
	major = (version >> 16) & 0xff;
	minor = (version >> 8) & 0xff;
	revision = version & 0xff;
	ui.retranslateUi(this);
	((QDialog*)this)->setWindowTitle(tr("GrfBuilder v%1.%2.%3 (libgrf v%4.%5.%6) by MagicalTux"
		).arg(GRFBUILDER_VERSION_MAJOR).arg(GRFBUILDER_VERSION_MINOR).arg(GRFBUILDER_VERSION_REVISION).arg(major).arg(minor).arg(revision));
}

bool MainWindow::progress_callback(void *grf, int pos, int max) {
	ui.progbar->setValue(pos * 100 / max);
	return true;
}

static bool grf_callback_caller(void *MW_, void *grf, int pos, int max) {
	MainWindow *MW = (MainWindow *)MW_;
	return MW->progress_callback(grf, pos, max);
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
	while(f != NULL) {
		QTreeWidgetItem *__item = new QTreeWidgetItem(ui.view_allfiles);
		__item->setText(0, QString("%1").arg(grf_file_get_id(f)));
		__item->setText(1, QString("%1").arg(grf_file_get_storage_size(f))); // compsize
		__item->setText(2, QString("%1").arg(grf_file_get_size(f))); // realsize
		__item->setText(3, QString("%1").arg(grf_file_get_storage_pos(f))); // pos
		__item->setText(4, QString::fromUtf8(euc_kr_to_utf8(grf_file_get_filename(f)))); // name
//		__item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
		f = grf_get_file_next(f);
	}
	ui.view_allfiles->sortItems(4, Qt::AscendingOrder);
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

void MainWindow::closeEvent(QCloseEvent *ev) {
	if (this->image_viewer) delete this->image_viewer;
	this->on_btn_close_clicked();
	ev->accept();
}

void MainWindow::on_actionAbout_triggered() {
	uint32_t version=grf_version();
	uint8_t major, minor, revision;
	major = (version >> 16) & 0xff;
	minor = (version >> 8) & 0xff;
	revision = version & 0xff;
	QMessageBox::information(this, tr("GrfBuilder"),
		tr(
			"<p align=\"center\"><b>GrfBuilder v%1.%2.%3 by MagicalTux</b></p>"
			"<p align=\"left\">Linked against libgrf v%4.%5.%6 (also by MagicalTux)<br />"
			"This tool is designed to allow easy read and write access to GRF files.<br />"
			"This was developped for the sole purpose of demonstrating that Gravity need better developpers."
			"</p>"
			"<p align=\"left\">You can contact MagicalTux on <a href=\"http://ookoo.org/cgi-bin/cgi-irc/irc.cgi\">irc://irc.ookoo.org/ooKoo</a></p>"
		).arg(GRFBUILDER_VERSION_MAJOR).arg(GRFBUILDER_VERSION_MINOR).arg(GRFBUILDER_VERSION_REVISION).arg(major).arg(minor).arg(revision)
	);
}

void MainWindow::on_view_allfiles_customContextMenuRequested(const QPoint point) {
	QMenu menu(this);
	menu.addAction(ui.action_Extract_All);
	menu.exec(ui.view_allfiles->viewport()->mapToGlobal(point));
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
	if (this->grf == NULL) return;
	QProgressDialog prog(tr("Extraction in progress..."), tr("Cancel"), 0, grf_filecount(this->grf), this);
	prog.setWindowModality(Qt::WindowModal);
	/* get files list */
	cur_file = grf_get_file_first(this->grf);
	while(cur_file != NULL) {
		c++;
		prog.setValue(c);
		if (prog.wasCanceled()) break;
		QString name(QString::fromUtf8(euc_kr_to_utf8(grf_file_get_filename(cur_file))));
		n -= grf_file_get_size(cur_file);
		if (n<=0) {
			n = 5000000;
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

void MainWindow::do_display_wav(void *f) {
	QFile tmp;
#ifndef __WIN32
	if (!QSound::isAvailable()) {
		QMessageBox::warning(this, tr("GrfBuilder"), tr("Your computer has no audio support. Please make sure you have an audio device available and retry."), QMessageBox::Cancel, QMessageBox::Cancel);
		return;
	}
	QMessageBox mb(tr("GrfBuilder"), tr("Currently playing file `%1'. Press \"Ok\" to stop.").arg(QString::fromUtf8(euc_kr_to_utf8(grf_file_get_filename(f)))), QMessageBox::Information, 0, 0, 0, this);
	mb.show();
#endif
	for(int i=0;1;i++) {
		tmp.setFileName(QString("%1_tmp.wav").arg(i));
		if (!tmp.exists()) break;
	}
	if (!tmp.open(QIODevice::WriteOnly)) return;
	if (grf_file_put_contents_to_fd(f, tmp.handle()) != grf_file_get_size(f)) {
		tmp.remove();
		return;
	}
	tmp.close();
	QSound snd(tmp.fileName());
	snd.play();
#ifndef __WIN32
	while(1) {
		QCoreApplication::processEvents();
		if (snd.isFinished()) break;
		if (!mb.isVisible()) break;
		usleep(10000);
	}
	snd.stop();
#endif
	tmp.remove();
}

void MainWindow::on_view_allfiles_doubleClicked(const QModelIndex idx) {
	// item = ui.view_allfiles->currentItem()
	// item = ui.view_allfiles->topLevelItem(idx.row())
	int id=ui.view_allfiles->topLevelItem(idx.row())->text(0).toInt();
//	printf("x=%d\n", ui.view_allfiles->topLevelItem(idx.row())->text(0).toInt());
	void *f = grf_get_file_by_id(this->grf, id);
	bool is_image = false;
	if (ui.view_allfiles->topLevelItem(idx.row())->text(4).toLower().endsWith(".wav")) return this->do_display_wav(grf_get_file_by_id(this->grf, id));
	if (
			(!ui.view_allfiles->topLevelItem(idx.row())->text(4).toLower().endsWith(".bmp"))
		&&	(!ui.view_allfiles->topLevelItem(idx.row())->text(4).toLower().endsWith(".jpg"))
		&&	(!ui.view_allfiles->topLevelItem(idx.row())->text(4).toLower().endsWith(".jpeg"))
		&&	(!ui.view_allfiles->topLevelItem(idx.row())->text(4).toLower().endsWith(".png"))
		&&	(!ui.view_allfiles->topLevelItem(idx.row())->text(4).toLower().endsWith(".gif"))
		&&	(!ui.view_allfiles->topLevelItem(idx.row())->text(4).toLower().endsWith(".gat"))
		) return;
	QByteArray im_data(grf_file_get_size(f), 0);
	if (grf_file_get_contents(f, im_data.data()) != grf_file_get_size(f)) return;
	QImage im;
	if (ui.view_allfiles->topLevelItem(idx.row())->text(4).toLower().endsWith(".gat")) {
		const char *data = im_data.constData();
		int sx = *(int*)(data+6);
		int sy = *(int*)(data+10);
		im = QImage(sx, sy, QImage::Format_Indexed8);
		im.setNumColors(2);
		im.setColor(0, qRgb(0,0,0));
		im.setColor(1, qRgb(255,255,255));
		for(int y=0;y<sy;y++) {
			for(int x=0;x<sx;x++) {
				int type = *(int*)(data + ((y*sx + x) * 20+14+16));
				im.setPixel(x,sy-y-1, type?0:1);
			}
		}
	} else {
		if (!im.loadFromData(im_data)) return;
		is_image = true;
	}

	if (this->image_viewer) delete this->image_viewer;

	QLabel *label;
	QHBoxLayout *hboxLayout;
	QVBoxLayout *vboxLayout;
	QSpacerItem *spacerItem;
	QPushButton *closeButton;
	QDialog *Dialog = new QDialog;

	Dialog->setObjectName(QString::fromUtf8("Dialog"));
	Dialog->resize(QSize(16, 16).expandedTo(Dialog->minimumSizeHint()));
	vboxLayout = new QVBoxLayout(Dialog);
	vboxLayout->setSpacing(6);
	vboxLayout->setMargin(9);
	vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
	label = new QLabel(Dialog);
	label->setObjectName(QString::fromUtf8("label"));

	vboxLayout->addWidget(label);

	hboxLayout = new QHBoxLayout();
	hboxLayout->setSpacing(6);
	hboxLayout->setMargin(0);
	hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
	spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

	hboxLayout->addItem(spacerItem);

	closeButton = new QPushButton(Dialog);
	closeButton->setObjectName(QString::fromUtf8("closeButton"));

	hboxLayout->addWidget(closeButton);

	vboxLayout->addLayout(hboxLayout);

	QObject::connect(closeButton, SIGNAL(clicked()), Dialog, SLOT(reject()));

	Dialog->setWindowTitle(QApplication::translate("Dialog", "Image Preview", 0, QApplication::UnicodeUTF8));
	closeButton->setText(QApplication::translate("Dialog", "Close", 0, QApplication::UnicodeUTF8));
	QPixmap im_pixmap(QPixmap::fromImage(im));
	if ((ui.actionImages_Transparency->isChecked()) && (is_image)) {
		im_pixmap.setMask(QBitmap::fromImage(im.createHeuristicMask()));
	}
	label->setPixmap(im_pixmap);

	Dialog->show();
	this->image_viewer = Dialog;
};

void MainWindow::on_actionEn_triggered() {
	QCoreApplication::removeTranslator(&this->translator);
	this->RetranslateStrings();
}

void MainWindow::on_actionFr_triggered() {
	this->on_actionEn_triggered();
	if (this->translator.load("grfbuilder_fr")) {
		QCoreApplication::installTranslator(&this->translator);
		this->RetranslateStrings();
	}
}


