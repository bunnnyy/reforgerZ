// Test-only (not part of the mod): a fake player (id 901) the systems see as online.
class RZT_Fake
{
	static const int ID = 901;
	static IEntity s_Ent;

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
