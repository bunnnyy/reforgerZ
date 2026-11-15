// Test-only (not part of the mod): eating and drinking (Interactions/RZ_Eating.c; docs/DESIGN.md,
// "Eating and drinking"). On the fake player, in the core run and the full run:
// - The hunger and thirst mod's five items (crackers, the meat leg, dry meat, the MRE, the water
//   bottle) carry its food component, so the hand's left click, R and RT and the inventory's Use eat them.
// - The inventory's Use (the server's eat): an MRE in the pack raises the food level and is gone; a
//   canteen (a drink the mod can't drink) raises the water level and is gone; an MRE on the ground,
//   not in the kit, is refused.
// Lines: "RZT eat OK|FAIL <what>" and "RZT eat: N checks, M failed".

class RZT_Eat
{
	static int s_iStep;
	static int s_iChecks;
	static int s_iFails;
	static float s_fFoodBefore;
	static float s_fWaterBefore;
	static IEntity s_Mre;
	static IEntity s_Canteen;
	static IEntity s_Ground;
	static int s_iWaitUntil;

	static const string MRE = "{EB21D26B8D5A3232}Prefabs/Items/Food/MRE.et";
	static const string CANTEEN = "{70BC751317551D9B}Prefabs/Items/Equipment/Canteens/Canteen_Soviet_01.et";

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT eat %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static IEntity Make(string prefab, vector at)
	{
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return null;

		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		p.Transform[3] = at;
		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
	}

	//------------------------------------------------------------------------------------------------
	static int IdOf(IEntity e)
	{
		if (!e)
			return 0;

		RplComponent rpl = RplComponent.Cast(e.FindComponent(RplComponent));
		if (!rpl)
			return 0;

		int id = rpl.Id();
		return id;
	}

	//------------------------------------------------------------------------------------------------
	//! Stepped once a second from the diag tick, from sec.
	static void Tick(int sec)
	{
		if (s_iStep < 0 || !RZT_Fake.s_Ent || !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
			return;

		IEntity me = RZT_Fake.s_Ent;
		B_MetabolismComponent m = B_MetabolismComponent.Cast(me.FindComponent(B_MetabolismComponent));
		if (s_iStep == 0)
		{
			Check(m != null, "the tester has the hunger and thirst mod's metabolism");
			if (!m)
			{
				Done();
				return;
			}

			// The five items carry the mod's food component
			array<string> five = {
				"{50A68300B537EDFA}Prefabs/Items/Food/ArmyCrackers_Soviet_01.et",
				"{128A19AA9F731095}Prefabs/Items/Food/MeatLeg_01.et",
				"{C22A2F123CD609FC}Prefabs/Items/Food/Meat_01_Dry.et",
				MRE,
				"{D5E10AA9B0808D76}prefabs/Items/Water/WaterBottle.et"};
			foreach (string prefab : five)
			{
				IEntity e = Make(prefab, me.GetOrigin() + Vector(0, -50, 0));
				Check(e && RZ_Eating.FoodOf(e) != null, "the food component on " + prefab);
				if (e)
					Check(RZ_Eating.IsEdible(e), string.Format("%1 can be eaten or drunk (%2)", prefab, RZ_Eating.Verb(e)));

				if (e)
					SCR_EntityHelper.DeleteEntityAndChildren(e);
			}

			// Hungry and thirsty enough that a meal shows
			m.SetFoodLevel(0.4);
			m.SetThirstLevel(0.4);
			s_Mre = Make(MRE, me.GetOrigin() + Vector(0, 1, 0));
			s_Canteen = Make(CANTEEN, me.GetOrigin() + Vector(0, 1, 0));
			s_Ground = Make(MRE, me.GetOrigin() + Vector(2, 0.5, 0));
			Check(s_Mre && RZ_Util.InsertIntoInventory(me, s_Mre), "an MRE goes in the tester's kit");
			Check(s_Canteen && RZ_Util.InsertIntoInventory(me, s_Canteen), "a canteen goes in the tester's kit");
			Check(s_Canteen && RZ_Eating.IsOtherDrink(s_Canteen), "a canteen is a drink the mod can't drink (Interactions > Drink keywords)");
			s_iStep = 1;
			s_iWaitUntil = sec + 2;
			return;
		}

		if (sec < s_iWaitUntil)
			return;

		if (s_iStep == 1)
		{
			Check(s_Mre && RZ_Eating.Carries(me, s_Mre), "the MRE is carried");
			Check(s_Ground && !RZ_Eating.Carries(me, s_Ground), "the MRE on the ground isn't");
			s_fFoodBefore = m.GetFoodLevel();
			s_fWaterBefore = m.GetThirstLevel();
			// The server's eat as CMD_EAT runs it (the fake player isn't in the player manager, so by entity)
			Check(RZ_Eating.EatBy(me, s_Mre), "the inventory's Use eats the MRE");
			Check(RZ_Eating.EatBy(me, s_Canteen), "the inventory's Use drinks the canteen");
			Check(!RZ_Eating.EatBy(me, s_Ground), "the MRE on the ground is refused");
			Check(m.IsIncreasing(), "eating starts the mod's gradual increase");
			s_iStep = 2;
			s_iWaitUntil = sec + 35;
			return;
		}

		if (s_iStep == 2)
		{
			float food = m.GetFoodLevel();
			float water = m.GetThirstLevel();
			Check(food > s_fFoodBefore, string.Format("the MRE raised the food level (%1 to %2)", s_fFoodBefore, food));
			Check(water > s_fWaterBefore, string.Format("the drinks raised the water level (%1 to %2)", s_fWaterBefore, water));
			Check(!s_Mre, "the eaten MRE is gone");
			Check(!s_Canteen, "the drunk canteen is gone");
			Check(s_Ground != null, "the MRE on the ground wasn't eaten");
			if (s_Ground)
				SCR_EntityHelper.DeleteEntityAndChildren(s_Ground);

			Done();
		}
	}

	//------------------------------------------------------------------------------------------------
	static void Done()
	{
		s_iStep = -1;
		Print(string.Format("RZT eat: %1 checks, %2 failed", s_iChecks, s_iFails));
	}
}
