//! The majors' scenes (docs/DESIGN.md, "The majors"): one stage mission class for every new act of
//! the four majors, filled in by the major's BuildScene(). A scene is people at places (spawned only
//! when a player comes near, through RZ_QM_HalcyonScene), each with up to four choices. A choice is
//! data: who may take it (a rank, a persuade check, a bribe, a flag), what it does (a sway in the
//! act's vote, a flag, a side's standing, the stage's outcome) and the three-beat line it says.
//! A scene can also hold a place against the infected (a hold: stay within 40 m while the noise
//! brings waves) and close at the end of its window with its vote's tally or a timeout outcome.
//!
//! Dialogue is off by default (decision 13), so every sway, ask and persuade here is an action on a
//! person; votes stay tallies in RZ_QuestVote either way.

//------------------------------------------------------------------------------------------------
//! One choice on a scene's person. Built with RZ_QM_MajorScene.Choice() and chained.
class RZ_SceneChoice
{
	int m_iSpot;
	string m_sLabel;
	int m_iApproach = RZ_EQuestApproach.TALK;
	int m_iOutcome = -1;			// >= 0: decides the stage with this outcome
	bool m_bSuccess = true;			// false: the giver pays nothing (siding against them, betrayal)
	int m_iFaction = -1;			// whose standing the checks read (-1: the scene's giver)
	int m_iRank = -99;				// the access rank needed
	int m_iPersuade;				// the persuade difficulty (0: none)
	string m_sFavoured = "";		// backgrounds that count one rank higher for it
	int m_iBribe;					// Favor it costs
	int m_iBribeFaction = -1;
	int m_iVote = -1;				// sways this spot's voter to this option
	int m_iTable = -1;				// tables an option in the scene's vote
	int m_iSwingTo = -1;			// swings voters (any lean) to this option
	int m_iSwingCount;
	string m_sFlag = "";			// a run flag it sets ("world:key" for a world flag)
	int m_iFlagValue = 1;
	string m_sNeed = "";			// a run flag it needs ("world:key"; "!flag": must not be set)
	int m_iSideWith = -1;
	int m_iSideAgainst = -1;
	int m_iSideFame;
	int m_iSideInfamy;
	string m_sSays = "";			// what happens, in three beats
	string m_sRefused = "\"No.\"";
	string m_sWhat = "";			// for the journal: "spoke for the shared pot"
	bool m_bHold;					// starts the scene's hold
	vector m_vHoldAt;				// its own hold (HoldAt): where, and the outcomes
	float m_fHoldFor;
	int m_iHoldWin = -1;
	int m_iHoldLost = -1;
	bool m_bSneak;					// needs to be unseen (the scene's quiet watch)
	int m_iFear;					// a threat (Fear) at this difficulty (0: none), at m_iFearFaction's person
	int m_iFearFaction = -1;
	int m_iFearInfamy = 3;
	int m_iTrick;					// a trick (the look in m_iTrickFaction's kit) at this difficulty (0: none)
	int m_iTrickFaction = -1;
	RZ_EErrandItem m_eTradeItem;	// items handed over (none: no trade)
	int m_iTradeCount;

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Decides(int outcome, bool success = true)
	{
		m_iOutcome = outcome;
		m_bSuccess = success;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Way(int approach)
	{
		m_iApproach = approach;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Needs(int faction, int rank)
	{
		m_iFaction = faction;
		m_iRank = rank;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Persuade(int faction, int difficulty, string favoured)
	{
		m_iFaction = faction;
		m_iPersuade = Math.ClampInt(difficulty, 1, 4);
		m_sFavoured = favoured;
		m_iApproach = RZ_EQuestApproach.PERSUADE;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Bribe(int faction, int favor)
	{
		m_iBribeFaction = faction;
		m_iBribe = favor;
		m_iApproach = RZ_EQuestApproach.BRIBE;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Sway(int option)
	{
		m_iVote = option;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Tables(int option)
	{
		m_iTable = option;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Swings(int option, int count)
	{
		m_iSwingTo = option;
		m_iSwingCount = count;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Sets(string flag, int value = 1)
	{
		m_sFlag = flag;
		m_iFlagValue = value;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Only(string flag)
	{
		m_sNeed = flag;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Side(int with, int against, int fame, int infamy)
	{
		m_iSideWith = with;
		m_iSideAgainst = against;
		m_iSideFame = fame;
		m_iSideInfamy = infamy;
		if (m_iApproach == RZ_EQuestApproach.TALK)
			m_iApproach = RZ_EQuestApproach.SIDE;

		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Says(string text, string what)
	{
		m_sSays = text;
		m_sWhat = what;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Refused(string text)
	{
		m_sRefused = text;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Hold()
	{
		m_bHold = true;
		m_iApproach = RZ_EQuestApproach.FORCE;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! A hold of its own (after its checks pass): `pos` for `seconds`, win and lost the outcomes.
	RZ_SceneChoice HoldAt(vector pos, float seconds, int win, int lost)
	{
		m_bHold = true;
		m_vHoldAt = pos;
		m_fHoldFor = seconds;
		m_iHoldWin = win;
		m_iHoldLost = lost;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SceneChoice Sneak()
	{
		m_bSneak = true;
		m_iApproach = RZ_EQuestApproach.SNEAK;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! A threat at this person: Fear (RZ_Checks.Threaten) at `difficulty`, +`infamy` with `faction`
	//! whatever happens, once a season per person by anyone.
	RZ_SceneChoice Threaten(int faction, int difficulty, int infamy = 3)
	{
		m_iFearFaction = faction;
		m_iFear = Math.ClampInt(difficulty, 1, 4);
		m_iFearInfamy = infamy;
		m_iApproach = RZ_EQuestApproach.PRESS;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! A trick: passing in `faction`'s kit (the look, RZ_Ways.PassAs) at `difficulty` (1-3).
	RZ_SceneChoice Trick(int faction, int difficulty)
	{
		m_iTrickFaction = faction;
		m_iTrick = Math.ClampInt(difficulty, 1, 3);
		m_iApproach = RZ_EQuestApproach.TRICK;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! A trade: `count` of an item kind (RZ_EErrandItem: food, fuel, a battery, medical) handed over.
	RZ_SceneChoice Trade(RZ_EErrandItem item, int count)
	{
		m_eTradeItem = item;
		m_iTradeCount = count;
		m_iApproach = RZ_EQuestApproach.TRADE;
		return this;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_QM_MajorScene : RZ_QM_HalcyonScene
{
	// The contract, set by BuildScene
	int m_iGiver = RZ_EFaction.SURVIVORS;
	RZ_ETier m_eTier = RZ_ETier.STRANGER;
	int m_iFame = 5;
	int m_iFavor = 10;
	float m_fWindow = 3600;			// real seconds the scene stays open
	bool m_bLate;					// runs after the cure (the codas)
	string m_sFail = "";

	// The vote this scene closes (RZ_QuestVote), and what each option means for the stage
	string m_sVote = "";
	ref array<int> m_aVoteOutcome = {};
	int m_iVoteTie = -1;
	int m_iTimeout = -1;			// the outcome at the end of the window with no vote (-1: it fails)

	// The hold
	vector m_vHold;
	float m_fHoldNeed;
	int m_iHoldWin = -1;
	int m_iHoldLost = -1;
	protected bool m_bHolding;
	protected float m_fHeld;
	protected float m_fHoldGone;
	protected float m_fNextWave;
	protected int m_iHoldBy = -1;

	protected bool m_bBuilt;
	protected ref array<ref RZ_SceneChoice> m_aChoices = {};
	protected ref array<string> m_aSpotNames = {};
	protected ref array<vector> m_aSpotPos = {};
	protected ref array<string> m_aSpotTalk = {};
	protected ref array<string> m_aSpotTalkLabel = {};
	protected ref array<int> m_aSpotVoter = {};
	protected ref map<string, int> m_mDone = new map<string, int>();

	static const float HOLD_RADIUS = 40;
	static const float HOLD_GONE = 45;
	static const float WAVE_SECONDS = 75;

	//------------------------------------------------------------------------------------------------
	//! A person in the scene: at `pos`, `name` for the where text, a "Talk to" label and what they
	//! say. voter: their index in the scene's vote (-1: none). Returns the spot.
	int Person(vector pos, string name, string talkLabel, string talk, int voter = -1)
	{
		m_aSpotPos.Insert(pos);
		m_aSpotNames.Insert(name);
		m_aSpotTalkLabel.Insert(talkLabel);
		m_aSpotTalk.Insert(talk);
		m_aSpotVoter.Insert(voter);
		return m_aSpotPos.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! A choice on spot `spot` (at most four a person; more are dropped).
	RZ_SceneChoice Choice(int spot, string label)
	{
		RZ_SceneChoice c = new RZ_SceneChoice();
		c.m_iSpot = spot;
		c.m_sLabel = label;
		int onSpot = 0;
		foreach (RZ_SceneChoice other : m_aChoices)
		{
			if (other.m_iSpot == spot)
				onSpot++;
		}

		if (onSpot < 4)
			m_aChoices.Insert(c);

		return c;
	}

	//------------------------------------------------------------------------------------------------
	//! The scene holds `pos` for `seconds` once a choice starts it: win and lost are the outcomes.
	void SetHold(vector pos, float seconds, int win, int lost)
	{
		m_vHold = pos;
		m_fHoldNeed = seconds;
		m_iHoldWin = win;
		m_iHoldLost = lost;
	}

	//------------------------------------------------------------------------------------------------
	//! The scene closes the vote `name` at the end of its window: option o decides outcome
	//! outcomes[o]; a tie goes to `tie`.
	void SetVote(string name, array<int> outcomes, int tie)
	{
		m_sVote = name;
		m_aVoteOutcome.Copy(outcomes);
		m_iVoteTie = tie;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (run && run.m_Line)
			m_bBuilt = run.m_Line.BuildScene(gm, run, run.m_iStage, this);
		// The majors' remaining approaches (Director/RZ_MajorRest.c)
		if (m_bBuilt && run && run.m_Line && run.m_Line.IsMajor())
			RZ_MajorRest.Get().More(gm, run, run.m_iStage, this);

		RZ_EFaction giver = m_iGiver;
		SetContract(giver, m_eTier, m_iFame, m_iFavor);
		m_Req.m_bAfterCure = m_bLate;
		m_fTimeLeft = m_fWindow;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!m_bBuilt || m_aSpotPos.IsEmpty())
			return false;

		for (int i = 0; i < m_aSpotPos.Count(); i++)
		{
			string labels = "";
			foreach (RZ_SceneChoice c : m_aChoices)
			{
				if (c.m_iSpot != i)
					continue;

				if (!labels.IsEmpty())
					labels += "|";

				labels += c.m_sLabel;
			}

			int clue = AddClue(m_aSpotNames[i], m_aSpotTalk[i]);
			AddSpot(m_aSpotPos[i], RZ_EPuzzleProp.CIVILIAN, i, clue, m_aSpotTalkLabel[i], labels, m_aSpotNames[i]);
		}

		if (m_vPos == vector.Zero)
			m_vPos = m_aSpotPos[0];

		if (m_iSettlement < 0)
			m_iSettlement = gm.FindNearestSettlement(m_vPos);

		m_Quiet.Watch(m_vPos, 120);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		RZ_SceneChoice c = FindChoice(objIdx, option);
		RZ_QuestRun run = Run(gm);
		if (!c || !run || m_bResolved)
			return;

		string ident = gm.GetProfile(playerId).m_sIdentity;
		string onceKey = string.Format("%1|%2|%3", ident, objIdx, option);
		if (m_mDone.Contains(onceKey))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've done that here already.");
			return;
		}

		if (!Allowed(gm, run, playerId, c))
			return;

		m_mDone.Set(onceKey, 1);
		Apply(gm, run, playerId, objIdx, c);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_SceneChoice FindChoice(int spot, int option)
	{
		int n = 0;
		foreach (RZ_SceneChoice c : m_aChoices)
		{
			if (c.m_iSpot != spot)
				continue;

			if (n == option)
				return c;

			n++;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The checks: a flag, the rank, unseen, the persuade and the bribe. Tells the player why not.
	protected bool Allowed(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, RZ_SceneChoice c)
	{
		if (!c.m_sNeed.IsEmpty() && !HasNeed(gm, run, c.m_sNeed))
		{
			gm.NotifyPlayer(playerId, m_sTitle, run.Fill(c.m_sRefused));
			return false;
		}

		int faction = c.m_iFaction;
		if (faction < 0)
			faction = m_iGiver;

		if (c.m_iRank > -99 && Rank(gm, playerId, faction) < c.m_iRank && !RZ_QuestApproach.HasBackground(gm, playerId, c.m_sFavoured))
		{
			Remember(gm, playerId, c.m_iApproach, "wasn't heard");
			gm.NotifyPlayer(playerId, m_sTitle, run.Fill(c.m_sRefused) + string.Format("\n(That takes %1 with %2.)", RankWord(c.m_iRank), RZ_Factions.GetNameInText(faction)));
			return false;
		}

		if (c.m_bSneak && !m_Quiet.IsQuiet(gm, playerId))
		{
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was seen");
			gm.NotifyPlayer(playerId, m_sTitle, m_Quiet.Why(gm, playerId));
			return false;
		}

		if (c.m_iPersuade > 0 && !TryPersuade(gm, playerId, faction, c.m_iPersuade, c.m_sFavoured))
		{
			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "wasn't believed");
			gm.NotifyPlayer(playerId, m_sTitle, run.Fill(c.m_sRefused));
			return false;
		}

		if (c.m_iBribe > 0)
		{
			int bf = c.m_iBribeFaction;
			if (bf < 0)
				bf = faction;

			if (!TryBribe(gm, playerId, bf, c.m_iBribe))
				return false;
		}

		return AllowedWays(gm, run, playerId, c);
	}

	//------------------------------------------------------------------------------------------------
	//! A threat, a trick or a trade: the item check first (nothing is taken on a refusal), then the
	//! roll. Each tells the player why not.
	protected bool AllowedWays(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, RZ_SceneChoice c)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (c.m_iTradeCount > 0 && (!me || RZ_Errands.CountItems(me, c.m_eTradeItem) < c.m_iTradeCount))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You haven't got that to give.");
			return false;
		}

		string person = "scene." + m_sTitle;
		if (c.m_iSpot >= 0 && c.m_iSpot < m_aSpotNames.Count())
			person = person + "." + m_aSpotNames[c.m_iSpot];

		m_Ways.m_sTitle = m_sTitle;
		if (c.m_iFear > 0)
		{
			int listener = c.m_iFearFaction;
			if (listener < 0)
				listener = m_iGiver;

			if (m_Ways.TryFear(gm, playerId, person, listener, m_iGiver, c.m_iFear, false, c.m_iFearInfamy) != RZ_ECheck.PASS)
			{
				Remember(gm, playerId, RZ_EQuestApproach.PRESS, "didn't scare them");
				gm.NotifyPlayer(playerId, m_sTitle, run.Fill(c.m_sRefused));
				return false;
			}
		}

		if (c.m_iTrick > 0 && m_Ways.PassAs(gm, playerId, person, c.m_iTrickFaction, c.m_iTrick, "") != RZ_ECheck.PASS)
		{
			Remember(gm, playerId, RZ_EQuestApproach.TRICK, "was seen through");
			gm.NotifyPlayer(playerId, m_sTitle, run.Fill(c.m_sRefused));
			return false;
		}

		if (c.m_iTradeCount > 0)
			RZ_Errands.TakeItems(me, c.m_eTradeItem, c.m_iTradeCount);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "flag" (a run flag set and not 0), "world:key" (a world flag set and not 0), "!..." (not).
	static bool HasNeed(RZ_GameModeComponent gm, RZ_QuestRun run, string need)
	{
		bool negate = need.StartsWith("!");
		string key = need;
		if (negate)
			key = need.Substring(1, need.Length() - 1);

		bool has;
		if (key.StartsWith("world:"))
			has = gm.GetQuestlines().WorldFlag(key.Substring(6, key.Length() - 6)) != 0;
		else
			has = run.Flag(key) != 0;

		if (negate)
			return !has;

		return has;
	}

	//------------------------------------------------------------------------------------------------
	protected static string RankWord(int rank)
	{
		switch (rank)
		{
			case 0: return "not being hunted";
			case 1: return "Known";
			case 2: return "Trusted";
			case 3: return "Champion";
		}

		return "standing";
	}

	//------------------------------------------------------------------------------------------------
	protected void Apply(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int spot, RZ_SceneChoice c)
	{
		AddParticipant(playerId);
		if (!c.m_sFlag.IsEmpty())
		{
			if (c.m_sFlag.StartsWith("world:"))
				gm.GetQuestlines().SetWorldFlag(c.m_sFlag.Substring(6, c.m_sFlag.Length() - 6), c.m_iFlagValue);
			else
				run.SetFlag(c.m_sFlag, c.m_iFlagValue);
		}

		if (c.m_iSideWith >= 0 || c.m_iSideAgainst >= 0)
			RZ_QuestApproach.SideWith(gm, playerId, c.m_iSideWith, c.m_iSideAgainst, c.m_iSideFame, c.m_iSideInfamy);

		string says = run.Fill(c.m_sSays);
		if (c.m_iVote >= 0 && spot < m_aSpotVoter.Count() && m_aSpotVoter[spot] >= 0 && !m_sVote.IsEmpty())
		{
			RZ_QuestVote vote = new RZ_QuestVote(run, m_sVote);
			if (!vote.Sway(gm, playerId, m_aSpotVoter[spot], c.m_iVote))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "You've had your say at this sitting.");
				return;
			}
		}

		if (!m_sVote.IsEmpty() && (c.m_iTable >= 0 || c.m_iSwingTo >= 0))
		{
			RZ_QuestVote tabled = new RZ_QuestVote(run, m_sVote);
			if (c.m_iTable >= 0)
				tabled.Table(c.m_iTable);

			if (c.m_iSwingTo >= 0)
				tabled.Swing(-1, c.m_iSwingTo, c.m_iSwingCount);
		}

		string what = c.m_sWhat;
		if (what.IsEmpty())
			what = c.m_sLabel;

		if (!says.IsEmpty())
			gm.NotifyPlayer(playerId, m_sTitle, says);

		if (c.m_bHold)
		{
			if (c.m_fHoldFor > 0 && !m_bHolding)
				SetHold(c.m_vHoldAt, c.m_fHoldFor, c.m_iHoldWin, c.m_iHoldLost);

			StartHold(gm, playerId);
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, what);
			return;
		}

		if (c.m_iOutcome >= 0)
		{
			Resolve(gm, playerId, c.m_iApproach, c.m_iOutcome, what, c.m_bSuccess);
			return;
		}

		Remember(gm, playerId, c.m_iApproach, what);
	}

	// ================================================================ The hold
	//------------------------------------------------------------------------------------------------
	protected void StartHold(RZ_GameModeComponent gm, int playerId)
	{
		if (m_bHolding || m_fHoldNeed <= 0)
			return;

		if (m_vHold == vector.Zero)
			m_vHold = m_vPos;

		m_bHolding = true;
		m_iHoldBy = playerId;
		m_fHeld = 0;
		m_fHoldGone = 0;
		m_fNextWave = 0;
		NotifyInterested(gm, m_sTitle, string.Format("Hold the place for %1 minutes. The noise will bring them.", RZ_Util.RoundToInt(m_fHoldNeed / 60)));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bHolding)
			return;

		bool anyone = false;
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vHold) <= HOLD_RADIUS)
			{
				anyone = true;
				AddParticipant(ids[i]);
			}
		}

		m_fNextWave -= dt;
		if (m_fNextWave <= 0)
		{
			m_fNextWave = WAVE_SECONDS;
			SoundAlarm(gm, m_vHold, 40, false);
		}

		if (anyone)
		{
			m_fHoldGone = 0;
			m_fHeld += dt;
		}
		else
		{
			m_fHoldGone += dt;
		}

		if (m_fHeld >= m_fHoldNeed)
		{
			m_bHolding = false;
			NotifyInterested(gm, m_sTitle, "It held.");
			Resolve(gm, m_iHoldBy, RZ_EQuestApproach.FORCE, m_iHoldWin, "held the place");
			return;
		}

		if (m_fHoldGone >= HOLD_GONE)
		{
			m_bHolding = false;
			NotifyInterested(gm, m_sTitle, "Nobody held it. It broke up.");
			if (m_iHoldLost >= 0)
				Resolve(gm, m_iHoldBy, RZ_EQuestApproach.FORCE, m_iHoldLost, "lost the hold", false);
		}
	}

	// ================================================================ The window
	//------------------------------------------------------------------------------------------------
	//! The window closes: the vote is taken (or the timeout outcome), else the stage fails.
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || m_bResolved)
		{
			super.OnTimeUp(gm);
			return;
		}

		if (!m_sVote.IsEmpty())
		{
			RZ_QuestVote vote = new RZ_QuestVote(run, m_sVote);
			int result = vote.Tally(m_iVoteTie);
			if (result >= 0 && result < m_aVoteOutcome.Count())
			{
				m_bResolved = true;
				run.SetOutcome(m_aVoteOutcome[result]);
				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}
		}

		if (m_iTimeout >= 0)
		{
			m_bResolved = true;
			run.SetOutcome(m_iTimeout);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		super.OnTimeUp(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (!m_sFail.IsEmpty())
			return m_sFail;

		return super.GetFailText(gm);
	}
}
