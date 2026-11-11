// Test-only (not part of the mod): the cure finale's phase of the runtime test (docs/FINALE.md; the
// plan's "scripted runtime finale at Wait scale 0.1"). It runs last, after the route walk, because
// the cure it ends with stands the Perimeter down. With questlines' Wait scale at 0.1:
// 1. a commit is refused below the gate;
// 2. the slow finish: the lab-alone track makes the batch with nobody holding the room (then reset);
// 3. the gate opens the five routes; the tester locks to the Haven, a stand-in identity to Bastion
//    and another to the Red Hand; two routes race (the Haven and Bastion), Bastion and the Red Hand
//    make a pact;
// 4. the Haven finishes: the others freeze, the batch is made ready and sealed, the stand starts;
// 5. the stand is lost (nobody held the lab): research back to 90, the Haven's remake step makes the
//    batch again, and the second stand starts with the Haven holding the room;
// 6. a scripted take: the Bastion-Red Hand side takes the room; the Haven's count runs into
//    overtime (20 s here); the stand is won: the controller, how it was won, the release's terms.
// Lines: "RZT finale check: <name>: OK|FAILED (...)", "RZT finale done: N checks OK, N FAILED".

modded class RZ_CureFinale
{
	//! A fresh finale (test only): nothing open, nobody locked.
	void RZT_Reset()
	{
		m_S = new RZ_FinaleSave();
		Pad();
		m_Items = new RZ_FinaleItems(m_GM, this);
		m_People = new RZ_FinalePeople(m_GM, this);
		m_bStandOn = false;
		m_bStandPrep = false;
		m_bInOvertime = false;
		m_fOvertime = 0;
		ClearGroups();
	}

	//! Lock an identity to route r and make it active (2 hours played, online now).
	void RZT_Lock(string ident, int r)
	{
		if (m_S.lockIdent.Find(ident) < 0)
		{
			m_S.lockIdent.Insert(ident);
			m_S.lockRoute.Insert(r);
			m_S.lockUnix.Insert(RZ_SeasonClock.Now() - 60);
		}

		TrackPlayer(ident, 3 * 3600);
	}

	//! A pact by tally: one vote from each route's stand-in.
	void RZT_Pact(int a, int b, string identA, string identB)
	{
		m_S.votes.Insert(string.Format("%1|%2|%3", a, b, identA));
		m_S.votes.Insert(string.Format("%1|%2|%3", b, a, identB));
		CheckPact(a, b);
	}

	//! The batch hold is over and the seal calls are done: the seal comes on the next check.
	void RZT_SealNow()
	{
		if (!m_S.batchReady)
			BatchReady("runtime test");

		m_S.sealUnix = RZ_SeasonClock.Now() - 1;
		m_S.calls = 7;
		m_fCheck = CHECK_SECONDS;
	}

	void RZT_Overtime(float seconds)
	{
		m_Settings.m_fOvertimeSeconds = seconds;
	}

	void RZT_Count(int side, float seconds)
	{
		m_S.counts[side] = seconds;
	}

	float RZT_GetCount(int side)
	{
		return m_S.counts[side];
	}

	void RZT_Take(int side)
	{
		TakeRoom(side, HOW_FORCE);
	}

	bool RZT_IsSealed()
	{
		return m_S.isSealed;
	}

	bool RZT_InOvertime()
	{
		return m_bInOvertime;
	}
}

modded class RZ_CureResearch
{
	//! Research at the gate (test only): 90 percent, the gate shut, no stand, not cured.
	void RZT_Gate()
	{
		m_bCured = false;
		m_bFinalStand = false;
		m_bGateOpen = false;
		m_fProgress = 90;
		m_iMilestone = 3;
		m_GM.OnCureChanged();
	}

	void RZT_StandLeft(float seconds)
	{
		m_fFinalStandLeft = seconds;
		m_bStandReached = true;
	}

	void RZT_Lose()
	{
		LoseBatch("The batch is lost", "The runtime test ruined the batch.");
	}
}

modded class RZ_Questlines
{
	void RZT_WaitScale(float scale)
	{
		m_Settings.m_fWaitScale = scale;
	}

	void RZT_Routes()
	{
		TryStartRoutes();
	}
}

class RZT_Finale
{
	protected static int s_iStep;
	protected static int s_iSec;
	protected static int s_iOk;
	protected static int s_iFailed;

	protected static const string BASTION_ID = "rzt_bastion";
	protected static const string REDHAND_ID = "rzt_redhand";

	//------------------------------------------------------------------------------------------------
	protected static void Check(string name, bool ok, string detail)
	{
		if (ok)
		{
			s_iOk++;
			Print(string.Format("RZT finale check: %1: OK", name));
			return;
		}

		s_iFailed++;
		Print(string.Format("RZT finale check: %1: FAILED (%2)", name, detail));
	}

	//------------------------------------------------------------------------------------------------
	//! Every second of the finale's phase. True when it's done.
	static bool Tick(RZ_GameModeComponent gm)
	{
		RZ_CureFinale f = RZ_CureFinale.Get();
		RZ_Questlines q = gm.GetQuestlines();
		RZ_CureResearch cure = gm.GetCure();
		if (!f || !q || !q.IsEnabled() || !RZ_CureFinale.IsOn() || !gm.HasLab())
		{
			Print("RZT finale: the finale or questlines are off (or no lab), skipped");
			return true;
		}

		s_iSec++;
		switch (s_iStep)
		{
			case 0: return Start(gm, f, q, cure);
			case 1: return SlowFinish(gm, f, q, cure);
			case 2: return Gate(gm, f, q, cure);
			case 3: return Race(gm, f, q, cure);
			case 4: return FirstStand(gm, f, q, cure);
			case 5: return Remake(gm, f, q, cure);
			case 6: return SecondStand(gm, f, q, cure);
			case 7: return Release(gm, f, q, cure);
		}

		Print(string.Format("RZT finale done: %1 checks OK, %2 FAILED", s_iOk, s_iFailed));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Next()
	{
		s_iStep++;
		s_iSec = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! 1. Wait scale 0.1; a commit below the gate is refused.
	protected static bool Start(RZ_GameModeComponent gm, RZ_CureFinale f, RZ_Questlines q, RZ_CureResearch cure)
	{
		q.RZT_EndAll();
		q.RZT_WaitScale(0.1);
		f.RZT_Reset();
		f.RZT_Overtime(20);
		cure.RZT_SetProgress(85);
		string why = f.CanCommit(RZT_Fake.ID, RZ_CureFinale.HAVEN);
		Check("commit refused below the gate", !why.IsEmpty(), "no refusal");
		Print("RZT finale: phase starts (Wait scale 0.1)");
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 2. The slow finish: the lab-alone track to 10 makes the batch, nobody holds the room.
	protected static bool SlowFinish(RZ_GameModeComponent gm, RZ_CureFinale f, RZ_Questlines q, RZ_CureResearch cure)
	{
		cure.RZT_Gate();
		f.OnGate();
		f.AddLabAlone(10);
		Check("slow finish: the lab alone finished", f.Finisher() == RZ_CureFinale.LAB, string.Format("finisher %1", f.Finisher()));
		Check("slow finish: nobody holds the room", f.Holder() == RZ_CureFinale.NONE, string.Format("holder %1", f.Holder()));
		Check("slow finish: cure.slow set", RZ_WorldFacts.Get().Flag("cure.slow") == 1, "flag not set");
		q.RZT_EndAll();
		q.RZT_Forget("route_haven");
		q.RZT_Forget("route_bastion");
		q.RZT_Forget("route_redhand");
		q.RZT_Forget("route_halcyon");
		q.RZT_Forget("route_one_each");
		RZ_WorldFacts.Get().SetFlag("cure.slow", 0);
		f.RZT_Reset();
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 3. The gate: the routes open and begin; three locks; two routes racing; a pact.
	protected static bool Gate(RZ_GameModeComponent gm, RZ_CureFinale f, RZ_Questlines q, RZ_CureResearch cure)
	{
		if (s_iSec == 1)
		{
			cure.RZT_Gate();
			f.OnGate();
			q.RZT_Routes();
			return false;
		}

		if (s_iSec < 5)
			return false;

		Check("the routes open at the gate", f.IsOpen(), "not open");
		int running = 0;
		for (int r = 0; r < RZ_CureFinale.ROUTES; r++)
		{
			if (q.IsRunning(RZ_CureFinale.RouteKey(r)))
				running++;
		}

		Check("the five routes begin", running == RZ_CureFinale.ROUTES, string.Format("%1 running", running));
		bool locked = f.Commit(RZT_Fake.ID, RZ_CureFinale.HAVEN);
		Check("the tester locks to the Haven", locked && f.RouteOfPlayer(RZT_Fake.ID) == RZ_CureFinale.HAVEN, "lock failed");
		Check("a second lock is refused", !f.CanCommit(RZT_Fake.ID, RZ_CureFinale.BASTION).IsEmpty(), "no refusal");
		f.RZT_Lock(gm.GetProfile(RZT_Fake.ID).m_sIdentity, RZ_CureFinale.HAVEN);
		f.RZT_Lock(BASTION_ID, RZ_CureFinale.BASTION);
		f.RZT_Lock(REDHAND_ID, RZ_CureFinale.REDHAND);
		f.RZT_Pact(RZ_CureFinale.BASTION, RZ_CureFinale.REDHAND, BASTION_ID, REDHAND_ID);
		Check("a pact by tally", f.PactOf(RZ_CureFinale.BASTION) == RZ_CureFinale.REDHAND, string.Format("pact %1", f.PactOf(RZ_CureFinale.BASTION)));
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 3b. Two routes racing: Bastion to 5, the Haven to 9 then its last step. The others freeze.
	protected static bool Race(RZ_GameModeComponent gm, RZ_CureFinale f, RZ_Questlines q, RZ_CureResearch cure)
	{
		f.AddStep(RZ_CureFinale.BASTION, 2, "runtime test, act I");
		f.AddStep(RZ_CureFinale.BASTION, 3, "runtime test, the piece");
		f.AddStep(RZ_CureFinale.HAVEN, 9, "runtime test, acts I-III");
		Check("a track never passes 9 before its last step", f.Track(RZ_CureFinale.HAVEN) == 9, string.Format("track %1", f.Track(RZ_CureFinale.HAVEN)));
		Check("the cure shows 90 plus the furthest track", RZ_Util.RoundToInt(cure.ShownProgress()) == 99, string.Format("shown %1", cure.ShownProgress()));
		f.AddStep(RZ_CureFinale.HAVEN, 3, "runtime test, the last step", true);
		Check("the Haven finishes", f.Finisher() == RZ_CureFinale.HAVEN && f.Holder() == RZ_CureFinale.HAVEN, string.Format("finisher %1, holder %2", f.Finisher(), f.Holder()));
		RZ_QuestRun bastion = q.FindRunByKey("route_bastion");
		Check("the other routes freeze", !bastion || bastion.m_iStage == RZ_FinaleRoute.S_AFTER, "Bastion's route still racing");
		f.AddStep(RZ_CureFinale.BASTION, 1, "after the finish");
		Check("no points after the finish", f.Track(RZ_CureFinale.BASTION) == 5, string.Format("Bastion %1", f.Track(RZ_CureFinale.BASTION)));
		f.RZT_SealNow();
		RZT_Fake.MoveTo(gm.GetLabPosition());
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 4. The seal and the first stand; then it's lost.
	protected static bool FirstStand(RZ_GameModeComponent gm, RZ_CureFinale f, RZ_Questlines q, RZ_CureResearch cure)
	{
		if (!cure.IsFinalStand())
		{
			if (s_iSec > 30)
			{
				Check("the seal starts the stand", false, "no stand after 30 s");
				Next();
				Next();
				Next();
			}

			return false;
		}

		Check("the seal starts the stand", f.RZT_IsSealed() && RZ_Util.RoundToInt(cure.m_fProgress) == 100, string.Format("research %1", cure.m_fProgress));
		Check("the finisher holds the room", f.Holder() == RZ_CureFinale.HAVEN, string.Format("holder %1", f.Holder()));
		cure.RZT_Lose();
		Check("a lost stand: research back to 90", RZ_Util.RoundToInt(cure.m_fProgress) == 90 && !cure.IsFinalStand(), string.Format("research %1", cure.m_fProgress));
		Check("a lost stand: the finisher stays finished", f.Finisher() == RZ_CureFinale.HAVEN, "finisher changed");
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 5. The Haven's remake step (its wait fast-forwarded) makes the batch ready again.
	protected static bool Remake(RZ_GameModeComponent gm, RZ_CureFinale f, RZ_Questlines q, RZ_CureResearch cure)
	{
		RZ_QuestRun run = q.FindRunByKey("route_haven");
		if (run && run.m_iStage == RZ_FinaleRoute.S_REMAKE)
		{
			run.m_fStageAt = run.m_fStageAt - 3600;
			run.m_iStageUnix = run.m_iStageUnix - 86400;
		}

		if (!f.IsBatchReady())
		{
			if (s_iSec > 60)
			{
				Check("the remake makes the batch again", false, string.Format("route stage %1", RunStage(run)));
				f.RZT_SealNow();
				Next();
			}

			return false;
		}

		Check("the remake makes the batch again", true, "");
		f.RZT_SealNow();
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static int RunStage(RZ_QuestRun run)
	{
		if (!run)
			return -1;

		return run.m_iStage;
	}

	//------------------------------------------------------------------------------------------------
	//! 6. The second stand: a scripted take by the Bastion-Red Hand side, the Haven's count into
	//! overtime, then the win.
	protected static bool SecondStand(RZ_GameModeComponent gm, RZ_CureFinale f, RZ_Questlines q, RZ_CureResearch cure)
	{
		if (!cure.IsFinalStand() && !cure.IsCured())
		{
			if (s_iSec > 30)
			{
				Check("the second stand starts", false, "no stand after 30 s");
				Next();
			}

			return false;
		}

		if (cure.IsCured())
		{
			Check("the stand is won after overtime", f.RZT_InOvertime(), "no overtime ran");
			Next();
			return false;
		}

		// The tester stands in the clean room: the Haven's side counts toward its take
		RZT_Fake.MoveTo(f.RoomPos());
		if (f.Holder() == RZ_CureFinale.HAVEN && f.RZT_GetCount(RZ_CureFinale.BASTION) == 0 && s_iSec < 10)
		{
			Check("the finisher holds the room again", true, "");
			f.RZT_Take(f.SideOf(RZ_CureFinale.BASTION));
			Check("a scripted take", f.Holder() == RZ_CureFinale.BASTION, string.Format("holder %1", f.Holder()));
			// The Haven (the tester, in the room) counts toward its take; the 15 minutes end now
			cure.RZT_StandLeft(2);
			return false;
		}

		if (s_iSec > 120)
		{
			Check("the stand is won after overtime", false, "not won after 2 minutes");
			Next();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 7. The release: the controller's route chooses terms (its scene ended as won with terms 1).
	protected static bool Release(RZ_GameModeComponent gm, RZ_CureFinale f, RZ_Questlines q, RZ_CureResearch cure)
	{
		if (s_iSec == 1)
		{
			int c = f.Controller();
			Check("a controller", c != RZ_CureFinale.NONE, "nobody controls");
			Check("how it was won", f.How() > 0, "no how");
			Check("the winner is the controller's", gm.GetSeasonWinner() == f.ControllerFaction(), string.Format("winner %1, controller %2", gm.GetSeasonWinner(), c));
			Check("the route's chunk went to season Fame", gm.GetSeasonFame(RZ_CureFinale.HAVEN) >= f.Worth() / 2, string.Format("Haven %1, worth %2", gm.GetSeasonFame(RZ_CureFinale.HAVEN), f.Worth()));
		}

		RZ_QuestRun run = q.FindRunByKey(RZ_CureFinale.RouteKey(f.Controller()));
		if (run && run.m_iMissionId >= 0)
		{
			RZ_Mission m = gm.GetMissions().FindById(run.m_iMissionId);
			if (m && m.m_eState == RZ_EMissionState.ACTIVE)
			{
				run.SetOutcome(RZ_FinaleRoute.OUT_TERMS + 1);
				m.m_eState = RZ_EMissionState.SUCCEEDED;
			}
		}

		if (f.Terms() > 0)
		{
			Check("the terms are chosen", true, "");
			Next();
			return false;
		}

		if (s_iSec > 240)
		{
			Check("the terms are chosen", false, string.Format("route run %1", run != null));
			Next();
		}

		return false;
	}
}
