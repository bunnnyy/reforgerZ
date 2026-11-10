//! Supply cache: reach a spot in a town and hold it. Guarded by zombies.
//! Players race each other to it, so expect PvP.
class RZ_Mission_SupplyCache : RZ_Mission
{
	protected float m_fHoldNeeded = 60;
	protected float m_fHeld;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 3;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 15, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius * 0.7);
		m_eGiver = RZ_EFaction.SURVIVORS;
		m_iRepReward = 10;
		m_sTitle = "Supply cache";
		m_sBrief = string.Format("A survivor radioed about an untouched supply cache in %1. Secure it and hold the area for a minute.", s.m_sName);

		for (int i = 0; i < 3; i++)
		{
			gm.GetSpawner().SpawnZombie(RZ_Util.RandomPointInRing(m_vPos, 15, 40), false, m_iSettlement, m_iId);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (CountPlayersNear(players, m_vPos, 25) > 0)
			m_fHeld += dt;

		if (m_fHeld >= m_fHoldNeeded)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//! Clear a town: kill enough zombies inside it. The military backs these while it still exists.
class RZ_Mission_ClearSettlement : RZ_Mission
{
	protected int m_iKillsNeeded;
	protected int m_iKills;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 2 * gm.GetDirector().GetStageSettings().m_fMilitaryPresence + 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 50, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 100;
		m_fTimeLeft = 3600;
		m_iKillsNeeded = 8 * s.m_iSize;
		m_iRepReward = 15;

		if (gm.GetDirector().GetStageSettings().m_fMilitaryPresence >= 0.3)
		{
			m_eGiver = RZ_EFaction.MILITARY;
			m_sTitle = "Retake " + s.m_sName;
			m_sBrief = string.Format("Military command wants %1 cleared. Kill %2 infected in the town.", s.m_sName, m_iKillsNeeded);
		}
		else
		{
			m_eGiver = RZ_EFaction.SURVIVORS;
			m_sTitle = "Take back " + s.m_sName;
			m_sBrief = string.Format("Survivors want to move back into %1. Kill %2 infected in the town.", s.m_sName, m_iKillsNeeded);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (settlementIdx != m_iSettlement)
			return;

		m_iKills += count;
		if (m_iKills >= m_iKillsNeeded)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return;

		s.AddInfection(-35);
		gm.NotifyAll("Town reclaimed", string.Format("%1 has been pushed back to %2.", s.m_sName, s.GetStateName()));
	}
}

//! Sample hunt: an Alpha (special) infected has been spotted. Kill it for research samples.
//! This is the main way the cure moves forward.
class RZ_Mission_SampleHunt : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetCure().IsCured())
			return 0;

		return 2.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 40, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius * 0.8);
		m_fParticipationRadius = s.m_fRadius + 100;
		m_eGiver = RZ_EFaction.INSTITUTE;
		m_iRepReward = 15;
		m_sTitle = "Alpha sighting";
		m_sBrief = string.Format("The Institute tracked a mutated infected to %1. Kill it and bring its samples to the lab.", s.m_sName);

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

//! Radio relay: find high ground near a town and keep a beacon running.
//! It's loud, so it builds heat fast and pulls a horde.
class RZ_Mission_RadioRelay : RZ_Mission
{
	protected float m_fHoldNeeded = 180;
	protected float m_fHeld;
	protected bool m_bHordeSent;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// Highest of a few sampled points near the town, so it works on any terrain
		m_vPos = RZ_Util.GroundPos(s.m_vCenter);
		for (int i = 0; i < 16; i++)
		{
			vector p = RZ_Util.RandomPointInRing(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius * 2);
			if (p[1] > m_vPos[1])
				m_vPos = p;
		}

		m_eGiver = RZ_EFaction.SURVIVORS;
		m_iRepReward = 15;
		m_sTitle = "Radio relay";
		m_sBrief = string.Format("Get to the high ground near %1 and keep the relay broadcasting for 3 minutes. Every infected nearby will hear it.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (CountPlayersNear(players, m_vPos, 30) == 0)
			return;

		m_fHeld += dt;
		gm.GetHeat().AddHeat(m_vPos, 2 * dt);

		if (!m_bHordeSent && m_fHeld >= 60)
		{
			m_bHordeSent = true;
			gm.GetSpawner().SpawnHorde(m_vPos, 350, m_iId);
		}

		if (m_fHeld >= m_fHoldNeeded)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}
