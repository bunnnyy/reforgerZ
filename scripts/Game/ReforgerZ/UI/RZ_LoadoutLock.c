//! Locked backgrounds in the vanilla deploy menu (docs/UI.md, "Background picker"): a background
//! loadout the player hasn't unlocked is unavailable, so the menu won't spawn it, and the
//! background panel says how to unlock it. Behind UI > Lock loadouts (on by default).
//!
//! The client asks the last backgrounds payload, which the panel asks for. The 1.9 deploy menu is
//! the only vanilla caller: it reads IsLoadoutAvailableClient(). Nothing in vanilla calls the
//! server-side IsLoadoutAvailable(playerId), so the server's own check is in
//! RZ_GameModeComponent.ResolveBackground: a locked loadout spawns as the default background.
modded class SCR_BasePlayerLoadout // VERIFY: what the player sees: the 1.9 deploy menu leaves unavailable loadouts out when it builds the list, and greys their buttons when it refreshes
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
