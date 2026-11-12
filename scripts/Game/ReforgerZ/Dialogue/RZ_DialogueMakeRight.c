//! Make it right (docs/DIALOGUE.md, "Make it right"; docs/REPUTATION.md, "Working off Infamy"). With
//! Dialogue > Trees and Dialogue > Make it right on, a leader who won't deal with you (attitude -2
//! or below, not Hunted: the Hunted have the parley) still offers one reply: "What would it take?".
//! It reserves that faction's amends contract for you for an hour (the parley's own pick). When
//! the job succeeds with you in it, that leader's regard for you goes to 0 and their crossed memory
//! of you is cleared, once a season per leader. The Infamy the job works off goes through Forgive(),
//! inside the one budget, as every amends contract does. Server only.

//------------------------------------------------------------------------------------------------
//! One open Make it right: the leader, who asked, and the job reserved for them.
class RZ_MakeRight
{
	string m_sIdent;
	int m_iPlayer;
	int m_iLeader;
	string m_sKey;					// the leader's regard key
	ref RZ_Mission m_Job;			// kept until it ends, so its result can be read
	float m_fUntil;
}

//------------------------------------------------------------------------------------------------
class RZ_DialogueMakeRight
{
	static const int VERB = 95;
	protected static const float WAIT = 7200;

	protected static ref array<ref RZ_MakeRight> s_aOpen = {};
	protected static float s_fClock;

	//------------------------------------------------------------------------------------------------
	static bool IsOn()
	{
		return RZ_DialogueSettings.Get().m_bMakeRight;
	}

	//------------------------------------------------------------------------------------------------
	//! The season flag set once this leader was made right with by this identity.
	protected static string DoneKey(string ident, string leaderKey)
	{
		return "made_right." + ident + "." + leaderKey;
	}

	//------------------------------------------------------------------------------------------------
	//! made_right: this leader was made right with this season (a greet can say so).
	static bool WasMadeRight(RZ_DCtx ctx)
	{
		RZ_WorldFacts facts = ctx.m_GM.GetFacts();
		return facts && ctx.m_Leader && facts.Flag(DoneKey(ctx.m_GM.GetProfile(ctx.m_iPlayer).m_sIdentity, ctx.m_sKey)) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Why it isn't offered ("" if it is): only at a leader who refuses you on attitude.
	static string Why(RZ_DCtx ctx)
	{
		if (!IsOn() || !ctx.m_Leader)
			return "off";

		RZ_GameModeComponent gm = ctx.m_GM;
		int f = ctx.m_Leader.m_eFaction;
		if (RZ_Reputation.IsHunted(gm.GetTier(ctx.m_iPlayer, f)))
			return "off";

		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees.Attitude(ctx) > -2)
			return "off";

		if (WasMadeRight(ctx))
			return "We did this once. I don't do it twice in a season.";

		if (Pending(ctx.m_iPlayer, ctx.m_Leader.m_iId))
			return "You know what it takes. Go and do it.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Pending(int playerId, int leaderId)
	{
		foreach (RZ_MakeRight m : s_aOpen)
		{
			if (m.m_iPlayer == playerId && m.m_iLeader == leaderId)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The hub's reply, ahead of the topics.
	static void Verb(RZ_TalkSession s, notnull array<ref RZ_TalkReply> replies)
	{
		if (!s.m_Tree)
			return;

		string why = Why(s.m_Tree.m_Ctx);
		if (why == "off")
			return;

		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = VERB;
		r.m_sLabel = "What would it take to fix this?";
		r.m_sTag = "[Make it right]";
		r.m_bOwn = true;
		if (!why.IsEmpty())
		{
			r.m_bEnabled = false;
			r.m_sNeed = "asked already";
		}

		replies.Insert(r);
	}

	//------------------------------------------------------------------------------------------------
	//! Picked: the faction's amends contract, reserved for this player for an hour.
	static void Pick(RZ_TalkSession s)
	{
		RZ_DCtx ctx = s.m_Tree.m_Ctx;
		string why = Why(ctx);
		if (!why.IsEmpty())
		{
			if (why != "off")
				s.m_aNew.Insert(why);

			return;
		}

		RZ_GameModeComponent gm = ctx.m_GM;
		RZ_Checkpoints posts = gm.GetCheckpoints();
		string offer = "";
		if (posts && posts.GetParley())
			offer = posts.GetParley().ReserveAmends(ctx.m_iPlayer, ctx.m_Leader.m_eFaction);

		if (offer.IsEmpty())
		{
			s.m_aNew.Insert("Nothing I'd trust you with today. Ask me tomorrow.");
			return;
		}

		RZ_MakeRight m = new RZ_MakeRight();
		m.m_sIdent = gm.GetProfile(ctx.m_iPlayer).m_sIdentity;
		m.m_iPlayer = ctx.m_iPlayer;
		m.m_iLeader = ctx.m_Leader.m_iId;
		m.m_sKey = ctx.m_sKey;
		m.m_Job = ReservedJob(gm, m.m_sIdent);
		m.m_fUntil = s_fClock + WAIT;
		s_aOpen.Insert(m);
		s.m_aNew.Insert(Line(ctx.m_Leader.m_eFaction));
		s.m_aNew.Insert(offer);
		s.m_aNew.Insert("(Make it right: when it's done, they start again with you. Once a season.)");
		RZ_Log.Info(string.Format("Dialogue: make it right %1 at %2 (%3)", RZ_Util.PlayerName(ctx.m_iPlayer), ctx.m_sKey, offer.Length() > 0));
	}

	//------------------------------------------------------------------------------------------------
	//! The leader's answer, in their faction's voice.
	protected static string Line(int f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "Work. The kind you do for people who can't thank you. Then we'll see.";
			case RZ_EFaction.MILITARY: return "A job, done properly, signed for. Then the file gets a new page.";
			case RZ_EFaction.BANDITS: return "Everything's for sale. Even this. Here's the price, and it isn't Favor.";
			case RZ_EFaction.INSTITUTE: return "A contribution. Measurable. We'll revise our notes afterwards.";
		}

		return "Do something useful. Then come back.";
	}

	//------------------------------------------------------------------------------------------------
	//! The amends job just reserved for this identity (RZ_Parley.ReserveAmends started it).
	protected static RZ_Mission ReservedJob(RZ_GameModeComponent gm, string ident)
	{
		RZ_MissionManager missions = gm.GetMissions();
		if (!missions)
			return null;

		RZ_Mission found;
		foreach (RZ_Mission m : missions.GetActive())
		{
			if (m.m_sReservedFor == ident && m.m_bAmends)
				found = m;
		}

		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! On the dialogue tick: a job that succeeded with its player in it makes it right.
	static void Tick(RZ_GameModeComponent gm, float dt)
	{
		s_fClock += dt;
		for (int i = s_aOpen.Count() - 1; i >= 0; i--)
		{
			RZ_MakeRight m = s_aOpen[i];
			if (!m.m_Job || s_fClock > m.m_fUntil || m.m_Job.m_eState == RZ_EMissionState.FAILED)
			{
				s_aOpen.Remove(i);
				continue;
			}

			if (m.m_Job.m_eState != RZ_EMissionState.SUCCEEDED)
				continue;

			bool there = m.m_Job.m_aParticipants.Contains(m.m_iPlayer) && gm.GetProfile(m.m_iPlayer).m_sIdentity == m.m_sIdent;
			if (there)
				Done(gm, m);

			s_aOpen.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Regard to 0, the crossed memory cleared, once a season per leader.
	protected static void Done(RZ_GameModeComponent gm, RZ_MakeRight m)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		RZ_Leaders leaders = gm.GetLeaders();
		if (!trees || !leaders)
			return;

		RZ_Leader l = leaders.Get(m.m_iLeader);
		if (!l)
			return;

		trees.GetMemory().SetRegard(m.m_iPlayer, m.m_sKey, 0);
		RZ_WorldFacts facts = gm.GetFacts();
		if (facts)
			facts.SetFlag(DoneKey(m.m_sIdent, m.m_sKey), 1);

		int idx = l.m_aMemIdent.Find(m.m_sIdent);
		if (idx >= 0 && idx < l.m_aMemFlags.Count())
			l.m_aMemFlags[idx] = l.m_aMemFlags[idx] - (l.m_aMemFlags[idx] & RZ_Leaders.MEM_CROSSED);

		gm.NotifyPlayer(m.m_iPlayer, l.GetHeader(), "That's done, then. We start again. Don't make me regret it.");
		RZ_Log.Info(string.Format("Dialogue: made right %1 with %2", RZ_Util.PlayerName(m.m_iPlayer), m.m_sKey));
	}
}
