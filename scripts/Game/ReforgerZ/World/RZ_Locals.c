//! Locals (docs/ERRANDS.md, "Locals"): the people the season remembers. A local is made when an
//! errand or a stranger ends with someone alive and placed: a seeded name, a short role, one line, a
//! home that is always a safehouse, a spot there (the gate, the stove, the bench), and what each
//! player did for them (helped, saved, lied to, crossed, sold). Pinned locals (a big act: saved,
//! sold, promised) are kept to the season's end, 30 a server and 4 a house; unpinned ones, 20, and
//! the oldest nobody has met for 3 real days goes first. A local is shown on one of the house's
//! plain occupants while a player is near (never on a named person or a leader), counts inside the
//! house's people, and can be among those it loses: the news goes only to players who knew them.
//! Across lives a local remembers the player for the season but greets a new life as a stranger,
//! with one sideways line. A local's news about you is a personal line to you only. Saved with the
//! errands. Server only.
class RZ_Local
{
	int id;
	string name;
	string role;			// "the man with the empty sack"
	string line;			// what they say to someone who helped them
	int house = -1;			// home: a safehouse index
	string spot;			// "gate", "stove", "bench"
	bool pinned;
	bool alive = true;
	int madeUnix;
	int metUnix;			// the last time someone who knew them was near
	ref array<string> idents = {};
	ref array<string> memos = {};		// helped, saved, lied, crossed, sold
	ref array<int> lives = {};			// the life each memo was made in
}

//------------------------------------------------------------------------------------------------
class RZ_Locals
{
	static const int TAG = -15;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: a local
	static const int MAX_PINNED = 30;
	static const int PINNED_PER_HOUSE = 4;
	static const int MAX_UNPINNED = 20;
	protected static const int STALE = 259200;	// 3 real days unmet

	//------------------------------------------------------------------------------------------------
	//! Pinned locals on the server (Missions > Errands > Pinned locals; MAX_PINNED by default).
	protected static int MaxPinned()
	{
		return RZ_ErrandSettings.Get().m_iPinnedLocals;
	}

	protected RZ_GameModeComponent m_GM;
	protected ref array<ref RZ_Local> m_aLocals = {};
	protected ref map<int, IEntity> m_mBodies = new map<int, IEntity>();	// local id -> the occupant wearing them
	protected int m_iNextId = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Locals(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aLocals.Count();
	}

	//------------------------------------------------------------------------------------------------
	RZ_Local Get(int id)
	{
		foreach (RZ_Local l : m_aLocals)
		{
			if (l.id == id)
				return l;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A new local at `house` (nobody without a home), remembered by this player as `memo`. Returns
	//! the local, or null if there's no room (pinned: 30, 4 a house).
	RZ_Local Make(int playerId, string role, string line, int house, string spot, bool pinned, string memo)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || !houses.Get(house))
			return null;

		if (pinned && (CountPinned(-1) >= MaxPinned() || CountPinned(house) >= PINNED_PER_HOUSE))
			pinned = false;

		if (!pinned)
			MakeRoom();

		RZ_Local l = new RZ_Local();
		l.id = m_iNextId;
		m_iNextId++;
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + l.id * 104729 + house);
		l.name = RZ_FactionVoice.PersonName(rng, RZ_EFaction.SURVIVORS);
		l.role = role;
		l.line = line;
		l.house = house;
		l.spot = spot;
		l.pinned = pinned;
		l.madeUnix = RZ_SeasonClock.Now();
		l.metUnix = l.madeUnix;
		m_aLocals.Insert(l);
		if (playerId > 0)
			Remember(l, playerId, memo);

		RZ_Log.Info(string.Format("Locals: %1 (%2) made at %3 by %4 (%5)", l.name, role, houses.Get(house).m_sName, RZ_Util.PlayerName(playerId), PinWord(pinned)));
		return l;
	}

	//------------------------------------------------------------------------------------------------
	protected static string PinWord(bool pinned)
	{
		if (pinned)
			return "pinned";

		return "unpinned";
	}

	//------------------------------------------------------------------------------------------------
	protected int CountPinned(int house)
	{
		int n = 0;
		foreach (RZ_Local l : m_aLocals)
		{
			if (l.pinned && (house < 0 || l.house == house))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Unpinned locals past 20: the oldest nobody has met for 3 real days goes (or the oldest).
	protected void MakeRoom()
	{
		int unpinned = 0;
		foreach (RZ_Local l : m_aLocals)
		{
			if (!l.pinned)
				unpinned++;
		}

		if (unpinned < RZ_ErrandSettings.Get().m_iUnpinnedLocals)
			return;

		int now = RZ_SeasonClock.Now();
		int oldest = -1;
		int oldestMet = int.MAX;
		foreach (int i, RZ_Local o : m_aLocals)
		{
			if (o.pinned)
				continue;

			int met = o.metUnix;
			if (now - met < STALE)
				met += STALE * 10;	// still met lately: only if nobody else will do

			if (met < oldestMet)
			{
				oldestMet = met;
				oldest = i;
			}
		}

		if (oldest >= 0)
		{
			m_mBodies.Remove(m_aLocals[oldest].id);
			m_aLocals.Remove(oldest);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What this player did for them, on the life they did it in (a newer memo replaces an older one).
	void Remember(RZ_Local l, int playerId, string memo)
	{
		if (!l || playerId <= 0 || memo.IsEmpty())
			return;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		int i = l.idents.Find(profile.m_sIdentity);
		if (i < 0)
		{
			l.idents.Insert(profile.m_sIdentity);
			l.memos.Insert(memo);
			l.lives.Insert(profile.m_iCharacter);
		}
		else
		{
			l.memos[i] = memo;
			l.lives[i] = profile.m_iCharacter;
		}

		if (memo == "saved" || memo == "sold" || memo == "promised")
			l.pinned = l.pinned || (CountPinned(-1) < MaxPinned() && CountPinned(l.house) < PINNED_PER_HOUSE);
	}

	//------------------------------------------------------------------------------------------------
	//! What this identity is to them: "" if nothing.
	string MemoOf(RZ_Local l, string ident)
	{
		int i = l.idents.Find(ident);
		if (i < 0)
			return "";

		return l.memos[i];
	}

	//------------------------------------------------------------------------------------------------
	//! Locals living at this house, alive.
	void AtHouse(int house, notnull array<RZ_Local> result)
	{
		foreach (RZ_Local l : m_aLocals)
		{
			if (l.alive && l.house == house)
				result.Insert(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_NamedPeople.DressHouse: the house's plain occupants after its named people wear its
	//! locals (one body each, the most lately met first). Leaders and named people keep their bodies.
	void Dress(int house, notnull array<IEntity> spare)
	{
		array<RZ_Local> here = {};
		AtHouse(house, here);
		int used = 0;
		for (int k = here.Count() - 1; k >= 0 && used < spare.Count(); k--)
		{
			RZ_Local l = here[k];
			IEntity body = spare[used];
			used++;
			if (!body || !body.FindComponent(RZ_PuzzleObjectComponent))
				continue;

			RZ_PuzzleObjectComponent.Tag(body, TAG, l.id, "", "Ask how they are");
			RZ_ClueComponent.Tag(body, TAG, l.id, "Talk to " + l.name);
			m_mBodies.Set(l.id, body);
		}
	}

	//------------------------------------------------------------------------------------------------
	IEntity BodyOf(int id)
	{
		IEntity e;
		m_mBodies.Find(id, e);
		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! Talk to a local: their line about you (this life), a sideways line (an earlier life), or a
	//! stranger's nod. Personal: to you only.
	void Talk(int playerId, int id)
	{
		RZ_Local l = Get(id);
		if (!l || !l.alive)
			return;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		int i = l.idents.Find(profile.m_sIdentity);
		string title = string.Format("%1, %2", l.name, l.role);
		l.metUnix = RZ_SeasonClock.Now();
		if (i < 0)
		{
			m_GM.Say(playerId, title, RZ_EFaction.SURVIVORS, "Evening. You'll be new. Mind the step, it's loose.");
			return;
		}

		if (l.lives[i] != profile.m_iCharacter)
		{
			m_GM.Say(playerId, title, RZ_EFaction.SURVIVORS, "You've a look of someone. Never mind. Evening.");
			return;
		}

		m_GM.Say(playerId, title, RZ_EFaction.SURVIVORS, LineFor(l, l.memos[i], l.metUnix / 3600));
	}

	//------------------------------------------------------------------------------------------------
	protected static void Three(notnull array<string> lines, string a, string b, string c)
	{
		lines.Insert(a);
		lines.Insert(b);
		lines.Insert(c);
	}

	//------------------------------------------------------------------------------------------------
	//! Three lines a memo, turning with the real hour (`turn`); helped and saved use their own line first.
	protected static string LineFor(RZ_Local l, string memo, int turn)
	{
		array<string> lines = {};
		switch (memo)
		{
			case "lied":
				Three(lines, "You told me a thing that wasn't so. I remember. Everyone here remembers.", "Oh, it's you. Say something true and I'll write it down. I'll need the paper.", "I believed you. I'm not proud of it. I'm not proud of you either.");
				break;
			case "crossed":
				Three(lines, "I've nothing to say to you. The step's still loose. Mind it.", "Not you. Not tonight. Try the others.", "You know what you did. So do I. That's two of us too many.");
				break;
			case "sold":
				Three(lines, "I know what you did. Don't make me say it in front of them.", "There's a chair empty at supper. You know whose.", "Go on, then. Walk past. You're good at it.");
				break;
			case "promised":
				Three(lines, "You said you'd bring it when it's made. I'm still here. I'll be here.", "Is it made yet? No. I know. I ask everyone. I ask you differently.", "Still waiting. Hands in my lap, like you said.");
				break;
			default:
				if (!l.line.IsEmpty())
					lines.Insert(l.line);
				else
					lines.Insert("It's you. Sit a minute, if you've a minute.");

				lines.Insert("Look who it is. The kettle's on. It's always on. Nothing in it.");
				lines.Insert("You came back. People don't, much. Sit.");
				break;
		}

		return lines[Math.AbsInt(turn) % lines.Count()];
	}

	//------------------------------------------------------------------------------------------------
	//! A house lost `count` people (RZ_Errands.WatchHouses): its locals are among them as often as
	//! their share. The news goes only to the players who knew them, online now.
	void OnHouseLost(int house, int count, int before)
	{
		if (count <= 0 || before <= 0)
			return;

		array<RZ_Local> here = {};
		AtHouse(house, here);
		float chance = Math.Min(1, count / Math.Max(1.0, before));
		foreach (RZ_Local l : here)
		{
			if (Math.RandomFloat01() >= chance)
				continue;

			l.alive = false;
			m_mBodies.Remove(l.id);
			RZ_Safehouse h = m_GM.GetSafehouses().Get(house);
			string place = "the house";
			if (h)
				place = h.m_sName;

			array<int> ids = {};
			array<IEntity> players = {};
			RZ_Util.GetAlivePlayers(ids, players);
			foreach (int pid : ids)
			{
				if (l.idents.Contains(m_GM.GetProfile(pid).m_sIdentity))
					m_GM.NotifyPlayer(pid, "Word from " + place, string.Format("%1, %2, didn't come down to breakfast. The house buried them by the wall.", l.name, l.role));
			}

			RZ_Log.Info(string.Format("Locals: %1 at %2 died with the house's losses", l.name, place));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Someone killed a local's body: they're dead, and the killer pays what killing a wanderer costs.
	void Killed(int id, int killer)
	{
		RZ_Local l = Get(id);
		if (!l || !l.alive)
			return;

		l.alive = false;
		m_mBodies.Remove(id);
		if (killer > 0)
			m_GM.AddInfamy(killer, RZ_EFaction.SURVIVORS, 15, "killed a local", 0, "killed someone the house knew");

		RZ_Log.Info(string.Format("Locals: %1 killed (by %2)", l.name, RZ_Util.PlayerName(killer)));
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 s: a dressed local whose body died was killed (by whoever the engine says).
	void Watch()
	{
		array<int> dead = {};
		foreach (int id, IEntity body : m_mBodies)
		{
			if (body && !RZ_Util.IsCharacterAlive(body))
				dead.Insert(id);
		}

		foreach (int d : dead)
		{
			Killed(d, RZ_Util.GetKillerPlayerId(BodyOf(d)));
		}
	}

	//------------------------------------------------------------------------------------------------
	void Save(notnull array<ref RZ_Local> result)
	{
		foreach (RZ_Local l : m_aLocals)
		{
			result.Insert(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Restore(array<ref RZ_Local> saved)
	{
		if (!saved)
			return;

		foreach (RZ_Local l : saved)
		{
			if (!l || l.house < 0)
				continue;

			if (!l.idents)
				l.idents = new array<string>();

			if (!l.memos)
				l.memos = new array<string>();

			if (!l.lives)
				l.lives = new array<int>();

			m_aLocals.Insert(l);
			m_iNextId = Math.Max(m_iNextId, l.id + 1);
		}
	}
}
