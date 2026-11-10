//! Survivor holdout: survivors are trapped in a town. Reach them, then hold out
//! through three waves until help arrives. If a survivor group prefab is set,
//! the survivors are real AI, and the mission fails if they all die.
class RZ_Mission_SurvivorHoldout : RZ_Mission
{
	protected IEntity m_Survivors;
	protected bool m_bStarted;
	protected float m_fHeld;
	protected float m_fWaveTimer;
	protected int m_iWaves;

	protected static const float HOLD_SECONDS = 150;
	protected static const float WAVE_SECONDS = 45;
	protected static const int MAX_WAVES = 3;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetDirector().GetStage() == RZ_EOutbreakStage.EARLY_OUTBREAK)
			return 1;

		return 2.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 30, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius * 0.6);
		m_eGiver = RZ_EFaction.SURVIVORS;
		m_iRepReward = 20;
		m_fTimeLeft = 2400;
		m_sTitle = "Survivors trapped";
		m_sBrief = string.Format("A family is barricaded inside a house in %1 and they're running out of time. Get to them and hold until help arrives.", s.m_sName);

		m_Survivors = RZ_Util.SpawnPrefab(gm.GetSurvivorGroupPrefab(), m_vPos);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_Survivors && RZ_Util.CountAlive(m_Survivors) == 0)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		bool present = CountPlayersNear(players, m_vPos, 40) > 0;
		if (!m_bStarted)
		{
			if (!present)
				return;

			m_bStarted = true;
			m_fTimeLeft = Math.Max(m_fTimeLeft, HOLD_SECONDS + 300);
			m_fWaveTimer = WAVE_SECONDS;
			NotifyParticipants(gm, "Hold the house", "They've heard the commotion. Hold on until help gets here!");
		}

		if (present)
			m_fHeld += dt;

		m_fWaveTimer += dt;
		if (m_iWaves < MAX_WAVES && m_fWaveTimer >= WAVE_SECONDS)
		{
			m_fWaveTimer = 0;
			m_iWaves++;
			gm.GetSpawner().SpawnHorde(m_vPos, 250, m_iId);
		}

		if (m_fHeld >= HOLD_SECONDS)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
		// The survivors leave with the rescue team
		RZ_Util.DeleteUnit(m_Survivors);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Survivors);
	}
}

//! Military evac: a helicopter leaves from a landing zone at a fixed time.
//! Only players standing at the LZ when it leaves get the reward, so
//! everyone converges on one spot. Expect PvP. Only happens while the
//! military is pulling out.
class RZ_Mission_EvacWindow : RZ_Mission
{
	protected bool m_bWarned;

	protected static const float LZ_RADIUS = 40;
	protected static const float COUNTDOWN = 720;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_EOutbreakStage stage = gm.GetDirector().GetStage();
		if (stage != RZ_EOutbreakStage.COLLAPSE && stage != RZ_EOutbreakStage.OVERRUN)
			return 0;

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
		for (int i = 0; i < 8; i++)
		{
			m_vPos = RZ_Util.RandomPointInRing(s.m_vCenter, s.m_fRadius, s.m_fRadius + 150);
			if (!RZ_Util.IsWater(m_vPos))
				break;
		}

		m_eGiver = RZ_EFaction.MILITARY;
		m_iRepReward = 25;
		m_fTimeLeft = COUNTDOWN;
		m_fParticipationRadius = 0; // decided at departure, see OnUpdate
		m_sTitle = "Evac flight";
		m_sBrief = string.Format("A military helicopter is leaving the LZ outside %1 in %2 minutes. Be there when it lifts off.",
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
			gm.NotifyAll("Evac flight", "Two minutes until the helicopter leaves " + GetLocationName(gm) + ".");
		}

		if (m_fTimeLeft > 0)
			return;

		m_aParticipants.Clear();
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) <= LZ_RADIUS)
				m_aParticipants.Insert(ids[i]);
		}

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

//! Burn the nest: destroy a zombie hive in a heavily infested town. Needs a
//! destructible nest prefab (e.g. from the Zombie Hive mod).
class RZ_Mission_BurnTheNest : RZ_Mission
{
	protected IEntity m_Nest;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetNestPrefab().IsEmpty())
			return 0;

		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 60, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius * 0.5);
		m_Nest = RZ_Util.SpawnPrefab(gm.GetNestPrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Nest)
			return false;

		m_eGiver = RZ_EFaction.SURVIVORS;
		m_iRepReward = 20;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = s.m_fRadius + 100;
		m_sTitle = "Burn the nest";
		m_sBrief = string.Format("Something is breeding them in %1. Find the nest and destroy it.", s.m_sName);

		for (int i = 0; i < 4; i++)
		{
			gm.GetSpawner().SpawnZombie(RZ_Util.RandomPointInRing(m_vPos, 10, 30), false, m_iSettlement, m_iId);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Nest || IsDestroyed(m_Nest))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsDestroyed(IEntity e)
	{
		SCR_DamageManagerComponent dmg = SCR_DamageManagerComponent.Cast(e.FindComponent(SCR_DamageManagerComponent));
		if (!dmg)
			return false;

		return dmg.GetState() == EDamageState.DESTROYED; // VERIFY: damage manager on the nest prefab
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
		{
			s.AddInfection(-30);
			gm.NotifyAll("Nest destroyed", s.m_sName + " is breathing easier.");
		}

		gm.SpawnSamples(m_vPos, 1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(15);

		RZ_Util.DeleteUnit(m_Nest);
	}
}

//! Bandit dead drop: the betrayal path. Bandits buy samples, paying in loot.
//! Every sample sold is one the lab never sees. Nobody else is told who sold
//! them, but the Institute finds out.
class RZ_Mission_BanditDeadDrop : RZ_Mission
{
	protected IEntity m_Drop;
	protected int m_iSold;

	protected static const int BANDIT_FAME_PER_SAMPLE = 8;
	protected static const int BANDIT_FAVOR_PER_SAMPLE = 15;
	protected static const int INSTITUTE_INFAMY_PER_SAMPLE = 6;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetDeadDropPrefab().IsEmpty() || gm.GetCure().IsCured())
			return 0;

		// Bandits get bolder as order collapses
		return 0.5 + gm.GetDirector().GetStage() * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 10, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = RZ_Util.RandomPointInRing(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius * 0.9);
		m_Drop = RZ_Util.SpawnPrefab(gm.GetDeadDropPrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Drop)
			return false;

		m_eGiver = RZ_EFaction.BANDITS;
		m_iRepReward = 0; // paid per sample instead
		m_fTimeLeft = 1500;
		m_fParticipationRadius = 0; // only sellers count
		m_sTitle = "Bandit dead drop";
		m_sBrief = string.Format("Intercepted radio: bandits are paying well for infected samples. No questions asked. There's a drop near %1.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDrop(IEntity e)
	{
		return e && e == m_Drop;
	}

	//------------------------------------------------------------------------------------------------
	void OnSold(RZ_GameModeComponent gm, int playerId, int value)
	{
		m_iSold += value;
		if (!m_aParticipants.Contains(playerId))
			m_aParticipants.Insert(playerId);

		gm.AddFame(playerId, RZ_EFaction.BANDITS, BANDIT_FAME_PER_SAMPLE * value, false);
		gm.AddFavor(playerId, RZ_EFaction.BANDITS, BANDIT_FAVOR_PER_SAMPLE * value);
		gm.AddInfamy(playerId, RZ_EFaction.INSTITUTE, INSTITUTE_INFAMY_PER_SAMPLE * value);

		// One crate per sale, bigger sales get two
		gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 2, 4));
		if (value >= 3)
			gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 2, 4));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The drop runs until the bandits pack up; selling doesn't end it early
		if (m_fTimeLeft <= 0)
		{
			if (m_iSold > 0)
				m_eState = RZ_EMissionState.SUCCEEDED;
			else
				m_eState = RZ_EMissionState.FAILED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Drop);
		gm.NotifyAll("Institute", string.Format("Someone sold %1 samples to bandits. That's research we'll never get back.", m_iSold));
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Drop);
	}
}
