//! Small lookups the newer contracts share (safehouses, fuel, messages near a spot).
class RZ_ContractHelp
{
	//------------------------------------------------------------------------------------------------
	static int CountSafehouses(RZ_GameModeComponent gm)
	{
		if (!gm.GetSafehouses())
			return 0;

		return gm.GetSafehouses().Count();
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Safehouse Safehouse(RZ_GameModeComponent gm, int idx)
	{
		if (!gm.GetSafehouses())
			return null;

		return gm.GetSafehouses().Get(idx);
	}

	//------------------------------------------------------------------------------------------------
	//! A random safehouse between minDist and maxDist from `from`, -1 if none.
	static int PickSafehouse(RZ_GameModeComponent gm, vector from, float minDist, float maxDist)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		array<int> options = {};
		for (int i = 0; i < houses.Count(); i++)
		{
			float d = RZ_Util.DistXZ(from, houses.Get(i).m_vPos);
			if (d >= minDist && d <= maxDist)
				options.Insert(i);
		}

		if (options.IsEmpty())
			return -1;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Fuel can keywords (Interactions > Cars).
	static string FuelWords(RZ_GameModeComponent gm)
	{
		RZ_CarSettings cs = gm.GetCarSettings();
		if (cs)
			return cs.m_sFuelWords;

		return "jerrycan,jerry_can,fuelcan,fuel_can,canister";
	}

	//------------------------------------------------------------------------------------------------
	//! A message for everyone within radius of pos.
	static void NotifyNear(RZ_GameModeComponent gm, vector pos, float radius, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				gm.NotifyPlayer(ids[i], title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds the players within radius of pos to `list`, each once (participants).
	static void AddNear(array<int> ids, array<IEntity> players, vector pos, float radius, notnull array<int> list)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius && !list.Contains(ids[i]))
				list.Insert(ids[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	static bool AnyNear(array<IEntity> players, vector pos, float radius)
	{
		return RZ_Util.NearestDistance(pos, players) <= radius;
	}

	//------------------------------------------------------------------------------------------------
	//! A point partway from a to b, on land (for routes).
	static vector Between(vector a, vector b, float share)
	{
		vector p = a + (b - a) * share;
		return RZ_Mission.LandPointNear(RZ_Util.GroundPos(p), 0, 40);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns `prefab` (or the puzzle prop `fallback`) with the generic object's actions for a
	//! puzzle mission. The mission still owns it and must delete it.
	static IEntity SpawnTagged(RZ_GameModeComponent gm, ResourceName prefab, RZ_EPuzzleProp fallback, vector pos, int missionId, int objIdx, string useLabels, string choiceLabels)
	{
		if (prefab.IsEmpty())
			prefab = gm.GetPuzzlePrefab(fallback);

		IEntity e = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(pos), Math.RandomFloat(0, 360));
		if (e)
			RZ_PuzzleObjectComponent.Tag(e, missionId, objIdx, useLabels, choiceLabels);

		return e;
	}
}
