//! Talk scenes and the story job's kit (docs/DESIGN.md, "Story blocks"): a scene person at a job's
//! place with 2-3 nodes, each a line and up to four options on its choice slots (the last is always
//! "Leave it", back to the first node); the speaker who holds the floor; lines to the speaker and
//! anyone within 8 m; the numbered fallback. RZ_StoryJob carries the job's truth, its background
//! reads and its scene, and the mission (RZ_Mission.m_Job) ticks it and routes its slots. Server only.

//------------------------------------------------------------------------------------------------
//! One option on a node: what it is, what it needs, where it goes.
class RZ_SceneOption
{
	string m_sLabel;		// the action part; the tag is added when the slots are labelled
	int m_iKind;			// RZ_TalkScene.ASK, TRUST, FEAR, FOOD, FAVOR, TRICK, DECIDE, WAY
	int m_iFaction = -1;	// TRUST, FAVOR, TRICK: whose; FEAR: the listener
	int m_iValue;			// TRUST, FEAR, TRICK: difficulty; FOOD, FAVOR: the price
	int m_iNext = -1;		// ASK: the node it opens (-1 stays)
	int m_iWay = -1;		// what the mission does on a pass (RZ_Mission.OnStoryWay)
	string m_sPass;			// said on a pass (or, for ASK, the reveal)
	string m_sFail;			// said on a fail
	bool m_bTruth;			// ASK: its line is a truth found (journal, the room)
	int m_iRoute = -1;		// DECIDE: the finale route its asset belongs to (-1 none)
	string m_sWhat;			// DECIDE: the hold's line ("leave the larder to the family")
	string m_sBackgrounds;	// SERVICE: who can do it ("nurse,doctor")
}

//------------------------------------------------------------------------------------------------
class RZ_SceneNode
{
	string m_sLine;
	ref array<ref RZ_SceneOption> m_aOptions = {};
}

//------------------------------------------------------------------------------------------------
//! The scene person: a stand-in (the civilian puzzle prefab) at m_vAt, spawned once someone comes
//! within 80 m, with the nodes on its choice slots (object index 91) and Talk, Object and Stand with
//! on its use slots. No prefab: no person, and the job goes on its own ways.
class RZ_TalkScene
{
	static const int OBJECT_INDEX = 91;
	static const int ASK = 0;		// opens a node, or reveals something: anyone in the room may pick it
	static const int TRUST = 1;
	static const int FEAR = 2;
	static const int FOOD = 3;		// pay tins: needs no standing
	static const int FAVOR = 4;
	static const int TRICK = 5;		// a claim in someone's kit (the look)
	static const int DECIDE = 6;	// decides the job: the decider rule and the 5 s hold
	static const int WAY = 7;		// a way with no check ("Agree to it")
	static const int SERVICE = 8;	// a background's work on the spot: worth 2 on the price table, a timed stand-by
	static const float SERVICE_RANGE = 4;
	static const float SHOW_RANGE = 80;
	static const float HEAR_RANGE = 8;
	static const float FLOOR_SECONDS = 60;
	static const float FLOOR_MAX = 180;
	static const int HOLD_BASE = 1000;	// scene holds pass HOLD_BASE + option index through RZ_Deciding

	string m_sName;			// "the scavenger"
	string m_sPerson;		// the person's key for one-try checks ("cache.scavenger")
	vector m_vAt;
	ref array<ref RZ_SceneNode> m_aNodes = {};
	bool m_bDone;			// a way passed or a decision settled: the person goes quiet
	protected int m_iNode;
	protected int m_iSpeaker = -1;
	protected float m_fFloor;		// seconds since the speaker's last pick
	protected float m_fHeld;		// seconds the speaker has held the floor in all
	protected ref array<string> m_aCutIn = {};
	protected IEntity m_Person;
	protected bool m_bShown;
	protected int m_iServing = -1;			// the player doing a service, -1 none
	protected float m_fServed;
	protected ref RZ_SceneOption m_Service;
	protected string m_sUse;
	protected string m_sChoice;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkScene(string name, string person, vector at)
	{
		m_sName = name;
		m_sPerson = person;
		m_vAt = at;
	}

	//------------------------------------------------------------------------------------------------
	//! A node: its line. Returns its index.
	int Node(string line)
	{
		RZ_SceneNode n = new RZ_SceneNode();
		n.m_sLine = line;
		m_aNodes.Insert(n);
		return m_aNodes.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_SceneOption Add(int node, string label, int kind)
	{
		if (node < 0 || node >= m_aNodes.Count() || m_aNodes[node].m_aOptions.Count() >= 3)
			return null;

		RZ_SceneOption o = new RZ_SceneOption();
		o.m_sLabel = label;
		o.m_iKind = kind;
		m_aNodes[node].m_aOptions.Insert(o);
		return o;
	}

	//------------------------------------------------------------------------------------------------
	//! A question: opens `next` (or stays), its line a truth found when `truth`.
	RZ_SceneOption Ask(int node, string label, int next, string line = "", bool truth = false)
	{
		RZ_SceneOption o = Add(node, label, ASK);
		if (!o)
			return null;

		o.m_iNext = next;
		o.m_sPass = line;
		o.m_bTruth = truth;
		return o;
	}

	//------------------------------------------------------------------------------------------------
	//! A way through at this person: a check (TRUST, FEAR, TRICK at `value`), a price (FOOD, FAVOR
	//! `value`) or none (WAY). Passed, the mission's OnStoryWay(way) runs.
	RZ_SceneOption Way(int node, string label, int kind, int faction, int value, int way, string pass, string fail = "")
	{
		RZ_SceneOption o = Add(node, label, kind);
		if (!o)
			return null;

		o.m_iFaction = faction;
		o.m_iValue = value;
		o.m_iWay = way;
		o.m_sPass = pass;
		o.m_sFail = fail;
		return o;
	}

	//------------------------------------------------------------------------------------------------
	//! A service (docs/DESIGN.md, "Story blocks"): someone with one of `backgrounds` does the work on
	//! the spot, staying within 4 m for `seconds`; worth 2 on the price table (two tins).
	RZ_SceneOption Service(int node, string label, string backgrounds, int seconds, int way, string pass)
	{
		RZ_SceneOption o = Add(node, label, SERVICE);
		if (!o)
			return null;

		o.m_sBackgrounds = backgrounds;
		o.m_iValue = seconds;
		o.m_iWay = way;
		o.m_sPass = pass;
		return o;
	}

	//------------------------------------------------------------------------------------------------
	//! A decision: someone it was offered to who took part, after the 5 s hold anyone may object to.
	RZ_SceneOption Decide(int node, string label, string what, int way, string pass, int route = -1)
	{
		RZ_SceneOption o = Add(node, label, DECIDE);
		if (!o)
			return null;

		o.m_sWhat = what;
		o.m_iWay = way;
		o.m_sPass = pass;
		o.m_iRoute = route;
		return o;
	}

	//------------------------------------------------------------------------------------------------
	//! Every tick: the person turns up near a player, the floor runs out, the use slots follow the hold.
	void Tick(RZ_GameModeComponent gm, RZ_Mission m, float dt, array<IEntity> players)
	{
		if (m_bDone)
		{
			if (m_Person)
				Cleanup();

			return;
		}

		if (!m_bShown)
		{
			if (m_aNodes.IsEmpty() || !RZ_ContractHelp.AnyNear(players, m_vAt, SHOW_RANGE))
				return;

			// Inside the body budget (a scene's person): no room, and they turn up when there is
			ResourceName prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
			RZ_Shared shared = RZ_Shared.Get();
			if (!prefab.IsEmpty() && shared && !shared.CanSpawnBody(RZ_BodyBudget.SCENE, m_vAt))
				return;

			m_bShown = true;
			if (!prefab.IsEmpty())
				m_Person = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(m_vAt), Math.RandomFloat(0, 360));

			if (m_Person && shared)
				shared.AddBody(RZ_BodyBudget.SCENE, m_Person);

			if (!m_Person)
			{
				m_bDone = true;
				RZ_Log.Info(string.Format("Story: %1 scene with %2 has no person (no civilian prefab)", m.m_sTitle, m_sName));
				return;
			}

			Label(gm, m);
		}

		TickService(gm, m, dt);
		if (m_iSpeaker > 0)
		{
			m_fFloor += dt;
			m_fHeld += dt;
			if (m_fFloor >= FLOOR_SECONDS || m_fHeld >= FLOOR_MAX)
				Release(gm, m);
		}

		Label(gm, m);
	}

	//------------------------------------------------------------------------------------------------
	//! "nurse,doctor" as a tag reads it: "Nurse or Doctor".
	protected static string Who(string backgrounds)
	{
		array<string> ids = {};
		backgrounds.Split(",", ids, true);
		string text = "";
		foreach (int i, string id : ids)
		{
			string name = RZ_Util.Capitalize(id);
			if (id == "excon")
				name = "Ex-Con";
			else if (id == "labtech")
				name = "Lab Tech";
			else if (id == "radioop")
				name = "Radio Op";

			if (i > 0)
				text += " or ";

			text += name;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A service under way: done after its seconds beside the person; walking off ends it.
	protected void TickService(RZ_GameModeComponent gm, RZ_Mission m, float dt)
	{
		if (m_iServing <= 0 || !m_Service)
			return;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iServing);
		if (!me || !m_Person || RZ_Util.DistXZ(me.GetOrigin(), m_Person.GetOrigin()) > SERVICE_RANGE)
		{
			gm.NotifyPlayer(m_iServing, m.m_sTitle, "You walked off halfway. They'll want it finished, or nothing.");
			m_iServing = -1;
			return;
		}

		m_fServed += dt;
		if (m_fServed < m_Service.m_iValue)
			return;

		int who = m_iServing;
		m_iServing = -1;
		Tell(gm, m, who, m_Service.m_sPass);
		m_bDone = true;
		m.m_Job.m_Ways.Remember(gm, who, RZ_EQuestApproach.TRADE);
		RZ_Log.Info(string.Format("Story: %1 service %2 at %3 by %4", m.m_sTitle, m_Service.m_iWay, m_sName, RZ_Util.PlayerName(who)));
		m.OnStoryWay(gm, who, m_Service.m_iWay);
	}

	//------------------------------------------------------------------------------------------------
	//! The slots' labels: the node's options and "Leave it"; Talk, and Object and Stand with while a
	//! hold runs. The numbered fallback (Missions > Story > Numbered scenes) reads "Option 1"...
	protected void Label(RZ_GameModeComponent gm, RZ_Mission m)
	{
		if (!m_Person)
			return;

		string use = "Talk";
		if (m.m_Story && m.m_Story.m_Deciding.Holder() > 0)
			use = "Talk|Object|Stand with";

		string choice = "";
		array<string> labels = {};
		Labels(gm, m_iNode, labels);
		foreach (int i, string l : labels)
		{
			if (i > 0)
				choice += "|";

			if (Numbered())
				choice += string.Format("Option %1", i + 1);
			else
				choice += l;
		}

		if (use == m_sUse && choice == m_sChoice)
			return;

		m_sUse = use;
		m_sChoice = choice;
		RZ_PuzzleObjectComponent.Tag(m_Person, m.m_iId, OBJECT_INDEX, use, choice);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Numbered()
	{
		return RZ_MissionStorySettings.Get().m_bNumberedScenes;
	}

	//------------------------------------------------------------------------------------------------
	protected void Labels(RZ_GameModeComponent gm, int node, notnull array<string> result)
	{
		if (node < 0 || node >= m_aNodes.Count())
			return;

		foreach (RZ_SceneOption o : m_aNodes[node].m_aOptions)
		{
			result.Insert(Tagged(gm, o));
		}

		result.Insert("Leave it");
	}

	//------------------------------------------------------------------------------------------------
	protected static string Tagged(RZ_GameModeComponent gm, RZ_SceneOption o)
	{
		if (o.m_iKind == TRUST)
			return string.Format("%1 %2", RZ_Checks.TrustTag(o.m_iFaction, o.m_iValue), o.m_sLabel);

		if (o.m_iKind == FEAR)
			return string.Format("%1 %2", RZ_Checks.FearTag(gm, -1, o.m_iFaction), o.m_sLabel);

		if (o.m_iKind == FOOD)
			return string.Format("%1 %2", RZ_Checks.PriceTag(o.m_iValue, "food"), o.m_sLabel);

		if (o.m_iKind == FAVOR)
			return string.Format("[%1 %2 Favor] %3", o.m_iValue, RZ_Factions.GetShortName(o.m_iFaction), o.m_sLabel);

		if (o.m_iKind == TRICK)
			return string.Format("[%1 kit] %2", RZ_Factions.GetShortName(o.m_iFaction), o.m_sLabel);

		if (o.m_iKind == SERVICE)
			return string.Format("[%1 - %2 s] %3", Who(o.m_sBackgrounds), o.m_iValue, o.m_sLabel);

		return o.m_sLabel;
	}

	//------------------------------------------------------------------------------------------------
	//! The node's line to the speaker and anyone within 8 m (the numbered fallback lists the options).
	protected void Say(RZ_GameModeComponent gm, RZ_Mission m, int playerId, string line)
	{
		string text = line;
		if (Numbered())
		{
			array<string> labels = {};
			Labels(gm, m_iNode, labels);
			foreach (int i, string l : labels)
			{
				text += string.Format("\n%1. %2", i + 1, l);
			}
		}

		Tell(gm, m, playerId, text);
	}

	//------------------------------------------------------------------------------------------------
	protected void Tell(RZ_GameModeComponent gm, RZ_Mission m, int playerId, string text)
	{
		if (text.IsEmpty())
			return;

		vector at = m_vAt;
		if (m_Person)
			at = m_Person.GetOrigin();

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] == playerId || RZ_Util.DistXZ(p.GetOrigin(), at) <= HEAR_RANGE)
				gm.NotifyPlayerAs(ids[i], m_sName, text, m.NoticeTag());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A use slot: 0 Talk (the node's line again, to you), 1 Object, 2 Stand with (while a hold runs).
	void Use(RZ_GameModeComponent gm, RZ_Mission m, int playerId, int value)
	{
		if (m_bDone)
			return;

		if (value == 0)
		{
			if (m.m_Story)
				m.m_Story.m_Deciding.Step(gm, playerId);

			Say(gm, m, playerId, m_aNodes[m_iNode].m_sLine);
			return;
		}

		if (!m.m_Story)
			return;

		if (value == 2)
		{
			m.m_Story.m_Deciding.StandWith(playerId);
			gm.NotifyPlayer(playerId, m.m_sTitle, "You stand with them. Whatever comes of it is yours too.");
			return;
		}

		// An objector takes the floor for 60 s
		if (value == 1 && m.m_Story.m_Deciding.Object(gm, m, playerId))
		{
			m_iSpeaker = playerId;
			m_fFloor = 0;
			m_fHeld = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A choice slot: the speaker rule, then what the option does.
	void Handle(RZ_GameModeComponent gm, RZ_Mission m, int playerId, int option)
	{
		if (m_bDone || m_iNode < 0 || m_iNode >= m_aNodes.Count())
			return;

		RZ_SceneNode node = m_aNodes[m_iNode];
		if (m.m_Story)
			m.m_Story.m_Deciding.Step(gm, playerId);

		// The last slot is always "Leave it": back to the first node, and the floor is let go
		if (option >= node.m_aOptions.Count())
		{
			if (playerId == m_iSpeaker || m_iSpeaker < 0)
			{
				m_iNode = 0;
				Release(gm, m);
				Say(gm, m, playerId, m_aNodes[0].m_sLine);
			}

			return;
		}

		RZ_SceneOption o = node.m_aOptions[option];
		bool revealOnly = o.m_iKind == ASK;
		if (!revealOnly && !TakeFloor(gm, m, playerId))
			return;

		if (revealOnly)
		{
			if (o.m_bTruth && m.m_Story && m.m_Job)
				m.m_Story.Reveal(gm, m, m.m_Job.m_sSubject, playerId, "talk", o.m_sPass);
			else
				Tell(gm, m, playerId, o.m_sPass);

			// Only the speaker (or anyone, with nobody holding it) moves the scene on
			if (o.m_iNext >= 0 && (m_iSpeaker < 0 || playerId == m_iSpeaker))
			{
				m_iNode = o.m_iNext;
				Label(gm, m);
				Say(gm, m, playerId, m_aNodes[m_iNode].m_sLine);
			}

			return;
		}

		if (o.m_iKind == DECIDE)
		{
			if (!RZ_StoryKit.MayDecide(gm, m, playerId))
				return;

			string refusal = RZ_RouteAssets.Refuse(gm, playerId, o.m_iRoute);
			if (!refusal.IsEmpty())
			{
				gm.NotifyPlayer(playerId, m.m_sTitle, refusal);
				return;
			}

			if (!m.m_Story)
			{
				Settle(gm, m, playerId, m_iNode * 10 + option);
				return;
			}

			m.m_Story.m_Deciding.StartHold(gm, m, playerId, HOLD_BASE + m_iNode * 10 + option, o.m_sWhat);
			return;
		}

		if (o.m_iKind == SERVICE)
		{
			if (!RZ_QuestApproach.HasBackground(gm, playerId, o.m_sBackgrounds))
			{
				gm.NotifyPlayer(playerId, m.m_sTitle, "You don't know how. Somebody with the trade might.");
				return;
			}

			m_iServing = playerId;
			m_fServed = 0;
			m_Service = o;
			gm.NotifyPlayer(playerId, m.m_sTitle, string.Format("You get to work. Stay with them a while (%1 s).", o.m_iValue));
			return;
		}

		bool pass = Try(gm, m, playerId, o);
		if (!pass)
		{
			Tell(gm, m, playerId, o.m_sFail);
			return;
		}

		Tell(gm, m, playerId, o.m_sPass);
		m_bDone = true;
		RZ_Log.Info(string.Format("Story: %1 way %2 at %3 by %4", m.m_sTitle, o.m_iWay, m_sName, RZ_Util.PlayerName(playerId)));
		m.OnStoryWay(gm, playerId, o.m_iWay);
	}

	//------------------------------------------------------------------------------------------------
	//! A scene hold ran out unobjected (RZ_Mission.OnHoldDone): the decision stands.
	void HoldDone(RZ_GameModeComponent gm, RZ_Mission m, int decider, int code)
	{
		Settle(gm, m, decider, code - HOLD_BASE);
	}

	//------------------------------------------------------------------------------------------------
	protected void Settle(RZ_GameModeComponent gm, RZ_Mission m, int decider, int code)
	{
		int nodeIdx = code / 10;
		int optIdx = code % 10;
		if (m_bDone || nodeIdx < 0 || nodeIdx >= m_aNodes.Count() || optIdx >= m_aNodes[nodeIdx].m_aOptions.Count())
			return;

		RZ_SceneOption o = m_aNodes[nodeIdx].m_aOptions[optIdx];
		m_bDone = true;
		Tell(gm, m, decider, o.m_sPass);
		if (decider > 0 && o.m_iRoute >= 0)
			RZ_RouteAssets.Add(gm, decider, o.m_iRoute, m.m_sTitle, RZ_FinaleAssets.ALLY);

		RZ_Log.Info(string.Format("Story: %1 decided %2 at %3 (by %4)", m.m_sTitle, o.m_iWay, m_sName, RZ_Util.PlayerName(decider)));
		m.OnStoryWay(gm, decider, o.m_iWay);
	}

	//------------------------------------------------------------------------------------------------
	//! The floor: the first eligible participant to pick keeps it for 60 s after each pick (3 minutes
	//! at most). Anyone else may only ask, or object during a hold. False (and told) if it isn't yours.
	protected bool TakeFloor(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (m_iSpeaker == playerId)
		{
			m_fFloor = 0;
			return true;
		}

		if (m_iSpeaker > 0 && GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iSpeaker))
		{
			gm.NotifyPlayer(playerId, m.m_sTitle, string.Format("%1 is talking to them. Ask, or wait your turn.", RZ_Util.PlayerName(m_iSpeaker)));
			return false;
		}

		if (!m.IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m.m_sTitle, "It's not your call. You weren't hired for this.");
			return false;
		}

		m_iSpeaker = playerId;
		m_fFloor = 0;
		m_fHeld = 0;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Release(RZ_GameModeComponent gm, RZ_Mission m)
	{
		m_iSpeaker = -1;
		m_fFloor = 0;
		m_fHeld = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A check or price at this person, once per player per job (RZ_Ways).
	protected bool Try(RZ_GameModeComponent gm, RZ_Mission m, int playerId, RZ_SceneOption o)
	{
		RZ_Ways ways = m.m_Job.m_Ways;
		if (o.m_iKind == WAY)
			return true;

		if (o.m_iKind == TRUST)
			return ways.TryTrust(gm, playerId, m_sPerson, o.m_iFaction, o.m_iValue, "") == RZ_ECheck.PASS;

		if (o.m_iKind == FEAR)
		{
			RZ_StoryKit.Dirty(gm, m, playerId);
			return ways.TryFear(gm, playerId, m_sPerson, o.m_iFaction, m.m_eGiver, o.m_iValue, false) == RZ_ECheck.PASS;
		}

		if (o.m_iKind == TRICK)
		{
			RZ_StoryKit.Dirty(gm, m, playerId);
			return ways.PassAs(gm, playerId, m_sPerson, o.m_iFaction, o.m_iValue, "") == RZ_ECheck.PASS;
		}

		if (o.m_iKind == FAVOR)
			return ways.TryBribe(gm, playerId, o.m_iFaction, o.m_iValue);

		if (o.m_iKind == FOOD)
		{
			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (me && RZ_HavenQuest.CountFood(gm, me) >= o.m_iValue && RZ_StoryStats.TakeFood(gm, me, o.m_iValue))
			{
				ways.Remember(gm, playerId, RZ_EQuestApproach.TRADE);
				return true;
			}

			gm.NotifyPlayer(playerId, m.m_sTitle, string.Format("You haven't got %1 tins on you.", o.m_iValue));
			return false;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		RZ_Leaving.Remove(m_Person);
		m_Person = null;
	}
}

//------------------------------------------------------------------------------------------------
//! One background read at a story job's place: once per player, to that player alone.
class RZ_StoryReadRow
{
	string m_sTruth;		// "" any truth
	string m_sBackgrounds;	// "nurse,doctor"
	string m_sLine;
}

//------------------------------------------------------------------------------------------------
//! A story job's kit beyond RZ_MissionStory (docs/DESIGN.md, "Story blocks"): the truth drawn for it,
//! its background reads at one place, its talk scene, and its ways' one-try book. A mission sets
//! m_Job in its constructor; RZ_Mission ticks it and routes the scene's slots.
class RZ_StoryJob
{
	string m_sTruth;
	string m_sSubject;
	vector m_vReadAt;
	float m_fReadRange = 30;
	ref RZ_Ways m_Ways = new RZ_Ways();
	ref RZ_TalkScene m_Scene;
	protected ref array<ref RZ_StoryReadRow> m_aReads = {};
	protected ref array<int> m_aRead = {};

	//------------------------------------------------------------------------------------------------
	//! The truth at `subject` (the deck, kept per subject; the first value is "as they said"). A value
	//! whose weight is 0 (its gate failed) is never drawn.
	string Draw(RZ_Mission m, string subject, notnull array<string> values, notnull array<float> weights)
	{
		m_sSubject = subject;
		m_Ways.m_sTitle = m.m_sTitle;
		m_sTruth = values[0];
		if (m.m_Story)
			m_sTruth = m.m_Story.Truth(subject, values, weights, m);

		int idx = values.Find(m_sTruth);
		if (idx < 0 || weights[idx] <= 0)
			m_sTruth = values[0];

		RZ_Log.Info(string.Format("Story: %1 at %2 is %3", m.m_Story.m_sKey, subject, m_sTruth));
		return m_sTruth;
	}

	//------------------------------------------------------------------------------------------------
	//! A background read at m_vReadAt for this truth ("" for any).
	void Read(string truth, string backgrounds, string line)
	{
		RZ_StoryReadRow r = new RZ_StoryReadRow();
		r.m_sTruth = truth;
		r.m_sBackgrounds = backgrounds;
		r.m_sLine = line;
		m_aReads.Insert(r);
	}

	//------------------------------------------------------------------------------------------------
	//! The job's talk scene, with its person at `at`.
	RZ_TalkScene Scene(string name, string person, vector at)
	{
		m_Scene = new RZ_TalkScene(name, person, at);
		return m_Scene;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(RZ_GameModeComponent gm, RZ_Mission m, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_Scene)
			m_Scene.Tick(gm, m, dt, players);

		Breakers(gm, m, dt, ids, players);

		if (m_aReads.IsEmpty() || m_vReadAt == vector.Zero)
			return;

		foreach (int i, IEntity p : players)
		{
			if (m_aRead.Contains(ids[i]) || RZ_Util.DistXZ(p.GetOrigin(), m_vReadAt) > m_fReadRange)
				continue;

			m_aRead.Insert(ids[i]);
			foreach (RZ_StoryReadRow r : m_aReads)
			{
				if (r.m_sTruth.IsEmpty() || r.m_sTruth == m_sTruth)
					RZ_StoryKit.Read(gm, m, ids[i], r.m_sBackgrounds, r.m_sLine);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A breaker: a participant on the sneak (hidden, built stealth) within 25 m of another player
	//! passing as the job's giver or its rival is a catch: the look is broken (Caught), once per player
	//! per job. Every 2 s.
	protected void Breakers(RZ_GameModeComponent gm, RZ_Mission m, float dt, array<int> ids, array<IEntity> players)
	{
		m_fBreakers += dt;
		if (m_fBreakers < 2 || !gm.GetHostility())
			return;

		m_fBreakers = 0;
		RZ_Disguise d = gm.GetHostility().GetDisguise();
		foreach (int i, IEntity watcher : players)
		{
			if (!m.m_aParticipants.Contains(ids[i]) || !RZ_Stealth.IsHidden(ids[i]) || RZ_Util.DistXZ(watcher.GetOrigin(), m.m_vPos) > 150)
				continue;

			foreach (int j, IEntity other : players)
			{
				if (i == j || m_aBroken.Contains(ids[j]) || RZ_Util.DistXZ(watcher.GetOrigin(), other.GetOrigin()) > BREAK_RANGE)
					continue;

				int f = -1;
				if (d.IsPassing(ids[j], m.m_eGiver))
					f = m.m_eGiver;
				else if (m.m_bHasRival && d.IsPassing(ids[j], m.m_eRival))
					f = m.m_eRival;

				if (f < 0)
					continue;

				m_aBroken.Insert(ids[j]);
				d.Caught(ids[j], f, other, string.Format("\"Funny way to wear their colours.\" %1 was watching from cover.", RZ_Util.PlayerName(ids[i])));
				gm.NotifyPlayer(ids[i], m.m_sTitle, "From cover you watch them walk, and it's wrong: the boots, the hands. Not one of theirs. You say so.");
				RZ_Log.Info(string.Format("Story: %1 breaker: %2 caught %3", m.m_sTitle, RZ_Util.PlayerName(ids[i]), RZ_Util.PlayerName(ids[j])));
			}
		}
	}

	static const float BREAK_RANGE = 25;
	protected float m_fBreakers;
	protected ref array<int> m_aBroken = {};

	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		if (m_Scene)
			m_Scene.Cleanup();
	}
}
