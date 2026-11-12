//! Street lamps and power (docs/DESIGN.md, "The power grid"), on each client. The map's own lamps
//! (street lamps, the lit lights on buildings: everything with vanilla's StreetLampComponent) light
//! themselves at night. In a Zone without power they stay dark, except where the game mode's
//! replicated list says there's power: the station and the towns it powers, a power surge's town
//! (RZ_PowerGrid.PushLamps). An empty list (World > Power grid > Street lamps off, or no power
//! grid) leaves every lamp as the map has it.
//!
//! The lamps are found once, a 100 m square of the map at a time, nearest first, out to 800 m from
//! the local player: one small sphere query every 0.1 s while there is new ground to read, nothing
//! after. A lamp is switched when it's found and when the list changes. Changes nothing in the
//! game: a dark lamp is only not drawn lit. Called from the HUD's update.

//------------------------------------------------------------------------------------------------
//! One square's read: every lamp whose light stands in it.
class RZ_LampScan
{
	int m_iX;
	int m_iZ;
	ref array<StreetLampComponent> m_aFound = {};
	ref array<vector> m_aPos = {};

	//------------------------------------------------------------------------------------------------
	bool OnEntity(IEntity e)
	{
		if (!e)
			return true;

		// The light itself, or the lamp's glass it hangs under
		Take(e);
		IEntity child = e.GetChildren();
		while (child)
		{
			Take(child);
			child = child.GetSibling();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Take(IEntity e)
	{
		StreetLampComponent lamp = StreetLampComponent.Cast(e.FindComponent(StreetLampComponent));
		if (!lamp)
			return;

		// A lamp belongs to the one square it stands in (the queries overlap)
		vector pos = e.GetOrigin();
		if (RZ_PowerLampsClient.CellOf(pos[0]) != m_iX || RZ_PowerLampsClient.CellOf(pos[2]) != m_iZ || m_aFound.Contains(lamp))
			return;

		m_aFound.Insert(lamp);
		m_aPos.Insert(pos);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_PowerLampsClient
{
	static const float CELL = 100;
	protected static const float REACH = 800;			// squares this near the local player are read
	protected static const float QUERY_RADIUS = 95;		// round a square's middle: its corners, and a hillside's rise
	protected static const float CHECK_SECONDS = 2;
	protected static const float SCAN_SECONDS = 0.1;
	protected static const int CELL_BASE = 1000;		// squares west or south of the map's corner still get an id
	protected static const int CELL_ROW = 4096;

	protected static RZ_GameModeComponent s_GM;			// this session's (a new one: start again)
	protected static float s_fCheck;
	protected static float s_fScan;
	protected static string s_sList;					// the list as applied
	protected static bool s_bOn;
	protected static vector s_vHere;
	protected static ref array<float> s_aCircles = {};	// x, z, radius of each place with power
	protected static ref array<StreetLampComponent> s_aLamps = {};
	protected static ref array<vector> s_aPos = {};
	protected static ref array<bool> s_aDark = {};		// we turned it off
	protected static ref set<int> s_aCells = new set<int>();	// read, or waiting to be
	protected static ref array<int> s_aQueue = {};

	//------------------------------------------------------------------------------------------------
	static int CellOf(float v)
	{
		return RZ_Util.RoundToInt(Math.Floor(v / CELL));
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(float timeSlice)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm != s_GM)
		{
			Reset();
			s_GM = gm;
		}

		if (!gm)
			return;

		s_fCheck += timeSlice;
		if (s_fCheck >= CHECK_SECONDS)
		{
			s_fCheck = 0;
			string list = gm.GetPowerLamps();
			if (list != s_sList)
				Apply(list);

			Queue();
		}

		if (!s_bOn)
			return;

		s_fScan += timeSlice;
		if (s_fScan >= SCAN_SECONDS)
		{
			s_fScan = 0;
			ScanNext();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A new session: the old world's lamps are gone with it.
	protected static void Reset()
	{
		s_sList = "";
		s_bOn = false;
		s_fCheck = CHECK_SECONDS;
		s_fScan = 0;
		s_aCircles.Clear();
		s_aLamps.Clear();
		s_aPos.Clear();
		s_aDark.Clear();
		s_aCells.Clear();
		s_aQueue.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! The server's list changed: "L;x z r;x z r" (or "" = leave the lamps alone). Every lamp found
	//! so far is switched to match.
	protected static void Apply(string list)
	{
		s_sList = list;
		s_bOn = !list.IsEmpty();
		s_aCircles.Clear();
		array<string> entries = {};
		list.Split(";", entries, true);
		foreach (string entry : entries)
		{
			array<string> f = {};
			entry.Split(" ", f, true);
			if (f.Count() < 3)
				continue;

			s_aCircles.Insert(f[0].ToFloat());
			s_aCircles.Insert(f[1].ToFloat());
			s_aCircles.Insert(f[2].ToFloat());
		}

		for (int i = 0; i < s_aLamps.Count(); i++)
		{
			Turn(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Is there power at pos, by the list? Everywhere, when the lamps are left alone.
	protected static bool Lit(vector pos)
	{
		if (!s_bOn)
			return true;

		for (int i = 0; i + 2 < s_aCircles.Count(); i += 3)
		{
			float dx = pos[0] - s_aCircles[i];
			float dz = pos[2] - s_aCircles[i + 1];
			if (dx * dx + dz * dz <= s_aCircles[i + 2] * s_aCircles[i + 2])
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: StreetLampComponent.SetBroken(true) on a client puts that lamp out there (its light
	//! and its glow) and SetBroken(false) lets it light itself at night again
	protected static void Turn(int i)
	{
		StreetLampComponent lamp = s_aLamps[i];
		if (!lamp)
			return;

		bool dark = !Lit(s_aPos[i]);
		if (dark == s_aDark[i])
			return;

		s_aDark[i] = dark;
		lamp.SetBroken(dark);
	}

	//------------------------------------------------------------------------------------------------
	//! The squares within reach of the local player that haven't been read yet go on the list.
	protected static void Queue()
	{
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		if (!me || !s_bOn)
			return;

		s_vHere = me.GetOrigin();
		int cx = CellOf(s_vHere[0]);
		int cz = CellOf(s_vHere[2]);
		int span = RZ_Util.RoundToInt(Math.Ceil(REACH / CELL));
		for (int dx = -span; dx <= span; dx++)
		{
			for (int dz = -span; dz <= span; dz++)
			{
				if (CellDistance(cx + dx, cz + dz) > REACH)
					continue;

				int id = (cx + dx + CELL_BASE) * CELL_ROW + cz + dz + CELL_BASE;
				if (s_aCells.Contains(id))
					continue;

				s_aCells.Insert(id);
				s_aQueue.Insert(id);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From the local player to the middle of a square (m).
	protected static float CellDistance(int ix, int iz)
	{
		float ox = (ix + 0.5) * CELL - s_vHere[0];
		float oz = (iz + 0.5) * CELL - s_vHere[2];
		return Math.Sqrt(ox * ox + oz * oz);
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the nearest waiting square; one left far behind is dropped, to be asked for again.
	protected static void ScanNext()
	{
		int best = -1;
		float bestDist = float.MAX;
		for (int i = s_aQueue.Count() - 1; i >= 0; i--)
		{
			int waiting = s_aQueue[i];
			float d = CellDistance(waiting / CELL_ROW - CELL_BASE, waiting % CELL_ROW - CELL_BASE);
			if (d > REACH + CELL * 3)
			{
				s_aCells.RemoveItem(waiting);
				s_aQueue.RemoveOrdered(i);
				if (best > i)
					best--;

				continue;
			}

			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		if (best < 0)
			return;

		int id = s_aQueue[best];
		s_aQueue.Remove(best);
		RZ_LampScan scan = new RZ_LampScan();
		scan.m_iX = id / CELL_ROW - CELL_BASE;
		scan.m_iZ = id % CELL_ROW - CELL_BASE;
		BaseWorld world = GetGame().GetWorld();
		vector middle = Vector((scan.m_iX + 0.5) * CELL, 0, (scan.m_iZ + 0.5) * CELL);
		middle[1] = world.GetSurfaceY(middle[0], middle[2]);
		world.QueryEntitiesBySphere(middle, QUERY_RADIUS, scan.OnEntity, null, EQueryEntitiesFlags.ALL);
		foreach (int n, StreetLampComponent lamp : scan.m_aFound)
		{
			s_aLamps.Insert(lamp);
			s_aPos.Insert(scan.m_aPos[n]);
			s_aDark.Insert(false);
			Turn(s_aLamps.Count() - 1);
		}
	}
}
