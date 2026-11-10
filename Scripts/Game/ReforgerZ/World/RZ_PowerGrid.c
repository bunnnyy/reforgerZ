//! The power grid (docs/WORLD.md, section 5, "Power grid"). One power station per map: a
//! placed RZ_PowerStationMarkerEntity, else a real power or industrial building found by
//! prefab name near a town, else the edge of the biggest town. A group restores it in a
//! short sequence at its control object (force the door, reset the breakers and hold the
//! area while the generator spins up, throw the main switch). Then the towns in its region:
//!
//! - are Powered (RZ_TownMemory), the hook for lights (IsPoweredAt; no lamps are switched on),
//! - get working radios: faction chatter comes more often (ChatterMult), and the radio
//!   station can broadcast twice as often,
//! - give an electronics loot bonus to searches (LootMult, RZ_Search),
//! - but the generator hums: steady heat at the station, which draws hordes.
//!
//! It slowly fails (about four hours with players online) unless someone maintains it.
//! The control object only exists while a player is near. Its state is saved.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Power grid.
[BaseContainerProps()]
class RZ_PowerSettings
{
	[Attribute("1", desc: "A power station players can restore (docs/WORLD.md, section 5)")]
	bool m_bEnabled = true;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Control object: a generator, control panel or breaker cabinet with the generic puzzle object components (RZ_PuzzleObjectComponent, RZ_ClueComponent, RZ_ReadClueAction, 4 RZ_PuzzleChoiceAction). Empty = Puzzles > Fuse box, then the generic object", "et")]
	ResourceName m_sControlPrefab;

	[Attribute("2500", desc: "Towns within this distance (m) of the station get the power")]
	float m_fRegionRadius = 2500;

	[Attribute("90", desc: "Seconds the generator needs to spin up, with someone within 60 m")]
	float m_fSpinUpSeconds = 90;

	[Attribute("10", desc: "Heat per minute the running station makes (it draws hordes)")]
	float m_fNoisePerMinute = 10;

	[Attribute("4", desc: "Hours (with players online) a fully maintained station runs before it fails")]
	float m_fRunHours = 4;

	[Attribute("0.6", desc: "Maintenance with a fuel item (jerrycan) restores this much of its condition (0..1)")]
	float m_fFuelRestore = 0.6;

	[Attribute("0.25", desc: "Maintenance without fuel (grease, patching) restores this much")]
	float m_fPatchRestore = 0.25;

	[Attribute("1.5", desc: "Search hook: electronics loot multiplier in powered towns")]
	float m_fElectronicsLootMult = 1.5;

	[Attribute("0.6", desc: "Faction radio chatter interval multiplier while the power is on (working radios)")]
	float m_fChatterMult = 0.6;
}

//------------------------------------------------------------------------------------------------
class RZ_PowerSave
{
	float x;
	float z;
	string name;
	int step;
	float condition;
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
	protected ref array<int> m_aRegion = {};
	protected ref map<string, float> m_mMaintained = new map<string, float>();

	//------------------------------------------------------------------------------------------------
	void RZ_PowerGrid(RZ_GameModeComponent gm, RZ_PowerSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsActive() { return m_bActive; }
	bool IsPowered() { return m_bActive && m_iStep == STEP_POWERED; }
	vector GetPosition() { return m_vPos; }
	string GetName() { return m_sName; }

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
	//! The grid only: is the town at pos in the power station's region while it runs?
	static bool IsGridPoweredAt(vector pos)
	{
		RZ_PowerGrid grid = Get();
		if (!grid || !grid.IsPowered())
			return false;

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

		string from = "picked";
		array<RZ_PowerStationMarkerEntity> markers = RZ_PowerStationMarkerEntity.GetAll();
		if (!markers.IsEmpty() && markers[0])
		{
			m_vPos = RZ_Util.GroundPos(markers[0].GetOrigin());
			m_sName = markers[0].GetMarkerName();
			from = "marker";
		}
		else if (m_Restored && m_Restored.name != "")
		{
			m_vPos = RZ_Util.GroundPos(Vector(m_Restored.x, 0, m_Restored.z));
			m_sName = m_Restored.name;
			from = "save";
		}
		else if (!PickSite())
		{
			RZ_Log.Info("Power grid: no place for a power station.");
			return;
		}

		if (m_sName.IsEmpty())
			m_sName = "the power station outside " + NearTown(m_vPos);

		if (m_Restored)
		{
			m_iStep = Math.Clamp(m_Restored.step, STEP_DARK, STEP_POWERED);
			m_fCondition = Math.Clamp(m_Restored.condition, 0, 1);
		}

		m_Restored = null;
		m_bActive = true;
		BuildRegion();
		ApplyPower(m_iStep == STEP_POWERED);
		RZ_Log.Info(string.Format("Power grid: %1 at grid %2 (%3), %4 towns in its region, %5.",
			m_sName, RZ_SettlementScanner.GridReference(m_vPos), from, m_aRegion.Count(), StepName()));
	}

	//------------------------------------------------------------------------------------------------
	//! A real power or industrial building near a town (biggest towns first), else the edge
	//! of the biggest town.
	protected bool PickSite()
	{
		array<ref RZ_Settlement> list = m_GM.GetSettlements();
		array<string> kinds = {"power", "industrial"};
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		RZ_Settlement biggest;
		for (int size = 3; size >= 1; size--)
		{
			foreach (RZ_Settlement s : list)
			{
				if (s.m_iSize != size)
					continue;

				if (!biggest)
					biggest = s;

				RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 200, kinds);
				if (!l || (perimeter && perimeter.IsNearLine(l.m_vPos)))
					continue;

				m_vPos = RZ_Util.GroundPos(l.m_vPos);
				m_sName = string.Format("%1 outside %2", l.m_sName, s.m_sName);
				return true;
			}
		}

		if (!biggest)
			return false;

		m_vPos = RZ_Mission.LandPointNear(biggest.m_vCenter, biggest.m_fRadius, biggest.m_fRadius + 80);
		m_sName = "the substation outside " + biggest.m_sName;
		return true;
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
		UpdateBody(players);

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
		UpdateLabels();
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
	}

	//------------------------------------------------------------------------------------------------
	protected void Restored(int playerId)
	{
		SetStep(STEP_POWERED);
		m_fCondition = 1;
		m_bWarned = false;
		ApplyPower(true);
		string text = string.Format("Somebody got %1 running. There are lights in %2 tonight, and the radios have something to run on. Everything for a mile can hear the generator.", m_sName, RegionNames());
		m_GM.NotifyAll("Power", text);
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(string.Format("The power is back in %1. %2 is running again.", RegionNames(), m_sName), false);

		RZ_Log.Info("Power grid: restored by player " + playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Fail()
	{
		SetStep(STEP_OPEN);
		m_fCondition = 0;
		ApplyPower(false);
		NotifyRegion("Power", string.Format("%1 has quit. The lights in %2 are out again.", m_sName, RegionNames()));
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(string.Format("The power failed in %1. Nobody kept %2 going.", RegionNames(), m_sName), false);
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
				m_GM.NotifyPlayer(ids[i], title, text);
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
			RZ_Util.DeleteUnit(m_Body);
			m_Body = null;
			m_Comp = null;
			return;
		}

		if (m_Body || nearest > VISIBLE_RADIUS)
			return;

		m_Body = RZ_Util.SpawnPrefab(ControlPrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Body)
			return;

		m_Comp = RZ_PuzzleObjectComponent.Tag(m_Body, TAG, 0, "", ChoiceLabels());
		RZ_ClueComponent.Tag(m_Body, TAG, 0, "Read the panel");
	}

	//------------------------------------------------------------------------------------------------
	protected string ChoiceLabels()
	{
		switch (m_iStep)
		{
			case RZ_EPowerStep.DARK: return "Force the control room door";
			case RZ_EPowerStep.OPEN: return "Reset the breakers and start the generator";
			case RZ_EPowerStep.SPINNING: return "";
			case RZ_EPowerStep.READY: return "Throw the main switch";
			case RZ_EPowerStep.POWERED: return "|Refuel and grease the generator";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLabels()
	{
		if (m_Comp)
			m_Comp.SetLabels("", ChoiceLabels());
	}

	//------------------------------------------------------------------------------------------------
	protected string StepName()
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
	//! From RZ_ReadClueAction: the panel.
	void HandleRead(int playerId)
	{
		string text;
		switch (m_iStep)
		{
			case RZ_EPowerStep.DARK: text = "A steel door with a padlock and a sign: DANGER - HIGH VOLTAGE. Through the louvres you can see a diesel generator and a wall of breakers. It looks like it could run, if someone made the noise to start it."; break;
			case RZ_EPowerStep.OPEN: text = "Breakers all tripped. The generator has fuel in it. Starting it will be loud, and someone has to stay with it until it settles."; break;
			case RZ_EPowerStep.SPINNING: text = string.Format("The generator is coming up to speed: about %1 seconds. Stay close.", RZ_Util.RoundToInt(m_fSpinLeft)); break;
			case RZ_EPowerStep.READY: text = "Every needle in the green. The main switch is waiting."; break;
			case RZ_EPowerStep.POWERED: text = string.Format("Running. The service gauge reads %1 percent. Powering %2.", RZ_Util.RoundToInt(m_fCondition * 100), RegionNames()); break;
		}

		m_GM.NotifyPlayer(playerId, "Power station", text);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleChoiceAction on the control object.
	void HandleChoice(int playerId, int option, IEntity owner, IEntity user)
	{
		if (!user)
			return;

		if (m_iStep == STEP_DARK && option == 0)
		{
			SetStep(STEP_OPEN);
			m_GM.GetHeat().AddHeat(m_vPos, 10);
			m_GM.NotifyPlayer(playerId, "Power station", "The padlock gives on the third try. It rings like a bell. Inside: the generator and a wall of tripped breakers.");
			return;
		}

		if (m_iStep == STEP_OPEN && option == 0)
		{
			SetStep(STEP_SPINNING);
			m_fSpinLeft = m_Settings.m_fSpinUpSeconds;
			m_fStall = 0;
			m_GM.GetHeat().AddHeat(m_vPos, 15);
			// The noise draws them in
			m_GM.GetSpawner().SpawnHorde(m_vPos, 350);
			m_GM.NotifyHorde(m_vPos);
			m_GM.NotifyPlayer(playerId, "Power station", string.Format("Breakers reset. The generator catches with a roar you can feel in your teeth. Stay within 60 m for about %1 seconds while it comes up to speed. They heard that.", RZ_Util.RoundToInt(m_fSpinLeft)));
			return;
		}

		if (m_iStep == STEP_READY && option == 0)
		{
			Restored(playerId);
			return;
		}

		if (m_iStep == STEP_POWERED && option == 1)
			Maintain(playerId, user);
	}

	//------------------------------------------------------------------------------------------------
	//! Fuel (a jerrycan) restores a lot; grease and patching a little. Once per player per 10 min.
	protected void Maintain(int playerId, IEntity user)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		if (m_mMaintained.Find(ident, last) && m_fClock - last < MAINTAIN_COOLDOWN)
		{
			m_GM.NotifyPlayer(playerId, "Power station", "You've done what you can for it. Give it time.");
			return;
		}

		m_mMaintained.Set(ident, m_fClock);
		array<string> fuelWords = {"jerrycan", "jerry_can", "fuel", "canister"};
		float amount = m_Settings.m_fPatchRestore;
		string how = "You grease what moves and tape what leaks.";
		if (RZ_FoodHelper.TakeFood(user, fuelWords, 1) > 0)
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
