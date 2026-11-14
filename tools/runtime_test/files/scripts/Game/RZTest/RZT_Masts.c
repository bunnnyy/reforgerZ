// Test-only (not part of the mod): the masts (docs/DESIGN.md, "The masts"). Early in the run, before
// the missions move the tester: (1) the masts are found, each with a part and a trouble; (2) a crew's
// promise settles a mast's trouble and binds it to the Red Hand; (3) a mast put up with its part (a
// jerrycan stand-in for the fuel) takes the part, is up, and clears the fog 800 m off it; (4) keeping
// it open makes it everyone's; a second, handed to the Haven, is the Haven's; (5) an up mast has
// something to overhear; (6) a trapped voice is found by bearings at two masts and a Distress Call
// starts at the spot. Lines: "RZT masts OK|FAIL ...", "RZT masts: ..." and "RZT masts: N checks, N failed".

class RZT_Masts
{
	static int s_iStep;
	static int s_iWait;
	static int s_iChecks;
	static int s_iFails;
	static bool s_bDone;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (ok)
		{
			Print("RZT masts OK " + what);
			return;
		}

		s_iFails++;
		Print("RZT masts FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	static void End()
	{
		Print(string.Format("RZT masts: %1 checks, %2 failed", s_iChecks, s_iFails));
		s_bDone = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from the test's tick. True when it's finished (or can't run).
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		RZ_Masts masts = gm.GetMasts();
		if (!masts || !masts.IsOn())
		{
			Print("RZT masts: the masts are off or none were found; not tried");
			s_bDone = true;
			return true;
		}

		IEntity me = RZT_Fake.s_Ent;
		if (!me || !RZ_Util.IsCharacterAlive(me))
			return false;

		int id = RZT_Fake.ID;
		if (s_iStep == 0)
		{
			Check(masts.Count() >= 1, string.Format("%1 masts found (%2 up)", masts.Count(), masts.UpCount()));
			for (int i = 0; i < masts.Count(); i++)
			{
				RZ_Mast m = masts.GetMast(i);
				Print(string.Format("RZT masts: %1 at %2, missing %3, trouble %4", m.m_sName, RZ_SettlementScanner.GridReference(m.m_vPos), RZ_Masts.PartWord(m.m_ePart), typename.EnumToString(RZ_EMastTrouble, m.m_eTrouble)));
				Check(RZ_Mainland.OnMainland(m.m_vPos) && !RZ_Util.IsWater(m.m_vPos), string.Format("%1 stands on dry mainland", m.m_sName));
			}

			// A crew's promise: the trouble settled, the mast the Red Hand's when it's up
			RZ_Mast crew = masts.GetMast(masts.Count() - 1);
			crew.m_eTrouble = RZ_EMastTrouble.CREW;
			crew.m_bSettled = false;
			crew.m_sKeeper = "Test Crewman";
			string said = masts.KeeperPick(crew.m_iId, id, 3, me);
			Check(crew.m_bSettled && crew.m_bPromised, string.Format("the crewman's promise settles it and binds it (\"%1\")", said));
			s_iStep = 1;
			return false;
		}

		if (s_iStep == 1)
		{
			RZ_Mast first = masts.GetMast(0);
			RZT_Fake.MoveTo(first.m_vPos + Vector(5, 0, 0));
			masts.TestPutUp(0, id, me, "open");
			Check(first.m_bUp, string.Format("%1 is up after the climb", first.m_sName));
			Check(RZ_Masts.CountPart(me, RZ_EMastPart.FUEL) == 0, "the part was taken from the pack");
			RZ_MapFog fog = gm.GetMapFog();
			if (fog && fog.IsOn())
			{
				vector off = first.m_vPos + Vector(0, 0, 800);
				if (fog.CellOf(off) >= 0)
					Check(fog.Seen(id, off), "the fog is cleared 800 m from the mast");
			}

			Check(first.m_eOwner == RZ_EMastOwner.OPEN, "kept open, it's everyone's");
			string heard = masts.HeardLine(first);
			Print("RZT masts: overheard: " + heard);
			Check(!heard.IsEmpty(), "an up mast has something to overhear");
			Print("RZT masts: a warning at the mast: " + masts.TestWarning(0, me));
			s_iStep = 2;
			return false;
		}

		if (s_iStep == 2)
		{
			if (masts.Count() < 2)
			{
				Print("RZT masts: one mast only; the second mast and the voice not tried");
				End();
				return true;
			}

			RZ_Mast second = masts.GetMast(1);
			masts.TestPutUp(1, id, me, "haven");
			Check(second.m_bUp && second.m_eOwner == RZ_EMastOwner.HAVEN, string.Format("%1 handed to the Haven is the Haven's", second.m_sName));
			Check(RZ_Masts.FactionOf(second.m_eOwner) == RZ_EFaction.SURVIVORS, "the Haven's mast serves the Haven");

			array<int> ids = {id};
			array<IEntity> players = {me};
			Check(masts.StartVoice(ids, players), "a trapped voice starts calling with two masts up");
			masts.TookBearing(masts.GetMast(0), id);
			Check(masts.HasVoice(), "one bearing doesn't fix him");
			vector voice = masts.VoiceAt();
			masts.TookBearing(second, id);
			Check(!masts.HasVoice(), "two bearings fix him");
			Check(masts.m_bLastCallStarted, string.Format("a Distress Call starts where they cross (%1)", RZ_SettlementScanner.GridReference(voice)));
			if (masts.m_bLastCallStarted)
				Check(RZ_Util.DistXZ(masts.m_vLastCallAt, voice) < 5, string.Format("the call stands at the spot (%1 m off)", RZ_Util.RoundToInt(RZ_Util.DistXZ(masts.m_vLastCallAt, voice))));

			End();
			return true;
		}

		return true;
	}
}
