//! The papers in the case's lid (docs/DESIGN.md, "The papers in the lid"): the runner was a courier
//! trying to get proof out past the line, and the cold case (RZ_OpeningCase) wasn't all he carried.
//! Folded into its lid are three papers that aren't Halcyon's, each the page a stronghold would
//! rather burn (docs/QUESTLINES.md, "The majors"): Bastion's Day 3 orders (Await Relief), a page torn
//! from the Red Hand's book (The Book Has to Balance) and a list of names in a Haven hand (Every Name).
//!
//! The character who takes the case takes the papers with it, once (RZ_OpeningCase.CharacterStamp).
//! They're quest items kept as data on the profile, so a death never loses them. Whoever the case
//! is handed to remarks on each. Then each paper is a reply at any faction's people, one at a time:
//! - at its own faction's people, **hand it over**: you're a participant of that major (RZ_QuestRun.Did)
//!   and they tell you its act and where; a major not running yet takes you when it starts;
//! - at anyone else's, **take it to its owner**: a tracked job to their stronghold (only a job you
//!   took is listed), which only the hand-over reply ends;
//! - and once you hold that job, **give it to them instead**: the page in the open (majors.pages_open,
//!   once a season per paper, never twice for the same page), the rival's line, small standing.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_CasePapers
{
	static const int ORDERS = 0;	// Bastion's Day 3 orders: Await Relief ("orders")
	static const int PAGE = 1;		// the Red Hand's book's page: The Book Has to Balance ("book")
	static const int LIST = 2;		// the Haven's list of names: Every Name ("every_name")
	static const int COUNT = 3;

	// A paper's state on the profile (RZ_PlayerProfile.m_aPapers)
	static const int NONE = 0;
	static const int CARRIED = 1;
	static const int OWNER = 2;		// handed to its own faction; you take part in its major
	static const int OWNER_WAIT = 3;	// handed to its own faction; its major isn't running yet
	static const int GIVEN = 10;	// 10 + the faction it was given to instead

	// Talk reply ids: 53 + paper x 3 + kind (the case's are 50-52)
	static const int REPLY_BASE = 53;
	static const int R_HAND = 0;	// hand it to its own faction
	static const int R_JOB = 1;		// take the job of carrying it to them
	static const int R_RIVAL = 2;	// give it to this speaker's faction instead

	protected static const int FAME = 3;		// small, through the caps
	protected static const int INFAMY = 3;
	protected static const float WAIT_TRY = 30;	// a waiting hand-over is tried this often (s)

	protected RZ_GameModeComponent m_GM;
	protected ref array<ref RZ_CaseStepDef> m_aJob = {};
	protected ref map<string, int> m_mJobs = new map<string, int>();	// "player:paper" -> the job's errand id
	protected float m_fClock;
	protected float m_fNextWait;

	//------------------------------------------------------------------------------------------------
	void RZ_CasePapers(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		for (int p = 0; p < COUNT; p++)
		{
			RZ_CaseStepDef d = new RZ_CaseStepDef("opening.case.paper" + p.ToString(), JobTitle(p), RZ_OpeningCase.KIND_PAPER);
			d.m_iPaper = p;
			d.m_fLapseMinutes = 1440;
			RZ_ErrandWay w = d.Way("do");
			w.m_ePlace = RZ_EErrandPlace.SUBJECT;
			w.m_fSeconds = 1;
			w.m_fRadius = 60;
			w.m_bEndsThere = true;
			w.m_sStep = JobStep(p);
			w.Take(JobTake(p), "{place}. Their people will know what it is.");
			m_aJob.Insert(d);
		}
	}

	// ================================================================ The papers
	//------------------------------------------------------------------------------------------------
	//! The major each paper belongs to.
	static string MajorKey(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "orders";
			case PAGE: return "book";
		}

		return "every_name";
	}

	//------------------------------------------------------------------------------------------------
	//! The faction whose page it is.
	static int Owner(int paper)
	{
		switch (paper)
		{
			case ORDERS: return RZ_EFaction.MILITARY;
			case PAGE: return RZ_EFaction.BANDITS;
		}

		return RZ_EFaction.SURVIVORS;
	}

	//------------------------------------------------------------------------------------------------
	static string Name(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "the Day 3 orders";
			case PAGE: return "the page from the book";
		}

		return "the list of names";
	}

	//------------------------------------------------------------------------------------------------
	//! What it looks like, for the case's read.
	static string Looks(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "an order sheet with Command's stamp, dated Day 3";
			case PAGE: return "a page torn from a ledger: names, amounts, a red thumbprint";
		}

		return "a list of names in pencil, some crossed out";
	}

	//------------------------------------------------------------------------------------------------
	protected static string OwnerName(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "Bastion";
			case PAGE: return "the Red Hand";
		}

		return "the Haven";
	}

	//------------------------------------------------------------------------------------------------
	protected static string JobTitle(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "The Day 3 Orders";
			case PAGE: return "A Page of the Book";
		}

		return "The List";
	}

	//------------------------------------------------------------------------------------------------
	protected static string JobStep(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "Take the Day 3 orders to Bastion's people at {place}.";
			case PAGE: return "Take the page back to the Red Hand's people at {place}.";
		}

		return "Take the list of names to the Haven's people at {place}.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string JobTake(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "Command's stamp, Day 3. Bastion's at {place}. Whatever's on that sheet, they'll want it back before anyone reads it out.";
			case PAGE: return "That's a page out of the book. The crews are at {place}. They count their pages.";
		}

		return "Those are Haven names. The chair's people are at {place}. Somebody there will know whose hand that is.";
	}

	//------------------------------------------------------------------------------------------------
	//! The papers' states on this profile, three long (an older save's empty list grows here).
	static array<int> States(RZ_PlayerProfile profile)
	{
		if (!profile.m_aPapers)
			profile.m_aPapers = {};

		while (profile.m_aPapers.Count() < COUNT)
		{
			profile.m_aPapers.Insert(NONE);
		}

		return profile.m_aPapers;
	}

	//------------------------------------------------------------------------------------------------
	//! This character's papers belong to them (not a dead character's: designs/PERSISTENCE.md decides
	//! what a character is, through RZ_OpeningCase.CharacterStamp).
	static bool Mine(RZ_PlayerProfile profile)
	{
		return profile && profile.m_iPapersLife == RZ_OpeningCase.CharacterStamp(profile);
	}

	//------------------------------------------------------------------------------------------------
	//! The state of one paper for this character, NONE when they never had them.
	static int StateOf(RZ_PlayerProfile profile, int paper)
	{
		if (!Mine(profile))
			return NONE;

		return States(profile)[paper];
	}

	//------------------------------------------------------------------------------------------------
	//! This character carries this paper.
	static bool Carries(RZ_PlayerProfile profile, int paper)
	{
		return StateOf(profile, paper) == CARRIED;
	}

	//------------------------------------------------------------------------------------------------
	//! Taken with the case, once a character: all three, in the lid.
	void Gain(int playerId)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!profile || Mine(profile))
			return;

		profile.m_iPapersLife = RZ_OpeningCase.CharacterStamp(profile);
		array<int> states = States(profile);
		for (int p = 0; p < COUNT; p++)
		{
			states[p] = CARRIED;
		}

		RZ_Log.Info("Papers: " + RZ_Util.PlayerName(playerId) + " has the three papers from the case's lid");
	}

	//------------------------------------------------------------------------------------------------
	//! The papers' part of the case's read: what's folded in the lid.
	static string ReadText()
	{
		return string.Format("\n\nFolded into the lid, three papers that aren't Halcyon's: %1; %2; %3.", Looks(ORDERS), Looks(PAGE), Looks(LIST));
	}

	// ================================================================ At the case's hand-over
	//------------------------------------------------------------------------------------------------
	//! Whoever takes the case notices the papers that stay with you: a pointed word on each, by who
	//! they are (Halcyon, the Red Hand, Bastion).
	void Remark(int playerId, int speakerFaction, notnull array<string> said)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		for (int p = 0; p < COUNT; p++)
		{
			if (!Carries(profile, p))
				continue;

			string line = RemarkLine(p, speakerFaction);
			if (!line.IsEmpty())
				said.Insert(line);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string RemarkLine(int paper, int speaker)
	{
		if (speaker == RZ_EFaction.INSTITUTE)
		{
			switch (paper)
			{
				case ORDERS: return "And that's not ours. That's Bastion's: Command's stamp, Day 3. I'd keep it away from the adjutant, or take it straight to him.";
				case PAGE: return "That's a page out of the Red Hand's book. They count those.";
			}

			return "Names. In a Haven hand. Somebody at the chair's table will want that back, or wish they didn't.";
		}

		if (speaker == RZ_EFaction.BANDITS)
		{
			switch (paper)
			{
				case ORDERS: return "Bastion's orders, Day 3. Worth more than the case, to the right people.";
				case PAGE: return "And that's a page of the book. Ours. We'll talk about that.";
			}

			return "The Haven's names. The chair would pay to have that read, or burned.";
		}

		if (speaker == RZ_EFaction.MILITARY)
		{
			switch (paper)
			{
				case ORDERS: return "That's Command's stamp. That sheet is ours, and the adjutant will want to know who else read it.";
				case PAGE: return "A Red Hand ledger page. Evidence, if anyone ever holds a court again.";
			}

			return "Haven names. Not our business. Yet.";
		}

		return "";
	}

	// ================================================================ The replies
	//------------------------------------------------------------------------------------------------
	static bool IsOurs(int replyId)
	{
		return replyId >= REPLY_BASE && replyId < REPLY_BASE + COUNT * 3;
	}

	//------------------------------------------------------------------------------------------------
	//! For each paper this character carries, one reply at any faction's people: at its own, hand it
	//! over; elsewhere, take the job of carrying it to them, or with that job held, give it to them
	//! instead.
	void AddReplies(RZ_TalkSession s, int speaker, notnull array<ref RZ_TalkReply> replies)
	{
		if (!RZ_Factions.IsValid(speaker))
			return;

		RZ_PlayerProfile profile = m_GM.GetProfile(s.m_iPlayer);
		for (int p = 0; p < COUNT; p++)
		{
			if (!Carries(profile, p))
				continue;

			RZ_TalkReply r = new RZ_TalkReply();
			r.m_bOwn = true;
			r.m_bJob = true;
			r.m_iRank = RZ_TalkReply.RANK_ON;
			if (speaker == Owner(p))
			{
				r.m_iId = REPLY_BASE + p * 3 + R_HAND;
				r.m_sLabel = "Hand over " + Name(p);
				r.m_sTag = RZ_TalkJobs.Tag("Errand");
			}
			else if (JobOf(s.m_iPlayer, p) < 0)
			{
				if (!HasOwnerPlace(p))
					continue;

				r.m_iId = REPLY_BASE + p * 3 + R_JOB;
				r.m_sLabel = string.Format("I'll take %1 to %2", Name(p), OwnerName(p));
				r.m_sTag = RZ_TalkJobs.Tag("Errand");
				r.m_iRank = RZ_TalkReply.RANK_NEW;
			}
			else
			{
				r.m_iId = REPLY_BASE + p * 3 + R_RIVAL;
				r.m_sLabel = string.Format("Give them %1 instead", Name(p));
				r.m_sTag = RZ_TalkJobs.Tag("Trade");
			}

			replies.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A paper's reply picked.
	void Pick(RZ_TalkSession s, int speaker, int replyId, IEntity user)
	{
		int playerId = s.m_iPlayer;
		int p = (replyId - REPLY_BASE) / 3;
		int kind = (replyId - REPLY_BASE) % 3;
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!Carries(profile, p))
		{
			s.m_aNew.Insert("What paper?");
			return;
		}

		if (kind == R_JOB)
		{
			TakeJob(s, p, user);
			return;
		}

		if (kind == R_HAND)
			ToOwner(s, p, user);
		else
			ToRival(s, p, speaker, user);
	}

	//------------------------------------------------------------------------------------------------
	//! To its own faction: they take it, you're in their major, and they say where it stands.
	protected void ToOwner(RZ_TalkSession s, int p, IEntity user)
	{
		int playerId = s.m_iPlayer;
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		s.m_aNew.Insert(OwnerLine(p));
		m_GM.AddFame(playerId, Owner(p), FAME, false, RZ_RepFlag.SMALL);
		States(profile)[p] = OWNER_WAIT;
		if (Join(playerId, p))
		{
			States(profile)[p] = OWNER;
			s.m_aNew.Insert(ActLine(p));
		}
		else
		{
			s.m_aNew.Insert(WaitLine(p));
		}

		EndJob(playerId, p, user, true);
		RZ_Log.Info(string.Format("Papers: %1 handed %2 to %3", RZ_Util.PlayerName(playerId), Name(p), OwnerName(p)));
	}

	//------------------------------------------------------------------------------------------------
	//! To a rival: the page is in the open (once a season for each paper), their line, small standing.
	protected void ToRival(RZ_TalkSession s, int p, int speaker, IEntity user)
	{
		int playerId = s.m_iPlayer;
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		s.m_aNew.Insert(RivalLine(p, speaker));
		m_GM.AddFame(playerId, speaker, FAME, false, RZ_RepFlag.SMALL);
		m_GM.AddInfamy(playerId, Owner(p), INFAMY, "gave away " + Name(p));
		States(profile)[p] = GIVEN + speaker;

		// The page in the open: the majors' own count, once a season for each of these three pages
		string flag = "papers." + MajorKey(p) + ".open";
		if (RZ_MajorHelp.World(m_GM, flag) == 0)
		{
			RZ_MajorHelp.SetWorld(m_GM, flag, 1);
			RZ_MajorHelp.OpenPage(m_GM, Name(p) + " from the runner's case, given to " + RZ_Factions.GetNameInText(speaker));
		}

		EndJob(playerId, p, user, false);
		RZ_Log.Info(string.Format("Papers: %1 gave %2 to %3 instead", RZ_Util.PlayerName(playerId), Name(p), RZ_Factions.GetNameInText(speaker)));
	}

	//------------------------------------------------------------------------------------------------
	//! This character is a participant of the paper's major. False: it isn't running yet.
	protected bool Join(int playerId, int p)
	{
		RZ_QuestRun run = MajorRun(p);
		if (!run)
			return false;

		run.Did(m_GM, playerId, RZ_EQuestApproach.TALK, "brought " + Name(p) + " from the runner's case");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_QuestRun MajorRun(int p)
	{
		RZ_Questlines lines = m_GM.GetQuestlines();
		if (!lines)
			return null;

		return lines.FindRunByKey(MajorKey(p));
	}

	//------------------------------------------------------------------------------------------------
	protected static string OwnerLine(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "Day 3. You read it? Then you know more than most of the men here. Leave it with me.";
			case PAGE: return "That's our page. Somebody tore it out, and somebody's going to explain why. Not you. You brought it back.";
		}

		return "That's the chair's hand. I'd know it anywhere. Thank you. I think.";
	}

	//------------------------------------------------------------------------------------------------
	//! Where the major stands: its act's name, and its place when it has one.
	protected string ActLine(int p)
	{
		RZ_QuestRun run = MajorRun(p);
		string act = "";
		if (run && run.m_Line)
		{
			RZ_MajorAct a = run.m_Line.ActOf(run.m_iStage);
			if (a)
				act = a.m_sName;
		}

		if (act.IsEmpty())
			return "You've seen what's on it. That makes it your business now. Come back when we call for people.";

		if (run.m_bHasPlace && !run.m_sPlaceName.IsEmpty())
			return string.Format("It's %1 now, at %2. You've seen the paper. You might as well see the rest.", act, run.m_sPlaceName);

		return string.Format("It's %1 now. You've seen the paper. You might as well see the rest.", act);
	}

	//------------------------------------------------------------------------------------------------
	protected static string WaitLine(int paper)
	{
		switch (paper)
		{
			case ORDERS: return "Nobody's calling a muster over it yet. When they do, you'll be one of the ones who knows why.";
			case PAGE: return "The book isn't open for this yet. When it is, you'll be standing there.";
		}

		return "The chair isn't ready to read it. When the houses sit, you'll be at the table.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string RivalLine(int paper, int speaker)
	{
		switch (speaker)
		{
			case RZ_EFaction.SURVIVORS: return "The chair will read this at supper. Everyone will hear it, whoever wrote it.";
			case RZ_EFaction.MILITARY: return "Evidence. It goes in the stores, and the CO decides who sees it.";
			case RZ_EFaction.BANDITS: return "Into the book it goes. Someone will pay to have it out again.";
		}

		return "We'll keep it with the other things nobody's supposed to know.";
	}

	// ================================================================ The job of carrying it
	//------------------------------------------------------------------------------------------------
	//! The paper's owner's stronghold (else their nearest holding). False: they have none.
	bool HasOwnerPlace(int p)
	{
		vector place;
		string text;
		return OwnerPlace(p, place, text);
	}

	//------------------------------------------------------------------------------------------------
	bool OwnerPlace(int p, out vector place, out string text)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory)
			return false;

		RZ_Holding seat = territory.GetStronghold(Owner(p));
		if (seat)
		{
			place = seat.m_vPos;
			text = seat.m_sName;
			return true;
		}

		array<RZ_Holding> list = {};
		territory.GetOutposts(Owner(p), list);
		if (list.IsEmpty())
			return false;

		place = list[0].m_vPos;
		text = list[0].m_sName;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TakeJob(RZ_TalkSession s, int p, IEntity user)
	{
		int playerId = s.m_iPlayer;
		RZ_Errands errands = m_GM.GetErrands();
		vector place;
		string text;
		if (!errands || !errands.IsOn() || !user || !OwnerPlace(p, place, text))
		{
			s.m_aNew.Insert("Then take it to them yourself. I can't tell you where they are.");
			return;
		}

		string giver = s.m_sName;
		if (giver.IsEmpty())
			giver = "The runner's case";

		RZ_Errand e = errands.Hand(m_aJob[p], playerId, "paper:" + playerId.ToString() + ":" + p.ToString(), giver, giver, Owner(p), user.GetOrigin(), place, text, user);
		if (!e)
			return;

		m_mJobs.Set(JobKey(playerId, p), e.m_iId);
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (feed && feed.GetTracker())
			feed.GetTracker().Follow(playerId, -(RZ_UIFeed.ERRAND_ID_BASE + e.m_iId), e.Fill(m_aJob[p].m_sTitle));

		RZ_Log.Info(string.Format("Papers: %1 took the job of carrying %2 to %3 at %4", RZ_Util.PlayerName(playerId), Name(p), OwnerName(p), text));
	}

	//------------------------------------------------------------------------------------------------
	protected static string JobKey(int playerId, int p)
	{
		return playerId.ToString() + ":" + p.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The job's errand id while it's held, -1 none (the runtime test reads it).
	int JobOf(int playerId, int p)
	{
		int id;
		if (!m_mJobs.Find(JobKey(playerId, p), id))
			return -1;

		RZ_Errands errands = m_GM.GetErrands();
		RZ_Errand e;
		if (errands)
			e = errands.FindById(id);

		if (e && e.m_eState == RZ_EErrandState.HELD)
			return id;

		m_mJobs.Remove(JobKey(playerId, p));
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The job ends with the paper's hand-over: done when it reached its owner, dropped otherwise.
	protected void EndJob(int playerId, int p, IEntity me, bool done)
	{
		int id = JobOf(playerId, p);
		m_mJobs.Remove(JobKey(playerId, p));
		if (id < 0)
			return;

		RZ_Errands errands = m_GM.GetErrands();
		RZ_Errand e = errands.FindById(id);
		if (!e)
			return;

		if (done && errands.FinishNow(e, me))
			return;

		errands.Drop(e, "the paper changed hands");
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! A paper handed over while its major wasn't running joins you to it once it starts.
	void Tick(float dt, array<int> ids)
	{
		m_fClock += dt;
		if (m_fClock < m_fNextWait)
			return;

		m_fNextWait = m_fClock + WAIT_TRY;
		foreach (int playerId : ids)
		{
			RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
			if (!Mine(profile))
				continue;

			array<int> states = States(profile);
			for (int p = 0; p < COUNT; p++)
			{
				if (states[p] != OWNER_WAIT || !Join(playerId, p))
					continue;

				states[p] = OWNER;
				RZ_Log.Info(string.Format("Papers: %1 joins %2 now it's begun (they handed in %3 earlier)", RZ_Util.PlayerName(playerId), MajorKey(p), Name(p)));
			}
		}
	}
}
