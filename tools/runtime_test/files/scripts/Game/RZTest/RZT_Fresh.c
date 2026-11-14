// Test-only (not part of the mod): a fresh spawn (the author's play-test: "just spawned in, can't
// see anyone and first mission is already skipped; I can still see 2 other quests I didn't start").
// Once, early, on the fake player, as a veteran (guidance level 3, where the faults showed):
// - The last character's jobs aren't this one's: a questline part from the last character doesn't make the
//   questline visible to the new one, and no running job keeps the player as participant or offer.
// - The opening: its contact stands within 15 m of where the player comes out, and its first
//   stage is the door's favour (a veteran's skips it once talked to: RZT_OpeningChain), not the lead
//   to the nearest work.
// - The Quests tab: no job of any kind is listed as taken.
// Lines: "RZT fresh OK|FAIL <what>" and "RZT fresh: N checks, M failed".

class RZT_Fresh
{
	static bool s_bDone;
	static int s_iChecks;
	static int s_iFails;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT fresh %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	//! Later, once a questline runs: the last character's part in it isn't the new character's (no questline
	//! runs at the spawn check's second). Tried each call until one is found; the part it adds and
	//! the life it bumps are put back.
	static bool s_bPartDone;
	static void PartTick(RZ_GameModeComponent gm)
	{
		if (s_bPartDone || !s_bDone)
			return;

		RZ_Questlines q = gm.GetQuestlines();
		if (!q || !q.IsEnabled())
			return;

		RZ_QuestRun run = null;
		foreach (RZ_QuestRun r : q.GetRuns())
		{
			if (r && !r.m_bDone && r.m_Line && !r.m_Line.IsMajor() && !r.m_Line.IsCureFinale() && !r.PlayerTookPart(gm, RZT_Fake.ID))
			{
				run = r;
				break;
			}
		}

		if (!run)
			return;

		s_bPartDone = true;
		int id = RZT_Fake.ID;
		RZ_PlayerProfile profile = gm.GetProfile(id);
		int lives = profile.m_iCharacter;
		int partsBefore = run.m_aParts.Count();
		run.Did(gm, id, RZ_EQuestApproach.NONE, "the last character was there");
		Check(run.PlayerTookPartThisCharacter(gm, id), "a part this character counts as this character's (" + run.m_Line.m_sTitle + ")");
		Check(q.IsVisibleTo(run, id), "the questline is in the journal of the life that took part");
		// A new character as the game makes one: the character count goes up, and the mission manager takes the
		// player off every running job (RZ_MissionManager.OnNewLife), the stage's mission included.
		// Only this run's stage mission is touched here, and put back after, so the route goes on
		profile.m_iCharacter = lives + 1;
		RZ_Mission stage = gm.GetMissions().FindById(run.m_iMissionId);
		bool wasIn = false;
		bool wasOffered = false;
		if (stage)
		{
			wasIn = stage.m_aParticipants.Contains(id);
			wasOffered = stage.m_aOfferedTo.Contains(id);
			stage.m_aParticipants.RemoveItem(id);
			stage.m_aOfferedTo.RemoveItem(id);
		}

		Check(!run.PlayerTookPartThisCharacter(gm, id), "a new character doesn't inherit the last one's part");
		Check(!q.IsVisibleTo(run, id), "the questline isn't in the new character's journal");
		Check(run.PlayerTookPart(gm, id), "the story still remembers the player by identity");
		if (stage && wasIn)
			stage.m_aParticipants.Insert(id);

		if (stage && wasOffered)
			stage.m_aOfferedTo.Insert(id);

		profile.m_iCharacter = lives;
		if (run.m_aParts.Count() > partsBefore)
			run.m_aParts.Remove(run.m_aParts.Count() - 1);

		Print(string.Format("RZT fresh: %1 checks, %2 failed (with the questline part)", s_iChecks, s_iFails));
	}

	//------------------------------------------------------------------------------------------------
	//! Once, from RZT_Tick. Always true (it never holds the route up).
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		s_bDone = true;
		IEntity me = RZT_Fake.s_Ent;
		int id = RZT_Fake.ID;
		if (!me || !gm.GetGuidance() || !gm.GetGuidance().GetOpenings())
		{
			Print("RZT fresh: skipped (no fake player or no openings)");
			return true;
		}

		RZ_PlayerProfile profile = gm.GetProfile(id);
		int guideLives = profile.m_iGuideLives;
		int lives = profile.m_iCharacter;
		profile.m_iGuideLives = 20;

		// 1. The last character took part in a questline; the new character doesn't see it
		RZ_Questlines q = gm.GetQuestlines();
		RZ_QuestRun run = null;
		if (q && q.IsEnabled())
		{
			foreach (RZ_QuestRun r : q.GetRuns())
			{
				if (r && !r.m_bDone && r.m_Line && !r.m_Line.IsMajor() && !r.m_Line.IsCureFinale())
				{
					run = r;
					break;
				}
			}
		}

		int partsBefore = 0;
		if (run)
		{
			partsBefore = run.m_aParts.Count();
			run.Did(gm, id, RZ_EQuestApproach.NONE, "the last character was there");
			Check(run.PlayerTookPartThisCharacter(gm, id), "a part this character counts as this character's");
			profile.m_iCharacter = lives + 1;
			Check(!run.PlayerTookPartThisCharacter(gm, id), "a new character doesn't inherit the last one's part in " + run.m_Line.m_sTitle);
		}
		else
		{
			Print("RZT fresh: no questline running yet, the part check skipped");
		}

		// 2. Running jobs drop the player at a new character
		RZ_MissionManager mm = gm.GetMissions();
		if (mm)
		{
			mm.OnNewLife(id);
			if (run)
				Check(!q.IsVisibleTo(run, id), "the questline isn't in the new character's journal: " + run.m_Line.m_sTitle);

			array<RZ_Mission> list = {};
			mm.ListFor(id, list);
			int kept = 0;
			foreach (RZ_Mission m : list)
			{
				if (m.m_aParticipants.Contains(id) || m.m_aOfferedTo.Contains(id))
					kept++;
			}

			Check(kept == 0, string.Format("no running job keeps the new character as participant or offer (%1)", kept));
		}

		// 3. The opening: a contact at the door and step 1 first
		RZ_OpeningPlace place = new RZ_OpeningPlace();
		place.m_vPos = me.GetOrigin();
		place.m_iTown = gm.FindNearestSettlement(place.m_vPos);
		RZ_Background bg = gm.GetBackground("nurse");
		if (!bg)
			bg = gm.GetDefaultBackground();

		RZ_Openings openings = gm.GetGuidance().GetOpenings();
		openings.Begin(id, me, profile, bg, place);
		int first;
		IEntity contact = openings.ContactOf(id, first);
		float dist = -1;
		if (contact)
			dist = RZ_Util.DistXZ(contact.GetOrigin(), me.GetOrigin());

		Check(contact != null, "a veteran's opening has its contact standing");
		Check(contact && dist <= 15, string.Format("the contact stands within 15 m (%1 m)", dist.ToString(4, 1)));
		Check(first == RZ_EOpeningStage.FAVOUR, string.Format("the chain starts at the door's favour (stage %1; a veteran's skips it once talked to), not the lead to work", first));

		// 4. The Quests tab: nothing taken but what the opening hands
		RZ_QuestTracker tracker = null;
		if (gm.GetUIFeed())
			tracker = gm.GetUIFeed().GetTracker();

		if (tracker)
		{
			array<ref RZ_QuestJob> jobs = {};
			tracker.Jobs(id, me, jobs);
			int taken = 0;
			string names = "";
			foreach (RZ_QuestJob j : jobs)
			{
				if (j.m_bTaken && j.m_iKind != RZ_EMissionKindUI.ERRAND)
				{
					taken++;
					names += " '" + j.m_sTitle + "'";
				}
			}

			Check(taken == 0, string.Format("a fresh character's Quests tab has no taken job but the opening's (%1:%2)", taken, names));
		}

		openings.Close(id, "the fresh-spawn check is done");
		// The test's own part goes again, so the route's questline checks run as before
		if (run && run.m_aParts.Count() > partsBefore)
			run.m_aParts.Remove(run.m_aParts.Count() - 1);

		profile.m_iGuideLives = guideLives;
		profile.m_iCharacter = lives;
		Print(string.Format("RZT fresh: %1 checks, %2 failed", s_iChecks, s_iFails));
		return true;
	}
}
