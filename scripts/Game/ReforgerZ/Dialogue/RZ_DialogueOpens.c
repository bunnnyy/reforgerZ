//! Server-opened greets (docs/DIALOGUE.md, "Who opens a conversation"): a greet written with
//! `opens approach <metres>` opens the talk screen by itself for a player who walks up within that
//! distance while its `when` holds, so the question is put whether or not they press Talk: a road
//! post's speaker asking someone in their kit below Known which unit they're with (35 m), a stranger
//! calling out (10 m), the host on the road. With Dialogue > Trees on, on the barks' rhythm (every
//! 5 s), for bodies already spawned near players. Never while the player is talking, in a vehicle,
//! in danger (the screen guard refuses), or within 10 minutes of the same speaker opening on them;
//! each is an unasked contact inside the notice lanes' budget (one every 4 minutes). Server only.
class RZ_DialogueOpens
{
	protected static const float EVERY = 5;
	protected static const float MAX_RADIUS = 40;
	protected static const float PER_SPEAKER = 600;

	protected float m_fClock;
	protected float m_fNext;
	protected int m_iOpens = -1;		// conversations that open by themselves (counted once)
	protected ref map<string, float> m_mOpened = new map<string, float>();	// "ident|speaker" -> clock
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
			if (!Opens(trees, ctx, dist))
				continue;

			RZ_EContact kind = RZ_EContact.WANDERER;
			if (ctx.m_iTableRole >= 0)
				kind = RZ_EContact.HOST;

			RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
			if (lanes && !lanes.TryContact(pid, kind))
				return;

			m_mOpened.Set(ident + "|" + ctx.m_sKey, m_fClock);
			if (dialogue.Open(pid, body, me))
				RZ_Log.Info(string.Format("Dialogue: %1 opened on %2 at %3 m", ctx.m_sKey, RZ_Util.PlayerName(pid), RZ_Util.RoundToInt(dist)));

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A greet for this speaker that opens by itself at this distance, and holds now.
	protected static bool Opens(RZ_DialogueTrees trees, RZ_DCtx ctx, float dist)
	{
		array<RZ_DConv> all = {};
		trees.GetDB().ForSpeaker(ctx.m_aKinds, all);
		foreach (RZ_DConv c : all)
		{
			if (c.m_fOpens <= 0 || dist > c.m_fOpens || !c.IsGreet() || !trees.FreeFor(ctx, c))
				continue;

			if (RZ_DialogueCond.Holds(ctx, c.m_When))
				return true;
		}

		return false;
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
