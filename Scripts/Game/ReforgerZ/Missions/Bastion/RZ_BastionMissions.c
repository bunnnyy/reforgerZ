//! Task Force Bastion (Military) contracts. See docs/MISSIONS.md, "Task Force Bastion".

//------------------------------------------------------------------------------------------------
//! #1 Retake the Town: kill enough zombies inside an infested town. Only while
//! Bastion is still around. ★ Police/Soldier: +25% Fame.
class RZ_Mission_ClearSettlement : RZ_Mission
{
	protected int m_iKillsNeeded;
	protected int m_iKills;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 15, 15);
		m_Req.m_fMinMilitary = 0.3;
		m_Req.Bonus("police");
		m_Req.Bonus("soldier");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
	}

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
		m_iKillsNeeded = 8 * s.m_iSize;
		SetText(s);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetText(RZ_Settlement s)
	{
		m_sTitle = "Retake " + s.m_sName;
		m_sBrief = string.Format("Bastion command wants %1 cleared. Kill %2 infected in the town.", s.m_sName, m_iKillsNeeded);
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

//------------------------------------------------------------------------------------------------
//! Haven #7 Take Back the Town: the same job once Bastion has pulled out (Known with the Haven).
class RZ_Mission_TakeBackTown : RZ_Mission_ClearSettlement
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 15);
		m_Req.m_fMaxMilitary = 0.29;
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 2;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetText(RZ_Settlement s)
	{
		m_sTitle = "Take back " + s.m_sName;
		m_sBrief = string.Format("The Haven wants to move back into %1. Kill %2 infected in the town.", s.m_sName, m_iKillsNeeded);
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Evac Flight: a helicopter leaves from a landing zone at a fixed time.
//! Only players standing at the LZ when it leaves get the reward, so
//! everyone converges on one spot. Expect PvP. Only while Bastion is pulling out.
class RZ_Mission_EvacWindow : RZ_Mission
{
	protected bool m_bWarned;

	protected static const float LZ_RADIUS = 40;
	protected static const float COUNTDOWN = 720;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 25, 20);
		m_Req.m_iMinStage = RZ_EOutbreakStage.COLLAPSE;
		m_Req.m_iMaxStage = RZ_EOutbreakStage.OVERRUN;
		m_fTimeLeft = COUNTDOWN;
		m_fParticipationRadius = 0; // decided at departure, see OnUpdate
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 3 * gm.GetDirector().GetStageSettings().m_fMilitaryPresence;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// LZ on the edge of town where a helicopter could land
		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 150);
		m_sTitle = "Evac flight";
		m_sBrief = string.Format("A Bastion helicopter is leaving the LZ outside %1 in %2 minutes. Be there when it lifts off.",
			s.m_sName, RZ_Util.RoundToInt(COUNTDOWN / 60));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The rotors are loud
		if (m_fTimeLeft < 120)
			gm.GetHeat().AddHeat(m_vPos, 4 * dt);

		if (!m_bWarned && m_fTimeLeft < 120)
		{
			m_bWarned = true;
			NotifyInterested(gm, "Evac flight", "Two minutes until the helicopter leaves " + GetLocationName(gm) + ".");
		}

		if (m_fTimeLeft > 0)
			return;

		m_aParticipants.Clear();
		GetPlayersNear(ids, players, m_vPos, LZ_RADIUS, m_aParticipants);

		if (m_aParticipants.IsEmpty())
			m_eState = RZ_EMissionState.FAILED;
		else
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// The crew kicks out what they can't carry
		gm.SpawnRewardCrate(m_vPos);
		gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 4, 8));
	}
}
