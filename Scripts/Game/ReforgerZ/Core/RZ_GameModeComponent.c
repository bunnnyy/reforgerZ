[ComponentEditorProps(category: "ReforgerZ", description: "ReforgerZ zombie survival logic. Attach to the GameMode entity of any world.")]
class RZ_GameModeComponentClass : SCR_BaseGameModeComponentClass
{
}

//! The hub of the mode. Owns every system, runs the server tick, handles player
//! spawns and backgrounds, saves state, and sends notifications to clients.
//!
//! Server-authoritative: everything except the replicated summary values
//! (stage, cure progress) exists only on the server.
class RZ_GameModeComponent : SCR_BaseGameModeComponent
{
	// ---------------------------------------------------------------- General
	[Attribute("5", UIWidgets.Slider, "Server tick interval (seconds)", "1 30 1", category: "ReforgerZ")]
	protected float m_fTickSeconds;

	[Attribute("SURV", desc: "Faction key players spawn as (the deploy menu, chat, groups and map markers stay on it; characters move onto its hidden variants, see Factions > Hostility)", category: "ReforgerZ")]
	protected string m_sPlayerFactionKey;

	[Attribute("0", desc: "Ignore the saved state on start (season wipe)", category: "ReforgerZ")]
	protected bool m_bWipeOnStart;

	[Attribute("120", desc: "Seconds between state saves", category: "ReforgerZ")]
	protected float m_fSaveIntervalSeconds;

	// ---------------------------------------------------------------- World
	[Attribute("300", desc: "Grid cell (m) used to find building clusters without a map label", category: "ReforgerZ|World")]
	protected float m_fScanCellSize;

	[Attribute("8", desc: "Buildings needed in one cell to count as a settlement", category: "ReforgerZ|World")]
	protected int m_iScanMinBuildings;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Spawn point prefab (SCR_SpawnPoint), placed in random towns", "et", category: "ReforgerZ|World")]
	protected ResourceName m_sSpawnPointPrefab;

	[Attribute("6", desc: "Number of active spawn points", category: "ReforgerZ|World")]
	protected int m_iSpawnPointCount;

	[Attribute("20", desc: "Minutes between moving one spawn point to a new town", category: "ReforgerZ|World")]
	protected float m_fSpawnRotateMinutes;

	[Attribute(desc: "Town memory: Looted, Barricaded, Powered, Burned per town as data, loot that returns, journals of the dead (docs/DESIGN.md, Town memory)", category: "ReforgerZ|World")]
	protected ref RZ_TownMemorySettings m_TownMemorySettings;

	[Attribute(desc: "Power grid: a power station players can restore (control object prefab, region, noise, upkeep; docs/DESIGN.md, Time and environment)", category: "ReforgerZ|World")]
	protected ref RZ_PowerSettings m_PowerSettings;

	[Attribute(desc: "Radio station: a broadcast point players can take over to send preset messages (console prefab, cooldown; cosmetic only, docs/DESIGN.md, Time and environment)", category: "ReforgerZ|World")]
	protected ref RZ_RadioStationSettings m_RadioStationSettings;

	// ---------------------------------------------------------------- Director / zombies
	[Attribute(desc: "Stage overrides. Leave empty to use the built-in defaults", category: "ReforgerZ|Director")]
	protected ref array<ref RZ_StageSettings> m_aStages;

	[Attribute("25", desc: "Minutes between government broadcasts to everyone (flavour only; also sent on first join). 0 = only on first join", category: "ReforgerZ|Director")]
	protected float m_fBroadcastMinutes;

	[Attribute(desc: "Time and weather: night (they hear further, more specials, fewer human patrols), rain, fog, storms grounding the helicopter (docs/DESIGN.md, Time and environment)", category: "ReforgerZ|Director")]
	protected ref RZ_EnvironmentSettings m_EnvironmentSettings;

	[Attribute(desc: "Story arcs: short storylines made of missions, rumours and headlines, including the Haven's rare dark side (docs/DESIGN.md, Story arcs and headlines)", category: "ReforgerZ|Director")]
	protected ref RZ_StorySettings m_StorySettings;

	[Attribute(desc: "Zombie prefabs and density", category: "ReforgerZ|Zombies")]
	protected ref RZ_ZombieSettings m_ZombieSettings;

	[Attribute(desc: "Infection adapts a little: specials after breakthroughs, one trait per stage (runner, helmet, screamer prefabs), favourite spots (docs/DESIGN.md, Infection adapts)", category: "ReforgerZ|Zombies")]
	protected ref RZ_AdaptationSettings m_AdaptationSettings;

	[Attribute("0.5", desc: "Heat per tick each player gives off just by being somewhere", category: "ReforgerZ|Zombies")]
	protected float m_fHeatPresence;

	[Attribute("4", desc: "Extra heat per tick while in a vehicle", category: "ReforgerZ|Zombies")]
	protected float m_fHeatVehicle;

	// ---------------------------------------------------------------- Backgrounds
	[Attribute(desc: "Civilian backgrounds. Leave empty to use the built-in eight", category: "ReforgerZ|Backgrounds")]
	protected ref array<ref RZ_Background> m_aBackgrounds;

	[Attribute("20", desc: "Base minutes from a bite to turning", category: "ReforgerZ|Backgrounds")]
	protected float m_fInfectionMinutes;

	// ---------------------------------------------------------------- Cure
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Lab prefab with RZ_LabComponent (spawned if none is placed)", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sLabPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Research sample item with RZ_SampleItemComponent", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sSamplePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Antidote item. After the second breakthrough the lab makes doses into a stock; quartermaster shop actions selling this prefab draw on it", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sAntidotePrefab;

	[Attribute("2", desc: "Research percent per sample (x background multiplier)", category: "ReforgerZ|Cure")]
	protected float m_fResearchPerSample;

	// ---------------------------------------------------------------- Rewards
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Loot crate spawned as a mission reward", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sRewardCratePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Friendly survivor AI group for holdout missions (optional)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sSurvivorGroupPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Destructible zombie nest, e.g. from Zombie Hive (optional, enables Burn the Nest)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sNestPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand dead drop with RZ_SellSamplesToBanditsAction (optional, enables the Dead Drop contract)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sDeadDropPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Mission item (black box, data drive, package) with RZ_MissionItemComponent. Enables delivery and choice missions", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sMissionItemPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven drop point: a prop with RZ_ChoiceDeliveryAction set to SURVIVORS", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sDropPointHaven;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion drop point: a prop with RZ_ChoiceDeliveryAction set to MILITARY", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sDropPointBastion;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand drop point: a prop with RZ_ChoiceDeliveryAction set to BANDITS", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sDropPointRedHand;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon drop point: a prop with RZ_ChoiceDeliveryAction set to INSTITUTE", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sDropPointHalcyon;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Helicopter wreck prop (optional, enables Crash Site and Crashed Helicopter)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sHeliWreckPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Corpse pile prop (optional, only visual: Burn the Bodies and Decontamination work without it)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sCorpsePilePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand AI group (optional, enables Raid the Red Hand and Trader Caravan, adds raiders to Lab Defence)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sRedHandGroupPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Destructible Bastion fuel depot with a damage manager (optional, enables Arson)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sFuelDepotPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Destructible Haven radio relay with a damage manager (optional, enables Sabotage the Relay)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sRelayPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Wandering trader: a character, vehicle or composition, e.g. with a Shop System trader (optional, enables Wandering Trader)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sTraderPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Explosion effect prefab, e.g. a mortar or artillery shell impact (optional, enables Bastion Shelling)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sExplosionPrefab;

	// ---------------------------------------------------------------- Puzzles
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Readable note: RZ_ClueComponent + RZ_ReadClueAction. Enables every puzzle mission", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sPuzzleNotePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Generic puzzle object: RZ_PuzzleObjectComponent, RZ_ClueComponent, RZ_ReadClueAction, 4 RZ_PuzzleUseAction (values 0-3), 4 RZ_PuzzleChoiceAction (options 0-3). Fallback for wells, bells, fuse boxes, masts, safes, people", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sPuzzleObjectPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Keypad: RZ_PuzzleObjectComponent + 12 RZ_KeypadAction (keys 0-9, 10 Clear, 11 Enter) + RZ_ClueComponent/RZ_ReadClueAction (optional)", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sKeypadPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Terminal (optional, falls back to the keypad): a computer prop with the keypad's components", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sTerminalPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Body with something readable (optional, falls back to the note): RZ_ClueComponent + RZ_ReadClueAction", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sPuzzleBodyPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Well (optional): the generic object's components on a well prop", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sWellPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bell (optional): the generic object's components on a bell prop", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sBellPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Fuse box (optional): the generic object's components on an electrical box", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sFuseBoxPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Radio mast (optional): the generic object's components on an antenna", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sRadioMastPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Safe (optional): the generic object's components on a safe", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sSafePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Scale crate: a crate with an inventory storage plus the generic object's components. Enables The Weighing Vault", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sScaleCratePrefab;

	[Attribute("0", desc: "The Weighing Vault counts items in the crate instead of weighing them (use if the storage weight API doesn't work)", category: "ReforgerZ|Puzzles")]
	protected bool m_bScaleCountsItems;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "A person to talk to (optional, falls back to the generic object): a civilian character or a seated figure prop with the generic object's components", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sCivilianPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Fallback landmark when a town has no church, tower or well, e.g. a wayside cross or a painted mast", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sLandmarkPrefab;

	[Attribute("the old cross", desc: "What notes call the fallback landmark", category: "ReforgerZ|Puzzles")]
	protected string m_sLandmarkName;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Morse light: a lamp with RZ_BlinkLightComponent (or any light prefab: it's then spawned per flash). Enables Lights in the Window", "et", category: "ReforgerZ|Puzzles")]
	protected ResourceName m_sBlinkLightPrefab;

	// ---------------------------------------------------------------- Perimeter
	[Attribute(desc: "The Perimeter: the edge of the map as the quarantine line (warning zone, kill zone, drones, road checkpoints) and the prefabs for its missions", category: "ReforgerZ|Perimeter")]
	protected ref RZ_PerimeterSettings m_PerimeterSettings;

	// ---------------------------------------------------------------- Haven safehouses and wanderers
	[Attribute(desc: "Haven safehouses: how many, starting population and food, occupant/guard/medic prefabs, upkeep, food keywords, house rules (docs/DESIGN.md)", category: "ReforgerZ|Haven")]
	protected ref RZ_SafehouseSettings m_SafehouseSettings;

	[Attribute(desc: "Named wanderers: roster size, body prefab per role, Trader goods, prices, replacement time (docs/DESIGN.md)", category: "ReforgerZ|Wanderers")]
	protected ref RZ_WandererSettings m_WandererSettings;

	// ---------------------------------------------------------------- Faction leaders
	[Attribute(desc: "Named faction leaders: body prefabs per faction and role, interim outposts, replacement time, rogue-job discovery (docs/REPUTATION.md)", category: "ReforgerZ|Leaders")]
	protected ref RZ_LeaderSettings m_LeaderSettings;

	// ---------------------------------------------------------------- Territory, supply trucks, faction life
	[Attribute(desc: "Background territory: strongholds and outposts, the territory tick, faction strength, outpost guards and named occupants (docs/DESIGN.md, Factions in the background)", category: "ReforgerZ|Territory")]
	protected ref RZ_TerritorySettings m_TerritorySettings;

	[Attribute(desc: "Supply trucks: truck and crew prefabs per faction, departures, unsticking, stock, shortages, escort and theft rewards (docs/DESIGN.md, Supply trucks)", category: "ReforgerZ|Territory")]
	protected ref RZ_SupplySettings m_SupplySettings;

	[Attribute(desc: "Faction life: needs, morale, radio chatter, life events, truces and feuds, remembered moments, rumours (docs/DESIGN.md, Factions in the background)", category: "ReforgerZ|Territory")]
	protected ref RZ_FactionLifeSettings m_FactionLifeSettings;

	// ---------------------------------------------------------------- New-player guidance
	[Attribute(desc: "Quiet in-world guidance: experience levels, contextual tips from nearby people, the first-life goal, the gentle start (docs/DESIGN.md)", category: "ReforgerZ|Guidance")]
	protected ref RZ_GuidanceSettings m_GuidanceSettings;

	// ---------------------------------------------------------------- World interactions
	[Attribute(desc: "World interactions: the interaction spot prefab, scan radius and caps, bells and sirens, water, treating other players (docs/DESIGN.md, World interactions)", category: "ReforgerZ|Interactions")]
	protected ref RZ_InteractionSettings m_InteractionSettings;

	[Attribute(desc: "Searching: loot pool and tables per building type, chances, locks and keys, readable things, bodies", category: "ReforgerZ|Interactions")]
	protected ref RZ_SearchSettings m_SearchSettings;

	[Attribute(desc: "Fixable cars (rare), wrecks, parts, fuel, hotwiring", category: "ReforgerZ|Interactions")]
	protected ref RZ_CarSettings m_CarSettings;

	[Attribute(desc: "Crafting at workbenches: built-in recipes' output prefabs, input keywords, extra recipes", category: "ReforgerZ|Interactions")]
	protected ref RZ_CraftSettings m_CraftSettings;

	[Attribute(desc: "Barricades, tripwires, spike traps, generators, molotovs and fire", category: "ReforgerZ|Interactions")]
	protected ref RZ_TrapSettings m_TrapSettings;

	[Attribute(desc: "Secure radio channels (Radio Operators) and house radios", category: "ReforgerZ|Interactions")]
	protected ref RZ_RadioCrackSettings m_RadioCrackSettings;

	// ---------------------------------------------------------------- Survival
	[Attribute("3", desc: "Heat per shot fired by a player (times the background noise multiplier)", category: "ReforgerZ|Survival")]
	protected float m_fHeatPerShot;

	[Attribute("0.3", desc: "Heat multiplier for shots from a suppressed weapon", category: "ReforgerZ|Survival")]
	protected float m_fSuppressedHeatMult;

	[Attribute("0.3", desc: "Chance that a zombie hit infects. 0 disables built-in bite detection", category: "ReforgerZ|Survival")]
	protected float m_fBiteChance;

	[Attribute("2.5", desc: "How close (m) a zombie must be for a health drop to count as its hit", category: "ReforgerZ|Survival")]
	protected float m_fBiteReach;

	// ---------------------------------------------------------------- Factions
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven quartermaster (RZ_QuartermasterComponent + shop actions). Spawned in the safest town if none is placed", "et", category: "ReforgerZ|Factions")]
	protected ResourceName m_sQuartermasterHaven;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion quartermaster (also add RZ_RequisitionAction for ammo and rations)", "et", category: "ReforgerZ|Factions")]
	protected ResourceName m_sQuartermasterBastion;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand quartermaster (black market)", "et", category: "ReforgerZ|Factions")]
	protected ResourceName m_sQuartermasterRedHand;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon quartermaster, spawned next to the lab", "et", category: "ReforgerZ|Factions")]
	protected ResourceName m_sQuartermasterHalcyon;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion hunter squad: AI group sent after players Hunted by Bastion (optional)", "et", category: "ReforgerZ|Factions")]
	protected ResourceName m_sBastionHunterPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand hunter squad (optional; falls back to the Red Hand group prefab)", "et", category: "ReforgerZ|Factions")]
	protected ResourceName m_sRedHandHunterPrefab;

	[Attribute("10", desc: "Minutes between hunter squads for Hunted players. 0 = off", category: "ReforgerZ|Factions")]
	protected float m_fHunterSquadMinutes;

	[Attribute("10", desc: "Minutes between public bounty announcements for players at Kill on Sight", category: "ReforgerZ|Factions")]
	protected float m_fBountyMinutes;

	[Attribute(desc: "Per-player AI hostility: mode (bitmask variants, outlaw fallback, off), from which standing each faction's AI shoots, the Haven exile rule, fights, wanderers fighting back (docs/REPUTATION.md)", category: "ReforgerZ|Factions")]
	protected ref RZ_HostilitySettings m_HostilitySettings;

	// ---------------------------------------------------------------- Map
	[Attribute("1", desc: "Show world events and the lab on the map (vanilla map markers). Turn off if markers misbehave", category: "ReforgerZ|Map")]
	protected bool m_bMapMarkers;

	// ---------------------------------------------------------------- Debug
	[Attribute("1", desc: "Log a world status summary every few minutes", category: "ReforgerZ|Debug")]
	protected bool m_bStatusLog;

	// ---------------------------------------------------------------- Replicated summary
	[RplProp()]
	protected float m_fReplCureProgress;

	[RplProp()]
	protected int m_iReplStage;

	[RplProp()]
	protected bool m_bReplCured;

	[RplProp()]
	protected string m_sReplLockedBuildings;	// ";key;key;": locked buildings near players (RZ_DoorLocks, for the door action)

	// ---------------------------------------------------------------- Runtime (server)
	protected static RZ_GameModeComponent s_Instance;

	protected ref array<ref RZ_Settlement> m_aSettlements = {};
	protected ref RZ_Director m_Director;
	protected ref RZ_HeatMap m_Heat;
	protected ref RZ_ZombieSpawner m_Spawner;
	protected ref RZ_Adaptation m_Adaptation;
	protected ref RZ_TownMemory m_TownMemory;
	protected ref RZ_Environment m_Environment;
	protected ref RZ_PowerGrid m_PowerGrid;
	protected ref RZ_RadioStation m_RadioStation;
	protected ref RZ_StoryArcs m_StoryArcs;
	protected ref RZ_Headlines m_Headlines;
	protected ref RZ_MissionManager m_Missions;
	protected ref RZ_CureResearch m_Cure;
	protected ref RZ_BiteDetector m_BiteDetector;
	protected ref RZ_FactionPerks m_Perks;
	protected ref RZ_Broadcast m_Broadcast;
	protected ref RZ_Perimeter m_Perimeter;
	protected ref RZ_Safehouses m_Safehouses;
	protected ref RZ_Wanderers m_Wanderers;
	protected ref RZ_Leaders m_Leaders;
	protected ref RZ_Hostility m_Hostility;
	protected ref RZ_Guidance m_Guidance;
	protected ref RZ_Territory m_Territory;
	protected ref RZ_SupplyLines m_Supply;
	protected ref RZ_FactionLife m_FactionLife;
	protected ref RZ_Interactions m_Interactions;
	protected ref array<int> m_aSeasonFame = {0, 0, 0, 0};	// Fame earned with each faction this season, all players
	protected int m_iSeasonWinner = -1;
	protected float m_fStatusTimer;
	protected ref map<string, ref RZ_PlayerProfile> m_mProfiles = new map<string, ref RZ_PlayerProfile>();
	protected ref array<IEntity> m_aSpawnPoints = {};
	protected ref set<int> m_aWelcomed = new set<int>();

	protected IEntity m_Lab;
	protected vector m_vLabPos;
	protected bool m_bHasLab;
	protected float m_fSaveTimer;
	protected float m_fSpawnRotateTimer;
	protected bool m_bServerRunning;

	//------------------------------------------------------------------------------------------------
	static RZ_GameModeComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		s_Instance = this;

		// On every machine: the modded faction check (RZ_FactionRelations.c) decodes keys with these
		if (m_HostilitySettings)
			RZ_Factions.Configure(m_sPlayerFactionKey, m_HostilitySettings.m_sBastionKey, m_HostilitySettings.m_sRedHandKey, m_HostilitySettings.m_sHalcyonKey);
		else
			RZ_Factions.Configure(m_sPlayerFactionKey, "", "", "");
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		if (Replication.IsServer())
			InitServer();
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_GameModeComponent()
	{
		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(ShowNextHint);

		if (m_bServerRunning && GetGame() && GetGame().GetCallqueue())
		{
			GetGame().GetCallqueue().Remove(ServerTick);
			GetGame().GetCallqueue().Remove(BiteCheck);
			SaveState();
		}

		if (s_Instance == this)
			s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitServer()
	{
		if (!m_ZombieSettings)
			m_ZombieSettings = new RZ_ZombieSettings();

		if (!m_PerimeterSettings)
			m_PerimeterSettings = new RZ_PerimeterSettings();

		if (!m_AdaptationSettings)
			m_AdaptationSettings = new RZ_AdaptationSettings();

		if (!m_TownMemorySettings)
			m_TownMemorySettings = new RZ_TownMemorySettings();

		if (!m_EnvironmentSettings)
			m_EnvironmentSettings = new RZ_EnvironmentSettings();

		if (!m_PowerSettings)
			m_PowerSettings = new RZ_PowerSettings();

		if (!m_RadioStationSettings)
			m_RadioStationSettings = new RZ_RadioStationSettings();

		if (!m_StorySettings)
			m_StorySettings = new RZ_StorySettings();

		if (!m_SafehouseSettings)
			m_SafehouseSettings = new RZ_SafehouseSettings();

		if (!m_WandererSettings)
			m_WandererSettings = new RZ_WandererSettings();

		if (!m_LeaderSettings)
			m_LeaderSettings = new RZ_LeaderSettings();

		if (!m_HostilitySettings)
			m_HostilitySettings = new RZ_HostilitySettings();

		if (!m_GuidanceSettings)
			m_GuidanceSettings = new RZ_GuidanceSettings();

		if (!m_TerritorySettings)
			m_TerritorySettings = new RZ_TerritorySettings();

		if (!m_SupplySettings)
			m_SupplySettings = new RZ_SupplySettings();

		if (!m_FactionLifeSettings)
			m_FactionLifeSettings = new RZ_FactionLifeSettings();

		if (!m_InteractionSettings)
			m_InteractionSettings = new RZ_InteractionSettings();

		if (!m_SearchSettings)
			m_SearchSettings = new RZ_SearchSettings();

		if (!m_CarSettings)
			m_CarSettings = new RZ_CarSettings();

		if (!m_CraftSettings)
			m_CraftSettings = new RZ_CraftSettings();

		if (!m_TrapSettings)
			m_TrapSettings = new RZ_TrapSettings();

		if (!m_RadioCrackSettings)
			m_RadioCrackSettings = new RZ_RadioCrackSettings();

		if (!m_aBackgrounds || m_aBackgrounds.IsEmpty())
		{
			m_aBackgrounds = new array<ref RZ_Background>();
			RZ_Background.CreateDefaults(m_aBackgrounds);
		}

		m_Heat = new RZ_HeatMap();
		m_Adaptation = new RZ_Adaptation(this, m_AdaptationSettings);
		m_TownMemory = new RZ_TownMemory(this, m_TownMemorySettings);
		m_Environment = new RZ_Environment(this, m_EnvironmentSettings);
		m_PowerGrid = new RZ_PowerGrid(this, m_PowerSettings);
		m_RadioStation = new RZ_RadioStation(this, m_RadioStationSettings);
		m_StoryArcs = new RZ_StoryArcs(this, m_StorySettings);
		m_Headlines = new RZ_Headlines(this);
		m_Cure = new RZ_CureResearch(this);
		m_Director = new RZ_Director(this, m_aStages);
		m_Spawner = new RZ_ZombieSpawner(this, m_ZombieSettings);
		m_Missions = new RZ_MissionManager(this);
		m_BiteDetector = new RZ_BiteDetector(this, m_fBiteChance, m_fBiteReach);
		m_Perks = new RZ_FactionPerks(this);
		m_Broadcast = new RZ_Broadcast(this);
		m_Perimeter = new RZ_Perimeter(this, m_PerimeterSettings);
		m_Safehouses = new RZ_Safehouses(this, m_SafehouseSettings);
		m_Wanderers = new RZ_Wanderers(this, m_WandererSettings);
		m_Leaders = new RZ_Leaders(this, m_LeaderSettings);
		m_Hostility = new RZ_Hostility(this, m_HostilitySettings);
		m_Guidance = new RZ_Guidance(this, m_GuidanceSettings);
		m_Territory = new RZ_Territory(this, m_TerritorySettings);
		m_Supply = new RZ_SupplyLines(this, m_SupplySettings);
		m_FactionLife = new RZ_FactionLife(this, m_FactionLifeSettings);
		m_Interactions = new RZ_Interactions(this, m_InteractionSettings, m_SearchSettings, m_CarSettings, m_CraftSettings, m_TrapSettings, m_RadioCrackSettings);

		bool loaded = false;
		if (!m_bWipeOnStart)
			loaded = LoadState();

		// The last finished season's ending. TODO (docs/ROADMAP.md): next-season rules from it.
		RZ_SeasonRecord last = RZ_Persistence.LoadSeason();
		if (last)
			RZ_Log.Info(string.Format("Last season (%1) the cure went to %2.", last.world, last.winnerName));

		if (m_aSettlements.IsEmpty())
		{
			RZ_SettlementScanner scanner = new RZ_SettlementScanner();
			scanner.Scan(m_fScanCellSize, m_iScanMinBuildings, m_aSettlements);
		}

		if (m_aSettlements.IsEmpty())
			RZ_Log.Error("No settlements found on this map. Place RZ_SettlementMarkerEntity entities in the world.");

		// Before anything is placed, so missions, spawn points and quartermasters keep off the line
		m_Perimeter.Init();
		m_TownMemory.Setup();
		m_PowerGrid.Setup();
		m_RadioStation.Setup();

		SetupLab();
		m_Perks.SetupQuartermasters();
		// Safehouses anchor on the Haven quartermaster; wanderers start from safehouses
		m_Safehouses.Setup();
		m_Wanderers.Setup();
		// Leaders sit at the quartermasters, the lab and the safehouses. With territory on, real
		// outposts replace the interim leader posts: territory registers their leaders
		m_Leaders.SetTerritoryOutposts(m_TerritorySettings.m_bEnabled);
		m_Leaders.Setup();
		m_FactionLife.Setup();
		m_Territory.Setup();
		m_Supply.Setup();
		// Searching, cars, crafting, traps, fire, radios: keep off safehouses, holdings and the lab
		m_Interactions.Setup();
		// Arcs are made of the missions, wanderers, safehouses, territory and trucks above
		m_StoryArcs.Setup();
		m_Hostility.Init();
		if (m_bHasLab)
			AddMapMarker(m_vLabPos, "Halcyon lab");
		RefreshSpawnPoints(true);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetOwner());
		if (gameMode)
			gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned); // VERIFY: invoker name

		UpdateReplicated();
		m_bServerRunning = true;
		GetGame().GetCallqueue().CallLater(ServerTick, m_fTickSeconds * 1000, true);
		// Bites need a faster check than the main tick to catch individual hits
		GetGame().GetCallqueue().CallLater(BiteCheck, 1000, true);

		RZ_Log.Info(string.Format("Started. %1 settlements, stage %2, cure %3 percent, state %4.",
			m_aSettlements.Count(), RZ_StageSettings.GetStageName(m_Director.GetStage()),
			RZ_Util.RoundToInt(m_Cure.m_fProgress), loaded));
	}

	//------------------------------------------------------------------------------------------------
	protected void ServerTick()
	{
		float dt = m_fTickSeconds;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);

		UpdatePlayerHeat(ids, players);
		m_Heat.Decay(dt);
		m_Director.Tick(dt);
		m_Spawner.Tick(dt, players);
		m_Adaptation.Tick(dt, ids, players);
		m_TownMemory.Tick(dt);
		m_Environment.Tick(dt, ids, players);
		m_PowerGrid.Tick(dt, ids, players);
		m_RadioStation.Tick(dt, ids, players);
		m_Interactions.Tick(dt, ids, players);
		m_Missions.Tick(dt, ids, players);
		m_Perimeter.Tick(dt, ids, players);
		m_Cure.Tick(dt, players);
		m_Perks.Tick(dt, ids, players);
		m_Safehouses.Tick(dt, ids, players);
		m_Wanderers.Tick(dt, ids, players);
		m_Leaders.Tick(dt, ids, players);
		m_Territory.Tick(dt, ids, players);
		m_Supply.Tick(dt, ids, players);
		m_FactionLife.Tick(dt, ids, players);
		m_StoryArcs.Tick(dt, ids, players);
		// After everything that changes standing or spawns fights this tick
		m_Hostility.Tick(dt, ids, players);
		m_Broadcast.Tick(dt, m_fBroadcastMinutes);
		RZ_GameTime.Tick(dt);
		RZ_NoisemakerComponent.TickAll();
		UpdateInfections(dt, ids);

		foreach (int i, IEntity ent : players)
		{
			RZ_Objectives.Tick(this, ids[i], GetProfile(ids[i]), ent, dt);
		}

		// After the objectives, which advance each life's clock
		m_Guidance.Tick(dt, ids, players);

		if (m_bStatusLog)
		{
			m_fStatusTimer += dt;
			if (m_fStatusTimer >= 300)
			{
				m_fStatusTimer = 0;
				LogStatus(ids.Count());
			}
		}

		m_fSpawnRotateTimer += dt;
		if (m_fSpawnRotateTimer >= m_fSpawnRotateMinutes * 60)
		{
			m_fSpawnRotateTimer = 0;
			RefreshSpawnPoints(false);
		}

		m_fSaveTimer += dt;
		if (m_fSaveTimer >= m_fSaveIntervalSeconds)
		{
			m_fSaveTimer = 0;
			SaveState();
		}
	}

	// ================================================================ Accessors
	array<ref RZ_Settlement> GetSettlements() { return m_aSettlements; }
	RZ_Director GetDirector() { return m_Director; }
	RZ_HeatMap GetHeat() { return m_Heat; }
	RZ_ZombieSpawner GetSpawner() { return m_Spawner; }
	RZ_Adaptation GetAdaptation() { return m_Adaptation; }
	RZ_TownMemory GetTownMemory() { return m_TownMemory; }
	RZ_Environment GetEnvironment() { return m_Environment; }
	RZ_PowerGrid GetPowerGrid() { return m_PowerGrid; }
	RZ_RadioStation GetRadioStation() { return m_RadioStation; }
	RZ_StoryArcs GetStoryArcs() { return m_StoryArcs; }
	RZ_Headlines GetHeadlines() { return m_Headlines; }
	RZ_MissionManager GetMissions() { return m_Missions; }
	RZ_CureResearch GetCure() { return m_Cure; }
	vector GetLabPosition() { return m_vLabPos; }
	ResourceName GetSurvivorGroupPrefab() { return m_sSurvivorGroupPrefab; }
	ResourceName GetNestPrefab() { return m_sNestPrefab; }
	ResourceName GetDeadDropPrefab() { return m_sDeadDropPrefab; }
	ResourceName GetMissionItemPrefab() { return m_sMissionItemPrefab; }
	ResourceName GetHeliWreckPrefab() { return m_sHeliWreckPrefab; }
	ResourceName GetCorpsePilePrefab() { return m_sCorpsePilePrefab; }
	ResourceName GetRedHandGroupPrefab() { return m_sRedHandGroupPrefab; }
	ResourceName GetFuelDepotPrefab() { return m_sFuelDepotPrefab; }
	ResourceName GetRelayPrefab() { return m_sRelayPrefab; }
	ResourceName GetTraderPrefab() { return m_sTraderPrefab; }
	ResourceName GetExplosionPrefab() { return m_sExplosionPrefab; }
	ResourceName GetAntidotePrefab() { return m_sAntidotePrefab; }
	float GetHunterSquadMinutes() { return m_fHunterSquadMinutes; }
	float GetBountyMinutes() { return Math.Max(1, m_fBountyMinutes); }
	RZ_FactionPerks GetPerks() { return m_Perks; }
	RZ_Perimeter GetPerimeter() { return m_Perimeter; }
	RZ_Safehouses GetSafehouses() { return m_Safehouses; }
	RZ_Wanderers GetWanderers() { return m_Wanderers; }
	RZ_Leaders GetLeaders() { return m_Leaders; }
	RZ_Hostility GetHostility() { return m_Hostility; }
	RZ_Guidance GetGuidance() { return m_Guidance; }
	RZ_Territory GetTerritory() { return m_Territory; }
	RZ_SupplyLines GetSupply() { return m_Supply; }
	RZ_FactionLife GetFactionLife() { return m_FactionLife; }
	RZ_Interactions GetInteractions() { return m_Interactions; }
	RZ_GuidanceSettings GetGuidanceSettings() { return m_GuidanceSettings; }
	float GetResearchPerSample() { return m_fResearchPerSample; }
	float GetTickSeconds() { return m_fTickSeconds; }
	RZ_Broadcast GetBroadcast() { return m_Broadcast; }
	float GetBroadcastMinutes() { return m_fBroadcastMinutes; }
	string GetLandmarkFallbackName() { return m_sLandmarkName; }
	bool ScaleCountsItems() { return m_bScaleCountsItems; }

	//------------------------------------------------------------------------------------------------
	//! Prefab for a puzzle prop, with the fallbacks described on RZ_EPuzzleProp. "" = not available.
	ResourceName GetPuzzlePrefab(RZ_EPuzzleProp kind)
	{
		switch (kind)
		{
			case RZ_EPuzzleProp.CLUE: return m_sPuzzleNotePrefab;
			case RZ_EPuzzleProp.OBJECT: return m_sPuzzleObjectPrefab;
			case RZ_EPuzzleProp.KEYPAD: return m_sKeypadPrefab;
			case RZ_EPuzzleProp.SCALE: return m_sScaleCratePrefab;
			case RZ_EPuzzleProp.LANDMARK: return m_sLandmarkPrefab;
			case RZ_EPuzzleProp.LIGHT: return m_sBlinkLightPrefab;
			case RZ_EPuzzleProp.TERMINAL: return PuzzleFallback(m_sTerminalPrefab, m_sKeypadPrefab);
			case RZ_EPuzzleProp.BODY: return PuzzleFallback(m_sPuzzleBodyPrefab, m_sPuzzleNotePrefab);
			case RZ_EPuzzleProp.WELL: return PuzzleFallback(m_sWellPrefab, m_sPuzzleObjectPrefab);
			case RZ_EPuzzleProp.BELL: return PuzzleFallback(m_sBellPrefab, m_sPuzzleObjectPrefab);
			case RZ_EPuzzleProp.FUSEBOX: return PuzzleFallback(m_sFuseBoxPrefab, m_sPuzzleObjectPrefab);
			case RZ_EPuzzleProp.RADIO_MAST: return PuzzleFallback(m_sRadioMastPrefab, m_sPuzzleObjectPrefab);
			case RZ_EPuzzleProp.SAFE: return PuzzleFallback(m_sSafePrefab, m_sPuzzleObjectPrefab);
			case RZ_EPuzzleProp.CIVILIAN: return PuzzleFallback(m_sCivilianPrefab, m_sPuzzleObjectPrefab);
			case RZ_EPuzzleProp.SMUGGLER: return PuzzleFallback(GetPerimeterPrefab(0), GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN));
			case RZ_EPuzzleProp.LINE_BODY: return PuzzleFallback(GetPerimeterPrefab(1), GetPuzzlePrefab(RZ_EPuzzleProp.BODY));
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The sealed package for the Perimeter missions: its own slot, else the mission item.
	ResourceName GetPerimeterPackagePrefab()
	{
		return PuzzleFallback(GetPerimeterPrefab(2), m_sMissionItemPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! 0 smuggler, 1 body at the line, 2 package. "" if not set.
	protected ResourceName GetPerimeterPrefab(int which)
	{
		if (!m_PerimeterSettings)
			return "";

		if (which == 0)
			return m_PerimeterSettings.m_sSmugglerPrefab;

		if (which == 1)
			return m_PerimeterSettings.m_sLineBodyPrefab;

		return m_PerimeterSettings.m_sPackagePrefab;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName PuzzleFallback(ResourceName preferred, ResourceName fallback)
	{
		if (preferred.IsEmpty())
			return fallback;

		return preferred;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetQuartermasterPrefab(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return m_sQuartermasterHaven;
			case RZ_EFaction.MILITARY: return m_sQuartermasterBastion;
			case RZ_EFaction.BANDITS: return m_sQuartermasterRedHand;
			case RZ_EFaction.INSTITUTE: return m_sQuartermasterHalcyon;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetHunterPrefab(RZ_EFaction faction)
	{
		if (faction == RZ_EFaction.MILITARY)
			return m_sBastionHunterPrefab;

		if (faction == RZ_EFaction.BANDITS)
		{
			if (m_sRedHandHunterPrefab.IsEmpty())
				return m_sRedHandGroupPrefab;

			return m_sRedHandHunterPrefab;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion Requisition perk, from RZ_RequisitionAction: one item at half price, once an hour.
	bool TryRequisition(int playerId, IEntity quartermaster, IEntity user, ResourceName prefab, string itemName, int fullCost)
	{
		return m_Perks.TryRequisition(playerId, quartermaster, user, prefab, itemName, fullCost);
	}
	bool HasLab() { return m_bHasLab; }

	//------------------------------------------------------------------------------------------------
	//! Index of the settlement the lab is in (or nearest to), -1 without a lab.
	int GetLabSettlement()
	{
		if (!m_bHasLab)
			return -1;

		return FindNearestSettlement(m_vLabPos);
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetDropPointPrefab(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return m_sDropPointHaven;
			case RZ_EFaction.MILITARY: return m_sDropPointBastion;
			case RZ_EFaction.BANDITS: return m_sDropPointRedHand;
			case RZ_EFaction.INSTITUTE: return m_sDropPointHalcyon;
		}

		return "";
	}

	float GetReplicatedCureProgress() { return m_fReplCureProgress; }
	RZ_EOutbreakStage GetReplicatedStage() { return m_iReplStage; }
	bool IsReplicatedCured() { return m_bReplCured; }
	string GetLockedBuildings() { return m_sReplLockedBuildings; }

	//------------------------------------------------------------------------------------------------
	//! Server: which buildings near players are locked, for the client side of the door action.
	void SetLockedBuildings(string list)
	{
		if (list == m_sReplLockedBuildings)
			return;

		m_sReplLockedBuildings = list;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	RZ_Settlement GetSettlement(int idx)
	{
		if (idx < 0 || idx >= m_aSettlements.Count())
			return null;

		return m_aSettlements[idx];
	}

	//------------------------------------------------------------------------------------------------
	int FindNearestSettlement(vector pos)
	{
		int best = -1;
		float bestDist = float.MAX;
		foreach (int i, RZ_Settlement s : m_aSettlements)
		{
			float d = RZ_Util.DistXZ(pos, s.m_vCenter);
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Public API for other mods: report a loud event (gunfire, explosion, alarm...).
	static void ReportNoise(vector pos, float amount)
	{
		if (s_Instance && s_Instance.m_Heat)
			s_Instance.m_Heat.AddHeat(pos, amount);
	}

	// ================================================================ Players, backgrounds, reputation
	//------------------------------------------------------------------------------------------------
	protected string GetIdentity(int playerId)
	{
		string ident = GetGame().GetBackendApi().GetPlayerIdentityId(playerId); // VERIFY
		if (ident.IsEmpty())
			ident = "local_" + GetGame().GetPlayerManager().GetPlayerName(playerId);

		return ident;
	}

	//------------------------------------------------------------------------------------------------
	RZ_PlayerProfile GetProfile(int playerId)
	{
		string ident = GetIdentity(playerId);
		RZ_PlayerProfile profile;
		if (!m_mProfiles.Find(ident, profile))
		{
			profile = new RZ_PlayerProfile();
			profile.m_sIdentity = ident;
			m_mProfiles.Insert(ident, profile);
		}

		return profile;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Background GetBackground(string id)
	{
		foreach (RZ_Background bg : m_aBackgrounds)
		{
			if (bg.m_sId == id)
				return bg;
		}

		return GetDefaultBackground();
	}

	//------------------------------------------------------------------------------------------------
	//! Unemployed if it exists, otherwise the first background that isn't locked.
	RZ_Background GetDefaultBackground()
	{
		foreach (RZ_Background bg : m_aBackgrounds)
		{
			if (bg.m_sId == "unemployed")
				return bg;
		}

		foreach (RZ_Background bg2 : m_aBackgrounds)
		{
			if (!bg2.m_bLocked)
				return bg2;
		}

		return m_aBackgrounds[0];
	}

	//------------------------------------------------------------------------------------------------
	RZ_Background GetPlayerBackground(int playerId)
	{
		return GetBackground(GetProfile(playerId).m_sBackgroundId);
	}

	//------------------------------------------------------------------------------------------------
	//! The background is whichever loadout the player picked in the deploy menu.
	protected RZ_Background ResolveBackground(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (pc)
		{
			SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
			if (loadoutComp)
			{
				SCR_BasePlayerLoadout loadout = loadoutComp.GetLoadout(); // VERIFY
				if (loadout)
				{
					string loadoutName = loadout.GetLoadoutName();
					foreach (RZ_Background bg : m_aBackgrounds)
					{
						if (bg.m_sLoadoutName == loadoutName)
							return bg;
					}

					RZ_Log.Warn("No background matches loadout '" + loadoutName + "'");
				}
			}
		}

		return GetDefaultBackground();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		RZ_Background bg = ResolveBackground(playerId);
		RZ_PlayerProfile profile = GetProfile(playerId);
		OnLegacyChanged(playerId); // older profiles may already have 10 Legacy

		// Locked backgrounds need an unlock. The loadout's gear stays (we can't take it back
		// cleanly), but the perks, flaws and standing are the default background's.
		if (bg.m_bLocked && !profile.HasUnlocked(bg.m_sUnlockId))
		{
			string why = bg.m_sUnlockText;
			if (why.IsEmpty())
				why = "You haven't unlocked it yet.";

			RZ_Background fallback = GetDefaultBackground();
			NotifyPlayer(playerId, bg.m_sName + " is locked", string.Format("%1\nYou are playing as %2 this life.", why, fallback.m_sName));
			bg = fallback;
		}

		profile.StartNewLife(bg);
		// Experience and refresher first: the first life that counts gets "Find the Haven"
		m_Guidance.BeginLife(playerId, profile);
		RZ_Objectives.Assign(this, profile, bg, controlledEntity.GetOrigin());

		m_BiteDetector.Reset(playerId);
		RegisterGunfire(controlledEntity);
		// Keys, timed actions and cracked channels belong to the last life
		m_Interactions.OnPlayerSpawned(playerId);

		// 1. Who you are this life
		string intro = string.Format("Perk: %1\nFlaw: %2\n\nYour goal: %3",
			bg.m_sPerk, bg.m_sFlaw, RZ_Objectives.Describe(this, profile));
		NotifyPlayer(playerId, bg.m_sName, intro);

		// Where you wake: a small clue nearby, and the wind-up radio (new-player guidance)
		m_Guidance.OnPlayerSpawned(playerId, controlledEntity);

		// 2. The state of the world (first spawn of this session only)
		if (!m_aWelcomed.Contains(playerId))
		{
			m_aWelcomed.Insert(playerId);
			string lab = "";
			if (m_bHasLab)
			{
				lab = string.Format("\nHalcyon's lab: %1. Bring it samples from special infected.",
					RZ_Util.DirectionText(controlledEntity.GetOrigin(), m_vLabPos));
			}

			NotifyPlayer(playerId, "Outbreak: " + RZ_StageSettings.GetStageName(m_Director.GetStage()),
				string.Format("Cure research: %1 percent.%2", RZ_Util.RoundToInt(m_Cure.m_fProgress), lab));

			if (!m_Cure.IsCured())
				NotifyPlayer(playerId, RZ_Broadcast.TITLE, m_Broadcast.GetText());

			// Session news: the latest headline, wanderers lost, the biggest rescue, the cure
			string news = m_Headlines.Build();
			if (!news.IsEmpty())
				NotifyPlayer(playerId, "Word on the roads", news);
		}

		// 3. What's on the radio
		NotifyPlayer(playerId, "Radio", m_Missions.DescribeActiveFor(controlledEntity, playerId));

		// The deploy menu can't hide a safehouse spawn point from one player, so an exile
		// who picks one is told the door stays shut (its services refuse them)
		if (m_Safehouses.FindAt(controlledEntity.GetOrigin()) >= 0 && m_Safehouses.IsExiled(playerId))
			NotifyPlayer(playerId, "The Haven", "You come to outside the safehouse door. It stays shut. Nobody inside will look at you, and if you're carrying a gun, the guards will in a minute. Move on.");
		m_Perks.OnPlayerSpawned(playerId, controlledEntity);

		// The new life's standing decides which hidden variant the character goes on
		m_Hostility.OnPlayerSpawned(playerId, controlledEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! World events: every player gets the brief with the distance and direction from
	//! where they are. Contracts are offered by RZ_MissionManager instead.
	void AnnounceMission(RZ_Mission mission)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			string where = mission.GetLocationName(this);
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (ent)
				where = mission.GetWhereText(this, ent.GetOrigin());

			NotifyPlayer(playerId, "World event: " + mission.m_sTitle, string.Format("%1\n(%2)", mission.m_sBrief, where));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Map marker for a world event or the lab. Returns a handle for RemoveMapMarker, -1 if off.
	int AddMapMarker(vector pos, string text)
	{
		if (!m_bMapMarkers)
			return -1;

		return RZ_MapMarkers.Add(pos, text);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveMapMarker(int handle)
	{
		RZ_MapMarkers.Remove(handle);
	}

	// ================================================================ Gunfire
	//------------------------------------------------------------------------------------------------
	protected void RegisterGunfire(IEntity character)
	{
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(character.FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RegisterScriptHandler("OnProjectileShot", this, OnPlayerShot); // VERIFY: event name and signature
	}

	//------------------------------------------------------------------------------------------------
	//! Every shot is heard. Hunters are quieter, Mechanics and Radio Operators louder,
	//! and a suppressor cuts the noise (m_fSuppressedHeatMult).
	protected void OnPlayerShot(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		IEntity shooter = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!shooter)
			return;

		float heat = m_fHeatPerShot * GetPlayerBackground(playerID).m_fNoiseMult;
		if (RZ_Util.IsSuppressed(weapon))
			heat = heat * m_fSuppressedHeatMult;

		m_Heat.AddHeat(shooter.GetOrigin(), heat);

		// House rules and wanderers who were shot at
		m_Safehouses.OnPlayerShot(playerID, shooter.GetOrigin());
		m_Wanderers.OnPlayerShot(playerID, shooter.GetOrigin());
		m_Leaders.OnPlayerShot(playerID, shooter.GetOrigin());
		m_Missions.OnPlayerShot(playerID, shooter.GetOrigin());
		// Retaliation: who fired, and when (a hit on a faction's AI is matched to it)
		m_Hostility.OnPlayerShot(playerID, shooter.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	protected void BiteCheck()
	{
		m_BiteDetector.Check();
	}

	//------------------------------------------------------------------------------------------------
	protected void LogStatus(int playerCount)
	{
		int safe, contested, infested, overrun;
		foreach (RZ_Settlement s : m_aSettlements)
		{
			switch (s.GetState())
			{
				case RZ_ESettlementState.SAFE: safe++; break;
				case RZ_ESettlementState.CONTESTED: contested++; break;
				case RZ_ESettlementState.INFESTED: infested++; break;
				case RZ_ESettlementState.OVERRUN: overrun++; break;
			}
		}

		RZ_Log.Info(string.Format("Status | %1 (%2 min) | avg infection %3 | towns safe/contested/infested/overrun %4/%5/%6/%7 | zombies %8 | missions %9",
			RZ_StageSettings.GetStageName(m_Director.GetStage()), RZ_Util.RoundToInt(m_Director.GetStageMinutes()),
			RZ_Util.RoundToInt(m_Director.GetAverageInfection()), safe, contested, infested, overrun,
			m_Spawner.GetTrackedCount(), m_Missions.GetActive().Count()));
		RZ_Log.Info(string.Format("Status | players %1 | cure %2 percent (milestone %3, final stand %4) | %5",
			playerCount, RZ_Util.RoundToInt(m_Cure.m_fProgress), m_Cure.m_iMilestone, m_Cure.IsFinalStand(), m_Environment.Describe()));
	}

	// ================================================================ Reputation (see docs/REPUTATION.md)
	//------------------------------------------------------------------------------------------------
	RZ_ETier GetTier(int playerId, RZ_EFaction faction)
	{
		if (playerId <= 0)
			return RZ_ETier.STRANGER;

		return GetProfile(playerId).GetTier(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Fame with `faction`. With applyRivals, the faction's rivals also add a little
	//! Infamy (RZ_Reputation.GetRivalShare). Negative amounts lower Fame.
	//! The player is only told when their tier changes, not on every point.
	void AddFame(int playerId, RZ_EFaction faction, int amount, bool applyRivals = true)
	{
		if (amount == 0 || playerId <= 0 || !RZ_Factions.IsValid(faction))
			return;

		RZ_PlayerProfile profile = GetProfile(playerId);
		RZ_ETier before = profile.GetTier(faction);
		int gained = profile.AddFame(faction, amount);
		OnFameGained(faction, gained);
		CheckTierChange(playerId, faction, before);

		if (!applyRivals || amount <= 0)
			return;

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			int infamy = RZ_Util.RoundToInt(amount * RZ_Reputation.GetRivalShare(faction, f) * RivalMult(faction, f));
			if (infamy > 0)
				AddInfamy(playerId, f, infamy);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Rival Infamy between two factions right now: halved during a truce, x1.5 during a feud.
	float RivalMult(int a, int b)
	{
		if (!m_FactionLife)
			return 1;

		return m_FactionLife.RivalMult(a, b);
	}

	//------------------------------------------------------------------------------------------------
	//! A contract ended (RZ_MissionManager.Finish): faction strength and morale move a little.
	void OnContractFinished(RZ_Mission m, bool success)
	{
		if (m_Territory)
			m_Territory.OnContractFinished(m, success);

		if (m_FactionLife)
			m_FactionLife.OnContractFinished(m, success);

		// A story arc waiting for this mission to happen
		if (m_StoryArcs)
			m_StoryArcs.OnMissionFinished(m, success);
	}

	//------------------------------------------------------------------------------------------------
	//! Infamy with `faction`. Negative amounts work Infamy off (amends).
	void AddInfamy(int playerId, RZ_EFaction faction, int amount)
	{
		if (amount == 0 || playerId <= 0 || !RZ_Factions.IsValid(faction))
			return;

		// The faction's stronghold leader sets its temper: Paranoid harsher, Merciful forgives faster
		if (m_Leaders)
			amount = m_Leaders.ScaleInfamy(faction, amount);

		RZ_PlayerProfile profile = GetProfile(playerId);
		RZ_ETier before = profile.GetTier(faction);
		profile.AddInfamy(faction, amount);
		CheckTierChange(playerId, faction, before);
	}

	//------------------------------------------------------------------------------------------------
	void AddFavor(int playerId, RZ_EFaction faction, int amount)
	{
		if (amount == 0 || playerId <= 0 || !RZ_Factions.IsValid(faction))
			return;

		GetProfile(playerId).AddFavor(faction, amount);
	}

	//------------------------------------------------------------------------------------------------
	//! Takes `amount` Favor if the player has it. Returns false (and takes nothing) otherwise.
	bool SpendFavor(int playerId, RZ_EFaction faction, int amount)
	{
		if (playerId <= 0 || !RZ_Factions.IsValid(faction))
			return false;

		if (amount <= 0)
			return true;

		RZ_PlayerProfile profile = GetProfile(playerId);
		if (profile.GetFavor(faction) < amount)
			return false;

		profile.AddFavor(faction, -amount);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! One line per faction: "The Haven: Known (Fame 20, Infamy 0, Favor 35)".
	string DescribeStanding(int playerId)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		string text = "";
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			text += string.Format("%1: %2 (Fame %3, Infamy %4, Favor %5)\n", RZ_Factions.GetName(f),
				RZ_Reputation.GetTierName(profile.GetTier(f)), profile.GetFame(f), profile.GetInfamy(f), profile.GetFavor(f));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckTierChange(int playerId, RZ_EFaction faction, RZ_ETier before)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		RZ_ETier after = profile.GetTier(faction);
		if (after == before)
			return;

		NotifyPlayer(playerId, RZ_Factions.GetName(faction), string.Format("Your standing is now %1 (was %2).",
			RZ_Reputation.GetTierName(after), RZ_Reputation.GetTierName(before)));

		if (after == RZ_ETier.CHAMPION && !profile.IsChampionOf(faction))
			OnBecameChampion(playerId, faction);

		OnTierChanged(playerId, faction, before, after);
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for perks and consequences that start at a tier.
	protected void OnTierChanged(int playerId, RZ_EFaction faction, RZ_ETier before, RZ_ETier after)
	{
		if (m_Perks)
			m_Perks.OnTierChanged(playerId, faction, before, after);

		// Hunted, Kill on Sight, exile and bounties all start at a tier: AI hostility follows
		if (m_Hostility)
			m_Hostility.OnStandingChanged(playerId);

		// First rise with a faction, Watched, Hunted: a line from whoever is near
		if (m_Guidance)
			m_Guidance.OnTierChanged(playerId, faction, before, after);
	}

	//------------------------------------------------------------------------------------------------
	//! Hook: Fame actually gained with a faction (after clamping), for season totals.
	protected void OnFameGained(RZ_EFaction faction, int gained)
	{
		if (gained <= 0)
			return;

		while (m_aSeasonFame.Count() < RZ_Factions.COUNT)
		{
			m_aSeasonFame.Insert(0);
		}

		m_aSeasonFame[faction] = m_aSeasonFame[faction] + gained;
	}

	// ================================================================ Season ending
	//------------------------------------------------------------------------------------------------
	//! Faction the server gave the most Fame this season, -1 if nobody earned any.
	int GetSeasonLeader()
	{
		int best = -1;
		int bestFame = 0;
		foreach (int f, int fame : m_aSeasonFame)
		{
			if (fame > bestFame)
			{
				bestFame = fame;
				best = f;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Called by RZ_CureResearch when the final stand is won. The cure goes to whichever
	//! faction got the most Fame this season (docs/REPUTATION.md). The result is saved
	//! so the next season can read it.
	void OnCureCompleted()
	{
		m_iSeasonWinner = GetSeasonLeader();
		if (m_iSeasonWinner < 0)
			m_iSeasonWinner = RZ_EFaction.INSTITUTE;

		NotifyAll("The cure goes to " + RZ_Factions.GetName(m_iSeasonWinner), GetEndingText(m_iSeasonWinner));
		RZ_Log.Info(string.Format("Season over. Cure goes to %1. Season Fame: Haven %2, Bastion %3, Red Hand %4, Halcyon %5.",
			RZ_Factions.GetName(m_iSeasonWinner), GetSeasonFame(0), GetSeasonFame(1), GetSeasonFame(2), GetSeasonFame(3)));

		RZ_SeasonRecord record = new RZ_SeasonRecord();
		record.world = GetWorldId();
		record.winner = m_iSeasonWinner;
		record.winnerName = RZ_Factions.GetName(m_iSeasonWinner);
		foreach (int fame : m_aSeasonFame)
		{
			record.fame.Insert(fame);
		}

		RZ_Persistence.SaveSeason(record);
		SaveState();
	}

	//------------------------------------------------------------------------------------------------
	int GetSeasonFame(int faction)
	{
		if (faction < 0 || faction >= m_aSeasonFame.Count())
			return 0;

		return m_aSeasonFame[faction];
	}

	//------------------------------------------------------------------------------------------------
	static string GetEndingText(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "The cure spreads from person to person. The Haven opens its doors, and towns begin to rebuild.";
			case RZ_EFaction.MILITARY: return "Martial law. Bastion holds the cure, and it only reaches those who comply.";
			case RZ_EFaction.BANDITS: return "The cure is sold to the highest bidder. The Red Hand owns the roads now.";
			case RZ_EFaction.INSTITUTE: return "Halcyon publishes the cure openly. Infection falls everywhere, but the infected that remain are adapting.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBecameChampion(int playerId, RZ_EFaction faction)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		profile.m_aChampionOf.Insert(faction);

		string name = GetGame().GetPlayerManager().GetPlayerName(playerId);
		NotifyAll("Champion", string.Format("%1 is now a Champion of %2.", name, RZ_Factions.GetName(faction)));
		TryUnlockBackground(playerId, RZ_Reputation.GetChampionUnlock(faction));
	}

	//------------------------------------------------------------------------------------------------
	//! Called when a personal objective adds Legacy. 10 Legacy unlocks the Drifter.
	void OnLegacyChanged(int playerId)
	{
		if (GetProfile(playerId).m_iLegacy >= DRIFTER_LEGACY)
			TryUnlockBackground(playerId, "drifter");
	}

	protected static const int DRIFTER_LEGACY = 10;

	//------------------------------------------------------------------------------------------------
	protected void TryUnlockBackground(int playerId, string unlockId)
	{
		if (!GetProfile(playerId).Unlock(unlockId))
			return;

		foreach (RZ_Background bg : m_aBackgrounds)
		{
			if (bg.m_sUnlockId == unlockId)
			{
				NotifyPlayer(playerId, "Background unlocked", string.Format("%1 is now available for your next life. Pick the '%2' loadout.", bg.m_sName, bg.m_sLoadoutName));
				return;
			}
		}
	}

	// ================================================================ Infection
	//------------------------------------------------------------------------------------------------
	//! Public API: call this from a zombie mod's bite or damage event.
	void InfectPlayer(int playerId)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		if (profile.IsInfected())
			return;

		RZ_Background bg = GetPlayerBackground(playerId);
		float seconds = m_fInfectionMinutes * 60 * bg.m_fInfectionTimeMult * m_Cure.GetInfectionTimeMultiplier();
		profile.m_fInfectionLeft = seconds;
		NotifyPlayer(playerId, "Infected", string.Format(
			"You've been bitten. You have about %1 minutes. Find an antidote, or decide whether to tell anyone. A safehouse medic can slow it, if you tell her.",
			RZ_Util.RoundToInt(seconds / 60)));

		// Whoever is nearest says something about it (always at least a short line)
		m_Guidance.OnBitten(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Public API: call from the antidote item's use action.
	void CurePlayer(int playerId)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		if (!profile.IsInfected())
			return;

		profile.m_fInfectionLeft = -1;
		NotifyPlayer(playerId, "Cured", "The fever breaks. You'll live, for now.");
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateInfections(float dt, array<int> ids)
	{
		foreach (int playerId : ids)
		{
			RZ_PlayerProfile profile = GetProfile(playerId);
			if (!profile.IsInfected())
				continue;

			float before = profile.m_fInfectionLeft;
			profile.m_fInfectionLeft = profile.m_fInfectionLeft - dt;

			if (before > 300 && profile.m_fInfectionLeft <= 300)
				NotifyPlayer(playerId, "Infection", "The fever is getting worse. You have about 5 minutes.");

			if (profile.m_fInfectionLeft <= 0)
			{
				profile.m_fInfectionLeft = -1;
				TurnPlayer(playerId);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The player dies and gets up as one of them.
	protected void TurnPlayer(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent)
			return;

		vector pos = ent.GetOrigin();
		RZ_Util.KillCharacter(ent);

		// World event: the turned player roams with their gear nearby
		RZ_Mission_TurnedPlayer turned = new RZ_Mission_TurnedPlayer();
		turned.Configure(this);
		turned.Prepare(RZ_Util.PlayerName(playerId), pos);
		if (!m_Missions.StartMission(turned))
		{
			m_Spawner.SpawnZombie(pos, false, -1, -1);
			NotifyAll("Turned", RZ_Util.PlayerName(playerId) + " has turned.");
		}
	}

	// ================================================================ Heat
	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerHeat(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity ent : players)
		{
			float noise = GetPlayerBackground(ids[i]).m_fNoiseMult;
			float heat = m_fHeatPresence * noise;
			if (RZ_Util.IsInVehicle(ent))
				heat += m_fHeatVehicle * noise;

			m_Heat.AddHeat(ent.GetOrigin(), heat);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Players near the horde's target are warned, and so are Radio Operators anywhere.
	void NotifyHorde(vector target)
	{
		int idx = FindNearestSettlement(target);
		string where = RZ_SettlementScanner.GridReference(target);
		RZ_Settlement s = GetSettlement(idx);
		if (s)
			where = s.m_sName;

		if (m_FactionLife && s)
			m_FactionLife.OnHorde(s.m_sName);

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), target) < 800)
				NotifyPlayer(ids[i], "Horde", "Something big heard you. A horde is moving in.");
			else if (GetPlayerBackground(ids[i]).m_bRadioIntel)
				NotifyPlayer(ids[i], "Radio chatter", "Someone on the open channel says a horde is moving toward " + where + ".");
		}
	}

	// ================================================================ Zombies, cure, rewards
	//------------------------------------------------------------------------------------------------
	void OnZombiesKilled(int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		RZ_Settlement s = GetSettlement(settlementIdx);
		if (s)
			s.AddInfection(-m_ZombieSettings.m_fInfectionDropPerKill * count);

		if (special)
		{
			SpawnSamples(pos, 1);
			m_Guidance.OnSpecialKilled(pos);
		}

		m_Wanderers.OnZombiesKilled(pos);
		m_Leaders.OnZombiesKilled(pos);
		// A body to search, for a few minutes, if a player saw it die
		m_Interactions.OnZombiesKilled(pos, count);
		m_Missions.OnZombiesKilled(settlementIdx, missionId, special, pos, count);
	}

	//------------------------------------------------------------------------------------------------
	void DeliverSamples(int playerId, int count)
	{
		m_Cure.AddSamples(playerId, count, m_fResearchPerSample);
	}

	//------------------------------------------------------------------------------------------------
	void SpawnSamples(vector pos, int count)
	{
		for (int i = 0; i < count; i++)
		{
			RZ_Util.SpawnPrefab(m_sSamplePrefab, RZ_Util.RandomPointInRing(pos, 0.3, 1.5) + Vector(0, 0.3, 0));
		}
	}

	//------------------------------------------------------------------------------------------------
	void SpawnRewardCrate(vector pos)
	{
		RZ_Util.SpawnPrefab(m_sRewardCratePrefab, RZ_Util.GroundPos(pos), Math.RandomFloat(0, 360));
	}

	//------------------------------------------------------------------------------------------------
	void OnCureChanged()
	{
		UpdateReplicated();
	}

	//------------------------------------------------------------------------------------------------
	void OnStageChanged(RZ_EOutbreakStage stage)
	{
		if (m_FactionLife)
			m_FactionLife.OnStageChanged(stage);

		UpdateReplicated();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateReplicated()
	{
		m_fReplCureProgress = m_Cure.m_fProgress;
		m_bReplCured = m_Cure.IsCured();
		m_iReplStage = m_Director.GetStage();
		Replication.BumpMe();
	}

	// ================================================================ Lab and spawn points
	//------------------------------------------------------------------------------------------------
	protected void SetupLab()
	{
		RZ_LabComponent placed = RZ_LabComponent.GetFirst();
		if (placed)
		{
			m_Lab = placed.GetOwner();
			m_vLabPos = m_Lab.GetOrigin();
			m_bHasLab = true;
			return;
		}

		// Saved position from a previous run
		if (!m_bHasLab)
		{
			int idx = -1;
			foreach (int i, RZ_Settlement s : m_aSettlements)
			{
				if (s.m_bHasLab)
					idx = i;
			}

			if (idx < 0)
				idx = ChooseLabSettlement();

			RZ_Settlement labTown = GetSettlement(idx);
			if (!labTown)
				return;

			labTown.m_bHasLab = true;
			m_vLabPos = RZ_Util.RandomPointInRing(labTown.m_vCenter, 20, Math.Max(30, labTown.m_fRadius * 0.4));
			m_bHasLab = true;
		}

		m_vLabPos = RZ_Util.GroundPos(m_vLabPos);
		m_Lab = RZ_Util.SpawnPrefab(m_sLabPrefab, m_vLabPos);
		if (!m_Lab)
			RZ_Log.Warn("No lab prefab set. Samples can't be delivered until one is configured or placed.");
	}

	//------------------------------------------------------------------------------------------------
	//! Prefer a town (size 2). It's busy enough to be dangerous but not a city death-trap.
	protected int ChooseLabSettlement()
	{
		array<int> towns = {};
		foreach (int i, RZ_Settlement s : m_aSettlements)
		{
			if (s.m_iSize == 2)
				towns.Insert(i);
		}

		if (!towns.IsEmpty())
			return towns.GetRandomElement();

		if (m_aSettlements.IsEmpty())
			return -1;

		return Math.RandomInt(0, m_aSettlements.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn points sit in random, not-too-infected towns and move over time, so
	//! the deploy menu offers different starting places every life.
	protected void RefreshSpawnPoints(bool all)
	{
		if (m_sSpawnPointPrefab.IsEmpty() || m_aSettlements.IsEmpty())
			return;

		if (all)
		{
			foreach (IEntity e : m_aSpawnPoints)
			{
				RZ_Util.DeleteUnit(e);
			}

			m_aSpawnPoints.Clear();
		}
		else if (!m_aSpawnPoints.IsEmpty())
		{
			RZ_Util.DeleteUnit(m_aSpawnPoints[0]);
			m_aSpawnPoints.Remove(0);
		}

		// Drop references to anything deleted elsewhere
		for (int i = m_aSpawnPoints.Count() - 1; i >= 0; i--)
		{
			if (!m_aSpawnPoints[i])
				m_aSpawnPoints.Remove(i);
		}

		array<int> candidates = {};
		foreach (int idx, RZ_Settlement s : m_aSettlements)
		{
			if (s.m_fInfection < 70)
				candidates.Insert(idx);
		}

		if (candidates.IsEmpty())
		{
			foreach (int idx2, RZ_Settlement s2 : m_aSettlements)
			{
				candidates.Insert(idx2);
			}
		}

		int guard = 0;
		while (m_aSpawnPoints.Count() < m_iSpawnPointCount && guard < 50)
		{
			guard++;
			RZ_Settlement town = m_aSettlements[candidates.GetRandomElement()];
			vector pos = RZ_Util.RandomPointInRing(town.m_vCenter, 0, town.m_fRadius * 0.8);
			if (RZ_Util.IsWater(pos) || m_Perimeter.IsNearLine(pos))
				continue;

			IEntity ent = SpawnNamedSpawnPoint(pos, town.m_sName);
			if (!ent)
				return;

			m_aSpawnPoints.Insert(ent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player spawn point (the World > Spawn point prefab) with a name in the deploy menu.
	//! Null without the prefab. Also used for the Haven safehouses' permanent spawn points.
	IEntity SpawnNamedSpawnPoint(vector pos, string name)
	{
		IEntity ent = RZ_Util.SpawnPrefab(m_sSpawnPointPrefab, pos, Math.RandomFloat(0, 360));
		if (!ent)
			return null;

		SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(ent);
		if (sp)
		{
			sp.SetFactionKey(m_sPlayerFactionKey); // VERIFY
			sp.SetSpawnPointName(name); // VERIFY
		}

		return ent;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	protected string GetWorldId()
	{
		return GetGame().GetWorldFile(); // VERIFY
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoadState()
	{
		RZ_SaveData data = RZ_Persistence.Load();
		if (!data)
			return false;

		if (data.world != GetWorldId())
		{
			RZ_Log.Info("Saved state is from another map, starting a fresh outbreak.");
			return false;
		}

		foreach (RZ_SettlementSave ss : data.settlements)
		{
			RZ_Settlement s = new RZ_Settlement();
			s.m_sName = ss.name;
			s.m_vCenter = Vector(ss.x, ss.y, ss.z);
			s.m_fRadius = ss.radius;
			s.m_iSize = ss.size;
			s.m_fInfection = ss.infection;
			s.m_bHasLab = ss.lab;
			m_aSettlements.Insert(s);
		}

		foreach (RZ_PlayerProfile p : data.profiles)
		{
			p.EnsureArrays(); // profiles from older saves have no Fame/Infamy/Favor yet
			m_mProfiles.Set(p.m_sIdentity, p);
		}

		m_Director.Restore(data.stage, data.stageMinutes);
		m_Cure.Restore(data.cureProgress, data.cureMilestone, data.cured, data.labAntidotes, data.stolenAntidotes);
		if (data.seasonFame)
		{
			for (int f = 0; f < data.seasonFame.Count() && f < RZ_Factions.COUNT; f++)
			{
				m_aSeasonFame[f] = data.seasonFame[f];
			}
		}

		m_iSeasonWinner = data.seasonWinner;

		m_bHasLab = data.hasLab;
		m_vLabPos = Vector(data.labX, 0, data.labZ);
		m_Perks.RestoreQuartermasters(data.qmX, data.qmZ, data.qmSet);
		m_Safehouses.Restore(data.safehouses);
		m_Wanderers.Restore(data.wanderers);
		m_Leaders.Restore(data.leaders);
		m_Territory.Restore(data.territory);
		m_Supply.Restore(data.supply);
		m_FactionLife.Restore(data.factionLife);
		m_TownMemory.Restore(data.towns);
		m_PowerGrid.Restore(data.power);
		m_RadioStation.Restore(data.radio);
		m_StoryArcs.Restore(data.story);
		m_Headlines.Restore(data.news);
		m_Interactions.Restore(data.interactions);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveState()
	{
		if (!m_Director)
			return;

		RZ_SaveData data = new RZ_SaveData();
		data.world = GetWorldId();
		data.stage = m_Director.GetStage();
		data.stageMinutes = m_Director.GetStageMinutes();
		data.cureProgress = m_Cure.m_fProgress;
		data.cureMilestone = m_Cure.m_iMilestone;
		data.cured = m_Cure.IsCured();
		data.labAntidotes = m_Cure.GetLabAntidotes();
		data.stolenAntidotes = m_Cure.GetStolenAntidotes();
		data.seasonWinner = m_iSeasonWinner;
		foreach (int fame : m_aSeasonFame)
		{
			data.seasonFame.Insert(fame);
		}
		data.hasLab = m_bHasLab;
		data.labX = m_vLabPos[0];
		data.labZ = m_vLabPos[2];
		if (m_Perks)
			m_Perks.SaveQuartermasters(data.qmX, data.qmZ, data.qmSet);

		if (m_Safehouses)
			m_Safehouses.Save(data.safehouses);

		if (m_Wanderers)
			data.wanderers = m_Wanderers.Save();

		if (m_Leaders)
			data.leaders = m_Leaders.Save();

		if (m_Territory && m_Territory.IsActive())
			data.territory = m_Territory.Save();

		if (m_Supply && m_Supply.IsActive())
			data.supply = m_Supply.Save();

		if (m_FactionLife && m_FactionLife.IsActive())
			data.factionLife = m_FactionLife.Save();

		if (m_TownMemory && m_TownMemory.IsEnabled())
			data.towns = m_TownMemory.Save();

		if (m_PowerGrid && m_PowerGrid.IsActive())
			data.power = m_PowerGrid.Save();

		if (m_RadioStation && m_RadioStation.IsActive())
			data.radio = m_RadioStation.Save();

		if (m_StoryArcs && m_StoryArcs.IsEnabled())
			data.story = m_StoryArcs.Save();

		if (m_Headlines)
			data.news = m_Headlines.Save();

		if (m_Interactions && m_Interactions.IsActive())
			data.interactions = m_Interactions.Save();

		foreach (RZ_Settlement s : m_aSettlements)
		{
			RZ_SettlementSave ss = new RZ_SettlementSave();
			ss.name = s.m_sName;
			ss.x = s.m_vCenter[0];
			ss.y = s.m_vCenter[1];
			ss.z = s.m_vCenter[2];
			ss.radius = s.m_fRadius;
			ss.size = s.m_iSize;
			ss.infection = s.m_fInfection;
			ss.lab = s.m_bHasLab;
			data.settlements.Insert(ss);
		}

		foreach (string ident, RZ_PlayerProfile p : m_mProfiles)
		{
			data.profiles.Insert(p);
		}

		RZ_Persistence.Save(data);
	}

	// ================================================================ Notifications
	//------------------------------------------------------------------------------------------------
	void NotifyAll(string title, string text)
	{
		RpcDo_Notify(-1, title, text);
		Rpc(RpcDo_Notify, -1, title, text);
	}

	//------------------------------------------------------------------------------------------------
	void NotifyPlayer(int playerId, string title, string text)
	{
		RpcDo_Notify(playerId, title, text);
		Rpc(RpcDo_Notify, playerId, title, text);
	}

	//------------------------------------------------------------------------------------------------
	//! Broadcast, with each client filtering for itself. Fine at this message rate.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Notify(int targetPlayerId, string title, string text)
	{
		if (System.IsConsoleApp())
			return; // dedicated server has no screen

		if (targetPlayerId != -1 && SCR_PlayerController.GetLocalPlayerId() != targetPlayerId)
			return;

		QueueHint(title, text);
	}

	// ---------------------------------------------------------------- Client hint queue
	//! The hint widget shows one hint at a time, so messages that arrive together
	//! (e.g. the three on spawn) are queued and shown one after another.
	protected ref array<string> m_aHintTitles = {};
	protected ref array<string> m_aHintTexts = {};
	protected bool m_bHintShowing;

	protected static const int HINT_SECONDS = 9;
	protected static const int HINT_QUEUE_MAX = 8;

	//------------------------------------------------------------------------------------------------
	protected void QueueHint(string title, string text)
	{
		// Under heavy traffic, drop the oldest messages rather than lag minutes behind
		if (m_aHintTitles.Count() >= HINT_QUEUE_MAX)
		{
			m_aHintTitles.Remove(0);
			m_aHintTexts.Remove(0);
		}

		m_aHintTitles.Insert(title);
		m_aHintTexts.Insert(text);
		if (!m_bHintShowing)
			ShowNextHint();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowNextHint()
	{
		if (m_aHintTitles.IsEmpty())
		{
			m_bHintShowing = false;
			return;
		}

		m_bHintShowing = true;
		string title = m_aHintTitles[0];
		string text = m_aHintTexts[0];
		m_aHintTitles.Remove(0);
		m_aHintTexts.Remove(0);

		SCR_HintManagerComponent.ShowCustomHint(text, title, HINT_SECONDS); // VERIFY
		GetGame().GetCallqueue().CallLater(ShowNextHint, HINT_SECONDS * 1000, false);
	}
}
