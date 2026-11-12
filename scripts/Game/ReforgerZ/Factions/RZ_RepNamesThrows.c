//! The throw hook for the crews' Smoke (docs/REPUTATION.md, "What people call you"): a player who
//! throws a smoke grenade gets the Red Hand crew name Smoke, if it's their first this life and they're
//! Known or better with the Red Hand (RZ_RepVoice.CrewEvent's own rules).
//!
//! Each alive player's character is listened to for "OnGrenadeThrown" (the character's event
//! handler, as the molotov's trap does); a new body is picked up on the next 5 s check. A throw is a
//! smoke when the thrown thing's prefab name says "smoke" (vanilla's M18 and RDG-2, Bastion's signal
//! smoke). Not by weapon type: the bottles and the tin rattle are smoke grenades underneath.
//! Server only; runtime only (a restart listens again).
class RZ_RepThrows : Managed
{
	protected static const float EVERY = 5;

	protected RZ_GameModeComponent m_GM;
	protected float m_fClock;
	protected float m_fNext;
	protected ref map<int, IEntity> m_mListened = new map<int, IEntity>();	// playerId -> the character listened to

	//------------------------------------------------------------------------------------------------
	void RZ_RepThrows(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! From the reputation watch's tick.
	void Tick(float dt)
	{
		m_fClock += dt;
		if (m_fClock < m_fNext)
			return;

		m_fNext = m_fClock + EVERY;
		if (!RZ_ReputationSettings.Get().m_bCrewNames)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity me : players)
		{
			IEntity had;
			if (m_mListened.Find(ids[i], had) && had == me)
				continue;

			if (had)
				Unlisten(had);

			Listen(me);
			m_mListened.Set(ids[i], me);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Listen(IEntity character)
	{
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(character.FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RegisterScriptHandler("OnGrenadeThrown", this, OnThrown); // VERIFY: OnGrenadeThrown fires on the server for a thrown smoke grenade (as for the molotov in RZ_Traps.c)
	}

	//------------------------------------------------------------------------------------------------
	protected void Unlisten(IEntity character)
	{
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(character.FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RemoveScriptHandler("OnGrenadeThrown", this, OnThrown);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnThrown(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (!IsSmoke(weapon, entity))
			return;

		int pid = playerID;
		if (pid <= 0 && weapon && weapon.GetOwner())
		{
			IEntity root = weapon.GetOwner().GetRootParent();
			if (root)
				pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(root);
		}

		if (pid <= 0)
			return;

		RZ_Log.Info(string.Format("Reputation: %1 threw a smoke", RZ_Util.PlayerName(pid)));
		RZ_RepVoice.CrewEvent(m_GM, pid, RZ_RepVoice.CREW_SMOKE);
	}

	//------------------------------------------------------------------------------------------------
	//! A smoke grenade, by the thrown thing's (else the weapon's) prefab name.
	protected static bool IsSmoke(BaseWeaponComponent weapon, IEntity entity)
	{
		string name = "";
		if (entity)
			name = RZ_Util.GetPrefabName(entity);

		if (name.IsEmpty() && weapon && weapon.GetOwner())
			name = RZ_Util.GetPrefabName(weapon.GetOwner());

		name.ToLower();
		return name.Contains("smoke");
	}
}
