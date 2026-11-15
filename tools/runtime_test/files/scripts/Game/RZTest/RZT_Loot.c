// Test-only (not part of the mod): the loot pool (docs/DESIGN.md, "Loot in houses": building kinds,
// groups and mixes, medicine in two levels, caps on the rare things, rare places, near the line).
// Half a minute in, alongside the rest (it never moves the tester):
//   (1) the kinds by name: Everon's own prefab names (and made-up ones for kinds Everon hasn't)
//       come out as their kind, and a cottage on a pier is a room, not a pier;
//   (2) the kinds in the world: the buildings within 1.5 km of the tester, counted by kind;
//   (3) every kind has a table, and each new kind's rolls come from its own table or the shared
//       food, drink and low medicine (or the Long Table's jar, near its farm);
//   (4) medicine in two levels: an open pharmacy, a house and an army post never roll the high level
//       (morphine, saline, first-aid kits); a hospital does;
//   (5) the caps: rifles held as data up to the cap, and an army post then rolls no rifle;
//   (6) near the line: rare finds x1.5 just inside the Perimeter's edge, x1 deep inside; a boot
//       by a sealed checkpoint is on the evacuation road;
//   (7) a downed drone: its spot is inside the Zone past the warning zone and within reach of the
//       line; one put 20 m from the tester comes out as data, then its wreck and its things.
// Lines: "RZT loot OK|FAIL ...", "RZT loot kinds ...", "RZT loot: N checks, N failed".

class RZT_Loot
{
	static const int START = 30;		// seconds in
	static const ResourceName DRONE_WRECK = "{1421CA58F1A924A6}Prefabs/ReforgerZ/Missions/RZ_DroneWreck.et";
	static int s_iStep;
	static int s_iWait;
	static int s_iChecks;
	static int s_iFails;
	static bool s_bDone;
	static string s_sDrone;
	static vector s_vDrone;
	static int s_iMoves;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (ok)
		{
			Print("RZT loot OK " + what);
			return;
		}

		s_iFails++;
		Print("RZT loot FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from the test's tick, from START on.
	static void Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return;

		RZ_Interactions hub = RZ_Interactions.Get();
		IEntity me = RZT_Fake.s_Ent;
		if (!hub || !hub.GetLoot() || !hub.GetSearch() || !me)
			return;

		RZ_Loot loot = hub.GetLoot();
		RZ_Search search = hub.GetSearch();
		if (s_iStep == 0)
		{
			Names(search);
			World(search, me.GetOrigin());
			Tables(search, loot, me.GetOrigin());
			Medicine(loot, me.GetOrigin());
			Caps(loot, me.GetOrigin());
			Line(gm, search);
			s_iStep = 1;
			// The test world leaves the crashed drone's slot empty: the mod's own wreck stands in
			RZ_ContractSettings cs = gm.GetContractSettings();
			if (cs && cs.m_sDroneWreckPrefab.IsEmpty())
				cs.m_sDroneWreckPrefab = DRONE_WRECK;

			if (!loot.DronesOn())
			{
				Print("RZT loot: downed drones are off here (the Perimeter, or no Crashed drone prefab); the drone's spot and put-out not tried");
				Finish();
				return;
			}

			vector spot;
			bool found = loot.FindDroneSpot(spot);
			RZ_Perimeter perimeter = gm.GetPerimeter();
			if (found && perimeter)
			{
				float edge = perimeter.EdgeDistance(spot);
				Check(edge > perimeter.GetWarnDistance() && edge <= 1100, string.Format("a downed drone's spot is inside the Zone past the warning zone, near the line: %1 m in (warning zone %2 m)", RZ_Util.RoundToInt(edge), RZ_Util.RoundToInt(perimeter.GetWarnDistance())));
			}
			else
			{
				Print("RZT loot: no dry mainland near the line found for a drone in 12 tries (the test world is small); spot not checked");
			}

			s_vDrone = me.GetOrigin() + Vector(20, 0, 5);
			s_sDrone = loot.AddDrone(s_vDrone);
			int dataItems;
			Check(loot.TestState(s_sDrone, dataItems) == "data" || loot.TestState(s_sDrone, dataItems) == "out", "a downed drone is a record (data until a player is near)");
			return;
		}

		if (s_iStep == 1)
		{
			// The loot tick (5 s) puts it out with the tester 20 m off; if something else walked him
			// away meanwhile, it's laid by him again (three times at most)
			if (RZ_Util.DistXZ(me.GetOrigin(), s_vDrone) > 100 && s_iMoves < 3)
			{
				s_iMoves++;
				loot.TestForget(s_sDrone);
				s_vDrone = me.GetOrigin() + Vector(20, 0, 5);
				s_sDrone = loot.AddDrone(s_vDrone);
				s_iWait = 0;
				return;
			}

			s_iWait++;
			if (s_iWait < 12)
				return;

			int items;
			string state = loot.TestState(s_sDrone, items);
			Check(state == "out", "the downed drone 20 m off is put out: " + state);
			Check(items > 0, string.Format("it has things beside it: %1", items));
			Check(loot.TestProp(s_sDrone) != null, "its wreck stands there (Missions > Contracts > Crashed drone)");
			loot.TestForget(s_sDrone);
			Check(loot.TestState(s_sDrone, items) == "", "forgotten, the drone and its things are gone");
			Finish();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Finish()
	{
		s_bDone = true;
		Print(string.Format("RZT loot: %1 checks, %2 failed", s_iChecks, s_iFails));
	}

	//------------------------------------------------------------------------------------------------
	//! (1) The kinds by name.
	protected static void Names(RZ_Search search)
	{
		array<string> names = {
			"prefabs/structures/military/houses/barracks_01/barracks_01_medical_white.et", "hospital",
			"prefabs/structures/military/houses/barracks_01/barracks_01_military_camo.et", "army",
			"prefabs/structures/military/houses/barrackswooden_01/barrackswooden_ussr_01_messhall_red.et", "army",
			"prefabs/structures/military/bunkers/bunker_01/bunker_01.et", "army",
			"prefabs/structures/cultural/churches/church_01/church_01_blue.et", "church",
			"prefabs/structures/cultural/chapels/chapel_01/chapel_01_pink.et", "church",
			"prefabs/structures/industrial/houses/office_e_01/office_e_01.et", "office",
			"prefabs/structures/industrial/houses/warehouse_01/warehouse_01_office.et", "office",
			"prefabs/structures/industrial/houses/warehouse_01/warehouse_01.et", "industrial",
			"prefabs/structures/industrial/houses/sawmill_e_01/sawmill_e_01.et", "industrial",
			"prefabs/structures/industrial/houses/factoryhall_e_01/factoryhall_e_01.et", "industrial",
			"prefabs/structures/infrastructure/naval/sheds/fishermanshed_01/fishermanshed_01_v1.et", "coast",
			"prefabs/structures/infrastructure/naval/sheds/boatshed_01/boatshed_01.et", "coast",
			"prefabs/structures/infrastructure/naval/houses/cottagepier_01/cottagepier_01.et", "coast",
			"prefabs/structures/houses/villa/villa_e_2i01/villa_e_2i01_police.et", "police",
			"prefabs/structures/industrial/garages/garage_e_02/garage_e_02.et", "garage",
			"prefabs/structures/commercial/shops/shophouse_e_2l01/shophouse_e_2i01t_drugstore.et", "pharmacy",
			"prefabs/structures/commercial/shops/shophouse_e_2l01/shophouse_e_2i01t_foodstore.et", "shop",
			"prefabs/structures/agriculture/barn/barn_e_01/barn_e_01.et", "farm",
			"prefabs/structures/houses/village/house_village_e_1i01/house_village_e_1i01.et", "house",
			"prefabs/test/school_01.et", "school",
			"prefabs/test/firestation_01.et", "firestation",
			"prefabs/test/prison_01.et", "prison",
			"prefabs/test/huntinglodge_01.et", "hunting",
			"prefabs/test/hospital_01.et", "hospital"
		};

		for (int i = 0; i + 1 < names.Count(); i += 2)
		{
			string got = search.BuildingType(names[i]);
			Check(got == names[i + 1], string.Format("%1 is a %2 (got %3)", ShortName(names[i]), names[i + 1], got));
		}

		Check(!search.IsNotBuilding("prefabs/structures/infrastructure/naval/houses/cottagepier_01/cottagepier_01.et"), "a cottage on a pier is a room, not a pier");
		Check(search.IsNotBuilding("prefabs/structures/infrastructure/naval/piers/pierconcrete_01_20m.et"), "a concrete pier is still no room");
	}

	//------------------------------------------------------------------------------------------------
	protected static string ShortName(string path)
	{
		int slash = path.LastIndexOf("/");
		if (slash < 0)
			return path;

		return path.Substring(slash + 1, path.Length() - slash - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Lower(string s)
	{
		string t = s;
		t.ToLower();
		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! (2) The kinds in the world round the tester.
	protected static void World(RZ_Search search, vector at)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(at, 1500, buildings, 400);
		map<string, int> counts = new map<string, int>();
		foreach (IEntity b : buildings)
		{
			if (!b)
				continue;

			string name = RZ_ItemHelper.LowerName(b);
			if (search.IsNotBuilding(name))
				continue;

			string kind = search.BuildingType(name);
			int n;
			counts.Find(kind, n);
			counts.Set(kind, n + 1);
		}

		string line = "";
		foreach (string k, int c : counts)
		{
			line += string.Format("%1 %2, ", k, c);
		}

		Print(string.Format("RZT loot kinds in the world (%1 buildings within 1.5 km): %2", buildings.Count(), line));
		Check(counts.Count() >= 2, string.Format("the buildings round the tester come out as %1 kinds", counts.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! (3) Every kind has a table; a new kind's rolls are its own or the shared food, drink and low medicine.
	protected static void Tables(RZ_Search search, RZ_Loot loot, vector at)
	{
		array<string> kinds = {};
		RZ_LootGroups.Kinds(kinds);
		foreach (string kind : kinds)
		{
			vector c = search.TableCounts(kind);
			Check(c[0] > 0, string.Format("the %1 table has everyday finds (%2 everyday, %3 rare, %4 electronics)", kind, c[0], c[1], c[2]));
		}

		Check(search.SharedCount(RZ_LootGroups.FOOD) >= 3 && search.SharedCount(RZ_LootGroups.DRINK) >= 2 && search.SharedCount(RZ_LootGroups.MED_LOW) >= 3,
			string.Format("the shared groups have items: %1 food, %2 drink, %3 low medicine", search.SharedCount(RZ_LootGroups.FOOD), search.SharedCount(RZ_LootGroups.DRINK), search.SharedCount(RZ_LootGroups.MED_LOW)));

		array<string> fresh = {"hospital", "firestation", "school", "office", "church", "industrial", "hunting", "coast", "prison", "army"};
		foreach (string f : fresh)
		{
			array<ResourceName> picks = {};
			loot.TestRoll(f, at, false, 24, picks);
			int own = 0;
			int shared = 0;
			string stray = "";
			foreach (ResourceName p : picks)
			{
				if (search.TableHas(f, p))
					own++;
				else if (RZ_LootGroups.IsShared(RZ_LootGroups.GroupOf(p)))
					shared++;
				else if (Lower(p).Contains("tablejar"))
					shared++;	// the Long Table's jar near its farm (RZ_LongTableReach.AddLoot): any building there
				else
					stray += ShortName(p) + " ";
			}

			Check(!picks.IsEmpty() && stray.IsEmpty(), string.Format("a %1 rolls %2 things, %3 its own, %4 shared food, drink or medicine; not its own: %5", f, picks.Count(), own, shared, stray));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! (4) Medicine in two levels.
	protected static void Medicine(RZ_Loot loot, vector at)
	{
		array<string> low = {"pharmacy", "house", "army"};
		foreach (string kind : low)
		{
			int high = 0;
			for (int n = 0; n < 10; n++)
			{
				array<ResourceName> picks = {};
				loot.TestRoll(kind, at, false, 20, picks);
				foreach (ResourceName p : picks)
				{
					if (RZ_LootGroups.IsMedHigh(p))
						high++;
				}
			}

			Check(high == 0, string.Format("an open %1 never rolls the high medicine (200 slots: %2)", kind, high));
		}

		int hospitalHigh = 0;
		for (int m = 0; m < 10; m++)
		{
			array<ResourceName> hospital = {};
			loot.TestRoll("hospital", at, false, 20, hospital);
			foreach (ResourceName h : hospital)
			{
				if (RZ_LootGroups.IsMedHigh(h))
					hospitalHigh++;
			}
		}

		Check(hospitalHigh > 0, string.Format("a hospital rolls the high medicine (200 slots: %1)", hospitalHigh));
	}

	//------------------------------------------------------------------------------------------------
	//! (5) The caps: rifles held as data up to the cap, and an army post then rolls none.
	protected static void Caps(RZ_Loot loot, vector at)
	{
		ResourceName rifle = "{3E413771E1834D2F}Prefabs/Weapons/Rifles/M16/Rifle_M16A2.et";
		int before = loot.CountCapped(RZ_LootGroups.CAP_RIFLE);
		array<ResourceName> held = {};
		for (int i = before; i < 4; i++)
		{
			held.Insert(rifle);
		}

		// Held far off (the map's corner), so nothing is put out by the tester
		string key = loot.TestHold(Vector(1, 0, 1), held);
		int capped = loot.CountCapped(RZ_LootGroups.CAP_RIFLE);
		Check(capped >= 4, string.Format("rifles held as data count against the cap: %1", capped));
		int rifles = 0;
		int rolled = 0;
		for (int n = 0; n < 20; n++)
		{
			array<ResourceName> picks = {};
			loot.TestRoll("army", at, true, 20, picks);
			rolled += picks.Count();
			foreach (ResourceName p : picks)
			{
				if (RZ_LootGroups.CapKind(p) == RZ_LootGroups.CAP_RIFLE)
					rifles++;
			}
		}

		Check(rifles == 0 && rolled > 0, string.Format("at the rifle cap an army post rolls no rifle (%1 things rolled, %2 rifles)", rolled, rifles));
		loot.TestForget(key);
		int after = loot.CountCapped(RZ_LootGroups.CAP_RIFLE);
		Check(after == before, string.Format("let go, the cap's count is back: %1 (was %2)", after, before));

		// Under the cap an army post does roll a rifle now and then (locked: twice the rare chance)
		if (after >= 4)
		{
			Print("RZT loot: the world's loot already holds the rifle cap; under the cap not tried");
			return;
		}

		int some = 0;
		for (int k = 0; k < 30 && some == 0; k++)
		{
			array<ResourceName> more = {};
			loot.TestRoll("army", at, true, 20, more);
			foreach (ResourceName q : more)
			{
				if (RZ_LootGroups.CapKind(q) == RZ_LootGroups.CAP_RIFLE)
					some++;
			}
		}

		Check(some > 0, "under the cap an army post rolls a rifle now and then");
	}

	//------------------------------------------------------------------------------------------------
	//! (6) Near the line and the evacuation road.
	protected static void Line(RZ_GameModeComponent gm, RZ_Search search)
	{
		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (!perimeter || !perimeter.IsActive() || !RZ_Features.On(RZ_EFeature.PERIMETER))
		{
			Print("RZT loot: the Perimeter is off here; near the line not tried");
			return;
		}

		vector near;
		if (perimeter.RandomNearLine(perimeter.GetWarnDistance() + 50, 600, near))
			Check(search.NearLineMult(near) > 1, string.Format("rare finds are better just inside the line (%1 m in: x%2)", RZ_Util.RoundToInt(perimeter.EdgeDistance(near)), search.NearLineMult(near)));

		vector deep = vector.Zero;
		vector probe;
		if (perimeter.RandomNearLine(1150, 1400, probe) && perimeter.EdgeDistance(probe) > 1100)
			deep = probe;

		if (deep != vector.Zero)
			Check(search.NearLineMult(deep) == 1, string.Format("deep inside (%1 m in) rare finds are as usual", RZ_Util.RoundToInt(perimeter.EdgeDistance(deep))));
		else
			Print("RZT loot: no ground 1.1 km inside the line (a small world); deep inside not tried");

		array<vector> checkpoints = {};
		perimeter.GetCheckpoints(checkpoints);
		if (checkpoints.IsEmpty())
		{
			Print("RZT loot: no sealed checkpoints in this world; the evacuation road not tried");
			return;
		}

		Check(search.IsEvacRoad(checkpoints[0] + Vector(30, 0, 30)), "a wreck by a sealed checkpoint is on the evacuation road");
		if (deep != vector.Zero && perimeter.GetCheckpointDistance(deep) > 600)
			Check(!search.IsEvacRoad(deep), "one deep inside isn't");
	}
}
