// Test-only (not part of the mod): the world state the runtime test forces so that every built
// questline starts and ends once, and so that every mission type can start in a fresh test world:
// stand-ins for the optional prefabs a mission needs, a spread of town infection before each one,
// trucks for the convoys and Mobile Lab (designs/PLAN.md, step 0.4). Each forced change is logged
// as "RZT forced: ...".

modded class RZ_Questlines
{
	//! No questline starts by itself: the test begins each one in turn.
	void RZT_NoAutoStart()
	{
		m_Settings.m_fStartChance = 0;
	}

	//! Ends every running questline (anything the mission phase or a stuck test left behind).
	void RZT_EndAll()
	{
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (run.m_bDone)
				continue;

			run.m_iEnding = -1;
			run.m_sEndText = "";
			EndRun(run);
		}
	}

	//! Ends one running questline, if it still runs.
	void RZT_End(string key)
	{
		RZ_QuestRun run = FindRunByKey(key);
		if (!run || run.m_bDone)
			return;

		run.m_iEnding = -1;
		run.m_sEndText = "";
		EndRun(run);
	}
}

modded class RZ_Seasons
{
	//! The season started `days` real days earlier (The Book waits for the first real day).
	void RZT_Age(int days)
	{
		if (m_iStartUnix > 0)
			m_iStartUnix = m_iStartUnix - days * DAY;
	}
}

modded class RZ_FactionLife
{
	void RZT_SetNeed(int f, int need)
	{
		if (RZ_Factions.IsValid(f))
			m_aNeed[f] = need;
	}
}

modded class RZ_LongTable
{
	//! A house this season, whatever the roll said. False if the Long Table is off or the map has
	//! no place for one.
	bool RZT_Ensure()
	{
		if (!m_Settings || !m_Settings.m_bEnabled)
			return false;

		if (!Exists())
		{
			Found();
			Publish();
		}

		return Exists();
	}

	//! The host has fed this player twice and offers them a place (A Place at the Table).
	bool RZT_Offer(int playerId)
	{
		RZ_TableMemory mem = Memory(playerId);
		if (!mem)
			return false;

		mem.m_iMeals = 2;
		mem.m_iFlags = 0;
		m_sOfferIdent = mem.m_sIdent;
		return true;
	}

	void RZT_ClearOffer()
	{
		m_sOfferIdent = "";
	}
}

modded class RZ_Leaders
{
	//! Every empty seat filled now (a season replaces them after Leaders > Replace minutes; the
	//! mission phase kills and dismisses seats faster than that), and every leader a finished
	//! mission still holds let go. Returns how many seats changed.
	int RZT_FillSeats()
	{
		int changed = 0;
		for (int i = 0; i < m_aList.Count(); i++)
		{
			RZ_Leader l = m_aList[i];
			if (!l.m_bAlive && !l.IsStronghold())
			{
				m_aList[i] = Replace(l);
				changed++;
				continue;
			}

			if (l.m_iHeldBy < 0)
				continue;

			RZ_Mission m = m_GM.GetMissions().FindById(l.m_iHeldBy);
			if (!m || m.m_eState != RZ_EMissionState.ACTIVE)
			{
				l.m_iHeldBy = -1;
				changed++;
			}
		}

		return changed;
	}

	//! "present P (held H, stronghold S), dead D" for one faction (the questlines' diagnostics).
	string RZT_Seats(int f)
	{
		int present = 0;
		int held = 0;
		int strong = 0;
		int dead = 0;
		foreach (RZ_Leader l : m_aList)
		{
			if (l.m_eFaction != f)
				continue;

			if (!l.IsPresent())
			{
				dead++;
				continue;
			}

			present++;
			if (l.m_iHeldBy >= 0)
				held++;

			if (l.IsStronghold())
				strong++;
		}

		return string.Format("present %1 (held %2, stronghold %3), away %4", present, held, strong, dead);
	}
}

modded class RZ_Wanderers
{
	//! A living, free wanderer of this role: one is given it if none has it.
	bool RZT_EnsureRole(RZ_EWandererRole role)
	{
		RZ_Wanderer pick;
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || w.m_iHeldBy >= 0)
				continue;

			if (w.m_eRole == role)
				return true;

			if (!pick)
				pick = w;
		}

		if (!pick)
			return false;

		pick.m_eRole = role;
		return true;
	}
}

modded class RZ_CureResearch
{
	void RZT_SetProgress(float progress)
	{
		m_fProgress = progress;
		m_GM.OnCureChanged();
	}

	void RZT_StartStand()
	{
		if (!m_bFinalStand && !m_bCured)
			StartFinalStand();
	}

	//! Ends a stand the test started, and puts research back where it was.
	void RZT_StopStand(float progress)
	{
		m_bFinalStand = false;
		m_bGateOpen = false;
		m_fProgress = progress;
		m_GM.OnCureChanged();
	}
}

class RZT_Prep
{
	// Vanilla stand-ins, as the mod's own supply prefabs use (Prefabs/ReforgerZ/Supply/)
	static const string BASTION_TRUCK = "{81FDAD5EB644CC3D}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport_covered.et";
	static const string BASTION_CREW = "{3BF36BDEEB33AEC9}Prefabs/Groups/BLUFOR/Group_US_SentryTeam.et";
	static const string REDHAND_TRUCK = "{C75DAB159B434B77}Prefabs/Vehicles/Wheeled/UAZ452/UAZ452_cargo_CIV_grey.et";
	static const string HALCYON_TRUCK = "{43C4AF1EEBD001CE}Prefabs/Vehicles/Wheeled/UAZ452/UAZ452_ambulance.et";
	static const string KEYPAD = "{7030822F7A54F479}Prefabs/Props/Crates/CrateWooden_01_base.et";
	// Stand-ins for the optional prefab slots the test world leaves empty
	static const string PROP = "{7030822F7A54F479}Prefabs/Props/Crates/CrateWooden_01_base.et";
	static const string PERSON = "{8C7093AF368F496A}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_CottonShirt_1.et";
	static const string GROUP = "{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et";

	// How many times the mission phase tries a Setup that picks a random spot (dry ground, a free town)
	static const int TRIES = 8;

	// The hour the questline phase holds: dusk (The Edge of Town), night (Await Relief's tape, the
	// Census's curfew), the nightly call (19:00-22:30) and supper at the Long Table all at once
	static const float HOUR = 20.75;

	protected static float s_fCureBefore = -1;
	protected static bool s_bSpreadSaid;
	protected static int s_iHavenFame = -1;
	protected static int s_iHavenInfamy;

	// ================================================================ The mission phase
	//------------------------------------------------------------------------------------------------
	//! Before the mission phase starts a mission of type t: what it needs that a fresh test world lacks.
	//! shift: the go (0 the first); each go after the first moves the spread on by one town.
	static void BeforeMission(RZ_GameModeComponent gm, typename t, int shift = 0)
	{
		string name = t.ToString();

		// Every mission starts from the same spread of towns, whatever the missions before it did to
		// them (Mobile Lab's calm towns would otherwise leave nothing for the 20-90 infection range).
		// Each further go shifts it by one, so on a map where nearly every town is lived in (Arland:
		// 13 of 14) the few empty ones still pass through every band
		array<float> spread = {20, 45, 75, 95};
		foreach (int idx, RZ_Settlement st : gm.GetSettlements())
		{
			st.m_fInfection = spread[(idx + shift) % spread.Count()];
		}

		if (!s_bSpreadSaid)
		{
			s_bSpreadSaid = true;
			Print("RZT forced: before each mission, towns at infection 20, 45, 75 and 95 in turn (shifted by one town on each further go)");
		}

		StandIns(gm, name);
		if (name == "RZ_Mission_ConvoyEscort" || name == "RZ_Mission_HijackTheConvoy")
		{
			// Both ride Bastion's next supply truck: Bastion's trucks must run
			RZ_SupplySettings ss = gm.GetSupplySettings();
			if (ss && ss.m_sBastionTruck.IsEmpty())
				ss.m_sBastionTruck = BASTION_TRUCK;
			if (ss && ss.m_sBastionCrew.IsEmpty())
				ss.m_sBastionCrew = BASTION_CREW;

			if (shift == 0)
				Print("RZT forced: Bastion supply truck and crew for " + name);
			return;
		}

		if (name == "RZ_Mission_MobileLab")
		{
			// A quiet town 1-4 km from the lab, and a truck to drive there
			RZ_SupplySettings labSupply = gm.GetSupplySettings();
			if (labSupply && labSupply.m_sHalcyonTruck.IsEmpty())
				labSupply.m_sHalcyonTruck = HALCYON_TRUCK;

			int calmed = 0;
			if (gm.HasLab())
			{
				vector lab = gm.GetLabPosition();
				foreach (RZ_Settlement s : gm.GetSettlements())
				{
					float d = RZ_Util.DistXZ(lab, s.m_vCenter);
					if (d >= 1000 && d <= 4000 && s.m_fInfection > 25)
					{
						s.m_fInfection = 15;
						calmed++;
					}
				}
			}

			if (shift == 0)
				Print(string.Format("RZT forced: Halcyon truck, %1 towns 1-4 km from the lab at infection 15, for %2", calmed, name));
			return;
		}

		if (name == "RZ_Event_LockedBunker")
		{
			// The keypad prop, and an empty town that isn't overrun
			gm.RZT_StandIn("keypad", KEYPAD);
			foreach (RZ_Settlement town : gm.GetSettlements())
			{
				if (town.m_fInfection > 90)
					town.m_fInfection = 60;
			}

			if (shift == 0)
				Print("RZT forced: a keypad prefab and no town above 90 infection, for " + name);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The optional prefab a mission needs (its GetWeight is 0 without it), as a vanilla stand-in:
	//! a crate for a prop, a civilian for a person, an FIA fire team for a group.
	protected static void StandIns(RZ_GameModeComponent gm, string name)
	{
		string slot;
		string prefab = PROP;
		if (name == "RZ_Mission_BurnTheNest" || name == "RZ_Mission_SpecimenFromTheNest")
			slot = "nest";
		else if (name == "RZ_Mission_CrashSite")
			slot = "heliwreck";
		else if (name == "RZ_Mission_BanditDeadDrop")
			slot = "deaddrop";
		else if (name == "RZ_Mission_Arson")
			slot = "fueldepot";
		else if (name == "RZ_Mission_SabotageRelay")
			slot = "relay";
		else if (name == "RZ_Mission_SmugglersLedger")
			slot = "landmark";
		else if (name == "RZ_Mission_WeighingVault")
			slot = "scale";
		else if (name == "RZ_Event_WanderingTrader")
		{
			slot = "trader";
			prefab = PERSON;
		}

		if (!slot.IsEmpty())
		{
			if (gm.RZT_StandIn(slot, prefab))
				Print(string.Format("RZT forced: a stand-in %1 prefab, for %2", slot, name));
			return;
		}

		// Collect the Bounty and The Sniper: the bounty target (a loner on SURV)
		RZ_ContractSettings cs = gm.GetContractSettings();
		if ((name == "RZ_Mission_CollectTheBounty" || name == "RZ_Event_TheSniper") && cs && cs.m_sBountyTargetPrefab.IsEmpty())
		{
			cs.m_sBountyTargetPrefab = PERSON;
			Print("RZT forced: a stand-in bounty target, for " + name);
			return;
		}

		// Rival Survivors: their armed group
		RZ_EventSettings es = gm.GetEventSettings();
		if (name == "RZ_Event_RivalSurvivors" && es && es.m_sRivalGroupPrefab.IsEmpty())
		{
			es.m_sRivalGroupPrefab = GROUP;
			Print("RZT forced: a stand-in rival group, for " + name);
			return;
		}

		// Kill the Officer: a commander body that can die (the test world's outposts already stand
		// the same civilian in, through the puzzle civilian fallback)
		RZ_LeaderSettings ls;
		if (gm.GetLeaders())
			ls = gm.GetLeaders().GetSettings();
		if (name == "RZ_Mission_KillTheOfficer" && ls && ls.m_sBastionOutpostPrefab.IsEmpty())
		{
			ls.m_sBastionOutpostPrefab = PERSON;
			Print("RZT forced: a stand-in Bastion outpost commander, for " + name);
		}
	}

	// ================================================================ The questline phase
	//------------------------------------------------------------------------------------------------
	//! Once, before the first questline: the conditions most of them share.
	static void BeforeQuestlines(RZ_GameModeComponent gm)
	{
		RZ_Questlines q = gm.GetQuestlines();
		q.RZT_NoAutoStart();
		q.RZT_EndAll();

		// No town overrun (The Well Story's village, The List's farm, every stage's town)
		foreach (RZ_Settlement s : gm.GetSettlements())
		{
			s.m_fInfection = 20;
		}

		// A real day into the season (The Book)
		if (gm.GetSeasons())
			gm.GetSeasons().RZT_Age(2);

		// Every safehouse full enough and hungry (The List, The Price of the Door, Short Weight,
		// The Edge of Town, The Census, The Other Two)
		RZ_Safehouses houses = gm.GetSafehouses();
		int houseCount = 0;
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (!h)
					continue;

				h.m_iPopulation = Math.Max(h.m_iPopulation, 6);
				h.m_bShort = true;
				h.m_iLost = Math.Max(h.m_iLost, 1);
				houseCount++;
			}
		}

		// Every seat but the strongholds Independent: free council heads (The List), a commander
		// with his own agenda (Await Relief), a field lead (The Three Weeks)
		int seats = 0;
		RZ_Leaders leaders = gm.GetLeaders();
		if (leaders)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				array<RZ_Leader> list = {};
				leaders.GetFactionLeaders(f, list);
				foreach (RZ_Leader l : list)
				{
					if (l.IsStronghold())
						continue;

					l.m_iScore = 40;
					l.m_eLoyalty = RZ_ELoyalty.INDEPENDENT;
					seats++;
				}
			}
		}

		// A Long Table this season (its two questlines), a scavenger on the roads (The Three Weeks)
		bool table = false;
		if (gm.GetLongTable())
			table = gm.GetLongTable().RZT_Ensure();

		bool scavenger = false;
		if (gm.GetWanderers())
			scavenger = gm.GetWanderers().RZT_EnsureRole(RZ_EWandererRole.SCAVENGER);

		// A Red Hand van (Cargo)
		RZ_SupplySettings ss = gm.GetSupplySettings();
		if (ss && ss.m_sRedHandTruck.IsEmpty())
			ss.m_sRedHandTruck = REDHAND_TRUCK;

		HoldHour();
		Print(string.Format("RZT forced: questlines by the test only, towns at infection 20, season aged 2 days, %1 safehouses full and hungry, %2 seats Independent, Long Table %3, scavenger %4, Red Hand van, %5:00 or so",
			houseCount, seats, table, scavenger, HOUR));
	}

	//------------------------------------------------------------------------------------------------
	//! The order the test runs them in: The Well Story before Cargo (Cargo's purge ending dismisses
	//! the crew boss The Well Story needs, and the test world has only one), A Place at the Table before The House
	//! That Went Quiet (which exposes the house), and Authors last (it needs the final stand).
	static void Order(notnull array<string> keys)
	{
		// The cure finale's routes start only at the gate: the finale's own phase runs them (RZT_Finale.c)
		for (int r = keys.Count() - 1; r >= 0; r--)
		{
			if (keys[r].StartsWith("route_"))
				keys.RemoveOrdered(r);
		}

		int well = keys.Find("well");
		int cargo = keys.Find("cargo");
		if (well > cargo && cargo >= 0)
		{
			keys.RemoveOrdered(well);
			keys.InsertAt("well", cargo);
		}

		array<string> last = {"table", "authors"};
		foreach (string key : last)
		{
			int idx = keys.Find(key);
			if (idx >= 0)
			{
				keys.RemoveOrdered(idx);
				keys.Insert(key);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Right before Begin(key): what its CanStart reads that the shared conditions don't cover.
	static void BeforeQuest(RZ_GameModeComponent gm, string key)
	{
		RZ_Questlines q = gm.GetQuestlines();
		HoldHour();
		// The seats the mission phase emptied are filled (a season fills them after two hours)
		if (gm.GetLeaders())
		{
			int seats = gm.GetLeaders().RZT_FillSeats();
			if (seats > 0)
				Print(string.Format("RZT forced: %1 empty or held seats filled, for %2", seats, key));
		}

		switch (key)
		{
			case "walk":
			{
				// The Walk Him Out contract was done (its CanStart's flag)
				q.SetWorldFlag("walk.contract", 1);
				Print("RZT forced: walk.contract");
				break;
			}
			case "census":
			{
				// Bastion short of supplies
				if (gm.GetFactionLife())
					gm.GetFactionLife().RZT_SetNeed(RZ_EFaction.MILITARY, RZ_ENeed.SUPPLIES);
				Print("RZT forced: Bastion needs supplies");
				break;
			}
			case "pilot":
			{
				// Someone kept the LZ on a Last Helicopter run, the pilot lives, the helicopter is up
				q.SetWorldFlag("pilot.lz_kept", 1);
				q.SetWorldFlag("pilot.dead", 0);
				q.SetWorldFlag("pilot.down_until", 0);
				Print("RZT forced: pilot.lz_kept");
				break;
			}
			case "weeks":
			{
				// Patient Zero's ledger was solved; a scavenger still on the roads (the driver)
				q.SetWorldFlag("weeks.records", 1);
				bool driver = gm.GetWanderers() && gm.GetWanderers().RZT_EnsureRole(RZ_EWandererRole.SCAVENGER);
				Print(string.Format("RZT forced: weeks.records, a scavenger %1", driver));
				break;
			}
			case "place":
			{
				// The host has fed the tester twice and offers a place
				bool offered = false;
				if (gm.GetLongTable())
					offered = gm.GetLongTable().RZT_Offer(RZT_Fake.ID);
				// The offer is for someone the Haven doesn't already take in (below Trusted/Respected);
				// the mission phase leaves the tester well liked
				RZ_PlayerProfile tester = gm.GetProfile(RZT_Fake.ID);
				s_iHavenFame = tester.m_aFame[RZ_EFaction.SURVIVORS];
				s_iHavenInfamy = tester.m_aInfamy[RZ_EFaction.SURVIVORS];
				tester.m_aFame[RZ_EFaction.SURVIVORS] = 0;
				tester.m_aInfamy[RZ_EFaction.SURVIVORS] = 0;
				Print(string.Format("RZT forced: the Long Table's offer to the tester (%1), the tester a stranger to the Haven", offered));
				break;
			}
			case "table":
			{
				// The List found the farm
				q.SetWorldFlag("list.found_table", 1);
				Print("RZT forced: list.found_table");
				break;
			}
			case "authors":
			{
				// Research past half (and past three quarters, for Ready the lab)
				if (gm.GetCure())
				{
					s_fCureBefore = gm.GetCure().m_fProgress;
					gm.GetCure().RZT_SetProgress(80);
				}
				Print("RZT forced: research at 80 percent");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static void AfterBegin(RZ_GameModeComponent gm, string key)
	{
		if (key == "place" && gm.GetLongTable())
			gm.GetLongTable().RZT_ClearOffer();
	}

	//------------------------------------------------------------------------------------------------
	//! Every second while the questline runs.
	static void Tick(RZ_GameModeComponent gm, string key, RZ_QuestRun run)
	{
		HoldHour();
		// The Three Weeks' driver won't be taken in hand while a player stands within 250 m of him
		// (his body would vanish in front of them), and a wanderer may walk up to a new player
		if (key == "weeks" && run.m_iStage == RZ_Quest_ThreeWeeks.S_DRIVER && run.m_iMissionId < 0)
			AwayFrom(gm, run.Flag("driver", -1));

		// Authors' last stages wait for the final stand
		if (key == "authors" && run.m_iStage == RZ_Quest_Authors.S_STAND_WAIT && gm.GetCure() && !gm.GetCure().IsFinalStand())
		{
			gm.GetCure().RZT_StartStand();
			Print("RZT forced: the final stand, for Authors");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! After the questline ended, fizzled or was given up on.
	static void AfterQuest(RZ_GameModeComponent gm, string key)
	{
		if (key == "place" && s_iHavenFame >= 0)
		{
			RZ_PlayerProfile tester = gm.GetProfile(RZT_Fake.ID);
			tester.m_aFame[RZ_EFaction.SURVIVORS] = s_iHavenFame;
			tester.m_aInfamy[RZ_EFaction.SURVIVORS] = s_iHavenInfamy;
			s_iHavenFame = -1;
			Print("RZT forced: the tester's standing with the Haven put back");
		}

		if (key == "authors" && gm.GetCure() && s_fCureBefore >= 0)
		{
			gm.GetCure().RZT_StopStand(s_fCureBefore);
			Print(string.Format("RZT forced: the stand stopped, research back to %1 percent", s_fCureBefore));
			s_fCureBefore = -1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The outcome the test reports for a stage mission it ends as won, where "done" alone loops
	//! the stage. -1: none (OUTCOME_DONE moves the story on). Read from each questline's Next().
	static int Outcome(string key, int stage)
	{
		switch (key)
		{
			case "haven_short_weight":
			{
				if (stage == RZ_Quest_ShortWeight.S_ASK)
					return RZ_Quest_ShortWeight.OUT_LEARNED;
				if (stage == RZ_Quest_ShortWeight.S_HANDOVER)
					return RZ_Quest_ShortWeight.OUT_STOPPED;
				break;
			}
			case "orders":
			{
				if (stage == RZ_Quest_AwaitRelief.S_TAPE)
					return RZ_Quest_AwaitRelief.OUT_LOOP;
				if (stage == RZ_Quest_AwaitRelief.S_RUNNER)
					return RZ_Quest_AwaitRelief.OUT_PAGE;
				if (stage == RZ_Quest_AwaitRelief.S_WHO)
					return RZ_Quest_AwaitRelief.OUT_STOVE;
				break;
			}
			case "census":
			{
				if (stage == RZ_Quest_Census.S_COUNT)
					return RZ_Quest_Census.OUT_COUNTED;
				if (stage == RZ_Quest_Census.S_CARDS)
					return RZ_Quest_Census.OUT_CARDS;
				if (stage == RZ_Quest_Census.S_CURFEW)
					return RZ_Quest_Census.OUT_ENFORCED;
				if (stage == RZ_Quest_Census.S_RAID)
					return RZ_Quest_Census.OUT_DEFENDED;
				if (stage == RZ_Quest_Census.S_CALL)
					return RZ_Quest_Census.OUT_TRUE;
				break;
			}
			case "pilot":
			{
				if (stage == RZ_Quest_LastPilot.S_FUEL)
					return RZ_Quest_LastPilot.OUT_FUELLED;
				if (stage == RZ_Quest_LastPilot.S_FLIGHT)
					return RZ_Quest_LastPilot.OUT_FLOWN;
				if (stage == RZ_Quest_LastPilot.S_WRECK)
					return RZ_Quest_LastPilot.OUT_BOX;
				if (stage == RZ_Quest_LastPilot.S_DECIDE)
					return RZ_Quest_LastPilot.OUT_GROUNDED;
				break;
			}
			case "book":
			{
				if (stage == RZ_Quest_TheBook.S_WHO)
					return RZ_Quest_TheBook.OUT_NAMED;
				if (stage == RZ_Quest_TheBook.S_FIND)
					return RZ_Quest_TheBook.OUT_CAUGHT;
				if (stage == RZ_Quest_TheBook.S_READ)
					return RZ_Quest_TheBook.OUT_READ;
				if (stage == RZ_Quest_TheBook.S_PAGE)
					return RZ_Quest_TheBook.OUT_RETURNED;
				break;
			}
			case "cargo":
			{
				if (stage == RZ_Quest_Cargo.S_DRIVER)
					return RZ_Quest_Cargo.OUT_MET;
				if (stage == RZ_Quest_Cargo.S_LOAD)
					return RZ_Quest_Cargo.OUT_DELIVERED;
				if (stage == RZ_Quest_Cargo.S_BUYS)
					return RZ_Quest_Cargo.OUT_KNOWN;
				if (stage == RZ_Quest_Cargo.S_TELL)
					return RZ_Quest_Cargo.OUT_PURGE;
				break;
			}
			case "well":
			{
				if (stage == RZ_Quest_WellStory.S_COLLECT)
					return RZ_Quest_WellStory.OUT_REFUSED;
				if (stage == RZ_Quest_WellStory.S_WHY)
					return RZ_Quest_WellStory.OUT_DUES;
				break;
			}
			case "authors":
			{
				if (stage == RZ_Quest_Authors.S_STAND)
					return RZ_Quest_Authors.OUT_WON;
				break;
			}
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Why a questline's CanStart said no, from what it reads (one line: "RZT quest <key> why: ...").
	static void WhyNotQuest(RZ_GameModeComponent gm, string key)
	{
		string why = "";
		RZ_Leaders leaders = gm.GetLeaders();
		if (leaders)
			why = string.Format("Red Hand seats %1; Halcyon seats %2", leaders.RZT_Seats(RZ_EFaction.BANDITS), leaders.RZT_Seats(RZ_EFaction.INSTITUTE));

		why += string.Format("; clue %1, object %2, civilian %3", RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE), RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT), RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN));
		if (key == "well" || key == "cargo")
		{
			int traits = (1 << RZ_ELeaderTrait.RUTHLESS) | (1 << RZ_ELeaderTrait.GREEDY);
			RZ_Leader crew = RZ_RedHandQuest.PickAny(gm, RZ_EFaction.BANDITS, traits);
			why += string.Format("; a crew boss %1", crew != null);
			if (key == "cargo" && crew)
			{
				vector toward;
				string refuge;
				why += string.Format(", van '%1', no vehicles %2, a Haven refuge %3", RZ_CargoVan.VanPrefab(gm), RZ_Seasons.Mods().m_bNoVehicles, RZ_RedHandEdges.HavenRefuge(gm, crew.m_vPos, toward, refuge));
			}
		}

		if (key == "weeks")
		{
			array<int> drivers = {};
			if (gm.GetWanderers())
				gm.GetWanderers().GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER), drivers);

			RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
			vector site;
			int anchor = -1;
			string place;
			why += string.Format("; records %1, lab %2, field lead %3, scavengers %4, a site %5", gm.GetQuestlines().WorldFlag(RZ_Quest_ThreeWeeks.FLAG_RECORDS), gm.HasLab(), RZ_QM_HalcyonScene.PickFieldLead(gm, false) != null, drivers.Count(), RZ_Quest_ThreeWeeks.FindSite(gm, rng, site, anchor, place));
		}

		Print(string.Format("RZT quest %1 why: %2", key, why));
	}

	//------------------------------------------------------------------------------------------------
	//! The tester goes to the town farthest from wanderer `id` if he's within 300 m of them.
	static void AwayFrom(RZ_GameModeComponent gm, int id)
	{
		vector at;
		if (!RZT_Fake.s_Ent || !gm.GetWanderers() || !gm.GetWanderers().GetPosition(id, at))
			return;

		if (RZ_Util.DistXZ(at, RZT_Fake.s_Ent.GetOrigin()) >= 300)
			return;

		vector dest = at;
		float farDist = 0;
		foreach (RZ_Settlement s : gm.GetSettlements())
		{
			float d = RZ_Util.DistXZ(at, s.m_vCenter);
			if (d > farDist)
			{
				farDist = d;
				dest = s.m_vCenter;
			}
		}

		RZT_Fake.MoveTo(dest);
		Print(string.Format("RZT forced: the tester moved %1 m away from wanderer %2", RZ_Util.RoundToInt(farDist), id));
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps the clock at HOUR, give or take ten minutes (test only: the mod never moves time).
	static void HoldHour()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity tm = world.GetTimeAndWeatherManager();
		if (!tm)
			return;

		float h = tm.GetTimeOfTheDay();
		if (h < HOUR - 0.15 || h > HOUR + 0.2)
			tm.SetTimeOfTheDay(HOUR); // VERIFY: the server's clock follows (base game: SCR_FPS_Autotest)
	}
}
