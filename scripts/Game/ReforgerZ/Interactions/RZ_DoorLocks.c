//! Real door locks (docs/DESIGN.md, "Locks and keys").
//!
//! - Which doors: the outside doors of the buildings RZ_Search already picks as locked
//!   places (seeded from the building's position, so the same ones after a restart). A door
//!   is a child entity of the building with a DoorComponent; it's an outside door when it
//!   stands within EDGE of the building's outline (its local bounding box). Interior doors are
//!   never locked. Doors are data, looked at only when a player comes near (the look-around,
//!   or the blockers' wider look) or tries one. The only thing spawned is a doorway blocker.
//! - The vanilla door action (RZ_DoorHooks.c) refuses to open a locked outside door from
//!   outside ("Locked"), on the client from a replicated list of locked buildings near
//!   players, and again on the server, which decides. From inside it always opens: that turns
//!   the latch, and the building is unlocked for everyone until the reset.
//! - At each locked building's outside doors, an interaction spot (the same invisible prop and
//!   routing as searching) offers Unlock (its key, quiet), Lock (its key) and Force the door
//!   (a crowbar, 10 s, loud). Forced locks are broken until the reset.
//! - Reset: an unlocked, latched or forced lock is back to locked after the relock time
//!   (Search > relock minutes), and its outside doors are pulled shut once nobody stands in
//!   them. Locked doors that are found open (AI opened them) are shut the same way.
//! - A building whose doors can't be found (no door children, no usable bounds) keeps the
//!   old lock on its loot. With Door locks off, every building does.
//! - Inside means anywhere in the building's box (0.5 m in from its sides), or past the door
//!   toward the middle: a shallow closet behind a door near the outline still opens.
//!
//! AI and the infected don't use the door action. Three layers keep them out: the AI's own
//! door opening is hooked (RZ_AIDoorHook.c, optional, asks AiMayOpenNear); a thin invisible
//! blocker stands in locked doorways near players and the navmesh is rebuilt around it
//! (RZ_DoorBlockers.c); and infected pressing at a locked doorway bang on it (heat).

//------------------------------------------------------------------------------------------------
//! Door and building geometry, on any machine (the client check and the server use the same).
class RZ_DoorGeometry
{
	static const float EDGE = 1.2;			// an outside door stands this close (m) to the building's outline
	static const float INSIDE_MARGIN = 0.3;	// inside means this far (m) past the door, toward the middle
	static const int CHILD_DEPTH = 3;
	static const float BOX_INSIDE = 0.5;	// in the building means at least this far (m) inside its box

	//------------------------------------------------------------------------------------------------
	static DoorComponent GetDoor(IEntity e)
	{
		if (!e)
			return null;

		return DoorComponent.Cast(e.FindComponent(DoorComponent));
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOpen(IEntity door)
	{
		DoorComponent dc = GetDoor(door);
		if (!dc)
			return false;

		return Math.AbsFloat(dc.GetControlValue()) > 0.05; // -1..1: 0 when shut, negative when it swings the other way
	}

	//------------------------------------------------------------------------------------------------
	//! Server only: shuts the door.
	static void Close(IEntity door)
	{
		DoorComponent dc = GetDoor(door);
		if (dc)
			dc.SetControlValue(0, RplId.Invalid()); // VERIFY: SetControlValue on the server shuts the door for every client
	}

	//------------------------------------------------------------------------------------------------
	//! The building a door belongs to: the nearest destructible building above it, else its top
	//! parent. null for a door that isn't part of a building.
	static IEntity BuildingOf(IEntity door)
	{
		if (!door)
			return null;

		IEntity top;
		IEntity p = door.GetParent(); // VERIFY: doors are child entities of the building prefab
		while (p)
		{
			if (SCR_DestructibleBuildingEntity.Cast(p))
				return p;

			top = p;
			p = p.GetParent();
		}

		return top;
	}

	//------------------------------------------------------------------------------------------------
	//! The same key RZ_Interactions gives the building's spots.
	static string BuildingKey(IEntity building)
	{
		return RZ_Interactions.PosKey(building.GetOrigin(), 4);
	}

	//------------------------------------------------------------------------------------------------
	//! Every door entity under e (children, a few levels down).
	static void FindDoors(IEntity e, int depth, notnull array<IEntity> result)
	{
		if (!e || depth > CHILD_DEPTH)
			return;

		IEntity child = e.GetChildren(); // VERIFY: a building's door entities are its children (GetChildren/GetSibling)
		while (child)
		{
			if (GetDoor(child))
				result.Insert(child);
			else
				FindDoors(child, depth + 1, result);

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The building's local bounding box. False if it's too small to tell inside from outside.
	static bool LocalBox(IEntity building, out vector mins, out vector maxs)
	{
		mins = vector.Zero;
		maxs = vector.Zero;
		if (!building)
			return false;

		building.GetBounds(mins, maxs); // the local box of the building's mesh
		return maxs[0] - mins[0] > 2 && maxs[2] - mins[2] > 2;
	}

	//------------------------------------------------------------------------------------------------
	//! World to the building's local space (rotation only, no scale).
	static vector ToLocal(IEntity building, vector world)
	{
		vector mat[4];
		building.GetWorldTransform(mat); // three axes, then the origin
		vector d = world - mat[3];
		vector ax = mat[0];
		vector ay = mat[1];
		vector az = mat[2];
		float x = d[0] * ax[0] + d[1] * ax[1] + d[2] * ax[2];
		float y = d[0] * ay[0] + d[1] * ay[1] + d[2] * ay[2];
		float z = d[0] * az[0] + d[1] * az[1] + d[2] * az[2];
		return Vector(x, y, z);
	}

	//------------------------------------------------------------------------------------------------
	static vector ToWorld(IEntity building, vector local)
	{
		vector mat[4];
		building.GetWorldTransform(mat);
		return mat[3] + mat[0] * local[0] + mat[1] * local[1] + mat[2] * local[2];
	}

	//------------------------------------------------------------------------------------------------
	//! 0 if the door is in a wall facing local x, 2 if facing local z (the nearer side of the box).
	protected static int WallAxis(vector d, vector mins, vector maxs)
	{
		float toX = Math.Min(d[0] - mins[0], maxs[0] - d[0]);
		float toZ = Math.Min(d[2] - mins[2], maxs[2] - d[2]);
		if (toX <= toZ)
			return 0;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	//! An outside door: on the building's outline. Never true without a usable box.
	static bool IsOuterDoor(IEntity building, IEntity door)
	{
		vector mins;
		vector maxs;
		if (!door || !LocalBox(building, mins, maxs))
			return false;

		vector d = ToLocal(building, door.GetOrigin());
		float toX = Math.Min(d[0] - mins[0], maxs[0] - d[0]);
		float toZ = Math.Min(d[2] - mins[2], maxs[2] - d[2]);
		if (toX < -EDGE || toZ < -EDGE)
			return false;

		return Math.Min(toX, toZ) <= EDGE;
	}

	//------------------------------------------------------------------------------------------------
	//! Anywhere inside the building's box, at least BOX_INSIDE in from its sides. Catches a
	//! shallow closet behind an interior door that sits near the outline (which counts as an
	//! outside door). False without a usable box.
	static bool InBuilding(IEntity building, vector worldPos)
	{
		vector mins;
		vector maxs;
		if (!LocalBox(building, mins, maxs))
			return false;

		vector p = ToLocal(building, worldPos);
		if (p[1] < mins[1] - 1 || p[1] > maxs[1])
			return false;

		if (p[0] < mins[0] + BOX_INSIDE || p[0] > maxs[0] - BOX_INSIDE)
			return false;

		return p[2] >= mins[2] + BOX_INSIDE && p[2] <= maxs[2] - BOX_INSIDE;
	}

	//------------------------------------------------------------------------------------------------
	//! Inside: anywhere in the building's box (InBuilding), or past the door toward the middle
	//! of the building and within its width. When in doubt this says inside, so nobody is shut in.
	static bool IsInside(IEntity building, IEntity door, vector worldPos)
	{
		vector mins;
		vector maxs;
		if (!door || !LocalBox(building, mins, maxs))
			return true;

		if (InBuilding(building, worldPos))
			return true;

		vector d = ToLocal(building, door.GetOrigin());
		vector p = ToLocal(building, worldPos);
		int axis = WallAxis(d, mins, maxs);
		int other = 2;
		if (axis == 2)
			other = 0;

		float mid = (mins[axis] + maxs[axis]) * 0.5;
		float midOther = (mins[other] + maxs[other]) * 0.5;
		float halfOther = (maxs[other] - mins[other]) * 0.5;

		// Round the corner, beyond the building's width: outside
		if (Math.AbsFloat(p[other] - midOther) > halfOther)
			return false;

		return Math.AbsFloat(p[axis] - mid) < Math.AbsFloat(d[axis] - mid) - INSIDE_MARGIN;
	}

	//------------------------------------------------------------------------------------------------
	//! A point `dist` m outside the door, straight out from its wall (where its spot stands).
	static vector OutsidePoint(IEntity building, IEntity door, float dist)
	{
		vector mins;
		vector maxs;
		if (!LocalBox(building, mins, maxs))
			return door.GetOrigin();

		vector d = ToLocal(building, door.GetOrigin());
		int axis = WallAxis(d, mins, maxs);
		float mid = (mins[axis] + maxs[axis]) * 0.5;
		float sign = 1;
		if (d[axis] < mid)
			sign = -1;

		vector outside = Vector(d[0] + sign * dist, d[1], d[2]);
		if (axis == 2)
			outside = Vector(d[0], d[1], d[2] + sign * dist);

		return ToWorld(building, outside);
	}

	//------------------------------------------------------------------------------------------------
	//! Straight out of the door's wall, in world space (flat, length 1). Zero without a box.
	static vector WallNormal(IEntity building, IEntity door)
	{
		vector mins;
		vector maxs;
		if (!door || !LocalBox(building, mins, maxs))
			return vector.Zero;

		vector d = ToLocal(building, door.GetOrigin());
		int axis = WallAxis(d, mins, maxs);
		float mid = (mins[axis] + maxs[axis]) * 0.5;
		float sign = 1;
		if (d[axis] < mid)
			sign = -1;

		vector mat[4];
		building.GetWorldTransform(mat);
		vector n = mat[axis] * sign;
		n[1] = 0;
		float len = n.Length();
		if (len < 0.01)
			return vector.Zero;

		return n * (1 / len);
	}

	//------------------------------------------------------------------------------------------------
	//! The middle of a shut door leaf (the doorway), from the door's own box. The door's
	//! origin (often the hinge) if it has no usable box.
	static vector DoorwayCentre(IEntity door)
	{
		vector mins;
		vector maxs;
		door.GetBounds(mins, maxs); // as for buildings: the door leaf's local box
		if (maxs[0] - mins[0] < 0.2 && maxs[2] - mins[2] < 0.2)
			return door.GetOrigin();

		vector mat[4];
		door.GetWorldTransform(mat);
		vector c = (mins + maxs) * 0.5;
		return mat[3] + mat[0] * c[0] + mat[1] * c[1] + mat[2] * c[2];
	}

	//------------------------------------------------------------------------------------------------
	//! The client side of the vanilla door action: locked for this user right now? Uses the
	//! game mode's replicated list of locked buildings near players. The server decides again.
	static bool LooksLocked(IEntity door, IEntity user)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !door || !user)
			return false;

		string list = gm.GetLockedBuildings();
		if (list.IsEmpty() || IsOpen(door))
			return false;

		IEntity building = BuildingOf(door);
		if (!building)
			return false;

		if (!list.Contains(";" + BuildingKey(building) + ";"))
			return false;

		if (!IsOuterDoor(building, door))
			return false;

		return !IsInside(building, door, user.GetOrigin());
	}
}

//------------------------------------------------------------------------------------------------
//! One locked place's outside doors, learned when a player first comes near or tries one.
class RZ_BuildingDoors
{
	string m_sKey;
	vector m_vPos;
	string m_sType;
	IEntity m_Building;
	ref array<IEntity> m_aDoors = {};	// outside doors only
}

//------------------------------------------------------------------------------------------------
//! Server: the door side of locks. The lock state itself is RZ_Search's (one per building).
class RZ_DoorLocks
{
	protected static const float LIST_RADIUS = 80;		// locked buildings this close to a player are replicated
	protected static const int LIST_MAX = 120;
	protected static const float FORGET_RADIUS = 300;	// learned doors further than this from everyone are dropped
	protected static const float HOLD_CLEAR = 2.5;		// a locked door is pulled shut once nobody is this close
	protected static const int SPOTS_PER_BUILDING = 3;
	protected static const float NOTICE_SECONDS = 4;
	protected static const float KEY_TURN_SECONDS = 5;	// one Lock or Unlock per building this often

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_Search m_Search;
	protected RZ_SearchSettings m_Settings;
	protected ref map<string, ref RZ_BuildingDoors> m_mBuildings = new map<string, ref RZ_BuildingDoors>();
	protected ref map<int, float> m_mNoticeAt = new map<int, float>();
	protected ref map<string, float> m_mKeyTurnedAt = new map<string, float>();	// lock and unlock spam guard
	protected ref RZ_DoorBlockers m_Blockers;
	protected string m_sList;
	protected ref array<string> m_aReinforced = {};

	protected static const float AI_DOOR_REACH = 3;	// the AI hook looks for a locked door this close (m) to the AI
	protected static const float AI_BUILDING_REACH = 40;

	//------------------------------------------------------------------------------------------------
	void RZ_DoorLocks(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_Search search, RZ_SearchSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Search = search;
		m_Settings = settings;
		m_Blockers = new RZ_DoorBlockers(gm, hub, this, settings);
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_Settings.m_bEnabled && m_Settings.m_bLocks && m_Settings.m_bDoorLocks;
	}

	//------------------------------------------------------------------------------------------------
	//! A doorway blocker stands within radius of pos (RZ_LockedSpots).
	bool IsNearBlocker(vector pos, float radius)
	{
		return m_Blockers.IsNear(pos, radius);
	}

	//------------------------------------------------------------------------------------------------
	//! A mission changed this building's lock for a keypad (Locked Bunker): no key fits it and a
	//! crowbar won't force it while it's set. The inside latch still opens it.
	void SetReinforced(string buildingKey, bool on)
	{
		if (on && !m_aReinforced.Contains(buildingKey))
			m_aReinforced.Insert(buildingKey);
		else if (!on)
			m_aReinforced.RemoveItem(buildingKey);
	}

	//------------------------------------------------------------------------------------------------
	bool IsReinforced(string buildingKey)
	{
		return m_aReinforced.Contains(buildingKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Learn a building's doors for a mission (Locked Bunker): its outside doors if it has a lock,
	//! null otherwise. Same as the look-around.
	RZ_BuildingDoors LearnForMission(IEntity building)
	{
		if (!building)
			return null;

		return LearnIfLocked(building, RZ_ItemHelper.LowerName(building));
	}

	//------------------------------------------------------------------------------------------------
	//! This building's lock is on its doors (door locks on, outside doors found), not its loot.
	bool UsesDoors(string buildingKey)
	{
		if (!IsOn())
			return false;

		RZ_BuildingDoors b;
		return m_mBuildings.Find(buildingKey, b) && !b.m_aDoors.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_BuildingDoors Learn(IEntity building, string key, string type)
	{
		RZ_BuildingDoors b;
		if (m_mBuildings.Find(key, b))
		{
			if (!b.m_Building)
				b.m_Building = building;

			return b;
		}

		b = new RZ_BuildingDoors();
		b.m_sKey = key;
		b.m_vPos = building.GetOrigin();
		b.m_sType = type;
		b.m_Building = building;

		array<IEntity> all = {};
		RZ_DoorGeometry.FindDoors(building, 0, all);
		foreach (IEntity door : all)
		{
			if (RZ_DoorGeometry.IsOuterDoor(building, door))
				b.m_aDoors.Insert(door);
		}

		m_mBuildings.Set(key, b);
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! A building with a lock (seeded, not burned): its outside doors, learned now if they
	//! weren't. null for anything else. From the look-around and the blockers' wider look.
	RZ_BuildingDoors LearnIfLocked(IEntity building, string lowerName)
	{
		if (!IsOn() || !building || RZ_DoorGeometry.GetDoor(building))
			return null;

		vector pos = building.GetOrigin();
		string type = m_Search.BuildingType(lowerName);
		if (!m_Search.HasLock(pos, type) || m_Hub.GetTraps().IsBurned(pos))
			return null;

		return Learn(building, RZ_Interactions.PosKey(pos, 4), type);
	}

	//------------------------------------------------------------------------------------------------
	//! Locked right now (not unlocked, latched or forced since the reset, not burned).
	bool IsLockedNow(RZ_BuildingDoors b)
	{
		if (!b || m_Hub.GetTraps().IsBurned(b.m_vPos))
			return false;

		return m_Search.IsBuildingLocked(b.m_vPos, b.m_sKey, b.m_sType);
	}

	//------------------------------------------------------------------------------------------------
	//! Every learned locked place (for RZ_DoorBlockers; don't change it from outside).
	map<string, ref RZ_BuildingDoors> GetBuildings()
	{
		return m_mBuildings;
	}

	//------------------------------------------------------------------------------------------------
	//! From the look-around: a building near a player. For a locked place with outside doors,
	//! a spot at each door (at most `room`). Returns how many spots were added.
	int OnBuildingSeen(IEntity building, string lowerName, int room)
	{
		RZ_BuildingDoors b = LearnIfLocked(building, lowerName);
		if (!b)
			return 0;

		int added = 0;
		foreach (int i, IEntity door : b.m_aDoors)
		{
			if (added >= room || added >= SPOTS_PER_BUILDING)
				break;

			if (!door)
				continue;

			vector at = RZ_DoorGeometry.OutsidePoint(b.m_Building, door, 0.6);
			if (m_Hub.AddDoorSpot(at, b.m_vPos, b.m_sKey, b.m_sType, DoorNoun(b.m_vPos, b.m_sType), i))
				added++;
		}

		return added;
	}

	//------------------------------------------------------------------------------------------------
	//! "the blue door", "the pharmacy door".
	string DoorNoun(vector buildingPos, string type)
	{
		string colour = m_Search.DoorColour(buildingPos, type);
		if (!colour.IsEmpty())
			return "the " + ColourWord(colour) + " door";

		switch (type)
		{
			case "pharmacy": return "the pharmacy door";
			case "garage": return "the garage door";
			case "police": return "the station door";
			case "shop": return "the shop door";
		}

		return "the door";
	}

	//------------------------------------------------------------------------------------------------
	protected static string ColourWord(string colour)
	{
		switch (colour)
		{
			case "Blue": return "blue";
			case "Green": return "green";
			case "Red": return "red";
			case "White": return "white";
			case "Yellow": return "yellow";
		}

		return "front";
	}

	// ================================================================ The vanilla door action
	//------------------------------------------------------------------------------------------------
	//! From the modded door action, on the server. False refuses (the door stays shut).
	static bool ServerAllowsOpen(IEntity door, IEntity user)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !hub.GetDoors())
			return true;

		return hub.GetDoors().AllowOpen(door, user);
	}

	//------------------------------------------------------------------------------------------------
	bool AllowOpen(IEntity door, IEntity user)
	{
		if (!IsOn() || !door || !user)
			return true;

		// Shutting a door is always fine
		if (RZ_DoorGeometry.IsOpen(door))
			return true;

		IEntity building = RZ_DoorGeometry.BuildingOf(door);
		if (!building)
			return true;

		vector pos = building.GetOrigin();
		string type = m_Search.BuildingType(RZ_ItemHelper.LowerName(building));
		if (!m_Search.HasLock(pos, type) || m_Hub.GetTraps().IsBurned(pos))
			return true;

		string key = RZ_Interactions.PosKey(pos, 4);
		RZ_BuildingDoors b = Learn(building, key, type);
		if (b.m_aDoors.Find(door) < 0)
			return true;

		if (!m_Search.IsBuildingLocked(pos, key, type))
			return true;

		int playerId = RZ_PuzzleActionHelper.GetPlayerId(user);

		// Inside always opens: the latch, and it's unlocked for everyone until the reset
		if (RZ_DoorGeometry.IsInside(building, door, user.GetOrigin()))
		{
			m_Search.OpenBuilding(key, false);
			Notice(playerId, "Unlocked", "You turn the latch from the inside. It's open now, for anyone who comes.");
			return true;
		}

		Notice(playerId, "Locked", string.Format("It won't budge. A key tagged \"%1\" would open it, or a crowbar and a lot of noise.", m_Search.KeyTagForBuilding(pos, type)));
		return false;
	}

	// ================================================================ AI at the door
	//------------------------------------------------------------------------------------------------
	//! From the optional AI door hook (RZ_AIDoorHook.c), on the server: may this AI open the
	//! door it's at? False only for a locked place's outside door, from outside. Anything not
	//! learned (no player within the blockers' range) is allowed.
	static bool AiMayOpenNear(IEntity ai)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !hub.GetDoors() || !ai)
			return true;

		return hub.GetDoors().AiAllowed(ai.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	bool AiAllowed(vector pos)
	{
		if (!IsOn() || !m_Settings.m_bAiDoorHook)
			return true;

		foreach (string key, RZ_BuildingDoors b : m_mBuildings)
		{
			if (!b.m_Building || b.m_aDoors.IsEmpty() || RZ_Util.DistXZ(b.m_vPos, pos) > AI_BUILDING_REACH)
				continue;

			if (!IsLockedNow(b))
				continue;

			foreach (IEntity door : b.m_aDoors)
			{
				if (!door || RZ_Util.DistXZ(door.GetOrigin(), pos) > AI_DOOR_REACH)
					continue;

				// Inside lets them out (without turning the latch for anyone else)
				return RZ_DoorGeometry.IsInside(b.m_Building, door, pos);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! At most one message every few seconds per player (people rattle locked doors).
	protected void Notice(int playerId, string title, string text)
	{
		if (playerId <= 0)
			return;

		float now = m_Hub.GetClock();
		float last;
		if (m_mNoticeAt.Find(playerId, last) && now - last < NOTICE_SECONDS && now >= last)
			return;

		m_mNoticeAt.Set(playerId, now);
		m_GM.NotifyPlayer(playerId, title, text);
	}

	// ================================================================ The spot at the door
	//------------------------------------------------------------------------------------------------
	//! 0 Unlock, 1 Lock, 2 Force the door. Slots that don't apply are hidden ("|" labels them all).
	void Labels(RZ_Spot s, out string read, out string choiceLabels)
	{
		read = "Look at " + s.m_sNoun;
		if (m_Search.IsLocked(s))
		{
			choiceLabels = "Unlock (key)||Force the door (crowbar)";
			return;
		}

		if (m_Search.IsForced(s.m_sBuildingKey))
		{
			read = "Look at the broken lock";
			choiceLabels = "|";
			return;
		}

		choiceLabels = "|Lock (key)|";
	}

	//------------------------------------------------------------------------------------------------
	void Describe(int playerId, RZ_Spot s)
	{
		string title = m_Hub.Title(s);
		if (m_Hub.GetTraps().IsBurned(s.m_vBuilding))
		{
			m_GM.NotifyPlayer(playerId, title, "Charred and hanging off one hinge. There's nothing left to lock.");
			return;
		}

		if (m_Search.IsLocked(s))
		{
			string text = string.Format("Locked, and solid. Its key would have a tag saying \"%1\". Or a crowbar: about %2 seconds, and everything for a mile hears it.",
				m_Search.KeyTagForBuilding(s.m_vBuilding, s.m_sType), RZ_Util.RoundToInt(m_Settings.m_fForceSeconds));
			if (m_Search.HasKey(playerId, m_Search.KeyForBuilding(s.m_vBuilding, s.m_sType)))
				text += " One of your keys should fit.";

			m_GM.NotifyPlayer(playerId, title, text);
			return;
		}

		if (m_Search.IsForced(s.m_sBuildingKey))
		{
			m_GM.NotifyPlayer(playerId, title, "Someone forced this. The frame's split and the lock hangs loose. It won't lock again for hours.");
			return;
		}

		m_GM.NotifyPlayer(playerId, title, "Unlocked. Whoever has its key could lock it again.");
	}

	//------------------------------------------------------------------------------------------------
	void HandleChoice(int playerId, RZ_Spot s, int option, IEntity user)
	{
		string title = m_Hub.Title(s);
		if (m_Hub.GetTraps().IsBurned(s.m_vBuilding))
		{
			m_GM.NotifyPlayer(playerId, title, "It's burned out. There's nothing left to lock.");
			return;
		}

		bool locked = m_Search.IsLocked(s);
		if (locked && (option == 0 || option == 2) && IsReinforced(s.m_sBuildingKey))
		{
			m_GM.NotifyPlayer(playerId, title, "Someone's changed this lock. A steel plate over the old keyhole, bolted through, and a keypad beside the frame. No key or crowbar will touch it. The code's out there somewhere.");
			return;
		}

		string keyId = m_Search.KeyForBuilding(s.m_vBuilding, s.m_sType);
		string tag = m_Search.KeyTagForBuilding(s.m_vBuilding, s.m_sType);

		// Locking and unlocking resend the locked list and move blockers: not every second
		float turned;
		if ((option == 0 || option == 1) && m_mKeyTurnedAt.Find(s.m_sBuildingKey, turned) && m_Hub.GetClock() - turned < KEY_TURN_SECONDS && m_Hub.GetClock() >= turned)
		{
			m_GM.NotifyPlayer(playerId, title, "The lock's stiff. Give it a moment.");
			return;
		}

		if (option == 0)
		{
			if (!locked)
				m_GM.NotifyPlayer(playerId, title, "It isn't locked.");
			else if (!m_Search.HasKey(playerId, keyId))
				m_GM.NotifyPlayer(playerId, title, string.Format("None of your keys fit. The one you want has a tag saying \"%1\".", tag));
			else
			{
				KeyTurned(s.m_sBuildingKey);
				m_Search.OpenBuilding(s.m_sBuildingKey, false);
				m_GM.NotifyPlayer(playerId, title, "The key turns, quietly. It's open.");
			}

			return;
		}

		if (option == 1)
		{
			if (locked)
				m_GM.NotifyPlayer(playerId, title, "It's already locked.");
			else if (m_Search.IsForced(s.m_sBuildingKey))
				m_GM.NotifyPlayer(playerId, title, "The lock's broken. It won't lock again until someone's fixed it, in a few hours.");
			else if (!m_Search.HasKey(playerId, keyId))
				m_GM.NotifyPlayer(playerId, title, string.Format("You'd need its key: a tag saying \"%1\".", tag));
			else
			{
				KeyTurned(s.m_sBuildingKey);
				m_Search.LockBuilding(s.m_sBuildingKey);
				m_GM.NotifyPlayer(playerId, title, "You turn the key. Locked. The door pulls shut once you step away from it.");
			}

			return;
		}

		if (option != 2)
			return;

		if (!locked)
		{
			m_GM.NotifyPlayer(playerId, title, "It's open.");
			return;
		}

		if (!RZ_ItemHelper.Has(user, m_Settings.m_sCrowbarWords))
		{
			m_GM.NotifyPlayer(playerId, title, "You'd need a crowbar for that door. Garages and sheds have them.");
			return;
		}

		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_FORCE, s.m_iId, 0, m_Settings.m_fForceSeconds, "the door", false);
		m_GM.GetHeat().AddHeat(s.m_vPos, m_Settings.m_fForceHeat * 0.4);
		m_GM.NotifyPlayer(playerId, title, string.Format("You jam the crowbar into the frame and lean on it. It's loud. About %1 seconds; stay close.", RZ_Util.RoundToInt(m_Settings.m_fForceSeconds)));
	}

	//------------------------------------------------------------------------------------------------
	protected void KeyTurned(string buildingKey)
	{
		if (m_mKeyTurnedAt.Count() > 200)
			m_mKeyTurnedAt.Clear();

		m_mKeyTurnedAt.Set(buildingKey, m_Hub.GetClock());
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! A lock changed: an unlocked building's blockers go at once, and the replicated list is
	//! sent now, not on the next tick.
	void OnLockChanged(string buildingKey)
	{
		m_Blockers.OnLockChanged(buildingKey);
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		Tick(players);
	}

	//------------------------------------------------------------------------------------------------
	//! On the 5 s tick: shut locked doors nobody is standing in, and replicate which buildings
	//! near players are locked (for the client side of the door action).
	void Tick(array<IEntity> players)
	{
		if (!IsOn())
		{
			m_Blockers.ClearAll();
			if (!m_sList.IsEmpty())
			{
				m_sList = "";
				m_GM.SetLockedBuildings("");
			}

			return;
		}

		string list = "";
		int listed = 0;
		array<string> forget = {};
		foreach (string key, RZ_BuildingDoors b : m_mBuildings)
		{
			float near = RZ_Util.NearestDistance(b.m_vPos, players);
			if (near > FORGET_RADIUS)
			{
				forget.Insert(key);
				continue;
			}

			if (b.m_aDoors.IsEmpty() || m_Hub.GetTraps().IsBurned(b.m_vPos))
				continue;

			if (!m_Search.IsBuildingLocked(b.m_vPos, b.m_sKey, b.m_sType))
				continue;

			HoldShut(b, players);
			if (near <= LIST_RADIUS && listed < LIST_MAX)
			{
				list += key + ";";
				listed++;
			}
		}

		foreach (string k : forget)
		{
			m_mBuildings.Remove(k);
		}

		if (!list.IsEmpty())
			list = ";" + list;

		if (list != m_sList)
		{
			m_sList = list;
			m_GM.SetLockedBuildings(list);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! On the 5 s tick, after Tick: blockers in locked doorways near players, the navmesh
	//! around them, and infected banging on locked doors (RZ_DoorBlockers.c).
	void TickBlockers(array<int> ids, array<IEntity> players)
	{
		if (IsOn())
			m_Blockers.Tick(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A locked building's open outside doors (after a reset or Lock, or opened by AI) are
	//! pulled shut once nobody stands in them.
	protected void HoldShut(RZ_BuildingDoors b, array<IEntity> players)
	{
		foreach (IEntity door : b.m_aDoors)
		{
			if (!door || !RZ_DoorGeometry.IsOpen(door))
				continue;

			if (RZ_Util.NearestDistance(door.GetOrigin(), players) > HOLD_CLEAR)
				RZ_DoorGeometry.Close(door);
		}
	}
}
