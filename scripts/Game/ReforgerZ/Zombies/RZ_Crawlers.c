//! The towns' crawlers (docs/INFECTED.md, "Towns: the crawlers"): a town's own slow infected. How
//! many comes from the town's infection as before, less the kill ledger and any lent out; where
//! they stand comes from the time of day; they drift to noise, refill from the buildings, come out
//! when the town is stirred, walk out to a kill site at night, and give up on a locked door. The
//! spawner (RZ_ZombieSpawner) asks this class where and how many, and spawns the bodies. Server
//! only; on with Zombies > Crawlers > Enabled.

//------------------------------------------------------------------------------------------------
//! Zombies > Crawlers. Off: towns are populated exactly as before.
[BaseContainerProps()]
class RZ_CrawlerSettings
{
	[Attribute("1", desc: "Crawlers: towns hold slow infected that lie up by day and come out at night, drift to noise, refill from the buildings, remember what was killed (the kill ledger) and give up on locked doors. Counted in agents, inside Zombies > Tribes' agent budget. Off: towns are populated exactly as before")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Crawler prefabs: the slowest civilian infected the zombie mods have (Bacon's civilian characters; characters are put in runtime groups of 1-3 with Zombies > Empty group). Empty = Zombies > Common prefabs", "et")]
	ref array<ResourceName> m_aPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bloated crawlers (optional): an exploding variant. From Overrun, a share of the crawlers in towns at 70+ infection", "et")]
	ref array<ResourceName> m_aBloatedPrefabs;

	[Attribute("0.05", desc: "Bloated share: of the crawlers in towns at 70+ infection from Overrun (0 to 0.2)")]
	float m_fBloatedShare;

	[Attribute("1", desc: "Group size, smallest: crawlers spawned together from a character prefab")]
	int m_iGroupMin;

	[Attribute("3", desc: "Group size, largest. A town holds the old count (4 per size class at 100% infection) times the average group size, in crawlers")]
	int m_iGroupMax;

	[Attribute("30", desc: "Kill ledger: each crawler killed in a town is one fewer there; the ledger halves every this many played minutes, and it's saved, so walking away and back never refills a town")]
	float m_fLedgerHalfMinutes;

	[Attribute("200", desc: "Reinforcements: once a town is full, what refills it spawns at the buildings at least this far (m) from every player and walks into the street")]
	float m_fReinforceDistance;

	[Attribute("300", desc: "Drift to noise: a crawler group this close (m) to a remembered noise walks to it once and stays a while (rain: x0.7; a stirred town: 600 m)")]
	float m_fDriftRadius;

	[Attribute("90", desc: "Drift to noise: seconds they stay at the noise before they go back")]
	float m_fDriftSeconds;

	[Attribute("50", desc: "Stir: a town at this stir or more (0-100) is stirred: its crawlers come out and drift to the last noise, and its square makes heat")]
	float m_fStirAt;

	[Attribute("20", desc: "Stir falls this much an hour played")]
	float m_fStirDecayPerHour;

	[Attribute("5", desc: "Stir heat: heat a minute a stirred town's square makes, never above 60 (below the horde's 80, so a stirred town alone never brings a horde)")]
	float m_fStirHeat;

	[Attribute("10", desc: "Siege minutes: infected banging on a locked door give up after this long and the door holds; no crawler spawns within 150 m of it for as long again")]
	float m_fSiegeMinutes;

	[Attribute("2", desc: "Scavengers, fewest: crawlers that walk out at night to a kill site within 1 km of their town")]
	int m_iScavengeMin;

	[Attribute("4", desc: "Scavengers, most")]
	int m_iScavengeMax;
}

//------------------------------------------------------------------------------------------------
//! Where a crawler spawn stands.
enum RZ_ECrawlerSpot
{
	REST,		// 2-6 m outside a building's door, or in its yard
	STREET,		// anywhere in the town
	FIELD,		// the fields at the town's edge (at night)
	STIR		// toward the kill site or the last noise (a stirred town)
}

//------------------------------------------------------------------------------------------------
//! Where it happened, and who the slow ones come out to the next night.
class RZ_KillSite
{
	vector m_vPos;
	int m_iTown = -1;		// within 1 km, or -1
	int m_iDawns;			// dawns since; it goes at the second
	bool m_bDrawn;			// its scavengers went out
	string m_sWho;
}

class RZ_KillSiteSave
{
	float x;
	float z;
	int town;
	int dawns;
	bool drawn;
	string who;
}

//! Saved in state.json: the kill sites (ledgers and stir are saved with the settlements).
class RZ_CrawlerSave
{
	ref array<ref RZ_KillSiteSave> sites = {};
}

//------------------------------------------------------------------------------------------------
class RZ_Crawlers
{
	static const int MAX_SITES = 6;
	static const float SITE_TOWN_RANGE = 1000;
	static const float PART_SPAWN_RANGE = 600;
	static const float HELD_DOOR_RADIUS = 150;
	static const float NOISE_MAX_AGE = 300;

	protected RZ_GameModeComponent m_GM;
	protected RZ_CrawlerSettings m_Settings;
	protected float m_fClock;
	protected bool m_bWasNight;
	protected bool m_bClockSeen;
	protected ref array<ref RZ_KillSite> m_aSites = {};
	protected ref array<ref RZ_InfectedPart> m_aParts = {};
	protected ref array<vector> m_aHeldDoors = {};
	protected ref array<float> m_aHeldUntil = {};
	protected ref array<bool> m_aFull = {};		// per settlement: filled once since a player came near

	//------------------------------------------------------------------------------------------------
	void RZ_Crawlers(RZ_GameModeComponent gm, RZ_CrawlerSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings && m_Settings.m_bEnabled; }
	RZ_CrawlerSettings GetSettings() { return m_Settings; }
	float GetClock() { return m_fClock; }
	array<ref RZ_InfectedPart> GetParts() { return m_aParts; }

	//------------------------------------------------------------------------------------------------
	//! Average crawlers in one spawn (the town's count is the old formula times this).
	float GroupAverage()
	{
		int lo = Math.Max(1, m_Settings.m_iGroupMin);
		int hi = Math.Max(lo, m_Settings.m_iGroupMax);
		return (lo + hi) * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	int RollGroupSize()
	{
		int lo = Math.Max(1, m_Settings.m_iGroupMin);
		int hi = Math.Max(lo, m_Settings.m_iGroupMax);
		return Math.RandomIntInclusive(lo, hi);
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! Every server tick, from the spawner: ledgers, stir, held doors, the night's scavengers.
	void Tick(float dt, array<IEntity> players)
	{
		if (!IsEnabled())
			return;

		m_fClock += dt;
		bool online = !players.IsEmpty();
		bool cured = m_GM.GetCure() && m_GM.GetCure().IsCured();
		float halve = Math.Max(1, m_Settings.m_fLedgerHalfMinutes) * 60;
		RZ_HeatMap heat = m_GM.GetHeat();
		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			// Played minutes only; after the cure a cleared town stays cleared
			if (online && !cured && s.m_fLedger > 0)
			{
				s.m_fLedger = s.m_fLedger * Math.Pow(0.5, dt / halve);
				if (s.m_fLedger < 0.05)
					s.m_fLedger = 0;
			}

			if (online && s.m_fStir > 0)
				s.m_fStir = Math.Max(0, s.m_fStir - m_Settings.m_fStirDecayPerHour * dt / 3600);

			// A stirred town's square makes heat, up to 60: never a horde by itself
			if (online && IsStirred(s))
			{
				float now = heat.GetHeat(s.m_vCenter);
				float add = Math.Min(m_Settings.m_fStirHeat * dt / 60, 60 - now);
				if (add > 0)
					heat.AddHeat(s.m_vCenter, add);
			}
		}

		for (int i = m_aHeldDoors.Count() - 1; i >= 0; i--)
		{
			if (m_fClock >= m_aHeldUntil[i])
			{
				m_aHeldDoors.Remove(i);
				m_aHeldUntil.Remove(i);
			}
		}

		TickNight();
		TickParts(dt, players);
	}

	//------------------------------------------------------------------------------------------------
	//! At nightfall a kill site within 1 km of a town draws a few of its crawlers out; at dawn they
	//! go home, and each kill site goes at its second dawn.
	protected void TickNight()
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (!facts || !facts.KnowsTime())
			return;

		bool night = facts.IsNight();
		if (!m_bClockSeen)
		{
			m_bClockSeen = true;
			m_bWasNight = night;
			return;
		}

		if (night == m_bWasNight)
			return;

		m_bWasNight = night;
		if (night)
		{
			foreach (RZ_KillSite site : m_aSites)
			{
				if (!site.m_bDrawn)
					DrawScavengers(site);
			}

			return;
		}

		// Dawn
		for (int i = m_aParts.Count() - 1; i >= 0; i--)
		{
			RZ_InfectedPart part = m_aParts[i];
			if (part.m_eRole != RZ_EInfectedRole.SCAVENGER)
				continue;

			if (part.HasBody())
				m_GM.GetSpawner().SendPartHome(part);

			m_aParts.Remove(i);
		}

		for (int j = m_aSites.Count() - 1; j >= 0; j--)
		{
			m_aSites[j].m_iDawns = m_aSites[j].m_iDawns + 1;
			if (m_aSites[j].m_iDawns >= 2)
				m_aSites.Remove(j);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawScavengers(RZ_KillSite site)
	{
		site.m_bDrawn = true;
		RZ_Settlement s = m_GM.GetSettlement(site.m_iTown);
		if (!s)
			return;

		int lo = Math.Max(1, m_Settings.m_iScavengeMin);
		int hi = Math.Max(lo, m_Settings.m_iScavengeMax);
		int n = Math.RandomIntInclusive(lo, hi);
		// Never more than the town has
		int have = Math.Floor(BaseAgents(s));
		n = Math.Min(n, have);
		if (n <= 0)
			return;

		vector dir = site.m_vPos - s.m_vCenter;
		dir[1] = 0;
		if (dir.Length() < 1)
			dir = Vector(1, 0, 0);
		dir.Normalize();

		RZ_InfectedPart part = new RZ_InfectedPart();
		part.m_iId = RZ_InfectedBodies.NewPartId();
		part.m_eRole = RZ_EInfectedRole.SCAVENGER;
		part.m_iTown = site.m_iTown;
		part.m_iCount = n;
		part.m_vPos = RZ_Util.GroundPos(s.m_vCenter + dir * s.m_fRadius);
		part.m_vTarget = site.m_vPos;
		part.m_bHasTarget = true;
		part.m_sNote = "from " + s.m_sName;
		m_aParts.Insert(part);
		RZ_Log.Info(string.Format("Crawlers: %1 from %2 are walking out to the kill site at %3.", n, s.m_sName, RZ_SettlementScanner.GridReference(site.m_vPos)));
	}

	//------------------------------------------------------------------------------------------------
	//! Parts move as data; within 600 m of a player they get bodies (the spawner checks the budget
	//! and sight). Parts emptied by kills go.
	protected void TickParts(float dt, array<IEntity> players)
	{
		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		for (int i = m_aParts.Count() - 1; i >= 0; i--)
		{
			RZ_InfectedPart part = m_aParts[i];
			if (part.m_iCount <= 0)
			{
				m_aParts.Remove(i);
				continue;
			}

			part.MoveAsData(dt);
			if (part.HasBody() || players.IsEmpty())
				continue;

			if (RZ_Util.NearestDistance(part.m_vPos, players) <= PART_SPAWN_RANGE)
				spawner.SpawnPart(part, players);
		}
	}

	// ================================================================ How many
	//------------------------------------------------------------------------------------------------
	//! The town's crawlers before the ledger: the old count of spawns times the average group.
	float BaseAgents(RZ_Settlement s)
	{
		RZ_StageSettings stage = m_GM.GetDirector().GetStageSettings();
		float entries = s.m_iSize * m_GM.GetSpawner().GetDensityPerSize() * (s.m_fInfection / 100) * stage.m_fDensityMult;
		return entries * GroupAverage();
	}

	//------------------------------------------------------------------------------------------------
	//! Crawlers that should be in the town now: from the spawner's count (the old formula with its
	//! event, heat and focus multipliers, in spawns) times the average group, less the kill ledger
	//! and those out as data (scavengers without bodies).
	float TargetAgents(int idx, RZ_Settlement s, float entries)
	{
		float target = entries * GroupAverage() - s.m_fLedger - DataOut(idx);
		return Math.Max(0, target);
	}

	//------------------------------------------------------------------------------------------------
	//! The town's crawlers out as data: scavenger parts with no bodies.
	int DataOut(int idx)
	{
		int n = 0;
		foreach (RZ_InfectedPart part : m_aParts)
		{
			if (part.m_iTown == idx && !part.HasBody())
				n += part.m_iCount;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A crawler killed in its town: one fewer there until the ledger halves it away.
	void OnKilled(int idx, int count)
	{
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (s && IsEnabled())
			s.m_fLedger = s.m_fLedger + count;
	}

	//------------------------------------------------------------------------------------------------
	//! Full once: from then on, refills are reinforcements. Reset when the town goes back to data.
	bool IsFull(int idx)
	{
		return idx >= 0 && idx < m_aFull.Count() && m_aFull[idx];
	}

	//------------------------------------------------------------------------------------------------
	void SetFull(int idx, bool full)
	{
		if (idx < 0)
			return;

		while (m_aFull.Count() <= idx)
		{
			m_aFull.Insert(false);
		}

		m_aFull[idx] = full;
	}

	// ================================================================ Where
	//------------------------------------------------------------------------------------------------
	//! Day, dusk, night or storm, from the game clock and the weather. No clock: day.
	protected RZ_ECrawlerSpot RollSpot(RZ_Settlement s, bool reinforce)
	{
		if (reinforce)
			return RZ_ECrawlerSpot.REST;

		RZ_Environment env = m_GM.GetEnvironment();
		if (env && env.IsStorm())
			return RZ_ECrawlerSpot.REST;

		float r = Math.RandomFloat01();
		if (IsStirred(s))
		{
			if (r < 0.2)
				return RZ_ECrawlerSpot.REST;
			if (r < 0.7)
				return RZ_ECrawlerSpot.STREET;
			return RZ_ECrawlerSpot.STIR;
		}

		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts && facts.IsNight())
		{
			if (r < 0.3)
				return RZ_ECrawlerSpot.REST;
			if (r < 0.9)
				return RZ_ECrawlerSpot.STREET;
			return RZ_ECrawlerSpot.FIELD;
		}

		if (facts && facts.IsDusk())
		{
			if (r < 0.5)
				return RZ_ECrawlerSpot.REST;
			return RZ_ECrawlerSpot.STREET;
		}

		if (r < 0.7)
			return RZ_ECrawlerSpot.REST;

		return RZ_ECrawlerSpot.STREET;
	}

	//------------------------------------------------------------------------------------------------
	//! A place for one crawler spawn in the town, by the time of day (unchecked: the spawner checks
	//! water, distance, sight and the rest). reinforce: at a building, to walk into the street.
	vector PickSpot(int idx, RZ_Settlement s, bool reinforce, out RZ_ECrawlerSpot kind)
	{
		kind = RollSpot(s, reinforce);
		switch (kind)
		{
			case RZ_ECrawlerSpot.REST: return RestPoint(s);
			case RZ_ECrawlerSpot.FIELD: return FieldPoint(s);
			case RZ_ECrawlerSpot.STIR: return StirPoint(idx, s);
		}

		return RZ_Util.GroundPos(RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius));
	}

	//------------------------------------------------------------------------------------------------
	//! 2-6 m outside an outer door of a building in the town, else in its yard. Never at a
	//! building's origin. With no building found, a street point.
	protected vector RestPoint(RZ_Settlement s)
	{
		vector probe = RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius);
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(probe, 50, buildings, 8);
		if (buildings.IsEmpty())
			return RZ_Util.GroundPos(probe);

		IEntity b = buildings.GetRandomElement();
		array<IEntity> doors = {};
		RZ_DoorGeometry.FindDoors(b, 0, doors);
		array<IEntity> outer = {};
		foreach (IEntity d : doors)
		{
			if (RZ_DoorGeometry.IsOuterDoor(b, d))
				outer.Insert(d);
		}

		if (!outer.IsEmpty())
			return RZ_Util.GroundPos(RZ_DoorGeometry.OutsidePoint(b, outer.GetRandomElement(), Math.RandomFloat(2, 6)));

		// The yard: just past the building's box
		float reach = 8;
		vector mins;
		vector maxs;
		if (RZ_DoorGeometry.LocalBox(b, mins, maxs))
			reach = Math.Max(maxs[0] - mins[0], maxs[2] - mins[2]) * 0.5 + Math.RandomFloat(3, 6);

		return RZ_Util.GroundPos(RZ_Util.RandomPointInRing(b.GetOrigin(), reach, reach + 1));
	}

	//------------------------------------------------------------------------------------------------
	//! The fields at the town's edge: a field square of the land grid when it has one near, else
	//! open ground just outside the town.
	protected vector FieldPoint(RZ_Settlement s)
	{
		RZ_Land land = m_GM.GetLand();
		vector field;
		if (land && land.IsReady() && land.NearestCellWith(s.m_vCenter, RZ_Land.BIT_FIELD, s.m_fRadius, s.m_fRadius + 600, field))
			return RZ_Util.GroundPos(RZ_Util.RandomPointInRing(field, 0, 100));

		return RZ_Util.GroundPos(RZ_Util.RandomPointInRing(s.m_vCenter, s.m_fRadius + 30, s.m_fRadius + 180));
	}

	//------------------------------------------------------------------------------------------------
	//! A stirred town's crawlers come out toward its kill site, else toward the last noise near it.
	protected vector StirPoint(int idx, RZ_Settlement s)
	{
		vector toward = s.m_vCenter;
		bool found = false;
		foreach (RZ_KillSite site : m_aSites)
		{
			if (site.m_iTown == idx)
			{
				toward = site.m_vPos;
				found = true;
				break;
			}
		}

		if (!found)
		{
			vector noise;
			if (m_GM.GetHeat().FindRecentNoise(s.m_vCenter, s.m_fRadius + 600, NOISE_MAX_AGE, noise))
			{
				toward = noise;
				found = true;
			}
		}

		if (!found)
			return RZ_Util.GroundPos(RZ_Util.RandomPointInRing(s.m_vCenter, s.m_fRadius * 0.6, s.m_fRadius + 100));

		vector dir = toward - s.m_vCenter;
		dir[1] = 0;
		float len = dir.Length();
		if (len < 1)
			return RZ_Util.GroundPos(RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius));

		dir.Normalize();
		float along = Math.RandomFloat(s.m_fRadius * 0.5, Math.Max(s.m_fRadius, Math.Min(len, s.m_fRadius + 300)));
		return RZ_Util.GroundPos(RZ_Util.RandomPointInRing(s.m_vCenter + dir * along, 0, 30));
	}

	//------------------------------------------------------------------------------------------------
	//! A crawler prefab for the town: now and then a bloated one in a dark town from Overrun. Empty
	//! when the list is empty (the spawner then uses Common).
	ResourceName PickPrefab(RZ_Settlement s)
	{
		if (s && !m_Settings.m_aBloatedPrefabs.IsEmpty() && s.m_fInfection >= 70)
		{
			RZ_EOutbreakStage stage = m_GM.GetDirector().GetStage();
			bool late = stage == RZ_EOutbreakStage.OVERRUN || stage == RZ_EOutbreakStage.AFTERMATH;
			if (late && Math.RandomFloat01() < Math.Clamp(m_Settings.m_fBloatedShare, 0, 0.2))
				return m_Settings.m_aBloatedPrefabs.GetRandomElement();
		}

		if (m_Settings.m_aPrefabs.IsEmpty())
			return "";

		return m_Settings.m_aPrefabs.GetRandomElement();
	}

	// ================================================================ Stir
	//------------------------------------------------------------------------------------------------
	bool IsStirred(RZ_Settlement s)
	{
		return s && s.m_fStir >= m_Settings.m_fStirAt;
	}

	//------------------------------------------------------------------------------------------------
	//! Stir for the town at pos (inside its radius + 1 km): a hunt passing through (+30), a kill
	//! in town (+30), a border fight (+20), a scout's call (+10). Tribes call this (phase 2). Stir
	//! never spreads from town to town. Returns the town, or -1.
	int AddStir(vector pos, float amount)
	{
		if (!IsEnabled())
			return -1;

		int idx = m_GM.FindNearestSettlement(pos);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s || RZ_Util.DistXZ(s.m_vCenter, pos) > s.m_fRadius + 1000)
			return -1;

		bool was = IsStirred(s);
		s.m_fStir = Math.Clamp(s.m_fStir + amount, 0, 100);
		if (!was && IsStirred(s))
			RZ_Log.Info(string.Format("Crawlers: %1 is stirred (%2).", s.m_sName, RZ_Util.RoundToInt(s.m_fStir)));

		return idx;
	}

	// ================================================================ Kill sites
	//------------------------------------------------------------------------------------------------
	//! Where someone was taken in the open (tribes call this, phase 2). The next night a few of the
	//! nearest town's crawlers walk out to it, if one is within 1 km. Six on the server at most.
	void AddKillSite(vector pos, string who)
	{
		if (!IsEnabled())
			return;

		RZ_KillSite site = new RZ_KillSite();
		site.m_vPos = RZ_Util.GroundPos(pos);
		site.m_sWho = who;
		int idx = m_GM.FindNearestSettlement(pos);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (s && RZ_Util.DistXZ(s.m_vCenter, pos) <= s.m_fRadius + SITE_TOWN_RANGE)
			site.m_iTown = idx;

		m_aSites.Insert(site);
		while (m_aSites.Count() > MAX_SITES)
		{
			m_aSites.RemoveOrdered(0);
		}

		string town = "no town near";
		if (s && site.m_iTown >= 0)
			town = s.m_sName;

		RZ_Log.Info(string.Format("Crawlers: a kill site at %1 (%2); the slow ones from %3 come out to it at night.", RZ_SettlementScanner.GridReference(pos), who, town));
	}

	// ================================================================ Locked doors
	//------------------------------------------------------------------------------------------------
	//! A door held after a siege: no crawler spawns within 150 m of it for Siege minutes.
	void HoldDoor(vector pos)
	{
		m_aHeldDoors.Insert(pos);
		m_aHeldUntil.Insert(m_fClock + Math.Max(1, m_Settings.m_fSiegeMinutes) * 60);
	}

	//------------------------------------------------------------------------------------------------
	bool IsNearHeldDoor(vector pos)
	{
		foreach (vector p : m_aHeldDoors)
		{
			if (RZ_Util.DistXZ(p, pos) <= HELD_DOOR_RADIUS)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	float SiegeSeconds()
	{
		return Math.Max(1, m_Settings.m_fSiegeMinutes) * 60;
	}

	// ================================================================ Drift to noise
	//------------------------------------------------------------------------------------------------
	//! A crawler group near a new noise walks to it once, stays Drift seconds, then goes back. Not
	//! with a player within P of it: by then Bacon's own senses have it.
	void UpdateDrift(RZ_TrackedZombie z, array<IEntity> players, ResourceName waypoint)
	{
		if (!SCR_AIGroup.Cast(z.m_Entity) || waypoint.IsEmpty())
			return;

		if (RZ_Util.NearestDistance(z.m_vLastPos, players) < RZ_InfectedBodies.P)
			return;

		if (z.m_bDrifting)
		{
			if (m_fClock < z.m_fDriftBackAt)
				return;

			z.m_bDrifting = false;
			RZ_Util.SendGroupTo(z.m_Entity, z.m_vHome, waypoint);
			return;
		}

		float radius = m_Settings.m_fDriftRadius;
		RZ_Environment env = m_GM.GetEnvironment();
		if (env && env.IsRaining())
			radius = radius * 0.7;

		RZ_Settlement s = m_GM.GetSettlement(z.m_iSettlement);
		if (IsStirred(s))
			radius = Math.Max(radius, 600);

		int id;
		vector pos;
		if (!m_GM.GetHeat().NewestNoise(z.m_iHeardNoise, z.m_vLastPos, radius, NOISE_MAX_AGE, id, pos))
			return;

		z.m_iHeardNoise = id;
		z.m_bDrifting = true;
		float walk = RZ_Util.DistXZ(z.m_vLastPos, pos) / RZ_InfectedBodies.DATA_SPEED;
		z.m_fDriftBackAt = m_fClock + walk + Math.Max(10, m_Settings.m_fDriftSeconds);
		RZ_Util.SendGroupTo(z.m_Entity, RZ_Util.RandomPointInRing(pos, 0, 12), waypoint);
	}

	// ================================================================ Save and the admin dump
	//------------------------------------------------------------------------------------------------
	RZ_CrawlerSave Save()
	{
		RZ_CrawlerSave data = new RZ_CrawlerSave();
		foreach (RZ_KillSite site : m_aSites)
		{
			RZ_KillSiteSave ks = new RZ_KillSiteSave();
			ks.x = site.m_vPos[0];
			ks.z = site.m_vPos[2];
			ks.town = site.m_iTown;
			ks.dawns = site.m_iDawns;
			ks.drawn = site.m_bDrawn;
			ks.who = site.m_sWho;
			data.sites.Insert(ks);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_CrawlerSave data)
	{
		m_aSites.Clear();
		if (!data || !data.sites)
			return;

		foreach (RZ_KillSiteSave ks : data.sites)
		{
			if (!ks || m_aSites.Count() >= MAX_SITES)
				continue;

			RZ_KillSite site = new RZ_KillSite();
			site.m_vPos = RZ_Util.GroundPos(Vector(ks.x, 0, ks.z));
			site.m_iTown = ks.town;
			site.m_iDawns = ks.dawns;
			site.m_bDrawn = ks.drawn;
			site.m_sWho = ks.who;
			m_aSites.Insert(site);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "ledger 12, stirred 1, out 3, sites 2"
	string StatusText()
	{
		if (!IsEnabled())
			return "crawlers off";

		float ledger = 0;
		int stirred = 0;
		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			ledger += s.m_fLedger;
			if (IsStirred(s))
				stirred++;
		}

		int away = 0;
		foreach (RZ_InfectedPart part : m_aParts)
		{
			away += part.m_iCount;
		}

		return string.Format("crawlers: ledger %1, stirred towns %2, out %3, kill sites %4, held doors %5",
			RZ_Util.RoundToInt(ledger), stirred, away, m_aSites.Count(), m_aHeldDoors.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! Every town with something going on, every part and kill site, to the log.
	void Dump()
	{
		RZ_Log.Info("Tribes dump | " + StatusText());
		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		foreach (int idx, RZ_Settlement s : m_GM.GetSettlements())
		{
			int present = spawner.CountAgentsForSettlement(idx);
			if (present == 0 && s.m_fLedger < 0.5 && s.m_fStir < 1 && DataOut(idx) == 0)
				continue;

			RZ_Log.Info(string.Format("Tribes dump | town %1: infection %2, crawlers %3 (base %4), ledger %5, stir %6, out %7, full %8",
				s.m_sName, RZ_Util.RoundToInt(s.m_fInfection), present, RZ_Util.RoundToInt(BaseAgents(s)),
				RZ_Util.RoundToInt(s.m_fLedger), RZ_Util.RoundToInt(s.m_fStir), DataOut(idx), IsFull(idx)));
		}

		foreach (RZ_KillSite site : m_aSites)
		{
			RZ_Log.Info(string.Format("Tribes dump | kill site at %1: %2, town %3, dawns %4, drawn %5",
				RZ_SettlementScanner.GridReference(site.m_vPos), site.m_sWho, site.m_iTown, site.m_iDawns, site.m_bDrawn));
		}

		foreach (RZ_InfectedPart part : m_aParts)
		{
			RZ_Log.Info("Tribes dump | " + part.Describe());
		}
	}
}
