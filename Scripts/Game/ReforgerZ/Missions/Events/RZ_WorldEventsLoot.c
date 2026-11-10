//! World events with something to take, and a price on it (docs/MISSIONS.md, "World events"):
//! Abandoned Convoy, Military Remnant Camp, Locked Bunker, Wedding Party. Everything here was
//! already inside the Zone: Bastion's own trucks and camps, a lock someone changed, the dead.

//------------------------------------------------------------------------------------------------
//! Shared by the loot events: crates the event spawned, taken back if nobody ever came.
class RZ_LootCache
{
	ref array<IEntity> m_aCrates = {};
	ref array<IEntity> m_aProps = {};
	bool m_bReached;

	//------------------------------------------------------------------------------------------------
	void AddCrate(IEntity crate)
	{
		if (crate)
			m_aCrates.Insert(crate);
	}

	//------------------------------------------------------------------------------------------------
	void AddProp(IEntity prop)
	{
		if (prop)
			m_aProps.Insert(prop);
	}

	//------------------------------------------------------------------------------------------------
	//! A crate still standing within radius of pos.
	bool AnyCrateNear(vector pos, float radius)
	{
		foreach (IEntity c : m_aCrates)
		{
			if (c && RZ_Util.DistXZ(c.GetOrigin(), pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Props always go. Crates stay once someone reached them (they're looting); otherwise they go too.
	void Cleanup()
	{
		RZ_EventHelp.DeleteAll(m_aProps);
		if (!m_bReached)
			RZ_EventHelp.DeleteAll(m_aCrates);

		m_aCrates.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! #6 Abandoned Convoy: Bastion trucks left on the road between two towns in the first week,
//! inside the Zone, their cargo still aboard. It's never free: 40% of the time the cargo is
//! booby-trapped (a Mechanic, Soldier or Police within 25 m spots and cuts the wire first;
//! otherwise the first one to the crates sets it off and one crate is lost), 35% it's watched
//! by a Red Hand crew (Missions > Red Hand group) that fights anyone but its friends, and it
//! always has a few infected. Truck wrecks: Missions > Events > Truck wreck (look only).
class RZ_Event_AbandonedConvoy : RZ_Mission
{
	protected ref RZ_LootCache m_Cache = new RZ_LootCache();
	protected ref RZ_EventGroup m_Watchers;
	protected bool m_bTrap;
	protected bool m_bTrapDone;
	protected bool m_bStirred;
	protected string m_sRoad;

	protected static const float TRAP_RADIUS = 6;
	protected static const float SPOT_RADIUS = 25;
	protected static const float LEAVE_RADIUS = 150;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 30;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Settlement a = gm.GetSettlement(PickSettlement(gm, 0, 100));
		if (!a)
			return false;

		RZ_Settlement b = gm.GetSettlement(PickSettlementAway(gm, a.m_vCenter, 1000, 3500, 0, 100));
		if (!b)
			return false;

		m_vPos = LandPointNear(RZ_ContractHelp.Between(a.m_vCenter, b.m_vCenter, Math.RandomFloat(0.35, 0.65)), 0, 60);
		m_sRoad = string.Format("the road between %1 and %2", a.m_sName, b.m_sName);

		ResourceName wreck = RZ_EventHelp.Settings(gm).m_sTruckWreckPrefab;
		if (!wreck.IsEmpty())
		{
			float yaw = Math.RandomFloat(0, 360);
			m_Cache.AddProp(RZ_Util.SpawnPrefab(wreck, LandPointNear(m_vPos, 6, 10), yaw));
			m_Cache.AddProp(RZ_Util.SpawnPrefab(wreck, LandPointNear(m_vPos, 12, 18), yaw + Math.RandomFloat(-20, 20)));
		}

		m_Cache.AddCrate(gm.SpawnRewardCrate(LandPointNear(m_vPos, 2, 5)));
		m_Cache.AddCrate(gm.SpawnRewardCrate(LandPointNear(m_vPos, 2, 5)));

		float roll = Math.RandomFloat01();
		m_bTrap = roll < 0.4;
		if (roll >= 0.4 && roll < 0.75 && !gm.GetRedHandGroupPrefab().IsEmpty())
			m_Watchers = new RZ_EventGroup(gm.GetRedHandGroupPrefab(), LandPointNear(m_vPos, 90, 150));

		m_sTitle = "Abandoned Convoy";
		m_sBrief = string.Format("Bastion trucks have stood on %1 since the first week, doors open, cargo still in the back. Somebody must have looked before now. Ask yourself why they didn't come back with it.", m_sRoad);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return RZ_SettlementScanner.GridReference(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bStirred && CountPlayersNear(players, m_vPos, 300) > 0)
		{
			m_bStirred = true;
			SpawnGuards(gm, m_vPos, 3, 8, 30);
		}

		if (m_Watchers)
		{
			m_Watchers.Tick(dt);
			if (!m_Watchers.m_bSpawned && CountPlayersNear(players, m_vPos, 500) > 0 && m_Watchers.Spawn())
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Watchers.m_Unit, 300, true);
		}

		if (m_bTrap && !m_bTrapDone)
			CheckTrap(gm, ids, players);

		if (!m_Cache.m_bReached && CountPlayersNear(players, m_vPos, 8) > 0)
			m_Cache.m_bReached = true;

		if (!m_Cache.m_bReached || CountPlayersNear(players, m_vPos, LEAVE_RADIUS) > 0)
			return;

		if (!m_Watchers || !m_Watchers.m_bSpawned || m_Watchers.IsDead())
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckTrap(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		array<string> spotters = {"mechanic", "soldier", "police"};
		int spotter = RZ_EventHelp.BackgroundNear(gm, ids, players, m_vPos, SPOT_RADIUS, spotters);
		if (spotter > 0)
		{
			m_bTrapDone = true;
			gm.NotifyPlayer(spotter, m_sTitle, "A glint of wire at ankle height between the crates, and a grenade taped under the tailboard. You know what you're looking at. You cut it, slowly. Now it's just cargo.");
			return;
		}

		if (CountPlayersNear(players, m_vPos, TRAP_RADIUS) == 0)
			return;

		m_bTrapDone = true;
		ResourceName blast = gm.GetExplosionPrefab();
		if (!blast.IsEmpty())
			RZ_Util.SpawnPrefab(blast, m_vPos);

		gm.GetHeat().AddHeat(m_vPos, 80);
		if (!m_Cache.m_aCrates.IsEmpty())
		{
			RZ_Util.DeleteUnit(m_Cache.m_aCrates[0]);
			m_Cache.m_aCrates.Remove(0);
		}

		RZ_ContractHelp.NotifyNear(gm, m_vPos, 400, m_sTitle, "A tripwire. The tailboard goes up in a flash and half the cargo with it. That's why nobody came back. Everything for a mile heard it.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_Watchers)
			m_Watchers.Delete();

		m_Cache.Cleanup();
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_Cache.m_bReached)
			return "Someone got to the Bastion trucks on " + m_sRoad + ".";

		return "Nobody went near the Bastion trucks on " + m_sRoad + ". Maybe that was wise.";
	}
}

//------------------------------------------------------------------------------------------------
//! #26 Military Remnant Camp: a camp Bastion left behind when the line moved, outside an
//! infested town, with turned soldiers still on guard and two crates of its stores. While
//! Bastion is still around (stage military presence 0.1 or more), anyone who gets within 4 m of
//! a crate takes Bastion Infamy (Missions > Events > Camp Infamy): Bastion keeps lists. Camp
//! prop: Missions > Events > Camp (look only).
class RZ_Event_RemnantCamp : RZ_Mission
{
	protected ref RZ_LootCache m_Cache = new RZ_LootCache();
	protected ref array<int> m_aTaken = {};
	protected bool m_bStirred;
	protected bool m_bBastion;

	protected static const float TAKE_RADIUS = 4;
	protected static const float LEAVE_RADIUS = 150;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 40;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion still holds ground in the Zone: it would notice its stores going.
	static bool BastionActive(RZ_GameModeComponent gm)
	{
		return gm.GetDirector().GetStageSettings().m_fMilitaryPresence >= 0.1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 30, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s || RZ_EventHelp.IsLivedIn(gm, m_iSettlement))
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 100, s.m_fRadius + 400);

		// Abandoned means abandoned: never next to one of Bastion's own holdings
		RZ_Territory territory = gm.GetTerritory();
		if (territory && territory.IsActive() && territory.FindHolding(RZ_EFaction.MILITARY, m_vPos, 1500))
			return false;

		ResourceName camp = RZ_EventHelp.Settings(gm).m_sCampPrefab;
		if (!camp.IsEmpty())
			m_Cache.AddProp(RZ_Util.SpawnPrefab(camp, m_vPos, Math.RandomFloat(0, 360)));

		m_Cache.AddCrate(gm.SpawnRewardCrate(LandPointNear(m_vPos, 3, 8)));
		m_Cache.AddCrate(gm.SpawnRewardCrate(LandPointNear(m_vPos, 3, 8)));

		m_bBastion = BastionActive(gm);
		m_sTitle = "Military Remnant Camp";
		if (m_bBastion)
			m_sBrief = string.Format("Bastion pulled out of a camp outside %1 when the line moved, and left its stores. Its soldiers are still there, after a fashion. It's still Bastion's kit, and Bastion keeps lists.", s.m_sName);
		else
			m_sBrief = string.Format("A camp outside %1 from when Bastion still held the line. Its stores are still there, and so are its soldiers, after a fashion. Nobody's coming back for any of it.", s.m_sName);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bStirred && CountPlayersNear(players, m_vPos, 300) > 0)
		{
			m_bStirred = true;
			SpawnGuards(gm, m_vPos, 4, 6, 25);
			SpawnGuards(gm, m_vPos, 1, 10, 25, true);
		}

		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aTaken.Contains(playerId) || !m_Cache.AnyCrateNear(p.GetOrigin(), TAKE_RADIUS))
				continue;

			m_aTaken.Insert(playerId);
			m_Cache.m_bReached = true;
			if (!m_bBastion)
				continue;

			int infamy = RZ_EventHelp.Settings(gm).m_iCampInfamy;
			gm.AddInfamy(playerId, RZ_EFaction.MILITARY, infamy);
			gm.NotifyPlayer(playerId, "Task Force Bastion", string.Format("Every crate is stencilled with a unit number and a list. Someone will check it. +%1 Infamy with Bastion.", infamy));
		}

		if (m_Cache.m_bReached && CountPlayersNear(players, m_vPos, LEAVE_RADIUS) == 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Cache.Cleanup();
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_Cache.m_bReached)
			return "The old Bastion camp near " + GetLocationName(gm) + " has been picked over.";

		return "Nobody touched the old Bastion camp near " + GetLocationName(gm) + ".";
	}
}

//------------------------------------------------------------------------------------------------
//! #23 Locked Bunker: someone changed the lock on a real locked building in a town to a keypad
//! (door locks: no key fits and no crowbar forces it while the event runs) and split its
//! six-digit code over three notes on bodies around the town. Two crates are inside. Enter the
//! code at the keypad by its door and the building unlocks for everyone. Three wrong codes lock
//! the keypad for 2 minutes and make noise. Without a lockable building (or with door locks
//! off) it's a steel hatch (Missions > Events > Hatch, else the generic object) and the crates
//! come up when it opens. Nobody opens it in 40 minutes: it fails and the crates go.
class RZ_Event_LockedBunker : RZ_PuzzleMission
{
	protected string m_sCode;
	protected IEntity m_Building;
	protected string m_sBuildingKey;
	protected vector m_vDoor;
	protected bool m_bOpened;
	protected int m_iTries;
	protected string m_sPlace;
	protected ref array<vector> m_aNotes = {};
	protected ref array<IEntity> m_aCrates = {};

	protected static const int NOTES = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.KEYPAD) || !HasPrefab(gm, RZ_EPuzzleProp.CLUE))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s || RZ_EventHelp.IsLivedIn(gm, m_iSettlement))
			return false;

		int code = m_Rng.Int(100000, 1000000);
		m_sCode = code.ToString();
		int screen = AddClue("Keypad", "A new steel keypad, bolted on crooked. Six digits, then Enter.");
		if (!FindLockedBuilding(gm, s) && !PlaceHatch(gm, s))
			return false;

		vector keypad = m_vDoor;
		if (!SpawnObject(gm, RZ_EPuzzleProp.KEYPAD, keypad, 0, "", "", screen, "Read the keypad"))
			return false;

		// Three notes on bodies around the town, each with two of the digits
		string writer = RZ_NoteText.FullName(m_Rng);
		for (int i = 0; i < NOTES; i++)
		{
			vector pos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.4, s.m_fRadius + 250);
			for (int tries = 0; tries < 6 && IsNearNote(pos, 120); tries++)
			{
				pos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.4, s.m_fRadius + 250);
			}

			string digits = string.Format("%1 and %2", m_sCode.Substring(i * 2, 1), m_sCode.Substring(i * 2 + 1, 1));
			int clue = AddClue("A folded note", string.Format("\"I changed the lock on %1 before they got in. Six numbers. I split them so no one person could open it. Numbers %2 and %3 are %4. If you're reading this, I didn't make it back. Share it.\" Signed %5.",
				m_sPlace, i * 2 + 1, i * 2 + 2, digits, writer));
			SpawnClue(gm, RZ_EPuzzleProp.BODY, pos, clue, "Search the body");
			m_aNotes.Insert(pos);
		}

		m_vPos = m_vDoor;
		m_sTitle = "Locked Bunker";
		m_sBrief = string.Format("Someone sealed %1 in %2 with a keypad and split the code over three notes before they died. Whatever they locked in there is still in there. Find the notes, or find whoever already has them.", m_sPlace, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsNearNote(vector pos, float dist)
	{
		foreach (vector n : m_aNotes)
		{
			if (RZ_Util.DistXZ(n, pos) < dist)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A real building in town with a lock on its doors, locked now: its lock becomes the keypad.
	protected bool FindLockedBuilding(RZ_GameModeComponent gm, RZ_Settlement s)
	{
		RZ_Interactions hub = gm.GetInteractions();
		if (!hub || !hub.IsActive() || !hub.GetDoors() || !hub.GetDoors().IsOn())
			return false;

		RZ_DoorLocks doors = hub.GetDoors();
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(s.m_vCenter, s.m_fRadius, buildings, 60);
		foreach (IEntity e : buildings)
		{
			RZ_BuildingDoors b = doors.LearnForMission(e);
			if (!b || b.m_aDoors.IsEmpty() || !b.m_aDoors[0] || !doors.IsLockedNow(b) || doors.IsReinforced(b.m_sKey))
				continue;

			m_Building = e;
			m_sBuildingKey = b.m_sKey;
			m_vDoor = RZ_DoorGeometry.OutsidePoint(e, b.m_aDoors[0], 1.2);
			m_sPlace = RZ_Search.TypeNoun(b.m_sType);
			doors.SetReinforced(m_sBuildingKey, true);
			for (int i = 0; i < 2; i++)
			{
				m_aCrates.Insert(gm.SpawnRewardCrate(InsidePoint(e, i), false));
			}

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A point on the building's ground floor, near the middle of its box.
	protected vector InsidePoint(IEntity building, int n)
	{
		vector mins;
		vector maxs;
		if (!RZ_DoorGeometry.LocalBox(building, mins, maxs))
			return building.GetOrigin() + Vector(0, 0.3, 0);

		float dx = (maxs[0] - mins[0]) * 0.15;
		if (n == 1)
			dx = -dx;

		vector local = Vector((mins[0] + maxs[0]) * 0.5 + dx, 0, (mins[2] + maxs[2]) * 0.5);
		vector world = RZ_DoorGeometry.ToWorld(building, local);
		vector ground = RZ_Util.GroundPos(world);
		world[1] = Math.Max(world[1], ground[1]) + 0.3;
		return world;
	}

	//------------------------------------------------------------------------------------------------
	//! No lockable building: a steel hatch in the ground, outside the town.
	protected bool PlaceHatch(RZ_GameModeComponent gm, RZ_Settlement s)
	{
		vector pos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius + 150);
		ResourceName hatch = RZ_EventHelp.Settings(gm).m_sHatchPrefab;
		if (hatch.IsEmpty())
			hatch = gm.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);

		IEntity e = RZ_Util.SpawnPrefab(hatch, pos, m_Rng.Range(0, 360));
		if (!e)
			return false;

		TrackProp(e);
		m_vDoor = LandPointNear(pos, 1.5, 2.5);
		m_vPos = pos;
		m_sPlace = "a steel hatch in the ground";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string where = "";
		foreach (int i, vector p : m_aNotes)
		{
			if (i > 0)
				where += ", ";

			where += RZ_SettlementScanner.GridReference(p);
		}

		return super.GetWhereText(gm, from) + "\nBodies reported near (grid): " + where;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCodeEntered(RZ_GameModeComponent gm, int playerId, int objIdx, string code, IEntity source)
	{
		if (code == m_sCode)
		{
			Open(gm, playerId);
			return;
		}

		m_iTries++;
		SoundAlarm(gm, m_vDoor, 10, false);
		if (m_iTries < 3)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "A flat buzz. Wrong.");
			return;
		}

		m_iTries = 0;
		LockOut(120);
		gm.NotifyPlayer(playerId, m_sTitle, "A long, loud buzz, and the keypad goes dark. It'll take two minutes to wake up again. Everything around heard it.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Open(RZ_GameModeComponent gm, int playerId)
	{
		m_bOpened = true;
		RZ_Interactions hub = gm.GetInteractions();
		if (m_Building && hub)
		{
			hub.GetDoors().SetReinforced(m_sBuildingKey, false);
			hub.GetSearch().OpenBuilding(m_sBuildingKey, false);
			gm.NotifyPlayer(playerId, m_sTitle, "Three beeps, and bolts draw back somewhere inside the frame. It's open, for anyone who comes.");
		}
		else
		{
			for (int i = 0; i < 2; i++)
			{
				gm.SpawnRewardCrate(LandPointNear(m_vPos, 1.5, 3));
			}

			gm.NotifyPlayer(playerId, m_sTitle, "Three beeps. The hatch lifts an inch on its own, and stale air sighs out. Somebody stocked this place for a long wait.");
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Interactions hub = gm.GetInteractions();
		if (m_Building && hub && hub.GetDoors())
			hub.GetDoors().SetReinforced(m_sBuildingKey, false);

		if (!m_bOpened)
			RZ_EventHelp.DeleteAll(m_aCrates);

		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Someone cracked the code on " + m_sPlace + " in " + GetLocationName(gm) + ".";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Nobody cracked the code on " + m_sPlace + " in " + GetLocationName(gm) + ". Whatever's in there stays there.";
	}
}

//------------------------------------------------------------------------------------------------
//! #27 Wedding Party: a wedding by a church that never finished. Six guests in their best
//! (Missions > Events > Wedding guests, else the common infected) and the bride (a special, or
//! Missions > Events > Bride) appear when a player is within 500 m. Some of the dead still wear
//! their jewellery (Missions > Events > Jewellery: about half drop a piece, the bride always,
//! at most 6), which Trader wanderers take instead of Favor. No jewellery prefab: the wedding
//! gifts (a crate) once all of them are down.
class RZ_Event_WeddingParty : RZ_Mission
{
	protected bool m_bSpawned;
	protected bool m_bCleared;
	protected float m_fGrace = 10;
	protected int m_iDropped;
	protected int m_iKilled;

	protected static const int GUESTS = 6;
	protected static const int MAX_JEWELLERY = 6;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 60;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 95);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s || RZ_EventHelp.IsLivedIn(gm, m_iSettlement))
			return false;

		array<string> kinds = {"church"};
		RZ_Landmark church = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 100, kinds);
		string where = "the village hall";
		vector at = s.m_vCenter;
		if (church)
		{
			at = church.m_vPos;
			where = church.m_sName;
		}

		m_vPos = LandPointNear(at, 8, 25);
		m_sTitle = "Wedding Party";
		m_sBrief = string.Format("Someone in %1 kept the reception going after the end of the world. The guests are still outside %2, dancing after a fashion, still in their best. The rings never came off. Nobody will miss the speeches.", s.m_sName, where);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSpawned)
		{
			if (CountPlayersNear(players, m_vPos, 500) == 0)
				return;

			SpawnParty(gm);
			return;
		}

		m_fGrace -= dt;
		if (m_fGrace <= 0 && gm.GetSpawner().CountForMission(m_iId) == 0)
		{
			m_bCleared = true;
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnParty(RZ_GameModeComponent gm)
	{
		m_bSpawned = true;
		RZ_EventSettings settings = RZ_EventHelp.Settings(gm);
		for (int i = 0; i < GUESTS; i++)
		{
			ResourceName guest = "";
			if (settings.m_aWeddingPrefabs && !settings.m_aWeddingPrefabs.IsEmpty())
				guest = settings.m_aWeddingPrefabs.GetRandomElement();

			gm.GetSpawner().SpawnZombie(LandPointNear(m_vPos, 2, 12), false, m_iSettlement, m_iId, guest);
		}

		gm.GetSpawner().SpawnZombie(LandPointNear(m_vPos, 0, 4), true, m_iSettlement, m_iId, settings.m_sBridePrefab);
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId != m_iId)
			return;

		m_iKilled += count;
		ResourceName jewel = RZ_EventHelp.Settings(gm).m_sJewelleryPrefab;
		if (jewel.IsEmpty())
			return;

		for (int i = 0; i < count && m_iDropped < MAX_JEWELLERY; i++)
		{
			if (!special && Math.RandomFloat01() >= 0.5)
				continue;

			RZ_Util.SpawnPrefab(jewel, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(pos, 0.3, 1.2)) + Vector(0, 0.2, 0));
			m_iDropped++;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_bCleared && RZ_EventHelp.Settings(gm).m_sJewelleryPrefab.IsEmpty())
			gm.SpawnRewardCrate(LandPointNear(m_vPos, 1, 4));
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_bCleared)
			return "The wedding party in " + GetLocationName(gm) + " is over. Someone finally threw the rice.";

		return "The wedding party in " + GetLocationName(gm) + " has wandered off. Nobody caught the bouquet.";
	}
}
