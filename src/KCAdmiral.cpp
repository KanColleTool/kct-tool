#include "KCAdmiral.h"

KCAdmiral::KCAdmiral(const QVariantMap &data, int loadId, QObject *parent):
	KCGameObject(parent)
{
	loadFrom(data, loadId);
}

KCAdmiral::~KCAdmiral()
{
	
}

void KCAdmiral::loadFrom(const QVariantMap &data, int loadId)
{
	Q_UNUSED(loadId);
	
	extract(data, id, "api_member_id");
	extract(data, nickname, "api_nickname");
	extract(data, activeFlag, "api_active_flag");
	extractTimestamp(data, startTime, "api_starttime");
	extract(data, level, "api_level");
	extract(data, rank, "api_rank");
	extract(data, experience, "api_experience");
	extract(data, maxShips, "api_max_chara");
	extract(data, maxEquipment, "api_max_slotitem");
	extract(data, maxFurniture, "api_max_kagu");
	extract(data, playtime, "api_playtime");
	extract(data, tutorial, "api_tutorial");
	extract(data, furniture, 6, "api_furniture");
	extract(data, fleetsCount, "api_count_deck");
	extract(data, cdockCount, "api_count_kdock");
	extract(data, rdockCount, "api_count_ndock");
	extract(data, fcoins, "api_fcoin");
	extract(data, sorties.won, "api_st_win");
	extract(data, sorties.lost, "api_st_lose");
	// Note: Expeditions are reported as successful and total, while everything else is
	//       success and failure, so just normalize expeditions into the same format
	extract(data, expeditions.won, "api_ms_success");
	{ int tmp; extract(data, tmp, "api_ms_count"); expeditions.lost = tmp - expeditions.won; }
	extract(data, pvp.won, "api_pt_win");
	extract(data, pvp.lost, "api_pt_lose");
	// And PvP Challenges seem to do the same thing (except they're {0, 0} for me)
	extract(data, pvpChallenged.won, "api_pt_challenged_win");
	{ int tmp; extract(data, tmp, "api_pt_challenged"); pvpChallenged.lost = tmp - pvpChallenged.won; }
	extract(data, firstFlag, "api_firstflag");
	extract(data, tutorialProgress, "api_tutorial_progress");
}
