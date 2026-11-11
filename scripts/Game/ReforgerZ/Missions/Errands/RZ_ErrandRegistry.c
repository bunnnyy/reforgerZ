//! Every errand the dealer knows: the routines (rows in RZ_ErrandRoutines) and the cards (one class
//! each in Cards/<Family>/). A card is hand-written, once per player per season, at most Cards a day.
//! Server only.
class RZ_ErrandRegistry
{
	//------------------------------------------------------------------------------------------------
	static void Build(notnull array<ref RZ_ErrandDef> list)
	{
		RZ_ErrandRoutine.Build(list);
		// The routines phase 4 added (RZ_ErrandRoutinesPeople.c)
		list.Insert(new RZ_Routine_WhosAtTheGate());
		list.Insert(new RZ_Routine_SitWithHim());
		list.Insert(new RZ_Routine_FreshChalk());
		list.Insert(new RZ_Routine_FellOff());
		list.Insert(new RZ_Routine_Sandbags());
		list.Insert(new RZ_Routine_RentsDue());
		list.Insert(new RZ_Routine_TheNumber());
		// The routines phase 8 added (RZ_ErrandRoutinesLate.c)
		list.Insert(new RZ_Routine_HisLighter());
		list.Insert(new RZ_Routine_Market());
		list.Insert(new RZ_Routine_Receiver());

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
		list.Insert(new RZ_Errand_OfficersCoat());
		// Disguise phases 2 and 3: the side jobs (docs/ERRANDS.md, the cards)
		list.Insert(new RZ_Errand_NameOnTheTape());
		list.Insert(new RZ_Errand_SpotTheTells());
		list.Insert(new RZ_Errand_Paint());
		list.Insert(new RZ_Errand_WrongWayRound());
		list.Insert(new RZ_Errand_LaundryDay());
		list.Insert(new RZ_Errand_Civvies());
		list.Insert(new RZ_Errand_MasksOff());
		list.Insert(new RZ_Errand_Notebook31());
		// Phase 4: the house, the lost, the crews, Bastion, help across lines, bad standing, the Long
		// Table, the infected, champions and backgrounds
		list.Insert(new RZ_Errand_Vouch());
		list.Insert(new RZ_Errand_YoureTheOne());
		list.Insert(new RZ_Errand_PayPacket());
		list.Insert(new RZ_Errand_RedDoor());
		list.Insert(new RZ_Errand_SomebodyTell());
		list.Insert(new RZ_Errand_BothHands());
		list.Insert(new RZ_Errand_NameDay());
		list.Insert(new RZ_Errand_Courtesy());
		list.Insert(new RZ_Errand_DebtGoneQuiet());
		list.Insert(new RZ_Errand_OffTheReport());
		list.Insert(new RZ_Errand_DressUniform());
		list.Insert(new RZ_Errand_BorrowedGreen());
		list.Insert(new RZ_Errand_BothDoors());
		list.Insert(new RZ_Errand_CharacterReference());
		list.Insert(new RZ_Errand_UnknownFace());
		list.Insert(new RZ_Errand_Stray());
		list.Insert(new RZ_Errand_WhosAsking());
		list.Insert(new RZ_Errand_JarRun());
		list.Insert(new RZ_Errand_ShoesInARow());
		list.Insert(new RZ_Errand_NotPastChapel());
		// Phase 8: to about 60 cards, every one with a live reason (the disguise side jobs are above)
		list.Insert(new RZ_Errand_Notebook31Fallout());
		list.Insert(new RZ_Errand_OtherHalf());
		list.Insert(new RZ_Errand_TheDrawing());
		list.Insert(new RZ_Errand_IsSheGone());
		list.Insert(new RZ_Errand_BoardedUp());
		list.Insert(new RZ_Errand_OldHabits());
		list.Insert(new RZ_Errand_NamesOnTheCall());
		list.Insert(new RZ_Errand_ChalkOnTheDoor());
		list.Insert(new RZ_Errand_AfterTheNoise());
		list.Insert(new RZ_Errand_LastOrders());
		list.Insert(new RZ_Errand_NewNumber());
		list.Insert(new RZ_Errand_LastApples());
		// The margin cards (the majors' waits) and the lock cards (the finale's lock)
		list.Insert(new RZ_Errand_CleanGlasses());
		list.Insert(new RZ_Errand_CountForTheSitting());
		list.Insert(new RZ_Errand_HandToHand());
		list.Insert(new RZ_Errand_WithSide());
		// The epilogue deck: dealt only in the week after the cure (Cards/Epilogue/, m_bAfterCure)
		list.Insert(new RZ_Errand_CarryTheDose());
		list.Insert(new RZ_Errand_PackForTheQueue());
		list.Insert(new RZ_Errand_Borrowed());
		list.Insert(new RZ_Errand_NameToTheBoard());
		list.Insert(new RZ_Errand_TollMansDebt());
		list.Insert(new RZ_Errand_SignHere());
		list.Insert(new RZ_Errand_CopiesFarHouse());
		list.Insert(new RZ_Errand_ClearTheWay());
		list.Insert(new RZ_Errand_LastWatch());
		list.Insert(new RZ_Errand_LastLetters());
		list.Insert(new RZ_Errand_OneLastToll());
		list.Insert(new RZ_Errand_IntoTheRecord());
		list.Insert(new RZ_Errand_ComeIn());
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
	//! The infected's nearest ground (a tribe) whose lair is within `range` of pos; null without tribes.
	static RZ_Tribe TribeNear(RZ_GameModeComponent gm, vector pos, float range)
	{
		RZ_Tribes tribes = gm.GetTribes();
		if (!tribes)
			return null;

		return tribes.NearestTribe(pos, range);
	}

	//------------------------------------------------------------------------------------------------
	//! What this player is called by a faction now (the sixteen names).
	static RZ_ERep RepOf(RZ_GameModeComponent gm, int playerId, int f)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		return RZ_RepNames.GetRep(p.GetFame(f), p.GetInfamy(f));
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest holding of `faction` (-1 any) within `radius` of pos, outside 100 m; null if none.
	static RZ_Holding HoldingNear(RZ_GameModeComponent gm, int faction, vector pos, float radius)
	{
		RZ_Territory territory = gm.GetTerritory();
		if (!territory)
			return null;

		RZ_Holding best;
		float bestDist = radius;
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			float d = RZ_Util.DistXZ(h.m_vPos, pos);
			if ((faction < 0 || h.m_eFaction == faction) && d >= 100 && d <= bestDist)
			{
				best = h;
				bestDist = d;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The card's own place and its words ({subject_place}).
	static void Place(RZ_Errand e, vector at, string words)
	{
		e.m_vSubject = at;
		string w = words;
		if (w.IsEmpty())
			w = RZ_Util.DirectionText(e.m_vGiver, at);

		e.SetText("subject_place", w);
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
