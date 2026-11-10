//! "Has a radio" and "listening" for radio puzzles. All messages go through
//! gm.NotifyPlayer, so nothing has to be broadcast on a real radio channel.
class RZ_Radio
{
	//------------------------------------------------------------------------------------------------
	//! Radio Operators always count. Everyone else needs a radio in their inventory.
	static bool HasRadio(RZ_GameModeComponent gm, int playerId, IEntity player)
	{
		if (playerId > 0 && gm.GetPlayerBackground(playerId).m_bRadioIntel)
			return true;

		return CarriesRadio(player);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsRadioOperator(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId <= 0)
			return false;

		return gm.GetPlayerBackground(playerId).m_bRadioIntel;
	}

	//------------------------------------------------------------------------------------------------
	//! An item with a radio component, or whose prefab name contains "radio".
	static bool CarriesRadio(IEntity player)
	{
		if (!player)
			return false;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;

		array<IEntity> items = {};
		inv.GetItems(items); // VERIFY: GetItems() lists items in every storage the character carries
		foreach (IEntity item : items)
		{
			if (!item)
				continue;

			if (item.FindComponent(BaseRadioComponent)) // VERIFY: BaseRadioComponent class name
				return true;

			string name = RZ_Util.GetPrefabName(item);
			name.ToLower();
			if (name.Contains("radio"))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "Listening" by being inside an area (e.g. near a mast).
	static bool IsInArea(IEntity player, vector center, float radius)
	{
		if (!player)
			return false;

		return RZ_Util.DistXZ(player.GetOrigin(), center) <= radius;
	}
}

//! "Being noticed" for stealth puzzles (docs/WORLD.md, unarmed missions).
//! A player is noticed if the heat where they stand is over a threshold, or if a
//! zombie has stayed within a few metres of them for too long. Each mission
//! owns one tracker, so the closeness timer is per player per mission.
class RZ_Stealth
{
	float m_fHeatThreshold = 45;
	float m_fZombieRadius = 10;
	float m_fZombieSeconds = 8;
	protected ref map<int, float> m_mClose = new map<int, float>();

	//------------------------------------------------------------------------------------------------
	//! Call once per mission tick. Only players within `radius` of `center` are tracked.
	void Track(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players, float dt, vector center, float radius)
	{
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (RZ_Util.DistXZ(p.GetOrigin(), center) > radius)
			{
				m_mClose.Remove(playerId);
				continue;
			}

			float close;
			m_mClose.Find(playerId, close);
			if (gm.GetSpawner().IsZombieNear(p.GetOrigin(), m_fZombieRadius))
				close += dt;
			else
				close = 0;

			m_mClose.Set(playerId, close);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsNoticed(RZ_GameModeComponent gm, IEntity player)
	{
		if (!player)
			return false;

		if (gm.GetHeat().GetHeat(player.GetOrigin()) >= m_fHeatThreshold)
			return true;

		int playerId = RZ_PuzzleActionHelper.GetPlayerId(player);
		float close;
		if (m_mClose.Find(playerId, close) && close >= m_fZombieSeconds)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Why, for the message. "" if not noticed.
	string GetReason(RZ_GameModeComponent gm, IEntity player)
	{
		if (!player)
			return "";

		if (gm.GetHeat().GetHeat(player.GetOrigin()) >= m_fHeatThreshold)
			return "You've made too much noise around here.";

		int playerId = RZ_PuzzleActionHelper.GetPlayerId(player);
		float close;
		if (m_mClose.Find(playerId, close) && close >= m_fZombieSeconds)
			return "One of them has been right next to you for a while now. It knows.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_mClose.Remove(playerId);
	}
}
