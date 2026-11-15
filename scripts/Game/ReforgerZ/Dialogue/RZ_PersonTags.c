//! Who the people are that nobody else names (docs/UI.md, "Nametags"): a holding's or a safehouse's
//! guards, a road post's crew, a road patrol, a supply truck's crew, a camp's extras, the
//! quartermasters and the medic. The system that spawns a body marks it with a stable key (the
//! place and the slot: "h12:g0", "s3:o2", "patrol:4"); the name comes from the season seed and that
//! key, so the same person has the same name each time they spawn, after a restart too. A group is
//! marked as a whole: its members are named as they're first asked about, slot by slot in the
//! group's own order (the same order as their looks). A slot whose person died (seen every 10 s)
//! moves on a generation, kept as a season flag ("name.<key>"), so whoever fills it next is someone
//! new. The talk screen's own people (leaders, named people, wanderers...) keep their own names;
//! this is only asked about where those have none. Changes nothing in the game. Server only.

//------------------------------------------------------------------------------------------------
class RZ_PersonTag
{
	IEntity m_Body;			// weak: the character, or the AI group it was marked on
	string m_sKey;			// the stable identity; a group's members: "<key>/<slot>"
	string m_sName;			// "" for a group's own entry
	string m_sRole;			// lower case: "sentry", "quartermaster"
	int m_iFaction = -1;
	bool m_bGroup;			// an AI group: names its members as they're asked about
	int m_iSlot = -1;		// a group member's slot
	string m_sGroupKey;		// a group member: the group's key
}

//------------------------------------------------------------------------------------------------
class RZ_PersonTags
{
	protected static const string FLAG = "name.";	// season flag: a slot's generation once its person died
	protected static const int MAX = 800;			// entries before the dead and the gone are swept at once
	protected static const int SLOTS = 16;			// a group's slots

	protected static ref map<int, ref RZ_PersonTag> s_mTags;		// a body's RplId -> its tag
	protected static ref array<ref RZ_PersonTag> s_aPending;		// marked before its RplId was there

	//------------------------------------------------------------------------------------------------
	//! A spawned unit (a character or an AI group) is the person, or the people, of `key`.
	static void Mark(IEntity unit, string key, int faction, string role)
	{
		if (!unit || key.IsEmpty())
			return;

		RZ_PersonTag t = new RZ_PersonTag();
		t.m_Body = unit;
		t.m_sKey = key;
		t.m_sRole = role;
		t.m_iFaction = faction;
		if (ChimeraCharacter.Cast(unit))
			t.m_sName = NameFor(key, faction, role);
		else if (SCR_AIGroup.Cast(unit))
			t.m_bGroup = true;
		else
			return;

		Put(t);
	}

	//------------------------------------------------------------------------------------------------
	//! The tag of a living character: its own, or one given now as a member of a marked group; null.
	static RZ_PersonTag Get(IEntity body)
	{
		if (!body)
			return null;

		Flush();
		RZ_PersonTag own = Find(body);
		if (own)
			return own;

		IEntity group = RZ_Util.GroupOf(body);
		if (!group)
			return null;

		RZ_PersonTag g = Find(group);
		if (!g || !g.m_bGroup)
			return null;

		NameMembers(g);
		return Find(body);
	}

	//------------------------------------------------------------------------------------------------
	//! The key a unit was marked with ("" none): a character's own, or a group's.
	static string KeyOf(IEntity unit)
	{
		RZ_PersonTag t = Get(unit);
		if (!t)
			return "";

		return t.m_sKey;
	}

	//------------------------------------------------------------------------------------------------
	//! A living person holds this key now.
	static bool InUse(string key)
	{
		Flush();
		if (!s_mTags)
			return false;

		foreach (int id, RZ_PersonTag t : s_mTags)
		{
			if (t.m_sKey == key && t.m_Body && RZ_Util.IsCharacterAlive(t.m_Body))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The name of someone with a Talk prompt and no name of their own (a quartermaster, the medic):
	//! marked now if nobody marked them at their spawn, by their role, side and where they stand
	//! (a placed one never moves, so it's the same person after a restart).
	static string StaffName(IEntity body, string role, int faction)
	{
		RZ_PersonTag t = Get(body);
		if (!t)
		{
			vector at = body.GetOrigin();
			string key = string.Format("staff:%1:%2:%3:%4", role, faction, RZ_Util.RoundToInt(at[0] / 5), RZ_Util.RoundToInt(at[2] / 5));
			Mark(body, key, faction, role);
			t = Get(body);
		}

		if (!t)
			return "";

		return t.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! The name a key's person has this season ("Private Emil Kral"): the same every time, until the
	//! slot's person dies.
	static string NameFor(string key, int faction, string role)
	{
		int seed = 1;
		int generation = 0;
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (facts)
		{
			seed = facts.GetSeed();
			generation = facts.Flag(FLAG + key);
		}

		string salted = key + "#" + generation.ToString();
		RZ_Rng rng = new RZ_Rng(seed + salted.Hash()); // VERIFY: string.Hash() is the same on every run (the dialogue and the finale already seed from it)
		return RZ_FactionVoice.PersonNameFor(rng, faction, role);
	}

	//------------------------------------------------------------------------------------------------
	//! A camp extra's work by its key ("cook", "mechanic"), the same each time.
	static string RoleFor(string key, int faction)
	{
		RZ_Rng rng = new RZ_Rng(key.Hash() + 7);
		if (!RZ_Factions.IsValid(faction))
			return "survivor";

		return RZ_FactionVoice.PersonRole(rng, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! A guard's word by side: a Haven or Halcyon guard, a Bastion sentry, a Red Hand lookout.
	static string GuardRole(int faction)
	{
		if (faction == RZ_EFaction.MILITARY)
			return "sentry";

		if (faction == RZ_EFaction.BANDITS)
			return "lookout";

		return "guard";
	}

	//------------------------------------------------------------------------------------------------
	//! Every 10 s from the nametags: the gone are dropped, the dead are dropped and their slot moves
	//! on a generation (whoever comes next in it is someone new).
	static void Sweep()
	{
		Flush();
		if (!s_mTags)
			return;

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		array<int> drop = {};
		foreach (int id, RZ_PersonTag t : s_mTags)
		{
			if (!t.m_Body)
			{
				drop.Insert(id);
				continue;
			}

			if (t.m_bGroup || RZ_Util.IsCharacterAlive(t.m_Body))
				continue;

			drop.Insert(id);
			if (facts)
				facts.SetFlag(FLAG + t.m_sKey, facts.Flag(FLAG + t.m_sKey) + 1);
		}

		foreach (int gone : drop)
		{
			s_mTags.Remove(gone);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Put(RZ_PersonTag t)
	{
		if (!s_mTags)
			s_mTags = new map<int, ref RZ_PersonTag>();

		if (!s_aPending)
			s_aPending = {};

		if (s_mTags.Count() > MAX)
			Sweep();

		int id;
		if (IdOf(t.m_Body, id))
			s_mTags.Set(id, t);
		else
			s_aPending.Insert(t);
	}

	//------------------------------------------------------------------------------------------------
	//! The ones marked the moment they spawned, once replication has given them an id.
	protected static void Flush()
	{
		if (!s_aPending || s_aPending.IsEmpty())
			return;

		for (int i = s_aPending.Count() - 1; i >= 0; i--)
		{
			RZ_PersonTag t = s_aPending[i];
			int id;
			if (!t.m_Body)
			{
				s_aPending.Remove(i);
			}
			else if (IdOf(t.m_Body, id))
			{
				s_mTags.Set(id, t);
				s_aPending.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_PersonTag Find(IEntity e)
	{
		int id;
		if (!s_mTags || !IdOf(e, id))
			return null;

		RZ_PersonTag t;
		if (!s_mTags.Find(id, t) || t.m_Body != e)
			return null;

		return t;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IdOf(IEntity e, out int id)
	{
		id = -1;
		if (!e)
			return false;

		RplComponent rpl = RplComponent.Cast(e.FindComponent(RplComponent));
		if (!rpl)
			return false;

		RplId rid = rpl.Id();
		if (rid == RplId.Invalid()) // VERIFY: a body spawned on the server has its RplId at once (else it waits in s_aPending)
			return false;

		id = rid;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A marked group's members without a tag get the lowest free slots, in the group's order.
	protected static void NameMembers(RZ_PersonTag g)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(g.m_Body);
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		array<bool> taken = {};
		for (int s = 0; s < SLOTS; s++)
		{
			taken.Insert(false);
		}

		array<IEntity> fresh = {};
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity member = agent.GetControlledEntity();
			if (!member)
				continue;

			RZ_PersonTag known = Find(member);
			if (known && known.m_sGroupKey == g.m_sKey && known.m_iSlot >= 0 && known.m_iSlot < SLOTS)
				taken[known.m_iSlot] = true;
			else if (!known)
				fresh.Insert(member);
		}

		int next = 0;
		foreach (IEntity body : fresh)
		{
			while (next < SLOTS && taken[next])
			{
				next++;
			}

			if (next >= SLOTS)
				return;

			RZ_PersonTag t = new RZ_PersonTag();
			t.m_Body = body;
			t.m_sKey = g.m_sKey + "/" + next.ToString();
			t.m_sGroupKey = g.m_sKey;
			t.m_iSlot = next;
			t.m_sRole = g.m_sRole;
			t.m_iFaction = g.m_iFaction;
			t.m_sName = NameFor(t.m_sKey, g.m_iFaction, g.m_sRole);
			taken[next] = true;
			Put(t);
		}
	}
}
