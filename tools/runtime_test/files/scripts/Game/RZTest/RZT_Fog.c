// Test-only (not part of the mod): the map's fog (docs/UI.md, "The map's fog"). Early in the run,
// before the missions move the tester: (1) where the tester stands is seen, and walking 600 m
// reveals the new ground round him while ground 1.5 km off stays unseen; (2) a paper map put in his
// pack, registered as found in a far town, reveals that town within the found-map check; (3) the
// fog survives a JSON round trip of the profile, as the save writes it; (4) the payload is small.
// Lines: "RZT fog OK|FAIL ..." and "RZT fog: N checks, N failed".

class RZT_Fog
{
	static const ResourceName PAPER_MAP = "{13772C903CB5E4F7}Prefabs/Items/Equipment/Maps/Map_Paper_01/PaperMap_01_folded.et";
	static int s_iStep;
	static int s_iWait;
	static int s_iChecks;
	static int s_iFails;
	static bool s_bDone;
	static vector s_vStart;
	static vector s_vTown;
	static int s_iBefore;
	static IEntity s_Map;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (ok)
		{
			Print("RZT fog OK " + what);
			return;
		}

		s_iFails++;
		Print("RZT fog FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from the test's tick. True when it's finished (or can't run).
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		RZ_MapFog fog = gm.GetMapFog();
		if (!fog || !fog.IsOn())
		{
			Print("RZT fog: the map's fog is off; not tried");
			s_bDone = true;
			return true;
		}

		IEntity me = RZT_Fake.s_Ent;
		if (!me || !RZ_Util.IsCharacterAlive(me))
			return false;

		int id = RZT_Fake.ID;
		if (s_iStep == 0)
		{
			// Let the second tick see him where he stands
			s_vStart = me.GetOrigin();
			s_iWait++;
			if (s_iWait < 3)
				return false;

			Check(fog.Seen(id, s_vStart), string.Format("where the tester stands is seen (%1 squares so far)", fog.SeenCount(id)));
			// Ground 1.5 km off, on the map (the test world is small: try eight ways)
			vector far = vector.Zero;
			for (int k = 0; k < 8; k++)
			{
				float ang = Math.PI2 * k / 8;
				vector probe = s_vStart + Vector(Math.Cos(ang) * 1500, 0, Math.Sin(ang) * 1500);
				if (fog.CellOf(probe) >= 0)
				{
					far = probe;
					break;
				}
			}

			if (far != vector.Zero)
				Check(!fog.Seen(id, far), string.Format("ground 1.5 km off (%1) is still unseen", RZ_SettlementScanner.GridReference(far)));
			s_iBefore = fog.SeenCount(id);
			RZT_Fake.MoveTo(s_vStart + Vector(600, 0, 0));
			s_iStep = 1;
			s_iWait = 0;
			return false;
		}

		if (s_iStep == 1)
		{
			s_iWait++;
			if (s_iWait < 3)
				return false;

			int after = fog.SeenCount(id);
			Check(after > s_iBefore, string.Format("walking 600 m revealed new ground (%1 to %2 squares)", s_iBefore, after));
			Check(fog.Seen(id, me.GetOrigin()), "the new spot is seen");

			// A far town for the found map: the settlement furthest within 4 km that isn't seen yet
			s_vTown = vector.Zero;
			float best = 0;
			for (int i = 0; i < 400; i++)
			{
				RZ_Settlement s = gm.GetSettlement(i);
				if (!s)
					break;

				float d = RZ_Util.DistXZ(s.m_vCenter, s_vStart);
				if (d > best && d < 4000 && !fog.Seen(id, s.m_vCenter))
				{
					best = d;
					s_vTown = s.m_vCenter;
				}
			}

			if (s_vTown == vector.Zero)
			{
				Print("RZT fog: no unseen town within 4 km; the found map not tried");
				s_iStep = 3;
				return false;
			}

			s_Map = RZ_Util.SpawnPrefab(PAPER_MAP, me.GetOrigin() + Vector(0, 0.5, 0), 0);
			if (!s_Map)
			{
				Check(false, "a paper map spawned");
				s_iStep = 3;
				return false;
			}

			fog.RegisterFoundMap(s_Map, s_vTown);
			RZ_Util.InsertIntoInventory(me, s_Map);
			Print(string.Format("RZT fog: a paper map found in the town at %1 (%2 m off), put in the tester's pack", RZ_SettlementScanner.GridReference(s_vTown), RZ_Util.RoundToInt(best)));
			s_iStep = 2;
			s_iWait = 0;
			return false;
		}

		if (s_iStep == 2)
		{
			s_iWait++;
			if (!fog.Seen(id, s_vTown) && s_iWait < 12)
				return false;

			bool carried = s_Map && s_Map.GetRootParent() == me;
			Check(fog.Seen(id, s_vTown), string.Format("the found map revealed its town (in the pack: %1, after %2 s)", carried, s_iWait));
			s_iStep = 3;
			return false;
		}

		// The save's round trip: the profile as JSON and back
		RZ_PlayerProfile p = gm.FindProfile(id);
		if (p)
		{
			JsonSaveContext save = new JsonSaveContext();
			save.WriteValue("p", p);
			string text = save.SaveToString();
			JsonLoadContext load = new JsonLoadContext();
			RZ_PlayerProfile back = new RZ_PlayerProfile();
			bool read = load.LoadFromString(text) && load.ReadValue("p", back);
			bool same = read && back.m_sFogGrid == p.m_sFogGrid && back.m_aFog && back.m_aFog.Count() == p.m_aFog.Count();
			if (same)
			{
				for (int w = 0; w < p.m_aFog.Count(); w++)
				{
					if (back.m_aFog[w] != p.m_aFog[w])
					{
						same = false;
						break;
					}
				}
			}

			Check(same, string.Format("the fog survives the save's JSON round trip (%1 words)", p.m_aFog.Count()));
		}
		else
		{
			Check(false, "the tester has a profile");
		}

		// Unseen places aren't on the map: no safehouse he hasn't seen has its pin in his map's list
		if (fog.IsOn() && gm.GetUIFeed() && gm.GetSafehouses())
		{
			string mapList = gm.GetUIFeed().BuildPayload(id, RZ_UIFeed.PAYLOAD_MAP);
			int unseen = 0;
			int pinned = 0;
			RZ_Safehouses houses = gm.GetSafehouses();
			for (int h = 0; h < houses.Count(); h++)
			{
				RZ_Safehouse house = houses.Get(h);
				if (!house || fog.Seen(id, house.m_vPos))
					continue;

				unseen++;
				string pin = string.Format("marker_safehouse|%1|%2|%3|", RZ_EFaction.SURVIVORS, RZ_Util.RoundToInt(house.m_vPos[0]), RZ_Util.RoundToInt(house.m_vPos[2]));
				if (mapList.Contains(pin))
					pinned++;
			}

			Check(pinned == 0 && !mapList.Contains("somewhere here"), string.Format("unseen safehouses aren't on the map (%1 unseen, %2 pinned)", unseen, pinned));
		}

		int size = fog.Build(id).Length();
		Check(size > 0 && size < 4000, string.Format("the payload is small (%1 characters)", size));

		Print(string.Format("RZT fog: %1 checks, %2 failed", s_iChecks, s_iFails));
		RZT_Fake.MoveTo(s_vStart);
		s_bDone = true;
		return true;
	}
}
