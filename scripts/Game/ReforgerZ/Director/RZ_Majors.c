//! The four majors' engine (docs/DESIGN.md, "The majors"; the stories in docs/QUESTLINES.md, "The
//! majors"). A major is a questline that calls Major() in its Define: one per faction, in a lane of
//! its own, its stages grouped into acts (RZ_MajorAct). Each act opens on its own trigger (a cure
//! milestone, a stage, the act before it) and has a default: what happens when nobody decides it.
//!
//! This file holds what every major shares:
//! - RZ_MajorSettings (Director > Majors) and RZ_MajorAct, an act's clock: anchor, need, patience,
//!   set-piece window, last call;
//! - RZ_QuestVote: tallies kept in a run's flags (the Haven's sittings, Bastion's muster);
//! - RZ_Majors: the dynamic fallbacks (decision 10). It reads research position and pace, hours
//!   played a day, room left before an act's anchor, the stage, the faction's state, offered time,
//!   touches and the beat gap, and decides squeezes, patience, as-data runs, last calls and grace,
//!   leads, between beats and the hard stops. Its counters live in the runs' flags ("mc.*"), its
//!   own readings in RZ_MajorsSave;
//! - the majors' pay caps and the budget context (NudgeMorale and the standing helpers go through
//!   the season budget's majors lane while a major is acting);
//! - the season story record (SeasonStory; last_season.json).
//! Read on the questlines' tick (every 10 s with players online). Nothing polls per frame and
//! nothing moves on an empty server. Server only.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Director > Majors. Attribute defaults only.
[BaseContainerProps()]
class RZ_MajorSettings
{
	[Attribute("1", desc: "The four majors on: one season-long story per faction, its climax on a cure milestone (docs/QUESTLINES.md, The majors). Needs Questlines on. Off: The List, The Three Weeks and Authors run on their own again, and The Book and Await Relief run as their built questlines")]
	bool m_bEnabled;

	[Attribute("5", desc: "How many research points before its milestone a climax may open while research has stalled (0: never)")]
	float m_fLeadPoints;

	[Attribute("1", desc: "Grace after a last call, in sessions (2 hours) of offered time, before the act's default fires")]
	float m_fGraceSessions;

	[Attribute("1", desc: "How long a running or recently touched act can hold its anchor, in sessions (an evening for a set piece)")]
	float m_fHoldLimit;

	[Attribute("1", desc: "Research gained over the last 3 real days with play below which research counts as stalled")]
	float m_fStallPoints;

	[Attribute("4", desc: "Sessions since a major's last beat before it fills the gap with what already exists (its faction's smaller stories, a line about what's coming)")]
	float m_fBetweenBeats;

	[Attribute("2", desc: "At most this many majors' stage missions offered at once, server-wide (a squeezed act takes a slot from one that isn't)")]
	int m_iSlots;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: the Red Hand lorry at Rudi's crossroads for the Book's send-off and its last line (spawned near players only). A wreck or static prop, never a drivable vehicle: the dressing guard deletes one. Empty: words only", "et")]
	ResourceName m_sLorryPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: the bonfire at Rudi's crossroads when the book is sent off loud (spawned near players only). Empty: words only", "et")]
	ResourceName m_sBonfirePrefab;
}

//------------------------------------------------------------------------------------------------
//! What an act must be settled before.
enum RZ_EMajorAnchor
{
	CURE,	// a cure milestone (percent)
	STAGE,	// an outbreak stage (RZ_EOutbreakStage)
	GATE,	// the 90 percent gate
	WIPE	// the epilogue's last day (the codas)
}

//------------------------------------------------------------------------------------------------
//! One act of a major: its stages and its clock (docs/QUESTLINES.md, "Every act's clock"). Built in
//! Define() with Act(id, name) and tuned by chaining:
//!   Act(A_SEND, "Send him off").Stage(S_SEND).AnchorCure(50).Need(1).PatienceEvenings(2).SetPiece(16, 21)
class RZ_MajorAct
{
	int m_iId;
	string m_sName;
	ref array<int> m_aStages = {};
	RZ_EMajorAnchor m_eAnchor = RZ_EMajorAnchor.GATE;
	float m_fAnchor = 90;			// the cure percent, or the outbreak stage
	float m_fNeed = 1;				// roughly how much play it takes, in sessions (an evening counts as one)
	float m_fPatience;				// sessions of offered time; 0 = none (open until squeezed)
	int m_iPatienceEvenings;		// offered evenings (set pieces); 0 = none
	bool m_bSetPiece;
	float m_fFrom = 16;				// the set piece's offer window, game hours
	float m_fTo = 21;
	string m_sLastCall;				// in the act's own voice; {keys} filled from the run
	bool m_bClimax;
	bool m_bPastGate;				// runs past the gate (A Week On, Before the stand, the codas)
	bool m_bCostsMorale;			// its patience default costs the faction morale (waits at a low ebb)
	bool m_bWatch;					// the clock runs on its wait stages too (an inner line it watches)
	float m_fOpens;					// the cure percent it opens at (0: none), for "between beats"
	int m_iRank = 1;				// the access rank a player needs to qualify (Known)

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct Stage(int stage)
	{
		m_aStages.Insert(stage);
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct AnchorCure(float percent)
	{
		m_eAnchor = RZ_EMajorAnchor.CURE;
		m_fAnchor = percent;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct AnchorStage(RZ_EOutbreakStage stage)
	{
		m_eAnchor = RZ_EMajorAnchor.STAGE;
		m_fAnchor = stage;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct AnchorWipe()
	{
		m_eAnchor = RZ_EMajorAnchor.WIPE;
		m_bPastGate = true;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct Need(float sessions)
	{
		m_fNeed = Math.Max(0, sessions);
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct Patience(float sessions)
	{
		m_fPatience = Math.Max(0, sessions);
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct PatienceEvenings(int evenings)
	{
		m_iPatienceEvenings = Math.MaxInt(0, evenings);
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct SetPiece(float fromHour, float toHour)
	{
		m_bSetPiece = true;
		m_fFrom = fromHour;
		m_fTo = toHour;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct LastCall(string line)
	{
		m_sLastCall = line;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct Climax()
	{
		m_bClimax = true;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct PastGate()
	{
		m_bPastGate = true;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! The act's wait stages count as open (Proof's acts II and III watch HC1 and HC3 run).
	RZ_MajorAct Watch()
	{
		m_bWatch = true;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! The act opens at a cure milestone: until then its major may be between beats.
	RZ_MajorAct Opens(float percent)
	{
		m_fOpens = percent;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct CostsMorale()
	{
		m_bCostsMorale = true;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct Rank(int rank)
	{
		m_iRank = rank;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	bool Has(int stage)
	{
		return m_aStages.Contains(stage);
	}
}

//------------------------------------------------------------------------------------------------
//! A tally kept in a run's flags (docs/DESIGN.md, "Votes"): voters with a lean each, options
//! tabled, one sway per player (by identity) per vote, the tally with a tie-break. Errand memory
//! moves one voter more only with Majors read errand memory on (RememberedErrands).
//!   RZ_QuestVote v = new RZ_QuestVote(run, "first");
//!   int i = v.AddVoter("{head0}", RULE_POT, leaderId);
//!   v.Sway(gm, playerId, i, RULE_POT);
//!   int rule = v.Tally(chairLean);
class RZ_QuestVote
{
	protected RZ_QuestRun m_Run;
	protected string m_sKey;
	protected string m_sName;

	static const int MAX_OPTIONS = 8;

	//------------------------------------------------------------------------------------------------
	void RZ_QuestVote(RZ_QuestRun run, string name)
	{
		m_Run = run;
		m_sName = name;
		m_sKey = "vote." + name + ".";
	}

	//------------------------------------------------------------------------------------------------
	//! The options' words for dialogue's tallies: a conversation's `sway:<major key>.<vote>.<word>`
	//! (tally.<...> in the world facts) swings one voter to that option per contribution at the tally.
	void Words(notnull array<string> words)
	{
		foreach (int i, string w : words)
		{
			m_Run.SetText(m_sKey + "w" + i.ToString(), w);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Dialogue's sways (Dialogue > Trees on) since the last tally, applied once.
	protected void TakeDialogue()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (!facts)
			return;

		for (int o = 0; o < MAX_OPTIONS; o++)
		{
			string word = m_Run.Text(m_sKey + "w" + o.ToString());
			if (word.IsEmpty() || !IsTabled(o))
				continue;

			string key = string.Format("tally.%1.%2.%3", m_Run.m_sKey, m_sName, word);
			int n = facts.Flag(key);
			if (n <= 0)
				continue;

			Swing(-1, o, n);
			facts.SetFlag(key, 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A voter with a lean (an option), and the leader who votes (-1: a named person). Returns the
	//! voter's index.
	int AddVoter(string name, int lean, int leaderId = -1)
	{
		int i = Count();
		m_Run.SetFlag(m_sKey + "n", i + 1);
		m_Run.SetText(m_sKey + "v" + i.ToString(), name);
		m_Run.SetFlag(m_sKey + "v" + i.ToString() + ".lean", lean);
		m_Run.SetFlag(m_sKey + "v" + i.ToString() + ".id", leaderId);
		return i;
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_Run.Flag(m_sKey + "n");
	}

	//------------------------------------------------------------------------------------------------
	string VoterName(int i)
	{
		return m_Run.Text(m_sKey + "v" + i.ToString());
	}

	//------------------------------------------------------------------------------------------------
	int VoterLeader(int i)
	{
		return m_Run.Flag(m_sKey + "v" + i.ToString() + ".id", -1);
	}

	//------------------------------------------------------------------------------------------------
	int Lean(int i)
	{
		return m_Run.Flag(m_sKey + "v" + i.ToString() + ".lean", -1);
	}

	//------------------------------------------------------------------------------------------------
	void SetLean(int i, int option)
	{
		if (i >= 0 && i < Count())
			m_Run.SetFlag(m_sKey + "v" + i.ToString() + ".lean", option);
	}

	//------------------------------------------------------------------------------------------------
	//! Every voter leaning `from` (any option: -1) leans `to` now, at most `count` of them.
	void Swing(int from, int to, int count)
	{
		for (int i = 0; i < Count() && count > 0; i++)
		{
			int lean = Lean(i);
			if (lean == to || (from >= 0 && lean != from))
				continue;

			SetLean(i, to);
			count--;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! An option is on the table (two are tabled from the start; offers only when carried).
	void Table(int option)
	{
		m_Run.SetFlag(m_sKey + "t" + option.ToString(), 1);
	}

	//------------------------------------------------------------------------------------------------
	bool IsTabled(int option)
	{
		return m_Run.Flag(m_sKey + "t" + option.ToString()) == 1;
	}

	//------------------------------------------------------------------------------------------------
	bool HasSwayed(RZ_GameModeComponent gm, int playerId)
	{
		string ident = Ident(gm, playerId);
		return !ident.IsEmpty() && m_Run.Flag(m_sKey + "s." + ident) == 1;
	}

	//------------------------------------------------------------------------------------------------
	//! One sway per player per vote: voter `i` leans `option` now. False if this player has swayed
	//! this vote already, or the option isn't on the table.
	bool Sway(RZ_GameModeComponent gm, int playerId, int i, int option)
	{
		string ident = Ident(gm, playerId);
		if (ident.IsEmpty() || i < 0 || i >= Count() || !IsTabled(option) || m_Run.Flag(m_sKey + "s." + ident) == 1)
			return false;

		m_Run.SetFlag(m_sKey + "s." + ident, 1);
		SetLean(i, option);
		RememberedErrands(gm, playerId, ident, i, option);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Missions > Errands > Majors read errand memory (off by default): a head whose house remembers
	//! this player's errands (a thread there, RZ_Errands.ThreadLineFor) brings one more voter round
	//! to the same option, once per head per season (the run's flags last the season).
	protected void RememberedErrands(RZ_GameModeComponent gm, int playerId, string ident, int i, int option)
	{
		if (!RZ_ErrandSettings.Get().m_bMajorsReadMemory || !gm.GetErrands() || !gm.GetLeaders())
			return;

		RZ_Leader head = gm.GetLeaders().Get(VoterLeader(i));
		if (!head || head.m_iSafehouse < 0)
			return;

		string key = string.Format("errmem.%1.%2", ident, head.m_iId);
		if (m_Run.Flag(key) == 1 || gm.GetErrands().ThreadLineFor(playerId, -1, head.m_iSafehouse).IsEmpty())
			return;

		m_Run.SetFlag(key, 1);
		Swing(-1, option, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Ident(RZ_GameModeComponent gm, int playerId)
	{
		if (!gm || playerId <= 0)
			return "";

		return gm.GetProfile(playerId).m_sIdentity;
	}

	//------------------------------------------------------------------------------------------------
	//! Voters leaning `option` (a lean to an option that isn't tabled counts for nobody).
	int Votes(int option)
	{
		if (!IsTabled(option))
			return 0;

		int n = 0;
		for (int i = 0; i < Count(); i++)
		{
			if (Lean(i) == option)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The winning option; a tie goes to `tieBreak` if it's among the tied, else the lowest tied
	//! option. Kept as the vote's result.
	int Tally(int tieBreak)
	{
		TakeDialogue();
		int best = -1;
		int bestVotes = -1;
		bool tieHasBreak = false;
		for (int o = 0; o < MAX_OPTIONS; o++)
		{
			if (!IsTabled(o))
				continue;

			int v = Votes(o);
			if (v > bestVotes)
			{
				best = o;
				bestVotes = v;
				tieHasBreak = o == tieBreak;
			}
			else if (v == bestVotes && o == tieBreak)
			{
				tieHasBreak = true;
			}
		}

		if (tieHasBreak)
			best = tieBreak;

		m_Run.SetFlag(m_sKey + "result", best);
		return best;
	}

	//------------------------------------------------------------------------------------------------
	int Result()
	{
		return m_Run.Flag(m_sKey + "result", -1);
	}

	//------------------------------------------------------------------------------------------------
	//! "Mara Novak leans to the shared pot" lines, one per voter, `names` giving each option's words.
	string LeanLines(notnull array<string> names)
	{
		string text = "";
		for (int i = 0; i < Count(); i++)
		{
			int lean = Lean(i);
			string what = "nothing yet";
			if (lean >= 0 && lean < names.Count())
				what = names[lean];

			if (!text.IsEmpty())
				text += "\n";

			text += string.Format("%1 leans to %2.", m_Run.Fill(VoterName(i)), what);
		}

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! One major's ending in the season's story (last_season.json, the epilogue line).
class RZ_StoryRecord
{
	int faction = -1;
	string major;
	int ending = -1;
	int branch = -1;
	string line;				// the epilogue line, by who controls the cure when the cure comes
	bool fallback;				// reached by defaults only (no next-season rule)
	int atCure = -1;			// the ending as it stood at the cure (the story rules read it), -1 none
	ref array<string> deciders = {};	// identities
	ref array<string> crews = {};	// the book: "identity|crew name" of its deciders (next season's greeting)
}

//------------------------------------------------------------------------------------------------
class RZ_MajorsSave
{
	ref array<int> dayKeys = {};		// real days (unix / 86400) with their hours played
	ref array<float> dayHours = {};
	ref array<int> researchDays = {};	// worked research at the end of each real day with play
	ref array<float> researchAt = {};
	ref array<string> payKeys = {};		// the majors' pay caps: "ident|faction|major|kind"
	ref array<int> payValues = {};
	ref array<ref RZ_StoryRecord> stories = {};
	ref array<string> told = {};		// as-data news still to tell, one a real day
	int toldDay;
	bool gateDone;
	bool wipeDone;
}

//------------------------------------------------------------------------------------------------
class RZ_Majors
{
	static const float SESSION = 7200;		// 2 real hours with players online, in clock seconds
	static const int PAY_FAME_MAJOR = 25;
	static const int PAY_INFAMY_MAJOR = 40;
	static const int PAY_FAVOR_MAJOR = 60;
	static const int PAY_FAME_SEASON = 40;
	static const int PAY_INFAMY_SEASON = 40;
	static const int PAY_FAVOR_SEASON = 80;
	static const float NO_ROOM = 100000;
	static const int DAY = 86400;

	protected static RZ_Majors s_Instance;
	protected static string s_sActing;		// the major acting now ("" = none): its pay and world effects are capped
	protected static bool s_bNoPay;			// a default is running: it pays nobody
	protected static float s_fEffect = 1;	// 0.5 at a hard stop

	protected RZ_GameModeComponent m_GM;
	protected RZ_Questlines m_Q;
	protected RZ_MajorSettings m_Settings;
	protected ref array<int> m_aDayKeys = {};
	protected ref array<float> m_aDayHours = {};
	protected ref array<int> m_aResDays = {};
	protected ref array<float> m_aResAt = {};
	protected ref map<string, int> m_mPay = new map<string, int>();
	protected ref array<ref RZ_StoryRecord> m_aStories = {};
	protected ref array<string> m_aTold = {};
	protected int m_iToldDay;
	protected bool m_bGateDone;
	protected bool m_bWipeDone;
	protected ref array<float> m_aBetweenUntil = {0, 0, 0, 0};

	//------------------------------------------------------------------------------------------------
	void RZ_Majors(RZ_GameModeComponent gm, RZ_Questlines q, RZ_MajorSettings settings)
	{
		m_GM = gm;
		m_Q = q;
		m_Settings = settings;
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Majors Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! The majors run: Majors on and Questlines on.
	static bool IsOn()
	{
		return s_Instance && s_Instance.m_Settings && s_Instance.m_Settings.m_bEnabled && s_Instance.m_Q && s_Instance.m_Q.IsEnabled();
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorSettings GetSettings()
	{
		return m_Settings;
	}

	// ================================================================ The acting context
	//------------------------------------------------------------------------------------------------
	//! The engine calls into a major (its Next, endings, defaults, a stage mission's reward): its
	//! world effects draw on the majors lane, and its pay on the majors' caps.
	static void Enter(string key, bool noPay = false, float effect = 1)
	{
		s_sActing = key;
		s_bNoPay = noPay;
		s_fEffect = effect;
	}

	//------------------------------------------------------------------------------------------------
	static void Leave()
	{
		s_sActing = "";
		s_bNoPay = false;
		s_fEffect = 1;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsNoPay()
	{
		return s_bNoPay;
	}

	//------------------------------------------------------------------------------------------------
	static string Acting()
	{
		return s_sActing;
	}

	//------------------------------------------------------------------------------------------------
	//! A morale change from a questline: inside a major, at the hard stop's effect and through the
	//! season budget's majors lane (anything clipped is logged and becomes words only).
	static float MoraleThroughBudget(int faction, float delta)
	{
		if (s_sActing.IsEmpty())
			return delta;

		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (!budget)
			return delta * s_fEffect;

		return budget.TakeMorale(RZ_EBudgetLane.MAJORS, faction, delta * s_fEffect, s_sActing, "major " + s_sActing);
	}

	//------------------------------------------------------------------------------------------------
	static float StrengthThroughBudget(int faction, float delta)
	{
		if (s_sActing.IsEmpty())
			return delta;

		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (!budget)
			return delta * s_fEffect;

		return budget.TakeStrength(RZ_EBudgetLane.MAJORS, faction, delta * s_fEffect, s_sActing, "major " + s_sActing);
	}

	//------------------------------------------------------------------------------------------------
	//! Pay from a major, inside its caps: per player per faction 25 Fame, 40 Infamy and 60 Favor over
	//! one major, and 40, 40 and 80 across every major this season. kind "fame", "infamy" or
	//! "favor". Losses (negative Fame) aren't capped. Outside a major: `amount` unchanged.
	static int ClipPay(string ident, int faction, string kind, int amount)
	{
		if (s_bNoPay)
			return 0;

		if (s_sActing.IsEmpty() || !s_Instance || amount <= 0 || ident.IsEmpty())
			return amount;

		int perMajor = PAY_FAME_MAJOR;
		int perSeason = PAY_FAME_SEASON;
		if (kind == "infamy")
		{
			perMajor = PAY_INFAMY_MAJOR;
			perSeason = PAY_INFAMY_SEASON;
		}
		else if (kind == "favor")
		{
			perMajor = PAY_FAVOR_MAJOR;
			perSeason = PAY_FAVOR_SEASON;
		}

		string mk = string.Format("%1|%2|%3|%4", ident, faction, s_sActing, kind);
		string sk = string.Format("%1|%2|all|%3", ident, faction, kind);
		int usedMajor;
		int usedSeason;
		s_Instance.m_mPay.Find(mk, usedMajor);
		s_Instance.m_mPay.Find(sk, usedSeason);
		int allowed = Math.MinInt(amount, Math.MinInt(perMajor - usedMajor, perSeason - usedSeason));
		allowed = Math.MaxInt(0, allowed);
		s_Instance.m_mPay.Set(mk, usedMajor + allowed);
		s_Instance.m_mPay.Set(sk, usedSeason + allowed);
		if (allowed < amount)
			RZ_Log.Info(string.Format("Majors: %1 %2 for one player clipped from %3 to %4 (the majors' caps)", s_sActing, kind, amount, allowed));

		return allowed;
	}

	// ================================================================ Acts
	//------------------------------------------------------------------------------------------------
	//! An act's result inside a major (RZ_Questline.ActEnd).
	static void ActEnd(RZ_GameModeComponent gm, RZ_QuestRun run, int act, int outcome, string line)
	{
		if (!gm || !run || !run.m_Line)
			return;

		RZ_Questlines q = gm.GetQuestlines();
		q.SetWorldFlag(string.Format("%1.act%2", run.m_sKey, act), outcome);
		RZ_MajorAct a = run.m_Line.GetAct(act);
		if (a && a.m_bClimax)
			q.SetWorldFlag(run.m_sKey + ".climax", outcome);

		if (!line.IsEmpty())
		{
			run.Note(line);
			RZ_Questline.TellParticipants(gm, run, run.m_Line.m_sTitle, run.Fill(line));
		}

		MarkBeat(run);
		string name = "?";
		if (a)
			name = a.m_sName;

		RZ_Log.Info(string.Format("Majors: %1 act %2 (%3) settled, result %4.", run.m_sKey, act, name, outcome));
	}

	//------------------------------------------------------------------------------------------------
	//! A beat: an act opened or ended, a set piece, a default. The beat gap counts from here.
	static void MarkBeat(RZ_QuestRun run)
	{
		if (run && run.m_Owner)
			run.SetFlag("mc.beat", RZ_Util.RoundToInt(run.m_Owner.GetClock()));
	}

	//------------------------------------------------------------------------------------------------
	//! Hours of play since the major's last beat.
	static float BeatGapHours(RZ_QuestRun run)
	{
		if (!run || !run.m_Owner)
			return 0;

		return (run.m_Owner.GetClock() - run.Flag("mc.beat")) / 3600;
	}

	//------------------------------------------------------------------------------------------------
	//! The climax of the major `key` has resolved (or that major isn't running this season and never
	//! began: the order never waits on a story that isn't there).
	static bool ClimaxDone(string key)
	{
		if (!s_Instance)
			return true;

		RZ_Questlines q = s_Instance.m_Q;
		if (q.HasWorldFlag(key + ".climax") || q.HasEnded(key))
			return true;

		return !q.IsRunning(key);
	}

	//------------------------------------------------------------------------------------------------
	//! The cure has reached `percent`, or may count as reached (a lead): research has stalled, the
	//! climax is within Lead points, the major's beat gap is at least 3 sessions, and a player who
	//! qualifies for the act is online. The caller checks the climax order and the acts before.
	//! Never past the gate, and never moves research.
	static bool Reached(RZ_QuestRun run, float percent)
	{
		float progress = Progress();
		if (progress >= percent)
			return true;

		if (!s_Instance || !run || percent > 90)
			return false;

		float lead = s_Instance.m_Settings.m_fLeadPoints;
		if (lead <= 0 || progress < percent - lead || !s_Instance.IsStalled())
			return false;

		if (BeatGapHours(run) < 3 * SESSION / 3600 || run.Flag("mc.qual") != 1)
			return false;

		if (run.Flag("mc.led") != RZ_Util.RoundToInt(percent))
		{
			run.SetFlag("mc.led", RZ_Util.RoundToInt(percent));
			RZ_Log.Info(string.Format("Majors: %1 leads at %2 percent toward %3 (research stalled 3 days).", run.m_sKey, RZ_Util.RoundToInt(progress), RZ_Util.RoundToInt(percent)));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Worked research, percent.
	static float Progress()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetCure())
			return 0;

		return gm.GetCure().m_fProgress;
	}

	//------------------------------------------------------------------------------------------------
	//! The gate: research at 90 percent (or the cure out). The majors finish before it.
	static bool AtGate()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetCure())
			return false;

		RZ_CureResearch cure = gm.GetCure();
		return cure.IsCured() || cure.IsFinalStand() || cure.m_fProgress >= 90;
	}

	//------------------------------------------------------------------------------------------------
	//! Hours until the wipe comes due (a large number if none is set).
	static float WipeDueInHours()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetSeasons())
			return NO_ROOM;

		int at = gm.GetSeasons().WipeAt();
		int now = RZ_SeasonClock.Now();
		if (at <= 0 || now <= 0)
			return NO_ROOM;

		float secs = at - now;
		return secs / 3600;
	}

	// ================================================================ Signals
	//------------------------------------------------------------------------------------------------
	//! Hours with someone online a real day, averaged over the last 7 real days (today included once
	//! it's the only one). 7.7 (the design point) before any day is counted.
	float HoursPerDay()
	{
		int today = Today();
		float sum = 0;
		int days = 0;
		foreach (int i, int day : m_aDayKeys)
		{
			if (day >= today - 7 && day < today)
			{
				sum += m_aDayHours[i];
				days++;
			}
		}

		if (days == 0)
		{
			int idx = m_aDayKeys.Find(today);
			if (idx >= 0 && m_aDayHours[idx] > 0)
				return Math.Max(1, m_aDayHours[idx]);

			return 7.7;
		}

		return Math.Max(0.5, sum / days);
	}

	//------------------------------------------------------------------------------------------------
	//! Worked research a real day, over the last 3 real days that had play. 0 before two days are
	//! counted.
	float ResearchPace()
	{
		int n = m_aResAt.Count();
		if (n < 1)
			return 0;

		int from = Math.MaxInt(0, n - 3);
		float gained = Progress() - m_aResAt[from];
		int days = n - from;
		if (days <= 0)
			return 0;

		return Math.Max(0, gained / days);
	}

	//------------------------------------------------------------------------------------------------
	//! Research rose less than Stall points over the last 3 real days with play, and the lab's
	//! backlog is empty.
	bool IsStalled()
	{
		if (m_aResAt.Count() < 3)
			return false;

		float gained = Progress() - m_aResAt[m_aResAt.Count() - 3];
		if (gained >= m_Settings.m_fStallPoints)
			return false;

		RZ_CureResearch cure = m_GM.GetCure();
		return !cure || cure.GetBacklog() <= 0.01;
	}

	//------------------------------------------------------------------------------------------------
	//! Hours of play left before `act`'s anchor (0 once it's there; NO_ROOM while research stalls).
	float RoomHours(RZ_MajorAct act)
	{
		if (!act)
			return NO_ROOM;

		switch (act.m_eAnchor)
		{
			case RZ_EMajorAnchor.CURE: return RoomToCure(act.m_fAnchor);
			case RZ_EMajorAnchor.GATE: return RoomToCure(90);
			case RZ_EMajorAnchor.STAGE:
			{
				if (!m_GM.GetDirector())
					return NO_ROOM;

				return m_GM.GetDirector().MinutesToStage(RZ_Util.RoundToInt(act.m_fAnchor)) / 60;
			}
			case RZ_EMajorAnchor.WIPE: return Math.Max(0, WipeDueInHours() - 24);
		}

		return NO_ROOM;
	}

	//------------------------------------------------------------------------------------------------
	protected float RoomToCure(float percent)
	{
		float progress = Progress();
		if (progress >= percent)
			return 0;

		float pace = ResearchPace();
		if (pace <= 0.05)
			return NO_ROOM;

		return (percent - progress) / pace * HoursPerDay();
	}

	//------------------------------------------------------------------------------------------------
	//! Hours of play the act and the acts after it with the same anchor still need.
	float NeedHours(RZ_Questline line, RZ_MajorAct act)
	{
		float sessions = 0;
		bool from = false;
		for (int i = 0; i < line.ActCount(); i++)
		{
			RZ_MajorAct a = line.ActAt(i);
			if (a == act)
				from = true;

			if (!from)
				continue;

			if (a.m_eAnchor != act.m_eAnchor || a.m_fAnchor != act.m_fAnchor)
				break;

			sessions += a.m_fNeed;
		}

		return sessions * SESSION / 3600;
	}

	//------------------------------------------------------------------------------------------------
	protected static int Today()
	{
		int now = RZ_SeasonClock.Now();
		if (now <= 0)
			return 0;

		return now / DAY;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! Every questline check (10 s) with players online.
	void Tick(float dt, array<int> ids)
	{
		if (!IsOn() || ids.IsEmpty())
			return;

		TrackDay(dt);
		TellQueued();
		HardStops();
		RZ_MajorRest.Get().Tick(dt, ids);
		array<ref RZ_QuestRun> runs = m_Q.GetRuns();
		for (int i = runs.Count() - 1; i >= 0; i--)
		{
			if (i >= runs.Count())
				continue;

			RZ_QuestRun run = runs[i];
			if (!run.m_bDone && run.m_Line && run.m_Line.IsMajor())
				TickRun(run, dt, ids);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Hours played today; at a new real day, the research reading for the day before (if it had play).
	protected void TrackDay(float dt)
	{
		int today = Today();
		if (today <= 0)
			return;

		int idx = m_aDayKeys.Find(today);
		if (idx < 0)
		{
			// A new day: the last day with play gets its research reading
			if (!m_aDayKeys.IsEmpty() && m_aDayHours[m_aDayHours.Count() - 1] > 0)
			{
				m_aResDays.Insert(m_aDayKeys[m_aDayKeys.Count() - 1]);
				m_aResAt.Insert(Progress());
				while (m_aResAt.Count() > 4)
				{
					m_aResAt.RemoveOrdered(0);
					m_aResDays.RemoveOrdered(0);
				}
			}

			m_aDayKeys.Insert(today);
			m_aDayHours.Insert(0);
			while (m_aDayKeys.Count() > 8)
			{
				m_aDayKeys.RemoveOrdered(0);
				m_aDayHours.RemoveOrdered(0);
			}

			idx = m_aDayKeys.Count() - 1;
		}

		m_aDayHours[idx] = m_aDayHours[idx] + dt / 3600;
	}

	//------------------------------------------------------------------------------------------------
	//! Defaults decided together (as data) are told one headline a real day, in order.
	protected void TellQueued()
	{
		if (m_aTold.IsEmpty())
			return;

		int today = Today();
		if (today == m_iToldDay)
			return;

		m_iToldDay = today;
		RZ_Questline.Headline(m_GM, m_aTold[0], true);
		m_aTold.RemoveOrdered(0);
	}

	//------------------------------------------------------------------------------------------------
	//! A headline for a default that ran as data: at once if nothing else was told today, else queued.
	void TellDefault(string text)
	{
		if (text.IsEmpty())
			return;

		int today = Today();
		if (m_aTold.IsEmpty() && today != m_iToldDay)
		{
			m_iToldDay = today;
			RZ_Questline.Headline(m_GM, text, true);
			return;
		}

		m_aTold.Insert(text);
	}

	//------------------------------------------------------------------------------------------------
	//! The gate (90 percent) and the wipe's last 24 hours with no cure: every act still open that
	//! doesn't run past the gate resolves on its branch at half world effect, paying nobody.
	protected void HardStops()
	{
		bool gate = AtGate();
		bool wipe = !m_GM.GetCure() || !m_GM.GetCure().IsCured();
		wipe = wipe && WipeDueInHours() < 24;
		if ((!gate || m_bGateDone) && (!wipe || m_bWipeDone))
			return;

		if (gate)
			m_bGateDone = true;

		if (wipe)
			m_bWipeDone = true;

		array<ref RZ_QuestRun> runs = {};
		foreach (RZ_QuestRun r : m_Q.GetRuns())
		{
			runs.Insert(r);
		}

		foreach (RZ_QuestRun run : runs)
		{
			if (run.m_bDone || !run.m_Line || !run.m_Line.IsMajor())
				continue;

			for (int guard = 0; guard < 8 && !run.m_bDone; guard++)
			{
				RZ_MajorAct act = run.m_Line.ActOf(run.m_iStage);
				if (!act || (act.m_bPastGate && !wipe))
					break;

				RZ_Log.Info(string.Format("Majors: %1 act %2 resolves at the hard stop, at half effect.", run.m_sKey, act.m_iId));
				FireDefault(run, act, "hard stop", 0.5);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One major's clock on one tick.
	protected void TickRun(RZ_QuestRun run, float dt, array<int> ids)
	{
		RZ_Questline line = run.m_Line;
		RZ_MajorAct act = line.ActOf(run.m_iStage);
		if (!act)
			return;

		float clock = m_Q.GetClock();
		if (run.Flag("mc.act", -1) != act.m_iId)
			NewAct(run, act);

		// A crisis is its own beat: the clocks pause (the hard stops still apply)
		if (UnderPressure(line.m_iFaction))
			return;

		bool qualOnline = AnyQualifies(run, act, ids);
		run.SetFlag("mc.qual", Bit(qualOnline));

		// An act that waits for its trigger (a milestone, the act before) isn't open yet: no clock,
		// but the gap before it may be filled (between beats)
		RZ_QuestStage st = line.GetStage(run.m_iStage);
		if (!st || (!st.m_bMission && !act.m_bWatch))
		{
			if (act.m_fOpens > 0)
				BetweenBeats(run, act, RoomToCure(act.m_fOpens));

			return;
		}

		// Offered only while the stage mission is on the offer list (one of the Slots), not while the
		// stage is merely open; a watched inner line counts while it runs
		bool open = run.m_iMissionId >= 0 || act.m_bWatch;
		bool window = InWindow(act);

		// Offered time, and offered evenings for set pieces
		if (open && qualOnline && window)
		{
			AddFlag(run, "mc.offered", dt);
			if (act.m_bSetPiece)
				CountEvening(run, dt);
		}

		if (qualOnline)
			run.SetFlag("mc.noq", 0);
		else
			AddFlag(run, "mc.noq", dt);

		// Nobody qualifies (set pieces): free evenings pass with players online but nobody who could
		if (act.m_bSetPiece && open && !qualOnline && RZ_SetPieceCalendar.Get())
		{
			int eve = RZ_SetPieceCalendar.Get().Evening();
			if (eve != run.Flag("mc.noqEve", -1) && RZ_WorldFacts.Get().IsDusk())
			{
				run.SetFlag("mc.noqEve", eve);
				run.SetFlag("mc.noqEves", run.Flag("mc.noqEves") + 1);
			}
		}

		if (qualOnline && act.m_bSetPiece)
			run.SetFlag("mc.noqEves", 0);

		float room = RoomHours(act);
		float need = NeedHours(line, act);
		bool squeezed = room < need;
		run.SetFlag("mc.squeezed", Bit(squeezed));

		string why = "";
		if (squeezed)
		{
			if (run.Flag("mc.call") != 1)
			{
				LastCall(run, act, string.Format("squeezed (room %1 h, need %2 h)", RZ_Util.RoundToInt(room * 10) / 10.0, RZ_Util.RoundToInt(need * 10) / 10.0));
				return;
			}

			if (room <= 0 || GraceOver(run, act))
				why = "squeezed";
		}
		else if (PatienceNearlyOut(run, act))
		{
			if (run.Flag("mc.call") != 1)
			{
				LastCall(run, act, "patience");
				return;
			}

			if (GraceOver(run, act))
				why = "patience";
		}
		else if (act.m_bSetPiece && run.Flag("mc.noqEves") >= 2)
		{
			why = "nobody qualifies";
		}

		if (why.IsEmpty())
		{
			run.SetFlag("mc.held", 0);
			return;
		}

		// The guards: an engaged act holds (once), the breather, the low ebb
		if (IsEngaged(run))
		{
			AddFlag(run, "mc.held", dt);
			if (run.Flag("mc.held") < m_Settings.m_fHoldLimit * SESSION)
				return;

			RZ_Log.Info(string.Format("Majors: %1 act %2 held its anchor as long as it may; the default comes anyway.", run.m_sKey, act.m_iId));
		}

		if (!squeezed && run.HasFlag("mc.lastDefault") && clock - run.Flag("mc.lastDefault") < SESSION)
			return;

		if (why == "patience" && act.m_bCostsMorale && RZ_Questline.GetMorale(m_GM, line.m_iFaction) < 30)
			return;

		FireDefault(run, act, why, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void NewAct(RZ_QuestRun run, RZ_MajorAct act)
	{
		run.SetFlag("mc.act", act.m_iId);
		array<string> keys = {"mc.offered", "mc.evenings", "mc.eveSecs", "mc.lastEve", "mc.call", "mc.callOffered", "mc.callClock", "mc.callEvenings", "mc.held", "mc.noqEves", "mc.noqEve", "mc.led"};
		foreach (string k : keys)
		{
			run.m_mFlags.Remove(k);
		}

		MarkBeat(run);
		RZ_Log.Info(string.Format("Majors: %1 act %2 (%3) opens.", run.m_sKey, act.m_iId, act.m_sName));
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddFlag(RZ_QuestRun run, string key, float seconds)
	{
		run.SetFlag(key, run.Flag(key) + RZ_Util.RoundToInt(seconds));
	}

	//------------------------------------------------------------------------------------------------
	static int Bit(bool on)
	{
		if (on)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A set piece's evening counts as offered once it has had 30 real minutes of offered time.
	protected void CountEvening(RZ_QuestRun run, float dt)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		if (!cal)
			return;

		int eve = cal.Evening();
		if (eve != run.Flag("mc.lastEve", -1))
		{
			run.SetFlag("mc.lastEve", eve);
			run.SetFlag("mc.eveSecs", 0);
		}

		int before = run.Flag("mc.eveSecs");
		AddFlag(run, "mc.eveSecs", dt);
		if (before < 1800 && run.Flag("mc.eveSecs") >= 1800)
			run.SetFlag("mc.evenings", run.Flag("mc.evenings") + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Inside a set piece's offer window (any time for other acts, and while the clock can't be read).
	static bool InWindow(RZ_MajorAct act)
	{
		if (!act.m_bSetPiece || !RZ_GameTime.IsCalibrated())
			return true;

		float h = RZ_GameTime.GetHour();
		return h >= act.m_fFrom && h < act.m_fTo;
	}

	//------------------------------------------------------------------------------------------------
	protected bool AnyQualifies(RZ_QuestRun run, RZ_MajorAct act, array<int> ids)
	{
		foreach (int playerId : ids)
		{
			if (run.m_Line.ActQualifies(m_GM, run, act, playerId))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool UnderPressure(int faction)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		return territory && territory.IsActive() && RZ_Factions.IsValid(faction) && territory.IsUnderPressure(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! A stage of the act is running with someone in it, or the act was touched in the last session.
	static bool IsEngaged(RZ_QuestRun run)
	{
		if (!run || !run.m_Owner)
			return false;

		if (run.HasFlag("mc.touch") && run.m_Owner.GetClock() - run.Flag("mc.touch") < SESSION)
			return true;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || run.m_iMissionId < 0)
			return false;

		RZ_Mission m = gm.GetMissions().FindById(run.m_iMissionId);
		return m && !m.m_aParticipants.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Patience is down to its last offered session (or evening).
	protected bool PatienceNearlyOut(RZ_QuestRun run, RZ_MajorAct act)
	{
		if (act.m_iPatienceEvenings > 0 && run.Flag("mc.evenings") >= act.m_iPatienceEvenings - 1)
			return true;

		return act.m_fPatience > 0 && run.Flag("mc.offered") >= (act.m_fPatience - 1) * SESSION;
	}

	//------------------------------------------------------------------------------------------------
	//! The last call: the act's own line (a headline and a rumour), then a grace.
	protected void LastCall(RZ_QuestRun run, RZ_MajorAct act, string why)
	{
		run.SetFlag("mc.call", 1);
		run.SetFlag("mc.callOffered", run.Flag("mc.offered"));
		run.SetFlag("mc.callClock", RZ_Util.RoundToInt(m_Q.GetClock()));
		run.SetFlag("mc.callEvenings", run.Flag("mc.evenings"));
		RZ_Log.Info(string.Format("Majors: %1 act %2 %3. Last call.", run.m_sKey, act.m_iId, why));
		if (act.m_sLastCall.IsEmpty())
			return;

		string line = run.Fill(act.m_sLastCall);
		run.SetText("mc.callLine", line);
		RZ_Questline.Headline(m_GM, line, true);
		RZ_Questline.SpreadRumour(m_GM, line, 8);
	}

	//------------------------------------------------------------------------------------------------
	//! The grace after a last call: a session of offered time (the next offered evening for a set
	//! piece). With nobody who qualifies on for two sessions, it counts plain hours played instead.
	protected bool GraceOver(RZ_QuestRun run, RZ_MajorAct act)
	{
		float grace = m_Settings.m_fGraceSessions * SESSION;
		if (act.m_bSetPiece && run.Flag("mc.evenings") > run.Flag("mc.callEvenings"))
			return true;

		if (run.Flag("mc.offered") - run.Flag("mc.callOffered") >= grace)
			return true;

		if (run.Flag("mc.noq") >= 2 * SESSION && m_Q.GetClock() - run.Flag("mc.callClock") >= grace)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The act's default fires: it pays nobody; `effect` 0.5 at a hard stop.
	void FireDefault(RZ_QuestRun run, RZ_MajorAct act, string why, float effect)
	{
		RZ_Log.Info(string.Format("Majors: %1 act %2 (%3) takes its default (%4).", run.m_sKey, act.m_iId, act.m_sName, why));
		Enter(run.m_sKey, true, effect);
		int next = run.m_Line.ActDefault(m_GM, run, act);
		Leave();
		run.SetFlag("mc.lastDefault", RZ_Util.RoundToInt(m_Q.GetClock()));
		run.SetFlag("mc.defaults", run.Flag("mc.defaults") + 1);
		MarkBeat(run);
		m_Q.JumpTo(run, next);
	}

	//------------------------------------------------------------------------------------------------
	//! Between beats: a long gap while the next act waits on a milestone far off. The major's
	//! faction's smaller stories are offered first (and twice as often) and one of its people says a
	//! line about what's coming, at most once a real day. Never opens an act, pays nobody, moves no vote.
	protected void BetweenBeats(RZ_QuestRun run, RZ_MajorAct act, float room)
	{
		int f = run.m_Line.m_iFaction;
		if (!RZ_Factions.IsValid(f))
			return;

		float gap = BeatGapHours(run);
		if (gap < m_Settings.m_fBetweenBeats * SESSION / 3600 || room < 6 * SESSION / 3600)
			return;

		m_aBetweenUntil[f] = m_Q.GetClock() + 600;
		int today = Today();
		if (run.Flag("mc.betweenDay") == today)
			return;

		run.SetFlag("mc.betweenDay", today);
		string line = run.m_Line.BetweenLine(m_GM, run);
		RZ_Log.Info(string.Format("Majors: %1 between beats (gap %2 h).", run.m_sKey, RZ_Util.RoundToInt(gap)));
		if (!line.IsEmpty())
			RZ_Questline.SpreadRumour(m_GM, run.Fill(line), 8);
	}

	//------------------------------------------------------------------------------------------------
	//! A faction whose major is between beats: its smaller stories go first.
	bool IsBetween(int faction)
	{
		if (!RZ_Factions.IsValid(faction))
			return false;

		return m_aBetweenUntil[faction] > m_Q.GetClock();
	}

	//------------------------------------------------------------------------------------------------
	bool AnyBetween()
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (IsBetween(f))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! May this major's stage mission be offered now? At most Slots of them at once; a squeezed act
	//! takes a slot from one that isn't (and isn't engaged).
	bool MayOffer(RZ_QuestRun run)
	{
		int slots = Math.MaxInt(1, m_Settings.m_iSlots);
		int used = 0;
		RZ_QuestRun yieldable;
		foreach (RZ_QuestRun other : m_Q.GetRuns())
		{
			if (other == run || other.m_bDone || !other.m_Line || !other.m_Line.IsMajor() || other.m_iMissionId < 0)
				continue;

			used++;
			if (other.Flag("mc.squeezed") != 1 && !IsEngaged(other))
				yieldable = other;
		}

		if (used < slots)
			return true;

		if (run.Flag("mc.squeezed") != 1 || !yieldable)
			return false;

		RZ_Log.Info(string.Format("Majors: %1 is squeezed and takes %2's slot.", run.m_sKey, yieldable.m_sKey));
		m_Q.Withdraw(yieldable);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The journal's season view for a major: the act, the rule or stance, the leans, any last call.
	string SeasonText(RZ_QuestRun run)
	{
		RZ_MajorAct act = run.m_Line.ActOf(run.m_iStage);
		string text = "";
		if (act)
			text = string.Format("The season: %1.", act.m_sName);

		string state = run.m_Line.SeasonState(m_GM, run);
		if (!state.IsEmpty())
			text += "\n" + run.Fill(state);

		if (run.Flag("mc.call") == 1 && !run.Text("mc.callLine").IsEmpty())
			text += "\nLast call: " + run.Text("mc.callLine");

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The notebook's season tab: each major as it stands (its act, the rule or stance, any last
	//! call), then each ending told.
	void SeasonLines(notnull array<string> result)
	{
		if (!m_Q)
			return;

		array<string> running = {};
		foreach (RZ_QuestRun run : m_Q.GetRuns())
		{
			if (!run || run.m_bDone || !run.m_Line.IsMajor())
				continue;

			running.Insert(run.m_sKey);
			string title = run.m_Line.m_sTitle;
			title.ToUpper();
			result.Insert(title);
			string text = SeasonText(run);
			array<string> parts = {};
			text.Split("\n", parts, true);
			foreach (string part : parts)
			{
				result.Insert(part);
			}

			result.Insert("");
		}

		foreach (RZ_StoryRecord r : m_aStories)
		{
			if (running.Contains(r.major))
				continue;

			string ending = EndingName(r.major, r.ending);
			if (ending.IsEmpty())
				continue;

			result.Insert(string.Format("%1: %2.", RZ_Factions.GetName(r.faction), ending));
			if (!r.line.IsEmpty())
				result.Insert(r.line);

			result.Insert("");
		}
	}

	// ================================================================ The stand's inputs
	//------------------------------------------------------------------------------------------------
	//! A faction's group at the final stand that a major sends (the finale caps them at Groups at the
	//! lab, the finisher's faction first, and adds Halcyon's own guard): the Haven at the gate after
	//! The Whole of It (Territory > Haven guard); Bastion by stance (1 and 2 at the gate, 3 on the
	//! lab's road, 4 none); the Red Hand on the lab's road when its last page balanced or stood for
	//! nothing, unless Bastion holds that road.
	static bool SendsStandGroup(RZ_GameModeComponent gm, int faction)
	{
		int stance = RZ_MajorHelp.World(gm, "orders.stance");
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				RZ_Territory territory = gm.GetTerritory();
				if (territory && !territory.GetSettings().m_bHavenGuard)
					return false;

				return RZ_MajorHelp.World(gm, "proof.haven_gate") != 0;
			}
			case RZ_EFaction.MILITARY:
			{
				// HC3 Authors: Bastion's terms (agreed or bargained), or the CO was told of the Red
				// Hand's offer; never under Expended
				int authors = RZ_MajorHelp.World(gm, "authors.terms");
				if (authors == 1 || authors == 2 || RZ_MajorHelp.World(gm, "authors.road_post") == 1)
					return stance != 4;

				return stance >= 1 && stance <= 3;
			}
			case RZ_EFaction.BANDITS:
			{
				int last = RZ_MajorHelp.World(gm, "book.last");
				return (last == 1 || last == 2) && stance != 3;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The group stands on the lab's road (150-250 m out) rather than at the gate: Bastion's
	//! checkpoint under stance 3 or HC3's road post (without its gate terms), the Red Hand's crews
	//! by the last page.
	static bool OnLabRoad(RZ_GameModeComponent gm, int faction)
	{
		if (faction == RZ_EFaction.MILITARY)
		{
			int terms = RZ_MajorHelp.World(gm, "authors.terms");
			bool post = RZ_MajorHelp.World(gm, "authors.road_post") == 1 && terms != 1 && terms != 2;
			return RZ_MajorHelp.World(gm, "orders.stance") == 3 || post;
		}

		if (faction == RZ_EFaction.BANDITS)
			return SendsStandGroup(gm, faction);

		return false;
	}

	// ================================================================ The season story
	//------------------------------------------------------------------------------------------------
	//! A major's ending in the season's record: its line (the epilogue line as it stands), whoever
	//! decided it (identities, at most four) and whether it was reached by defaults only.
	static void SeasonStory(int faction, string major, int ending, int branch, string line, array<string> deciders, bool fallback)
	{
		if (!s_Instance)
			return;

		RZ_StoryRecord rec;
		foreach (RZ_StoryRecord r : s_Instance.m_aStories)
		{
			if (r.major == major)
				rec = r;
		}

		if (!rec)
		{
			rec = new RZ_StoryRecord();
			s_Instance.m_aStories.Insert(rec);
		}

		rec.faction = faction;
		rec.major = major;
		rec.ending = ending;
		rec.branch = branch;
		rec.line = line;
		rec.fallback = fallback;
		// The story rules read the ending as it stood at the cure: nothing after it carries over
		RZ_CureResearch cure = s_Instance.m_GM.GetCure();
		if (!cure || !cure.IsCured())
			rec.atCure = ending;

		rec.deciders.Clear();
		if (deciders)
		{
			foreach (string d : deciders)
			{
				if (rec.deciders.Count() < 4 && !rec.deciders.Contains(d))
					rec.deciders.Insert(d);
			}
		}

		// The book's deciders' crew names (RZ_StoryRules.Greet next season)
		if (!rec.crews)
			rec.crews = {};

		rec.crews.Clear();
		foreach (string ident : rec.deciders)
		{
			int pid = s_Instance.m_GM.FindPlayerByIdentity(ident);
			if (major != "book" || pid <= 0)
				continue;

			string crew = RZ_RepVoice.CrewName(s_Instance.m_GM.GetProfile(pid));
			if (!crew.IsEmpty())
				rec.crews.Insert(ident + "|" + crew);
		}

		RZ_Log.Info(string.Format("Majors: season story for %1: ending %2 (%3).", major, ending, line));
		RZ_MajorRest.AtEnding(s_Instance.m_GM, major, ending, rec.deciders);
	}

	//------------------------------------------------------------------------------------------------
	//! The season story's records (the epilogue reads them).
	array<ref RZ_StoryRecord> GetStories()
	{
		return m_aStories;
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue line: the controlling faction's major first, then one line from each other major.
	//! controller: the faction whose rules apply (-1 for the Independent's open, sold or kept-back
	//! terms); cured: a cure came out at all.
	string EpilogueLine(int controller, bool cured)
	{
		string text = "";
		foreach (RZ_StoryRecord first : m_aStories)
		{
			if (first.faction == controller)
				text = LineFor(first, controller, cured);
		}

		foreach (RZ_StoryRecord r : m_aStories)
		{
			string line = LineFor(r, controller, cured);
			if (r.faction == controller || line.IsEmpty())
				continue;

			if (!text.IsEmpty())
				text += " ";

			text += line;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A major's epilogue line by who controls the cure: its own faction (the "own" variant), another
	//! or nobody's (the Independent's: the line it wrote), no cure at all (the no-cure variant).
	static string LineFor(RZ_StoryRecord r, int controller, bool cured)
	{
		string variant = "";
		if (!cured)
			variant = Variant(r.major, r.ending, false);
		else if (r.faction == controller)
			variant = Variant(r.major, r.ending, true);

		if (!variant.IsEmpty())
			return variant;

		return r.line;
	}

	//------------------------------------------------------------------------------------------------
	//! The per-ending variants (docs/QUESTLINES.md, each ending's epilogue line). own: the major's
	//! faction controls the cure; else no cure. "" keeps the ending's own line. The own variants say
	//! nothing about how the cure is handed out: the finale's terms line says that.
	static string Variant(string major, int ending, bool own)
	{
		string key = major + ":" + ending.ToString();
		if (own)
		{
			switch (key)
			{
				case "book:10": return "The Red Hand held the cure, and the book balanced the same week. The boss closed it with both hands.";
				case "book:11": return "The Red Hand held the cure. Everyone in the Zone had already bought their own page back.";
				case "book:12": return "The Red Hand held the cure with nothing written down. Nobody could say who owed what.";
				case "book:13": return "The Red Hand held the cure. Bastion held the ledger with their names in it.";
				case "book:14": return "The Red Hand held the cure, and the book still didn't balance.";
				case "orders:10": return "\"Command, Bastion. Cure secured. Requesting relief. Over.\" A long hiss. \"Bastion out.\"";
				case "orders:11": return "Bastion held the cure. By then everyone knew what Bastion was, because Bastion had told them.";
				case "orders:12": return "Bastion held the cure, and signed for it as Command.";
				case "orders:13": return "Bastion held the cure. Nobody on the roster expected thanks.";
				case "every_name:10": return "The Haven held the cure. Every name on the List was read out, the dead ones too.";
				case "every_name:11": return "The Haven held the cure. Nobody opened the book.";
				case "every_name:12": return "The Haven held the cure, and the far house was still on the List.";
				case "every_name:13": return "The Haven held the cure. Bastion's cards were still on the nail by every door.";
				case "every_name:14": return "The Haven held the cure. Some houses had paid at the door before, and everyone remembered which.";
				case "proof:10": return "Halcyon held the cure, and the paper with it. Its first page starts three weeks before Day 1.";
				case "proof:11": return "Halcyon held the cure. The paper went out under Bastion's signature.";
				case "proof:12": return "Halcyon held the cure. The paper's first page says Day 1.";
				case "proof:14": return "Halcyon held the cure. The Red Hand had already sold the paper's first page, two tins a copy.";
			}

			return "";
		}

		switch (key)
		{
			case "book:10": return "The gates never opened. The book balanced anyway.";
			case "book:11": return "The gates never opened. Everyone's page is in somebody's stove.";
			case "book:12": return "The gates never opened. The Red Hand never wrote another line.";
			case "book:13": return "The gates never opened. Bastion still has the ledger.";
			case "book:14": return "The gates never opened. Somebody still has the page.";
			case "orders:10": return "The gates never opened. Bastion read the numbers to Command every night until the end. No reply.";
			case "orders:11": return "The gates never opened. Bastion had already told its men the truth.";
			case "orders:12": return "The gates never opened. Bastion answered for Command to the end.";
			case "orders:13": return "The gates never opened. Each night the CO was alone on the net.";
			case "every_name:10": return "The gates never opened. The Haven read every name anyway.";
			case "every_name:11": return "The gates never opened. Nobody opened the book.";
			case "every_name:12": return "The gates never opened. The far house was still last.";
			case "every_name:13": return "The gates never opened. Bastion's cards were still on the nail.";
			case "every_name:14": return "The gates never opened. The tins ran out first.";
			case "proof:13": return "The paper is in the lab's safe, signed and never sent.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A major's ending by name, for the hall's lines.
	static string EndingName(string major, int ending)
	{
		switch (major + ":" + ending.ToString())
		{
			case "book:10": return "The book balances";
			case "book:11": return "Sold by the page";
			case "book:12": return "Sent off loud";
			case "book:13": return "Bastion's file";
			case "book:14": return "The book doesn't balance";
			case "orders:10": return "The Call Stands";
			case "orders:11": return "All Stations";
			case "orders:12": return "Bastion, Command";
			case "orders:13": return "Expended";
			case "every_name:10": return "Every Name";
			case "every_name:11": return "The Closed Book";
			case "every_name:12": return "The Far House";
			case "every_name:13": return "Counted";
			case "every_name:14": return "Paid at the Door";
			case "proof:10": return "The Whole of It";
			case "proof:11": return "Under Seal";
			case "proof:12": return "Day 1";
			case "proof:13": return "Unsigned";
			case "proof:14": return "Two Tins a Copy";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The hall's "The season's stories": one line a major, with up to four names who decided it
	//! (under the names policy), by who controls the cure (-1: nobody's or none) and whether a cure
	//! came out at all.
	void HallLines(int controller, bool cured, notnull array<string> result)
	{
		RZ_Epilogue epi = RZ_Epilogue.Get();
		foreach (RZ_StoryRecord r : m_aStories)
		{
			string ending = EndingName(r.major, r.ending);
			if (ending.IsEmpty())
				continue;

			string names = "";
			foreach (int i, string ident : r.deciders)
			{
				string known = ident;
				if (epi)
					known = epi.KnownName(ident);

				string shown = RZ_EpiNames.Show(ident, known);
				if (i > 0 && i == r.deciders.Count() - 1)
					names += " and ";
				else if (i > 0)
					names += ", ";

				names += shown;
			}

			string text = string.Format("The season's stories: %1.", ending);
			if (names.IsEmpty() || r.fallback)
				text = string.Format("The season's stories: %1, and nobody decided it.", ending);
			else
				text = string.Format("The season's stories: %1, decided by %2.", ending, names);

			string line = LineFor(r, controller, cured);
			if (!line.IsEmpty())
				text += " " + line;

			result.Insert(text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Into last_season.json: each major's ending, branch, line and deciders, with or without a cure.
	void FillRecord(RZ_SeasonRecord record)
	{
		if (!record)
			return;

		foreach (RZ_StoryRecord r : m_aStories)
		{
			record.stories.Insert(r);
		}
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	RZ_MajorsSave Save()
	{
		RZ_MajorsSave data = new RZ_MajorsSave();
		data.dayKeys.Copy(m_aDayKeys);
		data.dayHours.Copy(m_aDayHours);
		data.researchDays.Copy(m_aResDays);
		data.researchAt.Copy(m_aResAt);
		foreach (string k, int v : m_mPay)
		{
			data.payKeys.Insert(k);
			data.payValues.Insert(v);
		}

		foreach (RZ_StoryRecord r : m_aStories)
		{
			data.stories.Insert(r);
		}

		data.told.Copy(m_aTold);
		data.toldDay = m_iToldDay;
		data.gateDone = m_bGateDone;
		data.wipeDone = m_bWipeDone;
		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_MajorsSave data)
	{
		if (!data)
			return;

		if (data.dayKeys && data.dayHours && data.dayKeys.Count() == data.dayHours.Count())
		{
			m_aDayKeys.Copy(data.dayKeys);
			m_aDayHours.Copy(data.dayHours);
		}

		if (data.researchDays && data.researchAt && data.researchDays.Count() == data.researchAt.Count())
		{
			m_aResDays.Copy(data.researchDays);
			m_aResAt.Copy(data.researchAt);
		}

		if (data.payKeys && data.payValues)
		{
			for (int i = 0; i < data.payKeys.Count() && i < data.payValues.Count(); i++)
			{
				m_mPay.Set(data.payKeys[i], data.payValues[i]);
			}
		}

		if (data.stories)
		{
			foreach (RZ_StoryRecord r : data.stories)
			{
				if (r)
					m_aStories.Insert(r);
			}
		}

		if (data.told)
			m_aTold.Copy(data.told);

		m_iToldDay = data.toldDay;
		m_bGateDone = data.gateDone;
		m_bWipeDone = data.wipeDone;
	}
}
