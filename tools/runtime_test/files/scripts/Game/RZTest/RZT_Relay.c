// Test-only (not part of the mod): the loop's relay and Bastion's last call (docs/DESIGN.md, "The
// loop's relay and Bastion's last call"). After the masts' check: every mast is put up and open;
// (1) the relay's rows are there and open; (2) cutting it silences the government loop, and it can't
// be reached again until its cooldown; (3) a swapped tape plays in the loop's place and reverts when
// its evening's out; (4) Bastion's call runs once and never gets an answer, and not twice in a day;
// (5) a faction holding a mast talks about it. Lines: "RZT relay OK|FAIL ...", "RZT relay: ..." and
// "RZT relay: N checks, N failed".

class RZT_Relay
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
			Print("RZT relay OK " + what);
			return;
		}

		s_iFails++;
		Print("RZT relay FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	//! The row `act` is there; `open` says whether it's open.
	protected static bool HasRow(notnull array<string> acts, notnull array<string> locks, string act, out bool open)
	{
		foreach (int i, string a : acts)
		{
			if (a == act)
			{
				open = locks[i].IsEmpty();
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from the test's tick. True when it's finished (or can't run).
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		IEntity me = RZT_Fake.s_Ent;
		if (!me)
			return false;

		s_bDone = true;
		RZ_Masts masts = gm.GetMasts();
		RZ_LoopRelay relay = RZ_LoopRelay.Get();
		if (!masts || !masts.IsOn() || !relay || !relay.IsOn())
		{
			Print("RZT relay: the masts or the relay are off; not tried");
			return true;
		}

		int id = RZT_Fake.ID;

		// Every mast up and open
		for (int i = 0; i < masts.Count(); i++)
		{
			RZ_Mast m = masts.GetMast(i);
			m.m_bUp = true;
			m.m_eOwner = RZ_EMastOwner.OPEN;
			m.m_iCutFrom = 0;
		}

		relay.TestOpen();
		relay.TestExpire();
		int r = relay.RelayIndex();
		int t = relay.TallestIndex();
		Check(r >= 0 && r < masts.Count(), "the loop's relay is picked");
		Check(t >= 0 && t < masts.Count(), "Bastion's tallest mast is picked");
		if (r < 0 || r >= masts.Count() || t < 0 || t >= masts.Count())
		{
			Print(string.Format("RZT relay: %1 checks, %2 failed", s_iChecks, s_iFails));
			return true;
		}

		Print(string.Format("RZT relay: the relay is %1, Bastion calls from %2", masts.GetMast(r).m_sName, masts.GetMast(t).m_sName));

		array<string> acts = {};
		array<string> labels = {};
		array<string> locks = {};
		masts.Rows(r, me, acts, labels, locks);
		bool open;
		Check(HasRow(acts, locks, "relay:cut", open) && open, "with every mast up, the relay's 'Cut the loop' row is open");
		bool tapeOpen;
		Check(HasRow(acts, locks, "relay:tape:0", tapeOpen) && tapeOpen, "and its tapes can be swapped");

		// Cut: silent, counted, then shut until the cooldown
		int cutsBefore = gm.GetFacts().Flag("relay.cut");
		Check(relay.CutLoop(id), "the loop's relay is cut");
		Check(RZ_LoopRelay.LoopSilent(), "the government loop is silent");
		Check(gm.GetFacts().Flag("relay.cut") == cutsBefore + 1, "relay.cut is counted");
		acts.Clear();
		labels.Clear();
		locks.Clear();
		masts.Rows(r, me, acts, labels, locks);
		bool shut;
		Check(HasRow(acts, locks, "relay:cut", shut) && !shut, "after a cut the relay is shut until its cooldown");
		Check(!relay.SwapTape(id, 1), "and a tape can't be swapped in the meantime");

		// A swapped tape: it plays in the loop's place, then reverts
		relay.TestOpen();
		relay.TestExpire();
		Check(relay.SwapTape(id, 2), "a tape is swapped in");
		string over = RZ_LoopRelay.LoopOverride();
		Print("RZT relay: the loop now says \"" + over + "\"");
		Check(over == RZ_LoopRelay.TapeLine(2) && !RZ_LoopRelay.LoopSilent(), "the loop carries the swapped tape, not silence");
		relay.TestExpire();
		Check(RZ_LoopRelay.LoopOverride().IsEmpty() && !RZ_LoopRelay.LoopSilent(), "when its evening's out, the loop has its own words again");

		// Bastion's last call: once, no answer, not twice a day
		int callsBefore = gm.GetFacts().Flag("bastion.last_call");
		Check(relay.TestCall(id), "Bastion called Command from the tallest mast");
		Check(gm.GetFacts().Flag("bastion.last_call") == callsBefore + 1, "bastion.last_call is counted (nobody answered)");
		Check(!relay.TestCall(id), "and not again the same real day");

		// A faction holding a mast talks about it
		masts.GetMast(t).m_eOwner = RZ_EMastOwner.BASTION;
		string held = masts.HeldLine(RZ_EFaction.MILITARY);
		Print("RZT relay: Bastion's channel on its mast: \"" + held + "\"");
		Check(!held.IsEmpty(), "a faction holding a mast has a line about it");
		masts.GetMast(t).m_eOwner = RZ_EMastOwner.OPEN;

		relay.TestExpire();
		Print(string.Format("RZT relay: %1 checks, %2 failed", s_iChecks, s_iFails));
		return true;
	}
}
