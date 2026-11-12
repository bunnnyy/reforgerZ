//! Nametags, the server's half (docs/UI.md, "Nametags"). Every POLL seconds, for each living
//! player: the people they could talk to within Dialogue > Nametag range and a margin (a sphere
//! query round the player, as barks do), the nearest LIST_MAX, each as its body's RplId, name, role
//! and faction, sent to that player only (RZ_UIFeed.PAYLOAD_NAMETAGS) when the list changes. Who
//! someone is comes from the talk screen's adapters (RZ_TalkSpeaker.Describe, RZ_TalkStaff), worked
//! out once a minute per body. Players and the infected are never on it (only living characters
//! with a Talk prompt are). The client draws the tags it can see (UI/RZ_NametagClient.c).
//! Changes nothing in the game. Server only.
class RZ_Nametags
{
	protected static const float POLL = 1.5;
	protected static const float MARGIN = 4;		// the list reaches this far past the range, so a tag shows as you walk up
	protected static const float KNOWN_FOR = 60;	// seconds a body's tag is kept before it's worked out again
	protected static const int LIST_MAX = 4;		// bodies on one player's list
	protected static const int FOUND_MAX = 12;		// candidates one query keeps
	protected static const int KNOWN_MAX = 400;		// tags remembered before the memory starts again

	protected RZ_GameModeComponent m_GM;
	protected float m_fClock;
	protected float m_fNext;
	protected ref map<int, string> m_mSent = new map<int, string>();	// player -> the list last sent
	protected ref map<int, string> m_mTag = new map<int, string>();		// a body's RplId -> "name|role|faction" ("" not a talker)
	protected ref map<int, float> m_mTagAt = new map<int, float>();
	protected ref array<IEntity> m_aFound = {};

	//------------------------------------------------------------------------------------------------
	void RZ_Nametags(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! On the dialogue's tick.
	void Tick(float dt)
	{
		m_fClock += dt;
		if (m_fClock < m_fNext)
			return;

		m_fNext = m_fClock + POLL;
		RZ_DialogueSettings settings = RZ_DialogueSettings.Get();
		if (!settings.m_bNametags)
			return;

		if (m_mTag.Count() > KNOWN_MAX)
		{
			m_mTag.Clear();
			m_mTagAt.Clear();
		}

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		float reach = settings.m_fNametagRange + MARGIN;
		foreach (int i, IEntity me : players)
		{
			int pid = ids[i];
			string list = ListFor(pid, me, reach);
			string last;
			bool known = m_mSent.Find(pid, last);
			if (known && last == list)
				continue;

			m_mSent.Set(pid, list);
			if (!known && list.IsEmpty())
				continue;

			RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(pid);
			if (ui)
				ui.SendPayload(RZ_UIFeed.PAYLOAD_NAMETAGS, list);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_mSent.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest people to talk to round `me`, one N|id|name|role|faction record each, in a fixed
	//! order (so the list only changes when who's there does).
	protected string ListFor(int pid, IEntity me, float reach)
	{
		m_aFound.Clear();
		vector here = me.GetOrigin();
		GetGame().GetWorld().QueryEntitiesBySphere(here, reach, OnEntity, null, EQueryEntitiesFlags.DYNAMIC);
		array<string> records = {};
		while (!m_aFound.IsEmpty() && records.Count() < LIST_MAX)
		{
			int best = 0;
			for (int k = 1; k < m_aFound.Count(); k++)
			{
				if (vector.DistanceSq(m_aFound[k].GetOrigin(), here) < vector.DistanceSq(m_aFound[best].GetOrigin(), here))
					best = k;
			}

			IEntity body = m_aFound[best];
			m_aFound.Remove(best);
			string record = RecordOf(pid, me, body);
			if (!record.IsEmpty())
				records.Insert(record);
		}

		records.Sort();
		string list = "";
		foreach (int n, string rec : records)
		{
			if (n > 0)
				list += "\n";

			list += rec;
		}

		return list;
	}

	//------------------------------------------------------------------------------------------------
	//! A living character that isn't a player and has a Talk prompt: an RZ_TalkAction, or the
	//! puzzle components' read (not hidden). The infected have neither.
	protected bool OnEntity(IEntity e)
	{
		if (m_aFound.Count() >= FOUND_MAX)
			return false;

		if (!RZ_Dialogue.IsPerson(e) || GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(e) > 0)
			return true;

		RZ_ClueComponent clue = RZ_ClueComponent.Cast(e.FindComponent(RZ_ClueComponent));
		if (RZ_TalkAction.On(e) || (clue && clue.GetReadLabel() != RZ_ClueComponent.NO_READ))
			m_aFound.Insert(e);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "N|id|name|role|faction" for this body, or "" when there's nothing to call them.
	protected string RecordOf(int pid, IEntity me, IEntity body)
	{
		RplComponent rpl = RplComponent.Cast(body.FindComponent(RplComponent));
		if (!rpl)
			return "";

		int id = rpl.Id();
		string tag;
		float at;
		if (!m_mTag.Find(id, tag) || !m_mTagAt.Find(id, at) || m_fClock - at > KNOWN_FOR)
		{
			tag = Describe(pid, me, body);
			m_mTag.Set(id, tag);
			m_mTagAt.Set(id, m_fClock);
		}

		if (tag.IsEmpty())
			return "";

		return "N" + RZ_UIText.SEP + id.ToString() + RZ_UIText.SEP + tag;
	}

	//------------------------------------------------------------------------------------------------
	//! "name|role|faction" as the talk screen would call them; "" if it can't say who they are.
	protected string Describe(int pid, IEntity me, IEntity body)
	{
		string name = "";
		string role = "";
		int faction = -1;
		RZ_TalkSession s = new RZ_TalkSession();
		s.m_iPlayer = pid;
		s.m_Speaker = body;
		s.m_Adapter = RZ_TalkSpeaker.For(m_GM, body);
		RZ_TalkStaff staff = RZ_TalkStaff.Cast(s.m_Adapter);
		if (staff)
		{
			staff.Setup(m_GM, s, pid);
			name = RZ_Util.Capitalize(RZ_TalkAction.On(body).GetRole());
			faction = s.m_iFaction;
		}
		else if (s.m_Adapter)
		{
			RZ_DCtx ctx = new RZ_DCtx();
			ctx.m_GM = m_GM;
			ctx.m_iPlayer = pid;
			ctx.m_User = me;
			ctx.m_Body = body;
			if (s.m_Adapter.Describe(m_GM, s, ctx) && !ctx.m_aKinds.IsEmpty())
			{
				faction = ctx.m_iFaction;
				name = TagName(ctx);
				role = TagRole(ctx);
			}
		}

		// Nobody the talk screen can describe: what their Talk prompt calls them ("Talk to the captain")
		if (name.IsEmpty())
			name = FromRead(body);

		if (name.IsEmpty())
			return "";

		if (!RZ_Factions.IsValid(faction))
			faction = -1;

		string sep = RZ_UIText.SEP;
		return RZ_UIText.Clean(RZ_Util.Capitalize(name), 40) + sep + RZ_UIText.Clean(RZ_Util.Capitalize(role), 40) + sep + faction.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The name a tag shows: a wanderer's own name, "Resident" for a house's people, "Stranger";
	//! nothing for a mission's people (their Talk prompt names them).
	protected static string TagName(RZ_DCtx ctx)
	{
		if (ctx.m_Wanderer)
			return ctx.m_sShort;

		if (ctx.m_aKinds.Contains("occupant"))
			return "Resident";

		if (ctx.m_aKinds.Contains("stranger"))
			return "Stranger";

		if (ctx.m_aKinds.Contains("mission") || ctx.m_aKinds.Contains("quest"))
			return "";

		return ctx.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! The role a tag shows, where it says something the name doesn't: none for a house's people, a
	//! stranger, the Long Table, a post or a mission's people.
	protected static string TagRole(RZ_DCtx ctx)
	{
		array<string> plain = {"occupant", "stranger", "mission", "quest", "table", "post"};
		foreach (string kind : plain)
		{
			if (ctx.m_aKinds.Contains(kind))
				return "";
		}

		return ctx.m_sRole;
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to the captain" -> "the captain"; "" for anything else ("Talk to them" included).
	protected static string FromRead(IEntity body)
	{
		RZ_ClueComponent clue = RZ_ClueComponent.Cast(body.FindComponent(RZ_ClueComponent));
		if (!clue)
			return "";

		string label = clue.GetReadLabel();
		string verb = "Talk to ";
		if (!label.StartsWith(verb))
			return "";

		string who = label.Substring(verb.Length(), label.Length() - verb.Length());
		if (who == "them")
			return "";

		return who;
	}
}
