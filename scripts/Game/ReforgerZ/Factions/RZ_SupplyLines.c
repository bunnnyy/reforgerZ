//! Supply trucks (docs/DESIGN.md, "Supply trucks").
//!
//! Outposts and Haven safehouses only restock when a real truck (a vehicle with an AI
//! crew) drives in from the faction's Stronghold and parks. Each faction has at most one
//! truck on the road, leaving every 40-60 minutes at a random time. A truck that's stuck
//! with no player within 1 km is quietly moved past the obstacle; with a player near, it
//! stays stuck (a chance to help it, or rob it).
//!
//! A truck that's destroyed, robbed (its crew killed) or never arrives leaves its destination
//! short until the next delivery: some items out, prices +25-50%, maybe the top tier locked,
//! and for a Bastion outpost no Requisition. Players can escort a truck (Fame), drive it in
//! themselves once the crew is dead (more Fame), or steal it to a rival's stronghold (Red Hand
//! Fame, the owner's Infamy). Haven trucks carry food: arriving adds to the safehouse's stock.
//!
//! Needs territory (the strongholds and outposts) and, per faction, a truck and a crew
//! prefab. A faction without them counts its outposts as always supplied.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Supply.
[BaseContainerProps()]
class RZ_SupplySettings
{
	[Attribute("1", desc: "Supply trucks on (needs Territory, a move waypoint prefab and, per faction, a truck and a crew prefab)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven truck: a drivable vehicle (a civilian truck or van)", "et")]
	ResourceName m_sHavenTruck;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion truck: a drivable military truck", "et")]
	ResourceName m_sBastionTruck;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand truck: a drivable vehicle", "et")]
	ResourceName m_sRedHandTruck;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon truck: a drivable van or truck", "et")]
	ResourceName m_sHalcyonTruck;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven crew: an AI group on SURV (1-2 people); the first one drives", "et")]
	ResourceName m_sHavenCrew;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion crew: an AI group on MIL", "et")]
	ResourceName m_sBastionCrew;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand crew: an AI group on BAND", "et")]
	ResourceName m_sRedHandCrew;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon crew: an AI group on INST", "et")]
	ResourceName m_sHalcyonCrew;

	[Attribute("60", desc: "Shortest real minutes between one faction's trucks (divided by season speed)")]
	float m_fDepartMinMinutes;

	[Attribute("90", desc: "Longest real minutes between one faction's trucks")]
	float m_fDepartMaxMinutes;

	[Attribute("1000", desc: "A stuck truck is quietly moved on only if no player is this close (m)")]
	float m_fUnstickRadius;

	[Attribute("90", desc: "Seconds without moving before a truck counts as stuck")]
	float m_fStuckSeconds;

	[Attribute("8", desc: "Stock a delivery brings to an outpost (a Bastion Requisition draws one)")]
	int m_iDeliveryStock;

	[Attribute("6", desc: "Food units a Haven truck adds to a safehouse")]
	int m_iHavenFood;

	[Attribute("45", desc: "Real minutes for an outpost to use up one unit of stock (with players online)")]
	float m_fStockDrainMinutes;

	[Attribute("0.3", desc: "Share of a short shop's items that are out")]
	float m_fOutOfStockShare;

	[Attribute("4", desc: "Fame with the owner for escorting a truck that arrives (and its Favor below)")]
	int m_iEscortFame;

	[Attribute("5", desc: "Favor for escorting a truck that arrives")]
	int m_iEscortFavor;

	[Attribute("8", desc: "Fame with the owner for driving a truck in yourself after its crew died")]
	int m_iDriveFame;

	[Attribute("6", desc: "Red Hand Fame for driving a stolen truck to a rival's stronghold")]
	int m_iStealFame;

	[Attribute("10", desc: "Infamy with the truck's owner for that")]
	int m_iStealInfamy;
}

//------------------------------------------------------------------------------------------------
class RZ_SupplyStateSave
{
	int kind;
	int index;
	int faction;
	int stock;
	bool shortage;
	float priceMult = 1;
	bool topLocked;
	int seed;
}

//------------------------------------------------------------------------------------------------
class RZ_SupplySave
{
	float clock;
	ref array<float> nextDeparture = {};
	ref array<ref RZ_SupplyStateSave> states = {};
}

//------------------------------------------------------------------------------------------------
//! Stock and shortage at one destination: an outpost (by holding id) or a Haven safehouse (by index).
class RZ_SupplyState
{
	int m_iKind;
	int m_iIndex;
	RZ_EFaction m_eFaction;
	int m_iStock;
	bool m_bShortage;
	float m_fPriceMult = 1;
	bool m_bTopLocked;
	int m_iSeed;
	float m_fDrainTimer;
	float m_fLastDelivered = -1;

	//------------------------------------------------------------------------------------------------
	RZ_SupplyStateSave ToSave()
	{
		RZ_SupplyStateSave s = new RZ_SupplyStateSave();
		s.kind = m_iKind;
		s.index = m_iIndex;
		s.faction = m_eFaction;
		s.stock = m_iStock;
		s.shortage = m_bShortage;
		s.priceMult = m_fPriceMult;
		s.topLocked = m_bTopLocked;
		s.seed = m_iSeed;
		return s;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_SupplyState FromSave(RZ_SupplyStateSave s)
	{
		RZ_SupplyState st = new RZ_SupplyState();
		st.m_iKind = s.kind;
		st.m_iIndex = s.index;
		st.m_eFaction = s.faction;
		st.m_iStock = Math.Max(0, s.stock);
		st.m_bShortage = s.shortage;
		st.m_fPriceMult = Math.Clamp(s.priceMult, 1, 1.5);
		st.m_bTopLocked = s.topLocked;
		st.m_iSeed = s.seed;
		return st;
	}
}

//------------------------------------------------------------------------------------------------
//! One truck on the road. Runtime only: a restart takes it off the road (the next one leaves
//! on schedule, and nobody is short because of it).
class RZ_SupplyTruck
{
	RZ_EFaction m_eFaction;
	int m_iDestKind;
	int m_iDestIndex;
	vector m_vDest;
	string m_sDestName;
	string m_sFromName;
	string m_sDriver;
	IEntity m_Vehicle;
	IEntity m_Crew;
	float m_fAge;
	float m_fDeadline;
	bool m_bBoarded;		// its crew is aboard and on the way (until then it waits in the yard)
	bool m_bNoDriverLogged;	// the log has said nobody came to drive it
	vector m_vLastPos;
	float m_fStill;
	float m_fCheckTimer;
	bool m_bCrewDead;
	bool m_bFailed;
	bool m_bDone;			// delivered, failed for good or handed over: only cleanup left
	float m_fDoneAt;
	float m_fOrphanUntil;	// robbed: a player may still drive it in or away until then
	int m_iChecks;
	ref array<int> m_aEscortIds = {};
	ref array<int> m_aEscortCounts = {};
	bool m_bConvoy;			// sent by a story arc (The Last Convoy)
	ref array<int> m_aShooters = {};	// fired near it while the crew lived, with no infected about
}

//------------------------------------------------------------------------------------------------
class RZ_SupplyLines
{
	static const int KIND_OUTPOST = 0;
	static const int KIND_SAFEHOUSE = 1;

	//! Truck results told to missions (RZ_Mission.OnTruckEvent)
	static const int TRUCK_ARRIVED = 0;
	static const int TRUCK_DESTROYED = 1;
	static const int TRUCK_ROBBED = 2;
	static const int TRUCK_LATE = 3;
	static const int TRUCK_STOLEN = 4;
	//! The post it was for changed hands or was abandoned: a request that never left, or a truck
	//! that turned back. OnTruckEvent's `driver` is that holding's id instead of a player.
	static const int TRUCK_CALLED_OFF = 5;

	protected static const float CHECK_INTERVAL = 30;
	protected static const float ARRIVE_RADIUS = 50;
	protected static const float NEAR_STRONGHOLD = 300;
	protected static const float ESCORT_RADIUS = 150;
	protected static const float STEAL_RADIUS = 80;
	protected static const float ORPHAN_SECONDS = 1200;
	protected static const float PARKED_SECONDS = 900;
	protected static const float CLEANUP_RADIUS = 300;
	protected static const float TRUCK_SPEED = 4;			// m/s for the deadline (slow, generous)
	protected static const float BOARD_WAIT = 90;			// s a truck waits for its whole crew before it goes with whoever is aboard
	protected static const float BOARD_RADIUS = 30;			// crew further from the truck aren't seated (never pulled in from afar)
	protected static const float SHORTAGE_TIP_RADIUS = 150;
	protected static const float SHORTAGE_NEWS_RADIUS = 600;

	protected RZ_GameModeComponent m_GM;
	protected RZ_SupplySettings m_Settings;
	protected ref array<ref RZ_SupplyState> m_aStates = {};
	protected ref array<ref RZ_SupplyTruck> m_aTrucks = {};
	protected ref array<float> m_aNextDeparture = {0, 0, 0, 0};
	protected ref array<int> m_aConvoyTo = {-1, -1, -1, -1};	// story arcs: the next truck goes to this holding
	protected ref RZ_SupplySave m_Restored;
	protected ref set<string> m_aWarned = new set<string>();
	protected float m_fClock;
	protected float m_fCheckTimer;
	protected bool m_bActive;

	//------------------------------------------------------------------------------------------------
	void RZ_SupplyLines(RZ_GameModeComponent gm, RZ_SupplySettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsActive() { return m_bActive; }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_SupplySave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SupplySave Save()
	{
		RZ_SupplySave data = new RZ_SupplySave();
		data.clock = m_fClock;
		foreach (float next : m_aNextDeparture)
		{
			data.nextDeparture.Insert(next);
		}

		foreach (RZ_SupplyState st : m_aStates)
		{
			data.states.Insert(st.ToSave());
		}

		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After the territory.
	void Setup()
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!m_Settings.m_bEnabled || !territory || !territory.IsActive())
		{
			RZ_Log.Info("Supply trucks are off. Outposts count as supplied; safehouse food comes from players.");
			return;
		}

		m_bActive = true;
		if (m_Restored)
		{
			m_fClock = m_Restored.clock;
			if (m_Restored.nextDeparture)
			{
				for (int i = 0; i < m_Restored.nextDeparture.Count() && i < RZ_Factions.COUNT; i++)
				{
					m_aNextDeparture[i] = m_Restored.nextDeparture[i];
				}
			}

			if (m_Restored.states)
			{
				foreach (RZ_SupplyStateSave s : m_Restored.states)
				{
					m_aStates.Insert(RZ_SupplyState.FromSave(s));
				}
			}
		}

		m_Restored = null;
		string running = "";
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			// The first truck leaves 10-40 minutes in
			if (m_aNextDeparture[f] <= m_fClock)
				m_aNextDeparture[f] = m_fClock + Math.RandomFloat(10, 40) * 60;

			if (!IsFactionRunning(f))
				continue;

			if (!running.IsEmpty())
				running += ", ";

			running += RZ_Factions.GetShortName(f);
		}

		if (running.IsEmpty())
			running = "none (no truck or crew prefabs, or no move waypoint)";

		RZ_Log.Info("Supply trucks: " + running + ".");
	}

	//------------------------------------------------------------------------------------------------
	//! Trucks run for this faction: on, with a truck, a crew and the move waypoint prefab.
	bool IsFactionRunning(int f)
	{
		if (!m_bActive || !RZ_Factions.IsValid(f))
			return false;

		return !TruckPrefab(f).IsEmpty() && !CrewPrefab(f).IsEmpty() && !m_GM.GetSpawner().GetMoveWaypointPrefab().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Story arcs (The Last Convoy): the faction's next truck goes to this outpost, leaving within
	//! delaySeconds (or when its truck on the road is done). False if its trucks don't run.
	bool RequestConvoy(int f, int holdingId, float delaySeconds)
	{
		if (!m_bActive || !IsFactionRunning(f) || !m_GM.GetTerritory().GetHolding(holdingId))
			return false;

		m_aConvoyTo[f] = holdingId;
		m_aNextDeparture[f] = Math.Min(m_aNextDeparture[f], m_fClock + delaySeconds);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "" (trucks don't run) under the season's No Vehicles rule.
	protected ResourceName TruckPrefab(int f)
	{
		if (RZ_Seasons.Mods().m_bNoVehicles)
			return "";

		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return m_Settings.m_sHavenTruck;
			case RZ_EFaction.MILITARY: return m_Settings.m_sBastionTruck;
			case RZ_EFaction.BANDITS: return m_Settings.m_sRedHandTruck;
			case RZ_EFaction.INSTITUTE: return m_Settings.m_sHalcyonTruck;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName CrewPrefab(int f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return m_Settings.m_sHavenCrew;
			case RZ_EFaction.MILITARY: return m_Settings.m_sBastionCrew;
			case RZ_EFaction.BANDITS: return m_Settings.m_sRedHandCrew;
			case RZ_EFaction.INSTITUTE: return m_Settings.m_sHalcyonCrew;
		}

		return "";
	}

	// ================================================================ States
	//------------------------------------------------------------------------------------------------
	//! Matched by faction too: an outpost that changed hands starts a new state for its new owner.
	protected RZ_SupplyState FindState(int kind, int index, int f)
	{
		foreach (RZ_SupplyState st : m_aStates)
		{
			if (st.m_iKind == kind && st.m_iIndex == index && st.m_eFaction == f)
				return st;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The state for a destination, made on first use: a new outpost arrives with a little stock.
	protected RZ_SupplyState GetState(int kind, int index, RZ_EFaction f)
	{
		RZ_SupplyState st = FindState(kind, index, f);
		if (st)
			return st;

		st = new RZ_SupplyState();
		st.m_iKind = kind;
		st.m_iIndex = index;
		st.m_eFaction = f;
		st.m_iStock = Math.Max(1, m_Settings.m_iDeliveryStock / 2);
		st.m_iSeed = Math.RandomInt(1, 1000000);
		m_aStates.Insert(st);
		return st;
	}

	//------------------------------------------------------------------------------------------------
	//! Territory: an outpost changed hands or was abandoned. Its stock and shortage go with it,
	//! and a truck still on its way there turns back: no delivery, no reward, no shortage. The
	//! roads hear about it (RZ_WorldFacts.Changed), and The Last Convoy ends with it.
	void OnHoldingChanged(int holdingId)
	{
		for (int i = m_aStates.Count() - 1; i >= 0; i--)
		{
			if (m_aStates[i].m_iKind == KIND_OUTPOST && m_aStates[i].m_iIndex == holdingId)
				m_aStates.Remove(i);
		}

		// A convoy asked for and not yet gone: there's nothing left to send it to
		RZ_StoryArcs arcs = m_GM.GetStoryArcs();
		RZ_Holding post = m_GM.GetTerritory().GetHolding(holdingId);
		for (int f = 0; f < m_aConvoyTo.Count(); f++)
		{
			if (m_aConvoyTo[f] != holdingId)
				continue;

			m_aConvoyTo[f] = -1;
			if (arcs && post)
				arcs.OnConvoyCalledOff(f, holdingId, post.m_sName);

			// A mission waiting for that truck (Convoy Escort, Hijack the Convoy) hears it too
			vector at = vector.Zero;
			if (post)
				at = post.m_vPos;

			m_GM.GetMissions().OnTruckEvent(f, TRUCK_CALLED_OFF, at, holdingId);
		}

		foreach (RZ_SupplyTruck t : m_aTrucks)
		{
			if (t.m_bDone || t.m_iDestKind != KIND_OUTPOST || t.m_iDestIndex != holdingId)
				continue;

			// Only cleanup left (Cleanup removes it away from players)
			t.m_bDone = true;
			t.m_fDoneAt = t.m_fAge;
			RZ_Log.Info(string.Format("Supply: %1 truck to %2 called off (the outpost changed hands or was abandoned).", RZ_Factions.GetShortName(t.m_eFaction), t.m_sDestName));
			vector truckAt = t.m_vDest;
			if (t.m_Vehicle)
				truckAt = t.m_Vehicle.GetOrigin();

			m_GM.GetMissions().OnTruckEvent(t.m_eFaction, TRUCK_CALLED_OFF, truckAt, holdingId);
			if (t.m_bConvoy && arcs && arcs.OnConvoyCalledOff(t.m_eFaction, holdingId, t.m_sDestName))
				continue;

			TellCalledOff(t, post);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A truck turned back because its post changed hands (`post` has the new owner) or was
	//! abandoned (`post` still has the old one, or is gone).
	protected void TellCalledOff(RZ_SupplyTruck t, RZ_Holding post)
	{
		string who = RZ_Factions.GetNameInText(t.m_eFaction);
		string headline;
		string rumour;
		if (post && post.m_eFaction != t.m_eFaction)
		{
			headline = string.Format("%1's truck for %2 turned round on the road: the post there is %3's now.", RZ_Util.Capitalize(who), t.m_sDestName, RZ_Factions.GetNameInText(post.m_eFaction));
			rumour = string.Format("\"%1's truck for %2 came back full. No point unloading for %3.\"", RZ_Util.Capitalize(who), t.m_sDestName, RZ_Factions.GetNameInText(post.m_eFaction));
		}
		else
		{
			headline = string.Format("%1's truck for %2 turned round on the road. There's nobody left there to unload it.", RZ_Util.Capitalize(who), t.m_sDestName);
			rumour = string.Format("\"%1 gave up on %2. Their truck turned round halfway and came back full.\"", RZ_Util.Capitalize(who), t.m_sDestName);
		}

		RZ_WorldFacts.Get().Changed(string.Format("supply.%1.called_off", t.m_eFaction), headline, rumour, "", 3);
	}

	//------------------------------------------------------------------------------------------------
	//! The supply state of the shop a quartermaster at pos belongs to: an outpost of `faction`,
	//! or (the Haven) a safehouse. Null at a stronghold, without trucks for that faction, or
	//! anywhere else: those shops are never short.
	RZ_SupplyState FindShopState(int faction, vector pos)
	{
		if (!IsFactionRunning(faction))
			return null;

		RZ_Territory territory = m_GM.GetTerritory();
		RZ_Holding h = territory.FindHolding(faction, pos, 80, true);
		if (h && !IsNearStronghold(faction, h.m_vPos))
			return GetState(KIND_OUTPOST, h.m_iId, faction);

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (faction == RZ_EFaction.SURVIVORS && houses)
		{
			int idx = houses.FindNearest(pos, 80);
			RZ_Safehouse house = houses.Get(idx);
			if (house && !IsNearStronghold(faction, house.m_vPos))
				return GetState(KIND_SAFEHOUSE, idx, faction);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Is this shop item out while the shop is short? Decided per item name and shortage, so it
	//! stays the same until the next delivery.
	bool IsOutOfStock(RZ_SupplyState st, string itemName)
	{
		if (!st || !st.m_bShortage)
			return false;

		int hash = st.m_iSeed % 997;
		string alphabet = "abcdefghijklmnopqrstuvwxyz0123456789";
		string lower = itemName;
		lower.ToLower();
		for (int i = 0; i < lower.Length(); i++)
		{
			hash = (hash * 31 + alphabet.IndexOf(lower.Substring(i, 1)) + 2) % 100003;
		}

		return (hash % 100) < RZ_Util.RoundToInt(Math.Clamp(m_Settings.m_fOutOfStockShare, 0, 1) * 100);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsNearStronghold(int f, vector pos)
	{
		RZ_Holding top = m_GM.GetTerritory().GetStronghold(f);
		return top && RZ_Util.DistXZ(top.m_vPos, pos) < NEAR_STRONGHOLD;
	}

	// ================================================================ Requisition (Bastion outposts)
	//------------------------------------------------------------------------------------------------
	//! For RZ_FactionPerks.TryRequisition at a Bastion quartermaster: 1 = an outpost with stock
	//! (one unit is reserved in `state`), 0 = an outpost without (short, or not resupplied yet),
	//! -1 = not at a supplied outpost (the stronghold, or trucks off: the old rules apply).
	int RequisitionStock(vector qmPos, out RZ_SupplyState state)
	{
		state = null;
		if (!IsFactionRunning(RZ_EFaction.MILITARY))
			return -1;

		RZ_Holding h = m_GM.GetTerritory().FindHolding(RZ_EFaction.MILITARY, qmPos, 80, true);
		if (!h || IsNearStronghold(RZ_EFaction.MILITARY, h.m_vPos))
			return -1;

		state = GetState(KIND_OUTPOST, h.m_iId, RZ_EFaction.MILITARY);
		if (state.m_bShortage || state.m_iStock <= 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! "Morton, 1.2 km NE": the nearest Bastion outpost with stock, "" if none.
	string NearestSuppliedOutpost(vector from)
	{
		array<RZ_Holding> posts = {};
		m_GM.GetTerritory().GetOutposts(RZ_EFaction.MILITARY, posts);
		RZ_Holding best;
		float bestDist = 1000000;
		foreach (RZ_Holding h : posts)
		{
			RZ_SupplyState st = GetState(KIND_OUTPOST, h.m_iId, RZ_EFaction.MILITARY);
			if (st.m_bShortage || st.m_iStock <= 0)
				continue;

			float d = RZ_Util.DistXZ(from, h.m_vPos);
			if (d < bestDist)
			{
				bestDist = d;
				best = h;
			}
		}

		if (!best)
			return "";

		return best.m_sName + ", " + RZ_Util.DirectionText(from, best.m_vPos);
	}

	// ================================================================ Queries for chatter and rumours
	//------------------------------------------------------------------------------------------------
	//! Real seconds until this faction's next truck leaves, -1 if none is due (one is on the
	//! road, or trucks don't run for it).
	float SecondsToDeparture(int f)
	{
		if (!IsFactionRunning(f) || GetTruck(f))
			return -1;

		return Math.Max(0, m_aNextDeparture[f] - m_fClock);
	}

	//------------------------------------------------------------------------------------------------
	//! Where this faction's truck on the road is heading, "" if none.
	string TruckDestination(int f)
	{
		RZ_SupplyTruck t = GetTruck(f);
		if (!t || t.m_bDone || t.m_bFailed)
			return "";

		return t.m_sDestName;
	}

	//------------------------------------------------------------------------------------------------
	//! A short destination of this faction, "" if none.
	string ShortPlace(int f)
	{
		foreach (RZ_SupplyState st : m_aStates)
		{
			if (st.m_eFaction == f && st.m_bShortage)
				return StateName(st);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_SupplyTruck GetTruck(int f)
	{
		foreach (RZ_SupplyTruck t : m_aTrucks)
		{
			if (t.m_eFaction == f)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected string StateName(RZ_SupplyState st)
	{
		vector pos;
		string name;
		if (DestinationOf(st, pos, name))
			return name;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Where a destination is. False if it's gone (an abandoned outpost).
	protected bool DestinationOf(RZ_SupplyState st, out vector pos, out string name)
	{
		if (st.m_iKind == KIND_OUTPOST)
		{
			RZ_Holding h = m_GM.GetTerritory().GetHolding(st.m_iIndex);
			if (!h)
				return false;

			pos = h.m_vPos;
			name = h.m_sName;
			return true;
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_Safehouse house;
		if (houses)
			house = houses.Get(st.m_iIndex);

		if (!house)
			return false;

		pos = house.m_vPos;
		name = house.m_sName;
		return true;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bActive)
			return;

		m_fClock += dt;
		bool online = !ids.IsEmpty();

		for (int i = m_aTrucks.Count() - 1; i >= 0; i--)
		{
			RZ_SupplyTruck t = m_aTrucks[i];
			if (UpdateTruck(t, dt, ids, players))
				m_aTrucks.Remove(i);
		}

		if (online)
			DrainStock(dt);

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (!IsFactionRunning(f) || GetTruck(f) || m_fClock < m_aNextDeparture[f])
				continue;

			// Nobody to see it: wait for players
			if (!online)
			{
				m_aNextDeparture[f] = m_fClock + 300;
				continue;
			}

			TryDepart(f);
		}

		m_fCheckTimer += dt;
		if (m_fCheckTimer >= CHECK_INTERVAL)
		{
			m_fCheckTimer = 0;
			ShortageTips(ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ScheduleNext(int f)
	{
		float lo = Math.Max(5, m_Settings.m_fDepartMinMinutes);
		float hi = Math.Max(lo, m_Settings.m_fDepartMaxMinutes);
		m_aNextDeparture[f] = m_fClock + Math.RandomFloat(lo, hi) * 60 / m_GM.GetSeasonSpeed();
	}

	//------------------------------------------------------------------------------------------------
	//! Outposts slowly use their stock, so Requisition stops if no truck gets through for long.
	protected void DrainStock(float dt)
	{
		float every = Math.Max(5, m_Settings.m_fStockDrainMinutes) * 60;
		foreach (RZ_SupplyState st : m_aStates)
		{
			if (st.m_iKind != KIND_OUTPOST || st.m_iStock <= 0)
				continue;

			st.m_fDrainTimer = st.m_fDrainTimer + dt;
			if (st.m_fDrainTimer >= every)
			{
				st.m_fDrainTimer = 0;
				st.m_iStock = st.m_iStock - 1;
			}
		}
	}

	// ================================================================ Departure
	//------------------------------------------------------------------------------------------------
	protected void TryDepart(int f)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		ScheduleNext(f);

		// Under Pressure: no trucks leave
		if (territory.IsUnderPressure(f))
			return;

		RZ_Holding top = territory.GetStronghold(f);
		if (!top)
			return;

		RZ_SupplyState dest = PickDestination(f);
		bool convoy = false;
		RZ_Holding asked = territory.GetHolding(m_aConvoyTo[f]);
		if (asked && asked.m_eFaction == f)
		{
			dest = GetState(KIND_OUTPOST, asked.m_iId, f);
			convoy = true;
		}

		m_aConvoyTo[f] = -1;
		if (!dest)
			return;

		vector destPos;
		string destName;
		if (!DestinationOf(dest, destPos, destName))
			return;

		RZ_SupplyTruck t = SpawnTruck(f, top, destPos);
		if (!t)
			return;

		t.m_iDestKind = dest.m_iKind;
		t.m_iDestIndex = dest.m_iIndex;
		t.m_vDest = destPos;
		t.m_sDestName = destName;
		t.m_sFromName = top.m_sName;
		t.m_fDeadline = RZ_Util.DistXZ(top.m_vPos, destPos) / TRUCK_SPEED * 2 + 900;
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		t.m_sDriver = RZ_FactionVoice.DriverName(rng, f);
		t.m_bConvoy = convoy;
		m_aTrucks.Insert(t);
		if (convoy && m_GM.GetStoryArcs())
			m_GM.GetStoryArcs().OnConvoyDeparted(f, destName);

		RZ_Log.Info(string.Format("Supply: %1 truck leaves %2 for %3.", RZ_Factions.GetShortName(f), top.m_sName, destName));
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
			life.OnTruckDeparted(f, top.m_sName, destName);
	}

	//------------------------------------------------------------------------------------------------
	//! The destination that needs it most: short ones first, then the least stock, then the
	//! longest without a delivery. Outposts of the faction; for the Haven also its safehouses.
	//! Places right next to the stronghold are stocked by hand and never need a truck.
	protected RZ_SupplyState PickDestination(int f)
	{
		array<RZ_SupplyState> options = {};
		array<RZ_Holding> posts = {};
		m_GM.GetTerritory().GetOutposts(f, posts);
		foreach (RZ_Holding h : posts)
		{
			if (!IsNearStronghold(f, h.m_vPos))
				options.Insert(GetState(KIND_OUTPOST, h.m_iId, f));
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (f == RZ_EFaction.SURVIVORS && houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				if (!IsNearStronghold(f, houses.Get(i).m_vPos))
					options.Insert(GetState(KIND_SAFEHOUSE, i, f));
			}
		}

		RZ_SupplyState best;
		float bestScore = -1;
		foreach (RZ_SupplyState st : options)
		{
			float score = Math.RandomFloat(0, 2);
			if (st.m_bShortage)
				score += 20;

			score += Math.Max(0, m_Settings.m_iDeliveryStock - st.m_iStock);
			if (st.m_iKind == KIND_SAFEHOUSE)
			{
				RZ_Safehouse house = houses.Get(st.m_iIndex);
				if (house && house.m_bShort)
					score += 15;
				if (house && house.m_iFood < house.FoodNeed(5) * 2)
					score += 5;
			}

			if (st.m_fLastDelivered < 0)
				score += 3;
			else
				score += Math.Min(10, (m_fClock - st.m_fLastDelivered) / 600);

			if (score > bestScore)
			{
				bestScore = score;
				best = st;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The truck and its crew near the stronghold. The crew boards on the next tick (the
	//! group's members exist a moment after the group), then drives to a move waypoint.
	protected RZ_SupplyTruck SpawnTruck(int f, RZ_Holding top, vector destPos)
	{
		vector at = RZ_Mission.LandPointNear(top.m_vPos, 20, 45);
		float yaw = Math.Atan2(destPos[0] - at[0], destPos[2] - at[2]) * Math.RAD2DEG;
		IEntity vehicle = RZ_Util.SpawnPrefab(TruckPrefab(f), at + Vector(0, 0.5, 0), yaw);
		if (!vehicle)
			return null;

		IEntity crew = RZ_Util.SpawnPrefab(CrewPrefab(f), RZ_Mission.LandPointNear(at, 3, 6), yaw);
		if (!crew)
		{
			RZ_Util.DeleteUnit(vehicle);
			return null;
		}

		RZ_SupplyTruck t = new RZ_SupplyTruck();
		t.m_eFaction = f;
		t.m_Vehicle = vehicle;
		t.m_Crew = crew;
		t.m_vLastPos = at;

		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.WatchUnit(crew, f);

		return t;
	}

	//------------------------------------------------------------------------------------------------
	protected void Board(RZ_SupplyTruck t)
	{
		t.m_bBoarded = true;
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(t.m_Crew, members);
		foreach (int i, IEntity member : members)
		{
			RZ_Util.BoardVehicle(member, t.m_Vehicle, i == 0);
		}

		RZ_Util.SendGroupTo(t.m_Crew, t.m_vDest, m_GM.GetSpawner().GetMoveWaypointPrefab()); // VERIFY: an AI group seated in a vehicle drives to a move waypoint
	}

	// ================================================================ On the road
	//------------------------------------------------------------------------------------------------
	//! True when the truck is finished with (removed from the list).
	protected bool UpdateTruck(RZ_SupplyTruck t, float dt, array<int> ids, array<IEntity> players)
	{
		t.m_fAge = t.m_fAge + dt;

		if (t.m_bDone)
			return Cleanup(t, players);

		if (!t.m_Vehicle)
		{
			if (!t.m_bFailed)
				Fail(t, "destroyed");

			t.m_bDone = true;
			return Cleanup(t, players);
		}

		if (!t.m_bBoarded)
		{
			if (t.m_fAge >= 2)
				Board(t);

			return false;
		}

		vector pos = t.m_Vehicle.GetOrigin();
		if (RZ_Util.IsDestroyed(t.m_Vehicle))
		{
			if (!t.m_bFailed)
				Fail(t, "destroyed");

			t.m_bDone = true;
			t.m_fDoneAt = t.m_fAge;
			return false;
		}

		int driver = PlayerInTruck(t, ids, players);
		if (!t.m_bCrewDead && RZ_Util.CountAlive(t.m_Crew) == 0)
		{
			t.m_bCrewDead = true;
			t.m_fOrphanUntil = t.m_fAge + ORPHAN_SECONDS;
			if (!t.m_bFailed)
				Fail(t, "robbed");
		}

		// Robbed: a player can still drive it in, or away to a rival's stronghold
		if (t.m_bCrewDead)
		{
			if (driver > 0)
			{
				if (RZ_Util.DistXZ(pos, t.m_vDest) <= ARRIVE_RADIUS)
				{
					Deliver(t, driver);
					return false;
				}

				int receiver = RivalStrongholdAt(t.m_eFaction, pos);
				if (receiver >= 0)
				{
					Stolen(t, driver, receiver);
					return false;
				}
			}

			if (t.m_fAge >= t.m_fOrphanUntil)
			{
				t.m_bDone = true;
				t.m_fDoneAt = t.m_fAge;
			}

			return false;
		}

		if (RZ_Util.DistXZ(pos, t.m_vDest) <= ARRIVE_RADIUS)
		{
			Deliver(t, -1);
			return false;
		}

		t.m_fCheckTimer = t.m_fCheckTimer + dt;
		if (t.m_fCheckTimer >= CHECK_INTERVAL)
		{
			float moved = RZ_Util.DistXZ(pos, t.m_vLastPos);
			if (moved < 8)
				t.m_fStill = t.m_fStill + t.m_fCheckTimer;
			else
				t.m_fStill = 0;

			t.m_fCheckTimer = 0;
			t.m_vLastPos = pos;
			CountEscorts(t, ids, players, pos);

			if (t.m_fStill >= m_Settings.m_fStuckSeconds)
				TryUnstick(t, players, pos);
		}

		if (t.m_fAge >= t.m_fDeadline && !t.m_bFailed)
		{
			Fail(t, "late");
			t.m_bDone = true;
			t.m_fDoneAt = t.m_fAge;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A player sitting in the truck (in a vehicle, within a few metres of it), -1 if none.
	protected int PlayerInTruck(RZ_SupplyTruck t, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), t.m_Vehicle.GetOrigin()) <= 6 && RZ_Util.IsInVehicle(p))
				return ids[i];
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Players near the truck on the road, counted every check. Not the ones riding in it.
	protected void CountEscorts(RZ_SupplyTruck t, array<int> ids, array<IEntity> players, vector pos)
	{
		t.m_iChecks = t.m_iChecks + 1;
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) > ESCORT_RADIUS)
				continue;

			int idx = t.m_aEscortIds.Find(ids[i]);
			if (idx < 0)
			{
				t.m_aEscortIds.Insert(ids[i]);
				t.m_aEscortCounts.Insert(1);
			}
			else
			{
				t.m_aEscortCounts[idx] = t.m_aEscortCounts[idx] + 1;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Stuck with nobody within 1 km: moved 80-150 m on toward its destination, out of sight.
	//! With a player near, it stays stuck.
	protected void TryUnstick(RZ_SupplyTruck t, array<IEntity> players, vector pos)
	{
		if (RZ_Util.NearestDistance(pos, players) <= m_Settings.m_fUnstickRadius)
			return;

		float dist = RZ_Util.DistXZ(pos, t.m_vDest);
		float step = Math.Min(Math.RandomFloat(80, 150), dist - ARRIVE_RADIUS * 0.5);
		if (step <= 5)
			return;

		vector dir = t.m_vDest - pos;
		dir[1] = 0;
		dir = dir * (1 / Math.Max(1, dist));
		vector target = RZ_Mission.LandPointNear(pos + dir * step, 0, 12);
		if (RZ_Util.IsWater(target))
			return;

		float yaw = Math.Atan2(dir[0], dir[2]) * Math.RAD2DEG;
		RZ_Util.MoveVehicle(t.m_Vehicle, target, yaw);
		RZ_Util.RedirectGroup(t.m_Crew, t.m_vDest, m_GM.GetSpawner().GetMoveWaypointPrefab());
		t.m_fStill = 0;
		t.m_vLastPos = target;
	}

	//------------------------------------------------------------------------------------------------
	//! A rival faction's stronghold within reach of pos, -1 if none.
	protected int RivalStrongholdAt(int owner, vector pos)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (f == owner)
				continue;

			RZ_Holding top = territory.GetStronghold(f);
			if (top && RZ_Util.DistXZ(top.m_vPos, pos) <= STEAL_RADIUS)
				return f;
		}

		return -1;
	}

	// ================================================================ Outcomes
	//------------------------------------------------------------------------------------------------
	//! Parked at the destination: stock, food, shortage over. driver > 0: a player drove it in.
	protected void Deliver(RZ_SupplyTruck t, int driver)
	{
		t.m_bDone = true;
		t.m_fDoneAt = t.m_fAge;
		RZ_SupplyState st = GetState(t.m_iDestKind, t.m_iDestIndex, t.m_eFaction);
		st.m_iStock = Math.Max(st.m_iStock, m_Settings.m_iDeliveryStock);
		st.m_bShortage = false;
		st.m_fPriceMult = 1;
		st.m_bTopLocked = false;
		st.m_fDrainTimer = 0;
		st.m_fLastDelivered = m_fClock;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (t.m_iDestKind == KIND_SAFEHOUSE && houses)
			houses.AddFood(t.m_iDestIndex, m_Settings.m_iHavenFood);

		// A robbed truck that still got through counts as a delivery; schedule from now
		if (t.m_bFailed)
			ScheduleNext(t.m_eFaction);

		m_GM.GetTerritory().AddStrength(t.m_eFaction, 1);
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
			life.OnTruckArrived(t.m_eFaction, t.m_sDestName);

		string factionName = RZ_Factions.GetName(t.m_eFaction);
		// Nobody earns Fame for bringing in a truck whose crew they shot
		bool shooter = driver > 0 && t.m_aShooters.Contains(driver);
		if (shooter)
			m_GM.NotifyPlayer(driver, factionName, string.Format("You brought their truck in to %1. They know who was shooting near it before its crew died. Nobody thanks you.", t.m_sDestName));

		if (driver > 0 && !shooter)
		{
			m_GM.AddFame(driver, t.m_eFaction, m_Settings.m_iDriveFame);
			m_GM.NotifyPlayer(driver, factionName, string.Format("You brought their truck in to %1 after its crew was killed. They won't forget it. (+%2 Fame)", t.m_sDestName, m_Settings.m_iDriveFame));
		}

		RewardEscorts(t, driver);
		NotifyNear(t.m_vDest, factionName, RZ_FactionVoice.TruckArrives(t.m_eFaction, t.m_sDestName, t.m_sDriver), SHORTAGE_NEWS_RADIUS);
		m_GM.GetMissions().OnTruckEvent(t.m_eFaction, TRUCK_ARRIVED, t.m_vDest, driver);
		if (t.m_bConvoy && m_GM.GetStoryArcs())
			m_GM.GetStoryArcs().OnConvoyResult(t.m_eFaction, t.m_sDestName, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Players who stayed with the truck for at least 40% of its trip (and two checks).
	protected void RewardEscorts(RZ_SupplyTruck t, int driver)
	{
		int needed = RZ_Util.RoundToInt(Math.Max(2, t.m_iChecks * 0.4 + 0.49));
		string factionName = RZ_Factions.GetName(t.m_eFaction);
		foreach (int i, int playerId : t.m_aEscortIds)
		{
			if (playerId == driver || t.m_aEscortCounts[i] < needed || t.m_aShooters.Contains(playerId))
				continue;

			if (RZ_Reputation.IsHunted(m_GM.GetTier(playerId, t.m_eFaction)))
				continue;

			m_GM.AddFame(playerId, t.m_eFaction, m_Settings.m_iEscortFame);
			m_GM.AddFavor(playerId, t.m_eFaction, m_Settings.m_iEscortFavor);
			m_GM.NotifyPlayer(playerId, factionName, string.Format("The truck made it to %1 with you riding shotgun. (+%2 Fame, +%3 Favor)", t.m_sDestName, m_Settings.m_iEscortFame, m_Settings.m_iEscortFavor));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Destroyed, robbed or late: the destination is short until the next delivery.
	protected void Fail(RZ_SupplyTruck t, string why)
	{
		t.m_bFailed = true;
		ScheduleNext(t.m_eFaction);
		vector lostAt = t.m_vDest;
		if (t.m_Vehicle)
			lostAt = t.m_Vehicle.GetOrigin();

		int lostHow = TRUCK_LATE;
		if (why == "destroyed")
			lostHow = TRUCK_DESTROYED;
		else if (why == "robbed")
			lostHow = TRUCK_ROBBED;

		m_GM.GetMissions().OnTruckEvent(t.m_eFaction, lostHow, lostAt, -1);
		if (t.m_bConvoy && m_GM.GetStoryArcs())
			m_GM.GetStoryArcs().OnConvoyResult(t.m_eFaction, t.m_sDestName, false);

		RZ_SupplyState st = GetState(t.m_iDestKind, t.m_iDestIndex, t.m_eFaction);
		st.m_bShortage = true;
		st.m_fPriceMult = Math.RandomFloat(1.25, 1.5);
		st.m_bTopLocked = Math.RandomFloat01() < 0.5;
		st.m_iSeed = Math.RandomInt(1, 1000000);
		if (t.m_iDestKind == KIND_OUTPOST)
			st.m_iStock = 0;

		m_GM.GetTerritory().AddStrength(t.m_eFaction, -2);
		RZ_Log.Info(string.Format("Supply: %1 truck to %2 lost (%3).", RZ_Factions.GetShortName(t.m_eFaction), t.m_sDestName, why));

		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
		{
			vector at = t.m_vDest;
			if (t.m_Vehicle)
				at = t.m_Vehicle.GetOrigin();

			life.OnTruckLost(t.m_eFaction, t.m_sDestName, t.m_sDriver, why, at);
		}

		// The people at the destination hear it first
		ResetWarnings(st);
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), t.m_vDest) > SHORTAGE_NEWS_RADIUS)
				continue;

			m_GM.NotifyPlayer(ids[i], RZ_Factions.GetName(t.m_eFaction), RZ_FactionVoice.TruckLost(t.m_eFaction, t.m_sDestName));
			RZ_Guidance.Report(ids[i], "shortage", t.m_sDestName);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook: a player firing within 150 m of a truck whose crew is alive, with no
	//! infected near it, is remembered, so they can't kill the crew and then drive it in for Fame.
	void OnPlayerShot(int playerId, vector pos)
	{
		foreach (RZ_SupplyTruck t : m_aTrucks)
		{
			if (t.m_bDone || t.m_bCrewDead || !t.m_Vehicle || t.m_aShooters.Contains(playerId) || t.m_aShooters.Count() >= 8)
				continue;

			vector at = t.m_Vehicle.GetOrigin();
			if (RZ_Util.DistXZ(at, pos) > 150 || m_GM.GetSpawner().IsZombieNear(at, 40))
				continue;

			t.m_aShooters.Insert(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From retaliation: a player's shot hit this crew (from any range). They're a shooter for that
	//! truck, so a long-range kill can't be followed by a drive-in for Fame.
	void OnCrewHit(IEntity crew, int playerId)
	{
		if (!crew || playerId <= 0)
			return;

		foreach (RZ_SupplyTruck t : m_aTrucks)
		{
			if (t.m_Crew == crew && !t.m_aShooters.Contains(playerId) && t.m_aShooters.Count() < 8)
				t.m_aShooters.Insert(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Driven into a rival's stronghold: Red Hand Fame (the receiver's, for a Red Hand truck),
	//! the owner's Infamy, and the cargo is the receiver's now.
	protected void Stolen(RZ_SupplyTruck t, int driver, int receiver)
	{
		t.m_bDone = true;
		t.m_fDoneAt = t.m_fAge;

		int fameWith = RZ_EFaction.BANDITS;
		if (t.m_eFaction == RZ_EFaction.BANDITS)
			fameWith = receiver;

		m_GM.AddFame(driver, fameWith, m_Settings.m_iStealFame);
		m_GM.AddInfamy(driver, t.m_eFaction, m_Settings.m_iStealInfamy);
		m_GM.NotifyPlayer(driver, RZ_Factions.GetName(fameWith), string.Format("You drove %1's truck into %2's yard. They're unloading it already. (+%3 Fame with %4, +%5 Infamy with %1)",
			RZ_Factions.GetNameInText(t.m_eFaction), RZ_Factions.GetNameInText(receiver), m_Settings.m_iStealFame, RZ_Factions.GetNameInText(fameWith), m_Settings.m_iStealInfamy));

		m_GM.GetTerritory().AddStrength(receiver, 1);
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
			life.OnTruckStolen(t.m_eFaction, receiver);

		if (t.m_bConvoy && m_GM.GetStoryArcs())
			m_GM.GetStoryArcs().OnConvoyResult(t.m_eFaction, t.m_sDestName, false);

		m_GM.GetMissions().OnTruckEvent(t.m_eFaction, TRUCK_STOLEN, t.m_Vehicle.GetOrigin(), driver);
	}

	//------------------------------------------------------------------------------------------------
	//! Finished trucks stay parked while anyone's near, then go (no physical traces). A robbed
	//! truck a player sits in is never deleted under them.
	protected bool Cleanup(RZ_SupplyTruck t, array<IEntity> players)
	{
		vector pos = vector.Zero;
		if (t.m_Vehicle)
			pos = t.m_Vehicle.GetOrigin();
		else if (t.m_Crew)
			pos = RZ_Util.GetUnitPosition(t.m_Crew);

		float nearest = RZ_Util.NearestDistance(pos, players);
		bool parkedLong = t.m_fAge - t.m_fDoneAt >= PARKED_SECONDS;
		if (nearest <= CLEANUP_RADIUS && !(parkedLong && nearest > ESCORT_RADIUS))
			return false;

		if (t.m_Vehicle && nearest <= 10)
			return false;

		RZ_Util.DeleteUnit(t.m_Crew);
		RZ_Util.DeleteUnit(t.m_Vehicle);
		return true;
	}

	// ================================================================ Shortages near players
	//------------------------------------------------------------------------------------------------
	//! Players who walk up to a short destination hear about it (the "shortage" tip has its own
	//! cooldown; this only asks once per player per shortage).
	protected void ShortageTips(array<int> ids, array<IEntity> players)
	{
		foreach (RZ_SupplyState st : m_aStates)
		{
			if (!st.m_bShortage)
				continue;

			vector pos;
			string name;
			if (!DestinationOf(st, pos, name))
				continue;

			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), pos) > SHORTAGE_TIP_RADIUS)
					continue;

				string key = string.Format("%1:%2:%3:%4", st.m_iKind, st.m_iIndex, st.m_iSeed, ids[i]);
				if (m_aWarned.Contains(key))
					continue;

				m_aWarned.Insert(key);
				RZ_Guidance.Report(ids[i], "shortage", name);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetWarnings(RZ_SupplyState st)
	{
		if (m_aWarned.Count() > 500)
			m_aWarned.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyNear(vector pos, string title, string text, float radius)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				m_GM.NotifyPlayer(ids[i], title, text);
		}
	}

	// ================================================================ Missions (Missions/Blocks/RZ_VehicleZone.c)
	//------------------------------------------------------------------------------------------------
	//! The faction's truck on the road (or just finished), or null. Missions read its vehicle,
	//! crew and destination; they may stop and restart the crew, nothing else.
	RZ_SupplyTruck GetActiveTruck(int f)
	{
		if (!m_bActive)
			return null;

		return GetTruck(f);
	}

	//------------------------------------------------------------------------------------------------
	//! A convoy is already asked for (a story arc or a mission): don't ask again.
	bool HasConvoyRequest(int f)
	{
		if (!RZ_Factions.IsValid(f))
			return false;

		return m_aConvoyTo[f] >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! An outpost of the faction a truck can be sent to (not right next to the stronghold), or -1.
	int PickConvoyOutpost(int f)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory)
			return -1;

		array<RZ_Holding> posts = {};
		territory.GetOutposts(f, posts);
		array<int> options = {};
		foreach (RZ_Holding h : posts)
		{
			if (!IsNearStronghold(f, h.m_vPos))
				options.Insert(h.m_iId);
		}

		if (options.IsEmpty())
			return -1;

		return options.GetRandomElement();
	}
}
