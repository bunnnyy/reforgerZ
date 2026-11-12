//! Openings' place roll (docs/DESIGN.md, "Openings"): every life starts at a random place that fits
//! its background, rolled on the server when the character spawns. Nobody picks it.
//!
//! - The kinds of place a background starts at (RZ_OpeningText.KindsFor): a clinic, a garage, a
//!   farm, a police post, a radio mast... found by the building's prefab name (the door slots'
//!   kinds, RZ_DoorSlots.KindOf, and the landmark finder's keywords).
//! - Only in a town nobody holds, by a building with an outside door that isn't locked; the point
//!   400 m from any holding, safehouse, road post or the lab, 600 m from a sealed checkpoint, not
//!   at the line, no player within 300 m, no running mission within 500 m, 1 km from a pack's lair,
//!   2 km from this player's last two starts; infection under 60 for a brand-new player.
//! - Towns within 3 km of where the chain will end (RZ_Openings.EndPoints: the ending faction's
//!   holdings, Halcyon's lab, or the Haven's safehouses) are tried first, so the walk stays short.
//! - The fallback chain: no fitting building in any town tried, any house in one (the scene still
//!   comes); nothing at all, the spawn point the spawn screen asked for (the contact only).
//!
//! The spawn itself stays vanilla's: the spawn screen asks for one of the rotating spawn points
//! (UI/RZ_SpawnMenu.c), and the modded spawn point handler below moves the new character to the
//! rolled place before it's handed over, as vanilla's spawn point moves it onto itself. Server only.

//------------------------------------------------------------------------------------------------
//! Where a life starts: outside the door it came out of.
class RZ_OpeningPlace
{
	vector m_vPos;			// where the player stands, 2 m out from the door
	float m_fYaw;			// facing out of the door
	vector m_vOut;			// straight out of the door's wall (flat, length 1)
	string m_sKind;			// clinic, shop, garage, farm, public, house, police, radio, fuel, school, military
	int m_iTown = -1;
	bool m_bFits;			// one of the background's kinds (false: any house, the first fallback)
	bool m_bScene;			// a door was found: the scene and the contact (false: a spawn point, the contact only)
	IEntity m_Door;			// weak: the door you come out of
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningRoll
{
	protected static const float HOLDING_CLEAR = 400;
	protected static const float CHECKPOINT_CLEAR = 600;
	protected static const float PLAYER_CLEAR = 300;
	protected static const float MISSION_CLEAR = 500;
	protected static const float LAIR_CLEAR = 1000;
	protected static const float LAST_START_CLEAR = 2000;
	protected static const float DOOR_OUT = 2;			// the player stands this far out of the doorway
	protected static const float TOWN_QUERY = 350;		// buildings within this of a town's centre (its radius if smaller)
	protected static const int TOWN_BUILDINGS = 48;
	protected static const int TOWNS_TRIED = 8;
	protected static const float NEW_INFECTION = 60;	// a brand-new player's town is under this
	protected static const float ANY_INFECTION = 80;
	protected static const float NEAR_END = 3000;		// towns this close to where the chain ends go first

	protected RZ_GameModeComponent m_GM;
	protected ref array<IEntity> m_aPlayers = {};

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningRoll(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! A place for this background, by the rules above; null if no town has one. `ends`: where the
	//! chain would end (empty: anywhere).
	RZ_OpeningPlace Roll(string backgroundId, int level, RZ_PlayerProfile profile, notnull array<vector> ends)
	{
		array<int> ids = {};
		m_aPlayers.Clear();
		RZ_Util.GetAlivePlayers(ids, m_aPlayers);
		array<string> kinds = {};
		RZ_OpeningText.KindsFor(backgroundId, kinds);

		float maxInfection = ANY_INFECTION;
		if (level <= 0)
			maxInfection = NEW_INFECTION;

		array<int> towns = {};
		Towns(maxInfection, towns);
		if (towns.IsEmpty() && maxInfection < ANY_INFECTION)
			Towns(ANY_INFECTION, towns);

		if (towns.IsEmpty())
			return null;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		rng.ShuffleInts(towns);
		NearEndsFirst(towns, ends);
		RZ_OpeningPlace fallback;
		int tried = 0;
		foreach (int town : towns)
		{
			if (tried >= TOWNS_TRIED)
				break;

			tried++;
			RZ_OpeningPlace anyHouse;
			RZ_OpeningPlace fits = InTown(town, kinds, profile, rng, anyHouse);
			if (fits)
				return fits;

			if (!fallback && anyHouse)
				fallback = anyHouse;
		}

		return fallback;
	}

	//------------------------------------------------------------------------------------------------
	//! The towns within NEAR_END of any of `ends` move to the front, each part in its shuffled order.
	protected void NearEndsFirst(notnull array<int> towns, notnull array<vector> ends)
	{
		if (ends.IsEmpty())
			return;

		array<int> nearTowns = {};
		array<int> farTowns = {};
		foreach (int town : towns)
		{
			RZ_Settlement s = m_GM.GetSettlement(town);
			bool close = false;
			if (s)
			{
				foreach (vector endAt : ends)
				{
					if (RZ_Util.DistXZ(s.m_vCenter, endAt) <= NEAR_END)
						close = true;
				}
			}

			if (close)
				nearTowns.Insert(town);
			else
				farTowns.Insert(town);
		}

		towns.Clear();
		towns.InsertAll(nearTowns);
		towns.InsertAll(farTowns);
	}

	//------------------------------------------------------------------------------------------------
	//! Towns nobody holds, under the infection given.
	protected void Towns(float maxInfection, notnull array<int> result)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		RZ_Territory territory = m_GM.GetTerritory();
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (s.m_fInfection >= maxInfection || s.m_bHasLab)
				continue;

			if (facts && facts.IsLivedIn(i))
				continue;

			if (territory && territory.FindInTown(i))
				continue;

			result.Insert(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The town's buildings with an outside door at a clear point: one of the background's kinds at
	//! random, else null and `anyHouse` the first house that would do.
	protected RZ_OpeningPlace InTown(int town, array<string> kinds, RZ_PlayerProfile profile, RZ_Rng rng, out RZ_OpeningPlace anyHouse)
	{
		anyHouse = null;
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (!s)
			return null;

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(s.m_vCenter, Math.Min(TOWN_QUERY, Math.Max(120, s.m_fRadius)), buildings, TOWN_BUILDINGS);
		array<ref RZ_OpeningPlace> fits = {};
		foreach (IEntity b : buildings)
		{
			if (!b || RZ_LockedSpots.IsLockedBuilding(b))
				continue;

			string kind = KindOf(b, kinds);
			bool fit = !kind.IsEmpty();
			if (!fit && anyHouse)
				continue;

			if (!fit && RZ_DoorSlots.KindOf(b) != RZ_EDoorWant.HOUSE)
				continue;

			RZ_OpeningPlace p = AtDoor(b, profile);
			if (!p)
				continue;

			p.m_iTown = town;
			if (fit)
			{
				p.m_sKind = kind;
				p.m_bFits = true;
				fits.Insert(p);
			}
			else
			{
				p.m_sKind = "house";
				anyHouse = p;
			}
		}

		if (fits.IsEmpty())
			return null;

		return fits[rng.Int(0, fits.Count())];
	}

	//------------------------------------------------------------------------------------------------
	//! The first of `kinds` this building is, "" if none: the door slots' kinds by prefab name
	//! (clinic, shop, garage, farm, public, house) and the landmark finder's (police, radio, fuel,
	//! school, military).
	static string KindOf(IEntity b, array<string> kinds)
	{
		string name = RZ_Util.GetPrefabName(b);
		name.ToLower();
		int slot = RZ_DoorSlots.KindOf(b);
		foreach (string k : kinds)
		{
			switch (k)
			{
				case "clinic": if (slot == RZ_EDoorWant.CLINIC) return k; break;
				case "shop": if (slot == RZ_EDoorWant.SHOP) return k; break;
				case "garage": if (slot == RZ_EDoorWant.GARAGE) return k; break;
				case "farm": if (slot == RZ_EDoorWant.FARM) return k; break;
				case "public": if (slot == RZ_EDoorWant.PUBLIC) return k; break;
				case "house": if (slot == RZ_EDoorWant.HOUSE) return k; break;
				default: if (RZ_LandmarkFinder.Matches(k, name)) return k; break;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The building's first outside door whose point, DOOR_OUT out, is clear; null if none.
	protected RZ_OpeningPlace AtDoor(IEntity b, RZ_PlayerProfile profile)
	{
		array<IEntity> doors = {};
		RZ_DoorGeometry.FindDoors(b, 0, doors);
		foreach (IEntity door : doors)
		{
			if (!RZ_DoorGeometry.IsOuterDoor(b, door))
				continue;

			vector normal = RZ_DoorGeometry.WallNormal(b, door);
			if (normal == vector.Zero)
				continue;

			vector at = RZ_Util.GroundPos(RZ_DoorGeometry.DoorwayCentre(door) + normal * DOOR_OUT);
			if (!Clear(at, profile))
				continue;

			RZ_OpeningPlace p = new RZ_OpeningPlace();
			p.m_vPos = at;
			p.m_vOut = normal;
			p.m_fYaw = RZ_Compass.Bearing(at, at + normal);
			p.m_bScene = true;
			p.m_Door = door;
			return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Safe enough to stand up in (the rules at the top of this file).
	bool Clear(vector at, RZ_PlayerProfile profile)
	{
		if (RZ_Util.IsWater(at))
			return false;

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		vector checkpoint;
		if (perimeter && (perimeter.IsNearLine(at) || (perimeter.GetNearestCheckpoint(at, checkpoint) && RZ_Util.DistXZ(at, checkpoint) < CHECKPOINT_CLEAR)))
			return false;

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.FindHolding(-1, at, HOLDING_CLEAR))
			return false;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.FindNearest(at, HOLDING_CLEAR) >= 0)
			return false;

		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (posts && posts.PostNear(at, HOLDING_CLEAR))
			return false;

		if (m_GM.HasLab() && RZ_Util.DistXZ(at, m_GM.GetLabPosition()) < HOLDING_CLEAR)
			return false;

		if (RZ_Util.NearestDistance(at, m_aPlayers) < PLAYER_CLEAR)
			return false;

		RZ_MissionManager missions = m_GM.GetMissions();
		if (missions)
		{
			foreach (RZ_Mission m : missions.GetActive())
			{
				if (m && RZ_Util.DistXZ(m.m_vPos, at) < MISSION_CLEAR)
					return false;
			}
		}

		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes && tribes.GetPeople().IsOn() && tribes.GetPeople().NearLair(at, LAIR_CLEAR))
			return false;

		// The Long Table's farm, the evening's set piece, the lab's final stand (the errands' quiet rule)
		RZ_Errands errands = m_GM.GetErrands();
		if (errands && !errands.QuietHere(at))
			return false;

		profile.EnsureArrays();
		for (int i = 0; i + 1 < profile.m_aStarts.Count(); i += 2)
		{
			if (RZ_Util.DistXZ(at, Vector(profile.m_aStarts[i], 0, profile.m_aStarts[i + 1])) < LAST_START_CLEAR)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A start to remember: the last two are kept (x, z pairs, newest first).
	static void NoteStart(RZ_PlayerProfile profile, vector at)
	{
		profile.EnsureArrays();
		profile.m_aStarts.InsertAt(at[2], 0);
		profile.m_aStarts.InsertAt(at[0], 0);
		while (profile.m_aStarts.Count() > 4)
		{
			profile.m_aStarts.Remove(profile.m_aStarts.Count() - 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Moves a character that has just spawned (not yet handed over) to the place, facing out.
	//! VERIFY: SetAngles' y is the yaw in the compass bearing's sense (vanilla's spawn point sets its
	//! own angles the same way in PrepareSpawnedEntity_S).
	static void Move(IEntity character, RZ_OpeningPlace p)
	{
		character.SetOrigin(p.m_vPos + Vector(0, 0.1, 0));
		character.SetAngles(Vector(0, p.m_fYaw, 0));
	}

	//------------------------------------------------------------------------------------------------
	//! The door you came out of stands open behind you.
	//! VERIFY: DoorComponent.SetControlValue(1) on the server opens the door for every client (the
	//! locks shut doors the same way, RZ_DoorGeometry.Close).
	static void OpenDoor(IEntity door)
	{
		DoorComponent dc = RZ_DoorGeometry.GetDoor(door);
		if (dc)
			dc.SetControlValue(1, RplId.Invalid());
	}
}

//------------------------------------------------------------------------------------------------
//! Vanilla's spawn point handler, with one step added: once the character stands on the spawn point
//! the spawn screen asked for, an opening moves it to the rolled place (Guidance/RZ_Openings.c), and
//! the world streams in around that place instead of the spawn point.
modded class SCR_SpawnPointSpawnHandlerComponent
{
	//------------------------------------------------------------------------------------------------
	override protected bool PrepareEntity_S(SCR_SpawnRequestComponent requestComponent, IEntity entity, SCR_SpawnData data)
	{
		if (!super.PrepareEntity_S(requestComponent, entity, data))
			return false;

		RZ_Openings openings = RZ_Openings.Get();
		if (openings && entity && requestComponent)
			openings.PlaceSpawn(requestComponent.GetPlayerId(), entity);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: the preload at the moved character's place (vanilla preloads at data.GetPosition(),
	//! the spawn point) is what the client streams before it's handed the character.
	override void OnFinalizeBegin_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		RZ_Openings openings = RZ_Openings.Get();
		SCR_BaseGameMode gm = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (openings && entity && requestComponent && data && gm && openings.Moved(requestComponent.GetPlayerId()))
		{
			if (!data.GetSkipPreload() && gm.CanStartSpawnPreload())
				requestComponent.StartSpawnPreload(entity.GetOrigin());

			return;
		}

		super.OnFinalizeBegin_S(requestComponent, data, entity);
	}
}
