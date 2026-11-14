//! Shared by the Red Hand's three questlines (docs/QUESTLINES.md, "The Red Hand"): R1 The Book
//! (RZ_Quest_TheBook.c), R2 Cargo (RZ_Quest_Cargo.c) and R3 The Well Story (RZ_Quest_WellStory.c).
//!
//! - RZ_RedHandQuest: leaders kept on a run by seat and generation, so a stage can tell the
//!   leader who is still in the chair from a successor or an empty chair; world flags (the
//!   crossings); the one loyalty swing an ending may make; leaders' memories.
//! - RZ_RedHandQuestMission: a stage mission with several places, each put down only when a
//!   player comes within SPOT_RADIUS of it. A stand-in at a leader's seat speaks for that leader,
//!   whose own body is held meanwhile (as in Rein In).

//------------------------------------------------------------------------------------------------
class RZ_RedHandQuest
{
	static const int SWING = 25;			// an ending's one loyalty swing, at most (as Rein In)
	static const int NO_VOICE = 1000000;	// m_iLeaderId for a contract no leader voices or remembers (no seat has this id)

	//------------------------------------------------------------------------------------------------
	//! Keep `l` on the run as `key`: flags key_id and key_gen, texts key (the name), key_short and
	//! key_place.
	static void Keep(RZ_QuestRun run, string key, RZ_Leader l)
	{
		if (!run || !l)
			return;

		run.SetFlag(key + "_id", l.m_iId);
		run.SetFlag(key + "_gen", l.m_iGeneration);
		run.SetText(key, l.m_sName);
		run.SetText(key + "_short", l.m_sShort);
		run.SetText(key + "_place", l.m_sPlace);
	}

	//------------------------------------------------------------------------------------------------
	//! The seat kept as `key`, whoever is in it now (or an empty chair waiting for its next
	//! leader). Null if the seat is gone (an outpost lost or abandoned).
	static RZ_Leader Seat(RZ_GameModeComponent gm, RZ_QuestRun run, string key)
	{
		if (!gm || !run || !gm.GetLeaders() || !run.HasFlag(key + "_id"))
			return null;

		RZ_Leader l = gm.GetLeaders().Get(run.Flag(key + "_id", -1));
		if (!l || !l.m_bActive)
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! The leader kept as `key`, if the same person is still in the chair. Null otherwise.
	static RZ_Leader Same(RZ_GameModeComponent gm, RZ_QuestRun run, string key)
	{
		RZ_Leader l = Seat(gm, run, key);
		if (!l || !l.IsPresent() || l.m_iGeneration != run.Flag(key + "_gen", -1))
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone new sits in the seat kept as `key` (the one kept died or was moved out): keep them
	//! instead and return them. Null if it's still the same person, the chair is empty or the seat
	//! is gone.
	static RZ_Leader Successor(RZ_GameModeComponent gm, RZ_QuestRun run, string key)
	{
		RZ_Leader l = Seat(gm, run, key);
		if (!l || !l.IsPresent() || l.m_iGeneration == run.Flag(key + "_gen", -1))
			return null;

		Keep(run, key, l);
		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! Keep a place on the run as `key` (flags key_x, key_z, in metres).
	static void KeepPos(RZ_QuestRun run, string key, vector pos)
	{
		if (!run)
			return;

		run.SetFlag(key + "_x", RZ_Util.RoundToInt(pos[0]));
		run.SetFlag(key + "_z", RZ_Util.RoundToInt(pos[2]));
	}

	//------------------------------------------------------------------------------------------------
	static bool HasPos(RZ_QuestRun run, string key)
	{
		return run && run.HasFlag(key + "_x") && run.HasFlag(key + "_z");
	}

	//------------------------------------------------------------------------------------------------
	//! The place kept as `key`, on the ground.
	static vector PosOf(RZ_QuestRun run, string key)
	{
		if (!HasPos(run, key))
			return vector.Zero;

		float x = run.Flag(key + "_x");
		float z = run.Flag(key + "_z");
		return RZ_Util.GroundPos(Vector(x, 0, z));
	}

	//------------------------------------------------------------------------------------------------
	//! A leader's loyalty moves once, by at most SWING either way (the stronghold leaders never move).
	static void SwingLoyalty(RZ_GameModeComponent gm, RZ_Leader l, int delta)
	{
		if (!gm || !l || !gm.GetLeaders() || l.IsStronghold())
			return;

		gm.GetLeaders().ShiftLoyalty(l, Math.ClampInt(delta, -SWING, SWING), true);
	}

	//------------------------------------------------------------------------------------------------
	//! A leader remembers one identity (RZ_Leaders.MEM_*).
	static void Remembers(RZ_Leader l, string ident, int flag)
	{
		if (l && !ident.IsEmpty())
			l.Remember(ident, flag);
	}

	//------------------------------------------------------------------------------------------------
	//! A leader remembers everyone who took part.
	static void RemembersAll(RZ_Leader l, RZ_QuestRun run, int flag)
	{
		if (!l || !run)
			return;

		array<string> idents = {};
		run.GetIdents(idents);
		foreach (string ident : idents)
		{
			l.Remember(ident, flag);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A season-wide world flag (docs/QUESTLINES.md, "Crossings"). 0 without the manager.
	static int World(RZ_GameModeComponent gm, string key)
	{
		if (!gm || !gm.GetQuestlines())
			return 0;

		return gm.GetQuestlines().WorldFlag(key);
	}

	//------------------------------------------------------------------------------------------------
	static bool HasWorld(RZ_GameModeComponent gm, string key)
	{
		return gm && gm.GetQuestlines() && gm.GetQuestlines().HasWorldFlag(key);
	}

	//------------------------------------------------------------------------------------------------
	static void SetWorld(RZ_GameModeComponent gm, string key, int value)
	{
		if (gm && gm.GetQuestlines())
			gm.GetQuestlines().SetWorldFlag(key, value);
	}

	//------------------------------------------------------------------------------------------------
	//! Dusk (16:00 to 21:00 game time), or any time if the clock is unknown.
	static bool IsDusk()
	{
		float hour = RZ_GameTime.GetHour();
		return hour < 0 || (hour >= 16 && hour <= 21);
	}

	//------------------------------------------------------------------------------------------------
	//! Night, or any time if the clock is unknown.
	static bool IsDark()
	{
		return RZ_GameTime.GetHour() < 0 || RZ_GameTime.IsNight();
	}

	//------------------------------------------------------------------------------------------------
	//! The safehouse at (or right beside) pos, -1 if none.
	static int HouseAt(RZ_GameModeComponent gm, vector pos)
	{
		if (!gm.GetSafehouses())
			return -1;

		return gm.GetSafehouses().FindNearest(pos, 60);
	}

	//------------------------------------------------------------------------------------------------
	//! A present leader of `faction` (not the stronghold): Rogue ones first, then Independent,
	//! then anyone; `preferTraits` (RZ_ELeaderTrait bits, 0 = none) are twice as likely within a
	//! band. Null if none.
	static RZ_Leader PickLeader(RZ_GameModeComponent gm, int faction, int preferTraits)
	{
		if (!gm.GetLeaders() || !RZ_Factions.IsValid(faction))
			return null;

		array<RZ_Leader> all = {};
		gm.GetLeaders().GetFactionLeaders(faction, all);
		array<int> bands = {RZ_ELoyalty.ROGUE, RZ_ELoyalty.INDEPENDENT, RZ_ELoyalty.LOYAL};
		foreach (int band : bands)
		{
			array<RZ_Leader> options = {};
			foreach (RZ_Leader l : all)
			{
				if (l.IsStronghold() || l.m_iHeldBy >= 0 || l.m_eLoyalty != band)
					continue;

				options.Insert(l);
				if (preferTraits != 0 && (HasAnyTrait(l, preferTraits)))
					options.Insert(l);
			}

			if (!options.IsEmpty())
				return options.GetRandomElement();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Any present leader of `faction` but the stronghold, whatever their loyalty; `preferTraits`
	//! (RZ_ELeaderTrait bits) twice as likely. Null if none.
	static RZ_Leader PickAny(RZ_GameModeComponent gm, int faction, int preferTraits)
	{
		if (!gm.GetLeaders() || !RZ_Factions.IsValid(faction))
			return null;

		array<RZ_Leader> all = {};
		gm.GetLeaders().GetFactionLeaders(faction, all);
		array<RZ_Leader> options = {};
		foreach (RZ_Leader l : all)
		{
			if (l.IsStronghold() || l.m_iHeldBy >= 0)
				continue;

			options.Insert(l);
			if (preferTraits != 0 && HasAnyTrait(l, preferTraits))
				options.Insert(l);
		}

		if (options.IsEmpty())
			return null;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasAnyTrait(RZ_Leader l, int traitBits)
	{
		for (int t = 0; t < RZ_Leaders.TRAIT_COUNT; t++)
		{
			if ((traitBits & (1 << t)) != 0 && l.HasTrait(t))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a faction's stronghold leader sits, else its nearest other seat to `from`. False if it
	//! has none.
	static bool FactionSeat(RZ_GameModeComponent gm, int faction, vector from, out vector pos, out string name)
	{
		if (!gm.GetLeaders())
			return false;

		RZ_Leader l = gm.GetLeaders().GetStronghold(faction);
		if (!l || !l.IsPresent())
			l = gm.GetLeaders().FindNearest(faction, from, 100000);

		if (!l)
			return false;

		pos = l.m_vPos;
		name = l.m_sPlace;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Distinct men's first names (the texts say "he"), from the Red Hand's debtors' names.
	static void MenNames(RZ_Rng rng, int count, notnull array<string> result)
	{
		int guard = 0;
		while (result.Count() < count && guard < 200)
		{
			guard++;
			string n = RZ_RedHandEdges.DebtorName(rng);
			if (!result.Contains(n))
				result.Insert(n);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Mara": a player's name, "someone" if they're gone.
	static string NameOf(int playerId)
	{
		string name = RZ_Names.Public(playerId);
		if (name.IsEmpty())
			return "someone";

		return name;
	}
}

//------------------------------------------------------------------------------------------------
//! Base for the Red Hand questlines' stage missions: places (spots) whose people and props go
//! down only when a player comes within SPOT_RADIUS of them, and leaders held while a stand-in
//! speaks for them. Subclasses add spots in SetupPuzzle, put them down in SpawnSpot and call
//! TickSpots from OnUpdate.
class RZ_RedHandQuestMission : RZ_QuestMission
{
	protected ref array<vector> m_aSpots = {};
	protected ref array<string> m_aSpotNames = {};
	protected ref array<bool> m_aSpotUp = {};
	protected ref array<int> m_aHeld = {};

	protected static const float SPOT_RADIUS = 300;

	//------------------------------------------------------------------------------------------------
	//! A place this stage uses. Its people go down when a player comes within SPOT_RADIUS.
	protected int AddSpot(vector pos, string name)
	{
		m_aSpots.Insert(RZ_Util.GroundPos(pos));
		m_aSpotNames.Insert(name);
		m_aSpotUp.Insert(false);
		return m_aSpots.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Put a place's people and props down (override).
	protected void SpawnSpot(RZ_GameModeComponent gm, int spot)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Is this place still part of the stage (for the where text; override)?
	protected bool IsSpotOpen(int spot)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Once per tick, from OnUpdate.
	protected void TickSpots(RZ_GameModeComponent gm, array<IEntity> players)
	{
		for (int i = 0; i < m_aSpots.Count(); i++)
		{
			if (m_aSpotUp[i] || !IsSpotOpen(i) || !RZ_ContractHelp.AnyNear(players, m_aSpots[i], SPOT_RADIUS))
				continue;

			m_aSpotUp[i] = true;
			SpawnSpot(gm, i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The spot nearest `from` that is still open, -1 if none.
	protected int NearestOpenSpot(vector from)
	{
		int best = -1;
		float bestDist = float.MAX;
		for (int i = 0; i < m_aSpots.Count(); i++)
		{
			if (!IsSpotOpen(i))
				continue;

			float d = RZ_Util.DistXZ(from, m_aSpots[i]);
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A leader's own body goes while a stand-in speaks for them (as Rein In); released when the
	//! mission ends.
	protected void HoldLeader(RZ_GameModeComponent gm, int leaderId)
	{
		if (!gm.GetLeaders() || leaderId < 0 || m_aHeld.Contains(leaderId))
			return;

		gm.GetLeaders().Hold(leaderId, m_iId);
		m_aHeld.Insert(leaderId);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseLeader(RZ_GameModeComponent gm, int leaderId)
	{
		int idx = m_aHeld.Find(leaderId);
		if (idx < 0)
			return;

		m_aHeld.Remove(idx);
		if (gm.GetLeaders())
			gm.GetLeaders().Release(leaderId);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		for (int i = 0; i < m_aSpots.Count(); i++)
		{
			if (IsSpotOpen(i) && !m_aSpotNames[i].IsEmpty())
				text += string.Format("\n%1: %2", m_aSpotNames[i], RZ_Util.DirectionText(from, m_aSpots[i]));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		if (gm.GetLeaders())
		{
			foreach (int leaderId : m_aHeld)
			{
				gm.GetLeaders().Release(leaderId);
			}
		}

		m_aHeld.Clear();
	}
}
