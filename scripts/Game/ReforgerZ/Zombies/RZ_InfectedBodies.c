//! The plumbing between the infected as data and their bodies (docs/INFECTED.md, "Bodies"): what a
//! tracked unit is for (its role), the data part it writes back to, the sight rule for spawning,
//! the agent budget shared by player clusters, runtime groups and the silent kill. Server only.
//! Used by the spawner (RZ_ZombieSpawner) while Zombies > Crawlers or Zombies > Tribes is on; with
//! both off nothing here runs.

//------------------------------------------------------------------------------------------------
//! What a tracked unit is for. NONE: the spawner as before (missions, specials, the old roamers).
enum RZ_EInfectedRole
{
	NONE,
	CRAWLER,	// a town's own, slow infected
	LOOSE,		// a loose one walking somewhere in the countryside
	SCAVENGER,	// a town's crawlers out at a kill site (a data part)
	ALPHA,		// a tribe's (Zombies/RZ_Tribes.c); stealth reads the roles
	GUARD,
	HUNTER,
	SCOUT,
	YOUNG,
	DAY_PARTY
}

//------------------------------------------------------------------------------------------------
//! How a silent kill leaves them (RZ_ZombieSpawner.KillSilently).
enum RZ_ESilentKill
{
	DELETE,			// gone without a trace: no body, no loose item (out of sight only)
	KILL_AND_MARK	// killed where they stand: the body stays for a player to find
}

//------------------------------------------------------------------------------------------------
//! A group of infected as data: a count at a position, going somewhere. Its owner (the crawlers'
//! scavengers now, tribes later) keeps it; while it has bodies, the tracked unit writes its
//! position and its dead back to it every tick, so nothing snaps back when the bodies go.
class RZ_InfectedPart
{
	int m_iId;
	int m_iTribe = -1;
	int m_iTown = -1;				// whose crawlers these are (scavengers), -1 none
	RZ_EInfectedRole m_eRole;
	int m_iCount;					// members, as data
	vector m_vPos;					// where they are, as data
	vector m_vTarget;
	bool m_bHasTarget;
	float m_fUntil;					// the owner's clock: when it ends
	RZ_TrackedZombie m_Body;		// weak: the tracked unit while it has bodies, null as data
	int m_iKilled;					// members killed (not silently) since it was made
	string m_sNote;					// for the admin dump
	float m_fSpeed;					// m/s as data; 0 = the data speed (a day party's hop is slower)

	//------------------------------------------------------------------------------------------------
	bool HasBody()
	{
		return m_Body != null && m_Body.m_Entity != null;
	}

	//------------------------------------------------------------------------------------------------
	//! As data it walks toward its target at the data speed (3.5 m/s, phase 0: Bacon runs every
	//! member to an order, a group's centre covers about 3.5).
	void MoveAsData(float dt)
	{
		if (!m_bHasTarget || HasBody())
			return;

		float d = RZ_Util.DistXZ(m_vPos, m_vTarget);
		float speed = RZ_InfectedBodies.DATA_SPEED;
		if (m_fSpeed > 0)
			speed = m_fSpeed;

		float step = speed * dt;
		if (d <= step || d < 1)
		{
			m_vPos = m_vTarget;
			return;
		}

		vector dir = m_vTarget - m_vPos;
		dir[1] = 0;
		dir.Normalize();
		m_vPos = RZ_Util.GroundPos(m_vPos + dir * step);
	}

	//------------------------------------------------------------------------------------------------
	string Describe()
	{
		string where = RZ_SettlementScanner.GridReference(m_vPos);
		string body = "data";
		if (HasBody())
			body = "bodies";

		return string.Format("part %1 %2 x%3 at %4 (%5) killed %6 %7", m_iId, typename.EnumToString(RZ_EInfectedRole, m_eRole), m_iCount, where, body, m_iKilled, m_sNote);
	}
}

//------------------------------------------------------------------------------------------------
//! Sight for spawning (docs/INFECTED.md, "Bodies"). Phase 0 measured that an infected spawned in a
//! player's sight within reach charges at once from 300 m, and that a hill blocks sight completely.
//! So no body spawns within 300 m of a player who could see the spot: terrain alone (cheap, a few
//! dozen height samples), and, for towns, buildings and trees too (a physics trace, capped a tick).
class RZ_InfectedBodies
{
	static const float DATA_SPEED = 3.5;		// m/s a part moves as data
	static const float P = 250;					// Bacon's perception range in the open (phase 0)
	static const float SIGHT_SPAWN = 300;		// no spawn within this in line of sight
	static const float EYE = 1.7;
	static const int TRACES_PER_TICK = 16;		// physics traces for spawning, per server tick

	protected static int s_iTracesLeft = TRACES_PER_TICK;
	protected static int s_iNextPartId = 1;
	protected static int s_iNextMemberId = 1;

	//------------------------------------------------------------------------------------------------
	//! Each server tick: the trace budget refills.
	static void NewTick()
	{
		s_iTracesLeft = TRACES_PER_TICK;
	}

	//------------------------------------------------------------------------------------------------
	static int NewPartId()
	{
		s_iNextPartId++;
		return s_iNextPartId;
	}

	//------------------------------------------------------------------------------------------------
	static int NewMemberId()
	{
		s_iNextMemberId++;
		return s_iNextMemberId;
	}

	//------------------------------------------------------------------------------------------------
	//! The ground between the two points rises above the line between eyes at both ends.
	static bool TerrainBlocks(vector a, vector b)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		float ya = world.GetSurfaceY(a[0], a[2]) + EYE;
		float yb = world.GetSurfaceY(b[0], b[2]) + EYE;
		float dist = RZ_Util.DistXZ(a, b);
		int steps = dist / 10;
		steps = Math.ClampInt(steps, 2, 40);
		for (int i = 1; i < steps; i++)
		{
			float t = i;
			t = t / steps;
			float x = a[0] + (b[0] - a[0]) * t;
			float z = a[2] + (b[2] - a[2]) * t;
			float line = ya + (yb - ya) * t;
			if (world.GetSurfaceY(x, z) > line + 0.3)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Buildings, walls or trees between the player's eyes and the spot. False when the tick's
	//! traces are used up (then the spot counts as seen).
	static bool ObjectsBlock(IEntity viewer, vector b)
	{
		if (s_iTracesLeft <= 0 || !viewer)
			return false;

		s_iTracesLeft--;
		TraceParam param = new TraceParam();
		param.Start = viewer.GetOrigin() + Vector(0, EYE, 0);
		param.End = RZ_Util.GroundPos(b) + Vector(0, EYE, 0);
		param.Exclude = viewer;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		float hit = GetGame().GetWorld().TraceMove(param, null); // VERIFY: a physics trace from a player's eyes is stopped by buildings and trees, and 16 of them a tick cost little (the design's check 12)
		return hit < 0.98;
	}

	//------------------------------------------------------------------------------------------------
	//! Some player within 300 m could see pos (terrain, and with objects also buildings and trees).
	static bool InSight(vector pos, array<IEntity> players, bool objects)
	{
		foreach (IEntity p : players)
		{
			if (!p || RZ_Util.DistXZ(p.GetOrigin(), pos) >= SIGHT_SPAWN)
				continue;

			if (TerrainBlocks(p.GetOrigin(), pos))
				continue;

			if (objects && ObjectsBlock(p, pos))
				continue;

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Out of every player's sight and beyond P: a body may go back to data unseen.
	static bool Unseen(vector pos, array<IEntity> players)
	{
		foreach (IEntity p : players)
		{
			if (!p)
				continue;

			float d = RZ_Util.DistXZ(p.GetOrigin(), pos);
			if (d < P)
				return false;

			if (d < SIGHT_SPAWN && !TerrainBlocks(p.GetOrigin(), pos))
				return false;
		}

		return true;
	}

	// ================================================================ Runtime groups
	//------------------------------------------------------------------------------------------------
	//! A character on its own takes no waypoints and Bacon's stays inactive (phase 0): put it in
	//! a runtime group, an empty group prefab filled with AddAIEntityToGroup (phase 0 passed it with
	//! Bacon 1.8.0). Returns the group, or null (no or a bad empty group prefab: the character is
	//! left as it is).
	static SCR_AIGroup WrapInGroup(IEntity character, ResourceName emptyGroup)
	{
		if (!character || emptyGroup.IsEmpty() || !ChimeraCharacter.Cast(character))
			return null;

		IEntity spawned = RZ_Util.SpawnPrefab(emptyGroup, character.GetOrigin());
		SCR_AIGroup group = SCR_AIGroup.Cast(spawned);
		if (!group)
		{
			if (spawned)
			{
				RZ_Log.Warn("Zombies > Empty group isn't an AI group: " + emptyGroup);
				SCR_EntityHelper.DeleteEntityAndChildren(spawned);
			}

			return null;
		}

		if (!group.AddAIEntityToGroup(character))
		{
			SCR_EntityHelper.DeleteEntityAndChildren(group);
			return null;
		}

		return group;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds one more character to a runtime group. False if it couldn't join (it's deleted).
	static bool AddToGroup(SCR_AIGroup group, IEntity character)
	{
		if (!group || !character)
			return false;

		if (group.AddAIEntityToGroup(character))
			return true;

		SCR_EntityHelper.DeleteEntityAndChildren(character);
		return false;
	}
}

//------------------------------------------------------------------------------------------------
//! The agent budget (docs/INFECTED.md, "Bodies"): living infected counted in agents (group members),
//! not tracked entries. Players within 600 m of each other are one cluster; each cluster gets a
//! share, the smaller of Cluster share and the server's budget divided among the clusters, so a
//! busy night shrinks every presence a little instead of starving one. Worked out once a tick.
class RZ_InfectedBudget
{
	static const float CLUSTER_RADIUS = 600;

	protected ref array<int> m_aClusterOf = {};		// per player: its cluster
	protected ref array<int> m_aUsed = {};			// per cluster: agents near it
	protected ref array<IEntity> m_aPlayers = {};
	protected int m_iClusters;
	protected int m_iTotal;
	protected int m_iBudget = 100;
	protected int m_iShare = 28;

	//------------------------------------------------------------------------------------------------
	//! Recount: players into clusters, and every living tracked unit (missions too: they count
	//! toward the caps but are never trimmed) onto its nearest player's cluster.
	void Begin(array<IEntity> players, array<ref RZ_TrackedZombie> tracked, int budget, int share)
	{
		m_aPlayers.Clear();
		m_aClusterOf.Clear();
		m_aUsed.Clear();
		m_iClusters = 0;
		m_iTotal = 0;
		m_iBudget = Math.Max(1, budget);

		foreach (int i, IEntity p : players)
		{
			m_aPlayers.Insert(p);
			int joined = -1;
			for (int j = 0; j < i; j++)
			{
				if (RZ_Util.DistXZ(players[j].GetOrigin(), p.GetOrigin()) <= CLUSTER_RADIUS)
				{
					joined = m_aClusterOf[j];
					break;
				}
			}

			if (joined < 0)
			{
				joined = m_iClusters;
				m_iClusters++;
				m_aUsed.Insert(0);
			}

			m_aClusterOf.Insert(joined);
		}

		m_iShare = Math.Max(1, share);
		if (m_iClusters > 0)
			m_iShare = Math.Max(1, Math.Min(share, m_iBudget / m_iClusters));

		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Entity)
				continue;

			Add(z.m_vLastPos, z.Agents());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The cluster of the player nearest pos, -1 with nobody online.
	int ClusterOf(vector pos)
	{
		int best = -1;
		float bestDist = 1000000000;
		foreach (int i, IEntity p : m_aPlayers)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = m_aClusterOf[i];
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Agents that may still spawn near pos: what's left of its cluster's share and of the server's.
	int RoomAt(vector pos)
	{
		int c = ClusterOf(pos);
		int left = m_iBudget - m_iTotal;
		if (c >= 0)
			left = Math.Min(left, m_iShare - m_aUsed[c]);

		return Math.Max(0, left);
	}

	//------------------------------------------------------------------------------------------------
	void Add(vector pos, int agents)
	{
		m_iTotal += agents;
		int c = ClusterOf(pos);
		if (c >= 0)
			m_aUsed[c] = m_aUsed[c] + agents;
	}

	int GetTotal() { return m_iTotal; }
	int GetBudget() { return m_iBudget; }
	int GetShare() { return m_iShare; }
	int GetClusterCount() { return m_iClusters; }

	//------------------------------------------------------------------------------------------------
	//! "28/28, 12/28" for the status line.
	string DescribeClusters()
	{
		string text = "";
		foreach (int used : m_aUsed)
		{
			if (!text.IsEmpty())
				text += ", ";

			text += string.Format("%1/%2", used, m_iShare);
		}

		if (text.IsEmpty())
			return "none";

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! One body in the body budget's ledger.
class RZ_BodyEntry
{
	string m_sKind;
	IEntity m_Entity;		// weak: gone when the body is deleted
	int m_iAgents = -1;		// living members once read, -1 until then
	vector m_vPos;

	//------------------------------------------------------------------------------------------------
	//! Agents for the budget: members once read, else 4 for a group and 1 for a character.
	int Agents()
	{
		if (m_iAgents >= 0)
			return m_iAgents;

		if (SCR_AIGroup.Cast(m_Entity))
			return 4;

		return 1;
	}
}

//------------------------------------------------------------------------------------------------
//! The one body budget (docs/INFECTED.md, "The one body budget"): every body that isn't an infected
//! and comes from the shared pieces (the people of a mission, wanderers, and later scenes, errands,
//! strangers, epilogue scenes and post speakers) asks RZ_Shared.CanSpawnBody(kind, pos) first and is
//! entered with RZ_Shared.AddBody(kind, entity). They count in agents inside the infected's agent
//! budget (RZ_InfectedBudget), so the infected leave room for them; each kind has its own cap.
//! Owned by the spawner. Server only.
class RZ_BodyBudget
{
	static const string STORY = "story";		// a mission's people (escorts): always allowed, counted
	static const string WANDERER = "wanderer";	// wanderers: need room in the agent budget
	static const string SCENE = "scene";		// scene people and askers: 16 together
	static const string ASKER = "asker";
	static const string ERRAND = "errand";		// errand scenes: 10
	static const string STRANGER = "stranger";	// strangers: 6
	static const string EPILOGUE = "epilogue";	// epilogue scenes: 3 scenes of 6 people
	static const string SPEAKER = "speaker";	// post speakers: only within 100 m of a player

	static const int SCENE_CAP = 16;
	static const int ERRAND_CAP = 10;
	static const int STRANGER_CAP = 6;
	static const int EPILOGUE_CAP = 18;
	static const float SPEAKER_RANGE = 100;

	protected ref array<ref RZ_BodyEntry> m_aBodies = {};
	protected ref array<IEntity> m_aPlayers = {};
	protected RZ_InfectedBudget m_Budget;		// weak: the spawner's
	protected bool m_bCounted;					// the agent budget is recounted (Crawlers or Tribes on)

	//------------------------------------------------------------------------------------------------
	void RZ_BodyBudget(RZ_InfectedBudget budget)
	{
		m_Budget = budget;
	}

	//------------------------------------------------------------------------------------------------
	//! Each spawner tick, before the agent budget's recount: drop bodies that are gone or dead.
	void Tick(array<IEntity> players, bool counted)
	{
		m_bCounted = counted;
		m_aPlayers.Clear();
		foreach (IEntity p : players)
		{
			m_aPlayers.Insert(p);
		}

		for (int i = m_aBodies.Count() - 1; i >= 0; i--)
		{
			RZ_BodyEntry b = m_aBodies[i];
			if (!b.m_Entity)
			{
				m_aBodies.Remove(i);
				continue;
			}

			int alive = RZ_Util.CountAlive(b.m_Entity);
			if (alive > 0)
			{
				b.m_iAgents = alive;
				b.m_vPos = RZ_Util.GetUnitPosition(b.m_Entity);
			}
			else if (b.m_iAgents >= 0 || !SCR_AIGroup.Cast(b.m_Entity))
			{
				// Dead, or a group that had members and lost them all
				m_aBodies.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! After the agent budget's recount: every body in the ledger takes its agents near its spot.
	void AddTo(RZ_InfectedBudget budget)
	{
		foreach (RZ_BodyEntry b : m_aBodies)
		{
			budget.Add(b.m_vPos, b.Agents());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Whether a body of this kind may spawn at pos now. Without the agent budget (Crawlers and
	//! Tribes off) only the kinds' own caps apply.
	bool CanSpawn(string kind, vector pos)
	{
		if (kind == STORY)
			return true;

		if (kind == SPEAKER && (m_aPlayers.IsEmpty() || RZ_Util.NearestDistance(pos, m_aPlayers) > SPEAKER_RANGE))
			return false;

		if (kind == SCENE || kind == ASKER)
		{
			if (Count(SCENE) + Count(ASKER) >= SCENE_CAP)
				return false;
		}
		else if (kind == ERRAND && Count(ERRAND) >= ERRAND_CAP)
			return false;
		else if (kind == STRANGER && Count(STRANGER) >= STRANGER_CAP)
			return false;
		else if (kind == EPILOGUE && Count(EPILOGUE) >= EPILOGUE_CAP)
			return false;

		if (!m_bCounted || !m_Budget)
			return true;

		return m_Budget.GetTotal() < m_Budget.GetBudget();
	}

	//------------------------------------------------------------------------------------------------
	//! Enter a body that was just spawned, so it counts from now on.
	void Add(string kind, IEntity e)
	{
		if (!e)
			return;

		foreach (RZ_BodyEntry known : m_aBodies)
		{
			if (known.m_Entity == e)
				return;
		}

		RZ_BodyEntry b = new RZ_BodyEntry();
		b.m_sKind = kind;
		b.m_Entity = e;
		b.m_vPos = e.GetOrigin();
		m_aBodies.Insert(b);
		if (m_bCounted && m_Budget)
			m_Budget.Add(b.m_vPos, b.Agents());
	}

	//------------------------------------------------------------------------------------------------
	//! Agents of one kind in the ledger; "" counts them all.
	int Count(string kind)
	{
		int n = 0;
		foreach (RZ_BodyEntry b : m_aBodies)
		{
			if (kind.IsEmpty() || b.m_sKind == kind)
				n += b.Agents();
		}

		return n;
	}
}
