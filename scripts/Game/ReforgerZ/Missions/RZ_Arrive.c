//! Arriving (docs/DESIGN.md, "Mission building blocks"): the one rule for a step that's done by
//! getting somewhere, so nobody has to find the exact metre. A spot counts within Missions >
//! Contracts > Arrive radius (15 m), or the step's own radius if that's wider; a place with a
//! size (a town, a camp, a building) counts inside it or within Arrive margin (25 m) of its edge.
//! In a vehicle, where the vehicle is counts. Steps that stand for using a thing you can see (a
//! body's tags, a lock, a charge, a well) keep their own short reach and don't use this. Server
//! only, read on the callers' existing ticks.
class RZ_Arrive
{
	protected static const float SPOT_DEFAULT = 15;
	protected static const float MARGIN_DEFAULT = 25;
	protected static const float SPOT_MIN = 5;
	protected static const float SPOT_MAX = 40;		// never from across a river
	protected static const float MARGIN_MAX = 50;

	//------------------------------------------------------------------------------------------------
	//! The Arrive radius (m), kept to 5-40.
	static float SpotRadius()
	{
		RZ_ContractSettings cs = Settings();
		if (!cs)
			return SPOT_DEFAULT;

		return Math.Clamp(cs.m_fArriveRadius, SPOT_MIN, SPOT_MAX);
	}

	//------------------------------------------------------------------------------------------------
	//! The Arrive margin (m) round a place with a size, kept to 0-50.
	static float AreaMargin()
	{
		RZ_ContractSettings cs = Settings();
		if (!cs)
			return MARGIN_DEFAULT;

		return Math.Clamp(cs.m_fArriveAreaMargin, 0, MARGIN_MAX);
	}

	//------------------------------------------------------------------------------------------------
	//! A spot's radius: the step's own, or the Arrive radius if that's wider.
	static float Spot(float radius = 0)
	{
		return Math.Max(radius, SpotRadius());
	}

	//------------------------------------------------------------------------------------------------
	//! A place of this size (a town's radius; 0 for a building or a camp known only by its centre):
	//! inside it, or within the margin of it. Never less than a spot.
	static float Area(float areaRadius)
	{
		return Math.Max(Math.Max(areaRadius, 0) + AreaMargin(), SpotRadius());
	}

	//------------------------------------------------------------------------------------------------
	//! Where a player counts from: their vehicle if they sit in one, else themselves.
	static vector PosOf(IEntity player)
	{
		IEntity vehicle = RZ_Util.GetVehicleOf(player);
		if (vehicle)
			return vehicle.GetOrigin();

		return player.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	//! This player is at the spot (radius: the step's own, widened to the Arrive radius).
	static bool AtSpot(IEntity player, vector spot, float radius = 0)
	{
		return player && RZ_Util.DistXZ(PosOf(player), spot) <= Spot(radius);
	}

	//------------------------------------------------------------------------------------------------
	//! This player is in the place (centre and size), or within the margin of it.
	static bool InArea(IEntity player, vector center, float areaRadius)
	{
		return player && RZ_Util.DistXZ(PosOf(player), center) <= Area(areaRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! How many players are at the spot.
	static int CountAtSpot(array<IEntity> players, vector spot, float radius = 0)
	{
		int n = 0;
		foreach (IEntity p : players)
		{
			if (AtSpot(p, spot, radius))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The ids of the players at the spot, added to result.
	static void GetAtSpot(array<int> ids, array<IEntity> players, vector spot, float radius, notnull array<int> result)
	{
		foreach (int i, IEntity p : players)
		{
			if (AtSpot(p, spot, radius))
				result.Insert(ids[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_ContractSettings Settings()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return null;

		return gm.GetContractSettings();
	}
}
