//! Locked backgrounds (docs/UI.md, "Background picker"): a background loadout the player hasn't
//! unlocked can't be spawned. Behind UI > Lock loadouts (on by default).
//!
//! The server decides, whatever the client sends, so it holds with a gamepad, a console or a
//! modified client alike, and nothing here needs a mouse:
//! - SCR_LoadoutManager.CanAssignLoadout_S refuses to assign a locked loadout (the spawn screen's,
//!   or the deploy menu's, loadout request gets "no").
//! - SCR_BaseGameMode.CanPlayerSpawn_S refuses a spawn while the assigned loadout is locked, or
//!   when the spawn asks for a locked loadout's prefab (the client names the prefab it spawns as).
//! RZ_GameModeComponent.ResolveBackground stays as a last safety net: a locked pick that gets
//! through anyway plays as the default background.
//!
//! The client side only shows it: the spawn screen (RZ_SpawnMenu.c) marks a locked background and
//! says how to unlock it; with the spawn screen off, vanilla's deploy menu marks the loadout
//! unavailable from the last backgrounds payload (IsLoadoutAvailableClient).
modded class SCR_BasePlayerLoadout // VERIFY: with the spawn screen off, what the player sees: the 1.9 deploy menu leaves unavailable loadouts out when it builds the list, and greys their buttons when it refreshes
{
	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailable(int playerId)
	{
		if (!super.IsLoadoutAvailable(playerId))
			return false;

		return !RZ_LoadoutLock.IsLocked(playerId, GetLoadoutName());
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

		// Not asked yet: ask; the answer comes in a moment
		if (ui.GetBackgroundsVersion() == 0)
			ui.Ask(RZ_UIFeed.PAYLOAD_BACKGROUNDS);

		return !ui.IsLoadoutLocked(GetLoadoutName());
	}
}

//------------------------------------------------------------------------------------------------
//! Server: never assigns a locked background's loadout.
modded class SCR_LoadoutManager // VERIFY: 1.9's SCR_PlayerLoadoutComponent.Rpc_RequestLoadout_S asks CanAssignLoadout_S before it assigns a loadout
{
	//------------------------------------------------------------------------------------------------
	override bool CanAssignLoadout_S(SCR_PlayerLoadoutComponent playerLoadoutComponent, SCR_BasePlayerLoadout loadout)
	{
		if (!super.CanAssignLoadout_S(playerLoadoutComponent, loadout))
			return false;

		if (!playerLoadoutComponent || !loadout || !playerLoadoutComponent.GetPlayerController())
			return true;

		return !RZ_LoadoutLock.IsLocked(playerLoadoutComponent.GetPlayerId(), loadout.GetLoadoutName());
	}
}

//------------------------------------------------------------------------------------------------
//! Server: never spawns a player as a locked background's loadout.
modded class SCR_BaseGameMode // VERIFY: 1.9's SCR_RespawnSystemComponent.CanRequestSpawn_S asks CanPlayerSpawn_S for every spawn and can-spawn request
{
	//------------------------------------------------------------------------------------------------
	override bool CanPlayerSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (!super.CanPlayerSpawn_S(requestComponent, handlerComponent, data, result))
			return false;

		if (!requestComponent || !data)
			return true;

		string locked = RZ_LoadoutLock.LockedSpawn(requestComponent.GetPlayerId(), data.GetPrefab());
		if (locked.IsEmpty())
			return true;

		result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED;
		return false;
	}
}

//------------------------------------------------------------------------------------------------
//! The server's lock checks, shared by the three mods above.
class RZ_LoadoutLock
{
	//------------------------------------------------------------------------------------------------
	//! Server: a background loadout this player hasn't unlocked, with UI > Lock loadouts on.
	//! False on a client (it uses the backgrounds payload instead) and for loadouts that aren't
	//! backgrounds.
	static bool IsLocked(int playerId, string loadoutName)
	{
		if (!RZ_UISettings.Get().m_bLockLoadouts || !Replication.IsServer())
			return false;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return false;

		return gm.IsLoadoutLocked(playerId, loadoutName);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: the name of the locked loadout this spawn would use, "" if none. Locked when the
	//! player's assigned loadout is, or when the prefab asked for belongs to a locked loadout and
	//! isn't the assigned (unlocked) loadout's own.
	static string LockedSpawn(int playerId, ResourceName prefab)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || playerId <= 0)
			return "";

		SCR_BasePlayerLoadout assigned = gm.GetPickedLoadout(playerId);
		if (assigned && IsLocked(playerId, assigned.GetLoadoutName()))
		{
			Refused(playerId, assigned.GetLoadoutName());
			return assigned.GetLoadoutName();
		}

		if (prefab.IsEmpty() || (assigned && assigned.GetLoadoutResource() == prefab))
			return "";

		SCR_LoadoutManager manager = GetGame().GetLoadoutManager();
		if (!manager || !manager.GetPlayerLoadouts())
			return "";

		foreach (SCR_BasePlayerLoadout loadout : manager.GetPlayerLoadouts())
		{
			if (loadout && loadout.GetLoadoutResource() == prefab && IsLocked(playerId, loadout.GetLoadoutName()))
			{
				Refused(playerId, loadout.GetLoadoutName());
				return loadout.GetLoadoutName();
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static void Refused(int playerId, string loadoutName)
	{
		RZ_Log.Info("Player " + playerId + " asked to spawn with locked loadout '" + loadoutName + "': refused");
	}
}
