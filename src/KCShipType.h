#ifndef KCSHIPMASTER_H
#define KCSHIPMASTER_H

#include "KCGameObject.h"

#include <QObject>
#include <QVariant>

class KCShipType : public KCGameObject {
	Q_OBJECT

public:
	KCShipType(const QVariantMap &data, int loadId = 0, QObject *parent = 0);
	virtual ~KCShipType();

	virtual void loadFrom(const QVariantMap &data, int loadId) override;

	static const int expToLv[150];

	QString name, reading;
	QString description;
	int id, cardno, sclass, ctype, cindex;
	int rarity, buildTime;
	int maxAmmo, maxFuel;

	bool encountered;
	QString getMessage;

	struct { int level, into, ammo, fuel; } remodel;
	struct { int fuel, ammo, steel, baux; } dismantle;
	struct { int firepower, torpedo, antiair, armor; } modernization;

	struct { int base, max; } hp;
	struct { int base, max; } firepower;
	struct { int base, max; } torpedo;
	struct { int base, max; } antiair;
	struct { int base, max; } armor;
	struct { int base, max; } antisub;
	struct { int base, max; } evasion;
	struct { int base, max; } lineOfSight;
	struct { int base, max; } luck; // Luck can't be boosted anyways (and the max is stupidly high across the board...)
	int range;
	int speed;

	int planeCapacity[4];
	int equipmentSlots;

	int voicef;
};

#endif
