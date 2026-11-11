//! Approach helpers for questline stage missions (docs/DESIGN.md, "Questlines"): talk, persuade,
//! sneak, force, bribe, betray and siding with another faction. Each approach a player takes is
//! recorded on the questline's run (by identity), and the one that decides the stage sets its
//! outcome, which the questline reads in Next().
//!
//! - RZ_QuestApproach: static checks any mission can use (persuade score, bribe, siding).
//! - RZ_QuestStealth: "not seen, no shots near" for one place.
//! - RZ_QuestMission: a puzzle-mission base for stage missions, with people to talk to, the
//!   checks above wrapped once per player through the shared ways (Missions/RZ_Ways.c, keyed by
//!   identity), the quiet threat (TryMenace), and Resolve() to end the stage with an outcome.
//! The shared checks (Trust, Fear, the price table) are RZ_Checks (Missions/RZ_Checks.c).
//! Force needs nothing new: RZ_UnitWatch (Missions/Blocks/RZ_ContractHelp.c) for a spawned
//! unit, SpawnGuards and OnZombiesKilled for the infected, RZ_Util.IsArmed for a threat.

//------------------------------------------------------------------------------------------------
class RZ_QuestApproach
{
	//------------------------------------------------------------------------------------------------
	//! How persuasive this player is with `faction`'s people: the standing's access rank (Stranger
	//! 0, Known 1, Trusted 2, Champion 3; mixed tiers one less; Watched -1, Hunted -2, Kill on
	//! Sight -3), +1 with a background in `favoured` (comma-separated ids, e.g. "police,excon").
	//! faction -1: no standing counts (0).
	static int PersuadeScore(RZ_GameModeComponent gm, int playerId, int faction, string favoured)
	{
		if (!gm || playerId <= 0)
			return -10;

		int score = 0;
		if (RZ_Factions.IsValid(faction))
			score = RZ_Reputation.GetAccessRank(gm.GetTier(playerId, faction));

		if (HasBackground(gm, playerId, favoured))
			score++;

		return score;
	}

	//------------------------------------------------------------------------------------------------
	//! True if the score reaches `difficulty` (1 easy: Known, or a favoured Stranger; 2 normal:
	//! Trusted, or a favoured Known; 3 hard: Champion, or a favoured Trusted).
	static bool Persuade(RZ_GameModeComponent gm, int playerId, int faction, int difficulty, string favoured)
	{
		return PersuadeScore(gm, playerId, faction, favoured) >= difficulty;
	}

	//------------------------------------------------------------------------------------------------
	//! Is the player's background one of `ids` (comma-separated)?
	static bool HasBackground(RZ_GameModeComponent gm, int playerId, string ids)
	{
		if (ids.IsEmpty() || playerId <= 0)
			return false;

		string mine = gm.GetProfile(playerId).m_sBackgroundId;
		if (mine.IsEmpty())
			return false;

		array<string> list = {};
		ids.Split(",", list, true);
		foreach (string id : list)
		{
			if (id.Trim() == mine)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Pay `cost` Favor with `faction`. False (and the player is told) if they haven't got it.
	static bool Bribe(RZ_GameModeComponent gm, int playerId, int faction, int cost, string title)
	{
		if (!gm || playerId <= 0 || !RZ_Factions.IsValid(faction))
			return false;

		if (gm.SpendFavor(playerId, faction, Math.MaxInt(1, cost)))
			return true;

		gm.NotifyPlayer(playerId, title, string.Format("That takes %1 Favor with %2. You have %3.", cost, RZ_Factions.GetNameInText(faction), gm.GetProfile(playerId).GetFavor(faction)));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The player takes `with`'s side against `against`: Fame with one (at most 10), Infamy with
	//! the other (at most 15). Personal, at once.
	static void SideWith(RZ_GameModeComponent gm, int playerId, int with, int against, int fame, int infamy)
	{
		if (!gm || playerId <= 0)
			return;

		if (RZ_Factions.IsValid(with) && fame > 0)
			gm.AddFame(playerId, with, Math.MinInt(fame, RZ_Questline.MAX_FAME), false);

		if (RZ_Factions.IsValid(against) && infamy > 0)
			gm.AddInfamy(playerId, against, Math.MinInt(infamy, RZ_Questline.MAX_INFAMY), "sided against", RZ_RepFlag.SHARE);
	}

	//------------------------------------------------------------------------------------------------
	static string Name(int approach)
	{
		switch (approach)
		{
			case RZ_EQuestApproach.TALK: return "talk";
			case RZ_EQuestApproach.PERSUADE: return "persuade";
			case RZ_EQuestApproach.SNEAK: return "sneak";
			case RZ_EQuestApproach.FORCE: return "force";
			case RZ_EQuestApproach.BRIBE: return "bribe";
			case RZ_EQuestApproach.BETRAY: return "betray";
			case RZ_EQuestApproach.SIDE: return "side";
			case RZ_EQuestApproach.PRESS: return "press";
			case RZ_EQuestApproach.TRADE: return "trade";
			case RZ_EQuestApproach.TRICK: return "trick";
			case RZ_EQuestApproach.LEAVE: return "leave";
		}

		return "none";
	}
}

//------------------------------------------------------------------------------------------------
//! Stealth for one place: a player stays "quiet" while nobody fired within the radius since
//! Watch() and they haven't been noticed there (too much heat where they stand, or an infected
//! close for too long: RZ_NoticeTracker, slower to notice a player who is hard to see). Once
//! noticed, they stay noticed for this mission. A person in the scene who sees anyone within N m
//! should use RZ_Stealth.SightRadius(playerId, N): the same N at normal visibility, less for a
//! player lying low in the dark (docs/DESIGN.md, "Stealth").
class RZ_QuestStealth
{
	ref RZ_NoticeTracker m_Notice = new RZ_NoticeTracker();
	protected vector m_vCenter;
	protected float m_fRadius = 120;
	protected bool m_bWatching;
	protected bool m_bShots;
	protected ref array<int> m_aNoticed = {};

	//------------------------------------------------------------------------------------------------
	void Watch(vector center, float radius)
	{
		m_vCenter = center;
		m_fRadius = radius;
		m_bWatching = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Once per mission tick.
	void Track(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players, float dt)
	{
		if (!m_bWatching)
			return;

		m_Notice.Track(gm, ids, players, dt, m_vCenter, m_fRadius);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vCenter) <= m_fRadius && !m_aNoticed.Contains(ids[i]) && m_Notice.IsNoticed(gm, p))
				m_aNoticed.Insert(ids[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From the mission's OnPlayerShot.
	void OnShot(vector pos)
	{
		if (m_bWatching && RZ_Util.DistXZ(pos, m_vCenter) <= m_fRadius)
			m_bShots = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Mark a player as seen (a person in the scene spotted them).
	void Spot(int playerId)
	{
		if (playerId > 0 && !m_aNoticed.Contains(playerId))
			m_aNoticed.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	bool IsQuiet(RZ_GameModeComponent gm, int playerId)
	{
		if (m_bShots || m_aNoticed.Contains(playerId))
			return false;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		return !ent || !m_Notice.IsNoticed(gm, ent);
	}

	//------------------------------------------------------------------------------------------------
	bool WasLoud()
	{
		return m_bShots;
	}

	//------------------------------------------------------------------------------------------------
	//! Why not, for the message. "" if quiet.
	string Why(RZ_GameModeComponent gm, int playerId)
	{
		if (m_bShots)
			return "There's been shooting here. Everyone's awake now.";

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		string why = "";
		if (ent)
			why = m_Notice.GetReason(gm, ent);

		if (why.IsEmpty() && m_aNoticed.Contains(playerId))
			why = "They've already seen you here.";

		return why;
	}
}

//------------------------------------------------------------------------------------------------
//! Base for questline stage missions: a puzzle mission (clues, people, choice and use actions)
//! that knows its questline run. Never picked at random (weight 0, and never registered with the
//! mission manager): only its questline starts it.
//!
//! In Setup (SetupPuzzle) read the run's data with QFlag/QText. People and props can wait until
//! a player is near (SpawnWhenNear). In OnChoice/OnUse/OnClueRead use the approach helpers, and
//! call Resolve() with the approach and outcome that decided the stage.
class RZ_QuestMission : RZ_PuzzleMission
{
	// The shared ways through a job (Missions/RZ_Ways.c), keyed by identity: one try per person per
	// identity, who did what, the quiet threat
	protected ref RZ_Ways m_Ways = new RZ_Ways();
	protected ref RZ_QuestStealth m_Quiet = new RZ_QuestStealth();
	protected bool m_bResolved;
	protected bool m_bSceneUp;
	protected float m_fSceneRadius = 400;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The questline run this stage belongs to, null if it's gone.
	RZ_QuestRun Run(RZ_GameModeComponent gm)
	{
		if (!gm || !gm.GetQuestlines())
			return null;

		return gm.GetQuestlines().FindRun(m_iQuestRun);
	}

	//------------------------------------------------------------------------------------------------
	protected int QFlag(RZ_GameModeComponent gm, string key, int def = 0)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return def;

		return run.Flag(key, def);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetQFlag(RZ_GameModeComponent gm, string key, int value)
	{
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetFlag(key, value);
	}

	//------------------------------------------------------------------------------------------------
	protected string QText(RZ_GameModeComponent gm, string key)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return "";

		return run.Text(key);
	}

	//------------------------------------------------------------------------------------------------
	//! {keys} filled in from the run.
	protected string QFill(RZ_GameModeComponent gm, string text)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return text;

		return run.Fill(text);
	}

	//------------------------------------------------------------------------------------------------
	//! A line for the journal's "what you know so far".
	protected void QNote(RZ_GameModeComponent gm, string text)
	{
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Note(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Record what a player did without ending the stage (asked around, was turned down).
	protected void Remember(RZ_GameModeComponent gm, int playerId, int approach, string what)
	{
		AddParticipant(playerId);
		m_Ways.Remember(gm, playerId, approach);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Did(gm, playerId, approach, what);
	}

	//------------------------------------------------------------------------------------------------
	//! End the stage: `playerId` decided it by `approach`, and the questline gets `outcome`.
	//! success = true: the mission succeeds and pays its Fame and Favor as usual; false: it ends
	//! as failed for its giver (siding with someone else, betrayal) and pays nothing, but the
	//! questline still gets `outcome`. Only the first call counts.
	protected void Resolve(RZ_GameModeComponent gm, int playerId, int approach, int outcome, string what, bool success = true)
	{
		if (m_bResolved || m_eState != RZ_EMissionState.ACTIVE)
			return;

		m_bResolved = true;
		Remember(gm, playerId, approach, what);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetOutcome(outcome);

		if (success)
			m_eState = RZ_EMissionState.SUCCEEDED;
		else
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	//! A persuade check, once per player in this mission (by identity: leaving and rejoining gives a
	//! new id, never a second try). False when it fails or was tried.
	protected bool TryPersuade(RZ_GameModeComponent gm, int playerId, int faction, int difficulty, string favoured)
	{
		m_Ways.m_sTitle = m_sTitle;
		return m_Ways.TryPersuade(gm, playerId, faction, difficulty, favoured);
	}

	//------------------------------------------------------------------------------------------------
	protected bool TryBribe(RZ_GameModeComponent gm, int playerId, int faction, int cost)
	{
		m_Ways.m_sTitle = m_sTitle;
		return m_Ways.TryBribe(gm, playerId, faction, cost);
	}

	//------------------------------------------------------------------------------------------------
	//! The quiet threat on a FORCE option: set m_Ways.m_iMenaceDifficulty in the stage's Setup, then
	//! call this before the fight. True when the threat worked (Fear at that difficulty).
	protected bool TryMenace(RZ_GameModeComponent gm, int playerId, string person, int listener, bool stronghold)
	{
		m_Ways.m_sTitle = m_sTitle;
		return m_Ways.TryMenace(gm, playerId, person, listener, m_eGiver, stronghold);
	}

	//------------------------------------------------------------------------------------------------
	//! A person to talk to (the puzzle civilian, or the generic object without one): "Talk to"
	//! reads clue `talkClue`, and up to four choices ("Ask|Press him|Pay him"; empty slots hidden).
	protected IEntity SpawnPerson(RZ_GameModeComponent gm, vector pos, int objIdx, int talkClue, string talkLabel, string choices)
	{
		return SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, pos, objIdx, "", choices, talkClue, talkLabel);
	}

	//------------------------------------------------------------------------------------------------
	//! Override to put people and props down when the first player comes within m_fSceneRadius
	//! (nothing is spawned for nobody). False = it couldn't, and the mission fails.
	protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Update(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSceneUp && m_eState == RZ_EMissionState.ACTIVE && CountPlayersNear(players, m_vPos, m_fSceneRadius) > 0)
		{
			m_bSceneUp = true;
			if (!SpawnScene(gm))
				m_eState = RZ_EMissionState.FAILED;
		}

		m_Quiet.Track(gm, ids, players, dt);
		super.Update(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerShot(RZ_GameModeComponent gm, int playerId, vector pos)
	{
		m_Quiet.OnShot(pos);
	}
}
