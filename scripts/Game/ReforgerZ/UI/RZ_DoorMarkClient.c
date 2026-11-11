//! Door marks as decals, on each client (docs/TRACES.md, "Doors you can see"): reads the game
//! mode's replicated list of marked doors near players (World/Traces/RZ_TraceDraw.c) and draws a
//! decal per layer on the building's wall beside the door, from the door atlas (cells 0-15) and the
//! overlays (16-31), with the mark's age as its alpha. Draws only within 70 m of the local player;
//! removes what's out of range or gone from the list. With World > Traces > Kit > Door marks atlas
//! empty, nothing is drawn. Changes nothing in the game. Called from the HUD's update.
class RZ_DoorMarkClient
{
	protected static const float CHECK_SECONDS = 2;
	protected static const float DRAW_RADIUS = 70;
	protected static const float SIZE = 0.55;
	protected static const float LAYER_STEP = 0.004;	// later layers sit a hair nearer the wall's face

	protected static float s_fTimer;
	protected static string s_sList;
	protected static ref map<string, ref array<Decal>> s_mDrawn = new map<string, ref array<Decal>>();	// entry -> its decals

	//------------------------------------------------------------------------------------------------
	static void Tick(float timeSlice)
	{
		s_fTimer += timeSlice;
		if (s_fTimer < CHECK_SECONDS)
			return;

		s_fTimer = 0;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		if (!gm || !me)
		{
			Clear();
			return;
		}

		RZ_TraceKit kit = RZ_TraceSettings.Get().Kit();
		string list = gm.GetDoorMarks();
		if (list.IsEmpty() || kit.m_sDoorAtlas.IsEmpty())
		{
			Clear();
			return;
		}

		array<string> entries = {};
		list.Split(";", entries, true);
		vector here = me.GetOrigin();
		array<string> keep = {};
		foreach (string entry : entries)
		{
			array<string> f = {};
			entry.Split(" ", f, true);
			if (f.Count() < 7)
				continue;

			vector pos = Vector(f[0].ToFloat(), f[1].ToFloat(), f[2].ToFloat());
			if (vector.DistanceXZ(pos, here) > DRAW_RADIUS)
				continue;

			keep.Insert(entry);
			if (!s_mDrawn.Contains(entry))
				Draw(entry, f, pos, kit);
		}

		// What's out of range or no longer in the list comes off
		array<string> drop = {};
		foreach (string drawn, array<Decal> decals : s_mDrawn)
		{
			if (!keep.Contains(drawn))
				drop.Insert(drawn);
		}

		foreach (string d : drop)
		{
			Remove(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One door's decals, on the building found at the mark's point (VERIFY: a small sphere query
	//! on the client finds the same static building the server resolved the door from).
	protected static void Draw(string key, array<string> f, vector pos, RZ_TraceKit kit)
	{
		array<IEntity> found = {};
		RZ_BuildingQuery.FindBuildings(pos, 2, found, 4);
		array<Decal> decals = {};
		s_mDrawn.Set(key, decals);
		if (found.IsEmpty())
			return;

		World world = GetGame().GetWorld();
		vector normal = Vector(f[3].ToFloat(), 0, f[4].ToFloat());
		int layer = 0;
		for (int i = 5; i + 1 < f.Count(); i += 2)
		{
			int cell = f[i].ToInt();
			int alpha = Math.ClampInt(f[i + 1].ToInt(), 0, 255);
			string material = kit.m_sDoorAtlas;
			int atlas = cell;
			if (cell >= 16)
			{
				material = kit.m_sDoorOverlays;
				atlas = cell - 16;
			}

			if (material.IsEmpty())
				continue;

			// VERIFY: CreateDecal with lifetime 0 projects a static decal onto the wall along -normal
			// and returns it for RemoveDecal; the colour's alpha fades it; atlasNum picks the cell
			vector origin = pos + normal * (0.3 + layer * LAYER_STEP);
			Decal decal = world.CreateDecal(found[0], origin, normal * -0.6, 0, 0.6, 0, SIZE, 1, material, 0, ARGB(alpha, 255, 255, 255), atlas);
			if (decal)
				decals.Insert(decal);

			layer++;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Remove(string key)
	{
		array<Decal> decals;
		if (!s_mDrawn.Find(key, decals))
			return;

		World world = GetGame().GetWorld();
		if (world && decals)
		{
			foreach (Decal decal : decals)
			{
				if (decal)
					world.RemoveDecal(decal);
			}
		}

		s_mDrawn.Remove(key);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Clear()
	{
		array<string> all = {};
		foreach (string key, array<Decal> decals : s_mDrawn)
		{
			all.Insert(key);
		}

		foreach (string k : all)
		{
			Remove(k);
		}
	}
}
