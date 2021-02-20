#pragma once
#include <unordered_map>

#define OXEVENT_MAP_INDEX 113

enum class OXEventStatus
{
	OXEVENT_FINISH,
	OXEVENT_OPEN,
	OXEVENT_CLOSE,
	OXEVENT_QUIZ,
	OXEVENT_ERR
};

class COXEventManager : public singleton<COXEventManager>
{
private:
	enum
	{
		STATE_CHAR = (1 << 0),
		STATE_ATTENDER = (1 << 1),
		STATE_MISS = (1 << 2),
	};

	struct tag_Quiz
	{
		std::uint8_t level;
		std::string Quiz;
		bool answer;
		tag_Quiz(std::uint8_t m_level, const char* m_szQuestion, bool m_answer)
			: level(m_level), Quiz(m_szQuestion), answer(m_answer)
		{
		}
	};

	std::unordered_map<DWORD, std::uint8_t> m_common_ox;
	std::unordered_map<std::uint8_t, std::vector<std::shared_ptr<tag_Quiz>>> map_quiz;
	LPEVENT m_timedEvent = nullptr;
#if defined(BL_AUTOMATIC_OXEVENT)
	LPEVENT m_AutomaticOX = nullptr;
	struct SReward
	{
		DWORD vnum;
		std::uint8_t count;
		DWORD gold;
	};
	std::unordered_map <std::uint8_t, std::vector<std::shared_ptr<SReward>>> m_reward;
#endif

	bool Enter(LPCHARACTER pChar, std::uint8_t bState);

public:
	void Initialize();
	void Destroy();

	OXEventStatus GetStatus() const;
	void SetStatus(OXEventStatus status);

	bool Enter(LPCHARACTER pChar);

	void CloseEvent();

	void ClearQuiz();
	bool AddQuiz(std::uint8_t level, const char* pszQuestion, bool answer);
	bool ShowQuizList(LPCHARACTER pChar);

	bool Quiz(std::uint8_t level, int timelimit);
	void CheckAnswer(bool answer);

	void WarpToAudience();

	bool LogWinner();
	void GiveItemToAttender(DWORD dwItemVnum, std::uint8_t count);

#if defined(BL_AUTOMATIC_OXEVENT)
	enum { DAY, HOUR, MIN };
	enum EOXTIMER : std::uint8_t { SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY };
	static const std::vector<std::tuple<EOXTIMER, int, int>> vScheduleOXTable;
	bool LoadAutoOXSettings();
	void GiveAutoOXReward();
#endif

	std::uint16_t GetAttenderCount() const;
};
