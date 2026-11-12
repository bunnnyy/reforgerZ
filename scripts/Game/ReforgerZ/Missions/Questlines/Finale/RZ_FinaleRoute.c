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
//! This file also holds the desks (the commit node's fallback with Dialogue > Trees off: "What would it
//! mean?", then "Write me down" within 30 s; pacts; price doors; giving back what you found) and the
//! tissue hunts.

//------------------------------------------------------------------------------------------------
//! A route's stage scene: the majors' scene with the finale's gates.
class RZ_QM_RouteScene : RZ_QM_MajorScene
{
	int m_iRoute = -2;
	bool m_bSworn;
	protected ref map<int, string> m_mSubjects = new map<int, string>();	// spot -> the finale person standing there

	//------------------------------------------------------------------------------------------------
	//! This spot is one of the finale's people: a death on-screen is theirs (RZ_FinalePeople.Killed).
	void TrackSubject(int spot, string key)
	{
		m_mSubjects.Set(spot, key);
	}

	//------------------------------------------------------------------------------------------------
	//! A subject's body died on-screen: +20 Infamy with their faction for the nearest player within
	//! 150 m (the killer isn't read), and the stage is offered again, now with the route's branch.
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		super.OnScene(gm, dt, ids, players);
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (!finale || m_mSubjects.IsEmpty())
			return;

		foreach (int spot, string key : m_mSubjects)
		{
			RZ_HalcyonSpot s = GetSpot(spot);
			if (!s || !s.m_bUp || !s.m_Ent || RZ_Util.IsCharacterAlive(s.m_Ent) || !finale.GetPeople().IsAlive(key))
				continue;

			// The killer read from the kill; the nearest player within 150 m when the kill names nobody
			int killer = RZ_Util.GetKillerPlayerId(s.m_Ent);
			int nearest = -1;
			float best = 150;
			foreach (int i, IEntity p : players)
			{
				float d = RZ_Util.DistXZ(p.GetOrigin(), s.m_vPos);
				if (d < best)
				{
					best = d;
					nearest = ids[i];
				}
			}

			if (killer <= 0)
				killer = nearest;

			finale.GetPeople().Killed(key, killer);
			NotifyInterested(gm, m_sTitle, "They're dead. Whatever this route needed from them, it needs from somewhere else now.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}
	}

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

		// A rival's move ("do:rival:..."): only for players written down for another route
		bool rival = c.m_sFlag.StartsWith("do:rival:");
		string why = finale.MayDecide(playerId, m_iRoute, m_bSworn);
		if (rival)
		{
			why = "";
			int theirs = finale.RouteOfPlayer(playerId);
			if (theirs == RZ_CureFinale.NONE || theirs == m_iRoute)
				why = "That's for the other routes' people. Not you.";
		}

		// Guard the door: a rival's talk needs a pact; a sneak or a trick there is one step harder
		if (rival && why.IsEmpty() && run.m_iStage == RZ_FinaleRoute.S_DOOR)
			why = finale.GetRest().GuardedRefusal(m_iRoute, playerId, c.m_bSneak || c.m_iTrick > 0);

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

		// A route asset (its owner locked to this route) eases one check, once: a persuade or a
		// rank one step easier, or a price a third lower
		RZ_FinaleAssets assets = finale.GetAssets();
		string ident = gm.GetProfile(playerId).m_sIdentity;
		int faction = c.m_iFaction;
		if (faction < 0)
			faction = m_iGiver;

		int persuade = c.m_iPersuade;
		int rank = c.m_iRank;
		int bribe = c.m_iBribe;
		int asset = -1;

		// Spoil the case: someone told this act's people a true thing about the route
		if (c.m_iPersuade > 0 && !rival && run.Flag("spoiled." + run.m_iStage.ToString()) != 0)
			c.m_iPersuade = Math.MinInt(4, c.m_iPersuade + 1);

		array<int> kinds = {};
		if (c.m_iPersuade > 1)
		{
			kinds.Insert(RZ_FinaleAssets.PROOF);
			kinds.Insert(RZ_FinaleAssets.ALLY);
			kinds.Insert(RZ_FinaleAssets.STANDING);
			asset = assets.Find(ident, m_iRoute, kinds);
			if (asset >= 0)
				c.m_iPersuade = c.m_iPersuade - 1;
		}
		else if (c.m_iRank > 0 && c.m_iRank > -99 && Rank(gm, playerId, faction) < c.m_iRank)
		{
			kinds.Insert(RZ_FinaleAssets.ALLY);
			kinds.Insert(RZ_FinaleAssets.STANDING);
			asset = assets.Find(ident, m_iRoute, kinds);
			if (asset >= 0)
				c.m_iRank = c.m_iRank - 1;
		}
		else if (c.m_iBribe >= 3)
		{
			kinds.Insert(RZ_FinaleAssets.STANDING);
			asset = assets.Find(ident, m_iRoute, kinds);
			if (asset >= 0)
				c.m_iBribe = c.m_iBribe - c.m_iBribe / 3;
		}

		// The majors' pages in the open: each makes one finale check one step easier, once
		bool page = false;
		if (asset < 0 && c.m_iPersuade > 1 && !rival && finale.GetRest().PageLeft())
		{
			c.m_iPersuade = c.m_iPersuade - 1;
			page = true;
		}

		bool ok = super.Allowed(gm, run, playerId, c);
		c.m_iPersuade = persuade;
		c.m_iRank = rank;
		c.m_iBribe = bribe;
		if (ok && page)
		{
			finale.GetRest().UsePage();
			gm.NotifyPlayer(playerId, m_sTitle, "What's in the open already counts here. It won't count twice.");
		}

		if (ok && asset >= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("What you brought counts here (%1). It won't count twice.", RZ_FinaleAssets.KindName(assets.KindAt(asset))));
			assets.Play(asset);
		}

		return ok;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Apply(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int spot, RZ_SceneChoice c)
	{
		RZ_FinaleRoute line = RouteLine(run);
		int decide = -1;
		if (line && c.m_sFlag.StartsWith("do:"))
			decide = line.DoIt(gm, run, playerId, c.m_sFlag.Substring(3, c.m_sFlag.Length() - 3));

		super.Apply(gm, run, playerId, spot, c);

		// Taking part in another route's scene: a note about that route
		RZ_CureFinale noteFinale = RZ_CureFinale.Get();
		if (noteFinale)
			noteFinale.GetRest().Noted(playerId, m_iRoute);

		// Custody and a drained source change what the stage can be: it's offered again
		if (c.m_sFlag.StartsWith("do:rival:custody") || c.m_sFlag.StartsWith("do:rival:drain"))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

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

//! One scene type per route, so the five routes' scenes can run side by side.
class RZ_QM_SceneHaven : RZ_QM_RouteScene {}
class RZ_QM_SceneBastion : RZ_QM_RouteScene {}
class RZ_QM_SceneRedHand : RZ_QM_RouteScene {}
class RZ_QM_SceneHalcyon : RZ_QM_RouteScene {}
class RZ_QM_SceneOneEach : RZ_QM_RouteScene {}

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
	//! The person the route claims while it runs ("woman", "captain", "cook"; "" none).
	protected string SubjectKey()
	{
		return "";
	}

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

		// No active lock for 3 real days: the sworn work pauses and the subject is let go
		bool paused = f.RoutePaused(m_iRoute);
		string subject = SubjectKey();
		if (paused && run.Flag("paused") == 0)
		{
			run.SetFlag("paused", 1);
			if (!subject.IsEmpty())
				run.Unclaim(RZ_FinalePeople.Subject(subject));

			RZ_Log.Info(string.Format("Finale: %1 paused (no active lock for 3 real days).", RZ_CureFinale.RouteName(m_iRoute)));
		}
		else if (!paused && run.Flag("paused") != 0)
		{
			run.SetFlag("paused", 0);
			if (!subject.IsEmpty())
				run.Claim(RZ_FinalePeople.Subject(subject));
		}

		if (paused && IsSworn(stage))
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
		// One scene class a route: the mission manager runs one mission of each type at a time
		RZ_QM_RouteScene s;
		switch (m_iRoute)
		{
			case RZ_CureFinale.HAVEN: s = new RZ_QM_SceneHaven(); break;
			case RZ_CureFinale.BASTION: s = new RZ_QM_SceneBastion(); break;
			case RZ_CureFinale.REDHAND: s = new RZ_QM_SceneRedHand(); break;
			case RZ_CureFinale.HALCYON: s = new RZ_QM_SceneHalcyon(); break;
			default: s = new RZ_QM_SceneOneEach(); break;
		}

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

		if (!RouteScene(gm, run, stage, scene))
			return false;

		// The rest of the route's ways (RZ_FinaleRest.MoreChoices)
		Finale().GetRest().MoreChoices(gm, run, stage, scene, m_iRoute);
		return true;
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
				return "Not yet. The work takes the time it takes.";

			return "";
		}

		if (head == "rest")
			return Finale().GetRest().CanDo(gm, run, playerId, m_iRoute, parts);

		if (head == "rival" && parts.Count() >= 3)
			return CanMove(gm, run, playerId, parts[1], parts[2]);

		if (head == "free" && parts.Count() >= 2)
		{
			RZ_FinalePersonSave held = Finale().GetPeople().Get(parts[1]);
			if (!held || !held.alive || held.holder < 0 || held.holder == m_iRoute)
				return "Nobody's holding them.";

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
		if (head == "rest")
			return Finale().GetRest().DoIt(gm, run, playerId, m_iRoute, parts);

		if (head == "give" && parts.Count() >= 3)
		{
			GiveItem(gm, playerId, parts[1], ItemName(parts[1]), PlayerPos(playerId), ColdFor(gm, playerId, parts[2].ToFloat()));
			return -1;
		}

		if (head == "rival" && parts.Count() >= 3)
		{
			Move(gm, run, playerId, parts[1], parts[2]);
			return -1;
		}

		if (head == "free" && parts.Count() >= 2)
		{
			RZ_FinalePersonSave freed = Finale().GetPeople().Get(parts[1]);
			if (freed)
			{
				RZ_Log.Info(string.Format("Finale: %1 talked out of %2's hands.", parts[1], RZ_CureFinale.RouteName(freed.holder)));
				freed.holder = -1;
			}

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

	// ================================================================ The moves (rivals)
	//------------------------------------------------------------------------------------------------
	//! A rival's move on this route's scene (docs/FINALE.md, "The moves"). "" if it can happen now.
	//!   spoil:<person>    tell them a true thing about this route: its persuades one step harder for the act (once per subject per rival route)
	//!   custody:<person>  walk them to your own people: +20 Infamy with their faction; they give you nothing
	//!   drain:<source>    the source spoils for the season: the route moves to its branch
	protected string CanMove(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string move, string what)
	{
		int theirs = Finale().RouteOfPlayer(playerId);
		RZ_FinalePeople people = Finale().GetPeople();
		if (move == "spoil")
		{
			if (run.Flag(string.Format("spoil.%1.%2", what, theirs)) != 0)
				return "You've told me. I heard you the first time.";

			return "";
		}

		if (move == "custody")
		{
			RZ_FinalePersonSave p = people.Get(what);
			if (!p || !p.alive || p.holder >= 0)
				return "I'm not going anywhere with you.";

			return "";
		}

		if (move == "drain")
		{
			if (RZ_WorldFacts.Get().Flag(DrainKey()) != 0)
				return "There's nothing left in it to spoil.";

			return "";
		}

		return "Not now.";
	}

	//------------------------------------------------------------------------------------------------
	protected void Move(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string move, string what)
	{
		RZ_CureFinale f = Finale();
		int theirs = f.RouteOfPlayer(playerId);
		RZ_FinalePeople people = f.GetPeople();
		f.Hostile(theirs, m_iRoute);
		if (move == "spoil")
		{
			run.SetFlag(string.Format("spoil.%1.%2", what, theirs), 1);
			run.SetFlag("spoiled." + run.m_iStage.ToString(), 1);
			TellLocked(gm, run.Fill("Someone told them the truth about us. Asking just got harder."));
			RZ_Log.Info(string.Format("Finale: %1 spoiled %2's case with %3.", RZ_CureFinale.RouteName(theirs), RZ_CureFinale.RouteName(m_iRoute), what));
			return;
		}

		if (move == "custody")
		{
			RZ_FinalePersonSave p = people.Get(what);
			if (!p)
				return;

			int faction = p.faction;
			vector to = Seat(gm, RZ_CureFinale.PayFaction(theirs));
			if (theirs == RZ_CureFinale.INDEP)
				to = LabGate(gm);

			people.Custody(what, theirs, to);
			if (faction >= 0)
				f.PayIdent(IdentOf(gm, playerId), faction, 0, 0, 20, "took one of the finale's people into custody");

			TellLocked(gm, string.Format("%1 has them now. They'll give %1 nothing, and we need them back.", RZ_Util.Capitalize(RZ_CureFinale.RouteName(theirs))));
			RZ_Log.Info(string.Format("Finale: %1 took %2 into custody from %3.", RZ_CureFinale.RouteName(theirs), what, RZ_CureFinale.RouteName(m_iRoute)));
			return;
		}

		if (move == "drain")
		{
			RZ_WorldFacts.Get().SetFlag(DrainKey(), 1);
			int infamy = 5;
			if (LockedNear(PlayerPos(playerId), 100))
				infamy = 15;

			int victim = RZ_CureFinale.PayFaction(m_iRoute);
			if (victim >= 0)
				f.PayIdent(IdentOf(gm, playerId), victim, 0, 0, infamy, "drained a route's source");

			TellLocked(gm, "Someone spoiled it at the source. That way's gone for the season.");
			RZ_Log.Info(string.Format("Finale: %1 drained %2's source (%3).", RZ_CureFinale.RouteName(theirs), RZ_CureFinale.RouteName(m_iRoute), what));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The season flag a drained source sets: its route moves to its branch.
	string DrainKey()
	{
		return string.Format("finale.drained.%1", m_iRoute + 1);
	}

	//------------------------------------------------------------------------------------------------
	bool IsDrained()
	{
		return RZ_WorldFacts.Get().Flag(DrainKey()) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! One of this route's locked players is within `radius` of `pos` (they saw it).
	protected bool LockedNear(vector pos, float radius)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (Finale().RouteOfPlayer(ids[i]) == m_iRoute && RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A line to this route's locked players who are online.
	protected void TellLocked(RZ_GameModeComponent gm, string text)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int pid : all)
		{
			if (Finale().RouteOfPlayer(pid) == m_iRoute)
				gm.NotifyPlayer(pid, m_sTitle, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The rivals' way into a scene: a spot beside the subject or the source, with the moves on it.
	//! Locked players of other routes only (the label is shared; the refusal comes when you pick).
	protected void RivalSpot(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, vector at, string person, string personName, string source)
	{
		int spot = scene.Person(RZ_Mission.LandPointNear(at, 8, 16), "the way round the back", "Look round the back", "Nobody's watching this side. Somebody from another route could do a lot from here.");
		if (!person.IsEmpty())
		{
			scene.Choice(spot, "Tell " + personName + " the truth about them").Sets("do:rival:spoil:" + person).Says("Every word of it true. It lands.", "spoiled a rival route's case");
			scene.Choice(spot, "Walk " + personName + " to your own people").Way(RZ_EQuestApproach.BETRAY).Sets("do:rival:custody:" + person).Says("They come, because you're armed and they aren't. They won't give you anything.", "took one of the finale's people into custody");
		}

		if (!source.IsEmpty())
			scene.Choice(spot, "Spoil " + source).Way(RZ_EQuestApproach.FORCE).Sets("do:rival:drain:" + source).Says("It takes twenty seconds to spoil. Whatever was in it is gone.", "drained a rival route's source");
	}

	//------------------------------------------------------------------------------------------------
	//! A subject held by another route: talk them out of it (persuade 3 on the holder's rank).
	protected void FreeChoice(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, int spot, string key)
	{
		RZ_FinalePersonSave p = Finale().GetPeople().Get(key);
		if (!p || !p.alive || p.holder < 0 || p.holder == m_iRoute)
			return;

		int holder = RZ_CureFinale.PayFaction(p.holder);
		scene.Choice(spot, "Talk them out of it").Persuade(holder, 3, "elder,police").Sets("do:free:" + key).Says("Fine. Take them. They weren't giving us anything anyway.", "talked one of the finale's people out of custody").Refused("They stay with us.");
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
//! A route's desk (docs/FINALE.md "The lock"): the route's clerk near its stronghold leader (the
//! Independent's at the lab gate), from the gate until the cure. Not on the radio. With Dialogue >
//! Trees off it is the commit node: "What would it mean?" shows the lock card and arms "Write me
//! down" for 30 s. With Trees on, the leaders' [Point of no return] topics take the names
//! (dialogue/finale/routes.rzd) and the desk keeps the rest; the Independent's desk (the unsigned
//! sheet at the lab gate) still takes names while no Doctor or Courier is on the road. The
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
		string lockLabels = "What would it mean?|Write me down [no return]|";
		if (TreesTakeNames() && m_iRoute != RZ_CureFinale.INDEP)
		{
			lockLabels = "||";
			m_sBrief = string.Format("The cure is at the gate. %1's leaders take names for its route; this desk keeps the page, the pacts and the price.", RZ_CureFinale.RouteName(m_iRoute));
		}

		AddSpot(at, RZ_EPuzzleProp.CIVILIAN, 0, talk, "Talk to " + clerk, lockLabels + "How do things stand?|" + PriceLabel(), clerk);
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
		int found = AddClue("the lost-and-found", "\"Picked up something with our name on it? We'll take it back. No questions. Or we'll buy it back, and remember who sold it.\"");
		AddSpot(RZ_Mission.LandPointNear(at, 3, 6), RZ_EPuzzleProp.OBJECT, 2, found, "Read the note", "Give back what you found|Ransom it back [their Favor]|Spoil another route's find", "");
		// The night man (the moves and Last Mile's contracts) and the gate sheet (the deal at the gate)
		int night = AddClue("the night man", "\"Things change hands at night. What you know, what you carry, what somebody else carries. I just write it down.\"");
		AddSpot(RZ_Mission.LandPointNear(at, 4, 8), RZ_EPuzzleProp.CIVILIAN, 4, night, "Talk to the night man", RZ_FinaleRest.DeskLabels(), "the night man");
		int sheet = AddClue("the gate sheet", "\"One name from each route, on one sheet, at the lab gate, while a batch is in the hold. Then it's a deal.\"");
		AddSpot(RZ_Mission.LandPointNear(at, 4, 8), RZ_EPuzzleProp.OBJECT, 5, sheet, "Read the sheet", "Agree the deal at the gate", "");
		if (m_iRoute == RZ_CureFinale.REDHAND || m_iRoute == RZ_CureFinale.INDEP)
		{
			int plan = AddClue("someone who sells plans", PlanTalk());
			AddSpot(RZ_Mission.LandPointNear(at, 6, 12), RZ_EPuzzleProp.CIVILIAN, 3, plan, "Talk to them", PlanLabel(), "someone who sells plans");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The route's pitch, with its admission.
	protected string Pitch()
	{
		return RZ_CureFinale.PitchLine(m_iRoute) + " Then, quieter: " + RZ_CureFinale.Admission(m_iRoute);
	}

	//------------------------------------------------------------------------------------------------
	//! Dialogue > Trees on: the [Point of no return] topics take the names, not the desk.
	protected static bool TreesTakeNames()
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		return trees && trees.IsOn();
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
			gm.NotifyPlayer(playerId, m_sTitle, "Sign first. Then we'll talk about coats.");
			return;
		}

		if (!f.CoatRunOpen())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Not now. Another route's batch, in the cold, at a change of shift. And only once.");
			return;
		}

		RZ_Hostility hostility = gm.GetHostility();
		bool look = RZ_QuestApproach.HasBackground(gm, playerId, "labtech");
		if (!look && hostility && hostility.GetDisguise())
			look = hostility.GetDisguise().IsPassing(playerId, RZ_EFaction.INSTITUTE);

		// The night was sold: someone is waiting for the coat
		bool called = RZ_WorldFacts.Get().Flag("finale.coat_sold") != 0;
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
			if (option == 1)
				Ransom(gm, f, playerId);
			else if (option == 2)
				SpoilFind(gm, f, playerId);
			else
				GiveBack(gm, f, playerId);

			return;
		}

		if (objIdx == 4 || objIdx == 5)
		{
			int pick = option;
			if (objIdx == 5)
				pick = 4;

			f.GetRest().DeskChoice(m_iRoute, playerId, pick);
			return;
		}

		if (objIdx == 3)
		{
			SellPlan(gm, f, playerId);
			return;
		}

		// The unsigned sheet: with Trees on, the Doctor (else the Courier) on the road takes the names
		if (option <= 1 && m_iRoute == RZ_CureFinale.INDEP && TreesTakeNames() && f.HasIndependentVoice())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Not on a sheet. Find the Doctor on the road, or the Courier. Ask them about the cure.");
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
					gm.NotifyPlayer(playerId, m_sTitle, "Hear what it means first. Then tell me.");
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
			gm.NotifyPlayer(playerId, m_sTitle, "You don't need to pay that. Just ask.");
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
					gm.NotifyPlayer(playerId, m_sTitle, "At the knock. Dusk, and nothing in your hands.");
					return;
				}

				if (!RZ_HavenQuest.TakeFood(gm, me, 4))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "Four for the pot. And a name to read.");
					return;
				}

				done = "A council head writes you on the List, in pencil. It opens the route. It doesn't open the door.";
				break;
			}
			case RZ_CureFinale.BASTION:
			{
				if (!TakeFirearm(me))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "Hand over one firearm. Then we'll talk about the roster.");
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
					gm.NotifyPlayer(playerId, m_sTitle, "Unarmed, please. This is a field station, not a checkpoint.");
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
	static bool TakeFirearm(IEntity me)
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

	// ================================================================ The moves at the desk
	//------------------------------------------------------------------------------------------------
	//! Ambush a carry, ransomed: this route's items you carry go back to their seats, and its
	//! faction pays 15 Favor an item (inside the finale's caps). The two routes are fighting.
	protected void Ransom(RZ_GameModeComponent gm, RZ_CureFinale f, int playerId)
	{
		int mine = f.RouteOfPlayer(playerId);
		if (mine == m_iRoute)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "It's ours. Just give it back.");
			return;
		}

		int n = f.GetItems().GiveBack(gm.GetProfile(playerId).m_sIdentity, m_iRoute);
		if (n <= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Nothing of ours on you.");
			return;
		}

		f.PayIdent(gm.GetProfile(playerId).m_sIdentity, RZ_CureFinale.PayFaction(m_iRoute), 0, 15 * n, 0, "ransomed a route's find");
		if (mine != RZ_CureFinale.NONE)
			f.Hostile(mine, m_iRoute);

		gm.NotifyPlayer(playerId, m_sTitle, "Paid. Don't think we won't remember who we paid.");
		RZ_Log.Info(string.Format("Finale: %1 item(s) of %2 ransomed back.", n, RZ_CureFinale.RouteName(m_iRoute)));
	}

	//------------------------------------------------------------------------------------------------
	//! Ambush a carry, spoiled: an item of another route you carry is spoiled here. Her bag: +10
	//! Haven Infamy, and she gives again.
	protected void SpoilFind(RZ_GameModeComponent gm, RZ_CureFinale f, int playerId)
	{
		string ident = gm.GetProfile(playerId).m_sIdentity;
		int mine = f.RouteOfPlayer(playerId);
		int r = f.GetItems().Spoil(ident, mine);
		if (r == RZ_CureFinale.NONE)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You're carrying nothing of anyone else's.");
			return;
		}

		if (mine != RZ_CureFinale.NONE)
			f.Hostile(mine, r);

		if (r == RZ_CureFinale.HAVEN)
		{
			f.PayIdent(ident, RZ_EFaction.SURVIVORS, 0, 0, 10, "spoiled a bag of her blood");
			f.GetPeople().GiveAgain("woman");
		}

		gm.NotifyPlayer(playerId, m_sTitle, "Poured out behind the desk. Whoever needed it will have to start that step again.");
	}

	//------------------------------------------------------------------------------------------------
	protected string PlanTalk()
	{
		if (m_iRoute == RZ_CureFinale.REDHAND)
			return "Everyone wants to know when the lorry backs up. I know a man who knows.";

		return "The coat, the shift, the night. Somebody always knows. For a price.";
	}

	//------------------------------------------------------------------------------------------------
	protected string PlanLabel()
	{
		if (m_iRoute == RZ_CureFinale.REDHAND)
			return "Buy the lorry's timing [30 Red Hand Favor]";

		return "Buy the Coat Run's night [20 Haven Favor]";
	}

	//------------------------------------------------------------------------------------------------
	//! Sell the plan (once per route a season): Take the Lab's timing (finale.plan_sold: the bay takes
	//! 150 s) or the Coat Run's night (finale.coat_sold: the Coat Run is called out). Only for players
	//! written down for another route; the Red Hand's needs one of its seat leaders gone Rogue.
	protected void SellPlan(RZ_GameModeComponent gm, RZ_CureFinale f, int playerId)
	{
		int mine = f.RouteOfPlayer(playerId);
		if (mine == RZ_CureFinale.NONE || mine == m_iRoute)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Not to you. You'd only tell them.");
			return;
		}

		string key = "finale.coat_sold";
		int faction = RZ_EFaction.SURVIVORS;
		int price = 20;
		if (m_iRoute == RZ_CureFinale.REDHAND)
		{
			key = "finale.plan_sold";
			faction = RZ_EFaction.BANDITS;
			price = 30;
			if (!HasRogue(gm, RZ_EFaction.BANDITS))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "Nobody's selling. Every crew boss is the boss's, this season.");
				return;
			}
		}

		if (RZ_WorldFacts.Get().Flag(key) != 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Sold already. Plans only sell once.");
			return;
		}

		if (!gm.SpendFavor(playerId, faction, price))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 %2 Favor. That's the price.", price, RZ_Factions.GetName(faction)));
			return;
		}

		RZ_WorldFacts.Get().SetFlag(key, 1);
		f.Hostile(mine, m_iRoute);
		gm.NotifyPlayer(playerId, m_sTitle, "\"You didn't hear it from me.\" You hear it from them.");
		RZ_Log.Info(string.Format("Finale: %1's plan sold to %2.", RZ_CureFinale.RouteName(m_iRoute), RZ_CureFinale.RouteName(mine)));
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasRogue(RZ_GameModeComponent gm, RZ_EFaction faction)
	{
		if (!gm.GetLeaders())
			return false;

		array<RZ_Leader> all = {};
		gm.GetLeaders().GetFactionLeaders(faction, all);
		foreach (RZ_Leader l : all)
		{
			if (l.IsPresent() && l.m_eLoyalty == RZ_ELoyalty.ROGUE)
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
			gm.NotifyPlayer(playerId, m_sTitle, "Nothing of ours on you.");
			return;
		}

		int favor = 10;
		if (f.RouteOfPlayer(playerId) == m_iRoute)
			favor = 0;

		if (favor > 0)
			f.PayIdent(gm.GetProfile(playerId).m_sIdentity, RZ_CureFinale.PayFaction(m_iRoute), 0, favor, 0, "found it");

		gm.NotifyPlayer(playerId, m_sTitle, "Found it, did you. Good.");
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

		// Live tissue's rules: within reach of the lab, and never the same pack twice in 6 season days
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && !finale.GetRest().TissueFrom(GetTribe(gm)))
			return false;

		m_sTitle = RZ_FinaleTissue.Title(m_iRoute);
		m_sBrief = RZ_FinaleTissue.Brief(m_iRoute) + " " + m_sBrief;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_CureFinale f = RZ_CureFinale.Get();
		RZ_Tribe t = GetTribe(gm);
		if (!f)
			return;

		f.GetRest().TissueGiven(t);
		f.OnTissue(m_iRoute, RZ_FinaleTissue.Deliverer(m_aParticipants), true, t && t.m_bGreat);
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
			case RZ_CureFinale.HAVEN: return "Whoever they were, bring them in alive. We'll say the name if we know it.";
			case RZ_CureFinale.BASTION: return "A contact, alive. Bring it in on a stretcher and sign for it.";
			case RZ_CureFinale.REDHAND: return "A live one. The pit crews know where they lie up.";
			case RZ_CureFinale.HALCYON: return "The index. Alive, please, and tagged before it wakes.";
			case RZ_CureFinale.INDEP: return "Somebody's, once. Bring them in.";
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
