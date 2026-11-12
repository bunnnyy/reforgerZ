//! Call-outs (docs/DIALOGUE.md, "Who opens a conversation"): a greet written with
//! `opens approach <metres>` calls out to a player who walks up within that distance while its
//! `when` holds: its first line as that person's subtitle, with their name (a road post's speaker
//! asking someone in their kit below Known which unit they're with, 35 m; a stranger, 10 m; the
//! host on the road). It never opens the talk screen: that opens only when the player talks to
//! them, and the same greet puts its question there. With Dialogue > Trees on, on the barks' rhythm
//! (every 5 s), for bodies already spawned near players. Never while the player is talking or in a
//! vehicle, or within 10 minutes of the same speaker calling out to them; each is an unasked contact
//! inside the notice lanes' budget (one every 4 minutes). The greet isn't used up by calling out.
//! Server only.
class RZ_DialogueOpens
{
	protected static const float EVERY = 5;
	protected static const float MAX_RADIUS = 40;
	protected static const float PER_SPEAKER = 600;

	protected float m_fClock;
	protected float m_fNext;
	protected int m_iOpens = -1;		// greets that call out (counted once)
	protected ref map<string, float> m_mOpened = new map<string, float>();	// "ident|speaker" -> clock of the last call-out
	protected ref array<IEntity> m_aFound = {};

	//------------------------------------------------------------------------------------------------
	void Tick(RZ_GameModeComponent gm, float dt)
	{
		m_fClock += dt;
		if (m_fClock < m_fNext)
			return;

		m_fNext = m_fClock + EVERY;
		if (!RZ_DialogueSettings.Get().m_bOpens)
			return;

		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		RZ_Dialogue dialogue = gm.GetDialogue();
		if (!trees || !dialogue || !AnyOpens(trees))
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity me : players)
		{
			int pid = ids[i];
			if (dialogue.GetSession(pid) || RZ_Util.IsInVehicle(me))
				continue;

			TryNear(gm, trees, dialogue, pid, me);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool AnyOpens(RZ_DialogueTrees trees)
	{
		if (m_iOpens < 0)
			m_iOpens = trees.GetDB().CountOpens();

		return m_iOpens > 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void TryNear(RZ_GameModeComponent gm, RZ_DialogueTrees trees, RZ_Dialogue dialogue, int pid, IEntity me)
	{
		m_aFound.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(me.GetOrigin(), MAX_RADIUS, OnEntity, null, EQueryEntitiesFlags.DYNAMIC);
		string ident = gm.GetProfile(pid).m_sIdentity;
		foreach (IEntity body : m_aFound)
		{
			RZ_TalkSession s = new RZ_TalkSession();
			s.m_iPlayer = pid;
			s.m_Speaker = body;
			s.m_Adapter = RZ_TalkSpeaker.For(gm, body);
			if (!s.m_Adapter)
				continue;

			s.m_Adapter.Setup(gm, s, pid);
			RZ_DCtx ctx = trees.DescribeFor(s, me);
			if (!ctx || trees.Refuses(ctx))
				continue;

			float opened;
			if (m_mOpened.Find(ident + "|" + ctx.m_sKey, opened) && m_fClock - opened < PER_SPEAKER)
				continue;

			float dist = RZ_Util.DistXZ(body.GetOrigin(), me.GetOrigin());
			RZ_DConv greet = Opens(trees, ctx, dist);
			if (!greet)
				continue;

			array<string> lines = {};
			trees.LinesFor(ctx, greet.m_aNodes[0], lines);
			if (lines.IsEmpty())
				continue;

			RZ_EContact kind = RZ_EContact.WANDERER;
			if (ctx.m_iTableRole >= 0)
				kind = RZ_EContact.HOST;

			RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
			if (lanes && !lanes.TryContact(pid, kind))
				return;

			m_mOpened.Set(ident + "|" + ctx.m_sKey, m_fClock);
			string name = ctx.m_sName;
			if (!s.m_sName.IsEmpty())
				name = s.m_sName;

			gm.Say(pid, name, ctx.m_iFaction, lines[0], body);
			RZ_Log.Info(string.Format("Dialogue: %1 called out to %2 at %3 m", ctx.m_sKey, RZ_Util.PlayerName(pid), RZ_Util.RoundToInt(dist)));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A greet for this speaker that calls out at this distance, and holds now; null for none.
	protected static RZ_DConv Opens(RZ_DialogueTrees trees, RZ_DCtx ctx, float dist)
	{
		array<RZ_DConv> all = {};
		trees.GetDB().ForSpeaker(ctx.m_aKinds, all);
		foreach (RZ_DConv c : all)
		{
			if (c.m_fOpens <= 0 || dist > c.m_fOpens || !c.IsGreet() || c.m_aNodes.IsEmpty() || !trees.FreeFor(ctx, c))
				continue;

			if (RZ_DialogueCond.Holds(ctx, c.m_When))
				return c;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (m_aFound.Count() >= 8)
			return false;

		if (!ChimeraCharacter.Cast(e) || !RZ_Util.IsCharacterAlive(e) || GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(e) > 0)
			return true;

		if (e.FindComponent(RZ_ClueComponent) || e.FindComponent(RZ_PuzzleObjectComponent))
			m_aFound.Insert(e);

		return true;
	}
}
