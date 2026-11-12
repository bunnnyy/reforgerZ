// Test-only (not part of the mod): a fake player (id 901) the systems see as online.
class RZT_Fake
{
	static const int ID = 901;
	static IEntity s_Ent;
	//! The route's Perimeter test is on: until then the line leaves the tester alone (RZ_Perimeter
	//! below), so a mission spot the test teleports him to near the edge can't end his life
	static bool s_bLineTest;
	static bool s_bLineSaid;

	static void Add(array<int> ids, array<IEntity> ents)
	{
		if (!s_Ent || !RZ_Util.IsCharacterAlive(s_Ent))
			return;
		ids.Insert(ID);
		ents.Insert(s_Ent);
	}

	static IEntity Spawn(vector pos)
	{
		Resource res = Resource.Load("{8C7093AF368F496A}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_CottonShirt_1.et");
		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		p.Transform[3] = RZ_Util.GroundPos(pos);
		s_Ent = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
		return s_Ent;
	}

	static void MoveTo(vector pos)
	{
		if (s_Ent)
			s_Ent.SetOrigin(RZ_Util.GroundPos(pos));
	}

	static void Kill()
	{
		SCR_DamageManagerComponent dm = SCR_DamageManagerComponent.Cast(s_Ent.FindComponent(SCR_DamageManagerComponent));
		if (dm)
			dm.Kill(Instigator.CreateInstigatorGM());
	}
}

// Until the route's Perimeter test, the line ignores the tester (a mission or questline spot the test
// teleports him to may lie in the line's band on a small map); it says so once per visit
modded class RZ_Perimeter
{
	override protected void UpdatePlayer(int playerId, IEntity player, float dt)
	{
		if (playerId != RZT_Fake.ID || RZT_Fake.s_bLineTest)
		{
			super.UpdatePlayer(playerId, player, dt);
			return;
		}

		RZ_EPerimeterZone zone = GetZone(player.GetOrigin());
		if (zone == RZ_EPerimeterZone.NONE)
		{
			RZT_Fake.s_bLineSaid = false;
			return;
		}

		if (!RZT_Fake.s_bLineSaid)
		{
			RZT_Fake.s_bLineSaid = true;
			Print(string.Format("RZT perimeter: the tester is in the line's %1 zone at %2 (left alone until the route's Perimeter test)", typename.EnumToString(RZ_EPerimeterZone, zone), RZ_SettlementScanner.GridReference(player.GetOrigin())));
		}
	}
}
