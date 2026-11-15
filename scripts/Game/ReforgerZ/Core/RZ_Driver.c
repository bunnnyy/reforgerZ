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

	// Going somewhere on foot (read by the character controller each frame, below)
	protected static bool s_bGoing;
	protected static vector s_vGoal;
	protected static float s_fSpeed;
	protected static float s_fStopAt = 3;
	protected static float s_fJumpUntil;
	protected vector m_vLastPos;
	protected float m_fStillFor;
	protected int m_iJumps;

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
			case "leave": Pick(RZ_Dialogue.LEAVE); return;
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
		m_vLastPos = me.GetOrigin();
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
		if (left <= s_fStopAt)
		{
			Stop(string.Format("arrived (%1 m off) at %2", RZ_Util.RoundToInt(left), Pos(here)));
			return;
		}

		RZ_Dialogue talk = m_GM.GetDialogue();
		if (talk && talk.SessionOf(Me()))
		{
			Stop("someone is talking to us: stopped at " + Pos(here));
			return;
		}

		if (RZ_Util.DistXZ(here, m_vLastPos) < 0.3)
		{
			m_fStillFor += 0.25;
			if (m_fStillFor >= 1)
			{
				m_fStillFor = 0;
				m_iJumps++;
				s_fJumpUntil = GetGame().GetWorld().GetWorldTime() + 400;
				if (m_iJumps >= 6)
					Stop("stuck at " + Pos(here) + ", " + RZ_Util.RoundToInt(left).ToString() + " m to go");
			}
		}
		else
		{
			m_fStillFor = 0;
			m_iJumps = 0;
		}

		m_vLastPos = here;
	}

	//------------------------------------------------------------------------------------------------
	protected void Stop(string why)
	{
		bool was = s_bGoing;
		s_bGoing = false;
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

		vector way = s_vGoal - owner.GetOrigin();
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
		ctx.SetMovement(s_fSpeed, local); // VERIFY: CharacterInputContext.SetMovement from OnPrepareControls moves a player character (1 walk, 2 run, 3 sprint)
		if (GetGame().GetWorld().GetWorldTime() < s_fJumpUntil)
			ctx.SetJump(1);

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
}
#endif
