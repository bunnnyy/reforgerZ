//! The Workbench driver (docs/PC_SETUP.md, "Driving the game from a file"): Workbench play only.
//! Claude Code plays the game through it without a mouse: it writes commands to
//! $profile:RZ_Control/cmd.txt, the driver runs them on the next quarter second, deletes the file and
//! writes what it saw to $profile:RZ_Control/out.txt (each answer starts "#<n> <command>"). Nothing
//! here is part of the game: the server and the game's own builds never compile it, and it needs no
//! network (no BattlEye: Workbench play doesn't run it).
//!
//! Commands, one a line:
//!   state                 where you are, your heading, your tracked job and its objectives, the talk screen
//!   near [radius]         the people round you (index, name, role, metres, bearing); indices for tp/face/talk
//!   tp <x> <z> | tp <n>   teleport to a spot, or 2 m in front of near person n, facing them
//!   face <n> | face <x> <z>
//!   talk <n>              talk to near person n (as Talk would)
//!   pick <id> | leave     a reply on the talk screen by its id (as listed by state)
//!   actions [radius]      the prompts on things round you (index, prompt, thing, metres)
//!   act <n>               do prompt n (the whole action at once: a held one counts as held through)
//!   jobs                  your jobs and their objectives; `track <id>` follows one
//!   shot <name>           a screenshot to $profile:RZ_Control/<name>.png
//!   walkto|runto|sprintto <x> <z> [stop m]   go there on foot, steered in the game (no keyboard or
//!                         mouse: the character's own input each frame), jumping when it stops making
//!                         headway; "arrived" (or "stuck") is added to out.txt; `stop` ends it
//!   ui [verb] [arg]       the open screen: with no verb, its name and what it offers; otherwise the
//!                         verb goes to the screen itself (RZ_MenuBase.DriverDo: tabs, rows, buttons,
//!                         a game's moves); `ui close` closes it
#ifdef WORKBENCH
class RZ_Driver
{
	static const string DIR = "$profile:RZ_Control";
	static const string CMD = "$profile:RZ_Control/cmd.txt";
	static const string OUT = "$profile:RZ_Control/out.txt";

	protected static ref RZ_Driver s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected int m_iSeq;
	protected ref array<IEntity> m_aNear = {};
	protected ref array<IEntity> m_aActOwners = {};
	protected ref array<ScriptedUserAction> m_aActs = {};
	protected ref array<IEntity> m_aFound = {};
	protected ref array<string> m_aOut = {};
	protected IEntity m_TalkTo;		// walking up to talk to them (weak)

	// Going somewhere on foot (read by the character controller each frame, below)
	protected static bool s_bGoing;
	protected static vector s_vGoal;
	protected static float s_fSpeed;
	protected static float s_fStopAt = 3;
	protected static float s_fJumpUntil;
	protected vector m_vLastPos;
	protected float m_fStillFor;
	protected int m_iJumps;
	protected static int s_iStance = 1;		// the stance asked for: 1 standing, 2 crouched, 3 prone (STANCECHANGE_*)
	protected static int s_iStanceSet = 1;	// the one last set
	protected static vector s_vStart;		// where this trip began
	protected static bool s_bSkipped;		// the long-trip jump done (once a trip)
	protected static vector s_vSteer;
	protected static ref array<vector> s_aPath = {};	// the planned waypoints (BuildPath)
	protected static int s_iPathAt;
	protected static float s_fPathAge;		// where the look-ahead says to head now (Plan), zero: straight at the goal
	protected static bool s_bDetour;		// heading for a side spot round an obstacle first
	protected static vector s_vDetour;
	protected float m_fDetourFor;
	protected int m_iSide = 1;
	protected vector m_vGlideLast;
	protected int m_iSlow;				// frames the controls didn't move us (Workbench not in focus)
	protected static const float GLIDE_MS = 50;
	protected static const float LONG_TRIP = 500;	// a trip this long or more (the author): walk the first part, then on to NEAR_GOAL short
	protected static const float NEAR_GOAL = 350;
	protected static const float WALK_FIRST = 150;

	//------------------------------------------------------------------------------------------------
	static void Start(RZ_GameModeComponent gm)
	{
		if (!gm)
			return;

		FileIO.MakeDirectory(DIR);
		s_Instance = new RZ_Driver();
		s_Instance.m_GM = gm;
		GetGame().GetCallqueue().CallLater(s_Instance.Poll, 250, true);
		RZ_Log.Info("Driver: listening on " + CMD + " (Workbench only)");
	}

	//------------------------------------------------------------------------------------------------
	protected void Poll()
	{
		Going();
		if (!FileIO.FileExists(CMD))
			return;

		FileHandle file = FileIO.OpenFile(CMD, FileMode.READ);
		if (!file)
			return;

		array<string> lines = {};
		string line;
		while (file.ReadLine(line) >= 0)
		{
			line.Trim();
			if (!line.IsEmpty())
				lines.Insert(line);
		}

		file.Close();
		FileIO.DeleteFile(CMD);

		m_aOut.Clear();
		foreach (string cmd : lines)
		{
			m_iSeq++;
			m_aOut.Insert(string.Format("#%1 %2", m_iSeq, cmd));
			Run(cmd);
		}

		FileHandle w = FileIO.OpenFile(OUT, FileMode.WRITE);
		if (!w)
			return;

		foreach (string o : m_aOut)
		{
			w.WriteLine(o);
		}

		w.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void Say(string text)
	{
		m_aOut.Insert(text);
	}

	//------------------------------------------------------------------------------------------------
	protected int Me()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return 0;

		return pc.GetPlayerId();
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity MyBody()
	{
		return GetGame().GetPlayerManager().GetPlayerControlledEntity(Me());
	}

	//------------------------------------------------------------------------------------------------
	protected void Run(string cmd)
	{
		array<string> a = {};
		cmd.Split(" ", a, true);
		if (a.IsEmpty())
			return;

		string verb = a[0];
		verb.ToLower();
		IEntity me = MyBody();
		if (!me && verb != "shot")
		{
			Say("no character (dead, or the spawn screen)");
			return;
		}

		switch (verb)
		{
			case "state": State(me); return;
			case "near": Near(me, Arg(a, 1, 40)); return;
			case "tp": Teleport(me, a); return;
			case "face": Face(me, a); return;
			case "talk": Talk(me, Arg(a, 1, -1)); return;
			case "pick": Pick(Arg(a, 1, -1)); return;
			case "leave": Leave(); return;
			case "actions": Actions(me, Arg(a, 1, 4)); return;
			case "act": Act(me, Arg(a, 1, -1)); return;
			case "jobs": Jobs(me); return;
			case "track": Track(me, Arg(a, 1, 0)); return;
			case "shot": Shot(a); return;
			case "walkto": GoTo(me, a, 1); return;
			case "runto": GoTo(me, a, 2); return;
			case "sprintto": GoTo(me, a, 3); return;
			case "stop": Stop("stopped"); return;
			case "ui": Ui(a); return;
		}

		Say("unknown command: " + verb);
	}

	//------------------------------------------------------------------------------------------------
	protected static float Arg(array<string> a, int i, float fallback)
	{
		if (i >= a.Count())
			return fallback;

		return a[i].ToFloat();
	}

	//------------------------------------------------------------------------------------------------
	protected static string Pos(vector p)
	{
		return string.Format("%1 %2 %3 (%4)", RZ_Util.RoundToInt(p[0]), RZ_Util.RoundToInt(p[1]), RZ_Util.RoundToInt(p[2]), RZ_SettlementScanner.GridReference(p));
	}

	//------------------------------------------------------------------------------------------------
	protected void State(IEntity me)
	{
		vector pos = me.GetOrigin();
		Say("pos " + Pos(pos) + " heading " + RZ_Util.RoundToInt(me.GetYawPitchRoll()[0]).ToString());
		Jobs(me);

		RZ_Dialogue talk = m_GM.GetDialogue();
		RZ_TalkSession s;
		if (talk)
			s = talk.SessionOf(Me());

		if (!s)
		{
			Say("talk: none");
			return;
		}

		Say(string.Format("talk: %1 (%2) page %3/%4", s.m_sName, s.m_sSub, s.m_iPage + 1, s.m_iPages));
		foreach (string said : s.m_aSaid)
		{
			Say("  says: " + said);
		}

		foreach (RZ_TalkReply r : s.m_aReplies)
		{
			string flags = "";
			if (!r.m_bEnabled)
				flags = " [locked: " + r.m_sNeed + "]";

			Say(string.Format("  reply %1: %2 %3%4", r.m_iId, r.m_sTag, r.m_sLabel, flags));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnFound(IEntity e)
	{
		if (e && RZ_Dialogue.IsPerson(e) && RZ_Util.IsCharacterAlive(e))
			m_aFound.Insert(e);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Near(IEntity me, float radius)
	{
		m_aFound.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(me.GetOrigin(), radius, OnFound, null, EQueryEntitiesFlags.DYNAMIC);
		m_aNear.Clear();
		foreach (IEntity e : m_aFound)
		{
			if (e != me)
				m_aNear.Insert(e);
		}

		vector here = me.GetOrigin();
		foreach (int i, IEntity p : m_aNear)
		{
			float d = RZ_Util.DistXZ(here, p.GetOrigin());
			Say(string.Format("%1: %2, %3 m %4", i, Who(p), RZ_Util.RoundToInt(d), RZ_Util.DirectionText(here, p.GetOrigin())));
		}

		if (m_aNear.IsEmpty())
			Say("nobody within " + RZ_Util.RoundToInt(radius).ToString() + " m");
	}

	//------------------------------------------------------------------------------------------------
	//! A person in words: their stable tag, else their Talk prompt's label, else their prefab.
	protected static string Who(IEntity p)
	{
		RZ_PersonTag t = RZ_PersonTags.Get(p);
		if (t)
			return t.m_sName + " (" + t.m_sRole + ")";

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(p.FindComponent(RZ_PuzzleObjectComponent));
		if (comp)
			return comp.GetSlotLabel(true, 0) + " [talk]";

		return PrefabOf(p);
	}

	//------------------------------------------------------------------------------------------------
	protected static string PrefabOf(IEntity e)
	{
		if (!e || !e.GetPrefabData())
			return "something";

		return RZ_Util.PrettyPrefabName(e.GetPrefabData().GetPrefabName());
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity NearAt(float index)
	{
		int i = RZ_Util.RoundToInt(index);
		if (i < 0 || i >= m_aNear.Count())
			return null;

		return m_aNear[i];
	}

	//------------------------------------------------------------------------------------------------
	protected void Teleport(IEntity me, array<string> a)
	{
		vector to;
		IEntity who;
		if (a.Count() >= 3)
		{
			to = RZ_Util.GroundPos(Vector(a[1].ToFloat(), 0, a[2].ToFloat()));
		}
		else
		{
			who = NearAt(Arg(a, 1, -1));
			if (!who)
			{
				Say("tp: no such person (run near first)");
				return;
			}

			vector them = who.GetOrigin();
			vector dir = me.GetOrigin() - them;
			dir[1] = 0;
			if (dir.Length() < 0.1)
				dir = Vector(0, 0, 1);

			dir.Normalize();
			to = RZ_Util.GroundPos(them + dir * 2);
		}

		SCR_Global.TeleportLocalPlayer(to + Vector(0, 0.3, 0));
		if (who)
			Turn(me, who.GetOrigin());

		Say("at " + Pos(to));
	}

	//------------------------------------------------------------------------------------------------
	protected void Turn(IEntity me, vector at)
	{
		float yaw = RZ_Compass.Bearing(me.GetOrigin(), at);
		me.SetYawPitchRoll(Vector(yaw, 0, 0)); // VERIFY: a player character turns to a yaw set from script in Workbench play
	}

	//------------------------------------------------------------------------------------------------
	protected void Face(IEntity me, array<string> a)
	{
		if (a.Count() >= 3)
		{
			Turn(me, Vector(a[1].ToFloat(), 0, a[2].ToFloat()));
			Say("facing the spot");
			return;
		}

		IEntity who = NearAt(Arg(a, 1, -1));
		if (!who)
		{
			Say("face: no such person");
			return;
		}

		Turn(me, who.GetOrigin());
		Say("facing " + Who(who));
	}

	//------------------------------------------------------------------------------------------------
	protected void Talk(IEntity me, float index)
	{
		IEntity who = NearAt(index);
		RZ_Dialogue talk = m_GM.GetDialogue();
		if (!who || !talk)
		{
			Say("talk: no such person, or the talk screen is off");
			return;
		}

		// Out of talking range (the screen guard would close it as left): walk up first, then talk
		if (RZ_Util.DistXZ(me.GetOrigin(), who.GetOrigin()) > 2.5)
		{
			m_TalkTo = who;
			s_vGoal = who.GetOrigin();
			s_fSpeed = 1;
			s_fStopAt = 1.8;
			s_bGoing = true;
			s_aPath.Clear();
			s_vStart = me.GetOrigin();
			s_bSkipped = true;
			s_vSteer = vector.Zero;
			m_vLastPos = me.GetOrigin();
			m_vGlideLast = me.GetOrigin();
			GetGame().GetCallqueue().Remove(Glide);
			GetGame().GetCallqueue().CallLater(Glide, GLIDE_MS, true);
			Say("walking up to " + Who(who) + ", then talking");
			return;
		}

		TalkNow(me, who);
	}

	//------------------------------------------------------------------------------------------------
	protected void TalkNow(IEntity me, IEntity who)
	{
		RZ_Dialogue talk = m_GM.GetDialogue();
		Turn(me, who.GetOrigin());

		if (!talk.Open(Me(), who, me))
		{
			Say("talk: they won't talk now");
			return;
		}

		GetGame().GetCallqueue().CallLater(StateLater, 600, false);
		Say("talking to " + Who(who) + " (state follows in a moment)");
	}

	//------------------------------------------------------------------------------------------------
	//! The talk screen's state once the answer is in (written as its own answer).
	protected void StateLater()
	{
		IEntity me = MyBody();
		if (!me)
			return;

		m_aOut.Clear();
		m_iSeq++;
		m_aOut.Insert(string.Format("#%1 state (after talk/pick/act)", m_iSeq));
		State(me);
		// Added after what the command's own answers said, never over them
		FileHandle w = FileIO.OpenFile(OUT, FileMode.APPEND);
		if (!w)
			return;

		foreach (string o : m_aOut)
		{
			w.WriteLine(o);
		}

		w.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void Pick(float id)
	{
		RZ_Dialogue talk = m_GM.GetDialogue();
		RZ_TalkSession s;
		if (talk)
			s = talk.SessionOf(Me());

		if (!s)
		{
			Say("pick: no conversation open");
			return;
		}

		talk.Pick(Me(), s.m_iSerial, RZ_Util.RoundToInt(id));
		GetGame().GetCallqueue().CallLater(StateLater, 600, false);
		Say("picked " + RZ_Util.RoundToInt(id).ToString() + " (state follows in a moment)");
	}

	//------------------------------------------------------------------------------------------------
	//! The way out, and the talk screen closed on this machine as the player's B would (the server
	//! ending it alone leaves the screen up, and an open screen holds the character still).
	protected void Leave()
	{
		Pick(RZ_Dialogue.LEAVE);
		MenuManager mm = GetGame().GetMenuManager();
		if (mm && RZ_TalkMenu.Cast(mm.GetTopMenu()))
			mm.GetTopMenu().Close();
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnThing(IEntity e)
	{
		if (e && e.FindComponent(ActionsManagerComponent))
			m_aFound.Insert(e);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Actions(IEntity me, float radius)
	{
		m_aFound.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(me.GetOrigin(), radius, OnThing, null, EQueryEntitiesFlags.ALL);
		m_aActs.Clear();
		m_aActOwners.Clear();
		vector here = me.GetOrigin();
		foreach (IEntity e : m_aFound)
		{
			if (e == me)
				continue;

			ActionsManagerComponent am = ActionsManagerComponent.Cast(e.FindComponent(ActionsManagerComponent));
			array<BaseUserAction> list = {};
			am.GetActionsList(list);
			foreach (BaseUserAction act : list)
			{
				ScriptedUserAction sa = ScriptedUserAction.Cast(act);
				if (!sa)
					continue;

				string name;
				if (!sa.GetActionNameScript(name) || name.IsEmpty())
					name = sa.ClassName();

				string can = "";
				if (!sa.CanBeShownScript(me))
					continue;

				if (!sa.CanBePerformedScript(me))
					can = " [can't now]";

				m_aActs.Insert(sa);
				m_aActOwners.Insert(e);
				Say(string.Format("%1: %2 on %3, %4 m%5", m_aActs.Count() - 1, name, PrefabOf(e), RZ_Util.RoundToInt(RZ_Util.DistXZ(here, e.GetOrigin())), can));
			}
		}

		if (m_aActs.IsEmpty())
			Say("no prompts within " + RZ_Util.RoundToInt(radius).ToString() + " m");
	}

	//------------------------------------------------------------------------------------------------
	protected void Act(IEntity me, float index)
	{
		int i = RZ_Util.RoundToInt(index);
		if (i < 0 || i >= m_aActs.Count() || !m_aActOwners[i])
		{
			Say("act: no such prompt (run actions first)");
			return;
		}

		ScriptedUserAction act = m_aActs[i];
		act.PerformAction(m_aActOwners[i], me); // VERIFY: performing a scripted action straight from script does what the prompt does (in Workbench play the client is the server)
		GetGame().GetCallqueue().CallLater(StateLater, 600, false);
		Say("done (state follows in a moment)");
	}

	//------------------------------------------------------------------------------------------------
	protected void Jobs(IEntity me)
	{
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (!feed || !feed.GetTracker())
			return;

		RZ_QuestTracker tracker = feed.GetTracker();
		int tracked = tracker.TrackedFor(Me());
		array<ref RZ_QuestJob> jobs = {};
		tracker.Jobs(Me(), me, jobs);
		if (jobs.IsEmpty())
			Say("jobs: none");

		foreach (RZ_QuestJob j : jobs)
		{
			string mark = "";
			if (j.m_iId == tracked)
				mark = " [TRACKED]";

			Say(string.Format("job %1: %2%3", j.m_iId, j.m_sTitle, mark));
			RZ_ObjList list = new RZ_ObjList();
			tracker.Objectives(Me(), j.m_iId, me.GetOrigin(), list);
			foreach (int k, string line : list.m_aLines)
			{
				string where = "";
				if (k < list.m_aPlaces.Count() && list.m_aPlaces[k] && list.m_aPlaces[k].m_vPos != vector.Zero)
					where = " -> " + RZ_Util.DirectionText(me.GetOrigin(), list.m_aPlaces[k].m_vPos) + " at " + Pos(list.m_aPlaces[k].m_vPos);

				Say(string.Format("  [%1] %2%3", list.m_aStates[k], line, where));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Track(IEntity me, float id)
	{
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (feed && feed.GetTracker())
			feed.GetTracker().Follow(Me(), RZ_Util.RoundToInt(id), "");

		Say("tracking " + RZ_Util.RoundToInt(id).ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void Shot(array<string> a)
	{
		string name = "shot";
		if (a.Count() >= 2)
			name = a[1];

		System.MakeScreenshot(DIR + "/" + name);
		Say("screenshot " + name);
	}
	// ================================================================ On foot
	//------------------------------------------------------------------------------------------------
	protected void GoTo(IEntity me, array<string> a, float speed)
	{
		if (a.Count() < 3)
		{
			Say("walkto <x> <z> [stop m]");
			return;
		}

		s_vGoal = Vector(a[1].ToFloat(), 0, a[2].ToFloat());
		s_fSpeed = speed;
		s_fStopAt = Arg(a, 3, 3);
		s_bGoing = true;
		s_aPath.Clear();
		s_vStart = me.GetOrigin();
		s_bSkipped = false;
		s_vSteer = vector.Zero;
		m_vLastPos = me.GetOrigin();
		m_vGlideLast = me.GetOrigin();
		m_iSlow = 0;
		GetGame().GetCallqueue().Remove(Glide);
		GetGame().GetCallqueue().CallLater(Glide, GLIDE_MS, true);
		m_fStillFor = 0;
		m_iJumps = 0;
		Say(string.Format("on my way: %1 m (%2)", RZ_Util.RoundToInt(RZ_Util.DistXZ(me.GetOrigin(), s_vGoal)), RZ_Util.DirectionText(me.GetOrigin(), s_vGoal)));
	}

	//------------------------------------------------------------------------------------------------
	//! Each quarter second while going: arrived, a talk screen opened (someone called us), or no
	//! headway (a jump, then a step aside, then "stuck").
	protected void Going()
	{
		if (!s_bGoing)
			return;

		IEntity me = MyBody();
		if (!me)
		{
			Stop("no character");
			return;
		}

		vector here = me.GetOrigin();
		float left = RZ_Util.DistXZ(here, s_vGoal);
		// Walking up to someone: they may move, so the goal follows them
		if (m_TalkTo)
			s_vGoal = m_TalkTo.GetOrigin();

		if (left <= s_fStopAt)
		{
			Stop(string.Format("arrived (%1 m off) at %2", RZ_Util.RoundToInt(left), Pos(here)));
			if (m_TalkTo)
			{
				IEntity who = m_TalkTo;
				m_TalkTo = null;
				TalkNow(me, who);
			}

			return;
		}

		RZ_Dialogue talk = m_GM.GetDialogue();
		if (talk && talk.SessionOf(Me()))
		{
			Stop("someone is talking to us: stopped at " + Pos(here));
			return;
		}

		// A long trip (the author): walk the first 150 m, then on to 350 m short of the goal
		if (!s_bSkipped && left > LONG_TRIP && RZ_Util.DistXZ(s_vStart, here) >= WALK_FIRST)
		{
			s_bSkipped = true;
			vector way = s_vGoal - here;
			way[1] = 0;
			way.Normalize();
			vector near = RZ_Util.GroundPos(s_vGoal - way * NEAR_GOAL);
			if (RZ_Util.IsWater(near))
				near = DryToward(near, s_vGoal);

			SCR_Global.TeleportLocalPlayer(near + Vector(0, 0.3, 0));
			m_vLastPos = near;
			Note("long trip: on to 350 m short, at " + Pos(near));
			return;
		}

		// In water (swimming, or the sea under us) with the goal on land (the author: never swim a
		// big water, unless the goal's in it): out at the first dry ground on the way, else the nearest
		SCR_CharacterControllerComponent swim = SCR_CharacterControllerComponent.Cast(me.FindComponent(SCR_CharacterControllerComponent));
		bool wet = RZ_Util.IsWater(RZ_Util.GroundPos(here)) || (swim && swim.IsSwimming());
		if (wet && !RZ_Util.IsWater(RZ_Util.GroundPos(s_vGoal)))
		{
			// A lake or river: dry is ground above the water we're swimming in
			float waterline = -1000;
			if (swim && swim.IsSwimming())
				waterline = here[1];

			vector dry = DryToward(here, s_vGoal, waterline);
			SCR_Global.TeleportLocalPlayer(dry + Vector(0, 0.3, 0));
			m_vLastPos = dry;
			Note("out of the water at " + Pos(dry));
			return;
		}

		// The way: a path planned over the ground ahead (round fences and walls to a gate), followed
		// waypoint by waypoint, planned again at its end, when stuck, or every 20 s; the plain
		// look-ahead when no path is found
		s_vSteer = Follow(here);
		if (s_vSteer == vector.Zero)
			s_vSteer = Plan(here, s_vGoal);

		// What's right in front on that way: a door (open it), something low (jump or climb over),
		// something at chest height with room under (crouch), room only at the ground (prone)
		vector ahead = s_vGoal;
		if (s_vSteer != vector.Zero)
			ahead = s_vSteer;

		Obstacle(me, here, ahead);

		// A side spot round an obstacle: reached, or given 4 s
		if (s_bDetour)
		{
			m_fDetourFor += 0.25;
			if (RZ_Util.DistXZ(here, s_vDetour) < 1.5 || m_fDetourFor > 4)
				s_bDetour = false;
		}

		// No headway: open a door in the way, else go round (one side, then the other, wider each
		// time), a jump only now and then (a fence, a low wall)
		if (RZ_Util.DistXZ(here, m_vLastPos) < 0.15)
		{
			m_fStillFor += 0.25;
			if (m_fStillFor >= 1)
			{
				m_fStillFor = 0;
				m_iJumps++;
				s_aPath.Clear();
				if (!OpenDoorNear(me))
				{
					vector way = s_vGoal - here;
					way[1] = 0;
					way.Normalize();
					float sideSign = m_iSide;
					vector side = Vector(way[2] * sideSign, 0, -way[0] * sideSign);
					float wide = 4 + m_iJumps * 2;
					s_vDetour = here + side * wide - way * 1.5;
					s_bDetour = true;
					m_fDetourFor = 0;
					if (m_iJumps % 2 == 0)
						m_iSide = -m_iSide;
					if (m_iJumps % 3 == 0)
						s_fJumpUntil = GetGame().GetWorld().GetWorldTime() + 400;
				}

				if (m_iJumps >= 12)
					Stop("stuck at " + Pos(here) + ", " + RZ_Util.RoundToInt(left).ToString() + " m to go");
			}
		}
		else
		{
			m_fStillFor = 0;
			if (!s_bDetour)
				m_iJumps = 0;
		}

		m_vLastPos = here;
	}

	//------------------------------------------------------------------------------------------------
	//! A shut door within 2.5 m: opened (its own prompt). True when one was.
	protected bool OpenDoorNear(IEntity me)
	{
		m_aFound.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(me.GetOrigin(), 2.5, OnThing, null, EQueryEntitiesFlags.ALL);
		foreach (IEntity e : m_aFound)
		{
			if (!e.FindComponent(DoorComponent) || RZ_DoorGeometry.IsOpen(e))
				continue;

			ActionsManagerComponent am = ActionsManagerComponent.Cast(e.FindComponent(ActionsManagerComponent));
			array<BaseUserAction> list = {};
			am.GetActionsList(list);
			foreach (BaseUserAction act : list)
			{
				ScriptedUserAction sa = ScriptedUserAction.Cast(act);
				if (sa && sa.CanBePerformedScript(me))
				{
					sa.PerformAction(e, me);
					Note("opened a door at " + Pos(e.GetOrigin()));
					return true;
				}
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The first dry ground on the way from `from` to `to` (every 20 m), else the nearest dry ground
	//! round `from` (out to 2 km), else `to`.
	protected static vector DryToward(vector from, vector to, float waterline = -1000)
	{
		vector way = to - from;
		way[1] = 0;
		float len = way.Length();
		if (len > 0.1)
		{
			way.Normalize();
			for (float d = 20; d < len; d += 20)
			{
				vector p = RZ_Util.GroundPos(from + way * d);
				if (!RZ_Util.IsWater(p) && p[1] > waterline + 0.2)
					return p;
			}
		}

		for (int r = 50; r <= 2000; r += 50)
		{
			for (int k = 0; k < 16; k++)
			{
				vector q = RZ_Util.GroundPos(RZ_Compass.PointAt(from, k * 22.5, r));
				if (!RZ_Util.IsWater(q) && q[1] > waterline + 0.2)
					return q;
			}
		}

		return RZ_Util.GroundPos(to);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 50 ms while going: when the character's own controls don't move it (Workbench isn't the
	//! focused window, so the engine reads no controls), the driver moves it along the ground at the
	//! asked pace itself, never through a wall (a chest-high trace ahead), so Claude Code can play while
	//! the author uses the PC.
	protected void Glide()
	{
		IEntity me = MyBody();
		if (!s_bGoing || !me)
		{
			GetGame().GetCallqueue().Remove(Glide);
			return;
		}

		vector here = me.GetOrigin();
		float pace = 1.6;
		if (s_fSpeed >= 3)
			pace = 5.2;
		else if (s_fSpeed >= 2)
			pace = 3.4;

		float step = pace * GLIDE_MS / 1000;
		if (RZ_Util.DistXZ(here, m_vGlideLast) > step * 0.4)
		{
			m_iSlow = 0;
			m_vGlideLast = here;
			return;
		}

		m_iSlow++;
		if (m_iSlow < 4)
			return;

		vector aim = s_vGoal;
		if (s_vSteer != vector.Zero)
			aim = s_vSteer;

		if (s_bDetour)
			aim = s_vDetour;

		vector way = aim - here;
		way[1] = 0;
		if (way.Length() < 0.05)
			return;

		way.Normalize();
		vector chest = here + Vector(0, 1.2, 0);
		if (RZ_CampLife.TraceHits(chest, chest + way * 0.8, false))
			return;	// a wall: the stuck check goes round it

		vector next = RZ_Util.GroundPos(here + way * step);
		if (next[1] - here[1] > 0.6)
			return;	// a step too high to walk up

		me.SetOrigin(next); // VERIFY: moving the local player's character by SetOrigin in small steps keeps it there (no snap back) in Workbench play
		vector mat[4];
		me.GetWorldTransform(mat);
		vector pos = mat[3];
		Math3D.AnglesToMatrix(Vector(Math.Atan2(way[0], way[2]) * Math.RAD2DEG, 0, 0), mat);
		mat[3] = pos;
		me.SetWorldTransform(mat);
		m_vGlideLast = next;
	}

	//------------------------------------------------------------------------------------------------
	//! The look-ahead: 24 ways round the goal's bearing (nearest first, 15 degrees apart), each traced
	//! at knee and chest height out to 20 m (walls, fences, cars, trees, props) and its ground checked
	//! every 2 m (no climb steeper than 45 degrees, no sea). The first way clear for 8 m (or to the goal)
	//! wins; with none, the longest clear one. A spot up to 10 m along it, or zero for straight on.
	protected static vector Plan(vector here, vector goal)
	{
		vector toGoal = goal - here;
		toGoal[1] = 0;
		float left = toGoal.Length();
		if (left < 1)
			return vector.Zero;

		float bearing = Math.Atan2(toGoal[0], toGoal[2]) * Math.RAD2DEG;
		float need = Math.Min(8, left);
		float bestClear = -1;
		vector bestDir = vector.Zero;
		for (int i = 0; i < 24; i++)
		{
			// 0, +15, -15, +30, -30 ... 180
			int half = (i + 1) / 2;
			float off = half * 15;
			if (i % 2 == 0)
				off = -off;

			float rad = (bearing + off) * Math.DEG2RAD;
			vector dir = Vector(Math.Sin(rad), 0, Math.Cos(rad));
			float clear = Clearance(here, dir, Math.Min(20, left + 1));
			if (clear >= need)
				return RZ_Util.GroundPos(here + dir * Math.Min(clear, 10));

			if (clear > bestClear)
			{
				bestClear = clear;
				bestDir = dir;
			}
		}

		if (bestClear < 1)
			return vector.Zero;

		return RZ_Util.GroundPos(here + bestDir * bestClear);
	}

	//------------------------------------------------------------------------------------------------
	//! How far (m) one can walk from `here` along `dir`, up to `max`: traces at knee and chest height,
	//! and the ground every 2 m (a rise of over 2 m in 2 m, or the sea, ends it).
	protected static float Clearance(vector here, vector dir, float max)
	{
		vector knee = here + Vector(0, 0.5, 0);
		vector chest = here + Vector(0, 1.3, 0);
		float hit = max;
		float k = TraceFrac(knee, knee + dir * max);
		float c = TraceFrac(chest, chest + dir * max);
		hit = Math.Min(k, c) * max;

		vector last = here;
		for (float d = 2; d <= hit; d += 2)
		{
			vector g = RZ_Util.GroundPos(here + dir * d);
			if (g[1] - last[1] > 2 || RZ_Util.IsWater(g))
				return d - 2;

			last = g;
		}

		return hit;
	}

	//------------------------------------------------------------------------------------------------
	protected static float TraceFrac(vector from, vector to)
	{
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = to;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		// Objects only: the ground itself (a rise, a dip) is the slope check's, or every way on rolling
		// ground looks blocked and the choice flips each look (seen in play: turning back and forth)
		param.Flags = TraceFlags.ENTS;
		return ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param);
	}

	//------------------------------------------------------------------------------------------------
	//! The way 2 m ahead at five heights (0.3, 0.7, 1.1, 1.5, 1.85 m): sets the stance to ask for, a
	//! jump for something low, and opens a door in the way.
	protected void Obstacle(IEntity me, vector here, vector toward)
	{
		vector dir = toward - here;
		dir[1] = 0;
		if (dir.Length() < 0.1)
			return;

		dir.Normalize();
		bool ankle = TraceFrac(here + Vector(0, 0.3, 0), here + Vector(0, 0.3, 0) + dir * 2) < 0.999;
		bool knee = TraceFrac(here + Vector(0, 0.7, 0), here + Vector(0, 0.7, 0) + dir * 2) < 0.999;
		bool waist = TraceFrac(here + Vector(0, 1.1, 0), here + Vector(0, 1.1, 0) + dir * 2) < 0.999;
		bool chest = TraceFrac(here + Vector(0, 1.5, 0), here + Vector(0, 1.5, 0) + dir * 2) < 0.999;
		bool head = TraceFrac(here + Vector(0, 1.85, 0), here + Vector(0, 1.85, 0) + dir * 2) < 0.999;

		int want = 1;
		if ((ankle || knee) && !chest && !head)
		{
			// Low: a fence, a low wall, a log: over it
			s_fJumpUntil = GetGame().GetWorld().GetWorldTime() + 400;
		}
		else if ((chest || head) && !ankle && !knee && !waist)
		{
			want = 2;	// a beam, a low roof, a pipe: under it, crouched
		}
		else if (waist && !ankle)
		{
			want = 3;	// room only at the ground: prone
		}
		else if (ankle && knee && waist && chest)
		{
			OpenDoorNear(me);	// a wall: a door in it, if there is one (the look-ahead goes round otherwise)
		}

		s_iStance = want;
	}

	// ================================================================ The path
	protected static const float CELL = 2.5;		// the grid's square (m)
	protected static const int HALF = 32;			// squares each way from the middle (80 m)
	protected static const int EXPAND_MAX = 1200;	// squares looked at before giving up

	//------------------------------------------------------------------------------------------------
	//! The waypoint to head for now, planning a path when there's none (or it's used up, or old).
	protected vector Follow(vector here)
	{
		s_fPathAge += 0.25;
		if (s_aPath.IsEmpty() || s_iPathAt >= s_aPath.Count() || s_fPathAge > 20)
		{
			s_aPath.Clear();
			s_iPathAt = 0;
			s_fPathAge = 0;
			vector target = s_vGoal;
			vector way = s_vGoal - here;
			way[1] = 0;
			if (way.Length() > 70)
			{
				way.Normalize();
				target = here + way * 70;
			}

			if (!BuildPath(here, RZ_Util.GroundPos(target), s_aPath))
			{
				s_aPath.Clear();
				return vector.Zero;
			}
		}

		while (s_iPathAt < s_aPath.Count() - 1 && RZ_Util.DistXZ(here, s_aPath[s_iPathAt]) < 2)
		{
			s_iPathAt++;
		}

		return s_aPath[s_iPathAt];
	}

	//------------------------------------------------------------------------------------------------
	//! A* over a 65 x 65 grid of 2.5 m squares round `from` to `to`: a step is open when nothing's in
	//! the way at knee and chest height (objects), the ground doesn't rise more than 1.5 m and isn't
	//! the sea. False when no way is found within the search; `result` gets the waypoints (every
	//! second square), the last one the reachable square nearest `to`.
	protected static bool BuildPath(vector from, vector to, notnull array<vector> result)
	{
		int side = HALF * 2 + 1;
		int n = side * side;
		array<float> g = {};
		array<float> hgt = {};
		array<int> parent = {};
		array<bool> closed = {};
		g.Resize(n);
		hgt.Resize(n);
		parent.Resize(n);
		closed.Resize(n);
		for (int i = 0; i < n; i++)
		{
			g[i] = 1000000;
			hgt[i] = -9999;
			parent[i] = -1;
			closed[i] = false;
		}

		int start = HALF * side + HALF;
		int goalX = HALF + RZ_Util.RoundToInt((to[0] - from[0]) / CELL);
		int goalZ = HALF + RZ_Util.RoundToInt((to[2] - from[2]) / CELL);
		goalX = Math.ClampInt(goalX, 0, side - 1);
		goalZ = Math.ClampInt(goalZ, 0, side - 1);
		int goal = goalZ * side + goalX;

		g[start] = 0;
		hgt[start] = from[1];
		array<int> open = {start};
		int best = start;
		float bestH = 1000000;
		int expanded = 0;
		while (!open.IsEmpty() && expanded < EXPAND_MAX)
		{
			// The open square with the least cost so far plus the straight distance left
			int pick = 0;
			float pickF = 1000000000;
			foreach (int oi, int o : open)
			{
				int ox = o % side;
				int oz = o / side;
				float f = g[o] + Math.Sqrt((ox - goalX) * (ox - goalX) + (oz - goalZ) * (oz - goalZ)) * CELL;
				if (f < pickF)
				{
					pickF = f;
					pick = oi;
				}
			}

			int cur = open[pick];
			open.Remove(pick);
			if (closed[cur])
				continue;

			closed[cur] = true;
			expanded++;
			int cx = cur % side;
			int cz = cur / side;
			float left = Math.Sqrt((cx - goalX) * (cx - goalX) + (cz - goalZ) * (cz - goalZ)) * CELL;
			if (left < bestH)
			{
				bestH = left;
				best = cur;
			}

			if (cur == goal)
				break;

			vector curPos = Vector(from[0] + (cx - HALF) * CELL, hgt[cur], from[2] + (cz - HALF) * CELL);
			for (int dz = -1; dz <= 1; dz++)
			{
				for (int dx = -1; dx <= 1; dx++)
				{
					if (dx == 0 && dz == 0)
						continue;

					int nx = cx + dx;
					int nz = cz + dz;
					if (nx < 0 || nz < 0 || nx >= side || nz >= side)
						continue;

					int nb = nz * side + nx;
					if (closed[nb])
						continue;

					if (hgt[nb] < -9000)
						hgt[nb] = RZ_Util.GroundPos(Vector(from[0] + (nx - HALF) * CELL, 0, from[2] + (nz - HALF) * CELL))[1];

					vector nbPos = Vector(from[0] + (nx - HALF) * CELL, hgt[nb], from[2] + (nz - HALF) * CELL);
					if (nbPos[1] - curPos[1] > 1.5 || RZ_Util.IsWater(nbPos))
						continue;

					vector knee = Vector(0, 0.6, 0);
					vector chest = Vector(0, 1.3, 0);
					if (TraceFrac(curPos + knee, nbPos + knee) < 0.999 || TraceFrac(curPos + chest, nbPos + chest) < 0.999)
						continue;

					float step = CELL;
					if (dx != 0 && dz != 0)
						step = CELL * 1.414;

					float cost = g[cur] + step + Math.AbsFloat(nbPos[1] - curPos[1]);
					if (cost < g[nb])
					{
						g[nb] = cost;
						parent[nb] = cur;
						open.Insert(nb);
					}
				}
			}
		}

		if (best == start)
			return false;

		// Back from the best square to the start, every second square kept
		array<vector> back = {};
		int at = best;
		int k = 0;
		while (at != -1 && at != start)
		{
			if (k % 2 == 0 || at == best)
			{
				int bx = at % side;
				int bz = at / side;
				back.Insert(Vector(from[0] + (bx - HALF) * CELL, hgt[at], from[2] + (bz - HALF) * CELL));
			}

			at = parent[at];
			k++;
		}

		for (int b = back.Count() - 1; b >= 0; b--)
		{
			result.Insert(back[b]);
		}

		return !result.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected void Stop(string why)
	{
		bool was = s_bGoing;
		s_bGoing = false;
		s_iStance = 1;
		if (!was && why != "stopped")
			return;

		Note(why);
	}

	//------------------------------------------------------------------------------------------------
	//! A line added to out.txt on its own (after whatever is there).
	protected void Note(string line)
	{
		m_iSeq++;
		FileHandle w = FileIO.OpenFile(OUT, FileMode.APPEND);
		if (!w)
			return;

		w.WriteLine(string.Format("#%1 %2", m_iSeq, line));
		w.Close();
	}

	//------------------------------------------------------------------------------------------------
	//! From the player's character controller each frame (below): while going, the character's own
	//! input is the way to the goal, at the asked speed; a jump while it's stuck. True when it drove.
	static bool Drive(SCR_CharacterControllerComponent ctrl, IEntity owner)
	{
		if (!s_bGoing || !ctrl || !owner)
			return false;

		PlayerController pc = GetGame().GetPlayerController();
		if (!pc || GetGame().GetPlayerManager().GetPlayerControlledEntity(pc.GetPlayerId()) != owner)
			return false;

		CharacterInputContext ctx = ctrl.GetInputContext();
		if (!ctx)
			return false;

		vector aim = s_vGoal;
		if (s_vSteer != vector.Zero)
			aim = s_vSteer;

		if (s_bDetour)
			aim = s_vDetour;

		vector way = aim - owner.GetOrigin();
		way[1] = 0;
		if (way.Length() < 0.01)
			return false;

		way.Normalize();
		// Turn to face the way (heading in radians round the world's up axis), and move along it in
		// the character's own frame, so it goes the right way even before the turn has finished
		float bearing = Math.Atan2(way[0], way[2]);
		ctrl.SetHeadingAngle(bearing, true); // VERIFY: CharacterControllerComponent.SetHeadingAngle turns a player character, the aim following (adjustAimingYaw)
		vector local = owner.VectorToLocal(way);
		local[1] = 0;
		// The controller's own movement, as the base game's cutscenes move a character (its speed: 1
		// walk, 2 run, 3 sprint); the input context too, so the frame's own input doesn't win
		ctrl.SetMovement(s_fSpeed, local); // VERIFY: CharacterControllerComponent.SetMovement from the control hooks moves a player character
		ctx.SetMovement(s_fSpeed, local);
		if (GetGame().GetWorld().GetWorldTime() < s_fJumpUntil)
			ctx.SetJump(1);

		// Crouch or prone to get under, back up after
		if (s_iStance != s_iStanceSet && ctrl.CanChangeStance(s_iStance))
		{
			ctrl.SetStanceChange(s_iStance); // VERIFY: SetStanceChange from the control hooks crouches, prones and stands a player character
			s_iStanceSet = s_iStance;
		}

		return true;
	}

	// ================================================================ Screens
	//------------------------------------------------------------------------------------------------
	protected void Ui(array<string> a)
	{
		MenuManager mm = GetGame().GetMenuManager();
		MenuBase top;
		if (mm)
			top = mm.GetTopMenu();

		if (!top)
		{
			Say("ui: no screen open");
			return;
		}

		string verb = "";
		string arg = "";
		if (a.Count() >= 2)
			verb = a[1];

		if (a.Count() >= 3)
		{
			for (int i = 2; i < a.Count(); i++)
			{
				if (i > 2)
					arg += " ";

				arg += a[i];
			}
		}

		if (verb == "close")
		{
			top.Close();
			Say("closed " + top.ClassName());
			return;
		}

		RZ_MenuBase ours = RZ_MenuBase.Cast(top);
		if (!ours)
		{
			Say("ui: " + top.ClassName() + " (not ours: only `ui close`)");
			return;
		}

		array<string> lines = {};
		if (!ours.DriverDo(verb, arg, lines))
			lines.Insert(string.Format("ui: %1 doesn't know '%2'", top.ClassName(), verb));

		Say("ui: " + top.ClassName());
		foreach (string l : lines)
		{
			Say("  " + l);
		}
	}

}

//------------------------------------------------------------------------------------------------
//! The driver walks the player through the character's own controls (Workbench only).
modded class SCR_CharacterControllerComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player)
	{
		super.OnPrepareControls(owner, am, dt, player);
		if (player)
			RZ_Driver.Drive(this, owner);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnApplyControls(IEntity owner, float timeSlice)
	{
		super.OnApplyControls(owner, timeSlice);
		RZ_Driver.Drive(this, owner);
	}
}
#endif
