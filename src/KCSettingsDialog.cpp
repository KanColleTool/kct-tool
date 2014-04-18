#include "KCSettingsDialog.h"
#include "ui_KCSettingsDialog.h"
#include "KCMainWindow.h"
#include "KCDefaults.h"

KCSettingsDialog::KCSettingsDialog(KCMainWindow *parent, Qt::WindowFlags f):
	QDialog(parent, f),
	ui(new Ui::KCSettingsDialog)
{
	ui->setupUi(this);

	ui->minimizeToTrayCheckbox->setChecked(
		settings.value("minimizeToTray", kDefaultMinimizeToTray).toBool());
	ui->translationCheckbox->setChecked(
		settings.value("toolTranslation", kDefaultTranslation).toBool());
	ui->livestreamCheckbox->setChecked(
		settings.value("livestream", kDefaultLivestream).toBool());
	ui->useNetworkCheckbox->setChecked(
		settings.value("usenetwork", kDefaultUseNetwork).toBool());
	ui->autorefreshCheckbox->setChecked(
		settings.value("autorefresh", kDefaultAutorefresh).toBool());
	ui->autorefreshInterval->setValue(
		settings.value("autorefreshInterval", kDefaultAutorefreshInterval).toInt() / 60);
	ui->notifyCheckbox->setChecked(
		settings.value("notify", kDefaultNotify).toBool());
	ui->notifyRepairsCheckbox->setChecked(
		settings.value("notifyRepairs", kDefaultNotifyRepairs).toBool());
	ui->notifyConstructionCheckbox->setChecked(
		settings.value("notifyConstruction", kDefaultNotifyConstruction).toBool());
	ui->notifyExpeditionCheckbox->setChecked(
		settings.value("notifyExpedition", kDefaultNotifyExpedition).toBool());
	ui->notifyExpeditionReminderCheckbox->setChecked(
		settings.value("notifyExpeditionReminder", kDefaultNotifyExpeditionReminder).toBool());
	ui->notifyExpeditionReminderInterval->setValue(
		settings.value("notifyExpeditionReminderInterval", kDefaultNotifyExpeditionReminderInterval).toInt() / 60);
	ui->notifyExpeditionReminderRepeatCheckbox->setChecked(
		settings.value("notifyExpeditionReminderRepeat", kDefaultNotifyExpeditionRepeat).toBool());
	ui->notifyExpeditionReminderRepeatInterval->setValue(
		settings.value("notifyExpeditionReminderRepeatInterval", kDefaultNotifyExpeditionRepeatInterval).toInt() / 60);
	ui->notifyExpeditionReminderSuspendCheckbox->setChecked(
		settings.value("notifyExpeditionReminderSuspend", kDefaultNotifyExpeditionSuspend).toBool());
	ui->notifyExpeditionReminderSuspendInterval->setValue(
		settings.value("notifyExpeditionReminderSuspendInterval", kDefaultNotifyExpeditionSuspendInterval).toInt() / 60);

	// Disable the "?"-button on Windows
	this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

	// This whole thing makes no sense on OSX, so just hide the whole box there
	// The application is always running (only) in the menu bar there
#ifdef Q_OS_MAC
	ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	ui->minimizeToTrayContainer->hide();
#endif

	// Pretend this changed just to update the enabledness of that container
	this->on_useNetworkCheckbox_toggled(ui->useNetworkCheckbox->isChecked());
	this->on_autorefreshCheckbox_toggled(ui->autorefreshCheckbox->isChecked());
	this->on_notifyCheckbox_toggled(ui->notifyCheckbox->isChecked());
	this->on_notifyExpeditionReminderCheckbox_toggled(ui->notifyExpeditionReminderCheckbox->isChecked());
	this->on_notifyExpeditionReminderRepeatCheckbox_toggled(ui->notifyExpeditionReminderRepeatCheckbox->isChecked());
	this->on_notifyExpeditionReminderSuspendCheckbox_toggled(ui->notifyExpeditionReminderSuspendCheckbox->isChecked());

	// Autoadjust the size to fit, because that's easier than trying to make it
	// look everywhere good manually. Takes into account font size differences
	// and stuff too, so yeah.
	this->adjustSize();
	this->setFixedSize(this->size());
}

KCSettingsDialog::~KCSettingsDialog()
{
}

void KCSettingsDialog::accept()
{
	this->applySettings();

	QDialog::accept();
}

void KCSettingsDialog::applySettings()
{
	settings.setValue("minimizeToTray", ui->minimizeToTrayCheckbox->isChecked());
	settings.setValue("toolTranslation", ui->translationCheckbox->isChecked());
	settings.setValue("livestream", ui->livestreamCheckbox->isChecked());
	settings.setValue("usenetwork", ui->useNetworkCheckbox->isChecked());
	settings.setValue("autorefresh", ui->autorefreshCheckbox->isChecked());
	settings.setValue("autorefreshInterval", ui->autorefreshInterval->value() * 60);
	settings.setValue("notify", ui->notifyCheckbox->isChecked());
	settings.setValue("notifyRepairs", ui->notifyRepairsCheckbox->isChecked());
	settings.setValue("notifyConstruction", ui->notifyConstructionCheckbox->isChecked());
	settings.setValue("notifyExpedition", ui->notifyExpeditionCheckbox->isChecked());
	settings.setValue("notifyExpeditionReminder", ui->notifyExpeditionReminderCheckbox->isChecked());
	settings.setValue("notifyExpeditionReminderInterval", ui->notifyExpeditionReminderInterval->value());
	settings.setValue("notifyExpeditionReminderRepeat", ui->notifyExpeditionReminderRepeatCheckbox->isChecked());
	settings.setValue("notifyExpeditionReminderRepeatInterval", ui->notifyExpeditionReminderRepeatInterval->value());
	settings.setValue("notifyExpeditionReminderSuspend", ui->notifyExpeditionReminderSuspendCheckbox->isChecked());
	settings.setValue("notifyExpeditionReminderSuspendInterval", ui->notifyExpeditionReminderSuspendInterval->value());

	settings.sync();

	emit apply();
}

void KCSettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
		applySettings();
}

void KCSettingsDialog::on_useNetworkCheckbox_toggled(bool checked)
{
	ui->autorefreshContainer->setEnabled(checked);
}

void KCSettingsDialog::on_autorefreshCheckbox_toggled(bool checked)
{
	ui->autorefreshInterval->setEnabled(checked);
}

void KCSettingsDialog::on_notifyCheckbox_toggled(bool checked)
{
	ui->notifyContainer->setEnabled(checked);
}

void KCSettingsDialog::on_notifyExpeditionReminderCheckbox_toggled(bool checked)
{
	ui->notifyExpeditionReminderContainer->setEnabled(checked);
	ui->notifyExpeditionReminderInterval->setEnabled(checked);
}

void KCSettingsDialog::on_notifyExpeditionReminderRepeatCheckbox_toggled(bool checked)
{
	ui->notifyExpeditionReminderRepeatContainer->setEnabled(checked);
	ui->notifyExpeditionReminderRepeatInterval->setEnabled(checked);
}

void KCSettingsDialog::on_notifyExpeditionReminderSuspendCheckbox_toggled(bool checked)
{
	ui->notifyExpeditionReminderSuspendInterval->setEnabled(checked);
}
