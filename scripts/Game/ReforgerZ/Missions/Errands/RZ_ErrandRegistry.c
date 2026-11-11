//! Every errand the dealer knows: the routines (rows in RZ_ErrandRoutines) and the cards (one class
//! each in Cards/<Family>/). A card is hand-written, once per player per season, at most Cards a day.
//! Server only.
class RZ_ErrandRegistry
{
	//------------------------------------------------------------------------------------------------
	static void Build(notnull array<ref RZ_ErrandDef> list)
	{
		RZ_ErrandRoutine.Build(list);

		// The house
		list.Insert(new RZ_Errand_SleeveDown());
		list.Insert(new RZ_Errand_AWordTo());
		list.Insert(new RZ_Errand_RatherBeWrong());
		list.Insert(new RZ_Errand_AWordForMe());
		// Aftermath
		list.Insert(new RZ_Errand_TruckDidntCome());
		list.Insert(new RZ_Errand_SmokeWasOurs());
		list.Insert(new RZ_Errand_IsItTrue());
		// The lost
		list.Insert(new RZ_Errand_LetterNobody());
		list.Insert(new RZ_Errand_TradersDetour());
		list.Insert(new RZ_Errand_DesertersWay());
		// Road posts and the line
		list.Insert(new RZ_Errand_NothingGoesOut());
		// The dead
		list.Insert(new RZ_Errand_HisEffects());
		list.Insert(new RZ_Errand_NameOnTheTag());
		// The crews
		list.Insert(new RZ_Errand_NotForTheCrew());
		list.Insert(new RZ_Errand_WildcardMoney());
		// Amends
		list.Insert(new RZ_Errand_CleanSlate());
		// Fallout: a failed story job's (RZ_StoryKit.Fallout; Name on the Tag takes one too)
		list.Insert(new RZ_Errand_BuyItBack());
		// Disguises
		list.Insert(new RZ_Errand_DressRehearsal());
	}
}

//------------------------------------------------------------------------------------------------
//! The base of the cards: once per player per season; helpers for their Whys.
class RZ_ErrandCard : RZ_ErrandDef
{
	//------------------------------------------------------------------------------------------------
	void RZ_ErrandCard()
	{
		m_bCard = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The arsonist rule: never to the player, or their group, whose act made the fact (`cause`: the
	//! player ids that did it, server-only).
	static bool CausedBy(string cause, int playerId)
	{
		if (cause.IsEmpty())
			return false;

		array<string> ids = {};
		cause.Split(",", ids, true);
		foreach (string id : ids)
		{
			int who = id.ToInt();
			if (who == playerId || RZ_Util.SameGroup(who, playerId))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The signal kind of a faction's lost trucks (RZ_WorldWriters.TruckLost).
	static string TruckKind(int faction)
	{
		return "truck." + faction.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The newest signal of `kind` within `radius` of pos and `hours` of play, not caused by the
	//! player or their group; null if none.
	static RZ_Signal FreshSignal(string kind, vector pos, float radius, float hours, int playerId)
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		array<RZ_Signal> found = {};
		float days = hours * 60 / Math.Max(1, RZ_SeasonClock.DayMinutes());
		facts.Memory().RecentSignals(kind, pos, radius, days, found);
		foreach (RZ_Signal s : found)
		{
			if (!CausedBy(s.cause, playerId))
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Another safehouse within `radius` of pos (not `except`), lived in; -1 if none. hungry: only
	//! a short one; fed: only one above one upkeep.
	static int HouseNear(RZ_GameModeComponent gm, vector pos, float radius, int except, int want)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		int best = -1;
		float bestDist = radius;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || i == except || h.m_iPopulation <= 0)
				continue;

			bool shortNow = h.m_iFood < h.FoodNeed(houses.PeoplePerFood());
			if ((want == 1 && !shortNow) || (want == 2 && shortNow))
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, pos);
			if (d <= bestDist)
			{
				best = i;
				bestDist = d;
			}
		}

		return best;
	}
}
