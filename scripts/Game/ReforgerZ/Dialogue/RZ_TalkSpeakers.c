//! The talk screen's adapters (Dialogue/RZ_Dialogue.c): who a person is, how they greet you, what
//! you can say to them, and what each reply runs. Server side.

//------------------------------------------------------------------------------------------------
//! The base: a person's name and sub-line from the title their lines come under
//! ("Ines Calder, council head in Morton" is Ines Calder, then the rest).
class RZ_TalkSpeaker
{
	//------------------------------------------------------------------------------------------------
	//! The adapter for a person: a leader's own, else the generic one.
	static RZ_TalkSpeaker For(RZ_GameModeComponent gm, IEntity owner)
	{
		RZ_ClueComponent clue = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		if (clue && clue.GetMissionId() == RZ_Leaders.TAG && gm.GetLeaders())
		{
			RZ_Leader l = gm.GetLeaders().Get(clue.GetClueIndex());
			if (l)
				return new RZ_TalkLeader(l);
		}

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
	void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
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
	protected static RZ_TalkReply Reply(int id, string label)
	{
		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = id;
		r.m_sLabel = label;
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

	protected IEntity m_Owner;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkGeneric(IEntity owner)
	{
		m_Owner = owner;
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		if (m_Owner)
			RZ_PuzzleActionHelper.DoRead(gm, m_Owner, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Wanderers, safehouse occupants, named people at holdings, road post speakers and the Long
	//! Table's people, by the tag their body carries. Mission and questline people have no trees yet.
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

		if (tag == RZ_Territory.TAG && gm.GetTerritory())
			return DescribeNamed(gm, idx, ctx);

		if (tag == RZ_Checkpoints.TAG_POST && gm.GetCheckpoints())
			return DescribePost(gm, idx, ctx);

		if (tag == RZ_LongTable.TAG && gm.GetLongTable())
			return DescribeTable(gm, idx, ctx);

		return false;
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

		for (int slot = 0; slot < 8; slot++)
		{
			string choice = comp.GetSlotLabel(true, slot);
			if (!choice.IsEmpty())
				replies.Insert(Reply(slot, choice));
		}

		for (int u = 0; u < 8; u++)
		{
			string use = comp.GetSlotLabel(false, u);
			if (!use.IsEmpty())
				replies.Insert(Reply(10 + u, use));
		}

		// Server admins: the errands' view for this place, without typing (docs/ERRANDS.md, Telemetry)
		if (gm.GetErrands() && comp.GetMissionId() < 0 && RZ_NoticeBoard.IsAdmin(playerId))
			replies.Insert(Reply(ADMIN_ERRANDS, "[Admin] Errands near here"));

		// Server admins: what /talkdebug, /tribes and /rep do, for a pad (no typing)
		if (!RZ_NoticeBoard.IsAdmin(playerId))
			return;

		if (gm.GetDialogue())
			replies.Insert(Reply(ADMIN_TREES, "[Admin] Read this person's conversations"));

		if (gm.GetTribes())
			replies.Insert(Reply(ADMIN_TRIBES, "[Admin] The infected, to the server log"));

		if (gm.GetRepWatch())
			replies.Insert(Reply(ADMIN_REP, "[Admin] Standing of the nearest player"));
	}

	//------------------------------------------------------------------------------------------------
	//! The admin replies (none of them changes the game). True if replyId was one.
	protected bool PickAdmin(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		if (replyId != ADMIN_TREES && replyId != ADMIN_TRIBES && replyId != ADMIN_REP)
			return false;

		if (!RZ_NoticeBoard.IsAdmin(playerId))
			return true;

		if (replyId == ADMIN_TREES && gm.GetDialogue())
			gm.GetDialogue().GetTrees().Debug(playerId, s);
		else if (replyId == ADMIN_TRIBES && gm.GetTribes())
			gm.GetTribes().Command(playerId, "");
		else if (replyId == ADMIN_REP && gm.GetRepWatch())
			gm.GetRepWatch().Readout(playerId, NearestPlayerName(playerId, user, 50));

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
	protected RZ_Leader m_Leader;
	protected IEntity m_Seat;

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
		replies.Insert(Reply(0, "Ask for work"));
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
		// The epilogue week's replies (40-43)
		if (replyId >= 40 && replyId <= 43 && m_Leader && gm.GetLeaders())
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
