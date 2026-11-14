// Test-only (not part of the mod): road patrols (docs/SITES.md, "Road patrols"). Early in the run,
// before the missions move the tester: (1) a foot patrol of each faction with a holding sets out on
// the real roads, 1-3 km out and back the same way; (2) far from the tester it's data, within the body
// radius its people are put out and its leader tagged to talk to; (3) it meets the tester by standing:
// friendly when Known, a stranger asked for papers (Bastion), hostile when Hunted; (4) only one vehicle
// patrol at a time; (5) its fight leaves spent cases (an L9 trace); (6) a Bastion patrol lost on the
// road starts a Missing Patrol there. Lines: "RZT roadpatrols OK|FAIL ...", "RZT roadpatrols: ...".

class RZT_RoadPatrols
{
	static int s_iStep;
	static int s_iWait;
	static int s_iChecks;
	static int s_iFails;
	static bool s_bDone;
	static int s_iPatrol = -1;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (ok)
		{
			Print("RZT roadpatrols OK " + what);
			return;
		}

		s_iFails++;
		Print("RZT roadpatrols FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	static void End(RZ_RoadPatrols rp)
	{
		if (rp)
			rp.TestClear();

		Print(string.Format("RZT roadpatrols: %1 checks, %2 failed", s_iChecks, s_iFails));
		s_bDone = true;
	}

	//------------------------------------------------------------------------------------------------
	static float Length(RZ_RoadPatrol p, int upTo)
	{
		float total = 0;
		for (int i = 1; i <= upTo && i < p.m_aRoute.Count(); i++)
		{
			total += RZ_Util.DistXZ(p.m_aRoute[i - 1], p.m_aRoute[i]);
		}

		return total;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from the test's tick. True when it's finished (or can't run).
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		RZ_RoadPatrols rp = gm.GetRoadPatrols();
		if (!rp || !rp.IsOn())
		{
			Print("RZT roadpatrols: road patrols are off on this world (no territory or no road network); not tried");
			s_bDone = true;
			return true;
		}

		IEntity me = RZT_Fake.s_Ent;
		if (!me || !RZ_Util.IsCharacterAlive(me))
			return false;

		int id = RZT_Fake.ID;
		vector here = me.GetOrigin();
		if (s_iStep == 0)
		{
			rp.TestClear();
			int made = 0;
			float bestDist = 1000000;
			for (int f = 0; f < 4; f++)
			{
				RZ_RoadPatrol p = rp.TestSetOut(f, here, false);
				if (!p)
				{
					Print(string.Format("RZT roadpatrols: %1 sets out nobody (no holding, or no road %2 m long by it)", RZ_Factions.GetShortName(f), 1000));
					continue;
				}

				made++;
				float outLen = Length(p, p.m_iTurn);
				Check(outLen >= 950 && outLen <= 3300, string.Format("%1 walks %2 m out along the roads (%3 points) and back", p.Title(), RZ_Util.RoundToInt(outLen), p.m_aRoute.Count()));
				Check(RZ_Util.DistXZ(p.m_aRoute[0], p.m_aRoute[p.m_aRoute.Count() - 1]) < 1, string.Format("%1 ends where it started", p.Title()));
				bool dry = true;
				foreach (vector pt : p.m_aRoute)
				{
					if (RZ_Util.IsWater(pt) || RZ_Perimeter.IsNearLineAt(pt))
						dry = false;
				}

				Check(dry, string.Format("%1's road stays dry and off the line", p.Title()));
				float d = RZ_Util.DistXZ(p.m_aRoute[0], here);
				if (d < bestDist)
				{
					bestDist = d;
					s_iPatrol = p.m_iId;
				}
			}

			Check(made >= 1, string.Format("%1 foot patrols set out", made));
			if (s_iPatrol < 0)
			{
				End(rp);
				return true;
			}

			// Far away: it stays data
			RZ_RoadPatrol far = rp.GetPatrol(s_iPatrol);
			far.m_vPos = here + Vector(2500, 0, 0);
			s_iStep = 1;
			s_iWait = 0;
			return false;
		}

		RZ_RoadPatrol p2 = rp.GetPatrol(s_iPatrol);
		if (!p2)
		{
			Check(false, "the patrol under test is still out");
			End(rp);
			return true;
		}

		if (s_iStep == 1)
		{
			s_iWait++;
			if (s_iWait < 7)
				return false;

			Check(!p2.m_bHasBody, string.Format("%1 2.5 km off is data, nobody put out", p2.Title()));
			p2.m_vPos = RZ_Mission.LandPointNear(here, 40, 70);
			s_iStep = 2;
			s_iWait = 0;
			return false;
		}

		if (s_iStep == 2)
		{
			s_iWait++;
			if (!p2.m_bTagged && s_iWait < 40)
				return false;

			Check(p2.m_bHasBody && p2.m_Group != null, string.Format("%1 put out within %2 m of the tester", p2.Title(), 400));
			Check(p2.m_bTagged, string.Format("%1's leader can be talked to", p2.Title()));

			// Its meeting by standing (the tester's standing set, then put back)
			RZ_PlayerProfile tester = gm.GetProfile(id);
			int fame = tester.m_aFame[p2.m_eFaction];
			int infamy = tester.m_aInfamy[p2.m_eFaction];
			tester.m_aFame[p2.m_eFaction] = 0;
			tester.m_aInfamy[p2.m_eFaction] = 0;
			string stranger = rp.TestStance(p2.m_iId, id);
			array<int> ids = {};
			array<string> labels = {};
			array<string> tags = {};
			array<string> needs = {};
			rp.Replies(p2.m_iId, id, ids, labels, tags, needs);
			Check(stranger == "stranger", string.Format("unknown to %1, the tester is a stranger (%2)", p2.Title(), stranger));
			if (p2.m_eFaction == RZ_EFaction.MILITARY)
			{
				Check(ids.Contains(RZ_RoadPatrols.REPLY_PAPERS), "Bastion asks a stranger for papers");
				string said = rp.Pick(p2.m_iId, id, RZ_RoadPatrols.REPLY_PAPERS, me);
				Print("RZT roadpatrols: papers: " + said);
				Check(p2.m_aCleared.Contains(id), "the papers shown clear the tester");
			}
			else if (p2.m_eFaction == RZ_EFaction.BANDITS)
			{
				Check(ids.Contains(RZ_RoadPatrols.REPLY_TOLL), "the Red Hand asks a stranger for the road's toll");
			}

			tester.m_aFame[p2.m_eFaction] = 500;
			string friendly = rp.TestStance(p2.m_iId, id);
			Check(friendly == "friendly", string.Format("Known to %1, the tester is a friend (%2)", p2.Title(), friendly));
			ids.Clear();
			labels.Clear();
			tags.Clear();
			needs.Clear();
			rp.Replies(p2.m_iId, id, ids, labels, tags, needs);
			Check(ids.Contains(RZ_RoadPatrols.REPLY_NEWS) && ids.Contains(RZ_RoadPatrols.REPLY_WALK), "a friend gets road news and a stretch to walk");
			Print("RZT roadpatrols: news: " + rp.Pick(p2.m_iId, id, RZ_RoadPatrols.REPLY_NEWS, me));

			tester.m_aFame[p2.m_eFaction] = 0;
			tester.m_aInfamy[p2.m_eFaction] = 2000;
			string hostile = rp.TestStance(p2.m_iId, id);
			Check(hostile == "hostile (they fire)", string.Format("Hunted by %1, the tester is fired on (%2)", p2.Title(), hostile));
			tester.m_aFame[p2.m_eFaction] = fame;
			tester.m_aInfamy[p2.m_eFaction] = infamy;

			// The fight's trace: spent cases on the verge
			rp.TestFight(p2.m_iId);
			vector at;
			string key;
			RZ_Traces traces = gm.GetTraces();
			bool cases = traces && traces.GetScenes() && traces.GetScenes().SceneNear("L9", p2.m_vPos, 40, at, key);
			if (traces && traces.IsOn())
				Check(cases, "its fight leaves spent cases on the verge (L9)");
			else
				Print("RZT roadpatrols: traces are off; the fight's trace not tried");

			// One vehicle at a time
			RZ_RoadPatrol jeep = rp.TestSetOut(RZ_EFaction.MILITARY, here, true);
			if (!jeep)
				jeep = rp.TestSetOut(RZ_EFaction.BANDITS, here, true);

			if (jeep)
			{
				Print(string.Format("RZT roadpatrols: %1 out of %2 (%3 points)", jeep.Title(), jeep.m_sFrom, jeep.m_aRoute.Count()));
				Check(!rp.TestVehicleRoom(), "with a vehicle patrol out, no second one sets out");
			}
			else
			{
				Print("RZT roadpatrols: no vehicle patrol could set out here (no road long enough by a Bastion or Red Hand holding); the cap not tried");
			}

			// A Bastion patrol lost on the road: a Missing Patrol there
			RZ_RoadPatrol lost = rp.TestSetOut(RZ_EFaction.MILITARY, here, false);
			if (lost)
			{
				bool started = rp.TestLose(lost.m_iId);
				Check(started, "a Bastion patrol lost on the road starts a Missing Patrol there");
			}

			End(rp);
			return true;
		}

		return false;
	}
}
