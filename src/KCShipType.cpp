#include "KCShipType.h"

const int KCShipType::expToLv[150] = {
	0,       100,     300,     600,     1000,    1500,    2100,    2800,    3600,    4500,
	5500,    6600,    7800,    9100,    10500,   12000,   13600,   15300,   17100,   19000,
	21000,   23100,   25300,   27600,   30000,   32500,   35100,   37800,   40600,   43500,
	46500,   49600,   52800,   56100,   59500,   63000,   66600,   70300,   74100,   78000,
	82000,   86100,   90300,   94600,   99000,   103500,  108100,  112800,  117600,  122500,
	127500,  132700,  138100,  143700,  149500,  155500,  161700,  168100,  174700,  181500,
	188500,  195800,  203400,  211300,  219500,  228000,  236800,  245900,  255300,  265000,
	275000,  285400,  296200,  307400,  319000,  331000,  343400,  356200,  369400,  383000,
	397000,  411500,  426500,  442000,  458000,  474500,  491500,  509000,  527000,  545500,
	564500,  584500,  606500,  631500,  661500,  701500,  761500,  851500,  1000000, 1000000,
	1010000, 1011000, 1013000, 1016000, 1020000, 1025000, 1031000, 1038000, 1046000, 1055000,
	1065000, 1077000, 1091000, 1107000, 1125000, 1145000, 1168000, 1194000, 1223000, 1255000,
	1290000, 1329000, 1372000, 1419000, 1470000, 1525000, 1584000, 1647000, 1714000, 1785000,
	1860000, 1940000, 2025000, 2115000, 2210000, 2310000, 2415000, 2525000, 2640000, 2760000,
	2887000, 3021000, 3162000, 3310000, 3465000, 3628000, 3799000, 3978000, 4165000, 4360000
};

KCShipType::KCShipType(const QVariantMap &data, int loadId, QObject *parent) :
	KCGameObject(parent) {
	loadFrom(data, loadId);
}

KCShipType::~KCShipType() {

}

void KCShipType::loadFrom(const QVariantMap &data, int loadId) {
	Q_UNUSED(loadId);

	// All of these are retrieved in the order they are in the API responses
	// to make debugging eventual incorrect values easier.
	extract(data, id, "api_id");
	extract(data, cardno, "api_sortno");
	extract(data, name, "api_name");
	extract(data, reading, "api_yomi");
	extract(data, sclass, "api_stype");
	extract(data, ctype, "api_ctype");
	extract(data, cindex, "api_cnum");
	extract(data, encountered, "api_enqflg");
	extract(data, remodel.level, "api_afterlv");
	extract(data, remodel.into, "api_aftershipid");
	extract(data, hp.base, "api_taik", 0);
	extract(data, hp.max, "api_taik", 1);
	extract(data, armor.base, "api_souk", 0);
	extract(data, armor.max, "api_souk", 1);
	extract(data, _tous, 2, "api_tous");
	extract(data, firepower.base, "api_houg", 0);
	extract(data, firepower.max, "api_houg", 1);
	extract(data, torpedo.base, "api_raig", 0);
	extract(data, torpedo.max, "api_raig", 1);
	extract(data, _baku, 2, "api_baku");
	extract(data, antiair.base, "api_tyku", 0);
	extract(data, antiair.max, "api_tyku", 1);
	extract(data, _atap, 2, "api_atap");
	extract(data, antisub.base, "api_tais", 0);
	extract(data, antisub.max, "api_tais", 1);
	extract(data, _houm, 2, "api_houm");
	extract(data, _raim, 2, "api_raim");
	extract(data, evasion.base, "api_kaih", 0);
	extract(data, evasion.max, "api_kaih", 1);
	extract(data, _houk, 2, "api_houk");
	extract(data, _raik, 2, "api_raik");
	extract(data, _bakk, 2, "api_bakk");
	extract(data, lineOfSight.base, "api_saku", 0);
	extract(data, lineOfSight.max, "api_saku", 1);
	extract(data, _sakb, 2, "api_sakb");
	extract(data, luck.base, "api_luck", 0);
	extract(data, luck.max, "api_luck", 1);
	extract(data, _sokuh, "api_sokuh");
	extract(data, speed, "api_soku");
	extract(data, range, "api_leng");
	extract(data, _grow, 8, "api_grow");
	extract(data, equipmentSlots, "api_slot_num");
	extract(data, planeCapacity, 4, "api_maxeq");
	extract(data, _defeq, 4, "api_defeq");
	extract(data, buildTime, "api_buildtime");
	extract(data, dismantle.fuel, "api_broken", 0);
	extract(data, dismantle.ammo, "api_broken", 1);
	extract(data, dismantle.steel, "api_broken", 2);
	extract(data, dismantle.baux, "api_broken", 3);
	extract(data, modernization.firepower, "api_powup", 0);
	extract(data, modernization.torpedo, "api_powup", 1);
	extract(data, modernization.antiair, "api_powup", 2);
	extract(data, modernization.armor, "api_powup", 3);
	extract(data, _gumax, 4, "api_gumax");
	extract(data, rarity, "api_backs");
	extract(data, getMessage, "api_getmes");
	extract(data, _homemes, "api_homemes");
	extract(data, _gomes, "api_gomes");
	extract(data, _gomes2, "api_gomes2");
	extract(data, description, "api_sinfo");
	extract(data, remodel.ammo, "api_afterbull");
	extract(data, remodel.fuel, "api_afterfuel");
	extract(data, _touchs, 3, "api_touchs");
	extract(data, _missions, "api_missions");
	extract(data, _systems, "api_systems");
	extract(data, maxFuel, "api_fuel_max");
	extract(data, maxAmmo, "api_bull_max");
	extract(data, voicef, "api_voicef");
}
