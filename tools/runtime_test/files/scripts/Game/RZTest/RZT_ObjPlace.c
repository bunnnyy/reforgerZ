// Test-only (not part of the mod): every objective points where its step is done (docs/UI.md, "The
// quest tracker"). For each mission and questline stage the tester is on, the place the tracker
// reports for the current objective (RZ_QuestTracker.JobFor: the HUD's arrow, the map's diamond, the
// Quests tab's row) must be within the step's arrival radius of where the step really completes,
// read from the job's own data, not from its ObjectivePlace:
//   - a delivery's thing where it lies, then (the tester carries it) one of its drops;
//   - a visit-points job's points not done yet (the tester then walks to each place the tracker gives
//     until they're all done: a point done while the tracker pointed elsewhere fails);
//   - a step chain's current step;
//   - a hold's ground; anything else its own place (m_vPos, which escorts and scenes keep up to date),
//     or one of its props (GetPropSpots).
// A search area passes when the spot is inside it (the centre is off the spot on purpose).
// The tester then goes where the tracker points, not to the mission's centre.
// Lines: "RZT objplace FAIL <type> [<when>] ..." and "RZT objective places: N checked, N wrong, N without a place".

class RZT_Obj
{
	static int s_iChecked;
	static int s_iWrong;
	static int s_iNone;
	static int s_iWalked;
	protected static int s_iLastId;
	protected static vector s_vLast;
	protected static float s_fLastArrive;
	protected static const float SLACK = 5;

	//------------------------------------------------------------------------------------------------
	//! The tester goes where the tracker points for job `id` (its current objective's place); the
	//! mission's own place without one. Remembered, so a step done there can be checked (Done).
	static void Go(RZ_GameModeComponent gm, int id, vector fallback)
	{
		vector p;
		float arrive;
		float search;
		string line;
		if (!Place(gm, id, p, arrive, search, line))
		{
			if (fallback != vector.Zero)
				RZT_Fake.MoveTo(fallback);

			return;
		}

		s_iLastId = id;
		s_vLast = p;
		s_fLastArrive = arrive;
		RZT_Fake.MoveTo(p);
	}

	//------------------------------------------------------------------------------------------------
	//! A step of job `id` was done at `at` (within `radius`) while the tester followed the tracker:
	//! the tracker must have sent him there.
	static void Done(int id, string who, vector at, float radius)
	{
		if (id != s_iLastId || s_vLast == vector.Zero)
			return;

		s_iWalked++;
		float d = RZ_Util.DistXZ(s_vLast, at);
		float allowed = Math.Max(s_fLastArrive, radius) + SLACK;
		if (d <= allowed)
			return;

		s_iWrong++;
		Print(string.Format("RZT objplace FAIL %1 [step done]: the tracker sent the tester to %2, the step was done at %3 (%4 m off, allowed %5)",
			who, RZ_SettlementScanner.GridReference(s_vLast), RZ_SettlementScanner.GridReference(at), RZ_Util.RoundToInt(d), RZ_Util.RoundToInt(allowed)));
	}

	//------------------------------------------------------------------------------------------------
	//! The tracker's place for this job's current objective, as the tester sees it. False: no place.
	static bool Place(RZ_GameModeComponent gm, int id, out vector pos, out float arrive, out float search, out string line)
	{
		RZ_UIFeed feed = gm.GetUIFeed();
		if (!feed || !feed.GetTracker() || !RZT_Fake.s_Ent)
			return false;

		RZ_ObjList list = new RZ_ObjList();
		RZ_QuestJob j = feed.GetTracker().JobFor(RZT_Fake.ID, id, RZT_Fake.s_Ent, list);
		if (!j || !j.m_bHasPlace || j.m_vPlace == vector.Zero)
			return false;

		pos = j.m_vPlace;
		arrive = j.m_fArrive;
		search = 0;
		if (j.m_bRough)
			search = j.m_fRadius;

		line = list.First(RZ_EObjState.CURRENT);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check the tracker's place for job `id` against `truths` (where the step is done, each within
	//! `radius`). Returns the tracker's place (zero if none), for the tester to go to.
	static vector Check(RZ_GameModeComponent gm, int id, string who, string when, notnull array<vector> truths, float radius)
	{
		vector p;
		float arrive;
		float search;
		string line;
		if (!Place(gm, id, p, arrive, search, line))
		{
			s_iNone++;
			Print(string.Format("RZT objplace none %1 [%2]: the tracker has no place", who, when));
			return vector.Zero;
		}

		s_iChecked++;
		if (s_iChecked % 50 == 0)
			Print("RZT objective places (so far): " + Summary());

		if (truths.IsEmpty())
			return p;

		float best = -1;
		vector bestAt;
		foreach (vector t : truths)
		{
			float d = RZ_Util.DistXZ(p, t);
			if (best < 0 || d < best)
			{
				best = d;
				bestAt = t;
			}
		}

		float allowed = Math.Max(arrive, radius) + SLACK;
		if (search > 0)
			allowed = search + SLACK;

		if (best <= allowed)
			return p;

		s_iWrong++;
		Print(string.Format("RZT objplace FAIL %1 [%2] '%3': the tracker points at %4, the step is done at %5 (%6 m off, allowed %7)",
			who, when, line, RZ_SettlementScanner.GridReference(p), RZ_SettlementScanner.GridReference(bestAt), RZ_Util.RoundToInt(best), RZ_Util.RoundToInt(allowed)));
		return p;
	}

	//------------------------------------------------------------------------------------------------
	static string Summary()
	{
		return string.Format("%1 checked, %2 steps done where the tracker sent the tester, %3 wrong, %4 without a place", s_iChecked, s_iWalked, s_iWrong, s_iNone);
	}
}

// Where a mission's current step is done, from its own data (RZT_Obj's truths)
modded class RZ_Mission
{
	//! The spots where the current step completes, and how close counts there.
	float RZT_Truths(RZ_GameModeComponent gm, notnull array<vector> result)
	{
		RZ_StepChain chain = RZ_StepChain.ForMission(m_iId);
		if (chain && !chain.IsDone() && chain.GetCurrent())
		{
			result.Insert(chain.GetCurrent().m_vPos);
			return chain.GetCurrent().m_fRadius;
		}

		if (m_vPos != vector.Zero)
			result.Insert(m_vPos);

		GetPropSpots(result);
		return m_fParticipationRadius;
	}

	//! More steps done by being there, for the tester to walk to one by one (the tracker's place each time).
	bool RZT_MoreSteps()
	{
		return false;
	}
}

// The depot's bus gone: any bus, truck or van counts, so the step is the drive to the safehouse
modded class RZ_Mission_RecoverTheBus
{
	override float RZT_Truths(RZ_GameModeComponent gm, notnull array<vector> result)
	{
		float radius = super.RZT_Truths(gm, result);
		if (m_bBusLost || m_bArrived)
		{
			result.Insert(m_Zone.m_vPos);
			radius = Math.Max(radius, m_Zone.m_fRadius);
		}

		return radius;
	}
}

modded class RZ_Mission_Hold
{
	override float RZT_Truths(RZ_GameModeComponent gm, notnull array<vector> result)
	{
		result.Insert(m_vPos);
		return m_fHoldRadius;
	}
}

modded class RZ_Mission_VisitPoints
{
	override float RZT_Truths(RZ_GameModeComponent gm, notnull array<vector> result)
	{
		return GetOpenPoints(result);
	}

	override bool RZT_MoreSteps()
	{
		return m_iVisited < m_aPoints.Count();
	}

	//! A point done while the tester followed the tracker: it must be where he was sent.
	override protected void OnPointDone(RZ_GameModeComponent gm, int idx)
	{
		super.OnPointDone(gm, idx);
		if (m_aParticipants.Contains(RZT_Fake.ID))
			RZT_Obj.Done(m_iId, Type().ToString(), m_aPoints[idx], VisitRadius());
	}
}

modded class RZ_Mission_Delivery
{
	override float RZT_Truths(RZ_GameModeComponent gm, notnull array<vector> result)
	{
		if (m_bDelivered)
			return super.RZT_Truths(gm, result);

		if (IsCarried())
		{
			GetDropSpots(result);
			return 0;
		}

		if (m_bItemSpawned && m_Item)
		{
			result.Insert(m_Item.GetOrigin());
			IEntity root = m_Item.GetRootParent();
			if (root)
				result.Insert(root.GetOrigin());

			return 0;
		}

		if (m_bInsideLater)
		{
			result.Insert(m_vInside);
			return INSIDE_PLACE_RADIUS;
		}

		return super.RZT_Truths(gm, result);
	}

	//! The tester takes the thing (into his pack, or as if): the tracker must then point at a drop.
	bool RZT_Carry()
	{
		if (m_bDelivered || !m_bItemSpawned || !m_Item || IsCarried() || m_aDrops.IsEmpty() || !RZT_Fake.s_Ent)
			return false;

		// Into his pack, or as if (the check runs before the next tick reads where it is)
		RZ_Util.InsertIntoInventory(RZT_Fake.s_Ent, m_Item);
		m_bItemWasCarried = true;
		m_vItemLastPos = RZT_Fake.s_Ent.GetOrigin();
		return true;
	}
}
