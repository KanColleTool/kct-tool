#include "KCMainWindow.h"
#include <QApplication>
#include <QDir>
#include <QDebug>
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

	QStringList args = QCoreApplication::arguments();

	// We want to stay in the tray, not quit when the window goes away
	if(!args.contains("--exit-on-close"))
		a.setQuitOnLastWindowClosed(false);

	// Create the window, which owns the tray icon, but don't show it!
	KCMainWindow w;
	if(!w.init()) return 0;
	w.show(); // Not in the final version!

	// Aaand... start the main loop
	return a.exec();
}