//! Mirror offers (docs/DESIGN.md, "Story blocks"): one mission class with two givers. Rolled once
//! when the mission is set up (Missions > Story > Mirror chance, 10% by default, off on a PvE
//! server). The second side's eligible players hear about it 5 minutes later, with a rough area
//! twice as wide, so they can't camp the first side's drop; it's theirs once their own side offers
//! it in person (RZ_MissionManager.ListContractsFor: a leader's "Ask for work"). Two classes use it: The Cache (the
//! Haven; the Red Hand for players the Haven Watches) and The Armory (Bastion, Trusted; the Red
//! Hand, Known). The losing side gets a line, never a fallout errand. Server only.
class RZ_Mirror
{
	int m_iFaction = -1;			// the second giver
	RZ_ETier m_eTier = RZ_ETier.KNOWN;
	int m_iNeedFaction = -1;		// and a standing with another faction (The Cache: Watched by the Haven)
	RZ_EStandingCheck m_eNeed;
	bool m_bLive;
	protected float m_fClock;
	protected bool m_bHinted;

	static const float HINT_SECONDS = 300;

	//------------------------------------------------------------------------------------------------
	void RZ_Mirror(int faction, RZ_ETier tier)
	{
		m_iFaction = faction;
		m_eTier = tier;
	}

	//------------------------------------------------------------------------------------------------
	//! Once, in the mission's Setup: is this one a mirror?
	void Roll()
	{
		RZ_MissionStorySettings s = RZ_MissionStorySettings.Get();
		m_bLive = !s.m_bPvE && Math.RandomFloat01() < s.m_fMirrorChance;
	}

	//------------------------------------------------------------------------------------------------
	//! The second side has heard (5 minutes in).
	bool IsHinted()
	{
		return m_bLive && m_bHinted;
	}

	//------------------------------------------------------------------------------------------------
	//! Eligible on the second side (from the hint on: before it, they don't know).
	bool IsSide(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bLive || !m_bHinted || playerId <= 0)
			return false;

		RZ_ETier tier = gm.GetTier(playerId, m_iFaction);
		if (RZ_Reputation.IsHunted(tier) || !RZ_Reputation.IsAtLeast(tier, m_eTier))
			return false;

		if (RZ_Factions.IsValid(m_iNeedFaction) && !RZ_Reputation.Check(gm.GetTier(playerId, m_iNeedFaction), m_eNeed))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! On the mission's tick: the second side's hint, 5 minutes in.
	void Tick(RZ_GameModeComponent gm, RZ_Mission m, float dt, array<int> ids)
	{
		if (!m_bLive || m_bHinted)
			return;

		m_fClock += dt;
		if (m_fClock < HINT_SECONDS)
			return;

		m_bHinted = true;
		string area = RZ_SettlementScanner.GridReference(RZ_Util.RandomPointInRing(m.m_vPos, 200, 900));
		foreach (int playerId : ids)
		{
			if (IsSide(gm, playerId) && !m.m_Req.IsEligible(gm, playerId))
			{
				gm.NotifyPlayerAs(playerId, RZ_Factions.GetName(m_iFaction), string.Format("Word is %1 has people going for %2, somewhere within a kilometre of grid %3. %4 wants it first. %5",
					RZ_Factions.GetName(m.m_Req.m_eGiver), m.m_sTitle, area, RZ_Factions.GetName(m_iFaction), gm.GetMissions().GiverHint(null, m_iFaction)), RZ_ENoticeTag.HEARD);
			}
		}

		RZ_Log.Info(string.Format("Mirror: %1 hinted to %2", m.m_sTitle, RZ_Factions.GetName(m_iFaction)));
	}

	//------------------------------------------------------------------------------------------------
	//! A hold mirror (The Cache): whoever has more eligible holders within `radius` of `at` wins; a
	//! draw is "half each". Returns the winner's faction, or -2 for half each.
	int HoldWinner(RZ_GameModeComponent gm, RZ_Mission m, vector at, float radius)
	{
		if (!m_bLive)
			return m.m_Req.m_eGiver;

		int first = 0;
		int second = 0;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), at) > radius)
				continue;

			if (m.m_Req.IsEligible(gm, ids[i]))
				first++;
			else if (IsSide(gm, ids[i]))
				second++;
		}

		if (second > first)
			return m_iFaction;

		if (second > 0 && second == first)
			return -2;

		return m.m_Req.m_eGiver;
	}

	//------------------------------------------------------------------------------------------------
	//! The result goes to `winner` (a faction, or -2 for half each): who's paid, and the line the
	//! losing side hears. The mission pays its participants as usual afterwards.
	void Apply(RZ_GameModeComponent gm, RZ_Mission m, int winner)
	{
		if (!m_bLive)
			return;

		int original = m.m_Req.m_eGiver;
		array<int> keep = {};
		foreach (int part : m.m_aParticipants)
		{
			bool first = m.m_Req.IsEligible(gm, part);
			bool second = IsSide(gm, part) && !first;
			if (winner == -2)
			{
				if (first)
					keep.Insert(part);
				else if (second)
					PayHalf(gm, m, part);

				continue;
			}

			bool won = (winner == original && first) || (winner == m_iFaction && second);
			if (won)
			{
				keep.Insert(part);
				continue;
			}

			if (first || second)
				gm.NotifyPlayerAs(part, m.m_sTitle, "The other lot got there first. Nothing for you this time.", RZ_ENoticeTag.MISSION);
		}

		m.m_aParticipants.Clear();
		m.m_aParticipants.InsertAll(keep);
		if (winner == -2)
		{
			m.m_fRewardScale = m.m_fRewardScale * 0.5;
			m.m_bNoLeaderCredit = true;
			RZ_Log.Info(string.Format("Mirror: %1 won by both, half each", m.m_sTitle));
			return;
		}

		if (winner == m_iFaction)
		{
			m.m_eGiver = m_iFaction;
			m.m_bNoLeaderCredit = true;
			m.m_iLeaderId = -1;
			m.SetRivalPublic(original, 5);
		}

		RZ_Log.Info(string.Format("Mirror: %1 won by %2", m.m_sTitle, RZ_Factions.GetName(winner)));
	}

	//------------------------------------------------------------------------------------------------
	//! Half each: the second side's half, paid by its own giver (neither giver's leader counts it).
	protected void PayHalf(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		int fame = Math.Max(1, m.m_iFameReward / 2);
		int favor = Math.Max(1, m.m_iFavorReward / 2);
		gm.AddFame(playerId, m_iFaction, fame, false, RZ_RepFlag.SWITCH);
		gm.AddFavor(playerId, m_iFaction, favor);
		gm.NotifyPlayerAs(playerId, m.m_sTitle, string.Format("Half each, then. +%1 Fame, +%2 Favor with %3.", fame, favor, RZ_Factions.GetName(m_iFaction)), RZ_ENoticeTag.MISSION);
	}
}
