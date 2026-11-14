//! The case (docs/DESIGN.md, "The case"): the opening's last step and the main story's door.
//!
//! An opening's chain ends at a runner who fell short of the line (RZ_Openings, stage CASE): a body
//! by a road on the edge of town, and Halcyon's cold case lying by him (RZ_ColdCaseComponent: its
//! tag, "HALCYON. FIELD STATION. LIVE. KEEP COLD." and a date three weeks before Day 1, from the
//! season's canon). Taking it is the step. Then one lead, tracked: Halcyon's field station by name
//! (the nearest Halcyon holding, else the lab). Whoever carries a case gets a reply at Halcyon's
//! people (hand it over), the Red Hand's (sell it) and Bastion's (hand it in): only picking it
//! hands the case over, never walking up. Delivered to Halcyon it opens Proof for this character
//! (RZ_QuestRun.Did) and, before 25% cure, a stage of its own, The Date on the Tag (the clinic's
//! first file); the first live case of a season counts as live tissue for Halcyon's route
//! (RZ_CureFinale, the "case.tissue" season flag). Sold, Proof opens as Get It Back; handed to
//! Bastion, as The Case at the Post: each a stage that takes it back from their stores. Kept, it
//! stays the lead, and Halcyon asks after it on its channel once a day.
//!
//! The case is a real item, but a quest item stays yours: when its carrier dies it's taken off the
//! body and kept as data (RZ_PlayerProfile.m_bCaseHeld, with how warm it was), and the respawned
//! character has it in their pack again. Anyone else who picks one up has the lead. It warms over Cold case minutes in the open (half as fast by
//! night); warm, it still counts, and the station says so. Server only.

//------------------------------------------------------------------------------------------------
//! A handed step of the case's own: the lead, The Date on the Tag, Get It Back, The Case at the Post.
class RZ_CaseStepDef : RZ_ErrandDef
{
	int m_iKind;		// RZ_OpeningCase.KIND_*

	//------------------------------------------------------------------------------------------------
	void RZ_CaseStepDef(string key, string title, int kind)
	{
		m_sKey = key;
		m_sTitle = title;
		m_iKind = kind;
		m_sFamily = "opening";
		m_bHanded = true;
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		Over("It can wait. It's waited three weeks already.");
	}

	//------------------------------------------------------------------------------------------------
	//! Handed, not asked for: tracked at once (RZ_ErrandDef.IsOpening).
	override bool IsOpening()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool StepCounts(RZ_Errands dealer, RZ_Errand e, IEntity me)
	{
		RZ_OpeningCase c = RZ_OpeningCase.Get();
		return c && c.StepCounts(this, e, me);
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_OpeningCase c = RZ_OpeningCase.Get();
		if (c)
			c.OnStepDone(this, e, playerId, said);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningCase
{
	static const int KIND_LEAD = 1;		// take it to the station: done only by the hand-over reply
	static const int KIND_DATE = 2;		// The Date on the Tag: the clinic's first file
	static const int KIND_BACK = 3;		// Get It Back: from the Red Hand's lock-up
	static const int KIND_POST = 4;		// The Case at the Post: from Bastion's stores

	static const int STATE_NONE = 0;
	static const int STATE_CARRIED = 1;	// the lead is on: take it to the station
	static const int STATE_HALCYON = 2;
	static const int STATE_SOLD = 3;
	static const int STATE_BASTION = 4;

	// The talk screen's replies (RZ_Dialogue): ids free in every speaker's list
	static const int REPLY_HALCYON = 50;
	static const int REPLY_SELL = 51;
	static const int REPLY_BASTION = 52;

	static const string KEY_LEAD = "opening.case.lead";

	protected static const float RUNNER_MIN = 200;	// the runner: this far from where the chain is...
	protected static const float RUNNER_MAX = 600;	// ...and no further
	protected static const float RUNNER_WIDE = 1000;	// tried as far as this when nothing nearer fits
	protected static const float WARM_MINUTES = 60;	// a case in the open is warm after this long
	protected static const float REHAND = 60;		// a lost lead is handed again at most this often (s)
	protected static const float CLINIC_REACH = 250;	// the clinic's file: a clinic within this of the town's centre
	protected static const int CURE_DATE = 25;		// before this cure percent, the station hands The Date on the Tag

	protected RZ_GameModeComponent m_GM;
	protected ref RZ_CaseStepDef m_Lead;
	protected ref RZ_CaseStepDef m_Date;
	protected ref RZ_CaseStepDef m_Back;
	protected ref RZ_CaseStepDef m_Post;
	protected ref array<IEntity> m_aCases = {};			// weak: cases out in the world (to warm)
	protected ref map<int, int> m_mLead = new map<int, int>();	// player -> the lead's errand id
	protected ref map<int, float> m_mTried = new map<int, float>();	// player -> when the lead was last handed
	protected ref map<int, vector> m_mStores = new map<int, vector>();	// player -> where Bastion or the Red Hand keep it
	protected float m_fClock;

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningCase(RZ_GameModeComponent gm)
	{
		m_GM = gm;

		m_Lead = new RZ_CaseStepDef(KEY_LEAD, "The Case", KIND_LEAD);
		m_Lead.m_fLapseMinutes = 600;
		RZ_ErrandWay w = m_Lead.Way("do");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 1;
		w.m_fRadius = 60;
		w.m_bEndsThere = true;
		w.m_sStep = "Take the case to {place}. Hand it to Halcyon's people there.";
		w.Take("Halcyon's station is {place}. I can't keep it: I've enough to carry. Take it to them.",
			"{place}. That's where the lab people are. Go before it warms up.");

		m_Date = new RZ_CaseStepDef("opening.case.date", "The Date on the Tag", KIND_DATE);
		m_Date.m_fLapseMinutes = 600;
		w = m_Date.Way("search");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 10;
		w.m_bEndsThere = true;
		w.m_bQuiet = true;
		w.m_sInside = "clinic";
		w.m_sStep = "Find the first case's file in {place}.";
		w.Work(RZ_EWork.SEARCH, "Find the first case's file", 10);
		w.Take("The date on that tag is a filing error. I'd like you to forget you read it.",
			"If you can't forget it, there's {place}. They kept a file on their first case. Go and read the date on it.");
		w.Done("The same date. Three weeks before the gates. Somebody underlined it twice.",
			"So it wasn't a filing error.");

		m_Back = new RZ_CaseStepDef("opening.case.back", "Get It Back", KIND_BACK);
		m_Back.m_fLapseMinutes = 600;
		w = m_Back.Way("sneak");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 8;
		w.m_fRadius = 10;
		w.m_bEndsThere = true;
		w.m_bUnseen = true;
		w.m_sInside = "any";
		w.m_sStep = "Take the case back from the Red Hand's lock-up at {place}, unseen.";
		w.Work(RZ_EWork.SEARCH, "Take the case back, quietly", 8);
		w.Take("Somebody sold our case to the Red Hand. It's in their lock-up at {place}. We'd like it back. We won't ask how.",
			"The book has a new line in it: one Halcyon case. It's at {place}. It shouldn't be.");
		w.Done("You have it. Now take it where it was going.");

		m_Post = new RZ_CaseStepDef("opening.case.post", "The Case at the Post", KIND_POST);
		m_Post.m_fLapseMinutes = 600;
		w = m_Post.Way("sneak");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 8;
		w.m_fRadius = 10;
		w.m_bEndsThere = true;
		w.m_bUnseen = true;
		w.m_sInside = "any";
		w.m_sStep = "Take the case from Bastion's stores at {place}, unseen.";
		w.Work(RZ_EWork.SEARCH, "Take the case from the stores, quietly", 8);
		w.Take("Bastion signed our case into its stores at {place}. Evidence, they call it. It's warming on a shelf.",
			"Our case is on a shelf at {place}, with a Bastion label over ours. Bring it here.");
		w.Done("You have it. Now take it where it was going.");
	}

	//------------------------------------------------------------------------------------------------
	static RZ_OpeningCase Get()
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return null;

		return openings.GetCase();
	}

	//------------------------------------------------------------------------------------------------
	//! Guidance > Cold case prefab ("" none: no case, the chains end as before).
	static ResourceName CasePrefab(RZ_GameModeComponent gm)
	{
		if (!gm || !gm.GetGuidance() || !gm.GetGuidance().GetSettings())
			return "";

		return gm.GetGuidance().GetSettings().m_sColdCasePrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! "This character": the one place the case asks whether a record belongs to the character being
	//! played now. Today a death starts a new character (RZ_PlayerProfile.m_iLives); when it doesn't,
	//! only this changes (designs/PERSISTENCE.md).
	static int CharacterStamp(RZ_PlayerProfile profile)
	{
		if (!profile)
			return -1;

		return profile.m_iLives;
	}

	//------------------------------------------------------------------------------------------------
	//! This character took part in Proof (the same helper the case asks everywhere).
	static bool TookPart(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId)
	{
		return run && run.PlayerTookPartThisLife(gm, playerId);
	}

	// ================================================================ The opening's step
	//------------------------------------------------------------------------------------------------
	//! The case can be found at all: its prefab and the runner's body are set, the cure isn't out,
	//! and Halcyon has somewhere to take it.
	bool CanRun(vector from)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (CasePrefab(m_GM).IsEmpty() || m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN).IsEmpty() || (facts && facts.IsCured()))
			return false;

		vector station;
		string text;
		return FindStation(from, station, text);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the runner fell: by a road 200-600 m away (else a yard on the edge of town; then as far
	//! as 1 km), never in a field, the water or at the line (docs/PLACES.md). False: nowhere fits.
	bool RunnerPlace(vector from, out vector place, out string text)
	{
		array<float> reach = {RUNNER_MAX, RUNNER_WIDE};
		foreach (float far : reach)
		{
			place = RZ_PlaceFind.Near(from, RUNNER_MIN, far, RZ_PlaceNeeds.Get("opening.runner"));
			if (place != vector.Zero)
			{
				text = "by the road, " + RZ_Util.DirectionText(from, place);
				return true;
			}

			place = RZ_PlaceFind.Near(from, RUNNER_MIN, far, RZ_PlaceNeeds.Get("opening.runner.yard"));
			if (place != vector.Zero)
			{
				text = "in a yard on the edge of " + TownAt(place);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The runner and his case, put out when the step starts: a dead man where he fell (Puzzles >
	//! Civilian, killed and kept from the corpse cleanup until the opening ends) and the case by
	//! him, tagged. Returns the body (null if it didn't spawn); the case is in m_aCases.
	IEntity SpawnRunner(vector at, int playerId)
	{
		vector ground = RZ_Util.GroundPos(at);
		IEntity body = RZ_Util.SpawnPrefab(m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN), ground + Vector(0, 0.1, 0), Math.RandomFloat(0, 360));
		if (body)
		{
			// VERIFY: a character killed the moment it's spawned on the server falls where it stands (as A Sample's remains)
			RZ_Util.KillCharacter(body);
			RZ_OpeningTasks.KeepBody(body, true);
		}

		if (SpawnCaseBy(ground, playerId))
		{
			RZ_PlaceNeeds.VerifyThings("opening.runner", ground, "the runner");
			RZ_Log.Info(string.Format("Openings: the runner and his case put out for %1 at %2", RZ_Util.PlayerName(playerId), RZ_SettlementScanner.GridReference(ground)));
		}

		return body;
	}

	//------------------------------------------------------------------------------------------------
	//! The case by a runner's body (his pack spilled beside him), tagged: the line's runner where he
	//! fell (RZ_OpeningLine), or the case's own runner (SpawnRunner). False: the prefab has no case component.
	bool SpawnCaseBy(vector at, int playerId)
	{
		vector ground = RZ_Util.GroundPos(at);
		IEntity item = RZ_Util.SpawnPrefab(CasePrefab(m_GM), RZ_Util.RandomPointInRing(ground, 0.6, 1.1) + Vector(0, 0.3, 0), Math.RandomFloat(0, 360));
		RZ_ColdCaseComponent c = RZ_ColdCaseComponent.Of(item);
		if (!c)
		{
			if (item)
				SCR_EntityHelper.DeleteEntityAndChildren(item);

			RZ_Log.Warn("Openings: the cold case prefab has no RZ_ColdCaseComponent; no case for " + RZ_Util.PlayerName(playerId));
			return false;
		}

		c.SetTag(Tag(), playerId);
		RZ_GameModeComponent.TrackLooseItem(item);
		m_aCases.Insert(item);
		RZ_Log.Info(string.Format("Openings: the case lies by the runner for %1 at %2", RZ_Util.PlayerName(playerId), RZ_SettlementScanner.GridReference(ground)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The tag: Halcyon's words and the date it was packed, three weeks before Day 1 (the season's
	//! canon: the day the clinic told the Institute, origin.told and origin.told.month).
	string Tag()
	{
		int day = 14;
		int month = 3;
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts)
		{
			day = facts.CanonInt("origin.told", 3, 24);
			month = facts.CanonInt("origin.told.month", 1, 12);
		}

		return string.Format("HALCYON. FIELD STATION. LIVE. KEEP COLD.\nPacked %1.%2. Not for transport by road.", day, month);
	}

	//------------------------------------------------------------------------------------------------
	//! This character carries a case.
	static bool Carries(IEntity me)
	{
		return RZ_ColdCaseComponent.CarriedBy(me) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The opening's case step is done (the case is in your pack): the lead is handed by whoever
	//! gave the step (their name, their title), and tracked.
	void Begin(int playerId, IEntity me, string giverName, string giverTitle, vector giverAt)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_iCaseState = STATE_CARRIED;
		profile.m_iCaseLife = CharacterStamp(profile);
		HandLead(playerId, me, giverName, giverTitle, giverAt);
	}

	//------------------------------------------------------------------------------------------------
	//! The lead to the station, handed and tracked. False: nowhere to take it.
	protected bool HandLead(int playerId, IEntity me, string giverName, string giverTitle, vector giverAt)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands || !errands.IsOn() || !me)
			return false;

		vector station;
		string text;
		if (!FindStation(me.GetOrigin(), station, text))
			return false;

		m_mTried.Set(playerId, m_fClock);
		RZ_Errand e = errands.Hand(m_Lead, playerId, "case:" + playerId.ToString(), giverName, giverTitle, RZ_EFaction.INSTITUTE, giverAt, station, text, me);
		if (!e)
			return false;

		m_mLead.Set(playerId, e.m_iId);
		Track(playerId, e, m_Lead);
		RZ_Log.Info(string.Format("Openings: the case's lead for %1: %2 (%3)", RZ_Util.PlayerName(playerId), text, RZ_Util.DirectionText(me.GetOrigin(), station)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Track(int playerId, RZ_Errand e, RZ_CaseStepDef d)
	{
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (feed && feed.GetTracker())
			feed.GetTracker().Follow(playerId, -(RZ_UIFeed.ERRAND_ID_BASE + e.m_iId), e.Fill(d.m_sTitle));
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's field station: the nearest Halcyon holding, else the lab. False: Halcyon has none.
	bool FindStation(vector from, out vector place, out string text)
	{
		float best = float.MAX;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			array<RZ_Holding> list = {};
			territory.GetOutposts(RZ_EFaction.INSTITUTE, list);
			RZ_Holding seat = territory.GetStronghold(RZ_EFaction.INSTITUTE);
			if (seat)
				list.Insert(seat);

			foreach (RZ_Holding h : list)
			{
				float d = RZ_Util.DistXZ(from, h.m_vPos);
				if (d < best)
				{
					best = d;
					place = h.m_vPos;
					text = RZ_OpeningText.EndPlace(RZ_EFaction.INSTITUTE, h.m_sName, false);
				}
			}
		}

		if (m_GM.HasLab() && RZ_Util.DistXZ(from, m_GM.GetLabPosition()) < best)
		{
			best = RZ_Util.DistXZ(from, m_GM.GetLabPosition());
			place = m_GM.GetLabPosition();
			string labTown = "the lab";
			RZ_Settlement s = m_GM.GetSettlement(m_GM.GetLabSettlement());
			if (s)
				labTown = s.m_sName;

			text = RZ_OpeningText.EndPlace(RZ_EFaction.INSTITUTE, labTown, true);
		}

		return best < float.MAX;
	}

	// ================================================================ The steps
	//------------------------------------------------------------------------------------------------
	//! The lead is done only by the hand-over reply (never for standing at the station with the
	//! case: docs/MISSION_RULES.md, rule 19); the others' clocks run as any work site's.
	bool StepCounts(RZ_CaseStepDef d, RZ_Errand e, IEntity me)
	{
		return d.m_iKind != KIND_LEAD;
	}

	//------------------------------------------------------------------------------------------------
	//! A step of the case's own is done: Get It Back and The Case at the Post put the case back in
	//! your hands (and the lead again); The Date on the Tag counts as taking part in Proof.
	void OnStepDone(RZ_CaseStepDef d, RZ_Errand e, int playerId, notnull array<string> said)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (d.m_iKind == KIND_DATE)
		{
			RZ_QuestRun run = ProofRun();
			if (run)
				run.Did(m_GM, playerId, RZ_EQuestApproach.TALK, "read the clinic's first file");

			m_GM.NotifyPlayerAs(playerId, "The Date on the Tag", "The clinic's first file: a patient with a fever nobody could name, and \"Notify the Institute\" underlined twice. The date on it is the date on the case's tag.", RZ_ENoticeTag.MISSION);
			RZ_Log.Info("Openings: " + RZ_Util.PlayerName(playerId) + " read the clinic's first file (The Date on the Tag)");
			return;
		}

		if (d.m_iKind != KIND_BACK && d.m_iKind != KIND_POST)
			return;

		if (!me)
			return;

		IEntity item = RZ_Util.SpawnPrefab(CasePrefab(m_GM), RZ_Util.RandomPointInRing(me.GetOrigin(), 0.3, 0.6) + Vector(0, 0.3, 0), 0);
		RZ_ColdCaseComponent c = RZ_ColdCaseComponent.Of(item);
		if (c)
		{
			c.SetTag(Tag(), playerId);
			c.Warm(1);	// it sat on a shelf: warm, but still worth taking
			RZ_Util.InsertIntoInventory(me, item);
			RZ_GameModeComponent.TrackLooseItem(item);
			m_aCases.Insert(item);
		}

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_iCaseState = STATE_CARRIED;
		profile.m_iCaseLife = CharacterStamp(profile);
		HandLead(playerId, me, "Halcyon's field lead", "Halcyon's field lead, on the channel", me.GetOrigin());
		RZ_Log.Info(string.Format("Openings: %1 took the case back (%2)", RZ_Util.PlayerName(playerId), d.m_sKey));
	}

	// ================================================================ The hand-over replies
	//------------------------------------------------------------------------------------------------
	static bool IsOurs(int replyId)
	{
		return replyId == REPLY_HALCYON || replyId == REPLY_SELL || replyId == REPLY_BASTION;
	}

	//------------------------------------------------------------------------------------------------
	//! The speaker's faction for the hand-over: what the talk screen knows, else what they describe.
	protected int SpeakerFaction(RZ_TalkSession s)
	{
		if (s.m_iFaction >= 0)
			return s.m_iFaction;

		RZ_DCtx ctx = new RZ_DCtx();
		ctx.m_GM = m_GM;
		ctx.m_iPlayer = s.m_iPlayer;
		if (s.m_Adapter && s.m_Adapter.Describe(m_GM, s, ctx))
			return ctx.m_iFaction;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever carries a case: at Halcyon's people "[Errand] Hand over the runner's case", at the Red
	//! Hand's "[Trade] Sell the runner's case", at Bastion's "Hand the runner's case in". First in
	//! the list; nothing is handed until it's picked.
	void AddReplies(RZ_TalkSession s, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		if (!user || !Carries(user))
			return;

		int f = SpeakerFaction(s);
		RZ_TalkReply r;
		if (f == RZ_EFaction.INSTITUTE)
		{
			r = NewReply(REPLY_HALCYON, "Hand over the runner's case");
			r.m_sTag = RZ_TalkJobs.Tag("Errand");
		}
		else if (f == RZ_EFaction.BANDITS)
		{
			r = NewReply(REPLY_SELL, "Sell the runner's case");
			r.m_sTag = RZ_TalkJobs.Tag("Trade");
		}
		else if (f == RZ_EFaction.MILITARY)
		{
			r = NewReply(REPLY_BASTION, "Hand the runner's case in");
			r.m_sTag = RZ_TalkJobs.Tag("Errand");
		}

		if (!r)
			return;

		r.m_bOwn = true;
		r.m_bJob = true;
		r.m_iRank = RZ_TalkReply.RANK_ON;
		replies.InsertAt(r, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_TalkReply NewReply(int id, string label)
	{
		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = id;
		r.m_sLabel = label;
		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! A hand-over reply picked: the case leaves your pack, their words go into the open talk, and
	//! what it opens next.
	void Pick(RZ_TalkSession s, int replyId, IEntity user)
	{
		int playerId = s.m_iPlayer;
		IEntity item = RZ_ColdCaseComponent.CarriedBy(user);
		RZ_ColdCaseComponent c = RZ_ColdCaseComponent.Of(item);
		if (!c)
		{
			s.m_aNew.Insert("What case?");
			return;
		}

		bool warm = c.IsWarm();
		vector at = user.GetOrigin();
		if (!TakeCase(user, item))
		{
			s.m_aNew.Insert("Keep hold of it a moment. Your hands are full of something else.");
			return;
		}

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		bool director = RZ_TalkLeader.Cast(s.m_Adapter) != null;
		if (replyId == REPLY_HALCYON)
		{
			ToHalcyon(s, playerId, user, warm, director);
			profile.m_iCaseState = STATE_HALCYON;
		}
		else if (replyId == REPLY_SELL)
		{
			s.m_aNew.Insert("A Halcyon case. Do you know what the lab people would give for this? I do. Here's half of it.");
			m_GM.AddFavor(playerId, RZ_EFaction.BANDITS, 15);
			m_GM.AddInfamy(playerId, RZ_EFaction.INSTITUTE, 5, "sold Halcyon's case");
			profile.m_iCaseState = STATE_SOLD;
			StoreAt(playerId, RZ_EFaction.BANDITS, at);
			OpenStage(playerId, user, m_Back, RZ_EFaction.BANDITS);
		}
		else
		{
			s.m_aNew.Insert("Halcyon's? Then it's evidence. It goes in the stores, and the CO decides who sees it.");
			m_GM.AddFame(playerId, RZ_EFaction.MILITARY, 5, false, RZ_RepFlag.SMALL);
			profile.m_iCaseState = STATE_BASTION;
			StoreAt(playerId, RZ_EFaction.MILITARY, at);
			OpenStage(playerId, user, m_Post, RZ_EFaction.MILITARY);
		}

		EndLead(playerId, user, replyId == REPLY_HALCYON);
		RZ_Log.Info(string.Format("Openings: %1 handed the case over (%2, %3)", RZ_Util.PlayerName(playerId), ReplyName(replyId), WarmText(warm)));
	}

	//------------------------------------------------------------------------------------------------
	protected static string ReplyName(int replyId)
	{
		if (replyId == REPLY_HALCYON)
			return "to Halcyon";

		if (replyId == REPLY_SELL)
			return "sold to the Red Hand";

		return "to Bastion";
	}

	//------------------------------------------------------------------------------------------------
	protected static string WarmText(bool warm)
	{
		if (warm)
			return "warm";

		return "cold";
	}

	//------------------------------------------------------------------------------------------------
	//! To Halcyon: the director (or whoever stands for him) on the case and its date, Proof for
	//! this character, the tissue, and before 25% cure The Date on the Tag.
	protected void ToHalcyon(RZ_TalkSession s, int playerId, IEntity user, bool warm, bool director)
	{
		if (director)
		{
			if (warm)
				s.m_aNew.Insert("It's warm. It might still do. We've made do with less.");
			else
				s.m_aNew.Insert("Where did you get this? No. Don't tell me. It's alive, and it's cold, and that's all I need to know.");

			s.m_aNew.Insert("The date on the tag is a filing error. I'd be grateful if you forgot you read it.");
		}
		else
		{
			if (warm)
				s.m_aNew.Insert("It's warm. The director will want it anyway.");
			else
				s.m_aNew.Insert("That's ours. That's very much ours. It goes to the director tonight.");

			s.m_aNew.Insert("He'll want to know where you found it. He won't want to talk about the date on the tag. Neither do I.");
		}

		m_GM.AddFame(playerId, RZ_EFaction.INSTITUTE, 10, false, RZ_RepFlag.SMALL);
		Tissue(playerId);

		RZ_QuestRun run = ProofRun();
		if (run)
			run.Did(m_GM, playerId, RZ_EQuestApproach.TALK, "brought the runner's case in");

		RZ_CureResearch cure = m_GM.GetCure();
		if (cure && cure.ShownProgress() < CURE_DATE)
			OpenDate(playerId, user, s.m_sName);
	}

	//------------------------------------------------------------------------------------------------
	//! The first live case a season counts as live tissue for Halcyon's route (the finale reads the
	//! season flag "case.tissue": 1 waiting, 2 used); one per player a season counts at all: later
	//! ones are an ordinary sample, and a second from the same player nothing.
	protected void Tissue(int playerId)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		int season = 0;
		if (m_GM.GetSeasons())
			season = m_GM.GetSeasons().GetSeason();

		if (profile.m_iCaseSeason == season)
		{
			RZ_Log.Info("Openings: " + RZ_Util.PlayerName(playerId) + "'s second case this season counts for nothing");
			return;
		}

		profile.m_iCaseSeason = season;
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts && facts.Flag("case.tissue") == 0)
		{
			facts.SetFlag("case.tissue", 1);
			RZ_Log.Info("Openings: the season's first live case is in: live tissue for Halcyon's route, banked");
			return;
		}

		m_GM.DeliverSamples(playerId, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_QuestRun ProofRun()
	{
		RZ_Questlines lines = m_GM.GetQuestlines();
		if (!lines)
			return null;

		return lines.FindRunByKey("proof");
	}

	//------------------------------------------------------------------------------------------------
	//! The Date on the Tag: the clinic in Proof's clinic town (the canon's origin.clinic), its file
	//! inside on the ground floor; with no clinic there, the doctor's house.
	protected void OpenDate(int playerId, IEntity me, string giverName)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (!facts)
			return;

		RZ_Settlement town = m_GM.GetSettlement(facts.CanonSettlement("origin.clinic"));
		if (!town)
			return;

		vector room;
		string text = "the clinic in " + town.m_sName;
		vector place = RZ_Mission.LandPointNear(town.m_vCenter, 0, Math.Min(town.m_fRadius, CLINIC_REACH));
		if (RZ_WorkSites.Indoors(town.m_vCenter, CLINIC_REACH, "clinic", room))
			place = room;
		else
			text = "the doctor's house in " + town.m_sName;

		m_Date.m_aWays[0].m_sInside = "clinic";
		if (text.IndexOf("the clinic") != 0)
			m_Date.m_aWays[0].m_sInside = "house";

		Hand(playerId, me, m_Date, giverName, place, text, RZ_EFaction.INSTITUTE);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the Red Hand or Bastion keep it: their holding nearest where it changed hands.
	protected void StoreAt(int playerId, int faction, vector at)
	{
		vector best = at;
		float dist = float.MAX;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			array<RZ_Holding> list = {};
			territory.GetOutposts(faction, list);
			RZ_Holding seat = territory.GetStronghold(faction);
			if (seat)
				list.Insert(seat);

			foreach (RZ_Holding h : list)
			{
				float d = RZ_Util.DistXZ(at, h.m_vPos);
				if (d < dist)
				{
					dist = d;
					best = h.m_vPos;
				}
			}
		}

		m_mStores.Set(playerId, best);
	}

	//------------------------------------------------------------------------------------------------
	//! Get It Back or The Case at the Post, handed by Halcyon's field lead (the channel): the case,
	//! where they keep it.
	protected void OpenStage(int playerId, IEntity me, RZ_CaseStepDef d, int faction)
	{
		vector place;
		if (!m_mStores.Find(playerId, place))
			return;

		Hand(playerId, me, d, "Halcyon's field lead", place, TownAt(place), faction);
	}

	//------------------------------------------------------------------------------------------------
	protected void Hand(int playerId, IEntity me, RZ_CaseStepDef d, string giverName, vector place, string text, int faction)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands || !errands.IsOn() || !me)
			return;

		string title = giverName;
		if (title.IsEmpty())
			title = "Halcyon";

		RZ_Errand e = errands.Hand(d, playerId, "case:" + playerId.ToString(), title, title, faction, me.GetOrigin(), place, text, me);
		if (!e)
			return;

		Track(playerId, e, d);
		RZ_Log.Info(string.Format("Openings: %1 for %2 at %3", d.m_sTitle, RZ_Util.PlayerName(playerId), text));
	}

	//------------------------------------------------------------------------------------------------
	//! The lead ends with the hand-over: done when it went to Halcyon, dropped when it went elsewhere.
	protected void EndLead(int playerId, IEntity me, bool done)
	{
		int id;
		if (!m_mLead.Find(playerId, id))
			return;

		m_mLead.Remove(playerId);
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands)
			return;

		RZ_Errand e = errands.FindById(id);
		if (!e)
			return;

		if (done && errands.FinishNow(e, me))
			return;

		errands.Drop(e, "the case changed hands");
	}

	//------------------------------------------------------------------------------------------------
	protected bool TakeCase(IEntity user, IEntity item)
	{
		m_aCases.RemoveItem(item);
		if (item == RZ_ColdCaseComponent.s_TestCarried)
		{
			RZ_ColdCaseComponent.s_TestCarried = null;
			SCR_EntityHelper.DeleteEntityAndChildren(item);
			return true;
		}

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;

		return inv.TryDeleteItem(item);
	}

	//------------------------------------------------------------------------------------------------
	protected string TownAt(vector at)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(at);
	}

	// ================================================================ Death and respawn
	//------------------------------------------------------------------------------------------------
	//! A carrier died (RZ_FactionPerks.OnPlayerDied): the case comes off the body and is kept as
	//! data, with how warm it was, for the respawned character.
	void OnPlayerDied(int playerId, IEntity body)
	{
		IEntity item = RZ_ColdCaseComponent.CarriedBy(body);
		RZ_ColdCaseComponent c = RZ_ColdCaseComponent.Of(item);
		if (!c)
			return;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_bCaseHeld = true;
		profile.m_fCaseWarm = c.GetWarm();
		m_aCases.RemoveItem(item);
		if (item == RZ_ColdCaseComponent.s_TestCarried)
			RZ_ColdCaseComponent.s_TestCarried = null;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(body.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv || !inv.TryDeleteItem(item))
			SCR_EntityHelper.DeleteEntityAndChildren(item); // VERIFY: deleting an item still in a dead body's storage leaves the storage sound

		m_mLead.Remove(playerId);
		RZ_Log.Info("Openings: " + RZ_Util.PlayerName(playerId) + " died with the case; it's kept for the next spawn");
	}

	//------------------------------------------------------------------------------------------------
	//! A kept case back in the respawned character's pack (from the tick, once they stand).
	void GiveBack(int playerId, IEntity me)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!profile.m_bCaseHeld || !me)
			return;

		IEntity item = RZ_Util.SpawnPrefab(CasePrefab(m_GM), RZ_Util.RandomPointInRing(me.GetOrigin(), 0.3, 0.6) + Vector(0, 0.3, 0), 0);
		RZ_ColdCaseComponent c = RZ_ColdCaseComponent.Of(item);
		if (!c)
		{
			if (item)
				SCR_EntityHelper.DeleteEntityAndChildren(item);

			return;
		}

		profile.m_bCaseHeld = false;
		c.SetTag(Tag(), playerId);
		c.Warm(profile.m_fCaseWarm);
		RZ_Util.InsertIntoInventory(me, item);
		RZ_GameModeComponent.TrackLooseItem(item);
		m_aCases.Insert(item);
		m_GM.NotifyPlayer(playerId, "The case", "You still have the runner's case. Halcyon's people are waiting for it.");
		RZ_Log.Info("Openings: the kept case given back to " + RZ_Util.PlayerName(playerId));
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! From the openings' 1 s poll: cases warm; whoever carries one without the lead gets it (once a
	//! minute at most: a lead that lapsed, a case picked up off a body, a restart); and while anyone
	//! keeps one, Halcyon asks after it on its channel once a day.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		Warm(dt, ids, players);
		bool kept = false;
		foreach (int i, IEntity me : players)
		{
			GiveBack(ids[i], me);
			if (!Carries(me))
				continue;

			kept = true;
			int playerId = ids[i];
			if (HasLead(playerId))
				continue;

			float tried;
			if (m_mTried.Find(playerId, tried) && m_fClock - tried < REHAND)
				continue;

			// The opening's own step hands the lead when it's done
			if (RZ_Openings.IsRunningFor(playerId))
				continue;

			RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
			profile.m_iCaseState = STATE_CARRIED;
			profile.m_iCaseLife = CharacterStamp(profile);
			HandLead(playerId, me, "The tag on the case", "The tag on the case", me.GetOrigin());
		}

		if (kept)
			AskAfter();
	}

	//------------------------------------------------------------------------------------------------
	//! The lead's errand id for this player, -1 none (the runtime test reads it).
	int LeadOf(int playerId)
	{
		int id;
		if (m_mLead.Find(playerId, id))
			return id;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasLead(int playerId)
	{
		int id;
		if (!m_mLead.Find(playerId, id))
			return false;

		RZ_Errands errands = m_GM.GetErrands();
		RZ_Errand e;
		if (errands)
			e = errands.FindById(id);

		if (e && e.m_eState == RZ_EErrandState.HELD)
			return true;

		m_mLead.Remove(playerId);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Cases in the open warm over Cold case minutes; half as fast by night.
	protected void Warm(float dt, array<int> ids, array<IEntity> players)
	{
		float rate = dt / (WARM_MINUTES * 60);
		if (RZ_GameTime.IsNight())
			rate *= 0.5;

		for (int i = m_aCases.Count() - 1; i >= 0; i--)
		{
			RZ_ColdCaseComponent c = RZ_ColdCaseComponent.Of(m_aCases[i]);
			if (!c)
			{
				m_aCases.Remove(i);
				continue;
			}

			c.Warm(rate);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Once a season day while a case is out: Halcyon's channel asks after it.
	protected void AskAfter()
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (!facts || !m_GM.GetFactionLife())
			return;

		int day = RZ_SeasonClock.Day();
		if (facts.Flag("case.asked") == day)
			return;

		facts.SetFlag("case.asked", day);
		m_GM.GetFactionLife().OpenChannelLine(RZ_EFaction.INSTITUTE, "Halcyon field station. A cold case went missing on the north road. Whoever has it: it's no use to you, and it's warming. Bring it in. No questions.");
	}
}
