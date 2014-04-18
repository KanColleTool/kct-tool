#ifndef KCSETTINGSDIALOG_H
#define KCSETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

class QAbstractButton;
class KCMainWindow;
namespace Ui {
	class KCSettingsDialog;
}

class KCSettingsDialog : public QDialog {
	Q_OBJECT

public:
	explicit KCSettingsDialog(KCMainWindow *parent = 0, Qt::WindowFlags f = 0);
	virtual ~KCSettingsDialog();

public slots:
	virtual void accept();
	virtual void applySettings();

private slots:
	void on_buttonBox_clicked(QAbstractButton *button);
	void on_useNetworkCheckbox_toggled(bool checked);
	void on_autorefreshCheckbox_toggled(bool checked);
	void on_notifyCheckbox_toggled(bool checked);
	void on_notifyExpeditionReminderCheckbox_toggled(bool checked);
	void on_notifyExpeditionReminderRepeatCheckbox_toggled(bool checked);
	void on_notifyExpeditionReminderSuspendCheckbox_toggled(bool checked);

signals:
	void apply();

private:
	Ui::KCSettingsDialog *ui;
	QSettings settings;
};

#endif
