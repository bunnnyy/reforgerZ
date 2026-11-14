//! The power grid (docs/DESIGN.md, "The power grid"). One power station per map: a placed
//! RZ_PowerStationMarkerEntity, else the map's power plant when towns are in its reach, else real
//! power infrastructure found by prefab name near a town (a power plant, transformer station or
//! substation), else a factory, sawmill or warehouse, else a
//! real building on the edge of the biggest town (World/RZ_PowerSite.c). Its control is the map's
//! own electrical cabinet there, else the control prefab against the station's wall by a door.
//! A group restores it in a short sequence at the control (force the door, set the breakers and
//! hold the area while the generator spins up, throw the main switch): with UI > Power breaker
//! layout set, one prompt, Open the panel, opens the power panel screen where all of it happens
//! (World/RZ_PowerPanel.c, UI/RZ_PowerMenu.c); without it, the old world prompts. Then the towns
//! in its region:
//!
//! - are Powered (RZ_TownMemory), the hook for lights (IsPoweredAt), and the map's own lamps there
//!   light at night while every other town's stay dark (Street lamps; UI/RZ_PowerLampsClient.c),
//! - get working radios: faction chatter comes more often (ChatterMult), and the radio
//!   station can broadcast twice as often,
//! - give an electronics loot bonus to searches (LootMult, RZ_Search),
//! - but the generator hums: steady heat at the station, which draws hordes.
//!
//! A power surge (the Power Surge event) brings one town outside the region back for a few
//! minutes: Powered, lights, and the electronics bonus, with no generator to keep running.
//!
//! It slowly fails (about four hours with players online) unless someone maintains it.
//! The control object only exists while a player is near, and so do the station's dressing (fuel
//! drums and a sign beside it) and the running generator's sound (World/RZ_PowerLooks.c). Its state
//! is saved.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Power grid.
[BaseContainerProps()]
class RZ_PowerSettings
{
	[Attribute("1", desc: "A power station players can restore (docs/DESIGN.md, Time and environment)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Control object: a vanilla electrical cabinet that stands on the ground, with RZ_PuzzleObjectComponent, RZ_ClueComponent, RZ_ReadClueAction and 2 RZ_PuzzleChoiceAction (options 0-1). Stood against the station's wall by a door. Empty = Puzzles > Fuse box, then the generic object", "et")]
	ResourceName m_sControlPrefab;

	[Attribute("1", desc: "The control is the map's own electrical cabinet at the station when there is one within 45 m: an invisible Interactions > Spot prefab on its face, no prop of ours. Needs the Spot prefab set")]
	bool m_bUseCabinets;

	[Attribute("0", desc: "Degrees added to the control prefab's turn, if its front doesn't face out from the wall")]
	float m_fControlYaw;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Generator sound (optional): a prefab with no mesh that plays a diesel generator by itself (vanilla's DieselGenerator_01 sounds on a StaticSoundComponent). Up at the station while the generator turns and a player is near: at the map's own generator sets there, else at the control. Empty = silent", "et")]
	ResourceName m_sGeneratorSoundPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Dressing (optional): a composition of look-only props (fuel drums, a jerrycan, a warning sign) stood beside the control while a player is near, its front (+Z) facing out from the wall. Empty = none", "et")]
	ResourceName m_sDressingPrefab;

	[Attribute("1", desc: "Street lamps: the map's own lamps (every lamp that lights itself at night) stay dark, except in towns with power from the station or a power surge. Off = the lamps are left as the map has them")]
	bool m_bStreetLamps;

	[Attribute("2500", desc: "Towns within this distance (m) of the station get the power")]
	float m_fRegionRadius;

	[Attribute("90", desc: "Seconds the generator needs to spin up, with someone within 60 m")]
	float m_fSpinUpSeconds;

	[Attribute("10", desc: "Heat per minute the running station makes (it draws hordes)")]
	float m_fNoisePerMinute;

	[Attribute("4", desc: "Hours (with players online) a fully maintained station runs before it fails")]
	float m_fRunHours;

	[Attribute("0.6", desc: "Maintenance with a fuel item (jerrycan) restores this much of its condition (0..1)")]
	float m_fFuelRestore;

	[Attribute("0.25", desc: "Maintenance without fuel (grease, patching) restores this much")]
	float m_fPatchRestore;

	[Attribute("1.5", desc: "Search hook: electronics loot multiplier in powered towns")]
	float m_fElectronicsLootMult;

	[Attribute("0.6", desc: "Faction radio chatter interval multiplier while the power is on (working radios)")]
	float m_fChatterMult;

	[Attribute(desc: "Substations: the map's transformer yards past the station's region, switched on in person; lines that break; the Red Hand's tolls; sabotage (docs/DESIGN.md, The grid, region by region)")]
	ref RZ_SubstationSettings m_Substations;
}

//------------------------------------------------------------------------------------------------
class RZ_PowerSave
{
	float x;
	float z;
	string name;
	int step;
	float condition;
	ref RZ_SubstationsSave substations;
}

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "ReforgerZ", description: "The power station players can restore. If one is placed, the station isn't picked automatically.")]
class RZ_PowerStationMarkerEntityClass : GenericEntityClass
{
}

//! Optional. Put one where the region's power station is. Without it, RZ_PowerGrid picks a
//! power or industrial building near a town by prefab name, else the edge of the biggest town.
class RZ_PowerStationMarkerEntity : GenericEntity
{
	[Attribute("", desc: "Name (empty = \"the power station outside <nearest town>\")")]
	protected string m_sName;

	protected static ref array<RZ_PowerStationMarkerEntity> s_aAll;

	//------------------------------------------------------------------------------------------------
	static array<RZ_PowerStationMarkerEntity> GetAll()
	{
		if (!s_aAll)
			s_aAll = new array<RZ_PowerStationMarkerEntity>();

		return s_aAll;
	}

	string GetMarkerName() { return m_sName; }

	//------------------------------------------------------------------------------------------------
	void RZ_PowerStationMarkerEntity(IEntitySource src, IEntity parent)
	{
		GetAll().Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_PowerStationMarkerEntity()
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);
	}
}

//------------------------------------------------------------------------------------------------
//! The power station's steps (the same values as RZ_PowerGrid.STEP_*), for switch statements.
enum RZ_EPowerStep
{
	DARK,
	OPEN,
	SPINNING,
	READY,
	POWERED
}

//------------------------------------------------------------------------------------------------
class RZ_PowerGrid
{
	static const int TAG = -7;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id

	static const int STEP_DARK = 0;			// locked up
	static const int STEP_OPEN = 1;			// control room open
	static const int STEP_SPINNING = 2;		// generator spinning up: hold the area
	static const int STEP_READY = 3;		// ready to throw the main switch
	static const int STEP_POWERED = 4;

	protected static const float VISIBLE_RADIUS = 300;
	protected static const float DESPAWN_MARGIN = 50;
	protected static const float HOLD_RADIUS = 60;
	protected static const float STALL_SECONDS = 30;
	protected static const float MAINTAIN_COOLDOWN = 600;
	protected static const float WARN_AT = 0.2;
	protected static const float POWER_REACH = 500;	// power infrastructure this far (m) past a town's edge counts
	protected static const int POWER_TOWNS = 12;		// biggest towns looked around for a station
	protected static const int PLANT_TOWNS = 3;			// a power plant away from them is the station with this many towns in its region
	protected static const float CABINET_REACH = 45;	// a vanilla cabinet this close (m) to the station carries the control
	protected static const float ANCHOR_REACH = 60;		// a marker's or a saved site's building this close (m)
	protected static const float LAMP_MARGIN = 150;		// lamps this far (m) past a powered town's edge light too
	protected static const float LAMP_STATION = 200;	// and the station's own, this far (m) round it
	protected static const string READ_LABEL = "Read the panel";	// the old world prompts' read
	protected static const string OPEN_LABEL = "Open the panel";		// the one prompt that opens the panel

	protected RZ_GameModeComponent m_GM;
	protected RZ_PowerSettings m_Settings;
	protected ref RZ_PowerSave m_Restored;
	protected bool m_bActive;
	protected vector m_vPos;
	protected string m_sName;
	protected int m_iStep;
	protected float m_fCondition;
	protected float m_fSpinLeft;
	protected float m_fStall;
	protected float m_fClock;
	protected bool m_bWarned;
	protected IEntity m_Body;
	protected RZ_PuzzleObjectComponent m_Comp;
	protected vector m_vControlPos;
	protected float m_fControlYaw;
	protected bool m_bControlSpot;				// on a vanilla cabinet: the interaction spot, not the control prefab
	protected string m_sPlacedHow = "as placed";
	protected ref RZ_PowerPanel m_Panel;
	protected ref RZ_PowerLooks m_Looks;
	protected ref array<int> m_aRegion = {};
	protected ref map<string, float> m_mMaintained = new map<string, float>();
	protected int m_iSurgeTown = -1;			// the Power Surge event's town, -1 = none
	protected vector m_vSurgePos;
	protected float m_fSurgeRadius;
	protected ref RZ_Substations m_Subs;

	//------------------------------------------------------------------------------------------------
	void RZ_PowerGrid(RZ_GameModeComponent gm, RZ_PowerSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		if (m_Settings && !m_Settings.m_Substations)
			m_Settings.m_Substations = RZ_SubstationSettings.Cast(RZ_Util.NewWithDefaults("RZ_SubstationSettings"));

		if (m_Settings)
			m_Subs = new RZ_Substations(gm, this, m_Settings.m_Substations);
	}

	//------------------------------------------------------------------------------------------------
	//! The substations past the station's region (World/RZ_Substations.c).
	RZ_Substations GetSubstations() { return m_Subs; }

	//------------------------------------------------------------------------------------------------
	//! The station's lamps again, after a substation's towns changed.
	void RefreshLamps() { PushLamps(); }

	//------------------------------------------------------------------------------------------------
	//! For the substations: may the map's cabinets carry a control, and what stands where none does.
	bool CanUseCabinetsPublic() { return CanUseCabinets(); }

	//------------------------------------------------------------------------------------------------
	//! Test only: the station's region set to `radius` m (and its towns with it); returns the old one.
	float TestRegion(float radius)
	{
		float old = m_Settings.m_fRegionRadius;
		if (m_bActive)
			ApplyPower(false);

		m_Settings.m_fRegionRadius = radius;
		BuildRegion();
		if (m_bActive)
			ApplyPower(IsPowered());

		return old;
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: the station running (or dark) at once, its towns and lamps with it.
	void TestPower(bool on)
	{
		if (!m_bActive)
			return;

		if (on)
		{
			SetStep(STEP_POWERED);
			m_fCondition = 1;
		}
		else
		{
			SetStep(STEP_OPEN);
		}

		ApplyPower(on);
		PushLamps();
	}
	ResourceName GetControlPrefab() { return ControlPrefab(); }

	//------------------------------------------------------------------------------------------------
	bool IsActive() { return m_bActive; }
	bool IsPowered() { return m_bActive && m_iStep == STEP_POWERED; }
	vector GetPosition() { return m_vPos; }
	string GetName() { return m_sName; }
	int GetStep() { return m_iStep; }
	float GetSpinLeft() { return Math.Max(0, m_fSpinLeft); }
	float GetSpinTotal() { return m_Settings.m_fSpinUpSeconds; }
	float GetCondition() { return m_fCondition; }
	IEntity GetBody() { return m_Body; }
	RZ_PowerPanel GetPanel() { return m_Panel; }

	//------------------------------------------------------------------------------------------------
	protected static RZ_PowerGrid Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return null;

		return gm.GetPowerGrid();
	}

	// ================================================================ Hooks
	//------------------------------------------------------------------------------------------------
	//! Hook for lights and radios: is there power at pos right now? The grid's region, or a
	//! running generator close by (world interactions, RZ_Traps).
	static bool IsPoweredAt(vector pos)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (hub && hub.GetTraps().IsLitAt(pos))
			return true;

		return IsGridPoweredAt(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! The grid only: is the town at pos in the power station's region while it runs, or in a
	//! town a power surge brought back?
	static bool IsGridPoweredAt(vector pos)
	{
		RZ_PowerGrid grid = Get();
		if (!grid)
			return false;

		if (grid.m_iSurgeTown >= 0 && RZ_Util.DistXZ(pos, grid.m_vSurgePos) <= grid.m_fSurgeRadius)
			return true;

		if (!grid.IsPowered())
			return false;

		// Past the region: a town on a switched-on substation whose line is whole
		if (grid.m_Subs && grid.m_Subs.LitAt(pos))
			return true;

		return RZ_Util.DistXZ(pos, grid.m_vPos) <= grid.m_Settings.m_fRegionRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for the search system: a loot multiplier for a category at pos ("electronics" gets
	//! the bonus in towns the grid powers; everything else 1). A generator doesn't count.
	static float LootMult(vector pos, string category)
	{
		if (category != "electronics" || !IsGridPoweredAt(pos))
			return 1;

		RZ_PowerGrid grid = Get();
		return grid.m_Settings.m_fElectronicsLootMult;
	}

	//------------------------------------------------------------------------------------------------
	//! In the grid's region while it runs (a surge picks a town outside it).
	bool IsInRegion(int townIdx)
	{
		return m_aRegion.Contains(townIdx);
	}

	//------------------------------------------------------------------------------------------------
	//! The Power Surge event: the town's power is on until EndSurge. One town at a time.
	void StartSurge(int townIdx, vector center, float radius)
	{
		EndSurge();
		m_iSurgeTown = townIdx;
		m_vSurgePos = center;
		m_fSurgeRadius = radius;
		RZ_TownMemory mem = m_GM.GetTownMemory();
		if (mem)
			mem.SetPowered(townIdx, true);

		PushLamps();
	}

	//------------------------------------------------------------------------------------------------
	//! The surge is over: dark again, unless the grid powers that town.
	void EndSurge()
	{
		if (m_iSurgeTown < 0)
			return;

		RZ_TownMemory mem = m_GM.GetTownMemory();
		if (mem && !(IsPowered() && m_aRegion.Contains(m_iSurgeTown)))
			mem.SetPowered(m_iSurgeTown, false);

		m_iSurgeTown = -1;
		PushLamps();
	}

	//------------------------------------------------------------------------------------------------
	//! Faction chatter interval multiplier: working radios while the power is on.
	float ChatterMult()
	{
		if (!IsPowered())
			return 1;

		return Math.Clamp(m_Settings.m_fChatterMult, 0.2, 1);
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_PowerSave saved)
	{
		m_Restored = saved;
		if (saved && m_Subs)
			m_Subs.Restore(saved.substations);
	}

	//------------------------------------------------------------------------------------------------
	RZ_PowerSave Save()
	{
		RZ_PowerSave data = new RZ_PowerSave();
		data.x = m_vPos[0];
		data.z = m_vPos[2];
		data.name = m_sName;
		data.step = m_iStep;
		if (data.step == STEP_SPINNING || data.step == STEP_READY)
			data.step = STEP_OPEN; // a restart stops the generator mid spin-up

		data.condition = m_fCondition;
		if (m_Subs && m_Subs.IsOn())
			data.substations = m_Subs.Save();

		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After settlements, the Perimeter and town memory exist.
	void Setup()
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
		{
			RZ_Log.Info("Power grid is off.");
			return;
		}

		m_Panel = new RZ_PowerPanel(m_GM, this);
		m_Looks = new RZ_PowerLooks(m_Settings);
		string from = "picked";
		IEntity anchor;
		bool placed;
		array<RZ_PowerStationMarkerEntity> markers = RZ_PowerStationMarkerEntity.GetAll();
		// A station still dark when it was saved is picked again: the old pick may have been a field
		bool keepSave = m_Restored && m_Restored.name != "" && m_Restored.step != STEP_DARK;
		if (!markers.IsEmpty() && markers[0])
		{
			m_vPos = RZ_Util.GroundPos(markers[0].GetOrigin());
			m_sName = markers[0].GetMarkerName();
			from = "marker";
			anchor = RZ_PowerSite.AnchorNear(m_vPos, ANCHOR_REACH);
			if (!anchor)
			{
				// Placed by hand with nothing around it: the control stands at the marker, as turned
				vector angles = markers[0].GetAngles(); // rotation around X, Y, Z in degrees: [1] is the yaw
				m_vControlPos = m_vPos;
				m_fControlYaw = angles[1] + m_Settings.m_fControlYaw;
				placed = true;
				// The dressing beside it, facing the way the marker does
				float turn = angles[1] * Math.DEG2RAD;
				RZ_PowerPlacement beside = new RZ_PowerPlacement();
				RZ_PowerSite.DressBeside(m_vPos, Vector(Math.Sin(turn), 0, Math.Cos(turn)), null, beside);
				if (beside.m_bDress)
					m_Looks.SetDressing(beside.m_vDressPos, beside.m_fDressYaw);
				else
					m_Looks.SetNoDressing(beside.m_sDressWhy);
			}
		}
		else if (keepSave)
		{
			m_vPos = RZ_Util.GroundPos(Vector(m_Restored.x, 0, m_Restored.z));
			m_sName = m_Restored.name;
			from = "save";
			anchor = RZ_PowerSite.AnchorNear(m_vPos, ANCHOR_REACH);
		}
		else
		{
			anchor = PickSite();
			if (m_sName.IsEmpty())
			{
				RZ_Log.Info("Power grid: no place for a power station.");
				return;
			}
		}

		if (m_sName.IsEmpty())
			m_sName = "the power station outside " + NearTown(m_vPos);

		if (!placed)
			PlaceControl(anchor);

		PlaceSounds();

		if (m_Restored)
		{
			m_iStep = Math.Clamp(m_Restored.step, STEP_DARK, STEP_POWERED);
			m_fCondition = Math.Clamp(m_Restored.condition, 0, 1);
		}

		m_Restored = null;
		m_bActive = true;
		BuildRegion();
		if (m_Subs)
			m_Subs.Setup(m_vPos, m_Settings.m_fRegionRadius);

		ApplyPower(m_iStep == STEP_POWERED);
		PushLamps();
		RZ_Log.Info(string.Format("Power grid: %1 at grid %2 (%3), %4 towns in its region, %5. Control %6; %7.",
			m_sName, RZ_SettlementScanner.GridReference(m_vPos), from, m_aRegion.Count(), StepName(), m_sPlacedHow, m_Looks.Describe()));
	}

	//------------------------------------------------------------------------------------------------
	//! Where the running generator is heard from: the map's own generator sets at the station (a
	//! power plant has two), else the control.
	protected void PlaceSounds()
	{
		array<vector> spots = {};
		array<float> yaws = {};
		RZ_PowerSite.Generators(m_vPos, ANCHOR_REACH, spots, yaws);
		foreach (int i, vector spot : spots)
		{
			m_Looks.AddSound(spot, yaws[i]);
		}

		if (spots.IsEmpty())
			m_Looks.AddSound(m_vControlPos, m_fControlYaw);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the control stands at the station anchored on `anchor` (World/RZ_PowerSite.c).
	protected void PlaceControl(IEntity anchor)
	{
		vector toward = m_vPos;
		RZ_Settlement town = m_GM.GetSettlement(m_GM.FindNearestSettlement(m_vPos));
		if (town)
			toward = town.m_vCenter;

		RZ_PowerPlacement p = new RZ_PowerPlacement();
		RZ_PowerSite.Place(anchor, m_vPos, toward, CanUseCabinets(), CABINET_REACH, p);
		m_vControlPos = p.m_vPos;
		m_fControlYaw = p.m_fYaw + m_Settings.m_fControlYaw;
		m_bControlSpot = p.m_bSpot;
		m_sPlacedHow = p.m_sHow;
		if (p.m_bDress)
			m_Looks.SetDressing(p.m_vDressPos, p.m_fDressYaw);
		else
			m_Looks.SetNoDressing(p.m_sDressWhy);
	}

	//------------------------------------------------------------------------------------------------
	//! A vanilla cabinet may carry the control: allowed, and a real (invisible) spot prefab is set.
	protected bool CanUseCabinets()
	{
		RZ_Interactions hub = m_GM.GetInteractions();
		if (!m_Settings.m_bUseCabinets || !hub)
			return false;

		ResourceName spot = hub.GetSpotPrefab();
		return !spot.IsEmpty() && spot != m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);
	}

	//------------------------------------------------------------------------------------------------
	//! The map's power plant, wherever it stands, when at least three towns are in its region (the
	//! one with the most). Else real power infrastructure near a town (biggest towns first: a power
	//! plant beats a substation, a substation a transformer yard), else a factory, sawmill or
	//! warehouse, else a real building on the edge of the biggest town. Sets m_vPos and m_sName;
	//! returns the station's building or prop, null for a bare point (a map with no buildings in its
	//! biggest town).
	protected IEntity PickSite()
	{
		array<ref RZ_Settlement> list = m_GM.GetSettlements();
		array<RZ_Settlement> ordered = {};
		for (int size = 3; size >= 1; size--)
		{
			foreach (RZ_Settlement s : list)
			{
				if (s.m_iSize == size)
					ordered.Insert(s);
			}
		}

		if (ordered.IsEmpty())
			return null;

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		IEntity plant = PickPlant(perimeter);
		if (plant)
			return plant;

		IEntity best;
		int bestScore;
		RZ_Settlement bestTown;
		for (int mode = RZ_PowerSite.MODE_POWER; mode <= RZ_PowerSite.MODE_INDUSTRY; mode++)
		{
			float reach = 200;
			if (mode == RZ_PowerSite.MODE_POWER)
				reach = POWER_REACH;

			foreach (int i, RZ_Settlement town : ordered)
			{
				if (i >= POWER_TOWNS)
					break;

				int score;
				IEntity e = RZ_PowerSite.FindBest(town.m_vCenter, town.m_fRadius + reach, mode, score);
				if (!e || (perimeter && perimeter.IsNearLine(e.GetOrigin())) || score <= bestScore)
					continue;

				best = e;
				bestScore = score;
				bestTown = town;

				// A power plant: no need to look further
				if (mode == RZ_PowerSite.MODE_POWER && score >= 40)
					break;
			}

			if (best)
			{
				m_vPos = RZ_Util.GroundPos(best.GetOrigin());
				bool inTown = RZ_Util.DistXZ(m_vPos, bestTown.m_vCenter) < bestTown.m_fRadius;
				m_sName = RZ_PowerSite.NameFor(mode, bestScore, bestTown.m_sName, inTown);
				return best;
			}
		}

		// No station or works anywhere: a switch room in a real building on the biggest town's edge
		RZ_Settlement biggest = ordered[0];
		IEntity edge = RZ_PowerSite.EdgeBuilding(biggest);
		if (edge)
		{
			m_vPos = RZ_Util.GroundPos(edge.GetOrigin());
			m_sName = "the switch room on the edge of " + biggest.m_sName;
			return edge;
		}

		m_vPos = RZ_Mission.LandPointNear(biggest.m_vCenter, biggest.m_fRadius, biggest.m_fRadius + 80);
		m_sName = "the substation outside " + biggest.m_sName;
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A power plant anywhere on the map, off the line, with the most towns within the region's
	//! reach of it (PLANT_TOWNS at least): a real plant beats a substation in a bigger town. One
	//! with fewer is named in the log and left. Sets m_vPos and m_sName; null if there's none.
	protected IEntity PickPlant(RZ_Perimeter perimeter)
	{
		array<IEntity> plants = {};
		RZ_PowerSite.FindPlants(plants);
		IEntity best;
		int bestTowns;
		foreach (IEntity plant : plants)
		{
			if (!plant || (perimeter && perimeter.IsNearLine(plant.GetOrigin())))
				continue;

			int towns = 0;
			foreach (RZ_Settlement s : m_GM.GetSettlements())
			{
				if (RZ_Util.DistXZ(s.m_vCenter, plant.GetOrigin()) <= m_Settings.m_fRegionRadius)
					towns++;
			}

			if (towns < PLANT_TOWNS)
			{
				RZ_Log.Info(string.Format("Power grid: the power plant at grid %1 has %2 town(s) within its reach (%3 needed): not the station.",
					RZ_SettlementScanner.GridReference(plant.GetOrigin()), towns, PLANT_TOWNS));
				continue;
			}

			if (towns > bestTowns)
			{
				best = plant;
				bestTowns = towns;
			}
		}

		if (!best)
			return null;

		// Named for the nearest place with a name of its own (the plant's own yard is often a cluster
		// of buildings the scan calls "Outskirts" and a grid reference)
		m_vPos = RZ_Util.GroundPos(best.GetOrigin());
		RZ_Settlement near = m_GM.GetSettlement(m_GM.FindNearestSettlement(m_vPos));
		float nearDist = float.MAX;
		foreach (RZ_Settlement named : m_GM.GetSettlements())
		{
			float d = RZ_Util.DistXZ(named.m_vCenter, m_vPos);
			if (named.m_sName.IndexOf("Outskirts ") != 0 && d < nearDist)
			{
				near = named;
				nearDist = d;
			}
		}

		if (near)
			m_sName = RZ_PowerSite.NameFor(RZ_PowerSite.MODE_POWER, 40, near.m_sName, RZ_Util.DistXZ(m_vPos, near.m_vCenter) < near.m_fRadius);

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildRegion()
	{
		m_aRegion.Clear();
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (RZ_Util.DistXZ(s.m_vCenter, m_vPos) <= m_Settings.m_fRegionRadius)
				m_aRegion.Insert(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string NearTown(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! "Morton, Levie and Tyrone"
	protected string RegionNames()
	{
		string text = "";
		int shown = Math.Min(m_aRegion.Count(), 4);
		for (int i = 0; i < shown; i++)
		{
			RZ_Settlement s = m_GM.GetSettlement(m_aRegion[i]);
			if (!s)
				continue;

			if (i > 0 && i == shown - 1)
				text += " and ";
			else if (i > 0)
				text += ", ";

			text += s.m_sName;
		}

		if (text.IsEmpty())
			return "the towns around it";

		return text;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bActive)
			return;

		m_fClock += dt;
		if (m_Subs)
			m_Subs.Tick(dt, ids, players);

		UpdateBody(players);
		float reach = RZ_Util.NearestDistance(m_vPos, players);
		m_Looks.Sync(reach <= VISIBLE_RADIUS, reach > VISIBLE_RADIUS + DESPAWN_MARGIN, m_iStep >= STEP_SPINNING);

		if (m_iStep == STEP_SPINNING)
			TickSpinUp(dt, players);

		if (m_iStep != STEP_POWERED)
			return;

		// The generator hums: heat that draws hordes to the station
		m_GM.GetHeat().AddHeat(m_vPos, m_Settings.m_fNoisePerMinute * dt / 60);

		// It wears out while people are around to use the power
		if (ids.IsEmpty())
			return;

		m_fCondition = m_fCondition - dt / Math.Max(600, m_Settings.m_fRunHours * 3600);
		if (!m_bWarned && m_fCondition <= WARN_AT)
		{
			m_bWarned = true;
			NotifyRegion("The lights are flickering", string.Format("The lights in %1 dip and come back, dip and come back. Someone needs to see to %2 before it quits.", RegionNames(), m_sName));
		}

		if (m_fCondition <= 0)
			Fail();
	}

	//------------------------------------------------------------------------------------------------
	//! Someone has to stay within 60 m while the generator spins up. It's loud.
	protected void TickSpinUp(float dt, array<IEntity> players)
	{
		m_GM.GetHeat().AddHeat(m_vPos, 2 * dt / 5);
		if (RZ_Util.NearestDistance(m_vPos, players) <= HOLD_RADIUS)
		{
			m_fStall = 0;
			m_fSpinLeft = m_fSpinLeft - dt;
			if (m_fSpinLeft <= 0)
			{
				SetStep(STEP_READY);
				NotifyNear(m_vPos, "Power station", "The generator settles into a steady roar. The main switch is live.");
			}

			return;
		}

		m_fStall += dt;
		if (m_fStall >= STALL_SECONDS)
		{
			SetStep(STEP_OPEN);
			NotifyNear(m_vPos, "Power station", "With nobody feeding it, the generator coughs, shudders and dies.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetStep(int step)
	{
		m_iStep = step;
		if (step <= STEP_OPEN)
			m_Panel.Reset();

		UpdateLabels();
		m_Panel.Refresh();
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyPower(bool on)
	{
		RZ_TownMemory mem = m_GM.GetTownMemory();
		if (!mem)
			return;

		foreach (int idx : m_aRegion)
		{
			mem.SetPowered(idx, on);
		}

		// The substations' towns follow the station
		if (m_Subs && m_Subs.IsOn())
			m_Subs.ApplyTowns();
	}

	//------------------------------------------------------------------------------------------------
	//! Street lamps: where the map's own lamps may light, for every client (UI/RZ_PowerLampsClient.c).
	//! "L" and then ";x z radius" for the station and each town it powers, and a surge's town; "L"
	//! alone = dark everywhere; "" = the lamps are left alone (Street lamps off).
	protected void PushLamps()
	{
		if (!m_bActive || !m_Settings.m_bStreetLamps)
		{
			m_GM.SetPowerLamps("");
			return;
		}

		string list = "L";
		if (IsPowered())
		{
			list += LampCircle(m_vPos, LAMP_STATION);
			foreach (int idx : m_aRegion)
			{
				RZ_Settlement s = m_GM.GetSettlement(idx);
				if (s)
					list += LampCircle(s.m_vCenter, s.m_fRadius + LAMP_MARGIN);
			}

			if (m_Subs)
				list += m_Subs.LampCircles();
		}

		if (m_iSurgeTown >= 0)
			list += LampCircle(m_vSurgePos, m_fSurgeRadius + LAMP_MARGIN);

		m_GM.SetPowerLamps(list);
	}

	//------------------------------------------------------------------------------------------------
	protected static string LampCircle(vector center, float radius)
	{
		return string.Format(";%1 %2 %3", RZ_Util.RoundToInt(center[0]), RZ_Util.RoundToInt(center[2]), RZ_Util.RoundToInt(radius));
	}

	//------------------------------------------------------------------------------------------------
	protected void Restored(int playerId)
	{
		SetStep(STEP_POWERED);
		m_fCondition = 1;
		m_bWarned = false;
		ApplyPower(true);
		PushLamps();
		string text = string.Format("Somebody got %1 running. There are lights in %2 tonight, and the radios have something to run on. Everything for a mile can hear the generator.", m_sName, RegionNames());
		m_GM.NotifyAll("Power", text);
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(string.Format("The power is back in %1. %2 is running again, and there are lights in the windows tonight.", RegionNames(), m_sName), false);

		RZ_Log.Info("Power grid: restored by player " + playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Fail()
	{
		SetStep(STEP_OPEN);
		m_fCondition = 0;
		ApplyPower(false);
		PushLamps();
		NotifyRegion("Power", string.Format("%1 has quit. The lights in %2 are out again.", m_sName, RegionNames()));
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(string.Format("The power failed in %1. Nobody kept %2 going, and the towns on it are dark again.", RegionNames(), m_sName), false);
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyNear(vector pos, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= VISIBLE_RADIUS)
				m_Panel.Tell(ids[i], title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyRegion(string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) <= m_Settings.m_fRegionRadius)
				m_GM.NotifyPlayer(ids[i], title, text);
		}
	}

	// ================================================================ The control object
	//------------------------------------------------------------------------------------------------
	protected ResourceName ControlPrefab()
	{
		if (!m_Settings.m_sControlPrefab.IsEmpty())
			return m_Settings.m_sControlPrefab;

		return m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.FUSEBOX);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBody(array<IEntity> players)
	{
		float nearest = RZ_Util.NearestDistance(m_vPos, players);
		if (m_Body && nearest > VISIBLE_RADIUS + DESPAWN_MARGIN)
		{
			m_Panel.CloseAll("gone");
			RZ_Util.DeleteUnit(m_Body);
			m_Body = null;
			m_Comp = null;
			return;
		}

		if (m_Body || nearest > VISIBLE_RADIUS)
			return;

		ResourceName prefab = ControlPrefab();
		if (m_bControlSpot)
			prefab = m_GM.GetInteractions().GetSpotPrefab();

		m_Body = RZ_Util.SpawnPrefab(prefab, m_vControlPos, m_fControlYaw);
		if (!m_Body)
			return;

		m_Comp = RZ_PuzzleObjectComponent.Cast(m_Body.FindComponent(RZ_PuzzleObjectComponent));
		UpdateLabels();
	}

	//------------------------------------------------------------------------------------------------
	//! The old world prompts (no panel screen): one choice per step. "|" (every slot labelled empty)
	//! hides them all while the generator spins up; "" would show every slot unlabelled ("Use").
	protected string ChoiceLabels()
	{
		switch (m_iStep)
		{
			case RZ_EPowerStep.DARK: return "Force the control room door";
			case RZ_EPowerStep.OPEN: return "Reset the breakers and start the generator";
			case RZ_EPowerStep.SPINNING: return "|";
			case RZ_EPowerStep.READY: return "Throw the main switch";
			case RZ_EPowerStep.POWERED: return "|Refuel and grease the generator";
		}

		return "|";
	}

	//------------------------------------------------------------------------------------------------
	//! With the panel screen: one prompt, Open the panel, and every slot hidden. Without it: the read
	//! and the step's choice, as before. Never the object menu (the panel is the menu).
	protected void UpdateLabels()
	{
		if (!m_Body)
			return;

		string read = READ_LABEL;
		string choices = ChoiceLabels();
		if (m_Panel.IsUsed())
		{
			read = OPEN_LABEL;
			choices = "|";
		}

		if (m_Comp)
		{
			m_Comp.SetMenu(false);
			m_Comp.SetLabels("", choices);
		}

		RZ_ClueComponent.Tag(m_Body, TAG, 0, read);
	}

	//------------------------------------------------------------------------------------------------
	//! A client couldn't open the panel (RZ_PowerPanel): the old prompts come back, and this player
	//! is told what they see.
	void OnPanelFailed(int playerId)
	{
		UpdateLabels();
		m_GM.NotifyPlayer(playerId, "Power station", Description());
	}

	//------------------------------------------------------------------------------------------------
	//! The panel's line under its title.
	string StateLine()
	{
		switch (m_iStep)
		{
			case RZ_EPowerStep.DARK: return "Dark. The control room is locked.";
			case RZ_EPowerStep.OPEN: return "Dark. The control room is open.";
			case RZ_EPowerStep.SPINNING: return "The generator is spinning up.";
			case RZ_EPowerStep.READY: return "Ready. The main switch is live.";
			case RZ_EPowerStep.POWERED: return string.Format("Running. Powering %1.", RegionNames());
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	string StepName()
	{
		switch (m_iStep)
		{
			case RZ_EPowerStep.DARK: return "dark, locked up";
			case RZ_EPowerStep.OPEN: return "dark, control room open";
			case RZ_EPowerStep.SPINNING: return "spinning up";
			case RZ_EPowerStep.READY: return "ready to switch on";
			case RZ_EPowerStep.POWERED: return string.Format("running (%1 percent)", RZ_Util.RoundToInt(m_fCondition * 100));
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_ReadClueAction on the control: Open the panel opens the panel screen; without it (or
	//! when it can't open), what you see as a notice.
	void HandleRead(int playerId, IEntity owner)
	{
		if (m_Panel.IsUsed() && m_Panel.Open(playerId, owner))
			return;

		m_GM.NotifyPlayer(playerId, "Power station", Description());
	}

	//------------------------------------------------------------------------------------------------
	//! What you see at the control, by step.
	string Description()
	{
		string text;
		switch (m_iStep)
		{
			case RZ_EPowerStep.DARK: text = "A steel door with a padlock and a sign: DANGER - HIGH VOLTAGE. Through the louvres you can see a diesel generator and a wall of breakers. It looks like it could run, if someone made the noise to start it."; break;
			case RZ_EPowerStep.OPEN: text = "Breakers all tripped. A card inside the door says how they go back. The generator has fuel in it. Starting it will be loud, and someone has to stay with it until it settles."; break;
			case RZ_EPowerStep.SPINNING: text = string.Format("The generator is coming up to speed: about %1 seconds. Stay close.", RZ_Util.RoundToInt(m_fSpinLeft)); break;
			case RZ_EPowerStep.READY: text = "Every needle in the green. The main switch is waiting."; break;
			case RZ_EPowerStep.POWERED: text = string.Format("Running. The service gauge reads %1 percent. Powering %2.", RZ_Util.RoundToInt(m_fCondition * 100), RegionNames()); break;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleChoiceAction on the control object: the old world prompts (no panel screen).
	void HandleChoice(int playerId, int option, IEntity owner, IEntity user)
	{
		if (!user)
			return;

		if (m_iStep == STEP_DARK && option == 0)
		{
			ForceDoor(playerId);
			return;
		}

		if (m_iStep == STEP_OPEN && option == 0)
		{
			// The prompt does the breakers in one go
			m_Panel.SetAll();
			StartGenerator(playerId);
			return;
		}

		if (m_iStep == STEP_READY && option == 0)
		{
			ThrowSwitch(playerId);
			return;
		}

		if (m_iStep == STEP_POWERED && option == 1)
			Maintain(playerId, user);
	}

	//------------------------------------------------------------------------------------------------
	//! Step 1: the control room door (the panel's Force the door, or the old prompt).
	void ForceDoor(int playerId)
	{
		if (m_iStep != STEP_DARK)
			return;

		SetStep(STEP_OPEN);
		m_GM.GetHeat().AddHeat(m_vPos, 10);
		m_GM.NotifyPlayer(playerId, "Power station", "The padlock gives on the third try. It rings like a bell. Inside: the generator, a wall of tripped breakers and a card on the inside of the door.");
	}

	//------------------------------------------------------------------------------------------------
	//! Step 2: start the generator, once the breakers are set. Loud: a horde is sent at it.
	void StartGenerator(int playerId)
	{
		if (m_iStep != STEP_OPEN || !m_Panel.IsReady())
			return;

		m_fSpinLeft = m_Settings.m_fSpinUpSeconds;
		m_fStall = 0;
		SetStep(STEP_SPINNING);
		m_GM.GetHeat().AddHeat(m_vPos, 15);
		// The noise draws them in
		m_GM.GetSpawner().SpawnHorde(m_vPos, 350);
		m_GM.NotifyHorde(m_vPos);
		m_GM.NotifyPlayer(playerId, "Power station", string.Format("The generator catches with a roar you can feel in your teeth. Stay within 60 m for about %1 seconds while it comes up to speed. They heard that.", RZ_Util.RoundToInt(m_fSpinLeft)));
	}

	//------------------------------------------------------------------------------------------------
	//! Step 3: the main switch.
	void ThrowSwitch(int playerId)
	{
		if (m_iStep == STEP_READY)
			Restored(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Why this player can't see to the generator now ("" if they can): it isn't running, or they
	//! did what they could less than 10 minutes ago.
	string MaintainBlocked(int playerId)
	{
		if (m_iStep != STEP_POWERED)
			return "It isn't running";

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		float last;
		if (profile && m_mMaintained.Find(profile.m_sIdentity, last) && m_fClock - last < MAINTAIN_COOLDOWN)
			return "Done for now";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Fuel (a jerrycan) restores a lot; grease and patching a little. Once per player per 10 min.
	void Maintain(int playerId, IEntity user)
	{
		if (m_iStep != STEP_POWERED || !user)
			return;

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		if (m_mMaintained.Find(ident, last) && m_fClock - last < MAINTAIN_COOLDOWN)
		{
			m_GM.NotifyPlayer(playerId, "Power station", "You've done what you can for it. Give it time.");
			return;
		}

		m_mMaintained.Set(ident, m_fClock);
		float amount = m_Settings.m_fPatchRestore;
		string how = "You grease what moves and tape what leaks.";
		if (RZ_ItemHelper.Take(user, "jerrycan,jerry_can,fuel,canister" + RZ_ItemHelper.NOT_EMPTY, 1) > 0)
		{
			amount = m_Settings.m_fFuelRestore;
			how = "You pour in the fuel and grease what moves.";
		}

		m_fCondition = Math.Min(1, m_fCondition + amount);
		if (m_fCondition > WARN_AT)
			m_bWarned = false;

		m_GM.GetHeat().AddHeat(m_vPos, 5);
		m_GM.NotifyPlayer(playerId, "Power station", string.Format("%1 The gauge reads %2 percent.", how, RZ_Util.RoundToInt(m_fCondition * 100)));
	}
}
