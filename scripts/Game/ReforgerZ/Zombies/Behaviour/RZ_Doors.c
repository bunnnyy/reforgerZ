//! The infected at a locked door (docs/INFECTED.md, "Doors"). Built before this: the doorway
//! blockers, banging (heat) and the siege cap (10 minutes, then they give up and the door holds),
//! and Bacon opening shut, unlocked doors (lab B10). This adds:
//! - **Another way in:** after 30 s banging at a locked building's door, the infected there go round
//!   to another of its outside doors within 20 m that's standing open. If none, they wait out the siege.
//! - **A weak door breaks:** only for a pack's leader, or a bold pack's night hunt with 3 or more at the
//!   door, and only a door whose building's or door's prefab name has one of **Weak door words** (Shed,
//!   Barn, Garage, Wood). Never within 150 m of a lived-in place (a holding, its outposts and
//!   quartermaster, a safehouse, the lab). After 45 s of banging, with a tell at 20 s, the lock is
//!   forced (the built Forced state: open and broken until the reset).
//! Hiding behind a locked door stays a real escape, but not always behind a shed's.

//------------------------------------------------------------------------------------------------
class RZ_DoorWatch
{
	string m_sKey;
	float m_fSince;
	float m_fLast;
	bool m_bWentRound;
	bool m_bWarned;
}

//------------------------------------------------------------------------------------------------
class RZ_InfectedDoors
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Behaviour m_BH;
	protected ref map<string, ref RZ_DoorWatch> m_mWatch = new map<string, ref RZ_DoorWatch>();
	protected ref array<string> m_aWeakWords = {};
	protected int m_iWentRound;
	protected int m_iBroken;

	static const float AT_DOOR = 6;			// m: a unit with a member this close is at the door
	static const float ROUND_AFTER = 30;	// s of banging before they look for another way in
	static const float ROUND_RADIUS = 20;
	static const float TELL_AFTER = 20;
	static const float BREAK_AFTER = 45;
	static const float RESET_GAP = 30;		// s without a bang: the watch starts again
	static const float PROTECTED = 150;		// m round a lived-in place: never broken

	//------------------------------------------------------------------------------------------------
	void RZ_InfectedDoors(RZ_GameModeComponent gm, RZ_Behaviour bh)
	{
		m_GM = gm;
		m_BH = bh;
		if (bh.GetSettings())
			RZ_ItemHelper.Words(bh.GetSettings().m_sWeakDoorWords, m_aWeakWords);
	}

	//------------------------------------------------------------------------------------------------
	//! A locked doorway is being banged on (RZ_ZombieSpawner.OnDoorBang, from the door blockers).
	void OnBang(vector at)
	{
		RZ_Interactions hub = m_GM.GetInteractions();
		if (!hub || !hub.GetDoors())
			return;

		IEntity door;
		RZ_BuildingDoors b = FindBuilding(hub.GetDoors(), at, door);
		if (!b || !door)
			return;

		float now = m_BH.Now();
		RZ_DoorWatch w = m_mWatch.Get(b.m_sKey);
		if (!w)
		{
			if (m_mWatch.Count() > 40)
				Prune(now);

			w = new RZ_DoorWatch();
			w.m_sKey = b.m_sKey;
			m_mWatch.Set(b.m_sKey, w);
		}

		if (now - w.m_fLast > RESET_GAP)
		{
			w.m_fSince = now;
			w.m_bWentRound = false;
			w.m_bWarned = false;
		}

		w.m_fLast = now;
		float t = now - w.m_fSince;
		array<RZ_TrackedZombie> units = {};
		int bodies = UnitsAt(at, units);
		if (units.IsEmpty())
			return;

		if (t >= ROUND_AFTER && !w.m_bWentRound)
		{
			w.m_bWentRound = true;
			IEntity other = OtherWayIn(b, door);
			if (other)
			{
				GoRound(units, RZ_DoorGeometry.OutsidePoint(b.m_Building, other, 2));
				m_iWentRound++;
				return;
			}
		}

		if (!Breaker(units, bodies) || !IsWeak(b, door) || IsProtected(b.m_vPos))
			return;

		if (t >= TELL_AFTER && !w.m_bWarned)
		{
			w.m_bWarned = true;
			m_GM.GetHeat().AddHeat(at, 10);
			hub.NotifyNear(at, 40, -1, "Banging", "The door is starting to give. Wood, not steel, and it knows it.");
		}

		if (t < BREAK_AFTER)
			return;

		Break(hub, b, door, at);
		m_mWatch.Remove(b.m_sKey);
	}

	//------------------------------------------------------------------------------------------------
	protected void Prune(float now)
	{
		array<string> old = {};
		foreach (string key, RZ_DoorWatch w : m_mWatch)
		{
			if (now - w.m_fLast > RESET_GAP)
				old.Insert(key);
		}

		foreach (string k : old)
		{
			m_mWatch.Remove(k);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The learned locked building with an outside door within 4 m of at (the doors near players).
	protected static RZ_BuildingDoors FindBuilding(RZ_DoorLocks locks, vector at, out IEntity door)
	{
		foreach (string key, RZ_BuildingDoors b : locks.GetBuildings())
		{
			if (!b || !b.m_Building || RZ_Util.DistXZ(b.m_vPos, at) > 60)
				continue;

			foreach (IEntity d : b.m_aDoors)
			{
				if (d && RZ_Util.DistXZ(d.GetOrigin(), at) <= 4)
				{
					door = d;
					return b;
				}
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The tracked units with a member at the door; returns the bodies there.
	protected int UnitsAt(vector at, array<RZ_TrackedZombie> units)
	{
		int bodies = 0;
		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Entity || z.m_iMissionId != -1 || RZ_Util.DistXZ(z.m_vLastPos, at) > 80 || !z.m_aMembers)
				continue;

			int here = 0;
			foreach (IEntity m : z.m_aMembers)
			{
				if (m && RZ_Util.IsCharacterAlive(m) && RZ_Util.DistXZ(m.GetOrigin(), at) <= AT_DOOR)
					here++;
			}

			if (here == 0)
				continue;

			units.Insert(z);
			bodies += here;
		}

		return bodies;
	}

	//------------------------------------------------------------------------------------------------
	//! Another outside door of the same building within 20 m, standing open.
	protected static IEntity OtherWayIn(RZ_BuildingDoors b, IEntity door)
	{
		foreach (IEntity d : b.m_aDoors)
		{
			if (!d || d == door || RZ_Util.DistXZ(d.GetOrigin(), door.GetOrigin()) > ROUND_RADIUS)
				continue;

			if (RZ_DoorGeometry.IsOpen(d))
				return d;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Off to the other door: a group the layer drives goes round as one (RZ_GroupPlan.GoRound); any
	//! other is sent there with its held waypoint.
	protected void GoRound(array<RZ_TrackedZombie> units, vector via)
	{
		ResourceName wp = m_GM.GetSpawner().GetMoveWaypointPrefab();
		foreach (RZ_TrackedZombie z : units)
		{
			if (z.m_Plan && z.m_Plan.IsDriven())
				z.m_Plan.GoRound(m_BH, via);
			else if (wp)
				RZ_GroupOrders.Order(z.m_Entity, via, wp, false); // VERIFY: a Bacon group with a target in a house follows a move waypoint to another door
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Who can break a door: a pack's leader, or a bold pack's hunt at night with 3 or more at the door.
	protected bool Breaker(array<RZ_TrackedZombie> units, int bodies)
	{
		foreach (RZ_TrackedZombie z : units)
		{
			if (z.m_eRole == RZ_EInfectedRole.ALPHA)
				return true;
		}

		if (bodies < 3 || !m_BH.IsNight())
			return false;

		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes)
			return false;

		foreach (RZ_TrackedZombie u : units)
		{
			if (!u.m_bHunting || u.m_iTribe < 0)
				continue;

			RZ_Tribe t = tribes.Find(u.m_iTribe);
			if (t && t.m_eTemper == RZ_ETemperament.BOLD)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The building's or the door's prefab name has a weak door word.
	protected bool IsWeak(RZ_BuildingDoors b, IEntity door)
	{
		if (m_aWeakWords.IsEmpty())
			return false;

		return RZ_ItemHelper.NameHas(PrefabName(b.m_Building), m_aWeakWords) || RZ_ItemHelper.NameHas(PrefabName(door), m_aWeakWords);
	}

	//------------------------------------------------------------------------------------------------
	protected static string PrefabName(IEntity e)
	{
		if (!e || !e.GetPrefabData())
			return string.Empty;

		string name = e.GetPrefabData().GetPrefabName();
		name.ToLower();
		return name;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProtected(vector pos)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		return facts && facts.IsNearLivedIn(pos, PROTECTED);
	}

	//------------------------------------------------------------------------------------------------
	//! The lock is forced: open and broken until the reset (RZ_Search), the door swung open, loud.
	protected void Break(RZ_Interactions hub, RZ_BuildingDoors b, IEntity door, vector at)
	{
		m_iBroken++;
		hub.GetSearch().OpenBuilding(b.m_sKey, true);
		DoorComponent dc = DoorComponent.Cast(door.FindComponent(DoorComponent));
		if (dc)
			dc.SetControlValue(1, RplId.Invalid()); // VERIFY: SetControlValue on the server swings the door open for every client

		m_GM.GetHeat().AddHeat(at, 20);
		hub.NotifyNear(at, 60, -1, "Banging", "Wood splits with a crack you feel in your teeth. The door is down.");
		RZ_Log.Info(string.Format("Behaviour | the infected broke a weak door at %1", RZ_SettlementScanner.GridReference(at)));
	}

	//------------------------------------------------------------------------------------------------
	//! "doors: went round 1, broken 0"
	string StatusText()
	{
		return string.Format("doors: went round %1, broken %2", m_iWentRound, m_iBroken);
	}
}
