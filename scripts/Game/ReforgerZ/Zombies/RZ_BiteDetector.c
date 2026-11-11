//! Works out bites without hooking any zombie mod's damage code: if a player loses health while
//! one of our zombies is within arm's reach, it was probably a hit. Most hits are scratches; each
//! has a small chance to be an infecting bite (docs/DESIGN.md, "Zombies, bites and infection"):
//!
//!   base chance (Survival > Bite chance, 0.1)
//!   x2   surrounded: 3 or more infected within 5 m
//!   x1.5 grabbed: a second hit within 4 s
//!   x1.5 tired: stamina under a quarter (needs the stamina hook, RZ_Symptoms)
//!   x1.5 hurt: under 40% health
//!   x0.8 jacket, x0.8 gloves, x0.85 helmet (worn clothing prefab names)
//!   never above 0.6
//!
//! The upside is that it works with Bacon Zombies, Project Outbreak or anything else. The
//! downside: bleeding from a gunshot while a zombie happens to stand next to you can also count.
//! If a zombie mod exposes a proper melee event later, call RZ_GameModeComponent.InfectPlayer()
//! from it and set the bite chance here to 0.
class RZ_BiteDetector
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_InfectionSettings m_Settings;
	protected ref map<int, float> m_mLastHealth = new map<int, float>();
	protected ref map<int, float> m_mLastHitAt = new map<int, float>();
	protected ref map<int, float> m_mScratchToldAt = new map<int, float>();
	protected ref array<string> m_aJacketWords = {};
	protected ref array<string> m_aGloveWords = {};
	protected ref array<string> m_aHelmetWords = {};
	protected float m_fChance;
	protected float m_fReach;
	protected float m_fClock;

	protected static const float MIN_HEALTH_DROP = 0.5;
	protected static const float CHECK_SECONDS = 1;			// RZ_GameModeComponent calls Check() every second
	protected static const float SURROUND_RADIUS = 5;
	protected static const int SURROUND_COUNT = 3;
	protected static const float GRAB_SECONDS = 4;
	protected static const float SCRATCH_TELL_GAP = 60;

	//------------------------------------------------------------------------------------------------
	void RZ_BiteDetector(RZ_GameModeComponent gm, float chance, float reach, RZ_InfectionSettings settings)
	{
		m_GM = gm;
		m_fChance = chance;
		m_fReach = reach;
		m_Settings = settings;
		RZ_ItemHelper.Words(settings.m_sJacketWords, m_aJacketWords);
		RZ_ItemHelper.Words(settings.m_sGloveWords, m_aGloveWords);
		RZ_ItemHelper.Words(settings.m_sHelmetWords, m_aHelmetWords);
	}

	//------------------------------------------------------------------------------------------------
	//! Forget the stored health, e.g. on respawn, so a fresh body doesn't read as damage. Also
	//! after the mode itself takes health (a fever), so that doesn't read as a hit.
	void Reset(int playerId)
	{
		m_mLastHealth.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The player left: drop everything kept for them.
	void Forget(int playerId)
	{
		m_mLastHealth.Remove(playerId);
		m_mLastHitAt.Remove(playerId);
		m_mScratchToldAt.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second, with the living players.
	void Check(array<int> ids, array<IEntity> players)
	{
		m_fClock += CHECK_SECONDS;
		if (m_fChance <= 0)
			return;

		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			float hp = HealthOf(ent);
			float last;
			bool hadLast = m_mLastHealth.Find(playerId, last);
			m_mLastHealth.Set(playerId, hp);

			if (!hadLast || hp > last - MIN_HEALTH_DROP)
				continue;

			if (!m_GM.GetSpawner().IsZombieNear(ent.GetOrigin(), m_fReach))
				continue;

			OnHit(playerId, ent, hp);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHit(int playerId, IEntity ent, float hp)
	{
		float lastHit;
		bool grabbed = m_mLastHitAt.Find(playerId, lastHit) && m_fClock - lastHit <= GRAB_SECONDS;
		m_mLastHitAt.Set(playerId, m_fClock);

		if (m_GM.GetProfile(playerId).IsInfected())
			return;

		if (Math.RandomFloat01() < BiteChance(ent, hp, grabbed))
		{
			m_GM.InfectPlayer(playerId);
			return;
		}

		float told;
		if (m_mScratchToldAt.Find(playerId, told) && m_fClock - told < SCRATCH_TELL_GAP)
			return;

		m_mScratchToldAt.Set(playerId, m_fClock);
		m_GM.NotifyPlayer(playerId, "Scratched", "Nails, not teeth. It tore the cloth and the skin under it, but it didn't bite. Keep it that way.");
	}

	//------------------------------------------------------------------------------------------------
	//! The chance that this hit is an infecting bite.
	float BiteChance(IEntity ent, float hp, bool grabbed)
	{
		float chance = m_fChance;
		if (m_GM.GetSpawner().CountZombiesNear(ent.GetOrigin(), SURROUND_RADIUS, SURROUND_COUNT) >= SURROUND_COUNT)
			chance = chance * m_Settings.m_fSurroundedMult;

		if (grabbed)
			chance = chance * m_Settings.m_fGrabbedMult;

		float stamina = RZ_Symptoms.GetStamina(ent);
		if (stamina >= 0 && stamina < 0.25)
			chance = chance * m_Settings.m_fTiredMult;

		float full = RZ_Util.GetCharacterMaxHealth(ent);
		if (full > 0 && hp >= 0 && hp < full * 0.4)
			chance = chance * m_Settings.m_fHurtMult;

		array<string> worn = {};
		RZ_Util.GetWornNames(ent, worn);
		if (Wears(worn, m_aJacketWords))
			chance = chance * m_Settings.m_fJacketMult;

		if (Wears(worn, m_aGloveWords))
			chance = chance * m_Settings.m_fGloveMult;

		if (Wears(worn, m_aHelmetWords))
			chance = chance * m_Settings.m_fHelmetMult;

		return Math.Min(chance, m_Settings.m_fMaxBiteChance);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Wears(array<string> worn, array<string> words)
	{
		foreach (string name : worn)
		{
			if (RZ_ItemHelper.NameHas(name, words))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A character's health (the default hit zone), 0 if it has none. Also the screen guard's damage check.
	static float HealthOf(IEntity ent)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(ent);
		if (!c)
			return 0;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (!dmg)
			return 0;

		return dmg.GetHealth(); // the default hit zone's health
	}
}
