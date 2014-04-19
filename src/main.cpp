#include <QApplication>
#include <QLibraryInfo>
#include <QDir>
#include <QDebug>
#include "KCMainWindow.h"
#include "version.h"

int main(int argc, char *argv[])
{
	// Make an application!
	QApplication a(argc, argv);
#ifdef Q_OS_WIN
	QDir::setCurrent(QCoreApplication::applicationDirPath());
#endif

	// Setup some version stuff; this lets us use the default QSettings
	// constructor anywhere, without having to specify this over and over
	QCoreApplication::setApplicationName("KanColleTool");
	QCoreApplication::setApplicationVersion(KCT_VERSION);
	QCoreApplication::setOrganizationName("KanColleTool");

	qDebug() << "Locale:" << QLocale::system().name();

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	a.installTranslator(&qtTranslator);

	QTranslator translator;
	translator.load("kancolletool_" + QLocale::system().name());
	a.installTranslator(&translator);

	// We want to stay in the tray, not quit when the window goes away
	a.setQuitOnLastWindowClosed(false);

	// Load commandline arguments
	QStringList args = QCoreApplication::arguments();
	{
		// --exit-on-close = exit when the window is closed; for debugging
		if(args.contains("--exit-on-close"))
			a.setQuitOnLastWindowClosed(true);
	}

	// Create the window
	KCMainWindow w;
	if(!w.init())
		return 1;

	// Aaand... start!
	w.show();
	return a.exec();
}
