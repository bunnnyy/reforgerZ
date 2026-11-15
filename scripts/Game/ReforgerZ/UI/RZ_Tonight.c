//! Tonight (docs/UI.md, "Tonight"): up to three things worth doing near you right now, so the
//! first minute after coming round or joining has a direction. One of each kind, within walking
//! reach (UI > Tonight reach), each read from a live system and true when it's picked:
//! - Work: the nearest named person with work for you (an errand or a duty, as the talk screen's
//!   own look ahead finds it: RZ_Errands.PeekFor), at a safehouse or a holding that will have you;
//! - Something happening: a supply truck on its way to a place near you, a road patrol on a road
//!   near you, a world event you could join, a pack you know of that's restless;
//! - Something to find: a town near you you haven't seen (the map's fog), while landmarks and
//!   heirlooms don't exist yet;
//! - Yours: a job you hold that's near, in Work's place when nobody near has work.
//! Nothing is made up and nothing is spawned. The lines go on the spawn card (coming round, back
//! from a logout, a new character with no opening running) and on the Quests tab's Tonight rows;
//! a Tonight row tracks as a job does (UI/RZ_Quests.c): its place, the person or the thing.
//! Server only; runtime state (picked again at every spawn and join).

//------------------------------------------------------------------------------------------------
enum RZ_ETonightKind
{
	WORK,
	HAPPENING,
	FIND,
	YOURS
}

//------------------------------------------------------------------------------------------------
//! One line of Tonight, and what it points at.
class RZ_TonightLine
{
	int m_iSerial;			// the row id: -(RZ_UIFeed.TONIGHT_ID_BASE + serial)
	int m_iKind;			// RZ_ETonightKind
	string m_sLine;			// the card's line and the row's brief
	string m_sTitle;		// the row's name and the tracker's
	string m_sBy;			// the row's "from"
	string m_sGo;			// the tracker's objective
	vector m_vAt;			// where it is (a person: where they're put out)
	float m_fArrive = 15;	// within this it's reached: the line is done
	float m_fSearch;		// a rough area's radius (a pack you only heard of), 0 for a spot
	int m_iFaction = -1;
	int m_iJob;				// a real job's row id (a held job, a world event): tracking it tracks the job
	string m_sPerson;		// a named person's key: the tracker follows their body
	int m_iPatrol = -1;		// a road patrol's id: the tracker follows it
	int m_iTruck = -1;		// a faction whose truck it is
	int m_iTribe = -1;
	int m_iSettlement = -1;
	string m_sSource;		// what it was picked from (the log line)
	bool m_bReached;

	//------------------------------------------------------------------------------------------------
	int RowId()
	{
		if (m_iJob != 0)
			return m_iJob;

		return -(RZ_UIFeed.TONIGHT_ID_BASE + m_iSerial);
	}

	//------------------------------------------------------------------------------------------------
	//! Its own row in the Tonight group (never the job's id: the job has its row too).
	int OwnRow()
	{
		return -(RZ_UIFeed.TONIGHT_ID_BASE + m_iSerial);
	}
}

//------------------------------------------------------------------------------------------------
//! One player's Tonight.
class RZ_TonightState
{
	ref array<ref RZ_TonightLine> m_aLines = {};
	float m_fPickedAt = -1000;
	vector m_vPickedAt;
	float m_fCheckedAt = -1000;
}

//------------------------------------------------------------------------------------------------
class RZ_Tonight
{
	static const int SERIALS = 99999;
	static const float CHECK_SECONDS = 30;		// lines checked at most this often (the journal's tick)
	static const float REPICK_SECONDS = 900;	// picked again after this, keeping what's tracked
	static const float REPICK_MOVE = 1000;		// or once you're this far from where they were picked
	static const int NEAREST_PLACES = 4;		// work: the nearest places asked, at most

	protected RZ_GameModeComponent m_GM;
	protected ref map<int, ref RZ_TonightState> m_mStates = new map<int, ref RZ_TonightState>();
	protected int m_iSerial;
	protected float m_fClock;

	//------------------------------------------------------------------------------------------------
	void RZ_Tonight(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		RZ_UISettings settings = RZ_UISettings.Get();
		return settings && settings.m_bTonight;
	}

	//------------------------------------------------------------------------------------------------
	protected float Reach()
	{
		RZ_UISettings settings = RZ_UISettings.Get();
		if (!settings)
			return 1500;

		return Math.Clamp(settings.m_fTonightReach, 300, 5000);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TonightState State(int playerId)
	{
		RZ_TonightState st;
		if (!m_mStates.Find(playerId, st))
		{
			st = new RZ_TonightState();
			m_mStates.Set(playerId, st);
		}

		return st;
	}

	//------------------------------------------------------------------------------------------------
	//! From the UI feed's tick: the clock the checks are measured by.
	void Tick(float dt)
	{
		m_fClock += dt;
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_mStates.Remove(playerId);
	}

	// ================================================================ The card
	//------------------------------------------------------------------------------------------------
	//! Picked fresh (a spawn, a join): the spawn card's lines, "" for none. Never while an opening
	//! runs (Guidance/RZ_Openings.c): the opening is the first minute's direction then.
	string CardFor(int playerId, IEntity me)
	{
		if (!IsOn() || !me || RZ_Openings.IsRunningFor(playerId))
			return "";

		RZ_TonightState st = State(playerId);
		st.m_aLines.Clear();
		Pick(playerId, me, st);
		if (st.m_aLines.IsEmpty())
			return "";

		string s = "Tonight:";
		foreach (RZ_TonightLine l : st.m_aLines)
		{
			s += "\n" + l.m_sLine;
		}

		return s;
	}

	// ================================================================ The rows and the tracker
	//------------------------------------------------------------------------------------------------
	//! The lines as they stand now: each checked (at most every 30 s), the false ones dropped and
	//! picked again; all picked again after 15 minutes or a kilometre, keeping the tracked one.
	void LinesFor(int playerId, IEntity me, notnull array<RZ_TonightLine> result)
	{
		if (!IsOn() || !me || RZ_Openings.IsRunningFor(playerId))
			return;

		RZ_TonightState st = State(playerId);
		if (m_fClock - st.m_fCheckedAt >= CHECK_SECONDS)
		{
			st.m_fCheckedAt = m_fClock;
			Refresh(playerId, me, st);
		}

		foreach (RZ_TonightLine l : st.m_aLines)
		{
			result.Insert(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Refresh(int playerId, IEntity me, RZ_TonightState st)
	{
		int tracked = m_GM.GetProfile(playerId).m_iTrackedId;
		bool stale = m_fClock - st.m_fPickedAt >= REPICK_SECONDS || RZ_Util.DistXZ(st.m_vPickedAt, me.GetOrigin()) >= REPICK_MOVE;
		int dropped = 0;
		for (int i = st.m_aLines.Count() - 1; i >= 0; i--)
		{
			RZ_TonightLine l = st.m_aLines[i];
			bool keep = !l.m_bReached && Still(playerId, l);
			if (keep && stale && l.RowId() != tracked)
				keep = false;

			if (!keep)
			{
				st.m_aLines.Remove(i);
				dropped++;
			}
		}

		if (dropped > 0 || st.m_aLines.IsEmpty())
			Pick(playerId, me, st);
	}

	//------------------------------------------------------------------------------------------------
	//! Tonight's lines as jobs the tracker can follow (the ones that aren't a real job already).
	void Jobs(int playerId, vector from, notnull array<ref RZ_QuestJob> result)
	{
		RZ_TonightState st;
		if (!m_mStates.Find(playerId, st))
			return;

		foreach (RZ_TonightLine l : st.m_aLines)
		{
			if (l.m_iJob != 0 || l.m_bReached)
				continue;

			RZ_QuestJob j = new RZ_QuestJob();
			j.m_iId = l.OwnRow();
			j.m_iKind = RZ_EMissionKindUI.TONIGHT;
			j.m_iGiver = l.m_iFaction;
			j.m_sTitle = l.m_sTitle;
			j.m_bTaken = false;
			j.m_bHasPlace = true;
			j.PointAt(Place(l));
			result.Insert(j);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The line behind a row id (its own row, or the job it points at), null if none.
	RZ_TonightLine Find(int playerId, int id)
	{
		RZ_TonightState st;
		if (id == 0 || !m_mStates.Find(playerId, st))
			return null;

		foreach (RZ_TonightLine l : st.m_aLines)
		{
			if (l.OwnRow() == id)
				return l;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A Tonight row picked to track: the job it points at, else its own row.
	int TrackId(int playerId, int id)
	{
		RZ_TonightLine l = Find(playerId, id);
		if (!l)
			return id;

		return l.RowId();
	}

	//------------------------------------------------------------------------------------------------
	//! Its one objective: go to the person, the thing or the place, where it is now.
	void Objectives(int playerId, int id, notnull RZ_ObjList list)
	{
		RZ_TonightLine l = Find(playerId, id);
		if (!l || l.m_bReached)
			return;

		list.Add(l.m_sGo, RZ_EObjState.CURRENT, Place(l));
	}

	//------------------------------------------------------------------------------------------------
	//! Where a line points now: a person's body, a patrol's lead, the thing's spot or a rough area.
	RZ_ObjPlace Place(RZ_TonightLine l)
	{
		if (!l.m_sPerson.IsEmpty())
			return RZ_ObjPlace.Person(m_GM, l.m_sPerson, l.m_vAt, l.m_fArrive);

		if (l.m_iPatrol >= 0)
		{
			RZ_RoadPatrols patrols = RZ_RoadPatrols.Get();
			RZ_RoadPatrol p;
			if (patrols)
				p = patrols.GetPatrol(l.m_iPatrol);

			if (p)
			{
				IEntity lead = patrols.Leader(p);
				if (lead)
					return RZ_ObjPlace.Spot(lead.GetOrigin(), l.m_fArrive);

				return RZ_ObjPlace.Spot(p.m_vPos, l.m_fArrive);
			}
		}

		if (l.m_fSearch > 0)
			return RZ_ObjPlace.Area(l.m_vAt, l.m_fSearch);

		return RZ_ObjPlace.Spot(l.m_vAt, l.m_fArrive);
	}

	//------------------------------------------------------------------------------------------------
	//! From the tracker: a tracked Tonight line whose place you've reached is done; whoever or
	//! whatever is there carries on from here. True when it was.
	bool CheckReached(int playerId, int id, IEntity me)
	{
		RZ_TonightLine l = Find(playerId, id);
		if (!l || l.m_bReached || !me)
			return false;

		RZ_ObjPlace at = Place(l);
		if (!at)
			return false;

		float within = at.m_fArrive;
		if (at.IsRough())
			within = at.m_fSearch;

		if (RZ_Util.DistXZ(at.m_vPos, me.GetOrigin()) > within)
			return false;

		l.m_bReached = true;
		RZ_Log.Info(string.Format("Tonight: %1 reached %2 (%3)", RZ_Util.PlayerName(playerId), l.m_sTitle, l.m_sSource));
		return true;
	}

	// ================================================================ Picking
	//------------------------------------------------------------------------------------------------
	//! The kinds missing from `st` filled, one line each, nearest first. Work's place goes to a job
	//! of yours near when nobody near has work.
	protected void Pick(int playerId, IEntity me, RZ_TonightState st)
	{
		vector from = me.GetOrigin();
		st.m_fPickedAt = m_fClock;
		st.m_vPickedAt = from;
		float reach = Reach();
		bool haveWork = Has(st, RZ_ETonightKind.WORK) || Has(st, RZ_ETonightKind.YOURS);
		if (!haveWork)
		{
			RZ_TonightLine work = PickWork(playerId, me, from, reach);
			if (!work)
				work = PickYours(playerId, from, reach);

			Add(st, work);
		}

		if (!Has(st, RZ_ETonightKind.HAPPENING))
			Add(st, PickHappening(playerId, from, reach));

		if (!Has(st, RZ_ETonightKind.FIND))
			Add(st, PickFind(playerId, from, reach));

		// Work first, then what's happening, then what's to find
		for (int a = 0; a < st.m_aLines.Count(); a++)
		{
			for (int b = a + 1; b < st.m_aLines.Count(); b++)
			{
				if (Order(st.m_aLines[b].m_iKind) < Order(st.m_aLines[a].m_iKind))
					st.m_aLines.SwapItems(a, b);
			}
		}

		string kinds = "";
		foreach (RZ_TonightLine l : st.m_aLines)
		{
			kinds += string.Format(" [%1: %2]", KindWord(l.m_iKind), l.m_sSource);
		}

		if (kinds.IsEmpty())
			kinds = " nothing true near";

		RZ_Log.Info(string.Format("Tonight: %1, %2 lines within %3 m:%4", RZ_Util.PlayerName(playerId), st.m_aLines.Count(), RZ_Util.RoundToInt(reach), kinds));
	}

	//------------------------------------------------------------------------------------------------
	protected static int Order(int kind)
	{
		if (kind == RZ_ETonightKind.YOURS)
			return RZ_ETonightKind.WORK;

		return kind;
	}

	//------------------------------------------------------------------------------------------------
	static string KindWord(int kind)
	{
		switch (kind)
		{
			case RZ_ETonightKind.WORK: return "work";
			case RZ_ETonightKind.HAPPENING: return "happening";
			case RZ_ETonightKind.FIND: return "find";
			case RZ_ETonightKind.YOURS: return "yours";
		}

		return "?";
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Has(RZ_TonightState st, int kind)
	{
		foreach (RZ_TonightLine l : st.m_aLines)
		{
			if (l.m_iKind == kind)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void Add(RZ_TonightState st, RZ_TonightLine l)
	{
		if (!l)
			return;

		m_iSerial = (m_iSerial + 1) % SERIALS;
		l.m_iSerial = m_iSerial;
		st.m_aLines.Insert(l);
	}

	//------------------------------------------------------------------------------------------------
	//! Work: the nearest places that will have you (a safehouse that hasn't exiled you, a holding
	//! of a faction that doesn't hunt you), each person there asked as the talk screen asks.
	protected RZ_TonightLine PickWork(int playerId, IEntity me, vector from, float reach)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands || !errands.IsOn() || !errands.GetPeople())
			return null;

		// The places within reach, nearest first: kind 0 a house, 1 a holding
		array<int> kinds = {};
		array<int> ids = {};
		array<float> dists = {};
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && !houses.IsExiled(playerId))
		{
			for (int h = 0; h < houses.Count(); h++)
			{
				RZ_Safehouse house = houses.Get(h);
				if (!house || house.m_iPopulation <= 0)
					continue;

				float dh = RZ_Util.DistXZ(house.m_vPos, from);
				if (dh <= reach)
					InsertByDist(kinds, ids, dists, 0, h, dh);
			}
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding hold : territory.GetHoldings())
			{
				if (!hold || RZ_Reputation.IsHunted(m_GM.GetTier(playerId, hold.m_eFaction)))
					continue;

				float dk = RZ_Util.DistXZ(hold.m_vPos, from);
				if (dk <= reach)
					InsertByDist(kinds, ids, dists, 1, hold.m_iId, dk);
			}
		}

		RZ_NamedPeople people = errands.GetPeople();
		for (int i = 0; i < kinds.Count() && i < NEAREST_PLACES; i++)
		{
			for (int idx = 0; idx < 8; idx++)
			{
				RZ_NamedPerson p;
				if (kinds[i] == 0)
					p = people.FromHouse(ids[i], idx);
				else
					p = people.FromHolding(ids[i], idx);

				if (!p)
					continue;

				RZ_Errand e = errands.PeekFor(playerId, p, me);
				if (!e)
					continue;

				RZ_TonightLine l = new RZ_TonightLine();
				l.m_iKind = RZ_ETonightKind.WORK;
				l.m_sLine = string.Format("%1, the %2 at %3, has work for you.", p.m_sName, p.m_sRole, p.m_sPlace);
				l.m_sTitle = string.Format("Work at %1", p.m_sPlace);
				l.m_sBy = p.m_sName;
				l.m_sGo = string.Format("Ask %1, the %2, what needs doing.", p.m_sName, p.m_sRole);
				l.m_vAt = p.m_vPos;
				l.m_fArrive = 4;
				l.m_iFaction = p.m_iFaction;
				l.m_sPerson = p.m_sKey;
				l.m_sSource = string.Format("%1 at %2 (%3)", p.m_sName, p.m_sPlace, e.m_Def.m_sKey);
				return l;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void InsertByDist(array<int> kinds, array<int> ids, array<float> dists, int kind, int id, float d)
	{
		int at = 0;
		while (at < dists.Count() && dists[at] <= d)
		{
			at++;
		}

		kinds.InsertAt(kind, at);
		ids.InsertAt(id, at);
		dists.InsertAt(d, at);
	}

	//------------------------------------------------------------------------------------------------
	//! Yours: the nearest job you hold with a place within reach (a contract you took, an errand).
	protected RZ_TonightLine PickYours(int playerId, vector from, float reach)
	{
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (!feed)
			return null;

		array<ref RZ_QuestJob> jobs = {};
		feed.GetTracker().Jobs(playerId, GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId), jobs);
		RZ_QuestJob best;
		float bestDist = reach;
		foreach (RZ_QuestJob j : jobs)
		{
			if (!j.m_bTaken || !j.m_bHasPlace || j.m_vPlace == vector.Zero || j.m_iKind == RZ_EMissionKindUI.TONIGHT)
				continue;

			float d = RZ_Util.DistXZ(j.m_vPlace, from);
			if (d <= bestDist)
			{
				best = j;
				bestDist = d;
			}
		}

		if (!best)
			return null;

		RZ_TonightLine l = new RZ_TonightLine();
		l.m_iKind = RZ_ETonightKind.YOURS;
		l.m_sLine = string.Format("%1 is still yours: %2.", best.m_sTitle, RZ_QuestText.RoughArea(m_GM, from, best.m_vPlace));
		l.m_sTitle = best.m_sTitle;
		l.m_sBy = "Yours";
		l.m_sGo = best.m_sTitle;
		l.m_vAt = best.m_vPlace;
		l.m_iFaction = best.m_iGiver;
		l.m_iJob = best.m_iId;
		l.m_sSource = string.Format("held job %1", best.m_iId);
		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! Something happening, the nearest of: a truck on its way to a place within reach, a road
	//! patrol on a road within reach, a world event you could join, a pack you know of that's restless.
	protected RZ_TonightLine PickHappening(int playerId, vector from, float reach)
	{
		RZ_TonightLine best;
		float bestDist = reach;

		// A supply truck on the road to somewhere near
		RZ_SupplyLines supply = m_GM.GetSupply();
		for (int f = 0; supply && f < RZ_Factions.COUNT; f++)
		{
			RZ_SupplyTruck t = supply.GetActiveTruck(f);
			if (!t || t.m_bDone || t.m_bFailed || !t.m_bBoarded || t.m_vDest == vector.Zero)
				continue;

			float dt = RZ_Util.DistXZ(t.m_vDest, from);
			if (dt > bestDist)
				continue;

			RZ_TonightLine lt = new RZ_TonightLine();
			lt.m_iKind = RZ_ETonightKind.HAPPENING;
			lt.m_sLine = string.Format("A %1 truck is on its way to %2.", RZ_Factions.GetShortName(f), t.m_sDestName);
			lt.m_sTitle = string.Format("The truck to %1", t.m_sDestName);
			lt.m_sBy = RZ_Factions.GetName(f);
			lt.m_sGo = string.Format("Meet the truck at %1.", t.m_sDestName);
			lt.m_vAt = t.m_vDest;
			lt.m_fArrive = 30;
			lt.m_iFaction = f;
			lt.m_iTruck = f;
			lt.m_sSource = "truck to " + t.m_sDestName;
			best = lt;
			bestDist = dt;
		}

		// A road patrol on a road near you
		RZ_RoadPatrols patrols = RZ_RoadPatrols.Get();
		if (patrols && patrols.IsOn())
		{
			array<RZ_RoadPatrol> all = {};
			patrols.GetAll(all);
			foreach (RZ_RoadPatrol p : all)
			{
				float dp = RZ_Util.DistXZ(p.m_vPos, from);
				if (dp > bestDist || RZ_Reputation.IsHunted(m_GM.GetTier(playerId, p.m_eFaction)))
					continue;

				string dir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(from, p.m_vPos));
				RZ_TonightLine lp = new RZ_TonightLine();
				lp.m_iKind = RZ_ETonightKind.HAPPENING;
				lp.m_sLine = string.Format("A %1 from %2 is on the road %3 of you.", PatrolName(p), p.m_sFrom, dir);
				lp.m_sTitle = RZ_Util.Capitalize(PatrolName(p)) + " on the road";
				lp.m_sBy = p.m_sLeader;
				lp.m_sGo = string.Format("Catch up with %1.", p.m_sLeader);
				lp.m_vAt = p.m_vPos;
				lp.m_fArrive = 25;
				lp.m_iFaction = p.m_eFaction;
				lp.m_iPatrol = p.m_iId;
				lp.m_sSource = "patrol " + p.m_iId.ToString();
				best = lp;
				bestDist = dp;
			}
		}

		// A world event you could join
		RZ_MissionManager mm = m_GM.GetMissions();
		if (mm)
		{
			array<RZ_Mission> list = {};
			mm.ListFor(playerId, list);
			foreach (RZ_Mission m : list)
			{
				if (m.m_bGlobal || m.m_bContract || RZ_UIFeed.MissionKind(m) != RZ_EMissionKindUI.EVENT || m.m_aParticipants.Contains(playerId))
					continue;

				vector at = m.GetAreaFor(m_GM, playerId, from);
				float dm = RZ_Util.DistXZ(at, from);
				if (at == vector.Zero || dm > bestDist)
					continue;

				RZ_TonightLine lm = new RZ_TonightLine();
				lm.m_iKind = RZ_ETonightKind.HAPPENING;
				lm.m_sLine = string.Format("%1: %2.", m.m_sTitle, RZ_QuestText.RoughArea(m_GM, from, at));
				lm.m_sTitle = m.m_sTitle;
				lm.m_sBy = "World event";
				lm.m_sGo = m.m_sTitle;
				lm.m_vAt = at;
				lm.m_iJob = m.m_iId;
				lm.m_sSource = "event " + m.m_iId.ToString();
				best = lm;
				bestDist = dm;
			}
		}

		// A pack you know of, restless (only what you know: its rough area, never its lair)
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes && tribes.GetPeople() && tribes.GetPeople().IsOn())
		{
			array<int> tids = {};
			array<string> titles = {};
			array<string> bys = {};
			array<vector> wheres = {};
			array<string> briefs = {};
			tribes.GetPeople().JournalFor(playerId, from, tids, titles, bys, wheres, briefs);
			foreach (int ti, int tid : tids)
			{
				RZ_Tribe tr = tribes.Find(tid);
				if (!tr || !tr.m_bRestless || wheres[ti] == vector.Zero)
					continue;

				float dr = RZ_Util.DistXZ(wheres[ti], from);
				if (dr > bestDist)
					continue;

				RZ_TonightLine lr = new RZ_TonightLine();
				lr.m_iKind = RZ_ETonightKind.HAPPENING;
				lr.m_sLine = string.Format("The pack in %1 is restless.", tr.m_sPlace);
				lr.m_sTitle = "A restless pack";
				lr.m_sBy = titles[ti];
				lr.m_sGo = string.Format("Keep an eye on %1.", tr.m_sPlace);
				lr.m_vAt = wheres[ti];
				lr.m_fSearch = 400;
				lr.m_iTribe = tid;
				lr.m_sSource = "pack " + tid.ToString();
				best = lr;
				bestDist = dr;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected static string PatrolName(RZ_RoadPatrol p)
	{
		if (p.m_bVehicle)
			return RZ_RoadPatrols.KindName(p.m_eFaction, true);

		switch (p.m_eFaction)
		{
			case RZ_EFaction.BANDITS: return "Red Hand crew";
			case RZ_EFaction.SURVIVORS: return "Haven scavenging party";
			case RZ_EFaction.INSTITUTE: return "Halcyon sample team";
		}

		return "Bastion patrol";
	}

	//------------------------------------------------------------------------------------------------
	//! Something to find: the nearest town within reach you've never seen (the map's fog).
	protected RZ_TonightLine PickFind(int playerId, vector from, float reach)
	{
		RZ_MapFog fog = m_GM.GetMapFog();
		if (!fog || !fog.IsOn())
			return null;

		RZ_Settlement best;
		int bestIdx = -1;
		float bestDist = reach;
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (!s || s.m_bIsland || fog.Seen(playerId, s.m_vCenter))
				continue;

			float d = RZ_Util.DistXZ(s.m_vCenter, from);
			if (d <= s.m_fRadius || d > bestDist)
				continue;

			best = s;
			bestIdx = i;
			bestDist = d;
		}

		if (!best)
			return null;

		RZ_TonightLine l = new RZ_TonightLine();
		l.m_iKind = RZ_ETonightKind.FIND;
		l.m_sLine = string.Format("You've never been to %1. It's %2.", best.m_sName, RZ_Util.DirectionText(from, best.m_vCenter));
		l.m_sTitle = "See " + best.m_sName;
		l.m_sBy = "Not on your map";
		l.m_sGo = string.Format("Go and see %1.", best.m_sName);
		l.m_vAt = best.m_vCenter;
		l.m_fArrive = Math.Clamp(best.m_fRadius * 0.5, 30, 150);
		l.m_iSettlement = bestIdx;
		l.m_sSource = "unseen " + best.m_sName;
		return l;
	}

	// ================================================================ Still true?
	//------------------------------------------------------------------------------------------------
	protected bool Still(int playerId, RZ_TonightLine l)
	{
		if (l.m_iKind == RZ_ETonightKind.WORK)
		{
			RZ_Errands errands = m_GM.GetErrands();
			if (!errands || !RZ_Factions.IsValid(l.m_iFaction))
				return errands != null;

			// Asked already (now it's yours), or hunted since
			return errands.HeldIdFor(playerId, l.m_sPerson) < 0 && !RZ_Reputation.IsHunted(m_GM.GetTier(playerId, l.m_iFaction));
		}

		if (l.m_iJob != 0)
		{
			RZ_UIFeed feed = m_GM.GetUIFeed();
			if (!feed)
				return false;

			array<ref RZ_QuestJob> jobs = {};
			feed.GetTracker().Jobs(playerId, GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId), jobs);
			foreach (RZ_QuestJob j : jobs)
			{
				if (j.m_iId == l.m_iJob)
					return l.m_iKind == RZ_ETonightKind.YOURS || !j.m_bTaken;
			}

			return false;
		}

		if (l.m_iTruck >= 0)
		{
			RZ_SupplyLines supply = m_GM.GetSupply();
			RZ_SupplyTruck t;
			if (supply)
				t = supply.GetActiveTruck(l.m_iTruck);

			return t != null && !t.m_bDone && !t.m_bFailed && t.m_vDest == l.m_vAt;
		}

		if (l.m_iPatrol >= 0)
		{
			RZ_RoadPatrols patrols = RZ_RoadPatrols.Get();
			return patrols && patrols.GetPatrol(l.m_iPatrol) != null;
		}

		if (l.m_iTribe >= 0)
		{
			RZ_Tribes tribes = m_GM.GetTribes();
			RZ_Tribe tr;
			if (tribes)
				tr = tribes.Find(l.m_iTribe);

			return tr != null && tr.m_bRestless;
		}

		if (l.m_iKind == RZ_ETonightKind.FIND)
		{
			RZ_MapFog fog = m_GM.GetMapFog();
			return fog && fog.IsOn() && !fog.Seen(playerId, l.m_vAt);
		}

		return true;
	}

	// ================================================================ The journal
	//------------------------------------------------------------------------------------------------
	//! The Quests tab's Tonight rows (M records of kind TONIGHT), in the journal's format.
	string Rows(int playerId, IEntity me, vector from)
	{
		array<RZ_TonightLine> lines = {};
		LinesFor(playerId, me, lines);
		string s = "";
		foreach (RZ_TonightLine l : lines)
		{
			RZ_ObjPlace at = Place(l);
			string dir = "";
			string where = "";
			if (at)
			{
				dir = RZ_QuestText.RoughArea(m_GM, from, at.m_vPos);
				where = at.Words(m_GM);
			}

			RZ_UILine ln = new RZ_UILine("M");
			ln.AddInt(l.OwnRow()).AddInt(RZ_EMissionKindUI.TONIGHT).AddInt(l.m_iFaction).Add(l.m_sTitle).Add(l.m_sBy).Add(where).Add(dir);
			ln.AddInt(-1).Add("").AddInt(0).AddInt(0).Add(l.m_sLine).Add("0");
			ln.Add(RZ_QuestText.KindLabel(RZ_EMissionKindUI.TONIGHT, false)).Add(l.m_sGo);
			s += ln.End();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! A Tonight row's detail: the line, and what picking it does.
	string Detail(int playerId, int id)
	{
		RZ_TonightLine l = Find(playerId, id);
		if (!l)
			return "";

		return l.m_sLine + "\n\nIt's true now, that's all: nobody's promised you anything. Track it and the tracker takes you there.";
	}

	// ================================================================ Test hooks (tools/runtime_test)
	//------------------------------------------------------------------------------------------------
	int TestCount(int playerId)
	{
		RZ_TonightState st;
		if (!m_mStates.Find(playerId, st))
			return 0;

		return st.m_aLines.Count();
	}

	//------------------------------------------------------------------------------------------------
	RZ_TonightLine TestLine(int playerId, int i)
	{
		RZ_TonightState st;
		if (!m_mStates.Find(playerId, st) || i < 0 || i >= st.m_aLines.Count())
			return null;

		return st.m_aLines[i];
	}

	//------------------------------------------------------------------------------------------------
	//! Picked again now, whatever the clock says (the test).
	void TestRepick(int playerId, IEntity me)
	{
		RZ_TonightState st = State(playerId);
		st.m_aLines.Clear();
		Pick(playerId, me, st);
	}
}
