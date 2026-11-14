enum RZ_EMissionState
{
	ACTIVE,
	SUCCEEDED,
	FAILED
}

//! Base for every mission. There are two kinds (docs/MISSIONS.md):
//!
//! - World events (m_bContract = false) are news on the radio, not jobs: anyone who
//!   takes part shares the reward. A puzzle or arc event becomes a job only for a player
//!   who used its thing (read its clue: RZ_PuzzleMission.HandleRead).
//! - Faction contracts (m_bContract = true) are offered in person only (docs/MISSIONS.md, "Who
//!   gets offered what"): a player who meets m_Req asks its giver (a quartermaster, a leader, a
//!   safehouse radio, a wanderer) and is hired (Hire). The radio only says who to ask
//!   (RZ_MissionManager.TellOfWork). Anyone can turn up, but only players who were hired (or are in
//!   a hired player's group) and are eligible get paid.
//!
//! A mission picks a settlement on the current map, so every type works on any
//! terrain. Anyone who comes within m_fParticipationRadius while it runs counts
//! as taking part.
//!
//! Life cycle, driven by RZ_MissionManager:
//!   new -> Configure() -> GetWeight() -> Setup() -> Update() each tick ->
//!   OnSuccess()/OnFailed() -> rewards -> Cleanup()
class RZ_Mission
{
	int m_iId;
	string m_sTitle;
	string m_sBrief;
	int m_iSettlement = -1;
	vector m_vPos;
	vector m_vAskedAt;				// made on asking (MakeFor): where; that giver always offers it (RZ_OfferReach)

	bool m_bContract = true;		// false = world event, open to everyone
	bool m_bGlobal;					// no single location (Every Ridge Calls, Storm...)
	bool m_bHasGiver = true;		// false = no faction pays (most world events)
	RZ_EFaction m_eGiver = RZ_EFaction.SURVIVORS;
	int m_iFameReward = 10;
	int m_iFavorReward = 10;
	bool m_bHasRival;
	RZ_EFaction m_eRival = RZ_EFaction.BANDITS;
	int m_iRivalInfamy;
	bool m_bAmends;					// Watched players work off Infamy instead of gaining Fame
	float m_fBonusFameMult = 1;		// extra Fame for ★ backgrounds (see m_Req.m_aBonusBackgrounds)
	float m_fRewardScale = 1;		// the mission can lower or raise the payout (e.g. mercy kill)
	ref RZ_MissionRequirements m_Req = new RZ_MissionRequirements();

	float m_fTimeLeft = 2400;
	float m_fParticipationRadius = 150;
	RZ_EMissionState m_eState = RZ_EMissionState.ACTIVE;
	ref array<int> m_aParticipants = {};
	ref array<int> m_aOfferedTo = {};	// hired: offered in person by its giver or a thing of its (Hire)
	ref array<int> m_aHeardBy = {};		// told on the radio who to ask for it (not hired)
	int m_iMarkerId = -1;			// map marker handle (RZ_MapMarkers), -1 = none

	// Faction leaders (Factions/RZ_Leaders.c, Missions/Leaders/)
	int m_iLeaderId = -1;			// the leader who gives (voices and pays) this contract, -1 = none
	bool m_bInPersonOnly;			// a leader's personal job: offered by them in person, never on the radio
	bool m_bRogueJob;				// off the stronghold's books: may be discovered when it's done
	bool m_bSeenByFaction;			// a rogue job seen by the leader's own people
	bool m_bExposed;				// the player turned the rogue leader in instead
	bool m_bNoLeaderCredit;			// done, but not the way the leader wanted: they don't remember it as help
	int m_iJobFaction = -1;			// personal jobs: the faction and loyalty of the leaders who give them
	int m_iJobLoyalty = -1;
	int m_iJobTone;					// RZ_Leaders.TONE_* bits, which traits lean toward

	// Questlines (Director/RZ_Questlines.c): the run that started this mission as one of its stages
	int m_iQuestRun = -1;			// -1 = not part of a questline
	//! The story (arc or questline) that started it: subjects that story claimed count as free
	//! for this mission (RZ_WorldFacts claims). "" = none.
	string m_sClaimParent;
	string m_sDeed;			// what people say you did: a short past-tense phrase with {town} ("brought medicine to {town}")
	//! The story kit (Missions/Story/RZ_MissionStory.c): who took part and who decides, who benefits,
	//! truths, records. Null: the mission runs exactly as before.
	ref RZ_MissionStory m_Story;
	ref RZ_OneChoice m_Choice;		// one real choice before it ends (Missions/Blocks/RZ_OneChoice.c), null for none
	//! A story job's truth, reads and talk scene (Missions/Story/RZ_TalkScene.c). Null for none.
	ref RZ_StoryJob m_Job;
	//! A mirror offer (Missions/Story/RZ_Mirror.c): a second giver for the same job. Null: one giver.
	ref RZ_Mirror m_Mirror;
	//! An amends job reserved for one player by a parley or a knock (Factions/RZ_Parley.c): offered to
	//! them alone, though they're Hunted, and it works their Infamy off. "" = not reserved.
	string m_sReservedFor;
	bool m_bSwitched;				// its result went to another side than the giver's: pays at most the base
	string m_sWhyNow;				// the reason it was started for ("Levie's on half portions."): the brief opens with it
	vector m_vReasonAt;				// where that reason is: a reason-started contract must land near it
	//! Called off by the world, not failed by anyone (Withdraw): no pay, no blame, no penalty.
	bool m_bWithdrawn;
	//! A Bastion contract whose GetWeight already follows Bastion presence: the mission manager
	//! doesn't scale it again (RZ_MissionManager.PresenceMult).
	bool m_bPresenceWeighted;
	string m_sWithdrawText;

	// Rough areas (docs/MISSIONS.md, "Who gets offered what"): contracts name a circle, not a spot
	ref array<int> m_aExactFor = {};	// players who came close: they know the spot now
	protected vector m_vRoughOffset;	// the circle's centre, from the real spot (follows it if it moves)
	protected bool m_bRoughReady;

	//------------------------------------------------------------------------------------------------
	//! Set giver, rival, rewards and requirements here. Runs before GetWeight and Setup.
	void Configure(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Shortcut for Configure(): a contract from `giver`.
	protected void SetContract(RZ_EFaction giver, RZ_ETier minTier, int fame, int favor)
	{
		m_bContract = true;
		m_bHasGiver = true;
		m_eGiver = giver;
		m_Req.m_eGiver = giver;
		m_Req.m_eMinTier = minTier;
		m_iFameReward = fame;
		m_iFavorReward = favor;
	}

	//------------------------------------------------------------------------------------------------
	//! Shortcut for Configure(): a world event. Pass hasGiver = true if a faction still pays for it.
	protected void SetEvent(bool hasGiver, RZ_EFaction giver = RZ_EFaction.SURVIVORS, int fame = 0, int favor = 0)
	{
		m_bContract = false;
		m_bHasGiver = hasGiver;
		m_eGiver = giver;
		m_Req.m_eGiver = giver;
		m_iFameReward = fame;
		m_iFavorReward = favor;
	}

	//------------------------------------------------------------------------------------------------
	//! Shortcut for Configure(): a leader's personal job (docs/MISSIONS.md, "Leader missions").
	//! Only leaders of this faction and loyalty give it, in person. Their attitude, not the
	//! giver tier, decides who may take it. Rogue jobs pay no faction Fame by default.
	protected void SetLeaderJob(RZ_EFaction faction, RZ_ELoyalty loyalty, int tone, int fame, int favor)
	{
		SetContract(faction, RZ_ETier.STRANGER, fame, favor);
		m_Req.m_bLeaderDecides = true;
		m_bInPersonOnly = true;
		m_bRogueJob = loyalty == RZ_ELoyalty.ROGUE;
		m_iJobFaction = faction;
		m_iJobLoyalty = loyalty;
		m_iJobTone = tone;
		m_fParticipationRadius = 0;
		m_fTimeLeft = 2700;
	}

	//------------------------------------------------------------------------------------------------
	//! Personal jobs: how much this leader wants this job done (before trait bias). 0 = can't
	//! run here (missing prefabs, no second safehouse...). Never called for other missions.
	float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Personal jobs are only listed in person: near the leader's seat.
	bool CanOfferAt(RZ_GameModeComponent gm, vector playerPos)
	{
		if (!m_bInPersonOnly)
			return true;

		if (!gm.GetLeaders())
			return false;

		RZ_Leader l = gm.GetLeaders().Get(m_iLeaderId);
		if (!l)
			return false;

		return RZ_Util.DistXZ(l.m_vPos, playerPos) <= 80;
	}

	//------------------------------------------------------------------------------------------------
	//! True for contracts that must give exact positions: puzzles and others that give their own
	//! clues or grid references, points to visit, a rough ping of their own. Never a rough area.
	bool NeedsExactLocation()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! True for missions that hand players a vehicle (off under the season's No Vehicles rule).
	bool NeedsVehicle()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Gunfire by a player (all missions hear it; e.g. Local Law fails on bloodshed).
	void OnPlayerShot(RZ_GameModeComponent gm, int playerId, vector pos)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void SetRival(RZ_EFaction rival, int infamy)
	{
		m_bHasRival = true;
		m_eRival = rival;
		m_iRivalInfamy = infamy;
	}

	//------------------------------------------------------------------------------------------------
	//! Who benefits (RZ_Beneficiaries) sets the side the decider crossed.
	void SetRivalPublic(int rival, int infamy)
	{
		SetRival(rival, infamy);
	}

	//------------------------------------------------------------------------------------------------
	//! A scene decision's 5 s hold ran out with nobody objecting (RZ_Deciding): `decider` chose
	//! `option`. Missions with a scene decision override it and end the hold.
	void OnHoldDone(RZ_GameModeComponent gm, int decider, int option)
	{
		if (m_Story)
			m_Story.m_Deciding.EndHold();

		// A talk scene's decision
		if (option >= RZ_TalkScene.HOLD_BASE)
		{
			if (m_Job && m_Job.m_Scene)
				m_Job.m_Scene.HoldDone(gm, this, decider, option);

			return;
		}

		if (m_Choice)
			m_Choice.HoldDone(gm, this, decider, option);
	}

	//------------------------------------------------------------------------------------------------
	//! A way through at a talk scene's person passed, or its decision settled (RZ_TalkScene): `way`
	//! is the mission's own number for it.
	void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! A one-choice job's question was answered (or its default taken, decider -1): what it does.
	void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! A one-choice job asks its question only on its own: never as a questline's stage (the stage's
	//! story decides), and never in a subclass that reuses it (they override this).
	bool AsksChoice()
	{
		return m_iQuestRun < 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a one-choice job's person turns up when its work is done.
	vector ChoiceAt()
	{
		return m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	//! Relative chance of this mission type being picked right now. 0 = never.
	float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Pick a location and fill in the text. Return false if no valid location exists.
	bool Setup(RZ_GameModeComponent gm)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Subjects this mission needs for its whole run (docs/DESIGN.md, "World facts"): "lab", "heli",
	//! "pit", RZ_WorldFacts.Town(idx)... Known after Configure. The mission manager starts it only
	//! while they're all free, holds them from before Setup until it ends, and lets go if Setup
	//! fails. Subjects picked in Setup are claimed there with ClaimSubject.
	void GetClaims(RZ_GameModeComponent gm, notnull array<string> result)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Not started by a questline or a story arc: a contract on its own. A story that reuses a
	//! mission holds what it needs itself, so a mission claims shared subjects ("lab") and keeps out
	//! of a questline's premise (NotWhileQuest) only on its own.
	bool IsStandalone()
	{
		return m_iQuestRun < 0 && m_sClaimParent.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! In Setup: hold one more subject (a safehouse, a leader, a wanderer it just picked) until the
	//! mission ends. False if someone else holds it: pick another, or return false from Setup.
	protected bool ClaimSubject(RZ_GameModeComponent gm, string subject)
	{
		return gm.GetFacts().ClaimFor(subject, RZ_WorldFacts.MissionOwner(m_iId), m_sClaimParent);
	}

	//------------------------------------------------------------------------------------------------
	//! Free for this mission: nobody holds it but this mission or the story that started it.
	bool IsSubjectFree(RZ_GameModeComponent gm, string subject)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		string holder = facts.ClaimedBy(subject);
		if (holder.IsEmpty())
			return true;

		return holder == RZ_WorldFacts.MissionOwner(m_iId) || (!m_sClaimParent.IsEmpty() && holder == m_sClaimParent);
	}

	//------------------------------------------------------------------------------------------------
	//! In Setup: another mission uses this town, or a story holds it (the town, a safehouse in it,
	//! the lab) and it isn't this mission's or its own story's. Use it for the mission's own town;
	//! the shared pickers take the mission as `asker` for the same rule.
	protected bool IsTownBusy(RZ_GameModeComponent gm, int idx)
	{
		return gm.GetMissions().IsSettlementBusy(idx, this);
	}

	//------------------------------------------------------------------------------------------------
	//! The brief as players read it: the world's tokens ({day}, {leader:id}...) filled in now, so
	//! a name or a day in it is never stale (RZ_WorldFacts.Fill). Every place that shows a brief
	//! calls this; m_sBrief is the template.
	string GetBrief(RZ_GameModeComponent gm)
	{
		string brief = m_sBrief;
		if (!m_sWhyNow.IsEmpty())
			brief = m_sWhyNow + " " + brief;

		if (!gm || !gm.GetFacts())
			return brief;

		return gm.GetFacts().Fill(brief);
	}

	//------------------------------------------------------------------------------------------------
	//! The world called it off (the cure came out, the post it served changed hands): it ends at
	//! the next tick with `why` told to everyone it was offered to, and nobody is paid, blamed or
	//! penalised. Its arc or questline hears it as a failure.
	void Withdraw(string why)
	{
		if (m_eState != RZ_EMissionState.ACTIVE)
			return;

		m_bWithdrawn = true;
		m_sWithdrawText = why;
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	//! Reserve this amends job for one player for `seconds` (a parley's offer): theirs alone, though
	//! they're Hunted, paid as amends (Infamy worked off through the forgiveness budget).
	void Reserve(RZ_GameModeComponent gm, int playerId, float seconds)
	{
		m_sReservedFor = gm.GetProfile(playerId).m_sIdentity;
		Hire(playerId);
		m_bAmends = true;
		m_fTimeLeft = Math.Min(m_fTimeLeft, seconds);
	}

	//------------------------------------------------------------------------------------------------
	//! Why this player can't take it, said short for a locked reply on the talk screen ("Needs
	//! Known with Halcyon"); "" when they can (docs/DIALOGUE.md, "Jobs on the first replies"). A
	//! mission with its own rule in IsEligible says it here too.
	string LockReason(RZ_GameModeComponent gm, int playerId)
	{
		if (IsEligible(gm, playerId))
			return "";

		if (!m_sReservedFor.IsEmpty())
			return "Kept for someone else";

		string need = m_Req.NeedFor(gm.GetProfile(playerId));
		if (!need.IsEmpty())
			return need;

		if (m_bInPersonOnly && m_iLeaderId >= 0)
			return "They won't give it to you";

		return "Not for you now";
	}

	//------------------------------------------------------------------------------------------------
	//! What one of this mission's people's choices (`choice`) or uses needs that this player hasn't
	//! got, short, for its locked reply on the talk screen ("Needs Known with Halcyon", "After dark");
	//! "" when they can take it (docs/DIALOGUE.md, "Jobs on the first replies"). The mission's own
	//! handler still checks it when picked (the object menu shows no lock).
	string ChoiceLock(RZ_GameModeComponent gm, int playerId, int objIdx, bool choice, int slot)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! "Needs Known with Halcyon" for an access rank (0 not Hunted, 1 Known, 2 Trusted, 3 Champion)
	//! this player is below with `faction`; "" when they have it.
	static string RankNeed(RZ_GameModeComponent gm, int playerId, int faction, int rank)
	{
		if (!RZ_Factions.IsValid(faction))
			return "";

		return ScoreNeed(RZ_Reputation.GetAccessRank(gm.GetTier(playerId, faction)), rank, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! RankNeed for a score a mission works out itself (a rank with its background bonus): "" when
	//! `have` reaches `rank`.
	static string ScoreNeed(int have, int rank, int faction)
	{
		if (have >= rank)
			return "";

		if (rank <= 0)
			return "Not while Hunted by " + RZ_Factions.GetShortName(faction);

		string word = "Known";
		if (rank == 2)
			word = "Trusted";
		else if (rank >= 3)
			word = "Champion";

		return string.Format("Needs %1 with %2", word, RZ_Factions.GetShortName(faction));
	}

	//------------------------------------------------------------------------------------------------
	//! The word a reply that starts it, and its start and end cards, put in front (docs/WRITING.md,
	//! "Tag words"): Quest for a questline's or a major's stage, Event for a world event, else Job.
	string KindWord()
	{
		if (m_iQuestRun >= 0)
			return "Quest";

		if (!m_bContract)
			return "Event";

		return "Job";
	}

	//------------------------------------------------------------------------------------------------
	//! Contracts: does this player meet the requirements? World events are open to everyone.
	bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bContract)
			return true;

		if (!m_sReservedFor.IsEmpty())
			return playerId > 0 && gm.GetProfile(playerId).m_sIdentity == m_sReservedFor;

		if (!m_Req.IsEligible(gm, playerId))
			return m_Mirror && m_Mirror.IsSide(gm, playerId);

		// A leader's personal job: the leader decides who gets it (a Paranoid one sulks after a lie)
		if (m_bInPersonOnly && m_iLeaderId >= 0 && gm.GetLeaders())
			return !RZ_StoryKit.Sulks(gm, m_iLeaderId, playerId) && gm.GetLeaders().CanWorkFor(m_iLeaderId, playerId);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Offered to this player in person (docs/MISSIONS.md, "Who gets offered what"): by its giver or
	//! a thing of its. The one way a job becomes a player's. False for no player.
	bool Hire(int playerId)
	{
		if (playerId <= 0 || m_aOfferedTo.Contains(playerId))
			return false;

		m_aOfferedTo.Insert(playerId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Is this job this player's (their Quests tab): hired for it, reserved for them, or a stage of a
	//! questline they took part in (the story they took on goes on).
	bool IsOfferedTo(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId <= 0)
			return false;

		if (m_aOfferedTo.Contains(playerId))
			return true;

		if (!m_sReservedFor.IsEmpty())
			return gm.GetProfile(playerId).m_sIdentity == m_sReservedFor;

		if (m_iQuestRun >= 0 && gm.GetQuestlines())
		{
			RZ_QuestRun run = gm.GetQuestlines().FindRun(m_iQuestRun);
			if (run && run.PlayerTookPartThisLife(gm, playerId))
				return true;

			// A finale route's stage: the players locked to that route (their desk was the asking)
			RZ_CureFinale finale = RZ_CureFinale.Get();
			if (run && finale && run.m_Line && run.m_Line.IsCureFinale() && finale.RouteOfPlayer(playerId) == RZ_CureFinale.RouteOfKey(run.m_sKey))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! May this player be paid for it (with IsEligible)? A world event: anyone. A contract: whoever
	//! it was offered to, and anyone in a hired player's vanilla group.
	bool IsHired(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bContract || IsOfferedTo(gm, playerId))
			return true;

		foreach (int other : m_aOfferedTo)
		{
			if (other != playerId && RZ_Util.SameGroup(other, playerId))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! ★ background for this mission?
	bool HasBonus(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId <= 0)
			return false;

		return m_Req.IsBonus(gm.GetProfile(playerId).m_sBackgroundId);
	}

	//------------------------------------------------------------------------------------------------
	//! True if an eligible player with a ★ background is within radius of pos.
	bool AnyBonusNear(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players, vector pos, float radius)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius && HasBonus(gm, ids[i]) && IsEligible(gm, ids[i]))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool AnyEligible(RZ_GameModeComponent gm, array<int> ids)
	{
		foreach (int playerId : ids)
		{
			if (IsEligible(gm, playerId))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Update(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fTimeLeft -= dt;

		if (m_fParticipationRadius > 0)
		{
			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) <= m_fParticipationRadius && !m_aParticipants.Contains(ids[i]))
					m_aParticipants.Insert(ids[i]);
			}
		}

		RevealNear(gm, ids, players);
		if (m_Story)
			m_Story.Update(this, gm, dt, ids, players);

		if (m_Job)
			m_Job.Tick(gm, this, dt, ids, players);

		if (m_Mirror)
			m_Mirror.Tick(gm, this, dt, ids);

		OnUpdate(gm, dt, ids, players);

		// One real choice: the work is done, so the question comes before it ends
		if (m_Choice && !m_Choice.m_bDone && m_eState != RZ_EMissionState.FAILED && AsksChoice())
		{
			bool workDone = m_eState == RZ_EMissionState.SUCCEEDED;
			if (m_Choice.m_bAsking)
			{
				if (workDone && m_Choice.m_bEnds)
					m_eState = RZ_EMissionState.ACTIVE;

				m_Choice.Tick(gm, this, dt);
			}
			else if (workDone && !m_Choice.m_bManual)
			{
				m_eState = RZ_EMissionState.ACTIVE;
				m_Choice.Ask(gm, this, ChoiceAt());
			}
		}

		if (m_eState == RZ_EMissionState.ACTIVE && m_fTimeLeft <= 0)
			OnTimeUp(gm);
	}

	// ================================================================ Rough areas
	//------------------------------------------------------------------------------------------------
	//! The rough-area radius for this mission, 0 if it gives exact positions.
	protected float RoughRadius(RZ_GameModeComponent gm)
	{
		if (!m_bContract || m_bGlobal || NeedsExactLocation())
			return 0;

		RZ_ContractSettings cs = gm.GetContractSettings();
		if (!cs)
			return 0;

		return Math.Max(0, cs.m_fRoughRadius);
	}

	//------------------------------------------------------------------------------------------------
	protected float RevealDistance(RZ_GameModeComponent gm)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (!cs)
			return 500;

		return Math.Max(50, cs.m_fRevealDistance);
	}

	//------------------------------------------------------------------------------------------------
	//! Who knows only the rough area, whatever their distance: everyone but Radio Operators,
	//! players Trusted (or more) with the giver, and those who have been close.
	protected bool RoughByStanding(RZ_GameModeComponent gm, int playerId)
	{
		if (RoughRadius(gm) <= 0)
			return false;

		if (playerId <= 0)
			return true;

		if (m_aExactFor.Contains(playerId))
			return false;

		RZ_Background bg = gm.GetPlayerBackground(playerId);
		if (bg && bg.m_bRadioIntel)
			return false;

		RZ_ETier tier = gm.GetTier(playerId, m_Req.m_eGiver);
		return !RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED) || RZ_Reputation.IsHunted(tier);
	}

	//------------------------------------------------------------------------------------------------
	//! Does this player, standing at `from`, get only the rough area?
	bool IsRoughFor(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (!RoughByStanding(gm, playerId))
			return false;

		return from == vector.Zero || RZ_Util.DistXZ(from, m_vPos) > RevealDistance(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! The rough circle's centre: the real spot moved a random 30-70% of the radius, once.
	vector GetRoughCenter(RZ_GameModeComponent gm)
	{
		if (!m_bRoughReady)
		{
			m_bRoughReady = true;
			float r = RoughRadius(gm);
			float angle = Math.RandomFloat(0, Math.PI2);
			float dist = Math.RandomFloat(r * 0.3, r * 0.7);
			m_vRoughOffset = Vector(Math.Cos(angle) * dist, 0, Math.Sin(angle) * dist);
		}

		return m_vPos + m_vRoughOffset;
	}

	//------------------------------------------------------------------------------------------------
	//! "Somewhere around Morton: within 500 m of a point 1.2 km NE".
	string GetRoughWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = string.Format("Somewhere %1: within %2 m of a point", GetRoughAreaName(gm), RZ_Util.RoundToInt(RoughRadius(gm)));
		if (from != vector.Zero)
			text += " " + RZ_Util.DirectionText(from, GetRoughCenter(gm));
		else
			text += " at grid " + RZ_SettlementScanner.GridReference(GetRoughCenter(gm));

		return text + string.Format(". You'll know the spot within %1 m.", RZ_Util.RoundToInt(RevealDistance(gm)));
	}

	//------------------------------------------------------------------------------------------------
	//! "around Morton", or "near grid 045-112" without a settlement.
	string GetRoughAreaName(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			return "around " + s.m_sName;

		return "near grid " + RZ_SettlementScanner.GridReference(GetRoughCenter(gm));
	}

	//------------------------------------------------------------------------------------------------
	//! Where text for one player: the rough area, or the full text (GetWhereText) once they know
	//! the spot. Every brief, radio list and journal entry for a player goes through here.
	string GetWhereTextFor(RZ_GameModeComponent gm, vector from, int playerId)
	{
		if (IsRoughFor(gm, playerId, from))
			return GetRoughWhereText(gm, from);

		if (from == vector.Zero)
			return GetLocationName(gm);

		return GetWhereText(gm, from);
	}

	//------------------------------------------------------------------------------------------------
	//! The place's name for one player (the journal's "where"): "around Morton" while rough.
	string GetLocationNameFor(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (IsRoughFor(gm, playerId, from))
			return GetRoughAreaName(gm);

		return GetLocationName(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! Distance and direction for one player (the journal and the guidance): to the rough
	//! circle's centre while rough, "" without a position.
	string GetDirectionFor(RZ_GameModeComponent gm, vector from, int playerId)
	{
		if (m_bGlobal || from == vector.Zero)
			return "";

		if (IsRoughFor(gm, playerId, from))
			return "about " + RZ_Util.DirectionText(from, GetRoughCenter(gm));

		return RZ_Util.DirectionText(from, m_vPos);
	}

	// ================================================================ Objectives
	// docs/MISSIONS.md, "Objectives": what the tracker and the Quests tab show. Every mission names
	// its ask (m_sObjective, set beside its brief) or declares its steps (ObjSteps, ObjStepDone,
	// ObjReveal); the default is get to the place, then the ask. An objective stands on its own: who,
	// what and where in one short line (docs/WRITING.md, "Objective lines"; voice_lint checks it).
	string m_sObjective;					// the ask in one line ("Hold the cellar cache in Levie for a minute."); world tokens filled when shown
	string m_sObjThen;					// a second place's step after it ("Bring it to the lab."), its own line (ObjThen)
	protected ref array<string> m_aObjSteps;	// declared steps, in order (ObjSteps)
	protected ref array<bool> m_aObjOptional;
	protected int m_iObjStep;					// the current step
	protected int m_iObjKnown = 1;		// steps the players have learned of (the current one and any revealed)

	//------------------------------------------------------------------------------------------------
	//! Declares the steps, in order. Only the current one is known until ObjStepDone or ObjReveal.
	protected void ObjSteps(string a, string b = "", string c = "", string d = "", string e = "")
	{
		m_aObjSteps = {};
		m_aObjOptional = {};
		array<string> all = {a, b, c, d, e};
		foreach (string st : all)
		{
			if (st.IsEmpty())
				continue;

			m_aObjSteps.Insert(st);
			m_aObjOptional.Insert(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A job done at two places (find the thing, then bring it somewhere): two objective lines, one
	//! action and one place each (docs/MISSION_RULES.md, rule 23). The first is current until
	//! ObjFirstDone says it's done; then the second, pointed where ObjectivePlace points then.
	protected void ObjThen(string first, string then)
	{
		m_sObjective = first;
		m_sObjThen = then;
	}

	//------------------------------------------------------------------------------------------------
	//! The first of ObjThen's two lines is done (a delivery's thing is carried, a captive caged...).
	protected bool ObjFirstDone()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! An optional step after the declared ones: "(Optional)" in the list once it's known.
	protected void ObjOptional(string text)
	{
		if (!m_aObjSteps)
		{
			m_aObjSteps = {};
			m_aObjOptional = {};
		}

		m_aObjSteps.Insert(text);
		m_aObjOptional.Insert(true);
	}

	//------------------------------------------------------------------------------------------------
	//! The current step is done; the next one becomes current (and known).
	void ObjStepDone()
	{
		m_iObjStep++;
		m_iObjKnown = Math.Max(m_iObjKnown, m_iObjStep + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The players have learned of the steps up to `count` (a clue read, a person talked to).
	void ObjReveal(int count)
	{
		m_iObjKnown = Math.Max(m_iObjKnown, count);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the job points this player: the rough circle's centre while they only know the area.
	vector GetAreaFor(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_bGlobal)
			return vector.Zero;

		if (IsRoughFor(gm, playerId, from))
			return GetRoughCenter(gm);

		return m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	//! The map's ring for the tracked job: the rough circle's radius, 0 for an exact place.
	float GetAreaRadius(RZ_GameModeComponent gm)
	{
		return RoughRadius(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! The mission's own place as an objective's place (docs/UI.md, "The quest tracker"): the search
	//! area while this player knows only the rough one, else the spot, reached within the radius it
	//! counts you as there at. Null for a mission with no place (the whole Zone).
	RZ_ObjPlace AreaPlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_bGlobal || m_vPos == vector.Zero)
			return null;

		if (IsRoughFor(gm, playerId, from))
			return RZ_ObjPlace.Area(GetRoughCenter(gm), RoughRadius(gm));

		return RZ_ObjPlace.Spot(m_vPos, m_fParticipationRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! Where this player's current objective is done now (docs/MISSIONS.md, "Objectives"): the
	//! tracker's arrow, the map's diamond and the Quests tab follow it. Default: the mission's own
	//! place (AreaPlace). A mission whose work moves (a delivery's drop once the thing is carried,
	//! the next point, an escort's destination, the next site) overrides it; keep the rough rule
	//! (AreaPlace while IsRoughFor) for anything the player hasn't been told the spot of. Null: no
	//! place of its own (the job's place is used).
	RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		// A step chain's current step (Missions/Blocks/RZ_StepChain.c)
		RZ_StepChain chain = RZ_StepChain.ForMission(m_iId);
		if (chain && !chain.IsDone())
		{
			RZ_ObjPlace step = chain.NowPlace();
			if (step)
				return step;
		}

		return AreaPlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	//! Where "Get to <place>" is done: the mission's place (it counts you within its radius). With
	//! no radius (a leader's job) the work itself counts you, so it's where the work is. A delivery
	//! points at the thing (picking it up counts you).
	RZ_ObjPlace GetTherePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_fParticipationRadius <= 0)
		{
			RZ_ObjPlace work = ObjectivePlace(gm, playerId, from);
			if (work)
				return work;
		}

		return AreaPlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	//! A thing to fetch and take somewhere, as an objective's place: where it lies (or `lastSeen`
	//! once it's gone), then `dest` once someone carries it. Null without either.
	static RZ_ObjPlace CarryPlace(IEntity item, vector lastSeen, vector dest, float destRadius = 0)
	{
		bool carried = item && item.GetRootParent() && item.GetRootParent() != item;
		if (carried && dest != vector.Zero)
			return RZ_ObjPlace.Spot(dest, destRadius);

		if (item && !carried)
			return RZ_ObjPlace.Spot(item.GetOrigin());

		if (lastSeen != vector.Zero)
			return RZ_ObjPlace.Spot(lastSeen);

		return RZ_ObjPlace.Spot(dest, destRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! A declared step's place (ObjSteps): the current one's is ObjectivePlace; the others none
	//! unless the mission says.
	RZ_ObjPlace StepPlace(RZ_GameModeComponent gm, int playerId, vector from, int step)
	{
		if (step == m_iObjStep)
			return ObjectivePlace(gm, playerId, from);

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The ask in one line: m_sObjective with the world's tokens ({lab_town}, {leader:id}...) filled
	//! in now, so a name in it is never stale; the title if a mission names none.
	string ObjectiveText(RZ_GameModeComponent gm)
	{
		if (m_sObjective.IsEmpty())
			return m_sTitle;

		if (!gm || !gm.GetFacts())
			return m_sObjective;

		return gm.GetFacts().Fill(m_sObjective);
	}

	//------------------------------------------------------------------------------------------------
	//! This mission's objectives for one player, only what they know: the declared steps (done,
	//! current, the known next ones), or by default "get there" until they're taking part, then
	//! the ask.
	void GetObjectives(RZ_GameModeComponent gm, int playerId, vector from, notnull RZ_ObjList list)
	{
		// A step chain across towns (Missions/Blocks/RZ_StepChain.c): the steps reached, the current one
		RZ_StepChain chain = RZ_StepChain.ForMission(m_iId);
		if (chain && chain.Count() > 0)
		{
			chain.AddObjectives(list);
			return;
		}

		if (m_aObjSteps && !m_aObjSteps.IsEmpty())
		{
			foreach (int i, string step : m_aObjSteps)
			{
				if (i >= m_iObjKnown && i > m_iObjStep)
					break;

				int state = RZ_EObjState.NEXT;
				if (i < m_iObjStep)
					state = RZ_EObjState.DONE;
				else if (i == m_iObjStep)
					state = RZ_EObjState.CURRENT;

				if (state != RZ_EObjState.DONE && i < m_aObjOptional.Count() && m_aObjOptional[i])
					state = RZ_EObjState.OPTIONAL;

				list.Add(step, state, StepPlace(gm, playerId, from, i));
			}

			return;
		}

		// The ask is done where the work is now (ObjectivePlace)
		string ask = ObjectiveText(gm);
		RZ_ObjPlace work = ObjectivePlace(gm, playerId, from);
		bool there = m_bGlobal || m_aParticipants.Contains(playerId);

		// Two places (ObjThen): the first line, then the second where the work moves to
		string then = "";
		if (!m_sObjThen.IsEmpty())
		{
			then = m_sObjThen;
			if (gm && gm.GetFacts())
				then = gm.GetFacts().Fill(m_sObjThen);

			if (ObjFirstDone())
			{
				list.Add(ask, RZ_EObjState.DONE, null);
				list.Add(then, RZ_EObjState.CURRENT, work);
				return;
			}
		}

		if (there)
		{
			list.Add(ask, RZ_EObjState.CURRENT, work);
			if (!then.IsEmpty())
				list.Add(then, RZ_EObjState.NEXT, null);

			return;
		}

		RZ_ObjPlace getThere = GetTherePlace(gm, playerId, from);
		string place = GetLocationName(gm);
		if (IsRoughFor(gm, playerId, from))
			list.Add("Search " + GetRoughAreaName(gm) + ".", RZ_EObjState.CURRENT, getThere);
		else if (!place.IsEmpty())
			list.Add("Get to " + place + ".", RZ_EObjState.CURRENT, getThere);

		list.Add(ask, RZ_EObjState.NEXT, work);
		if (!then.IsEmpty())
			list.Add(then, RZ_EObjState.NEXT, null);
	}

	//------------------------------------------------------------------------------------------------
	//! Players who were only given the rough area and come within the reveal distance learn the
	//! spot (and are told, if the contract was offered to them).
	protected void RevealNear(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		if (RoughRadius(gm) <= 0)
			return;

		float reveal = RevealDistance(gm);
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aExactFor.Contains(playerId) || RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > reveal)
				continue;

			bool wasRough = RoughByStanding(gm, playerId);
			m_aExactFor.Insert(playerId);
			if (wasRough && m_aOfferedTo.Contains(playerId))
				gm.NotifyPlayer(playerId, m_sTitle, "This is the place. " + RZ_Util.Capitalize(GetWhereText(gm, p.GetOrigin())));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Default: running out of time is a failure. Survival events override this to succeed.
	protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! A mission item was handed in at a drop point. Return true if this mission takes it.
	bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to"/"Read" on a person or thing a non-puzzle mission tagged with RZ_ClueComponent
	//! (e.g. the patient in Medicine Before Dark). True if handled.
	bool HandleTalk(RZ_GameModeComponent gm, int playerId, int clueIdx)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A choice slot on a person or thing this (non-puzzle) mission tagged with RZ_PuzzleObjectComponent.
	void HandleObjectChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option)
	{
		if (m_Choice && objIdx == RZ_OneChoice.OBJECT_INDEX)
			m_Choice.Handle(gm, this, playerId, option);

		if (m_Job && m_Job.m_Scene && objIdx == RZ_TalkScene.OBJECT_INDEX)
			m_Job.m_Scene.Handle(gm, this, playerId, option);
	}

	//------------------------------------------------------------------------------------------------
	//! A use slot on a person or thing this (non-puzzle) mission tagged: the one-choice stand-in's
	//! "Leave it to them" (Leave: no pay, no blame).
	void HandleObjectUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value)
	{
		if (m_Choice && objIdx == RZ_OneChoice.OBJECT_INDEX)
		{
			// Leave it to them; Object and Stand with while a hold runs
			if (value == 0)
				RZ_StoryKit.Leave(gm, this, playerId);
			else if (value == 1 && m_Story)
				m_Story.m_Deciding.Object(gm, this, playerId);
			else if (value == 2 && m_Story)
				m_Story.m_Deciding.StandWith(playerId);
		}

		if (m_Job && m_Job.m_Scene && objIdx == RZ_TalkScene.OBJECT_INDEX)
			m_Job.m_Scene.Use(gm, this, playerId, value);
	}

	//------------------------------------------------------------------------------------------------
	//! A player died (or turned) at pos.
	void OnPlayerDied(RZ_GameModeComponent gm, int playerId, vector pos)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! A supply truck arrived or was lost (RZ_SupplyLines.TRUCK_*). driver: the player who drove
	//! it in or away, -1 if none; for TRUCK_CALLED_OFF (its post changed hands or was abandoned,
	//! before or after it left), the id of that holding. Convoy missions
	//! (Missions/Blocks/RZ_VehicleZone.c) use it.
	void OnTruckEvent(RZ_GameModeComponent gm, int faction, int what, vector pos, int driver)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnSuccess(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnFailed(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Extra per-player reward after Fame/Favor were paid (only eligible participants).
	void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Where this mission has props, items, people or crates on the ground. No building within
	//! RZ_MissionManager.PROP_CLEAR of one has a lock while it runs (docs/DESIGN.md, "Locks and
	//! keys"), so nothing a mission put down ends up behind a door. Default: its position.
	void GetPropSpots(notnull array<vector> result)
	{
		if (!m_bGlobal && m_vPos != vector.Zero)
			result.Insert(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! A building whose lock is part of this mission (Locked Bunker's keypad): it keeps its lock
	//! even with the mission's props beside it.
	bool KeepsLock(string buildingKey)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Delete whatever the mission spawned. Runs after OnSuccess/OnFailed.
	void Cleanup(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("%1 (%2) is done.", m_sTitle, GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("%1 (%2): the window has closed.", m_sTitle, GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	//! "Supply cache: Morton (1.2 km NE)" for the player at `from`.
	string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		if (m_bGlobal)
			return GetLocationName(gm);

		return GetLocationName(gm) + ", " + RZ_Util.DirectionText(from, m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Listed in the notebook's Radio tab under Missions (a contract) or World (a world event).
	void NotifyParticipants(RZ_GameModeComponent gm, string title, string text)
	{
		RZ_ENoticeTag tag = NoticeTag();
		foreach (int playerId : m_aParticipants)
		{
			gm.NotifyPlayerAs(playerId, title, text, tag);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What this mission's notices are (docs/UI.md, "Notices").
	RZ_ENoticeTag NoticeTag()
	{
		if (m_bContract)
			return RZ_ENoticeTag.MISSION;

		return RZ_ENoticeTag.EVENT;
	}

	//------------------------------------------------------------------------------------------------
	//! Contracts tell the players they were offered to; world events tell everyone.
	void NotifyInterested(RZ_GameModeComponent gm, string title, string text)
	{
		if (!m_bContract)
		{
			gm.NotifyAllAs(title, text, RZ_ENoticeTag.EVENT);
			return;
		}

		foreach (int playerId : m_aOfferedTo)
		{
			gm.NotifyPlayerAs(playerId, title, text, RZ_ENoticeTag.MISSION);
		}

		foreach (int playerId2 : m_aParticipants)
		{
			if (!m_aOfferedTo.Contains(playerId2))
				gm.NotifyPlayerAs(playerId2, title, text, RZ_ENoticeTag.MISSION);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The deed phrase for the giver's best deed (docs/REPUTATION.md, "Deeds and tallies"): the
	//! contract's own, else its class's (RZ_RepDeeds), else "did a job for us near {town}"; {town} is
	//! the nearest named settlement.
	string DeedPhrase(RZ_GameModeComponent gm)
	{
		string phrase = m_sDeed;
		if (phrase.IsEmpty())
			phrase = RZ_RepDeeds.ForMission(this);

		if (phrase.IsEmpty())
			phrase = "did a job for us near {town}";

		string town = "the roads";
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			s = gm.GetSettlement(gm.FindNearestSettlement(m_vPos));

		if (s)
			town = s.m_sName;

		phrase.Replace("{town}", town);
		return phrase;
	}

	//------------------------------------------------------------------------------------------------
	string GetLocationName(RZ_GameModeComponent gm)
	{
		if (m_bGlobal)
			return "everywhere";

		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			return s.m_sName;

		return "grid " + RZ_SettlementScanner.GridReference(m_vPos);
	}

	// ================================================================ Helpers for mission types
	//------------------------------------------------------------------------------------------------
	//! Random settlement whose infection is within [minInf, maxInf] that no mission or story is
	//! using (with `asker`, what it or its story holds counts as free). -1 if none. To filter by
	//! who holds a town, RZ_WorldFacts.PickTown. A town at the quarantine line is taken only when
	//! there's no other: a job there has its ground in the warning zone.
	static int PickSettlement(RZ_GameModeComponent gm, float minInf, float maxInf, RZ_Mission asker = null)
	{
		array<int> candidates = {};
		array<int> atLine = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection < minInf || s.m_fInfection > maxInf || s.m_bIsland || gm.GetMissions().IsSettlementBusy(i, asker))
				continue;

			if (RZ_Perimeter.IsNearLineAt(s.m_vCenter))
				atLine.Insert(i);
			else
				candidates.Insert(i);
		}

		RZ_OfferReach.NarrowTowns(gm, candidates);
		if (!candidates.IsEmpty())
			return candidates.GetRandomElement();

		RZ_OfferReach.NarrowTowns(gm, atLine);
		if (atLine.IsEmpty())
			return -1;

		return atLine.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Random settlement between minDist and maxDist from `from`, within the infection range, other
	//! than `exclude`, not busy (as PickSettlement, with `asker`). -1 if none.
	static int PickSettlementAway(RZ_GameModeComponent gm, vector from, float minDist, float maxDist, float minInf, float maxInf, int exclude = -1, RZ_Mission asker = null)
	{
		array<int> candidates = {};
		array<int> atLine = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i == exclude)
				continue;

			float d = RZ_Util.DistXZ(from, s.m_vCenter);
			if (d < minDist || d > maxDist)
				continue;

			if (s.m_fInfection < minInf || s.m_fInfection > maxInf || s.m_bIsland || gm.GetMissions().IsSettlementBusy(i, asker))
				continue;

			if (RZ_Perimeter.IsNearLineAt(s.m_vCenter))
				atLine.Insert(i);
			else
				candidates.Insert(i);
		}

		RZ_OfferReach.NarrowTowns(gm, candidates);
		if (!candidates.IsEmpty())
			return candidates.GetRandomElement();

		RZ_OfferReach.NarrowTowns(gm, atLine);
		if (atLine.IsEmpty())
			return -1;

		return atLine.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Town pickers by who holds a town (docs/DESIGN.md, "World facts"): RZ_WorldFacts.PickTown
	//! with the infection range, not busy (with `asker`, what it or its story holds is free).
	//! A town nobody lives in: no safehouse, holding or lab. For work in an abandoned or overrun
	//! town (a nest, a cache, a ruin to clear). -1 if none.
	static int PickEmptySettlement(RZ_GameModeComponent gm, float minInf, float maxInf, RZ_Mission asker = null)
	{
		RZ_TownFilter f = TownFilter(minInf, maxInf);
		f.m_iHeldBy = -1;
		return gm.GetFacts().PickTown(f, asker);
	}

	//------------------------------------------------------------------------------------------------
	//! As PickEmptySettlement, between minDist and maxDist from `from`, other than `exclude`.
	static int PickEmptySettlementAway(RZ_GameModeComponent gm, vector from, float minDist, float maxDist, float minInf, float maxInf, int exclude = -1, RZ_Mission asker = null)
	{
		RZ_TownFilter f = TownFilterAway(from, minDist, maxDist, minInf, maxInf, exclude);
		f.m_iHeldBy = -1;
		return gm.GetFacts().PickTown(f, asker);
	}

	//------------------------------------------------------------------------------------------------
	//! A town that none of `notHeldBy` holds (RZ_Factions.Bit of each; a town nobody holds is fine).
	//! For work a faction does in the open, never in its rivals' towns. -1 if none.
	static int PickSettlementNotHeldBy(RZ_GameModeComponent gm, float minInf, float maxInf, int notHeldBy, RZ_Mission asker = null)
	{
		RZ_TownFilter f = TownFilter(minInf, maxInf);
		f.m_iNotHeldBy = notHeldBy;
		return gm.GetFacts().PickTown(f, asker);
	}

	//------------------------------------------------------------------------------------------------
	//! As PickSettlementNotHeldBy, between minDist and maxDist from `from`, other than `exclude`.
	static int PickSettlementAwayNotHeldBy(RZ_GameModeComponent gm, vector from, float minDist, float maxDist, float minInf, float maxInf, int notHeldBy, int exclude = -1, RZ_Mission asker = null)
	{
		RZ_TownFilter f = TownFilterAway(from, minDist, maxDist, minInf, maxInf, exclude);
		f.m_iNotHeldBy = notHeldBy;
		return gm.GetFacts().PickTown(f, asker);
	}

	//------------------------------------------------------------------------------------------------
	//! A town `faction` holds (a stronghold, an outpost, a Haven safehouse, the lab), between minDist
	//! and maxDist from `from` (maxDist 0: anywhere), other than `exclude`. For a drop, a fence or a
	//! post that should be where that faction really is. -1 if none: fall back to another picker.
	static int PickHeldSettlementAway(RZ_GameModeComponent gm, RZ_EFaction faction, vector from, float minDist, float maxDist, int exclude = -1, RZ_Mission asker = null)
	{
		RZ_TownFilter f = TownFilterAway(from, minDist, maxDist, 0, 100, exclude);
		f.m_iHeldBy = faction;
		return gm.GetFacts().PickTown(f, asker);
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_TownFilter TownFilter(float minInf, float maxInf)
	{
		RZ_TownFilter f = new RZ_TownFilter();
		f.m_fMinInfection = minInf;
		f.m_fMaxInfection = maxInf;
		return f;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_TownFilter TownFilterAway(vector from, float minDist, float maxDist, float minInf, float maxInf, int exclude)
	{
		RZ_TownFilter f = TownFilter(minInf, maxInf);
		f.m_vFrom = from;
		f.m_fMinDist = minDist;
		f.m_fMaxDist = maxDist;
		f.m_iExclude = exclude;
		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! The factions' bits for RZ_TownFilter.m_iNotHeldBy: everyone but `faction` (who may hold it).
	static int OthersBits(RZ_EFaction faction)
	{
		int bits = 0;
		for (int i = 0; i < RZ_Factions.COUNT; i++)
		{
			if (i != faction)
				bits = bits | RZ_Factions.Bit(i);
		}

		return bits;
	}

	//------------------------------------------------------------------------------------------------
	//! Least infected settlement (a likely Haven safehouse). -1 if none.
	static int PickSafestSettlement(RZ_GameModeComponent gm, int exclude = -1)
	{
		int best = -1;
		float bestInf = float.MAX;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i == exclude)
				continue;

			if (s.m_fInfection < bestInf)
			{
				bestInf = s.m_fInfection;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Highest of a few sampled dry points in a ring, never behind a lock. Works on any terrain.
	static vector HighPointNear(vector center, float minR, float maxR, int samples = 16)
	{
		vector best = LandPointNear(center, minR, maxR);
		for (int i = 0; i < samples; i++)
		{
			vector p = RZ_Util.RandomPointInRing(center, minR, maxR);
			if (RZ_Util.IsWater(p) || p[1] <= best[1])
				continue;

			if (!RZ_LockedSpots.IsBlocked(p))
				best = p;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A dry point in a ring, away from the quarantine line and never inside a locked building or
	//! by its doorway blocker (RZ_LockedSpots), so mission props are never behind a lock. Falls
	//! back to the first dry point moved out past the locked building's wall, then the last try.
	static vector LandPointNear(vector center, float minR, float maxR)
	{
		vector p = RZ_Util.RandomPointInRing(center, minR, maxR);
		vector dry = p;
		bool haveDry = false;
		for (int i = 0; i < 10; i++)
		{
			if (!RZ_Util.IsWater(p) && !RZ_Perimeter.IsNearLineAt(p))
			{
				if (!RZ_LockedSpots.IsBlocked(p))
					return p;

				if (!haveDry)
				{
					dry = p;
					haveDry = true;
				}
			}

			p = RZ_Util.RandomPointInRing(center, minR, maxR);
		}

		if (haveDry)
			return RZ_LockedSpots.MoveOut(dry);

		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! Zombies tagged with this mission, so they don't despawn while it runs.
	void SpawnGuards(RZ_GameModeComponent gm, vector center, int count, float minR, float maxR, bool special = false)
	{
		for (int i = 0; i < count; i++)
		{
			gm.GetSpawner().SpawnZombie(LandPointNear(center, minR, maxR), special, m_iSettlement, m_iId);
		}
	}

	//------------------------------------------------------------------------------------------------
	static int CountPlayersNear(array<IEntity> players, vector pos, float radius)
	{
		int n = 0;
		foreach (IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Player ids within radius of pos.
	static void GetPlayersNear(array<int> ids, array<IEntity> players, vector pos, float radius, notnull array<int> result)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				result.Insert(ids[i]);
		}
	}
}
