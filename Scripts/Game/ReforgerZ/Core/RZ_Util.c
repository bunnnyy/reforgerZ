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
	//! "1.2 km NE" from one point to another. Enfusion uses +X east and +Z north.
	static string DirectionText(vector from, vector to)
	{
		float dx = to[0] - from[0];
		float dz = to[2] - from[2];
		float deg = Math.Atan2(dx, dz) * Math.RAD2DEG;
		if (deg < 0)
			deg += 360;

		array<string> names = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
		int idx = RoundToInt(deg / 45);
		idx = idx % 8;

		float dist = DistXZ(from, to);
		if (dist < 1000)
			return string.Format("%1 m %2", RoundToInt(dist / 10) * 10, names[idx]);

		int tenths = RoundToInt(dist / 100);
		return string.Format("%1.%2 km %3", tenths / 10, tenths % 10, names[idx]);
	}

	//------------------------------------------------------------------------------------------------
	//! The player carrying an item in their inventory, or -1 if it's on the ground or held by AI.
	static int GetCarrierPlayerId(IEntity item)
	{
		if (!item)
			return -1;

		IEntity root = item.GetRootParent(); // VERIFY: GetRootParent() (inventory items are children of the character)
		if (!root || root == item)
			return -1;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(root);
		if (playerId <= 0)
			return -1;

		return playerId;
	}

	//------------------------------------------------------------------------------------------------
	//! Give an AI group a move waypoint. Does nothing for non-groups or without a waypoint prefab.
	static void SendGroupTo(IEntity e, vector target, ResourceName waypointPrefab)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group || waypointPrefab.IsEmpty())
			return;

		AIWaypoint wp = AIWaypoint.Cast(SpawnPrefab(waypointPrefab, target));
		if (wp)
			group.AddWaypoint(wp);
	}

	//------------------------------------------------------------------------------------------------
	//! Replace an AI group's waypoints with one move waypoint (hordes that change course).
	//! Deletes the waypoints it removes. Returns the new waypoint, or null.
	static IEntity RedirectGroup(IEntity e, vector target, ResourceName waypointPrefab)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group || waypointPrefab.IsEmpty())
			return null;

		array<AIWaypoint> current = {};
		group.GetWaypoints(current); // VERIFY: AIGroup.GetWaypoints() / RemoveWaypoint()
		foreach (AIWaypoint old : current)
		{
			group.RemoveWaypoint(old);
			if (old)
				SCR_EntityHelper.DeleteEntityAndChildren(old);
		}

		AIWaypoint wp = AIWaypoint.Cast(SpawnPrefab(waypointPrefab, target));
		if (wp)
			group.AddWaypoint(wp);

		return wp;
	}

	//------------------------------------------------------------------------------------------------
	static string PlayerName(int playerId)
	{
		return GetGame().GetPlayerManager().GetPlayerName(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! True if the weapon has a suppressor or silencer attached. Looks at the
	//! weapon's child entities (attachments) up to two levels deep and checks
	//! their prefab names, so it works with modded suppressors that follow the naming.
	static bool IsSuppressed(BaseWeaponComponent weapon)
	{
		if (!weapon)
			return false;

		return HasSuppressorChild(weapon.GetOwner(), 0);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasSuppressorChild(IEntity e, int depth)
	{
		if (!e || depth > 2)
			return false;

		IEntity child = e.GetChildren(); // VERIFY: GetChildren()/GetSibling() walk attachments
		while (child)
		{
			EntityPrefabData data = child.GetPrefabData(); // VERIFY: GetPrefabData().GetPrefabName()
			if (data)
			{
				string name = data.GetPrefabName();
				name.ToLower(); // VERIFY: ToLower() changes the string in place
				if (name.Contains("suppress") || name.Contains("silenc"))
					return true;
			}

			if (HasSuppressorChild(child, depth + 1))
				return true;

			child = child.GetSibling();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Prefab path of an entity, or its mesh path for map objects that aren't prefab
	//! instances. "" if neither is known. Used for keyword matching (landmarks, radios).
	static string GetPrefabName(IEntity e)
	{
		if (!e)
			return "";

		EntityPrefabData data = e.GetPrefabData(); // VERIFY: GetPrefabData().GetPrefabName()
		if (data)
		{
			string prefab = data.GetPrefabName();
			if (!prefab.IsEmpty())
				return prefab;
		}

		VObject obj = e.GetVObject(); // VERIFY: GetVObject().GetResourceName() for placed map meshes
		if (obj)
			return obj.GetResourceName();

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static int RoundToInt(float f)
	{
		int i = Math.Round(f);
		return i;
	}
}
