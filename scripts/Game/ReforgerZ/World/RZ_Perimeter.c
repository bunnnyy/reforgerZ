//! Settings for the Perimeter (docs/LORE.md, "The Perimeter"). Every value has a
//! default, so the Perimeter runs with nothing set; prefabs are optional.
[BaseContainerProps()]
class RZ_PerimeterSettings
{
	[Attribute("1", desc: "Turn the Perimeter on: warning zone, kill zone, drones and road checkpoints")]
	bool m_bEnabled;

	[Attribute("300", desc: "Warning zone: distance (m) from the edge where the drone and the loudspeaker start")]
	float m_fWarnDistance;

	[Attribute("100", desc: "Kill zone: distance (m) from the edge where the warning shot comes, then the fatal one")]
	float m_fKillDistance;

	[Attribute("8", desc: "Seconds between the warning shot and the fatal one (rounded up to the server tick)")]
	float m_fGraceSeconds;

	[Attribute("1", desc: "Pull the edge in to the dry land plus the sea margin, so on island maps the line sits offshore. Off = the whole world box")]
	bool m_bFitToLand;

	[Attribute("200", desc: "How far out to sea (m) the edge sits beyond the outermost land, when fitting to land. Set it above the warning distance and beaches are never warned. Missions at the line measure from where the land starts, so they work either way")]
	float m_fSeaMargin;

	[Attribute("0", desc: "Use the rectangle below instead of detecting the edge (if detection gets it wrong)")]
	bool m_bOverride;

	[Attribute("0 0 0", desc: "Override rectangle: south-west corner (x and z; y is ignored)")]
	vector m_vOverrideMin;

	[Attribute("0 0 0", desc: "Override rectangle: north-east corner (x and z; y is ignored)")]
	vector m_vOverrideMax;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Drone that hovers over a player in the warning zone (optional: just the loudspeaker without it). No collision or physics", "et")]
	ResourceName m_sDronePrefab;

	[Attribute("25", desc: "Drone height above the player (m)")]
	float m_fDroneHeight;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Warning shot: a gunshot sound or bullet impact effect spawned a few metres from the player (optional: a message without it)", "et")]
	ResourceName m_sWarningShotPrefab;

	[Attribute("1", desc: "Sealed road checkpoints where roads reach the edge")]
	bool m_bCheckpoints;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Checkpoint barricade: a small composition of concrete blocks and a closed barrier. Checkpoints are off without it", "et")]
	ResourceName m_sBarricadePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Checkpoint guard: a soldier character without AI, or a prop. Stands outside the playable area and can't be hurt", "et")]
	ResourceName m_sGuardPrefab;

	[Attribute("2", UIWidgets.Slider, "Guards behind each barricade", "0 4 1")]
	int m_iGuardsPerCheckpoint;

	[Attribute("6", UIWidgets.Slider, "Most checkpoints on one map", "1 8 1")]
	int m_iMaxCheckpoints;

	[Attribute("60", desc: "Barricade distance (m) inside the edge")]
	float m_fCheckpointInset;

	[Attribute("800", desc: "Minimum distance (m) between two checkpoints")]
	float m_fCheckpointSpacing;

	[Attribute("150", desc: "Loudspeaker warning within this distance (m) of a checkpoint")]
	float m_fCheckpointWarnRadius;

	[Attribute("60", desc: "Lethal within this distance (m) of a checkpoint (warning shot first)")]
	float m_fCheckpointKillRadius;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The Blind Spot's smuggler: a civilian character without AI, or a figure prop (optional: falls back to Puzzles > Civilian)", "et")]
	ResourceName m_sSmugglerPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Sealed package for The Blind Spot and The Crossing: an item with RZ_MissionItemComponent (optional: falls back to Missions > Mission item)", "et")]
	ResourceName m_sPackagePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Body at the line for Shot at the Line: RZ_ClueComponent + RZ_ReadClueAction (optional: falls back to Puzzles > Body)", "et")]
	ResourceName m_sLineBodyPrefab;
}

enum RZ_EPerimeterZone
{
	NONE,
	WARNING,
	KILL
}

//! One player near the line.
class RZ_PerimeterContact
{
	RZ_EPerimeterZone m_eZone = RZ_EPerimeterZone.NONE;
	IEntity m_Drone;
	float m_fSinceShot = -1;	// seconds since the warning shot, -1 = no shot yet
}

//! A stretch of the line that isn't covered for a while (The Blind Spot).
class RZ_BlindSpot
{
	int m_iHandle;
	vector m_vCenter;
	float m_fRadius;
}

//! One point along the edge, for missions at the line: how far in the land starts.
class RZ_ShorePoint
{
	vector m_vEdge;		// on the edge, on the ground (or the sea)
	vector m_vInward;	// unit vector into the Zone
	float m_fLand;		// metres from the edge to the first dry ground: 0 where the edge runs over land
}

//! The Perimeter: the unseen cordon at the edge of the playable world, on any map.
//! Nothing is built along it except the sealed road checkpoints.
//!
//! - The edge is a rectangle: the world's bounds pulled in to the dry land plus a sea
//!   margin (on islands the line sits offshore), or a rectangle the server sets.
//!   Only that outer edge counts, so swimming at a beach inside it is fine.
//! - Warning zone: one drone over the player (attached to them, so it follows without
//!   polling) and the loudspeaker. The drone goes when they leave.
//! - Kill zone: a warning shot, then after a grace period, a fatal one.
//! - Road checkpoints: a barricade where a road reaches the edge, guards outside the
//!   playable area. Found from the road network or RZ_CheckpointMarkerEntity. Spawned
//!   once at start. Loudspeaker near them, lethal close to them.
//! - Blind spots: a stretch where nothing happens for a while (The Blind Spot mission).
//! - After the cure (RZ_WorldFacts.IsCured) the line stands down: the drones go home, nobody
//!   shoots, and the loudspeakers read the new order. The barriers stay closed: the gates open
//!   with the wipe (docs/LORE.md, "Endings"). Anyone who walks into the kill zone is turned back
//!   to the warning zone (no shot, no drone): the line is not open.
//!
//! Server only, polled on the game mode's tick.
class RZ_Perimeter
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_PerimeterSettings m_Settings;
	protected bool m_bActive;
	protected float m_fMinX;
	protected float m_fMinZ;
	protected float m_fMaxX;
	protected float m_fMaxZ;
	protected float m_fWarn = 300;
	protected float m_fKill = 100;
	protected float m_fClock;
	protected ref map<int, ref RZ_PerimeterContact> m_mContacts = new map<int, ref RZ_PerimeterContact>();
	protected ref map<int, float> m_mLastWarned = new map<int, float>();
	protected ref array<vector> m_aCheckpoints = {};
	protected ref array<IEntity> m_aCheckpointProps = {};
	protected ref array<ref RZ_BlindSpot> m_aBlindSpots = {};
	protected int m_iNextBlindSpot = 1;
	protected ref array<IEntity> m_aEffects = {};
	protected ref array<float> m_aEffectTimes = {};
	protected ref array<ref RZ_ShorePoint> m_aShore = {};
	protected bool m_bShoreBuilt;
	protected ref map<string, bool> m_mSpotFound = new map<string, bool>();

	protected static const float LEAVE_MARGIN = 25;		// leave the zone by this much before the drone goes (no flicker)
	protected static const float TURN_BACK_MARGIN = 40;	// after the cure: turned back this far inside the kill zone's edge
	protected static const float WARN_REPEAT = 30;		// seconds before the loudspeaker repeats itself on re-entry
	protected static const float EFFECT_LIFETIME = 20;
	protected static const float CORNER_MARGIN = 500;	// missions keep away from the corners
	protected static const float GUARD_DEPTH = 20;		// guards stand this far outside the edge
	protected static const float MIN_SIZE = 1000;
	protected static const int MAX_CHECKPOINTS = 8;
	protected static const float SHORE_SPACING = 100;	// metres between shore points along the edge
	protected static const int MAX_SHORE_POINTS = 400;	// spacing grows on big maps to stay under this
	protected static const float SHORE_WALK = 600;		// how far in (m) to look for land; further in, the point is left out
	protected static const float SHORE_STEP = 10;

	static const string LOUDSPEAKER = "Loudspeaker";

	//------------------------------------------------------------------------------------------------
	void RZ_Perimeter(RZ_GameModeComponent gm, RZ_PerimeterSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsActive()
	{
		return m_bActive;
	}

	float GetWarnDistance() { return m_fWarn; }
	float GetKillDistance() { return m_fKill; }

	//------------------------------------------------------------------------------------------------
	//! Works out the edge and builds the checkpoints. Called once by the game mode at start.
	void Init()
	{
		if (!m_Settings.m_bEnabled)
		{
			RZ_Log.Info("The Perimeter is off.");
			return;
		}

		m_fKill = Math.Max(10, m_Settings.m_fKillDistance);
		m_fWarn = Math.Max(m_fKill + 50, m_Settings.m_fWarnDistance);

		if (!ComputeBounds())
		{
			RZ_Log.Error("Couldn't work out the map edge, so the Perimeter is off. Set the override rectangle on the game mode.");
			return;
		}

		m_bActive = true;
		RZ_Log.Info(string.Format("Perimeter: x %1 to %2, z %3 to %4. Warning zone %5 m, kill zone %6 m.",
			RZ_Util.RoundToInt(m_fMinX), RZ_Util.RoundToInt(m_fMaxX), RZ_Util.RoundToInt(m_fMinZ), RZ_Util.RoundToInt(m_fMaxZ),
			RZ_Util.RoundToInt(m_fWarn), RZ_Util.RoundToInt(m_fKill)));

		SetupCheckpoints();
	}

	// ================================================================ The edge
	//------------------------------------------------------------------------------------------------
	protected bool ComputeBounds()
	{
		vector oMin = m_Settings.m_vOverrideMin;
		vector oMax = m_Settings.m_vOverrideMax;
		if (m_Settings.m_bOverride)
		{
			if (oMax[0] - oMin[0] >= MIN_SIZE && oMax[2] - oMin[2] >= MIN_SIZE)
			{
				SetBounds(oMin[0], oMin[2], oMax[0], oMax[2]);
				return true;
			}

			RZ_Log.Warn("The Perimeter override rectangle is smaller than 1 km or inverted; detecting the edge instead.");
		}

		vector mins, maxs;
		if (!GetWorldBounds(mins, maxs))
			return false;

		SetBounds(mins[0], mins[2], maxs[0], maxs[2]);
		if (m_Settings.m_bFitToLand)
			FitToLand();

		return m_fMaxX - m_fMinX >= MIN_SIZE && m_fMaxZ - m_fMinZ >= MIN_SIZE;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool GetWorldBounds(out vector mins, out vector maxs)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		world.GetBoundBox(mins, maxs); // VERIFY: BaseWorld.GetBoundBox() covers the terrain (it may be larger; FitToLand trims it)
		return maxs[0] - mins[0] >= MIN_SIZE && maxs[2] - mins[2] >= MIN_SIZE;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBounds(float minX, float minZ, float maxX, float maxZ)
	{
		m_fMinX = minX;
		m_fMinZ = minZ;
		m_fMaxX = maxX;
		m_fMaxZ = maxZ;
	}

	//------------------------------------------------------------------------------------------------
	//! Shrink the rectangle to the dry land plus the sea margin. Samples the terrain on a
	//! coarse grid (about 200 x 200) once at start; sea is anything RZ_Util.IsWater calls water.
	protected void FitToLand()
	{
		BaseWorld world = GetGame().GetWorld();
		float step = Math.Max(50, Math.Max(m_fMaxX - m_fMinX, m_fMaxZ - m_fMinZ) / 200);
		float landMinX = m_fMaxX + 1;
		float landMinZ = m_fMaxZ + 1;
		float landMaxX = m_fMinX - 1;
		float landMaxZ = m_fMinZ - 1;

		for (float x = m_fMinX; x <= m_fMaxX; x += step)
		{
			for (float z = m_fMinZ; z <= m_fMaxZ; z += step)
			{
				if (RZ_Util.IsWater(Vector(x, world.GetSurfaceY(x, z), z)))
					continue;

				landMinX = Math.Min(landMinX, x);
				landMaxX = Math.Max(landMaxX, x);
				landMinZ = Math.Min(landMinZ, z);
				landMaxZ = Math.Max(landMaxZ, z);
			}
		}

		if (landMaxX < landMinX)
			return; // no land at all: keep the world box

		// Land can reach up to one step past the outermost sample: add it, so the margin is at least the setting
		float margin = Math.Max(0, m_Settings.m_fSeaMargin) + step;
		SetBounds(Math.Max(m_fMinX, landMinX - margin), Math.Max(m_fMinZ, landMinZ - margin),
			Math.Min(m_fMaxX, landMaxX + margin), Math.Min(m_fMaxZ, landMaxZ + margin));
	}

	//------------------------------------------------------------------------------------------------
	//! Distance (m) from pos to the edge. Negative = outside the playable area.
	float EdgeDistance(vector pos)
	{
		float d = pos[0] - m_fMinX;
		d = Math.Min(d, m_fMaxX - pos[0]);
		d = Math.Min(d, pos[2] - m_fMinZ);
		d = Math.Min(d, m_fMaxZ - pos[2]);
		return d;
	}

	//------------------------------------------------------------------------------------------------
	//! Unit vector from the nearest side of the edge into the Zone.
	vector InwardAt(vector pos)
	{
		float best = pos[0] - m_fMinX;
		vector dir = Vector(1, 0, 0);

		float east = m_fMaxX - pos[0];
		if (east < best)
		{
			best = east;
			dir = Vector(-1, 0, 0);
		}

		float south = pos[2] - m_fMinZ;
		if (south < best)
		{
			best = south;
			dir = Vector(0, 0, 1);
		}

		float north = m_fMaxZ - pos[2];
		if (north < best)
			dir = Vector(0, 0, -1);

		return dir;
	}

	//------------------------------------------------------------------------------------------------
	//! The ground `depth` metres inside the edge from an edge point.
	vector PointInside(vector edge, vector inward, float depth)
	{
		return RZ_Util.GroundPos(edge + inward * depth);
	}

	// ================================================================ The shore (missions at the line)
	//------------------------------------------------------------------------------------------------
	//! Points along the edge, away from the corners, each with how far in the land starts. Where
	//! the edge runs over land that's 0; where it runs offshore (FitToLand), the land starts at
	//! the coast, at least the sea margin in. Points with no land within SHORE_WALK are left out.
	//! Built once, the first time a mission asks.
	int GetShoreCount()
	{
		if (!m_bShoreBuilt)
			BuildShore();

		return m_aShore.Count();
	}

	//------------------------------------------------------------------------------------------------
	RZ_ShorePoint GetShorePoint(int idx)
	{
		if (!m_bShoreBuilt)
			BuildShore();

		if (idx < 0 || idx >= m_aShore.Count())
			return null;

		return m_aShore[idx];
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildShore()
	{
		m_bShoreBuilt = true;
		if (!m_bActive)
			return;

		float lenX = m_fMaxX - m_fMinX - 2 * CORNER_MARGIN;
		float lenZ = m_fMaxZ - m_fMinZ - 2 * CORNER_MARGIN;
		if (lenX < CORNER_MARGIN || lenZ < CORNER_MARGIN)
			return;

		float spacing = Math.Max(SHORE_SPACING, 2 * (lenX + lenZ) / MAX_SHORE_POINTS);
		for (float x = m_fMinX + CORNER_MARGIN; x <= m_fMaxX - CORNER_MARGIN; x += spacing)
		{
			AddShorePoint(Vector(x, 0, m_fMinZ), Vector(0, 0, 1));
			AddShorePoint(Vector(x, 0, m_fMaxZ), Vector(0, 0, -1));
		}

		for (float z = m_fMinZ + CORNER_MARGIN; z <= m_fMaxZ - CORNER_MARGIN; z += spacing)
		{
			AddShorePoint(Vector(m_fMinX, 0, z), Vector(1, 0, 0));
			AddShorePoint(Vector(m_fMaxX, 0, z), Vector(-1, 0, 0));
		}

		int sea = 0;
		foreach (RZ_ShorePoint p : m_aShore)
		{
			if (p.m_fLand > 0)
				sea++;
		}

		RZ_Log.Info(string.Format("Perimeter shore: %1 points along the edge with land within %2 m, %3 of them on a sea edge.",
			m_aShore.Count(), RZ_Util.RoundToInt(SHORE_WALK), sea));
	}

	//------------------------------------------------------------------------------------------------
	//! Walk in from the edge until the ground is dry.
	protected void AddShorePoint(vector edge, vector inward)
	{
		for (float depth = 0; depth <= SHORE_WALK; depth += SHORE_STEP)
		{
			if (RZ_Util.IsWater(PointInside(edge, inward, depth)))
				continue;

			RZ_ShorePoint p = new RZ_ShorePoint();
			p.m_vEdge = RZ_Util.GroundPos(edge);
			p.m_vInward = inward;
			p.m_fLand = depth;
			m_aShore.Insert(p);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Whether a mission at the line found anywhere on this map: -1 not checked yet, 0 no, 1 yes.
	//! Missions cache it here so one that can't fit isn't drawn again and again.
	int GetSpotState(string mission)
	{
		bool found;
		if (!m_mSpotFound.Find(mission, found))
			return -1;

		if (found)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	void SetSpotState(string mission, bool found)
	{
		m_mSpotFound.Set(mission, found);
	}

	// ================================================================ Zones
	//------------------------------------------------------------------------------------------------
	//! Which zone pos is in, counting blind spots and checkpoints. `extra` widens the
	//! warning zone (a player already near the line has to leave it by that much).
	RZ_EPerimeterZone GetZone(vector pos, float extra = 0)
	{
		if (!m_bActive)
			return RZ_EPerimeterZone.NONE;

		RZ_EPerimeterZone zone = RZ_EPerimeterZone.NONE;
		float d = EdgeDistance(pos);
		if (d < m_fKill)
			zone = RZ_EPerimeterZone.KILL;
		else if (d < m_fWarn + extra)
			zone = RZ_EPerimeterZone.WARNING;

		// A blind spot covers the playable side of the line only
		if (zone != RZ_EPerimeterZone.NONE && d >= 0 && IsInBlindSpot(pos))
			zone = RZ_EPerimeterZone.NONE;

		float cp = GetCheckpointDistance(pos);
		if (cp < m_Settings.m_fCheckpointKillRadius)
			zone = RZ_EPerimeterZone.KILL;
		else if (cp < m_Settings.m_fCheckpointWarnRadius + extra && zone == RZ_EPerimeterZone.NONE)
			zone = RZ_EPerimeterZone.WARNING;

		return zone;
	}

	//------------------------------------------------------------------------------------------------
	//! True for anywhere the cordon would warn or shoot (blind spots ignored). Missions and
	//! spawn points stay out of it.
	bool IsNearLine(vector pos)
	{
		if (!m_bActive)
			return false;

		return EdgeDistance(pos) < m_fWarn || GetCheckpointDistance(pos) < m_Settings.m_fCheckpointWarnRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! IsNearLine with the warning zone and the checkpoints' warning radius widened by `extra` metres
	//! (the land grid keeps wild land 200 m back from the line). False if there's no Perimeter.
	bool IsNearLineWithin(vector pos, float extra)
	{
		if (!m_bActive)
			return false;

		return EdgeDistance(pos) < m_fWarn + extra || GetCheckpointDistance(pos) < m_Settings.m_fCheckpointWarnRadius + extra;
	}

	//------------------------------------------------------------------------------------------------
	//! IsNearLine without a game mode reference. False if there's no Perimeter.
	static bool IsNearLineAt(vector pos)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return false;

		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (!perimeter)
			return false;

		return perimeter.IsNearLine(pos);
	}

	//------------------------------------------------------------------------------------------------
	float GetCheckpointDistance(vector pos)
	{
		float best = float.MAX;
		foreach (vector cp : m_aCheckpoints)
		{
			float d = RZ_Util.DistXZ(pos, cp);
			if (d < best)
				best = d;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest road checkpoint to pos (for notes that warn about it). False if there are none.
	bool GetNearestCheckpoint(vector pos, out vector checkpoint)
	{
		float best = float.MAX;
		bool found = false;
		foreach (vector cp : m_aCheckpoints)
		{
			float d = RZ_Util.DistXZ(pos, cp);
			if (d < best)
			{
				best = d;
				checkpoint = cp;
				found = true;
			}
		}

		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! The sealed road checkpoints (the map's icon layer shows them).
	void GetCheckpoints(notnull array<vector> result)
	{
		foreach (vector cp : m_aCheckpoints)
		{
			result.Insert(cp);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Points on roads within `radius` of `center` (centre lines, one every few metres as the road
	//! network stores them), for posts inside the Zone (RZ_Checkpoints). Empty without a road network.
	static void RoadPointsNear(vector center, float radius, notnull array<vector> result)
	{
		RoadNetworkManager roads = GetRoadNetwork();
		if (!roads)
			return;

		vector mins = Vector(center[0] - radius, -1000, center[2] - radius);
		vector maxs = Vector(center[0] + radius, 10000, center[2] + radius);
		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(mins, maxs, found);
		foreach (BaseRoad road : found)
		{
			if (!road)
				continue;

			array<vector> points = {};
			road.GetPoints(points);
			foreach (vector pt : points)
			{
				if (RZ_Util.DistXZ(pt, center) <= radius)
					result.Insert(pt);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Turn the cordon off within `radius` of `center` until RemoveBlindSpot. Returns a handle.
	int AddBlindSpot(vector center, float radius)
	{
		RZ_BlindSpot spot = new RZ_BlindSpot();
		spot.m_iHandle = m_iNextBlindSpot;
		spot.m_vCenter = center;
		spot.m_fRadius = radius;
		m_iNextBlindSpot++;
		m_aBlindSpots.Insert(spot);
		return spot.m_iHandle;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveBlindSpot(int handle)
	{
		for (int i = m_aBlindSpots.Count() - 1; i >= 0; i--)
		{
			if (m_aBlindSpots[i].m_iHandle == handle)
				m_aBlindSpots.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsInBlindSpot(vector pos)
	{
		foreach (RZ_BlindSpot spot : m_aBlindSpots)
		{
			if (RZ_Util.DistXZ(pos, spot.m_vCenter) <= spot.m_fRadius)
				return true;
		}

		return false;
	}

	// ================================================================ Players
	//------------------------------------------------------------------------------------------------
	//! Server tick. Cheap: a rectangle test and a few checkpoint distances per player.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bActive)
			return;

		m_fClock += dt;
		ExpireEffects();

		// Players who died or left lose their drone
		array<int> gone = {};
		foreach (int contactId, RZ_PerimeterContact contact : m_mContacts)
		{
			if (!ids.Contains(contactId))
				gone.Insert(contactId);
		}

		foreach (int goneId : gone)
		{
			Release(goneId);
		}

		// The cure is out: the line has stood down
		bool standDown = RZ_WorldFacts.Get().IsCured();
		foreach (int i, IEntity p : players)
		{
			if (standDown)
				UpdateStoodDown(ids[i], p);
			else
				UpdatePlayer(ids[i], p, dt);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! After the cure: no drone and no shots, only the loudspeaker's new order, once per approach.
	protected void UpdateStoodDown(int playerId, IEntity player)
	{
		vector pos = player.GetOrigin();
		RZ_PerimeterContact c;
		bool known = m_mContacts.Find(playerId, c);
		float extra = 0;
		if (known)
			extra = LEAVE_MARGIN;

		RZ_EPerimeterZone zone = GetZone(pos, extra);
		if (zone == RZ_EPerimeterZone.NONE)
		{
			if (known)
				Release(playerId);

			return;
		}

		if (!known)
		{
			c = new RZ_PerimeterContact();
			m_mContacts.Insert(playerId, c);
		}

		// A drone still up from before the cure goes home
		if (c.m_Drone)
		{
			RZ_Util.DeleteUnit(c.m_Drone);
			c.m_Drone = null;
		}

		c.m_fSinceShot = -1;

		// Nobody shoots any more, but the line isn't open: whoever walks into the kill zone (or up
		// to a barrier) is turned back
		if (zone == RZ_EPerimeterZone.KILL)
		{
			TurnBack(playerId, pos);
			c.m_eZone = RZ_EPerimeterZone.WARNING;
			return;
		}

		if (c.m_eZone == RZ_EPerimeterZone.NONE)
		{
			float last;
			if (!m_mLastWarned.Find(playerId, last) || m_fClock - last >= WARN_REPEAT)
			{
				m_mLastWarned.Set(playerId, m_fClock);
				m_GM.Say(playerId, LOUDSPEAKER, -1, GetStoodDownText(pos));
			}
		}

		c.m_eZone = zone;
	}

	//------------------------------------------------------------------------------------------------
	//! After the cure: back to the warning zone, TURN_BACK_MARGIN inside the kill zone's edge (or up
	//! the road from a checkpoint), with a voice from past the line. No shot and no drone.
	protected void TurnBack(int playerId, vector pos)
	{
		vector back;
		string text;
		vector checkpoint;
		if (GetCheckpointDistance(pos) < m_Settings.m_fCheckpointKillRadius && GetNearestCheckpoint(pos, checkpoint))
		{
			back = checkpoint + InwardAt(checkpoint) * (m_Settings.m_fCheckpointKillRadius + TURN_BACK_MARGIN);
			text = "\"Back behind the barrier. You will be called for processing.\" Nobody raises a rifle, and nobody lets you closer either. You walk back up the road.";
		}
		else
		{
			back = pos + InwardAt(pos) * (m_fKill + TURN_BACK_MARGIN - EdgeDistance(pos));
			text = "\"Stop. Turn around.\" The voice comes from past the line, close, from someone you never see. \"The quarantine is lifted. The line is not open. Processing will begin at the road checkpoints.\" You walk back to the trees.";
		}

		if (!RZ_Util.MovePlayer(playerId, back))
			return;

		m_GM.Say(playerId, LOUDSPEAKER, -1, text);
		RZ_Log.Info(string.Format("Perimeter (stood down): player %1 turned back from the line.", playerId));
	}

	//------------------------------------------------------------------------------------------------
	protected string GetStoodDownText(vector pos)
	{
		if (GetCheckpointDistance(pos) < m_Settings.m_fCheckpointWarnRadius + LEAVE_MARGIN)
			return "\"Attention. The quarantine order for this area has been lifted. This checkpoint will open for processing. Remain behind the barrier until you are called.\"\nNobody behind the concrete raises a rifle. Nobody opens the barrier either.";

		return "\"Attention. The quarantine order for this area has been lifted. Remain where you are. Processing will begin at the road checkpoints.\"\nNo drone comes down. Somewhere out past the line, an engine starts and drives away.";
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayer(int playerId, IEntity player, float dt)
	{
		vector pos = player.GetOrigin();
		RZ_PerimeterContact c;
		bool known = m_mContacts.Find(playerId, c);
		float extra = 0;
		if (known)
			extra = LEAVE_MARGIN;

		RZ_EPerimeterZone zone = GetZone(pos, extra);
		if (zone == RZ_EPerimeterZone.NONE)
		{
			if (known)
				Release(playerId);

			return;
		}

		if (!known)
		{
			c = new RZ_PerimeterContact();
			m_mContacts.Insert(playerId, c);
		}

		if (c.m_Drone)
			KeepDroneOver(c.m_Drone, player);
		else
			c.m_Drone = SpawnDrone(player);

		// The loudspeaker, once per approach
		if (c.m_eZone == RZ_EPerimeterZone.NONE)
		{
			float last;
			if (!m_mLastWarned.Find(playerId, last) || m_fClock - last >= WARN_REPEAT)
			{
				m_mLastWarned.Set(playerId, m_fClock);
				m_GM.Say(playerId, LOUDSPEAKER, -1, GetWarningText(pos));
			}
		}

		if (zone == RZ_EPerimeterZone.KILL)
		{
			if (c.m_fSinceShot < 0)
			{
				c.m_fSinceShot = 0;
				WarningShot(playerId, player);
			}
			else
			{
				c.m_fSinceShot = c.m_fSinceShot + dt;
				if (c.m_fSinceShot >= m_Settings.m_fGraceSeconds)
				{
					c.m_fSinceShot = -1;
					Shoot(playerId, player);
				}
			}
		}
		else
		{
			c.m_fSinceShot = -1;
		}

		c.m_eZone = zone;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetWarningText(vector pos)
	{
		if (GetCheckpointDistance(pos) < m_Settings.m_fCheckpointWarnRadius + LEAVE_MARGIN)
			return "\"This road is closed. You are approaching the quarantine line. Turn back.\"";

		return "\"You are approaching the quarantine line. Turn back.\"";
	}

	//------------------------------------------------------------------------------------------------
	protected void WarningShot(int playerId, IEntity player)
	{
		SpawnEffect(RZ_Util.RandomPointInRing(player.GetOrigin(), 3, 6));
		m_GM.Say(playerId, LOUDSPEAKER, -1, "A shot cracks past you and kicks up the ground.\n\"Final warning. Turn back now.\"\nGet away from the line: the next one won't miss.");
	}

	//------------------------------------------------------------------------------------------------
	//! The same for players on foot and in vehicles.
	protected void Shoot(int playerId, IEntity player)
	{
		SpawnEffect(player.GetOrigin());
		RZ_Util.KillCharacter(player);
		m_GM.NotifyPlayer(playerId, "The quarantine line", "You never saw the shooter.");
		RZ_Log.Info(string.Format("%1 was shot at the quarantine line at %2.", RZ_Util.PlayerName(playerId), RZ_SettlementScanner.GridReference(player.GetOrigin())));
	}

	//------------------------------------------------------------------------------------------------
	//! The warning-shot effect at pos, if one is set. Removed again after a while.
	void SpawnEffect(vector pos)
	{
		IEntity e = RZ_Util.SpawnPrefab(m_Settings.m_sWarningShotPrefab, pos);
		if (!e)
			return;

		m_aEffects.Insert(e);
		m_aEffectTimes.Insert(m_fClock);
	}

	//------------------------------------------------------------------------------------------------
	protected void ExpireEffects()
	{
		for (int i = m_aEffects.Count() - 1; i >= 0; i--)
		{
			if (m_aEffects[i] && m_fClock - m_aEffectTimes[i] < EFFECT_LIFETIME)
				continue;

			RZ_Util.DeleteUnit(m_aEffects[i]);
			m_aEffects.Remove(i);
			m_aEffectTimes.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity SpawnDrone(IEntity player)
	{
		if (m_Settings.m_sDronePrefab.IsEmpty())
			return null;

		IEntity drone = RZ_Util.SpawnPrefab(m_Settings.m_sDronePrefab, player.GetOrigin() + Vector(0, m_Settings.m_fDroneHeight, 0));
		// In a vehicle or a boat the character hangs off its seat: don't attach to that, the
		// tick moves the drone over them instead (KeepDroneOver)
		if (drone && !RZ_Util.IsInVehicle(player))
		{
			// RECALC_LOCAL_TRANSFORM keeps the drone where it is (above the player) instead of
			// reading its world position as an offset from the player
			player.AddChild(drone, -1, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM); // VERIFY: the attached drone follows the player on every client, with no polling
		}

		return drone;
	}

	//------------------------------------------------------------------------------------------------
	//! Fallback if attaching didn't work: move the drone over the player on each tick.
	protected void KeepDroneOver(IEntity drone, IEntity player)
	{
		if (drone.GetParent() == player)
			return;

		vector want = player.GetOrigin() + Vector(0, m_Settings.m_fDroneHeight, 0);
		if (RZ_Util.DistXZ(drone.GetOrigin(), want) > 10)
			drone.SetOrigin(want); // VERIFY: SetOrigin on a spawned prop also moves it for clients
	}

	//------------------------------------------------------------------------------------------------
	protected void Release(int playerId)
	{
		RZ_PerimeterContact c;
		if (m_mContacts.Find(playerId, c) && c && c.m_Drone)
			RZ_Util.DeleteUnit(c.m_Drone);

		m_mContacts.Remove(playerId);
	}

	// ================================================================ Checkpoints
	//------------------------------------------------------------------------------------------------
	//! Placed markers win; otherwise roads that reach the edge. Spawned once, never moved.
	protected void SetupCheckpoints()
	{
		if (!m_Settings.m_bCheckpoints)
			return;

		if (m_Settings.m_sBarricadePrefab.IsEmpty())
		{
			RZ_Log.Info("No checkpoint barricade prefab set: road checkpoints are off.");
			return;
		}

		array<vector> spots = {};
		string source = "markers";
		array<RZ_CheckpointMarkerEntity> markers = RZ_CheckpointMarkerEntity.GetAll();
		if (!markers.IsEmpty())
		{
			foreach (RZ_CheckpointMarkerEntity marker : markers)
			{
				spots.Insert(marker.GetOrigin());
			}
		}
		else
		{
			source = "roads";
			FindRoadExits(spots);
		}

		int cap = Math.Min(m_Settings.m_iMaxCheckpoints, MAX_CHECKPOINTS);
		foreach (vector spot : spots)
		{
			if (m_aCheckpoints.Count() >= cap)
				break;

			BuildCheckpoint(spot);
		}

		RZ_Log.Info(string.Format("Perimeter: %1 road checkpoints (from %2).", m_aCheckpoints.Count(), source));
	}

	//------------------------------------------------------------------------------------------------
	//! Where roads cross the checkpoint line (m_fCheckpointInset inside the edge), widest
	//! roads first, spaced apart. Empty if the map has no road network.
	protected void FindRoadExits(notnull array<vector> result)
	{
		RoadNetworkManager roads = GetRoadNetwork();
		if (!roads)
		{
			RZ_Log.Warn("No road network found: place RZ_CheckpointMarkerEntity entities for road checkpoints.");
			return;
		}

		array<vector> crossings = {};
		array<float> widths = {};
		float inset = m_Settings.m_fCheckpointInset;
		float band = m_fWarn + 50;

		for (int side = 0; side < 4; side++)
		{
			vector stripMin, stripMax;
			GetEdgeStrip(side, band, stripMin, stripMax);

			array<BaseRoad> found = {};
			roads.GetRoadsInAABB(stripMin, stripMax, found);
			foreach (BaseRoad road : found)
			{
				if (!road)
					continue;

				array<vector> points = {};
				road.GetPoints(points);
				float width = road.GetWidth(); // main roads are wider

				for (int k = 1; k < points.Count(); k++)
				{
					float d0 = EdgeDistance(points[k - 1]) - inset;
					float d1 = EdgeDistance(points[k]) - inset;
					if (d0 * d1 > 0 || d0 == d1)
						continue;

					float t = d0 / (d0 - d1);
					vector crossing = points[k - 1] + (points[k] - points[k - 1]) * t;
					crossings.Insert(RZ_Util.GroundPos(crossing));
					widths.Insert(width);
				}
			}
		}

		while (!crossings.IsEmpty() && result.Count() < MAX_CHECKPOINTS)
		{
			int best = 0;
			foreach (int j, float w : widths)
			{
				if (w > widths[best])
					best = j;
			}

			vector candidate = crossings[best];
			crossings.Remove(best);
			widths.Remove(best);

			bool tooClose = false;
			foreach (vector taken : result)
			{
				if (RZ_Util.DistXZ(taken, candidate) < m_Settings.m_fCheckpointSpacing)
					tooClose = true;
			}

			if (!tooClose)
				result.Insert(candidate);
		}
	}

	//------------------------------------------------------------------------------------------------
	static RoadNetworkManager GetRoadNetwork()
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return null;

		return aiWorld.GetRoadNetworkManager();
	}

	//------------------------------------------------------------------------------------------------
	//! Box along one side of the edge (0 west, 1 east, 2 south, 3 north), `band` metres deep.
	protected void GetEdgeStrip(int side, float band, out vector mins, out vector maxs)
	{
		mins = Vector(m_fMinX - 10, -1000, m_fMinZ - 10);
		maxs = Vector(m_fMaxX + 10, 10000, m_fMaxZ + 10);
		switch (side)
		{
			case 0:
				maxs[0] = m_fMinX + band;
				break;
			case 1:
				mins[0] = m_fMaxX - band;
				break;
			case 2:
				maxs[2] = m_fMinZ + band;
				break;
			default:
				mins[2] = m_fMaxZ - band;
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Barricade on the road facing the edge; guards behind it, outside the playable area.
	protected void BuildCheckpoint(vector spot)
	{
		vector inward = InwardAt(spot);
		vector outward = inward * -1;
		float yaw = RZ_Compass.Bearing(spot, spot + outward * 10);
		IEntity barricade = RZ_Util.SpawnPrefab(m_Settings.m_sBarricadePrefab, RZ_Util.GroundPos(spot), yaw);
		if (!barricade)
			return;

		m_aCheckpointProps.Insert(barricade);
		m_aCheckpoints.Insert(spot);

		vector edge = spot - inward * EdgeDistance(spot);
		vector across = Vector(inward[2], 0, -inward[0]);
		int guards = Math.Clamp(m_Settings.m_iGuardsPerCheckpoint, 0, 4);
		for (int g = 0; g < guards; g++)
		{
			float lateral = (g - (guards - 1) * 0.5) * 4;
			vector guardPos = RZ_Util.GroundPos(edge + outward * GUARD_DEPTH + across * lateral);
			IEntity guard = RZ_Util.SpawnPrefab(m_Settings.m_sGuardPrefab, guardPos, yaw + 180);
			if (!guard)
				continue;

			MakeUntouchable(guard);
			m_aCheckpointProps.Insert(guard);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Guards can't be killed or looted: they're out of reach, and damage is off as well.
	protected static void MakeUntouchable(IEntity e)
	{
		RZ_Util.MakeUntouchable(e);
	}
}
