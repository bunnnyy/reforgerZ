//! Works out bites without hooking any zombie mod's damage code: if a player
//! loses health while one of our zombies is within arm's reach, it was probably
//! a hit, and each hit has a chance to infect.
//!
//! The upside is that it works with Bacon Zombies, Project Outbreak or anything
//! else. The downside: bleeding from a gunshot while a zombie happens to stand
//! next to you can also count. That's an acceptable trade for now. If a zombie
//! mod exposes a proper melee event later, call RZ_GameModeComponent.InfectPlayer() from it
//! and set the bite chance here to 0.
class RZ_BiteDetector
{
	protected RZ_GameModeComponent m_GM;
	protected ref map<int, float> m_mLastHealth = new map<int, float>();
	protected float m_fChance;
	protected float m_fReach;

	protected static const float MIN_HEALTH_DROP = 0.5;

	//------------------------------------------------------------------------------------------------
	void RZ_BiteDetector(RZ_GameModeComponent gm, float chance, float reach)
	{
		m_GM = gm;
		m_fChance = chance;
		m_fReach = reach;
	}

	//------------------------------------------------------------------------------------------------
	//! Forget the stored health, e.g. on respawn, so a fresh body doesn't read as damage.
	void Reset(int playerId)
	{
		m_mLastHealth.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void Check()
	{
		if (m_fChance <= 0)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);

		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			float hp = GetHealth(ent);
			float last;
			bool hadLast = m_mLastHealth.Find(playerId, last);
			m_mLastHealth.Set(playerId, hp);

			if (!hadLast || hp > last - MIN_HEALTH_DROP)
				continue;

			if (!m_GM.GetSpawner().IsZombieNear(ent.GetOrigin(), m_fReach))
				continue;

			if (Math.RandomFloat01() < m_fChance)
				m_GM.InfectPlayer(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float GetHealth(IEntity ent)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(ent);
		if (!c)
			return 0;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (!dmg)
			return 0;

		return dmg.GetHealth(); // VERIFY: total health accessor (may be GetDefaultHitZone().GetHealth())
	}
}
