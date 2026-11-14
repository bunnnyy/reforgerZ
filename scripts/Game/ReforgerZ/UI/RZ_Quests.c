//! Jobs as the player follows them (docs/UI.md, "The quest tracker" and "Notebook: Quests"): each job's
//! objectives, its area in words (the tracker adds an arrow and metres within a km), and the one job
//! the player tracks.
//!
//! Server side: RZ_QuestTracker (held by RZ_UIFeed) keeps each player's tracked job (saved on the
//! profile), the order they took their jobs in, and sends the HUD's tracker payload when it
//! changes (owner-only, on the UI feed's tick, every TRACK_SECONDS). The client draws it top left
//! (RZ_HudDisplay.DrawTracker) and lists the jobs in the notebook's Quests tab (RZ_QuestsPage).
//!
//! Objectives come from the job itself: a mission's GetObjectives (its own steps, or the
//! framework's default), a questline's GetObjectives (its stages), an errand's way. A step shows
//! only once the player has learned of it: nothing hidden, no placeholders.
//!
//! Each objective carries its own place (RZ_ObjPlace): where that step is done (a delivery's drop
//! once you carry the thing, the next point to visit, the giver again for "go back"). The tracker's
//! area line and arrow, the map's diamond or search area and the Quests tab's row all follow the
//! current objective's place; the job's own place is only the fallback for an objective without one.

//------------------------------------------------------------------------------------------------
//! One objective's state, New Vegas style: the current one bright, the known next one dim, the
//! done ones ticked.
enum RZ_EObjState
{
	CURRENT,
	NEXT,
	DONE,
	OPTIONAL,
	FAILED
}

//------------------------------------------------------------------------------------------------
//! Where one objective is done (docs/UI.md, "The quest tracker"): a spot known to the player, or a
//! search area (its centre off the hidden spot, never the spot itself).
class RZ_ObjPlace
{
	static const float ARRIVE_MAX = 50;	// a step's own radius, at most this: the arrow leads into a big site

	vector m_vPos;			// the spot, or the search area's centre
	float m_fArrive = 25;	// within this of the spot it's reached: the tracker's arrow hides
	float m_fSearch;		// the search area's radius (m); 0 for a known spot

	//------------------------------------------------------------------------------------------------
	//! A known spot, reached within `radius` (the step's own, widened to the Arrive radius, at most 50 m).
	static RZ_ObjPlace Spot(vector pos, float radius = 0)
	{
		if (pos == vector.Zero)
			return null;

		RZ_ObjPlace p = new RZ_ObjPlace();
		p.m_vPos = pos;
		p.m_fArrive = Math.Min(RZ_Arrive.Spot(radius), ARRIVE_MAX);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! A person to go back to or talk to: their body where it stands now when it's out (it moves
	//! with them), else `usual`, where they're put out. Every person objective comes through here
	//! (docs/UI.md, "The tracker"), never a place's centre.
	static RZ_ObjPlace Person(RZ_GameModeComponent gm, string key, vector usual, float radius = 0)
	{
		IEntity body = BodyOf(gm, key);
		if (body)
			return Spot(body.GetOrigin(), radius);

		return Spot(usual, radius);
	}

	//------------------------------------------------------------------------------------------------
	//! The body wearing the named person `key` now, or null (put away, or nobody by that key).
	static IEntity BodyOf(RZ_GameModeComponent gm, string key)
	{
		if (!gm || key.IsEmpty() || !gm.GetErrands() || !gm.GetErrands().GetPeople())
			return null;

		return gm.GetErrands().GetPeople().BodyOf(key);
	}

	//------------------------------------------------------------------------------------------------
	//! A search area: the rough circle's centre and radius. Never the hidden spot.
	static RZ_ObjPlace Area(vector centre, float radius)
	{
		if (centre == vector.Zero)
			return null;

		RZ_ObjPlace p = new RZ_ObjPlace();
		p.m_vPos = centre;
		p.m_fSearch = Math.Max(radius, 0);
		p.m_fArrive = Math.Min(RZ_Arrive.Spot(), ARRIVE_MAX);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	bool IsRough()
	{
		return m_fSearch > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The place in words, alone ("East of Levie"): the objective rows' place.
	string Words(RZ_GameModeComponent gm)
	{
		return RZ_QuestText.RoughArea(gm, vector.Zero, m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! A job's objectives, in order, each with its place if it has one.
class RZ_ObjList
{
	static const int MAX = 8;

	ref array<string> m_aLines = {};
	ref array<int> m_aStates = {};
	ref array<ref RZ_ObjPlace> m_aPlaces = {};

	//------------------------------------------------------------------------------------------------
	//! A line in `state`, done at `at` (null: no place of its own, the job's is used). False if it
	//! wasn't added (empty, a repeat, or the list is full).
	bool Add(string text, int state, RZ_ObjPlace at = null)
	{
		string t = text.Trim();
		if (t.IsEmpty() || m_aLines.Count() >= MAX || m_aLines.Contains(t))
			return false;

		m_aLines.Insert(t);
		m_aStates.Insert(state);
		m_aPlaces.Insert(at);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The place of the first line in `state`, null if there's none or it has no place.
	RZ_ObjPlace PlaceOf(int state)
	{
		foreach (int i, string line : m_aLines)
		{
			if (m_aStates[i] == state)
				return m_aPlaces[i];
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The current objective's place, null if none.
	RZ_ObjPlace Now()
	{
		return PlaceOf(RZ_EObjState.CURRENT);
	}

	//------------------------------------------------------------------------------------------------
	//! The first line in `state`, "" if none.
	string First(int state)
	{
		foreach (int i, string line : m_aLines)
		{
			if (m_aStates[i] == state)
				return line;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return m_aLines.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Copies the current and next lines of another list (a questline's stage mission).
	void TakeOpen(RZ_ObjList other)
	{
		foreach (int i, string line : other.m_aLines)
		{
			int st = other.m_aStates[i];
			if (st == RZ_EObjState.CURRENT || st == RZ_EObjState.NEXT || st == RZ_EObjState.OPTIONAL)
				Add(line, st, other.m_aPlaces[i]);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Text helpers for jobs: rough areas in words, the tracker's metres, short briefs.
class RZ_QuestText
{
	//------------------------------------------------------------------------------------------------
	//! "Morton, a few km north-west", "East of Levie, under a km south", "Close by". Bands: close
	//! by (under 150 m), under a km, a few km (under 4 km), far off. The nearest settlement names
	//! the place; never metres (the tracker's own arrow and metres: docs/UI.md, "The quest tracker").
	//! `from` zero: the place alone ("East of Levie").
	static string RoughArea(RZ_GameModeComponent gm, vector from, vector to)
	{
		if (!gm || to == vector.Zero)
			return "";

		string town = "";
		string side = "";
		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(to));
		if (s)
		{
			town = s.m_sName;
			if (RZ_Util.DistXZ(s.m_vCenter, to) > s.m_fRadius + 150)
				side = RZ_Compass.DirectionWord(RZ_Compass.Bearing(s.m_vCenter, to));
		}

		string place = town;
		if (!side.IsEmpty())
			place = Cap(side) + " of " + town;

		if (from == vector.Zero)
			return place;

		// Under a km: the metres and the way (no "close by": it counts down to 0 m)
		float dist = RZ_Util.DistXZ(from, to);
		string way = RZ_Compass.DirectionWord(RZ_Compass.Bearing(from, to));
		if (dist < 1000)
		{
			string near = Metres(dist);
			if (dist >= 10)
				near = near + " " + way;

			if (place.IsEmpty())
				return near;

			return place + ", " + near;
		}

		string band = "far off";
		if (dist < 4000)
			band = "a few km";

		if (place.IsEmpty())
			return Cap(band + " " + way);

		return string.Format("%1, %2 %3", place, band, way);
	}

	//------------------------------------------------------------------------------------------------
	//! The tracker's distance: "120 m", to 10 m from 100 m up, to 5 m below (at least 5 m).
	static string Metres(float metres)
	{
		int step = 10;
		if (metres < 10)
			step = 1;
		else if (metres < 100)
			step = 5;

		int m = RZ_Util.RoundToInt(metres / step) * step;
		if (m < 0)
			m = 0;

		return string.Format("%1 m", m);
	}

	//------------------------------------------------------------------------------------------------
	//! The spawn card's line without a scenario: "You come to near Morton."
	static string SpawnWhere(RZ_GameModeComponent gm, vector pos)
	{
		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(pos));
		if (!s)
			return "You come to in the open, nobody in sight.";

		if (RZ_Util.DistXZ(s.m_vCenter, pos) <= s.m_fRadius)
			return string.Format("You come to in %1.", s.m_sName);

		string side = RZ_Compass.DirectionWord(RZ_Compass.Bearing(s.m_vCenter, pos));
		return string.Format("You come to %1 of %2.", side, s.m_sName);
	}

	//------------------------------------------------------------------------------------------------
	//! What kind of job, for the tracker's tag and the Quests tab ("The season's story").
	static string KindLabel(int kind, bool major)
	{
		switch (kind)
		{
			case RZ_EMissionKindUI.CONTRACT: return "Contract";
			case RZ_EMissionKindUI.EVENT: return "World event";
			case RZ_EMissionKindUI.PUZZLE: return "Puzzle";
			case RZ_EMissionKindUI.LEADER: return "Leader's job";
			case RZ_EMissionKindUI.UNARMED: return "Job";
			case RZ_EMissionKindUI.PERIMETER: return "The Perimeter";
			case RZ_EMissionKindUI.ARC: return "Story";
			case RZ_EMissionKindUI.ERRAND: return "Errand";
			case RZ_EMissionKindUI.FINALE: return "The finale";
			case RZ_EMissionKindUI.PACK: return "A pack";
			case RZ_EMissionKindUI.DOORS: return "Doors";
		}

		if (major)
			return "The season's story";

		return "Questline";
	}

	//------------------------------------------------------------------------------------------------
	//! Up to three sentences and about `maxChars`, cut at a sentence's end: the Quests tab's brief.
	static string Short(string text, int maxChars = 480)
	{
		string t = text.Trim();
		if (t.Length() <= maxChars)
			return t;

		int cut = -1;
		for (int i = maxChars - 1; i > 40; i--)
		{
			string ch = t.Get(i);
			if (ch == "." || ch == "!" || ch == "?")
			{
				cut = i;
				break;
			}
		}

		if (cut < 0)
			return t.Substring(0, maxChars).Trim() + "...";

		return t.Substring(0, cut + 1);
	}

	//------------------------------------------------------------------------------------------------
	static string Cap(string text)
	{
		if (text.IsEmpty())
			return text;

		string first = text.Substring(0, 1);
		first.ToUpper();
		return first + text.Substring(1, text.Length() - 1);
	}
}

//------------------------------------------------------------------------------------------------
//! One job a player has, for the tracker and its order.
class RZ_QuestJob
{
	int m_iId;
	int m_iKind;
	int m_iGiver = -1;
	string m_sTitle;
	bool m_bMajor;
	bool m_bTaken;
	int m_iMinutes = -1;
	bool m_bHasPlace;
	vector m_vPlace;		// where the job points this player now (a rough centre while rough)
	bool m_bRough;			// they know only the rough area: the map's search area, not the spot
	float m_fRadius;		// the search area's radius (m), while rough
	float m_fArrive = 25;	// within this of the spot it's reached (unless the job says): the tracker's arrow hides

	//------------------------------------------------------------------------------------------------
	//! The job points where this objective is done (its current one); null leaves the job's own place.
	void PointAt(RZ_ObjPlace at)
	{
		if (!at)
			return;

		m_bHasPlace = true;
		m_vPlace = at.m_vPos;
		m_bRough = at.IsRough();
		m_fRadius = at.m_fSearch;
		m_fArrive = at.m_fArrive;
	}
}

//------------------------------------------------------------------------------------------------
//! Server: per player, the order they took their jobs in and what the tracker last sent.
class RZ_TrackState
{
	ref array<int> m_aTakenOrder = {};
	bool m_bPrimed;			// the jobs taken before the first look don't count as newly taken
	string m_sSent = "-";
	float m_fTimer;
}

//------------------------------------------------------------------------------------------------
//! Server: the tracked job and the HUD's tracker (docs/UI.md, "The quest tracker").
class RZ_QuestTracker
{
	static const float TRACK_SECONDS = 2;
	static const float ARRIVE_MAX = 50;		// a mission's counting radius, at most this: the arrow leads into a big site
	static const float ERRAND_RETURN = 15;	// back with the giver: RZ_Errands.RETURN_RANGE

	protected RZ_GameModeComponent m_GM;
	protected ref map<int, ref RZ_TrackState> m_mStates = new map<int, ref RZ_TrackState>();

	//------------------------------------------------------------------------------------------------
	void RZ_QuestTracker(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TrackState State(int playerId)
	{
		RZ_TrackState st;
		if (!m_mStates.Find(playerId, st))
		{
			st = new RZ_TrackState();
			m_mStates.Set(playerId, st);
		}

		return st;
	}

	//------------------------------------------------------------------------------------------------
	//! A new life or a join: the tracker is sent fresh.
	void Reset(int playerId)
	{
		State(playerId).m_sSent = "-";
	}

	//------------------------------------------------------------------------------------------------
	//! The tracked job's row id, 0 for none.
	int TrackedFor(int playerId)
	{
		return m_GM.GetProfile(playerId).m_iTrackedId;
	}

	// ================================================================ Jobs
	//------------------------------------------------------------------------------------------------
	//! A questline's stage job while the questline has its own row for this player (a major once
	//! they took part, any other questline they can see): the questline's row says where its work is
	//! now, so the stage job beside it would be the same job listed twice.
	static bool IsStageOfListedRun(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (!gm || !m || m.m_iQuestRun < 0)
			return false;

		RZ_Questlines q = gm.GetQuestlines();
		if (!q || !q.IsEnabled())
			return false;

		RZ_QuestRun run = q.FindRun(m.m_iQuestRun);
		if (!run || !q.IsVisibleTo(run, playerId))
			return false;

		if (!run.m_Line.IsMajor())
			return true;

		return run.PlayerTookPartThisLife(gm, playerId) || m.m_aParticipants.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Every job this player has: missions and contracts they see, the questlines they're part of
	//! (a major only once they took part), the errands they hold.
	void Jobs(int playerId, IEntity ent, notnull array<ref RZ_QuestJob> result)
	{
		vector from = vector.Zero;
		if (ent)
			from = ent.GetOrigin();

		RZ_MissionManager mm = m_GM.GetMissions();
		if (mm)
		{
			array<RZ_Mission> list = {};
			mm.ListFor(playerId, list);
			foreach (RZ_Mission m : list)
			{
				// A stage job of a questline listed below is that questline's row, not one of its own
				if (IsStageOfListedRun(m_GM, m, playerId))
					continue;

				RZ_QuestJob j = new RZ_QuestJob();
				j.m_iId = m.m_iId;
				j.m_iKind = RZ_UIFeed.MissionKind(m);
				if (m.m_bHasGiver)
					j.m_iGiver = m.m_Req.m_eGiver;

				j.m_sTitle = m.m_sTitle;
				j.m_bTaken = m.m_aParticipants.Contains(playerId);
				j.m_iMinutes = RZ_Util.RoundToInt(m.m_fTimeLeft / 60);
				j.m_bHasPlace = !m.m_bGlobal;
				j.m_vPlace = m.GetAreaFor(m_GM, playerId, from);
				j.m_bRough = j.m_bHasPlace && m.IsRoughFor(m_GM, playerId, from);
				if (j.m_bRough)
					j.m_fRadius = m.GetAreaRadius(m_GM);

				// Reached once it counts you as there (its participation radius), at most 50 m
				j.m_fArrive = Math.Min(RZ_Arrive.Spot(m.m_fParticipationRadius), ARRIVE_MAX);

				// The job's own place, for an objective without one: where its work is now
				j.PointAt(m.ObjectivePlace(m_GM, playerId, from));

				result.Insert(j);
			}
		}

		RZ_Questlines q = m_GM.GetQuestlines();
		if (q && q.IsEnabled())
		{
			foreach (RZ_QuestRun run : q.GetRuns())
			{
				if (!q.IsVisibleTo(run, playerId))
					continue;

				bool took = run.PlayerTookPartThisLife(m_GM, playerId);
				RZ_Mission stageMission = null;
				if (mm)
					stageMission = mm.FindById(run.m_iMissionId);

				if (stageMission && stageMission.m_aParticipants.Contains(playerId))
					took = true;

				// The majors are everyone's season story (the Season tab): a job once you took part
				if (run.m_Line.IsMajor() && !took)
					continue;

				RZ_QuestJob jq = new RZ_QuestJob();
				jq.m_iId = -run.m_iId;
				jq.m_iKind = RZ_EMissionKindUI.QUESTLINE;
				jq.m_iGiver = run.m_Line.m_iFaction;
				jq.m_sTitle = run.m_Line.m_sTitle;
				jq.m_bMajor = run.m_Line.IsMajor();
				jq.m_bTaken = took;
				jq.m_bHasPlace = run.m_bHasPlace;
				jq.m_vPlace = run.m_vPlace;
				// A stage running: where its work is now, not where the questline began
				if (stageMission)
					jq.PointAt(stageMission.ObjectivePlace(m_GM, playerId, from));

				result.Insert(jq);
			}
		}

		RZ_Errands errands = m_GM.GetErrands();
		if (errands)
		{
			array<RZ_Errand> held = {};
			errands.ForJournal(playerId, held);
			foreach (RZ_Errand e : held)
			{
				RZ_QuestJob je = new RZ_QuestJob();
				je.m_iId = -(RZ_UIFeed.ERRAND_ID_BASE + e.m_iId);
				je.m_iKind = RZ_EMissionKindUI.ERRAND;
				je.m_iGiver = e.m_iFaction;
				je.m_sTitle = e.Fill(e.m_Def.m_sTitle);
				je.m_bTaken = true;
				je.m_iMinutes = errands.MinutesLeft(e);
				je.m_bHasPlace = true;
				je.m_vPlace = e.m_vPlace;
				RZ_ErrandWay way = e.GetWay();
				float wayR = 0;
				if (way)
					wayR = way.m_fRadius;

				je.m_fArrive = RZ_Arrive.Spot(wayR);

				if (e.m_bStepDone)
				{
					je.m_vPlace = errands.GiverAt(e);
					je.m_fArrive = RZ_Arrive.Spot(ERRAND_RETURN);
				}

				result.Insert(je);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_QuestJob FindJob(array<ref RZ_QuestJob> jobs, int id)
	{
		foreach (RZ_QuestJob j : jobs)
		{
			if (j.m_iId == id)
				return j;
		}

		return null;
	}

	// ================================================================ Objectives
	//------------------------------------------------------------------------------------------------
	//! The objectives of one of this player's jobs, by its row id (a mission's id, -run id for a
	//! questline, -(errand base + id) for an errand). Only what the player has learned.
	void Objectives(int playerId, int id, vector from, notnull RZ_ObjList list)
	{
		if (id > 0)
		{
			RZ_Mission m = null;
			if (m_GM.GetMissions())
				m = m_GM.GetMissions().FindById(id);

			if (m)
				m.GetObjectives(m_GM, playerId, from, list);

			return;
		}

		if (id <= -RZ_UIFeed.ERRAND_ID_BASE && id > -RZ_UIFeed.TRIBE_ID_BASE)
		{
			ErrandObjectives(playerId, -id - RZ_UIFeed.ERRAND_ID_BASE, list);
			return;
		}

		RZ_Questlines q = m_GM.GetQuestlines();
		if (!q || id >= 0 || id <= -RZ_UIFeed.ERRAND_ID_BASE)
			return;

		foreach (RZ_QuestRun run : q.GetRuns())
		{
			if (run.m_iId == -id && q.IsVisibleTo(run, playerId))
			{
				run.m_Line.GetObjectives(m_GM, run, playerId, from, list);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! An errand: the step you agreed to, then back to the giver (they told you both).
	protected void ErrandObjectives(int playerId, int errandId, RZ_ObjList list)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands)
			return;

		RZ_Errand e = errands.FindById(errandId);
		if (!e || e.m_iAsker != playerId)
			return;

		RZ_ErrandWay w = e.GetWay();
		string step = "";
		float wayR = 0;
		if (w)
		{
			step = e.Fill(WithoutBack(w.StepText()));
			wayR = w.m_fRadius;
		}

		// The step where the way is done (a subject that walks moves it); back: the giver where
		// they stand now (a wanderer walks on)
		RZ_ObjPlace stepAt = RZ_ObjPlace.Spot(e.m_vPlace, wayR);
		RZ_ObjPlace backAt = RZ_ObjPlace.Person(m_GM, e.m_sGiver, errands.GiverAt(e), ERRAND_RETURN);

		// An opening's last step ends where it's done: nobody to go back to
		if (w && w.m_bEndsThere)
		{
			list.Add(step, RZ_EObjState.CURRENT, stepAt);
			return;
		}

		string back = e.Fill(BackLine(e));

		// Carried for real (Missions/Blocks/RZ_Carry.c): picking one up, then putting it down where
		// it goes, each at its own place, with the count; then the way back
		RZ_Carries carries = RZ_Carries.Get();
		RZ_CarryJob job;
		if (carries && !e.m_bStepDone)
			job = carries.Job(RZ_Errands.WorkKey(e));

		if (job)
		{
			string pickLine;
			string dropLine;
			carries.Lines(job, pickLine, dropLine);
			vector from = job.m_vDrop;
			IEntity carrier = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (carrier)
				from = carrier.GetOrigin();

			RZ_ObjPlace pickAt = RZ_ObjPlace.Spot(carries.NowAt(job, from), 3);
			RZ_ObjPlace dropAt = RZ_ObjPlace.Spot(job.m_vDrop, 3);
			if (job.m_bCarrying)
			{
				list.Add(dropLine, RZ_EObjState.CURRENT, dropAt);
				if (job.m_iDelivered + 1 < job.m_iCount)
					list.Add(pickLine, RZ_EObjState.NEXT, pickAt);
			}
			else
			{
				list.Add(pickLine, RZ_EObjState.CURRENT, pickAt);
				list.Add(dropLine, RZ_EObjState.NEXT, dropAt);
			}

			list.Add(back, RZ_EObjState.NEXT, backAt);
			return;
		}

		// A duty walked with their people (a patrol's round): the marker rides on their lead, and
		// moves to what they stop for (docs/MISSION_RULES.md, rule 9); then the way back
		RZ_DutyScenes scenes = errands.GetScenes();
		RZ_DutySquads squads = errands.GetSquads();
		if (squads && !e.m_bStepDone && squads.Objectives(e, list, scenes))
		{
			list.Add(back, RZ_EObjState.NEXT, backAt);
			return;
		}

		// A duty's scene names its own steps (take the box, carry it, hand it over, sign): each
		// is one objective, then the way back
		if (scenes && scenes.Objectives(e, list, wayR, e.m_bStepDone))
		{
			if (e.m_bStepDone || list.First(RZ_EObjState.CURRENT).IsEmpty())
				list.Add(back, RZ_EObjState.CURRENT, backAt);
			else
				list.Add(back, RZ_EObjState.NEXT, backAt);

			return;
		}

		// A first place before the step's own (RZ_ErrandWay.First): its own line
		if (w && w.HasFirst() && e.m_vFirst != vector.Zero)
		{
			string first = e.Fill(w.m_sFirst);
			if (!e.m_bFirstDone)
			{
				list.Add(first, RZ_EObjState.CURRENT, RZ_ObjPlace.Spot(e.m_vFirst, wayR));
				list.Add(step, RZ_EObjState.NEXT, stepAt);
				list.Add(back, RZ_EObjState.NEXT, backAt);
				return;
			}

			list.Add(first, RZ_EObjState.DONE, null);
		}

		if (e.m_bStepDone)
		{
			list.Add(step, RZ_EObjState.DONE, stepAt);
			list.Add(back, RZ_EObjState.CURRENT, backAt);
			return;
		}

		list.Add(step, RZ_EObjState.CURRENT, stepAt);
		list.Add(back, RZ_EObjState.NEXT, backAt);
	}

	//------------------------------------------------------------------------------------------------
	//! A step's words without their last clause when it only sends you back to the giver ("..., then
	//! tell {giver}."): the way back is an objective of its own (docs/WRITING.md, "Objective lines").
	static string WithoutBack(string step)
	{
		int cut = -1;
		int from = 0;
		array<string> joins = {", then ", ". Then ", " and bring ", ", and bring "};
		foreach (string join : joins)
		{
			int k = step.LastIndexOf(join);
			if (k > cut)
			{
				cut = k;
				from = k + join.Length();
			}
		}

		if (cut <= 0)
			return step;

		string tail = step.Substring(from, step.Length() - from);
		if (!IsBackWord(tail))
			return step;

		string head = step.Substring(0, cut);
		if (!head.EndsWith("."))
			head += ".";

		return head;
	}

	//------------------------------------------------------------------------------------------------
	//! The clause only takes you, or what you carry, back to the giver ("tell {giver}", "come back
	//! to {giver}", "bring it to {giver}"): it names the giver and starts with one of these.
	protected static bool IsBackWord(string tail)
	{
		if (!tail.Contains("{giver}"))
			return false;

		string low = tail;
		low.ToLower();
		array<string> starts = {"tell ", "come back", "go back", "report", "find {giver}", "see {giver}", "hand ", "bring ", "carry it to", "carry them to", "it to {giver}", "them to {giver}"};
		foreach (string word : starts)
		{
			if (low.StartsWith(word))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! An errand's way back, naming the giver and where they are (docs/WRITING.md, "Objective
	//! lines"): a safehouse's person at their house, a holding's at the holding, a wanderer on the road.
	protected static string BackLine(RZ_Errand e)
	{
		if (e.m_iWanderer >= 0)
			return "Find {giver} on the road again.";

		if (e.GetText("here").IsEmpty())
			return "Go back to {giver}.";

		if (e.m_iHouse >= 0)
			return "Go back to {giver} at the safehouse in {here}.";

		return "Go back to {giver} at {here}.";
	}

	// ================================================================ Tracking
	//------------------------------------------------------------------------------------------------
	//! Track picked in the Quests tab: that job is followed; the same one again stops it.
	void Toggle(int playerId, int id)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		array<ref RZ_QuestJob> jobs = {};
		Jobs(playerId, ent, jobs);
		RZ_QuestJob j = FindJob(jobs, id);
		if (!j || p.m_iTrackedId == id)
		{
			p.m_iTrackedId = 0;
			p.m_sTrackedTitle = "";
			p.m_bTrackOff = true;
		}
		else
		{
			p.m_iTrackedId = id;
			p.m_sTrackedTitle = j.m_sTitle;
			p.m_bTrackOff = false;
		}

		Push(playerId, ent, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Another system wants this job followed now (an opening's next step, Guidance/RZ_Openings.c):
	//! tracked, and the tracker sent at once.
	void Follow(int playerId, int id, string title)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		p.m_iTrackedId = id;
		p.m_sTrackedTitle = title;
		p.m_bTrackOff = false;
		Push(playerId, GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId), true);
	}

	//------------------------------------------------------------------------------------------------
	//! From the UI feed's tick.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			RZ_TrackState st = State(playerId);
			st.m_fTimer -= dt;
			if (st.m_fTimer > 0)
				continue;

			st.m_fTimer = TRACK_SECONDS;
			Push(playerId, ent, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps the tracked job valid (a newly taken job is always tracked; one that ended
	//! falls back to the newest taken, unless the player untracked on purpose) and sends the
	//! tracker if it changed.
	protected void Push(int playerId, IEntity ent, bool force)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!ui)
			return;

		RZ_TrackState st = State(playerId);
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		array<ref RZ_QuestJob> jobs = {};
		Jobs(playerId, ent, jobs);

		// The order they were taken in, newest last
		int newest = 0;
		foreach (RZ_QuestJob j : jobs)
		{
			if (j.m_bTaken && !st.m_aTakenOrder.Contains(j.m_iId))
			{
				st.m_aTakenOrder.Insert(j.m_iId);
				if (st.m_bPrimed)
					newest = j.m_iId;
			}
		}

		st.m_bPrimed = true;

		for (int k = st.m_aTakenOrder.Count() - 1; k >= 0; k--)
		{
			RZ_QuestJob still = FindJob(jobs, st.m_aTakenOrder[k]);
			if (!still || !still.m_bTaken)
				st.m_aTakenOrder.Remove(k);
		}

		// A saved choice from an earlier session: the same job by its title
		// Tracking a questline's stage job is tracking the questline (its one row)
		if (p.m_iTrackedId > 0 && m_GM.GetMissions())
		{
			RZ_Mission stage = m_GM.GetMissions().FindById(p.m_iTrackedId);
			if (stage && IsStageOfListedRun(m_GM, stage, playerId))
				p.m_iTrackedId = -stage.m_iQuestRun;
		}

		RZ_QuestJob tracked = FindJob(jobs, p.m_iTrackedId);
		if (!tracked && p.m_iTrackedId != 0 && !p.m_sTrackedTitle.IsEmpty())
		{
			foreach (RZ_QuestJob byTitle : jobs)
			{
				if (byTitle.m_sTitle == p.m_sTrackedTitle)
				{
					tracked = byTitle;
					p.m_iTrackedId = byTitle.m_iId;
					break;
				}
			}
		}

		// A job just taken (agreed to in a talk) is followed at once, whatever was followed before
		if (newest != 0)
		{
			tracked = FindJob(jobs, newest);
			p.m_bTrackOff = false;
		}
		else if (!tracked && !p.m_bTrackOff && !st.m_aTakenOrder.IsEmpty())
		{
			tracked = FindJob(jobs, st.m_aTakenOrder[st.m_aTakenOrder.Count() - 1]);
		}

		if (p.m_bTrackOff)
			tracked = null;

		if (tracked)
		{
			p.m_iTrackedId = tracked.m_iId;
			p.m_sTrackedTitle = tracked.m_sTitle;
		}
		else
		{
			p.m_iTrackedId = 0;
		}

		string payload = Build(playerId, ent, tracked);
		if (!force && payload == st.m_sSent)
			return;

		st.m_sSent = payload;
		ui.SendPayload(RZ_UIFeed.PAYLOAD_TRACKER, payload);
	}

	//------------------------------------------------------------------------------------------------
	//! T|id|kind|giver|title|kind label|rough area|place|x|z|arrive|search
	//!   all of the CURRENT objective's place (the job's own when it has none): place: the area's
	//!   place alone ("East of Levie"); x, z: where it points (the rough circle's centre while rough,
	//!   never the hidden spot), empty without a place; arrive: the spot is reached within it (m);
	//!   search: the rough circle's radius (m), 0 for a known spot. The HUD turns them into an arrow
	//!   and metres within a km (RZ_HudDisplay.DrawTrackerDir).
	//! O|state|line|place (the tracked job's current objective and the known next one; the next
	//!   one's place in words only when it's somewhere else than the current one's)
	//! Only the tracked job: the other jobs taken are the Quests tab's, never the HUD's.
	//! "" when nothing is tracked: the box hides.
	protected string Build(int playerId, IEntity ent, RZ_QuestJob tracked)
	{
		if (!tracked)
			return "";

		vector from = vector.Zero;
		if (ent)
			from = ent.GetOrigin();

		// Where the current objective is done, not the job's start (the job's place without one)
		RZ_ObjList list = new RZ_ObjList();
		Objectives(playerId, tracked.m_iId, from, list);
		tracked.PointAt(list.Now());

		string area = "";
		string place = "";
		string x = "";
		string z = "";
		int search = 0;
		if (tracked.m_bHasPlace)
			area = RZ_QuestText.RoughArea(m_GM, from, tracked.m_vPlace);

		if (tracked.m_bHasPlace && tracked.m_vPlace != vector.Zero)
		{
			place = RZ_QuestText.RoughArea(m_GM, vector.Zero, tracked.m_vPlace);
			int xi = RZ_Util.RoundToInt(tracked.m_vPlace[0]);
			int zi = RZ_Util.RoundToInt(tracked.m_vPlace[2]);
			x = xi.ToString();
			z = zi.ToString();
			if (tracked.m_bRough && tracked.m_fRadius > 0)
				search = RZ_Util.RoundToInt(tracked.m_fRadius);
		}

		RZ_UILine lnT = new RZ_UILine("T");
		lnT.AddInt(tracked.m_iId).AddInt(tracked.m_iKind).AddInt(tracked.m_iGiver).Add(tracked.m_sTitle).Add(RZ_QuestText.KindLabel(tracked.m_iKind, tracked.m_bMajor)).Add(area);
		string s = lnT.Add(place).Add(x).Add(z).AddInt(RZ_Util.RoundToInt(tracked.m_fArrive)).AddInt(search).End();

		string now = list.First(RZ_EObjState.CURRENT);
		string next = list.First(RZ_EObjState.NEXT);
		if (now.IsEmpty())
			now = tracked.m_sTitle;

		RZ_UILine lnNow = new RZ_UILine("O");
		s += lnNow.AddInt(RZ_EObjState.CURRENT).Add(now).Add(place).End();
		if (!next.IsEmpty())
		{
			// The next one's place in words when it's somewhere else ("Go back to Marek" · Morton)
			string nextPlace = OtherPlace(list.PlaceOf(RZ_EObjState.NEXT), place);
			RZ_UILine lnNext = new RZ_UILine("O");
			s += lnNext.AddInt(RZ_EObjState.NEXT).Add(next).Add(nextPlace).End();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! An objective's place in words ("East of Levie"), "" without one or when it's `same`.
	string OtherPlace(RZ_ObjPlace at, string same)
	{
		if (!at)
			return "";

		string words = at.Words(m_GM);
		if (words == same)
			return "";

		return words;
	}

	//------------------------------------------------------------------------------------------------
	//! Where one of this player's jobs points now (the journal's row): its current objective's place,
	//! null when that has none (the job's own place then).
	RZ_ObjPlace NowPlace(int playerId, int id, vector from)
	{
		RZ_ObjList list = new RZ_ObjList();
		Objectives(playerId, id, from, list);
		return list.Now();
	}

	//------------------------------------------------------------------------------------------------
	//! One of this player's jobs as the tracker shows it: its place is its current objective's
	//! (the job's own without one), its objectives in `list`. Null if it isn't theirs. `ent`: the
	//! player's character (where "from" is), null for none.
	RZ_QuestJob JobFor(int playerId, int id, IEntity ent, notnull RZ_ObjList list)
	{
		array<ref RZ_QuestJob> jobs = {};
		Jobs(playerId, ent, jobs);
		RZ_QuestJob j = FindJob(jobs, id);
		if (!j)
			return null;

		vector from = vector.Zero;
		if (ent)
			from = ent.GetOrigin();

		Objectives(playerId, id, from, list);
		j.PointAt(list.Now());
		return j;
	}

	//------------------------------------------------------------------------------------------------
	//! The map's mark for the tracked job's current objective (docs/UI.md, "The map"), for this
	//! player alone; "" if nothing is tracked or it has no place. Labelled with the job's name only
	//! (the objective's words stay on the tracker). A known spot is a diamond on it:
	//! P|objective_spot|-1|x|z|name. A search only is the rough circle, never the spot:
	//! P|objective_area|-1|x|z|name|radius. Every other job stays off the map.
	string MapObjective(int playerId)
	{
		int id = TrackedFor(playerId);
		if (id == 0)
			return "";

		RZ_ObjList list = new RZ_ObjList();
		RZ_QuestJob j = JobFor(playerId, id, GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId), list);
		if (!j || !j.m_bHasPlace || j.m_vPlace == vector.Zero)
			return "";

		string now = j.m_sTitle;
		if (now.IsEmpty())
			now = list.First(RZ_EObjState.CURRENT);

		int x = RZ_Util.RoundToInt(j.m_vPlace[0]);
		int z = RZ_Util.RoundToInt(j.m_vPlace[2]);
		RZ_UILine ln = new RZ_UILine("P");
		if (j.m_bRough && j.m_fRadius > 0)
			return ln.Add("objective_area").AddInt(-1).AddInt(x).AddInt(z).Add(now).AddInt(RZ_Util.RoundToInt(j.m_fRadius)).End();

		return ln.Add("objective_spot").AddInt(-1).AddInt(x).AddInt(z).Add(now).End();
	}

	//------------------------------------------------------------------------------------------------
	//! Where the tracked job's current objective is (the map's mark, and the map's fog when asking
	//! about the roads). False if nothing is tracked or it has no place.
	bool ObjectivePlace(int playerId, out vector at, out bool rough, out float radius)
	{
		int id = TrackedFor(playerId);
		if (id == 0)
			return false;

		RZ_ObjList list = new RZ_ObjList();
		RZ_QuestJob j = JobFor(playerId, id, GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId), list);
		if (!j || !j.m_bHasPlace || j.m_vPlace == vector.Zero)
			return false;

		at = j.m_vPlace;
		rough = j.m_bRough;
		radius = j.m_fRadius;
		return true;
	}
}
