//! Disguises (docs/REPUTATION.md, "Disguises"). Wearing enough of a faction's uniform makes its
//! AI look past you while it would otherwise shoot you for your standing, as long as you're not
//! Hunted by it: with the default settings that only matters where a faction is set to shoot from
//! Watched or Strangers. Missions can ask for it too (Bust Out walks you past the stockade guards).
//!
//! Caught: any Infamy you earn with that faction while wearing its uniform is doubled, and the
//! disguise is blown for a while. Fights (mission groups, hunter squads) and retaliation still
//! shoot you: a uniform doesn't hide a muzzle flash.
//!
//! What you wear is read from the prefab names of the character's child entities, checked every
//! few seconds per player (RZ_Util.GetWornNames). Capped and simple: one faction at a time, no
//! partial disguises, nothing saved.
class RZ_Disguise
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_HostilitySettings m_Settings;
	protected ref map<int, int> m_mWearing = new map<int, int>();			// playerId -> faction worn (-1 none)
	protected ref map<int, float> m_mBlownUntil = new map<int, float>();	// playerId -> clock
	protected ref array<string> m_aBastionWords = {};
	protected ref array<string> m_aRedHandWords = {};
	protected ref array<string> m_aHalcyonWords = {};
	protected float m_fClock;
	protected float m_fCheckTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_Disguise(RZ_GameModeComponent gm, RZ_HostilitySettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		RZ_ItemHelper.Words(settings.m_sBastionUniformWords, m_aBastionWords);
		RZ_ItemHelper.Words(settings.m_sRedHandUniformWords, m_aRedHandWords);
		RZ_ItemHelper.Words(settings.m_sHalcyonUniformWords, m_aHalcyonWords);
	}

	//------------------------------------------------------------------------------------------------
	protected array<string> WordsFor(int f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return m_aBastionWords;
			case RZ_EFaction.BANDITS: return m_aRedHandWords;
			case RZ_EFaction.INSTITUTE: return m_aHalcyonWords;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bDisguises)
			return;

		m_fClock += dt;
		m_fCheckTimer += dt;
		if (m_fCheckTimer < Math.Max(2, m_Settings.m_fDisguiseCheckSeconds))
			return;

		m_fCheckTimer = 0;
		foreach (int i, IEntity p : players)
		{
			int before = Wearing(ids[i]);
			int now = Detect(p);
			m_mWearing.Set(ids[i], now);
			if (now >= 0 && now != before)
				OnPutOn(ids[i], now);
		}

		array<int> gone = {};
		foreach (int playerId, int worn : m_mWearing)
		{
			if (!ids.Contains(playerId))
				gone.Insert(playerId);
		}

		foreach (int goneId : gone)
		{
			m_mWearing.Remove(goneId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The faction whose uniform this character wears (enough pieces of it), or -1.
	protected int Detect(IEntity character)
	{
		array<string> names = {};
		RZ_Util.GetWornNames(character, names);
		if (names.IsEmpty())
			return -1;

		int best = -1;
		int bestCount = 0;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			array<string> words = WordsFor(f);
			if (!words || words.IsEmpty())
				continue;

			int count = 0;
			foreach (string name : names)
			{
				if (RZ_ItemHelper.NameHas(name, words))
					count++;
			}

			if (count >= Math.Max(1, m_Settings.m_iDisguiseItems) && count > bestCount)
			{
				best = f;
				bestCount = count;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPutOn(int playerId, int f)
	{
		string text = string.Format("In %1's uniform, their people look past you, unless they already want you dead. Get caught doing something to them and they'll remember it twice over.", RZ_Factions.GetNameInText(f));
		if (IsBlown(playerId))
			text = string.Format("%1 knows your face now. The uniform won't fool them for a while.", RZ_Factions.GetName(f));

		m_GM.NotifyPlayer(playerId, "Disguise", text);
	}

	//------------------------------------------------------------------------------------------------
	//! The faction whose uniform the player wears, -1 if none (as of the last check).
	int Wearing(int playerId)
	{
		int f;
		if (m_mWearing.Find(playerId, f))
			return f;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	bool IsBlown(int playerId)
	{
		float until;
		return m_mBlownUntil.Find(playerId, until) && m_fClock < until;
	}

	//------------------------------------------------------------------------------------------------
	//! Wearing `f`'s uniform and not blown (missions: Bust Out).
	bool IsDisguisedAs(int playerId, int f)
	{
		return m_Settings.m_bDisguises && Wearing(playerId) == f && f >= 0 && !IsBlown(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! `f`'s AI looks past this player for their standing: disguised as `f`, and not Hunted by it.
	bool Covers(int playerId, int f)
	{
		if (!IsDisguisedAs(playerId, f))
			return false;

		return !RZ_Reputation.IsHunted(m_GM.GetTier(playerId, f));
	}

	//------------------------------------------------------------------------------------------------
	//! Caught in the uniform: Infamy with that faction doubles, and the disguise is blown.
	int ScaleInfamy(int playerId, int f, int amount)
	{
		if (amount <= 0 || !IsDisguisedAs(playerId, f))
			return amount;

		m_mBlownUntil.Set(playerId, m_fClock + Math.Max(1, m_Settings.m_fDisguiseBlownMinutes) * 60);
		m_GM.NotifyPlayer(playerId, "Disguise", string.Format("They saw who was inside %1's uniform. That costs you twice.", RZ_Factions.GetNameInText(f)));
		return amount * 2;
	}
}
