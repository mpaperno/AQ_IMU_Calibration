#include "config.h"
#include "MainWindow.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// Set application name
	a.setApplicationName(APP_NAME);
	a.setApplicationVersion(APP_VERSION_TXT);
	a.setOrganizationName(APP_ORG);
	a.setOrganizationDomain(APP_DOMAIN);

	// Set settings format
	QSettings::setDefaultFormat(QSettings::IniFormat);

	MainWindow w;
	w.show();

	return a.exec();
}
