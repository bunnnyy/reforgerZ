//! World events that change a town or a region for a while: Fog Bank, Power Surge, Gas Station
//! Fire, Burning Town, Tunnel Rats (docs/MISSIONS.md, "World events"). They reuse the systems
//! that own each effect: the environment (fog), the power grid and town memory (power), the
//! fire system (one building at a time, never a whole town) and the zombie spawner.

//------------------------------------------------------------------------------------------------
//! Helpers shared by the town events.
class RZ_EventHelp
{
	//------------------------------------------------------------------------------------------------
	static RZ_EventSettings Settings(RZ_GameModeComponent gm)
	{
		RZ_EventSettings settings = gm.GetEventSettings();
		if (!settings)
			settings = new RZ_EventSettings();

		return settings;
	}

	//------------------------------------------------------------------------------------------------
	//! Fire works: world interactions are running and Traps > Fire is on.
	static bool FireOn(RZ_GameModeComponent gm)
	{
		RZ_Interactions hub = gm.GetInteractions();
		return hub && hub.IsActive() && hub.GetTraps().IsFireOn();
	}

	//------------------------------------------------------------------------------------------------
	//! A town where people live or a faction holds ground: a safehouse, a holding, the lab.
	static bool IsLivedIn(RZ_GameModeComponent gm, int idx)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.IsSafehouseTown(idx))
			return true;

		RZ_Territory territory = gm.GetTerritory();
		if (territory && territory.IsActive() && territory.FindInTown(idx))
			return true;

		return gm.HasLab() && gm.GetLabSettlement() == idx;
	}

	//------------------------------------------------------------------------------------------------
	//! Up to `max` infected within radius of pos die (fire, a blast). Returns how many.
	static int KillInfectedNear(RZ_GameModeComponent gm, vector pos, float radius, int max)
	{
		int killed = 0;
		for (int i = 0; i < max; i++)
		{
			IEntity z = gm.GetSpawner().FindZombieNear(pos, radius);
			if (!z)
				break;

			RZ_Util.KillCharacter(z);
			killed++;
		}

		return killed;
	}

	//------------------------------------------------------------------------------------------------
	//! Is anyone with one of these backgrounds within radius of pos? Their player id, else -1.
	static int BackgroundNear(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players, vector pos, float radius, array<string> backgrounds)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) > radius)
				continue;

			RZ_Background bg = gm.GetPlayerBackground(ids[i]);
			if (bg && backgrounds.Contains(bg.m_sId))
				return ids[i];
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	static void DeleteAll(array<IEntity> list)
	{
		foreach (IEntity e : list)
		{
			RZ_Util.DeleteUnit(e);
		}

		list.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! #4 Fog Bank: a region drops to low visibility for 20 minutes. Inside it, heat is muffled
//! and infected spawn much closer. Hiding is easy; so are ambushes. The engine's fog is
//! global and is never changed: an optional effect prop is the look.
class RZ_Event_FogBank : RZ_Event_Timed
{
	protected ref array<IEntity> m_aEffects = {};

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1200;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.GetEnvironment() || gm.GetMissions().IsTypeRunning(RZ_Event_Storm))
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		RZ_EventSettings settings = RZ_EventHelp.Settings(gm);
		float radius = Math.Max(200, settings.m_fFogRadius);
		m_vPos = s.m_vCenter;
		gm.GetEnvironment().StartFogBank(m_vPos, radius, settings.m_fFogHeatMult, settings.m_fFogSpawnMult);

		if (!settings.m_sFogEffectPrefab.IsEmpty())
		{
			m_aEffects.Insert(RZ_Util.SpawnPrefab(settings.m_sFogEffectPrefab, m_vPos, 0));
			for (int i = 0; i < 4; i++)
			{
				IEntity e = RZ_Util.SpawnPrefab(settings.m_sFogEffectPrefab, RZ_Util.RandomPointInRing(m_vPos, radius * 0.4, radius * 0.6), 0);
				if (e)
					m_aEffects.Insert(e);
			}
		}

		m_sTitle = "Fog Bank";
		m_sBrief = string.Format("A fog bank has rolled into %1 and won't lift for twenty minutes. In there you'll be hard to hear and hard to see. So will they, until they're on top of you.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Restore(RZ_GameModeComponent gm)
	{
		if (gm.GetEnvironment())
			gm.GetEnvironment().EndFogBank();

		RZ_EventHelp.DeleteAll(m_aEffects);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The fog over " + GetLocationName(gm) + " has lifted.";
	}
}

//------------------------------------------------------------------------------------------------
//! #7 Power Surge: a dark town's power comes back for 10 minutes (the power grid and town
//! memory: Powered, lights, the electronics search bonus). Its alarms go off first: loud for
//! a minute and a half. A town the grid already powers is never picked.
class RZ_Event_PowerSurge : RZ_Event_Timed
{
	protected ref array<vector> m_aAlarms = {};
	protected float m_fAlarmLeft = 90;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = Math.Max(2, RZ_EventHelp.Settings(gm).m_fSurgeMinutes) * 60;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.GetPowerGrid() || !gm.GetTownMemory())
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_PowerGrid grid = gm.GetPowerGrid();
		RZ_TownMemory mem = gm.GetTownMemory();
		array<int> candidates = {};
		foreach (int i, RZ_Settlement t : gm.GetSettlements())
		{
			if (t.m_fInfection > 90 || gm.GetMissions().IsSettlementBusy(i))
				continue;

			if (mem.HasFlag(i, RZ_TownMemory.FLAG_POWERED) || (grid.IsPowered() && grid.IsInRegion(i)))
				continue;

			candidates.Insert(i);
		}

		if (candidates.IsEmpty())
			return false;

		m_iSettlement = candidates.GetRandomElement();
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		m_vPos = s.m_vCenter;
		grid.StartSurge(m_iSettlement, s.m_vCenter, s.m_fRadius + 100);
		for (int a = 0; a < 3; a++)
		{
			m_aAlarms.Insert(LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.7));
		}

		m_sTitle = "Power Surge";
		m_sBrief = string.Format("Something on the old lines has kicked in: %1 has power again, for ten minutes at most. Every shop alarm in town just went off. Anything electric in there is worth looking for while it's on.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_fAlarmLeft <= 0)
			return;

		m_fAlarmLeft -= dt;
		foreach (vector p : m_aAlarms)
		{
			gm.GetHeat().AddHeat(p, 1.5 * dt);
		}

		if (m_fAlarmLeft <= 0)
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 800, m_sTitle, "The alarms in " + GetLocationName(gm) + " have finally given up. The lights are still on.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void Restore(RZ_GameModeComponent gm)
	{
		if (gm.GetPowerGrid())
			gm.GetPowerGrid().EndSurge();
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The power in " + GetLocationName(gm) + " has died again.";
	}
}

//------------------------------------------------------------------------------------------------
//! #10 Gas Station Fire: a petrol station's tanks are leaking and something is smouldering. In
//! three minutes it goes up: the station and up to two buildings beside it burn (the fire
//! system: nothing to search there for a while), infected within 30 m die, and the town's
//! infection drops. Lead them there, or keep clear. Needs a real petrol station (by name).
class RZ_Event_GasStationFire : RZ_Mission
{
	protected bool m_bBlown;
	protected bool m_bWarned;
	protected int m_iKilled;

	protected static const float FUSE = 180;
	protected static const float BLAST_RADIUS = 30;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_EventHelp.FireOn(gm))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Interactions hub = gm.GetInteractions();
		array<int> towns = {};
		foreach (int i, RZ_Settlement t : gm.GetSettlements())
		{
			if (!gm.GetMissions().IsSettlementBusy(i) && !RZ_EventHelp.IsLivedIn(gm, i))
				towns.Insert(i);
		}

		array<string> kinds = {"fuel"};
		for (int tries = 0; tries < 12 && !towns.IsEmpty(); tries++)
		{
			int pick = Math.RandomInt(0, towns.Count());
			int idx = towns[pick];
			towns.Remove(pick);
			RZ_Settlement s = gm.GetSettlement(idx);
			RZ_Landmark station = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 150, kinds);
			if (!station || hub.IsProtected(station.m_vPos))
				continue;

			m_iSettlement = idx;
			m_vPos = station.m_vPos;
			m_sTitle = "Gas Station Fire";
			m_sBrief = string.Format("The tanks under the petrol station in %1 are leaking, and something in there is smouldering. It'll go up in about three minutes. Anything standing near it then won't be standing after. Lead them in, or keep well clear.", s.m_sName);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bBlown)
			return;

		float elapsed = 600 - m_fTimeLeft;
		if (!m_bWarned && elapsed >= FUSE - 45)
		{
			m_bWarned = true;
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 1000, m_sTitle, "The smell of petrol is everywhere around the station in " + GetLocationName(gm) + ". Not long now.");
		}

		if (elapsed >= FUSE)
			Blow(gm);
	}

	//------------------------------------------------------------------------------------------------
	protected void Blow(RZ_GameModeComponent gm)
	{
		m_bBlown = true;
		RZ_Interactions hub = gm.GetInteractions();
		ResourceName blast = gm.GetExplosionPrefab();
		if (!blast.IsEmpty())
		{
			RZ_Util.SpawnPrefab(blast, m_vPos);
			RZ_Util.SpawnPrefab(blast, RZ_Util.RandomPointInRing(m_vPos, 4, 10));
		}

		m_iKilled = RZ_EventHelp.KillInfectedNear(gm, m_vPos, BLAST_RADIUS, 12);
		gm.GetHeat().AddHeat(m_vPos, 120);

		// The station, then up to two buildings beside it: one at a time, never the town
		if (hub && hub.IsActive())
		{
			hub.GetTraps().BurnNear(m_vPos, 25, false);
			array<IEntity> near = {};
			RZ_BuildingQuery.FindBuildings(m_vPos, 45, near, 24);
			int more = 0;
			foreach (IEntity b : near)
			{
				if (more >= 2)
					break;

				if (b && hub.GetTraps().BurnBuilding(b.GetOrigin(), true))
					more++;
			}
		}

		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(-RZ_EventHelp.Settings(gm).m_fStationInfectionDrop);

		m_fTimeLeft = Math.Min(m_fTimeLeft, 60);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		if (!m_bBlown)
			Blow(gm);

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_iKilled > 0)
			return string.Format("The petrol station in %1 went up and took %2 of them with it. The buildings round it are still burning.", GetLocationName(gm), m_iKilled);

		return string.Format("The petrol station in %1 went up. The buildings round it are still burning.", GetLocationName(gm));
	}
}

//------------------------------------------------------------------------------------------------
//! #18 Burning Town: a fire takes hold in a badly infested town for 15 minutes. Every 100 s
//! another building catches (the fire system, one building at a time): nothing to search
//! there for a while, the infected near it die, and the town's infection drops sharply. At
//! most Events > Burn max buildings and never more than 40% of the town; never a town people
//! live in or a faction holds.
class RZ_Event_BurningTown : RZ_Mission
{
	protected ref array<vector> m_aTargets = {};
	protected int m_iBurned;
	protected float m_fNext = 10;

	protected static const float GAP = 100;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 900;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_EventHelp.FireOn(gm))
			return 0;

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Interactions hub = gm.GetInteractions();
		RZ_EventSettings settings = RZ_EventHelp.Settings(gm);
		for (int tries = 0; tries < 6; tries++)
		{
			int idx = PickSettlement(gm, 50, 100);
			RZ_Settlement s = gm.GetSettlement(idx);
			if (!s)
				return false;

			if (RZ_EventHelp.IsLivedIn(gm, idx))
				continue;

			int cap = settings.m_iBurnMax;
			if (gm.GetTownMemory())
			{
				int share = RZ_Util.RoundToInt(Math.Floor(gm.GetTownMemory().BuildingCount(idx) * 0.4));
				if (share < cap)
					cap = share;
			}

			array<IEntity> buildings = {};
			RZ_BuildingQuery.FindBuildings(s.m_vCenter, s.m_fRadius, buildings, 80);
			m_aTargets.Clear();
			foreach (IEntity b : buildings)
			{
				if (m_aTargets.Count() >= cap)
					break;

				vector pos = b.GetOrigin();
				if (!hub.IsProtected(pos) && !hub.GetTraps().IsBurned(pos))
					m_aTargets.Insert(pos);
			}

			if (m_aTargets.Count() < 2)
				continue;

			m_iSettlement = idx;
			m_vPos = s.m_vCenter;
			m_sTitle = "Burning Town";
			m_sBrief = string.Format("%1 is on fire. Nobody's fighting it. Over the next quarter of an hour it'll go house by house, and the infected in there with it. Whatever's in those houses goes too.", s.m_sName);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iBurned >= m_aTargets.Count())
			return;

		m_fNext -= dt;
		if (m_fNext > 0)
			return;

		m_fNext = GAP;
		vector pos = m_aTargets[m_iBurned];
		m_iBurned++;
		RZ_Interactions hub = gm.GetInteractions();
		if (!hub || !hub.GetTraps().BurnBuilding(pos, m_iBurned > 1))
			return;

		RZ_EventHelp.KillInfectedNear(gm, pos, 15, 2);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(-RZ_EventHelp.Settings(gm).m_fBurnInfectionDrop);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("The fire in %1 has burned itself out: %2 buildings gone, and far fewer of them walking around in there.", GetLocationName(gm), m_iBurned);
	}
}

//------------------------------------------------------------------------------------------------
//! #25 Tunnel Rats: infected pour out of three culverts at a town's edge. Every minute each open
//! one sends two more into town while a player is within 700 m, and adds to the town's
//! infection either way (at most Events > Tunnel infection cap). Collapse a culvert (its
//! object, with no infected within 12 m) to stop it; all three sealed leaves a crate someone
//! stashed in the tunnel and takes 5 off the town's infection.
class RZ_Event_TunnelRats : RZ_PuzzleMission
{
	protected ref array<vector> m_aMouths = {};
	protected ref array<bool> m_aSealed = {};
	protected float m_fWaveTimer = 20;
	protected float m_fRise;
	protected int m_iSpawned;
	protected int m_iSealed;
	protected vector m_vLastSealed;

	protected static const int MOUTHS = 3;
	protected static const float WAVE_GAP = 60;
	protected static const float BODY_RADIUS = 700;
	protected static const int MAX_SPAWNED = 30;
	protected static const float CLEAR_RADIUS = 12;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 900;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// Drains run downhill: the lowest of a few points at the edge of town, spread out
		for (int m = 0; m < MOUTHS; m++)
		{
			vector best = LandPointNear(s.m_vCenter, s.m_fRadius * 0.6, s.m_fRadius + 60);
			for (int i = 0; i < 6; i++)
			{
				vector p = LandPointNear(s.m_vCenter, s.m_fRadius * 0.6, s.m_fRadius + 60);
				if (p[1] < best[1] && !IsNearMouth(p, 60))
					best = p;
			}

			int clue = AddClue("A culvert", "A concrete drain mouth, half full of black water. Something moves in the dark further in, and the grate's been pushed off from the inside. The roof's cracked: enough weight on it and it would come down.");
			if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, best, m, "", "Collapse the culvert", clue, "Look into the culvert"))
				return false;

			m_aMouths.Insert(best);
			m_aSealed.Insert(false);
		}

		m_vPos = s.m_vCenter;
		m_sTitle = "Tunnel Rats";
		m_sBrief = string.Format("They're coming up out of the drains in %1. Three culverts at the edge of town, and every minute more of them. Bring the roofs down on them, or the town fills up.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsNearMouth(vector p, float dist)
	{
		foreach (vector m : m_aMouths)
		{
			if (RZ_Util.DistXZ(m, p) < dist)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string where = "";
		foreach (int i, vector p : m_aMouths)
		{
			if (m_aSealed[i])
				continue;

			if (!where.IsEmpty())
				where += ", ";

			where += RZ_SettlementScanner.GridReference(p);
		}

		return super.GetWhereText(gm, from) + "\nOpen culverts (grid): " + where;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fWaveTimer -= dt;
		if (m_fWaveTimer > 0)
			return;

		m_fWaveTimer = WAVE_GAP;
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return;

		float cap = RZ_EventHelp.Settings(gm).m_fTunnelInfectionCap;
		foreach (int i, vector mouth : m_aMouths)
		{
			if (m_aSealed[i])
				continue;

			if (m_fRise < cap)
			{
				s.AddInfection(1);
				m_fRise += 1;
			}

			if (m_iSpawned >= MAX_SPAWNED || RZ_Util.NearestDistance(mouth, players) > BODY_RADIUS)
				continue;

			for (int n = 0; n < 2; n++)
			{
				RZ_TrackedZombie z = gm.GetSpawner().SpawnZombie(RZ_Util.RandomPointInRing(mouth, 1, 4), false, m_iSettlement, m_iId);
				if (!z)
					continue;

				m_iSpawned++;
				RZ_Util.SendGroupTo(z.m_Entity, LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5), gm.GetSpawner().GetMoveWaypointPrefab());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx < 0 || objIdx >= m_aMouths.Count() || m_aSealed[objIdx])
			return;

		vector mouth = m_aMouths[objIdx];
		if (gm.GetSpawner().IsZombieNear(mouth, CLEAR_RADIUS))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Not with them still crawling out of it. Clear the mouth first.");
			return;
		}

		m_aSealed[objIdx] = true;
		m_iSealed++;
		m_vLastSealed = mouth;
		SetObjectLabels(objIdx, "", "");
		gm.GetHeat().AddHeat(mouth, 15);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You kick the cracked slab until the roof gives. Concrete and mud come down with a roar, and whatever was coming up is under it. %1 of %2 sealed.", m_iSealed, m_aMouths.Count()));
		if (m_iSealed >= m_aMouths.Count())
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_iSealed < m_aMouths.Count())
			return;

		SpawnRewardAt(gm, m_vLastSealed);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(-5);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_iSealed >= m_aMouths.Count())
			return "All three culverts in " + GetLocationName(gm) + " are down. Someone found a stash in the last one.";

		return string.Format("The flood out of the drains in %1 has slowed by itself. %2 of 3 culverts were sealed; the town is worse for it.", GetLocationName(gm), m_iSealed);
	}
}
