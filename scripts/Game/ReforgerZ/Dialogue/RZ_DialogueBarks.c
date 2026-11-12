//! Barks (docs/DIALOGUE.md, "Barks"): one line said to a player passing within 6 m of someone,
//! without anyone pressing Talk. A bark is a conversation (`bark` instead of a topic) whose `when`
//! holds for that speaker and that player: one node, its lines (variants pick one), no choices.
//! Said owner-only as the person's subtitle. With Dialogue > Trees on, on the server tick, only
//! for bodies already spawned near players (a 6 m query round each player). Limits: one bark per
//! player a minute, one per speaker per player every 10 minutes, none while you're talking to
//! anyone, none from someone who refuses you; `once` barks once a life per speaker. Server only.
class RZ_DialogueBarks
{

	protected RZ_GameModeComponent m_GM;
	protected RZ_DialogueTrees m_Trees;
	protected float m_fClock;
	protected ref map<int, float> m_mLast = new map<int, float>();			// player -> clock of their last bark
	protected ref map<string, float> m_mSpeaker = new map<string, float>();	// "ident|speaker key" -> clock
	protected ref array<IEntity> m_aFound = {};
	protected int m_iBarks = -1;		// barks in the database (counted once)

	//------------------------------------------------------------------------------------------------
	void RZ_DialogueBarks(RZ_GameModeComponent gm, RZ_DialogueTrees trees)
	{
		m_GM = gm;
		m_Trees = trees;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		m_fClock += dt;
		RZ_Dialogue dialogue = m_GM.GetDialogue();
		if (!dialogue || !AnyBarks())
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity me : players)
		{
			int pid = ids[i];
			float last;
			if (m_mLast.Find(pid, last) && m_fClock - last < RZ_DialogueSettings.Get().m_fBarkPerPlayer)
				continue;

			if (dialogue.GetSession(pid))
				continue;

			if (TryNear(pid, me))
				m_mLast.Set(pid, m_fClock);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool AnyBarks()
	{
		if (m_iBarks >= 0)
			return m_iBarks > 0;

		m_iBarks = m_Trees.GetDB().CountBarks();
		return m_iBarks > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The people within 6 m of this player, nearest first; the first with a bark for them barks.
	protected bool TryNear(int pid, IEntity me)
	{
		m_aFound.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(me.GetOrigin(), RZ_DialogueSettings.Get().m_fBarkRadius, OnEntity, null, EQueryEntitiesFlags.DYNAMIC);
		while (!m_aFound.IsEmpty())
		{
			int best = 0;
			for (int k = 1; k < m_aFound.Count(); k++)
			{
				if (RZ_Util.DistXZ(m_aFound[k].GetOrigin(), me.GetOrigin()) < RZ_Util.DistXZ(m_aFound[best].GetOrigin(), me.GetOrigin()))
					best = k;
			}

			IEntity body = m_aFound[best];
			m_aFound.Remove(best);
			if (BarkFrom(pid, me, body))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A living person who isn't a player and carries the puzzle components (a speaker).
	protected bool OnEntity(IEntity e)
	{
		if (m_aFound.Count() >= 6)
			return false;

		if (!ChimeraCharacter.Cast(e) || !RZ_Util.IsCharacterAlive(e) || GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(e) > 0)
			return true;

		if (e.FindComponent(RZ_ClueComponent) || e.FindComponent(RZ_PuzzleObjectComponent))
			m_aFound.Insert(e);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool BarkFrom(int pid, IEntity me, IEntity body)
	{
		RZ_TalkSession s = new RZ_TalkSession();
		s.m_iPlayer = pid;
		s.m_Speaker = body;
		s.m_Adapter = RZ_TalkSpeaker.For(m_GM, body);
		if (!s.m_Adapter)
			return false;

		s.m_Adapter.Setup(m_GM, s, pid);
		RZ_DCtx ctx = m_Trees.DescribeFor(s, me);
		if (!ctx || m_Trees.Refuses(ctx))
			return false;

		string ident = m_GM.GetProfile(pid).m_sIdentity;
		float said;
		if (m_mSpeaker.Find(ident + "|" + ctx.m_sKey, said) && m_fClock - said < RZ_DialogueSettings.Get().m_fBarkPerSpeaker)
			return false;

		array<RZ_DConv> all = {};
		m_Trees.GetDB().ForSpeaker(ctx.m_aKinds, all);
		RZ_DConv best;
		foreach (RZ_DConv c : all)
		{
			if (!c.IsBark() || c.m_aNodes.IsEmpty() || !m_Trees.FreeFor(ctx, c) || !RZ_DialogueCond.Holds(ctx, c.m_When))
				continue;

			if (!best || c.m_iPrio < best.m_iPrio)
				best = c;
		}

		if (!best)
			return false;

		array<string> lines = {};
		m_Trees.LinesFor(ctx, best.m_aNodes[0], lines);
		if (lines.IsEmpty())
			return false;

		m_Trees.NoteFor(ctx, best);
		m_mSpeaker.Set(ident + "|" + ctx.m_sKey, m_fClock);
		string name = ctx.m_sName;
		if (!s.m_sName.IsEmpty())
			name = s.m_sName;

		m_GM.Say(pid, name, ctx.m_iFaction, lines[0]);
		RZ_Log.Info(string.Format("Dialogue: bark %1 to %2", best.m_sId, RZ_Util.PlayerName(pid)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_mLast.Remove(playerId);
	}
}
