//! The line (docs/DESIGN.md, "Openings": the line): the opening's beat 3. Favour done, the contact
//! wants out; you walk with him to a road above the line (a viewpoint 150-250 m outside the
//! Perimeter's warning zone, 600 m to 2 km from where the chain is). There the loudspeakers start
//! and a drone goes over; a man with a pack breaks from cover 100 m ahead and runs at the line, and
//! the Perimeter's unseen snipers drop him just inside the warning zone. Quiet; then the contact's
//! line. His body is where the case is found (RZ_OpeningCase, the next stage).
//!
//! The player is never the target and is never kept out of the zone: walk toward the line and the
//! Perimeter's own warnings run as built. The runner is a one-man civilian group on the escort block
//! (Guidance > Companion), spawned only for the player watching; no prefab or no escort room, and he
//! is found already down where he would have fallen. Server only.

//------------------------------------------------------------------------------------------------
//! One opening's line, while it runs (RZ_Opening.m_Line).
class RZ_LineScene
{
	static const int WALK = 0;	// walking with him to the road above the line
	static const int WAIT = 1;	// there: the loudspeakers, a drone
	static const int RUN = 2;	// a man breaks from cover and runs at the line
	static const int DOWN = 3;	// shot in the warning zone: quiet, then his line
	static const int OVER = 4;

	vector m_vView;				// where you watch from (on the road)
	vector m_vBreak;			// where the runner breaks from cover
	vector m_vFall;				// just inside the warning zone: where he's dropped
	vector m_vOut;				// unit vector toward the line
	int m_iPlayer;
	int m_iPhase;
	float m_fAt;				// seconds into the phase
	ref RZ_Escort m_Runner;
	IEntity m_Body;				// weak: the runner where he fell
	IEntity m_Drone;			// weak
	bool m_bSpeakers;
	bool m_bWarned;
	bool m_bSaid;
	bool m_bStalled;			// he never got there: shot where he stood, or laid down as data
	float m_fShotDepth;			// how far inside the warning zone he fell (m)
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningLine
{
	static const string LINE = "Eleven days. They said aid was coming.";
	static string s_sWhy;		// the last search's reasons, when it found nothing (logs)

	protected static const float VIEW_OUT_MIN = 150;	// the viewpoint: this far outside the warning zone...
	protected static const float VIEW_OUT_MAX = 250;	// ...and no further
	protected static const float VIEW_FROM_MIN = 600;	// and this far from where the chain is...
	protected static const float VIEW_FROM_MAX = 2000;	// ...and no further
	protected static const float BREAK_AHEAD = 100;		// the runner breaks from cover this far ahead of you
	protected static const float FALL_IN = 15;			// he's dropped this far inside the warning zone
	protected static const float RUN_ON = 60;			// he runs for a point this far past it (water or not)
	protected static const float CHECKPOINT_CLEAR = 300;	// never by a sealed checkpoint (its guards would be seen)
	protected static const int MAX_CHECKS = 40;		// place checks per search
	protected static const float CROWD_RADIUS = 800;	// other players this close to the viewpoint...
	protected static const int CROWD = 2;				// ...this many or more: the short form
	protected static const float SPEAKERS_AFTER = 3;	// seconds at the viewpoint before the loudspeakers
	protected static const float RUNNER_AFTER = 12;		// ...and before he breaks
	protected static const float RUN_STALL = 60;		// a runner who hasn't got there by now is shot where he is
	protected static const float BODY_WAIT = 25;		// no body by now: he's found down where he'd have fallen
	protected static const float LINE_AFTER = 6;		// quiet, then the contact's line
	protected static const float OVER_AFTER = 6;		// and then on to him

	//------------------------------------------------------------------------------------------------
	//! A viewpoint on the line in reach of `from`: a road point 150-250 m outside the warning zone,
	//! 600 m to 2 km away, dry land where he breaks from cover and where he falls (he may run on for
	//! the water: on an island map the line is past the shore), away from sealed checkpoints. False:
	//! none here (the short form); s_sWhy then says what ruled the road points out.
	static bool FindViewpoint(RZ_GameModeComponent gm, vector from, out vector view, out vector breakAt, out vector fall, out vector outward)
	{
		s_sWhy = "";
		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (!perimeter || !perimeter.IsActive())
		{
			s_sWhy = "the line is off";
			return false;
		}

		float warn = perimeter.GetWarnDistance();
		float clear = perimeter.GetCheckpointWarnRadius() + CHECKPOINT_CLEAR;
		array<vector> points = {};
		RZ_Perimeter.RoadPointsNear(from, VIEW_FROM_MAX, points);
		RZ_PlaceNeed need = RZ_PlaceNeeds.Get("opening.line");
		// No road network in this world (a test world, a map without one): open ground on a ring of
		// points instead, said in the reasons
		string ground = "road";
		if (points.IsEmpty())
		{
			ground = "open ground (no road network)";
			need = RZ_PlaceNeeds.Get("opening.line.open");
			for (float r = VIEW_FROM_MIN; r <= VIEW_FROM_MAX; r += 200)
			{
				for (int a = 0; a < 360; a += 10)
				{
					float rad = a * Math.DEG2RAD;
					points.Insert(RZ_Util.GroundPos(from + Vector(Math.Sin(rad), 0, Math.Cos(rad)) * r));
				}
			}
		}

		int count = points.Count();
		int checks = 0;
		int far = 0;
		int band = 0;
		int wet = 0;
		int post = 0;
		int bad = 0;
		int first = 0;
		if (count > 0)
			first = Math.RandomInt(0, count);

		for (int i = 0; i < count && checks < MAX_CHECKS; i++)
		{
			vector pt = points[(first + i) % count];
			float d = RZ_Util.DistXZ(from, pt);
			if (d < VIEW_FROM_MIN || d > VIEW_FROM_MAX)
			{
				far++;
				continue;
			}

			float edge = perimeter.EdgeDistance(pt);
			if (edge < warn + VIEW_OUT_MIN || edge > warn + VIEW_OUT_MAX)
			{
				band++;
				continue;
			}

			vector away = -perimeter.InwardAt(pt);
			vector b = RZ_Util.GroundPos(pt + away * BREAK_AHEAD);
			vector f = RZ_Util.GroundPos(pt + away * (edge - warn + FALL_IN));
			if (RZ_Util.IsWater(b) || RZ_Util.IsWater(f))
			{
				wet++;
				continue;
			}

			if (perimeter.GetCheckpointDistance(pt) < clear || perimeter.GetCheckpointDistance(f) < clear)
			{
				post++;
				continue;
			}

			checks++;
			string why;
			if (!RZ_PlaceCheck.Check(RZ_Util.GroundPos(pt), need, -1, "", why))
			{
				bad++;
				continue;
			}

			view = RZ_Util.GroundPos(pt);
			breakAt = b;
			fall = f;
			outward = away;
			return true;
		}

		s_sWhy = string.Format("%1 %7 points: %2 too near or far, %3 not 150-250 m outside the warning zone, %4 with water where he'd break or fall, %5 by a checkpoint, %6 failing the place's need", count, far, band, wet, post, bad, ground);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Other players near the viewpoint: with this many the line runs as its short form (a body found,
	//! no scene played for a crowd).
	static bool Crowded(vector view, int playerId, array<int> ids, array<IEntity> players)
	{
		int near = 0;
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != playerId && p && RZ_Util.DistXZ(p.GetOrigin(), view) <= CROWD_RADIUS)
				near++;
		}

		return near >= CROWD;
	}

	//------------------------------------------------------------------------------------------------
	//! The scene from WAIT on, once a second while the opening's watch step runs. `contact` says the
	//! line (null: nobody there to say it). The phase reaches OVER when it's done.
	static void Tick(RZ_GameModeComponent gm, RZ_LineScene sc, IEntity contact, string speaker, ResourceName runnerPrefab, float dt, array<int> ids, array<IEntity> players)
	{
		if (!sc || sc.m_iPhase == RZ_LineScene.WALK || sc.m_iPhase == RZ_LineScene.OVER)
			return;

		sc.m_fAt += dt;
		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (sc.m_iPhase == RZ_LineScene.WAIT)
		{
			if (!sc.m_bSpeakers && sc.m_fAt >= SPEAKERS_AFTER)
			{
				sc.m_bSpeakers = true;
				gm.Say(sc.m_iPlayer, RZ_Perimeter.LOUDSPEAKER, -1, "This area is under federal quarantine. Remain indoors. Do not approach the perimeter. Aid is on its way.");
				sc.m_Drone = SpawnDrone(perimeter, sc.m_vBreak);
				RZ_Log.Info(string.Format("Openings: the line for %1 at %2: the loudspeakers, a drone %3", RZ_Util.PlayerName(sc.m_iPlayer), RZ_SettlementScanner.GridReference(sc.m_vView), sc.m_Drone != null));
			}

			if (sc.m_fAt >= RUNNER_AFTER)
				StartRunner(gm, sc, runnerPrefab);

			return;
		}

		if (sc.m_iPhase == RZ_LineScene.RUN)
		{
			TickRunner(gm, sc, perimeter, dt, ids, players);
			return;
		}

		// DOWN: quiet, then his line, then on
		if (!sc.m_bSaid && sc.m_fAt >= LINE_AFTER)
		{
			sc.m_bSaid = true;
			gm.Say(sc.m_iPlayer, speaker, -1, LINE, contact);
			RZ_Log.Info(string.Format("Openings: the line for %1: \"%2\" said by %3 (%4)", RZ_Util.PlayerName(sc.m_iPlayer), LINE, speaker, contact != null));
		}

		if (sc.m_fAt >= LINE_AFTER + OVER_AFTER)
		{
			SetPhase(sc, RZ_LineScene.OVER);
			RZ_Util.DeleteUnit(sc.m_Drone);
			sc.m_Drone = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetPhase(RZ_LineScene sc, int phase)
	{
		sc.m_iPhase = phase;
		sc.m_fAt = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! He breaks from cover: a one-man civilian group on the escort block, running for a point past
	//! the warning zone's edge. No prefab or no escort room: he's laid down where he'd have fallen.
	protected static void StartRunner(RZ_GameModeComponent gm, RZ_LineScene sc, ResourceName prefab)
	{
		SetPhase(sc, RZ_LineScene.RUN);
		if (prefab.IsEmpty())
		{
			LayDown(gm, sc, "no runner prefab (Guidance > Companion)");
			return;
		}

		RZ_Escort runner = new RZ_Escort();
		runner.m_sPrefab = prefab;
		runner.m_sName = "the runner";
		runner.m_bHide = false;
		runner.m_fLeash = 1000;
		runner.m_fArriveRadius = 4;
		if (!runner.Begin(gm, sc.m_vBreak))
		{
			LayDown(gm, sc, "no escort room");
			return;
		}

		runner.SetDestination(RZ_Util.GroundPos(sc.m_vFall + sc.m_vOut * (RUN_ON - FALL_IN)));
		runner.Go();
		sc.m_Runner = runner;
		RZ_Log.Info(string.Format("Openings: the line for %1: a runner breaks from cover at %2", RZ_Util.PlayerName(sc.m_iPlayer), RZ_SettlementScanner.GridReference(sc.m_vBreak)));
	}

	//------------------------------------------------------------------------------------------------
	protected static void TickRunner(RZ_GameModeComponent gm, RZ_LineScene sc, RZ_Perimeter perimeter, float dt, array<int> ids, array<IEntity> players)
	{
		if (!sc.m_Runner)
			return;

		sc.m_Runner.Update(gm, dt, ids, players);
		IEntity person = sc.m_Runner.GetPerson();
		if (!person || !RZ_Util.IsCharacterAlive(person))
		{
			// Never came out, or something else got him first: he's found where he'd have fallen
			if (sc.m_fAt >= BODY_WAIT || sc.m_Runner.IsDead())
			{
				sc.m_Runner.Cleanup();
				sc.m_Runner = null;
				LayDown(gm, sc, "no body in time");
			}

			return;
		}

		// VERIFY: a civilian AI on a move waypoint sprints when its movement type is asked for (the zombies' behaviour layer does the same)
		RZ_Behaviour.WantMove(person, EMovementType.SPRINT);
		vector pos = person.GetOrigin();
		KeepDroneOver(perimeter, sc.m_Drone, pos);
		float warn = perimeter.GetWarnDistance();
		float edge = perimeter.EdgeDistance(pos);
		if (!sc.m_bWarned && edge < warn)
		{
			sc.m_bWarned = true;
			gm.Say(sc.m_iPlayer, RZ_Perimeter.LOUDSPEAKER, -1, "You on the road. Stop where you are. Stop now.");
		}

		if (edge <= warn - FALL_IN)
		{
			Shoot(gm, sc, perimeter, person);
			return;
		}

		// Stuck on the way: shot where he is (he's near enough the line to be in their sights)
		if (sc.m_fAt >= RUN_STALL)
		{
			sc.m_bStalled = true;
			Shoot(gm, sc, perimeter, person);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Perimeter's own shot (its effect and the kill), at him only.
	protected static void Shoot(RZ_GameModeComponent gm, RZ_LineScene sc, RZ_Perimeter perimeter, IEntity person)
	{
		vector pos = person.GetOrigin();
		perimeter.SpawnEffect(pos);
		RZ_Util.KillCharacter(person);
		RZ_OpeningTasks.KeepBody(person, true);
		sc.m_Body = person;
		sc.m_fShotDepth = perimeter.GetWarnDistance() - perimeter.EdgeDistance(pos);
		sc.m_Runner.Release();
		sc.m_Runner = null;
		SetPhase(sc, RZ_LineScene.DOWN);
		RZ_Log.Info(string.Format("Openings: the line for %1: the runner was shot at %2, %3 m inside the warning zone%4", RZ_Util.PlayerName(sc.m_iPlayer), RZ_SettlementScanner.GridReference(pos), RZ_Util.RoundToInt(sc.m_fShotDepth), StalledText(sc)));
	}

	//------------------------------------------------------------------------------------------------
	protected static string StalledText(RZ_LineScene sc)
	{
		if (sc.m_bStalled)
			return " (stalled on the way)";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! No runner could run: a dead man where he'd have fallen (Puzzles > Civilian), as the case's
	//! own runner is put out, and the shot heard.
	protected static void LayDown(RZ_GameModeComponent gm, RZ_LineScene sc, string why)
	{
		sc.m_bStalled = true;
		RZ_Perimeter perimeter = gm.GetPerimeter();
		vector at = RZ_Util.GroundPos(sc.m_vFall);
		IEntity body = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN), at + Vector(0, 0.1, 0), Math.RandomFloat(0, 360));
		if (body)
		{
			RZ_Util.KillCharacter(body);
			RZ_OpeningTasks.KeepBody(body, true);
		}

		if (perimeter)
		{
			perimeter.SpawnEffect(at);
			sc.m_fShotDepth = perimeter.GetWarnDistance() - perimeter.EdgeDistance(at);
		}

		sc.m_Body = body;
		SetPhase(sc, RZ_LineScene.DOWN);
		RZ_Log.Info(string.Format("Openings: the line for %1: the runner laid down at %2 (%3)", RZ_Util.PlayerName(sc.m_iPlayer), RZ_SettlementScanner.GridReference(at), why));
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity SpawnDrone(RZ_Perimeter perimeter, vector over)
	{
		if (!perimeter || perimeter.GetDronePrefab().IsEmpty())
			return null;

		return RZ_Util.SpawnPrefab(perimeter.GetDronePrefab(), RZ_Util.GroundPos(over) + Vector(0, perimeter.GetDroneHeight(), 0));
	}

	//------------------------------------------------------------------------------------------------
	protected static void KeepDroneOver(RZ_Perimeter perimeter, IEntity drone, vector pos)
	{
		if (!drone || !perimeter)
			return;

		vector want = pos + Vector(0, perimeter.GetDroneHeight(), 0);
		if (RZ_Util.DistXZ(drone.GetOrigin(), want) > 10)
			drone.SetOrigin(want); // VERIFY: SetOrigin on a spawned prop also moves it for clients (as the Perimeter's own drone)
	}

	//------------------------------------------------------------------------------------------------
	//! The scene's leftovers when the opening ends: the drone, a runner still standing. His body
	//! stays: the opening keeps it (m_Remains) until it ends.
	static void Cleanup(RZ_LineScene sc)
	{
		if (!sc)
			return;

		RZ_Util.DeleteUnit(sc.m_Drone);
		sc.m_Drone = null;
		if (sc.m_Runner)
		{
			sc.m_Runner.Cleanup();
			sc.m_Runner = null;
		}
	}
}
