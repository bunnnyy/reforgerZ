// Test-only (not part of the mod): carrying for real (Missions/Blocks/RZ_Carry.c, docs/DESIGN.md
// "Carrying"). Once, a few minutes in, three crates are carried by the tester from the open into the
// nearest building: each picked up at the pile (the pick-up's own crate and the others beside it
// stand there), put down indoors on the ground floor (under a roof), stacked. The tester can't hold
// a prompt, so each hold is done for him (the server's path).
// Lines: "RZT carry: ..." and "RZT carry FAIL ...".

class RZT_Carry
{
	static const string KEY = "rzt.carry";
	static const ResourceName CRATE = "{2B1D0128ED660BE9}Prefabs/Props/Crates/BoxWooden_01.et";
	static const int START = 240;		// seconds of the work sites' tick before it starts
	static int s_iTicks;
	static int s_iStep;
	static int s_iRound;
	static int s_iFails;
	static int s_iPileSeen;
	static int s_iHeldSeen;
	static int s_iInside;
	static int s_iRoofed;
	static bool s_bDone;

	//------------------------------------------------------------------------------------------------
	static void Fail(string what)
	{
		s_iFails++;
		Print("RZT carry FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	//! Something overhead between 1.8 m and 7 m above the spot (a floor or a roof).
	static bool Roofed(vector at)
	{
		TraceParam param = new TraceParam();
		param.Start = at + Vector(0, 1.8, 0);
		param.End = at + Vector(0, 7, 0);
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Flags = TraceFlags.ENTS;
		float frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param);
		return frac < 0.999;
	}

	//------------------------------------------------------------------------------------------------
	static void Step(RZ_WorkSites sites)
	{
		if (s_bDone || !RZT_Fake.s_Ent || !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
			return;

		s_iTicks++;
		if (s_iTicks < START)
			return;

		RZ_Carries carries = sites.Carries();
		if (!carries || !RZ_Carries.Available())
		{
			Print("RZT carry: off (work sites or Carry for real off)");
			s_bDone = true;
			return;
		}

		if (s_iStep == 0)
		{
			vector room;
			if (!RZ_WorkSites.Indoors(RZT_Fake.s_Ent.GetOrigin(), 600, "", room))
			{
				if (s_iTicks > START + 120)
				{
					Print("RZT carry: no building with a ground floor within 600 m of the tester; not tried");
					s_bDone = true;
				}

				return;
			}

			vector pick = RZ_Util.GroundPos(RZ_WorkSites.Spot(room, 6, 12));
			if (!carries.Open(KEY, RZT_Fake.ID, 3, CRATE, "crate", "crates", pick, false, "outside", room, true, "indoors, on the ground floor", true))
			{
				Fail("the carry didn't open");
				s_bDone = true;
				return;
			}

			Print(string.Format("RZT carry: 3 crates from %1 into the building at %2", RZ_SettlementScanner.GridReference(pick), RZ_SettlementScanner.GridReference(room)));
			RZT_Fake.MoveTo(pick);
			s_iStep = 1;
			return;
		}

		RZ_CarryJob j = carries.Job(KEY);
		if (!j)
		{
			Fail("the job is gone");
			s_bDone = true;
			return;
		}

		string pickKey = j.PickKey();
		string dropKey = j.DropKey();
		if (s_iStep == 1)
		{
			// At the pile: the pick-up's own crate stands, and the others beside it
			RZT_Fake.MoveTo(j.m_vPick);
			if (!sites.Has(pickKey))
			{
				Fail(string.Format("round %1: no pick-up prompt at the pile", s_iRound + 1));
				s_bDone = true;
				return;
			}

			int others = 0;
			foreach (IEntity e : j.m_aPileProps)
			{
				if (e)
					others++;
			}

			if (sites.PropOf(pickKey) || others > 0)
				s_iPileSeen++;

			Print(string.Format("RZT carry: round %1, at the pile: its crate %2, %3 others beside it (%4 in the pile)", s_iRound + 1, sites.PropOf(pickKey) != null, others, j.InPile()));
			sites.CompleteFor(pickKey, RZT_Fake.ID);
			s_iStep = 2;
			return;
		}

		if (s_iStep == 2)
		{
			if (!j.m_bCarrying)
			{
				Fail(string.Format("round %1: picked up but not carrying", s_iRound + 1));
				s_bDone = true;
				return;
			}

			if (j.m_Held)
				s_iHeldSeen++;

			if (!sites.Has(dropKey))
			{
				Fail(string.Format("round %1: carrying, but no put-down prompt", s_iRound + 1));
				s_bDone = true;
				return;
			}

			vector drop = sites.PosOf(dropKey);
			float above = drop[1] - RZ_Util.GroundPos(drop)[1];
			bool roof = Roofed(drop);
			if (j.m_bDropInside)
				s_iInside++;

			if (roof)
				s_iRoofed++;

			Print(string.Format("RZT carry: round %1, carrying (in the arms: %2), the put-down at %3, %4 m above the ground, a roof over it: %5", s_iRound + 1, j.m_Held != null, RZ_SettlementScanner.GridReference(drop), above, roof));
			RZT_Fake.MoveTo(drop);
			sites.CompleteFor(dropKey, RZT_Fake.ID);
			s_iStep = 3;
			return;
		}

		if (s_iStep == 3)
		{
			if (j.m_bCarrying || j.m_iDelivered != s_iRound + 1)
			{
				Fail(string.Format("round %1: put down, but %2 carried in and carrying %3", s_iRound + 1, j.m_iDelivered, j.m_bCarrying));
				s_bDone = true;
				return;
			}

			s_iRound++;
			if (s_iRound < 3)
			{
				s_iStep = 1;
				return;
			}

			s_iStep = 4;
			return;
		}

		// The stack: three put down and standing, one on another
		int stacked = 0;
		float top = -1000;
		foreach (IEntity put : j.m_aStackProps)
		{
			if (!put)
				continue;

			stacked++;
			top = Math.Max(top, put.GetOrigin()[1]);
		}

		float rise = top - j.m_vDrop[1];
		if (!carries.IsDone(KEY))
			Fail("three put down, but the job isn't done");

		if (stacked != 3)
			Fail(string.Format("%1 crates standing at the stack, not 3", stacked));

		if (s_iInside != 3 || s_iRoofed == 0)
			Fail(string.Format("the stack isn't indoors (inside %1 of 3, roofed %2 of 3)", s_iInside, s_iRoofed));

		Print(string.Format("RZT carry: done %1, 3 of 3 carried in; the pile seen %2 of 3; in the arms %3 of 3; put down indoors %4 of 3, under a roof %5 of 3; %6 standing at the stack, the top %7 m up (a crate %8 m); %9 failures",
			carries.IsDone(KEY), s_iPileSeen, s_iHeldSeen, s_iInside, s_iRoofed, stacked, rise, j.m_fItemHeight, s_iFails));
		carries.Close(KEY);
		s_bDone = true;
	}
}

modded class RZ_WorkSites
{
	//------------------------------------------------------------------------------------------------
	override void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		RZT_Carry.Step(this);
		super.Tick(dt, ids, players);
	}
}
