#ifndef KCSHIP_H
#define KCSHIP_H

#include "KCGameObject.h"

#include <QObject>
#include <QVariant>
#include <QMap>

class KCShip : public KCGameObject {
	Q_OBJECT

public:
	KCShip(const QVariantMap &data, int loadId = 0, QObject *parent = 0);
	virtual ~KCShip();

	virtual void loadFrom(const QVariantMap &data, int loadId) override;

	int type;

	int id;
	int level, exp;

	struct { int cur, max; } hp;
	int armor, evasion;
	int firepower, torpedo, antiair, antisub;
	int lineOfSight;
	int luck;

	int ammo, fuel;
	int condition;

	int equipment[5];
	int equipmentSlots;

	struct { int steel, fuel; } repairCost;
	QTime repairTime;
	QString repairTimeStr;

	bool heartLock;

	// Unknown values
	int _kyouka[4];
	int _onslot[5];
};

#endif
