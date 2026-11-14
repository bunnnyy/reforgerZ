//! World events about people (docs/MISSIONS.md, "World events"): Distress Call (its three
//! truths), Prison Break, The Sniper, Rival Survivors, Collapse Shelter. AI groups are data until a
//! player comes near (RZ_EventGroup); people you talk to are the puzzle framework's objects.

//------------------------------------------------------------------------------------------------
//! An AI group an event spawns only once a player is near, and watches until it's dead.
class RZ_EventGroup
{
	ResourceName m_sPrefab;
	vector m_vPos;				// where it spawns (and, before then, where it is as data)
	IEntity m_Unit;
	bool m_bSpawned;
	float m_fGrace = 15;		// groups spawn their members a frame later: don't count them dead yet

	//------------------------------------------------------------------------------------------------
	void RZ_EventGroup(ResourceName prefab, vector pos)
	{
		m_sPrefab = prefab;
		m_vPos = pos;
	}

	//------------------------------------------------------------------------------------------------
	bool Spawn()
	{
		if (m_bSpawned)
			return m_Unit != null;

		m_bSpawned = true;
		m_Unit = RZ_Util.SpawnPrefab(m_sPrefab, m_vPos, Math.RandomFloat(0, 360));
		return m_Unit != null;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		if (m_bSpawned && m_fGrace > 0)
			m_fGrace -= dt;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDead()
	{
		if (!m_bSpawned || m_fGrace > 0)
			return false;

		return !m_Unit || RZ_Util.CountAlive(m_Unit) <= 0;
	}

	//------------------------------------------------------------------------------------------------
	vector GetPos()
	{
		if (m_Unit && RZ_Util.CountAlive(m_Unit) > 0)
			return RZ_Util.GetUnitPosition(m_Unit);

		return m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	void Delete()
	{
		RZ_Util.DeleteUnit(m_Unit);
		m_Unit = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Every group spawned and dead.
	static bool AllDead(array<ref RZ_EventGroup> groups)
	{
		if (groups.IsEmpty())
			return false;

		foreach (RZ_EventGroup g : groups)
		{
			if (!g.IsDead())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static int CountAlive(array<ref RZ_EventGroup> groups)
	{
		int n = 0;
		foreach (RZ_EventGroup g : groups)
		{
			if (!g.m_bSpawned || !g.IsDead())
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	static void DeleteAll(array<ref RZ_EventGroup> groups)
	{
		foreach (RZ_EventGroup g : groups)
		{
			g.Delete();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! The distress call's announcement and the Radio Operator's ear: one voice, whatever the truth
//! behind it (live, a loop or a crew telling him what to say).
class RZ_Event_DistressBase : RZ_PuzzleMission
{
	protected vector m_vSpot;
	protected string m_sCaller;
	protected ref array<int> m_aWarned = {};

	protected static const string RADIO_OP = "radioham";
	protected static const float HINT_RADIUS = 1500;

	//------------------------------------------------------------------------------------------------
	protected bool PickSpot(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vSpot = LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius + 150);
		m_vPos = m_vSpot;
		m_sCaller = RZ_NoteText.FirstName(m_Rng);
		m_sTitle = "Distress Call";
		m_sBrief = string.Format("A voice on the open band, from just outside %1: \"This is %2. There's two of us, one's hurt, and they're all round the house. Please. Anyone.\" It comes again every few minutes.", s.m_sName, m_sCaller);
		m_sObjective = string.Format("Get %1 and the injured friend out of the house outside %2.", m_sCaller, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! What a Radio Operator hears in it. Override.
	protected string RadioOpHint()
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Radio Operators online now, and any who come within range later, get their hint once.
	protected void HintRadioOps(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aWarned.Contains(playerId) || !RZ_Interactions.IsBackground(gm, playerId, RADIO_OP))
				continue;

			if (RZ_Util.DistXZ(p.GetOrigin(), m_vSpot) > HINT_RADIUS && m_fClock > 5)
				continue;

			m_aWarned.Insert(playerId);
			gm.NotifyPlayer(playerId, m_sTitle + " (Radio Operator)", RadioOpHint());
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The voice on the band near " + GetLocationName(gm) + " has stopped.";
	}
}

//------------------------------------------------------------------------------------------------
//! #5 Distress Call (Fake Distress Call merged in): one announcement, three truths kept per town for a
//! season day. Real: two survivors cornered in a house; clear the infected and talk them out. A loop:
//! a tape on a crate and two Red Hand groups in the trees. Bait: two real people tied up in the house,
//! and the crew waiting for whoever cuts them loose. Answering on your radio (on the way in, 800 m)
//! tells the three apart; so does a Radio Operator's ear, a Hunter at 150 m and, for a Trusted Red
//! Hand player, the secure net. 25 minutes. Story key `distress`.
class RZ_Event_DistressCall : RZ_Event_DistressBase
{
	protected string m_sTruth;
	protected bool m_bStirred;
	protected bool m_bSprung;
	protected bool m_bFreed;
	protected bool m_bSold;
	protected int m_iDead;
	protected ref array<ref RZ_EventGroup> m_aGroups = {};
	protected ref array<int> m_aSpared = {};
	protected ref array<int> m_aAnswered = {};
	protected ref array<int> m_aHunted = {};
	protected ref array<int> m_aSeenBite = {};
	protected ref RZ_Ways m_Ways = new RZ_Ways("Distress Call");
	protected bool m_bBitten;		// Real: the man's "fall" is a bite (one time in four), a fixed fact
	protected int m_iHidBite = -1;	// the Nurse who dressed it and said nothing
	protected int m_iWhere;			// where they go: WHERE_*
	protected int m_iSister = -1;	// "My brother's in {town}": another house

	protected static const int WHERE_NEAREST = 0;
	protected static const int WHERE_SISTER = 1;
	protected static const int WHERE_BASTION = 2;
	protected static const int HID_INFAMY = 8;

	protected static const float CLEAR_RADIUS = 20;
	protected static const float SPAWN_RADIUS = 400;
	protected static const float SPRING_RADIUS = 60;
	protected static const float ANSWER_RADIUS = 800;
	protected static const float HUNTER_RADIUS = 150;
	protected static const int BUY_FAVOR = 30;
	protected static const int CUT_FAVOR = 10;
	protected static const int CUT_INFAMY = 12;

	protected static const string REAL = "real";
	protected static const string LOOP = "loop";
	protected static const string BAIT = "bait";

	//------------------------------------------------------------------------------------------------
	void RZ_Event_DistressCall()
	{
		m_Story = new RZ_MissionStory("distress");
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1500;
		m_fParticipationRadius = 80;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 1.1;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanBait(RZ_GameModeComponent gm)
	{
		return !gm.GetRedHandGroupPrefab().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!PickSpot(gm))
			return false;

		// The truth, kept per town (the deck): a loop or bait needs the crew, and a loop the radio prop
		array<string> values = {REAL, LOOP, BAIT};
		array<float> weights = {0.5, 0, 0};
		if (CanBait(gm))
		{
			if (HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
				weights[1] = 0.3;

			weights[2] = 0.2;
			// The Cargo driver is out (R2): fewer people to tie up
			if (gm.GetQuestlines() && gm.GetQuestlines().WorldFlag("cargo.driver") == 1)	// DRIVER_OUT
				weights[2] = 0.1;
		}

		string town = "town:" + m_iSettlement.ToString();
		m_sTruth = m_Story.Truth(town, values, weights, this);
		if ((m_sTruth == LOOP && weights[1] <= 0) || (m_sTruth == BAIT && weights[2] <= 0))
			m_sTruth = REAL;

		if (m_sTruth == LOOP)
		{
			int look = AddClue("A radio", string.Format("A radio on an upturned crate, a cassette player taped to the handset. \"This is %1. There's two of us...\" Click. Hiss. \"This is %1.\" Nobody's here. Nobody ever was.", m_sCaller));
			if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vSpot, 0, "", "Switch it off", look, "Look at the radio"))
				return false;
		}
		else if (m_sTruth == BAIT)
		{
			int tied = AddClue(m_sCaller, string.Format("%1 and a man, tied back to back to a radiator. %1 shakes his head at you, very slightly, and looks at the window. \"They're in the trees,\" he breathes.", m_sCaller));
			if (!SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vSpot, 0, "", "Cut them loose|[Known · Red Hand] \"They're mine.\"|Buy them [30 Red Hand Favor]|Side with the crew", tied, "Look at them"))
				return false;
		}
		else
		{
			int talk = AddClue(m_sCaller, string.Format("%1, through a barricaded door, very quietly: \"Are they gone? Tell me when they're gone. He can't run.\"", m_sCaller));
			if (!SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vSpot, 0, "", "Tell them it's clear|[Sneak] Out the back, now|[Nurse] Dress his leg, say nothing", talk, "Talk to " + m_sCaller))
				return false;

			// Real: where they go is one choice when they're out; his leg is a fixed fact
			m_bBitten = m_Rng.Chance(0.25);
			m_iSister = SisterHouse(gm);
			string sister = "Bastion's post: \"the soldiers\"";
			string labels = "The nearest house";
			if (m_iSister >= 0)
				labels += "|" + string.Format("My brother's in %1", gm.GetSafehouses().Get(m_iSister).m_sName);
			else
				labels += "|The nearest house, the long way";

			labels += "|" + sister;
			m_Choice = new RZ_OneChoice();
			m_Choice.Set(string.Format("They're out. %1 holds the man up and looks at you: \"Where now? My brother's house is further, but he's there. Or the soldiers.\" Answer him here.", m_sCaller), labels, "decide where they go", WHERE_NEAREST);
		}

		if (m_sTruth != REAL)
		{
			for (int i = 0; i < 2; i++)
			{
				m_aGroups.Insert(new RZ_EventGroup(gm.GetRedHandGroupPrefab(), LandPointNear(m_vSpot, 50, 90)));
			}

			TellTheNet(gm);
		}

		RZ_Log.Info(string.Format("Story: distress at %1 is %2", GetLocationName(gm), m_sTruth));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A Trusted Red Hand player hears the secure net: the house is spoken for.
	protected void TellTheNet(RZ_GameModeComponent gm)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int playerId : ids)
		{
			if (RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED))
				gm.NotifyPlayer(playerId, "Red Hand (secure)", string.Format("Leave the house outside %1 alone. It's spoken for.", GetLocationName(gm)));
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected string RadioOpHint()
	{
		if (m_sTruth == LOOP)
			return "Same breath in the same place, the same crackle after \"Anyone\", every time. That's a tape. The Red Hand baits people with recordings like this. Go in careful, or don't go.";

		if (m_sTruth == BAIT)
			return "It's live. But there's a gap before every answer, and once a man's voice, off the mic, says \"Say it again.\" Someone is telling him what to say.";

		return "The voice breaks in different places every time, and once there's banging behind it. It's live. Someone really is out there.";
	}

	//------------------------------------------------------------------------------------------------
	//! What your own radio gets when you answer the call (on the way in).
	protected string RadioAnswer()
	{
		if (m_sTruth == LOOP)
			return string.Format("You answer on the open band. \"This is %1. There's two of us...\" It doesn't stop for you. It doesn't answer.", m_sCaller);

		if (m_sTruth == BAIT)
			return string.Format("You answer on the open band. A young man starts to reply, \"Thank God, we're...\" and a man's voice, further off, says \"Say it again.\" He says it again: \"This is %1.\"", m_sCaller);

		return string.Format("You answer on the open band. A gasp. \"You hear me? You actually hear me? We're in the house with the blue door. Hurry. Please.\" That's %1, and he's real.", m_sCaller);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		HintRadioOps(gm, ids, players);
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			float dist = RZ_Util.DistXZ(p.GetOrigin(), m_vSpot);
			if (dist <= ANSWER_RADIUS && !m_aAnswered.Contains(playerId))
			{
				m_aAnswered.Insert(playerId);
				gm.NotifyPlayer(playerId, m_sTitle, RadioAnswer());
			}

			if (m_sTruth != REAL && dist <= HUNTER_RADIUS && !m_aHunted.Contains(playerId))
			{
				m_aHunted.Insert(playerId);
				RZ_StoryKit.Read(gm, this, playerId, "hunter", "Boots in the trees. Six, maybe eight. Nobody hunting walks like that.");
			}
		}

		if (m_sTruth == REAL)
		{
			UpdateReal(gm, players);
			return;
		}

		UpdateCrew(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! "My brother's in {town}": a lived-in house 1.2-4 km off that isn't the nearest; -1 if none.
	protected int SisterHouse(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		int nearest = houses.FindNearest(m_vSpot);
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			float d = 0;
			if (h)
				d = RZ_Util.DistXZ(h.m_vPos, m_vSpot);

			if (h && i != nearest && h.m_iPopulation > 0 && d >= 1200 && d <= 4000)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override vector ChoiceAt()
	{
		return m_vSpot;
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		m_iWhere = option;
		if (option == WHERE_SISTER && m_iSister < 0)
			m_iWhere = WHERE_NEAREST;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateReal(RZ_GameModeComponent gm, array<IEntity> players)
	{
		// ★ Nurse or Doctor: his leg isn't a fall
		if (m_bBitten)
		{
			foreach (int i, IEntity p : players)
			{
				PlayerManager pm = GetGame().GetPlayerManager();
				int pid = pm.GetPlayerIdFromControlledEntity(p);
				if (pid > 0 && !m_aSeenBite.Contains(pid) && RZ_Util.DistXZ(p.GetOrigin(), m_vSpot) <= 30)
				{
					m_aSeenBite.Insert(pid);
					RZ_StoryKit.Read(gm, this, pid, "nurse,doctor", "The man's leg isn't a fall. It's a bite, two days old, and he's wrapped it in a tea towel.");
				}
			}
		}

		if (m_bStirred || CountPlayersNear(players, m_vSpot, 300) == 0)
			return;

		// The infected round the house, and more coming at the noise
		m_bStirred = true;
		SpawnGuards(gm, m_vSpot, 4, 6, 20);
		gm.GetSpawner().SpawnHorde(m_vSpot, 250, m_iId);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCrew(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (RZ_EventGroup g : m_aGroups)
		{
			g.Tick(dt);
			if (g.m_bSpawned || CountPlayersNear(players, m_vSpot, SPAWN_RADIUS) == 0)
				continue;

			if (g.Spawn())
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, g.m_Unit, 300, true);
		}

		// A loop springs when someone reaches the radio; bait waits for the knife
		if (m_sTruth == LOOP && !m_bSprung && CountPlayersNear(players, m_vSpot, SPRING_RADIUS) > 0)
			Spring(gm, "There's nobody here. Just a radio on a crate, playing the same words. Then a branch snaps in the treeline. It was never a call for help.");

		// Players the Red Hand counts as friends are waved off once
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aSpared.Contains(playerId) || RZ_Util.DistXZ(p.GetOrigin(), m_vSpot) > SPRING_RADIUS)
				continue;

			if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
				continue;

			m_aSpared.Insert(playerId);
			gm.NotifyPlayer(playerId, m_sTitle, "A whistle from the trees, then a voice you half know: \"Not you. Keep walking, and keep your mouth shut about this.\"");
		}

		if (!m_bSprung || !RZ_EventGroup.AllDead(m_aGroups))
			return;

		// The crew is dead: a loop is over, and bait's people walk out if they're still alive
		if (m_sTruth == LOOP || m_bFreed)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void Spring(RZ_GameModeComponent gm, string line)
	{
		m_bSprung = true;
		ResourceName wp = gm.GetSpawner().GetMoveWaypointPrefab();
		foreach (RZ_EventGroup g : m_aGroups)
		{
			if (g.m_Unit)
				RZ_Util.SendGroupTo(g.m_Unit, m_vSpot, wp);
		}

		RZ_ContractHelp.NotifyNear(gm, m_vSpot, 150, m_sTitle, line);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		m_Story.m_Deciding.Step(gm, playerId);
		if (m_sTruth == LOOP)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Click. The voice stops. The quiet after it is worse.");
			SetObjectLabels(0, "", "");
			return;
		}

		if (m_sTruth == BAIT)
		{
			OnBaitChoice(gm, playerId, option);
			return;
		}

		// The one choice: where they go (Missions/Blocks/RZ_OneChoice.c)
		if (objIdx == RZ_OneChoice.OBJECT_INDEX)
		{
			if (m_Choice)
				m_Choice.Handle(gm, this, playerId, option);

			return;
		}

		if (m_bFreed)
			return;

		// [Nurse] dress his leg and say nothing: the house won't see it at the door
		if (option == 2)
		{
			if (!m_bBitten || !RZ_QuestApproach.HasBackground(gm, playerId, "nurse"))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "\"He's fine. He fell.\" There's nothing for you to dress.");
				return;
			}

			m_iHidBite = playerId;
			gm.NotifyPlayer(playerId, m_sTitle, "You dress the bite tight and clean and pull his trouser leg down over it. He looks at you. You don't say anything. Neither does he.");
			return;
		}

		// [Sneak] out the back while the infected are at the front: unseen, whatever's at the door
		if (option == 1)
		{
			if (!RZ_Stealth.IsHidden(playerId))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "\"Not with them looking this way.\" Get out of sight first, then come round the back.");
				return;
			}

			m_bFreed = true;
			AddParticipant(playerId);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("Through the kitchen window and out the back, one at a time, while the infected stand at the front door. %1 carries the man to the edge of town. \"There's a bag under the stairs. Take it.\"", m_sCaller));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (gm.GetSpawner().IsZombieNear(m_vSpot, CLEAR_RADIUS))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Not yet. I can hear them right outside.\" The door stays shut. Lead the infected away from the house, or put them down, then knock again.");
			return;
		}

		m_bFreed = true;
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("The barricade comes down a plank at a time. %1 comes out with a man on his arm, grey and limping. \"There's a bag under the stairs. Take it. We won't need it where we're going.\"", m_sCaller));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaitChoice(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (m_bFreed || m_bSold)
			return;

		// Cut them loose: quietly at night from the back, while the crew hasn't moved; otherwise they come
		if (option == 0)
		{
			m_bFreed = true;
			SetObjectLabels(0, "", "");
			if (RZ_GameTime.IsNight() && !m_bSprung)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "You saw through the rope from the back, in the dark, a strand at a time. Nobody in the trees moves. The three of you go out the back like smoke.");
				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}

			Spring(gm, "The rope parts, and somebody in the trees shouts. They were waiting for exactly this. Get them down and keep them down.");
			return;
		}

		// "Not these two. They're mine.": Known with the Red Hand, persuade 2
		if (option == 1)
		{
			if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "You'd need the Red Hand to know your face for that to mean anything.");
				return;
			}

			if (m_Ways.TryTrust(gm, playerId, "crew", RZ_EFaction.BANDITS, 2, "excon") != RZ_ECheck.PASS)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "A laugh from the trees. \"Yours? Everything's yours today, is it?\" Nobody moves.");
				return;
			}

			m_bFreed = true;
			SetObjectLabels(0, "", "");
			gm.NotifyPlayer(playerId, m_sTitle, "A long pause from the trees. \"Fine. Take them. You owe us one.\" Boots, going away.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		// Buy them
		if (option == 2)
		{
			if (!m_Ways.TryBribe(gm, playerId, RZ_EFaction.BANDITS, BUY_FAVOR))
				return;

			m_bFreed = true;
			SetObjectLabels(0, "", "");
			gm.NotifyPlayer(playerId, m_sTitle, "Someone comes out of the trees, counts nothing, and cuts the rope himself. \"Pleasure.\"");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		// Side with the crew: the speaker only, after the hold
		if (!RZ_StoryKit.MayDecide(gm, this, playerId))
			return;

		m_Story.m_Deciding.StartHold(gm, this, playerId, 3, "walk away and leave these two to the crew");
	}

	//------------------------------------------------------------------------------------------------
	override void OnHoldDone(RZ_GameModeComponent gm, int decider, int option)
	{
		super.OnHoldDone(gm, decider, option);
		if (option != 3 || m_bFreed || m_bSold)
			return;

		m_bSold = true;
		SetObjectLabels(0, "", "");
		gm.AddFavor(decider, RZ_EFaction.BANDITS, CUT_FAVOR);
		gm.NotifyPlayer(decider, m_sTitle, string.Format("A man steps out of the trees and presses tins into your hand. \"Good lad.\" %1 doesn't look at you. +%2 Red Hand Favor.", m_sCaller, CUT_FAVOR));

		// Seen by the Haven, or a Haven-Known player near: named now. Otherwise a lie the next Tied Down finds
		if (RZ_Witness.Seen(gm, decider, m_vSpot, RZ_EFaction.SURVIVORS) || HavenKnownNear(gm, decider))
		{
			gm.AddInfamy(decider, RZ_EFaction.SURVIVORS, CUT_INFAMY, "sold a distress call to the crew");
			gm.NotifyPlayer(decider, m_sTitle, "Somebody saw. The Haven will have your name by tonight.");
		}
		else
		{
			RZ_WorldFacts.Get().Memory().Lies().Tell(gm, decider, "distress.cut", m_sCaller, RZ_EFaction.SURVIVORS, "", m_vSpot, RZ_ELieTest.TICK, CUT_INFAMY,
				string.Format("Tied Down freed a truckload, and %1 was in it. He told them who walked away.", m_sCaller));
		}

		array<int> who = {decider};
		RZ_StoryKit.Record(this, "taken", m_sCaller, "crew", who, "", "");
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HavenKnownNear(RZ_GameModeComponent gm, int decider)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != decider && RZ_Util.DistXZ(p.GetOrigin(), m_vSpot) <= 150 && RZ_Reputation.IsAtLeast(gm.GetTier(ids[i], RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		// A loop just goes quiet; real or bait people are lost
		if (m_sTruth == LOOP)
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		super.OnTimeUp(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_sTruth == LOOP)
		{
			if (RZ_EventGroup.AllDead(m_aGroups))
				SpawnRewardAt(gm, m_vSpot);

			return;
		}

		SpawnRewardAt(gm, m_vSpot);
		RZ_Safehouses houses = gm.GetSafehouses();
		if (m_sTruth == REAL && !Delivered(gm, houses))
		{
			RZ_StoryKit.Record(this, "saved", m_sCaller, "bastion", m_aParticipants, string.Format("In %1's school, a radio with the batteries taken out, and a note: \"In case we need to call again. We won't.\"", GetLocationName(gm)), "board");
			return;
		}

		if (houses && houses.Count() > 0 && m_iWhere != WHERE_SISTER)
		{
			houses.AddRescued(m_vSpot, Rescued(gm), m_aParticipants);
			// The callback at his house: some hours of each rescuer's play later, near it
			RZ_Safehouse home = houses.Get(houses.FindNearest(m_vSpot));
			RZ_Shared shared = gm.GetShared();
			if (home && shared)
			{
				foreach (int rescuer : m_aParticipants)
				{
					shared.GetCallbacks().Queue(rescuer, "At " + home.m_sName, string.Format("The man with the limp is on the bench by the door. \"You took the barricade down. %1 still talks about it. He tells it better every time.\"", m_sCaller), home.m_vPos);
				}
			}
		}

		RZ_StoryKit.Record(this, "saved", m_sCaller, m_sTruth, m_aParticipants, string.Format("In %1's school, a radio with the batteries taken out, and a note: \"In case we need to call again. We won't.\"", GetLocationName(gm)), "board");
	}

	//------------------------------------------------------------------------------------------------
	//! Where they went, when it's not the nearest house. False: they went to Bastion's post (no house).
	protected bool Delivered(RZ_GameModeComponent gm, RZ_Safehouses houses)
	{
		if (m_iWhere == WHERE_BASTION)
		{
			// The soldiers: Bastion's thanks to those with the decider, and the sheltered count
			if (m_Choice)
				m_Choice.PayWith(gm, this, RZ_EFaction.MILITARY, 5);

			RZ_StoryKit.Sheltered(gm, this, 2);
			return false;
		}

		if (m_iWhere == WHERE_SISTER && houses && m_iSister >= 0)
		{
			RZ_Safehouse sister = houses.Get(m_iSister);
			houses.AddPopulationAt(m_iSister, Rescued(gm));
			RZ_Shared shared = gm.GetShared();
			if (sister && shared)
			{
				foreach (int rescuer : m_aParticipants)
				{
					shared.GetCallbacks().Queue(rescuer, "At " + sister.m_sName, string.Format("Two men on the bench by the door, one with a limp, both talking at once. \"%1 said you'd come by. He said you'd walk us all the way. You did.\"", m_sCaller), sister.m_vPos);
				}
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Two walk in, unless the house finds his bite at the door (then one). Hidden by a Nurse who knew:
	//! both, and it's found by the house within a season day (+8 Haven Infamy, to him only).
	protected int Rescued(RZ_GameModeComponent gm)
	{
		if (!m_bBitten)
			return 2;

		if (m_iHidBite > 0)
		{
			int doorHouse = m_iSister;
			if (m_iWhere != WHERE_SISTER && gm.GetSafehouses() && gm.GetSafehouses().Count() > 0)
				doorHouse = gm.GetSafehouses().FindNearest(m_vSpot);

			RZ_StoryKit.HidBite(gm, m_iHidBite, doorHouse);
			RZ_StoryKit.Dirty(gm, this, m_iHidBite);
			gm.GetFacts().Memory().Lies().Tell(gm, m_iHidBite, "distress.bite", m_sCaller, RZ_EFaction.SURVIVORS, "", m_vSpot, RZ_ELieTest.TICK, HID_INFAMY,
				"The man with the limp turned in the night. The house found the dressing, and knows whose knots those are.");
			return 2;
		}

		NotifyParticipants(gm, m_sTitle, "At the door the house checks everyone, as it always does. His leg. He goes out at dusk with a lamp, and the other walks with him as far as the gate.");
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		if (m_sTruth == LOOP || m_bSold)
			return;

		// Inside a pack's range, the dead run with it now, by name (Bring Them Home can find them)
		RZ_Tribes tribes = gm.GetTribes();
		RZ_Tribe t = null;
		if (tribes && tribes.GetPeople().IsOn())
			t = tribes.NearestTribe(m_vSpot, 3000);

		if (t && RZ_Util.DistXZ(t.RangeCentre(), m_vSpot) <= t.m_fRadius)
			tribes.GetPeople().OnNamedTaken(t, m_sCaller, m_vSpot, false);

		RZ_StoryKit.Record(this, "dead", m_sCaller, m_sTruth, m_aParticipants, "", "");
		RZ_StoryKit.Fallout(gm, this, "name_on_tag", m_vSpot, string.Format("%1 and the man from the distress call", m_sCaller));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_EventGroup.DeleteAll(m_aGroups);
		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_sTruth == LOOP)
		{
			if (RZ_EventGroup.AllDead(m_aGroups))
				return "The distress call near " + GetLocationName(gm) + " was a Red Hand trap. The people who set it won't set another.";

			return "The distress call near " + GetLocationName(gm) + " has gone quiet. It was a Red Hand trap: a tape on a crate.";
		}

		return "Someone answered the distress call near " + GetLocationName(gm) + ". Two people walked out.";
	}
}

//------------------------------------------------------------------------------------------------
//! #11 Prison Break: convicts broke out of the holding cells in a town and are walking to the
//! Red Hand's nearest camp. Two groups (Missions > Events > Convicts, else Missions > Red Hand
//! group), data until a player is within 600 m. They fight anyone near them except players Known
//! or better with the Red Hand. Stop them all: what they took is where the last one fell. If any
//! reach the camp (or the time runs out), the Red Hand takes them in: a little stronger, and its
//! friends who walked in with them get a little Fame.
//! A story job (key `prison`, kept per town): convicts, as they said; half held for nothing (Bastion's
//! curfew sweep: a Bastion holding on the map); or a crew opened the cells from outside (a Red Hand
//! camp they walk to). The jailer with no keys (the talk scene): [Known - Bastion] or two tins and he
//! rings the town's bell, and the infected come for the convicts (no gun, no standing); swept: tell
//! the curfew lot to go home (one group fewer, the nearest house +2, a Haven asset); opened: tell
//! Bastion's post who opened it (5 Bastion Favor, +4 Red Hand Infamy, a Bastion asset).
class RZ_Event_PrisonBreak : RZ_Mission
{
	protected static const int WAY_BELL = 0;
	protected static const int WAY_HOME = 1;
	protected static const int WAY_TELL = 2;

	//------------------------------------------------------------------------------------------------
	void RZ_Event_PrisonBreak()
	{
		m_Story = new RZ_MissionStory("prison");
		m_Job = new RZ_StoryJob();
	}

	protected ref array<ref RZ_EventGroup> m_aGroups = {};
	protected vector m_vTarget;
	protected string m_sTarget;
	protected bool m_bToRedHand;
	protected bool m_bRecruited;
	protected vector m_vLast;

	protected static const float BODY_RADIUS = 600;
	protected static const float ARRIVE_RADIUS = 150;
	protected static const float DATA_SPEED = 1.2;

	//------------------------------------------------------------------------------------------------
	static ResourceName Prefab(RZ_GameModeComponent gm)
	{
		ResourceName prefab = RZ_EventHelp.Settings(gm).m_sConvictGroupPrefab;
		if (prefab.IsEmpty())
			prefab = gm.GetRedHandGroupPrefab();

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty())
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<string> kinds = {"police"};
		vector start = s.m_vCenter;
		RZ_Landmark cells = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 100, kinds);
		if (cells)
			start = cells.m_vPos;

		start = LandPointNear(start, 8, 30);

		// The Red Hand's nearest camp, else into the hills toward another town
		RZ_Territory territory = gm.GetTerritory();
		RZ_Holding camp;
		if (territory && territory.IsActive())
			camp = territory.FindHolding(RZ_EFaction.BANDITS, start, 8000);

		if (camp)
		{
			m_vTarget = camp.m_vPos;
			m_sTarget = camp.GetTitle();
			m_bToRedHand = true;
		}
		else
		{
			RZ_Settlement t = gm.GetSettlement(PickSettlementAway(gm, start, 1500, 4000, 0, 100));
			if (!t)
				return false;

			m_vTarget = t.m_vCenter;
			m_sTarget = t.m_sName;
		}

		for (int i = 0; i < 2; i++)
		{
			m_aGroups.Insert(new RZ_EventGroup(Prefab(gm), LandPointNear(start, 0, 15)));
		}

		m_vPos = start;
		m_vLast = start;
		m_sTitle = "Prison Break";
		m_sBrief = string.Format("The holding cells in %1 didn't hold. A dozen convicts are out and armed with whatever they found, walking toward %2. The Red Hand will take in anyone who gets there. Stop them on the road, or stay out of their way.", s.m_sName, m_sTarget);
		m_sObjective = string.Format("Stop the convicts on the road from %1 to %2.", s.m_sName, m_sTarget);
		Truth(gm, start);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm, vector start)
	{
		array<string> values = {"convicts", "swept", "opened"};
		array<float> weights = {0.5, 0, 0};
		if (RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.MILITARY, start, 100000))
			weights[1] = 0.3;

		if (m_bToRedHand)
			weights[2] = 0.2;

		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		m_Job.m_vReadAt = start;
		m_Job.Read("swept", "police", "The charge sheets say 'out after curfew', twelve times, in one hand. That's a sweep, not a crime wave.");
		m_Job.Read("opened", "mechanic", "The lock wasn't picked. It was cut from the outside, with good cutters.");
		m_Job.Read("", "hunter", "Twelve sets of prints, fresh, all one way. They're not running. They know where they're going.");

		string reveal = "Thieves, mostly. One poet. I'd have let the poet go if he'd stopped reciting.";
		if (truth == "swept")
			reveal = "Half of them were only out after curfew. Bastion brought them in by the lorry. I said I hadn't the room.";
		else if (truth == "opened")
			reveal = string.Format("Men with red on their hands came at two and cut the lock. Said they were from %1, and to thank you for the hospitality.", m_sTarget);

		RZ_TalkScene sc = m_Job.Scene("The jailer", "prison.jailer", LandPointNear(start, 4, 8));
		int n0 = sc.Node("A jailer stands by the open cells. \"They took the keys. I had them on a string. They took the string.\"");
		int n1 = sc.Node("The bell's still mine. They didn't think to take the bell.");
		sc.Ask(n0, "Who was in there?", n1, reveal, true);
		sc.Way(n1, "Ring the bell.", RZ_TalkScene.TRUST, RZ_EFaction.MILITARY, 1, WAY_BELL, "He hauls on the rope. The whole town hears it, and so does everything in it.", "For you? They'd hang me for it.");
		sc.Way(n1, "Two tins and he rings it", RZ_TalkScene.FOOD, -1, 2, WAY_BELL, "He pockets the tins and rings like he means it.");
		if (truth == "swept")
			sc.Decide(n0, "Tell the curfew lot to go home", "send the curfew lot home", WAY_HOME, "Half of them stop on the road, look at each other, and walk off toward the houses.", RZ_EFaction.SURVIVORS);
		else if (truth == "opened")
			sc.Decide(n0, "Tell the post who opened it", "tell Bastion who opened the cells", WAY_TELL, "You'll tell the post. The jailer writes it down for you, slowly, in capitals.", RZ_EFaction.MILITARY);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		array<int> who = {playerId};
		string town = RZ_WorldFacts.Town(m_iSettlement);
		if (way == WAY_BELL)
		{
			// The bell brings the infected down on whoever's on the road
			gm.GetHeat().AddHeat(m_vLast, 60);
			gm.GetSpawner().SpawnHorde(m_vLast, 120, m_iId);
			RZ_Log.Info(string.Format("Story: prison bell rung at %1", RZ_SettlementScanner.GridReference(m_vLast)));
			if (m_Job.m_Scene)
				m_Job.m_Scene.m_bDone = false;

			return;
		}

		if (way == WAY_HOME)
		{
			if (m_aGroups.Count() > 1)
			{
				m_aGroups[0].Delete();
				m_aGroups.Remove(0);
			}

			if (gm.GetSafehouses())
				gm.GetSafehouses().AddPopulation(m_vLast, 2);

			RZ_StoryKit.Record(this, "fate", town, "sent_home", who);
			RZ_StoryKit.Later(gm, playerId, "Bastion's nightly call", "The operator reads the curfew numbers and stops halfway. \"Correction. Six fewer.\"", m_vLast);
			return;
		}

		RZ_StoryKit.PayDecider(gm, this, playerId, RZ_EFaction.MILITARY, 5);
		RZ_StoryKit.BlameDecider(gm, this, playerId, RZ_EFaction.BANDITS, 4, "named a crew to Bastion");
		RZ_StoryKit.Record(this, "fate", town, "named", who);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		bool anySpawned = false;
		foreach (RZ_EventGroup g : m_aGroups)
		{
			g.Tick(dt);
			if (g.m_bSpawned)
			{
				anySpawned = true;
				if (!g.IsDead())
					m_vLast = g.GetPos();

				continue;
			}

			// As data they walk on toward the camp
			vector dir = m_vTarget - g.m_vPos;
			dir[1] = 0;
			float len = dir.Length();
			if (len > 1)
				g.m_vPos = RZ_Util.GroundPos(g.m_vPos + dir * (Math.Min(len, DATA_SPEED * dt) / len));

			m_vLast = g.m_vPos;
			if (RZ_Util.NearestDistance(g.m_vPos, players) > BODY_RADIUS)
				continue;

			if (g.Spawn())
			{
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, g.m_Unit, 250, true);
				RZ_Util.SendGroupTo(g.m_Unit, m_vTarget, gm.GetSpawner().GetMoveWaypointPrefab());
			}
		}

		m_vPos = m_vLast;
		if (RZ_EventGroup.AllDead(m_aGroups))
		{
			RZ_ContractHelp.AddNear(ids, players, m_vLast, 250, m_aParticipants);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (RZ_Util.DistXZ(m_vLast, m_vTarget) <= ARRIVE_RADIUS)
			Recruit(gm, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	protected void Recruit(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		m_bRecruited = true;
		m_eState = RZ_EMissionState.SUCCEEDED;
		if (!m_bToRedHand)
			return;

		int alive = RZ_EventGroup.CountAlive(m_aGroups);
		RZ_Territory territory = gm.GetTerritory();
		if (territory)
			territory.AddStrength(RZ_EFaction.BANDITS, 1.5 * alive);

		if (gm.GetLeaders())
			gm.GetLeaders().AddHeadline(string.Format("The Red Hand has new faces at %1: convicts who walked out of the cells in %2.", m_sTarget, GetSettlementName(gm)), false);

		// The Red Hand's friends who walked in with them
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vLast) > 250 || !RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
				continue;

			gm.AddFame(playerId, RZ_EFaction.BANDITS, 4);
			gm.NotifyPlayer(playerId, "The Red Hand", "You walked them in. The Red Hand notices who brings it new hands: +4 Fame.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string GetSettlementName(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			return s.m_sName;

		return "town";
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return RZ_SettlementScanner.GridReference(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		// They made it off the roads: the Red Hand finds them sooner or later
		array<int> noIds = {};
		array<IEntity> noPlayers = {};
		Recruit(gm, noIds, noPlayers);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (!m_bRecruited)
			SpawnRewardCrateAt(gm, m_vLast);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnRewardCrateAt(RZ_GameModeComponent gm, vector pos)
	{
		gm.SpawnRewardCrate(LandPointNear(pos, 1, 4));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_EventGroup.DeleteAll(m_aGroups);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (!m_bRecruited)
			return "The convicts from " + GetSettlementName(gm) + " never made it. What they took is lying where the last one fell.";

		if (m_bToRedHand)
			return "The convicts from " + GetSettlementName(gm) + " reached the Red Hand. It has more guns tonight.";

		return "The convicts from " + GetSettlementName(gm) + " have scattered into the hills.";
	}
}

//------------------------------------------------------------------------------------------------
//! #14 The Sniper: someone has gone mad on the high ground above a town and shoots at anything
//! that moves. One armed AI (Missions > Events > Sniper, else Contracts > Bounty target), there
//! only while a player is within 900 m; he fights everyone near him, Red Hand friends included.
//! His shots draw infected. Take him down: his stash is up there. After 30 minutes he moves on.
class RZ_Event_TheSniper : RZ_Mission
{
	protected ref RZ_EventGroup m_Sniper;

	protected static const float BODY_RADIUS = 900;
	protected static const float FIGHT_RADIUS = 500;

	//------------------------------------------------------------------------------------------------
	static ResourceName Prefab(RZ_GameModeComponent gm)
	{
		ResourceName prefab = RZ_EventHelp.Settings(gm).m_sSniperPrefab;
		if (prefab.IsEmpty() && gm.GetContractSettings())
			prefab = gm.GetContractSettings().m_sBountyTargetPrefab;

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty())
			return 0;

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty())
			return false;

		m_iSettlement = RZ_EventHelp.PickEmptyTown(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = HighPointNear(s.m_vCenter, s.m_fRadius + 80, s.m_fRadius + 450);
		m_Sniper = new RZ_EventGroup(Prefab(gm), m_vPos);
		m_sTitle = "The Sniper";
		m_sBrief = string.Format("Someone's been shooting from the high ground above %1 since dawn. Not infected, not any faction's. Whoever it is has a stash up there and nothing left to lose. Come at him from cover.", s.m_sName);
		m_sObjective = string.Format("Take out the sniper on the high ground above %1.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Sniper.Tick(dt);
		if (!m_Sniper.m_bSpawned)
		{
			if (CountPlayersNear(players, m_vPos, BODY_RADIUS) == 0)
				return;

			if (!m_Sniper.Spawn())
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			// The sniper prefab is a Red Hand (BAND) group; the bounty target it falls back to is on
			// SURV, as Collect the Bounty needs. Mark the fight for the faction he's really on.
			RZ_EFaction side = RZ_EFaction.BANDITS;
			if (RZ_EventHelp.Settings(gm).m_sSniperPrefab.IsEmpty())
				side = RZ_EFaction.SURVIVORS;

			RZ_Hostility.MarkFight(side, m_Sniper.m_Unit, FIGHT_RADIUS, false);
			return;
		}

		if (m_Sniper.IsDead())
		{
			RZ_ContractHelp.AddNear(ids, players, m_vPos, 300, m_aParticipants);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		// His shots carry
		if (CountPlayersNear(players, m_vPos, FIGHT_RADIUS) > 0)
			gm.GetHeat().AddHeat(m_vPos, dt);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_Sniper.IsDead())
			gm.SpawnRewardCrate(LandPointNear(m_vPos, 1, 4));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_Sniper)
			m_Sniper.Delete();
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_Sniper.IsDead())
			return "The shooting above " + GetLocationName(gm) + " has stopped for good. His stash is still up there.";

		return "The shooting above " + GetLocationName(gm) + " has stopped. He's moved on.";
	}
}

//------------------------------------------------------------------------------------------------
//! #22 Rival Survivors: an unaligned group camps outside a town. How they treat you depends on
//! your reputation overall: Hunted by anyone, or Watched by two factions, and they fight (only
//! you); Trusted by someone and Watched by nobody, and they trade for 2 food; anyone else pays
//! 3. Two trades in all, one per player, each a crate of their spare kit. Gun them down when
//! they hadn't turned on anyone and the Haven hears of it (+6 Infamy); if they had, their kit is
//! yours. Group: Missions > Events > Rival group, else Missions > Survivor group.
class RZ_Event_RivalSurvivors : RZ_PuzzleMission
{
	protected ref RZ_EventGroup m_Group;
	protected vector m_vCamp;
	protected bool m_bProvoked;
	protected int m_iTrades;
	protected ref array<int> m_aTraded = {};
	protected ref array<int> m_aMarked = {};

	protected static const float BODY_RADIUS = 500;
	protected static const float NOTICE_RADIUS = 120;
	protected static const int MAX_TRADES = 2;
	protected static const int MURDER_INFAMY = 6;

	//------------------------------------------------------------------------------------------------
	static ResourceName Prefab(RZ_GameModeComponent gm)
	{
		ResourceName prefab = RZ_EventHelp.Settings(gm).m_sRivalGroupPrefab;
		if (prefab.IsEmpty())
			prefab = gm.GetSurvivorGroupPrefab();

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty() || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty())
			return false;

		m_iSettlement = RZ_EventHelp.PickEmptyTown(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vCamp = LandPointNear(s.m_vCenter, s.m_fRadius + 50, s.m_fRadius + 300);
		m_vPos = m_vCamp;
		string leader = RZ_NoteText.FirstName(m_Rng);
		int look = AddClue(leader, string.Format("%1 does the talking for them: five or six people, nobody's colours, rifles held low. They watch your hands the whole time. \"We've heard of most people. Depends what we've heard.\"", leader));
		if (!SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vCamp, 3, 8), 0, "", "[Trade] Food for some of their kit|Ask what they've seen", look, "Talk to " + leader))
			return false;

		m_Group = new RZ_EventGroup(Prefab(gm), m_vCamp);
		m_sTitle = "Rival Survivors";
		m_sBrief = string.Format("A group nobody's heard from has made camp outside %1. They answer to no faction. They'll trade with people they've heard good things about. The others, they shoot.", s.m_sName);
		m_sObjective = string.Format("Trade with the camp outside %1, or deal with the camp.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! -1 they fight you, 0 wary (trade dearer), 1 they like what they've heard.
	protected int Attitude(RZ_GameModeComponent gm, int playerId)
	{
		int watched = 0;
		bool trusted = false;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			RZ_ETier tier = gm.GetTier(playerId, f);
			if (RZ_Reputation.IsHunted(tier))
				return -1;

			if (RZ_Reputation.IsWatchedOrWorse(tier))
				watched++;
			else if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED))
				trusted = true;
		}

		if (watched >= 2)
			return -1;

		if (trusted && watched == 0)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Group.Tick(dt);
		if (!m_Group.m_bSpawned)
		{
			if (CountPlayersNear(players, m_vCamp, BODY_RADIUS) > 0)
				m_Group.Spawn();

			return;
		}

		if (m_Group.IsDead())
		{
			OnGroupDead(gm, ids, players);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		// People they've heard bad things about: they fight them, and only them
		RZ_Hostility hostility = gm.GetHostility();
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aMarked.Contains(playerId) || RZ_Util.DistXZ(p.GetOrigin(), m_vCamp) > NOTICE_RADIUS)
				continue;

			if (Attitude(gm, playerId) >= 0)
				continue;

			m_aMarked.Insert(playerId);
			m_bProvoked = true;
			if (hostility && m_Group.m_Unit)
				hostility.MarkHostileAround(RZ_EFaction.SURVIVORS, m_Group.m_Unit, 200, playerId);

			gm.NotifyPlayer(playerId, m_sTitle, "Someone in the camp says your name, and every rifle comes up at once. They've heard of you.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupDead(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		array<int> near = {};
		GetPlayersNear(ids, players, m_vCamp, 250, near);
		foreach (int playerId : near)
		{
			if (!m_aParticipants.Contains(playerId))
				m_aParticipants.Insert(playerId);

			if (m_bProvoked)
				continue;

			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, MURDER_INFAMY);
			gm.NotifyPlayer(playerId, "The Haven", string.Format("They came to trade, and somebody gunned them down. Word gets around: +%1 Infamy with the Haven.", MURDER_INFAMY));
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		int attitude = Attitude(gm, playerId);
		if (attitude < 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "We know who you are. Walk away while we're letting you.");
			return;
		}

		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, SeenText(gm));
			return;
		}

		if (m_iTrades >= MAX_TRADES || m_aTraded.Contains(playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "That's all we can spare. We've a long way to go.");
			return;
		}

		int cost = 3;
		if (attitude > 0)
			cost = 2;

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		array<string> keywords = {};
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
			keywords = houses.GetFoodKeywords();
		else
			RZ_FoodHelper.DefaultKeywords(keywords);

		if (!user || RZ_ItemHelper.Count(user, keywords) < cost)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 tins, or whatever you've got that's food. Then we talk.", cost));
			return;
		}

		RZ_FoodHelper.TakeFood(user, keywords, cost);
		m_iTrades++;
		m_aTraded.Insert(playerId);
		AddParticipant(playerId);
		gm.SpawnRewardCrate(LandPointNear(m_vCamp, 2, 5));
		gm.NotifyPlayer(playerId, m_sTitle, "They count the food twice and push a crate across with their boots. \"Fair. Don't follow us.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The most infected town within 3 km: where not to go.
	protected string SeenText(RZ_GameModeComponent gm)
	{
		RZ_Settlement worst;
		foreach (RZ_Settlement t : gm.GetSettlements())
		{
			if (RZ_Util.DistXZ(t.m_vCenter, m_vCamp) > 3000)
				continue;

			if (!worst || t.m_fInfection > worst.m_fInfection)
				worst = t;
		}

		if (!worst)
			return "Nothing worth telling. It's quiet. Too quiet, if you ask me.";

		return string.Format("Stay out of %1. We went in for water and left two behind. It's crawling.", worst.m_sName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_Group.IsDead() && m_bProvoked)
			gm.SpawnRewardCrate(LandPointNear(m_vCamp, 1, 4));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_Group)
			m_Group.Delete();

		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_Group.IsDead())
			return "The camp outside " + GetLocationName(gm) + " is empty now. Nobody from it walked away.";

		return "The group outside " + GetLocationName(gm) + " has packed up and moved on.";
	}
}

//------------------------------------------------------------------------------------------------
//! #28 Collapse Shelter: a town's shelter (its school or church) is overrun and four survivors
//! have scattered round the town, each hiding with a couple of infected close by. Find them
//! and tell them the way to the safehouse (nothing within 15 m of them) before they turn: the
//! first in 15 minutes, the others a little later. Each one saved joins the nearest safehouse.
//! The Haven pays its Fame and Favor to the rescuers, scaled by how many were saved.
//! A story job (key `shelter`, kept per town): overrun, as they said; it started inside (the warden
//! hid a bite on his list); or the doors were chained from outside (a Bastion chain: a Bastion holding
//! on the map). The warden on the steps (the talk scene): [Known - Haven] "Where would they hide?"
//! (five more minutes for each still hiding, the grids exact) or two tins for his whistle (the nearest
//! still hiding comes to him: saved, no gun, no standing); inside: tell the house it was him (Haven
//! morale -1); chained: take the chain to the house (6 Haven Favor, +3 Bastion Infamy, a Haven asset).
class RZ_Event_CollapseShelter : RZ_PuzzleMission
{
	protected static const int WAY_LIST = 0;
	protected static const int WAY_WHISTLE = 1;
	protected static const int WAY_NAME = 2;
	protected static const int WAY_CHAIN = 3;

	//------------------------------------------------------------------------------------------------
	void RZ_Event_CollapseShelter()
	{
		m_Story = new RZ_MissionStory("shelter");
		m_Job = new RZ_StoryJob();
	}

	protected vector m_vShelter;
	protected ref array<vector> m_aPeople = {};
	protected ref array<IEntity> m_aBodies = {};
	protected ref array<float> m_aLeft = {};
	protected ref array<int> m_aState = {};		// 0 hiding, 1 saved, 2 turned
	protected ref array<bool> m_aGuarded = {};
	protected bool m_bStirred;
	protected int m_iSaved;
	protected int m_iLost;

	protected static const int PEOPLE = 4;
	protected static const float CLEAR_RADIUS = 15;
	protected static const float GUARD_RADIUS = 300;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(true, RZ_EFaction.SURVIVORS, 12, 8);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() == 0 || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = RZ_EventHelp.PickEmptyTown(gm, 30, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<string> kinds = {"school", "church"};
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		string shelter = "the town hall";
		m_vShelter = s.m_vCenter;
		if (l)
		{
			m_vShelter = l.m_vPos;
			shelter = l.m_sName;
		}

		for (int i = 0; i < PEOPLE; i++)
		{
			vector pos = LandPointNear(m_vShelter, 80, 350);
			string name = RZ_NoteText.FirstName(m_Rng);
			int talk = AddClue(name, string.Format("%1 is pressed into a doorway, shaking. \"They came through the shelter in the night. I ran. I don't know where the others went. Where do I go?\"", name));
			IEntity body = SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, pos, i, "", "Tell them the way to the safehouse", talk, "Talk to " + name);
			if (!body)
				return false;

			m_aPeople.Insert(pos);
			m_aBodies.Insert(body);
			m_aLeft.Insert(900 + i * 150);
			m_aState.Insert(0);
			m_aGuarded.Insert(false);
		}

		m_vPos = m_vShelter;
		m_sTitle = "Collapse Shelter";
		m_sBrief = string.Format("The shelter in %1 (%2) was overrun in the night. Four people got out and scattered round the town. They're hiding, and some of them are bitten. Find them before they turn and point them to a safehouse.", s.m_sName, shelter);
		m_sObjective = string.Format("Find the four who fled the shelter in %1 and send each to a safehouse.", s.m_sName);
		Truth(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm)
	{
		array<string> values = {"overrun", "inside", "chained"};
		array<float> weights = {0.5, 0.3, 0};
		if (RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.MILITARY, m_vShelter, 100000))
			weights[2] = 0.2;

		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		m_Job.m_vReadAt = m_vShelter;
		m_Job.m_fReadRange = 40;
		m_Job.Read("inside", "nurse,doctor", "The first bodies are by the cots, not the doors. It started inside, with someone who was already bitten.");
		m_Job.Read("chained", "soldier", "A Bastion padlock on the outside of the shelter's doors, opened since. Somebody sealed it with them in.");
		m_Job.Read("", "hunter", "Four sets of prints run out of the back, each a different way. They scattered like they were told to.");

		string reveal = "They came through the windows. I counted them in. I'd counted everyone in.";
		if (truth == "inside")
			reveal = "He looks at his list. \"Number nine had a bandage. He said it was a dog. I wrote 'dog'.\"";
		else if (truth == "chained")
			reveal = "Bastion chained the doors at dusk. For our safety, the corporal said. We broke a window to get out.";

		RZ_TalkScene sc = m_Job.Scene("The warden", "shelter.warden", LandPointNear(m_vShelter, 5, 10));
		int n0 = sc.Node("A man in a warden's armband sits on the steps. \"I was in charge of the shelter. I'd like that to stop being true.\"");
		int n1 = sc.Node("Forty-one names on the list. I've been crossing them off.");
		sc.Ask(n0, "What happened?", n1, reveal, true);
		sc.Way(n0, "Where would they hide?", RZ_TalkScene.TRUST, RZ_EFaction.SURVIVORS, 1, WAY_LIST, "He runs a finger down the list and names doorways. You've a few more minutes than you thought.", "How would I know? I'm only the warden.");
		sc.Way(n0, "Two tins for his whistle", RZ_TalkScene.FOOD, -1, 2, WAY_WHISTLE, "He blows three short notes. Somewhere near, someone who knows that whistle starts running toward it.");
		if (truth == "inside")
			sc.Decide(n1, "Tell the house it was number nine", "tell the house how it started", WAY_NAME, "You'll tell them. He nods and goes on crossing out names.");
		else if (truth == "chained")
			sc.Decide(n1, "Take the chain to the house", "take Bastion's chain to the house", WAY_CHAIN, "The chain goes in your pack, padlock and all. The house will want to see it.", RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		array<int> who = {playerId};
		string town = RZ_WorldFacts.Town(m_iSettlement);
		if (way == WAY_LIST)
		{
			foreach (int i, int state : m_aState)
			{
				if (state == 0)
					m_aLeft[i] = m_aLeft[i] + 300;
			}

			gm.NotifyPlayer(playerId, m_sTitle, GetWhereText(gm, m_vShelter));
			ReopenScene();
			return;
		}

		if (way == WAY_WHISTLE)
		{
			int best = -1;
			float bestDist = 100000;
			foreach (int j, vector p : m_aPeople)
			{
				float d = RZ_Util.DistXZ(p, m_vShelter);
				if (m_aState[j] == 0 && d < bestDist)
				{
					best = j;
					bestDist = d;
				}
			}

			if (best >= 0)
				Save(gm, playerId, best);

			ReopenScene();
			return;
		}

		if (way == WAY_NAME)
		{
			RZ_OneChoice.Morale(gm, RZ_EFaction.SURVIVORS, -1, "a shelter lost to a hidden bite");
			RZ_StoryKit.Record(this, "fate", town, "named", who);
			return;
		}

		RZ_StoryKit.PayDecider(gm, this, playerId, RZ_EFaction.SURVIVORS, 6);
		RZ_StoryKit.BlameDecider(gm, this, playerId, RZ_EFaction.MILITARY, 3, "showed a house Bastion's chain");
		RZ_StoryKit.Record(this, "fate", town, "chain", who);
		RZ_StoryKit.Later(gm, playerId, "At the house", "Bastion's padlock hangs on a nail by the house's door, under a sign: 'For our safety.'", m_vShelter);
	}

	//------------------------------------------------------------------------------------------------
	//! The warden still talks after a way (the choice in his second node stays open).
	protected void ReopenScene()
	{
		if (m_Job.m_Scene)
			m_Job.m_Scene.m_bDone = false;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string where = "";
		foreach (int i, vector p : m_aPeople)
		{
			if (m_aState[i] != 0)
				continue;

			if (!where.IsEmpty())
				where += ", ";

			where += RZ_SettlementScanner.GridReference(p);
		}

		return super.GetWhereText(gm, from) + "\nLast seen (grid): " + where;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bStirred && CountPlayersNear(players, m_vShelter, 400) > 0)
		{
			m_bStirred = true;
			SpawnGuards(gm, m_vShelter, 5, 5, 25);
		}

		foreach (int i, vector p : m_aPeople)
		{
			if (m_aState[i] != 0)
				continue;

			if (!m_aGuarded[i] && CountPlayersNear(players, p, GUARD_RADIUS) > 0)
			{
				m_aGuarded[i] = true;
				SpawnGuards(gm, p, 2, 12, 25);
			}

			m_aLeft[i] = m_aLeft[i] - dt;
			if (m_aLeft[i] <= 0)
				Turn(gm, i);
		}

		if (m_iSaved + m_iLost >= PEOPLE)
			Finish();
	}

	//------------------------------------------------------------------------------------------------
	protected void Turn(RZ_GameModeComponent gm, int i)
	{
		m_aState[i] = 2;
		m_iLost++;
		RZ_Util.DeleteUnit(m_aBodies[i]);
		gm.GetSpawner().SpawnZombie(m_aPeople[i], false, m_iSettlement, m_iId);
		RZ_ContractHelp.NotifyNear(gm, m_aPeople[i], 600, m_sTitle, "Somewhere close a scream turns into something else. One of the people from the shelter didn't make it.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish()
	{
		if (m_iSaved > 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
		else
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx < 0 || objIdx >= m_aPeople.Count() || m_aState[objIdx] != 0)
			return;

		vector p = m_aPeople[objIdx];
		if (gm.GetSpawner().IsZombieNear(p, CLEAR_RADIUS))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "They won't move with those things that close. Clear them off first.");
			return;
		}

		Save(gm, playerId, objIdx);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone from the shelter sets off for the safehouse.
	protected void Save(RZ_GameModeComponent gm, int playerId, int idx)
	{
		vector p = m_aPeople[idx];
		m_aState[idx] = 1;
		m_iSaved++;
		AddParticipant(playerId);
		RZ_Util.DeleteUnit(m_aBodies[idx]);
		array<int> rescuer = {playerId};
		gm.GetSafehouses().AddRescued(p, 1, rescuer);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("They nod, and go, keeping low. %1 of %2 found.", m_iSaved + m_iLost, PEOPLE));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		foreach (int i, int state : m_aState)
		{
			if (state == 0)
				Turn(gm, i);
		}

		Finish();
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		m_fRewardScale = m_iSaved;
		m_fRewardScale = m_fRewardScale / PEOPLE;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("%1 of the %2 people from the shelter in %3 reached safety.", m_iSaved, PEOPLE, GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Nobody from the shelter in " + GetLocationName(gm) + " made it.";
	}
}
