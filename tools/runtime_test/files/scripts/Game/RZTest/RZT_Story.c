// Test-only (not part of the mod): the mission style's and errands' data checks (the design's "log
// lines and tests"), run once, 150 s into the test, from the errands' tick (no change to RZT_Diag.c).
// Each is a pure-data path the play can't be relied on to reach in 90 minutes: a truth kept for its
// subject, the check results, a record written and read, a callback queued, a fallout dealt to its
// participant, a lie told and found by its owner's tick, a talk scene's node limits, mission goods'
// tag, the reasons the story jobs and majors answer, a route asset never refusing a free player, the
// door mark, and the dealer's context cache. Not checked here (needs play): a truth kept after a
// restart, an objection to a hold, a reason starting a contract, a locked player refused.
// Lines: "RZT story check: <name>: OK|FAILED (...)" and "RZT story done: N checks OK, N FAILED".

modded class RZ_ErrandsDealer
{
	override void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		super.Tick(dt, ids, players);
		RZT_Story.Tick(m_Dealer.GetGM(), dt);
	}
}

class RZT_Story
{
	protected static float s_fClock;
	protected static bool s_bDone;
	protected static int s_iOk;
	protected static int s_iFailed;

	//------------------------------------------------------------------------------------------------
	static void Tick(RZ_GameModeComponent gm, float dt)
	{
		if (s_bDone || !gm)
			return;

		s_fClock += dt;
		if (s_fClock < 150 || !RZT_Fake.s_Ent)
			return;

		s_bDone = true;
		Run(gm);
		Print(string.Format("RZT story done: %1 checks OK, %2 FAILED", s_iOk, s_iFailed));
	}

	//------------------------------------------------------------------------------------------------
	protected static void Check(string name, bool ok, string detail)
	{
		if (ok)
		{
			s_iOk++;
			Print(string.Format("RZT story check: %1: OK", name));
			return;
		}

		s_iFailed++;
		Print(string.Format("RZT story check: %1: FAILED (%2)", name, detail));
	}

	//------------------------------------------------------------------------------------------------
	protected static void Run(RZ_GameModeComponent gm)
	{
		int me = RZT_Fake.ID;
		vector pos = RZT_Fake.s_Ent.GetOrigin();
		array<int> who = {me};

		// A truth is kept for its subject: drawn twice, the same
		RZ_MissionStory st = new RZ_MissionStory("rzt");
		array<string> values = {"said", "hidden"};
		array<float> w1 = {0.5, 0.5};
		array<float> w2 = {0.5, 0.5};
		string t1 = st.Truth("rzt:subject", values, w1);
		string t2 = st.Truth("rzt:subject", values, w2);
		Check("truth kept for its subject", t1 == t2 && !t1.IsEmpty(), t1 + " then " + t2);

		// Each check result
		bool results = RZ_Checks.Result(3, 2) == RZ_ECheck.PASS && RZ_Checks.Result(1, 2) == RZ_ECheck.SHORT && RZ_Checks.Result(0, 2) == RZ_ECheck.REFUSED;
		Check("check results (pass, short, refused)", results, "");

		// A record written and read
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		mem.Record("rzt.record", "rzt", "written", RZ_WorldMemory.SOURCE_STORY, false, who);
		RZ_WorldRecord r = mem.FindRecord("rzt.record", "rzt");
		Check("record written and read", r && r.value == "written", "");

		// A callback queued for the character, and the door mark
		RZ_Shared shared = gm.GetShared();
		bool queued = shared && shared.GetCallbacks() && shared.GetCallbacks().Queue(me, "RZT", "A line some hours later.", pos);
		Check("callback queued", queued, "");
		string plain;
		Check("door callback mark", RZ_DoorCallers.Strip(RZ_DoorCallers.MARK + "At the house", plain) && plain == "At the house", plain);

		// A fallout errand dealt to its participant first
		RZ_Errands errands = gm.GetErrands();
		if (errands)
		{
			errands.Fallout("name_on_tag", pos, who, 5, "RZT", "someone from the test");
			Check("fallout errand for its participant", errands.FalloutFor("name_on_tag", pos, me) != null, "");
			Check("the dealer's context cache", errands.Dealer().Context(me) != null, "no context for the fake player");
		}

		// A lie told, and found by its owner's tick
		RZ_Lies lies = mem.Lies();
		lies.Tell(gm, me, "rzt.lie", "rzt", RZ_EFaction.SURVIVORS, "", pos, RZ_ELieTest.TICK, 2, "Found out, as tested.");
		Check("lie found by the owner's tick", lies.Test(gm, "rzt.lie", "rzt") > 0, "");

		// A talk scene: up to three options a node, the fourth slot always Leave it
		RZ_TalkScene sc = new RZ_TalkScene("Tester", "rzt.person", pos);
		int n0 = sc.Node("A line.");
		sc.Ask(n0, "Ask", -1, "An answer.");
		sc.Way(n0, "Pay", RZ_TalkScene.FOOD, -1, 2, 0, "Paid.");
		sc.Decide(n0, "Decide", "decide it", 1, "Decided.");
		Check("talk scene: three options a node", sc.Ask(n0, "A fourth", -1, "") == null, "");

		// Mission goods are someone's
		RZ_MissionGoods.Tag(RZT_Fake.s_Ent, "Levie");
		Check("mission goods tagged", RZ_MissionGoods.IsGoods(RZT_Fake.s_Ent) && RZ_MissionGoods.Owner(RZT_Fake.s_Ent) == "Levie", "");

		// The reasons the story jobs and the majors answer
		bool reasons = RZ_MissionReasons.Answers(RZ_Mission_SupplyCache, "house.hungry") && RZ_MissionReasons.Answers(RZ_Mission_ResearchData, "major.proof") && RZ_MissionReasons.Answers(RZ_Mission_MissingPatrol, "major.orders");
		Check("story jobs answer their reasons", reasons, "");

		// Work sites: a stand-in site prefab (the crate: no prompt, the server's path only), a site
		// opened for the fake player and done for it (it can't hold a key), its after look up near it
		RZ_WorkSiteSettings ws = RZ_SharedSettings.Get().WorkSites();
		if (ws.m_sSitePrefab.IsEmpty())
			ws.m_sSitePrefab = "{7030822F7A54F479}Prefabs/Props/Crates/CrateWooden_01_base.et";
		RZ_WorkSites sites = RZ_WorkSites.Get();
		bool opened = sites && RZ_WorkSites.Available() && sites.Open("rzt.work", RZ_EWork.LATRINE, "Dig the latrine", 15, pos + Vector(4, 0, 0), me, "", "{7030822F7A54F479}Prefabs/Props/Crates/CrateWooden_01_base.et") >= 0;
		Check("work site opened", opened && sites.Has("rzt.work") && !sites.IsDone("rzt.work"), "");
		bool worked = opened && sites.CompleteFor("rzt.work", me) && sites.IsDone("rzt.work") && sites.DoneBy("rzt.work") == me;
		Check("work site done for its player, its after look up", worked && sites.PropOf("rzt.work") != null, "");
		if (sites)
			sites.Close("rzt.work");
		string cut = RZ_WorkSites.WithoutTime("Dig it (30 s there), then go.");
		string kept = RZ_WorkSites.WithoutTime("Walk up (20 s there, nothing in your hands).");
		Check("work site: a held step's line loses its time", cut == "Dig it, then go." && kept == "Walk up (nothing in your hands).", cut + " / " + kept);

		// A route asset never refuses a player locked to nothing
		Check("route assets: a free player isn't refused", RZ_RouteAssets.Refuse(gm, me, RZ_EFaction.MILITARY).IsEmpty(), "");
	}
}
