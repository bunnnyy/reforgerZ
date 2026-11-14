//! The map's fog of war (docs/UI.md, "The map's fog"): the map shows only what this character has
//! seen. The server keeps, for each profile, one bit per square of the map (Map > Fog > Square, 100 m)
//! and sends the owning client the whole fog as run lengths when the map opens and while it
//! changes; the client draws the unseen squares over the map (UI/RZ_MapFogDraw.c).
//!
//! What reveals ground: walking (Reveal radius round you), high ground (High radius on a tower or a
//! roof, or a hill well above the land round it), a paper map found as loot (its town, once you carry
//! it), a town's notice board when read, asking the Haven about the roads (a strip to the job you
//! follow), and a faction you're Known with (the ground round its camps).
//!
//! The fog lives on RZ_PlayerProfile (m_aFog), so it's kept across deaths and a wipe's fresh state
//! clears it. Server only, on the game mode's second tick.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Map fog.
[BaseContainerProps()]
class RZ_MapFogSettings
{
	[Attribute("1", desc: "Map fog: the map shows only what this character has seen; the mod's places show as rumours until their ground is seen (docs/UI.md, The map's fog). Off: the map as it was")]
	bool m_bOn;

	[Attribute("150", UIWidgets.Slider, "Reveal radius (m): the ground revealed round you as you walk", "50 500 10")]
	float m_fRevealRadius;

	[Attribute("400", UIWidgets.Slider, "High radius (m): what you see from high ground (a tower, a roof, a hill above the land round it)", "100 1500 25")]
	float m_fHighRadius;

	[Attribute("6", UIWidgets.Slider, "Tower height (m): standing this far above the ground under you counts as high ground (a tower, a roof)", "2 30 1")]
	float m_fTowerHeight;

	[Attribute("35", UIWidgets.Slider, "Hill rise (m): your ground this much above the average of the land 400 m round counts as high ground", "10 150 5")]
	float m_fHillRise;

	[Attribute("100", UIWidgets.Slider, "Square (m): the fog's grain. Changing it clears every player's fog", "50 250 10")]
	float m_fCell;

	[Attribute("0.82", UIWidgets.Slider, "How dark the fog is (0 to 1); below about 0.9 the roads and coast show faintly through it", "0.3 1 0.01")]
	float m_fOpacity;

	[Attribute("500", UIWidgets.Slider, "Town radius (m): what a found paper map or a town's notice board reveals round the town", "200 1500 25")]
	float m_fTownRadius;

	[Attribute("300", UIWidgets.Slider, "Camp radius (m): what a faction you're Known with shares round each of its camps", "100 1000 25")]
	float m_fCampRadius;

	[Attribute("200", UIWidgets.Slider, "Road strip (m): how wide the way to your job is when you ask about the roads", "50 600 25")]
	float m_fRoadWidth;

	[Attribute("1", desc: "Places as rumours: the mod's places (camps, the lab, the stations, the checkpoints) show as a rough circle until their ground is seen; off, they show as pins as before")]
	bool m_bRumours;
}

//------------------------------------------------------------------------------------------------
//! A player's fog state on the server that isn't saved: where they were, when it was last sent.
class RZ_MapFogState
{
	int m_iCell = -1;
	bool m_bDirty = true;
	float m_fSent = -1000;
	float m_fHighCheck;
	float m_fCampCheck = -1000;
}

//------------------------------------------------------------------------------------------------
class RZ_MapFog
{
	protected static const float SEND_GAP = 3;		// seconds between two sends while it changes
	protected static const float HIGH_GAP = 10;		// the high-ground look while standing still
	protected static const float CAMP_GAP = 30;		// friends' camps shared
	protected static const float MAP_GAP = 5;		// found maps looked for
	protected static const int MAX_SIDE = 256;		// squares a side at most (a 25 km map at 100 m)
	protected static const float RUMOUR_RADIUS = 350;

	protected RZ_GameModeComponent m_GM;
	protected RZ_MapFogSettings m_Settings;
	protected ref map<int, ref RZ_MapFogState> m_mState = new map<int, ref RZ_MapFogState>();
	protected ref array<IEntity> m_aMaps = {};		// paper maps put out as loot, not yet carried
	protected ref array<vector> m_aMapTowns = {};	// the town each was found in
	protected float m_fClock;
	protected float m_fMapCheck;

	// The grid: from the world's box
	protected float m_fX0;
	protected float m_fZ0;
	protected float m_fCell = 100;
	protected int m_iW;
	protected int m_iH;
	protected string m_sGrid;						// "x0|z0|cell|w|h": a saved fog for another grid is cleared

	//------------------------------------------------------------------------------------------------
	void RZ_MapFog(RZ_GameModeComponent gm, RZ_MapFogSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		MakeGrid();
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_Settings && m_Settings.m_bOn && m_iW > 0 && m_iH > 0;
	}

	//------------------------------------------------------------------------------------------------
	bool RumoursOn()
	{
		return IsOn() && m_Settings.m_bRumours;
	}

	//------------------------------------------------------------------------------------------------
	//! The grid over the world's box, at most MAX_SIDE squares a side.
	protected void MakeGrid()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world || !m_Settings)
			return;

		vector mins, maxs;
		world.GetBoundBox(mins, maxs); // VERIFY: BaseWorld.GetBoundBox() covers the terrain (as RZ_Perimeter reads it)
		m_fCell = Math.Max(m_Settings.m_fCell, 25);
		float sizeX = maxs[0] - mins[0];
		float sizeZ = maxs[2] - mins[2];
		if (sizeX <= 0 || sizeZ <= 0)
			return;

		m_fCell = Math.Max(m_fCell, Math.Max(sizeX, sizeZ) / MAX_SIDE);
		m_fX0 = mins[0];
		m_fZ0 = mins[2];
		m_iW = Math.Ceil(sizeX / m_fCell);
		m_iH = Math.Ceil(sizeZ / m_fCell);
		m_sGrid = string.Format("%1|%2|%3|%4|%5", RZ_Util.RoundToInt(m_fX0), RZ_Util.RoundToInt(m_fZ0), RZ_Util.RoundToInt(m_fCell), m_iW, m_iH);
		RZ_Log.Info(string.Format("Map fog: %1 x %2 squares of %3 m from %4, %5 (%6)", m_iW, m_iH, RZ_Util.RoundToInt(m_fCell), RZ_Util.RoundToInt(m_fX0), RZ_Util.RoundToInt(m_fZ0), BoolText(IsOn())));
	}

	//------------------------------------------------------------------------------------------------
	protected static string BoolText(bool on)
	{
		if (on)
			return "on";

		return "off";
	}

	// ================================================================ The bits
	//------------------------------------------------------------------------------------------------
	//! The profile's fog, made (all unseen) or cleared if it was kept for another grid.
	protected array<int> Bits(RZ_PlayerProfile p)
	{
		if (!p)
			return null;

		int words = (m_iW * m_iH + 31) / 32;
		if (!p.m_aFog)
			p.m_aFog = {};

		if (p.m_sFogGrid != m_sGrid || p.m_aFog.Count() != words)
		{
			p.m_aFog.Clear();
			for (int i = 0; i < words; i++)
			{
				p.m_aFog.Insert(0);
			}

			p.m_sFogGrid = m_sGrid;
		}

		return p.m_aFog;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Has(array<int> bits, int cell)
	{
		int word = bits[cell >> 5];
		return (word & (1 << (cell & 31))) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets one square; true if it was unseen.
	protected static bool Set(array<int> bits, int cell)
	{
		int w = cell >> 5;
		int mask = 1 << (cell & 31);
		if ((bits[w] & mask) != 0)
			return false;

		bits[w] = bits[w] | mask;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The square under a world spot, -1 off the grid.
	int CellOf(vector pos)
	{
		int cx = Math.Floor((pos[0] - m_fX0) / m_fCell);
		int cz = Math.Floor((pos[2] - m_fZ0) / m_fCell);
		if (cx < 0 || cz < 0 || cx >= m_iW || cz >= m_iH)
			return -1;

		return cz * m_iW + cx;
	}

	//------------------------------------------------------------------------------------------------
	//! This player has seen the ground at `pos` (always true with the fog off).
	bool Seen(int playerId, vector pos)
	{
		if (!IsOn())
			return true;

		int cell = CellOf(pos);
		if (cell < 0)
			return true;

		array<int> bits = Bits(m_GM.FindProfile(playerId));
		if (!bits)
			return true;

		return Has(bits, cell);
	}

	//------------------------------------------------------------------------------------------------
	//! How many squares this player has seen (the admin's read-out and the runtime test).
	int SeenCount(int playerId)
	{
		array<int> bits = Bits(m_GM.FindProfile(playerId));
		if (!bits)
			return 0;

		int n = 0;
		int total = m_iW * m_iH;
		for (int c = 0; c < total; c++)
		{
			if (Has(bits, c))
				n++;
		}

		return n;
	}

	// ================================================================ Revealing
	//------------------------------------------------------------------------------------------------
	//! Every square whose middle is within `radius` of `pos`. Returns how many were new.
	int RevealDisc(int playerId, vector pos, float radius)
	{
		if (!IsOn())
			return 0;

		array<int> bits = Bits(m_GM.FindProfile(playerId));
		if (!bits)
			return 0;

		float r = radius + m_fCell * 0.5;
		int x0 = Math.Max(0, Math.Floor((pos[0] - r - m_fX0) / m_fCell));
		int x1 = Math.Min(m_iW - 1, Math.Floor((pos[0] + r - m_fX0) / m_fCell));
		int z0 = Math.Max(0, Math.Floor((pos[2] - r - m_fZ0) / m_fCell));
		int z1 = Math.Min(m_iH - 1, Math.Floor((pos[2] + r - m_fZ0) / m_fCell));
		float r2 = r * r;
		int added = 0;
		for (int cz = z0; cz <= z1; cz++)
		{
			float mz = m_fZ0 + (cz + 0.5) * m_fCell - pos[2];
			for (int cx = x0; cx <= x1; cx++)
			{
				float mx = m_fX0 + (cx + 0.5) * m_fCell - pos[0];
				if (mx * mx + mz * mz > r2)
					continue;

				if (Set(bits, cz * m_iW + cx))
					added++;
			}
		}

		if (added > 0)
			MarkDirty(playerId);

		return added;
	}

	//------------------------------------------------------------------------------------------------
	//! A strip `width` wide from `a` to `b` (asking about the roads). Returns how many were new.
	int RevealStrip(int playerId, vector a, vector b, float width)
	{
		float len = RZ_Util.DistXZ(a, b);
		float step = Math.Max(m_fCell * 0.5, 25);
		int steps = Math.Min(400, Math.Ceil(len / step));
		int added = 0;
		for (int i = 0; i <= steps; i++)
		{
			float t = 0;
			float fi = i;
			if (steps > 0)
				t = fi / steps;

			vector p = Vector(a[0] + (b[0] - a[0]) * t, 0, a[2] + (b[2] - a[2]) * t);
			added += RevealDisc(playerId, p, width * 0.5);
		}

		return added;
	}

	//------------------------------------------------------------------------------------------------
	//! A town and the land round it (a found map, a notice board).
	int RevealTown(int playerId, vector at)
	{
		if (!IsOn())
			return 0;

		vector centre = at;
		float radius = m_Settings.m_fTownRadius;
		int idx = m_GM.FindNearestSettlement(at);
		RZ_Settlement town = m_GM.GetSettlement(idx);
		if (town && RZ_Util.DistXZ(town.m_vCenter, at) <= Math.Max(town.m_fRadius, 300) + 200)
		{
			centre = town.m_vCenter;
			radius = Math.Max(radius, town.m_fRadius + 150);
		}

		int added = RevealDisc(playerId, centre, radius);
		if (added > 0)
			RZ_Log.Info(string.Format("Map fog: player %1 learnt the ground round %2 (%3 squares)", playerId, RZ_SettlementScanner.GridReference(centre), added));

		return added;
	}

	//------------------------------------------------------------------------------------------------
	//! Every square (admin), or none (reset).
	void SetAll(int playerId, bool seen)
	{
		array<int> bits = Bits(m_GM.FindProfile(playerId));
		if (!bits)
			return;

		int fill = 0;
		if (seen)
			fill = -1;

		for (int i = 0; i < bits.Count(); i++)
		{
			bits[i] = fill;
		}

		RZ_MapFogState st = State(playerId);
		st.m_iCell = -1;
		MarkDirty(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A character starts over (character reset, designs/PERSISTENCE.md): their fog is cleared.
	void ClearFor(RZ_PlayerProfile p)
	{
		if (!p)
			return;

		p.m_aFog = {};
		p.m_sFogGrid = "";
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! Once a second (RZ_GameModeComponent.SecondTick).
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		m_fClock += 1;
		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			if (!ent)
				continue;

			RZ_MapFogState st = State(playerId);
			vector pos = ent.GetOrigin();
			int cell = CellOf(pos);
			bool moved = cell >= 0 && cell != st.m_iCell;
			if (moved || m_fClock - st.m_fHighCheck >= HIGH_GAP)
			{
				st.m_fHighCheck = m_fClock;
				float radius = m_Settings.m_fRevealRadius;
				if (OnHighGround(ent, pos))
					radius = Math.Max(radius, m_Settings.m_fHighRadius);

				if (moved || radius > m_Settings.m_fRevealRadius)
					RevealDisc(playerId, pos, radius);

				st.m_iCell = cell;
			}

			if (m_fClock - st.m_fCampCheck >= CAMP_GAP)
			{
				st.m_fCampCheck = m_fClock;
				ShareCamps(playerId);
			}

			if (st.m_bDirty && m_fClock - st.m_fSent >= SEND_GAP)
				Send(playerId);
		}

		if (m_fClock - m_fMapCheck >= MAP_GAP)
		{
			m_fMapCheck = m_fClock;
			CheckFoundMaps(ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_MapFogState State(int playerId)
	{
		RZ_MapFogState st;
		if (!m_mState.Find(playerId, st))
		{
			st = new RZ_MapFogState();
			m_mState.Insert(playerId, st);
		}

		return st;
	}

	//------------------------------------------------------------------------------------------------
	protected void MarkDirty(int playerId)
	{
		State(playerId).m_bDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_mState.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! On a tower or a roof (well above the ground under you), or on a hill above the land round it.
	protected bool OnHighGround(IEntity ent, vector pos)
	{
		float ground = RZ_Util.GroundPos(pos)[1];
		if (pos[1] - ground >= m_Settings.m_fTowerHeight)
			return true;

		float sum = 0;
		for (int k = 0; k < 8; k++)
		{
			float a = Math.PI2 * k / 8;
			vector p = pos + Vector(Math.Cos(a) * 400, 0, Math.Sin(a) * 400);
			sum += RZ_Util.GroundPos(p)[1];
		}

		return ground - sum / 8 >= m_Settings.m_fHillRise;
	}

	//------------------------------------------------------------------------------------------------
	//! A faction you're Known with shares the ground round its camps (its holdings; the Haven's
	//! safehouses too).
	protected void ShareCamps(int playerId)
	{
		float r = m_Settings.m_fCampRadius;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding hold : territory.GetHoldings())
			{
				if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, hold.m_eFaction), RZ_ETier.KNOWN))
					RevealDisc(playerId, hold.m_vPos, r);
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN))
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (h)
					RevealDisc(playerId, h.m_vPos, r);
			}
		}
	}

	// ================================================================ Found maps
	//------------------------------------------------------------------------------------------------
	//! A paper map put out as loot (Interactions/RZ_Loot.c): whoever carries it learns its town.
	void RegisterFoundMap(IEntity mapItem, vector foundAt)
	{
		if (!IsOn() || !mapItem)
			return;

		m_aMaps.Insert(mapItem);
		m_aMapTowns.Insert(foundAt);
	}

	//------------------------------------------------------------------------------------------------
	//! Is a prefab a paper map (the found-map hook asks).
	static bool IsPaperMap(ResourceName prefab)
	{
		string low = prefab;
		low.ToLower();
		return low.Contains("papermap") || low.Contains("map_paper");
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckFoundMaps(array<int> ids, array<IEntity> players)
	{
		for (int i = m_aMaps.Count() - 1; i >= 0; i--)
		{
			IEntity item = m_aMaps[i];
			if (!item)
			{
				m_aMaps.Remove(i);
				m_aMapTowns.Remove(i);
				continue;
			}

			IEntity root = item.GetRootParent(); // VERIFY: an item in a character's inventory has the character as its root parent
			if (!root || root == item)
				continue;

			// Carried by one of the players online (the characters this tick sees)
			int at = players.Find(root);
			if (at < 0)
				continue;

			int carrier = ids[at];

			int added = RevealTown(carrier, m_aMapTowns[i]);
			if (added > 0)
				m_GM.NotifyPlayer(carrier, "A map", "Somebody's map of the town, with their own pencil on it: which houses were empty, where the water is.");

			m_aMaps.Remove(i);
			m_aMapTowns.Remove(i);
		}
	}

	// ================================================================ The payload
	//------------------------------------------------------------------------------------------------
	//! Sends this player's fog now (the map opened, or it changed).
	void Send(int playerId)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_MapFogState st = State(playerId);
		st.m_bDirty = false;
		st.m_fSent = m_fClock;
		if (!ui)
			return;

		ui.SendPayload(RZ_UIFeed.PAYLOAD_FOG, Build(playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! F|x0|z0|cell|w|h|opacity, then R|runs: alternating unseen and seen run lengths over the squares
	//! row by row (z up, x across), starting with unseen (which may be 0). "" with the fog off.
	string Build(int playerId)
	{
		if (!IsOn())
			return "";

		array<int> bits = Bits(m_GM.GetProfile(playerId));
		if (!bits)
			return "";

		string head = string.Format("F|%1|%2|%3", m_sGrid, RZ_Util.RoundToInt(m_Settings.m_fOpacity * 100), 1);
		string runs = "R|";
		int total = m_iW * m_iH;
		bool seen = false;
		int run = 0;
		bool first = true;
		for (int c = 0; c < total; c++)
		{
			bool has = Has(bits, c);
			if (has == seen)
			{
				run++;
				continue;
			}

			if (!first)
				runs += ",";

			runs += run.ToString();
			first = false;
			seen = has;
			run = 1;
		}

		if (!first)
			runs += ",";

		runs += run.ToString();
		return head + "\n" + runs + "\n";
	}

	// ================================================================ Rumours
	//------------------------------------------------------------------------------------------------
	//! Where a place's rumour circle sits: somewhere round it, never on it, the same every time.
	static vector RumourCentre(vector pos)
	{
		int seed = RZ_Util.RoundToInt(pos[0] * 7 + pos[2] * 13);
		if (seed < 0)
			seed = -seed;

		int deg = seed % 360;
		int part = seed % 37;
		float a = deg * Math.DEG2RAD;
		float d = RUMOUR_RADIUS * (0.35 + part / 100.0);
		return Vector(pos[0] + Math.Cos(a) * d, pos[1], pos[2] + Math.Sin(a) * d);
	}

	//------------------------------------------------------------------------------------------------
	static float RumourRadius()
	{
		return RUMOUR_RADIUS;
	}

	// ================================================================ Admin
	//------------------------------------------------------------------------------------------------
	//! /fog (admins): "all" reveals the whole map, "reset" covers it again, else the read-out.
	void Command(int playerId, string arg)
	{
		if (!RZ_NoticeBoard.IsAdmin(playerId))
			return;

		if (!IsOn())
		{
			m_GM.NotifyPlayer(playerId, "Map fog", "The map's fog is off (Map fog > On).");
			return;
		}

		// The talk screen's one admin row: clear it, or fog it again if it's clear
		if (arg.Contains("toggle"))
		{
			if (SeenCount(playerId) >= m_iW * m_iH)
				arg = "reset";
			else
				arg = "all";
		}

		if (arg.Contains("all"))
		{
			SetAll(playerId, true);
			Send(playerId);
			m_GM.NotifyPlayer(playerId, "Map fog", "Your whole map is clear.");
		}
		else if (arg.Contains("reset"))
		{
			SetAll(playerId, false);
			Send(playerId);
			m_GM.NotifyPlayer(playerId, "Map fog", "Your map is fogged again.");
		}

		string line = string.Format("Map fog: player %1 has seen %2 of %3 squares; payload %4 characters", playerId, SeenCount(playerId), m_iW * m_iH, Build(playerId).Length());
		RZ_Log.Info(line);
		m_GM.NotifyPlayer(playerId, "Map fog", line);
	}
}
