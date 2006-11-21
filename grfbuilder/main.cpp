#include <QApplication>
#include <QMessageBox>
#include <QLocale>
#include "MainWindow.h"
#include <libgrf.h>

#include <stdio.h>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	if (grf_version() < (0x117)) { // 0.1.23
		QMessageBox::warning(NULL, "GrfBuilder", "I need at least libgrf v0.1.23 to run. Make sure you have the last one installed.", QMessageBox::Cancel, QMessageBox::Cancel);
		return 1;
	}

	QString locale = QLocale::system().name();

	MainWindow MW;
	if (MW.translator.load(QString("grfbuilder_") + locale)) {
		app.installTranslator(&MW.translator);
	} else {
		printf("Locale initialisation failed!\n");
	}
 
	MW.RetranslateStrings();
	MW.show();
	return app.exec();
}

/*

    QTreeWidgetItem *__item = new QTreeWidgetItem(view_allfiles);
    __item->setText(0, QApplication::translate("main_window", "15156", 0, QApplication::UnicodeUTF8));
    __item->setText(1, QApplication::translate("main_window", "165645", 0, QApplication::UnicodeUTF8));
    __item->setText(2, QApplication::translate("main_window", "0", 0, QApplication::UnicodeUTF8));
    __item->setText(3, QApplication::translate("main_window", "data\\prontera.gat", 0, QApplication::UnicodeUTF8));
    tab_sel->setTabText(tab_sel->indexOf(grf_allfiles), QApplication::translate("main_window", "All files", 0, QApplication::UnicodeUTF8));
    view_filestree->headerItem()->setText(0, QApplication::translate("main_window", "Filename", 0, QApplication::UnicodeUTF8));
    view_filestree->headerItem()->setText(1, QApplication::translate("main_window", "File size", 0, QApplication::UnicodeUTF8));
    view_filestree->clear();

    QTreeWidgetItem *__item1 = new QTreeWidgetItem(view_filestree);
    __item1->setText(0, QApplication::translate("main_window", "data", 0, QApplication::UnicodeUTF8));
    __item1->setText(1, QApplication::translate("main_window", "", 0, QApplication::UnicodeUTF8));

    QTreeWidgetItem *__item2 = new QTreeWidgetItem(__item1);
    __item2->setText(0, QApplication::translate("main_window", "prontera.gat", 0, QApplication::UnicodeUTF8));
    __item2->setText(1, QApplication::translate("main_window", "156465", 0, QApplication::UnicodeUTF8));

    QTreeWidgetItem *__item3 = new QTreeWidgetItem(__item1);
    __item3->setText(0, QApplication::translate("main_window", "palette", 0, QApplication::UnicodeUTF8));
    __item3->setText(1, QApplication::translate("main_window", "", 0, QApplication::UnicodeUTF8));

    QTreeWidgetItem *__item4 = new QTreeWidgetItem(__item3);
    __item4->setText(0, QApplication::translate("main_window", "\355\225\234\352\265\255\354\226\264", 0, QApplication::UnicodeUTF8));
    __item4->setText(1, QApplication::translate("main_window", "", 0, QApplication::UnicodeUTF8));
    tab_sel->setTabText(tab_sel->indexOf(grf_treeview), QApplication::translate("main_window", "Tree view", 0, QApplication::UnicodeUTF8));

*/
