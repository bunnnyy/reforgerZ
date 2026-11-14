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

	[Attribute("SURV", desc: "Faction key players spawn as (the spawn screen, chat, groups and map markers stay on it; characters move onto its hidden variants, see Factions > Hostility)", category: "ReforgerZ")]
	protected string m_sPlayerFactionKey;

	[Attribute("0", desc: "Ignore the saved state on start (season wipe)", category: "ReforgerZ")]
	protected bool m_bWipeOnStart;

	[Attribute("120", desc: "Seconds between state saves", category: "ReforgerZ")]
	protected float m_fSaveIntervalSeconds;

	[Attribute(desc: "Seasons: the last winner's starting rules, rule sets (Winter, Fog, Military Collapse, No Vehicles, your own), the wipe cycle and the hall of fame (docs/DESIGN.md, Seasons)", category: "ReforgerZ")]
	protected ref RZ_SeasonSettings m_SeasonSettings;

	[Attribute("1", UIWidgets.Slider, "Season speed: everything that moves the season along runs this many times as fast: research and the lab's work, outbreak stages and spread, territory, supply trucks, safehouse upkeep, the Long Table. 1 = the design point, about 45 real days (Cure > Pace by play fits the research to how much the server plays); 1.5 for about a month, 2 for about three and a half weeks. Cure > Cure pace changes only the research (docs/DESIGN.md, Season pacing)", "0.25 4 0.05", category: "ReforgerZ")]
	protected float m_fSeasonSpeed;

	[Attribute("4", UIWidgets.Slider, "Day length: real hours a whole game day takes (the engine's day duration; 4 = x6, a night of about 85 minutes; at most x12, so 2 hours or more). The infected's design assumes about 4 (docs/INFECTED.md). 0 = leave the server's own clock", "0 24 0.5", category: "ReforgerZ")]
	protected float m_fDayLengthHours;

	[Attribute("1", desc: "Keep the time of day across restarts: saved in state.json and set again at start (the engine's clock goes back to the scenario's start on a restart). Not a time skip: the day carries on where it stopped", category: "ReforgerZ")]
	protected bool m_bKeepTimeOfDay;

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

	[Attribute("30", desc: "Loose items: reward crates, dropped samples, key items, purchases and finds left lying this many real minutes with no player within 150 m (and not at a running mission) are removed. Never from a pack or a vehicle. 0 = off (docs/DESIGN.md, Loose items)", category: "ReforgerZ|World")]
	protected float m_fLooseItemMinutes;

	[Attribute(desc: "Town memory: Looted, Barricaded, Powered, Burned per town as data, loot that returns, journals of the dead (docs/DESIGN.md, Town memory)", category: "ReforgerZ|World")]
	protected ref RZ_TownMemorySettings m_TownMemorySettings;

	[Attribute(desc: "Power grid: a power station players can restore (control object prefab, region, noise, upkeep; docs/DESIGN.md, Time and environment)", category: "ReforgerZ|World")]
	protected ref RZ_PowerSettings m_PowerSettings;

	[Attribute(desc: "Radio station: a broadcast point players can take over to send preset messages (console prefab, cooldown; cosmetic only, docs/DESIGN.md, Time and environment)", category: "ReforgerZ|World")]
	protected ref RZ_RadioStationSettings m_RadioStationSettings;

	[Attribute(desc: "Wildlife: deer herds out in the country near players, from the Workshop mod WildlifeExpansion (doe and buck prefabs, caps, meat; docs/DESIGN.md, Wildlife). Off while the doe prefab is empty", category: "ReforgerZ|World")]
	protected ref RZ_WildlifeSettings m_WildlifeSettings;

	[Attribute(desc: "Placed things: what players build (a fireplace, a tent a mod brings) is cleared after a few real days unused (prefab keywords, days, radii, cap; docs/DESIGN.md, Things players build)", category: "ReforgerZ|World")]
	protected ref RZ_PlacedSettings m_PlacedSettings;

	[Attribute(desc: "The Long Table: some seasons, one small family in one farmhouse in an empty part of the map, who are kind to strangers and eat people; its chance, hunger, takings, the host on the road, the guest rule, trade keywords (docs/DESIGN.md, The Long Table)", category: "ReforgerZ|World")]
	protected ref RZ_LongTableSettings m_LongTableSettings;

	[Attribute(desc: "Traces: the world's environmental storytelling. Door marks written by the people who were there (Bastion's search code, the Haven's bears and candles, the Red Hand's hands and tallies, Halcyon's beaks, residents' words, the infected's scratches), read by standing at a door; decals or boards; player traces; the kit of optional prefabs (docs/TRACES.md)", category: "ReforgerZ|World")]
	protected ref RZ_TraceSettings m_TraceSettings;

	// ---------------------------------------------------------------- Director / zombies
	[Attribute(desc: "Stage overrides. Leave empty to use the built-in defaults", category: "ReforgerZ|Director")]
	protected ref array<ref RZ_StageSettings> m_aStages;

	[Attribute("25", desc: "Minutes between government broadcasts to everyone (flavour only; also sent on first join). 0 = only on first join", category: "ReforgerZ|Director")]
	protected float m_fBroadcastMinutes;

	[Attribute(desc: "Time and weather: night (they hear further, more specials, fewer human patrols), rain, fog, storms grounding the helicopter (docs/DESIGN.md, Time and environment)", category: "ReforgerZ|Director")]
	protected ref RZ_EnvironmentSettings m_EnvironmentSettings;

	[Attribute(desc: "Story arcs: short storylines made of missions, rumours and headlines, including the Haven's rare dark side (docs/DESIGN.md, Story arcs and headlines)", category: "ReforgerZ|Director")]
	protected ref RZ_StorySettings m_StorySettings;

	[Attribute(desc: "Questlines: long faction storylines in stages (contracts and waits) where how you do each stage changes the ending, saved across restarts (docs/DESIGN.md, Questlines)", category: "ReforgerZ|Director")]
	protected ref RZ_QuestlineSettings m_QuestlineSettings;

	[Attribute(desc: "Majors: one season-long story per faction, climaxes on cure milestones, defaults that fire from what's happening on the server (docs/DESIGN.md, The majors)", category: "ReforgerZ|Director")]
	protected ref RZ_MajorSettings m_MajorSettings;

	[Attribute(desc: "The cure finale (docs/FINALE.md): at 90 percent five routes race to finish the batch, a lock per player for the season, the stand as a fight for the clean room, the finisher's capped Fame chunk, terms", category: "ReforgerZ|Director")]
	protected ref RZ_FinaleSettings m_FinaleSettings;

	[Attribute(desc: "The epilogue week (docs/EPILOGUE.md): after the cure the world tells the season for Season > Epilogue hours: the record, beats at places, the dose outlets, the queue at the north road, leaders' season topics, the grey choices and the closing work, the hall of fame frozen at the cure", category: "ReforgerZ|Director")]
	protected ref RZ_EpilogueSettings m_EpilogueSettings;

	[Attribute(desc: "Zombie prefabs and density", category: "ReforgerZ|Zombies")]
	protected ref RZ_ZombieSettings m_ZombieSettings;

	[Attribute(desc: "Infection adapts a little: specials after breakthroughs, one trait per stage (runner, helmet, screamer prefabs), favourite spots (docs/DESIGN.md, Infection adapts)", category: "ReforgerZ|Zombies")]
	protected ref RZ_AdaptationSettings m_AdaptationSettings;

	[Attribute(desc: "Stealth: each player's visibility (stance, speed, light, weather, noise), the infected near them seeing the most visible one, losing a hidden player, the missions' sight and notice rules (docs/DESIGN.md, Stealth)", category: "ReforgerZ|Zombies")]
	protected ref RZ_StealthSettings m_StealthSettings;

	[Attribute(desc: "Tribes (docs/INFECTED.md): the land grid (wild land, read once and cached), packs in the wild with lairs, scouts, day parties, dusk calls and night hunts, the agent budget, the engine's active-AI limit, loose ones walking somewhere and heat hordes from towns", category: "ReforgerZ|Zombies")]
	protected ref RZ_TribeSettings m_TribeSettings;

	[Attribute(desc: "Behaviour (docs/INFECTED.md, How they move and hunt): our code drives the tribes' bodies beyond striking range (blind steering, formations, flanks, stalks, the climb gate, water and stairs, speed caps, LOD) and Bacon fights at striking range", category: "ReforgerZ|Zombies")]
	protected ref RZ_BehaviourSettings m_BehaviourSettings;

	[Attribute(desc: "Crawlers (docs/INFECTED.md): the towns' slow infected, placed by the time of day, drifting to noise, the kill ledger, reinforcements from the buildings, stir, scavengers and sieges. Off: towns as before", category: "ReforgerZ|Zombies")]
	protected ref RZ_CrawlerSettings m_CrawlerSettings;

	[Attribute("0.5", desc: "Heat per tick each player gives off just by being somewhere", category: "ReforgerZ|Zombies")]
	protected float m_fHeatPresence;

	[Attribute("4", desc: "Extra heat per tick while in a vehicle", category: "ReforgerZ|Zombies")]
	protected float m_fHeatVehicle;

	// ---------------------------------------------------------------- Backgrounds
	[Attribute(desc: "Civilian backgrounds. Leave empty to use the built-in eight", category: "ReforgerZ|Backgrounds")]
	protected ref array<ref RZ_Background> m_aBackgrounds;

	[Attribute("60", desc: "Base minutes from a bite taking hold to turning, through Fever, Weakness and Final (x background and cure multipliers). Hardcore 40, casual 90", category: "ReforgerZ|Backgrounds")]
	protected float m_fInfectionMinutes;

	// ---------------------------------------------------------------- Cure
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Lab prefab with RZ_LabComponent (spawned if none is placed)", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sLabPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Research sample item with RZ_SampleItemComponent", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sSamplePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Antidote item. From the start the lab makes doses into a stock (faster with each breakthrough); quartermaster shop actions selling this prefab draw on it. Rare finds in pharmacies and on bodies", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sAntidotePrefab;

	[Attribute("1", desc: "Research percent per sample (x the background and Halcyon Champion multipliers, x2 at most together; x Cure pace and season speed)", category: "ReforgerZ|Cure")]
	protected float m_fResearchPerSample;

	[Attribute("0.75", desc: "Research from missions (data, readings, live specimens) is multiplied by this (x Cure pace and season speed)", category: "ReforgerZ|Cure")]
	protected float m_fMissionResearchMult;

	[Attribute("0.3", desc: "Cure pace: research from samples and missions counts this many times. 0.3 = about 45 real days to the cure (tools/pacing_sim.py); with Pace by play on, that holds from about 2 to 20 regulars. Lower for a longer season, higher for a shorter one. Only the research: the outbreak and everything else keep their pace (docs/DESIGN.md, Season pacing)", category: "ReforgerZ|Cure")]
	protected float m_fCurePace;

	[Attribute("12.5", desc: "Pace by play: the player-hours a day the research pace is set for (12.5 = 5 regulars at 2.5 hours). Research counts x (this / the player-hours really played a day over the last 7 whole real days), x0.25 to x2.5, so a quiet server isn't held back and a busy one doesn't fill the lab. x1 for the season's first two days. 0 = off: research follows Cure pace alone", category: "ReforgerZ|Cure")]
	protected float m_fPlayPaceHours;

	[Attribute("3", desc: "Lab research per day: delivered research waits in the lab's backlog and is worked through at this many percent a real day (x season speed), also while nobody is online. Milestones fire on worked research. 0 = no limit: research counts at once", category: "ReforgerZ|Cure")]
	protected float m_fLabPerDay;

	[Attribute("4", desc: "Lab backlog days: the backlog holds at most this many days of the lab's work, a backstop for spikes. While it's full the lab refuses samples (no Fame or Favor; players keep them) and mission research beyond it is lost. 0 = no cap", category: "ReforgerZ|Cure")]
	protected float m_fLabBacklogDays;

	[Attribute("0.25", desc: "Empty lab bonus: the part of a delivery that fills the backlog up to half a day of the lab's work counts this much more (0.25 = +25%), so an idle lab doesn't waste its days (0 to 1)", category: "ReforgerZ|Cure")]
	protected float m_fEmptyLabBonus;

	[Attribute("1", desc: "Finale fallback: only with a cure finale questline registered (none is built yet), research stops at 90% until a finale takes it on. After a real week at the gate (/ season speed) the lab inches up this many percent a real day by itself (x season speed), so a season whose finale nobody finishes still ends. 0 = never", category: "ReforgerZ|Cure")]
	protected float m_fFinaleFallbackPerDay;

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

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Helicopter wreck prop (optional, enables Crash Site)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sHeliWreckPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Corpse pile prop (optional): the piles in Burn the Bodies and the Memorial. Without it they use the puzzle object prefab", "et", category: "ReforgerZ|Missions")]
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

	[Attribute(desc: "Contracts: escort, live capture, vehicle delivery and health-cost limits, and the optional prefabs of the contracts built on them (docs/DESIGN.md, Mission building blocks)", category: "ReforgerZ|Missions")]
	protected ref RZ_ContractSettings m_ContractSettings;

	[Attribute(desc: "Errands: the small, personal jobs named people at safehouses, posts, camps and stations (and the wanderers) ask, each true right now; the offer gap, held errands, cards a day, pay and its caps, faults, call-outs, telemetry (docs/ERRANDS.md)", category: "ReforgerZ|Missions")]
	protected ref RZ_ErrandSettings m_ErrandSettings;

	[Attribute(desc: "The mission style's framework: the story kit, who decides and who benefits (a switched side pays at most the base), the reason lane and its gap, the one weight clamp (docs/DESIGN.md, Story blocks)", category: "ReforgerZ|Missions")]
	protected ref RZ_MissionStorySettings m_MissionStorySettings;

	[Attribute(desc: "World events: the numbers and optional prefabs of the newer world events (docs/MISSIONS.md, World events)", category: "ReforgerZ|Missions")]
	protected ref RZ_EventSettings m_EventSettings;

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

	[Attribute(desc: "Camp life: each faction's scenery around its holdings, safehouses and road posts (Kits), and its people's routines; piece and extra-people caps, waypoint prefabs (docs/DESIGN.md, Camp life)", category: "ReforgerZ|Territory")]
	protected ref RZ_CampLifeSettings m_CampLifeSettings;

	// ---------------------------------------------------------------- New-player guidance
	[Attribute(desc: "Quiet in-world guidance: experience levels, contextual tips from nearby people, the first-life goal, the gentle start (docs/DESIGN.md)", category: "ReforgerZ|Guidance")]
	protected ref RZ_GuidanceSettings m_GuidanceSettings;

	// ---------------------------------------------------------------- World interactions
	[Attribute(desc: "World interactions: the interaction spot prefab, scan radius and caps, bells and sirens, water, a medic slowing a bite (docs/DESIGN.md, World interactions)", category: "ReforgerZ|Interactions")]
	protected ref RZ_InteractionSettings m_InteractionSettings;

	[Attribute(desc: "Search: loot pool and tables per building type, chances, locks and keys, readable things, car boots and bodies", category: "ReforgerZ|Interactions")]
	protected ref RZ_SearchSettings m_SearchSettings;

	[Attribute(desc: "Loose loot in houses: spawn and despawn radius, refill minutes, items per building type, the server cap, town loot per item (docs/DESIGN.md, Loot in houses)", category: "ReforgerZ|Interactions")]
	protected ref RZ_LootSettings m_LootSettings;

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

	[Attribute("0.1", desc: "Base chance that a zombie hit is an infecting bite rather than a scratch (more when surrounded, grabbed, tired or hurt; less in a jacket, gloves, helmet). Hardcore 0.15, casual 0.07. 0 disables built-in bite detection", category: "ReforgerZ|Survival")]
	protected float m_fBiteChance;

	[Attribute(desc: "Infection: bite modifiers and protective clothing, the golden window and wound cleaning, the stages, treatments, their prefabs and where they're found (docs/DESIGN.md, Zombies, bites and infection)", category: "ReforgerZ|Survival")]
	protected ref RZ_InfectionSettings m_InfectionSettings;

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

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven quartermaster looks (optional): variants of the quartermaster in other clothes; each one spawned picks one by its place (docs/DESIGN.md, People's looks). Empty = the quartermaster prefab", "et", category: "ReforgerZ|Factions")]
	protected ref array<ResourceName> m_aQuartermasterHavenLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion quartermaster looks (optional)", "et", category: "ReforgerZ|Factions")]
	protected ref array<ResourceName> m_aQuartermasterBastionLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand quartermaster looks (optional)", "et", category: "ReforgerZ|Factions")]
	protected ref array<ResourceName> m_aQuartermasterRedHandLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon quartermaster looks (optional)", "et", category: "ReforgerZ|Factions")]
	protected ref array<ResourceName> m_aQuartermasterHalcyonLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion hunter squad looks (optional): variant groups whose members wear other kit", "et", category: "ReforgerZ|Factions")]
	protected ref array<ResourceName> m_aBastionHunterLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand hunter squad looks (optional)", "et", category: "ReforgerZ|Factions")]
	protected ref array<ResourceName> m_aRedHandHunterLooks;

	[Attribute("10", desc: "Minutes between hunter squads for Hunted players. 0 = off", category: "ReforgerZ|Factions")]
	protected float m_fHunterSquadMinutes;

	[Attribute("10", desc: "Minutes between public bounty announcements for players at Kill on Sight", category: "ReforgerZ|Factions")]
	protected float m_fBountyMinutes;

	[Attribute(desc: "Per-player AI hostility: mode (bitmask variants, outlaw fallback, off), from which standing each faction's AI shoots, the Haven exile rule, fights, wanderers fighting back (docs/REPUTATION.md)", category: "ReforgerZ|Factions")]
	protected ref RZ_HostilitySettings m_HostilitySettings;

	[Attribute(desc: "Faction services and the last perks: Fire Mission, the fence, the bounty board, Halcyon's medic and immunity booster, paying for amends, reputation clues (docs/REPUTATION.md)", category: "ReforgerZ|Factions")]
	protected ref RZ_ServiceSettings m_ServiceSettings;

	[Attribute(desc: "Bastion checkpoints and Red Hand roadblocks inside the Zone, surrender, camps that warn and refuse, safe zones (docs/REPUTATION.md)", category: "ReforgerZ|Factions")]
	protected ref RZ_CheckpointSettings m_CheckpointSettings;

	[Attribute(desc: "Reputation: the 16 names and what each counts as, the forgiveness budget (30 per faction per life), the credit rule at 75 Fame, the Wildcard tier at posts, threats per life, the change notice (docs/REPUTATION.md, Reputations)", category: "ReforgerZ|Factions")]
	protected ref RZ_ReputationSettings m_ReputationSettings;

	[Attribute(desc: "Voice: the season mood the broadcast and every voice read, the voice bank's extra and disabled lines, and its debug log (docs/WRITING.md, The voice bank)", category: "ReforgerZ|Voice")]
	protected ref RZ_VoiceSettings m_VoiceSettings;

	// ------------------------------------------------------------------------------------------------ Shared
	[Attribute(desc: "The shared engine: notice lanes and the interruption budget, the callback queue, the marks spawner, the season budget and its lanes, the set-piece calendar (docs/DESIGN.md, The shared engine)", category: "ReforgerZ|Shared")]
	protected ref RZ_SharedSettings m_SharedSettings;

	// ---------------------------------------------------------------- Map
	[Attribute("1", desc: "Show world events and the lab on the map as vanilla text markers, when the icon layer isn't set up (UI > Map icon layout). Turn off if markers misbehave", category: "ReforgerZ|Map")]
	protected bool m_bMapMarkers;

	// ---------------------------------------------------------------- UI
	[Attribute(desc: "UI: styled notice cards (off = the vanilla hint), the infection timer, noise meter, radio log size, icon imageset and row layouts (docs/UI.md, UI/LAYOUTS.md)", category: "ReforgerZ|UI")]
	protected ref RZ_UISettings m_UISettings;

	// ---------------------------------------------------------------- Dialogue
	[Attribute(desc: "Talking to people: the talk screen, its range and idle time, and the danger that closes it and the reader (docs/DESIGN.md, Talking to people)", category: "ReforgerZ|Dialogue")]
	protected ref RZ_DialogueSettings m_DialogueSettings;

	// ---------------------------------------------------------------- Debug
	[Attribute("1", desc: "Log a world status summary every few minutes", category: "ReforgerZ|Debug")]
	protected bool m_bStatusLog;

	// ---------------------------------------------------------------- Replicated summary
	[RplProp()]
	protected float m_fReplCureProgress;
	[RplProp()]
	protected float m_fReplCureBacklog;		// research waiting at the lab
	[RplProp()]
	protected bool m_bReplLabFull;			// the backlog is at its cap: the lab refuses samples
	[RplProp()]
	protected bool m_bReplAtGate;			// research waits at 90 percent for a cure finale
	[RplProp()]
	protected float m_fReplPlayPace;		// the pace by play (RZ_CureResearch.PlayPace), for a sample's worth

	[RplProp()]
	protected int m_iReplStage;

	[RplProp()]
	protected bool m_bReplCured;

	[RplProp()]
	protected string m_sReplLockedBuildings;	// ";key;key;": locked buildings near players (RZ_DoorLocks, for the door action)

	[RplProp()]
	protected string m_sReplDoorMarks;			// marked doors near players, for the clients' decals (World/Traces/RZ_TraceDraw.c)

	[RplProp()]
	protected string m_sReplTraceLights;		// lit windows near players, for the clients' lights (World/Traces/RZ_TraceSounds.c)

	[RplProp()]
	protected string m_sReplPowerLamps;			// where the map's own lamps may light, for the clients (RZ_PowerGrid.PushLamps); "" = left alone

	// ---------------------------------------------------------------- Runtime (server)
	protected static RZ_GameModeComponent s_Instance;

	protected ref array<ref RZ_Settlement> m_aSettlements = {};
	protected ref RZ_Director m_Director;
	protected ref RZ_HeatMap m_Heat;
	protected ref RZ_ZombieSpawner m_Spawner;
	protected ref RZ_Stealth m_Stealth;
	protected ref RZ_Behaviour m_Behaviour;
	protected ref RZ_Adaptation m_Adaptation;
	protected ref RZ_TownMemory m_TownMemory;
	protected ref RZ_Environment m_Environment;
	protected ref RZ_PowerGrid m_PowerGrid;
	protected ref RZ_Wildlife m_Wildlife;
	protected ref RZ_PlacedThings m_PlacedThings;
	protected ref RZ_RadioStation m_RadioStation;
	protected ref RZ_StoryArcs m_StoryArcs;
	protected ref RZ_Questlines m_Questlines;
	protected ref RZ_Headlines m_Headlines;
	protected ref RZ_MissionManager m_Missions;
	protected ref RZ_CureResearch m_Cure;
	protected ref RZ_BiteDetector m_BiteDetector;
	protected ref RZ_Infection m_Infection;
	protected ref RZ_FactionPerks m_Perks;
	protected ref RZ_FactionServices m_Services;
	protected ref RZ_Checkpoints m_Checkpoints;
	protected ref RZ_Broadcast m_Broadcast;
	protected ref RZ_Perimeter m_Perimeter;
	protected ref RZ_Safehouses m_Safehouses;
	protected ref RZ_Wanderers m_Wanderers;
	protected ref RZ_Leaders m_Leaders;
	protected ref RZ_Hostility m_Hostility;
	protected ref RZ_Guidance m_Guidance;
	protected ref RZ_Territory m_Territory;
	protected ref RZ_Looks m_Looks;
	protected ref RZ_SupplyLines m_Supply;
	protected ref RZ_FactionLife m_FactionLife;
	protected ref RZ_CampLife m_CampLife;
	protected ref RZ_LongTable m_LongTable;
	protected ref RZ_Traces m_Traces;
	protected ref RZ_Land m_Land;
	protected ref RZ_Tribes m_Tribes;
	protected bool m_bClockSet;			// the day length and the saved time of day were applied
	protected float m_fSavedHour = -1;	// the time of day from state.json, -1 none
	protected ref RZ_Interactions m_Interactions;
	protected ref RZ_Errands m_Errands;
	protected ref RZ_LooseItems m_LooseItems;
	protected ref RZ_UIFeed m_UIFeed;
	protected ref RZ_ScreenGuard m_ScreenGuard;
	protected ref RZ_Dialogue m_Dialogue;
	protected ref RZ_Seasons m_Seasons;
	protected ref RZ_WorldFacts m_Facts;
	protected ref RZ_Shared m_Shared;
	protected ref RZ_RepWatch m_RepWatch;
	protected ref RZ_CureFinale m_Finale;
	protected ref RZ_Epilogue m_Epilogue;
	protected ref array<int> m_aSeasonFame = {0, 0, 0, 0};	// Fame earned with each faction this season, all players
	protected int m_iSeasonWinner = -1;
	protected float m_fStatusTimer;
	protected ref map<string, ref RZ_PlayerProfile> m_mProfiles = new map<string, ref RZ_PlayerProfile>();
	protected ref map<int, string> m_mIdentities = new map<int, string>();
	protected ref array<IEntity> m_aSpawnPoints = {};
	protected ref RZ_Mainland m_Mainland;
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
	//! Per-session caches keyed by player id are dropped when the player leaves. The identity cache
	//! stays: late callbacks (a reward to someone who just left) still need it to reach the right
	//! saved profile, and ids aren't reused, so it grows by one short entry per connection.
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		if (!m_bServerRunning)
			return;

		m_aWelcomed.RemoveItem(playerId);
		if (m_BiteDetector)
			m_BiteDetector.Forget(playerId);

		if (m_Stealth)
			m_Stealth.Forget(playerId);

		if (m_ScreenGuard)
			m_ScreenGuard.Forget(playerId);

		RZ_ObjectScreen.Forget(playerId);
		RZ_DeathBrief.Forget(playerId);
		if (m_PowerGrid && m_PowerGrid.GetPanel())
			m_PowerGrid.GetPanel().Forget(playerId);

		if (m_Wildlife)
			m_Wildlife.Forget(playerId);

		if (m_Dialogue)
			m_Dialogue.Forget(playerId);

		if (m_Shared)
			m_Shared.Forget(playerId);

		if (m_Epilogue)
			m_Epilogue.Forget(playerId);

		if (m_RepWatch)
			m_RepWatch.Forget(playerId);

		if (m_Errands)
			m_Errands.OnDisconnect(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_GameModeComponent()
	{
		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(ShowNextHint);

		if (m_bServerRunning && GetGame() && GetGame().GetCallqueue())
		{
			GetGame().GetCallqueue().Remove(ServerTick);
			GetGame().GetCallqueue().Remove(SecondTick);
			GetGame().GetCallqueue().Remove(FastTick);
			SaveState();
		}

		if (s_Instance == this)
			s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitServer()
	{
		if (!m_ZombieSettings)
			m_ZombieSettings = RZ_ZombieSettings.Cast(RZ_Util.NewWithDefaults("RZ_ZombieSettings"));

		if (!m_PerimeterSettings)
			m_PerimeterSettings = RZ_PerimeterSettings.Cast(RZ_Util.NewWithDefaults("RZ_PerimeterSettings"));

		if (!m_AdaptationSettings)
			m_AdaptationSettings = RZ_AdaptationSettings.Cast(RZ_Util.NewWithDefaults("RZ_AdaptationSettings"));

		if (!m_StealthSettings)
			m_StealthSettings = RZ_StealthSettings.Cast(RZ_Util.NewWithDefaults("RZ_StealthSettings"));

		if (!m_TribeSettings)
			m_TribeSettings = RZ_TribeSettings.Cast(RZ_Util.NewWithDefaults("RZ_TribeSettings"));

		if (!m_BehaviourSettings)
			m_BehaviourSettings = RZ_BehaviourSettings.Cast(RZ_Util.NewWithDefaults("RZ_BehaviourSettings"));

		if (!m_CrawlerSettings)
			m_CrawlerSettings = RZ_CrawlerSettings.Cast(RZ_Util.NewWithDefaults("RZ_CrawlerSettings"));

		if (!m_TownMemorySettings)
			m_TownMemorySettings = RZ_TownMemorySettings.Cast(RZ_Util.NewWithDefaults("RZ_TownMemorySettings"));

		if (!m_EnvironmentSettings)
			m_EnvironmentSettings = RZ_EnvironmentSettings.Cast(RZ_Util.NewWithDefaults("RZ_EnvironmentSettings"));

		if (!m_PowerSettings)
			m_PowerSettings = RZ_PowerSettings.Cast(RZ_Util.NewWithDefaults("RZ_PowerSettings"));

		if (!m_RadioStationSettings)
			m_RadioStationSettings = RZ_RadioStationSettings.Cast(RZ_Util.NewWithDefaults("RZ_RadioStationSettings"));

		if (!m_WildlifeSettings)
			m_WildlifeSettings = RZ_WildlifeSettings.Cast(RZ_Util.NewWithDefaults("RZ_WildlifeSettings"));

		if (!m_PlacedSettings)
			m_PlacedSettings = RZ_PlacedSettings.Cast(RZ_Util.NewWithDefaults("RZ_PlacedSettings"));

		if (!m_LongTableSettings)
			m_LongTableSettings = RZ_LongTableSettings.Cast(RZ_Util.NewWithDefaults("RZ_LongTableSettings"));

		if (!m_TraceSettings)
			m_TraceSettings = RZ_TraceSettings.Cast(RZ_Util.NewWithDefaults("RZ_TraceSettings"));

		if (!m_StorySettings)
			m_StorySettings = RZ_StorySettings.Cast(RZ_Util.NewWithDefaults("RZ_StorySettings"));

		if (!m_QuestlineSettings)
			m_QuestlineSettings = RZ_QuestlineSettings.Cast(RZ_Util.NewWithDefaults("RZ_QuestlineSettings"));

		if (!m_MajorSettings)
			m_MajorSettings = RZ_MajorSettings.Cast(RZ_Util.NewWithDefaults("RZ_MajorSettings"));

		if (!m_FinaleSettings)
			m_FinaleSettings = RZ_FinaleSettings.Cast(RZ_Util.NewWithDefaults("RZ_FinaleSettings"));

		if (!m_EpilogueSettings)
			m_EpilogueSettings = RZ_EpilogueSettings.Cast(RZ_Util.NewWithDefaults("RZ_EpilogueSettings"));

		if (!m_SafehouseSettings)
			m_SafehouseSettings = RZ_SafehouseSettings.Cast(RZ_Util.NewWithDefaults("RZ_SafehouseSettings"));

		if (!m_WandererSettings)
			m_WandererSettings = RZ_WandererSettings.Cast(RZ_Util.NewWithDefaults("RZ_WandererSettings"));

		if (!m_LeaderSettings)
			m_LeaderSettings = RZ_LeaderSettings.Cast(RZ_Util.NewWithDefaults("RZ_LeaderSettings"));

		if (!m_HostilitySettings)
			m_HostilitySettings = RZ_HostilitySettings.Cast(RZ_Util.NewWithDefaults("RZ_HostilitySettings"));

		if (!m_ServiceSettings)
			m_ServiceSettings = RZ_ServiceSettings.Cast(RZ_Util.NewWithDefaults("RZ_ServiceSettings"));

		if (!m_CheckpointSettings)
			m_CheckpointSettings = RZ_CheckpointSettings.Cast(RZ_Util.NewWithDefaults("RZ_CheckpointSettings"));

		if (!m_GuidanceSettings)
			m_GuidanceSettings = RZ_GuidanceSettings.Cast(RZ_Util.NewWithDefaults("RZ_GuidanceSettings"));

		if (!m_DialogueSettings)
			m_DialogueSettings = RZ_DialogueSettings.Cast(RZ_Util.NewWithDefaults("RZ_DialogueSettings"));

		if (!m_TerritorySettings)
			m_TerritorySettings = RZ_TerritorySettings.Cast(RZ_Util.NewWithDefaults("RZ_TerritorySettings"));

		if (!m_SupplySettings)
			m_SupplySettings = RZ_SupplySettings.Cast(RZ_Util.NewWithDefaults("RZ_SupplySettings"));

		if (!m_FactionLifeSettings)
			m_FactionLifeSettings = RZ_FactionLifeSettings.Cast(RZ_Util.NewWithDefaults("RZ_FactionLifeSettings"));

		if (!m_CampLifeSettings)
			m_CampLifeSettings = RZ_CampLifeSettings.Cast(RZ_Util.NewWithDefaults("RZ_CampLifeSettings"));

		if (!m_InteractionSettings)
			m_InteractionSettings = RZ_InteractionSettings.Cast(RZ_Util.NewWithDefaults("RZ_InteractionSettings"));

		if (!m_SearchSettings)
			m_SearchSettings = RZ_SearchSettings.Cast(RZ_Util.NewWithDefaults("RZ_SearchSettings"));

		if (!m_LootSettings)
			m_LootSettings = RZ_LootSettings.Cast(RZ_Util.NewWithDefaults("RZ_LootSettings"));

		if (!m_CarSettings)
			m_CarSettings = RZ_CarSettings.Cast(RZ_Util.NewWithDefaults("RZ_CarSettings"));

		if (!m_CraftSettings)
			m_CraftSettings = RZ_CraftSettings.Cast(RZ_Util.NewWithDefaults("RZ_CraftSettings"));

		if (!m_TrapSettings)
			m_TrapSettings = RZ_TrapSettings.Cast(RZ_Util.NewWithDefaults("RZ_TrapSettings"));

		if (!m_RadioCrackSettings)
			m_RadioCrackSettings = RZ_RadioCrackSettings.Cast(RZ_Util.NewWithDefaults("RZ_RadioCrackSettings"));

		if (!m_ContractSettings)
			m_ContractSettings = RZ_ContractSettings.Cast(RZ_Util.NewWithDefaults("RZ_ContractSettings"));

		if (!m_EventSettings)
			m_EventSettings = RZ_EventSettings.Cast(RZ_Util.NewWithDefaults("RZ_EventSettings"));

		if (!m_ErrandSettings)
			m_ErrandSettings = RZ_ErrandSettings.Cast(RZ_Util.NewWithDefaults("RZ_ErrandSettings"));

		if (!m_MissionStorySettings)
			m_MissionStorySettings = RZ_MissionStorySettings.Cast(RZ_Util.NewWithDefaults("RZ_MissionStorySettings"));

		if (!m_InfectionSettings)
			m_InfectionSettings = RZ_InfectionSettings.Cast(RZ_Util.NewWithDefaults("RZ_InfectionSettings"));

		if (!m_SeasonSettings)
			m_SeasonSettings = RZ_SeasonSettings.Cast(RZ_Util.NewWithDefaults("RZ_SeasonSettings"));

		if (!m_ReputationSettings)
			m_ReputationSettings = RZ_ReputationSettings.Cast(RZ_Util.NewWithDefaults("RZ_ReputationSettings"));
		if (!m_VoiceSettings)
			m_VoiceSettings = RZ_VoiceSettings.Cast(RZ_Util.NewWithDefaults("RZ_VoiceSettings"));

		if (!m_SharedSettings)
			m_SharedSettings = RZ_SharedSettings.Cast(RZ_Util.NewWithDefaults("RZ_SharedSettings"));

		if (!m_aBackgrounds || m_aBackgrounds.IsEmpty())
		{
			m_aBackgrounds = new array<ref RZ_Background>();
			RZ_Background.CreateDefaults(m_aBackgrounds);
		}

		// First: the season's rules (RZ_Seasons.Mods) are read by the systems below
		m_Seasons = new RZ_Seasons(this, m_SeasonSettings);
		// The shared world facts (season flags, canon, claims) every system and mission reads
		m_Facts = new RZ_WorldFacts(this);
		// The shared engine (notice lanes, callbacks, marks, the season budget and calendar, the
		// season's deeds) and the reputation names' change notice
		m_Shared = new RZ_Shared(this, m_SharedSettings);
		// Faction people's looks: every person slot's variants, read by every spawn of one
		RegisterLooks();
		m_RepWatch = new RZ_RepWatch(this);
		m_Heat = new RZ_HeatMap();
		m_LooseItems = new RZ_LooseItems(this, m_fLooseItemMinutes);
		m_Adaptation = new RZ_Adaptation(this, m_AdaptationSettings);
		m_TownMemory = new RZ_TownMemory(this, m_TownMemorySettings);
		m_Environment = new RZ_Environment(this, m_EnvironmentSettings);
		m_PowerGrid = new RZ_PowerGrid(this, m_PowerSettings);
		m_RadioStation = new RZ_RadioStation(this, m_RadioStationSettings);
		m_Wildlife = new RZ_Wildlife(this, m_WildlifeSettings);
		m_PlacedThings = new RZ_PlacedThings(this, m_PlacedSettings);
		m_StoryArcs = new RZ_StoryArcs(this, m_StorySettings);
		m_Questlines = new RZ_Questlines(this, m_QuestlineSettings);
		// The cure finale's controller, before the cure (its Restore reads whether the stand waits)
		m_Finale = new RZ_CureFinale(this, m_FinaleSettings);
		// The epilogue week (Director/Epilogue/): built at the cure, saved with the season
		m_Epilogue = new RZ_Epilogue(this, m_EpilogueSettings);
		m_Headlines = new RZ_Headlines(this);
		m_Cure = new RZ_CureResearch(this);
		m_Director = new RZ_Director(this, m_aStages);
		m_Spawner = new RZ_ZombieSpawner(this, m_ZombieSettings, m_CrawlerSettings, m_TribeSettings);
		m_Stealth = new RZ_Stealth(this, m_StealthSettings);
		m_Behaviour = new RZ_Behaviour(this, m_BehaviourSettings);
		m_Missions = new RZ_MissionManager(this);
		m_UIFeed = new RZ_UIFeed(this);
		m_ScreenGuard = new RZ_ScreenGuard(this);
		m_Dialogue = new RZ_Dialogue(this);
		m_Infection = new RZ_Infection(this, m_InfectionSettings);
		m_BiteDetector = new RZ_BiteDetector(this, m_fBiteChance, m_fBiteReach, m_InfectionSettings);
		m_Perks = new RZ_FactionPerks(this);
		m_Services = new RZ_FactionServices(this, m_ServiceSettings);
		m_Checkpoints = new RZ_Checkpoints(this, m_CheckpointSettings);
		m_CampLife = new RZ_CampLife(this, m_CampLifeSettings);
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
		m_LongTable = new RZ_LongTable(this, m_LongTableSettings);
		m_Land = new RZ_Land(this, m_TribeSettings);
		m_Tribes = new RZ_Tribes(this, m_TribeSettings);
		m_Interactions = new RZ_Interactions(this, m_InteractionSettings, m_SearchSettings, m_LootSettings, m_CarSettings, m_CraftSettings, m_TrapSettings, m_RadioCrackSettings);
		m_Errands = new RZ_Errands(this, m_ErrandSettings);
		m_Traces = new RZ_Traces(this, m_TraceSettings);

		// A wipe that came due last run (the wipe cycle) or one by hand is carried out now:
		// backup, then fresh
		int wipedSeason;
		bool wipe = RZ_Seasons.TakeWipe(m_bWipeOnStart, wipedSeason);

		bool loaded = false;
		if (!wipe)
			loaded = LoadState();

		// The last finished season's ending: a fresh season starts with the winner's rules
		RZ_SeasonRecord last = RZ_Persistence.LoadSeason();
		if (last)
			RZ_Log.Info(string.Format("Last season (%1, season %2) the cure went to %3.", last.world, last.season, last.winnerName));

		m_Seasons.Begin(loaded, last, wipedSeason);
		// A fresh season (no save, or a wipe) starts with no flags, no canon and a new seed
		m_Facts.Setup();
		// Conversation trees and their memory (dialogue.json, dropped when the season changes)
		m_Dialogue.GetTrees().Load();

		// The mainland (docs/PLACES.md): islands are close to impossible to reach, so a fresh scan keeps
		// no town on one, and every place need checks it
		m_Mainland = new RZ_Mainland();
		m_Mainland.Build();
		if (m_aSettlements.IsEmpty())
		{
			RZ_SettlementScanner scanner = new RZ_SettlementScanner();
			scanner.Scan(m_fScanCellSize, m_iScanMinBuildings, m_aSettlements);
			RZ_Log.Info("Mainland: " + m_Mainland.Describe(m_aSettlements));
			DropIslandTowns();
			DryTownMiddles();
		}
		else
		{
			RZ_Log.Info("Mainland: " + m_Mainland.Describe(m_aSettlements));
			int saved = 0;
			foreach (RZ_Settlement st : m_aSettlements)
			{
				st.m_bIsland = !RZ_Mainland.OnMainland(st.m_vCenter);
				if (st.m_bIsland)
					saved++;
			}

			if (saved > 0)
				RZ_Log.Warn(string.Format("Mainland: %1 of the saved season's towns are on an island; the pickers skip them until the next fresh season", saved));
		}

		if (m_aSettlements.IsEmpty())
			RZ_Log.Error("No settlements found on this map. Place RZ_SettlementMarkerEntity entities in the world.");

		// Before anything is placed, so missions, spawn points and quartermasters keep off the line
		m_Perimeter.Init();
		m_TownMemory.Setup();
		m_PowerGrid.Setup();
		m_Wildlife.Setup();
		m_PlacedThings.Setup();
		m_RadioStation.Setup();

		SetupLab();
		// Fresh season only: starting infection and safe towns from the season's rules. After the
		// lab is chosen, so a safe town is never the lab's; before anything reads infection
		m_Seasons.ApplyStartRules();
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
		// The Long Table: once a season, a farm far from every holding, safehouse and the lab
		m_LongTable.Setup();
		// The land grid (Zombies > Tribes): its wild land keeps clear of the towns, holdings, safehouses,
		// the lab, the Long Table's farm and the line, so after all of them. Loads its cache or starts
		// reading the map, a few squares a second from the one-second tick
		m_Land.Setup();
		// The engine's active-AI limit, raised for the infected's budget; the behaviour layer
		m_Tribes.Setup();
		m_Behaviour.Setup();
		// Searching, cars, crafting, traps, fire, radios: keep off safehouses, holdings and the lab
		m_Interactions.Setup();
		// Arcs are made of the missions, wanderers, safehouses, territory and trucks above
		m_StoryArcs.Setup();
		// Questlines too, and they come back from the save with their stage, flags and people
		m_Questlines.Setup();
		// Errands stand on the safehouses, holdings, wanderers, trucks and the world memory above
		m_Errands.Setup();
		// A fresh season: last season's story rules that aren't season numbers (weights, a dose, flags)
		if (m_Seasons.IsFresh())
			RZ_StoryRules.ApplyFresh(this);

		// Traces (docs/TRACES.md): the first days' door marks from the canon, the safehouses, the
		// holdings, the lab and the Long Table above, under the live ones the save kept
		m_Traces.Setup();
		m_Hostility.Init();
		if (m_bHasLab)
			AddMapMarker(m_vLabPos, "Halcyon lab");
		RefreshSpawnPoints(true);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetOwner());
		if (gameMode)
			gameMode.GetOnPlayerSpawned().Insert(HandlePlayerSpawned);

		// What the lab did while the server was down, now that every system its milestones touch
		// is set up, so the first replicated state (the backlog, the cold store) is current
		m_Cure.CatchUp();
		UpdateReplicated();
		m_bServerRunning = true;
		GetGame().GetCallqueue().CallLater(ServerTick, m_fTickSeconds * 1000, true);
		// Bites need a faster check than the main tick to catch individual hits, and stealth a
		// faster update for the infected near players
		GetGame().GetCallqueue().CallLater(SecondTick, 1000, true);
		// What appears near players (camp life's pieces, a site read for a visit) goes on every
		// tenth of a second, a few ms at a time, so a place is full by the time you get there
		GetGame().GetCallqueue().CallLater(FastTick, 100, true);

		RZ_Log.Info(string.Format("Started. %1 settlements, stage %2, cure %3 percent, state %4.",
			m_aSettlements.Count(), RZ_StageSettings.GetStageName(m_Director.GetStage()),
			RZ_Util.RoundToInt(m_Cure.m_fProgress), loaded));
#ifdef WORKBENCH
		// Workbench play only, and only with $profile:RZ_PhotoTour.txt (docs/PC_SETUP.md, "Photos of the sites")
		RZ_PhotoTour.Start(this);
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void ServerTick()
	{
		float dt = m_fTickSeconds;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);

		// The day length and the saved time of day, once the world's clock is running
		if (!m_bClockSet)
			ApplyClock();

		UpdatePlayerHeat(ids, players);
		m_Heat.Decay(dt);
		m_Director.Tick(dt, !ids.IsEmpty()); // the outbreak's clock runs only while someone plays
		m_Spawner.Tick(dt, players);
		// The tribes after the spawner: their bodies spawn inside the same budget (docs/INFECTED.md)
		m_Tribes.Tick(dt, ids, players);
		m_Adaptation.Tick(dt, ids, players);
		m_TownMemory.Tick(dt);
		m_Environment.Tick(dt, ids, players);
		m_PowerGrid.Tick(dt, ids, players);
		m_Wildlife.Tick(dt, ids, players);
		m_PlacedThings.Tick(dt, ids, players);
		m_RadioStation.Tick(dt, ids, players);
		m_Interactions.Tick(dt, ids, players);
		m_Missions.Tick(dt, ids, players);
		m_Errands.Tick(dt, ids, players);
		m_Dialogue.Tick(dt);
		m_Perimeter.Tick(dt, ids, players);
		m_Cure.Tick(dt, players);
		m_Finale.Tick(dt, ids, players);
		m_Perks.Tick(dt, ids, players);
		m_Services.Tick(dt, ids, players);
		m_Safehouses.Tick(dt, ids, players);
		m_Wanderers.Tick(dt, ids, players);
		m_Leaders.Tick(dt, ids, players);
		m_Territory.Tick(dt, ids, players);
		m_Supply.Tick(dt, ids, players);
		m_FactionLife.Tick(dt, ids, players);
		m_LongTable.Tick(dt, ids, players);
		m_Traces.Tick(dt, ids, players);
		m_StoryArcs.Tick(dt, ids, players);
		m_Questlines.Tick(dt, ids, players);
		m_Checkpoints.Tick(dt, ids, players);
		// After the holdings, safehouses and road posts spawn or remove their bodies
		m_CampLife.Tick(dt, ids, players);
		// After everything that changes standing or spawns fights this tick
		m_Hostility.Tick(dt, ids, players);
		m_Broadcast.Tick(dt, m_fBroadcastMinutes, !ids.IsEmpty());
		RZ_NoisemakerComponent.TickAll();
		// Move waypoints of groups that died out by themselves this tick (RZ_GroupOrders)
		RZ_GroupOrders.Sweep();
		m_Infection.Tick(dt, ids, players);
		m_UIFeed.Tick(dt, ids, players);
		m_LooseItems.Tick(dt, players);
		m_Seasons.Tick(dt, ids);
		m_Facts.Tick(dt, ids);
		m_Shared.Tick(dt, ids, players);
		// The epilogue week: after the shared engine (its marks) and every system it reads
		m_Epilogue.Tick(dt, ids, players);
		m_RepWatch.Tick(dt);

		foreach (int i, IEntity ent : players)
		{
			RZ_Objectives.Tick(this, ids[i], GetProfile(ids[i]), ent, dt);
			RZ_DeathBrief.Tick(ids[i], GetProfile(ids[i]), ent);
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
	RZ_Stealth GetStealth() { return m_Stealth; }
	RZ_Adaptation GetAdaptation() { return m_Adaptation; }
	RZ_TownMemory GetTownMemory() { return m_TownMemory; }
	RZ_Environment GetEnvironment() { return m_Environment; }
	RZ_PowerGrid GetPowerGrid() { return m_PowerGrid; }
	RZ_Wildlife GetWildlife() { return m_Wildlife; }
	RZ_PlacedThings GetPlacedThings() { return m_PlacedThings; }
	//! On every machine: the client's own deer check reads the prefabs from it (null where none is set).
	RZ_WildlifeSettings GetWildlifeSettings() { return m_WildlifeSettings; }
	RZ_RadioStation GetRadioStation() { return m_RadioStation; }
	RZ_StoryArcs GetStoryArcs() { return m_StoryArcs; }
	RZ_Questlines GetQuestlines() { return m_Questlines; }
	RZ_MajorSettings GetMajorSettings() { return m_MajorSettings; }
	RZ_CureFinale GetFinale() { return m_Finale; }
	RZ_Epilogue GetEpilogue() { return m_Epilogue; }
	ResourceName GetSamplePrefab() { return m_sSamplePrefab; }
	ResourceName GetSpawnPointPrefab() { return m_sSpawnPointPrefab; }
	RZ_Mainland GetMainland() { return m_Mainland; }
	RZ_Headlines GetHeadlines() { return m_Headlines; }
	RZ_MissionManager GetMissions() { return m_Missions; }
	RZ_CureResearch GetCure() { return m_Cure; }
	RZ_Infection GetInfection() { return m_Infection; }
	RZ_UIFeed GetUIFeed() { return m_UIFeed; }
	RZ_UISettings GetUISettings() { return m_UISettings; }
	RZ_DialogueSettings GetDialogueSettings() { return m_DialogueSettings; }
	//! Server: open talk screens and readers, closed when danger comes (UI/RZ_ScreenGuard.c).
	RZ_ScreenGuard GetScreenGuard() { return m_ScreenGuard; }
	//! Server: open conversations in the talk screen (Dialogue/RZ_Dialogue.c).
	RZ_Dialogue GetDialogue() { return m_Dialogue; }
	RZ_Seasons GetSeasons() { return m_Seasons; }
	//! What's true in the world right now, for content (docs/DESIGN.md, "World facts").
	RZ_WorldFacts GetFacts() { return m_Facts; }
	//! Server: the shared engine (Core/RZ_Shared.c): notice lanes, callbacks, marks, the season budget.
	RZ_Shared GetShared() { return m_Shared; }
	RZ_SharedSettings GetSharedSettings() { return m_SharedSettings; }
	RZ_ReputationSettings GetReputationSettings() { return m_ReputationSettings; }
	RZ_VoiceSettings GetVoiceSettings() { return m_VoiceSettings; }
	//! Server: the reputation names' change notice and the admin read-out (Factions/RZ_RepNames.c).
	RZ_RepWatch GetRepWatch() { return m_RepWatch; }
	RZ_CheckpointSettings GetCheckpointSettings() { return m_CheckpointSettings; }
	int GetSeasonWinner() { return m_iSeasonWinner; }

	//------------------------------------------------------------------------------------------------
	//! The hall of fame, or null (off, or before the server started).
	RZ_HallOfFame GetHall()
	{
		if (!m_Seasons || !m_Seasons.GetHall() || !m_Seasons.GetHall().IsEnabled())
			return null;

		return m_Seasons.GetHall();
	}
	array<ref RZ_Background> GetBackgrounds() { return m_aBackgrounds; }
	static int GetDrifterLegacy() { return DRIFTER_LEGACY; }
	float GetMissionResearchMult() { return m_fMissionResearchMult; }

	//------------------------------------------------------------------------------------------------
	//! Cure pace (docs/DESIGN.md, Season pacing), 0.02 to 10: research from samples and missions
	//! counts this many times. Nothing else follows it.
	float GetCurePace() { return Math.Clamp(m_fCurePace, 0.02, 10); }

	//------------------------------------------------------------------------------------------------
	//! Pace by play: the player-hours a day the research pace is set for (RZ_CureResearch.PlayPace). 0 = off.
	float GetPlayPaceHours() { return Math.Max(0, m_fPlayPaceHours); }

	//------------------------------------------------------------------------------------------------
	//! Percent of research the lab works through in a real day, before the season speed. 0 = no limit.
	float GetLabPerDay() { return Math.Max(0, m_fLabPerDay); }

	//------------------------------------------------------------------------------------------------
	//! Days of the lab's work its backlog holds. 0 = no cap.
	float GetLabBacklogDays() { return Math.Max(0, m_fLabBacklogDays); }

	//------------------------------------------------------------------------------------------------
	//! The part of a delivery that fills the backlog up to half a day of work counts 1 + this (0 to 1).
	float GetEmptyLabBonus() { return Math.Clamp(m_fEmptyLabBonus, 0, 1); }

	//------------------------------------------------------------------------------------------------
	//! Percent a real day the lab gains by itself after a week at the finale's 90 percent gate. 0 = never.
	float GetFinaleFallbackPerDay() { return Math.Max(0, m_fFinaleFallbackPerDay); }

	//------------------------------------------------------------------------------------------------
	//! Season speed (docs/DESIGN.md, Season pacing), 0.25 to 4. Research gains, the lab's work and the
	//! season's clocks (stages, spread, territory, trucks, upkeep, the Long Table) are scaled by it.
	float GetSeasonSpeed()
	{
		if (m_fSeasonSpeed <= 0)
			return 1;

		return Math.Clamp(m_fSeasonSpeed, 0.25, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! Day length (docs/DESIGN.md, The clock): real hours a game day takes; 0 = the server's own clock.
	float GetDayLengthHours() { return m_fDayLengthHours; }
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
	RZ_FactionServices GetServices() { return m_Services; }
	RZ_ServiceSettings GetServiceSettings() { return m_ServiceSettings; }
	RZ_Checkpoints GetCheckpoints() { return m_Checkpoints; }
	RZ_Perimeter GetPerimeter() { return m_Perimeter; }
	RZ_Safehouses GetSafehouses() { return m_Safehouses; }
	RZ_Wanderers GetWanderers() { return m_Wanderers; }
	RZ_Leaders GetLeaders() { return m_Leaders; }
	RZ_Hostility GetHostility() { return m_Hostility; }
	RZ_Guidance GetGuidance() { return m_Guidance; }
	RZ_Territory GetTerritory() { return m_Territory; }
	RZ_Looks GetLooks() { return m_Looks; }
	RZ_SupplyLines GetSupply() { return m_Supply; }
	RZ_FactionLife GetFactionLife() { return m_FactionLife; }
	RZ_CampLife GetCampLife() { return m_CampLife; }
	//! Camp life's settings as set in Workbench (the client has them too: the build bar's cone at the cursor).
	RZ_CampLifeSettings GetCampLifeSettings() { return m_CampLifeSettings; }
	RZ_LongTable GetLongTable() { return m_LongTable; }
	//! Traces (docs/TRACES.md): door marks, their reading, the player traces. Server only.
	RZ_Traces GetTraces() { return m_Traces; }
	//! World > Traces, on every machine (the clients' decals read the kit).
	RZ_TraceSettings GetTraceSettings() { return m_TraceSettings; }
	//! The land grid (docs/DESIGN.md, The land grid). Check IsReady(): off by default, and read over
	//! the first minutes of the first start.
	RZ_Land GetLand() { return m_Land; }
	RZ_Tribes GetTribes() { return m_Tribes; }
	RZ_Behaviour GetBehaviour() { return m_Behaviour; }
	RZ_BiteDetector GetBiteDetector() { return m_BiteDetector; }
	RZ_Interactions GetInteractions() { return m_Interactions; }
	RZ_GuidanceSettings GetGuidanceSettings() { return m_GuidanceSettings; }
	RZ_ContractSettings GetContractSettings() { return m_ContractSettings; }
	//! Server: the errands (Missions/Errands/RZ_Errands.c), the named people and the faults table.
	RZ_Errands GetErrands() { return m_Errands; }
	RZ_ErrandSettings GetErrandSettings() { return m_ErrandSettings; }
	RZ_MissionStorySettings GetMissionStorySettings() { return m_MissionStorySettings; }
	RZ_EventSettings GetEventSettings() { return m_EventSettings; }
	RZ_CarSettings GetCarSettings() { return m_CarSettings; }
	RZ_TrapSettings GetTrapSettings() { return m_TrapSettings; }
	RZ_SupplySettings GetSupplySettings() { return m_SupplySettings; }
	ResourceName GetLabPrefab() { return m_sLabPrefab; }
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
	//! The look a faction person of this slot wears: one of the slot's variants by the seed (the same
	//! seed, the same clothes), else the prefab itself (RZ_Looks). Use it at every spawn of one.
	ResourceName PickLook(ResourceName prefab, int seed)
	{
		if (!m_Looks)
			return prefab;

		return m_Looks.Pick(prefab, seed);
	}

	//------------------------------------------------------------------------------------------------
	//! Every faction person slot with its looks (docs/DESIGN.md, "People's looks").
	protected void RegisterLooks()
	{
		m_Looks = new RZ_Looks();
		m_Looks.Add(m_sQuartermasterHaven, m_aQuartermasterHavenLooks);
		m_Looks.Add(m_sQuartermasterBastion, m_aQuartermasterBastionLooks);
		m_Looks.Add(m_sQuartermasterRedHand, m_aQuartermasterRedHandLooks);
		m_Looks.Add(m_sQuartermasterHalcyon, m_aQuartermasterHalcyonLooks);
		m_Looks.Add(m_sBastionHunterPrefab, m_aBastionHunterLooks);
		m_Looks.Add(m_sRedHandHunterPrefab, m_aRedHandHunterLooks);

		RZ_TerritorySettings t = m_TerritorySettings;
		m_Looks.Add(t.m_sHavenGuardPrefab, t.m_aHavenGuardLooks);
		m_Looks.Add(t.m_sBastionGuardPrefab, t.m_aBastionGuardLooks);
		m_Looks.Add(t.m_sRedHandGuardPrefab, t.m_aRedHandGuardLooks);
		m_Looks.Add(t.m_sHalcyonGuardPrefab, t.m_aHalcyonGuardLooks);
		m_Looks.Add(t.m_sHavenOccupantPrefab, t.m_aHavenOccupantLooks);
		m_Looks.Add(t.m_sBastionOccupantPrefab, t.m_aBastionOccupantLooks);
		m_Looks.Add(t.m_sRedHandOccupantPrefab, t.m_aRedHandOccupantLooks);
		m_Looks.Add(t.m_sHalcyonOccupantPrefab, t.m_aHalcyonOccupantLooks);

		m_Looks.Add(m_SafehouseSettings.m_sGuardPrefab, m_SafehouseSettings.m_aGuardLooks);

		RZ_CheckpointSettings c = m_CheckpointSettings;
		m_Looks.Add(c.m_sBastionCrewPrefab, c.m_aBastionCrewLooks);
		m_Looks.Add(c.m_sRedHandCrewPrefab, c.m_aRedHandCrewLooks);

		RZ_SupplySettings s = m_SupplySettings;
		m_Looks.Add(s.m_sHavenCrew, s.m_aHavenCrewLooks);
		m_Looks.Add(s.m_sBastionCrew, s.m_aBastionCrewLooks);
		m_Looks.Add(s.m_sRedHandCrew, s.m_aRedHandCrewLooks);
		m_Looks.Add(s.m_sHalcyonCrew, s.m_aHalcyonCrewLooks);

		RZ_LeaderSettings l = m_LeaderSettings;
		m_Looks.Add(l.m_sCouncilHeadPrefab, l.m_aCouncilHeadLooks);
		m_Looks.Add(l.m_sBastionOutpostPrefab, l.m_aBastionOutpostLooks);
		m_Looks.Add(l.m_sRedHandOutpostPrefab, l.m_aRedHandOutpostLooks);
		m_Looks.Add(l.m_sHalcyonOutpostPrefab, l.m_aHalcyonOutpostLooks);
		m_Looks.Add(l.m_sHavenLeaderPrefab, l.m_aHavenLeaderLooks);
		m_Looks.Add(l.m_sBastionLeaderPrefab, l.m_aBastionLeaderLooks);
		m_Looks.Add(l.m_sRedHandLeaderPrefab, l.m_aRedHandLeaderLooks);
		m_Looks.Add(l.m_sHalcyonLeaderPrefab, l.m_aHalcyonLeaderLooks);
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
	//! The lab entity (placed or spawned), null without one.
	IEntity GetLabEntity() { return m_Lab; }

	//------------------------------------------------------------------------------------------------
	//! The cure's lab moved to a hand-built site's lab spot (docs/SITES.md, "Role spots"): only a site
	//! in the lab's own town (within 400 m of it), and only a lab this mode spawned (one placed in the
	//! world by the map stays). Deliveries go to it where it now stands.
	void MoveLab(vector pos, float yaw)
	{
		if (!m_bHasLab || m_sLabPrefab.IsEmpty())
			return;

		if (m_Lab && m_Lab.GetPrefabData() && m_Lab.GetPrefabData().GetPrefabName() != m_sLabPrefab)
			return;

		float d = RZ_Util.DistXZ(m_vLabPos, pos);
		if (d < 0.5 || d > 400)
			return;

		if (m_Lab)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Lab);

		m_vLabPos = RZ_Util.GroundPos(pos);
		m_Lab = RZ_Util.SpawnPrefab(m_sLabPrefab, m_vLabPos, yaw);
		RZ_Log.Info(string.Format("Lab: moved %1 m to its hand-built spot at %2.", RZ_Util.RoundToInt(d), RZ_SettlementScanner.GridReference(m_vLabPos)));
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
	float GetReplicatedCureBacklog() { return m_fReplCureBacklog; }
	bool IsReplicatedLabFull() { return m_bReplLabFull; }
	bool IsReplicatedAtGate() { return m_bReplAtGate; }

	//------------------------------------------------------------------------------------------------
	//! The lab's line ("Research 63 percent. Waiting: 14 samples' worth, about 4 days of work.") from
	//! the replicated values, on any machine. The settings it reads are the same everywhere; the pace
	//! by play is measured on the server and replicated.
	string DescribeLabReplicated()
	{
		float pace = 1;
		if (m_fReplPlayPace > 0)
			pace = m_fReplPlayPace;

		return RZ_CureResearch.DescribeLab(m_fReplCureProgress, m_fReplCureBacklog, m_fResearchPerSample * GetCurePace() * pace * GetSeasonSpeed(),
			GetLabPerDay() * GetSeasonSpeed(), m_bReplLabFull, m_bReplAtGate, m_bReplCured);
	}
	RZ_EOutbreakStage GetReplicatedStage() { return m_iReplStage; }
	bool IsReplicatedCured() { return m_bReplCured; }
	string GetLockedBuildings() { return m_sReplLockedBuildings; }
	string GetDoorMarks() { return m_sReplDoorMarks; }
	string GetTraceLights() { return m_sReplTraceLights; }
	string GetPowerLamps() { return m_sReplPowerLamps; }

	//------------------------------------------------------------------------------------------------
	//! Server: where the map's own lamps may light (RZ_PowerGrid.PushLamps), for the clients'
	//! street lamps (UI/RZ_PowerLampsClient.c).
	void SetPowerLamps(string list)
	{
		if (list == m_sReplPowerLamps)
			return;

		m_sReplPowerLamps = list;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Server: the lit windows near players, for the clients' lights (RZ_TraceAmbience, S2).
	void SetTraceLights(string list)
	{
		if (list == m_sReplTraceLights)
			return;

		m_sReplTraceLights = list;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Server: the marked doors near players, for the clients' decals (RZ_TraceDraw).
	void SetDoorMarks(string list)
	{
		if (list == m_sReplDoorMarks)
			return;

		m_sReplDoorMarks = list;
		Replication.BumpMe();
	}

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
	//! Asked for many times a tick (every standing check goes through GetProfile), so a backend
	//! identity is remembered per player id once known. The name fallback isn't cached.
	//! Empty for an id nobody answers to (a player who left before we ever saw them).
	protected string GetIdentity(int playerId)
	{
		if (playerId <= 0)
			return "";

		string ident;
		if (m_mIdentities.Find(playerId, ident))
			return ident;

		UUID uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (!uid.IsNull())
		{
			ident = uid;
			m_mIdentities.Set(playerId, ident);
			return ident;
		}

		string name = GetGame().GetPlayerManager().GetPlayerName(playerId);
		if (name.IsEmpty())
			return "";

		return "local_" + name;
	}

	//------------------------------------------------------------------------------------------------
	//! Never null. For an id with no identity (gone before we knew them) it's a blank profile
	//! that's never stored or saved, so late callbacks (a wound described after a disconnect, a
	//! reward to someone who left) can't create junk "local_" profiles.
	RZ_PlayerProfile GetProfile(int playerId)
	{
		string ident = GetIdentity(playerId);
		RZ_PlayerProfile profile;
		if (ident.IsEmpty())
		{
			profile = new RZ_PlayerProfile();
			return profile;
		}

		if (!m_mProfiles.Find(ident, profile))
		{
			profile = new RZ_PlayerProfile();
			profile.m_sIdentity = ident;
			m_mProfiles.Insert(ident, profile);
		}

		return profile;
	}

	//------------------------------------------------------------------------------------------------
	//! The stored profile, or null: never creates one. For code that may run for a player who
	//! has left.
	RZ_PlayerProfile FindProfile(int playerId)
	{
		string ident = GetIdentity(playerId);
		if (ident.IsEmpty())
			return null;

		RZ_PlayerProfile profile;
		if (m_mProfiles.Find(ident, profile))
			return profile;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The player id online now with this identity, or -1.
	int FindPlayerByIdentity(string ident)
	{
		if (ident.IsEmpty())
			return -1;

		foreach (int id, string known : m_mIdentities)
		{
			if (known == ident && !GetGame().GetPlayerManager().GetPlayerName(id).IsEmpty())
				return id;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The stored profile for an identity, or null: never creates one. Works for players who are
	//! offline (profiles are kept by identity).
	//! Every saved profile (the epilogue's record).
	void GetAllProfiles(notnull array<RZ_PlayerProfile> result)
	{
		foreach (string ident, RZ_PlayerProfile p : m_mProfiles)
		{
			if (p)
				result.Insert(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_PlayerProfile FindProfileByIdentity(string ident)
	{
		if (ident.IsEmpty())
			return null;

		RZ_PlayerProfile profile;
		if (m_mProfiles.Find(ident, profile))
			return profile;

		return null;
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
	//! The loadout the server assigned the player (their pick in the spawn screen), null if none.
	SCR_BasePlayerLoadout GetPickedLoadout(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return null;

		SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		if (!loadoutComp)
			return null;

		return loadoutComp.GetLoadout();
	}

	//------------------------------------------------------------------------------------------------
	//! The loadout the player picked in the spawn screen, "" if unknown.
	string GetPickedLoadoutName(int playerId)
	{
		SCR_BasePlayerLoadout loadout = GetPickedLoadout(playerId);
		if (!loadout)
			return "";

		return loadout.GetLoadoutName();
	}

	//------------------------------------------------------------------------------------------------
	//! The background this life plays as: the picked one, or the default while it's locked (as the
	//! spawn decides). For the openings' place roll, before the spawn is finished.
	RZ_Background LifeBackground(int playerId)
	{
		RZ_Background bg = ResolveBackground(playerId);
		if (bg.m_bLocked && !GetProfile(playerId).HasUnlocked(bg.m_sUnlockId))
			return GetDefaultBackground();

		return bg;
	}

	//------------------------------------------------------------------------------------------------
	//! The background is whichever loadout the player picked in the spawn screen.
	protected RZ_Background ResolveBackground(int playerId)
	{
		string loadoutName = GetPickedLoadoutName(playerId);
		if (loadoutName.IsEmpty())
			return GetDefaultBackground();

		// A last safety net: the server already refuses a locked loadout and a spawn with its
		// prefab (UI/RZ_LoadoutLock.c), so this only catches a spawn that went another way
		if (RZ_UISettings.Get().m_bLockLoadouts && IsLoadoutLocked(playerId, loadoutName))
		{
			RZ_Log.Warn("Player " + playerId + " spawned with locked loadout '" + loadoutName + "': default background");
			return GetDefaultBackground();
		}

		foreach (RZ_Background bg : m_aBackgrounds)
		{
			if (bg.m_sLoadoutName == loadoutName)
				return bg;
		}

		RZ_Log.Warn("No background matches loadout '" + loadoutName + "'");
		return GetDefaultBackground();
	}

	//------------------------------------------------------------------------------------------------
	//! Server: a background loadout this player hasn't unlocked (the server won't assign it or
	//! spawn its prefab, UI/RZ_LoadoutLock.c, and ResolveBackground falls back to the default).
	//! False for loadouts that aren't backgrounds.
	bool IsLoadoutLocked(int playerId, string loadoutName)
	{
		if (!m_aBackgrounds || playerId <= 0)
			return false;

		foreach (RZ_Background bg : m_aBackgrounds)
		{
			if (bg.m_sLoadoutName == loadoutName)
				return bg.m_bLocked && !GetProfile(playerId).HasUnlocked(bg.m_sUnlockId);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! From the game mode's spawn invoker. Not named OnPlayerSpawned: SCR_BaseGameModeComponent has a
	//! method of that name, and overriding it as well would run all of this twice per spawn.
	protected void HandlePlayerSpawned(int playerId, IEntity controlledEntity)
	{
		if (!controlledEntity)
			return;

		RZ_Background bg = ResolveBackground(playerId);
		RZ_PlayerProfile profile = GetProfile(playerId);
		OnLegacyChanged(playerId); // older profiles may already have 10 Legacy

		// Locked backgrounds need an unlock. The loadout's gear stays (we can't take it back
		// cleanly), but the perks, flaws and standing are the default background's. Said on the
		// spawn card's second line; the why goes into the notebook.
		string lockedLine = "";
		if (bg.m_bLocked && !profile.HasUnlocked(bg.m_sUnlockId))
		{
			string why = bg.m_sUnlockText;
			if (why.IsEmpty())
				why = "You haven't unlocked it yet.";

			RZ_Background fallback = GetDefaultBackground();
			lockedLine = string.Format("%1 is locked. You're %2 this life.", bg.m_sName, fallback.m_sName);
			NotifyPlayerAs(playerId, bg.m_sName + " is locked", string.Format("%1\nYou are playing as %2 this life.", why, fallback.m_sName), RZ_ENoticeTag.QUIET);
			bg = fallback;
		}

		// The old character's waiting callbacks become board notes before the new life starts
		if (m_Shared)
			m_Shared.OnNewLife(playerId, profile);

		profile.StartNewLife(bg);
		// The last character's jobs aren't this one's (a fresh Quests tab: only the opening's step)
		if (m_Missions)
			m_Missions.OnNewLife(playerId);

		// Experience and refresher first: the first life that counts gets "Find the Haven"
		m_Guidance.BeginLife(playerId, profile);
		// The opening at the place rolled for this spawn: the scene, someone from before, the chain
		// (Guidance/RZ_Openings.c). Its whole chain replaces "Find the Haven"
		m_Guidance.GetOpenings().OnSpawned(playerId, controlledEntity, profile, bg);
		RZ_Objectives.Assign(this, profile, bg, controlledEntity.GetOrigin());

		m_BiteDetector.Reset(playerId);
		m_Stealth.Forget(playerId);
		m_Infection.OnPlayerSpawned(playerId);
		m_UIFeed.OnPlayerSpawned(playerId);
		m_UIFeed.RecordGearSoon(playerId, GetPickedLoadoutName(playerId));
		RegisterGunfire(controlledEntity);
		// Keys, timed actions and cracked channels belong to the last life
		m_Interactions.OnPlayerSpawned(playerId);

		// Where you wake: a small clue nearby, and the wind-up radio (new-player guidance). The
		// scene's full text goes into the notebook; its one line is the spawn card's.
		m_Guidance.OnPlayerSpawned(playerId, controlledEntity);

		// 1. The one card at spawn (docs/UI.md, "Notices"): who you are and where, in a line or
		// two. Everything else at spawn goes quietly into the notebook.
		string whereLine = m_Guidance.TakeSpawnLine(playerId);
		if (whereLine.IsEmpty())
			whereLine = RZ_QuestText.SpawnWhere(this, controlledEntity.GetOrigin());

		if (!lockedLine.IsEmpty())
			whereLine += "\n" + lockedLine;

		NotifyPlayerAs(playerId, bg.m_sName, whereLine, RZ_ENoticeTag.SPAWN);
		string intro = string.Format("Perk: %1\nFlaw: %2\n\nYour goal: %3",
			bg.m_sPerk, bg.m_sFlaw, RZ_Objectives.Describe(this, profile));
		NotifyPlayerAs(playerId, bg.m_sName, intro, RZ_ENoticeTag.QUIET);

		// 2. The state of the world (first spawn of this session only)
		if (!m_aWelcomed.Contains(playerId))
		{
			m_aWelcomed.Insert(playerId);
			string lab = "";
			if (m_bHasLab && m_Cure.IsCured())
			{
				lab = string.Format("\nHalcyon's lab: %1. The cure is out; it takes no more samples.",
					RZ_Util.DirectionText(controlledEntity.GetOrigin(), m_vLabPos));
			}
			else if (m_bHasLab)
			{
				lab = string.Format("\nHalcyon's lab: %1. Bring it samples from special infected.",
					RZ_Util.DirectionText(controlledEntity.GetOrigin(), m_vLabPos));
			}

			// "Research 63 percent. Waiting: 14 samples' worth, about 4 days of work."
			string research = m_Cure.Describe();
			if (m_Cure.IsCured())
				research = "The cure is out. The gates open when the season ends.";

			NotifyPlayerAs(playerId, "Outbreak: " + RZ_StageSettings.GetStageName(m_Director.GetStage()), research + lab, RZ_ENoticeTag.QUIET);

			// After the cure the broadcast reads the new order (RZ_Broadcast.GetText)
			NotifyPlayerAs(playerId, RZ_Broadcast.TITLE, m_Broadcast.GetText(), RZ_ENoticeTag.QUIET);

			// A final stand that was running (or restarted with the server) needs defenders
			if (m_Cure.IsFinalStand())
				NotifyPlayerAs(playerId, "THE FINAL STAND", string.Format("Halcyon is synthesizing the cure and the lab is under siege. About %1 minutes left to hold it.", m_Cure.GetFinalStandMinutesLeft()), RZ_ENoticeTag.URGENT);

			// The season in one notebook entry, no card: its number and rules (the last winner's,
			// the rule sets, a coming wipe), the session's news, the hall of fame so far
			string season = m_Seasons.DescribeForWelcome();
			string news = m_Headlines.Build();
			news = news.Trim();
			if (!news.IsEmpty())
				season += "\n\nWord on the roads: " + news;

			if (GetHall())
			{
				string hall = GetHall().BuildText();
				if (!hall.IsEmpty())
					season += "\n\n" + hall;
			}

			NotifyPlayerAs(playerId, "This season", season, RZ_ENoticeTag.QUIET);
		}

		// 3. What's on the radio, in the notebook (the Quests tab lists the jobs themselves)
		NotifyPlayerAs(playerId, "Radio", m_Missions.DescribeActiveFor(controlledEntity, playerId), RZ_ENoticeTag.QUIET);

		// The epilogue week: the cure line and your own first line, its news, your last goal
		m_Epilogue.OnSpawn(playerId, controlledEntity);

		m_Perks.OnPlayerSpawned(playerId, controlledEntity);
		m_Services.OnPlayerSpawned(playerId, controlledEntity);
		if (m_Tribes)
			m_Tribes.GetPeople().OnPlayerSpawned(playerId);
		m_Checkpoints.OnPlayerSpawned(playerId);

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

			NotifyPlayerAs(playerId, "World event: " + mission.m_sTitle, string.Format("%1\n(%2)", mission.GetBrief(this), where), RZ_ENoticeTag.EVENT);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Map marker for a world event or the lab. Returns a handle for RemoveMapMarker, -1 if off.
	//! With the icon layer set up (UI > Map icon layout) the icons show these instead, per player.
	int AddMapMarker(vector pos, string text)
	{
		if (!m_bMapMarkers || RZ_UISettings.Get().UsesMapIcons())
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
			ev.RegisterScriptHandler("OnProjectileShot", this, OnPlayerShot);
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
		bool suppressed = RZ_Util.IsSuppressed(weapon);
		if (suppressed)
			heat = heat * m_fSuppressedHeatMult;

		m_Heat.AddHeat(shooter.GetOrigin(), heat);

		// A shot gives you away at once (docs/DESIGN.md, Stealth), and closes a talk or a note
		m_Stealth.OnPlayerShot(playerID, suppressed);
		m_Behaviour.OnShot(playerID, shooter.GetOrigin(), suppressed);
		if (!suppressed)
			m_Tribes.OnShot(shooter.GetOrigin());
		m_ScreenGuard.OnShot(playerID);
		// A deer this player reports in the next moments is theirs (docs/DESIGN.md, Wildlife)
		m_Wildlife.OnShot(playerID);

		// House rules and wanderers who were shot at
		m_Safehouses.OnPlayerShot(playerID, shooter.GetOrigin());
		m_Wanderers.OnPlayerShot(playerID, shooter.GetOrigin());
		m_Leaders.OnPlayerShot(playerID, shooter.GetOrigin());
		m_Missions.OnPlayerShot(playerID, shooter.GetOrigin());
		if (m_Errands)
			m_Errands.OnPlayerShot(playerID, shooter.GetOrigin());
		if (m_Supply)
			m_Supply.OnPlayerShot(playerID, shooter.GetOrigin());

		// Safe zones: firing inside a stronghold or the lab
		m_Checkpoints.OnPlayerShot(playerID, shooter.GetOrigin());
		// The epilogue's scenes: the queue and the lab gate are safe ground
		m_Epilogue.OnPlayerShot(playerID, shooter.GetOrigin());

		// Retaliation: who fired, and when (a hit on a faction's AI is matched to it)
		m_Hostility.OnPlayerShot(playerID, shooter.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	//! Every tenth of a second: camp life's spawning and the read of a site a player came near
	//! (docs/SITES.md, How fast a place fills).
	protected void FastTick()
	{
		if (m_CampLife)
			m_CampLife.FastTick();
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second: bites, stealth (which updates at its own pace, Stealth > Update seconds) and the
	//! land grid's first reading.
	protected void SecondTick()
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		m_BiteDetector.Check(ids, players);
		m_Stealth.Tick(1, ids, players);
		// The tribes' scouts, and the behaviour layer (senses, plans, bodies) once a second
		m_Tribes.SecondTick(ids, players);
		m_Behaviour.Tick(ids, players);
		// Traces: a town's doors on entry, a door read after 2 s standing at it
		m_Traces.SecondTick(ids, players);

		// The land grid's first reading, a few squares a second (nothing once it's read)
		m_Land.Step();
		// Camp life's site grids: the site being read reads on (docs/SITES.md)
		m_CampLife.SecondTick();

		// Open talk screens and readers close when danger comes
		m_ScreenGuard.Tick();

		// Work sites: who is at which, their props near players
		if (m_Shared)
			m_Shared.SecondTick(ids, players);

		// A duty's table opens when its work site has been held
		if (m_Errands)
			m_Errands.SecondTick(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! Once, on the first server tick: the engine's day length from Day length (phase 0: the default
	//! is real time, x1; SetDayDuration works, capped at x12), and the time of day from state.json
	//! (the clock otherwise goes back to the scenario's start on a restart).
	protected void ApplyClock()
	{
		m_bClockSet = true;
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity tm = world.GetTimeAndWeatherManager();
		if (!tm)
		{
			RZ_Log.Info("Clock: no time and weather manager in the world; every hour reads as day.");
			return;
		}

		if (m_fDayLengthHours > 0)
		{
			float seconds = Math.Clamp(m_fDayLengthHours, 2, 24) * 3600;
			tm.SetDayDuration(seconds);
		}

		if (m_bKeepTimeOfDay && m_fSavedHour >= 0)
			tm.SetTimeOfTheDay(m_fSavedHour, true);

		RZ_Log.Info(string.Format("Clock: a game day takes %1 real minutes; the time of day is %2 (saved %3).",
			RZ_Util.RoundToInt(tm.GetDayDuration() / 60), RZ_GameTime.Clock(tm.GetTimeOfTheDay()), RZ_GameTime.Clock(m_fSavedHour)));
	}

	//------------------------------------------------------------------------------------------------
	//! "clock 21:40, day 240 min"
	protected string ClockText()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world || !world.GetTimeAndWeatherManager())
			return "no clock";

		TimeAndWeatherManagerEntity tm = world.GetTimeAndWeatherManager();
		return string.Format("clock %1, day %2 min", RZ_GameTime.Clock(tm.GetTimeOfTheDay()), RZ_Util.RoundToInt(tm.GetDayDuration() / 60));
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
			m_Spawner.GetTrackedCount(), m_Missions.GetActive().Count()) + " | move waypoints " + RZ_GroupOrders.CountWaypoints().ToString());
		// The infected (docs/INFECTED.md, Admin visibility): the budget, crawlers, the land, the engine's AI
		if (m_Spawner.IsPlumbingOn())
		{
			RZ_Log.Info("Status | " + m_Spawner.StatusText() + " | " + RZ_Tribes.ActiveAIText() + " | " + m_Tribes.LandText() + " | " + ClockText());
			m_Tribes.StatusLines();
			if (RZ_Behaviour.IsOn())
				RZ_Log.Info("Status | " + m_Behaviour.StatusText());
		}
		if (m_Traces.IsOn())
			RZ_Log.Info("Status | " + m_Traces.StatusText());

		RZ_Log.Info(string.Format("Status | players %1 | cure %2 percent (milestone %3, final stand %4, waiting %5, lab full %6, at the 90 gate %7, pace by play %8 percent) | %9",
			playerCount, RZ_Util.RoundToInt(m_Cure.m_fProgress), m_Cure.m_iMilestone, m_Cure.IsFinalStand(),
			RZ_Util.RoundToInt(m_Cure.GetBacklog()), m_Cure.IsLabFull(), m_Cure.IsAtFinaleGate(),
			RZ_Util.RoundToInt(m_Cure.PlayPace() * 100), m_Environment.Describe()));
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
	//! Infamy (RZ_Reputation.GetRivalShare), as rival shares (RZ_RepFlag.SHARE). Negative amounts
	//! lower Fame. flags: RZ_RepFlag.SMALL (errands) is left out of season Fame and the hall of
	//! fame. deed: a short past-tense phrase for the life's best deed (RZ_Deeds), "" for none.
	//! The player is told when their tier changes, and (debounced) when their name does.
	void AddFame(int playerId, RZ_EFaction faction, int amount, bool applyRivals = true, int flags = 0, string deed = "")
	{
		if (amount == 0 || playerId <= 0 || !RZ_Factions.IsValid(faction))
			return;

		RZ_PlayerProfile profile = GetProfile(playerId);
		RZ_ETier before = profile.GetTier(faction);
		int gained = profile.AddFame(faction, amount);
		bool smallPay = (flags & RZ_RepFlag.SMALL) != 0;
		bool switched = (flags & RZ_RepFlag.SWITCH) != 0;
		if (!smallPay)
		{
			// A switched side's Fame never counts toward the season's (who benefits)
			if (!switched)
				OnFameGained(faction, gained);

			if (gained > 0 && GetHall())
				GetHall().NoteFame(playerId, faction, profile.GetFame(faction));

			if (gained > 0)
				RZ_Deeds.NoteBest(profile, faction, gained, deed);
		}

		CheckTierChange(playerId, faction, before);

		if (!applyRivals || amount <= 0)
			return;

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			int infamy = RZ_Util.RoundToInt(amount * RZ_Reputation.GetRivalShare(faction, f) * RivalMult(faction, f));
			if (infamy > 0)
				AddInfamy(playerId, f, infamy, "rival share", RZ_RepFlag.SHARE);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Infamy with `faction` (docs/REPUTATION.md, "Standing tiers"). Gains only: Infamy comes off
	//! through Forgive(), the one forgiveness budget (a negative amount is passed on to it).
	//! why: a few words for the log. flags (RZ_RepFlag):
	//! - SHARE: a share, not something the faction saw you do (the rival share of Fame, a mission's
	//!   rival Infamy, a questline's standing). Never doubled by a uniform, never counts as Feared.
	//! - ACT: done in sight: while you're passing in that faction's kit it doubles (RZ_Disguise).
	//!   Direct gains without SHARE are treated as done in sight, as before.
	//! deed: a short past-tense phrase for the life's worst deed (RZ_Deeds), "" for none.
	//! The credit rule: while Fame with the faction is 75 or more (Reputation > Credit from), the
	//! same amount also comes off that Fame, after the disguise and the stronghold leader's temper.
	void AddInfamy(int playerId, RZ_EFaction faction, int amount, string why = "", int flags = 0, string deed = "")
	{
		if (amount == 0 || playerId <= 0 || !RZ_Factions.IsValid(faction))
			return;

		if (amount < 0)
		{
			Forgive(playerId, faction, -amount, why);
			return;
		}

		bool share = (flags & RZ_RepFlag.SHARE) != 0;
		// Caught in a faction's uniform: Infamy with it doubles (RZ_Disguise). Never a share
		if (!share && m_Hostility)
			amount = m_Hostility.ScaleInfamyForDisguise(playerId, faction, amount);

		// The faction's stronghold leader sets its temper: Paranoid harsher, Merciful a little softer
		if (m_Leaders)
			amount = m_Leaders.ScaleInfamy(faction, amount);

		RZ_PlayerProfile profile = GetProfile(playerId);
		RZ_ETier before = profile.GetTier(faction);
		int added = profile.AddInfamy(faction, amount, share);

		// The credit runs out: new Infamy at 75 Fame or more costs the same Fame
		RZ_ReputationSettings rs = RZ_ReputationSettings.Get();
		if (rs.m_bCreditRule && added > 0 && profile.GetFame(faction) >= rs.m_iCreditFrom)
		{
			profile.AddFame(faction, -added);
			RZ_Log.Info(string.Format("Reputation: %1 %2 credit rule, -%3 Fame for the Infamy (%4)", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(faction), added, why));
		}

		if (added > 0 && !share)
			RZ_Deeds.NoteWorst(profile, faction, added, deed);

		CheckTierChange(playerId, faction, before);
	}

	//------------------------------------------------------------------------------------------------
	//! The one forgiveness budget (docs/REPUTATION.md, "Working off Infamy"): takes up to `amount`
	//! Infamy off with `f`, after the stronghold leader's temper (a Merciful leader makes each
	//! amount count for more, never the total), within what's left of the budget this life
	//! (Reputation > Forgiveness budget, 30) and the Infamy there is. Every route that works Infamy
	//! off calls it: amends contracts, paying the quartermaster, surrender. Returns what came off,
	//! 0 when the budget is spent. tempered: the caller already applied the temper (paying, which
	//! clamps the tempered amount to its own floor). Logged.
	int Forgive(int playerId, RZ_EFaction f, int amount, string reason, bool tempered = false)
	{
		if (amount <= 0 || playerId <= 0 || !RZ_Factions.IsValid(f))
			return 0;

		RZ_PlayerProfile profile = GetProfile(playerId);
		int want = amount;
		if (m_Leaders && !tempered)
			want = -m_Leaders.ScaleInfamy(f, -amount);

		want = Math.Min(want, Math.Min(profile.ForgiveLeft(f), profile.GetInfamy(f)));
		if (want <= 0)
		{
			RZ_Log.Info(string.Format("Reputation: %1 %2 forgive %3 refused, %4 left to forgive (%5)", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), amount, profile.ForgiveLeft(f), reason));
			return 0;
		}

		RZ_ETier before = profile.GetTier(f);
		int taken = -profile.AddInfamy(f, -want);
		profile.EnsureArrays();
		profile.m_aForgiven[f] = profile.m_aForgiven[f] + taken;
		if (m_Shared)
			m_Shared.GetSeasonDeeds().NoteForgiven(profile.m_sIdentity, f, taken);

		RZ_Log.Info(string.Format("Reputation: %1 %2 forgave %3 (%4), %5 left to forgive", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), taken, reason, profile.ForgiveLeft(f)));
		CheckTierChange(playerId, f, before);
		return taken;
	}

	//------------------------------------------------------------------------------------------------
	//! The budget is spent: "The Haven has forgiven you all it will this life."
	string ForgiveSpentText(RZ_EFaction f)
	{
		return RZ_Util.Capitalize(RZ_Factions.GetNameInText(f)) + " has forgiven you all it will this life.";
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

		// A questline stage this mission was
		if (m_Questlines)
			m_Questlines.OnMissionFinished(m, success);
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
	//! After every standing change: the tier notice at once (leading with the name when it isn't
	//! the tier word), and the name's own notice, debounced (RZ_RepWatch).
	protected void CheckTierChange(int playerId, RZ_EFaction faction, RZ_ETier before)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		RZ_ETier after = profile.GetTier(faction);
		if (m_RepWatch)
			m_RepWatch.OnStanding(playerId, faction, after != before);

		if (after == before)
			return;

		int fame = profile.GetFame(faction);
		int infamy = profile.GetInfamy(faction);
		string text = string.Format("Your standing is now %1 (was %2).", RZ_Reputation.GetTierName(after), RZ_Reputation.GetTierName(before));
		string shown = RZ_RepNames.Shown(fame, infamy);
		if (shown != RZ_Reputation.GetTierName(after))
		{
			text = string.Format("They call you %1 now. Your standing is %2 (was %3).", RZ_RepNames.NameWithRule(fame, infamy),
				RZ_Reputation.GetTierName(after), RZ_Reputation.GetTierName(before));
		}

		// A Champion who drops because new Infamy ate into the Fame: say why
		if (before == RZ_ETier.CHAMPION && after == RZ_ETier.TRUSTED && RZ_ReputationSettings.Get().m_bCreditRule && infamy > 0)
			text += " Your credit ran out: at 75 Fame, every new wrong costs Fame too.";

		NotifyPlayerAs(playerId, RZ_Factions.GetName(faction), text, RZ_ENoticeTag.HIGH);

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

		// Reputation clues: the armband or the ribbon at Trusted
		if (m_Services)
			m_Services.OnTierChanged(playerId, faction, after);

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
	//! A capped bonus into a faction's season Fame (never into a player's standing): the finale's
	//! route chunk (docs/FINALE.md, "Fame"). Saved with the season Fame and logged.
	void AddFactionBonus(int faction, int amount, string why)
	{
		if (!RZ_Factions.IsValid(faction) || amount <= 0)
			return;

		while (m_aSeasonFame.Count() < RZ_Factions.COUNT)
		{
			m_aSeasonFame.Insert(0);
		}

		m_aSeasonFame[faction] = m_aSeasonFame[faction] + amount;
		RZ_Log.Info(string.Format("Season Fame: %1 +%2 (%3), now %4.", RZ_Factions.GetName(faction), amount, why, m_aSeasonFame[faction]));
	}

	//------------------------------------------------------------------------------------------------
	//! Called by RZ_CureResearch when the final stand is won. With the cure finale (docs/FINALE.md):
	//! whoever holds the clean room controls the cure, and the faction whose rules apply in full is
	//! the controller's (the Independent's only by its "one side" terms, chosen at the release); when
	//! nobody holds the room it's handed over to the faction with the most season Fame. Without the
	//! finale: the most season Fame, as before. The result is saved so the next season can read it.
	void OnCureCompleted()
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		bool contest = finale && RZ_CureFinale.IsOn() && finale.Controller() != RZ_CureFinale.NONE;
		if (contest)
			m_iSeasonWinner = finale.ControllerFaction();
		else
			m_iSeasonWinner = GetSeasonLeader();

		if (m_iSeasonWinner < 0 && !contest)
			m_iSeasonWinner = RZ_EFaction.INSTITUTE;

		string title = "The cure goes to " + RZ_Factions.GetName(m_iSeasonWinner);
		string ending = GetEndingText(m_iSeasonWinner);
		if (contest)
		{
			title = "The cure: " + RZ_CureFinale.RouteName(finale.Controller());
			ending = FinaleEndingText(finale);
		}

		NotifyAll(title, ending);
		RZ_Log.Info(string.Format("Season over. Cure goes to %1. Season Fame: Haven %2, Bastion %3, Red Hand %4, Halcyon %5.",
			RZ_Factions.GetName(m_iSeasonWinner), GetSeasonFame(0), GetSeasonFame(1), GetSeasonFame(2), GetSeasonFame(3)));

		RZ_SeasonRecord record = new RZ_SeasonRecord();
		record.world = GetWorldId();
		record.season = m_Seasons.GetSeason();
		record.winner = m_iSeasonWinner;
		record.winnerName = RZ_Factions.GetName(m_iSeasonWinner);
		if (m_iSeasonWinner < 0)
			record.winnerName = "nobody";

		foreach (int fame : m_aSeasonFame)
		{
			record.fame.Insert(fame);
		}

		FillFinaleRecord(record);

		// The majors' endings (the season story) go with it
		if (RZ_Majors.Get())
		{
			RZ_Majors.Get().FillRecord(record);
			string story = RZ_Majors.Get().EpilogueLine(m_iSeasonWinner, true);
			if (!story.IsEmpty())
				RZ_Questline.Headline(this, story, false);
		}

		RZ_Persistence.SaveSeason(record);
		if (GetHall())
			GetHall().SetSeasonLine(SeasonLine());

		// The epilogue starts: the wipe cycle counts from now (RZ_Seasons)
		m_Seasons.OnCureCompleted();
		// The epilogue week: the record is taken over the next ticks, then the story commits
		m_Epilogue.OnCure();
		SaveState();
	}

	//------------------------------------------------------------------------------------------------
	//! The finale's part of the season record: the route, how, the terms, the finisher, the
	//! deciders, the half rule set for next season.
	protected void FillFinaleRecord(RZ_SeasonRecord record)
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (!finale || !RZ_CureFinale.IsOn())
			return;

		record.route = RZ_CureFinale.RouteFlag(finale.Controller());
		record.finisher = RZ_CureFinale.RouteFlag(finale.Finisher());
		record.how = finale.How();
		record.terms = finale.Terms();
		record.partner = RZ_CureFinale.RouteFlag(finale.Partner());
		record.deciders.Clear();
		foreach (string ident : finale.GetDeciders())
		{
			RZ_PlayerProfile p = FindProfileByIdentity(ident);
			string name = "someone";
			int pid = m_Questlines.OnlineId(ident);
			if (pid > 0)
				name = RZ_Names.Public(pid);

			if (p || pid > 0)
				record.deciders.Insert(name);
		}

		// At most one half rule set on top: the deal partner, else the Fame leader when it isn't the
		// controller (Finale > Fame leader rule)
		record.halfRules = -1;
		int leader = GetSeasonLeader();
		if (finale.Partner() >= 0 && finale.Partner() < RZ_CureFinale.INDEP && finale.How() == RZ_CureFinale.HOW_DEAL)
			record.halfRules = finale.Partner();
		else if (finale.GetSettings().m_bFameLeaderRule && leader >= 0 && leader != m_iSeasonWinner && finale.Controller() != RZ_CureFinale.NONE)
			record.halfRules = leader;
	}

	//------------------------------------------------------------------------------------------------
	//! The terms came in at the release (RZ_CureFinale.OnTerms): the winner (the Independent's one
	//! side), the record and the hall's line follow them.
	void OnFinaleReleased()
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (!finale)
			return;

		m_iSeasonWinner = finale.ControllerFaction();
		RZ_SeasonRecord record = RZ_Persistence.LoadSeason();
		if (record && record.season == m_Seasons.GetSeason())
		{
			record.winner = m_iSeasonWinner;
			record.winnerName = RZ_Factions.GetName(m_iSeasonWinner);
			if (m_iSeasonWinner < 0)
				record.winnerName = "nobody";

			FillFinaleRecord(record);
			RZ_Persistence.SaveSeason(record);
		}

		string line = FinaleEndingText(finale);
		if (!line.IsEmpty())
			NotifyAll("The cure", line);

		if (GetHall())
			GetHall().SetSeasonLine(SeasonLine());

		SaveState();
	}

	//------------------------------------------------------------------------------------------------
	//! "The cure: the Haven, by force. The Zone's Fame: Bastion, 1640." (the hall of fame).
	string SeasonLine()
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && RZ_CureFinale.IsOn())
			return finale.SeasonLine(m_iSeasonWinner);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The cure's line under the finale: by route and terms (docs/FINALE.md, "Endings").
	static string FinaleEndingText(RZ_CureFinale finale)
	{
		int c = finale.Controller();
		int t = finale.Terms();
		switch (c)
		{
			case RZ_CureFinale.HAVEN:
			{
				if (t == 2)
					return "The Haven holds the cure. Somebody finally drove out to the far house.";
				if (t == 3)
					return "The Haven holds the cure. The houses read the bite book, not the List.";
				if (t == 4)
					return "The Haven holds the cure. They went down the List from the bottom.";
				return "The Haven holds the cure. It goes hand to hand, and the first box went to his house.";
			}
			case RZ_CureFinale.BASTION:
			{
				if (t == 2)
					return "Bastion holds the cure. The column's slips go first, by the numbers on them.";
				if (t == 3)
					return "Bastion holds the cure. \"The sick first. Whoever.\"";
				if (t == 4)
					return "Bastion holds the cure. Names are taken at the table, for everyone.";
				return "Bastion holds the cure. Soldiers first, then civilians attached.";
			}
			case RZ_CureFinale.REDHAND:
			{
				if (t == 2)
					return "The Red Hand holds the cure. They read the book from the back. The last name was on the first line. Paid.";
				if (t == 3)
					return "The Red Hand holds the cure. The first pan went to a Haven house for nothing.";
				if (t == 4)
					return "The Red Hand holds the cure. Every dose went on somebody's line. The book has never been so full.";
				return "The Red Hand holds the cure. Two tins a dose at the crossroads. Lab stickers extra.";
			}
			case RZ_CureFinale.HALCYON:
			{
				if (t == 2)
					return "Halcyon holds the cure. The volunteers first, by name.";
				if (t == 3)
					return "Halcyon holds the cure. The newly bitten first: they're likeliest to walk out.";
				if (t == 4)
					return "Halcyon holds the cure. THE METHOD. READ IT. COPY IT. PASS IT ON.";
				return "Halcyon holds the cure. The authors first.";
			}
			case RZ_CureFinale.INDEP:
			{
				if (t == 2)
					return "Nobody's batch went to one side, with a condition chalked on the crate.";
				if (t == 3)
					return "Nobody's batch is sold from a crate at the queue. The payment goes nowhere.";
				if (t == 4)
					return "Nobody's batch is behind a padlock that isn't the lab's. BACK SOON.";
				return "Nobody's batch: ONE EACH. Free, at the queue, first come first served.";
			}
		}

		return "Nobody held the clean room. The lab handed the cure to the faction the Zone had backed all season.";
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
		NotifyAll("Champion", string.Format("%1 is now a Champion of %2.", name, RZ_Factions.GetNameInText(faction)));
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
				NotifyPlayerAs(playerId, "Background unlocked", string.Format("%1 is now available for your next life. Pick the '%2' loadout.", bg.m_sName, bg.m_sLoadoutName), RZ_ENoticeTag.HIGH);
				return;
			}
		}
	}

	// ================================================================ Infection
	//------------------------------------------------------------------------------------------------
	//! Public API: call this from a zombie mod's bite event. The bite opens the golden window,
	//! then takes hold and runs through its stages (RZ_Infection).
	void InfectPlayer(int playerId)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		if (profile.IsInfected())
			return;

		// Halcyon's immunity booster takes one bite (RZ_FactionServices)
		if (m_Services && m_Services.TryShrugOffBite(playerId))
			return;

		RZ_Background bg = GetPlayerBackground(playerId);
		float seconds = m_fInfectionMinutes * 60 * bg.m_fInfectionTimeMult * m_Cure.GetInfectionTimeMultiplier();
		profile.ClearInfection();
		profile.m_fInfectionLeft = seconds;
		profile.m_fInfectionTotal = seconds;
		m_Infection.OnBite(playerId, profile);

		// The signal ring: a bite here (where and how, never who)
		IEntity bitten = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (bitten)
			RZ_WorldWriters.Bite(playerId, bitten.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	//! Public API: call from the antidote item's use action.
	void CurePlayer(int playerId)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		if (!profile.IsInfected())
			return;

		profile.ClearInfection();
		m_Infection.OnCured(playerId);
		NotifyPlayer(playerId, "Cured", "The fever breaks. You'll live, for now.");
	}

	//------------------------------------------------------------------------------------------------
	//! The mode took health itself (a fever): the bite detector mustn't read it as a hit.
	void ForgetBiteHealth(int playerId)
	{
		m_BiteDetector.Reset(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The player dies and gets up as one of them (RZ_Infection, when the time runs out).
	void TurnPlayer(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent)
			return;

		vector pos = ent.GetOrigin();
		RZ_DeathBrief.MarkTurned(playerId);
		RZ_Util.KillCharacter(ent);
		// Grief: nobody jokes near a player who turned, for a while (docs/WRITING.md, the season mood)
		RZ_ToneGuard.Death(pos, -1);
		// Turned in the open near a tribe's range: it takes them (docs/INFECTED.md, the taken)
		m_Tribes.OnTurned(playerId, pos);

		// World event: the turned player roams with their gear nearby
		RZ_Mission_TurnedPlayer turned = new RZ_Mission_TurnedPlayer();
		turned.Configure(this);
		turned.Prepare(RZ_Names.Public(playerId), pos);
		if (!m_Missions.StartMission(turned))
		{
			m_Spawner.SpawnZombie(pos, false, -1, -1);
			NotifyAll("Turned", RZ_Names.Public(playerId) + " has turned.");
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
				NotifyPlayerAs(ids[i], "Horde", "Something big heard you. A horde is moving in.", RZ_ENoticeTag.URGENT);
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

		// The kill ledger: one fewer crawler there until it halves away (Zombies > Crawlers)
		if (s && m_Spawner.GetCrawlers())
			m_Spawner.GetCrawlers().OnKilled(settlementIdx, count);

		// After the cure nothing needs their tissue: no samples, no tip about them
		if (special && !m_Cure.IsCured())
		{
			SpawnSamples(pos, 1);
			m_Tribes.OnSample(1, false);
			m_Guidance.OnSpecialKilled(pos);
		}

		RZ_DeathBrief.OnInfectedKilled(this, pos, count);
		m_Wanderers.OnZombiesKilled(pos);
		m_Leaders.OnZombiesKilled(pos);
		// Kit still on the body is off the turned (RZ_DisguiseMarks)
		if (m_Hostility && m_Hostility.GetDisguise().MarksOn())
			m_Hostility.GetDisguise().GetMarks().OnInfectedKilled(pos);

		// The body carries some loot (its inventory, or beside it), if a player saw it die
		m_Interactions.OnZombiesKilled(pos, count);
		m_Missions.OnZombiesKilled(settlementIdx, missionId, special, pos, count);
		if (m_Epilogue)
			m_Epilogue.OnZombiesKilled(settlementIdx, pos);
	}

	//------------------------------------------------------------------------------------------------
	void DeliverSamples(int playerId, int count)
	{
		m_Cure.AddSamples(playerId, count, m_fResearchPerSample);
	}

	//------------------------------------------------------------------------------------------------
	void SpawnSamples(vector pos, int count)
	{
		// After the cure nothing takes samples (the lab, the Red Hand): none drop
		if (m_Cure && m_Cure.IsCured())
			return;

		for (int i = 0; i < count; i++)
		{
			TrackLooseItem(RZ_Util.SpawnPrefab(m_sSamplePrefab, RZ_Util.RandomPointInRing(pos, 0.3, 1.5) + Vector(0, 0.3, 0)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Public API: an item spawned for players (a crate, a purchase, a find) that should be cleaned
	//! up once it lies abandoned (RZ_LooseItems). Never removed from a pack or a vehicle.
	static void TrackLooseItem(IEntity e)
	{
		if (e && s_Instance && s_Instance.m_LooseItems)
			s_Instance.m_LooseItems.Track(e);
	}

	//------------------------------------------------------------------------------------------------
	//! The crate, or null (no prefab). Missions that may take it back (a booby trap) keep it.
	//! onGround = false: exactly at pos (inside a building, on its floor). Left alone for half an
	//! hour (and not at a running mission), it's cleaned up (RZ_LooseItems).
	IEntity SpawnRewardCrate(vector pos, bool onGround = true)
	{
		if (onGround)
			pos = RZ_Util.GroundPos(pos);

		IEntity crate = RZ_Util.SpawnPrefab(m_sRewardCratePrefab, pos, Math.RandomFloat(0, 360));
		TrackLooseItem(crate);
		return crate;
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
		m_fReplCureProgress = m_Cure.ShownProgress();
		m_fReplCureBacklog = m_Cure.GetBacklog();
		m_bReplLabFull = m_Cure.IsLabFull();
		m_bReplAtGate = m_Cure.IsAtFinaleGate();
		m_fReplPlayPace = m_Cure.PlayPace();
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
	//! Spawn points sit in random, not-too-infected towns and move over time: the spawn screen asks
	//! for one at random (nobody picks a start), and with openings on the spawn moves on from there.
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
			m_aSpawnPoints.RemoveOrdered(0);
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
			if (s.m_fInfection < 70 && !s.m_bIsland)
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
			vector pos = SpawnPointSpot(town);
			if (pos == vector.Zero)
				continue;

			IEntity ent = SpawnNamedSpawnPoint(pos, town.m_sName);
			if (!ent)
				return;

			m_aSpawnPoints.Insert(ent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A fresh scan keeps no town on an island: nothing is ever put there.
	protected void DropIslandTowns()
	{
		for (int i = m_aSettlements.Count() - 1; i >= 0; i--)
		{
			RZ_Settlement s = m_aSettlements[i];
			if (RZ_Mainland.OnMainland(s.m_vCenter))
				continue;

			RZ_Log.Info(string.Format("Mainland: %1 at %2 is on an island: not a settlement", s.m_sName, RZ_SettlementScanner.GridReference(s.m_vCenter)));
			m_aSettlements.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A town whose middle came out in the water (a cluster on a shore, its buildings' average) has it
	//! moved to its nearest dry house, or building: what walks to a town's middle (wanderers, the
	//! errands' towns) never walks into the sea.
	protected void DryTownMiddles()
	{
		foreach (RZ_Settlement s : m_aSettlements)
		{
			if (!RZ_PlaceCheck.IsWet(s.m_vCenter))
				continue;

			array<IEntity> buildings = {};
			RZ_PlaceCheck.Houses(s.m_vCenter, 0, s.m_fRadius, buildings);
			if (buildings.IsEmpty())
				RZ_BuildingQuery.FindBuildings(s.m_vCenter, s.m_fRadius, buildings, 200);

			vector best = vector.Zero;
			float bestDist = float.MAX;
			foreach (IEntity b : buildings)
			{
				vector at = RZ_Util.GroundPos(b.GetOrigin());
				float d = RZ_Util.DistXZ(at, s.m_vCenter);
				if (d < bestDist && !RZ_PlaceCheck.IsWet(at))
				{
					bestDist = d;
					best = at;
				}
			}

			if (best == vector.Zero)
				continue;

			RZ_Log.Info(string.Format("Settlements: %1's middle was in the water: moved %2 m to %3", s.m_sName, RZ_Util.RoundToInt(bestDist), RZ_SettlementScanner.GridReference(best)));
			s.m_vCenter = best;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where a spawn point in this town stands: just outside a house in it (docs/PLACES.md, spawn.point:
	//! a point anywhere in the town's ring stood in woods or fields as often as not); zero if the try
	//! missed (the place audit asks it too).
	vector SpawnPointSpot(RZ_Settlement town)
	{
		vector pos = RZ_PlaceFind.Near(town.m_vCenter, 0, town.m_fRadius * 0.8, RZ_PlaceNeeds.Get("spawn.point"), m_aSettlements.Find(town));
		if (pos == vector.Zero || RZ_Util.IsWater(pos) || m_Perimeter.IsNearLine(pos))
			return vector.Zero;

		// Never within 1 km of a pack's lair (docs/INFECTED.md, People)
		if (m_Tribes && m_Tribes.GetPeople().IsOn() && m_Tribes.GetPeople().NearLair(pos, 1000))
			return vector.Zero;

		return pos;
	}

	//------------------------------------------------------------------------------------------------
	//! A player spawn point (the World > Spawn point prefab), named after its town. Null without the
	//! prefab. The spawn screen asks for one at random; an opening then moves the new character to the
	//! place it rolled (Guidance/RZ_OpeningPlaces.c).
	IEntity SpawnNamedSpawnPoint(vector pos, string name)
	{
		IEntity ent = RZ_Util.SpawnPrefab(m_sSpawnPointPrefab, pos, Math.RandomFloat(0, 360));
		if (!ent)
			return null;

		SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(ent);
		if (sp)
		{
			sp.SetFactionKey(m_sPlayerFactionKey);
			sp.SetSpawnPointName(name);
		}

		return ent;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	protected string GetWorldId()
	{
		return GetGame().GetWorldFile();
	}

	//------------------------------------------------------------------------------------------------
	//! The map, as saves and season records name it.
	string GetWorldName()
	{
		return GetWorldId();
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

		if (!data.settlements || !data.profiles)
		{
			RZ_Log.Warn("Save file is corrupt or from an incompatible version, ignoring.");
			return false;
		}

		foreach (RZ_SettlementSave ss : data.settlements)
		{
			if (!ss)
				continue;

			RZ_Settlement s = new RZ_Settlement();
			s.m_sName = ss.name;
			s.m_vCenter = Vector(ss.x, ss.y, ss.z);
			s.m_fRadius = ss.radius;
			s.m_iSize = ss.size;
			s.m_fInfection = ss.infection;
			s.m_bHasLab = ss.lab;
			s.m_fLedger = Math.Max(0, ss.ledger);
			s.m_fStir = Math.Clamp(ss.stir, 0, 100);
			m_aSettlements.Insert(s);
		}

		foreach (RZ_PlayerProfile p : data.profiles)
		{
			if (!p || p.m_sIdentity.IsEmpty() || p.m_sIdentity == "local_")
				continue;

			p.EnsureArrays(); // profiles from older saves have no Fame/Infamy/Favor yet
			m_mProfiles.Set(p.m_sIdentity, p);
		}

		m_Director.Restore(data.stage, data.stageMinutes, data.playedMinutes);
		// The finale first: whether the batch waits for its seal decides if a saved stand restarts
		m_Finale.Restore(data.finale);
		m_Epilogue.Restore(data.epilogue);
		m_Cure.Restore(data.cureProgress, data.cureMilestone, data.cured, data.labAntidotes, data.stolenAntidotes, data.labStabilisers);
		m_Cure.RestoreLab(data);
		if (data.seasonFame)
		{
			for (int f = 0; f < data.seasonFame.Count() && f < RZ_Factions.COUNT; f++)
			{
				m_aSeasonFame[f] = data.seasonFame[f];
			}
		}

		m_iSeasonWinner = data.seasonWinner;
		m_Seasons.Restore(data);

		m_bHasLab = data.hasLab;
		m_vLabPos = Vector(data.labX, 0, data.labZ);
		m_Perks.RestoreQuartermasters(data.qmX, data.qmZ, data.qmSet);
		m_Safehouses.Restore(data.safehouses);
		m_Wanderers.Restore(data.wanderers);
		m_Leaders.Restore(data.leaders);
		m_Territory.Restore(data.territory);
		m_Supply.Restore(data.supply);
		m_FactionLife.Restore(data.factionLife);
		m_LongTable.Restore(data.longTable);
		m_TownMemory.Restore(data.towns);
		m_PowerGrid.Restore(data.power);
		m_PlacedThings.Restore(data.placed);
		m_RadioStation.Restore(data.radio);
		m_Facts.Restore(data.facts);
		m_Shared.Restore(data.shared);
		m_StoryArcs.Restore(data.story);
		m_Questlines.Restore(data.questlines);
		m_Headlines.Restore(data.news);
		m_Interactions.Restore(data.interactions);
		m_Errands.Restore(data.errands);
		m_Traces.Restore(data.traces);
		if (m_Spawner.GetCrawlers())
			m_Spawner.GetCrawlers().Restore(data.crawlers);

		m_Tribes.Restore(data.tribes);
		m_Hostility.GetDisguise().GetMarks().Restore(data.disguise);
		m_Perks.Restore(data.perks);
		m_Services.Restore(data.perks);

		if (data.hasTimeOfDay)
			m_fSavedHour = data.timeOfDay;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A save now (the epilogue's commit asks for one).
	void SaveNow()
	{
		SaveState();
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
		data.playedMinutes = m_Director.GetPlayedMinutes();
		data.cureProgress = m_Cure.m_fProgress;
		data.cureMilestone = m_Cure.m_iMilestone;
		data.cured = m_Cure.IsCured();
		data.labAntidotes = m_Cure.GetLabAntidotes();
		data.stolenAntidotes = m_Cure.GetStolenAntidotes();
		data.labStabilisers = m_Cure.GetLabStabilisers();
		m_Cure.SaveLab(data);
		data.seasonWinner = m_iSeasonWinner;
		foreach (int fame : m_aSeasonFame)
		{
			data.seasonFame.Insert(fame);
		}
		if (m_Seasons)
			m_Seasons.Save(data);

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

		if (m_LongTable && m_LongTable.IsEnabled())
			data.longTable = m_LongTable.Save();

		if (m_TownMemory && m_TownMemory.IsEnabled())
			data.towns = m_TownMemory.Save();

		if (m_PowerGrid && m_PowerGrid.IsActive())
			data.power = m_PowerGrid.Save();

		if (m_PlacedThings && m_PlacedThings.IsOn())
			data.placed = m_PlacedThings.Save();

		if (m_RadioStation && m_RadioStation.IsActive())
			data.radio = m_RadioStation.Save();

		if (m_StoryArcs && m_StoryArcs.IsEnabled())
			data.story = m_StoryArcs.Save();

		if (m_Questlines && m_Questlines.IsEnabled())
			data.questlines = m_Questlines.Save();

		if (m_Finale && RZ_CureFinale.IsOn())
			data.finale = m_Finale.Save();

		if (m_Epilogue && RZ_Epilogue.IsOn())
			data.epilogue = m_Epilogue.Save();

		if (m_Facts)
			data.facts = m_Facts.Save();

		if (m_Shared)
			data.shared = m_Shared.Save();

		if (m_Headlines)
			data.news = m_Headlines.Save();

		if (m_Interactions && m_Interactions.IsActive())
			data.interactions = m_Interactions.Save();

		if (m_Errands)
			data.errands = m_Errands.Save();

		if (m_Traces && m_Traces.IsOn())
			data.traces = m_Traces.Save();

		if (m_Spawner && m_Spawner.CrawlersOn())
			data.crawlers = m_Spawner.GetCrawlers().Save();

		if (m_Tribes && m_Tribes.IsEnabled())
			data.tribes = m_Tribes.Save();

		if (m_Hostility && m_Hostility.GetDisguise().IsOn())
			data.disguise = m_Hostility.GetDisguise().GetMarks().Save();

		if (m_Perks)
		{
			data.perks = m_Perks.Save();
			if (m_Services)
				m_Services.Save(data.perks);
		}

		// Dialogue memory has its own file, written only when something changed
		if (m_Dialogue)
			m_Dialogue.GetTrees().SaveIfChanged();

		// The time of day, so a restart carries on where the day stopped (Keep time of day)
		float hour = RZ_GameTime.GetHour();
		if (m_bKeepTimeOfDay && hour >= 0)
		{
			data.hasTimeOfDay = true;
			data.timeOfDay = hour;
		}

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
			ss.ledger = s.m_fLedger;
			ss.stir = s.m_fStir;
			data.settlements.Insert(ss);
		}

		foreach (string ident, RZ_PlayerProfile p : m_mProfiles)
		{
			// A profile that never lived and holds nothing lasting is noise: don't save it
			if (p.IsBlank())
				continue;

			data.profiles.Insert(p);
		}

		RZ_Persistence.Save(data);
	}

	// ================================================================ Notifications
	//------------------------------------------------------------------------------------------------
	//! Everyone: a broadcast.
	void NotifyAll(string title, string text)
	{
		RpcDo_Notify(-1, title, text, RZ_ENoticeTag.AUTO);
		Rpc(RpcDo_Notify, -1, title, text, RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	//! Everyone, with a tag (RZ_ENoticeTag): a broadcast.
	void NotifyAllAs(string title, string text, RZ_ENoticeTag tag)
	{
		RpcDo_Notify(-1, title, text, tag);
		Rpc(RpcDo_Notify, -1, title, text, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! One player: a notice card (docs/UI.md, "Notices"). Its importance comes from its title.
	void NotifyPlayer(int playerId, string title, string text)
	{
		NotifyPlayerAs(playerId, title, text, RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	//! One player, with a tag that sets what the notice is (a mission, a world event, the world at
	//! spawn) or how important it is (RZ_ENoticeTag). Sent to that player's machine only, through
	//! RZ_PlayerUIComponent; only a controller without it gets the old broadcast that each client
	//! filters for itself. Never both.
	void NotifyPlayerAs(int playerId, string title, string text, RZ_ENoticeTag tag)
	{
		// What a car job or a purchase answers goes into that screen's status line
		if (RZ_ScreenCapture.Take(playerId, title, text))
			return;

		// A line for a player who is talking goes into the talk screen (RZ_Dialogue.Capture)
		if (m_Dialogue && m_Dialogue.Capture(playerId, title, text, tag))
			return;

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return; // not connected: no screen to show it on

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.Cast(pc.FindComponent(RZ_PlayerUIComponent));
		if (ui)
		{
			ui.SendNotice(title, text, tag);
			return;
		}

		RpcDo_Notify(playerId, title, text, tag);
		Rpc(RpcDo_Notify, playerId, title, text, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! A person speaks to one player: a subtitle at the bottom of the screen (docs/UI.md,
	//! "Subtitles"), owner-only like notices. speaker: "Name, role", as their notices are titled;
	//! faction: RZ_EFaction or -1. who: the speaker's body when the caller has it. The line goes with
	//! the name, the role, how it reaches the player and, in person, where the speaker stands
	//! (RZ_SayLine); without a body that's the nearest person within RZ_SayLine.NEAR of the player.
	void Say(int playerId, string speaker, int faction, string text, IEntity who = null)
	{
		SayFrom(playerId, speaker, faction, text, who, vector.Zero);
	}

	//------------------------------------------------------------------------------------------------
	//! Say, from a place when there's no body (`at`, vector.Zero for none).
	protected void SayFrom(int playerId, string speaker, int faction, string text, IEntity who, vector at)
	{
		if (m_Dialogue && m_Dialogue.Capture(playerId, speaker, text, RZ_ENoticeTag.AUTO, true))
			return;

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.Cast(pc.FindComponent(RZ_PlayerUIComponent));
		if (!ui || !RZ_UISettings.Get().m_bSubtitles)
		{
			NotifyPlayer(playerId, speaker, text);
			return;
		}

		RZ_SayLine line = RZ_SayLine.Make(speaker, faction, text);
		if (line.m_iSource == RZ_ESaySource.IN_PERSON)
		{
			IEntity listener = pc.GetControlledEntity();
			IEntity body = who;
			if (!body && at == vector.Zero && listener)
				body = RZ_SpeakerFinder.Nearest(listener.GetOrigin(), RZ_SayLine.NEAR);

			line.Place(body, at);
		}

		ui.SendSayLine(line);
	}

	//------------------------------------------------------------------------------------------------
	//! A person speaks to everyone alive within `radius` of `pos` (from `who`, else from `pos`).
	void SayNear(vector pos, float radius, string speaker, int faction, string text, IEntity who = null)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				SayFrom(ids[i], speaker, faction, text, who, pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Client: a notice for this machine's player (RZ_PlayerUIComponent.RpcDo_Notice).
	void ShowNotice(string title, string text, int tag)
	{
		if (System.IsConsoleApp())
			return;

		QueueHint(title, text, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! Client: a plain spoken line for this machine's player (RZ_PlayerUIComponent.RpcDo_Say), worked
	//! out from its title.
	void ShowSay(string speaker, int faction, string text)
	{
		ShowSayLine(RZ_SayLine.Make(speaker, faction, text));
	}

	//------------------------------------------------------------------------------------------------
	//! Client: a spoken line for this machine's player (RZ_PlayerUIComponent.RpcDo_SayLine): a
	//! subtitle, a radio line in the radio-line box, or a card where the HUD has no place for it.
	void ShowSayLine(RZ_SayLine line)
	{
		if (System.IsConsoleApp() || !line)
			return;

		if (RZ_HudDisplay.PushLine(line))
			return;

		QueueHint(line.m_sSpeaker, line.m_sText, RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	//! The fallback for NotifyAll and for controllers without RZ_PlayerUIComponent: a broadcast,
	//! with each client filtering for itself.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Notify(int targetPlayerId, string title, string text, int tag)
	{
		if (System.IsConsoleApp())
			return; // dedicated server has no screen

		if (targetPlayerId != -1 && SCR_PlayerController.GetLocalPlayerId() != targetPlayerId)
			return;

		QueueHint(title, text, tag);
	}

	// ---------------------------------------------------------------- Client hint queue
	//! The fallback: the vanilla hint widget shows one hint at a time, so messages that arrive
	//! together (e.g. the three on spawn) are queued and shown one after another.
	protected ref array<string> m_aHintTitles = {};
	protected ref array<string> m_aHintTexts = {};
	protected ref array<int> m_aHintPriority = {};
	protected bool m_bHintShowing;

	protected static const int HINT_SECONDS = 9;
	protected static const int HINT_QUEUE_MAX = 12;

	//------------------------------------------------------------------------------------------------
	protected void QueueHint(string title, string text, int tag)
	{
		// Chat commands (/note, /noteclear): registered once, here, on the client
		RZ_ChatCommands.Register();

		// The ReforgerZ UI (UI/): the notebook's Radio tab keeps radio, mission and world notices,
		// and the styled card shows it if it can. Otherwise the vanilla hint below.
		RZ_NoticeStyle style = RZ_NoticeStyle.Classify(title, tag);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();

		// The quiet start (docs/UI.md, "Notices"): for UI > Quiet start after a new life only the
		// spawn card, a job of yours starting and urgent notices are cards; the rest go into the
		// notebook without one
		if (ui && !style.m_bSpawn && !style.m_bStart && style.m_iPriority < RZ_ENoticePriority.URGENT)
		{
			float lifeAt = ui.GetNewLifeAt();
			if (lifeAt >= 0 && RZ_UIW.Now() - lifeAt < RZ_UISettings.Get().m_fNoticeCalm)
				style.m_bQuiet = true;
		}

		if (ui)
			ui.OnNotice(title, text, style);

		if (style.m_bQuiet && ui)
		{
			RZ_HudDisplay.NoteQuiet();
			return;
		}
		// A radio channel's voice goes to the radio-line box at the top left, not a card (docs/UI.md,
		// "Subtitles"); it's in the Radio tab all the same
		string channel = RZ_SayLine.ChannelOf(title);
		if (!channel.IsEmpty() && RZ_HudDisplay.PushLine(RZ_SayLine.Radio(title, channel, style.m_iFaction, text), true))
			return;

		if (RZ_HudDisplay.PushNotice(title, text, style))
			return;

		// The vanilla hint shows one at a time: the most important first, then the oldest. It has
		// nowhere else to go, so nothing expires; past the cap the least important newest is dropped
		int idx = m_aHintTitles.Count();
		foreach (int i, int prio : m_aHintPriority)
		{
			if (prio < style.m_iPriority)
			{
				idx = i;
				break;
			}
		}

		m_aHintTitles.InsertAt(title, idx);
		m_aHintTexts.InsertAt(text, idx);
		m_aHintPriority.InsertAt(style.m_iPriority, idx);
		while (m_aHintTitles.Count() > HINT_QUEUE_MAX)
		{
			int last = m_aHintTitles.Count() - 1;
			m_aHintTitles.Remove(last);
			m_aHintTexts.Remove(last);
			m_aHintPriority.Remove(last);
		}

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
		m_aHintTitles.RemoveOrdered(0);
		m_aHintTexts.RemoveOrdered(0);
		m_aHintPriority.RemoveOrdered(0);

		SCR_HintManagerComponent.ShowCustomHint(text, title, HINT_SECONDS);
		GetGame().GetCallqueue().CallLater(ShowNextHint, HINT_SECONDS * 1000, false);
	}
}
