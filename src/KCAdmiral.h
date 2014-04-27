#ifndef KCADMIRAL_H
#define KCADMIRAL_H

#include "KCGameObject.h"

class KCAdmiral : public KCGameObject
{
public:
	KCAdmiral(const QVariantMap &data, int loadId = 0, QObject *parent = 0);
	virtual ~KCAdmiral();
	
	virtual void loadFrom(const QVariantMap &data, int loadId = 0) override;
	
	int id;
	QString nickname;
	
	int playtime;
	QDateTime startTime;
	
	int level, rank, experience;
	int maxShips, maxEquipment, maxFurniture;
	int fleetsCount, cdockCount, rdockCount;
	struct { int won, lost; } sorties, expeditions, pvp, pvpChallenged;
	
	int furniture[6];	// TODO: Make this a struct; I dunno which order these are in though
	int fcoins;
	
	int activeFlag, firstFlag, tutorial, tutorialProgress;
};

#endif
