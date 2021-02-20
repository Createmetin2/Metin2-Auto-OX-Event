//Find
		COXEventManager::instance().SetStatus(OXEventStatus::OXEVENT_OPEN);
		
///Add
#if defined(BL_AUTOMATIC_OXEVENT)
		quest::CQuestManager::instance().RequestSetEventFlag("auto_ox", 0); // to avoid conflict
#endif

///Add
	int oxevent_get_auto_table(lua_State* L)
	{
		lua_newtable(L);
#if defined(BL_AUTOMATIC_OXEVENT)
		int iCount = 0;
		for (const auto & v : COXEventManager::vScheduleOXTable) {
			std::uint8_t i = 1;
			lua_newtable(L);

			lua_pushnumber(L, std::get<COXEventManager::DAY>(v));
			lua_rawseti(L, -2, i++);

			lua_pushnumber(L, std::get<COXEventManager::HOUR>(v));
			lua_rawseti(L, -2, i++);

			lua_pushnumber(L, std::get<COXEventManager::MIN>(v));
			lua_rawseti(L, -2, i++);

			lua_rawseti(L, -2, ++iCount);
		}
#endif
		return 1;
	}

//Find
			{	"give_item",	oxevent_give_item	},
			
///Add
// #if defined(BL_AUTOMATIC_OXEVENT)
			{	"get_auto_table",	oxevent_get_auto_table	},
// #endif