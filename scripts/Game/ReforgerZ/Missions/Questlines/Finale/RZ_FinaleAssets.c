//! Route assets (docs/FINALE.md, "Route assets"): what a story job's outcome leans toward one of the
//! five routes. At most one per outcome, at most 3 per player per route a season, kept by identity.
//! An asset does nothing until its owner locks to that route; then it's a card played once in that
//! route's scenes:
//!   ALLY      someone the route can call on: one rank or persuade check one step easier (a vouch)
//!   PROOF     one talk check (a persuade) one step easier
//!   STANDING  one price a third lower, or one leader's check one step easier
//! Assets for other routes go dormant at the lock and lapse at the cure. No asset closes a route.
//! The missions' side: RZ_StoryKit.Record hands every story record here (FromRecord), and the table
//! below says which outcomes lean where. Saved with the finale.

//------------------------------------------------------------------------------------------------
class RZ_FinaleAssetsSave
{
	ref array<string> ident = {};
	ref array<int> route = {};
	ref array<int> kind = {};
	ref array<string> key = {};
	ref array<bool> used = {};
}

//------------------------------------------------------------------------------------------------
class RZ_FinaleAssets
{
	static const int ALLY = 1;
	static const int PROOF = 2;
	static const int STANDING = 3;

	static const int MOST_PER_ROUTE = 3;

	protected RZ_GameModeComponent m_GM;
	protected ref RZ_FinaleAssetsSave m_S = new RZ_FinaleAssetsSave();

	//------------------------------------------------------------------------------------------------
	void RZ_FinaleAssets(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_FinaleAssetsSave data)
	{
		if (data)
			m_S = data;
	}

	//------------------------------------------------------------------------------------------------
	RZ_FinaleAssetsSave Save()
	{
		return m_S;
	}

	//------------------------------------------------------------------------------------------------
	static string KindName(int kind)
	{
		switch (kind)
		{
			case ALLY: return "an ally";
			case PROOF: return "proof";
			case STANDING: return "standing";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A new asset for this identity: `key` is the outcome (one asset each), at most 3 a route.
	//! False if it was refused (the cure is out, the outcome already gave one, or the route is full).
	bool Add(int r, int kind, string ident, string key)
	{
		if (ident.IsEmpty() || r < 0 || r >= RZ_CureFinale.ROUTES || kind < ALLY || kind > STANDING)
			return false;

		if (m_GM.GetCure() && m_GM.GetCure().IsCured())
			return false;

		int onRoute = 0;
		foreach (int i, string who : m_S.ident)
		{
			if (who != ident)
				continue;

			if (m_S.key[i] == key)
				return false;

			if (m_S.route[i] == r)
				onRoute++;
		}

		if (onRoute >= MOST_PER_ROUTE)
			return false;

		m_S.ident.Insert(ident);
		m_S.route.Insert(r);
		m_S.kind.Insert(kind);
		m_S.key.Insert(key);
		m_S.used.Insert(false);
		RZ_Log.Info(string.Format("Finale: asset %1 for %2 to %3 (%4).", KindName(kind), RZ_CureFinale.RouteName(r), ident, key));
		int pid = m_GM.GetQuestlines().OnlineId(ident);
		if (pid > 0)
			m_GM.NotifyPlayer(pid, "What you'd bring", string.Format("That counts for something with %1, if you ever stand with them: %2.", RZ_CureFinale.RouteName(r), KindName(kind)));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "an ally, proof" for this identity's unplayed assets for route r ("" none).
	string Text(string ident, int r)
	{
		string s = "";
		foreach (int i, string who : m_S.ident)
		{
			if (who != ident || m_S.route[i] != r || m_S.used[i])
				continue;

			if (!s.IsEmpty())
				s += ", ";

			s += KindName(m_S.kind[i]);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The first unplayed asset of this identity for route r among `kinds` (ALLY, PROOF, STANDING in
	//! the order given), -1 none. Only for an identity locked to r: the others are dormant.
	int Find(string ident, int r, array<int> kinds)
	{
		RZ_CureFinale f = RZ_CureFinale.Get();
		if (!f || f.RouteOf(ident) != r || (m_GM.GetCure() && m_GM.GetCure().IsCured()))
			return -1;

		foreach (int kind : kinds)
		{
			foreach (int i, string who : m_S.ident)
			{
				if (who == ident && m_S.route[i] == r && m_S.kind[i] == kind && !m_S.used[i])
					return i;
			}
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	int KindAt(int idx)
	{
		if (idx < 0 || idx >= m_S.kind.Count())
			return 0;

		return m_S.kind[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! The card is played.
	void Play(int idx)
	{
		if (idx < 0 || idx >= m_S.used.Count())
			return;

		m_S.used[idx] = true;
		RZ_Log.Info(string.Format("Finale: asset %1 played for %2 by %3.", KindName(m_S.kind[idx]), RZ_CureFinale.RouteName(m_S.route[idx]), m_S.ident[idx]));
	}

	// ================================================================ Where they come from
	//------------------------------------------------------------------------------------------------
	//! A story job's world record (RZ_StoryKit.Record): the outcomes that lean toward a route give
	//! each of its deciders one asset.
	static void FromRecord(string key, string subject, string value, array<int> players)
	{
		RZ_CureFinale f = RZ_CureFinale.Get();
		if (!f || !players || !RZ_CureFinale.IsOn())
			return;

		int r = RZ_CureFinale.NONE;
		int kind = 0;
		if (key == "crash.box")
		{
			if (value.StartsWith("stove"))
			{
				r = RZ_CureFinale.BASTION;		// the black box burnt in the stove
				kind = STANDING;
			}
			else if (value == "redhand:inventory")
			{
				r = RZ_CureFinale.INDEP;		// the manifest read on the open channel
				kind = PROOF;
			}
			else if (value.StartsWith("redhand"))
			{
				r = RZ_CureFinale.REDHAND;		// the fence holding it
				kind = PROOF;
			}
			else if (value.StartsWith("bastion"))
			{
				r = RZ_CureFinale.BASTION;
				kind = PROOF;
			}
		}
		else if (key == "trapped.family")
		{
			r = RZ_CureFinale.HAVEN;
			if (value == "bastion")
				r = RZ_CureFinale.BASTION;

			kind = ALLY;
		}
		else if (key == "struck.fate")
		{
			if (value == "haven")
			{
				r = RZ_CureFinale.HAVEN;		// a deserter the Haven hid
				kind = ALLY;
			}
			else if (value == "roster")
			{
				r = RZ_CureFinale.BASTION;
				kind = STANDING;
			}
			else if (value == "gone")
			{
				r = RZ_CureFinale.INDEP;		// let go
				kind = ALLY;
			}
		}
		else if (key == "colleague.fate" && value == "left")
		{
			r = RZ_CureFinale.INDEP;			// a scientist who walked out
			kind = ALLY;
		}
		else if (key == "distress.saved")
		{
			r = RZ_CureFinale.INDEP;			// given to people, not to a faction
			kind = ALLY;
		}
		else if (key.StartsWith("racket."))
		{
			if (value == "receipt")
			{
				r = RZ_CureFinale.REDHAND;
				kind = STANDING;
			}
			else if (value == "covered")
			{
				r = RZ_CureFinale.HAVEN;
				kind = STANDING;
			}
			else if (value == "secret")
			{
				r = RZ_CureFinale.HAVEN;
				kind = PROOF;
			}
		}

		if (r == RZ_CureFinale.NONE || kind == 0)
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		foreach (int pid : players)
		{
			if (pid > 0)
				f.GetAssets().Add(r, kind, gm.GetProfile(pid).m_sIdentity, key + "|" + subject);
		}
	}
}
