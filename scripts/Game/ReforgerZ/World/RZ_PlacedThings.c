//! Placed things (docs/DESIGN.md, "Things players build"): what players build in the world is
//! cleared when nobody uses it, so nothing they leave behind stays for good. Today that's the
//! fireplace of Make Fire Campfire & Wood (docs/MOD_STACK.md); a tent or shelter a later mod lets
//! players put up is cleared the same way once its prefab's name is in Placed prefabs. No class of
//! another mod is named: a thing is known by a keyword in its prefab's name.
//!
//! - Found, not told: once a minute the server looks around each player for entities whose prefab
//!   name has a keyword. One that isn't ours (nothing under Prefabs/ReforgerZ/, no part of a
//!   composition), doesn't stand at a faction's own place and isn't known yet becomes a record:
//!   where, which prefab, when first seen, when last used. Data only; the entity is the mod's.
//! - Used: a player within Use radius. Each look that finds one there sets its last use to now.
//! - Cleared: unused for Unused days of real time (System.GetUnixTime, so it counts across
//!   restarts and while the server is empty), and no player within Clear distance. The entity is
//!   deleted and the record dropped. Silent: nobody is told.
//! - A record whose thing is gone when a player stands by it (dismantled, or lost at a restart)
//!   is dropped. Past the cap the longest-unused one out of sight is cleared early; if every one
//!   is in sight, the oldest record is forgotten and its thing left standing.
//!
//! Saved (state.json): the records, never the entities. After a restart a record finds its thing
//! again by prefab and place the first time a player comes near.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Placed things.
[BaseContainerProps()]
class RZ_PlacedSettings
{
	[Attribute("1", desc: "Things players build (a fireplace, a tent a mod brings) are cleared when nobody has used them for Unused days")]
	bool m_bEnabled;

	[Attribute("fm_campfire", desc: "Placed prefabs: keywords in the prefab names of things players build, separated by commas. The fireplace of Make Fire Campfire & Wood is fm_campfire; a tent mod's own prefab names go here. Use a name only player-built things have (never a plain 'tent': the map's and the camps' tents carry that too)")]
	string m_sPrefabWords;

	[Attribute("3", desc: "Unused days: real days (they pass while the server is empty or down) with no player within Use radius before a thing is cleared. Fractions work: 0.01 is about a quarter of an hour, for a test")]
	float m_fUnusedDays;

	[Attribute("15", desc: "Use radius (m): a player this close is using it")]
	float m_fUseRadius;

	[Attribute("100", desc: "Clear distance (m): a thing is never cleared with a player this close, however long it stood unused")]
	float m_fClearDistance;

	[Attribute("40", desc: "Most things kept track of. Past it the longest-unused one out of sight is cleared early")]
	int m_iMaxTracked;

	[Attribute("4", desc: "Fire heat: heat a minute a lit fireplace a player built makes where it stands, with a player within 60 m (the light and the smoke draw the infected). 0 = none")]
	float m_fFireHeat;

	[Attribute("2", desc: "Fire heat at night: the fire's heat is multiplied by this between dusk and dawn (it shows further)")]
	float m_fFireNightMult;
}

//------------------------------------------------------------------------------------------------
class RZ_PlacedSave
{
	ref array<float> x = {};
	ref array<float> y = {};
	ref array<float> z = {};
	ref array<string> prefab = {};
	ref array<int> seen = {};
	ref array<int> used = {};
}

//------------------------------------------------------------------------------------------------
//! One thing a player built. Data; m_Entity is known only once a look around a player found it.
class RZ_PlacedThing
{
	vector m_vPos;
	string m_sPrefab;		// lower case, as the look-around reads it
	int m_iSeen;			// real time (s) it was first seen
	int m_iUsed;			// real time (s) a player last stood by it
	IEntity m_Entity;
	bool m_bFound;			// this look found it
}

//------------------------------------------------------------------------------------------------
//! The look-around's collector: entities near a point whose prefab name has a keyword.
class RZ_PlacedQuery
{
	ref array<IEntity> m_aFound = {};
	ref array<string> m_aNames = {};
	protected ref array<string> m_aWords;

	//------------------------------------------------------------------------------------------------
	void RZ_PlacedQuery(array<string> words)
	{
		m_aWords = words;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: QueryEntitiesBySphere with EQueryEntitiesFlags.ALL returns a thing a mod spawned at run time (Make Fire's fireplace, built on the editable E_Fireplace_01 prop), and GetPrefabData().GetPrefabName() gives its own prefab's path, not its parent prefab's
	void Run(vector at, float radius)
	{
		GetGame().GetWorld().QueryEntitiesBySphere(at, radius, OnEntity, null, EQueryEntitiesFlags.ALL);
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (!e || !e.GetPrefabData())
			return true;

		string name = e.GetPrefabData().GetPrefabName();
		if (name.IsEmpty())
			return true;

		name.ToLower();
		if (!RZ_ItemHelper.NameHas(name, m_aWords))
			return true;

		m_aFound.Insert(e);
		m_aNames.Insert(name);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_PlacedThings
{
	protected static const float CHECK_INTERVAL = 60;	// the look-around, the uses and the clearing
	protected static const float LOOK_RADIUS = 60;		// around each player (at least the use radius)
	protected static const float SAME = 1.5;			// a record and a found thing this close (m) are the same
	protected static const float LIVED_MARGIN = 10;		// nothing this close to a faction's own place is a player's
	protected static const string OURS = "prefabs/reforgerz/";

	protected RZ_GameModeComponent m_GM;
	protected RZ_PlacedSettings m_Settings;
	protected ref array<ref RZ_PlacedThing> m_aThings = {};
	protected ref array<string> m_aWords = {};
	protected float m_fTimer = CHECK_INTERVAL;
	protected bool m_bOn;

	//------------------------------------------------------------------------------------------------
	void RZ_PlacedThings(RZ_GameModeComponent gm, RZ_PlacedSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn() { return m_bOn; }
	int Count() { return m_aThings.Count(); }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	RZ_PlacedSave Save()
	{
		RZ_PlacedSave data = new RZ_PlacedSave();
		foreach (RZ_PlacedThing t : m_aThings)
		{
			data.x.Insert(t.m_vPos[0]);
			data.y.Insert(t.m_vPos[1]);
			data.z.Insert(t.m_vPos[2]);
			data.prefab.Insert(t.m_sPrefab);
			data.seen.Insert(t.m_iSeen);
			data.used.Insert(t.m_iUsed);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	//! Older saves have none. The entities are found again when a player comes near.
	void Restore(RZ_PlacedSave saved)
	{
		m_aThings.Clear();
		if (!saved || !saved.x || !saved.y || !saved.z || !saved.prefab || !saved.seen || !saved.used)
			return;

		int n = saved.x.Count();
		n = Math.Min(n, Math.Min(saved.y.Count(), saved.z.Count()));
		n = Math.Min(n, Math.Min(saved.prefab.Count(), Math.Min(saved.seen.Count(), saved.used.Count())));
		for (int i = 0; i < n; i++)
		{
			RZ_PlacedThing t = new RZ_PlacedThing();
			t.m_vPos = Vector(saved.x[i], saved.y[i], saved.z[i]);
			t.m_sPrefab = saved.prefab[i];
			t.m_iSeen = saved.seen[i];
			t.m_iUsed = saved.used[i];
			m_aThings.Insert(t);
		}
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After the save was restored.
	void Setup()
	{
		RZ_ItemHelper.Words(m_Settings.m_sPrefabWords, m_aWords);
		m_bOn = m_Settings.m_bEnabled && !m_aWords.IsEmpty() && m_Settings.m_fUnusedDays > 0;
		if (!m_bOn)
		{
			m_aThings.Clear();
			RZ_Log.Info("Placed things: off (turned off, no prefab keywords, or Unused days 0). Nothing players build is cleared.");
			return;
		}

		RZ_Log.Info(string.Format("Placed things: on. What players build (%1) is cleared after %2 real days unused (a player within %3 m is using it), never with a player within %4 m; at most %5 tracked, %6 known from the save.",
			m_Settings.m_sPrefabWords, m_Settings.m_fUnusedDays, RZ_Util.RoundToInt(UseRadius()), RZ_Util.RoundToInt(ClearDistance()), Cap(), m_aThings.Count()));
	}

	//------------------------------------------------------------------------------------------------
	protected float UseRadius() { return Math.Max(1, m_Settings.m_fUseRadius); }
	protected float ClearDistance() { return Math.Max(UseRadius(), m_Settings.m_fClearDistance); }
	protected float LookRadius() { return Math.Max(LOOK_RADIUS, UseRadius() + 5); }
	protected int Cap() { return Math.Max(1, m_Settings.m_iMaxTracked); }

	//------------------------------------------------------------------------------------------------
	protected int UnusedSeconds()
	{
		return RZ_Util.RoundToInt(Math.Max(0.001, m_Settings.m_fUnusedDays) * 86400);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bOn)
			return;

		m_fTimer -= dt;
		if (m_fTimer > 0)
			return;

		m_fTimer = CHECK_INTERVAL;
		int now = RZ_SeasonClock.Now();
		Look(players, now);
		Burn(players);
		DropGone(players);
		ClearUnused(players, now);
	}

	//------------------------------------------------------------------------------------------------
	//! Around each player: new things become records, known ones get their entity back, and one
	//! with a player beside it was used now.
	protected void Look(array<IEntity> players, int now)
	{
		foreach (RZ_PlacedThing known : m_aThings)
		{
			known.m_bFound = false;
		}

		float use = UseRadius();
		foreach (IEntity player : players)
		{
			if (!player)
				continue;

			RZ_PlacedQuery q = new RZ_PlacedQuery(m_aWords);
			q.Run(player.GetOrigin(), LookRadius());
			foreach (int i, IEntity e : q.m_aFound)
			{
				if (!e)
					continue;

				vector at = e.GetOrigin();
				RZ_PlacedThing t = Find(e, q.m_aNames[i], at);
				if (!t)
				{
					if (!IsPlayers(e, at))
						continue;

					t = Add(e, q.m_aNames[i], at, now);
				}

				if (!t)
					continue;

				t.m_Entity = e;
				t.m_vPos = at;
				t.m_bFound = true;
				if (RZ_Util.NearestDistance(at, players) <= use)
					t.m_iUsed = now;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A lit fire a player built, found this minute within 60 m of a player, makes heat where it stands once a
	//! minute (more at night): the light and the smoke carry. A fire nobody is near makes none (the
	//! infected near nobody are data).
	protected void Burn(array<IEntity> players)
	{
		if (m_Settings.m_fFireHeat <= 0 || !m_GM.GetHeat())
			return;

		float amount = m_Settings.m_fFireHeat;
		if (RZ_GameTime.IsNight())
			amount = amount * Math.Max(1, m_Settings.m_fFireNightMult);

		foreach (RZ_PlacedThing t : m_aThings)
		{
			if (!t.m_bFound || !RZ_Fires.IsLit(t.m_Entity))
				continue;

			m_GM.GetHeat().AddHeat(t.m_vPos, amount);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The record for this entity: by the entity, else the same prefab at the same spot (a record
	//! from the save, whose entity isn't known yet).
	protected RZ_PlacedThing Find(IEntity e, string name, vector at)
	{
		foreach (RZ_PlacedThing t : m_aThings)
		{
			if (t.m_Entity == e)
				return t;
		}

		foreach (RZ_PlacedThing saved : m_aThings)
		{
			if (!saved.m_Entity && saved.m_sPrefab == name && vector.Distance(saved.m_vPos, at) <= SAME)
				return saved;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A player built it: not one of ours (a prefab of ours, or a part of any composition: camp
	//! life's pieces and the scenes' props hang from theirs), and not at a faction's own place.
	protected bool IsPlayers(IEntity e, vector at)
	{
		// VERIFY: a thing a player's action spawned stands alone (no parent entity), while the pieces of a camp's or a scene's composition have the composition as their parent
		if (e.GetParent())
			return false;

		string own = RZ_Util.GetPrefabName(e);
		own.ToLower();
		if (own.Contains(OURS))
			return false;

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (facts && facts.IsNearLivedIn(at, LIVED_MARGIN))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A new record. Past the cap the longest-unused one out of sight is cleared first; if every
	//! one is in sight the oldest record is forgotten (its thing is left standing).
	protected RZ_PlacedThing Add(IEntity e, string name, vector at, int now)
	{
		if (m_aThings.Count() >= Cap())
			MakeRoom(now);

		RZ_PlacedThing t = new RZ_PlacedThing();
		t.m_Entity = e;
		t.m_sPrefab = name;
		t.m_vPos = at;
		t.m_iSeen = now;
		t.m_iUsed = now;
		m_aThings.Insert(t);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	protected void MakeRoom(int now)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		int oldest = -1;
		int oldestFar = -1;
		foreach (int i, RZ_PlacedThing t : m_aThings)
		{
			if (oldest < 0 || t.m_iUsed < m_aThings[oldest].m_iUsed)
				oldest = i;

			if (RZ_Util.NearestDistance(t.m_vPos, players) <= ClearDistance())
				continue;

			if (oldestFar < 0 || t.m_iUsed < m_aThings[oldestFar].m_iUsed)
				oldestFar = i;
		}

		if (oldestFar >= 0)
		{
			Clear(m_aThings[oldestFar], now, "over the cap");
			m_aThings.Remove(oldestFar);
			return;
		}

		if (oldest >= 0)
			m_aThings.Remove(oldest);
	}

	//------------------------------------------------------------------------------------------------
	//! A record whose thing wasn't found with a player standing where it was: dismantled, or lost
	//! at a restart.
	protected void DropGone(array<IEntity> players)
	{
		float near = LookRadius() - 10;
		for (int i = m_aThings.Count() - 1; i >= 0; i--)
		{
			RZ_PlacedThing t = m_aThings[i];
			if (t.m_bFound || t.m_Entity)
				continue;

			if (RZ_Util.NearestDistance(t.m_vPos, players) <= near)
				m_aThings.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Unused long enough and out of sight: gone.
	protected void ClearUnused(array<IEntity> players, int now)
	{
		int limit = UnusedSeconds();
		float far = ClearDistance();
		for (int i = m_aThings.Count() - 1; i >= 0; i--)
		{
			RZ_PlacedThing t = m_aThings[i];
			// A clock set back (the machine's time changed) never clears early
			if (t.m_iUsed > now)
				t.m_iUsed = now;

			if (now - t.m_iUsed < limit)
				continue;

			if (RZ_Util.NearestDistance(t.m_vPos, players) <= far)
				continue;

			Clear(t, now, "unused");
			m_aThings.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes the thing (found again by prefab and place if the record came from the save and
	//! nobody has been near since) and says so in the log. The caller drops the record.
	protected void Clear(RZ_PlacedThing t, int now, string why)
	{
		IEntity e = t.m_Entity;
		if (!e)
			e = FindAt(t);

		float days = (now - t.m_iUsed) / 86400.0;
		string unused = (Math.Round(days * 10) / 10).ToString();
		if (!e)
		{
			RZ_Log.Info(string.Format("Placed things: %1 at %2 was gone already (%3, %4 days); forgotten.", ShortName(t.m_sPrefab), RZ_SettlementScanner.GridReference(t.m_vPos), why, unused));
			return;
		}

		Delete(e);
		RZ_Log.Info(string.Format("Placed things: cleared %1 at %2 (%3, %4 days since anyone was within %5 m).", ShortName(t.m_sPrefab), RZ_SettlementScanner.GridReference(t.m_vPos), why, unused, RZ_Util.RoundToInt(UseRadius())));
	}

	//------------------------------------------------------------------------------------------------
	//! The record's thing where it stood, if it's still there.
	protected IEntity FindAt(RZ_PlacedThing t)
	{
		RZ_PlacedQuery q = new RZ_PlacedQuery(m_aWords);
		q.Run(t.m_vPos, SAME + 0.5);
		foreach (int i, IEntity e : q.m_aFound)
		{
			if (e && q.m_aNames[i] == t.m_sPrefab && !e.GetParent())
				return e;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: SCR_EntityHelper.DeleteEntityAndChildren on the server removes a mod's replicated thing (the fireplace, its fire and light children) on every client
	protected static void Delete(IEntity e)
	{
		SCR_EntityHelper.DeleteEntityAndChildren(e);
	}

	//------------------------------------------------------------------------------------------------
	//! "fm_campfire_01" from the prefab's path.
	protected static string ShortName(string prefab)
	{
		string name = prefab;
		int slash = name.LastIndexOf("/");
		if (slash >= 0)
			name = name.Substring(slash + 1, name.Length() - slash - 1);

		if (name.EndsWith(".et"))
			name = name.Substring(0, name.Length() - 3);

		return name;
	}
}
