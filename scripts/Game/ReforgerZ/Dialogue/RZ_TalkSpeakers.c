//! The talk screen's adapters (Dialogue/RZ_Dialogue.c): who a person is, how they greet you, what
//! you can say to them, and what each reply runs. Server side.

//------------------------------------------------------------------------------------------------
//! The base: a person's name and sub-line from the title their lines come under
//! ("Ivan Calder, council head in Morton" is Ivan Calder, then the rest).
class RZ_TalkSpeaker
{
	//------------------------------------------------------------------------------------------------
	//! The adapter for a person: a leader's own, a stranger's, a quartermaster's or the medic's
	//! (RZ_TalkStaff), else the generic one.
	static RZ_TalkSpeaker For(RZ_GameModeComponent gm, IEntity owner)
	{
		RZ_ClueComponent clue = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		if (clue && clue.GetMissionId() == RZ_Leaders.TAG && gm.GetLeaders())
		{
			RZ_Leader l = gm.GetLeaders().Get(clue.GetClueIndex());
			if (l)
				return new RZ_TalkLeader(l);
		}

		// A stranger (errands) carries its tag on its puzzle object, not its clue
		RZ_PuzzleObjectComponent obj = RZ_PuzzleObjectComponent.Cast(owner.FindComponent(RZ_PuzzleObjectComponent));
		if (obj && obj.GetMissionId() == RZ_Strangers.TAG)
			return new RZ_TalkStranger(owner, obj.GetObjectIndex());

		// A duty's person: a trader at the swap table, a passer-by at the toll, an arrival at the gate
		if (obj && obj.GetMissionId() == RZ_DutyScenes.TAG_PERSON)
			return new RZ_TalkDutyPerson(owner, obj.GetObjectIndex());

		// An opening's contact, someone from before (Guidance/RZ_Openings.c)
		if (obj && obj.GetMissionId() == RZ_Openings.TAG)
			return new RZ_TalkOpening(owner, obj.GetObjectIndex());

		// A quartermaster or the safehouse medic: their own actions are the replies
		RZ_TalkAction talk = RZ_TalkAction.On(owner);
		if (talk && !obj)
			return new RZ_TalkStaff(owner, talk);

		return new RZ_TalkGeneric(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Before the greeting: whatever is known of the person (name, sub-line, faction, header).
	void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The first title caught becomes the header; the name and sub-line come from it if unset.
	void FromHeader(RZ_GameModeComponent gm, RZ_TalkSession s, string header)
	{
		string lower = header;
		lower.ToLower();
		if (s.m_iFaction < 0)
			s.m_iFaction = RZ_NoticeStyle.FactionIn(lower);

		if (!s.m_sName.IsEmpty())
			return;

		int comma = header.IndexOf(", ");
		if (comma <= 0)
		{
			s.m_sName = header;
			return;
		}

		s.m_sName = header.Substring(0, comma);
		string rest = header.Substring(comma + 2, header.Length() - comma - 2);
		if (s.m_iFaction >= 0 && s.m_sSub.IsEmpty())
			s.m_sSub = RZ_Factions.GetName(s.m_iFaction) + " - " + rest;
		else if (s.m_sSub.IsEmpty())
			s.m_sSub = rest;
	}

	//------------------------------------------------------------------------------------------------
	//! Their greeting. While s.m_bQuiet (the usual case) it says only what's real now: a refusal, a
	//! first meeting, a callback, news, a job waiting; never a plain hello (docs/DIALOGUE.md).
	void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! They have something to say before you speak (an opening's contact as a step starts, a
	//! stranger who called you over): their greeting is said in full.
	bool SpeaksFirst(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! They're there to talk to: a quiet greeting that said nothing still opens the screen on the
	//! replies. False (a leader away from their seat): no screen.
	bool Here(RZ_GameModeComponent gm)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Dialogue > Trees: who this is for conversation conditions (speaker selectors, memory key,
	//! faction, the leader or wanderer, the house or post). False: no trees for this person.
	bool Describe(RZ_GameModeComponent gm, RZ_TalkSession s, RZ_DCtx ctx)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
	}

	//------------------------------------------------------------------------------------------------
	void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! They wait on your answer to something they just put to you (a job's pitch, an errand's ways):
	//! with Trees, the hub shows only that answer's replies, no topics.
	bool Asking(RZ_GameModeComponent gm, RZ_TalkSession s)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Built slots a conversation may run (call:slot:N) that aren't replies of their own on the hub
	//! (a leader's job slot: their jobs are listed one by one instead). With Trees, a topic carrying
	//! one still shows.
	void Carryable(notnull array<int> slots)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The kind of job a slot starts or opens ("Job", "Errand"), "" when it isn't one: a
	//! conversation's choice that runs it is tagged with it (docs/DIALOGUE.md, "Checks and tags").
	string SlotJob(RZ_GameModeComponent gm, int slot)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Why that job slot has nothing for this player now ("Nothing today. Ask again later"), "" when
	//! it has: a conversation's choice that runs it shows locked with it.
	string SlotLock(RZ_GameModeComponent gm, RZ_TalkSession s, int slot, IEntity user)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_TalkReply Reply(int id, string label)
	{
		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = id;
		r.m_sLabel = label;
		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! A server admin's reply: the talk screen lists it under [Admin], not with the others.
	protected static RZ_TalkReply AdminReply(int id, string label)
	{
		RZ_TalkReply r = Reply(id, label);
		r.m_bAdmin = true;
		return r;
	}
}

//------------------------------------------------------------------------------------------------
//! Every person with the puzzle components: wanderers, safehouse occupants, named people at
//! holdings, the Long Table's people, the people missions and questlines spawn. The greeting is
//! their Talk action; the replies are their visible choice slots (ids 0-7) and use slots (10-17).
class RZ_TalkGeneric : RZ_TalkSpeaker
{
	static const int ADMIN_ERRANDS = 30;	// a server admin's view of the errands here
	// The admins' chat commands without typing (docs/UI.md, "Controllers"): /talkdebug, /tribes,
	// and /rep for the nearest player
	static const int ADMIN_TREES = 31;
	static const int ADMIN_TRIBES = 32;
	static const int ADMIN_REP = 33;
	static const int ADMIN_TRACES = 34;	// /traces: the door marks near you, to the server log (docs/TRACES.md)
	static const int ADMIN_TALKTEST = 35;	// /talkdebug test: every conversation run once for you, to the log
	static const int DUTY_ROW = 37;			// the duty a giver offers beside their pressing errand (RZ_Errands.TalkDuty)
	static const int ADMIN_TALKRESET = 36;	// /talkdebug reset: this person's scenes undecided again
	static const int ADMIN_SITES = 38;		// /sites: the site builder (World/HandBuilt/RZ_SiteBuilder.c)
	static const int ADMIN_FOG = 39;		// /fog all or /fog reset: your whole map clear, or fogged again if it is (World/RZ_MapFog.c)

	protected IEntity m_Owner;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkGeneric(IEntity owner)
	{
		m_Owner = owner;
	}

	//------------------------------------------------------------------------------------------------
	//! Their name and sub-line before the greeting, so they show even when a quiet greeting says
	//! nothing: the same title their handler speaks under.
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		string title = TitleFor(gm);
		if (title.IsEmpty())
			return;

		s.m_sHeader = title;
		FromHeader(gm, s, title);
	}

	//------------------------------------------------------------------------------------------------
	//! The title a wanderer's, an occupant's, a safehouse resident's or a holding's person's lines come
	//! under (their greeting handlers' own); "" for anyone else (the greeting's first title is used).
	protected string TitleFor(RZ_GameModeComponent gm)
	{
		if (!m_Owner)
			return "";

		RZ_ClueComponent clue = RZ_ClueComponent.Cast(m_Owner.FindComponent(RZ_ClueComponent));
		if (!clue)
			return "";

		int tag = clue.GetMissionId();
		int idx = clue.GetClueIndex();
		if (tag == RZ_Wanderers.TAG && gm.GetWanderers())
		{
			RZ_Wanderer w = gm.GetWanderers().FindById(idx);
			if (w)
				return w.GetTitle();
		}

		if (tag == RZ_Safehouses.TAG_OCCUPANT && gm.GetSafehouses())
		{
			RZ_Safehouse h = gm.GetSafehouses().Get(idx);
			if (h)
				return "Safehouse: " + h.m_sName;
		}

		if (tag == RZ_NamedPeople.TAG_HOUSE && gm.GetErrands())
		{
			RZ_NamedPerson p = gm.GetErrands().GetPeople().FromHouse(idx / 10, idx % 10);
			if (p)
				return p.m_sTitle;
		}

		if (tag == RZ_Territory.TAG && gm.GetTerritory())
		{
			RZ_Holding hold = gm.GetTerritory().GetHolding(idx / 10);
			int person = idx % 10;
			if (hold && person >= 0 && person < hold.m_aPeople.Count())
				return hold.m_aPeople[person] + ", " + hold.GetTitle();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		if (m_Owner)
			RZ_PuzzleActionHelper.DoRead(gm, m_Owner, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Wanderers, safehouse occupants and named residents, named people at holdings, road post
	//! speakers, the Long Table's people and the people missions spawn, by the tag their body carries.
	override bool Describe(RZ_GameModeComponent gm, RZ_TalkSession s, RZ_DCtx ctx)
	{
		if (!m_Owner)
			return false;

		RZ_ClueComponent clue = RZ_ClueComponent.Cast(m_Owner.FindComponent(RZ_ClueComponent));
		if (!clue)
			return false;

		int tag = clue.GetMissionId();
		int idx = clue.GetClueIndex();
		if (tag == RZ_Wanderers.TAG && gm.GetWanderers())
			return DescribeWanderer(gm, idx, ctx);

		if (tag == RZ_Safehouses.TAG_OCCUPANT && gm.GetSafehouses())
			return DescribeOccupant(gm, idx, ctx);

		if (tag == RZ_NamedPeople.TAG_HOUSE && gm.GetErrands() && gm.GetSafehouses())
			return DescribeResident(gm, idx, ctx);

		if (tag == RZ_Territory.TAG && gm.GetTerritory())
			return DescribeNamed(gm, idx, ctx);

		if (tag == RZ_Checkpoints.TAG_POST && gm.GetCheckpoints())
			return DescribePost(gm, idx, ctx);

		if (tag == RZ_LongTable.TAG && gm.GetLongTable())
			return DescribeTable(gm, idx, ctx);

		if (tag > 0 && gm.GetMissions())
			return DescribeMission(gm, tag, idx, ctx);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone a contract or a questline stage spawned: `quest:<questline>` and `quest` for a
	//! questline's people, `mission` for a contract's. Their built choices stay the verbs; a
	//! conversation picks one with call:slot:N, so the stage resolves as built.
	protected bool DescribeMission(RZ_GameModeComponent gm, int missionId, int idx, RZ_DCtx ctx)
	{
		RZ_Mission m = gm.GetMissions().FindById(missionId);
		if (!m)
			return false;

		RZ_QuestRun run;
		if (gm.GetQuestlines())
			run = gm.GetQuestlines().RunOf(m);

		if (run)
		{
			ctx.m_aKinds.Insert("quest:" + run.m_sKey);
			ctx.m_aKinds.Insert("quest");
		}
		else
		{
			ctx.m_aKinds.Insert("mission");
		}

		ctx.m_sKey = string.Format("mission:%1:%2", missionId, idx);
		ctx.m_sRun = "mission:" + missionId.ToString();
		if (m.m_bHasGiver)
			ctx.m_iFaction = m.m_eGiver;

		ctx.m_sName = "Someone";
		ctx.m_sShort = "them";
		ctx.m_sRole = "person";
		ctx.m_sPlace = m.GetLocationNameFor(gm, ctx.m_iPlayer, ctx.Pos());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DescribeWanderer(RZ_GameModeComponent gm, int idx, RZ_DCtx ctx)
	{
		RZ_Wanderer w = gm.GetWanderers().FindById(idx);
		if (!w)
			return false;

		string role = RZ_Wanderers.RoleName(w.m_eRole);
		role.ToLower();
		ctx.m_aKinds.Insert("wanderer:" + role);
		ctx.m_aKinds.Insert("wanderer");
		ctx.m_sKey = string.Format("wanderer:%1:%2", w.m_iId, w.m_iGeneration);
		ctx.m_Wanderer = w;
		ctx.m_sName = w.GetTitle();
		ctx.m_sShort = w.GetShortName();
		ctx.m_sRole = role;
		ctx.m_iFaction = -1;
		if (w.m_eRole == RZ_EWandererRole.KID)
			ctx.m_bStronghold = true;	// nobody threatens the Kid

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DescribeOccupant(RZ_GameModeComponent gm, int idx, RZ_DCtx ctx)
	{
		RZ_Safehouse h = gm.GetSafehouses().Get(idx);
		if (!h)
			return false;

		ctx.m_aKinds.Insert("occupant");
		ctx.m_sKey = "house:" + idx.ToString();
		ctx.m_iHouse = idx;
		ctx.m_iFaction = RZ_EFaction.SURVIVORS;
		ctx.m_sName = "The people here";
		ctx.m_sShort = "the house";
		ctx.m_sRole = "occupant";
		ctx.m_sPlace = h.m_sName;
		ctx.m_bStronghold = true;	// nobody fires near a house, and nobody threatens in one
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse's named person (errands' givers): resident:<role> (cook, lookout, storekeeper,
	//! clerk) and resident. The memory key is errands' own (house:<index>:<i>).
	protected bool DescribeResident(RZ_GameModeComponent gm, int code, RZ_DCtx ctx)
	{
		int house = code / 10;
		RZ_NamedPerson p = gm.GetErrands().GetPeople().FromHouse(house, code % 10);
		if (!p || !gm.GetSafehouses().Get(house))
			return false;

		ctx.m_aKinds.Insert("resident:" + p.m_sRole);
		ctx.m_aKinds.Insert("resident");
		ctx.m_sKey = RZ_NamedPeople.HouseKey(house, code % 10);
		ctx.m_iHouse = house;
		ctx.m_iFaction = RZ_EFaction.SURVIVORS;
		ctx.m_sName = p.m_sName;
		ctx.m_sShort = p.m_sName;
		ctx.m_sRole = p.m_sRole;
		ctx.m_sPlace = p.m_sPlace;
		ctx.m_bStronghold = true;	// nobody threatens in a house
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DescribeNamed(RZ_GameModeComponent gm, int idx, RZ_DCtx ctx)
	{
		RZ_Holding hold = gm.GetTerritory().GetHolding(idx / 10);
		int person = idx % 10;
		if (!hold || person < 0 || person >= hold.m_aPeople.Count() || person >= hold.m_aRoles.Count())
			return false;

		string f = RZ_DialogueDB.FactionWord(hold.m_eFaction);
		string job = hold.m_aRoles[person];
		string jobKey = job;
		jobKey.Replace(" ", "_");
		ctx.m_aKinds.Insert("named:" + f + "/" + jobKey);
		ctx.m_aKinds.Insert("named:" + f);
		ctx.m_aKinds.Insert("named");
		ctx.m_sKey = string.Format("named:%1:%2", hold.m_iId, person);
		ctx.m_iFaction = hold.m_eFaction;
		ctx.m_sName = hold.m_aPeople[person];
		ctx.m_sShort = hold.m_aPeople[person];
		ctx.m_sRole = job;
		ctx.m_sPlace = hold.m_sName;
		ctx.m_bStronghold = hold.m_bStronghold;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DescribePost(RZ_GameModeComponent gm, int idx, RZ_DCtx ctx)
	{
		RZ_RoadPost post = gm.GetCheckpoints().FindPost(idx);
		if (!post)
			return false;

		string pf = RZ_DialogueDB.FactionWord(post.m_eFaction);
		ctx.m_aKinds.Insert("post:" + pf);
		ctx.m_aKinds.Insert("post");
		ctx.m_sKey = string.Format("post:%1:%2", pf, post.m_sNear);
		ctx.m_iPost = idx;
		ctx.m_iFaction = post.m_eFaction;
		ctx.m_sName = post.SpeakerName();
		ctx.m_sShort = post.SpeakerName();
		ctx.m_sRole = "post";
		ctx.m_sPlace = post.m_sNear;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DescribeTable(RZ_GameModeComponent gm, int idx, RZ_DCtx ctx)
	{
		int tableRole;
		bool onRoad;
		string name;
		if (!gm.GetLongTable().MemberInfo(idx, tableRole, onRoad, name))
			return false;

		string roleWord = RZ_LongTable.RoleWord(tableRole);
		if (onRoad)
			ctx.m_aKinds.Insert("table:road");
		else
			ctx.m_aKinds.Insert("table:" + roleWord);

		ctx.m_aKinds.Insert("table");
		ctx.m_sKey = "table:" + roleWord;
		ctx.m_iTableRole = tableRole;
		ctx.m_iFaction = -1;
		ctx.m_sName = name;
		ctx.m_sShort = name;
		ctx.m_sRole = roleWord;
		ctx.m_bStronghold = true;	// never threatened
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		if (!m_Owner)
			return;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(m_Owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!comp)
			return;

		// An errand on offer: its ways are what you'd agree to do ("I'll fetch the cleaning kits."),
		// each tagged [Errand], locked with what it needs when you can't take it
		RZ_Errands errands = gm.GetErrands();
		bool ways = errands && errands.OffersTo(m_Owner, playerId);
		int errandSlot = -1;
		string giver = "";
		if (!ways)
			giver = ErrandGiver(gm, errandSlot);

		// A mission's or questline stage's person: their choices move the job you're on, locked with
		// what each needs (RZ_Mission.ChoiceLock)
		RZ_Mission mission = MissionOf(gm, comp);
		for (int slot = 0; slot < 8; slot++)
		{
			string choice = comp.GetSlotLabel(true, slot);
			if (choice.IsEmpty())
				continue;

			RZ_TalkReply said = Reply(slot, choice);
			said.m_bWay = ways;
			if (ways)
				Way(errands, playerId, user, said);
			else if (slot == errandSlot)
				ErrandRow(gm, playerId, giver, user, said);
			else if (mission)
				Step(gm, mission, playerId, comp.GetObjectIndex(), true, slot, said);

			replies.Insert(said);
		}

		// Always a duty on offer: beside a pressing errand, listed after it
		string dutyTitle;
		if (!ways && !giver.IsEmpty() && errands.TalkDuty(playerId, giver, user, dutyTitle))
		{
			RZ_TalkReply duty = Reply(DUTY_ROW, dutyTitle);
			duty.m_bOwn = true;
			duty.m_bJob = true;
			duty.m_sTag = RZ_TalkJobs.Tag("Duty");
			duty.m_iRank = RZ_TalkReply.RANK_NEW;
			replies.Insert(duty);
		}

		for (int u = 0; u < 8; u++)
		{
			string use = comp.GetSlotLabel(false, u);
			if (use.IsEmpty())
				continue;

			RZ_TalkReply used = Reply(10 + u, use);
			if (mission)
				Step(gm, mission, playerId, comp.GetObjectIndex(), false, u, used);

			replies.Insert(used);
		}

		// Server admins: the errands' view for this place, without typing (docs/ERRANDS.md, Telemetry)
		if (gm.GetErrands() && comp.GetMissionId() < 0 && RZ_NoticeBoard.IsAdmin(playerId))
			replies.Insert(AdminReply(ADMIN_ERRANDS, "[Admin] Errands near here"));

		// Server admins: what /talkdebug, /tribes and /rep do, for a pad (no typing)
		if (!RZ_NoticeBoard.IsAdmin(playerId))
			return;

		if (gm.GetDialogue())
		{
			replies.Insert(AdminReply(ADMIN_TREES, "[Admin] Read this person's conversations"));
			replies.Insert(AdminReply(ADMIN_TALKTEST, "[Admin] Test every conversation, to the log"));
			replies.Insert(AdminReply(ADMIN_TALKRESET, "[Admin] Undecide this person's scenes"));
		}

		if (gm.GetTribes())
			replies.Insert(AdminReply(ADMIN_TRIBES, "[Admin] The infected, to the server log"));

		if (gm.GetRepWatch())
			replies.Insert(AdminReply(ADMIN_REP, "[Admin] Standing of the nearest player"));

		if (gm.GetTraces() && gm.GetTraces().IsOn())
			replies.Insert(AdminReply(ADMIN_TRACES, "[Admin] Door marks near here, to the log"));

		if (gm.GetCampLife())
			replies.Insert(AdminReply(ADMIN_SITES, "[Admin] Site builder"));

		if (gm.GetMapFog() && gm.GetMapFog().IsOn())
			replies.Insert(AdminReply(ADMIN_FOG, "[Admin] Clear my map's fog (again: fog it)"));
	}

	//------------------------------------------------------------------------------------------------
	//! An errand giver's key and the slot their errand is asked in (a safehouse's or a holding's
	//! named person: slot 1, "Anything need doing?"; a wanderer: slot 2 while it's "Need a hand?");
	//! "" for anyone else.
	protected string ErrandGiver(RZ_GameModeComponent gm, out int slot)
	{
		slot = -1;
		RZ_Errands errands = gm.GetErrands();
		if (!m_Owner || !errands || !errands.IsOn())
			return "";

		RZ_ClueComponent clue = RZ_ClueComponent.Cast(m_Owner.FindComponent(RZ_ClueComponent));
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(m_Owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!clue || !comp)
			return "";

		int tag = clue.GetMissionId();
		int idx = clue.GetClueIndex();
		if (tag == RZ_NamedPeople.TAG_HOUSE)
		{
			slot = 1;
			return RZ_NamedPeople.HouseKey(idx / 10, idx % 10);
		}

		if (tag == RZ_Territory.TAG && comp.GetSlotLabel(true, 1) == "Anything need doing?")
		{
			slot = 1;
			return RZ_NamedPeople.HoldingKey(idx / 10, idx % 10);
		}

		if (tag == RZ_Wanderers.TAG && errands.AsksOn() && comp.GetSlotLabel(true, 2) == "Need a hand?")
		{
			RZ_NamedPerson p = errands.GetPeople().FromWanderer(idx);
			if (!p)
				return "";

			slot = 2;
			return p.m_sKey;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The mission (a contract's or a questline stage's) whose person this is, while it runs; null.
	protected RZ_Mission MissionOf(RZ_GameModeComponent gm, RZ_PuzzleObjectComponent comp)
	{
		if (!comp || comp.GetMissionId() <= 0 || !gm.GetMissions())
			return null;

		RZ_Mission m = gm.GetMissions().FindById(comp.GetMissionId());
		if (!m || m.m_eState != RZ_EMissionState.ACTIVE)
			return null;

		return m;
	}

	//------------------------------------------------------------------------------------------------
	//! One of an errand's ways on offer: [Errand] in front, locked with what it needs.
	protected void Way(RZ_Errands errands, int playerId, IEntity user, RZ_TalkReply r)
	{
		if (r.m_sLabel == "Not now")
			return;

		r.m_bJob = true;
		r.m_iRank = RZ_TalkReply.RANK_NEW;
		r.m_sTag = RZ_TalkJobs.Tag("Errand");
		string need = errands.WayLock(m_Owner, playerId, r.m_iId, user);
		if (need.IsEmpty())
			return;

		r.m_bEnabled = false;
		r.m_sNeed = need;
		r.m_iRank = RZ_TalkReply.RANK_LOCKED;
	}

	//------------------------------------------------------------------------------------------------
	//! The errand reply on a giver, on their first replies (docs/DIALOGUE.md, "Jobs on the first
	//! replies"): "[Errand] Water for Morton" when they have one for you, "[Errand] It's done: ..."
	//! when yours is ready to hand in, else "Anything need doing?" locked with why. A wanderer with
	//! no errand keeps "Need a hand?" open while Barter Chain could start from them.
	protected void ErrandRow(RZ_GameModeComponent gm, int playerId, string giver, IEntity user, RZ_TalkReply r)
	{
		RZ_Errands errands = gm.GetErrands();
		string title;
		bool held;
		string why;
		r.m_bJob = true;
		r.m_sTag = RZ_TalkJobs.Tag("Errand");
		if (errands.TalkErrand(playerId, giver, user, title, held, why))
		{
			r.m_bOwn = true;	// written here, not a built label
			r.m_iRank = RZ_TalkReply.RANK_NEW;
			r.m_sLabel = title;
			if (held)
			{
				r.m_iRank = RZ_TalkReply.RANK_ON;
				r.m_sLabel = "About " + title + ".";
				if (errands.ReadyToHandIn(playerId, giver))
					r.m_sLabel = "It's done: " + title + ".";
			}

			return;
		}

		if (r.m_sLabel == "Need a hand?" && RZ_ErrandRoutine.CanBarterChain(gm))
		{
			r.m_sTag = RZ_TalkJobs.Tag("Job");
			r.m_iRank = RZ_TalkReply.RANK_NEW;
			return;
		}

		r.m_bEnabled = false;
		r.m_sNeed = why;
		r.m_iRank = RZ_TalkReply.RANK_LOCKED;
	}

	//------------------------------------------------------------------------------------------------
	//! A mission person's choice or use: it moves the job you're on (first in the list), locked with
	//! what it needs when you can't do it (RZ_Mission.ChoiceLock: "Needs Known with Halcyon").
	protected void Step(RZ_GameModeComponent gm, RZ_Mission m, int playerId, int objIdx, bool choice, int slot, RZ_TalkReply r)
	{
		r.m_iRank = RZ_TalkReply.RANK_ON;
		// A hand-over for the job ("Hand over Bastion's rations", "Give the page to ...") is tagged by
		// the job's kind, like any turn-in
		string l = r.m_sLabel;
		l.ToLower();
		if (l.StartsWith("hand ") || l.StartsWith("give ") || l.StartsWith("deliver "))
			r.m_sTag = RZ_TalkJobs.Tag(m.KindWord());

		string need = m.ChoiceLock(gm, playerId, objIdx, choice, slot);
		if (need.IsEmpty())
			return;

		r.m_bEnabled = false;
		r.m_sNeed = need;
	}

	//------------------------------------------------------------------------------------------------
	override bool Asking(RZ_GameModeComponent gm, RZ_TalkSession s)
	{
		return gm.GetErrands() && gm.GetErrands().OffersTo(m_Owner, s.m_iPlayer);
	}

	//------------------------------------------------------------------------------------------------
	override string SlotJob(RZ_GameModeComponent gm, int slot)
	{
		int errandSlot;
		if (!ErrandGiver(gm, errandSlot).IsEmpty() && slot == errandSlot)
			return "Errand";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override string SlotLock(RZ_GameModeComponent gm, RZ_TalkSession s, int slot, IEntity user)
	{
		int errandSlot;
		string giver = ErrandGiver(gm, errandSlot);
		if (giver.IsEmpty() || slot != errandSlot)
			return "";

		RZ_TalkReply probe = Reply(slot, "");
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(m_Owner.FindComponent(RZ_PuzzleObjectComponent));
		if (comp)
			probe.m_sLabel = comp.GetSlotLabel(true, slot);

		ErrandRow(gm, s.m_iPlayer, giver, user, probe);
		if (probe.m_bEnabled)
			return "";

		return probe.m_sNeed;
	}

	//------------------------------------------------------------------------------------------------
	//! The admin replies (none of them changes the game). True if replyId was one.
	protected bool PickAdmin(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		if (replyId != ADMIN_TREES && replyId != ADMIN_TRIBES && replyId != ADMIN_REP && replyId != ADMIN_TRACES && replyId != ADMIN_TALKTEST && replyId != ADMIN_TALKRESET && replyId != ADMIN_SITES && replyId != ADMIN_FOG)
			return false;

		if (!RZ_NoticeBoard.IsAdmin(playerId))
			return true;

		if (replyId == ADMIN_TREES && gm.GetDialogue())
			gm.GetDialogue().GetTrees().Debug(playerId, s);
		else if (replyId == ADMIN_TALKTEST && gm.GetDialogue())
			gm.GetDialogue().GetTrees().Command(playerId, s, "test");
		else if (replyId == ADMIN_TALKRESET && gm.GetDialogue())
			gm.GetDialogue().GetTrees().Command(playerId, s, "reset");
		else if (replyId == ADMIN_TRIBES && gm.GetTribes())
			gm.GetTribes().Command(playerId, "");
		else if (replyId == ADMIN_REP && gm.GetRepWatch())
			gm.GetRepWatch().Readout(playerId, NearestPlayerName(playerId, user, 50));
		else if (replyId == ADMIN_TRACES && gm.GetTraces())
			gm.GetTraces().Command(playerId, "");
		else if (replyId == ADMIN_SITES)
			RZ_SiteBuilder.Open(playerId);
		else if (replyId == ADMIN_FOG && gm.GetMapFog())
			gm.GetMapFog().Command(playerId, "toggle");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The name of the nearest other player within `radius` m of `user`; "" (the admin's own) if none.
	protected string NearestPlayerName(int playerId, IEntity user, float radius)
	{
		if (!user)
			return "";

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		int best = -1;
		float bestDist = radius;
		foreach (int other : ids)
		{
			if (other == playerId)
				continue;

			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(other);
			if (!ent)
				continue;

			float d = RZ_Util.DistXZ(ent.GetOrigin(), user.GetOrigin());
			if (d < bestDist)
			{
				bestDist = d;
				best = other;
			}
		}

		if (best < 0)
			return "";

		return RZ_Util.PlayerName(best);
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		if (!m_Owner)
			return;

		if (replyId == ADMIN_ERRANDS)
		{
			if (gm.GetErrands() && RZ_NoticeBoard.IsAdmin(playerId) && user)
				s.m_aNew.Insert(gm.GetErrands().AdminText(playerId, user.GetOrigin()));

			return;
		}

		if (PickAdmin(gm, s, playerId, replyId, user))
			return;

		if (replyId == DUTY_ROW)
		{
			int dutySlot;
			string dutyGiver = ErrandGiver(gm, dutySlot);
			if (!dutyGiver.IsEmpty() && gm.GetErrands())
				gm.GetErrands().AskDuty(playerId, dutyGiver, m_Owner, user);

			return;
		}

		if (replyId >= 10)
			RZ_PuzzleActionHelper.DoUse(gm, m_Owner, playerId, replyId - 10, user);
		else
			RZ_PuzzleActionHelper.DoChoice(gm, m_Owner, playerId, replyId, user);
	}
}

//------------------------------------------------------------------------------------------------
//! A faction leader, on their own body or at their seat (a quartermaster's "Talk to whoever's in
//! charge"): the sub-line is the faction, their title and what they think of you; the replies are
//! asking for work, for news, and what they make of you (RZ_Leaders.HandleChoice 0-2).
class RZ_TalkLeader : RZ_TalkSpeaker
{
	static const int TAKE_JOB = 6;		// "I'll do it.": a pitched job taken (RZ_Leaders.TakeWork)
	static const int LEAVE_JOB = 7;		// "Not today.": left with them

	protected RZ_Leader m_Leader;
	protected IEntity m_Seat;
	protected int m_iPitch = -1;		// the job they've just explained and wait on your answer for, -1 none
	protected ref array<int> m_aJobs = {};	// what RZ_TalkJobs.BASE + i is (mission ids), as last listed

	//------------------------------------------------------------------------------------------------
	void RZ_TalkLeader(RZ_Leader l)
	{
		m_Leader = l;
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		if (!m_Leader)
			return;

		m_Seat = s.m_Speaker;
		s.m_sHeader = m_Leader.GetHeader();
		s.m_sName = m_Leader.m_sName;
		s.m_iFaction = m_Leader.m_eFaction;
		s.m_sSub = string.Format("%1 - %2 - %3", RZ_Factions.GetName(m_Leader.m_eFaction), m_Leader.m_sTitle,
			gm.GetLeaders().AttitudeWords(m_Leader, playerId));
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		if (m_Leader && gm.GetLeaders())
			gm.GetLeaders().HandleTalk(playerId, m_Leader.m_iId, m_Seat);
	}

	//------------------------------------------------------------------------------------------------
	override bool Here(RZ_GameModeComponent gm)
	{
		return m_Leader && m_Leader.IsPresent();
	}

	//------------------------------------------------------------------------------------------------
	//! stronghold:<f> or seat:<f>/<kind>, leader:<f>, leader. The memory key is the seat and the
	//! generation, so someone new in the chair starts with no regard or claims.
	override bool Describe(RZ_GameModeComponent gm, RZ_TalkSession s, RZ_DCtx ctx)
	{
		if (!m_Leader)
			return false;

		string f = RZ_DialogueDB.FactionWord(m_Leader.m_eFaction);
		if (m_Leader.IsStronghold())
			ctx.m_aKinds.Insert("stronghold:" + f);
		else
			ctx.m_aKinds.Insert("seat:" + f + "/" + SeatKind(m_Leader.m_eFaction));

		ctx.m_aKinds.Insert("leader:" + f);
		ctx.m_aKinds.Insert("leader");
		ctx.m_sKey = string.Format("leader:%1:%2", m_Leader.m_iId, m_Leader.m_iGeneration);
		ctx.m_iFaction = m_Leader.m_eFaction;
		ctx.m_Leader = m_Leader;
		ctx.m_sName = m_Leader.m_sName;
		ctx.m_sShort = m_Leader.m_sShort;
		ctx.m_sRole = m_Leader.m_sTitle;
		ctx.m_sPlace = m_Leader.m_sPlace;
		ctx.m_iHouse = m_Leader.m_iSafehouse;
		ctx.m_bStronghold = m_Leader.IsStronghold();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Asking(RZ_GameModeComponent gm, RZ_TalkSession s)
	{
		return m_iPitch >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Slot 0, the job: listed one by one (RZ_TalkJobs), so the work topics that run it still show.
	override void Carryable(notnull array<int> slots)
	{
		slots.Insert(0);
	}

	//------------------------------------------------------------------------------------------------
	override string SlotJob(RZ_GameModeComponent gm, int slot)
	{
		if (slot == 0)
			return "Job";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Slot 0 with nothing you can take: the reason the locked "Got any work for me?" gives.
	override string SlotLock(RZ_GameModeComponent gm, RZ_TalkSession s, int slot, IEntity user)
	{
		if (slot != 0 || !m_Leader || !user)
			return "";

		array<int> ids = {};
		array<ref RZ_TalkReply> rows = {};
		RZ_TalkJobs.Build(gm, s.m_iPlayer, user, m_Leader.m_eFaction, m_Leader, ids, rows);
		string why = "Nothing you can take yet";
		foreach (RZ_TalkReply r : rows)
		{
			if (r.m_bEnabled)
				return "";

			if (r.m_iId == RZ_TalkJobs.NOTHING)
				why = r.m_sNeed;
		}

		return why;
	}

	//------------------------------------------------------------------------------------------------
	protected static string SeatKind(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "council_head";
			case RZ_EFaction.MILITARY: return "outpost_lead";
			case RZ_EFaction.BANDITS: return "crew_boss";
		}

		return "field_lead";
	}

	//------------------------------------------------------------------------------------------------
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		// A job just explained: yes or no first (docs/WRITING.md, "Talk")
		if (m_iPitch >= 0)
		{
			RZ_TalkReply yes = Reply(TAKE_JOB, "I'll do it.");
			yes.m_bJob = true;
			yes.m_iRank = RZ_TalkReply.RANK_NEW;
			RZ_Mission pitched = gm.GetMissions().FindById(m_iPitch);
			if (pitched)
				yes.m_sTag = RZ_TalkJobs.Tag(pitched.KindWord());

			replies.Insert(yes);
			replies.Insert(Reply(LEAVE_JOB, "Not today."));
			return;
		}

		// Their jobs first, plainly: one reply a job, locked with what it needs (RZ_TalkJobs)
		m_aJobs.Clear();
		if (m_Leader && user)
			RZ_TalkJobs.Build(gm, playerId, user, m_Leader.m_eFaction, m_Leader, m_aJobs, replies);

		replies.Insert(Reply(1, "Heard anything?"));
		replies.Insert(Reply(2, "What do you make of me?"));

		// The epilogue week: About the season, Where should I look?, the stronghold's question
		RZ_Epilogue epi = RZ_Epilogue.Get();
		if (epi && m_Leader)
		{
			array<string> labels = {};
			array<int> ids = {};
			epi.LeaderReplies(m_Leader, playerId, labels, ids);
			foreach (int i, string label : labels)
			{
				replies.Insert(Reply(ids[i], label));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		// Your answer to the job they've just explained
		int pitched = m_iPitch;
		m_iPitch = -1;
		if ((replyId == TAKE_JOB || replyId == LEAVE_JOB) && pitched >= 0 && m_Leader && gm.GetLeaders())
		{
			gm.GetLeaders().TakeWork(playerId, m_Leader.m_iId, pitched, replyId == TAKE_JOB, user);
			return;
		}

		// A job on the list: they explain it, then you say yes or no (one conversation is enough)
		int job = replyId - RZ_TalkJobs.BASE;
		if (job >= 0 && job < RZ_TalkJobs.MAX && m_Leader && gm.GetLeaders())
		{
			if (job < m_aJobs.Count() && m_aJobs[job] >= 0)
				m_iPitch = gm.GetLeaders().PitchJob(playerId, m_Leader.m_iId, m_aJobs[job], user);

			return;
		}

		// The job slot run by a conversation (call:slot:0, Trees): the first job they have for you
		// is explained, or they say there's nothing today
		if (replyId == 0 && m_Leader && gm.GetLeaders())
		{
			m_iPitch = gm.GetLeaders().PitchFirst(playerId, m_Leader.m_iId, user);
			return;
		}

		// The epilogue week's replies (40-59)
		if (replyId >= 40 && replyId <= 59 && m_Leader && gm.GetLeaders())
		{
			gm.GetLeaders().SeasonPick(playerId, m_Leader, replyId);
			return;
		}

		// Dialogue > Trees: what they make of you is said in their faction's voice (RZ_RepVoice)
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (replyId == 2 && trees && trees.IsOn() && m_Leader && gm.GetLeaders() && gm.GetLeaders().Attitude(m_Leader, playerId) > -2)
		{
			array<string> lines = {};
			RZ_RepVoice.Verdict(gm, playerId, m_Leader.m_eFaction, m_Leader, lines);
			s.m_aNew.InsertAll(lines);
		}
		else if (m_Leader && gm.GetLeaders())
		{
			gm.GetLeaders().HandleChoice(playerId, m_Leader.m_iId, replyId, m_Seat, user);
		}

		// What they think of you can change with the answer
		if (m_Leader && gm.GetLeaders())
			s.m_sSub = string.Format("%1 - %2 - %3", RZ_Factions.GetName(m_Leader.m_eFaction), m_Leader.m_sTitle,
				gm.GetLeaders().AttitudeWords(m_Leader, playerId));
	}
}
