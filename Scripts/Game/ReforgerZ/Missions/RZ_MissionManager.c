//! Keeps a handful of random missions running. The count scales with the stage
//! and the player count. Mission types are chosen by weight, and each type's
//! weight depends on the world state (e.g. military missions dry up as the
//! military pulls out, sample hunts stop once the cure is done).
class RZ_MissionManager
{
	protected RZ_GameModeComponent m_GM;
	protected ref array<ref RZ_Mission> m_aActive = {};
	protected ref array<typename> m_aTypes = {};
	protected int m_iNextId = 1;
	protected float m_fCooldown = 60;

	protected static const float NEW_MISSION_COOLDOWN = 90;

	//------------------------------------------------------------------------------------------------
	void RZ_MissionManager(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		RegisterType(RZ_Mission_SupplyCache);
		RegisterType(RZ_Mission_ClearSettlement);
		RegisterType(RZ_Mission_SampleHunt);
		RegisterType(RZ_Mission_RadioRelay);
		RegisterType(RZ_Mission_SurvivorHoldout);
		RegisterType(RZ_Mission_EvacWindow);
		RegisterType(RZ_Mission_BurnTheNest);
		RegisterType(RZ_Mission_BanditDeadDrop);
	}

	//------------------------------------------------------------------------------------------------
	//! Other mods can add mission types (subclasses of RZ_Mission) here.
	void RegisterType(typename type)
	{
		if (!m_aTypes.Contains(type))
			m_aTypes.Insert(type);
	}

	//------------------------------------------------------------------------------------------------
	array<ref RZ_Mission> GetActive()
	{
		return m_aActive;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSettlementBusy(int idx)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_iSettlement == idx)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		for (int i = m_aActive.Count() - 1; i >= 0; i--)
		{
			RZ_Mission m = m_aActive[i];
			m.Update(m_GM, dt, ids, players);
			if (m.m_eState != RZ_EMissionState.ACTIVE)
			{
				Finish(m);
				m_aActive.Remove(i);
			}
		}

		if (ids.IsEmpty())
			return;

		m_fCooldown -= dt;
		int target = m_GM.GetDirector().GetStageSettings().m_iBaseMissions + ids.Count() / 4;
		if (m_fCooldown <= 0 && m_aActive.Count() < target)
		{
			m_fCooldown = NEW_MISSION_COOLDOWN;
			CreateRandom();
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnZombiesKilled(int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			m.OnZombiesKilled(m_GM, settlementIdx, missionId, special, pos, count);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnSamplesSoldToBandits(int playerId, int value, IEntity drop)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			RZ_Mission_BanditDeadDrop deadDrop = RZ_Mission_BanditDeadDrop.Cast(m);
			if (deadDrop && deadDrop.IsDrop(drop))
			{
				deadDrop.OnSold(m_GM, playerId, value);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Text for a newly spawned player: what's going on and where.
	string DescribeActiveFor(IEntity player)
	{
		if (m_aActive.IsEmpty())
			return "The radio is quiet.";

		string text = "";
		foreach (RZ_Mission m : m_aActive)
		{
			text += string.Format("%1: %2 (%3)\n", m.m_sTitle, m.GetLocationName(m_GM), RZ_Util.DirectionText(player.GetOrigin(), m.m_vPos));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateRandom()
	{
		array<ref RZ_Mission> candidates = {};
		array<float> weights = {};
		float total = 0;
		foreach (typename t : m_aTypes)
		{
			RZ_Mission m = RZ_Mission.Cast(t.Spawn());
			if (!m)
				continue;

			float w = m.GetWeight(m_GM);
			if (w <= 0)
				continue;

			candidates.Insert(m);
			weights.Insert(w);
			total += w;
		}

		// Try in weighted order until one finds a valid location
		while (!candidates.IsEmpty())
		{
			float roll = Math.RandomFloat(0, total);
			int pick = candidates.Count() - 1;
			for (int i = 0; i < candidates.Count(); i++)
			{
				roll -= weights[i];
				if (roll <= 0)
				{
					pick = i;
					break;
				}
			}

			RZ_Mission mission = candidates[pick];
			mission.m_iId = m_iNextId;
			if (mission.Setup(m_GM))
			{
				m_iNextId++;
				m_aActive.Insert(mission);
				m_GM.AnnounceMission(mission);
				RZ_Log.Info(string.Format("Mission %1 '%2' at %3", mission.m_iId, mission.m_sTitle, mission.GetLocationName(m_GM)));
				return;
			}

			total -= weights[pick];
			candidates.Remove(pick);
			weights.Remove(pick);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_Mission m)
	{
		m_GM.GetSpawner().ReleaseMission(m.m_iId);

		if (m.m_eState == RZ_EMissionState.SUCCEEDED)
		{
			m.OnSuccess(m_GM);
			foreach (int playerId : m.m_aParticipants)
			{
				RZ_Background bg = m_GM.GetPlayerBackground(playerId);
				int rep = RZ_Util.RoundToInt(m.m_iRepReward * bg.m_fMissionRewardMult);
				m_GM.AddFame(playerId, m.m_eGiver, rep);
				RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
				profile.m_iMissionsCompleted++;
				profile.m_iLifeMissions++;
			}

			m_GM.NotifyAll("Mission complete", string.Format("%1 (%2) is done.", m.m_sTitle, m.GetLocationName(m_GM)));
		}
		else
		{
			m.OnFailed(m_GM);
			m_GM.NotifyAll("Mission failed", string.Format("%1 (%2): the window has closed.", m.m_sTitle, m.GetLocationName(m_GM)));
		}
	}
}
