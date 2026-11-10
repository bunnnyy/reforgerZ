//! Settings for the Perimeter (docs/LORE.md, "The Perimeter"). Every value has a
//! default, so the Perimeter runs with nothing set; prefabs are optional.
[BaseContainerProps()]
class RZ_PerimeterSettings
{
	[Attribute("1", desc: "Turn the Perimeter on: warning zone, kill zone, drones and road checkpoints")]
	bool m_bEnabled = true;

	[Attribute("300", desc: "Warning zone: distance (m) from the edge where the drone and the loudspeaker start")]
	float m_fWarnDistance = 300;

	[Attribute("100", desc: "Kill zone: distance (m) from the edge where the warning shot comes, then the fatal one")]
	float m_fKillDistance = 100;

	[Attribute("8", desc: "Seconds between the warning shot and the fatal one (rounded up to the server tick)")]
	float m_fGraceSeconds = 8;

	[Attribute("1", desc: "Pull the edge in to the dry land plus the sea margin, so on island maps the line sits offshore. Off = the whole world box")]
	bool m_bFitToLand = true;

	[Attribute("200", desc: "How far out to sea (m) the edge sits beyond the outermost land, when fitting to land. Above the warning distance, beaches are never warned (and The Crossing only uses land edges)")]
	float m_fSeaMargin = 200;

	[Attribute("0", desc: "Use the rectangle below instead of detecting the edge (if detection gets it wrong)")]
	bool m_bOverride;

	[Attribute("0 0 0", desc: "Override rectangle: south-west corner (x and z; y is ignored)")]
	vector m_vOverrideMin;

	[Attribute("0 0 0", desc: "Override rectangle: north-east corner (x and z; y is ignored)")]
	vector m_vOverrideMax;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Drone that hovers over a player in the warning zone (optional: just the loudspeaker without it). No collision or physics", "et")]
	ResourceName m_sDronePrefab;

	[Attribute("25", desc: "Drone height above the player (m)")]
	float m_fDroneHeight = 25;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Warning shot: a gunshot sound or bullet impact effect spawned a few metres from the player (optional: a message without it)", "et")]
	ResourceName m_sWarningShotPrefab;

	[Attribute("1", desc: "Sealed road checkpoints where roads reach the edge")]
	bool m_bCheckpoints = true;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Checkpoint barricade: a small composition of concrete blocks and a closed barrier. Checkpoints are off without it", "et")]
	ResourceName m_sBarricadePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Checkpoint guard: a soldier character without AI, or a prop. Stands outside the playable area and can't be hurt", "et")]
	ResourceName m_sGuardPrefab;

	[Attribute("2", UIWidgets.Slider, "Guards behind each barricade", "0 4 1")]
	int m_iGuardsPerCheckpoint = 2;

	[Attribute("6", UIWidgets.Slider, "Most checkpoints on one map", "1 8 1")]
	int m_iMaxCheckpoints = 6;

	[Attribute("60", desc: "Barricade distance (m) inside the edge")]
	float m_fCheckpointInset = 60;

	[Attribute("800", desc: "Minimum distance (m) between two checkpoints")]
	float m_fCheckpointSpacing = 800;

	[Attribute("150", desc: "Loudspeaker warning within this distance (m) of a checkpoint")]
	float m_fCheckpointWarnRadius = 150;

	[Attribute("60", desc: "Lethal within this distance (m) of a checkpoint (warning shot first)")]
	float m_fCheckpointKillRadius = 60;

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

	protected static const float LEAVE_MARGIN = 25;		// leave the zone by this much before the drone goes (no flicker)
	protected static const float WARN_REPEAT = 30;		// seconds before the loudspeaker repeats itself on re-entry
	protected static const float EFFECT_LIFETIME = 20;
	protected static const float CORNER_MARGIN = 500;	// missions keep away from the corners
	protected static const float GUARD_DEPTH = 20;		// guards stand this far outside the edge
	protected static const float MIN_SIZE = 1000;
	protected static const int MAX_CHECKPOINTS = 8;

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

		float margin = Math.Max(0, m_Settings.m_fSeaMargin);
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
	//! A random point on the edge, away from the corners, and the direction into the Zone.
	bool RandomEdgePoint(out vector edge, out vector inward)
	{
		if (!m_bActive || m_fMaxX - m_fMinX < 3 * CORNER_MARGIN || m_fMaxZ - m_fMinZ < 3 * CORNER_MARGIN)
			return false;

		float alongX = Math.RandomFloat(m_fMinX + CORNER_MARGIN, m_fMaxX - CORNER_MARGIN);
		float alongZ = Math.RandomFloat(m_fMinZ + CORNER_MARGIN, m_fMaxZ - CORNER_MARGIN);
		int side = Math.RandomInt(0, 4);
		switch (side)
		{
			case 0:
				edge = Vector(m_fMinX, 0, alongZ);
				inward = Vector(1, 0, 0);
				break;
			case 1:
				edge = Vector(m_fMaxX, 0, alongZ);
				inward = Vector(-1, 0, 0);
				break;
			case 2:
				edge = Vector(alongX, 0, m_fMinZ);
				inward = Vector(0, 0, 1);
				break;
			default:
				edge = Vector(alongX, 0, m_fMaxZ);
				inward = Vector(0, 0, -1);
				break;
		}

		edge = RZ_Util.GroundPos(edge);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The ground `depth` metres inside the edge from an edge point.
	vector PointInside(vector edge, vector inward, float depth)
	{
		return RZ_Util.GroundPos(edge + inward * depth);
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

		foreach (int i, IEntity p : players)
		{
			UpdatePlayer(ids[i], p, dt);
		}
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
				m_GM.NotifyPlayer(playerId, LOUDSPEAKER, GetWarningText(pos));
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
		m_GM.NotifyPlayer(playerId, LOUDSPEAKER, "A shot cracks past you and kicks up the ground.\n\"Final warning. Turn back now.\"\nGet away from the line: the next one won't miss.");
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
		if (drone)
			player.AddChild(drone, -1); // VERIFY: IEntity.AddChild(child, pivot) keeps the drone's world position; it then follows the player with no polling

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
			roads.GetRoadsInAABB(stripMin, stripMax, found); // VERIFY: RoadNetworkManager.GetRoadsInAABB(mins, maxs, roads)
			foreach (BaseRoad road : found)
			{
				if (!road)
					continue;

				array<vector> points = {};
				road.GetPoints(points); // VERIFY: BaseRoad.GetPoints() gives the road's centre line
				float width = road.GetWidth(); // VERIFY: BaseRoad.GetWidth(); main roads are wider

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
	protected static RoadNetworkManager GetRoadNetwork()
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld()); // VERIFY: GetGame().GetAIWorld() returns SCR_AIWorld
		if (!aiWorld)
			return null;

		return aiWorld.GetRoadNetworkManager(); // VERIFY: AIWorld.GetRoadNetworkManager()
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
