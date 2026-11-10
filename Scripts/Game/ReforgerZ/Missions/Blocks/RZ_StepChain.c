//! Multi-step chain block (docs/DESIGN.md, "Mission building blocks"): a mission made of
//! ordered steps across towns (Patient Zero, Hunt the Deserter's trail). Only the current step
//! is live: its guards and its clue appear when a player comes within 250 m of it, and go when
//! the step is done.
//!
//! A step is done when a player has stood within its radius for its dwell time and, if it has a
//! clue, someone has read it. Clue texts live in the owning puzzle mission (AddClue); the chain
//! only spawns the readable thing and is told when it was read (OnClueRead).
class RZ_ChainStep
{
	vector m_vPos;
	int m_iSettlement = -1;
	string m_sPlace;					// "Morton"
	float m_fRadius = 25;
	float m_fDwell;						// seconds someone must stay (0 = just reach it)
	int m_iGuards;						// infected waiting there
	bool m_bSpecialGuard;				// one of them is a special
	int m_iClue = -1;					// the owning mission's clue index, -1 = none
	RZ_EPuzzleProp m_eClueProp = RZ_EPuzzleProp.CLUE;
	string m_sReadLabel = "Read";

	// Runtime
	float m_fDwelt;
	bool m_bLive;
	bool m_bRead;
	IEntity m_Clue;
}

class RZ_StepChain
{
	protected ref array<ref RZ_ChainStep> m_aSteps = {};
	protected int m_iCurrent;
	protected int m_iMissionId = -1;

	protected static const float LIVE_RADIUS = 250;

	//------------------------------------------------------------------------------------------------
	void SetMission(int missionId)
	{
		m_iMissionId = missionId;
	}

	//------------------------------------------------------------------------------------------------
	RZ_ChainStep AddStep(RZ_GameModeComponent gm, int settlement, vector pos, float radius, float dwell)
	{
		RZ_ChainStep st = new RZ_ChainStep();
		st.m_iSettlement = settlement;
		st.m_vPos = RZ_Util.GroundPos(pos);
		st.m_fRadius = radius;
		st.m_fDwell = dwell;
		RZ_Settlement s = gm.GetSettlement(settlement);
		if (s)
			st.m_sPlace = s.m_sName;
		else
			st.m_sPlace = "grid " + RZ_SettlementScanner.GridReference(pos);

		m_aSteps.Insert(st);
		return st;
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aSteps.Count();
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentIndex()
	{
		return m_iCurrent;
	}

	//------------------------------------------------------------------------------------------------
	RZ_ChainStep GetStep(int idx)
	{
		if (idx < 0 || idx >= m_aSteps.Count())
			return null;

		return m_aSteps[idx];
	}

	//------------------------------------------------------------------------------------------------
	RZ_ChainStep GetCurrent()
	{
		return GetStep(m_iCurrent);
	}

	//------------------------------------------------------------------------------------------------
	bool IsDone()
	{
		return !m_aSteps.IsEmpty() && m_iCurrent >= m_aSteps.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Once per mission tick. Returns the index of a step finished this tick, else -1.
	//! Players at the current step are added to `participants`.
	int Update(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players, notnull array<int> participants)
	{
		RZ_ChainStep st = GetCurrent();
		if (!st)
			return -1;

		if (!st.m_bLive && RZ_Util.NearestDistance(st.m_vPos, players) <= LIVE_RADIUS)
			MakeLive(gm, st);

		array<int> near = {};
		RZ_Mission.GetPlayersNear(ids, players, st.m_vPos, st.m_fRadius, near);
		foreach (int playerId : near)
		{
			if (!participants.Contains(playerId))
				participants.Insert(playerId);
		}

		if (!near.IsEmpty())
			st.m_fDwelt = st.m_fDwelt + dt;

		// Never reached yet, or not stayed long enough
		if (st.m_fDwelt <= 0 || st.m_fDwelt < st.m_fDwell)
			return -1;

		if (st.m_iClue >= 0 && !st.m_bRead)
			return -1;

		int done = m_iCurrent;
		Retire(st);
		m_iCurrent++;
		return done;
	}

	//------------------------------------------------------------------------------------------------
	//! The current step's guards and clue appear (near players only).
	protected void MakeLive(RZ_GameModeComponent gm, RZ_ChainStep st)
	{
		st.m_bLive = true;
		for (int i = 0; i < st.m_iGuards; i++)
		{
			bool special = st.m_bSpecialGuard && i == 0;
			gm.GetSpawner().SpawnZombie(RZ_Mission.LandPointNear(st.m_vPos, 8, 40), special, st.m_iSettlement, m_iMissionId);
		}

		if (st.m_iClue < 0)
			return;

		st.m_Clue = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(st.m_eClueProp), st.m_vPos, Math.RandomFloat(0, 360));
		if (st.m_Clue && !RZ_ClueComponent.Tag(st.m_Clue, m_iMissionId, st.m_iClue, st.m_sReadLabel))
		{
			// The body or prop can't be read: a note beside it
			IEntity note = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE), RZ_Util.RandomPointInRing(st.m_vPos, 0.8, 1.5));
			if (note && RZ_ClueComponent.Tag(note, m_iMissionId, st.m_iClue, st.m_sReadLabel))
			{
				RZ_Util.DeleteUnit(st.m_Clue);
				st.m_Clue = note;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Retire(RZ_ChainStep st)
	{
		RZ_Util.DeleteUnit(st.m_Clue);
		st.m_Clue = null;
	}

	//------------------------------------------------------------------------------------------------
	//! From the mission's OnClueRead: a step's clue was read.
	void OnClueRead(int clueIdx)
	{
		foreach (RZ_ChainStep st : m_aSteps)
		{
			if (st.m_iClue == clueIdx)
				st.m_bRead = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Distinct towns for a chain: each `minGap`..`maxGap` from the one before, within the
	//! infection range, not busy. False if fewer than `count` can be found.
	static bool PickTowns(RZ_GameModeComponent gm, vector from, int count, float minGap, float maxGap, float minInf, float maxInf, notnull array<int> result)
	{
		vector at = from;
		int guard = 0;
		while (result.Count() < count && guard < count * 6)
		{
			guard++;
			array<int> options = {};
			foreach (int i, RZ_Settlement s : gm.GetSettlements())
			{
				if (result.Contains(i) || gm.GetMissions().IsSettlementBusy(i))
					continue;

				if (s.m_fInfection < minInf || s.m_fInfection > maxInf)
					continue;

				float d = RZ_Util.DistXZ(at, s.m_vCenter);
				if (d >= minGap && d <= maxGap)
					options.Insert(i);
			}

			if (options.IsEmpty())
				return false;

			int pick = options.GetRandomElement();
			result.Insert(pick);
			at = gm.GetSettlement(pick).m_vCenter;
		}

		return result.Count() >= count;
	}

	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		foreach (RZ_ChainStep st : m_aSteps)
		{
			Retire(st);
		}
	}
}
