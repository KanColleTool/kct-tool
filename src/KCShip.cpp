#include "KCShip.h"
#include "KCShipType.h"

#include <QDebug>

KCShip::KCShip(const QVariantMap &data, int loadId, QObject *parent) :
	KCGameObject(parent) {
	loadFrom(data, loadId);
}

KCShip::~KCShip() {

}

void KCShip::loadFrom(const QVariantMap &data, int loadId) {
	Q_UNUSED(loadId)
	
	// Unlike KCShipType, I'm keeping only the values we actually use here.
	// The type object has most of this stuff already, so we're just keeping
	// the individual girl's values here.
	// (Including her name, because Sauyon is a mean person.)

	// Shared values
	extract(data, id, "api_id");
	extract(data, type, "api_ship_id");
	extract(data, level, "api_lv");
	extract(data, hp.cur, "api_nowhp");
	extract(data, hp.max, "api_maxhp");
	extract(data, equipment, 5, "api_slot");
	extract(data, _onslot, 5, "api_onslot");
	extract(data, _kyouka, 4, "api_kyouka");
	extract(data, fuel, "api_fuel");
	extract(data, ammo, "api_bull");
	extract(data, equipmentSlots, "api_slotnum");
	extract(data, repairCost.steel, "api_ndock_item", 0);
	extract(data, repairCost.fuel, "api_ndock_item", 1);
	extract(data, condition, "api_cond");
	extract(data, firepower, "api_karyoku", 0);
	extract(data, torpedo, "api_raisou", 0);
	extract(data, antiair, "api_taiku", 0);
	extract(data, armor, "api_soukou", 0);
	extract(data, evasion, "api_kaihi", 0);
	extract(data, antisub, "api_taisen", 0);
	extract(data, lineOfSight, "api_sakuteki", 0);
	extract(data, luck, "api_lucky", 0);
	
	int repairTime_;
	extract(data, repairTime_, "api_ndock_time");
	repairTime = QTime(0,0).addMSecs(repairTime_);
	
	if(data.contains("api_locked"))
	{
		extract(data, exp, "api_exp", 0);
		extract(data, heartLock, "api_locked");
	}
	else
	{
		extract(data, exp, "api_exp");
		extract(data, repairTimeStr, "api_ndock_time_str");
	}
}
