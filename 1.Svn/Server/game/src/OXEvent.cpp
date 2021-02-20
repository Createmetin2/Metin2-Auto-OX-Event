/**
-blackdragonx61/Mali61
-https://github.com/blackdragonx61/Metin2-Auto-OX-Event
**/

#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "questmanager.h"
#include "start_position.h"
#include "packet.h"
#include "buffer_manager.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "OXEvent.h"
#include "desc.h"
#include "item.h"
#include <chrono>
#include <ctime>

/*PROCESS*/
bool COXEventManager::Enter(LPCHARACTER pkChar)
{
	if (GetStatus() == OXEventStatus::OXEVENT_FINISH) {
		sys_log(0, "OXEVENT : map finished. but char enter. %s", pkChar->GetName());
		return false;
	}

	const auto& pos = pkChar->GetXYZ();

	if (pos.x == 896500 && pos.y == 24600)
		return Enter(pkChar, STATE_CHAR | STATE_ATTENDER);
	else if (pos.x == 896300 && pos.y == 28900)
		return Enter(pkChar, STATE_CHAR);

	sys_log(0, "OXEVENT : wrong pos enter %d %d", pos.x, pos.y);
	return false;
}

bool COXEventManager::Enter(LPCHARACTER pkChar, std::uint8_t bState)
{
	m_common_ox[pkChar->GetPlayerID()] = bState;
	return true;
}

void COXEventManager::CheckAnswer(bool answer)
{
	if (m_common_ox.empty())
		return;

	static constexpr int arrRect[2][4] =
	{
		{892600, 22900, 896300, 26400},
		{896600, 22900, 900300, 26400},
	};

	auto rect = &arrRect[answer != true ? 0 : 1];

	for (auto iter = m_common_ox.begin(); iter != m_common_ox.end(); ++iter) {
		if (!(iter->second & STATE_ATTENDER))
			continue;

		auto pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->first);
		if (pkChar)
		{
			const auto& pos = pkChar->GetXYZ();

			if (pos.x < (*rect)[0] || pos.x >(*rect)[2] || pos.y < (*rect)[1] || pos.y >(*rect)[3])
			{
				pkChar->EffectPacket(SE_FAIL);
				iter->second = STATE_CHAR | STATE_MISS;
			}
			else
			{
				pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("沥翠涝聪促!"));

				char chatbuf[256];
				int len = snprintf(chatbuf, sizeof(chatbuf), "%s %lu %d", number(0, 1) == 1 ? "cheer1" : "cheer2", (DWORD)pkChar->GetVID(), 0);
				if (len < 0 || len >= (int)sizeof(chatbuf))
					len = sizeof(chatbuf) - 1;
				++len;

				TEMP_BUFFER buf;
				TPacketGCChat pack_chat;
				pack_chat.header = HEADER_GC_CHAT;
				pack_chat.size = static_cast<WORD>(sizeof(TPacketGCChat) + len);
				pack_chat.type = CHAT_TYPE_COMMAND;
				pack_chat.id = 0;
				buf.write(&pack_chat, sizeof(TPacketGCChat));
				buf.write(chatbuf, len);

				pkChar->PacketAround(buf.read_peek(), buf.size());
				pkChar->EffectPacket(SE_SUCCESS);
			}
		}
		else
			iter = m_common_ox.erase(iter);
	}
}

void COXEventManager::WarpToAudience()
{
	for (auto& [pid, state] : m_common_ox) {
		if (!(state & STATE_MISS))
			continue;

		state = STATE_CHAR;
		auto pkChar = CHARACTER_MANAGER::instance().FindByPID(pid);
		if (pkChar) {
			static constexpr long arr_Coordinates[3][2] =
			{
				{ 896300, 28900 },
				{ 890900, 28100 },
				{ 896600, 20500 },
			};
			constexpr auto size = std::size(arr_Coordinates) - 1;
			auto iRand = number(0, size);
			pkChar->Show(OXEVENT_MAP_INDEX, arr_Coordinates[iRand][0], arr_Coordinates[iRand][1]);
		}
	}
}

/*STATE*/
OXEventStatus COXEventManager::GetStatus() const
{
	auto ret = static_cast<OXEventStatus>(quest::CQuestManager::instance().GetEventFlag("oxevent_status"));
	if (ret >= OXEventStatus::OXEVENT_ERR)
		return OXEventStatus::OXEVENT_ERR;

	return ret;
}

void COXEventManager::SetStatus(OXEventStatus status)
{
	quest::CQuestManager::instance().RequestSetEventFlag("oxevent_status", static_cast<std::uint8_t>(status));
}

std::uint16_t COXEventManager::GetAttenderCount() const
{
	return static_cast<std::uint16_t>(std::count_if(m_common_ox.begin(), m_common_ox.end(), [](const auto& v) { return (v.second & STATE_ATTENDER); }));
}

/*TIMER*/
EVENTINFO(OXEventInfoData)
{
	bool answer;
	OXEventInfoData(bool bAnswer) : answer(bAnswer) {}
};

EVENTFUNC(oxevent_timer)
{
	auto info = dynamic_cast<OXEventInfoData*>(event->info);

	if (info == nullptr) {
		sys_err("oxevent_timer> <Factor> Null pointer");
		return 0;
	}

	enum class EOXTIMERSTATE
	{
		GET_POSITION,
		CHECK_ANSWER,
		SEE_RESULT,
	};

	static auto flag = EOXTIMERSTATE::GET_POSITION;

	switch (flag)
	{
	case EOXTIMERSTATE::GET_POSITION:
		SendNoticeMap(LC_TEXT("10檬第 魄沥窍摆嚼聪促."), OXEVENT_MAP_INDEX, true);
		flag = EOXTIMERSTATE::CHECK_ANSWER;
		return PASSES_PER_SEC(10);

	case EOXTIMERSTATE::CHECK_ANSWER:
		SendNoticeMap(LC_TEXT("沥翠篮"), OXEVENT_MAP_INDEX, true);

		COXEventManager::instance().CheckAnswer(info->answer);
		SendNoticeMap(info->answer ? LC_TEXT("O 涝聪促") : LC_TEXT("X 涝聪促"), OXEVENT_MAP_INDEX, true);

		SendNoticeMap(LC_TEXT("5檬 第 撇府脚 盒甸阑 官冰栏肺 捞悼 矫虐摆嚼聪促."), OXEVENT_MAP_INDEX, true);

		flag = EOXTIMERSTATE::SEE_RESULT;
		return PASSES_PER_SEC(5);

	case EOXTIMERSTATE::SEE_RESULT:
		COXEventManager::instance().WarpToAudience();
		COXEventManager::instance().SetStatus(OXEventStatus::OXEVENT_CLOSE);
		SendNoticeMap(LC_TEXT("促澜 巩力 霖厚秦林技夸."), OXEVENT_MAP_INDEX, true);
		flag = EOXTIMERSTATE::GET_POSITION;
		break;
	}

	return 0;
}

#if defined(BL_AUTOMATIC_OXEVENT)
#include "group_text_parse_tree.h"

static std::uint16_t REQUIRED_START_PLAYER_COUNT = 2;
static std::uint16_t REQUIRED_LAST_PLAYER_COUNT = 1;

const std::vector<std::tuple<COXEventManager::EOXTIMER, int, int>> COXEventManager::vScheduleOXTable =
{
	std::make_tuple(EOXTIMER::SATURDAY, 12, 49),
	std::make_tuple(EOXTIMER::THURSDAY, 21, 0),
};

void COXEventManager::GiveAutoOXReward()
{
	for (const auto& [pid, state] : m_common_ox) {
		if (state & STATE_ATTENDER) {
			auto pkChar = CHARACTER_MANAGER::instance().FindByPID(pid);
			if (pkChar) {
				auto it = m_reward.find(pkChar->GetJob());
				if (it == m_reward.end())
					continue;

				const auto& vec = it->second;
				if (vec.empty())
					continue;

				int iRand = number(0, static_cast<int>(vec.size()) - 1);
				const auto& reward = vec.at(iRand);
	
				auto item = pkChar->AutoGiveItem(reward->vnum, reward->count);
				if (item != nullptr) {
					char msg[128];
					snprintf(msg, sizeof(msg), "Winner: %s, Reward: %s(%dx)", pkChar->GetName(), item->GetName(), item->GetCount());
					SendNoticeMap(msg, OXEVENT_MAP_INDEX, true);
				}

				pkChar->GiveGold(reward->gold);

				if (pkChar->GetDesc())
					LogManager::instance().ItemLog(pkChar->GetPlayerID(), 0, reward->count, reward->vnum, "OXEVENT_REWARD", "", pkChar->GetDesc()->GetHostName(), reward->vnum);
			}
		}
	}
}

bool COXEventManager::LoadAutoOXSettings()
{
	m_reward.clear();
	
	char c_pszFileName[FILE_MAX_LEN];
	snprintf(c_pszFileName, sizeof(c_pszFileName), "%s/auto_ox_settings.txt", LocaleService_GetBasePath().c_str());
	auto loader = std::make_unique<CGroupTextParseTreeLoader>();

	if (!loader->Load(c_pszFileName)) {
		sys_err("%s: load error", c_pszFileName);
		return false;
	}

	auto OxRewardGroup = loader->GetGroup("main");
	if (!OxRewardGroup) {
		sys_err("ox_reward.txt need main group.");
		return false;
	}

	if (!OxRewardGroup->GetValue("req_start_player", 0, REQUIRED_START_PLAYER_COUNT)) {
		sys_err("Group %s does not have req_start_player.", OxRewardGroup->GetNodeName().c_str());
		return false;
	}

	if (!OxRewardGroup->GetValue("req_last_player", 0, REQUIRED_LAST_PLAYER_COUNT)) {
		sys_err("Group %s does not have req_last_player.", OxRewardGroup->GetNodeName().c_str());
		return false;
	}

	std::uint8_t job = 0;
	for (const auto& v : { "warrior", "assassin", "sura", "shaman", "lycan" })
	{
		auto pItemGroup = OxRewardGroup->GetChildNode(v);
		if (!pItemGroup)
		{
			sys_err("Group %s does not have %s group.", OxRewardGroup->GetNodeName().c_str(), v);
			return false;
		}

		auto itemGroupSize = pItemGroup->GetRowCount();

		for (int i = 0; i < itemGroupSize; i++)
		{
			auto r = std::make_shared<SReward>();

			if (!pItemGroup->GetValue(i, "item", r->vnum))
			{
				sys_err("row(%d) of group items of group %s does not have item column", i, v);
				return false;
			}

			if (!pItemGroup->GetValue(i, "count", r->count))
			{
				sys_err("row(%d) of group items of group %s does not have count column", i, v);
				return false;
			}

			if (!pItemGroup->GetValue(i, "gold", r->gold))
			{
				sys_err("row(%d) of group items of group %s does not have gold column", i, v);
				return false;
			}

			m_reward[job].emplace_back(std::move(r));
		}

		job++;
	}

	return true;
}

EVENTINFO(AutoOXEventInfoData)
{
	COXEventManager* pOX;
	AutoOXEventInfoData(COXEventManager* c)
		: pOX(c)
	{}
};

EVENTFUNC(auto_oxevent_timer)
{
	auto info = dynamic_cast<AutoOXEventInfoData*>(event->info);

	if (info == nullptr || info->pOX == nullptr) {
		sys_err("auto_oxevent_timer> <Factor> Null pointer");
		return 0;
	}
	
	if (quest::CQuestManager::instance().GetEventFlag("auto_ox") == 0)
		return PASSES_PER_SEC(60);

	auto OXManager = info->pOX;

	switch (OXManager->GetStatus())
	{
		case OXEventStatus::OXEVENT_FINISH:
		{
			const auto t = std::time(nullptr);
			const auto l = localtime(&t);
			const auto& vTime = COXEventManager::vScheduleOXTable;

			auto it = std::find_if(vTime.begin(), vTime.end(), [l](const auto& v) {
				return l->tm_wday == std::get<COXEventManager::DAY>(v) && l->tm_hour == std::get<COXEventManager::HOUR>(v) && l->tm_min == std::get<COXEventManager::MIN>(v);
			});

			if (it != vTime.end()) {
				OXManager->ClearQuiz();
				if (OXManager->LoadAutoOXSettings() == false)
					break;

				static const auto sFileName = LocaleService_GetBasePath() + "/oxquiz.lua";
				if (lua_dofile(quest::CQuestManager::instance().GetLuaState(), sFileName.c_str()) == 0) {
					OXManager->SetStatus(OXEventStatus::OXEVENT_OPEN);
					BroadcastNotice("OX event is starting in a few minutes.");
					BroadcastNotice("Talk to Uriel to enter the challenge!");
				}
				else
					sys_err("Cannot load %s file!", sFileName.c_str());
			}

			break;
		}
		case OXEventStatus::OXEVENT_OPEN:
		{
			const auto PlayerCount = OXManager->GetAttenderCount();
			if (PlayerCount >= REQUIRED_START_PLAYER_COUNT) {
				OXManager->SetStatus(OXEventStatus::OXEVENT_CLOSE);
				
				char msg[128];
				snprintf(msg, sizeof(msg), "OX Event will start in 10 seconds with %d characters!", PlayerCount);
				BroadcastNotice(msg);
				
				return PASSES_PER_SEC(10);
			}
			break;
		}
		case OXEventStatus::OXEVENT_CLOSE:
		{
			if (OXManager->GetAttenderCount() <= REQUIRED_LAST_PLAYER_COUNT)
			{
				OXManager->GiveAutoOXReward();
				OXManager->LogWinner();

				SendNoticeMap("The OX event is over.", OXEVENT_MAP_INDEX, true);
				SendNoticeMap("Participants will be teleported to the city in 30 sec.", OXEVENT_MAP_INDEX, true);

				OXManager->SetStatus(OXEventStatus::OXEVENT_ERR);
			}
			else if (OXManager->Quiz(1, 30) == false) // level, timelimit
			{
				SendNoticeMap("<OX Event> Error Creating Quiz", OXEVENT_MAP_INDEX, true);
				OXManager->SetStatus(OXEventStatus::OXEVENT_ERR);
				return PASSES_PER_SEC(5);
			}

			return PASSES_PER_SEC(30);
		}
		case OXEventStatus::OXEVENT_QUIZ: // wait
			return PASSES_PER_SEC(10);
		case OXEventStatus::OXEVENT_ERR:
			OXManager->CloseEvent(); //OXEventStatus::OXEVENT_FINISH
			break;
	}

	return PASSES_PER_SEC(60);
}
#endif

/*QUIZ*/
bool COXEventManager::AddQuiz(std::uint8_t level, const char* pszQuestion, bool answer)
{
	auto quiz = std::make_shared<tag_Quiz>(level, pszQuestion, answer);
	map_quiz[level].emplace_back(std::move(quiz));

	return true;
}

bool COXEventManager::ShowQuizList(LPCHARACTER pkChar)
{
	std::size_t c = 0;
	for (const auto& [level, vec] : map_quiz) {
		c += vec.size();
		for (const auto& Quiz : vec)
			pkChar->ChatPacket(CHAT_TYPE_INFO, "%d %s %s", Quiz->level, Quiz->Quiz.c_str(), Quiz->answer ? LC_TEXT("曼") : LC_TEXT("芭窿"));
	}

	pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("醚 柠令 荐: %d"), c);
	return true;
}

void COXEventManager::ClearQuiz()
{
	map_quiz.clear();
}

bool COXEventManager::Quiz(std::uint8_t level, int timelimit)
{
	auto it = map_quiz.find(level);
	if (it == map_quiz.end())
		return false;

	const auto& vec = it->second;
	if (vec.empty())
		return false;

	auto idx = static_cast<size_t>(number(0, static_cast<int>(vec.size()) - 1));
	const auto& Quiz = vec.at(idx);

	if (timelimit < 0)
		timelimit = 30;
	timelimit -= 15;

	SendNoticeMap(LC_TEXT("巩力 涝聪促."), OXEVENT_MAP_INDEX, true);
	SendNoticeMap(Quiz->Quiz.c_str(), OXEVENT_MAP_INDEX, true);
	SendNoticeMap(LC_TEXT("嘎栏搁 O, 撇府搁 X肺 捞悼秦林技夸"), OXEVENT_MAP_INDEX, true);

	if (m_timedEvent != nullptr)
		event_cancel(&m_timedEvent);

	m_timedEvent = event_create(oxevent_timer, new OXEventInfoData(Quiz->answer), PASSES_PER_SEC(timelimit));

	SetStatus(OXEventStatus::OXEVENT_QUIZ);

	return true;
}

/*Constructor-Destructor*/
void COXEventManager::Initialize()
{
	CloseEvent();

#if defined(BL_AUTOMATIC_OXEVENT)
	if (g_bChannel == 99)
		m_AutomaticOX = event_create(auto_oxevent_timer, new AutoOXEventInfoData(this), PASSES_PER_SEC(60));
#endif
}

void COXEventManager::Destroy()
{
	CloseEvent();
#if defined(BL_AUTOMATIC_OXEVENT)
	if (m_AutomaticOX != nullptr)
		event_cancel(&m_AutomaticOX);
	m_reward.clear();
#endif
}

/*END OF EVENT*/
void COXEventManager::CloseEvent()
{
	if (m_timedEvent != nullptr)
		event_cancel(&m_timedEvent);

	for (const auto& [pid, state] : m_common_ox) {
		auto pkChar = CHARACTER_MANAGER::instance().FindByPID(pid);
		if (pkChar)
			pkChar->WarpSet(static_cast<long>(EMPIRE_START_X(pkChar->GetEmpire())), static_cast<long>(EMPIRE_START_Y(pkChar->GetEmpire())));
	}

	SetStatus(OXEventStatus::OXEVENT_FINISH);
	m_common_ox.clear();
	ClearQuiz();
}

bool COXEventManager::LogWinner()
{
	for (const auto& [pid, state] : m_common_ox) {
		if (state & STATE_ATTENDER) {
			auto pkChar = CHARACTER_MANAGER::instance().FindByPID(pid);
			if (pkChar)
				LogManager::instance().CharLog(pkChar, 0, "OXEVENT", "LastManStanding");
		}
	}

	return true;
}

void COXEventManager::GiveItemToAttender(DWORD dwItemVnum, std::uint8_t count)
{
	for (const auto& [pid, state] : m_common_ox) {
		if (state & STATE_ATTENDER) {
			auto pkChar = CHARACTER_MANAGER::instance().FindByPID(pid);
			if (pkChar) {
				auto item = pkChar->AutoGiveItem(dwItemVnum, count);
				if (item != nullptr) {
					char msg[128];
					snprintf(msg, sizeof(msg), "Winner: %s, Reward: %s(%dx)", pkChar->GetName(), item->GetName(), count);
					SendNoticeMap(msg, OXEVENT_MAP_INDEX, true);
				}
				LogManager::instance().ItemLog(pkChar->GetPlayerID(), 0, count, dwItemVnum, "OXEVENT_REWARD", "", pkChar->GetDesc()->GetHostName(), dwItemVnum);
			}
		}
	}
}
