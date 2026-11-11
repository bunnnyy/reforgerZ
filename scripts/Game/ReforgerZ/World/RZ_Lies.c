//! The one lie store (docs/DESIGN.md, "Lies and witnesses"), shared by missions, errands and
//! dialogue: per player per season, each record with what would expose it. Lies are found out by
//! events first, never by hidden dice:
//! - VISIT: a player Known or better with the faction lied to comes within 30 m of the subject's place;
//! - ROAD: a wanderer passes within 300 m of it (road news carries the truth to the faction's seat);
//! - TICK: the owning system's own check (the next upkeep, the next collection), through Test();
//! - RUMOUR: only for a claim with nothing in the world to test it: 15% at the next half-hour round,
//!   and once more a real day later (+10 for a Talker speaker), then it's buried.
//! Found out, on the life it was told in (if that life is still alive): the speaker's regard records
//! *lied*, Infamy with the faction (2-5, a mission may name more), the life record "lied.<f>" (the
//! liar count: -1 at Trust), and anyone who vouched for the liar answers for it. The life gone, the
//! lie is buried with it. A lie never pays more than the truth: at most 60% and no Fame (each caller's
//! rule). At most 12 a player (the oldest first). Saved with the world memory. Server only.
enum RZ_ELieTest
{
	VISIT,
	ROAD,
	TICK,
	RUMOUR
}

//------------------------------------------------------------------------------------------------
class RZ_Lie
{
	string ident;
	int life;
	string key;			// what was lied about: "colleague.turned", "errand.chalk", "talk.<conv>/@<choice>"
	string subject;		// the place, house or person it's about (Test() matches it)
	int faction = -1;	// the faction lied to (-1: nobody's)
	string speaker;		// the regard key of who was lied to ("" none)
	float x;
	float z;
	int test;			// RZ_ELieTest
	int infamy = 2;
	float createdUnix;	// server time
	int rolls;			// rumour rounds used
	bool talker;
	string voucher;		// the identity who vouched for the liar, "" none
	string found;		// the line the liar hears when it's found out
}

//------------------------------------------------------------------------------------------------
class RZ_Lies
{
	static const int MAX_PER_PLAYER = 12;
	static const float VISIT_RADIUS = 30;
	static const float ROAD_RADIUS = 300;
	static const int RUMOUR_FIRST = 1800;		// seconds: the next half-hour round
	static const int RUMOUR_SECOND = 86400;
	static const float RUMOUR_CHANCE = 0.15;
	static const int VOUCH_INFAMY = 5;

	protected ref array<ref RZ_Lie> m_aLies = {};

	//------------------------------------------------------------------------------------------------
	//! A lie believed: stored with what would expose it. `at` is the subject's place.
	RZ_Lie Tell(RZ_GameModeComponent gm, int playerId, string key, string subject, int faction, string speaker, vector at, int test, int infamy, string foundLine)
	{
		if (!gm || playerId <= 0)
			return null;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		RZ_Lie lie = new RZ_Lie();
		lie.ident = profile.m_sIdentity;
		lie.life = profile.m_iLives;
		lie.key = key;
		lie.subject = subject;
		lie.faction = faction;
		lie.speaker = speaker;
		lie.x = at[0];
		lie.z = at[2];
		lie.test = test;
		lie.infamy = Math.ClampInt(infamy, 0, 20);
		lie.createdUnix = RZ_SeasonClock.Now();
		lie.found = foundLine;

		// The oldest of this player's goes first
		int mine = 0;
		int oldest = -1;
		foreach (int i, RZ_Lie l : m_aLies)
		{
			if (l.ident != lie.ident)
				continue;

			mine++;
			if (oldest < 0)
				oldest = i;
		}

		if (mine >= MAX_PER_PLAYER && oldest >= 0)
			m_aLies.Remove(oldest);

		m_aLies.Insert(lie);
		RZ_Log.Info(string.Format("Story: lie %1 told by %2 (%3)", key, RZ_Util.PlayerName(playerId), TestName(test)));
		return lie;
	}

	//------------------------------------------------------------------------------------------------
	//! A vouch covered this lie: the voucher answers for it if it's found out.
	void Vouched(RZ_Lie lie, string voucherIdent)
	{
		if (lie)
			lie.voucher = voucherIdent;
	}

	//------------------------------------------------------------------------------------------------
	//! The owning system checks: every lie about `key` at `subject` ("" any subject) is found out.
	int Test(RZ_GameModeComponent gm, string key, string subject)
	{
		int n = 0;
		for (int i = m_aLies.Count() - 1; i >= 0; i--)
		{
			RZ_Lie lie = m_aLies[i];
			if (lie.key == key && (subject.IsEmpty() || lie.subject == subject))
			{
				FoundOut(gm, lie, "tick");
				m_aLies.Remove(i);
				n++;
			}
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A lie told by this player in this life (for a mission's own branch). Null if none.
	RZ_Lie Find(RZ_GameModeComponent gm, int playerId, string key)
	{
		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		foreach (RZ_Lie lie : m_aLies)
		{
			if (lie.ident == profile.m_sIdentity && lie.key == key)
				return lie;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Caught now (a call-out, a tell after the fact): found out at once.
	void CatchNow(RZ_GameModeComponent gm, RZ_Lie lie, string how)
	{
		int i = m_aLies.Find(lie);
		if (i < 0)
			return;

		FoundOut(gm, lie, how);
		m_aLies.Remove(i);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 s (from the errands' slow tick): visits, road news and the rumour rounds.
	void Tick(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		int now = RZ_SeasonClock.Now();
		RZ_Wanderers wanderers = gm.GetWanderers();
		for (int i = m_aLies.Count() - 1; i >= 0; i--)
		{
			RZ_Lie lie = m_aLies[i];
			vector at = Vector(lie.x, 0, lie.z);
			string how = "";
			if (lie.test == RZ_ELieTest.VISIT && RZ_Factions.IsValid(lie.faction))
			{
				foreach (int p, IEntity e : players)
				{
					if (RZ_Util.DistXZ(e.GetOrigin(), at) <= VISIT_RADIUS && RZ_Reputation.IsAtLeast(gm.GetTier(ids[p], lie.faction), RZ_ETier.KNOWN) && gm.GetProfile(ids[p]).m_sIdentity != lie.ident)
					{
						how = "visit";
						break;
					}
				}
			}
			else if (lie.test == RZ_ELieTest.ROAD && wanderers && wanderers.AnyNear(at, ROAD_RADIUS))
			{
				how = "road news";
			}
			else if (lie.test == RZ_ELieTest.RUMOUR)
			{
				int due = RUMOUR_FIRST;
				if (lie.rolls > 0)
					due = RUMOUR_SECOND;

				if (now - lie.createdUnix >= due)
				{
					lie.rolls++;
					float chance = RUMOUR_CHANCE;
					if (lie.talker)
						chance += 0.1;

					if (Math.RandomFloat01() < chance)
						how = "rumour";
					else if (lie.rolls >= 2)
					{
						m_aLies.Remove(i);
						continue;
					}
				}
			}

			if (how.IsEmpty())
				continue;

			FoundOut(gm, lie, how);
			m_aLies.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FoundOut(RZ_GameModeComponent gm, RZ_Lie lie, string how)
	{
		RZ_Log.Info(string.Format("Story: lie %1 found out by %2", lie.key, how));
		int playerId = gm.FindPlayerByIdentity(lie.ident);
		RZ_PlayerProfile profile;
		if (playerId > 0)
			profile = gm.GetProfile(playerId);

		// Regard is the player's (identity); Infamy and the liar count only on the life it was told in
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees && !lie.speaker.IsEmpty() && playerId > 0)
			trees.GetMemory().AddRegard(playerId, lie.speaker, "lied", 3);

		if (profile && profile.m_iLives == lie.life)
		{
			if (RZ_Factions.IsValid(lie.faction) && lie.infamy > 0)
				gm.AddInfamy(playerId, lie.faction, lie.infamy, "a lie found out", RZ_RepFlag.SHARE);

			if (RZ_Factions.IsValid(lie.faction))
				RZ_Deeds.AddLifeRecord(profile, "lied." + lie.faction.ToString(), true);

			string line = lie.found;
			if (line.IsEmpty())
				line = "Someone found out what you told them. It'll come up next time you talk.";

			gm.NotifyPlayer(playerId, "Found out", line);
		}

		// Whoever vouched for the liar answers for it, once per vouch
		if (!lie.voucher.IsEmpty() && RZ_Factions.IsValid(lie.faction))
		{
			int voucher = gm.FindPlayerByIdentity(lie.voucher);
			if (voucher > 0)
			{
				gm.AddInfamy(voucher, lie.faction, VOUCH_INFAMY, "vouched for a liar", RZ_RepFlag.SHARE);
				if (trees && !lie.speaker.IsEmpty())
					trees.GetMemory().AddRegard(voucher, lie.speaker, "vouched_liar");

				gm.NotifyPlayer(voucher, RZ_Factions.GetName(lie.faction), string.Format("You vouched for someone who lied to them. +%1 Infamy with %2.", VOUCH_INFAMY, RZ_Factions.GetNameInText(lie.faction)));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static string TestName(int test)
	{
		switch (test)
		{
			case RZ_ELieTest.VISIT: return "visit";
			case RZ_ELieTest.ROAD: return "road news";
			case RZ_ELieTest.TICK: return "tick";
		}

		return "rumour";
	}

	//------------------------------------------------------------------------------------------------
	void Save(notnull array<ref RZ_Lie> result)
	{
		foreach (RZ_Lie l : m_aLies)
		{
			result.Insert(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Restore(array<ref RZ_Lie> saved)
	{
		m_aLies.Clear();
		if (!saved)
			return;

		foreach (RZ_Lie l : saved)
		{
			if (l && !l.ident.IsEmpty())
				m_aLies.Insert(l);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! The one witness rule (docs/DESIGN.md, "Lies and witnesses"): "if seen" always means this. A
//! witness is the act faction's people near the act: its holdings and road posts within 60 m (their
//! guards and named people), a safehouse within 60 m (the Haven), a wanderer within 40 m (the Haven),
//! or the giver (always, the caller's). A player hidden by stealth isn't seen. Where no bodies are
//! (the act is data), a witness is rolled: 25% within 300 m of a lived-in place, else 5%. Witnesses
//! name the uniform, never the player, in gossip; the Infamy is the player's alone.
class RZ_Witness
{
	//------------------------------------------------------------------------------------------------
	static bool Seen(RZ_GameModeComponent gm, int playerId, vector pos, int faction)
	{
		if (playerId > 0 && RZ_Stealth.IsHidden(playerId))
			return false;

		RZ_Territory territory = gm.GetTerritory();
		if (territory && territory.IsActive() && RZ_Factions.IsValid(faction) && territory.FindHolding(faction, pos, 60))
			return true;

		RZ_Checkpoints posts = gm.GetCheckpoints();
		if (posts && RZ_Factions.IsValid(faction))
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(faction, pos, 60, near);
			if (!near.IsEmpty())
				return true;
		}

		if (faction == RZ_EFaction.SURVIVORS)
		{
			RZ_Safehouses houses = gm.GetSafehouses();
			if (houses && houses.FindNearest(pos, 60) >= 0)
				return true;

			RZ_Wanderers wanderers = gm.GetWanderers();
			if (wanderers && wanderers.AnyNear(pos, 40))
				return true;
		}

		// Data only: a passer-by, maybe
		RZ_WorldFacts facts = gm.GetFacts();
		int town = gm.FindNearestSettlement(pos);
		RZ_Settlement s = gm.GetSettlement(town);
		float chance = 0.05;
		if (s && facts && facts.IsLivedIn(town) && RZ_Util.DistXZ(s.m_vCenter, pos) <= s.m_fRadius + 300)
			chance = 0.25;

		return Math.RandomFloat01() < chance;
	}
}
