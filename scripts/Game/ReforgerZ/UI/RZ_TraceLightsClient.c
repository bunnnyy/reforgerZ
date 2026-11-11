//! Lit windows (docs/TRACES.md, "Sound and light", S2), on each client: reads the game mode's
//! replicated list of light points inside lived-in doors (World/Traces/RZ_TraceSounds.c) and keeps a
//! small warm point light at each within 150 m of the local player; removes what's out of range or
//! gone from the list. The list is empty unless World > Traces > Lit windows is on (off by default,
//! until the Workbench spike shows the light at the windows and not through the walls). Changes
//! nothing in the game. Called from the HUD's update.
class RZ_TraceLightsClient
{
	protected static const float CHECK_SECONDS = 2;
	protected static const float DRAW_RADIUS = 150;
	protected static const float RADIUS = 5;
	protected static const float LV = 3;

	protected static float s_fTimer;
	protected static ref map<string, LightEntity> s_mLit = new map<string, LightEntity>();	// entry -> its light

	//------------------------------------------------------------------------------------------------
	static void Tick(float timeSlice)
	{
		s_fTimer += timeSlice;
		if (s_fTimer < CHECK_SECONDS)
			return;

		s_fTimer = 0;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		string list = "";
		if (gm)
			list = gm.GetTraceLights();

		if (!me || list.IsEmpty())
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
			if (f.Count() < 3)
				continue;

			vector pos = Vector(f[0].ToFloat() / 10, f[1].ToFloat() / 10, f[2].ToFloat() / 10);
			if (vector.DistanceXZ(pos, here) > DRAW_RADIUS)
				continue;

			keep.Insert(entry);
			if (s_mLit.Contains(entry))
				continue;

			// VERIFY: a client-side point light 1.5 m inside a door shows at the windows and doesn't
			// bleed through the walls (Traces' spike, S2)
			LightEntity light = LightEntity.CreateLight(LightType.POINT, LightFlags.DIFFUSE_ONLY, RADIUS, Color.FromRGBA(255, 196, 120, 255), LV, pos);
			s_mLit.Set(entry, light);
		}

		array<string> drop = {};
		foreach (string lit, LightEntity l : s_mLit)
		{
			if (!keep.Contains(lit))
				drop.Insert(lit);
		}

		foreach (string d : drop)
		{
			Remove(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Remove(string key)
	{
		LightEntity light;
		if (s_mLit.Find(key, light) && light)
			SCR_EntityHelper.DeleteEntityAndChildren(light);

		s_mLit.Remove(key);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Clear()
	{
		if (s_mLit.IsEmpty())
			return;

		array<string> all = {};
		foreach (string key, LightEntity l : s_mLit)
		{
			all.Insert(key);
		}

		foreach (string k : all)
		{
			Remove(k);
		}
	}
}
