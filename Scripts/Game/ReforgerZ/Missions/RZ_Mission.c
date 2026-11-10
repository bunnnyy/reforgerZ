enum RZ_EMissionState
{
	ACTIVE,
	SUCCEEDED,
	FAILED
}

//! Base for random missions. A mission picks a settlement on the current map,
//! so every type works on any terrain. Anyone who comes within
//! m_fParticipationRadius while it runs shares the reward, whether they arrived
//! as a group or as strangers.
class RZ_Mission
{
	int m_iId;
	string m_sTitle;
	string m_sBrief;
	int m_iSettlement = -1;
	vector m_vPos;
	RZ_EFaction m_eGiver = RZ_EFaction.SURVIVORS;
	int m_iRepReward = 10;
	float m_fTimeLeft = 2400;
	float m_fParticipationRadius = 150;
	RZ_EMissionState m_eState = RZ_EMissionState.ACTIVE;
	ref array<int> m_aParticipants = {};

	//------------------------------------------------------------------------------------------------
	//! Relative chance of this mission type being picked right now. 0 = never.
	float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Pick a location and fill in the text. Return false if no valid location exists.
	bool Setup(RZ_GameModeComponent gm)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Update(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fTimeLeft -= dt;

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) <= m_fParticipationRadius && !m_aParticipants.Contains(ids[i]))
				m_aParticipants.Insert(ids[i]);
		}

		OnUpdate(gm, dt, ids, players);

		if (m_eState == RZ_EMissionState.ACTIVE && m_fTimeLeft <= 0)
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players) {}

	//------------------------------------------------------------------------------------------------
	void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count) {}

	//------------------------------------------------------------------------------------------------
	void OnSuccess(RZ_GameModeComponent gm) {}

	//------------------------------------------------------------------------------------------------
	void OnFailed(RZ_GameModeComponent gm) {}

	//------------------------------------------------------------------------------------------------
	string GetLocationName(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Random settlement whose infection is within [minInf, maxInf]. -1 if none.
	static int PickSettlement(RZ_GameModeComponent gm, float minInf, float maxInf)
	{
		array<int> candidates = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection >= minInf && s.m_fInfection <= maxInf && !gm.GetMissions().IsSettlementBusy(i))
				candidates.Insert(i);
		}

		if (candidates.IsEmpty())
			return -1;

		return candidates.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	static int CountPlayersNear(array<IEntity> players, vector pos, float radius)
	{
		int n = 0;
		foreach (IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				n++;
		}

		return n;
	}
}
