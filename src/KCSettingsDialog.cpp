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

#ifdef Q_OS_MAC
	// The application is always running only in the menu bar on OSX, so having
	// it *not* minimize to the tray would make no sense there
	ui->minimizeToTrayContainer->hide();
	
	// OSX also doesn't use Apply buttons; preferrably, everything should take
	// effect immediately, but that's hard to do with Qt (no setting bindings)
	ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
#endif

	// Pretend this changed just to update the enabledness of that container
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
	settings.setValue("notify", ui->notifyCheckbox->isChecked());
	settings.setValue("notifyRepairs", ui->notifyRepairsCheckbox->isChecked());
	settings.setValue("notifyConstruction", ui->notifyConstructionCheckbox->isChecked());
	settings.setValue("notifyExpedition", ui->notifyExpeditionCheckbox->isChecked());
	settings.setValue("notifyExpeditionReminder", ui->notifyExpeditionReminderCheckbox->isChecked());
	settings.setValue("notifyExpeditionReminderInterval", ui->notifyExpeditionReminderInterval->value() * 60);
	settings.setValue("notifyExpeditionReminderRepeat", ui->notifyExpeditionReminderRepeatCheckbox->isChecked());
	settings.setValue("notifyExpeditionReminderRepeatInterval", ui->notifyExpeditionReminderRepeatInterval->value() * 60);
	settings.setValue("notifyExpeditionReminderSuspend", ui->notifyExpeditionReminderSuspendCheckbox->isChecked());
	settings.setValue("notifyExpeditionReminderSuspendInterval", ui->notifyExpeditionReminderSuspendInterval->value() * 60);
	settings.sync();

	emit apply();
}

void KCSettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
		applySettings();
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
