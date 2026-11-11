//! The world memory's writers (docs/DESIGN.md, "World memory"): one small static helper per kind of
//! fact, so the system that owns the fact writes it with one line and no system keeps its own rules.
//! - Signals (the ring): bites and deaths (where and how, never who), trucks lost, shells, burned
//!   buildings, trial doses. A player's own bites and deaths add at most one signal per 10 minutes.
//! - Reasons: a house short of food, dark or dry; a truck lost on the way to a post or a house; a
//!   town gone bad; a leader dead; a wanderer bitten; a cure milestone.
//! - The town log: what was done in a town (a contract finished or failed there, people brought in,
//!   a town burned, a house emptied), with its season mark.
//! Errands and missions read them ("Why"); the epilogue will show the town log and the records.
//! Server only.
class RZ_WorldWriters
{
	protected static const float FLOOD_SECONDS = 600;
	protected static ref map<string, int> s_mFlood = new map<string, int>();	// "kind|ident" -> unix
	protected static ref map<int, bool> s_mBad = new map<int, bool>();			// town -> infested or overrun

	//------------------------------------------------------------------------------------------------
	protected static RZ_WorldMemory Memory()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetFacts())
			return null;

		return gm.GetFacts().Memory();
	}

	//------------------------------------------------------------------------------------------------
	//! One player's own signals: at most one of a kind per 10 minutes.
	protected static bool Flooding(string kind, int playerId)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || playerId <= 0)
			return false;

		string key = kind + "|" + gm.GetProfile(playerId).m_sIdentity;
		int now = RZ_SeasonClock.Now();
		int last;
		if (s_mFlood.Find(key, last) && now - last < FLOOD_SECONDS)
			return true;

		s_mFlood.Set(key, now);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "a bite on the Morton road": the nearest town's name, or the grid.
	protected static string Where(vector pos)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return "";

		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(pos));
		if (s && RZ_Util.DistXZ(s.m_vCenter, pos) <= 1500)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(pos);
	}

	// ================================================================ Signals
	//------------------------------------------------------------------------------------------------
	//! A player was bitten at pos (RZ_GameModeComponent.InfectPlayer).
	static void Bite(int playerId, vector pos)
	{
		RZ_WorldMemory mem = Memory();
		if (!mem || Flooding("bite", playerId))
			return;

		mem.Signal("bite", pos, "a bite near " + Where(pos));
	}

	//------------------------------------------------------------------------------------------------
	//! A player died at pos (RZ_FactionPerks): the background and how long the life lasted, never
	//! the name.
	static void Death(int playerId, vector pos)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_WorldMemory mem = Memory();
		if (!gm || !mem || Flooding("death", playerId))
			return;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		RZ_Background bg = gm.GetPlayerBackground(playerId);
		string who = "someone";
		if (bg)
			who = "a " + bg.m_sName;

		who.ToLower();
		int days = Math.Max(1, RZ_Util.RoundToInt(profile.m_fLifeSeconds / Math.Max(60, RZ_SeasonClock.DayMinutes() * 60)));
		mem.Signal("death", pos, string.Format("%1, %2 days in, near %3", who, days, Where(pos)));
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's supply truck was lost at pos on the way to `dest` (RZ_SupplyLines.Fail). cause:
	//! who robbed it ("12,15"), server-only, for the arsonist rule. Also a reason at the destination.
	static void TruckLost(int faction, bool toHouse, int destIndex, string dest, vector pos, vector destPos, string cause)
	{
		RZ_WorldMemory mem = Memory();
		if (!mem)
			return;

		mem.Signal("truck." + faction.ToString(), pos, dest, cause);
		string text = string.Format("The truck for %1 didn't come.", dest);
		if (toHouse)
			mem.PostReason("house.truck_lost", RZ_WorldFacts.Safehouse(destIndex), text, destPos, faction, 0.5);
		else
			mem.PostReason("post.truck_lost", "holding:" + destIndex.ToString(), text, destPos, faction, 0.5);
	}

	//------------------------------------------------------------------------------------------------
	//! Shells landed at pos (RZ_Artillery: the Fire Mission perk and contract). cause: who called them.
	static void Shell(vector pos, string cause)
	{
		// The tribes: a lair under the shells is disturbed (docs/INFECTED.md, Lair moves)
		RZ_GameModeComponent gmTribes = RZ_GameModeComponent.GetInstance();
		if (gmTribes && gmTribes.GetTribes())
			gmTribes.GetTribes().OnShell(pos);

		RZ_WorldMemory mem = Memory();
		if (!mem)
			return;

		string place = Where(pos);
		mem.Signal("shell", pos, place, cause);
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int town = gm.FindNearestSettlement(pos);
		mem.PostReason("town.shelled", RZ_WorldFacts.Town(town), string.Format("Shells came down near %1.", place), pos, RZ_EFaction.MILITARY, 0.5);
	}

	//------------------------------------------------------------------------------------------------
	//! A building burned at pos (RZ_TownMemory.ReportBuildingBurned).
	static void Fire(vector pos)
	{
		RZ_WorldMemory mem = Memory();
		if (mem)
			mem.Signal("fire", pos, "a fire near " + Where(pos));
	}

	//------------------------------------------------------------------------------------------------
	//! A trial dose at the lab (RZ_Infection.TrialDose): whether it took. Never who.
	static void Dose(vector pos, bool cured)
	{
		RZ_WorldMemory mem = Memory();
		if (!mem)
			return;

		string what = "a trial dose that didn't take";
		if (cured)
			what = "a trial dose that took";

		mem.Signal("dose", pos, what);
	}

	// ================================================================ Reasons
	//------------------------------------------------------------------------------------------------
	//! A safehouse's food after its upkeep (RZ_Safehouses.Upkeep): short posts house.hungry, fed clears it.
	static void HouseFood(int idx, string name, vector pos, bool shortNow)
	{
		RZ_WorldMemory mem = Memory();
		if (!mem)
			return;

		if (shortNow)
			mem.PostReason("house.hungry", RZ_WorldFacts.Safehouse(idx), string.Format("%1's on half portions.", name), pos, RZ_EFaction.SURVIVORS);
		else
			mem.ClearReason("house.hungry", RZ_WorldFacts.Safehouse(idx));
	}

	//------------------------------------------------------------------------------------------------
	//! A town's state changed (RZ_Director): infested or overrun posts town.infested, safe clears it.
	static void TownState(int idx, string name, vector pos, bool infested)
	{
		RZ_WorldMemory mem = Memory();
		if (!mem)
			return;

		if (infested)
			mem.PostReason("town.infested", RZ_WorldFacts.Town(idx), string.Format("%1 belongs to the infected now.", name), pos);
		else
			mem.ClearReason("town.infested", RZ_WorldFacts.Town(idx));
	}

	//------------------------------------------------------------------------------------------------
	//! A leader died (RZ_FactionLife.OnLeaderDied).
	static void LeaderDied(int faction, int leaderId, string name, string place, vector pos)
	{
		RZ_WorldMemory mem = Memory();
		if (mem)
			mem.PostReason("leader.died", RZ_WorldFacts.Leader(leaderId), string.Format("%1 is dead, in %2.", name, place), pos, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer was bitten on the road (RZ_Wanderers.BiteOnRoad).
	static void WandererBitten(int wandererId, string name, vector pos)
	{
		RZ_WorldMemory mem = Memory();
		if (mem)
			mem.PostReason("wanderer.bitten", RZ_WorldFacts.Wanderer(wandererId), string.Format("%1 took a bite on the road.", name), pos, -1, 0.5);
	}

	//------------------------------------------------------------------------------------------------
	//! A cure milestone (RZ_CureResearch): cure.<percent>, about the lab.
	static void CureMilestone(int percent, vector lab)
	{
		RZ_WorldMemory mem = Memory();
		if (mem)
			mem.PostReason("cure." + percent.ToString(), RZ_WorldFacts.LAB, string.Format("The lab is at %1 percent.", percent), lab, RZ_EFaction.INSTITUTE);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 s (from the errands' tick): a town that went infested or overrun posts town.infested,
	//! one that came back clears it.
	static void WatchTowns(RZ_GameModeComponent gm)
	{
		if (!gm)
			return;

		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			RZ_ESettlementState state = s.GetState();
			bool bad = state == RZ_ESettlementState.INFESTED || state == RZ_ESettlementState.OVERRUN;
			bool was;
			if (s_mBad.Find(i, was) && was == bad)
				continue;

			bool known = s_mBad.Contains(i);
			s_mBad.Set(i, bad);
			if (known || bad)
				TownState(i, s.m_sName, s.m_vCenter, bad);
		}
	}

	// ================================================================ The town log
	//------------------------------------------------------------------------------------------------
	//! A mission ended at its town (RZ_MissionManager.Finish): done goes in the best slot (a contract
	//! 2, a world event or a questline stage 3), failed in the worst (1, 2 for an event). The players
	//! who took part are named, frozen at the deed.
	static void MissionEnded(RZ_Mission m, bool success, array<int> players)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_WorldMemory mem = Memory();
		if (!gm || !mem || m.m_bGlobal || m.m_bWithdrawn)
			return;

		int town = m.m_iSettlement;
		if (town < 0)
			town = gm.FindNearestSettlement(m.m_vPos);

		RZ_Settlement s = gm.GetSettlement(town);
		if (!s || RZ_Util.DistXZ(s.m_vCenter, m.m_vPos) > 1500)
			return;

		int day = RZ_SeasonClock.Day();
		if (success)
		{
			int salience = 2;
			if (!m.m_bContract || m.m_iQuestRun >= 0)
				salience = 3;

			string phrase = m.DeedPhrase(gm);
			mem.LogTown(town, true, "mission", salience, string.Format("Day %1: someone %2.", day, phrase), players);
		}
		else
		{
			int worst = 1;
			if (!m.m_bContract)
				worst = 2;

			mem.LogTown(town, false, "mission", worst, string.Format("Day %1: %2 went wrong here.", day, m.m_sTitle), players);
		}

		if (players)
		{
			foreach (int playerId : players)
			{
				mem.Visit(town, gm.GetProfile(playerId).m_sIdentity);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! People were brought in to a safehouse (RZ_Safehouses.AddRescued).
	static void Rescued(vector pos, int count, array<int> players)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_WorldMemory mem = Memory();
		if (!gm || !mem || count <= 0)
			return;

		int town = gm.FindNearestSettlement(pos);
		mem.LogTown(town, true, "rescue", 3, string.Format("Day %1: %2 people were brought in alive.", RZ_SeasonClock.Day(), count), players);
	}

	//------------------------------------------------------------------------------------------------
	//! A town burned (RZ_TownMemory.OnTownBurned).
	static void TownBurned(int idx, string name)
	{
		RZ_WorldMemory mem = Memory();
		if (mem)
			mem.LogTown(idx, false, "burned", 4, string.Format("Day %1: %2 burned, and nobody put it out.", RZ_SeasonClock.Day(), name));
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse emptied (RZ_Safehouses): the worst slot of its town.
	static void HouseEmptied(int town, string name)
	{
		RZ_WorldMemory mem = Memory();
		if (mem)
			mem.LogTown(town, false, "emptied", 4, string.Format("Day %1: the last people left the safehouse in %2.", RZ_SeasonClock.Day(), name));
	}

	//------------------------------------------------------------------------------------------------
	//! An outpost changed hands (RZ_Territory): the worst slot for the town it was taken in.
	static void HoldingTaken(int town, string taker, string loser, string name)
	{
		RZ_WorldMemory mem = Memory();
		if (mem)
			mem.LogTown(town, false, "taken", 3, string.Format("Day %1: %2 took %3 from %4.", RZ_SeasonClock.Day(), taker, name, loser));
	}
}
