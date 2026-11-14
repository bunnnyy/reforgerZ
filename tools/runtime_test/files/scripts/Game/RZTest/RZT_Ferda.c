// Test-only (not part of the mod): the opening's fixer (Guidance/RZ_OpeningFixer.c, docs/DESIGN.md
// "The fixer"), on the fake player at the route's first town, after the line's check:
// - He's coming: the Red Hand's people leave the case to him (no sale reply) while he is.
// - He comes: Ferda and two men stand near the tester, "Talk to Ferda Kroupa" is tracked on him, and
//   his replies are hand it over, the story about Bastion (locked without the check) and the street.
// - Rally the street: the door's job stands at a real doorway; each job done hands the next; with all
//   done, standing with the street ends it: Haven Fame up, Red Hand Infamy up, he goes.
// - Hand it over: the case is sold (Get It Back is next), the neighbour can be warned, and warning him
//   ends it as WARNED.
// - Talk him off (the good liar): TALKED, no standing changed. Slip away: SLIPPED, Haven Infamy up.
// - A man of his down: FOUGHT, Red Hand Infamy up.
// - His note lies on the step of a real door; a veteran gets only the note.
// Lines: "RZT ferda OK|FAIL <what>" and "RZT ferda: N checks, M failed".

class RZT_Ferda
{
	static bool s_bDone;
	static int s_iSec = -1;
	static int s_iChecks;
	static int s_iFails;
	static int s_iStage;
	static int s_iWait;
	static int s_iHavenFame;
	static int s_iHavenInfamy;
	static int s_iRedInfamy;
	static vector s_vDoor;
	static vector s_vTown;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT ferda %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static void End()
	{
		Print(string.Format("RZT ferda: %1 checks, %2 failed", s_iChecks, s_iFails));
		RZ_ColdCaseComponent.s_TestCarried = null;
		RZ_ColdCaseComponent.s_TestHolder = null;
		s_bDone = true;
	}

	//------------------------------------------------------------------------------------------------
	//! A fresh visit: what this character did with him forgotten, a case in hand, him on his way, then here.
	static bool Fresh(RZ_GameModeComponent gm, RZ_OpeningFixer f, IEntity me, int level)
	{
		f.Forget(RZT_Fake.ID);
		RZ_PlayerProfile profile = gm.GetProfile(RZT_Fake.ID);
		profile.m_iFerdaWay = RZ_EFerdaWay.NONE;
		profile.m_iFerdaLife = -1;
		profile.m_iFerdaAfter = 0;
		profile.m_iCaseState = RZ_OpeningCase.STATE_CARRIED;
		profile.m_iCaseLife = RZ_OpeningCase.CharacterStamp(profile);
		RZT_Fake.MoveTo(s_vTown);
		GiveCase(gm, me);
		s_iHavenFame = profile.GetFame(RZ_EFaction.SURVIVORS);
		s_iHavenInfamy = profile.GetInfamy(RZ_EFaction.SURVIVORS);
		s_iRedInfamy = profile.GetInfamy(RZ_EFaction.BANDITS);
		f.Schedule(RZT_Fake.ID, s_vTown, "the test street", null, "", "", level);
		if (level >= 3)
			return true;

		return f.ComeNowAt(RZT_Fake.ID, me);
	}

	//------------------------------------------------------------------------------------------------
	static void GiveCase(RZ_GameModeComponent gm, IEntity me)
	{
		RZ_OpeningCase c = RZ_OpeningCase.Get();
		IEntity item = RZ_Util.SpawnPrefab(RZ_OpeningCase.CasePrefab(gm), me.GetOrigin() + Vector(0, 0.5, 0), 0);
		RZ_ColdCaseComponent cc = RZ_ColdCaseComponent.Of(item);
		if (cc)
			cc.SetTag(c.Tag(), RZT_Fake.ID);

		if (RZ_Util.InsertIntoInventory(me, item) || !item)
			return;

		RZ_ColdCaseComponent.s_TestHolder = me;
		RZ_ColdCaseComponent.s_TestCarried = item;
	}

	//------------------------------------------------------------------------------------------------
	static bool HasReply(array<ref RZ_TalkReply> replies, int id, out bool enabled)
	{
		enabled = false;
		foreach (RZ_TalkReply r : replies)
		{
			if (r.m_iId == id)
			{
				enabled = r.m_bEnabled;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A real outside door's doorway within `range` of pos.
	static bool DoorWithin(vector pos, float range)
	{
		IEntity door;
		vector centre;
		vector normal;
		return RZ_DoorGeometry.NearestOuterDoor(pos, 0, range, false, door, centre, normal);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from the diag's tick until done. False while it runs.
	static bool Tick(RZ_GameModeComponent gm, vector town)
	{
		if (s_bDone)
			return true;

		s_iSec++;
		RZ_OpeningFixer f = RZ_OpeningFixer.Get();
		IEntity me = RZT_Fake.s_Ent;
		if (!f || !me || !gm.GetErrands() || !gm.GetErrands().IsOn() || RZ_OpeningCase.CasePrefab(gm).IsEmpty())
		{
			Print("RZT ferda: skipped (the fixer, the errands, the case prefab or the tester missing)");
			s_bDone = true;
			return true;
		}

		RZ_PlayerProfile profile = gm.GetProfile(RZT_Fake.ID);
		if (s_iSec == 0)
		{
			s_vTown = town;
			Check(Fresh(gm, f, me, 0), "he comes for the tester (the full visit)");
			IEntity ferda = f.FerdaOf(RZT_Fake.ID);
			Check(ferda && RZ_Util.IsCharacterAlive(ferda) && RZ_Util.DistXZ(ferda.GetOrigin(), me.GetOrigin()) < 40, "Ferda stands within 40 m, alive");
			Check(f.CrewOf(RZT_Fake.ID) == 2, "with two men");
			RZ_Errand talk = f.StepFor(RZT_Fake.ID);
			Check(talk && ferda && RZ_Util.DistXZ(talk.m_vPlace, ferda.GetOrigin()) < 3, "\"Talk to Ferda Kroupa\" is tracked, its place on him");

			RZ_TalkSession rh = new RZ_TalkSession();
			rh.m_iPlayer = RZT_Fake.ID;
			rh.m_iFaction = RZ_EFaction.BANDITS;
			array<ref RZ_TalkReply> rhReplies = {};
			RZ_OpeningCase.Get().AddReplies(rh, me, rhReplies);
			bool sale = false;
			foreach (RZ_TalkReply rr : rhReplies)
			{
				if (rr.m_iId == RZ_OpeningCase.REPLY_SELL)
					sale = true;
			}

			Check(!sale, "while he's here, the Red Hand's people leave the sale to him");

			int code = f.CodeOf(RZT_Fake.ID, RZ_OpeningFixer.ROLE_FERDA);
			array<ref RZ_TalkReply> replies = {};
			f.Replies(code, RZT_Fake.ID, me, replies);
			bool on;
			Check(HasReply(replies, RZ_OpeningFixer.REPLY_HAND, on) && on, "his replies: hand it over (enabled with the case)");
			string lockText = "open";
			bool listed = HasReply(replies, RZ_OpeningFixer.REPLY_BLUFF, on);
			if (!on)
				lockText = "locked without the check";

			Check(listed, "the story about Bastion is there (" + lockText + ")");
			Check(HasReply(replies, RZ_OpeningFixer.REPLY_RALLY, on) && on, "and the street (the full visit)");

			array<string> said = {};
			f.Pick(code, RZT_Fake.ID, RZ_OpeningFixer.REPLY_RALLY, me, said);
			Check(f.PhaseOf(RZT_Fake.ID) == RZ_OpeningFixer.PHASE_RALLY, "asking the street: he waits, the street's first job is handed");
			s_iStage = 1;
			s_iWait = 0;
			return false;
		}

		// The street's jobs, one by one: at each, stand at its work site and finish it
		if (s_iStage == 1)
		{
			s_iWait++;
			RZ_WorkSites sites = RZ_WorkSites.Get();
			RZ_Errand step = f.StepFor(RZT_Fake.ID);
			if (f.PhaseOf(RZT_Fake.ID) == RZ_OpeningFixer.PHASE_STAND)
			{
				int code1 = f.CodeOf(RZT_Fake.ID, RZ_OpeningFixer.ROLE_FERDA);
				array<string> said1 = {};
				f.Pick(code1, RZT_Fake.ID, RZ_OpeningFixer.REPLY_STAND, me, said1);
				Check(f.WayOf(RZT_Fake.ID) == RZ_EFerdaWay.RALLIED, "the street stood with the tester: RALLIED");
				Check(profile.GetFame(RZ_EFaction.SURVIVORS) > s_iHavenFame, "Haven Fame up");
				Check(profile.GetInfamy(RZ_EFaction.BANDITS) > s_iRedInfamy, "Red Hand Infamy up");
				Check(f.PhaseOf(RZT_Fake.ID) == RZ_OpeningFixer.PHASE_LEAVING, "and he goes");
				s_iStage = 2;
				return false;
			}

			if (s_iWait > 40)
			{
				Check(false, "the street's three jobs didn't all end in 40 s (phase " + f.PhaseOf(RZT_Fake.ID).ToString() + ")");
				s_iStage = 2;
				return false;
			}

			if (!step || !sites)
				return false;

			string key = RZ_Errands.WorkKey(step);
			if (!sites.Has(key))
				return false;

			vector at = sites.PosOf(key);
			if (step.m_Def.m_sKey == "opening.fixer.board")
			{
				s_vDoor = at;
				Check(DoorWithin(at, 1.5), "the door's job stands at a real doorway (" + RZ_SettlementScanner.GridReference(at) + ")");
			}

			RZT_Fake.MoveTo(at);
			if (sites.CompleteFor(key, RZT_Fake.ID))
				Print("RZT ferda: done for the tester: " + step.m_Def.m_sKey);

			return false;
		}

		// Hand it over, then warn the street
		if (s_iStage == 2)
		{
			bool came = Fresh(gm, f, me, 0);
			Check(came, "he comes again (the hand-over)");
			int code2 = f.CodeOf(RZT_Fake.ID, RZ_OpeningFixer.ROLE_FERDA);
			array<string> said2 = {};
			f.Pick(code2, RZT_Fake.ID, RZ_OpeningFixer.REPLY_HAND, me, said2);
			Check(profile.m_iCaseState == RZ_OpeningCase.STATE_SOLD && !RZ_OpeningCase.Carries(me), "handed over: the case is sold, Get It Back is next");
			int phase = f.PhaseOf(RZT_Fake.ID);
			if (phase == RZ_OpeningFixer.PHASE_WARN)
			{
				int code3 = f.CodeOf(RZT_Fake.ID, RZ_OpeningFixer.ROLE_NEIGHBOUR);
				array<ref RZ_TalkReply> warn = {};
				f.Replies(code3, RZT_Fake.ID, me, warn);
				bool w;
				Check(HasReply(warn, RZ_OpeningFixer.REPLY_WARN, w) && w, "the neighbour can be warned");
				array<string> said3 = {};
				f.Pick(code3, RZT_Fake.ID, RZ_OpeningFixer.REPLY_WARN, me, said3);
				Check(f.WayOf(RZT_Fake.ID) == RZ_EFerdaWay.WARNED, "warned: WARNED");
			}
			else
			{
				Check(f.WayOf(RZT_Fake.ID) == RZ_EFerdaWay.HANDED, "no neighbour here: HANDED (phase " + phase.ToString() + ")");
			}

			s_iStage = 3;
			return false;
		}

		// Talk him off, as the good liar
		if (s_iStage == 3)
		{
			int habit = RZ_Habits.Of(profile);
			RZ_Habits.Set(profile, RZ_EPlayerHabit.LIAR);
			bool again = Fresh(gm, f, me, 0);
			int code4 = f.CodeOf(RZT_Fake.ID, RZ_OpeningFixer.ROLE_FERDA);
			array<ref RZ_TalkReply> bluff = {};
			f.Replies(code4, RZT_Fake.ID, me, bluff);
			bool b;
			Check(again && HasReply(bluff, RZ_OpeningFixer.REPLY_BLUFF, b) && b, "the good liar can tell him about Bastion");
			array<string> said4 = {};
			f.Pick(code4, RZT_Fake.ID, RZ_OpeningFixer.REPLY_BLUFF, me, said4);
			Check(f.WayOf(RZT_Fake.ID) == RZ_EFerdaWay.TALKED, "talked him off: TALKED");
			Check(profile.GetFame(RZ_EFaction.SURVIVORS) == s_iHavenFame && profile.GetInfamy(RZ_EFaction.BANDITS) == s_iRedInfamy, "no standing changed");
			RZ_Habits.Set(profile, habit);
			s_iStage = 4;
			return false;
		}

		// Slip away
		if (s_iStage == 4)
		{
			Fresh(gm, f, me, 0);
			f.SlipNow(RZT_Fake.ID);
			Check(f.WayOf(RZT_Fake.ID) == RZ_EFerdaWay.SLIPPED && profile.GetInfamy(RZ_EFaction.SURVIVORS) > s_iHavenInfamy, "slipped away: SLIPPED, Haven Infamy up");
			s_iStage = 5;
			return false;
		}

		// A fight: one of his down
		if (s_iStage == 5)
		{
			Fresh(gm, f, me, 0);
			IEntity him = f.FerdaOf(RZT_Fake.ID);
			if (him)
				RZ_Util.KillCharacter(him);
			s_iStage = 6;
			return false;
		}

		if (s_iStage == 6)
		{
			Check(f.WayOf(RZT_Fake.ID) == RZ_EFerdaWay.FOUGHT && profile.GetInfamy(RZ_EFaction.BANDITS) > s_iRedInfamy, "Ferda down: FOUGHT, Red Hand Infamy up");
			s_iStage = 7;
			return false;
		}

		// His note: on the step of a real door; a veteran gets only the note
		if (s_iStage == 7)
		{
			if (s_vDoor != vector.Zero)
				RZT_Fake.MoveTo(s_vDoor);

			profile.m_iFerdaAfter = 0;
			bool put = f.PutNote(RZT_Fake.ID, me);
			IEntity note = f.NoteOf(RZT_Fake.ID);
			Check(put && note && DoorWithin(note.GetOrigin(), 1.5), "his note lies on the step of a real door");
			if (note)
				Print("RZT ferda: the note reads: " + f.NoteText(RZT_Fake.ID));

			Fresh(gm, f, me, 3);
			Check(f.WayOf(RZT_Fake.ID) == RZ_EFerdaWay.NOTE && f.PhaseOf(RZT_Fake.ID) == RZ_OpeningFixer.PHASE_DONE, "a veteran gets only his note");
			Print("RZT ferda: the radio would say: " + RZ_OpeningFixer.RadioLine(RZ_EFerdaWay.RALLIED, "the test street"));
			End();
			return true;
		}

		return false;
	}
}
