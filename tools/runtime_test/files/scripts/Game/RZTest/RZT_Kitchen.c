// Test-only (not part of the mod): kitchen work stands at a kitchen (docs/ERRANDS.md, "Work sites").
// Once, five minutes in: (1) "Stir the pot" opened beside the nearest building, with the stove and
// the kitchen table as its looks, must stand with a kitchen thing (the stove, the kitchen table or
// the pot), never a cardboard box; (2) the nearest cook pot a camp has laid out (a kit's, or a
// hand-built site's cook spot) takes the same work at itself, with nothing more put out.
// Lines: "RZT kitchen: ..." and "RZT kitchen FAIL ...".

class RZT_Kitchen
{
	static const string KEY_TABLE = "rzt.kitchen.table";
	static const string KEY_PIECE = "rzt.kitchen.piece";
	static const ResourceName STOVE = "{0DFBDF240FC11A1A}Prefabs/ReforgerZ/Work/RZ_Work_Stove.et";
	static const ResourceName TABLE = "{87F5BB277D49A412}Prefabs/ReforgerZ/Work/RZ_Work_KitchenTable.et";
	static const int START = 300;		// seconds of the work sites' tick before it starts
	static int s_iTicks;
	static int s_iStep;
	static int s_iWait;
	static int s_iFails;
	static bool s_bDone;
	static bool s_bPiece;

	//------------------------------------------------------------------------------------------------
	static void Fail(string what)
	{
		s_iFails++;
		Print("RZT kitchen FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsKitchen(ResourceName look)
	{
		string low = look;
		low.ToLower();
		return low.Contains("rz_work_stove") || low.Contains("kitchentable") || low.Contains("servetable") || low.Contains("rz_work_pot") || low.Contains("cookpot");
	}

	//------------------------------------------------------------------------------------------------
	static void Step(RZ_WorkSites sites)
	{
		if (s_bDone || !RZT_Fake.s_Ent || !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
			return;

		s_iTicks++;
		if (s_iTicks < START)
			return;

		if (s_iStep == 0)
		{
			vector room;
			if (!RZ_WorkSites.Indoors(RZT_Fake.s_Ent.GetOrigin(), 600, "", room))
			{
				if (s_iTicks > START + 120)
				{
					Print("RZT kitchen: no building within 600 m of the tester; not tried");
					s_bDone = true;
				}

				return;
			}

			vector at = RZ_Util.GroundPos(RZ_WorkSites.Spot(room, 2, 5));
			if (sites.Open(KEY_TABLE, RZ_EWork.HANDS, "Stir the pot", 20, at, RZT_Fake.ID, STOVE) < 0)
			{
				Fail("the stir the pot site didn't open");
				s_bDone = true;
				return;
			}

			array<ResourceName> chain = {TABLE};
			sites.SetLooks(KEY_TABLE, chain);
			Print(string.Format("RZT kitchen: stir the pot by the building at %1", RZ_SettlementScanner.GridReference(at)));
			RZT_Fake.MoveTo(at + Vector(3, 0, 0));
			s_iStep = 1;
			s_iWait = 0;
			return;
		}

		if (s_iStep == 1)
		{
			s_iWait++;
			vector pos;
			ResourceName before;
			bool atPiece;
			sites.PlaceOf(KEY_TABLE, pos, before, atPiece);
			// Its look is worked out when the props first go out (a second or two)
			if (s_iWait < 5)
				return;

			if (before.IsEmpty())
				Fail(string.Format("by the building: nothing of its kind fits at %1, not even the pot (only its prompt)", RZ_SettlementScanner.GridReference(pos)));
			else if (!IsKitchen(before))
				Fail(string.Format("by the building: %1 stands at the stir the pot prompt, not a kitchen thing", RZ_Util.PrettyPrefabName(before)));
			else
				Print(string.Format("RZT kitchen: by the building, %1 stands at the prompt", RZ_Util.PrettyPrefabName(before)));

			sites.Close(KEY_TABLE);
			s_iStep = 2;
			s_iWait = 0;
			return;
		}

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_CampLife camp;
		if (gm)
			camp = gm.GetCampLife();

		if (s_iStep == 2)
		{
			vector spot;
			float yaw;
			bool hand;
			if (!camp || !camp.UseSpotNear(RZT_Fake.s_Ent.GetOrigin(), 8000, RZ_EDressUse.COOK, 1.3, spot, yaw, hand))
			{
				Print("RZT kitchen: no camp has a cook pot laid out within 8 km; the camp half not tried");
				Finish();
				return;
			}

			if (sites.Open(KEY_PIECE, RZ_EWork.HANDS, "Stir the pot", 20, spot, RZT_Fake.ID, STOVE) < 0)
			{
				Fail("the camp's stir the pot site didn't open");
				Finish();
				return;
			}

			sites.SetAtPiece(KEY_PIECE, spot, yaw);
			s_bPiece = hand;
			RZT_Fake.MoveTo(spot + Vector(4, 0, 0));
			Print(string.Format("RZT kitchen: stir the pot at the camp's cook pot at %1 (hand-built %2)", RZ_SettlementScanner.GridReference(spot), hand));
			s_iStep = 3;
			return;
		}

		if (s_iStep == 3)
		{
			s_iWait++;
			if (s_iWait < 10)
				return;

			vector where;
			ResourceName look;
			bool onPiece;
			sites.PlaceOf(KEY_PIECE, where, look, onPiece);
			if (!onPiece)
				Fail("at the camp: the site isn't at the cook pot");
			else if (!look.IsEmpty() || sites.RZT_LookProp(KEY_PIECE))
				Fail(string.Format("at the camp: %1 was put out beside the camp's own cook pot", RZ_Util.PrettyPrefabName(look)));
			else
				Print("RZT kitchen: at the camp, the work is at its own cook pot and nothing more is put out");

			sites.Close(KEY_PIECE);
			Finish();
		}
	}

	//------------------------------------------------------------------------------------------------
	static void Finish()
	{
		Print(string.Format("RZT kitchen: done, %1 failures", s_iFails));
		s_bDone = true;
	}
}

modded class RZ_WorkSites
{
	//------------------------------------------------------------------------------------------------
	override void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		RZT_Kitchen.Step(this);
		super.Tick(dt, ids, players);
	}
}

modded class RZ_WorkSites
{
	//------------------------------------------------------------------------------------------------
	//! The site's look as put out (its before prop), not its prompt's box: the box always stands once
	//! a player is near, and is no prop of ours beside the camp's pot
	IEntity RZT_LookProp(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return null;

		return s.m_BeforeProp;
	}
}
