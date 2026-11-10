//! Locked backgrounds in the vanilla deploy menu (docs/UI.md, "Background picker"): a background
//! loadout the player hasn't unlocked is unavailable, so the menu shows it but won't spawn it, and
//! the background panel says how to unlock it. Behind UI > Lock loadouts (on by default).
//!
//! The server decides (RZ_GameModeComponent.IsLoadoutLocked, from the saved profile); the client
//! asks the same question of the last backgrounds payload, which the panel asks for. If this file
//! doesn't compile in your game version, delete it: locked loadouts then spawn as the default
//! background with their gear, as before.
modded class SCR_BasePlayerLoadout // VERIFY: SCR_BasePlayerLoadout has IsLoadoutAvailable(int playerId) (server) and IsLoadoutAvailableClient() (deploy menu), and the menu greys out unavailable loadouts
{
	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailable(int playerId)
	{
		if (!super.IsLoadoutAvailable(playerId))
			return false;

		if (!RZ_UISettings.Get().m_bLockLoadouts)
			return true;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !Replication.IsServer())
			return true;

		return !gm.IsLoadoutLocked(playerId, GetLoadoutName());
	}

	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailableClient()
	{
		if (!super.IsLoadoutAvailableClient())
			return false;

		if (!RZ_UISettings.Get().m_bLockLoadouts)
			return true;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return true;

		// Not asked yet (no background panel in the menu): ask; the answer comes in a moment
		if (ui.GetBackgroundsVersion() == 0)
			ui.Ask(RZ_UIFeed.PAYLOAD_BACKGROUNDS);

		return !ui.IsLoadoutLocked(GetLoadoutName());
	}
}
