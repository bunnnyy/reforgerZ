//! Grey choices in missions (docs/DESIGN.md, "Story blocks"; how they read: docs/WRITING.md, "Grey
//! choices"): the shared layer on top of each job's own options and pay.
//!
//! - **RZ_Outcome**: an outcome told in three voiced beats with the system line kept separate, and
//!   "someone else made the call" for the others who took part.
//! - **RZ_GreyChoice**: what every one-choice job adds when it's settled: the giver's people's
//!   reaction (the voice bank), "nobody chose" when the default was taken, the callback deed for those
//!   with the decider (RZ_Deeds), and the clever way closing the honest one.
//! - **RZ_BriefLies**: twelve briefs whose giver may leave one thing out, the version picked once a
//!   season; the truth found at the scene by anyone (a background faster), named in the outcome, and
//!   made public through talk at a capped price.
//! - **RZ_RecordBoards**: boards of record in the season, worked out from the season flags.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_Outcome
{
	//------------------------------------------------------------------------------------------------
	//! Three beats (what happened; what it cost and to whom; one reaction from `reactionFaction`'s
	//! people, the bank's `reactionKind` lines, "" none) to `players`, then the system line (standing
	//! and pay as numbers) on its own line. "" parts are left out.
	static void Tell(RZ_GameModeComponent gm, notnull array<int> players, string title, string what, string cost, int reactionFaction, string reactionKind, string systemLine, vector at)
	{
		foreach (int playerId : players)
		{
			string text = what;
			if (!cost.IsEmpty())
				text = (text + " " + cost).Trim();

			string reaction = "";
			if (!reactionKind.IsEmpty() && RZ_Factions.IsValid(reactionFaction))
				reaction = RZ_VoiceBank.PickFor(gm, reactionKind, reactionFaction, "", null, playerId, at);

			if (!reaction.IsEmpty())
				text = (text + " " + reaction).Trim();

			if (!systemLine.IsEmpty())
				text = (text + "\n" + systemLine).Trim();

			if (!text.IsEmpty())
				gm.NotifyPlayerAs(playerId, title, text, RZ_ENoticeTag.MISSION);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The others who took part but weren't with the decider: someone else made the call.
	static void ElseCall(RZ_GameModeComponent gm, RZ_Mission m, notnull array<int> with)
	{
		foreach (int playerId : m.m_aParticipants)
		{
			if (with.Contains(playerId))
				continue;

			string line = RZ_VoiceBank.PickFor(gm, "else_call", m.m_eGiver, "", null, playerId, m.m_vPos);
			if (line.IsEmpty())
				line = "Someone else made the call before you could. It went their way.";

			gm.NotifyPlayerAs(playerId, m.m_sTitle, line, RZ_ENoticeTag.MISSION);
		}
	}
}

//------------------------------------------------------------------------------------------------
class RZ_GreyChoice
{
	//------------------------------------------------------------------------------------------------
	//! The honest option of a job whose other options include a trick or a betrayal, -1 none.
	static int HonestOption(RZ_Mission m)
	{
		switch (m.ClassName())
		{
			case "RZ_Mission_FightPit": return 0;			// fight it out (going down is a fix)
			case "RZ_Mission_KillTheOfficer": return 1;		// do the job (the stores are a betrayal)
			case "RZ_Mission_ConvoyEscort": return 0;		// the crates go in the store (raiders is a lie)
			case "RZ_Mission_MissingScout": return 0;		// tell the council (selling the page back is a betrayal)
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The clever way closes the honest one: once anyone in this run has pressed, tricked, switched
	//! or hidden something, its honest option refuses. "" if `option` is open.
	static string Closed(RZ_GameModeComponent gm, RZ_Mission m, int option)
	{
		if (!m || !m.m_Story || option != HonestOption(m))
			return "";

		foreach (int playerId : m.m_aParticipants)
		{
			if (m.m_Story.m_Deciding.IsDirty(gm, playerId))
				return "Somebody here already played it crooked. The straight way's closed for this one.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! After a one-choice job's own outcome (RZ_OneChoice.Settle): nobody chose, the reaction, the
	//! callback deed, and the others' line.
	static void Settled(RZ_GameModeComponent gm, RZ_Mission m, RZ_OneChoice c, int decider, int option)
	{
		if (!gm || !m || !c)
			return;

		array<int> with = {};
		c.With(gm, m, with);
		if (decider <= 0)
		{
			// Failure is a branch: nobody choosing has its own line before the default stands
			foreach (int playerId : m.m_aParticipants)
			{
				string nobody = RZ_VoiceBank.PickFor(gm, "nobody_chose", m.m_eGiver, "", null, playerId, m.m_vPos);
				if (!nobody.IsEmpty())
					gm.NotifyPlayerAs(playerId, m.m_sTitle, nobody, RZ_ENoticeTag.MISSION);
			}

			return;
		}

		// The third beat: a reaction from the giver's people, through grief and joke memory
		RZ_Outcome.Tell(gm, with, m.m_sTitle, "", "", m.m_eGiver, "reaction", "", m.m_vPos);
		RZ_Outcome.ElseCall(gm, m, with);

		// A choice that involves a person, a house or a leader is a callback deed this life
		string phrase = DeedPhrase(gm, m, c);
		int weight = DeedWeight(c);
		foreach (int pid : with)
		{
			RZ_PlayerProfile p = gm.FindProfile(pid);
			if (p)
				RZ_Deeds.Callback(p, "choice." + m.m_iId.ToString(), phrase, weight);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "decided where the kids go near Levie", from the hold's line.
	protected static string DeedPhrase(RZ_GameModeComponent gm, RZ_Mission m, RZ_OneChoice c)
	{
		string what = c.m_sWhat;
		if (what.StartsWith("decide "))
			what = "decided " + what.Substring(7, what.Length() - 7);
		else
			what = "had the say at " + m.m_sTitle;

		string town = "";
		RZ_Settlement s = gm.GetSettlement(m.m_iSettlement);
		if (!s)
			s = gm.GetSettlement(gm.FindNearestSettlement(m.m_vPos));

		if (s)
			town = " near " + s.m_sName;

		return what + town;
	}

	//------------------------------------------------------------------------------------------------
	//! Heavy when it's about a person's life or freedom, else medium.
	protected static int DeedWeight(RZ_OneChoice c)
	{
		array<string> people = {"he lives", "driver", "raider", "kids", "they go", "barrier", "patrol", "round", "family"};
		foreach (string w : people)
		{
			if (c.m_sWhat.Contains(w))
				return RZ_Deeds.HEAVY;
		}

		return RZ_Deeds.MEDIUM;
	}
}

//------------------------------------------------------------------------------------------------
//! One lying brief: its giver's faction, the versions (each an omission added to the brief in the
//! giver's voice, the truth as found, and the outcome that names it), and the honest outcome.
class RZ_BriefLie
{
	string m_sKey;
	string m_sClass;
	int m_iFaction;
	string m_sReads;							// backgrounds that find it faster
	ref array<string> m_aOmission = {};
	ref array<string> m_aTruth = {};
	ref array<string> m_aOutcome = {};
	string m_sHonest;
}

//------------------------------------------------------------------------------------------------
//! A running job's lie: which version, who's found it.
class RZ_BriefRun
{
	int m_iMission;
	ref RZ_BriefLie m_Lie;
	int m_iVersion;								// 0 honest, else the omission + 1
	int m_iLeader = -1;
	vector m_vScene;
	ref array<string> m_aFound = {};			// identities that know the truth
	ref map<int, float> m_mNear = new map<int, float>();	// playerId -> seconds at the scene
}

//------------------------------------------------------------------------------------------------
class RZ_BriefLies
{
	protected static const float SCENE_RADIUS = 25;
	protected static const float SCENE_SECONDS = 15;
	protected static const float READ_RADIUS = 150;
	protected static const float ROOM = 30;
	protected static const float EVERY = 5;

	protected static ref array<ref RZ_BriefLie> s_aLies;
	protected static ref array<ref RZ_BriefRun> s_aRuns = {};
	protected static float s_fClock;

	//------------------------------------------------------------------------------------------------
	protected static void Add(string key, string cls, int faction, string reads, string honest)
	{
		RZ_BriefLie l = new RZ_BriefLie();
		l.m_sKey = key;
		l.m_sClass = cls;
		l.m_iFaction = faction;
		l.m_sReads = reads;
		l.m_sHonest = honest;
		s_aLies.Insert(l);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Version(string omission, string truth, string outcome)
	{
		RZ_BriefLie l = s_aLies[s_aLies.Count() - 1];
		l.m_aOmission.Insert(omission);
		l.m_aTruth.Insert(truth);
		l.m_aOutcome.Insert(outcome);
	}

	//------------------------------------------------------------------------------------------------
	//! The twelve (docs/MISSIONS.md lists them with their versions).
	protected static array<ref RZ_BriefLie> Lies()
	{
		if (s_aLies)
			return s_aLies;

		s_aLies = new array<ref RZ_BriefLie>();
		int HAVEN = RZ_EFaction.SURVIVORS;
		int BASTION = RZ_EFaction.MILITARY;
		int REDHAND = RZ_EFaction.BANDITS;
		int HALCYON = RZ_EFaction.INSTITUTE;

		Add("drones", "RZ_Mission_LearningTheDrones", REDHAND, "excon,raider", "He really did volunteer. He said it was the only way he'd ever be out of the book.");
		Version("He volunteered, for the record.", "The debtor says it quietly, without looking at you: he didn't volunteer. They told him his sister was in the book too.", "He hadn't volunteered. His sister was in the book.");
		Version("The drones only go for runners. Walk and he's fine.", "Scratched into the marker post: three tallies and the word 'walked'. The drones don't care how you move.", "The drones never cared whether he walked. The crew knew that.");
		Version("He's done this before. Knows the way.", "His boots are new and his hands won't keep still. He's never been near the line.", "He'd never been near the line before today. The crew said he had.");

		Add("arrangement", "RZ_Mission_QuietArrangement", REDHAND, "elder,excon", "It was his idea after all. The note under the board says so, in his own hand.");
		Version("It was his idea, this. He came to us.", "Under the board with the envelope, a note in the keeper's hand: 'Pay or they burn it. Don't tell the house.'", "It was never his idea. It was the crew's, and a can of petrol.");
		Version("It's once a month. Nobody goes short.", "Four more stubs under the board, a week apart. It isn't monthly.", "It was every week, not every month. The stubs under the board said so.");

		Add("requisition", "RZ_Mission_Requisition", BASTION, "soldier,elder", "The house really had signed for it, grudgingly, in its own hand.");
		Version("The house signed for it. It's agreed.", "The form in the crate is signed, but not by anyone who lives here. It's the sergeant's hand.", "Nobody at the house signed anything. The sergeant did it for them.");
		Version("It's surplus. They won't miss it.", "The storekeeper's chalk on the shelf: what's in these crates is next week's meals.", "It wasn't surplus. It was next week's meals.");

		Add("names", "RZ_Job_NamesForBastion", HAVEN, "elder,soldier", "The list was what they said: the names of people long gone.");
		Version("They're only names of people who've already left.", "Two of the names on the list belong to people still sleeping in the house that wrote it.", "Not everyone on the list had left. Two still slept in the house that wrote it.");
		Version("Bastion only wants to count them.", "Bastion's clerk has a second column ruled and waiting: 'for collection'.", "Bastion never wanted to count them. There was a second column.");

		Add("offbook", "RZ_Job_OffBookRequisition", BASTION, "soldier,labtech", "It really was for the post's stores, and the post really was eating paste.");
		Version("They've plenty. They won't miss it.", "The ledger by their door: two meals a day, one on Sundays.", "They hadn't plenty. The ledger by the door said so.");
		Version("The stronghold knows. It's sanctioned.", "The crate for the contribution has no stencil and no log number.", "The stronghold never knew. The crate had no number for a reason.");

		Add("rifles", "RZ_Job_RiflesForSale", BASTION, "soldier,mechanic", "The deal was what it looked like: rifles for fuel and food, and nobody pretended otherwise.");
		Version("The buyer only wants them for hunting.", "The buyer's crates carry a roadblock's mark, and nobody hunts deer with twelve rifles.", "They weren't for hunting. They went to a roadblock.");
		Version("The stronghold signed off on it.", "The manifest's been torn off the crate, carefully.", "The stronghold never signed off. The manifest was torn off for a reason.");

		Add("orders", "RZ_Job_RealOrders", BASTION, "soldier,nurse,doctor", "The courier was ambushed, as the post said, and the orders were real.");
		Version("The courier was ambushed on the road.", "The courier was shot in the back, facing away from the post. He was running from it.", "The courier wasn't ambushed. He was running from the post.");
		Version("Command still answers, now and then.", "The satchel's seal is weeks old. The orders inside are dated Day 3.", "Command hadn't answered since Day 3. The orders said so.");

		Add("trial", "RZ_Job_UnsanctionedTrial", HALCYON, "doctor,nurse,labtech", "He had asked, and his family had too. The paper was real.");
		Version("His family agreed as well.", "His wife is at the door. Nobody asked her anything.", "Nobody asked his family. His wife found out from the needle.");
		Version("The serum's been through animal trials.", "The vial's label: batch one. No trials noted, animal or otherwise.", "It had never been tried on anything. Batch one.");

		Add("collection", "RZ_Job_PrivateCollection", HALCYON, "labtech,hunter", "It was for the work after all, just not the committee's.");
		Version("The director knows. Off the record.", "The case is addressed to a house, not to the lab.", "The director never knew. The case was addressed to a house.");
		Version("It's for the cure. Mine, not theirs.", "The case's paperwork names a buyer, not a study.", "It wasn't for a cure. The paperwork named a buyer.");

		Add("formula", "RZ_Job_SellTheFormula", HALCYON, "labtech,doctor", "It was the full formula, and the director did know. Nobody was proud of it.");
		Version("The director approved it. Quietly.", "The copy carries the director's stamp: NOT FOR RELEASE.", "The director never approved it. The stamp said not for release.");
		Version("It's the whole formula, every step.", "Two steps are missing from the copy. Whoever makes it from this will make it wrong.", "It wasn't the whole formula. Two steps were missing.");

		Add("debtor", "RZ_Mission_PitsDebtor", REDHAND, "excon,raider", "He did owe sixty, every tin of it, and said so himself.");
		Version("Sixty tins, all his own.", "The book's page says twelve. The rest is the crowd's money, riding on him.", "He owed twelve, not sixty. The rest was the crowd's.");
		Version("He agreed to the fight.", "Under the bandages, his wrists are tied.", "He never agreed. His wrists were tied under the bandages.");

		Add("debt", "RZ_Job_DebtCollection", REDHAND, "excon,raider,elder", "They had borrowed it, and they could pay. They just didn't want to.");
		Version("They borrowed it, fair and square.", "The crew's mark on their door is older than any loan. The favour was protection they never asked for.", "They never borrowed anything. The favour was protection they never asked for.");
		Version("They can afford it.", "Their larder's empty but for two tins with children's names on them.", "They couldn't afford it. Two tins, with names on.");

		return s_aLies;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_BriefLie ForClass(string cls)
	{
		foreach (RZ_BriefLie l : Lies())
		{
			if (l.m_sClass == cls)
				return l;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_BriefRun RunOf(int missionId)
	{
		foreach (RZ_BriefRun r : s_aRuns)
		{
			if (r.m_iMission == missionId)
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A job started (RZ_MissionManager.StartMission, after its leader is assigned): one of its
	//! versions, the same all season (0 is honest); a lie adds its omission to the brief. A Talker
	//! giver leaks the truth into the rumours.
	static void OnStarted(RZ_GameModeComponent gm, RZ_Mission m)
	{
		RZ_BriefLie lie = ForClass(m.ClassName());
		RZ_WorldFacts facts = gm.GetFacts();
		if (!lie || !facts)
			return;

		array<string> options = {"0"};
		for (int i = 0; i < lie.m_aOmission.Count(); i++)
		{
			options.Insert((i + 1).ToString());
		}

		RZ_BriefRun run = new RZ_BriefRun();
		run.m_iMission = m.m_iId;
		run.m_Lie = lie;
		run.m_iVersion = facts.CanonPick("brief." + lie.m_sKey, options).ToInt();
		run.m_iLeader = m.m_iLeaderId;
		run.m_vScene = m.m_vPos;
		s_aRuns.Insert(run);
		if (run.m_iVersion <= 0)
		{
			RZ_Log.Info(string.Format("Story: brief %1 is honest this season", lie.m_sKey));
			return;
		}

		int v = run.m_iVersion - 1;
		m.m_sBrief = (m.m_sBrief + " " + lie.m_aOmission[v]).Trim();
		RZ_Log.Info(string.Format("Story: brief %1 leaves something out (version %2)", lie.m_sKey, run.m_iVersion));

		RZ_Leader giver;
		if (m.m_iLeaderId >= 0 && gm.GetLeaders())
			giver = gm.GetLeaders().Get(m.m_iLeaderId);

		if (giver && giver.HasTrait(RZ_ELeaderTrait.TALKER))
			RZ_Questline.SpreadRumour(gm, "\"" + lie.m_aTruth[v] + "\"", 12);
	}

	//------------------------------------------------------------------------------------------------
	//! On the mission manager's tick: anyone who took part, at the scene for 15 s (a clue there, open
	//! to everyone), or with a listed background within 150 m, finds the truth; the room hears it.
	static void Tick(RZ_GameModeComponent gm, float dt, notnull array<ref RZ_Mission> active)
	{
		s_fClock += dt;
		if (s_fClock < EVERY || s_aRuns.IsEmpty())
			return;

		float step = s_fClock;
		s_fClock = 0;
		foreach (RZ_Mission m : active)
		{
			RZ_BriefRun run = RunOf(m.m_iId);
			if (!run || run.m_iVersion <= 0)
				continue;

			foreach (int playerId : m.m_aParticipants)
			{
				RZ_PlayerProfile p = gm.FindProfile(playerId);
				IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
				if (!p || !ent || run.m_aFound.Contains(p.m_sIdentity))
					continue;

				float d = RZ_Util.DistXZ(ent.GetOrigin(), run.m_vScene);
				bool reads = d <= READ_RADIUS && RZ_QuestApproach.HasBackground(gm, playerId, run.m_Lie.m_sReads);
				float there;
				run.m_mNear.Find(playerId, there);
				if (d <= SCENE_RADIUS)
					there += step;
				else
					there = 0;

				run.m_mNear.Set(playerId, there);
				if (reads || there >= SCENE_SECONDS)
					Reveal(gm, m, run, playerId, ent.GetOrigin());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Reveal(RZ_GameModeComponent gm, RZ_Mission m, RZ_BriefRun run, int finder, vector at)
	{
		string truth = run.m_Lie.m_aTruth[run.m_iVersion - 1];
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity e : players)
		{
			int pid = ids[i];
			if (pid != finder && RZ_Util.DistXZ(e.GetOrigin(), at) > ROOM)
				continue;

			RZ_PlayerProfile p = gm.FindProfile(pid);
			if (!p || run.m_aFound.Contains(p.m_sIdentity))
				continue;

			run.m_aFound.Insert(p.m_sIdentity);
			gm.NotifyPlayerAs(pid, m.m_sTitle, truth, RZ_ENoticeTag.MISSION);
			if (m.m_Story && !m.m_Story.m_aFound.Contains(truth))
				m.m_Story.m_aFound.Insert(truth);
		}

		RZ_Log.Info(string.Format("Story: brief %1's truth found by %2", run.m_Lie.m_sKey, RZ_Util.PlayerName(finder)));
	}

	//------------------------------------------------------------------------------------------------
	//! The job ended (RZ_MissionManager.Finish): the outcome names it, either way, to those who took
	//! part; those who found a lie can take it public (talk, `call:brief_public`).
	static void OnFinished(RZ_GameModeComponent gm, RZ_Mission m, bool success)
	{
		RZ_BriefRun run = RunOf(m.m_iId);
		if (!run)
			return;

		s_aRuns.RemoveItem(run);
		if (m.m_bWithdrawn)
			return;

		string line = run.m_Lie.m_sHonest;
		if (run.m_iVersion > 0)
			line = run.m_Lie.m_aOutcome[run.m_iVersion - 1];

		RZ_WorldFacts facts = gm.GetFacts();
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		foreach (int playerId : m.m_aParticipants)
		{
			gm.NotifyPlayerAs(playerId, m.m_sTitle, line, RZ_ENoticeTag.MISSION);
			RZ_PlayerProfile p = gm.FindProfile(playerId);
			if (run.m_iVersion <= 0 || !p || !run.m_aFound.Contains(p.m_sIdentity) || !facts)
				continue;

			// Kept for the season: which lie this player can take to the giver's people
			string fw = RZ_DialogueDB.FactionWord(run.m_Lie.m_iFaction);
			facts.SetFlag(string.Format("brieflie.%1.%2", p.m_sIdentity, fw), Lies().Find(run.m_Lie) + 1);
			facts.SetFlag(string.Format("briefgiver.%1.%2", p.m_sIdentity, fw), run.m_iLeader + 1);
			if (trees)
				trees.GetMemory().Learn(playerId, "brieflie." + fw, 168);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Taking a caught lie public at one of the giver's faction's leaders (talk, `call:brief_public`):
	//! the faction loses 1 morale (at most once per faction per real day, once per lie per season,
	//! inside the morale caps); an outpost leader who gave it loses 5 loyalty. No reward of its own.
	//! The line to the player, "" if there was nothing to tell.
	static string GoPublic(RZ_GameModeComponent gm, int playerId, int faction)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		RZ_PlayerProfile p = gm.FindProfile(playerId);
		if (!facts || !p || !RZ_Factions.IsValid(faction))
			return "";

		string fw = RZ_DialogueDB.FactionWord(faction);
		string lieKey = string.Format("brieflie.%1.%2", p.m_sIdentity, fw);
		int idx = facts.Flag(lieKey) - 1;
		int leaderId = facts.Flag(string.Format("briefgiver.%1.%2", p.m_sIdentity, fw)) - 1;
		facts.SetFlag(lieKey, 0);
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees)
			trees.GetMemory().Learn(playerId, "brieflie." + fw, 0.001);

		if (idx < 0 || idx >= Lies().Count())
			return "";

		RZ_BriefLie lie = Lies()[idx];
		string once = "briefpublic." + lie.m_sKey;
		string dayKey = "briefpublic.day." + fw;
		int today = RZ_SeasonClock.Now() / 86400;
		if (facts.Flag(once) > 0 || facts.Flag(dayKey) == today + 1)
			return "They've heard it already. It doesn't land twice.";

		facts.SetFlag(once, 1);
		facts.SetFlag(dayKey, today + 1);
		RZ_OneChoice.Morale(gm, faction, -1, "a giver's lie made public");
		RZ_Leaders leaders = gm.GetLeaders();
		RZ_Leader giver;
		if (leaders && leaderId >= 0)
			giver = leaders.Get(leaderId);

		if (giver && !giver.IsStronghold())
			RZ_StoryKit.Loyalty(gm, giver, -5);

		RZ_Log.Info(string.Format("Story: brief %1's lie made public by %2", lie.m_sKey, RZ_Util.PlayerName(playerId)));
		return "It goes round the fires by evening. Nobody thanks you for it.";
	}
}

//------------------------------------------------------------------------------------------------
//! Boards of record (late joiners read the season in the world): a few stable boards whose text is
//! worked out from the season flags each time it's read, the same for everyone, with nothing new
//! saved but the toll's count. The epilogue's props keep their own week text; these are the in-season
//! words, read where the board stands: the Haven council's pinned notes (a safehouse's Ask how
//! things are), Bastion's roster board (a Bastion road post's speaker), the toll sign's tally (a Red
//! Hand roadblock's speaker), the lab door's sheet (the journal's lab line). Server only.
class RZ_RecordBoards
{
	//------------------------------------------------------------------------------------------------
	protected static int F(string key)
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (!facts)
			return 0;

		return facts.Flag(key);
	}

	//------------------------------------------------------------------------------------------------
	//! The council's pinned notes at a safehouse: the house rule, the List, who came in from Bastion.
	static string Council()
	{
		string text = "";
		switch (F("haven.rule"))
		{
			case 1: text = "EVERY NAME. SHARE THE POT."; break;
			case 2: text = "WORKERS EAT FIRST. THE FAR HOUSE FOR THE REST."; break;
			case 3: text = "COUNTED. Bring your card."; break;
			case 4: text = "Paid. (Small letters, low down.)"; break;
		}

		if (F("list.read") > 0)
			text = (text + " The List was read out. The names are pinned under it.").Trim();
		else if (F("list.buried") > 0)
			text = (text + " A pin with nothing under it, where the List was.").Trim();

		int sheltered = F("bastion.sheltered");
		if (sheltered > 0)
			text = (text + string.Format(" Taken in from Bastion's line this season: %1.", sheltered)).Trim();

		if (text.IsEmpty())
			return "";

		return "Pinned by the council: " + text;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's roster board at a road post: the call, the struck-off men, the civilians sheltered.
	static string Roster()
	{
		string text = "";
		switch (F("orders.stance"))
		{
			case 1: text = "STATUS: AWAITING RELIEF."; break;
			case 2: text = "STATUS: ALL STATIONS. CIVILIANS FIRST."; break;
			case 3: text = "STATUS: BASTION, COMMAND."; break;
			case 4: text = "STATUS: EXPENDED."; break;
		}

		int home = F("struck.home");
		int stillOut = F("struck.out");
		if (home > 0)
			text = (text + string.Format(" Struck off, walked home: %1.", home)).Trim();

		if (stillOut > 0)
			text = (text + string.Format(" Struck off, still out: %1.", stillOut)).Trim();

		if (text.IsEmpty())
			return "";

		return "The roster board by the barrier: " + text;
	}

	//------------------------------------------------------------------------------------------------
	//! The toll sign's tally at a Red Hand roadblock: tolls paid this season, or the book's end.
	static string Tally()
	{
		if (F("book.burnt") > 0 || F("book.debts_dead") > 0)
			return "Under the toll sign, the old tally's been scrubbed off. Nobody's started a new one.";

		int paid = F("tolls.paid");
		if (paid <= 0)
			return "";

		return string.Format("Under the toll sign, a tally in chalk: %1 paid this season. Somebody's drawn a smile on the fifties.", paid);
	}

	//------------------------------------------------------------------------------------------------
	//! A toll paid (RZ_Checkpoints): the sign's count, kept with the season.
	static void NoteToll()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (facts)
			facts.SetFlag("tolls.paid", facts.Flag("tolls.paid") + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The lab door's sheet: what the lab has put on paper this season.
	static string LabSheet()
	{
		string text = "";
		switch (F("proof.first_page"))
		{
			case 1: text = "Origin: Day 1. Under investigation."; break;
			case 2: text = "The whole of it, from the start, with the site's grid."; break;
			case 4: text = "Under seal. Enquiries to Bastion."; break;
		}

		if (F("other_two.numbers") > 0)
			text = (text + " Trial subjects listed by number.").Trim();

		if (text.IsEmpty())
			return "";

		return "On the lab door, a sheet: " + text;
	}
}
