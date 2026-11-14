// Test-only (not part of the mod): a patrol's finds and its marker (docs/ERRANDS.md, "Things that
// happen"; docs/MISSION_RULES.md, rules 2, 3 and 9). Once, six minutes in:
// (1) the finds: RZ_DutyFinds.Patrol at eight stops round the tester's town. Every find returned
//     must stand within its reach of the stop; a door, a house or a box under a step must have its
//     prompt within 1.5 m of a real doorway; every other find must have its thing to stand there
//     (its look). A stop where nothing fits returns nothing (counted, not a failure).
// (2) the marker: an escort walking a route, as a patrol does, must give its objective at itself,
//     where it is now, never at the route's end; checked as it walks.
// Lines: "RZT patrol: ..." and "RZT patrol FAIL ...".

class RZT_Patrol
{
	static const int START = 360;		// seconds of the work sites' tick before it starts
	static const float DOOR_NEAR = 1.5;
	static int s_iTicks;
	static int s_iStep;
	static int s_iWait;
	static int s_iFails;
	static bool s_bDone;
	static ref RZ_Escort s_Escort;
	static vector s_vEnd;
	static vector s_vFirst;

	//------------------------------------------------------------------------------------------------
	static void Fail(string what)
	{
		s_iFails++;
		Print("RZT patrol FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	static bool ByDoor(vector at)
	{
		IEntity door;
		vector centre;
		vector normal;
		if (!RZ_DoorGeometry.NearestOuterDoor(at, 0, 6, false, door, centre, normal))
			return false;

		return RZ_Util.DistXZ(centre, at) <= DOOR_NEAR;
	}

	//------------------------------------------------------------------------------------------------
	static void Finds(RZ_GameModeComponent gm)
	{
		RZ_Errand e = new RZ_Errand();
		e.m_iId = 990001;
		e.m_iAsker = RZT_Fake.ID;
		vector home = RZT_Fake.s_Ent.GetOrigin();
		int found = 0;
		int none = 0;
		for (int i = 0; i < 8; i++)
		{
			float bearing = i * 45 * Math.DEG2RAD;
			vector stop = RZ_Util.GroundPos(home + Vector(Math.Sin(bearing), 0, Math.Cos(bearing)) * 180);
			array<ref RZ_DutyBeat> beats = {};
			string call;
			if (!RZ_DutyFinds.Patrol(gm, e, stop, "Lance Corporal Test", beats, call))
			{
				none++;
				continue;
			}

			found++;
			RZ_DutyBeat b = beats[0];
			float far = RZ_Util.DistXZ(b.m_vAt, stop);
			bool door = b.m_sKey == "door" || b.m_sKey == "house" || b.m_sKey == "cache";
			Print(string.Format("RZT patrol: find %1 at %2, %3 m from the stop; called \"%4\"; goal \"%5\"", b.m_sKey, RZ_SettlementScanner.GridReference(b.m_vAt), RZ_Util.RoundToInt(far), call, b.m_sGoal));
			if (far > RZ_DutyFinds.FIND_REACH + 5)
				Fail(string.Format("%1 at %2 is %3 m from its stop (reach %4)", b.m_sKey, RZ_SettlementScanner.GridReference(b.m_vAt), RZ_Util.RoundToInt(far), RZ_DutyFinds.FIND_REACH));

			if (door && !ByDoor(b.m_vAt))
				Fail(string.Format("%1's prompt at %2 has no real doorway within %3 m", b.m_sKey, RZ_SettlementScanner.GridReference(b.m_vAt), DOOR_NEAR));

			if (!door && b.m_sProp.IsEmpty())
				Fail(string.Format("%1 at %2 has nothing to stand there (no look)", b.m_sKey, RZ_SettlementScanner.GridReference(b.m_vAt)));

			if (b.m_sGoal.IsEmpty() || call.IsEmpty())
				Fail(string.Format("%1 has no step for the tracker or nothing for the lead to say", b.m_sKey));
		}

		Print(string.Format("RZT patrol: finds at %1 of 8 stops, %2 with none that fits (left out)", found, none));
	}

	//------------------------------------------------------------------------------------------------
	static void Step(RZ_WorkSites sites)
	{
		if (s_bDone || !RZT_Fake.s_Ent || !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
			return;

		s_iTicks++;
		if (s_iTicks < START)
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (s_iStep == 0)
		{
			Finds(gm);
			// A data escort (no body: nothing to spawn) walking a route away from the tester
			s_Escort = new RZ_Escort();
			s_Escort.m_sName = "the test patrol";
			vector start = RZT_Fake.s_Ent.GetOrigin();
			if (!s_Escort.Begin(gm, start))
			{
				Print("RZT patrol: no escort room; the marker half not tried");
				Finish();
				return;
			}

			s_vEnd = RZ_Util.GroundPos(start + Vector(300, 0, 0));
			array<vector> route = {RZ_Util.GroundPos(start + Vector(150, 0, 0)), s_vEnd};
			s_Escort.SetRoute(route);
			s_Escort.Go();
			RZ_ObjPlace at = s_Escort.ObjPlace();
			if (at)
				s_vFirst = at.m_vPos;

			s_iStep = 1;
			s_iWait = 0;
			return;
		}

		if (s_iStep == 1)
		{
			s_iWait++;
			array<int> ids = {};
			array<IEntity> players = {};
			s_Escort.Update(gm, 1, ids, players);
			if (s_iWait < 20)
				return;

			RZ_ObjPlace now = s_Escort.ObjPlace();
			if (!now)
			{
				Print("RZT patrol: the escort arrived or died before the check");
			}
			else
			{
				float toEnd = RZ_Util.DistXZ(now.m_vPos, s_vEnd);
				float toHim = RZ_Util.DistXZ(now.m_vPos, s_Escort.NowAt());
				Print(string.Format("RZT patrol: walking, the marker is %1 m from him and %2 m from the route's end; it moved %3 m", RZ_Util.RoundToInt(toHim), RZ_Util.RoundToInt(toEnd), RZ_Util.RoundToInt(RZ_Util.DistXZ(now.m_vPos, s_vFirst))));
				if (toHim > 1)
					Fail("an escort walking a route gives its objective away from itself");

				if (toEnd < 1 && toHim > 1)
					Fail("an escort walking a route gives its objective at the route's end");
			}

			Finish();
		}
	}

	//------------------------------------------------------------------------------------------------
	static void Finish()
	{
		if (s_Escort)
			s_Escort.Cleanup();

		Print(string.Format("RZT patrol: done, %1 failures", s_iFails));
		s_bDone = true;
	}
}

modded class RZ_WorkSites
{
	//------------------------------------------------------------------------------------------------
	override void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		RZT_Patrol.Step(this);
		super.Tick(dt, ids, players);
	}
}
