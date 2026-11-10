//! Halcyon (Institute) contracts. See docs/MISSIONS.md, "Halcyon".

//------------------------------------------------------------------------------------------------
//! #1 Alpha Sighting: a special infected has been spotted. Kill it for research
//! samples. This is the main way the cure moves forward.
class RZ_Mission_SampleHunt : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 15, 15);
		m_Req.m_bNotCured = true;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 2.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 40, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.8);
		m_fParticipationRadius = s.m_fRadius + 100;
		m_sTitle = "Alpha sighting";
		m_sBrief = string.Format("Halcyon tracked a mutated infected to %1. Kill it and bring its samples to the lab.", s.m_sName);

		RZ_TrackedZombie alpha = gm.GetSpawner().SpawnZombie(m_vPos, true, m_iSettlement, m_iId);
		return alpha != null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId == m_iId && special)
		{
			m_vPos = pos;
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// The kill already dropped one sample; an Alpha is worth more
		gm.SpawnSamples(m_vPos, 2);
	}
}
