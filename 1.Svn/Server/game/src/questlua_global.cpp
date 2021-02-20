///Add
	int _auto_ox_settings(lua_State* L)
	{
#if defined(BL_AUTOMATIC_OXEVENT)
		auto section = lua_tostring(L, 1);
		auto val = static_cast<std::uint16_t>(lua_tonumber(L, 2));
		COXEventManager::instance().SetAutoOXSettings(section, val);
#endif
		return 0;
	}

//Find
			{	"add_ox_quiz",					_add_ox_quiz					},
			
///Add
// #if defined(BL_AUTOMATIC_OXEVENT)
			{	"auto_ox_settings",				_auto_ox_settings				},
// #endif