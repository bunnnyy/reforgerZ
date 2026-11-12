//! Shared pieces for the Haven's questlines (docs/QUESTLINES.md, "The Haven": The List, The Price
//! of the Door, The Edge of Town): the people and places a line keeps on its run, checked again
//! whenever a stage begins (a council head replaced, a safehouse emptied), food a player carries,
//! and RZ_HavenQM, the base for their stage missions.

//------------------------------------------------------------------------------------------------
class RZ_HavenQuest
{
	// ================================================================ Leaders kept on a run
	//------------------------------------------------------------------------------------------------
	//! The leader with this id if they're still in their seat: present and the same person. A seat
	//! keeps its id when someone new takes the chair; the generation tells them apart.
	static RZ_Leader SameLeader(RZ_GameModeComponent gm, int id, int generation)
	{
		if (!gm.GetLeaders() || id < 0)
			return null;

		RZ_Leader l = gm.GetLeaders().Get(id);
		if (!l || !l.IsPresent() || l.m_iGeneration != generation)
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever sits in that seat now (a successor counts), null if nobody.
	static RZ_Leader SeatHolder(RZ_GameModeComponent gm, int id)
	{
		if (!gm.GetLeaders() || id < 0)
			return null;

		RZ_Leader l = gm.GetLeaders().Get(id);
		if (!l || !l.IsPresent())
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps a leader on the run: flags "<key>" (seat id) and "<key>_gen", text "<key>" (their name).
	static void KeepLeader(RZ_QuestRun run, string key, RZ_Leader l)
	{
		if (!l)
			return;

		run.SetFlag(key, l.m_iId);
		run.SetFlag(key + "_gen", l.m_iGeneration);
		run.SetText(key, l.m_sShort);
		run.SetText(key + "_place", l.m_sPlace);
	}

	//------------------------------------------------------------------------------------------------
	//! The kept leader, if the same person is still in the seat.
	static RZ_Leader KeptLeader(RZ_GameModeComponent gm, RZ_QuestRun run, string key)
	{
		if (!run)
			return null;

		return SameLeader(gm, run.Flag(key, -1), run.Flag(key + "_gen", -1));
	}

	//------------------------------------------------------------------------------------------------
	//! The council head of safehouse `house` (the chair, for the house at the Haven stronghold).
	static RZ_Leader CouncilHead(RZ_GameModeComponent gm, int house)
	{
		RZ_Safehouse h = LiveHouse(gm, house);
		if (!h || !gm.GetLeaders())
			return null;

		array<RZ_Leader> list = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.SURVIVORS, list);
		foreach (RZ_Leader l : list)
		{
			if (l.m_eRole == RZ_ELeaderRole.SAFEHOUSE && l.m_iSafehouse == house)
				return l;
		}

		RZ_Leader chair = gm.GetLeaders().GetStronghold(RZ_EFaction.SURVIVORS);
		if (chair && RZ_Util.DistXZ(chair.m_vPos, h.m_vPos) < 80)
			return chair;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The safehouse at the Haven stronghold (the chair's house), -1 if there's none.
	static int ChairHouse(RZ_GameModeComponent gm)
	{
		if (!gm.GetLeaders() || !gm.GetSafehouses())
			return -1;

		RZ_Leader chair = gm.GetLeaders().GetStronghold(RZ_EFaction.SURVIVORS);
		if (!chair)
			return -1;

		return gm.GetSafehouses().FindNearest(chair.m_vPos, 80);
	}

	//------------------------------------------------------------------------------------------------
	//! Council heads (not the chair) who run their house their own way, Independent or Rogue, of
	//! safehouses with at least minPeople.
	static void FreeHeads(RZ_GameModeComponent gm, int minPeople, notnull array<RZ_Leader> result)
	{
		if (!gm.GetLeaders())
			return;

		array<RZ_Leader> list = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.SURVIVORS, list);
		foreach (RZ_Leader l : list)
		{
			if (l.m_eRole != RZ_ELeaderRole.SAFEHOUSE || l.m_eLoyalty == RZ_ELoyalty.LOYAL)
				continue;

			RZ_Safehouse h = LiveHouse(gm, l.m_iSafehouse);
			if (h && h.m_iPopulation >= minPeople)
				result.Insert(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest present leader of `faction` within maxDist of pos, an outpost's before the
	//! stronghold's when both are in reach. Null if none.
	static RZ_Leader NearestSeat(RZ_GameModeComponent gm, int faction, vector pos, float maxDist)
	{
		if (!gm.GetLeaders())
			return null;

		array<RZ_Leader> list = {};
		gm.GetLeaders().GetFactionLeaders(faction, list);
		RZ_Leader best;
		RZ_Leader top;
		float bestDist = maxDist;
		foreach (RZ_Leader l : list)
		{
			float d = RZ_Util.DistXZ(pos, l.m_vPos);
			if (d > maxDist)
				continue;

			if (l.IsStronghold())
			{
				top = l;
				continue;
			}

			if (d <= bestDist)
			{
				bestDist = d;
				best = l;
			}
		}

		if (best)
			return best;

		return top;
	}

	//------------------------------------------------------------------------------------------------
	//! Everyone who took part is remembered by this leader (RZ_Leaders.MEM_*), online or not.
	static void LeaderRemembers(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_Leader l, int flag)
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

	// ================================================================ Safehouses
	//------------------------------------------------------------------------------------------------
	//! A safehouse that still has people in it, null if it's gone or empty.
	static RZ_Safehouse LiveHouse(RZ_GameModeComponent gm, int idx)
	{
		if (!gm.GetSafehouses())
			return null;

		RZ_Safehouse h = gm.GetSafehouses().Get(idx);
		if (!h || h.m_iPopulation <= 0)
			return null;

		return h;
	}

	//------------------------------------------------------------------------------------------------
	//! Hungry: the last upkeep ran short, or there's less than two upkeeps' food.
	static bool IsHungry(RZ_Safehouse h)
	{
		if (!h)
			return false;

		return h.m_bShort || h.m_iFood < h.FoodNeed(5) * 2;
	}

	//------------------------------------------------------------------------------------------------
	//! Under pressure: hungry, or it has lost people lately (to hunger, raids, contracts).
	static bool IsPressed(RZ_Safehouse h)
	{
		return h && (IsHungry(h) || h.m_iLost > 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Any safehouse that has gone hungry or lost people lately (to hunger, raids, contracts).
	static bool AnyHungry(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return false;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (h && (h.m_bShort || h.m_iLost > 0))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! People leave safehouse `house` (taken, lost), at most `count` and never below the upkeep floor
	//! of two (docs/QUESTLINES.md, "Limits every questline keeps"). Returns how many left.
	static int TakePeople(RZ_GameModeComponent gm, int house, int count)
	{
		RZ_Safehouse h = LiveHouse(gm, house);
		if (!h)
			return 0;

		int gone = Math.MinInt(count, h.m_iPopulation - RZ_Safehouses.UPKEEP_FLOOR);
		if (gone <= 0)
			return 0;

		RZ_Questline.ChangePopulation(gm, house, -gone);
		return gone;
	}

	//------------------------------------------------------------------------------------------------
	//! The house has gone to the Preacher's prayers (story arc The Quiet Congregation).
	static bool IsPraying(RZ_GameModeComponent gm, int idx)
	{
		RZ_StoryArcs arcs = gm.GetStoryArcs();
		return arcs && arcs.UpkeepMult(idx) < 1;
	}

	// ================================================================ Food a player carries
	//------------------------------------------------------------------------------------------------
	static int CountFood(RZ_GameModeComponent gm, IEntity character)
	{
		if (!character || !gm.GetSafehouses())
			return 0;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<IEntity> items = {};
		inv.GetItems(items);
		int count = 0;
		foreach (IEntity item : items)
		{
			if (item && RZ_FoodHelper.IsFood(item, gm.GetSafehouses().GetFoodKeywords()))
				count++;
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes exactly `count` food items from the character, or nothing (false) if they carry fewer.
	static bool TakeFood(RZ_GameModeComponent gm, IEntity character, int count)
	{
		if (CountFood(gm, character) < count)
			return false;

		return RZ_FoodHelper.TakeFood(character, gm.GetSafehouses().GetFoodKeywords(), count) >= count;
	}

	// ================================================================ Words
	//------------------------------------------------------------------------------------------------
	static string ManName(RZ_Rng rng)
	{
		array<string> names = {"Pavel", "Henri", "Radim", "Emil", "Anton", "Viktor", "Karel", "Oskar", "Milan", "Jakub", "Bruno", "Leon"};
		return rng.Pick(names);
	}

	//------------------------------------------------------------------------------------------------
	static string WomanName(RZ_Rng rng)
	{
		array<string> names = {"Marta", "Jana", "Lucie", "Ruth", "Irena", "Elise", "Nadia", "Vera", "Sabine", "Ilse", "Dana", "Agata"};
		return rng.Pick(names);
	}

	//------------------------------------------------------------------------------------------------
	//! Dark now, or the clock isn't known (then night-only approaches are never shut).
	static bool IsDark()
	{
		if (RZ_GameTime.GetHour() < 0)
			return true;

		return RZ_GameTime.IsNight();
	}

	//------------------------------------------------------------------------------------------------
	//! Around dusk (16:30 to 21:00, as Walk Him Out), or the clock isn't known.
	static bool IsDusk()
	{
		float hour = RZ_GameTime.GetHour();
		if (hour < 0)
			return true;

		return hour >= 16.5 && hour <= 21;
	}

	//------------------------------------------------------------------------------------------------
	static string Grid(vector pos)
	{
		return RZ_SettlementScanner.GridReference(pos);
	}
}

//------------------------------------------------------------------------------------------------
//! Base for the Haven questlines' stage missions: the brief in a kept leader's voice, who may settle
//! a stage, places whose props go up when a player first comes near (a mission over several
//! houses), something carried by one player (an envelope, the copies of the List: a player, not an
//! item, as The Informant's names), and a hold with waves.
class RZ_HavenQM : RZ_QuestMission
{
	protected ref array<vector> m_aSites = {};
	protected ref array<bool> m_aSiteUp = {};
	protected float m_fSiteRadius = 300;
	protected int m_iCarrier = -1;
	protected float m_fHeld;
	protected float m_fWaveTimer;
	protected int m_iWaves;
	protected int m_iHoldStep;

	//------------------------------------------------------------------------------------------------
	//! The radio brief in this kept leader's words, while the same person is in the seat.
	protected void VoiceBy(RZ_GameModeComponent gm, string key)
	{
		RZ_Leader l = RZ_HavenQuest.KeptLeader(gm, Run(gm), key);
		if (l)
			m_iLeaderId = l.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! Only someone this job is open to may settle it (a passer-by is turned away by `who`).
	protected bool MayDecide(RZ_GameModeComponent gm, int playerId, string who)
	{
		if (IsEligible(gm, playerId))
			return true;

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 looks you over and shakes their head. \"Not you. This isn't yours to settle.\"", who));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity PlayerEnt(int playerId)
	{
		if (playerId <= 0)
			return null;

		return GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Is the player within radius of pos?
	protected static bool IsNear(int playerId, vector pos, float radius)
	{
		IEntity e = PlayerEnt(playerId);
		return e && RZ_Util.DistXZ(e.GetOrigin(), pos) <= radius;
	}

	// ================================================================ Places
	//------------------------------------------------------------------------------------------------
	protected int AddSite(vector pos)
	{
		m_aSites.Insert(pos);
		m_aSiteUp.Insert(false);
		return m_aSites.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! A player came near place `site` for the first time: put its people and props down.
	protected void OnSiteNear(RZ_GameModeComponent gm, int site)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void TickSites(RZ_GameModeComponent gm, array<IEntity> players)
	{
		foreach (int i, vector p : m_aSites)
		{
			if (m_aSiteUp[i] || !RZ_ContractHelp.AnyNear(players, p, m_fSiteRadius))
				continue;

			m_aSiteUp[i] = true;
			OnSiteNear(gm, i);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		foreach (vector p : m_aSites)
		{
			result.Insert(p);
		}
	}

	// ================================================================ Carried by one player
	//------------------------------------------------------------------------------------------------
	protected void GiveTo(int playerId)
	{
		m_iCarrier = playerId;
		AddParticipant(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The carrier, alive and online; -1 if nobody carries it (or they're gone).
	protected int Carrier()
	{
		IEntity e = PlayerEnt(m_iCarrier);
		if (!e || !RZ_Util.IsCharacterAlive(e))
			return -1;

		return m_iCarrier;
	}

	// ================================================================ Holding a place
	//------------------------------------------------------------------------------------------------
	//! At least one player within radius of center for `needed` seconds in all, with a wave of the
	//! infected every waveEvery seconds (at most maxWaves). True once it's held long enough.
	protected bool TickHold(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players, vector center, float radius, float needed, float waveEvery, int maxWaves)
	{
		if (CountPlayersNear(players, center, radius) == 0)
			return false;

		RZ_ContractHelp.AddNear(ids, players, center, radius, m_aParticipants);
		m_fHeld += dt;
		if (waveEvery > 0 && m_iWaves < maxWaves)
		{
			m_fWaveTimer += dt;
			if (m_fWaveTimer >= waveEvery)
			{
				m_fWaveTimer = 0;
				m_iWaves++;
				gm.GetSpawner().SpawnHorde(center, 220, m_iId);
				NotifyParticipants(gm, m_sTitle, "They're coming. Hold here.");
			}
		}

		int step = Math.Floor(4 * m_fHeld / Math.Max(1, needed));
		if (step > m_iHoldStep && step < 4)
		{
			m_iHoldStep = step;
			NotifyParticipants(gm, m_sTitle, string.Format("Holding: %1 percent.", step * 25));
		}

		return m_fHeld >= needed;
	}
}
