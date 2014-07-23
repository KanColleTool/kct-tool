#include "KCClient.h"
#include <functional>
#include <map>
#include <QString>

#define pf [](KCClient *client, const QVariant &data)

/*
 * Extract or update data from an API response into a QMap<int, modelT*>&
 */
template<class modelT>
inline void modelizeResponse(const QVariant &data, QMap<int, modelT*> &target,
                             KCClient *client, int loadId=0, QString idKey="api_id") {
	for(QVariant item : data.toList()) {
		QVariantMap itemMap = item.toMap();
		modelT *&ship = target[itemMap.value(idKey).toInt()];

		if(!ship)
			ship = new modelT(itemMap, loadId, client);
		else
			ship->loadFrom(itemMap, loadId);
	}
}

const std::map<QString, KCClient::processFunc> KCClient::processFuncs = {
	{ "/kctool/focus", // Focus request
	  pf {
			Q_UNUSED(data);
			emit client->focusRequested();
		}
	},

	{ "/kcsapi/api_req_member/get_incentive", nullptr }, // Login
	{ "/kcsapi/api_start2",
		pf {
			QVariantMap map = data.toMap();
			modelizeResponse(map["api_mst_ship"], client->shipTypes, client);
			emit client->receivedShipTypes();
		}
	},

	// Global info -------------------------------------------------------------
	{ "/kctool/mastership.json", // Ships from our server
		pf {
			modelizeResponse(data, client->shipTypes, client);
			emit client->receivedShipTypes();
		}
	},
	{ "/kcsapi/api_get_master/ship", // Ships
		pf {
			modelizeResponse(data, client->shipTypes, client);
			emit client->receivedShipTypes();
		}
	},
	{ "/kcsapi/api_get_master/stype", nullptr }, // Ship types
	{ "/kcsapi/api_get_master/slotitem", nullptr },	// Items
	{ "/kcsapi/api_get_master/furniture", nullptr }, // Furniture
	{ "/kcsapi/api_get_master/mission", nullptr }, // Expeditions
	
	//  Sortie maps
	{ "/kcsapi/api_get_master/mapinfo", nullptr },
	{ "/kcsapi/api_get_master/mapcell", nullptr },
	
	// ???
	{ "/kcsapi/api_get_master/maparea", nullptr },

	// Member info -------------------------------------------------------------
	{ "/kcsapi/api_get_member/basic", 
		pf {
			QVariantMap map = data.toMap();
			if(!client->admiral)
				client->admiral = new KCAdmiral(map, 0, client);
			else
				client->admiral->loadFrom(map);
			
			emit client->receivedAdmiral();
		}
	},
	{ "/kcsapi/api_get_member/record", nullptr },
	
	//  Items
	{ "/kcsapi/api_get_member/slotitem", nullptr }, // Current items
	{ "/kcsapi/api_get_member/unsetslot", nullptr }, // Remove item
	{ "/kcsapi/api_get_member/useitem", nullptr },
	
	// ALL OF THE DATA
	{ "/kcsapi/api_port/port",
		pf {
			QVariantMap map = data.toMap();
			modelizeResponse(map.value("api_deck_port"), client->fleets, client);
			modelizeResponse(map.value("api_ndock"), client->repairDocks, client);
			modelizeResponse(map.value("api_ship"), client->ships, client);
			emit client->receivedFleets();
			emit client->receivedRepairs();
			emit client->receivedShips();
		}
	},
	
	// Ships
	{ "/kcsapi/api_get_member/ship2",
		pf {
		  modelizeResponse(data, client->ships, client);
	  }
	},
	
	//  Various statuses
	{ "/kcsapi/api_get_member/ndock", // Repair Docks
		pf {
			modelizeResponse(data, client->repairDocks, client);
			emit client->receivedRepairs();
		}
	},
	{ "/kcsapi/api_get_member/kdock", // Construction Docks
		pf {
			modelizeResponse(data, client->constructionDocks, client);
			emit client->receivedConstructions();
		}
	},
	
	//  Quests
	{ "/kcsapi/api_get_member/questlist", nullptr },
	{ "/kcsapi/api_req_quest/start", nullptr }, // Start tracking
	{ "/kcsapi/api_req_quest/stop", nullptr }, // Stop tracking
	{ "/kcsapi/api_req_quest/clearitemget", nullptr }, // Finish quest
	
	//  Actions
	{ "/kcsapi/api_req_kaisou/powerup", nullptr }, // Modernization
	{ "/kcsapi/api_req_kaisou/remodeling", nullptr },
	{ "/kcsapi/api_req_kaisou/slotset", nullptr }, // Equip an item
	{ "/kcsapi/api_req_hokyu/charge",
		pf {
		  QVariantMap map = data.toMap();
		  for(QVariant it : map.value("api_ship").toList())
		  {
			  QVariantMap shipMap = it.toMap();
			  KCShip *ship = client->ships[shipMap.value("api_id").toInt()];
			  if(ship)
			  {
				  ship->fuel = shipMap.value("api_fuel").toInt();
				  ship->ammo = shipMap.value("api_bull").toInt();
			  }
		  }
	  }
	}, // Resupplying
	{ "/kcsapi/api_req_hensei/change", nullptr }, // Swap out ships
	{ "/kcsapi/api_req_nyukyo/start", nullptr }, // Start a bath
	{ "/kcsapi/api_req_nyukyo/speedchange", nullptr }, // Buckets are cool
	{ "/kcsapi/api_req_kousyou/createship", nullptr },
	{ "/kcsapi/api_req_kousyou/createship_speedchange", nullptr }, // FLAME
	{ "/kcsapi/api_req_kousyou/getship", nullptr }, // Finish construction
	{ "/kcsapi/api_req_kousyou/destroyship", nullptr },
	{ "/kcsapi/api_req_kousyou/createitem", nullptr },
	{ "/kcsapi/api_req_kousyou/destroyitem2", nullptr },
	{ "/kcsapi/api_get_member/actionlog", nullptr },
	
	//  Furniture
	{ "/kcsapi/api_req_furniture/change", nullptr },
	{ "/kcsapi/api_get_member/furniture", nullptr },
	
	//  Store
	{ "/kcsapi/api_get_master/payitem", nullptr }, // Load the pay shop
	{ "/kcsapi/api_req_furniture/buy", nullptr },
	
	// ???
	{ "/kcsapi/api_get_member/deck_port", nullptr },
	{ "/kcsapi/api_get_master/useitem", nullptr },

	// Battle info -------------------------------------------------------------
	
	//  Expeditions
	{ "/kcsapi/api_req_mission/start", nullptr },
	{ "/kcsapi/api_req_mission/result", nullptr },
	
	//  Sorties
	{ "/kcsapi/api_req_map/start", nullptr }, // Start a sortie
	{ "/kcsapi/api_req_map/next", nullptr }, // Next cell to go to
	{ "/kcsapi/api_req_sortie/battle", nullptr },
	{ "/kcsapi/api_req_battle_midnight/battle", nullptr },
	{ "/kcsapi/api_req_sortie/battleresult", nullptr },
	/*	pf {
			Q_UNUSED(client);
			//Q_UNUSED(data);
			QVariantMap dataMap = data.toMap();
			QVariantMap droppedShipData = dataMap.value("api_get_ship").toMap();
			
			int ship = droppedShipData.value("api_ship_id").toInt();
			int world = dataMap.value("api_dests").toInt();
			int map = dataMap.value("api_destsf").toInt();
			
			client->logger->logDrop(ship, world, map);
		}
	},*/
	
	//  Practice
	{ "/kcsapi/api_get_member/practice", nullptr },
	{ "/kcsapi/api_req_member/getothersdeck", nullptr },
	{ "/kcsapi/api_req_practice/battle", nullptr },
	{ "/kcsapi/api_req_practice/midnight_battle", nullptr },
	{ "/kcsapi/api_req_practice/battle_result", nullptr },

	// Miscellaneous ------------------------------------------------------------
	{ "/kcsapi/api_auth_member/logincheck", nullptr }
};
