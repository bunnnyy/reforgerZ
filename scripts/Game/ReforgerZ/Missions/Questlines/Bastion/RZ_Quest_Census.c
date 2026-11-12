//! The Census (Task Force Bastion; B2 in docs/QUESTLINES.md). The nightly call reads "civilians
//! sheltered" to Command, and everything Bastion looks after is signed for, people too. The
//! commanding officer wants a real count: every safehouse, every name, for ration cards, a curfew
//! and a "contribution". The Haven remembers what Bastion's lists were for on Day 2.
//!
//! 0. The count begins (no wait): with the Haven's List already in Bastion's hands (H1's
//!    list.to_bastion) it goes straight to the curfew.
//! 1. Count a safehouse (mission): ask its council head (Known with the Haven), or count it from a
//!    watch point outside, unnoticed (sneak). ★ Police: the count is exact either way. A house H2
//!    put on Bastion's census (door.bastion; door.house names it) counts itself.
//! 2. Ration cards (mission): a crate each for two houses. Skim one (15 Bastion Favor; 25% the
//!    quartermaster notices), or give extra from your own pack (Haven Fame).
//! 3. The curfew (mission, at night): walk the beat in the first house's town (the second's if it
//!    has emptied). Enforce it (+3 Haven Infamy), patrol gently (★ Police, or Known with the Haven:
//!    half pay), or let a Red Hand toll in for the night (Known with the Red Hand).
//! 4. The contribution (mission, not for Elders): four food from a counted house (never the
//!    chair's after Await Relief's orders.haven). Carry it to Bastion, or to another house; pay it
//!    yourself (15 Bastion Favor); or falsify the ledger (Trusted, or ★ Police; 25% found out at 6).
//! 5. The Red Hand hears (wait, then a mission with the Red Hand group prefab): a crew raids a
//!    counted house. Defend it, hold its stores for the crew (the Red Hand's count), or let it be.
//! 6. The nightly call (mission at the stronghold, 20:30): read the count as it is, high, low (25%
//!    found out), or tear it up.
//!
//! Endings: Counted, Forged, Refused, The Red Hand's count.
class RZ_Quest_Census : RZ_Questline
{
	static const string KEY = "census";

	static const int S_START = 0;
	static const int S_COUNT = 1;
	static const int S_CARDS = 2;
	static const int S_CURFEW = 3;
	static const int S_CONTRIB = 4;
	static const int S_HEARS = 5;
	static const int S_RAID = 6;
	static const int S_CALL = 7;

	static const int OUT_COUNTED = 10;
	static const int OUT_CARDS = 11;
	static const int OUT_ENFORCED = 12;
	static const int OUT_GENTLE = 13;
	static const int OUT_TOLL = 14;
	static const int OUT_COLLECTED = 15;
	static const int OUT_PAID = 16;
	static const int OUT_RETURNED = 17;
	static const int OUT_FALSIFIED = 18;
	static const int OUT_DEFENDED = 19;
	static const int OUT_SIDED = 20;
	static const int OUT_RAIDED = 21;
	static const int OUT_TRUE = 22;
	static const int OUT_HIGH = 23;
	static const int OUT_LOW = 24;
	static const int OUT_REFUSED = 25;

	static const int E_COUNTED = 0;
	static const int E_FORGED = 1;
	static const int E_REFUSED = 2;
	static const int E_REDHAND = 3;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(KEY, "The Census", RZ_EFaction.MILITARY);

		WaitStage(S_START, "The commanding officer's count", "{co} wants a real count: every safehouse, every name, for ration cards, a curfew and a \"contribution\". Everything is signed for. People too.");

		RZ_QuestStage count = MissionStage(S_COUNT, "Count a safehouse", "Count the safehouse in {town_a}: people, food, names. Ask its council head, or count it from outside without being seen.");
		count.GiveUp(4, "Nobody counted {town_a}. At {stronghold} the old number goes out every night.");
		count.Visits(2);

		RZ_QuestStage cards = MissionStage(S_CARDS, "Ration cards", "Ration cards for {town_a} and {town_b}, on the books: a crate each, from {store}.");
		cards.GiveUp(4, "The ration cards were printed. The rations never left {store}.");
		cards.Visits(2);

		RZ_QuestStage curfew = MissionStage(S_CURFEW, "The curfew", "A curfew in {town_f} tonight. Walk the beat. How hard is up to you.");
		curfew.GiveUp(6, "Nobody walked the curfew in {town_f}. The houses kept their own hours.");
		curfew.Visits(2);

		RZ_QuestStage contrib = MissionStage(S_CONTRIB, "The contribution", "The census says {town_c} can spare four food. Bastion calls it a contribution. Collect it, pay it, or make the ledger say you did.");
		contrib.GiveUp(4, "Nobody collected the contribution from {town_c}. The ledger has a blank line.");

		RZ_QuestStage hears = WaitStage(S_HEARS, "The Red Hand hears", "The ration cards say which houses have full stores. The Red Hand can read too.");
		hears.Wait(90, 12);

		RZ_QuestStage raid = MissionStage(S_RAID, "The raid", "A Red Hand crew has heard the cards mean full stores. They're coming for {town_r}.");
		raid.GiveUp(3, "The crew never came for {town_r}. Somebody must have warned them off.");

		RZ_QuestStage call = MissionStage(S_CALL, "The nightly call", "At 20:30 the count goes to Command from the radio at {stronghold}: as it is, high, low, or not at all. Command doesn't answer.");
		call.GiveUp(8, "The count stayed in a drawer at {stronghold}. Bastion read the old number to Command.");
		call.Visits(2);
	}

	// ================================================================ Starting
	//------------------------------------------------------------------------------------------------
	protected static bool IsCountable(RZ_GameModeComponent gm, int house)
	{
		RZ_Safehouse h = GetHouse(gm, house);
		return h && h.m_iPopulation >= 2 && h.m_iSettlement >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The commanding officer's contract, while Bastion needs supplies or security (faction life
	//! off: any time). Needs two safehouses, a person to talk to and the mission item (the crates).
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || gm.GetMissionItemPrefab().IsEmpty())
			return false;

		if (gm.GetCure().IsCured() || gm.GetDirector().GetStageSettings().m_fMilitaryPresence <= 0)
			return false;

		// Await Relief's Expended: nobody left to count for (docs/QUESTLINES.md, Crossings)
		if (RZ_MajorHelp.World(gm, "orders.stance") == 4)
			return false;

		RZ_FactionLife life = gm.GetFactionLife();
		if (life && life.IsActive())
		{
			int need = life.GetNeed(RZ_EFaction.MILITARY);
			if (need != RZ_ENeed.SUPPLIES && need != RZ_ENeed.SECURITY)
				return false;
		}

		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!co || !houses || houses.Count() < 2)
			return false;

		// The house H2 put on Bastion's census (door.bastion; door.house is its index + 1) counts itself
		int first = -1;
		int door = -1;
		if (RZ_BastionQuest.World(gm, "door.bastion") > 0)
			door = RZ_BastionQuest.World(gm, "door.house") - 1;
		if (IsCountable(gm, door))
		{
			first = door;
			run.SetFlag("self", 1);
		}

		if (first < 0)
		{
			array<int> options = {};
			for (int i = 0; i < houses.Count(); i++)
			{
				if (IsCountable(gm, i))
					options.Insert(i);
			}

			if (options.IsEmpty())
				return false;

			first = options.GetRandomElement();
		}

		RZ_Safehouse ha = houses.Get(first);
		int second = RZ_LeaderJob.SafehouseNear(gm, ha.m_vPos, 0, 100000, first);
		RZ_Safehouse hb = houses.Get(second);
		if (!hb)
			return false;

		RZ_BastionQuest.Keep(run, "co", co);
		run.SetFlag("house_a", first);
		run.SetFlag("house_b", second);
		run.SetFlag("house_c", first);
		run.SetFlag("house_f", first);
		run.SetFlag("house_r", first);
		run.SetText("town_a", ha.m_sName);
		run.SetText("town_b", hb.m_sName);
		run.SetText("town_c", ha.m_sName);
		run.SetText("town_f", ha.m_sName);
		run.SetText("town_r", ha.m_sName);
		run.SetText("co", co.m_sShort);
		run.SetText("stronghold", co.m_sPlace);
		run.SetText("store", co.m_sPlace);
		run.SetPlace(ha.m_vPos, ha.m_sName);
		if (RZ_BastionQuest.World(gm, "list.to_bastion") > 0)
			run.SetFlag("listed", 1);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		run.Note("{co} wants every safehouse counted, every name: for ration cards, a curfew and a \"contribution\". The Haven remembers what Bastion's lists were for on Day 2.");
		if (run.Flag("self") == 1)
			run.Note("The safehouse in {town_a} already put itself on Bastion's census. It'll count itself.");
	}

	//------------------------------------------------------------------------------------------------
	//! A counted house that still has people in it, -1 if not.
	protected static int Live(RZ_GameModeComponent gm, RZ_QuestRun run, string key)
	{
		int house = run.Flag(key, -1);
		RZ_Safehouse h = GetHouse(gm, house);
		if (!h || h.m_iPopulation <= 0)
			return -1;

		return house;
	}

	//------------------------------------------------------------------------------------------------
	//! Would Bastion's contribution never be asked of this house? The chair's, after Await
	//! Relief's "Haven's leverage" (orders.haven).
	static bool IsChairProtected(RZ_GameModeComponent gm, int house)
	{
		return house >= 0 && RZ_BastionQuest.World(gm, "orders.haven") > 0 && house == RZ_BastionQuest.ChairHouse(gm);
	}

	// ================================================================ Requisition (a contract that reads this line)
	//------------------------------------------------------------------------------------------------
	//! Bastion's Requisition never comes to this house this season: the chair's after orders.haven,
	//! or one the census read low (census.spared).
	static bool IsSpared(RZ_GameModeComponent gm, int house)
	{
		if (house < 0)
			return false;

		if (IsChairProtected(gm, house))
			return true;

		return (RZ_BastionQuest.World(gm, KEY + ".spared") & RZ_BastionQuest.HouseBit(house)) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Requisition's weight: x1.2 for the season once a census was refused (census.pressed).
	static float RequisitionWeight(RZ_GameModeComponent gm)
	{
		if (RZ_BastionQuest.World(gm, KEY + ".pressed") != 0)
			return 1.2;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The town Requisition goes to: a house that refused the census first, else the safest town
	//! whose nearest safehouse isn't spared. -1 if none is free.
	static int RequisitionTown(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		int pressed = RZ_BastionQuest.World(gm, KEY + ".pressed");
		if (houses && pressed != 0)
		{
			array<int> options = {};
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if ((pressed & RZ_BastionQuest.HouseBit(i)) == 0 || h.m_iSettlement < 0 || IsSpared(gm, i))
					continue;

				if (!gm.GetMissions().IsSettlementBusy(h.m_iSettlement))
					options.Insert(h.m_iSettlement);
			}

			if (!options.IsEmpty())
				return options.GetRandomElement();
		}

		int best = -1;
		float bestInfection = float.MAX;
		foreach (int idx, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection >= bestInfection || gm.GetMissions().IsSettlementBusy(idx))
				continue;

			if (houses && IsSpared(gm, houses.FindNearest(s.m_vCenter, 3000)))
				continue;

			best = idx;
			bestInfection = s.m_fInfection;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The contribution's house, chosen as the stage comes up from what's still standing: the first
	//! house, else the second; never the chair's after orders.haven. False if there's none.
	protected bool PickContribution(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int c = Live(gm, run, "house_a");
		if (c < 0 || IsChairProtected(gm, c))
			c = Live(gm, run, "house_b");

		RZ_Safehouse hc = GetHouse(gm, c);
		if (!hc || IsChairProtected(gm, c))
			return false;

		run.SetFlag("house_c", c);
		run.SetText("town_c", hc.m_sName);
		run.SetPlace(hc.m_vPos, hc.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The curfew's town: the first house's, else the second's if the first has emptied. False if
	//! neither has people.
	protected bool PickCurfew(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int f = Live(gm, run, "house_a");
		if (f < 0)
			f = Live(gm, run, "house_b");

		RZ_Safehouse hf = GetHouse(gm, f);
		if (!hf)
			return false;

		run.SetFlag("house_f", f);
		run.SetText("town_f", hf.m_sName);
		run.SetPlace(hf.m_vPos, hf.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! After the cards (or the List): the curfew, else straight to the contribution, else the wait.
	protected int ToCurfew(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (PickCurfew(gm, run))
			return S_CURFEW;

		if (PickContribution(gm, run))
			return S_CONTRIB;

		return S_HEARS;
	}

	//------------------------------------------------------------------------------------------------
	//! The raid's house: the counted one with the fuller stores. False if neither has people.
	protected bool PickRaid(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int ra = Live(gm, run, "house_a");
		int rb = Live(gm, run, "house_b");
		int r = ra;
		RZ_Safehouse ha = GetHouse(gm, ra);
		RZ_Safehouse hb = GetHouse(gm, rb);
		if (!ha || (hb && hb.m_iFood > ha.m_iFood))
			r = rb;

		RZ_Safehouse hr = GetHouse(gm, r);
		if (!hr)
			return false;

		run.SetFlag("house_r", r);
		run.SetText("town_r", hr.m_sName);
		run.SetPlace(hr.m_vPos, hr.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_HEARS)
			run.Note("The ration cards say which houses have full stores. The Red Hand can read too.");
	}

	//------------------------------------------------------------------------------------------------
	//! The curfew only at night (or at nightfall); the nightly call only around 20:30.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (gm.GetCure().IsCured())
			return false;

		switch (stage)
		{
			case S_COUNT: return Live(gm, run, "house_a") >= 0;
			case S_CURFEW: return Live(gm, run, "house_f") >= 0 && IsNightSoon();
			case S_CONTRIB: return Live(gm, run, "house_c") >= 0;
			case S_RAID: return Live(gm, run, "house_r") >= 0;
			case S_CALL: return IsCallSoon();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsNightSoon()
	{
		if (RZ_GameTime.GetHour() < 0 || !RZ_GameTime.IsCalibrated() || RZ_GameTime.IsNight())
			return true;

		float secs = RZ_GameTime.RealSecondsUntil(RZ_GameTime.NIGHT_START);
		return secs >= 0 && secs <= 600;
	}

	//------------------------------------------------------------------------------------------------
	//! Within an hour and a half before the call, or during it (20:30 to 22:30).
	static bool IsCallSoon()
	{
		float h = RZ_GameTime.GetHour();
		if (h < 0 || !RZ_GameTime.IsCalibrated())
			return true;

		return RZ_GameTime.IsWithin(h, 19, 3.5);
	}

	//------------------------------------------------------------------------------------------------
	//! The call itself: 20:30 to 22:30 (any time on a server whose clock doesn't move).
	static bool IsCallTime()
	{
		float h = RZ_GameTime.GetHour();
		if (h < 0 || !RZ_GameTime.IsCalibrated())
			return true;

		return RZ_GameTime.IsWithin(h, RZ_GameTime.NIGHT_START, 2);
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_COUNT: return new RZ_QM_CensusCount();
			case S_CARDS: return new RZ_QM_CensusCards();
			case S_CURFEW: return new RZ_QM_CensusCurfew();
			case S_CONTRIB: return new RZ_QM_CensusContrib();
			case S_RAID: return new RZ_QM_CensusRaid();
			case S_CALL: return new RZ_QM_CensusCall();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The commanding officer voices every stage's contract.
	static int Voice(RZ_GameModeComponent gm)
	{
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (co)
			return co.m_iId;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected void Count(RZ_QuestRun run, int house)
	{
		run.SetFlag("counted", run.Flag("counted") | RZ_BastionQuest.HouseBit(house));
	}

	// ================================================================ The story
	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_START:
			{
				if (run.Flag("listed") != 1)
					return S_COUNT;

				// The List did the counting: both houses are on the books already
				Count(run, run.Flag("house_a", -1));
				Count(run, run.Flag("house_b", -1));
				RZ_BastionQuest.SetWorld(gm, KEY + ".counted", run.Flag("house_a", -1) + 1);
				run.SetFlag("no_cards", 1);
				run.Note("The Haven's List reached {stronghold}: every house, every name, crossed-out ones too. The count is done before anyone walked it.");
				return ToCurfew(gm, run);
			}
			case S_COUNT:
			{
				if (outcome == OUT_COUNTED)
				{
					Count(run, run.Flag("house_a", -1));
					RZ_BastionQuest.SetWorld(gm, KEY + ".counted", run.Flag("house_a", -1) + 1);
					run.Note("{town_a}, counted: {count_a}.");
					return S_CARDS;
				}

				run.Note("Nobody got a count of {town_a}. {co} will ask again.");
				return S_COUNT;
			}
			case S_CARDS:
				return AfterCards(gm, run, outcome);
			case S_CURFEW:
				return AfterCurfew(gm, run, outcome);
			case S_CONTRIB:
				return AfterContribution(run, outcome);
			case S_HEARS:
			{
				if (gm.GetRedHandGroupPrefab().IsEmpty() || !PickRaid(gm, run))
				{
					run.Note("The Red Hand heard about the cards. Nobody came for the stores, this time.");
					return S_CALL;
				}

				return S_RAID;
			}
			case S_RAID:
			{
				if (outcome == OUT_SIDED)
				{
					run.SetFlag("raided", 1);
					RZ_BastionQuest.SetWorld(gm, KEY + ".raided", run.Flag("house_r", -1) + 1);
					return Finish(run, E_REDHAND, "The Red Hand's count. The crew loaded up {town_r}'s stores while someone held the door for them.");
				}

				if (outcome == OUT_DEFENDED)
				{
					run.SetFlag("defended", 1);
					run.Note("The crew came for {town_r} and didn't leave.");
					return S_CALL;
				}

				run.SetFlag("raided", 1);
				RZ_BastionQuest.SetWorld(gm, KEY + ".raided", run.Flag("house_r", -1) + 1);
				run.Note("The crew emptied part of {town_r}'s stores. The ration cards told them where to look.");
				return S_CALL;
			}
			case S_CALL:
				return AfterCall(gm, run, outcome);
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterCards(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_CARDS)
		{
			Count(run, run.Flag("house_b", -1));
			if (run.Flag("skimmed") == 1)
				run.Note("Ration cards went to {town_a} and {town_b}. One crate went somewhere else on the way.");
			else
				run.Note("Ration cards went to {town_a} and {town_b}, with a crate each. Both houses are on the books now.");

			return ToCurfew(gm, run);
		}

		if (run.Visits(S_CARDS) < 2)
		{
			run.Note("The crates never reached the houses. {store} will send them out again.");
			return S_CARDS;
		}

		run.SetFlag("no_cards", 1);
		Count(run, run.Flag("house_b", -1));
		run.Note("The ration cards went out without the rations.");
		return ToCurfew(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterCurfew(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_ENFORCED)
		{
			run.SetFlag("enforced", 1);
			run.Note("The curfew was enforced in {town_f}. Doors shut at dusk, and stayed shut.");
		}
		else if (outcome == OUT_GENTLE)
		{
			run.SetFlag("gentle", 1);
			run.Note("The curfew in {town_f} was walked gently: a knock, a word, a lamp turned down.");
		}
		else if (outcome == OUT_TOLL)
		{
			run.SetFlag("toll", 1);
			run.Note("A Red Hand toll stood on {town_f}'s road all night, where the curfew patrol should have been.");
		}
		else if (run.Visits(S_CURFEW) < 2)
		{
			run.Note("Nobody walked the curfew in {town_f}. Tomorrow night, then.");
			return S_CURFEW;
		}
		else
		{
			run.Note("Nobody walked the curfew in {town_f}. The houses kept their own hours.");
		}

		if (PickContribution(gm, run))
			return S_CONTRIB;

		run.Note("There's no counted house left to ask for a contribution.");
		return S_HEARS;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterContribution(RZ_QuestRun run, int outcome)
	{
		switch (outcome)
		{
			case OUT_COLLECTED:
			{
				run.SetFlag("collected", 1);
				run.Note("Four food left {town_c} for Bastion, and went into the ledger as a contribution.");
				break;
			}
			case OUT_RETURNED:
			{
				run.SetFlag("returned", 1);
				run.Note("Four food left {town_c}, and went to another house instead of to Bastion.");
				break;
			}
			case OUT_PAID:
			{
				run.SetFlag("paid", 1);
				run.Note("Someone paid {town_c}'s contribution for them. The house kept its food.");
				break;
			}
			case OUT_FALSIFIED:
			{
				run.SetFlag("falsified", 1);
				run.Note("The ledger says {town_c} paid its contribution. It didn't.");
				break;
			}
			default:
			{
				run.Note("Nobody collected the contribution from {town_c}.");
				break;
			}
		}

		return S_HEARS;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterCall(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_TRUE)
			return Finish(run, E_COUNTED, "Counted. Bastion read {names} names to Command. Command didn't answer.");

		if (outcome == OUT_HIGH || outcome == OUT_LOW)
		{
			int forged = 1;
			if (outcome == OUT_LOW)
				forged = 2;

			run.SetFlag("forged", forged);
			RZ_BastionQuest.SetWorld(gm, KEY + ".forged", forged);
			if (run.Flag("found_out") == 1)
			{
				RZ_BastionQuest.SetWorld(gm, KEY + ".found_out", 1);
				return Finish(run, E_FORGED, "Forged, and found out. {co} has the real numbers now, and knows whose voice read the false ones.");
			}

			return Finish(run, E_FORGED, "Forged. The count went to Command a little wrong, on purpose. Command didn't notice. Nobody at {stronghold} did either.");
		}

		if (outcome == OUT_REFUSED)
		{
			RZ_BastionQuest.SetWorld(gm, KEY + ".refused", 1);
			return Finish(run, E_REFUSED, "Refused. The count is in pieces, and the Haven knows who tore it up.");
		}

		run.Note("Nobody stood by the radio at {stronghold} at 20:30. The old number went out.");
		return S_CALL;
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		int ha = Live(gm, run, "house_a");
		int hb = Live(gm, run, "house_b");
		switch (ending)
		{
			case E_COUNTED:
			{
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 10, 0);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 0, 5);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -5);
				ChangeFood(gm, ha, 2);
				ChangeFood(gm, hb, 2);
				Headline(gm, run.Fill("Bastion read {names} names to Command last night. No reply."));
				SpreadRumour(gm, run.Fill("Bastion has every name in {town_a} and {town_b} on a card now. The trucks stop there first. So will everything else."), 8);
				break;
			}
			case E_FORGED:
			{
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				if (run.Flag("found_out") == 1)
					ChangeStanding(gm, run, RZ_EFaction.MILITARY, 0, 15);
				else
					ChangeStanding(gm, run, RZ_EFaction.MILITARY, 10, 0);

				if (run.Flag("forged") == 1)
				{
					ChangeFood(gm, ha, 2);
					ChangeFood(gm, hb, 2);
				}
				else
				{
					// Low: Bastion's Requisition never comes to these two this season
					int spared = RZ_BastionQuest.World(gm, KEY + ".spared") | RZ_BastionQuest.HouseBit(ha) | RZ_BastionQuest.HouseBit(hb);
					RZ_BastionQuest.SetWorld(gm, KEY + ".spared", spared);
				}

				SpreadRumour(gm, "Bastion's numbers don't add up. Somebody's made sure of it.", 6);
				break;
			}
			case E_REFUSED:
			{
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 0, 10);

				// Requisition comes to these houses first, and a little more often
				int pressed = RZ_BastionQuest.World(gm, KEY + ".pressed") | RZ_BastionQuest.HouseBit(ha) | RZ_BastionQuest.HouseBit(hb);
				RZ_BastionQuest.SetWorld(gm, KEY + ".pressed", pressed);
				RZ_BastionQuest.SetWorld(gm, KEY + ".refused", 1);
				SpreadRumour(gm, run.Fill("Somebody tore up Bastion's count at the radio. {co} wants to know who. The Haven isn't saying."), 8);
				break;
			}
			case E_REDHAND:
			{
				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0, RZ_EQuestApproach.SIDE);
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 0, 15, RZ_EQuestApproach.SIDE);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 0, 10, RZ_EQuestApproach.SIDE);
				NudgeStrength(gm, RZ_EFaction.BANDITS, 3);
				Headline(gm, run.Fill("A Red Hand crew went straight to {town_r}'s stores last night, as if someone had counted them first."));
				break;
			}
		}
	}

	// ================================================================ Words
	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage == S_COUNT && run.Flag("self") == 1)
			return run.Fill("The safehouse in {town_a} put itself on Bastion's census already. Its council head has the list ready: go and take it.");

		if (run.m_iStage == S_CALL && run.Flag("raided") == 1)
			return run.Fill("At 20:30 the count goes to Command from the radio at {stronghold}: as it is, high, low, or not at all. {town_r} is a little lighter since the raid. Command doesn't answer.");

		return super.JournalText(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage <= S_CARDS)
			return "Bastion's counting heads in {town_a}. Says it's for ration cards. Last time they had a list, we ended up at the line.";

		if (run.m_iStage == S_CURFEW)
			return "Curfew in {town_f} tonight. Bastion's orders. In a Haven town.";

		if (run.m_iStage <= S_RAID)
			return "The Red Hand's been asking which houses got Bastion's ration cards. Funny question.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The council head of the first house has an opinion about Bastion's lists.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		if (run.m_iStage > S_CONTRIB || !gm.GetLeaders())
			return "";

		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (!l || l.m_eRole != RZ_ELeaderRole.SAFEHOUSE || l.m_iSafehouse != run.Flag("house_a", -1))
			return "";

		return string.Format("%1 folds their arms. \"Bastion wants our names. On Day 2 they had a list too. We all walked to the line on it.\"", l.m_sShort);
	}
}

//------------------------------------------------------------------------------------------------
//! The Census, stage 1 (Bastion contract, Known; ★ Police): count the first safehouse. Ask its
//! council head for an honest count (Known with the Haven; a house on Bastion's census already
//! hands over its own list), or watch it from a point outside for 45 s unnoticed and write it down
//! (sneak; seen: +3 Haven Infamy). ★ Police: the count is exact either way; otherwise a watched
//! count is a person or two out.
class RZ_QM_CensusCount : RZ_QuestMission
{
	protected int m_iHouse = -1;
	protected vector m_vHead;
	protected vector m_vWatch;
	protected int m_iHeadClue = -1;
	protected int m_iWatchClue = -1;
	protected ref map<int, float> m_mWatched = new map<int, float>();

	protected static const int OBJ_HEAD = 0;
	protected static const int OBJ_WATCH = 1;
	protected static const float WATCH_SECONDS = 45;
	protected static const float WATCH_RADIUS = 12;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 10, 10);
		m_Req.Bonus("police");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_Census.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house_a", -1);
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		if (!h || h.m_iPopulation <= 0 || gm.GetMissions().IsSettlementBusy(h.m_iSettlement))
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vPos = h.m_vPos;
		m_vHead = LandPointNear(h.m_vPos, 5, 12);
		m_vWatch = LandPointNear(h.m_vPos, 70, 110);
		string head = "The council head looks at your clipboard, then at you. \"Ration cards, is it. On Day 2 Bastion had a list too.\" A long breath. \"Ask properly and I'll tell you properly.\"";
		if (QFlag(gm, "self") == 1)
			head = "The council head already has a list written out, every name. \"We're on Bastion's census. That was the deal. Take it.\"";

		m_iHeadClue = AddClue("The council head", head);
		m_iWatchClue = AddClue("A place to watch from", QFill(gm, "A good spot: you can see the door of the house in {town_a}, the window where the bowls are washed, and the yard. Give it a while: who goes in, who comes out, how many bowls."));
		m_Quiet.Watch(h.m_vPos, 140);
		m_sTitle = "The Census: count a safehouse";
		m_sBrief = QFill(gm, "Count the Haven safehouse in {town_a}: people, food, names. For the ration cards. Ask nicely. If they won't say, count them anyway.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string ask = "Ask for an honest count (Known with the Haven)";
		if (QFlag(gm, "self") == 1)
			ask = "Take the house's own list";

		if (!SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vHead, OBJ_HEAD, "", ask, m_iHeadClue, "Talk to the council head"))
			return false;

		return SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vWatch, OBJ_WATCH, "Write down the count", "", m_iWatchClue, "Look around") != null;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vHead);
		result.Insert(m_vWatch);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return GetLocationName(gm) + ", " + RZ_Util.DirectionText(from, m_vPos) + "\nA place to watch it from (grid): " + RZ_SettlementScanner.GridReference(m_vWatch);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			if (!RZ_Arrive.AtSpot(p, m_vWatch, WATCH_RADIUS))
				continue;

			float watched;
			m_mWatched.Find(ids[i], watched);
			m_mWatched.Set(ids[i], watched + dt);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_HEAD || option != 0)
			return;

		if (QFlag(gm, "self") == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "They hand it over without a word. Every name, every mouth, every tin.");
			Counted(gm, playerId, RZ_EQuestApproach.TALK, true, "took the house's own list");
			return;
		}

		if (RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.SURVIVORS) >= 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "They tell you everything, slowly, watching you write it down. \"If those cards turn into a line at a checkpoint, I'll remember your face.\"");
			Counted(gm, playerId, RZ_EQuestApproach.TALK, true, "got an honest count from the council head");
			return;
		}

		Remember(gm, playerId, RZ_EQuestApproach.TALK, "was refused a count");
		gm.NotifyPlayer(playerId, m_sTitle, "\"Bastion's lists were for something else on Day 2. I don't know you.\" The door closes. You could count them from outside.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_WATCH)
			return;

		float watched;
		m_mWatched.Find(playerId, watched);
		if (watched < WATCH_SECONDS)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You haven't watched long enough. Give it a while: who goes in, who comes out, how many bowls at the window.");
			return;
		}

		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			string why = m_Quiet.Why(gm, playerId);
			m_Quiet.Spot(playerId);
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.SURVIVORS, 0, 3);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was seen counting the house");
			gm.NotifyPlayer(playerId, m_sTitle, why + " Someone at the door has been watching you watch them. The house will remember the face with the notebook.");
			return;
		}

		bool exact = RZ_QuestApproach.HasBackground(gm, playerId, "police");
		Counted(gm, playerId, RZ_EQuestApproach.SNEAK, exact, "counted the house from outside, unseen");
	}

	//------------------------------------------------------------------------------------------------
	//! The count as written down: exact, or a person or two out.
	protected void Counted(RZ_GameModeComponent gm, int playerId, int approach, bool exact, string what)
	{
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		RZ_QuestRun run = Run(gm);
		if (!h || !run)
			return;

		int people = h.m_iPopulation;
		int food = h.m_iFood;
		if (!exact)
		{
			people = Math.MaxInt(1, people + m_Rng.Int(-2, 3));
			food = Math.MaxInt(0, food + m_Rng.Int(-2, 3));
		}

		run.SetFlag("exact", 0);
		if (exact)
			run.SetFlag("exact", 1);

		run.SetFlag("count_people", people);
		run.SetText("count_a", string.Format("%1 people, %2 days of food", people, food));
		if (approach == RZ_EQuestApproach.SNEAK)
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You write it down: %1 people, %2 days of food. Near enough.", people, food));

		Resolve(gm, playerId, approach, RZ_Quest_Census.OUT_COUNTED, what);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: {town_a} is counted.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: nobody counted {town_a}.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Census, stage 2 (Bastion contract, Known): ration cards. Two crates at the Bastion seat
//! nearest the first house (the stronghold if none), one for each counted house's storekeeper
//! (+2 food each). Skim one at the store (15 Bastion Favor; 25% the quartermaster notices: +10
//! Bastion Infamy; then only one house gets a crate), or give food from your own pack at a house
//! (+1 food there; Haven Fame +3, once each).
class RZ_QM_CensusCards : RZ_QuestMission
{
	protected ref RZ_QuestCarry m_CrateA = new RZ_QuestCarry();
	protected ref RZ_QuestCarry m_CrateB = new RZ_QuestCarry();
	protected int m_iHouseA = -1;
	protected int m_iHouseB = -1;
	protected vector m_vStore;
	protected vector m_vKeeperA;
	protected vector m_vKeeperB;
	protected bool m_bStoreShown;
	protected bool m_bKeeperAShown;
	protected bool m_bKeeperBShown;
	protected bool m_bSkimmed;
	protected bool m_bGotA;
	protected bool m_bGotB;
	protected int m_iTaken;
	protected ref array<int> m_aGave = {};

	protected static const int OBJ_STORE = 0;
	protected static const int OBJ_A = 1;
	protected static const int OBJ_B = 2;
	protected static const int CRATE_FOOD = 2;
	protected static const int SKIM_FAVOR = 15;
	protected static const float NOTICE_CHANCE = 0.25;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 12, 12);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_Census.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		m_iHouseA = QFlag(gm, "house_a", -1);
		m_iHouseB = QFlag(gm, "house_b", -1);
		RZ_Safehouse ha = RZ_Questline.GetHouse(gm, m_iHouseA);
		RZ_Safehouse hb = RZ_Questline.GetHouse(gm, m_iHouseB);
		if (!run || !ha || !hb)
			return false;

		RZ_Leader seat = RZ_BastionQuest.Nearest(gm, RZ_EFaction.MILITARY, ha.m_vPos, 6000);
		if (!seat)
			seat = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);

		if (!seat)
			return false;

		run.SetText("store", seat.m_sPlace);
		m_vStore = LandPointNear(seat.m_vPos, 5, 10);
		m_vKeeperA = LandPointNear(ha.m_vPos, 4, 9);
		m_vKeeperB = LandPointNear(hb.m_vPos, 4, 9);
		m_iSettlement = gm.FindNearestSettlement(m_vStore);
		m_vPos = m_vStore;
		m_sTitle = "The Census: ration cards";
		m_sBrief = QFill(gm, "Ration cards for {town_a} and {town_b}. Two crates at {store}, one each. On the books. Everything is signed for.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vStore);
		result.Insert(m_vKeeperA);
		result.Insert(m_vKeeperB);
		if (m_CrateA.IsOnGround())
			result.Insert(m_CrateA.GetPos());

		if (m_CrateB.IsOnGround())
			result.Insert(m_CrateB.GetPos());
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return QFill(gm, string.Format("The ration store, {store}: %1\n{town_a}: %2\n{town_b}: %3", RZ_Util.DirectionText(from, m_vStore), RZ_Util.DirectionText(from, m_vKeeperA), RZ_Util.DirectionText(from, m_vKeeperB)));
	}

	//------------------------------------------------------------------------------------------------
	protected int Needed()
	{
		if (m_bSkimmed)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (!m_bStoreShown && RZ_ContractHelp.AnyNear(players, m_vStore, 300))
		{
			m_bStoreShown = true;
			int storeClue = AddClue("The ration store", "Two crates stencilled with the Task Force's mark, and a clipboard. Two lines on it, two signatures to come.");
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vStore, OBJ_STORE, "Take a ration crate", string.Format("Skim a crate for yourself (%1 Bastion Favor)", SKIM_FAVOR), storeClue, "Read the clipboard");
		}

		if (!m_bKeeperAShown && RZ_ContractHelp.AnyNear(players, m_vKeeperA, 300))
		{
			m_bKeeperAShown = true;
			SpawnKeeper(gm, m_vKeeperA, OBJ_A, QText(gm, "town_a"));
		}

		if (!m_bKeeperBShown && RZ_ContractHelp.AnyNear(players, m_vKeeperB, 300))
		{
			m_bKeeperBShown = true;
			SpawnKeeper(gm, m_vKeeperB, OBJ_B, QText(gm, "town_b"));
		}

		if (!m_CrateA.Update(gm, this) || !m_CrateB.Update(gm, this))
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnKeeper(RZ_GameModeComponent gm, vector pos, int objIdx, string town)
	{
		int clue = AddClue("The storekeeper", string.Format("\"Ration cards.\" The storekeeper in %1 turns one over. \"Our names on Bastion's card. And a crate, you say.\"", town));
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, pos, objIdx, "Hand over a ration crate", "Give food from your own pack", clue, "Talk to the storekeeper");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx == OBJ_STORE)
		{
			if (m_iTaken >= Needed())
			{
				gm.NotifyPlayer(playerId, m_sTitle, "The crates are gone. The clipboard has nothing left to sign for.");
				return;
			}

			RZ_QuestCarry crate = m_CrateA;
			if (m_iTaken == 1)
				crate = m_CrateB;

			if (!crate.Spawn(gm, m_iId, RZ_Util.RandomPointInRing(m_vStore, 1, 2)))
				return;

			m_iTaken++;
			Remember(gm, playerId, RZ_EQuestApproach.NONE, "signed for a ration crate");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You sign for it. The crate is at your feet. {town_a} and {town_b} each get one."));
			return;
		}

		if (objIdx != OBJ_A && objIdx != OBJ_B)
			return;

		bool isA = objIdx == OBJ_A;
		if ((isA && m_bGotA) || (!isA && m_bGotB))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "They've had theirs. The other house is waiting.");
			return;
		}

		if (!m_CrateA.TakeFrom(user) && !m_CrateB.TakeFrom(user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You've no ration crate. They're at {store}."));
			return;
		}

		int house = m_iHouseB;
		if (isA)
		{
			house = m_iHouseA;
			m_bGotA = true;
		}
		else
		{
			m_bGotB = true;
		}

		RZ_Questline.ChangeFood(gm, house, CRATE_FOOD);
		gm.NotifyPlayer(playerId, m_sTitle, "The storekeeper signs the card and carries the crate in. Somebody inside scrubs the stencil off it before the children see.");
		int delivered = 0;
		if (m_bGotA)
			delivered++;

		if (m_bGotB)
			delivered++;

		if (delivered >= Needed())
			Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_Census.OUT_CARDS, "delivered the ration cards");
		else
			Remember(gm, playerId, RZ_EQuestApproach.NONE, "delivered a ration crate");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx == OBJ_STORE && option == 0)
		{
			Skim(gm, playerId);
			return;
		}

		if ((objIdx != OBJ_A && objIdx != OBJ_B) || option != 0)
			return;

		if (m_aGave.Contains(playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've given what you could spare.");
			return;
		}

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		array<string> keywords = {};
		if (gm.GetSafehouses())
			keywords = gm.GetSafehouses().GetFoodKeywords();
		else
			RZ_FoodHelper.DefaultKeywords(keywords);

		if (RZ_FoodHelper.TakeFood(user, keywords, 2) <= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've nothing to eat to give them.");
			return;
		}

		m_aGave.Insert(playerId);
		int house = m_iHouseB;
		if (objIdx == OBJ_A)
			house = m_iHouseA;

		RZ_Questline.ChangeFood(gm, house, 1);
		Remember(gm, playerId, RZ_EQuestApproach.TALK, "gave from their own pack");
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, 3, 0, "The house noticed. Not on Bastion's card: from your own pack.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Skim(RZ_GameModeComponent gm, int playerId)
	{
		if (m_bSkimmed || m_iTaken >= 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "There's nothing left to skim.");
			return;
		}

		m_bSkimmed = true;
		SetQFlag(gm, "skimmed", 1);
		Remember(gm, playerId, RZ_EQuestApproach.BETRAY, "skimmed a ration crate");
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, RZ_EFaction.MILITARY, 0, SKIM_FAVOR, "A crate's worth, off the books.");

		if (m_Rng.Chance(NOTICE_CHANCE))
		{
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.MILITARY, 0, 10);
			gm.NotifyPlayer(playerId, m_sTitle, "The quartermaster counts the crates twice, then looks at you. He writes something on the back of the clipboard.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "One crate fewer on the stack. The clipboard still says two. Only one house gets a crate now.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: ration cards issued in {town_a} and {town_b}.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: the ration crates never reached {town_a} and {town_b}.");
	}

	//------------------------------------------------------------------------------------------------
	//! The store until a crate is carried; then the nearest storekeeper still waiting; a crate left
	//! lying, where it lies.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_CrateA.IsCarried() || m_CrateB.IsCarried())
		{
			vector keeper = m_vKeeperA;
			if (m_bGotA || (!m_bGotB && from != vector.Zero && RZ_Util.DistXZ(from, m_vKeeperB) < RZ_Util.DistXZ(from, m_vKeeperA)))
				keeper = m_vKeeperB;

			return RZ_ObjPlace.Spot(keeper);
		}

		if (m_CrateA.IsOnGround())
			return RZ_ObjPlace.Spot(m_CrateA.GetPos());

		if (m_CrateB.IsOnGround())
			return RZ_ObjPlace.Spot(m_CrateB.GetPos());

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_CrateA.Cleanup();
		m_CrateB.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Census, stage 3 (Bastion contract, Known, at night until dawn; ★ Police: the beat goes 1.5
//! times faster): the curfew in the first house's town (the second's if it has emptied). Four
//! patrol points, 15 s at each. At the curfew board: enforce it (+3 Haven Infamy each for the
//! patrol), or patrol gently (★ Police, or Known with the Haven: half pay). At the edge of town a
//! Red Hand toll-man: let him set up for the night (Known with the Red Hand; betray: Red Hand Fame
//! +6, Bastion pays nothing). The crew holds the road till dawn (with the Red Hand group prefab
//! it's real, and it fights strangers).
class RZ_QM_CensusCurfew : RZ_QuestMission
{
	protected ref array<vector> m_aPoints = {};
	protected ref array<float> m_aDwell = {};
	protected ref array<bool> m_aDone = {};
	protected ref RZ_UnitWatch m_Crew = new RZ_UnitWatch();
	protected vector m_vBoard;
	protected vector m_vToll;
	protected int m_iWalked;
	protected bool m_bGentle;
	protected bool m_bToll;
	protected int m_iBoardClue = -1;
	protected int m_iTollClue = -1;

	protected static const int OBJ_BOARD = 0;
	protected static const int OBJ_TOLL = 1;
	protected static const float POINT_RADIUS = 15;
	protected static const float POINT_SECONDS = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 15, 15);
		m_Req.Bonus("police");
		m_fTimeLeft = NightDuration(1200, 3600, 2400);
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_Census.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, QFlag(gm, "house_f", -1));
		RZ_Settlement s = null;
		if (h)
			s = gm.GetSettlement(h.m_iSettlement);

		if (!s || gm.GetMissions().IsSettlementBusy(h.m_iSettlement))
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vPos = s.m_vCenter;
		for (int i = 0; i < 4; i++)
		{
			m_aPoints.Insert(LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.8));
			m_aDwell.Insert(0);
			m_aDone.Insert(false);
		}

		m_vBoard = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.2);
		m_vToll = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 60);
		m_iBoardClue = AddClue("The curfew notice", "TASK FORCE BASTION. CURFEW IN FORCE FROM DUSK. Residents will remain indoors. Lights to be kept low. Persons abroad after dark will be questioned. By order.");
		m_iTollClue = AddClue("A Red Hand toll-man", "He's leaning on the signpost as if it's his. \"Bastion's curfew, eh? Nobody on the road all night but you and me.\" He grins. \"Seems a shame to waste a road like that.\"");
		m_sTitle = "The Census: the curfew";
		m_sBrief = QFill(gm, "Curfew in {town_f} tonight, by order. Walk the beat, four points. How firmly is up to you, within reason.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vBoard, OBJ_BOARD, "", "Enforce the curfew|Patrol gently (a Police officer, or Known with the Haven)", m_iBoardClue, "Read the curfew notice"))
			return false;

		return SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vToll, OBJ_TOLL, "", "Let the Red Hand set up a toll tonight (Known with the Red Hand)", m_iTollClue, "Talk to the toll-man") != null;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vBoard);
		result.Insert(m_vToll);
		foreach (vector p : m_aPoints)
		{
			result.Insert(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = GetLocationName(gm) + ", " + RZ_Util.DirectionText(from, m_vPos) + "\nPatrol points (grid): ";
		foreach (int i, vector p : m_aPoints)
		{
			if (i > 0)
				text += ", ";

			text += RZ_SettlementScanner.GridReference(p);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved || m_bToll)
			return;

		foreach (int i, vector p : m_aPoints)
		{
			if (m_aDone[i] || RZ_Arrive.CountAtSpot(players, p, POINT_RADIUS) == 0)
				continue;

			RZ_ContractHelp.AddNear(ids, players, p, POINT_RADIUS, m_aParticipants);
			float rate = 1;
			if (AnyBonusNear(gm, ids, players, p, POINT_RADIUS))
				rate = 1.5;

			m_aDwell[i] = m_aDwell[i] + dt * rate;
			if (m_aDwell[i] < POINT_SECONDS)
				continue;

			m_aDone[i] = true;
			m_iWalked++;
			if (m_iWalked < m_aPoints.Count())
				NotifyParticipants(gm, m_sTitle, string.Format("Patrol point %1 of %2.", m_iWalked, m_aPoints.Count()));
		}

		if (m_iWalked < m_aPoints.Count() || m_aParticipants.IsEmpty())
			return;

		int who = m_aParticipants[0];
		if (m_bGentle)
		{
			m_fRewardScale = 0.5;
			NotifyParticipants(gm, m_sTitle, QFill(gm, "The beat is walked: a knock, a word, a lamp turned down. {town_f} sleeps, and nobody in it is afraid of you."));
			Resolve(gm, who, RZ_EQuestApproach.TALK, RZ_Quest_Census.OUT_GENTLE, "patrolled the curfew gently");
			return;
		}

		foreach (int playerId : m_aParticipants)
		{
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "enforced the curfew");
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.SURVIVORS, 0, 3);
		}

		NotifyParticipants(gm, m_sTitle, QFill(gm, "The beat is walked. Every door in {town_f} is shut, and stays shut. Behind them, people remember who knocked."));
		Resolve(gm, who, RZ_EQuestApproach.FORCE, RZ_Quest_Census.OUT_ENFORCED, "enforced the curfew");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_bToll)
			return;

		if (objIdx == OBJ_BOARD)
		{
			if (option == 0)
			{
				m_bGentle = false;
				gm.NotifyPlayer(playerId, m_sTitle, "By the book, then. Anyone out after dusk gets questioned.");
				return;
			}

			if (option != 1)
				return;

			if (!RZ_QuestApproach.HasBackground(gm, playerId, "police") && RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.SURVIVORS) < 1)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "The houses don't know you well enough to open the door to a soft knock. It's the book or nothing.");
				return;
			}

			m_bGentle = true;
			gm.NotifyPlayer(playerId, m_sTitle, "Gently, then. A knock, a word, a lamp turned down. Half of it won't go in the report.");
			return;
		}

		if (objIdx != OBJ_TOLL || option != 0)
			return;

		if (RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.BANDITS) < 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You? I don't know you. Walk your little beat.");
			return;
		}

		LetTollIn(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The toll holds the road till dawn: the stage ends then, as the Red Hand's.
	protected void LetTollIn(RZ_GameModeComponent gm, int playerId)
	{
		m_bToll = true;
		m_vPos = m_vToll;
		AddParticipant(playerId);
		Remember(gm, playerId, RZ_EQuestApproach.BETRAY, "let a Red Hand toll in for the night");
		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, -1, 6, 0);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetOutcome(RZ_Quest_Census.OUT_TOLL);

		ResourceName crew = gm.GetRedHandGroupPrefab();
		if (!crew.IsEmpty() && m_Crew.Spawn(crew, LandPointNear(m_vToll, 6, 15)))
		{
			TrackProp(m_Crew.m_Unit);
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew.m_Unit, 80, true);
		}

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He whistles, and a car wreck rolls across the road. \"Pleasure doing business. The night's ours.\" Nobody walks the curfew in {town_f} tonight."));
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: the curfew in {town_f} is walked.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bToll)
			return QFill(gm, "The Census: a Red Hand toll held {town_f}'s road till dawn.");

		return QFill(gm, "The Census: nobody walked the curfew in {town_f}.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Crew.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! The Census, stage 4 (Bastion contract, Known, 🔒 not Elder): four food from a counted house.
//! Collect it (the house -4 food) and carry it to Bastion's clerk at the nearest Bastion seat, or
//! give it to another house instead (that house +4; Haven Fame +4, Bastion Infamy +5; no pay); pay
//! it yourself (15 Bastion Favor); or falsify the ledger (Trusted, or ★ Police; 25% found out at the
//! nightly call).
class RZ_QM_CensusContrib : RZ_QuestMission
{
	protected ref RZ_QuestCarry m_Sack = new RZ_QuestCarry();
	protected int m_iHouse = -1;
	protected int m_iOther = -1;
	protected vector m_vKeeper;
	protected vector m_vClerk;
	protected vector m_vOther;
	protected string m_sClerkPlace;
	protected string m_sOther;
	protected bool m_bClerkShown;
	protected bool m_bOtherShown;
	protected bool m_bCollected;
	protected bool m_bReturned;

	protected static const int OBJ_KEEPER = 0;
	protected static const int OBJ_CLERK = 1;
	protected static const int OBJ_OTHER = 2;
	protected static const int FOOD = 4;
	protected static const int PAY_FAVOR = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 12, 15);
		m_Req.Deny("elder");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_Census.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house_c", -1);
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		if (!h || h.m_iPopulation <= 0 || RZ_Quest_Census.IsChairProtected(gm, m_iHouse))
			return false;

		RZ_Leader seat = RZ_BastionQuest.Nearest(gm, RZ_EFaction.MILITARY, h.m_vPos, 6000);
		if (!seat)
			seat = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);

		if (!seat)
			return false;

		m_iOther = QFlag(gm, "house_b", -1);
		if (m_iOther == m_iHouse)
			m_iOther = QFlag(gm, "house_a", -1);

		RZ_Safehouse other = RZ_Questline.GetHouse(gm, m_iOther);
		if (!other || m_iOther == m_iHouse)
		{
			m_iOther = RZ_LeaderJob.SafehouseNear(gm, h.m_vPos, 0, 100000, m_iHouse);
			other = RZ_Questline.GetHouse(gm, m_iOther);
		}

		if (!other)
			return false;

		m_vKeeper = LandPointNear(h.m_vPos, 4, 9);
		m_vClerk = LandPointNear(seat.m_vPos, 5, 10);
		m_vOther = LandPointNear(other.m_vPos, 4, 9);
		m_sClerkPlace = seat.m_sPlace;
		m_sOther = other.m_sName;
		m_iSettlement = h.m_iSettlement;
		m_vPos = m_vKeeper;
		m_sTitle = "The Census: the contribution";
		m_sBrief = QFill(gm, "The census says {town_c} can spare four food. Collect the contribution. It's for their protection.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		int clue = AddClue("The storekeeper", QFill(gm, "The storekeeper in {town_c} has the census card in her hand already. \"Four. It says four. For our protection.\" She doesn't move toward the shelves."));
		string labels = string.Format("Collect four food for Bastion|Pay it yourself (%1 Bastion Favor)|Falsify the ledger (Trusted, or a Police officer)", PAY_FAVOR);
		return SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vKeeper, OBJ_KEEPER, "", labels, clue, "Talk to the storekeeper") != null;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vKeeper);
		if (m_Sack.IsOnGround())
			result.Insert(m_Sack.GetPos());

		if (m_bCollected)
		{
			result.Insert(m_vClerk);
			result.Insert(m_vOther);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = QFill(gm, "{town_c}: ") + RZ_Util.DirectionText(from, m_vKeeper);
		if (m_bCollected)
			text += string.Format("\nBastion's clerk, %1: %2\nOr the safehouse in %3: %4", m_sClerkPlace, RZ_Util.DirectionText(from, m_vClerk), m_sOther, RZ_Util.DirectionText(from, m_vOther));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved || !m_bCollected)
			return;

		if (!m_Sack.Update(gm, this))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_bClerkShown && RZ_ContractHelp.AnyNear(players, m_vClerk, 300))
		{
			m_bClerkShown = true;
			int clerkClue = AddClue("Bastion's clerk", "\"Contributions here. Sign there.\" He has a ledger, a pencil and no curiosity at all.");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vClerk, OBJ_CLERK, "Hand over the contribution", "", clerkClue, "Talk to the clerk");
		}

		if (!m_bOtherShown && RZ_ContractHelp.AnyNear(players, m_vOther, 300))
		{
			m_bOtherShown = true;
			int otherClue = AddClue("The storekeeper", string.Format("The storekeeper in %1 looks at what you're carrying. \"Is that for us?\"", m_sOther));
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vOther, OBJ_OTHER, "Give the food to this house instead", "", otherClue, "Talk to the storekeeper");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_KEEPER || m_bCollected)
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "She looks at you a long time. \"You're not here for Bastion. Go home.\"");
			return;
		}

		if (option == 0)
		{
			if (!m_Sack.Spawn(gm, m_iId, RZ_Util.RandomPointInRing(m_vKeeper, 1, 2)))
				return;

			RZ_Questline.ChangeFood(gm, m_iHouse, -FOOD);
			m_bCollected = true;
			m_vPos = m_vClerk;
			Remember(gm, playerId, RZ_EQuestApproach.NONE, "collected the contribution");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("She fills the sack herself, slowly, and doesn't hand it to you: she puts it on the ground. Bastion's clerk is at %1. Another house, in %2, would take it too.", m_sClerkPlace, m_sOther));
			return;
		}

		if (option == 1)
		{
			if (!TryBribe(gm, playerId, RZ_EFaction.MILITARY, PAY_FAVOR))
				return;

			gm.NotifyPlayer(playerId, m_sTitle, "You settle it with the quartermaster yourself. The ledger says \"paid\". The shelves stay full.");
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_Census.OUT_PAID, "paid the contribution themselves");
			return;
		}

		if (option != 2)
			return;

		if (!RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.MILITARY, 2, "police"))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You'd need the stronghold's trust (or a policeman's hand) to make a ledger lie convincingly.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "You write \"4, received\" in a hand that isn't yours, and initial it. The storekeeper watches you do it and says nothing at all.");
		Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_Census.OUT_FALSIFIED, "falsified the ledger");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_CLERK && objIdx != OBJ_OTHER)
			return;

		if (!m_Sack.TakeFrom(user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You haven't got the contribution. It's at {town_c}."));
			return;
		}

		if (objIdx == OBJ_CLERK)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Four. Received.\" He writes it down. That's all.");
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Census.OUT_COLLECTED, "delivered the contribution");
			return;
		}

		m_bReturned = true;
		RZ_Questline.ChangeFood(gm, m_iOther, FOOD);
		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, 4, 5);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("The storekeeper in %1 doesn't ask where it came from. Bastion's ledger has a gap where four food should be.", m_sOther));
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_Census.OUT_RETURNED, "gave the contribution to another house", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: {town_c}'s contribution is in the ledger.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bReturned)
			return string.Format("The Census: the contribution went to %1 instead. Bastion won't thank you.", m_sOther);

		return QFill(gm, "The Census: nobody collected the contribution from {town_c}.");
	}

	//------------------------------------------------------------------------------------------------
	//! The keeper; the sack where it lies, then (carried) Bastion's clerk.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		RZ_ObjPlace at = m_Sack.ObjPlace(m_vClerk);
		if (at)
			return at;

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Sack.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Census, stage 5 (Bastion contract, Known; only with the Red Hand group prefab): a crew
//! raids a counted house (the one with the fuller stores). It turns up 250-350 m out when someone
//! comes near, and walks to the stores 45 s later. Defend it (kill the crew: +5 Red Hand Infamy);
//! hold the stores for them 90 s (Known with the Red Hand; the Red Hand's count: Red Hand Fame +5
//! and 30 Favor; Bastion pays nothing); or let them load up 90 s. Raided: the house -3 food.
class RZ_QM_CensusRaid : RZ_QuestMission
{
	protected ref RZ_UnitWatch m_Crew = new RZ_UnitWatch();
	protected int m_iHouse = -1;
	protected vector m_vStores;
	protected bool m_bShown;
	protected bool m_bSent;
	protected float m_fSendIn = 45;
	protected float m_fLoading;
	protected float m_fHeld;
	protected int m_iSider = -1;

	protected static const int OBJ_STORES = 0;
	protected static const float LOAD_SECONDS = 90;
	protected static const float STORES_RADIUS = 20;
	protected static const int RAID_FOOD = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 15, 15);
		SetRival(RZ_EFaction.BANDITS, 5);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 120;
		m_iLeaderId = RZ_Quest_Census.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house_r", -1);
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		if (!h || h.m_iPopulation <= 0 || gm.GetRedHandGroupPrefab().IsEmpty() || gm.GetMissions().IsSettlementBusy(h.m_iSettlement))
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vStores = LandPointNear(h.m_vPos, 3, 8);
		m_vPos = m_vStores;
		m_sTitle = "The Census: the raid";
		m_sBrief = QFill(gm, "Red Hand chatter: a crew is going for the stores in {town_r}. Somebody told them the cards mean full shelves. Get there first.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		int clue = AddClue("The stores", QFill(gm, "Shelves, sacks and a padlock that wouldn't stop anyone. Bastion's ration card is pinned to the door: {town_r}, every name, every tin."));
		return SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vStores, OBJ_STORES, "", "Hold the stores for the crew (Known with the Red Hand)", clue, "Look at the stores") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (!m_bShown)
		{
			if (!RZ_ContractHelp.AnyNear(players, m_vStores, 400))
				return;

			m_bShown = true;
			if (!m_Crew.Spawn(gm.GetRedHandGroupPrefab(), LandPointNear(m_vStores, 250, 350)))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			TrackProp(m_Crew.m_Unit);
		}

		if (m_Crew.IsDead(dt))
		{
			Defended(gm, ids, players);
			return;
		}

		if (!m_bSent)
		{
			m_fSendIn -= dt;
			if (m_fSendIn > 0)
				return;

			m_bSent = true;
			RZ_Util.SendGroupTo(m_Crew.m_Unit, m_vStores, gm.GetSpawner().GetMoveWaypointPrefab());
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew.m_Unit, -1, true);
			NotifyParticipants(gm, m_sTitle, "Engines on the road, then boots. The crew is coming in.");
			return;
		}

		// Someone holds the door for them
		if (m_iSider > 0)
		{
			IEntity sider = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iSider);
			if (sider && RZ_Util.DistXZ(sider.GetOrigin(), m_vStores) <= STORES_RADIUS)
				m_fHeld += dt;

			if (m_fHeld >= LOAD_SECONDS)
			{
				Raided(gm);
				RZ_QuestApproach.SideWith(gm, m_iSider, RZ_EFaction.BANDITS, RZ_EFaction.MILITARY, 5, 5);
				RZ_QuestRun run = Run(gm);
				if (run)
					run.Reward(gm, m_iSider, RZ_EFaction.BANDITS, 0, 30, "The crew's cut, for holding the door.");

				NotifyParticipants(gm, m_sTitle, QFill(gm, "The crew loads up and goes, waving. {town_r}'s shelves are half empty, and the ration card is still on the door."));
				Resolve(gm, m_iSider, RZ_EQuestApproach.SIDE, RZ_Quest_Census.OUT_SIDED, "held the stores for the Red Hand", false);
			}

			return;
		}

		// Nobody stops them: they load up
		if (RZ_Util.DistXZ(m_Crew.GetPos(m_vStores), m_vStores) <= STORES_RADIUS + 10)
			m_fLoading += dt;

		if (m_fLoading < LOAD_SECONDS)
			return;

		Raided(gm);
		NotifyParticipants(gm, m_sTitle, QFill(gm, "The crew loads up and goes. {town_r}'s shelves are half empty."));
		int who = -1;
		if (!m_aParticipants.IsEmpty())
			who = m_aParticipants[0];

		Resolve(gm, who, RZ_EQuestApproach.NONE, RZ_Quest_Census.OUT_RAIDED, "watched the stores go", false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Defended(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		array<int> near = {};
		GetPlayersNear(ids, players, m_vStores, 200, near);
		int who = -1;
		foreach (int playerId : near)
		{
			if (playerId == m_iSider)
				continue;

			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "fought off the Red Hand crew");
			if (who < 0)
				who = playerId;
		}

		NotifyParticipants(gm, m_sTitle, QFill(gm, "The crew is down. {town_r} keeps its stores, and its ration card."));
		Resolve(gm, who, RZ_EQuestApproach.FORCE, RZ_Quest_Census.OUT_DEFENDED, "fought off the Red Hand crew");
	}

	//------------------------------------------------------------------------------------------------
	protected void Raided(RZ_GameModeComponent gm)
	{
		RZ_Questline.ChangeFood(gm, m_iHouse, -RAID_FOOD);
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody stopped them in time (or nobody came): the crew had the stores anyway.
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		if (m_bResolved)
			return;

		Raided(gm);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetOutcome(RZ_Quest_Census.OUT_RAIDED);

		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_STORES || option != 0 || m_iSider > 0)
			return;

		if (RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.BANDITS) < 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The crew doesn't know you. If you stand at the door, you'll get shot along with it.");
			return;
		}

		m_iSider = playerId;
		Remember(gm, playerId, RZ_EQuestApproach.SIDE, "held the stores for the Red Hand");
		gm.NotifyPlayer(playerId, m_sTitle, "You stand by the door and wave them in. Stay with the stores while they load up.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: {town_r} kept its stores.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: the Red Hand emptied part of {town_r}'s stores.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Crew.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! The Census, stage 6 (Bastion contract, Known): the nightly call. At the radio by the
//! commanding officer's seat, from 20:30 to 22:30: read the count as it is, read it high (more
//! rations for the houses) or low (fewer collections) (forged: 25% found out, and then no pay), or
//! tear it up (side with the Haven: Haven Fame +5, no pay). A falsified ledger from stage 4 is found
//! out here 25% of the time: the house pays after all, and whoever falsified it (if online) takes
//! +10 Bastion Infamy. Command doesn't answer.
class RZ_QM_CensusCall : RZ_QuestMission
{
	protected vector m_vRadio;
	protected int m_iSheetClue = -1;
	protected int m_iStrength;
	protected int m_iNames;

	protected static const int OBJ_RADIO = 0;
	protected static const float FOUND_CHANCE = 0.25;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 10, 10);
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_Census.Voice(gm);
		m_fTimeLeft = 3600;
		float secs = RZ_GameTime.RealSecondsUntil(22.5);
		if (secs > 0)
			m_fTimeLeft = Math.Clamp(secs + 300, 900, 5400);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!run || !co)
			return false;

		m_vRadio = LandPointNear(co.m_vPos, 4, 8);
		m_iSettlement = gm.FindNearestSettlement(m_vRadio);
		m_vPos = m_vRadio;

		// What the call reads out: the people in the counted houses, and the Task Force's strength
		m_iNames = run.Flag("count_people");
		RZ_Safehouse hb = RZ_Questline.GetHouse(gm, run.Flag("house_b", -1));
		if (hb)
			m_iNames += hb.m_iPopulation;

		RZ_Safehouse ha = RZ_Questline.GetHouse(gm, run.Flag("house_a", -1));
		if (run.Flag("count_people") <= 0 && ha)
			m_iNames += ha.m_iPopulation;

		m_iStrength = m_Rng.Int(28, 47);
		run.SetText("names", RZ_BastionQuest.NumberWords(m_iNames));
		m_iSheetClue = AddClue("The count", QFill(gm, string.Format("Typed, then corrected in pencil. \"Civilians sheltered: {town_a} and {town_b}, %1 in all. Rations issued. Contributions received.\" Space at the bottom for whoever reads it to Command.", RZ_BastionQuest.NumberWords(m_iNames))));
		m_sTitle = "The Census: the nightly call";
		m_sBrief = QFill(gm, "Stand by the radio at {stronghold} at 20:30 and give Command the count. It's the first one in weeks that's real.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		RZ_GuidanceSettings guide = gm.GetGuidanceSettings();
		string labels = "Read the count as it is|Read it high (more rations for the houses)|Read it low (fewer collections)|Tear up the count";
		IEntity radio = SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vRadio, OBJ_RADIO, "", labels, m_iSheetClue, "Read the count");
		if (radio && guide && !guide.m_sTableRadioPrefab.IsEmpty())
			TrackProp(RZ_Util.SpawnPrefab(guide.m_sTableRadioPrefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(m_vRadio, 0.8, 1.2)) + Vector(0, 0.3, 0)));

		return radio != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_RADIO || option < 0 || option > 3)
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The operator puts his hand over the handset. \"Task Force only.\"");
			return;
		}

		if (!RZ_Quest_Census.IsCallTime())
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("Not yet. The call goes out at 20:30. It's %1.", RZ_GameTime.Clock(RZ_GameTime.GetHour())));
			return;
		}

		RZ_QuestRun run = Run(gm);
		if (!run)
			return;

		CheckLedger(gm, run);
		string hiss = " A long hiss. \"Bastion out.\" The operator writes \"No reply\", and the sergeant chalks up one more day.";
		if (option == 3)
		{
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.SURVIVORS, -1, 5, 0);
			gm.NotifyPlayer(playerId, m_sTitle, "You tear the count in half, then in half again, and drop it in the stove. The operator reads last week's number instead." + hiss);
			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_Census.OUT_REFUSED, "tore up the count", false);
			return;
		}

		int names = m_iNames;
		int outcome = RZ_Quest_Census.OUT_TRUE;
		string what = "read the true count to Command";
		if (option == 1)
		{
			names = m_iNames + m_Rng.Int(8, 15);
			outcome = RZ_Quest_Census.OUT_HIGH;
			what = "read the count high";
		}
		else if (option == 2)
		{
			names = Math.MaxInt(1, m_iNames - m_Rng.Int(4, 8));
			outcome = RZ_Quest_Census.OUT_LOW;
			what = "read the count low";
		}

		run.SetText("names", RZ_BastionQuest.NumberWords(names));
		string text = string.Format("Command, Bastion. Effective strength %1. Civilians sheltered, %2. Requesting relief. Over.", RZ_BastionQuest.NumberWords(m_iStrength), RZ_BastionQuest.NumberWords(names)) + hiss;
		if (outcome == RZ_Quest_Census.OUT_TRUE)
		{
			run.Reward(gm, playerId, RZ_EFaction.MILITARY, 5, 30, "Everything is signed for.");
			gm.NotifyPlayer(playerId, m_sTitle, text);
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, outcome, what);
			return;
		}

		bool found = m_Rng.Chance(FOUND_CHANCE);
		if (found)
		{
			run.SetFlag("found_out", 1);
			gm.NotifyPlayer(playerId, m_sTitle, text + " The duty officer has the typed sheet in his hand, and he heard every number.");
		}
		else
		{
			gm.NotifyPlayer(playerId, m_sTitle, text);
		}

		Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, outcome, what, !found);
	}

	//------------------------------------------------------------------------------------------------
	//! The falsified ledger from the contribution: 25% the quartermaster checks it tonight.
	protected void CheckLedger(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.Flag("falsified") != 1 || run.Flag("ledger_checked") == 1)
			return;

		run.SetFlag("ledger_checked", 1);
		if (!m_Rng.Chance(FOUND_CHANCE))
			return;

		run.SetFlag("ledger_found", 1);
		RZ_Questline.ChangeFood(gm, run.Flag("house_c", -1), -4);
		run.Note("The quartermaster checked the ledger against the shelves. {town_c} paid its contribution after all, twice over in shame.");
		array<string> idents = {};
		run.GetIdents(idents);
		foreach (string ident : idents)
		{
			if (!RZ_BastionQuest.DidAt(run, ident, RZ_EQuestApproach.BETRAY, RZ_Quest_Census.S_CONTRIB))
				continue;

			int falsifier = gm.GetQuestlines().OnlineId(ident);
			if (falsifier <= 0)
				continue;

			RZ_QuestApproach.SideWith(gm, falsifier, -1, RZ_EFaction.MILITARY, 0, 10);
			gm.NotifyPlayer(falsifier, m_sTitle, run.Fill("The quartermaster checked {town_c}'s ledger against the shelves. The initials on the false line are yours."));
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: the count went out from {stronghold}. Command didn't answer.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Census: the count didn't go out the way {co} wanted.");
	}
}
