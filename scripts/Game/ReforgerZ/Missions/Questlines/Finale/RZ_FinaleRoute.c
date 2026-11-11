//! The finale's routes (docs/FINALE.md): five questlines that open at the 90 percent gate, in a
//! lane of their own (outside Max active and the faction slots). They never fizzle (a give-up tries
//! the stage again), freeze at another route's finish and end at the cure. Every route has the same
//! four acts:
//!   I    open, +2   the question: what's missing, where it is, who holds the door
//!   II   sworn, +3  the door (or its slower other way) +1, the piece +2
//!   III  open, +2   live tissue (the controller's tissue hunts; RZ_CureFinale.TickTissue)
//!   IV   sworn, +3  bring it in, the last step at the lab (needs a tray in the lab) -> the finish
//! Open work: anyone not locked to another route who can side with the route's faction (anyone at
//! all for the Independent) decides it. Sworn work: only the route's locked players. Every stage
//! is a scene (RZ_QM_RouteScene: people at places with choices); a choice's "do:<code>" runs the
//! route's own code (items, people, trays). Route stages never claim the lab.
//!
//! This file also holds the desks (the commit node's fallback: "What would it mean?", then "Write
//! me down" within 30 s; pacts; price doors; giving back what you found) and the tissue hunts.

//------------------------------------------------------------------------------------------------
//! A route's stage scene: the majors' scene with the finale's gates.
class RZ_QM_RouteScene : RZ_QM_MajorScene
{
	int m_iRoute = -2;
	bool m_bSworn;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		// Sworn work isn't on the radio: the route's locked players are told (RZ_FinaleRoute)
		if (m_bSworn)
		{
			m_bInPersonOnly = true;
			m_Req.m_bLeaderDecides = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_FinaleRoute RouteLine(RZ_QuestRun run)
	{
		if (!run)
			return null;

		return RZ_FinaleRoute.Cast(run.m_Line);
	}

	//------------------------------------------------------------------------------------------------
	//! The finale's gates first (open or sworn, the route's own checks), then the scene's own.
	override protected bool Allowed(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, RZ_SceneChoice c)
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (!finale)
			return false;

		string why = finale.MayDecide(playerId, m_iRoute, m_bSworn);
		if (!why.IsEmpty())
		{
			gm.NotifyPlayer(playerId, m_sTitle, why);
			return false;
		}

		RZ_FinaleRoute line = RouteLine(run);
		if (line && c.m_sFlag.StartsWith("do:"))
		{
			string refusal = line.CanDo(gm, run, playerId, c.m_sFlag.Substring(3, c.m_sFlag.Length() - 3));
			if (!refusal.IsEmpty())
			{
				gm.NotifyPlayer(playerId, m_sTitle, refusal);
				return false;
			}
		}

		return super.Allowed(gm, run, playerId, c);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Apply(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int spot, RZ_SceneChoice c)
	{
		RZ_FinaleRoute line = RouteLine(run);
		int decide = -1;
		if (line && c.m_sFlag.StartsWith("do:"))
			decide = line.DoIt(gm, run, playerId, c.m_sFlag.Substring(3, c.m_sFlag.Length() - 3));

		super.Apply(gm, run, playerId, spot, c);
		if (decide < 0)
			return;

		string what = c.m_sWhat;
		if (what.IsEmpty())
			what = c.m_sLabel;

		Resolve(gm, playerId, c.m_iApproach, decide, what);
	}

	//------------------------------------------------------------------------------------------------
	//! A choice with its own code ("do:", a draw, a hand-in) can be taken again by the same player.
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		super.OnChoice(gm, playerId, objIdx, option, source);
		RZ_SceneChoice c = FindChoice(objIdx, option);
		if (c && c.m_sFlag.StartsWith("do:"))
			m_mDone.Remove(string.Format("%1|%2|%3", gm.GetProfile(playerId).m_sIdentity, objIdx, option));
	}

	//------------------------------------------------------------------------------------------------
	//! Who decided the stage: kept on the run for the route's pay.
	override protected void Resolve(RZ_GameModeComponent gm, int playerId, int approach, int outcome, string what, bool success = true)
	{
		RZ_QuestRun run = Run(gm);
		if (run && playerId > 0 && !m_bResolved)
			run.SetText("decider", gm.GetProfile(playerId).m_sIdentity);

		super.Resolve(gm, playerId, approach, outcome, what, success);
	}
}

//------------------------------------------------------------------------------------------------
//! The base for the five routes.
class RZ_FinaleRoute : RZ_Questline
{
	static const int S_ACT1 = 100;
	static const int S_DOOR = 110;
	static const int S_PIECE = 120;
	static const int S_TISSUE = 130;
	static const int S_ACT4 = 140;
	static const int S_AFTER = 150;
	static const int S_REMAKE = 160;
	static const int S_RELEASE = 170;

	static const int OUT_ACT1 = 10;
	static const int OUT_DOOR = 20;		// through the door
	static const int OUT_OTHER = 21;	// the other way
	static const int OUT_PIECE = 30;
	static const int OUT_LAST = 40;
	static const int OUT_TERMS = 50;	// + the terms (51-54)
	static const int OUT_TERMS_DEFAULT = 59;

	static const int E_CONTROLLED = 10;
	static const int E_FINISHED_LOST = 11;
	static const int E_NEVER = 12;
	static const int E_HANDED = 13;

	int m_iRoute = -2;

	//------------------------------------------------------------------------------------------------
	override bool IsCureFinale()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Never offered by the half-hourly roll: the controller's lane starts the routes at the gate.
	override bool StartsOnItsOwn()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! In Define(): the route's key, title and faction, and the shared stages with the route's texts.
	protected void Route(int r)
	{
		m_iRoute = r;
		Info(RZ_CureFinale.RouteKey(r), RZ_CureFinale.RouteTitle(r), RZ_CureFinale.RouteFaction(r));
		MissionStage(S_ACT1, StageTitle(S_ACT1), StageKnown(S_ACT1)).GiveUp(6, "");
		MissionStage(S_DOOR, StageTitle(S_DOOR), StageKnown(S_DOOR)).GiveUp(6, "");
		MissionStage(S_PIECE, StageTitle(S_PIECE), StageKnown(S_PIECE)).GiveUp(6, "");
		WaitStage(S_TISSUE, "Live tissue", "The route's piece is in. Act IV waits for live tissue in the lab's isolation room.").Wait(0);
		MissionStage(S_ACT4, StageTitle(S_ACT4), StageKnown(S_ACT4)).GiveUp(6, "");
		WaitStage(S_AFTER, "The batch", "The route's work is done or frozen. What's left is the stand.").Wait(0);
		WaitStage(S_REMAKE, "Again", RemakeText()).Wait(RemakeMinutes(), RemakeHours());
		MissionStage(S_RELEASE, "The release", "The duty officer brings the release form to the clean room. The route's people choose the terms.").GiveUp(1, "");
	}

	//------------------------------------------------------------------------------------------------
	//! The route's stage titles and journal lines.
	protected string StageTitle(int stage)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected string StageKnown(int stage)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The remake step after a lost stand (the finisher only).
	protected string RemakeText()
	{
		return "The stand was lost. The route makes the batch again.";
	}

	protected float RemakeMinutes() { return 120; }
	protected float RemakeHours() { return 24; }

	//------------------------------------------------------------------------------------------------
	//! Sworn: Acts II and IV (only the route's locked players decide them).
	bool IsSworn(int stage)
	{
		return stage == S_DOOR || stage == S_PIECE || stage == S_ACT4 || stage == S_RELEASE;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_CureFinale Finale()
	{
		return RZ_CureFinale.Get();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_CureFinale f = Finale();
		if (!f || !f.IsOpen() || !gm.HasLab() || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return false;

		run.SetPlace(gm.GetLabPosition(), "the lab");
		return Places(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	//! Pick the route's people and places (run.SetText...). False: not now.
	protected bool Places(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		// The route's locked players hear their sworn work is on; the open acts go on the radio
		RZ_QuestStage st = GetStage(stage);
		RZ_CureFinale f = Finale();
		if (!st || !f || !IsSworn(stage) || stage == S_RELEASE)
			return;

		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int pid : all)
		{
			if (f.RouteOfPlayer(pid) == m_iRoute)
				gm.NotifyPlayer(pid, m_sTitle, run.Fill(st.m_sTitle + ": " + st.m_sKnown));
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_CureFinale f = Finale();
		if (!f)
			return false;

		switch (stage)
		{
			case S_TISSUE: return f.TissueIn(m_iRoute) && !f.IsFrozen();
			case S_AFTER: return false;
			case S_REMAKE: return true;
			case S_RELEASE: return true;
			case S_ACT4:
			{
				if (f.IsFrozen() || !f.IsOpen() || !f.TrayForFinish(m_iRoute))
					return false;

				return Window(gm, run, stage);
			}
		}

		if (f.IsFrozen() || !f.IsOpen())
			return false;

		return Window(gm, run, stage);
	}

	//------------------------------------------------------------------------------------------------
	//! The route's own time windows (dawn, dusk, the night shift). Default: any time.
	protected bool Window(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! True between from and to (game hours), or when the clock isn't known.
	protected static bool Between(float from, float to)
	{
		if (!RZ_GameTime.IsCalibrated())
			return true;

		float h = RZ_GameTime.GetHour();
		if (from <= to)
			return h >= from && h < to;

		return h >= from || h < to;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_QM_RouteScene s = new RZ_QM_RouteScene();
		s.m_iRoute = m_iRoute;
		s.m_bSworn = IsSworn(stage);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	override bool BuildScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		scene.m_iGiver = RZ_CureFinale.PayFaction(m_iRoute);
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iFame = 0;
		scene.m_iFavor = 0;
		scene.m_fWindow = 5400;
		if (stage == S_RELEASE)
		{
			scene.m_bLate = true;
			scene.m_fWindow = 180;
			scene.m_iTimeout = OUT_TERMS_DEFAULT;
			return ReleaseScene(gm, run, scene);
		}

		return RouteScene(gm, run, stage, scene);
	}

	//------------------------------------------------------------------------------------------------
	//! The route's scene for S_ACT1, S_DOOR, S_PIECE and S_ACT4.
	protected bool RouteScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The release: the route's terms (outcomes OUT_TERMS + 1..4).
	protected bool ReleaseScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The terms nobody chose: the stronghold leader's traits pick. Default 1.
	int DefaultTerms(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The route's lines when it controls (by terms), finished and lost, never finished.
	protected string ControlledLine(int terms) { return ""; }
	protected string LostLine() { return ""; }
	protected string NeverLine() { return ""; }

	//------------------------------------------------------------------------------------------------
	//! A choice's own code ("do:<code>"): "" if it can happen now, else why not. Shared codes:
	//!   food3                      three food for a hungry house (taken here)
	//!   give:<kind>:<seconds>      a route item into the player's hands (seconds out of the cold; -1 none)
	//!   store:<kind>:<need>:<out>  hand in one carried <kind>; at <need> stored the stage is <out>
	//!   wait:<flag>                the real time in run flag <flag> (unix) has passed
	string CanDo(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		array<string> parts = {};
		code.Split(":", parts, false);
		string head = parts[0];
		if (head == "food3")
		{
			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!me || !RZ_HavenQuest.TakeFood(gm, me, 3))
				return "That takes three food.";

			return "";
		}

		if (head == "store" && parts.Count() >= 2)
		{
			if (Carries(gm, playerId, parts[1]) <= 0)
				return "You haven't got it with you.";

			return "";
		}

		if (head == "wait" && parts.Count() >= 2)
		{
			if (RZ_SeasonClock.Now() < run.Flag(parts[1]))
				return "\"Not yet. The work takes the time it takes.\"";

			return "";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A choice's own code, after its checks passed. Returns an outcome that decides the stage now,
	//! or -1.
	int DoIt(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		array<string> parts = {};
		code.Split(":", parts, false);
		string head = parts[0];
		if (head == "give" && parts.Count() >= 3)
		{
			GiveItem(gm, playerId, parts[1], ItemName(parts[1]), PlayerPos(playerId), ColdFor(gm, playerId, parts[2].ToFloat()));
			return -1;
		}

		if (head == "store" && parts.Count() >= 4)
		{
			HandIn(gm, playerId, parts[1], 1);
			string key = "stored." + parts[1];
			int stored = run.Flag(key) + 1;
			run.SetFlag(key, stored);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The lab signs it into the cold store: %1 of %2.", stored, parts[2]));
			if (stored >= parts[2].ToInt())
				return parts[3].ToInt();
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A route item's name for the player ("a bag of her blood").
	protected string ItemName(string kind)
	{
		return kind;
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds out of the cold: +10 minutes with a Nurse or Doctor carrying. -1: no clock.
	protected float ColdFor(RZ_GameModeComponent gm, int playerId, float seconds)
	{
		if (seconds < 0)
			return -1;

		if (RZ_QuestApproach.HasBackground(gm, playerId, "nurse,doctor"))
			return seconds + 600;

		return seconds;
	}

	//------------------------------------------------------------------------------------------------
	protected vector PlayerPos(int playerId)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me)
			return vector.Zero;

		return me.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	protected string IdentOf(RZ_GameModeComponent gm, int playerId)
	{
		return gm.GetProfile(playerId).m_sIdentity;
	}

	// ================================================================ How it goes on
	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		RZ_CureFinale f = Finale();
		if (!f)
			return END;

		switch (stage)
		{
			case S_ACT1:
			{
				if (outcome != OUT_ACT1)
					return S_ACT1;

				AfterStep(gm, run, stage, outcome);
				Pay(gm, run, stage, false);
				f.MarkActOne(m_iRoute);
				f.AddStep(m_iRoute, 2, "act I");
				return S_DOOR;
			}
			case S_DOOR:
			{
				if (outcome != OUT_DOOR && outcome != OUT_OTHER)
					return S_DOOR;

				run.SetFlag("door", outcome);
				AfterStep(gm, run, stage, outcome);
				Pay(gm, run, stage, true);
				f.AddStep(m_iRoute, 1, DoorWhy(outcome));
				return S_PIECE;
			}
			case S_PIECE:
			{
				if (outcome != OUT_PIECE)
					return S_PIECE;

				AfterStep(gm, run, stage, outcome);
				Pay(gm, run, stage, true);
				f.AddStep(m_iRoute, 2, "the piece");
				return S_TISSUE;
			}
			case S_TISSUE: return S_ACT4;
			case S_ACT4:
			{
				if (outcome != OUT_LAST)
					return S_ACT4;

				AfterStep(gm, run, stage, outcome);
				Pay(gm, run, stage, true);
				f.AddStep(m_iRoute, 3, "the last step", true);
				return S_AFTER;
			}
			case S_REMAKE:
			{
				f.OnRemake(m_iRoute);
				return S_AFTER;
			}
			case S_RELEASE: return Release(gm, run, outcome);
		}

		return S_AFTER;
	}

	//------------------------------------------------------------------------------------------------
	protected string DoorWhy(int outcome)
	{
		if (outcome == OUT_OTHER)
			return "the other way round the door";

		return "through the door";
	}

	//------------------------------------------------------------------------------------------------
	//! A step's own consequences (items, flags, words). Called before the points.
	protected void AfterStep(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The stage's pay: the decider (at most 10 Fame and 20 Favor with the route's faction; the
	//! Independent 20 Haven Favor and no Fame), once per route per player for open acts; on sworn
	//! acts, unlocked helpers get half the Favor and no Fame. Inside the finale's caps.
	protected void Pay(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, bool sworn)
	{
		RZ_CureFinale f = Finale();
		string decider = run.Text("decider");
		int faction = RZ_CureFinale.PayFaction(m_iRoute);
		int fame = 10;
		if (m_iRoute == RZ_CureFinale.INDEP)
			fame = 0;

		if (!decider.IsEmpty())
		{
			string once = string.Format("paid.%1.%2", stage, decider.Hash());
			if (run.Flag(once) == 0)
			{
				run.SetFlag(once, 1);
				f.PayIdent(decider, faction, fame, 20, 0, m_sTitle);
			}
		}

		if (!sworn)
			return;

		foreach (RZ_QuestPart p : run.m_aParts)
		{
			if (p.m_iStage != stage || p.m_sIdent == decider || f.RouteOf(p.m_sIdent) != RZ_CureFinale.NONE)
				continue;

			string help = string.Format("help.%1.%2", stage, p.m_sIdent.Hash());
			if (run.Flag(help) != 0)
				continue;

			run.SetFlag(help, 1);
			f.PayIdent(p.m_sIdent, faction, 0, 10, 0, m_sTitle);
		}

		run.SetText("decider", "");
	}

	//------------------------------------------------------------------------------------------------
	//! The terms are chosen (or the timeout's default): the controller records them.
	protected int Release(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		int terms = outcome - OUT_TERMS;
		if (outcome == OUT_TERMS_DEFAULT || terms < 1 || terms > 4)
			terms = DefaultTerms(gm);

		array<string> deciders = {};
		RZ_MajorHelp.Deciders(run, S_RELEASE, -1, deciders);
		int condition = run.Flag("condition");
		Finale().OnTerms(m_iRoute, terms, condition, deciders);
		return Finish(run, E_CONTROLLED, ControlledLine(terms));
	}

	//------------------------------------------------------------------------------------------------
	//! The cure: this route's ending (RZ_Questlines.EndRoutes). Controller: the release; else the
	//! finished-and-lost or the never-finished line.
	int AtCure(RZ_GameModeComponent gm, RZ_QuestRun run, bool controller)
	{
		if (controller)
			return S_RELEASE;

		RZ_CureFinale f = Finale();
		if (f && f.Finisher() == m_iRoute)
		{
			run.m_iEnding = E_FINISHED_LOST;
			run.m_sEndText = run.Fill(LostLine());
		}
		else
		{
			run.m_iEnding = E_NEVER;
			run.m_sEndText = run.Fill(NeverLine());
		}

		return END;
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		if (!run.m_sEndText.IsEmpty())
			Headline(gm, run.m_sEndText, false);
	}

	//------------------------------------------------------------------------------------------------
	//! The journal: the stage's own text and the lab's readout (tracks by faction, never by player).
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		string s = super.JournalText(gm, run);
		RZ_CureFinale f = Finale();
		if (f && f.IsOpen())
			s += "\n" + f.Readout();

		return s;
	}

	// ================================================================ Shared scene helpers
	//------------------------------------------------------------------------------------------------
	//! The stronghold leader's seat for a faction, or the lab.
	protected vector Seat(RZ_GameModeComponent gm, int faction)
	{
		RZ_Leader l = RZ_MajorHelp.Top(gm, faction);
		if (l)
			return RZ_MajorHelp.NearSeat(l.m_vPos);

		return RZ_MajorHelp.NearSeat(gm.GetLabPosition());
	}

	//------------------------------------------------------------------------------------------------
	//! A spot at the lab gate (40-60 m out).
	protected vector LabGate(RZ_GameModeComponent gm)
	{
		return RZ_Mission.LandPointNear(gm.GetLabPosition(), 40, 60);
	}

	//------------------------------------------------------------------------------------------------
	//! A person's spot from the finale's people, or near `fallback`.
	protected vector PersonAt(string key, vector fallback)
	{
		RZ_CureFinale f = Finale();
		vector p = vector.Zero;
		if (f)
			p = f.GetPeople().Pos(key);

		if (p == vector.Zero)
			p = fallback;

		return RZ_Mission.LandPointNear(p, 4, 12);
	}

	//------------------------------------------------------------------------------------------------
	//! The live-tissue hand-in and the Act IV wait on the lab's work, as a choice on the lab's duty
	//! officer: the lab tests the piece against a tray.
	protected int DutyOfficer(RZ_GameModeComponent gm, RZ_QM_MajorScene scene, string says)
	{
		return scene.Person(RZ_MajorHelp.NearSeat(gm.GetLabPosition()), "the lab's duty officer", "Talk to the duty officer", says);
	}

	//------------------------------------------------------------------------------------------------
	//! Gives a route item to this player (and the run's note).
	protected void GiveItem(RZ_GameModeComponent gm, int playerId, string kind, string name, vector seat, float cold)
	{
		RZ_CureFinale f = Finale();
		if (f)
			f.GetItems().Give(kind, name, m_iRoute, IdentOf(gm, playerId), seat, cold);
	}

	//------------------------------------------------------------------------------------------------
	protected int Carries(RZ_GameModeComponent gm, int playerId, string kind)
	{
		RZ_CureFinale f = Finale();
		if (!f)
			return 0;

		return f.GetItems().Count(IdentOf(gm, playerId), kind, m_iRoute);
	}

	//------------------------------------------------------------------------------------------------
	protected int HandIn(RZ_GameModeComponent gm, int playerId, string kind, int count, bool anyRoute = false)
	{
		RZ_CureFinale f = Finale();
		if (!f)
			return 0;

		return f.GetItems().HandIn(IdentOf(gm, playerId), kind, m_iRoute, count, anyRoute);
	}

	//------------------------------------------------------------------------------------------------
	//! The questline flag a major left, read as 0 when that major didn't run.
	protected int Major(RZ_GameModeComponent gm, string key)
	{
		return RZ_MajorHelp.World(gm, key);
	}
}

//------------------------------------------------------------------------------------------------
//! A route's desk (the commit node's fallback, docs/FINALE.md "The lock"): the route's clerk near
//! its stronghold leader (the Independent's at the lab gate), from the gate until the cure. Not on
//! the radio. "What would it mean?" shows the lock card and arms "Write me down" for 30 s. The
//! go-between takes pact votes; the clerk takes a price door; the lost-and-found takes back what
//! you found (10 Favor with the route's faction: "found it").
class RZ_Mission_FinaleDesk : RZ_QM_HalcyonScene
{
	int m_iRoute = -2;
	protected ref map<int, float> m_mArmed = new map<int, float>();
	protected float m_fDeskClock;
	protected ref array<int> m_aPactTo = {};

	protected static const float CONFIRM_SECONDS = 30;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 0, 0);
		m_bHasGiver = false;
		m_Req.m_bLeaderDecides = true;
		m_Req.m_bNotCured = true;
		m_bInPersonOnly = true;
		m_fParticipationRadius = 0;
		m_fTimeLeft = 604800;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_CureFinale f = RZ_CureFinale.Get();
		if (!f || !f.IsOpen())
			return false;

		vector at;
		string clerk = "the route's clerk";
		if (m_iRoute == RZ_CureFinale.INDEP)
		{
			at = RZ_Mission.LandPointNear(gm.GetLabPosition(), 45, 65);
			clerk = "the Doctor at the lab gate";
		}
		else
		{
			RZ_Leader l = RZ_MajorHelp.Top(gm, m_iRoute);
			if (!l)
				return false;

			at = RZ_MajorHelp.NearSeat(l.m_vPos);
			clerk = string.Format("%1's clerk", RZ_Factions.GetName(m_iRoute));
		}

		if (at == vector.Zero)
			return false;

		m_vPos = at;
		m_iSettlement = gm.FindNearestSettlement(at);
		m_sTitle = RZ_CureFinale.RouteTitle(m_iRoute);
		m_sBrief = string.Format("The cure is at the gate. %1 takes names for its route here.", RZ_CureFinale.RouteName(m_iRoute));
		int talk = AddClue(clerk, Pitch());
		AddSpot(at, RZ_EPuzzleProp.CIVILIAN, 0, talk, "Talk to " + clerk, "What would it mean?|Write me down [no return]|How do things stand?|" + PriceLabel(), clerk);
		string pacts = "";
		for (int r = 0; r < RZ_CureFinale.ROUTES; r++)
		{
			if (r == m_iRoute)
				continue;

			m_aPactTo.Insert(r);
			if (!pacts.IsEmpty())
				pacts += "|";

			pacts += "Pact with " + RZ_CureFinale.RouteName(r);
		}

		int between = AddClue("the go-between", "\"A pact is both routes' regulars agreeing within a day. One a route. You stand together at the stand.\"");
		AddSpot(RZ_Mission.LandPointNear(at, 3, 6), RZ_EPuzzleProp.CIVILIAN, 1, between, "Talk to the go-between", pacts, "the go-between");
		int found = AddClue("the lost-and-found", "\"Picked up something with our name on it? We'll take it back. No questions.\"");
		AddSpot(RZ_Mission.LandPointNear(at, 3, 6), RZ_EPuzzleProp.OBJECT, 2, found, "Read the note", "Give back what you found", "");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The route's pitch, with its admission.
	protected string Pitch()
	{
		switch (m_iRoute)
		{
			case RZ_CureFinale.HAVEN: return "\"The cure goes hand to hand, to the people the others would leave till last.\" Then, quieter: " + RZ_CureFinale.Admission(m_iRoute);
			case RZ_CureFinale.BASTION: return "\"Order: a queue, a register, nobody trampled at the gate.\" Then, quieter: " + RZ_CureFinale.Admission(m_iRoute);
			case RZ_CureFinale.REDHAND: return "\"It reaches anyone who can pay, including people nobody else would serve.\" Then: " + RZ_CureFinale.Admission(m_iRoute);
			case RZ_CureFinale.HALCYON: return "\"Published, fast, checked, with every author named.\" Then, quieter: " + RZ_CureFinale.Admission(m_iRoute);
			case RZ_CureFinale.INDEP: return "\"Four lots of people are about to tell you the cure is theirs. It isn't anyone's yet.\" Then: " + RZ_CureFinale.Admission(m_iRoute);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected string PriceLabel()
	{
		switch (m_iRoute)
		{
			case RZ_CureFinale.HAVEN: return "In pencil [4 food, dusk]";
			case RZ_CureFinale.BASTION: return "Sign on, attached [a firearm]";
			case RZ_CureFinale.REDHAND: return "A line in the book [60 Favor owed]";
			case RZ_CureFinale.HALCYON: return "The volunteers' book [unarmed]";
			case RZ_CureFinale.INDEP: return "The Coat Run [a Halcyon coat]";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The Coat Run (the Independent's, once a season): during another route's batch hold, at a
	//! change of shift, an Independent-locked player passing in Halcyon's kit (or a Lab Tech) walks
	//! past the duty officer and re-labels the sealed batch. A holder's player within 15 m of the
	//! clean room calls them out. The batch never leaves the lab.
	protected void CoatRun(RZ_GameModeComponent gm, RZ_CureFinale f, int playerId)
	{
		if (f.RouteOfPlayer(playerId) != RZ_CureFinale.INDEP)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Sign first. Then we'll talk about coats.\"");
			return;
		}

		if (!f.CoatRunOpen())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Not now. Another route's batch, in the cold, at a change of shift. And only once.\"");
			return;
		}

		RZ_Hostility hostility = gm.GetHostility();
		bool look = RZ_QuestApproach.HasBackground(gm, playerId, "labtech");
		if (!look && hostility && hostility.GetDisguise())
			look = hostility.GetDisguise().IsPassing(playerId, RZ_EFaction.INSTITUTE);

		bool called = false;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			int r = f.RouteOfPlayer(ids[i]);
			if (r != RZ_CureFinale.NONE && r == f.Holder() && RZ_Util.DistXZ(p.GetOrigin(), f.RoomPos()) <= 15)
				called = true;
		}

		if (look && !called)
		{
			f.CoatRun(playerId, true);
			gm.NotifyPlayer(playerId, m_sTitle, "Past the duty officer, into the clean room. The label now says: Released to: -");
			return;
		}

		f.CoatRun(playerId, false);
		gm.AddInfamy(playerId, RZ_EFaction.INSTITUTE, 3, "caught in a borrowed coat", RZ_RepFlag.ACT);
		gm.NotifyPlayer(playerId, m_sTitle, "\"That's not your coat.\" The duty officer takes it back. The batch's route will hear.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fDeskClock += dt;
		RZ_CureFinale f = RZ_CureFinale.Get();
		if (!f || !f.IsOpen())
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		RZ_CureFinale f = RZ_CureFinale.Get();
		if (!f)
			return;

		if (objIdx == 1)
		{
			if (option >= 0 && option < m_aPactTo.Count())
				gm.NotifyPlayer(playerId, m_sTitle, f.PactVote(playerId, m_iRoute, m_aPactTo[option]));

			return;
		}

		if (objIdx == 2)
		{
			GiveBack(gm, f, playerId);
			return;
		}

		switch (option)
		{
			case 0:
			{
				string why = f.CanCommit(playerId, m_iRoute);
				if (!why.IsEmpty())
				{
					gm.NotifyPlayer(playerId, m_sTitle, why);
					return;
				}

				m_mArmed.Set(playerId, m_fDeskClock);
				gm.NotifyPlayerAs(playerId, "[Point of no return]", f.LockCard(playerId, m_iRoute) + "\nTo confirm: Write me down, within 30 seconds.", RZ_ENoticeTag.HIGH);
				return;
			}
			case 1:
			{
				float at;
				if (!m_mArmed.Find(playerId, at) || m_fDeskClock - at > CONFIRM_SECONDS)
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"Hear what it means first. Then tell me.\"");
					return;
				}

				m_mArmed.Remove(playerId);
				if (f.Commit(playerId, m_iRoute))
					Remember(gm, playerId, RZ_EQuestApproach.SIDE, "locked to " + RZ_CureFinale.RouteName(m_iRoute));

				return;
			}
			case 2:
			{
				gm.NotifyPlayer(playerId, m_sTitle, f.PageFor(playerId));
				return;
			}
			case 3:
			{
				if (m_iRoute == RZ_CureFinale.INDEP)
					CoatRun(gm, f, playerId);
				else
					PriceDoor(gm, f, playerId, source);

				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The price door for a player the route's faction hunts. It opens the route only; Infamy stays.
	protected void PriceDoor(RZ_GameModeComponent gm, RZ_CureFinale f, int playerId, IEntity source)
	{
		if (m_iRoute == RZ_CureFinale.INDEP)
			return;

		if (!RZ_Reputation.IsHunted(gm.GetTier(playerId, m_iRoute)))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"You don't need to pay that. Just ask.\"");
			return;
		}

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me)
			return;

		string done = "";
		switch (m_iRoute)
		{
			case RZ_CureFinale.HAVEN:
			{
				if (!RZ_WorldFacts.Get().IsDusk() || RZ_Util.IsArmed(me))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"At the knock. Dusk, and nothing in your hands.\"");
					return;
				}

				if (!RZ_HavenQuest.TakeFood(gm, me, 4))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"Four for the pot. And a name to read.\"");
					return;
				}

				done = "A council head writes you on the List, in pencil. It opens the route. It doesn't open the door.";
				break;
			}
			case RZ_CureFinale.BASTION:
			{
				if (!TakeFirearm(me))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"Hand over one firearm. Then we'll talk about the roster.\"");
					return;
				}

				done = "The corporal takes your firearm. Sedlak writes you on the roster: \"Civilian, attached.\"";
				break;
			}
			case RZ_CureFinale.REDHAND:
			{
				done = "The boss writes you a line in the book. Whatever Red Hand Favor you had goes on it.";
				gm.AddFavor(playerId, RZ_EFaction.BANDITS, -60);
				break;
			}
			case RZ_CureFinale.HALCYON:
			{
				if (RZ_Util.IsArmed(me))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"Unarmed, please. This is a field station, not a checkpoint.\"");
					return;
				}

				done = "A draw of your own blood, by your own yes, and a notebook number beside your name. The volunteers' book is open to you.";
				break;
			}
		}

		f.PayPrice(playerId, m_iRoute);
		gm.NotifyPlayer(playerId, m_sTitle, done);
		RZ_Log.Info(string.Format("Finale: a price door paid for %1.", RZ_CureFinale.RouteName(m_iRoute)));
	}

	//------------------------------------------------------------------------------------------------
	//! Takes the first firearm in a weapon slot. False if there's none.
	protected static bool TakeFirearm(IEntity me)
	{
		BaseWeaponManagerComponent wm = BaseWeaponManagerComponent.Cast(me.FindComponent(BaseWeaponManagerComponent));
		if (!wm || !RZ_Util.IsArmed(me))
			return false;

		array<IEntity> weapons = {};
		wm.GetWeaponsList(weapons);
		foreach (IEntity w : weapons)
		{
			if (!w)
				continue;

			BaseWeaponComponent wc = BaseWeaponComponent.Cast(w.FindComponent(BaseWeaponComponent));
			if (!wc || (wc.GetWeaponType() != EWeaponType.WT_RIFLE && wc.GetWeaponType() != EWeaponType.WT_HANDGUN && wc.GetWeaponType() != EWeaponType.WT_SNIPERRIFLE && wc.GetWeaponType() != EWeaponType.WT_MACHINEGUN))
				continue;

			RZ_ItemHelper.DeleteCarried(me, w);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void GiveBack(RZ_GameModeComponent gm, RZ_CureFinale f, int playerId)
	{
		int n = f.GetItems().GiveBack(gm.GetProfile(playerId).m_sIdentity, m_iRoute);
		if (n <= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Nothing of ours on you.\"");
			return;
		}

		int favor = 10;
		if (f.RouteOfPlayer(playerId) == m_iRoute)
			favor = 0;

		if (favor > 0)
			f.PayIdent(gm.GetProfile(playerId).m_sIdentity, RZ_CureFinale.PayFaction(m_iRoute), 0, favor, 0, "found it");

		gm.NotifyPlayer(playerId, m_sTitle, "\"Found it, did you. Good.\"");
	}
}

//------------------------------------------------------------------------------------------------
//! Live tissue (Act III, open): a leader taken alive with Capture the Index's rules. The lab counts
//! it at once (+2). Credited to the deliverer's own route if they're locked to one that still needs
//! it, else to the route it was offered for. Never picked at random.
class RZ_QM_RouteTissueIndex : RZ_Mission_CaptureTheIndex
{
	int m_iRoute = -2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		SetContract(RZ_CureFinale.PayFaction(m_iRoute), RZ_ETier.STRANGER, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!super.Setup(gm))
			return false;

		m_sTitle = RZ_FinaleTissue.Title(m_iRoute);
		m_sBrief = RZ_FinaleTissue.Brief(m_iRoute) + " " + m_sBrief;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_CureFinale f = RZ_CureFinale.Get();
		if (f)
			f.OnTissue(m_iRoute, RZ_FinaleTissue.Deliverer(m_aParticipants), true);
	}
}

//------------------------------------------------------------------------------------------------
//! Live tissue's second best: one of a pack's guard or young, or a special (Live Capture's rules).
//! The lab works on it for 12 real hours before it counts (+2).
class RZ_QM_RouteTissueLone : RZ_Mission_LiveCapture
{
	int m_iRoute = -2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		SetContract(RZ_CureFinale.PayFaction(m_iRoute), RZ_ETier.STRANGER, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!super.SetupPuzzle(gm))
			return false;

		m_sTitle = RZ_FinaleTissue.Title(m_iRoute);
		m_sBrief = RZ_FinaleTissue.Brief(m_iRoute) + " " + m_sBrief;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_CureFinale f = RZ_CureFinale.Get();
		if (f)
			f.OnTissue(m_iRoute, RZ_FinaleTissue.Deliverer(m_aParticipants), false);
	}
}

//------------------------------------------------------------------------------------------------
//! Each route's words for its tissue act (never "alpha", "tribe" or "crawler").
class RZ_FinaleTissue
{
	//------------------------------------------------------------------------------------------------
	static string Title(int r)
	{
		switch (r)
		{
			case RZ_CureFinale.HAVEN: return "Who It Was";
			case RZ_CureFinale.BASTION: return "Contact, Captured";
			case RZ_CureFinale.REDHAND: return "Fresh Stock";
			case RZ_CureFinale.HALCYON: return "The Index";
			case RZ_CureFinale.INDEP: return "Nobody's Index";
		}

		return "Live tissue";
	}

	//------------------------------------------------------------------------------------------------
	static string Brief(int r)
	{
		switch (r)
		{
			case RZ_CureFinale.HAVEN: return "\"Whoever they were, bring them in alive. We'll say the name if we know it.\"";
			case RZ_CureFinale.BASTION: return "\"A contact, alive. Bring it in on a stretcher and sign for it.\"";
			case RZ_CureFinale.REDHAND: return "\"A live one. The pit crews know where they lie up.\"";
			case RZ_CureFinale.HALCYON: return "\"The index. Alive, please, and tagged before it wakes.\"";
			case RZ_CureFinale.INDEP: return "\"Somebody's, once. Bring them in.\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Who handed it in: the last participant (the drop's carrier is added last).
	static int Deliverer(array<int> participants)
	{
		if (!participants || participants.IsEmpty())
			return -1;

		return participants[participants.Count() - 1];
	}
}
