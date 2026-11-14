//! Road patrols (docs/SITES.md, "Road patrols"): a faction's people walking the real roads out of
//! one of its holdings and back, and now and then (never more than one on the server) a vehicle
//! driving the road out of a Bastion or Red Hand holding and back.
//!
//! - **Data until someone's near.** A patrol is a point moving along its road points. Its people
//!   (and its vehicle) are put out only while a player is within the body radius, and taken away
//!   once everyone is past it and a margin. With a body the AI walks (or drives) to its next road
//!   point and its position is read back.
//! - **Few.** Two at once with anyone online, one more for every four players, at most five; a
//!   faction leaves a holding only every 20-35 minutes of play, half as often while its strength or
//!   morale is low. The Haven and Halcyon don't set out at night.
//! - **Met by standing** with the talk screen (Dialogue/RZ_TalkPatrol.c): road news for anyone, a
//!   stretch walked with them for the Known, papers (Bastion) or the road's toll (the Red Hand) for
//!   strangers. The Hunted are fired on by the hostility rules as built.
//! - **What they leave.** A fight with the infected on the way leaves spent cases on the verge (a
//!   live trace, L9); a Bastion patrol wiped out on the road becomes a Missing Patrol there. Up
//!   masts warn of a patrol on a road and overhear them (World/RZ_Masts.c).

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Road patrols.
[BaseContainerProps()]
class RZ_RoadPatrolSettings
{
	[Attribute("1", desc: "Road patrols on: a faction's people walk the real roads out of its holdings and back, and a rare vehicle drives them (needs Territory, a road network and the patrol group prefabs)")]
	bool m_bEnabled;

	[Attribute("2", UIWidgets.Slider, "Patrols at once on the server with anyone online", "0 5 1")]
	int m_iBase;

	[Attribute("4", UIWidgets.Slider, "One more patrol at once for every this many players online", "1 16 1")]
	int m_iPlayersPerExtra;

	[Attribute("5", UIWidgets.Slider, "Most patrols at once on the server, the vehicle included", "1 8 1")]
	int m_iMax;

	[Attribute("20", desc: "Shortest minutes of play between one faction's foot patrols setting out")]
	float m_fGapMinMinutes;

	[Attribute("35", desc: "Longest minutes of play between one faction's foot patrols setting out")]
	float m_fGapMaxMinutes;

	[Attribute("1000", desc: "Shortest walk out along the roads (m); a holding whose roads give less sends nobody")]
	float m_fRouteMin;

	[Attribute("3000", desc: "Longest walk out along the roads (m); the patrol then walks back the same way")]
	float m_fRouteMax;

	[Attribute("1.3", desc: "Walking speed while it's data (m/s)")]
	float m_fWalkSpeed;

	[Attribute("400", desc: "Its people are put out only while a player is this close (m), and taken away 100 m past it")]
	float m_fBodyRadius;

	[Attribute("0.25", desc: "Chance a foot patrol meets the infected on its way, at its route's most infected point, times that town's infection (0-1); the fight leaves spent cases (a live trace)")]
	float m_fFightChance;

	[Attribute("1", desc: "Vehicle patrols on: Bastion's jeep or the Red Hand's pickup drives the road out of a holding and back; never more than one on the server")]
	bool m_bVehicles;

	[Attribute("45", desc: "Shortest minutes of play between one faction's vehicle patrols")]
	float m_fVehicleGapMinMinutes;

	[Attribute("60", desc: "Longest minutes of play between one faction's vehicle patrols")]
	float m_fVehicleGapMaxMinutes;

	[Attribute("9", desc: "A vehicle patrol's speed while it's data (m/s)")]
	float m_fDriveSpeed;

	[Attribute("700", desc: "A vehicle patrol is put out only while a player is this close (m), and taken away 150 m past it")]
	float m_fVehicleBodyRadius;

	[Attribute("{C99FC1A45A249AE4}Prefabs/ReforgerZ/Patrols/RZ_Patrol_Bastion.et", UIWidgets.ResourcePickerThumbnail, "Bastion patrol: an AI group of 3-5 on MIL (a fire team). Empty = Supply > Bastion crew", "et")]
	ResourceName m_sBastionPatrol;

	[Attribute("{B1071DB2AA81415E}Prefabs/ReforgerZ/Patrols/RZ_Patrol_RedHand.et", UIWidgets.ResourcePickerThumbnail, "Red Hand patrol: an AI group of 3-5 on BAND (a crew). Empty = Supply > Red Hand crew", "et")]
	ResourceName m_sRedHandPatrol;

	[Attribute("{1647358EC6843ADB}Prefabs/ReforgerZ/Patrols/RZ_Patrol_Haven.et", UIWidgets.ResourcePickerThumbnail, "Haven patrol: an AI group of 3-5 on SURV (a scavenging party). Empty = Supply > Haven crew", "et")]
	ResourceName m_sHavenPatrol;

	[Attribute("{654AF80F1AD4FC41}Prefabs/ReforgerZ/Patrols/RZ_Patrol_Halcyon.et", UIWidgets.ResourcePickerThumbnail, "Halcyon patrol: an AI group of 3 on INST (a sample team in hazmat, one armed guard). Empty = Supply > Halcyon crew", "et")]
	ResourceName m_sHalcyonPatrol;

	[Attribute("{86D830868F026D54}Prefabs/Vehicles/Wheeled/M151A2/M151A2_MERDC.et", UIWidgets.ResourcePickerThumbnail, "Bastion patrol vehicle: a drivable jeep. Empty = Supply > Bastion truck", "et")]
	ResourceName m_sBastionVehicle;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand patrol vehicle: a drivable pickup or car. Empty = Supply > Red Hand truck", "et")]
	ResourceName m_sRedHandVehicle;
}

//------------------------------------------------------------------------------------------------
class RZ_RoadPatrol
{
	int m_iId;
	int m_eFaction;
	int m_iHolding;
	string m_sFrom;						// the holding's town ("Morton")
	string m_sLeader;					// "Corporal Pavel Novak"
	bool m_bVehicle;

	ref array<vector> m_aRoute = {};	// out along the roads and back the same way
	int m_iLeg;							// the point it's heading for
	int m_iTurn;						// the first point of the way back
	vector m_vPos;
	float m_fAge;

	IEntity m_Group;					// weak: its people while someone's near
	IEntity m_Vehicle;					// weak: its vehicle while someone's near
	bool m_bHasBody;
	bool m_bSeenAlive;
	bool m_bTagged;
	bool m_bBoarded;
	float m_fBodyAge;
	float m_fStill;						// seconds with a body and no new point reached
	int m_iOrdered = -1;				// the point its people were last sent to

	int m_iFightLeg = -1;				// it meets the infected here, as data
	bool m_bFought;

	ref array<int> m_aGreeted = {};		// players it has spoken to
	ref array<int> m_aCleared = {};		// players who showed papers or paid
	ref array<int> m_aWalked = {};		// players who walked a stretch with it
	int m_iWalker = -1;					// a player walking a stretch with it now
	int m_iWalkTo = -1;					// to this point

	//------------------------------------------------------------------------------------------------
	string Title()
	{
		return m_sLeader + ", " + RZ_RoadPatrols.KindName(m_eFaction, m_bVehicle);
	}

	//------------------------------------------------------------------------------------------------
	bool OnWayBack()
	{
		return m_iLeg > m_iTurn;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_RoadPatrols
{
	static const int TAG = -31;					// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: a patrol's leader

	static const int REPLY_NEWS = 1;
	static const int REPLY_WALK = 2;
	static const int REPLY_PAPERS = 3;
	static const int REPLY_TOLL = 4;

	protected static const float START_REACH = 300;		// the road point it starts from is this close to its holding
	protected static const float JOIN = 20;				// road ends this close are joined
	protected static const int ROUTE_TRIES = 6;			// other ways out tried when the first runs out short
	protected static const float FOOT_SPACING = 25;		// route points a walk this far apart
	protected static const float DRIVE_SPACING = 70;	// and a drive
	protected static const float RIVAL_CLEAR = 250;		// never this close to another faction's holding
	protected static const float GREET_RANGE = 30;		// it speaks to a player this close
	protected static const float REACH_FOOT = 12;		// a body reached its point
	protected static const float REACH_DRIVE = 35;
	protected static const float STILL_SKIP = 90;		// seconds with no point reached: the next one
	protected static const float WALK_KEEP = 40;		// a player walking with it stays this close
	protected static const float WALK_COOL = 3600;		// once an hour a player is thanked for a stretch
	protected static const int WALK_POINTS = 8;			// a stretch walked with it

	protected static RZ_RoadPatrols s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected RZ_RoadPatrolSettings m_Settings;
	protected ref array<ref RZ_RoadPatrol> m_aPatrols = {};
	protected ref array<float> m_aNextFoot = {};
	protected ref array<float> m_aNextVehicle = {};
	protected ref map<string, float> m_mWalkThanked = new map<string, float>();
	protected ref array<int> m_aNoRoad = {};			// holdings with no road to walk (logged once)
	protected float m_fClock;
	protected string m_sWhyShort;						// why the last route came out short (for the log)
	protected int m_iNextId = 1;
	protected bool m_bReady;
	protected bool m_bLoggedOff;

	//------------------------------------------------------------------------------------------------
	void RZ_RoadPatrols(RZ_GameModeComponent gm, RZ_RoadPatrolSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_RoadPatrols Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_bReady && m_Settings && m_Settings.m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		m_aNextFoot.Clear();
		m_aNextVehicle.Clear();
		for (int f = 0; f < 4; f++)
		{
			// The first ones go out a few minutes into the session, spread over the factions
			m_aNextFoot.Insert(Math.RandomFloat(120, 600) + f * 60);
			m_aNextVehicle.Insert(Gap(m_Settings.m_fVehicleGapMinMinutes, m_Settings.m_fVehicleGapMaxMinutes) * Math.RandomFloat(0.5, 1));
		}

		m_bReady = m_GM.GetTerritory() != null && RZ_Perimeter.GetRoadNetwork() != null;
		if (!m_Settings.m_bEnabled)
			RZ_Log.Info("Road patrols: off (Road patrols > on).");
		else if (!m_bReady)
			RZ_Log.Info("Road patrols: off (no territory or no road network on this map).");
		else
			RZ_Log.Info(string.Format("Road patrols: on, %1 at once plus one every %2 players (at most %3); vehicles %4.",
				m_Settings.m_iBase, m_Settings.m_iPlayersPerExtra, m_Settings.m_iMax, m_Settings.m_bVehicles));
	}

	//------------------------------------------------------------------------------------------------
	//! "Bastion patrol", "Red Hand crew on the road", "the Haven's scavengers", "Halcyon sample team".
	static string KindName(int f, bool vehicle)
	{
		if (vehicle)
		{
			if (f == RZ_EFaction.BANDITS)
				return "Red Hand pickup";

			return "Bastion jeep";
		}

		switch (f)
		{
			case RZ_EFaction.MILITARY: return "Bastion patrol";
			case RZ_EFaction.BANDITS: return "Red Hand crew on the road";
			case RZ_EFaction.SURVIVORS: return "Haven scavenging party";
			case RZ_EFaction.INSTITUTE: return "Halcyon sample team";
		}

		return "patrol";
	}

	//------------------------------------------------------------------------------------------------
	protected float Gap(float minMinutes, float maxMinutes)
	{
		return Math.RandomFloat(Math.Max(1, minMinutes), Math.Max(minMinutes, maxMinutes)) * 60;
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aPatrols.Count();
	}

	//------------------------------------------------------------------------------------------------
	RZ_RoadPatrol GetPatrol(int id)
	{
		foreach (RZ_RoadPatrol p : m_aPatrols)
		{
			if (p.m_iId == id)
				return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void GetAll(notnull array<RZ_RoadPatrol> result)
	{
		foreach (RZ_RoadPatrol p : m_aPatrols)
		{
			result.Insert(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int VehicleCount()
	{
		int n = 0;
		foreach (RZ_RoadPatrol p : m_aPatrols)
		{
			if (p.m_bVehicle)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected int CapNow(int online)
	{
		if (online <= 0)
			return 0;

		int extra = online / Math.Max(1, m_Settings.m_iPlayersPerExtra);
		return Math.Min(m_Settings.m_iMax, m_Settings.m_iBase + extra);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		for (int i = m_aPatrols.Count() - 1; i >= 0; i--)
		{
			if (Update(m_aPatrols[i], dt, ids, players))
				m_aPatrols.Remove(i);
		}

		// Minutes of play: the departures wait while nobody's online
		if (ids.IsEmpty())
			return;

		m_fClock += dt;
		int cap = CapNow(ids.Count());
		for (int f = 0; f < 4; f++)
		{
			if (m_aPatrols.Count() >= cap)
				break;

			if (m_fClock >= m_aNextFoot[f])
			{
				m_aNextFoot[f] = m_fClock + Gap(m_Settings.m_fGapMinMinutes, m_Settings.m_fGapMaxMinutes) * Slowdown(f);
				SetOut(f, false);
			}
		}

		if (!m_Settings.m_bVehicles || VehicleCount() > 0 || m_aPatrols.Count() >= cap)
			return;

		array<int> drivers = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS};
		foreach (int df : drivers)
		{
			if (m_fClock < m_aNextVehicle[df])
				continue;

			m_aNextVehicle[df] = m_fClock + Gap(m_Settings.m_fVehicleGapMinMinutes, m_Settings.m_fVehicleGapMaxMinutes) * Slowdown(df);
			if (SetOut(df, true))
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Twice the gap while the faction is weak this season (strength or morale under 35).
	protected float Slowdown(int f)
	{
		float strength = 50;
		float morale = 60;
		if (m_GM.GetTerritory())
			strength = m_GM.GetTerritory().GetStrength(f);

		if (m_GM.GetFactionLife())
			morale = m_GM.GetFactionLife().GetMorale(f);

		if (strength < 35 || morale < 35)
			return 2;

		return 1;
	}

	// ================================================================ Setting out
	//------------------------------------------------------------------------------------------------
	//! A patrol of faction f sets out from one of its holdings. True if one did.
	bool SetOut(int f, bool vehicle)
	{
		if (!vehicle && (f == RZ_EFaction.SURVIVORS || f == RZ_EFaction.INSTITUTE) && RZ_GameTime.IsNight())
			return false;

		if (GroupPrefab(f).IsEmpty())
			return false;

		if (vehicle && VehiclePrefab(f).IsEmpty())
			return false;

		RZ_Territory territory = m_GM.GetTerritory();
		array<RZ_Holding> mine = {};
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h && h.m_eFaction == f && !m_aNoRoad.Contains(h.m_iId) && !Busy(h.m_iId))
				mine.Insert(h);
		}

		if (mine.IsEmpty())
			return false;

		RZ_Holding from = mine.GetRandomElement();
		return SetOutFrom(f, from, vehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Busy(int holding)
	{
		foreach (RZ_RoadPatrol p : m_aPatrols)
		{
			if (p.m_iHolding == holding)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A patrol from `from`. A vehicle heads for another of the faction's holdings where there is one.
	RZ_RoadPatrol SetOutFrom(int f, RZ_Holding from, bool vehicle)
	{
		if (!from)
			return null;

		vector target = vector.Zero;
		bool hasTarget = false;
		float wanted = Math.RandomFloat(m_Settings.m_fRouteMin, Math.Max(m_Settings.m_fRouteMin, m_Settings.m_fRouteMax));
		float spacing = FOOT_SPACING;
		if (vehicle)
		{
			spacing = DRIVE_SPACING;
			wanted = Math.Max(wanted, 2000);
			RZ_Holding other = OtherHolding(f, from);
			if (other)
			{
				target = other.m_vPos;
				hasTarget = true;
				wanted = Math.Min(6000, RZ_Util.DistXZ(from.m_vPos, other.m_vPos) * 1.6);
			}
		}

		array<vector> outward = {};
		bool built = BuildRoute(from.m_vPos, wanted, target, hasTarget, f, spacing, outward, false);

		// Roads end, or run into water or someone else's ground: other ways out are tried (a vehicle
		// whose way toward the other holding runs out early just drives the roads instead)
		for (int attempt = 0; attempt < ROUTE_TRIES && !built; attempt++)
		{
			built = BuildRoute(from.m_vPos, wanted, vector.Zero, false, f, spacing, outward, true);
		}

		if (!built)
		{
			if (!m_aNoRoad.Contains(from.m_iId) && !vehicle)
			{
				m_aNoRoad.Insert(from.m_iId);
				RZ_Log.Info(string.Format("Road patrols: %1 at %2 sends nobody: no road to walk %3 m on within %4 m of it (%5).",
					RZ_Factions.GetShortName(f), from.m_sName, RZ_Util.RoundToInt(m_Settings.m_fRouteMin), RZ_Util.RoundToInt(START_REACH), m_sWhyShort));
			}

			return null;
		}

		RZ_RoadPatrol p = new RZ_RoadPatrol();
		p.m_iId = m_iNextId;
		m_iNextId++;
		p.m_eFaction = f;
		p.m_iHolding = from.m_iId;
		p.m_sFrom = from.m_sName;
		p.m_bVehicle = vehicle;
		foreach (vector v : outward)
		{
			p.m_aRoute.Insert(v);
		}

		p.m_iTurn = outward.Count() - 1;
		for (int k = outward.Count() - 2; k >= 0; k--)
		{
			p.m_aRoute.Insert(outward[k]);
		}

		p.m_vPos = p.m_aRoute[0];
		p.m_iLeg = 1;
		p.m_sLeader = LeaderName(f, p.m_iId + from.m_iId * 31);
		if (!vehicle)
			PlanFight(p);

		m_aPatrols.Insert(p);
		RZ_Log.Info(string.Format("Road patrols: %1 sets out from %2 (%3 points, %4 m out%5).",
			p.Title(), from.m_sName, p.m_aRoute.Count(), RZ_Util.RoundToInt(RouteLength(outward)), FightNote(p)));
		return p;
	}

	//------------------------------------------------------------------------------------------------
	protected string FightNote(RZ_RoadPatrol p)
	{
		if (p.m_iFightLeg < 0)
			return "";

		return ", meets the infected at " + RZ_SettlementScanner.GridReference(p.m_aRoute[p.m_iFightLeg]);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Holding OtherHolding(int f, RZ_Holding from)
	{
		RZ_Holding best;
		float bestDist = 6000;
		foreach (RZ_Holding h : m_GM.GetTerritory().GetHoldings())
		{
			if (!h || h == from || h.m_eFaction != f)
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, from.m_vPos);
			if (d < 800 || d > bestDist)
				continue;

			bestDist = d;
			best = h;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected string LeaderName(int f, int seed)
	{
		RZ_Rng rng = new RZ_Rng(Math.AbsInt(seed) + 17);
		string name = RZ_NoteText.ManName(rng) + " " + RZ_NoteText.Surname(rng);
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "Corporal " + name;
			case RZ_EFaction.INSTITUTE: return "Technician " + name;
		}

		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! The point on its way out where it meets the infected (as data), by the infection of the town
	//! nearest each point: the most infected point, at chance x infection.
	protected void PlanFight(RZ_RoadPatrol p)
	{
		int best = -1;
		float bestInfection = 0;
		for (int i = 2; i <= p.m_iTurn; i += 4)
		{
			int town = m_GM.FindNearestSettlement(p.m_aRoute[i]);
			RZ_Settlement s = m_GM.GetSettlement(town);
			if (!s || RZ_Util.DistXZ(s.m_vCenter, p.m_aRoute[i]) > s.m_fRadius + 500)
				continue;

			if (s.m_fInfection > bestInfection)
			{
				bestInfection = s.m_fInfection;
				best = i;
			}
		}

		if (best >= 0 && Math.RandomFloat01() < m_Settings.m_fFightChance * bestInfection / 100)
			p.m_iFightLeg = best;
	}

	// ================================================================ Routes on the roads
	//------------------------------------------------------------------------------------------------
	//! Out along the real roads from the point nearest `from` (within START_REACH): along one road to
	//! its end, then onto a road that starts there or passes it (a T-junction) and hasn't been walked yet (toward `target` when
	//! there is one, else at random), until it's `wanted` metres long. The first road's way is the longer
	//! side, or either at random with `anyWay`. Never into water, near the line,
	//! off the mainland or within RIVAL_CLEAR of another faction's holding. False if it's shorter than
	//! the shortest walk.
	bool BuildRoute(vector from, float wanted, vector target, bool hasTarget, int f, float spacing, notnull array<vector> result, bool anyWay)
	{
		result.Clear();
		RoadNetworkManager roads = RZ_Perimeter.GetRoadNetwork();
		if (!roads)
			return false;

		float r = wanted + 600;
		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(Vector(from[0] - r, -1000, from[2] - r), Vector(from[0] + r, 10000, from[2] + r), found); // VERIFY: road points are world positions along each road's spline, its two ends included
		array<ref array<vector>> lines = {};
		foreach (BaseRoad road : found)
		{
			if (!road)
				continue;

			array<vector> pts = {};
			road.GetPoints(pts);
			if (pts.Count() >= 2)
				lines.Insert(pts);
		}

		int line = -1;
		int idx = -1;
		float nearest = START_REACH;
		foreach (int li, array<vector> candidate : lines)
		{
			foreach (int pi, vector pt : candidate)
			{
				float d = RZ_Util.DistXZ(pt, from);
				if (d < nearest)
				{
					nearest = d;
					line = li;
					idx = pi;
				}
			}
		}

		if (line < 0)
		{
			m_sWhyShort = string.Format("%1 roads within %2 m, none within %3 m of it", lines.Count(), RZ_Util.RoundToInt(r), RZ_Util.RoundToInt(START_REACH));
			return false;
		}

		// Which way along the first road: toward the target, else the longer side
		array<vector> first = lines[line];
		int dir = 1;
		if (hasTarget)
		{
			if (RZ_Util.DistXZ(first[0], target) < RZ_Util.DistXZ(first[first.Count() - 1], target))
				dir = -1;
		}
		else if (anyWay)
		{
			if (Math.RandomFloat01() < 0.5)
				dir = -1;
		}
		else if (idx > first.Count() / 2)
		{
			dir = -1;
		}

		array<int> used = {line};
		result.Insert(RZ_Util.GroundPos(first[idx]));
		float length = 0;
		int guard = 0;
		string stop = "the walk was long enough";
		while (length < wanted && guard < 5000)
		{
			guard++;
			array<vector> pts2 = lines[line];
			int next = idx + dir;
			if (next < 0 || next >= pts2.Count())
			{
				vector end = pts2[idx];
				int pickLine = -1;
				int pickIdx = 0;
				int pickDir = 1;
				float pickScore = 1000000;
				foreach (int oi, array<vector> other : lines)
				{
					if (oi == line)
						continue;

					int last = other.Count() - 1;

					// Where it meets this road's end: one of its ends, or a point along it (a T-junction)
					int at = -1;
					float meet = JOIN;
					foreach (int ki, vector kp : other)
					{
						float km = RZ_Util.DistXZ(kp, end);
						if (km <= meet)
						{
							meet = km;
							at = ki;
						}
					}

					if (at < 0)
						continue;

					for (int side = 0; side < 2; side++)
					{
						int go = 1;
						int far = last;
						if (side == 1)
						{
							go = -1;
							far = 0;
						}

						// Nowhere to go that way from the meeting point
						if (at + go < 0 || at + go > last)
							continue;

						if (used.Contains(oi))
							continue;

						float score = Math.RandomFloat(0, 100);
						if (hasTarget)
							score = RZ_Util.DistXZ(other[far], target);

						if (score < pickScore)
						{
							pickScore = score;
							pickLine = oi;
							pickIdx = at;
							pickDir = go;
						}
					}
				}

				if (pickLine < 0)
				{
					stop = "a road ended with no road on from it";
					break;
				}

				line = pickLine;
				idx = pickIdx;
				dir = pickDir;
				if (!used.Contains(line))
					used.Insert(line);

				continue;
			}

			vector p = RZ_Util.GroundPos(pts2[next]);
			if (!Allowed(p, f))
			{
				stop = "the road ran into " + Barrier(p, f);
				break;
			}

			length += RZ_Util.DistXZ(pts2[idx], pts2[next]);
			idx = next;
			if (RZ_Util.DistXZ(result[result.Count() - 1], p) >= spacing)
				result.Insert(p);

			if (hasTarget && RZ_Util.DistXZ(p, target) <= 300)
				break;
		}

		if (length < m_Settings.m_fRouteMin || result.Count() < 3)
		{
			m_sWhyShort = string.Format("%1 m on %2 roads: %3", RZ_Util.RoundToInt(length), used.Count(), stop);
			result.Clear();
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! What stopped a road at p: "water", "the line", "the coast" (off the mainland) or "<faction>'s ground".
	protected string Barrier(vector p, int f)
	{
		if (RZ_Util.IsWater(p))
			return "water";

		if (RZ_Perimeter.IsNearLineAt(p))
			return "the line";

		if (!RZ_Mainland.OnMainland(p))
			return "the coast";

		foreach (RZ_Holding h : m_GM.GetTerritory().GetHoldings())
		{
			if (h && h.m_eFaction != f && RZ_Util.DistXZ(h.m_vPos, p) < RIVAL_CLEAR)
				return RZ_Factions.GetShortName(h.m_eFaction) + "'s ground at " + h.m_sName;
		}

		return "nothing";
	}

	//------------------------------------------------------------------------------------------------
	protected bool Allowed(vector p, int f)
	{
		if (RZ_Util.IsWater(p) || RZ_Perimeter.IsNearLineAt(p) || !RZ_Mainland.OnMainland(p))
			return false;

		foreach (RZ_Holding h : m_GM.GetTerritory().GetHoldings())
		{
			if (h && h.m_eFaction != f && RZ_Util.DistXZ(h.m_vPos, p) < RIVAL_CLEAR)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected float RouteLength(array<vector> pts)
	{
		float total = 0;
		for (int i = 1; i < pts.Count(); i++)
		{
			total += RZ_Util.DistXZ(pts[i - 1], pts[i]);
		}

		return total;
	}

	// ================================================================ On the road
	//------------------------------------------------------------------------------------------------
	//! True when it's done with (home, lost or given up).
	protected bool Update(RZ_RoadPatrol p, float dt, array<int> ids, array<IEntity> players)
	{
		p.m_fAge += dt;
		float radius = BodyRadius(p);
		float nearest = RZ_Util.NearestDistance(p.m_vPos, players);

		if (p.m_bHasBody)
		{
			if (UpdateBody(p, dt, ids, players))
				return true;

			if (nearest > radius + Margin(p) && !PlayerHasVehicle(p))
				TakeAway(p);
		}
		else
		{
			MoveAsData(p, dt);
			if (p.m_iLeg >= p.m_aRoute.Count())
			{
				RZ_Log.Info(string.Format("Road patrols: %1 is back at %2.", p.Title(), p.m_sFrom));
				return true;
			}

			if (nearest <= radius)
				PutOut(p);
		}

		// Give up a patrol that's been out far too long (stuck, or a road that goes nowhere)
		if (p.m_fAge > 4 * 3600)
		{
			RZ_Log.Info(string.Format("Road patrols: %1 is called in after four hours out.", p.Title()));
			TakeAway(p);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected float BodyRadius(RZ_RoadPatrol p)
	{
		if (p.m_bVehicle)
			return m_Settings.m_fVehicleBodyRadius;

		return m_Settings.m_fBodyRadius;
	}

	//------------------------------------------------------------------------------------------------
	protected float Margin(RZ_RoadPatrol p)
	{
		if (p.m_bVehicle)
			return 150;

		return 100;
	}

	//------------------------------------------------------------------------------------------------
	protected void MoveAsData(RZ_RoadPatrol p, float dt)
	{
		float speed = m_Settings.m_fWalkSpeed;
		if (p.m_bVehicle)
			speed = m_Settings.m_fDriveSpeed;

		float budget = speed * dt;
		while (budget > 0 && p.m_iLeg < p.m_aRoute.Count())
		{
			vector to = p.m_aRoute[p.m_iLeg];
			float d = RZ_Util.DistXZ(p.m_vPos, to);
			if (d <= budget)
			{
				p.m_vPos = to;
				budget -= d;
				ReachedPoint(p);
				continue;
			}

			vector step = p.m_vPos;
			step[0] = step[0] + (to[0] - step[0]) * budget / d;
			step[2] = step[2] + (to[2] - step[2]) * budget / d;
			p.m_vPos = step;
			budget = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A point reached, as data or by its people.
	protected void ReachedPoint(RZ_RoadPatrol p)
	{
		if (p.m_iLeg == p.m_iFightLeg && !p.m_bFought && !p.m_bHasBody)
			Fight(p, p.m_aRoute[p.m_iLeg]);

		if (p.m_iWalker > 0 && p.m_iLeg >= p.m_iWalkTo)
			WalkDone(p);

		p.m_iLeg++;
		p.m_fStill = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! It met the infected here: spent cases on the verge (a live trace).
	protected void Fight(RZ_RoadPatrol p, vector at)
	{
		p.m_bFought = true;
		RZ_Traces traces = m_GM.GetTraces();
		if (traces && traces.GetScenes())
			traces.GetScenes().PatrolFight(p.m_eFaction, at);

		RZ_Log.Info(string.Format("Road patrols: %1 met the infected at %2 and went on.", p.Title(), RZ_SettlementScanner.GridReference(at)));
	}

	// ================================================================ Its people
	//------------------------------------------------------------------------------------------------
	protected ResourceName GroupPrefab(int f)
	{
		ResourceName prefab;
		switch (f)
		{
			case RZ_EFaction.MILITARY: prefab = m_Settings.m_sBastionPatrol; break;
			case RZ_EFaction.BANDITS: prefab = m_Settings.m_sRedHandPatrol; break;
			case RZ_EFaction.SURVIVORS: prefab = m_Settings.m_sHavenPatrol; break;
			case RZ_EFaction.INSTITUTE: prefab = m_Settings.m_sHalcyonPatrol; break;
		}

		if (!prefab.IsEmpty())
			return prefab;

		RZ_SupplySettings supply = m_GM.GetSupplySettings();
		if (!supply)
			return prefab;

		switch (f)
		{
			case RZ_EFaction.MILITARY: return supply.m_sBastionCrew;
			case RZ_EFaction.BANDITS: return supply.m_sRedHandCrew;
			case RZ_EFaction.SURVIVORS: return supply.m_sHavenCrew;
			case RZ_EFaction.INSTITUTE: return supply.m_sHalcyonCrew;
		}

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName VehiclePrefab(int f)
	{
		RZ_SupplySettings supply = m_GM.GetSupplySettings();
		if (f == RZ_EFaction.MILITARY)
		{
			if (!m_Settings.m_sBastionVehicle.IsEmpty())
				return m_Settings.m_sBastionVehicle;

			if (supply)
				return supply.m_sBastionTruck;
		}

		if (f == RZ_EFaction.BANDITS)
		{
			if (!m_Settings.m_sRedHandVehicle.IsEmpty())
				return m_Settings.m_sRedHandVehicle;

			if (supply)
				return supply.m_sRedHandTruck;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected void PutOut(RZ_RoadPatrol p)
	{
		vector at = RZ_Util.GroundPos(p.m_vPos);
		if (RZ_Util.IsWater(at))
			at = RZ_Mission.LandPointNear(at, 3, 30);

		RZ_Shared shared = m_GM.GetShared();
		if (shared && !shared.CanSpawnBody(RZ_BodyBudget.WANDERER, at))
			return;

		float yaw = 0;
		if (p.m_iLeg < p.m_aRoute.Count())
			yaw = RZ_Compass.Bearing(at, p.m_aRoute[p.m_iLeg]);

		if (p.m_bVehicle)
		{
			p.m_Vehicle = RZ_Util.SpawnPrefab(VehiclePrefab(p.m_eFaction), at + Vector(0, 0.5, 0), yaw);
			if (!p.m_Vehicle)
				return;
		}

		ResourceName prefab = m_GM.PickLook(GroupPrefab(p.m_eFaction), RZ_Looks.SeedAt(at));
		vector feet = at;
		if (p.m_bVehicle)
			feet = RZ_Mission.LandPointNear(at, 3, 6);

		p.m_Group = RZ_Util.SpawnPrefab(prefab, feet, yaw);
		if (!p.m_Group)
		{
			if (p.m_Vehicle)
				RZ_Util.DeleteUnit(p.m_Vehicle);

			p.m_Vehicle = null;
			return;
		}

		if (shared)
			shared.AddBody(RZ_BodyBudget.WANDERER, p.m_Group);

		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.WatchUnit(p.m_Group, p.m_eFaction);

		p.m_bHasBody = true;
		p.m_bSeenAlive = false;
		p.m_bTagged = false;
		p.m_bBoarded = !p.m_bVehicle;
		p.m_fBodyAge = 0;
		p.m_fStill = 0;
		p.m_iOrdered = -1;
		p.m_vPos = at;
		RZ_Log.Info(string.Format("Road patrols: %1 put out at %2 (a player within %3 m).", p.Title(), RZ_SettlementScanner.GridReference(at), RZ_Util.RoundToInt(BodyRadius(p))));
	}

	//------------------------------------------------------------------------------------------------
	//! Its people (and vehicle) taken away; it goes on as data from where they were.
	protected void TakeAway(RZ_RoadPatrol p)
	{
		if (p.m_Group)
			RZ_Util.DeleteUnit(p.m_Group);

		if (p.m_Vehicle && !PlayerHasVehicle(p))
			RZ_Util.DeleteUnit(p.m_Vehicle);

		p.m_Group = null;
		p.m_Vehicle = null;
		p.m_bHasBody = false;
		p.m_bTagged = false;
	}

	//------------------------------------------------------------------------------------------------
	//! A player sits at the wheel of its vehicle (it's theirs now: the patrol ends, the vehicle stays).
	protected bool PlayerHasVehicle(RZ_RoadPatrol p)
	{
		if (!p.m_Vehicle)
			return false;

		IEntity driver = RZ_Util.GetDriver(p.m_Vehicle);
		if (!driver)
			return false;

		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(driver) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! True when it's done with (lost, or its vehicle taken).
	protected bool UpdateBody(RZ_RoadPatrol p, float dt, array<int> ids, array<IEntity> players)
	{
		p.m_fBodyAge += dt;
		if (!p.m_Group)
		{
			// Removed by someone else (cleanup, an admin): not a loss
			p.m_bHasBody = false;
			return false;
		}

		int alive = RZ_Util.CountAlive(p.m_Group);
		if (alive > 0)
			p.m_bSeenAlive = true;

		if (p.m_bSeenAlive && alive == 0)
		{
			Lost(p, RZ_Util.GetUnitPosition(p.m_Group));
			return true;
		}

		if (p.m_bVehicle && PlayerHasVehicle(p))
		{
			RZ_Log.Info(string.Format("Road patrols: %1's vehicle was taken by a player at %2; the patrol ends.", p.Title(), RZ_SettlementScanner.GridReference(p.m_Vehicle.GetOrigin())));
			p.m_Vehicle = null;
			TakeAway(p);
			return true;
		}

		if (!p.m_bTagged)
			TryTag(p);

		if (p.m_bVehicle && !p.m_bBoarded)
		{
			Board(p);
			return false;
		}

		vector now = RZ_Util.GetUnitPosition(p.m_Group);
		if (p.m_bVehicle && p.m_Vehicle)
			now = p.m_Vehicle.GetOrigin();

		if (now != vector.Zero)
			p.m_vPos = now;

		float reach = REACH_FOOT;
		if (p.m_bVehicle)
			reach = REACH_DRIVE;

		p.m_fStill += dt;
		while (p.m_iLeg < p.m_aRoute.Count() && RZ_Util.DistXZ(p.m_vPos, p.m_aRoute[p.m_iLeg]) <= reach)
		{
			ReachedPoint(p);
		}

		if (p.m_fStill >= STILL_SKIP && p.m_iLeg < p.m_aRoute.Count())
		{
			p.m_iLeg++;
			p.m_fStill = 0;
		}

		if (p.m_iLeg >= p.m_aRoute.Count())
		{
			RZ_Log.Info(string.Format("Road patrols: %1 is back at %2.", p.Title(), p.m_sFrom));
			TakeAway(p);
			return true;
		}

		Order(p);
		Meet(p, ids, players);
		WatchWalker(p);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Its people sent to the next point (a vehicle a point further, so it keeps rolling).
	protected void Order(RZ_RoadPatrol p)
	{
		int target = p.m_iLeg;
		if (p.m_bVehicle)
			target = Math.Min(p.m_iLeg + 1, p.m_aRoute.Count() - 1);

		if (target == p.m_iOrdered)
			return;

		p.m_iOrdered = target;
		RZ_Util.RedirectGroup(p.m_Group, p.m_aRoute[target], m_GM.GetSpawner().GetMoveWaypointPrefab()); // VERIFY: a group seated in a vehicle drives to its move waypoint along the roads, and on foot a group walks to it
	}

	//------------------------------------------------------------------------------------------------
	//! Its crew into the vehicle, the driver first (as the supply trucks do); after 30 s whoever isn't
	//! in goes on foot and the vehicle is left by the road.
	protected void Board(RZ_RoadPatrol p)
	{
		bool waited = p.m_fBodyAge >= 30;
		if (!p.m_Vehicle)
		{
			p.m_bBoarded = true;
			return;
		}

		if (!waited && !RZ_Util.HasAllMembers(p.m_Group))
			return;

		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(p.m_Group, members);
		vector at = p.m_Vehicle.GetOrigin();
		if (!RZ_Util.GetDriver(p.m_Vehicle))
		{
			foreach (IEntity candidate : members)
			{
				if (RZ_Util.DistXZ(candidate.GetOrigin(), at) <= 12 && RZ_Util.BoardVehicle(candidate, p.m_Vehicle, true))
					break;
			}
		}

		int aboard = 0;
		foreach (IEntity member : members)
		{
			if (RZ_Util.GetVehicleOf(member) == p.m_Vehicle)
			{
				aboard++;
				continue;
			}

			if (RZ_Util.DistXZ(member.GetOrigin(), at) <= 12)
				RZ_Util.BoardVehicle(member, p.m_Vehicle, false);
		}

		IEntity driver = RZ_Util.GetDriver(p.m_Vehicle);
		if (driver && members.Contains(driver) && (aboard >= members.Count() || waited))
		{
			p.m_bBoarded = true;
			RZ_Log.Info(string.Format("Road patrols: %1 rolls with %2 of %3 aboard.", p.Title(), aboard, members.Count()));
			return;
		}

		if (waited)
		{
			p.m_bBoarded = true;
			RZ_Log.Warn(string.Format("Road patrols: %1 has nobody at the wheel after 30 s; its people go on foot.", p.Title()));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Its leader can be talked to (the talk screen: Dialogue/RZ_TalkPatrol.c). The leader exists a
	//! frame after the group, so this is tried again on the next ticks.
	protected void TryTag(RZ_RoadPatrol p)
	{
		IEntity leader = Leader(p);
		if (!leader)
			return;

		RZ_PuzzleObjectComponent.Tag(leader, TAG, p.m_iId, "", "");
		RZ_ClueComponent.Tag(leader, TAG, p.m_iId, "Talk to " + p.m_sLeader);
		p.m_bTagged = true;
	}

	//------------------------------------------------------------------------------------------------
	IEntity Leader(RZ_RoadPatrol p)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(p.m_Group);
		if (!group)
			return p.m_Group;

		AIAgent agent = group.GetLeaderAgent();
		if (!agent)
			return null;

		return agent.GetControlledEntity();
	}

	//------------------------------------------------------------------------------------------------
	//! Lost on the road: everyone in it dead. A Bastion patrol becomes a Missing Patrol there (true if
	//! one started).
	protected bool Lost(RZ_RoadPatrol p, vector at)
	{
		if (at == vector.Zero)
			at = p.m_vPos;

		RZ_Log.Info(string.Format("Road patrols: %1 was lost at %2.", p.Title(), RZ_SettlementScanner.GridReference(at)));
		Fight(p, at);
		if (p.m_bVehicle && p.m_Vehicle && !PlayerHasVehicle(p))
			p.m_Vehicle = null;

		if (p.m_eFaction != RZ_EFaction.MILITARY)
			return false;

		RZ_MissionManager missions = m_GM.GetMissions();
		if (!missions)
			return false;

		RZ_Mission_MissingPatrol.ForceSpot(at);
		RZ_Mission_MissingPatrol job = new RZ_Mission_MissingPatrol();
		job.Configure(m_GM);
		bool started = missions.StartMission(job);
		RZ_Mission_MissingPatrol.ForceSpot(vector.Zero);
		RZ_Log.Info(string.Format("Road patrols: Missing Patrol at %1: %2.", RZ_SettlementScanner.GridReference(at), started));
		return started;
	}

	// ================================================================ Meeting one
	//------------------------------------------------------------------------------------------------
	//! Its leader speaks once to each player who comes close, by their standing (rule 29: the person
	//! says it). The Hunted hear nothing: they're fired on.
	protected void Meet(RZ_RoadPatrol p, array<int> ids, array<IEntity> players)
	{
		IEntity leader = Leader(p);
		if (!leader)
			return;

		vector at = leader.GetOrigin();
		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			if (p.m_aGreeted.Contains(playerId) || RZ_Util.DistXZ(ent.GetOrigin(), at) > GREET_RANGE)
				continue;

			p.m_aGreeted.Insert(playerId);
			RZ_ETier tier = m_GM.GetTier(playerId, p.m_eFaction);
			string stance = Stance(p, playerId);
			RZ_Log.Info(string.Format("Road patrols: %1 meets %2: %3.", p.Title(), RZ_Names.Public(playerId), stance));
			if (RZ_Reputation.IsHunted(tier))
				continue;

			m_GM.Say(playerId, p.Title(), p.m_eFaction, Greeting(p, playerId), leader);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "friendly", "stranger" or "hostile (they fire)": how it meets this player.
	string Stance(RZ_RoadPatrol p, int playerId)
	{
		RZ_ETier tier = m_GM.GetTier(playerId, p.m_eFaction);
		if (RZ_Reputation.IsHunted(tier))
			return "hostile (they fire)";

		if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN) && tier != RZ_ETier.WILDCARD)
			return "friendly";

		return "stranger";
	}

	//------------------------------------------------------------------------------------------------
	string Greeting(RZ_RoadPatrol p, int playerId)
	{
		bool friendly = Stance(p, playerId) == "friendly";
		switch (p.m_eFaction)
		{
			case RZ_EFaction.MILITARY:
			{
				if (friendly)
					return "Afternoon. Road's quiet this far. Keep it that way.";

				return "Patrol. Hands where I can see them. Papers, if you've got any.";
			}
			case RZ_EFaction.BANDITS:
			{
				if (friendly)
					return "One of ours. Walk on, the road's paid for.";

				return "This road's ours today. There's a toll on walking it.";
			}
			case RZ_EFaction.SURVIVORS:
			{
				if (friendly)
					return "Good to see a face. We're working the houses up the road, if you're going that way.";

				return "We're out for what's left in the houses. Mind yourself, and mind us.";
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (friendly)
					return "Hello. Sample run. Nothing to see, unless you like swabs.";

				return "Sample team. Three metres back, please. That's for your sake.";
			}
		}

		return "Keep walking.";
	}

	//------------------------------------------------------------------------------------------------
	//! The talk screen's replies for this player: ids, labels, tags, and why each is locked ("" if not).
	void Replies(int id, int playerId, notnull array<int> idsOut, notnull array<string> labels, notnull array<string> tags, notnull array<string> needs)
	{
		RZ_RoadPatrol p = GetPatrol(id);
		if (!p)
			return;

		string stance = Stance(p, playerId);
		idsOut.Insert(REPLY_NEWS);
		labels.Insert("Anything on the road?");
		tags.Insert("");
		needs.Insert("");

		if (stance == "friendly")
		{
			if (!p.m_aWalked.Contains(playerId) && p.m_iWalker < 0 && p.m_iLeg + 2 < p.m_aRoute.Count())
			{
				idsOut.Insert(REPLY_WALK);
				labels.Insert("I'll walk the next stretch with you.");
				tags.Insert("");
				needs.Insert("");
			}

			return;
		}

		if (stance != "stranger" || p.m_aCleared.Contains(playerId))
			return;

		if (p.m_eFaction == RZ_EFaction.MILITARY)
		{
			idsOut.Insert(REPLY_PAPERS);
			labels.Insert("Show them your papers.");
			tags.Insert("Papers");
			needs.Insert("");
		}
		else if (p.m_eFaction == RZ_EFaction.BANDITS)
		{
			int toll = Toll(playerId);
			idsOut.Insert(REPLY_TOLL);
			labels.Insert(string.Format("Pay for the road (%1 Favor).", toll));
			tags.Insert("Trade");
			if (m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS) < toll)
				needs.Insert(string.Format("%1 Favor with the Red Hand", toll));
			else
				needs.Insert("");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int Toll(int playerId)
	{
		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (posts)
			return posts.TollFor(playerId);

		return 5;
	}

	//------------------------------------------------------------------------------------------------
	//! A reply picked: what the leader says back.
	string Pick(int id, int playerId, int reply, IEntity user)
	{
		RZ_RoadPatrol p = GetPatrol(id);
		if (!p)
			return "";

		switch (reply)
		{
			case REPLY_NEWS: return News(p);
			case REPLY_WALK:
			{
				if (p.m_iWalker > 0 || p.m_aWalked.Contains(playerId))
					return "We've got company already. Next time.";

				p.m_iWalker = playerId;
				p.m_iWalkTo = Math.Min(p.m_iLeg + WALK_POINTS, p.m_aRoute.Count() - 1);
				RZ_Log.Info(string.Format("Road patrols: %1 walks a stretch with %2.", p.Title(), RZ_Names.Public(playerId)));
				return "Fall in at the back, then. Eyes on the hedges.";
			}
			case REPLY_PAPERS: return Papers(p, playerId);
			case REPLY_TOLL:
			{
				int toll = Toll(playerId);
				if (!m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, toll))
					return "That's not enough. Walk the ditch, then.";

				p.m_aCleared.Insert(playerId);
				RZ_Log.Info(string.Format("Road patrols: %1 paid %2's toll: %3 Favor.", RZ_Names.Public(playerId), p.Title(), toll));
				return "Pleasure. The road's yours till sundown.";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's papers: in their kit it's a look (as at a road post); in your own clothes, a quick
	//! search, and you're on your way. A Watched face is told to turn round.
	protected string Papers(RZ_RoadPatrol p, int playerId)
	{
		RZ_ETier tier = m_GM.GetTier(playerId, p.m_eFaction);
		RZ_Disguise disguise = Disguise();
		if (disguise && disguise.InTheirKit(playerId, p.m_eFaction))
		{
			p.m_aCleared.Insert(playerId);
			if (disguise.Look(playerId, p.m_eFaction, 2, "patrol" + p.m_iId.ToString(), "", 0))
				return "Right. Which post are you with? Never mind. Go on.";

			return "That's not one of ours. Stay where you are.";
		}

		if (RZ_Reputation.IsWatchedOrWorse(tier))
			return "I know your face from the board. Turn round and walk the other way.";

		p.m_aCleared.Insert(playerId);
		RZ_Log.Info(string.Format("Road patrols: %1 checked %2's papers.", p.Title(), RZ_Names.Public(playerId)));
		return "Fine. On your way. Stay off the road after dark.";
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Disguise Disguise()
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		if (!hostility || !hostility.GetDisguise() || !hostility.GetDisguise().IsOn())
			return null;

		return hostility.GetDisguise();
	}

	//------------------------------------------------------------------------------------------------
	//! Road news, true: the wanderers' word where the tribes keep it, else a truck on the road.
	protected string News(RZ_RoadPatrol p)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes && tribes.GetPeople())
		{
			string word = tribes.GetPeople().RoadNews(p.m_vPos);
			if (!word.IsEmpty())
				return word;
		}

		RZ_SupplyLines supply = m_GM.GetSupply();
		if (supply)
		{
			array<int> order = {RZ_EFaction.MILITARY, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, RZ_EFaction.BANDITS};
			foreach (int f : order)
			{
				string dest = supply.TruckDestination(f);
				if (!dest.IsEmpty())
					return string.Format("A truck's on the road for %1. That's all that's moving.", dest);
			}
		}

		return "Nothing but us and the crows.";
	}

	//------------------------------------------------------------------------------------------------
	//! The player walking with it fell behind: the stretch is off.
	protected void WatchWalker(RZ_RoadPatrol p)
	{
		if (p.m_iWalker < 0)
			return;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(p.m_iWalker);
		if (!ent || RZ_Util.DistXZ(ent.GetOrigin(), p.m_vPos) > WALK_KEEP * 3)
		{
			p.m_aWalked.Insert(p.m_iWalker);
			p.m_iWalker = -1;
			p.m_iWalkTo = -1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The stretch walked: a little Fame, once a patrol and once an hour a player.
	protected void WalkDone(RZ_RoadPatrol p)
	{
		int playerId = p.m_iWalker;
		p.m_iWalker = -1;
		p.m_iWalkTo = -1;
		p.m_aWalked.Insert(playerId);
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent || RZ_Util.DistXZ(ent.GetOrigin(), p.m_vPos) > WALK_KEEP)
			return;

		string key = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		if (m_mWalkThanked.Find(key, last) && m_fClock - last < WALK_COOL)
		{
			m_GM.Say(playerId, p.Title(), p.m_eFaction, "That's the stretch. Thanks.", Leader(p));
			return;
		}

		m_mWalkThanked.Set(key, m_fClock);
		m_GM.AddFame(playerId, p.m_eFaction, 2, true, 0, "walked the road with a patrol");
		m_GM.Say(playerId, p.Title(), p.m_eFaction, "That's the stretch. Obliged. I'll say so at the post.", Leader(p));
		RZ_Log.Info(string.Format("Road patrols: %1 walked a stretch with %2: +2 Fame.", RZ_Names.Public(playerId), p.Title()));
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to" without the talk screen: the leader's greeting, said.
	void HandleRead(int playerId, int id)
	{
		RZ_RoadPatrol p = GetPatrol(id);
		if (!p)
			return;

		m_GM.Say(playerId, p.Title(), p.m_eFaction, Greeting(p, playerId), Leader(p));
	}

	// ================================================================ What others hear (the masts)
	//------------------------------------------------------------------------------------------------
	//! A mast's warning: a patrol on a road within `radius` of `pos`, "" if none.
	string WarningNear(vector pos, float radius)
	{
		if (!IsOn())
			return "";

		foreach (RZ_RoadPatrol p : m_aPatrols)
		{
			float d = RZ_Util.DistXZ(p.m_vPos, pos);
			if (d > radius)
				continue;

			string dir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(pos, p.m_vPos));
			string how = "heading out";
			if (p.OnWayBack())
				how = "heading home to " + p.m_sFrom;

			return string.Format("A %1 on the road %2 of you, %3.", RZ_RoadPatrols.KindName(p.m_eFaction, p.m_bVehicle), dir, how);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Overheard on a faction's channel: one of its patrols checking in, "" if none of `f`'s is out.
	string HeardLine(int f)
	{
		if (!IsOn())
			return "";

		foreach (RZ_RoadPatrol p : m_aPatrols)
		{
			if (p.m_eFaction != f)
				continue;

			if (p.OnWayBack())
				return string.Format("\"%1 here. On our way back to %2. Road's clear behind us.\"", p.m_sLeader, p.m_sFrom);

			return string.Format("\"%1, out of %2. Walking the road, no contact.\"", p.m_sLeader, p.m_sFrom);
		}

		return "";
	}

	// ================================================================ Test hooks (tools/runtime_test)
	//------------------------------------------------------------------------------------------------
	//! A patrol set out now from the nearest holding of `f` to `near` (the runtime test).
	RZ_RoadPatrol TestSetOut(int f, vector near, bool vehicle)
	{
		RZ_Holding best;
		float bestDist = 1000000;
		foreach (RZ_Holding h : m_GM.GetTerritory().GetHoldings())
		{
			if (!h || h.m_eFaction != f)
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, near);
			if (d < bestDist)
			{
				bestDist = d;
				best = h;
			}
		}

		if (!best)
			return null;

		return SetOutFrom(f, best, vehicle);
	}

	//------------------------------------------------------------------------------------------------
	//! Its meeting with a player now, as if they'd walked up (the runtime test).
	string TestStance(int id, int playerId)
	{
		RZ_RoadPatrol p = GetPatrol(id);
		if (!p)
			return "";

		return Stance(p, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Its fight played now where it stands (the runtime test).
	void TestFight(int id)
	{
		RZ_RoadPatrol p = GetPatrol(id);
		if (p)
			Fight(p, p.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! It's lost where it stands, as if its last man fell (the runtime test). True if a Missing Patrol
	//! started.
	bool TestLose(int id)
	{
		RZ_RoadPatrol p = GetPatrol(id);
		if (!p)
			return false;

		bool started = Lost(p, p.m_vPos);
		TakeAway(p);
		m_aPatrols.RemoveItem(p);
		return started;
	}

	//------------------------------------------------------------------------------------------------
	//! All patrols gone (the runtime test, between its checks).
	void TestClear()
	{
		foreach (RZ_RoadPatrol p : m_aPatrols)
		{
			TakeAway(p);
		}

		m_aPatrols.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to set out a second vehicle as the Tick would: false when one is already out (the cap).
	bool TestVehicleRoom()
	{
		return VehicleCount() == 0;
	}
}
