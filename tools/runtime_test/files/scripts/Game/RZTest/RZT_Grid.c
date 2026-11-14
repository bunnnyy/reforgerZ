// Test-only (not part of the mod): the grid past the station (docs/DESIGN.md, "The grid, region by
// region"). Early in the run, right after the masts: (1) the substations found on the map's own
// transformer yards, each with its towns; (2) with the station running, a switched-off substation's
// town is dark, switched on it's lit, the station stopped it's dark again; (3) a line breaks and the
// town goes dark, a splice brings it back; a Red Hand toll break has its crewman; (4) the lab's
// power multiplier and the morale from the power stay within their caps; (5) a breaker cut darkens
// its towns; (6) a mast's cable cut takes it down for its old holder and putting it back up gives it
// back. The station's step is put back as it was. Lines: "RZT grid OK|FAIL ...", "RZT grid: ..." and
// "RZT grid: N checks, N failed".

class RZT_Grid
{
	static bool s_bDone;
	static float s_fOldRegion = -1;
	static int s_iChecks;
	static int s_iFails;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (ok)
		{
			Print("RZT grid OK " + what);
			return;
		}

		s_iFails++;
		Print("RZT grid FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from the test's tick; it runs in one go. True when it's finished (or can't run).
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		s_bDone = true;
		RZ_PowerGrid grid = gm.GetPowerGrid();
		if (!grid || !grid.IsActive())
		{
			Print("RZT grid: no power station on this map; not tried");
			return true;
		}

		IEntity me = RZT_Fake.s_Ent;
		int id = RZT_Fake.ID;
		RZ_Substations subs = grid.GetSubstations();
		if (!subs)
		{
			Print("RZT grid: substations are off; not tried");
			MastCut(gm, id, me);
			End();
			return true;
		}

		// A small map's station covers every town (Arland: all 14): its region is shrunk for the test
		// so the map's other yards become substations, and put back after
		s_fOldRegion = -1;
		if (subs.Count() == 0)
		{
			s_fOldRegion = grid.TestRegion(400);
			int found = subs.TestFind(grid.GetPosition(), 400);
			Print(string.Format("RZT grid: none on this map at the real region (%1 m); with it shrunk to 400 m, %2 found", RZ_Util.RoundToInt(s_fOldRegion), found));
		}

		if (subs.Count() == 0)
		{
			Print("RZT grid: no transformer yard on this map past the station; the station alone");
			MastCut(gm, id, me);
			Finish(grid, grid.IsPowered());
			return true;
		}

		bool wasPowered = grid.IsPowered();
		RZ_Substations.s_bTestNoParts = true;

		// (1) Found
		Check(subs.Count() >= 1, string.Format("%1 substations found", subs.Count()));
		for (int i = 0; i < subs.Count(); i++)
		{
			RZ_Substation s = subs.GetSub(i);
			Print(string.Format("RZT grid: %1 at %2, towns %3", s.m_sName, RZ_SettlementScanner.GridReference(s.m_vPos), subs.TownNames(s)));
			Check(!s.m_aTowns.IsEmpty(), string.Format("%1 has towns of its own", s.m_sName));
		}

		RZ_Substation sub = subs.GetSub(0);
		RZ_Settlement town = gm.GetSettlement(subs.TestFirstTown(0));
		if (!town)
		{
			Check(false, "the first substation's town exists");
			Finish(grid, wasPowered);
			return true;
		}

		vector at = town.m_vCenter;

		// (2) Switched on with the station running: lit; the station stopped: dark
		grid.TestPower(true);
		if (sub.m_bOn)
			subs.CutBreaker(sub, id);

		Check(!RZ_PowerGrid.IsGridPoweredAt(at), string.Format("%1 is dark while %2 is switched off", town.m_sName, sub.m_sName));
		Check(subs.SwitchOn(sub, id, me), "the breaker goes in");
		Check(RZ_PowerGrid.IsGridPoweredAt(at), string.Format("%1 is lit once it's switched on and the station runs", town.m_sName));
		grid.TestPower(false);
		Check(!RZ_PowerGrid.IsGridPoweredAt(at), string.Format("%1 is dark again with the station stopped", town.m_sName));
		grid.TestPower(true);

		// (3) A line breaks and is spliced; a toll break has a keeper
		Check(subs.Break(sub, RZ_Substations.BREAK_WEAR), "a line breaks");
		RZ_Settlement broken = gm.GetSettlement(sub.m_iBroken);
		if (broken)
		{
			Print(string.Format("RZT grid: the line to %1 is down at %2 (%3 m from the yard, at a pole: %4)", broken.m_sName, RZ_SettlementScanner.GridReference(sub.m_vBreak), RZ_Util.RoundToInt(RZ_Util.DistXZ(sub.m_vBreak, sub.m_vPos)), sub.m_bPole));
			Check(!RZ_PowerGrid.IsGridPoweredAt(broken.m_vCenter), string.Format("%1 is dark with its line down", broken.m_sName));
			Check(subs.Splice(sub, id, me), "the splice holds");
			Check(RZ_PowerGrid.IsGridPoweredAt(broken.m_vCenter), string.Format("%1 is lit again after the splice", broken.m_sName));
		}

		Check(subs.Break(sub, RZ_Substations.BREAK_TOLL) && !sub.m_sKeeper.IsEmpty(), string.Format("a Red Hand toll on a line has its crewman (%1)", sub.m_sKeeper));
		subs.Splice(sub, id, me);

		// (4) The factions' stakes, within their caps
		float lab = RZ_Substations.LabMult(gm);
		Check(lab >= 1 && lab <= 1.5, string.Format("the lab's power multiplier is %1 (between 1 and 1.5)", lab));
		Check(RZ_Substations.LitStreetsMult(at) >= 0.5 && RZ_Substations.LitStreetsMult(at) <= 1, "lit streets' spawn multiplier is within 0.5 and 1");
		array<int> ids = {id};
		subs.TestMorale(3600 * 24, ids);
		float cap = subs.GetSettings().m_fMoraleDayCap;
		float haven = subs.TestMoraleToday(RZ_EFaction.SURVIVORS);
		float bastion = subs.TestMoraleToday(RZ_EFaction.MILITARY);
		Print(string.Format("RZT grid: a day's power gave the Haven %1 morale and Bastion %2 (cap %3)", haven, bastion, cap));
		Check(haven <= cap + 0.01 && bastion <= cap + 0.01, "the power's morale stays within its day's cap");

		// (5) A breaker cut darkens its towns
		Check(subs.CutBreaker(sub, id) && !RZ_PowerGrid.IsGridPoweredAt(at), string.Format("a cut breaker darkens %1", town.m_sName));

		// (6) A mast cut and put back
		MastCut(gm, id, me);
		Finish(grid, wasPowered);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void MastCut(RZ_GameModeComponent gm, int id, IEntity me)
	{
		RZ_Masts masts = gm.GetMasts();
		if (!masts || !masts.IsOn() || masts.Count() == 0 || !me)
		{
			Print("RZT grid: no masts; the cable cut not tried");
			return;
		}

		masts.TestPutUp(0, id, me, "haven");
		RZ_Mast m = masts.GetMast(0);
		if (!m.m_bUp)
		{
			Print("RZT grid: the mast didn't go up for the cut; not tried");
			return;
		}

		Check(masts.TestCut(0, id) && !m.m_bUp && m.m_iCutFrom > 0, string.Format("a cut takes %1 down for its old holder", m.m_sName));
		masts.TestPutUp(0, id, me, "");
		Check(m.m_bUp && m.m_eOwner == RZ_EMastOwner.HAVEN && m.m_iCutFrom == 0, string.Format("put back up, %1 is the Haven's again", m.m_sName));
	}

	//------------------------------------------------------------------------------------------------
	protected static void Finish(RZ_PowerGrid grid, bool wasPowered)
	{
		RZ_Substations.s_bTestNoParts = false;
		grid.TestPower(wasPowered);
		if (s_fOldRegion > 0)
		{
			grid.TestRegion(s_fOldRegion);
			grid.GetSubstations().TestFind(grid.GetPosition(), s_fOldRegion);
		}
		End();
	}

	//------------------------------------------------------------------------------------------------
	protected static void End()
	{
		Print(string.Format("RZT grid: %1 checks, %2 failed", s_iChecks, s_iFails));
	}
}
