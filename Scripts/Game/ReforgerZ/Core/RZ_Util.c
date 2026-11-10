//! Shared helpers. Every engine call here is kept small so it is easy to fix
//! if an API name differs in your game version. Search the codebase for
//! "VERIFY:" to find all calls that still need checking in Workbench.
class RZ_Log
{
	static void Info(string msg)
	{
		Print("[ReforgerZ] " + msg, LogLevel.NORMAL);
	}

	static void Warn(string msg)
	{
		Print("[ReforgerZ] " + msg, LogLevel.WARNING);
	}

	static void Error(string msg)
	{
		Print("[ReforgerZ] " + msg, LogLevel.ERROR);
	}
}

class RZ_Util
{
	//------------------------------------------------------------------------------------------------
	static float DistXZ(vector a, vector b)
	{
		float dx = a[0] - b[0];
		float dz = a[2] - b[2];
		return Math.Sqrt(dx * dx + dz * dz);
	}

	//------------------------------------------------------------------------------------------------
	static vector GroundPos(vector pos)
	{
		pos[1] = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
		return pos;
	}

	//------------------------------------------------------------------------------------------------
	//! Most Reforger terrains have sea level at 0. Good enough to keep zombies out of the sea.
	static bool IsWater(vector groundPos)
	{
		return groundPos[1] < 0.5;
	}

	//------------------------------------------------------------------------------------------------
	static vector RandomPointInRing(vector center, float minR, float maxR)
	{
		float angle = Math.RandomFloat(0, Math.PI2);
		float r = Math.RandomFloat(minR, maxR);
		vector p = center;
		p[0] = p[0] + Math.Cos(angle) * r;
		p[2] = p[2] + Math.Sin(angle) * r;
		return GroundPos(p);
	}

	//------------------------------------------------------------------------------------------------
	static float NearestDistance(vector pos, array<IEntity> entities)
	{
		float best = float.MAX;
		foreach (IEntity e : entities)
		{
			if (!e)
				continue;

			float d = DistXZ(pos, e.GetOrigin());
			if (d < best)
				best = d;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	static IEntity SpawnPrefab(ResourceName prefab, vector pos, float yaw = 0)
	{
		if (prefab.IsEmpty())
			return null;

		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
		{
			RZ_Log.Warn("Invalid prefab: " + prefab);
			return null;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), params.Transform);
		params.Transform[3] = pos;
		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes a character or an AI group including all of its members.
	static void DeleteUnit(IEntity e)
	{
		if (!e)
			return;

		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (group)
		{
			array<AIAgent> agents = {};
			group.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				if (agent && agent.GetControlledEntity())
					SCR_EntityHelper.DeleteEntityAndChildren(agent.GetControlledEntity());
			}
		}

		SCR_EntityHelper.DeleteEntityAndChildren(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Number of living units: agents for a group, 1/0 for a character.
	static int CountAlive(IEntity e)
	{
		if (!e)
			return 0;

		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (group)
			return group.GetAgentsCount();

		if (IsCharacterAlive(e))
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsCharacterAlive(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return e != null;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (!dmg)
			return true;

		return dmg.GetState() != EDamageState.DESTROYED;
	}

	//------------------------------------------------------------------------------------------------
	//! Position of a unit. Groups use their leader, since the group entity itself does not move.
	static vector GetUnitPosition(IEntity e)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (group)
		{
			AIAgent leader = group.GetLeaderAgent();
			if (leader && leader.GetControlledEntity())
				return leader.GetControlledEntity().GetOrigin();
		}

		return e.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	static void KillCharacter(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (dmg)
			dmg.Kill(Instigator.CreateInstigator(null)); // VERIFY: Kill() signature in your game version
	}

	//------------------------------------------------------------------------------------------------
	static bool IsInVehicle(IEntity e)
	{
		CompartmentAccessComponent cac = CompartmentAccessComponent.Cast(e.FindComponent(CompartmentAccessComponent));
		return cac && cac.IsInCompartment(); // VERIFY: IsInCompartment()
	}

	//------------------------------------------------------------------------------------------------
	static void GetAlivePlayers(notnull array<int> outIds, notnull array<IEntity> outEntities)
	{
		array<int> all = {};
		PlayerManager pm = GetGame().GetPlayerManager();
		pm.GetPlayers(all);
		foreach (int id : all)
		{
			IEntity ent = pm.GetPlayerControlledEntity(id);
			if (!ent || !IsCharacterAlive(ent))
				continue;

			outIds.Insert(id);
			outEntities.Insert(ent);
		}
	}

	//------------------------------------------------------------------------------------------------
	static int RoundToInt(float f)
	{
		int i = Math.Round(f);
		return i;
	}
}
