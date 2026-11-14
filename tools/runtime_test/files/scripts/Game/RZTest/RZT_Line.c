// Test-only (not part of the mod): the opening's line (Guidance/RZ_OpeningLine.c, docs/DESIGN.md
// "Openings", the line), on the fake player, after the case's check:
// - A road above the line is found near some town: 150-250 m outside the warning zone, 600 m to
//   2 km from the town, and where the runner would fall is inside the warning zone, not the kill zone.
//   None near any town: every start takes the short form (said, and the rest skipped).
// - The scene runs with the tester standing at the viewpoint: the loudspeakers, a runner who breaks
//   from cover (a body, or laid down when none could run), shot by the line in the warning zone.
// - He lies dead there; the line is said; the case can lie by him.
// - The tester is never shot and never in the line's zones while he watches.
// Lines: "RZT line OK|FAIL <what>", "RZT line: ..." and "RZT line: N checks, M failed".

class RZT_Line
{
	static bool s_bDone;
	static int s_iSec = -1;
	static int s_iChecks;
	static int s_iFails;
	static ref RZ_LineScene s_Scene;
	static bool s_bRunnerSeen;
	static bool s_bInZone;
	static string s_sStandIn;
	static const string STAND_IN = "{6E725D44CA973C24}Prefabs/Groups/INDFOR/Group_FIA_SentryTeam.et";

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT line %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static void End()
	{
		if (s_Scene)
		{
			RZ_OpeningLine.Cleanup(s_Scene);
			RZ_OpeningTasks.KeepBody(s_Scene.m_Body, false);
		}

		Print(string.Format("RZT line: %1 checks, %2 failed", s_iChecks, s_iFails));
		s_bDone = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from RZT_Tick until done. False while it runs.
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		s_iSec++;
		RZ_Perimeter perimeter = gm.GetPerimeter();
		IEntity me = RZT_Fake.s_Ent;
		if (!perimeter || !perimeter.IsActive() || !me)
		{
			Print("RZT line: skipped (the line is off, or no tester)");
			s_bDone = true;
			return true;
		}

		float warn = perimeter.GetWarnDistance();
		float kill = perimeter.GetKillDistance();
		if (s_iSec == 0)
		{
			RZ_Settlement from;
			vector view;
			vector breakAt;
			vector fall;
			vector away;
			foreach (RZ_Settlement s : gm.GetSettlements())
			{
				if (!s || s.m_bIsland)
					continue;

				if (RZ_OpeningLine.FindViewpoint(gm, s.m_vCenter, view, breakAt, fall, away))
				{
					from = s;
					break;
				}
			}

			if (!from)
			{
				Print("RZT line: no road above the line within 2 km of any town: every start takes the short form here (the last: " + RZ_OpeningLine.s_sWhy + ")");
				End();
				return true;
			}

			float viewEdge = perimeter.EdgeDistance(view);
			float fallEdge = perimeter.EdgeDistance(fall);
			float d = RZ_Util.DistXZ(from.m_vCenter, view);
			Check(viewEdge >= warn + 149 && viewEdge <= warn + 251, string.Format("the road above the line near %1 at %2: %3 m outside the warning zone", from.m_sName, RZ_SettlementScanner.GridReference(view), RZ_Util.RoundToInt(viewEdge - warn)));
			Check(d >= 599 && d <= 2001, string.Format("%1 m from the town", RZ_Util.RoundToInt(d)));
			Check(fallEdge < warn && fallEdge > kill, string.Format("where he'd fall is in the warning zone, not the kill zone (%1 m from the edge; warning %2, kill %3)", RZ_Util.RoundToInt(fallEdge), RZ_Util.RoundToInt(warn), RZ_Util.RoundToInt(kill)));

			s_Scene = new RZ_LineScene();
			s_Scene.m_vView = view;
			s_Scene.m_vBreak = breakAt;
			s_Scene.m_vFall = fall;
			s_Scene.m_vOut = away;
			s_Scene.m_iPlayer = RZT_Fake.ID;
			s_Scene.m_iPhase = RZ_LineScene.WAIT;
			RZT_Fake.MoveTo(view);
			return false;
		}

		RZT_Fake.MoveTo(s_Scene.m_vView);
		if (perimeter.EdgeDistance(me.GetOrigin()) < warn)
			s_bInZone = true;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		ResourceName prefab;
		if (gm.GetGuidance() && gm.GetGuidance().GetSettings())
			prefab = gm.GetGuidance().GetSettings().m_sCompanionPrefab;

		if (s_iSec == 1)
		{
			// Can this server put the runner's prefab out at all (a text prefab may not be registered here)?
			IEntity probe = RZ_Util.SpawnPrefab(prefab, s_Scene.m_vBreak + Vector(0, 0.2, 0), 0);
			Print(string.Format("RZT line: the runner's prefab %1 spawns here: %2", prefab, probe != null));
			if (probe)
				SCR_EntityHelper.DeleteEntityAndChildren(probe);
			else
				s_sStandIn = STAND_IN;
		}

		// The companion is a text prefab, unregistered on this server: a base-game group runs in his
		// place so the run, the sprint and the shot are seen (in play it's Guidance > Companion)
		if (!s_sStandIn.IsEmpty())
			prefab = s_sStandIn;

		RZ_OpeningLine.Tick(gm, s_Scene, null, "Pavel Test", prefab, 1, ids, players);
		if (s_Scene.m_Runner && s_Scene.m_iPhase == RZ_LineScene.RUN && s_iSec % 5 == 0)
			Print(string.Format("RZT line: the runner %1 s in: state %2, a body %3, a person %4, at %5", s_Scene.m_fAt, typename.EnumToString(RZ_EEscortState, s_Scene.m_Runner.m_eState), s_Scene.m_Runner.HasBody(), s_Scene.m_Runner.GetPerson() != null, RZ_SettlementScanner.GridReference(s_Scene.m_Runner.NowAt())));
		if (s_Scene.m_Runner && s_Scene.m_Runner.GetPerson())
			s_bRunnerSeen = true;

		if (s_Scene.m_iPhase != RZ_LineScene.OVER && s_iSec < 150)
			return false;

		Check(s_Scene.m_iPhase == RZ_LineScene.OVER, "the scene played to its end");
		Check(SpeakersSaid(), "the loudspeakers spoke");
		if (s_bRunnerSeen)
			Check(true, "a runner broke from cover (a body ran)");
		else
			Print("RZT line: no runner could run here (no prefab, no escort room, or no body in time): he was laid down where he'd have fallen");

		IEntity body = s_Scene.m_Body;
		Check(body != null && !RZ_Util.IsCharacterAlive(body), "he lies dead");
		if (body)
		{
			float edge = perimeter.EdgeDistance(body.GetOrigin());
			Check(edge < warn && edge > kill, string.Format("shot in the warning zone: %1 m from the edge (warning %2, kill %3)%4", RZ_Util.RoundToInt(edge), RZ_Util.RoundToInt(warn), RZ_Util.RoundToInt(kill), Stalled()));
			RZ_OpeningCase c = RZ_OpeningCase.Get();
			if (c && !RZ_OpeningCase.CasePrefab(gm).IsEmpty())
			{
				c.SpawnCaseBy(body.GetOrigin(), RZT_Fake.ID);
				RZ_PlaceThing thing = new RZ_PlaceThing();
				thing.m_eKind = RZ_EThingKind.ITEM;
				thing.m_sSlot = "case";
				thing.m_fRange = 4;
				Check(RZ_PlaceThings.Near(body.GetOrigin(), thing), "the case lies by him");
			}
		}

		Check(s_Scene.m_bSaid, "the line was said: " + RZ_OpeningLine.LINE);
		Check(RZ_Util.IsCharacterAlive(me), "the tester is alive (never the target)");
		Check(!s_bInZone, "the tester was never in the line's zones while he watched");
		End();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SpeakersSaid()
	{
		return s_Scene && s_Scene.m_bSpeakers;
	}

	//------------------------------------------------------------------------------------------------
	static string Stalled()
	{
		if (s_Scene && s_Scene.m_bStalled)
			return " (stalled or laid down)";

		return "";
	}
}
