//! Shared by the four majors (docs/QUESTLINES.md, "The majors"): the canon places (Rudi's
//! crossroads, the north road), pages in the open, the epilogue's evenings and small reads every
//! major needs.
class RZ_MajorHelp
{
	//------------------------------------------------------------------------------------------------
	//! Rudi's crossroads (canon redhand.crossroads).
	static vector Crossroads()
	{
		return RZ_WorldFacts.Get().CanonPos("redhand.crossroads");
	}

	//------------------------------------------------------------------------------------------------
	//! A point on the north road `metres` short of where the Day 2 column was turned back (canon
	//! day2.checkpoint), toward the Haven stronghold: 150 for the walks and readings, 110 for the
	//! paper's table. Outside the 60 m turn-back and the 100 m kill zone.
	static vector NorthRoad(RZ_GameModeComponent gm, float metres)
	{
		vector gate = RZ_WorldFacts.Get().CanonPos("day2.checkpoint");
		if (gate == vector.Zero)
			return vector.Zero;

		vector home = gate;
		int chair = RZ_HavenQuest.ChairHouse(gm);
		if (chair >= 0)
			home = gm.GetSafehouses().Get(chair).m_vPos;

		vector dir = home - gate;
		dir[1] = 0;
		float len = dir.Length();
		if (len < 1)
			return RZ_Mission.LandPointNear(gate, metres, metres + 20);

		dir = dir * (1 / len);
		return RZ_Util.GroundPos(gate + dir * Math.Min(metres, len));
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Leader Top(RZ_GameModeComponent gm, int faction)
	{
		if (!gm || !gm.GetLeaders())
			return null;

		RZ_Leader l = gm.GetLeaders().GetStronghold(faction);
		if (!l || !l.IsPresent())
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! The stronghold leader of `faction` has `trait` (read only: they never change).
	static bool TopHas(RZ_GameModeComponent gm, int faction, int trait)
	{
		RZ_Leader l = Top(gm, faction);
		return l && l.HasTrait(trait);
	}

	//------------------------------------------------------------------------------------------------
	static int World(RZ_GameModeComponent gm, string key, int def = 0)
	{
		if (!gm || !gm.GetQuestlines())
			return def;

		return gm.GetQuestlines().WorldFlag(key, def);
	}

	//------------------------------------------------------------------------------------------------
	static void SetWorld(RZ_GameModeComponent gm, string key, int value)
	{
		if (gm && gm.GetQuestlines())
			gm.GetQuestlines().SetWorldFlag(key, value);
	}

	//------------------------------------------------------------------------------------------------
	//! A stronghold's page put in the open (majors.pages_open): the Red Hand's book read whole,
	//! Bastion's All Stations, the Haven's List read.
	static void OpenPage(RZ_GameModeComponent gm, string what)
	{
		SetWorld(gm, "majors.pages_open", World(gm, "majors.pages_open") + 1);
		RZ_Log.Info("Majors: a page in the open: " + what);
	}

	//------------------------------------------------------------------------------------------------
	//! One step off a "read it" check, once per major (another stronghold's page is already out).
	static int PageStep(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (World(gm, "majors.pages_open") > 0 && run.Flag("page_step_used") == 0)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The page step was used (a persuade that counted it passed).
	static void UsePageStep(RZ_QuestRun run)
	{
		run.SetFlag("page_step_used", 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue week runs (the cure is out), and the coda before this one has run (or that
	//! major isn't running: the order never waits on a story that isn't there).
	static bool CodaDue(RZ_GameModeComponent gm, string before)
	{
		if (!RZ_SeasonClock.IsEpilogue())
			return false;

		if (before.IsEmpty())
			return true;

		return World(gm, before + ".coda") != 0 || !gm.GetQuestlines().IsRunning(before);
	}

	//------------------------------------------------------------------------------------------------
	//! At least `hours` of play since the run's stage began.
	static bool PlayedSince(RZ_QuestRun run, float hours)
	{
		return run.StageMinutes() >= hours * 60 * run.m_Owner.WaitScale();
	}

	//------------------------------------------------------------------------------------------------
	//! The identities who did `approach` at `stage` (-1: any approach), at most four.
	static void Deciders(RZ_QuestRun run, int stage, int approach, notnull array<string> result)
	{
		foreach (RZ_QuestPart p : run.m_aParts)
		{
			if (p.m_iStage != stage || p.m_iApproach == RZ_EQuestApproach.NONE)
				continue;

			if (approach >= 0 && p.m_iApproach != approach)
				continue;

			if (!result.Contains(p.m_sIdent) && result.Count() < 4)
				result.Insert(p.m_sIdent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Standing for the identities in `idents` (online now or on their saved profile), inside the
	//! majors' caps.
	static void PayIdents(RZ_GameModeComponent gm, notnull array<string> idents, int faction, int fame, int infamy)
	{
		foreach (string ident : idents)
		{
			RZ_Questline.ChangeStandingFor(gm, ident, faction, fame, infamy);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A truce or a feud between two factions (faction life's caps: one per pair, a second start
	//! extends the first). False with faction life off.
	static bool Relation(RZ_GameModeComponent gm, int a, int b, int kind)
	{
		RZ_FactionLife life = gm.GetFactionLife();
		if (!life || !life.IsActive())
			return false;

		life.StartRelation(a, b, kind);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A set piece asks for this evening (RZ_SetPieceCalendar): true when it holds it (or has waited
	//! its evenings and runs anyway). A held evening counts as one wait, once per evening.
	static bool HoldEvening(RZ_QuestRun run, string key, vector pos)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		if (!cal || cal.Holder() == key)
			return true;

		if (cal.IsFree())
			return cal.Hold(key, pos);

		if (cal.MustRun(key))
			return true;

		int eve = cal.Evening();
		if (run.Flag("cal." + key, -1) != eve)
		{
			run.SetFlag("cal." + key, eve);
			cal.Hold(key, pos);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A stand-in's spot near a seat (never behind a lock).
	static vector NearSeat(vector seat)
	{
		return RZ_Mission.LandPointNear(seat, 5, 12);
	}
}
