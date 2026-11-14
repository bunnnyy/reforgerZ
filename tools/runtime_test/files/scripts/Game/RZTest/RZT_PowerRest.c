// Test-only (not part of the mod): the power and radio leftovers (docs/DESIGN.md, "The masts", "The
// grid, region by region", "The loop's relay and Bastion's last call"), right after the grid's check:
// (1) masts on the map's own cabinet are counted; a mast handed to the Haven is talked about by the
// Haven's people in its region; (2) a cut breaker takes its fuse and is remembered as cut; (3) Lights
// Back is offered for it by the faction holding its towns and is done when the breaker's back in;
// (4) Lights Out is offered against a mast a rival holds, holds the book's tip back, and is done
// when the mast's cut; (5) a lit town's cold room (the Haven's stores) runs slower; (6) Bastion's call
// brings a Red Hand crew when one of its holdings is near the tallest mast; (7) the call counts a
// night for Await Relief. Lines: "RZT power OK|FAIL ...", "RZT power: ..." and "RZT power: N
// checks, N failed".

class RZT_PowerRest
{
	static bool s_bDone;
	static int s_iChecks;
	static int s_iFails;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (ok)
		{
			Print("RZT power OK " + what);
			return;
		}

		s_iFails++;
		Print("RZT power FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from the test's tick; it runs in one go. True when it's finished (or can't run).
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		s_bDone = true;
		IEntity me = RZT_Fake.s_Ent;
		int id = RZT_Fake.ID;
		Masts(gm, id, me);
		Grid(gm, id, me);
		Call(gm, id);
		Print(string.Format("RZT power: %1 checks, %2 failed", s_iChecks, s_iFails));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Masts(RZ_GameModeComponent gm, int id, IEntity me)
	{
		RZ_Masts masts = gm.GetMasts();
		if (!masts || !masts.IsOn() || masts.Count() == 0 || !me)
		{
			Print("RZT power: no masts; the masts' leftovers not tried");
			return;
		}

		int onMap = 0;
		for (int i = 0; i < masts.Count(); i++)
		{
			RZ_Mast m = masts.GetMast(i);
			if (m.m_bMapCabinet)
				onMap++;

			Print(string.Format("RZT power: %1 at %2, on the map's own cabinet: %3", m.m_sName, RZ_SettlementScanner.GridReference(m.m_vPos), m.m_bMapCabinet));
		}

		Print(string.Format("RZT power: %1 of %2 masts carry their prompt on the map's own cabinet", onMap, masts.Count()));

		// Handed to the Haven: the Haven's people round it say so, and who climbed it
		RZ_Mast first = masts.GetMast(0);
		if (first.m_bUp)
			masts.TestCut(0, id);

		masts.TestPutUp(0, id, me, "haven");
		Check(first.m_bUp && first.m_eOwner == RZ_EMastOwner.HAVEN, string.Format("%1 is put up for the Haven", first.m_sName));
		string line = masts.RegionLine(RZ_EFaction.SURVIVORS, first.m_vPos);
		Print("RZT power: the Haven's people say: " + line);
		Check(!line.IsEmpty() && line.Contains(first.m_sShort), "the Haven's people in the mast's region talk about it");
		Check(masts.RegionLine(RZ_EFaction.MILITARY, first.m_vPos).IsEmpty(), "Bastion's people don't talk about a mast that isn't theirs");

		// Lights Out against that mast (the Haven holds it): the job pays, the book's tip doesn't
		RZ_Substations subs = RZ_Substations.Get();
		RZ_Mission_LightsOut job = new RZ_Mission_LightsOut();
		job.Configure(gm);
		bool weighed = job.GetWeight(gm) > 0;
		Check(weighed, "Lights Out is offered while a rival holds a mast (or a lit substation)");
		if (!weighed || !job.Setup(gm))
			return;

		int target = job.TestTarget();
		Print(string.Format("RZT power: Lights Out's target %1 (a substation's index, or -1-mast)", target));
		if (target >= 0)
		{
			Print("RZT power: Lights Out took a substation first; the mast's cut not tried here");
			job.Cleanup(gm);
			return;
		}

		int mast = -1 - target;
		Check(subs && subs.IsContracted("mast:" + mast.ToString()), "the job holds the book's own tip for this cut back");
		Check(masts.TestCut(mast, id), "the mast is cut");
		job.TestTick(gm);
		Check(job.m_eState == RZ_EMissionState.SUCCEEDED, "Lights Out is done once the mast is cut");
		job.Cleanup(gm);
		Check(!subs || !subs.IsContracted("mast:" + mast.ToString()), "the job's hold on the tip ends with the job");
	}

	//------------------------------------------------------------------------------------------------
	protected static void Grid(RZ_GameModeComponent gm, int id, IEntity me)
	{
		RZ_PowerGrid grid = gm.GetPowerGrid();
		RZ_Substations subs = RZ_Substations.Get();
		if (!grid || !grid.IsActive() || !subs || !me)
		{
			Print("RZT power: no grid past the station; the fuse, the jobs' put-back and the cold room not tried");
			return;
		}

		float oldRegion = -1;
		if (subs.Count() == 0)
		{
			oldRegion = grid.TestRegion(400);
			subs.TestFind(grid.GetPosition(), 400);
		}

		if (subs.Count() == 0)
		{
			Print("RZT power: no substation on this map; the fuse and Lights Back not tried");
			Restore(grid, subs, oldRegion);
			return;
		}

		bool wasPowered = grid.IsPowered();
		grid.TestPower(true);
		RZ_Substations.s_bTestNoParts = true;
		RZ_Substations.s_iTestHurt = RZ_EFaction.SURVIVORS;
		RZ_Substation sub = subs.GetSub(0);
		if (!sub.m_bOn)
		{
			subs.FitFuse(sub, id, me);
			subs.SwitchOn(sub, id, me);
		}

		// The cold room: the Haven's stores keep in a lit town, not in a dark one
		RZ_Settlement town = gm.GetSettlement(subs.TestFirstTown(0));
		if (town)
		{
			float lit = RZ_Substations.ColdRoomMult(town.m_vCenter);
			Print(string.Format("RZT power: the cold room in lit %1 runs at %2", town.m_sName, lit));
			Check(lit < 1 && lit >= 0.5, "a lit town's cold room keeps the stores longer");
		}

		// A cut takes the fuse and is remembered; Lights Back is offered and done when it's back in
		Check(subs.CutBreaker(sub, id) && !sub.m_bFused && sub.m_bCut, "a cut breaker takes its fuse and is remembered as cut");
		if (town)
			Check(RZ_Substations.ColdRoomMult(town.m_vCenter) == 1, "a dark town's cold room is off");

		RZ_Mission_LightsBack back = new RZ_Mission_LightsBack();
		back.Configure(gm);
		Check(back.GetWeight(gm) > 0, "Lights Back is offered for the cut substation by the faction holding its towns");
		if (back.Setup(gm))
		{
			Check(back.ObjectiveText(gm).Contains("Fit a fuse"), "Lights Back's first step is the fuse");
			Check(subs.FitFuse(sub, id, me), "a fuse goes back in");
			Check(back.ObjectiveText(gm).Contains("Close the breaker"), "then its step is the breaker");
			Check(subs.SwitchOn(sub, id, me), "the breaker goes back in");
			back.TestTick(gm);
			Check(back.m_eState == RZ_EMissionState.SUCCEEDED, "Lights Back is done once the breaker's back in");
			back.Cleanup(gm);
		}

		RZ_Substations.s_iTestHurt = -1;
		RZ_Substations.s_bTestNoParts = false;
		grid.TestPower(wasPowered);
		Restore(grid, subs, oldRegion);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Restore(RZ_PowerGrid grid, RZ_Substations subs, float oldRegion)
	{
		if (oldRegion <= 0)
			return;

		grid.TestRegion(oldRegion);
		subs.TestFind(grid.GetPosition(), oldRegion);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Call(RZ_GameModeComponent gm, int id)
	{
		RZ_Masts masts = gm.GetMasts();
		if (!masts || !masts.IsOn() || !masts.GetRelay())
		{
			Print("RZT power: no relay; the Red Hand at the call not tried");
			return;
		}

		RZ_LoopRelay relay = masts.GetRelay();
		RZ_Mast tallest = masts.GetMast(relay.TallestIndex());
		if (!tallest)
		{
			Print("RZT power: no tallest mast; the call not tried");
			return;
		}

		// The Red Hand come for the set only with one of its holdings near (logged either way)
		bool came = relay.TestCrew(tallest, id);
		Print(string.Format("RZT power: a Red Hand crew for Bastion's call at %1, with the holdings as they are: %2", tallest.m_sName, came));
		if (!came)
		{
			// Whatever holdings are near (the test's map may have none within 4 km): the spawn path itself
			RZ_LoopRelay.s_bTestCrewAnywhere = true;
			came = relay.TestCrew(tallest, id);
			RZ_LoopRelay.s_bTestCrewAnywhere = false;
			Print(string.Format("RZT power: with the holdings' rule set aside: %1 (Red Hand group prefab set: %2)", came, !gm.GetRedHandGroupPrefab().IsEmpty()));
		}

		if (!gm.GetRedHandGroupPrefab().IsEmpty())
			Check(came, "a Red Hand crew comes for Bastion's call");

		if (came)
		{
			Check(relay.HasCrew(), "the Red Hand's crew stands for the call");
			relay.TestCrewHome();
			Check(!relay.HasCrew(), "the crew goes home");
		}

		// The night counts for Await Relief
		int before = RZ_WorldFacts.Get().Flag("bastion.last_call");
		relay.TestOpen();
		relay.TestCall(id);
		Check(RZ_MajorHelp.World(gm, "bastion.last_call") == before + 1, "Bastion's call counts a night Await Relief reads");
	}
}
