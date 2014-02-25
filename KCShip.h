#ifndef KCSHIP_H
#define KCSHIP_H

#include <QObject>
#include <QVariant>
#include <QMap>

class KCShip : public QObject
{
public:
	KCShip(QObject *parent = 0);
	KCShip(QVariantMap data = QVariantMap(), QObject *parent = 0, bool use2=false);
	virtual ~KCShip();

	void loadFrom(const QVariantMap &data);
	void loadFrom2(const QVariantMap &data);

	int master;

	int id, admiral;
	int level, exp;

	struct { int cur, max; } hpBase, hp;
	struct { int cur, max; } firepower;
	struct { int cur, max; } torpedo;
	struct { int cur, max; } antiair;
	struct { int cur, max; } antisub;
	struct { int cur, max; } armor;
	struct { int cur, max; } evasion;
	struct { int cur, max; } lineOfSight;
	struct { int cur, max; } luck;

	int ammo, fuel;
	int condition;

	int equipment[5];
	int equipmentSlots;

	struct { int steel, fuel; } repairCost;
	int repairTime;	// In Seconds
	QString repairTimeStr;

	// Unknown values
	int _houm[2], _raim[2];
	int _kyouka[4];
	int _onslot[5];
};

#endif
