//! Shared helpers. Every engine call here is kept small so it is easy to fix
//! if an API changes in a later game version. Search the codebase for
//! "VERIFY:" to find the runtime behaviour a play-test still has to confirm.
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
	//! Distance from p to the segment a-b, on the ground plane.
	static float DistToSegmentXZ(vector p, vector a, vector b)
	{
		float abx = b[0] - a[0];
		float abz = b[2] - a[2];
		float len2 = abx * abx + abz * abz;
		if (len2 < 0.01)
			return DistXZ(p, a);

		float t = Math.Clamp(((p[0] - a[0]) * abx + (p[2] - a[2]) * abz) / len2, 0, 1);
		vector q = Vector(a[0] + abx * t, p[1], a[2] + abz * t);
		return DistXZ(p, q);
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
	//! Deletes a character or an AI group including all of its members and its held move waypoint
	//! (RZ_GroupOrders).
	static void DeleteUnit(IEntity e)
	{
		if (!e)
			return;

		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (group)
		{
			RZ_GroupOrders.Release(group);
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
			dmg.Kill(Instigator.CreateInstigator(null));
	}

	//------------------------------------------------------------------------------------------------
	//! Patch a character up fully (safehouse medic, Doctor wanderer).
	static void HealCharacter(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (dmg)
			dmg.FullHeal(); // also stops bleeding
	}

	//------------------------------------------------------------------------------------------------
	//! Damage off: checkpoint guards, stronghold leaders. Does nothing without a damage manager.
	static void MakeUntouchable(IEntity e)
	{
		if (!e)
			return;

		SCR_DamageManagerComponent dmg = SCR_DamageManagerComponent.Cast(e.FindComponent(SCR_DamageManagerComponent));
		if (dmg)
			dmg.EnableDamageHandling(false);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsInVehicle(IEntity e)
	{
		CompartmentAccessComponent cac = CompartmentAccessComponent.Cast(e.FindComponent(CompartmentAccessComponent));
		return cac && cac.IsInCompartment();
	}

	//------------------------------------------------------------------------------------------------
	//! The player whose damage killed this character (bounties), -1 if it wasn't a player: the
	//! infected, a fall, a fire, or nothing the engine recorded. Read it from the body after death.
	static int GetKillerPlayerId(IEntity victim)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(victim);
		if (!c)
			return -1;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (!dmg)
			return -1;

		Instigator killer = dmg.GetInstigator();
		if (!killer)
			return -1;

		int playerId = killer.GetInstigatorPlayerID();
		if (playerId <= 0)
			return -1;

		return playerId;
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
		if (dist < 15)
			return "right here";

		if (dist < 1000)
			return string.Format("%1 m %2", RoundToInt(dist / 10) * 10, names[idx]);

		int tenths = RoundToInt(dist / 100);
		return string.Format("%1.%2 km %3", tenths / 10, tenths % 10, names[idx]);
	}

	//------------------------------------------------------------------------------------------------
	//! Puts a spawned item into a character's inventory. False if it doesn't fit (or the call
	//! doesn't work): the item then stays where it was spawned.
	static bool InsertIntoInventory(IEntity character, IEntity item)
	{
		if (!character || !item)
			return false;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;

		return inv.TryInsertItem(item);
	}

	//------------------------------------------------------------------------------------------------
	//! The player carrying an item in their inventory, or -1 if it's on the ground or held by AI.
	static int GetCarrierPlayerId(IEntity item)
	{
		if (!item)
			return -1;

		// Inventory items are children of the character, but a seated character is itself a
		// child of the vehicle, so walk up rather than take the root parent.
		IEntity parent = item.GetParent();
		while (parent)
		{
			int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(parent);
			if (playerId > 0)
				return playerId;

			parent = parent.GetParent();
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Two players in the same player group (friends, for the bounty board). False if either has
	//! no group or the groups manager isn't there.
	static bool SameGroup(int a, int b)
	{
		if (a <= 0 || b <= 0)
			return false;

		if (a == b)
			return true;

		SCR_GroupsManagerComponent groups = SCR_GroupsManagerComponent.GetInstance();
		if (!groups)
			return false;

		SCR_AIGroup ga = groups.GetPlayerGroup(a);
		if (!ga)
			return false;

		return ga == groups.GetPlayerGroup(b);
	}

	//------------------------------------------------------------------------------------------------
	//! Send an AI group to a point with its one held move waypoint (RZ_GroupOrders): it goes there
	//! after any waypoints of its own ahead of it. A new order replaces the last one, and nothing is
	//! spawned after the first. Does nothing for non-groups or without a waypoint prefab.
	static void SendGroupTo(IEntity e, vector target, ResourceName waypointPrefab)
	{
		RZ_GroupOrders.Order(e, target, waypointPrefab, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Send an AI group straight to a point (hordes that change course): every other waypoint it has
	//! is taken off and deleted, and its held waypoint (RZ_GroupOrders) is moved there. Returns the
	//! held waypoint (the same entity on every order; DeleteUnit deletes it with the group), or null.
	static IEntity RedirectGroup(IEntity e, vector target, ResourceName waypointPrefab)
	{
		return RZ_GroupOrders.Order(e, target, waypointPrefab, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Put a move waypoint in front of an AI group's own: it goes there first, then carries on with
	//! them (unlike RedirectGroup, nothing of the group's is removed). Returns the waypoint (take it
	//! back with PopGroupWaypoint), or null.
	static IEntity PushGroupWaypoint(IEntity e, vector target, ResourceName waypointPrefab)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group || waypointPrefab.IsEmpty())
			return null;

		AIWaypoint wp = AIWaypoint.Cast(SpawnPrefab(waypointPrefab, target));
		if (wp)
			group.AddWaypointAt(wp, 0);

		return wp;
	}

	//------------------------------------------------------------------------------------------------
	//! Take back a waypoint PushGroupWaypoint gave: off the group's list if it's still on it, and
	//! deleted (also when the group already finished it). True if the group has waypoints left.
	static bool PopGroupWaypoint(IEntity e, IEntity waypoint)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		AIWaypoint wp = AIWaypoint.Cast(waypoint);
		if (group && wp && GroupHasWaypoint(group, wp))
			group.RemoveWaypoint(wp);

		if (waypoint)
			SCR_EntityHelper.DeleteEntityAndChildren(waypoint);

		return CountGroupWaypoints(e) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The AI group still has this waypoint (it hasn't finished it, and nothing took it away).
	static bool GroupHasWaypoint(IEntity e, IEntity waypoint)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		AIWaypoint wp = AIWaypoint.Cast(waypoint);
		if (!group || !wp)
			return false;

		array<AIWaypoint> current = {};
		group.GetWaypoints(current);
		return current.Contains(wp);
	}

	//------------------------------------------------------------------------------------------------
	//! How many waypoints an AI group has (0 for anything that isn't a group).
	static int CountGroupWaypoints(IEntity e)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group)
			return 0;

		array<AIWaypoint> current = {};
		group.GetWaypoints(current);
		return current.Count();
	}

	// ================================================================ Jobs for AI groups (camp life)
	//------------------------------------------------------------------------------------------------
	//! Add one waypoint of any kind (move, loiter, observation post, patrol) at the end of an AI
	//! group's list. radius > 0 sets its completion radius. Returns the waypoint, or null.
	static IEntity AddGroupWaypoint(IEntity e, vector at, ResourceName waypointPrefab, float radius)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group || waypointPrefab.IsEmpty())
			return null;

		IEntity spawned = SpawnPrefab(waypointPrefab, at);
		AIWaypoint wp = AIWaypoint.Cast(spawned);
		if (!wp)
		{
			if (spawned)
				SCR_EntityHelper.DeleteEntityAndChildren(spawned);

			return null;
		}

		if (radius > 0)
			wp.SetCompletionRadius(radius);

		group.AddWaypoint(wp);
		return wp;
	}

	//------------------------------------------------------------------------------------------------
	//! Send an AI group to use a smart action (a sit, lean, loiter or observation sentinel) on
	//! `target` or one of its direct children whose tags include `tag`. Returns the waypoint, or
	//! null (no group, no target, or the prefab isn't an SCR_SmartActionWaypoint).
	static IEntity SetGroupSmartAction(IEntity e, IEntity target, string tag, ResourceName waypointPrefab)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group || !target || tag.IsEmpty() || waypointPrefab.IsEmpty())
			return null;

		IEntity spawned = SpawnPrefab(waypointPrefab, target.GetOrigin());
		SCR_SmartActionWaypoint wp = SCR_SmartActionWaypoint.Cast(spawned);
		if (!wp)
		{
			if (spawned)
				SCR_EntityHelper.DeleteEntityAndChildren(spawned);

			return null;
		}

		wp.SetSmartActionEntity(target, tag); // VERIFY: a smart-action waypoint set from script sends a ReforgerZ faction group to the tagged sentinel (vanilla's SA_* behaviours run for our groups in a mod world)
		group.AddWaypoint(wp);
		return wp;
	}

	//------------------------------------------------------------------------------------------------
	//! Give an AI group a loop: a cycle waypoint that repeats `loop` (waypoints made with
	//! SpawnPrefab, not added to the group). The cycle puts them on the group's list itself.
	//! Returns the cycle waypoint, or null (then the loop's waypoints are the caller's to delete).
	static IEntity AddGroupCycle(IEntity e, notnull array<IEntity> loop, ResourceName cyclePrefab)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group || cyclePrefab.IsEmpty() || loop.IsEmpty())
			return null;

		IEntity spawned = SpawnPrefab(cyclePrefab, loop[0].GetOrigin());
		AIWaypointCycle cycle = AIWaypointCycle.Cast(spawned);
		if (!cycle)
		{
			if (spawned)
				SCR_EntityHelper.DeleteEntityAndChildren(spawned);

			return null;
		}

		array<AIWaypoint> wps = {};
		foreach (IEntity l : loop)
		{
			AIWaypoint w = AIWaypoint.Cast(l);
			if (w)
				wps.Insert(w);
		}

		cycle.SetWaypoints(wps); // as vanilla's patrol command: only the cycle goes on the group
		group.AddWaypoint(cycle);
		return cycle;
	}

	//------------------------------------------------------------------------------------------------
	//! The AI group a character is in, or null.
	static IEntity GroupOf(IEntity character)
	{
		if (!character)
			return null;

		AIControlComponent control = AIControlComponent.Cast(character.FindComponent(AIControlComponent));
		if (!control)
			return null;

		AIAgent agent = control.GetControlAIAgent();
		if (!agent)
			return null;

		return SCR_AIGroup.Cast(agent.GetParentGroup());
	}

	//------------------------------------------------------------------------------------------------
	//! Put a lone AI character into a new, empty AI group on the character's own faction, so it
	//! can take waypoints. Returns the group, or null (no prefab, no AI, or it didn't join).
	//! Take it out again with UnwrapGroup before deleting the group.
	static IEntity WrapInGroup(IEntity character, ResourceName groupPrefab)
	{
		if (!character || groupPrefab.IsEmpty() || !ChimeraCharacter.Cast(character))
			return null;

		if (!character.FindComponent(AIControlComponent))
			return null;

		IEntity spawned = SpawnPrefab(groupPrefab, character.GetOrigin());
		SCR_AIGroup group = SCR_AIGroup.Cast(spawned);
		if (!group)
		{
			if (spawned)
				SCR_EntityHelper.DeleteEntityAndChildren(spawned);

			return null;
		}

		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(character.FindComponent(FactionAffiliationComponent));
		if (fac && fac.GetAffiliatedFaction())
			group.SetFaction(fac.GetAffiliatedFaction());

		if (!group.AddAIEntityToGroup(character)) // VERIFY: an empty group prefab (vanilla Group_Base.et) stays alive until a member joins, and a spawned lone character joins it and takes its waypoints
		{
			SCR_EntityHelper.DeleteEntityAndChildren(group);
			return null;
		}

		return group;
	}

	//------------------------------------------------------------------------------------------------
	//! Undo WrapInGroup: the character leaves the group (it stays where it is) and the group goes.
	static void UnwrapGroup(IEntity character, IEntity group)
	{
		SCR_AIGroup g = SCR_AIGroup.Cast(group);
		if (!g)
			return;

		if (character)
			g.RemoveAIEntityFromGroup(character);

		SCR_EntityHelper.DeleteEntityAndChildren(g);
	}

	//------------------------------------------------------------------------------------------------
	//! A new object of a [BaseContainerProps] class with its [Attribute] defaults, as a prefab would
	//! make it. Use this instead of `new` for any class with attributes: `new` leaves every field
	//! at zero, because attribute fields have no initialisers (an initialiser overrides the value
	//! set in Workbench).
	static Managed NewWithDefaults(string className)
	{
		Resource holder = BaseContainerTools.CreateContainer(className);
		if (!holder || !holder.IsValid())
		{
			RZ_Log.Error("Could not make default " + className);
			return null;
		}

		return BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer());
	}

	//------------------------------------------------------------------------------------------------
	//! The text with its first letter upper-case (for sentences that start with a name or a place).
	static string Capitalize(string text)
	{
		if (text.IsEmpty())
			return text;

		string first = text.Substring(0, 1);
		first.ToUpper();
		return first + text.Substring(1, text.Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! A string-table key (#AR-...) in the server's language; anything else unchanged.
	static string Localize(string text)
	{
		if (!text.StartsWith("#"))
			return text;

		return WidgetManager.Translate(text);
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

		IEntity child = e.GetChildren(); // VERIFY: a weapon's attachments (the suppressor) are child entities of the weapon
		while (child)
		{
			EntityPrefabData data = child.GetPrefabData();
			if (data)
			{
				string name = data.GetPrefabName();
				name.ToLower();
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

		EntityPrefabData data = e.GetPrefabData();
		if (data)
		{
			string prefab = data.GetPrefabName();
			if (!prefab.IsEmpty())
				return prefab;
		}

		VObject obj = e.GetVObject(); // VERIFY: GetVObject().GetResourceName() names the mesh of map objects that aren't prefab instances
		if (obj)
			return obj.GetResourceName();

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Whether a prefab's entity is a vehicle (its entity class is Vehicle or inherits it), without
	//! spawning it. False for an empty or unloadable prefab.
	static bool IsVehiclePrefab(ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return false;

		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return false;

		IEntitySource src = res.GetResource().ToEntitySource(); // VERIFY: a prefab's entity source names its entity class (Vehicle for cars and trucks)
		if (!src)
			return false;

		typename t = src.GetClassName().ToType();
		return t && t.IsInherited(Vehicle);
	}

	//------------------------------------------------------------------------------------------------
	//! True if the FactionManager has a faction with this key.
	static bool HasFaction(string key)
	{
		FactionManager fm = GetGame().GetFactionManager();
		return fm && fm.GetFactionByKey(key) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Key of the faction a character (not the player controller) is affiliated with, "" if none.
	static string GetCharacterFactionKey(IEntity e)
	{
		if (!e)
			return "";

		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(e.FindComponent(FactionAffiliationComponent));
		if (!fac)
			return "";

		Faction f = fac.GetAffiliatedFaction();
		if (!f)
			return "";

		return f.GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	//! Moves only the character onto another faction. The player controller's faction (deploy
	//! menu, chat, groups, map markers) is a different component and stays as it is.
	//! False if the faction isn't in the FactionManager or the character can't be changed.
	static bool SetCharacterFaction(IEntity e, string key)
	{
		FactionManager fm = GetGame().GetFactionManager();
		if (!e || !fm)
			return false;

		Faction f = fm.GetFactionByKey(key);
		if (!f)
			return false;

		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(e.FindComponent(FactionAffiliationComponent));
		if (!fac)
			return false;

		fac.SetAffiliatedFaction(f); // VERIFY: SetAffiliatedFaction on the server replicates, and AI perception reads it
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Every member of an AI group (or one character) onto the faction. Returns how many moved.
	//! Group members only exist a moment after the group, so call it once they're there.
	static int SetUnitFaction(IEntity unit, string key)
	{
		if (!unit)
			return 0;

		SCR_AIGroup group = SCR_AIGroup.Cast(unit);
		if (!group)
		{
			if (SetCharacterFaction(unit, key))
				return 1;

			return 0;
		}

		int moved = 0;
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (agent && SetCharacterFaction(agent.GetControlledEntity(), key))
				moved++;
		}

		return moved;
	}

	//------------------------------------------------------------------------------------------------
	//! True if the character has a firearm in a weapon slot: in their hands, slung or holstered.
	//! Grenades and anything without a firearm weapon type don't count; nor does a gun packed
	//! away in a backpack.
	static bool IsArmed(IEntity character)
	{
		if (!character)
			return false;

		BaseWeaponManagerComponent wm = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		if (!wm)
			return false;

		array<IEntity> weapons = {};
		wm.GetWeaponsList(weapons);
		foreach (IEntity w : weapons)
		{
			if (!w)
				continue;

			BaseWeaponComponent wc = BaseWeaponComponent.Cast(w.FindComponent(BaseWeaponComponent));
			if (wc && IsFirearmType(wc.GetWeaponType()))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsFirearmType(EWeaponType t)
	{
		switch (t)
		{
			case EWeaponType.WT_RIFLE: return true;
			case EWeaponType.WT_SNIPERRIFLE: return true;
			case EWeaponType.WT_MACHINEGUN: return true;
			case EWeaponType.WT_HANDGUN: return true;
			case EWeaponType.WT_GRENADELAUNCHER: return true;
			case EWeaponType.WT_ROCKETLAUNCHER: return true;
		}

		return false;
	}

	// ================================================================ Vehicles and units (supply trucks, retaliation)
	//------------------------------------------------------------------------------------------------
	//! The characters of a unit: a group's members, or the character itself.
	static void GetUnitMembers(IEntity unit, notnull array<IEntity> result)
	{
		if (!unit)
			return;

		SCR_AIGroup group = SCR_AIGroup.Cast(unit);
		if (!group)
		{
			if (ChimeraCharacter.Cast(unit))
				result.Insert(unit);

			return;
		}

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (agent && agent.GetControlledEntity())
				result.Insert(agent.GetControlledEntity());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! True once a unit has all its members: a group whose spawn queue has filled every slot (they
	//! spawn over several frames), or a character. False for nothing.
	static bool HasAllMembers(IEntity unit)
	{
		if (!unit)
			return false;

		SCR_AIGroup group = SCR_AIGroup.Cast(unit);
		if (!group)
			return true;

		return group.GetAgentsCount() > 0 && group.IsExpandComplete(); // VERIFY: IsExpandComplete turns true once SCR_AIWorld's spawn queue has given every member slot a living character
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever sits in a vehicle's driver's seat, or null.
	static IEntity GetDriver(IEntity vehicle)
	{
		Vehicle v = Vehicle.Cast(vehicle);
		if (!v)
			return null;

		return v.GetPilot();
	}

	//------------------------------------------------------------------------------------------------
	//! Seats a character in a vehicle at once: the driver's seat, or a passenger seat. False if
	//! they're already in a seat or getting in, or there's no free seat of that kind.
	static bool BoardVehicle(IEntity character, IEntity vehicle, bool driver)
	{
		if (!character || !vehicle)
			return false;

		SCR_CompartmentAccessComponent cac = SCR_CompartmentAccessComponent.Cast(character.FindComponent(SCR_CompartmentAccessComponent));
		if (!cac || cac.IsInCompartment() || cac.IsGettingIn())
			return false;

		ECompartmentType type = ECompartmentType.CARGO;
		if (driver)
			type = ECompartmentType.PILOT;

		return cac.MoveInVehicle(vehicle, type); // VERIFY: an AI is seated at once (MoveInVehicle sends an RPC to the character's owner, the server for AI); PILOT is the driver's seat
	}

	//------------------------------------------------------------------------------------------------
	//! A vehicle or prop whose damage manager says it's destroyed. False without one.
	static bool IsDestroyed(IEntity e)
	{
		if (!e)
			return true;

		SCR_DamageManagerComponent dmg = SCR_DamageManagerComponent.Cast(e.FindComponent(SCR_DamageManagerComponent));
		if (!dmg)
			return false;

		return dmg.GetState() == EDamageState.DESTROYED;
	}

	//------------------------------------------------------------------------------------------------
	//! Puts a vehicle (and whoever sits in it) somewhere else, stopped. Stuck supply trucks only,
	//! and only with no player near.
	static void MoveVehicle(IEntity vehicle, vector pos, float yaw)
	{
		if (!vehicle)
			return;

		vector mat[4];
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), mat);
		mat[3] = pos + Vector(0, 0.5, 0);
		vehicle.SetWorldTransform(mat); // VERIFY: SetWorldTransform moves a physics vehicle with its crew (else use the vehicle's teleport helper)

		Physics phys = vehicle.GetPhysics();
		if (phys)
			phys.SetVelocity(vector.Zero);
	}

	//------------------------------------------------------------------------------------------------
	//! Puts a player somewhere else (with their vehicle, if they're in one), on the ground at pos,
	//! or at sea level over water (a swimmer or a boat stays afloat). The Perimeter turns people
	//! back from the line with it after the cure. False if it couldn't.
	static bool MovePlayer(int playerId, vector pos)
	{
		vector at = GroundPos(pos);
		if (IsWater(at))
			at[1] = 0;

		return SCR_Global.TeleportPlayer(playerId, at); // VERIFY: SCR_Global.TeleportPlayer on the dedicated server moves the player's character (or their vehicle) for every client, as it does from the editor
	}

	//------------------------------------------------------------------------------------------------
	//! A point at the front of an entity's bounding box (a vehicle's grille), `up` metres above the
	//! bottom of the box and `inset` metres back from its front (negative: in front of it), in world space.
	static vector FrontOf(IEntity e, float up, float inset)
	{
		if (!e)
			return vector.Zero;

		vector boxMin;
		vector boxMax;
		e.GetBounds(boxMin, boxMax); // VERIFY: GetBounds is the model's box in the entity's own space and a vehicle's front is local +Z (vanilla SCR_AIDangerReaction_Vehicle reads GetBounds with CoordToParent the same way); if the spot lands behind the boot, use boxMin[2] + inset
		vector atFront = Vector((boxMin[0] + boxMax[0]) * 0.5, boxMin[1] + up, boxMax[2] - inset);
		return e.CoordToParent(atFront);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets a vehicle's fuel to a share of its tanks (a freshly fixed car). Does nothing without
	//! a fuel manager.
	static void SetVehicleFuel(IEntity vehicle, float share)
	{
		if (!vehicle)
			return;

		FuelManagerComponent fm = FuelManagerComponent.Cast(vehicle.FindComponent(FuelManagerComponent));
		if (!fm)
			return;

		array<BaseFuelNode> nodes = {};
		fm.GetFuelNodesList(nodes);
		foreach (BaseFuelNode node : nodes)
		{
			if (node)
				node.SetFuel(node.GetMaxFuel() * Math.Clamp(share, 0, 1));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A character's health (the same accessor the bite detector uses), -1 without a damage manager.
	static float GetCharacterHealth(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return -1;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (!dmg)
			return -1;

		return dmg.GetHealth();
	}

	//------------------------------------------------------------------------------------------------
	static int RoundToInt(float f)
	{
		int i = Math.Round(f);
		return i;
	}

	// ================================================================ Mission building blocks (Missions/Blocks/)
	//------------------------------------------------------------------------------------------------
	//! The vehicle a character sits in, or null if they're on foot.
	static IEntity GetVehicleOf(IEntity character)
	{
		if (!character || !IsInVehicle(character))
			return null;

		return CompartmentAccessComponent.GetVehicleIn(character);
	}

	//------------------------------------------------------------------------------------------------
	//! A character's full health (the default hit zone's maximum), -1 without a damage manager.
	static float GetCharacterMaxHealth(IEntity e)
	{
		HitZone hz = DefaultHitZone(e);
		if (!hz)
			return -1;

		return hz.GetMaxHealth();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets a character's health (health-cost actions such as the Blood Drive). False without a hit zone.
	static bool SetCharacterHealth(IEntity e, float health)
	{
		HitZone hz = DefaultHitZone(e);
		if (!hz)
			return false;

		hz.SetHealth(Math.Max(1, health)); // VERIFY: HitZone.SetHealth on the server replicates and doesn't knock the character out above zero
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static HitZone DefaultHitZone(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return null;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (!dmg)
			return null;

		return dmg.GetDefaultHitZone();
	}

	//------------------------------------------------------------------------------------------------
	//! "Hunting_Rifle_CR527.et" -> "Hunting Rifle CR527": a readable name from a prefab path.
	static string PrettyPrefabName(string path)
	{
		string name = path;
		int slash = name.LastIndexOf("/");
		if (slash >= 0)
			name = name.Substring(slash + 1, name.Length() - slash - 1);

		int dot = name.LastIndexOf(".");
		if (dot > 0)
			name = name.Substring(0, dot);

		name.Replace("_", " ");
		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! Readable names of what a character wears and carries, for the deploy panel's gear list.
	//! At most `cap`, no repeats.
	static void GearNames(IEntity character, int cap, notnull array<string> result)
	{
		if (!character)
			return;

		array<IEntity> things = {};
		IEntity child = character.GetChildren();
		while (child)
		{
			things.Insert(child);
			child = child.GetSibling();
		}

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inv)
		{
			array<IEntity> items = {};
			inv.GetItems(items);
			foreach (IEntity item : items)
			{
				things.Insert(item);
			}
		}

		foreach (IEntity e : things)
		{
			if (result.Count() >= cap)
				return;

			string pretty = PrettyPrefabName(GetPrefabName(e));
			if (!pretty.IsEmpty() && !result.Contains(pretty))
				result.Insert(pretty);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Lower-case prefab names of what a character wears and carries on them (disguises).
	static void GetWornNames(IEntity character, notnull array<string> result)
	{
		CollectChildNames(character, 0, result);
	}

	//------------------------------------------------------------------------------------------------
	protected static void CollectChildNames(IEntity e, int depth, notnull array<string> result)
	{
		if (!e || depth > 1 || result.Count() > 40)
			return;

		IEntity child = e.GetChildren(); // VERIFY: worn clothing is attached to the character as child entities (else read the loadout storage's slots)
		while (child)
		{
			string name = GetPrefabName(child);
			name.ToLower();
			if (!name.IsEmpty())
				result.Insert(name);

			CollectChildNames(child, depth + 1, result);
			child = child.GetSibling();
		}
	}
}
