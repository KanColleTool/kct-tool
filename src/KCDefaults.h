#ifndef KCDEFAULTS_H
#define KCDEFAULTS_H

#include "KCNotificationCenter.h"

static const bool kDefaultMinimizeToTray =  true;
static const bool kDefaultTranslation = true;
static const bool kDefaultNotify = true;
static const bool kDefaultNotifyRepairs = true;
static const bool kDefaultNotifyConstruction = true;
static const bool kDefaultNotifyExpedition = true;
static const bool kDefaultNotifyExpeditionReminder = false;
static const int  kDefaultNotifyExpeditionReminderInterval = 5*60;
static const bool kDefaultNotifyExpeditionRepeat = false;
static const int  kDefaultNotifyExpeditionRepeatInterval = 20*60;
static const bool kDefaultNotifyExpeditionSuspend = true;
static const int  kDefaultNotifyExpeditionSuspendInterval = 60*60;
static const int  kDefaultNotificationBackend = KCNotificationCenter::DefaultBackend;

#endif
