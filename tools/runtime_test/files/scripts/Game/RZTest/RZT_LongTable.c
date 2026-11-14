// Test-only (not part of the mod): the Long Table's farm and its reach (docs/DESIGN.md, "The Long
// Table"). From 90 s in, with a house made sure of: (1) the family's bodies stand at the farm with
// the tester 60 m off, in their own looks, and the farm is laid out; (2) supper signs at junctions,
// one put out with the tester beside it; (3) the bell and the voice at supper; (4) a death near the
// farm taken at dawn (a tell); (5) a jar in loot near the farm; (6) the door mark; (7) the host on
// the road; (8) the injured man at dusk on a road; (9) the boy in a town near the farm; (10) the
// harvest supper. Lines: "RZT table: ...", "RZT table OK ...", "RZT table FAIL ...".

class RZT_Table
{
	static const float START = 90;
	static float s_fClock;
	static float s_fWait;
	static int s_iStep;
	static int s_iFails;
	static int s_iOks;
	static bool s_bDone;
	static vector s_vPoint;

	//------------------------------------------------------------------------------------------------
	static void Ok(string what)
	{
		s_iOks++;
		Print("RZT table OK " + what);
	}

	//------------------------------------------------------------------------------------------------
	static void Fail(string what)
	{
		s_iFails++;
		Print("RZT table FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	static void SetHour(float h)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (world && world.GetTimeAndWeatherManager())
			world.GetTimeAndWeatherManager().SetTimeOfTheDay(h);
	}

	//------------------------------------------------------------------------------------------------
	static void Next(int step)
	{
		s_iStep = step;
		s_fWait = 0;
	}

	//------------------------------------------------------------------------------------------------
	static void Finish()
	{
		Print(string.Format("RZT table: done, %1 checks OK, %2 failures", s_iOks, s_iFails));
		s_bDone = true;
	}
}

modded class RZ_LongTableReach
{
	//------------------------------------------------------------------------------------------------
	void RZT_From(float hour)
	{
		m_fLastHour = hour;
	}

	int RZT_Signs() { return m_aJunctions.Count(); }
	IEntity RZT_SignEntity(int i) { return m_aSigns[i]; }
	int RZT_Dead() { return m_aDead.Count(); }
	void RZT_Harvest() { m_S.m_iHarvestDay = 1; m_bHarvestDone = false; }
	void RZT_Mark() { m_iMarkUnix = 0; TryDoorMark(); }
	bool RZT_Marked() { return m_iMarkUnix != 0; }
	void RZT_ClearMet() { m_aMetBoy.Clear(); m_aMetInjured.Clear(); }
}

modded class RZ_LongTable
{

	//------------------------------------------------------------------------------------------------
	int RZT_Tells() { return m_aTells.Count(); }
	void RZT_Hunger(float h) { m_fHunger = h; }
	int RZT_RoadPlayer() { return m_iRoadPlayer; }
	void RZT_ClearMet() { m_aMetHost.Clear(); }

	//------------------------------------------------------------------------------------------------
	int RZT_BodiesNear(float radius, out int looked)
	{
		int n = 0;
		looked = 0;
		foreach (RZ_TableMember m : m_aMembers)
		{
			if (!m.m_Body || m.m_bOnRoad || RZ_Util.DistXZ(m.m_Body.GetOrigin(), m_vHouse) > radius)
				continue;

			n++;
			EntityPrefabData pd = m.m_Body.GetPrefabData();
			if (pd && pd.GetPrefabName().Contains("RZ_Table_"))
				looked++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		super.Tick(dt, ids, players);
		RZT_Step(dt);
	}

	//------------------------------------------------------------------------------------------------
	protected void RZT_Step(float dt)
	{
		if (RZT_Table.s_bDone || !RZT_Fake.s_Ent || !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
			return;

		RZT_Table.s_fClock += dt;
		RZT_Table.s_fWait += dt;
		if (RZT_Table.s_fClock < RZT_Table.START)
			return;

		RZ_LongTableReach reach = GetReach();
		vector farm = m_vHouse;
		switch (RZT_Table.s_iStep)
		{
			case 0:
			{
				// A house this season, whatever the roll said (as RZT_Prep's RZT_Ensure, which compiles after this)
				if (m_Settings && m_Settings.m_bEnabled && !Exists())
				{
					Found();
					Publish();
				}

				if (m_eState == RZ_ETableState.QUIET)
					Relocate();

				if (!IsOpen())
				{
					Print("RZT table: no Long Table this run (off, or no place on this map); not tried");
					RZT_Table.Finish();
					return;
				}

				Print(string.Format("RZT table: the %1 farm past %2 at %3 (chance setting %4)", m_sSurname, GetTownName(), RZ_SettlementScanner.GridReference(farm), m_Settings.m_fChance));
				RZT_Hunger(80);
				RZT_Table.SetHour(19.6);
				RZT_Fake.MoveTo(RZ_Mission.LandPointNear(farm, 55, 65));
				RZT_Table.Next(1);
				return;
			}
			case 1:
			{
				if (RZT_Table.s_fWait < 20)
					return;

				int looked;
				int n = RZT_BodiesNear(40, looked);
				if (n >= 2)
					RZT_Table.Ok(string.Format("the family stands at the farm: %1 bodies with the tester 60 m off", n));
				else
					RZT_Table.Fail(string.Format("only %1 of the family at the farm with the tester 60 m off", n));

				// Their looks are built on the mod's own civilian; the test world sets a vanilla one
				ResourceName civ = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
				if (!civ.Contains("RZ_Civilian.et"))
				{
					// Then the look itself: picked for the mod's civilian, and it spawns
					ResourceName look = reach.LookFor("{D5006C19E40CE491}Prefabs/ReforgerZ/Puzzles/RZ_Civilian.et", 3);
					IEntity probe;
					if (look.Contains("RZ_Table_"))
						probe = RZ_Util.SpawnPrefab(look, RZ_Mission.LandPointNear(farm, 20, 30));

					if (probe)
						RZT_Table.Ok(string.Format("their look %1 is picked for the mod's civilian and spawns", RZ_Util.PrettyPrefabName(look)));
					else
						RZT_Table.Fail(string.Format("the family's look didn't spawn (%1)", look));

					RZ_Util.DeleteUnit(probe);
				}
				else if (looked == n && n > 0)
					RZT_Table.Ok(string.Format("in their own looks (%1 of %2)", looked, n));
				else
					RZT_Table.Fail(string.Format("%1 of %2 in the family's looks", looked, n));

				if (reach.IsDressed() && reach.DressCount() > 0)
					Print(string.Format("RZT table: the farm laid out, %1 things", reach.DressCount()));
				else
					Print("RZT table: the farm wasn't laid out (no room for the long table there, logged)");

				RZT_Table.Next(2);
				return;
			}
			case 2:
			{
				int signs = reach.RZT_Signs();
				Print(string.Format("RZT table: %1 junctions for signs", signs));
				if (signs == 0)
				{
					Print("RZT table: no road junctions near the farm on this map; the signs weren't tried");
					RZT_Table.Next(4);
					return;
				}

				RZT_Fake.MoveTo(reach.SignAt(0) + Vector(6, 0, 0));
				RZT_Table.Next(3);
				return;
			}
			case 3:
			{
				if (RZT_Table.s_fWait < 15)
					return;

				if (reach.RZT_SignEntity(0))
					RZT_Table.Ok(string.Format("a supper sign stands at the junction at %1", RZ_SettlementScanner.GridReference(reach.SignAt(0))));
				else
					RZT_Table.Fail("no sign at the nearest junction with the tester beside it");

				RZT_Table.Next(4);
				return;
			}
			case 4:
			{
				// The bell and the voice: the evening crossed from 18:40 to 19:00
				RZT_Fake.MoveTo(RZ_Mission.LandPointNear(farm, 900, 1100));
				reach.RZT_From(18.4);
				RZT_Table.SetHour(18.9);
				NoteDeath(RZ_Mission.LandPointNear(farm, 500, 700), "a test man");
				RZT_Table.Next(5);
				return;
			}
			case 5:
			{
				if (RZT_Table.s_fWait < 12)
					return;

				Print("RZT table: the evening crossed (the bell and the voice lines above)");
				if (reach.RZT_Dead() > 0)
					RZT_Table.Ok("a death near the farm waits for the night");
				else
					RZT_Table.Fail("a death 600 m from the farm wasn't kept for the night");

				int before = RZT_Tells();
				reach.RZT_From(5.4);
				RZT_Table.SetHour(5.6);
				RZT_Table.s_vPoint = Vector(before, 0, 0);
				RZT_Table.Next(6);
				return;
			}
			case 6:
			{
				if (RZT_Table.s_fWait < 12)
					return;

				if (RZT_Tells() > RZT_Table.s_vPoint[0] && reach.RZT_Dead() == 0)
					RZT_Table.Ok("at dawn the dead are gone, with drag marks (a tell)");
				else
					RZT_Table.Fail("at dawn no drag marks for the death near the farm");

				// Jars in loot near the farm
				int jars = 0;
				for (int i = 0; i < 300; i++)
				{
					array<ResourceName> picks = {};
					RZ_LongTableReach.AddLoot(farm + Vector(300, 0, 0), picks);
					if (!picks.IsEmpty())
						jars++;
				}

				if (jars > 0)
					RZT_Table.Ok(string.Format("their jar in loot near the farm: %1 of 300 houses", jars));
				else
					RZT_Table.Fail("no jar in 300 houses near the farm");

				reach.RZT_Mark();
				if (reach.RZT_Marked())
					RZT_Table.Ok("a plate and a fork on a door in a town near the farm");
				else
					Print("RZT table: no door mark written (no town 600 m to 4 km from the farm with a free house door)");

				// The host on the road: 800 m out, alone
				RZT_ClearMet();
				RZT_Table.SetHour(12);
				RZT_Fake.MoveTo(RZ_Mission.LandPointNear(farm, 750, 850));
				RZT_Table.Next(7);
				return;
			}
			case 7:
			{
				if (RZT_RoadPlayer() == RZT_Fake.ID)
				{
					RZT_Table.Ok("the host met the tester on the road");
					RZT_Table.Next(8);
					return;
				}

				if (RZT_Table.s_fWait > 300)
				{
					RZT_Table.Fail("the host never met the tester on the road in 5 minutes");
					RZT_Table.Next(8);
				}

				return;
			}
			case 8:
			{
				// The injured man: dusk, on a road 1-1.9 km off, out of town
				array<vector> roads = {};
				RZ_Perimeter.RoadPointsNear(farm, 1900, roads);
				vector spot = vector.Zero;
				foreach (vector rp : roads)
				{
					float d = RZ_Util.DistXZ(rp, farm);
					RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(rp));
					bool town = s && RZ_Util.DistXZ(s.m_vCenter, rp) <= s.m_fRadius + 80;
					if (d >= 1000 && !town)
					{
						spot = rp;
						break;
					}
				}

				if (spot == vector.Zero)
				{
					Print("RZT table: no road out of town 1-1.9 km from the farm; the injured man wasn't tried");
					RZT_Table.Next(10);
					return;
				}

				reach.RZT_ClearMet();
				RZT_Table.SetHour(19.5);
				RZT_Fake.MoveTo(spot);
				RZT_Table.Next(9);
				return;
			}
			case 9:
			{
				if (reach.InjuredBody())
				{
					RZT_Table.Ok(string.Format("a man lies hurt on the road at dusk, %1 m from the tester", RZ_Util.RoundToInt(RZ_Util.DistXZ(reach.InjuredBody().GetOrigin(), RZT_Fake.s_Ent.GetOrigin()))));
					RZT_Table.Next(10);
					return;
				}

				if (RZT_Table.s_fWait > 300)
				{
					RZT_Table.Fail("no injured man on the road at dusk in 5 minutes");
					RZT_Table.Next(10);
				}

				return;
			}
			case 10:
			{
				// The boy: a town 800 m to 3.5 km from the farm
				vector townAt = vector.Zero;
				foreach (int ti, RZ_Settlement st : m_GM.GetSettlements())
				{
					float td = RZ_Util.DistXZ(st.m_vCenter, farm);
					if (ti != m_iSettlement && td >= 900 && td <= 3300)
					{
						townAt = st.m_vCenter;
						break;
					}
				}

				if (townAt == vector.Zero)
				{
					Print("RZT table: no town 0.9-3.3 km from the farm; the boy wasn't tried");
					RZT_Table.Next(12);
					return;
				}

				reach.RZT_ClearMet();
				RZT_Table.SetHour(12);
				RZT_Fake.MoveTo(RZ_Mission.LandPointNear(townAt, 0, 30));
				RZT_Table.Next(11);
				return;
			}
			case 11:
			{
				if (reach.BoyBody())
				{
					RZT_Table.Ok("the boy asked the tester to supper in a town near the farm");
					RZT_Table.Next(12);
					return;
				}

				if (RZT_Table.s_fWait > 300)
				{
					RZT_Table.Fail("the boy never came in 5 minutes");
					RZT_Table.Next(12);
				}

				return;
			}
			case 12:
			{
				reach.RZT_Harvest();
				reach.RZT_From(16.9);
				RZT_Table.SetHour(17.2);
				RZT_Table.Next(13);
				return;
			}
			case 13:
			{
				if (RZT_Table.s_fWait < 12)
					return;

				if (reach.IsHarvestTonight())
					RZT_Table.Ok("the harvest supper began on its day");
				else
					RZT_Table.Fail("the harvest supper didn't begin");

				RZT_Table.SetHour(12);
				RZT_Table.Finish();
				return;
			}
		}
	}
}
