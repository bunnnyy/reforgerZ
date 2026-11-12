# Workbench setup

How to turn the scripts in this repo into a playable mod. The scripts are plain Enforce
Script; prefabs and configs are made in Workbench, because they need GUIDs that Workbench
generates. The repo ships its own `addon.gproj` (mod ID `A88C8A4C8C74454A`, picked at random;
change it before the first Workshop upload if you like). The game scripts compile cleanly on the dedicated server
([how](#compile-check-without-workbench)); the `VERIFY` list is what compiling can't prove.

Everything you must create or configure is listed once, in the **prefab manifest**
(`Configs/ReforgerZ/PrefabManifest.json`). The Workbench plugin **ReforgerZ: Create prefabs**
reads it and makes what it can; the tables in sections 2 to 5 are generated from it and double
as the manual checklist.

A first pass is done, in Workbench 1.9 on Arland and Everon: the repo holds the plugin's prefabs
(the quartermasters' and the notebook's actions among them), camp life's 49 compositions, its
smart-action waypoint and six kits, the faction and loadout managers, the worlds (Everon, the main
one, and Arland) and their scenarios, and most of the UI. Many prefabs are built on stand-ins
([what that means](#stand-in-bases)), and the spawn screen and the map key aren't set up yet
([the UI](#setting-up-the-ui)); the map's icons and the car picture are. This doc says how it was done, what's left, and
how to do it again on another map.

Work through the sections in order:

1. [Create the prefabs](#1-create-the-prefabs)
2. [The minimum playable subset](#2-the-minimum-playable-subset)
3. [Factions and loadouts](#3-factions-and-loadouts)
4. [The prefab manifest](#4-the-prefab-manifest)
5. [Settings objects](#5-settings-objects)
6. [The VERIFY list](#6-the-verify-list)
7. [Log lines to check](#7-log-lines-to-check)
8. [Play-test checklist](#8-play-test-checklist)

How each system behaves is in [DESIGN.md](DESIGN.md); this doc only says how to set it up.
Getting the PC ready first (Steam apps, the repo, opening the project, local tests) is in
[PC_SETUP.md](PC_SETUP.md).

## 1. Create the prefabs

Run **ReforgerZ: Create prefabs** (or follow the manual checklist). First:

### 1.1 Before you start: project and dependencies

1. Subscribe to the dependencies in the game ([MOD_STACK.md](MOD_STACK.md)): at minimum
   **Bacon Zombies** and **Special Zombies** (both are in `addon.gproj`); Metabolism is recommended.
2. Arma Reforger Tools → **+ Add Project → Add Existing Project** → pick this repo's
   `addon.gproj`, then **Open**.
3. **File → Project Settings → Dependencies**: the repo's `addon.gproj` already lists Arma
   Reforger, Bacon Zombies and Special Zombies. To add another mod, click **+**, then the new
   row's **...**, and pick the mod's `addon.gproj` in the game's Workshop folder. Workbench only
   loads a dependency it can find, so start it with that folder in `-addonsDir`
   ([PC_SETUP.md](PC_SETUP.md#5-workbench)). Commit the changed `addon.gproj`.
4. Open the Script Editor and compile (**F7**). The game scripts compile cleanly on 1.9; the plugin
   compiles with one warning (`Workbench.SearchResources` is obsolete in 1.9, but still works).
   [The VERIFY list](#6-the-verify-list) is what to watch in play.
5. Open the repo's main world, `Worlds/RZ_Everon/RZ_Everon.ent` (a **sub-scene of Eden**), in the
   World Editor; `Worlds/RZ_Arland/RZ_Arland.ent` is the small test world. Or make another
   ([world steps](#the-world)).

Look up classes in the Script Editor or the
[Script API docs](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/).

### 1.2 Run "ReforgerZ: Create prefabs"

The plugin in `scripts/WorkbenchGame/ReforgerZ/RZ_CreatePrefabsPlugin.c` reads the manifest and
does as much of sections 2 to 4 as it can. It has run for real in Workbench 1.9 on the repo's
world: **334 steps done, 8 skipped, 0 failed, 54 manual steps left**. What it made is in the repo:
156 prefabs under `Prefabs/ReforgerZ/`, each with its `.meta`, and `RZ_GameMode.et` with 147 slots
filled. Run a second time, it did nothing (0 done, 341 skipped, 0 failed): it reads back what's
there and only fills what's empty. The real run settled every editor call the plugin wasn't sure
of (saving a prefab with its `.meta`, editing a prefab loaded from its file, the editor's names,
reading values back, the factions and loadouts inline on the managers, making folders), so it has
no `VERIFY` left. Use the [manual checklist](#13-by-hand-the-manual-checklist) for whatever it
can't do.

**Known issue: check the game mode after a run.** The first run placed the game mode in the
world as a plain `SCR_BaseGameMode` entity, not as an instance of `RZ_GameMode.et`, so without
`RZ_GameModeComponent` (it was fixed by hand). Select it in the World Editor's Hierarchy: the
prefab column must show `RZ_GameMode`. If it doesn't, delete it and drag `RZ_GameMode.et` in from
`Prefabs/ReforgerZ/Core/`.

1. Compile the scripts (the plugin is in the Workbench script module, so `RZ_` classes must exist).
2. Open your world (the Everon or Arland sub-scene) in the World Editor.
3. **Plugins → ReforgerZ: Create prefabs.** The options dialog opens with **Dry run** on. Run it
   once like that: it changes nothing and lists what it would do in the log (search for
   `[ReforgerZ]`).
4. Turn **Dry run** off and run it again. Workbench's Diag build shows **Script Authorization
   Required** when the plugin makes its folders (`FileIO.MakeDirectory`): answer **Yes**, or
   **Yes to All** so it stops asking ([Workbench's dialogs](PC_SETUP.md#how-claude-drives-the-pc)).
   Check the game mode (above), then save the world.
5. Read the summary: *done*, *skipped* (already there), *failed* (with a reason) and *manual steps
   left*. Fix what failed, do the manual steps, and run it again: it never touches what exists.

**What it does, in order:**

| Step | What | Idempotent because |
|---|---|---|
| Prefabs | Creates each prefab at its path from its base, adds the components and actions with their values, and an `RplComponent` and `ActionsManagerComponent` where needed. The manager's action context sits 0.5 m up with a 1.2 m radius: resize it by hand on big props (a tent, a wreck). `@Antidote`-style values become that prefab's ResourceName | a prefab whose file exists is skipped |
| Loadouts | Creates the 13 loadout characters, each with `RZ_TreatOtherAction` | as above |
| Game mode | Creates `RZ_GameMode.et`, creates its settings objects and fills every slot it has a prefab for | only empty slots are filled; lists only when empty |
| World | Places the game mode in the open world (check it: the known issue above); checks for a FactionManager and LoadoutManager | skipped if the world already has a game mode with `RZ_GameModeComponent` |
| Factions | Adds the missing `SURV_H1`..`SURV_H15` and `SURV_OUTLAW` to the placed FactionManager as bare factions (`SURV`'s colour and name, not playable, but no base config, so no flag or identity) and the missing friendly keys; reports hostile pairs that are friendly | existing keys and friends are left alone |
| Loadout entries | Adds the 13 loadouts on `SURV` to the LoadoutManager | existing names are skipped |
| Additions | Lists the changes to prefabs it doesn't own | — |

**Options:**

| Option | Default | What |
|---|---|---|
| Dry run | on | Only report |
| Only the minimum playable subset | off | Just the entries marked minimum (plus the game mode, loadouts and world) |
| Make prefabs with no known base from an empty entity | off | Off: a prefab whose base isn't known is a manual step. On: it's made from a `GenericEntity` with all its parts, and you add the mesh |
| Set up the open world | on | The world, faction and loadout steps |
| Manifest file | `$ReforgerZ:Configs/ReforgerZ/PrefabManifest.json` | Where the manifest is |
| Addon prefix | `$ReforgerZ:` | Where new prefabs are saved |

**Bases.** The plugin builds each prefab on the entry's `basePrefab` (a ResourceName), else on a
search for `baseFind` (a vanilla file name), else it leaves the prefab as a manual step (or, with
the empty-entity option on, makes it from a `GenericEntity`). Only three entries have a `baseFind`
(`GameMode_Plain.et`, `SpawnPoint_Base.et`, `AIWaypoint_Move.et`), because vanilla names change
between versions. Which bases the manifest names, and which are stand-ins, is in
[section 4](#4-the-prefab-manifest).

**The fallback.** **Plugins → ReforgerZ: Write prefab checklist** writes the whole manifest as a
checklist to `PrefabChecklist.md` in the project folder (git ignores it), with what already
exists ticked. If even that fails, the tables in this doc are the same checklist.

**Known limitations:**
- It can't choose meshes, fill inventories (loadout gear, the reward crate), set collision layers
  (the interaction spot, the doorway blocker), make AI groups' members or set a group's faction
  (`m_faction` was set by hand on the 23 group prefabs). It doesn't set characters' faction
  affiliation either (set afterwards on the 13 loadouts and the 30 other characters).
- Its faction variants are bare (no base config, so no flag or identity): keep the variants in
  `RZ_FactionManager.et` ([section 3](#3-factions-and-loadouts)). If a FactionManager keeps its
  factions in a referenced `.conf`, it can't edit them.
- It doesn't set the UI slots or the game mode's player controller
  ([Setting up the UI](#setting-up-the-ui)).
- It doesn't overwrite anything: to redo a prefab, delete it first.
- Shop stock beyond the antidote, stabiliser, cut dose and signal smoke, requisition items and loot
  tables are by hand.

### 1.3 By hand: the manual checklist

Work down the tables: [the minimum playable subset](#2-the-minimum-playable-subset) first, then
[factions and loadouts](#3-factions-and-loadouts), then the rest of [the manifest](#4-the-prefab-manifest)
system by system. For each prefab:

1. Take the entry's `basePrefab` from the manifest, or find the suggested base in the Resource
   Browser (vanilla paths and names differ between game versions, so search by description), and
   **Create inherited prefab** at the path in the table (all under `Prefabs/ReforgerZ/`).
2. Add the components and actions in the **Add** column, with the attribute values shown.
   **The generic set** is `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`, `RZ_ReadClueAction`,
   4 × `RZ_PuzzleUseAction` (Value 0–3) and 4 × `RZ_PuzzleChoiceAction` (Option 0–3).
   **The clue set** is `RZ_ClueComponent` and `RZ_ReadClueAction`.
3. Every prefab with an `RZ_` component needs an `RplComponent`; every prefab with an action
   needs an `ActionsManagerComponent` with at least one action context, and each action's
   parent context set to it. Give the context a position and a radius (the plugin uses 0.5 m up
   and 1.2 m): without them the game never offers its actions.
4. Open `RZ_GameMode.et` and set the slot (the attribute's category, then its label).

Slot names are the attribute's category, then its label: **Cure > Lab prefab** is the Lab prefab
field under the ReforgerZ|Cure category of `RZ_GameModeComponent`, and **Survival > Infection >
Stabiliser** is a field of the Infection settings object in the Survival category.

### 1.4 Changing the manifest

The manifest is the one source of truth for setup. After changing a prefab attribute, a
component or an action in the scripts, update the manifest and run:

```bash
python3 tools/prefab_manifest.py check   # every prefab field, component and action covered; plugin keys match
python3 tools/prefab_manifest.py sync    # copy the settings objects' defaults from the scripts
python3 tools/prefab_manifest.py docs    # regenerate the tables in this doc
```

`docs` rewrites everything between each `<!-- manifest:... -->` marker in this doc and its end
marker, so never edit inside them by hand: change the manifest and run it again.

To use a prefab you already have (or a confirmed vanilla base), put its ResourceName
(`{GUID}path.et`) in the entry's `basePrefab`. Lists (zombies, cars, the loot pool) take several,
comma-separated. The plugin doesn't rebuild a prefab that exists: delete it first.

## 2. The minimum playable subset

A feature whose prefab is empty turns itself off, so you can start with this and add the rest
over time. With the [world steps](#the-world), the [factions and loadouts](#3-factions-and-loadouts)
(the 15 hidden variants can wait) and these prefabs, you get spawning, backgrounds, the infected,
hordes, the cure, mission rewards, shops, leaders, safehouse services, the trial dose, delivery
and choice missions, and puzzles:

<!-- manifest:minimum -->
| Prefab | Suggested base | Add | Slot | Gets you |
|---|---|---|---|---|
| `RZ_GameMode.et` | vanilla GameMode_Plain (or any SCR_BaseGameMode prefab) | `RZ_GameModeComponent` | (placed in the world) | the mode |
| `RZ_SpawnPoint.et` | a vanilla spawn point (SCR_SpawnPoint), e.g. SpawnPoint_Base.et or SpawnPoint_US.et; confirm the name in Workbench | — | World > Spawn point prefab | spawn points in towns, safehouse spawns |
| `RZ_Sample.et` | a small item (a vial or sample tube) | `RZ_SampleItemComponent` (Value 1) | Cure > Sample | samples from specials |
| `RZ_Antidote.et` | vanilla morphine or saline item (keeps its consumable component) | `RZ_AntidoteItemComponent`; `RZ_InjectAntidoteAction` | Cure > Antidote | antidotes; optionally set the consumable effect to RZ_ConsumableAntidote by hand |
| `RZ_Lab.et` | a field hospital tent or lab container prop | `RZ_LabComponent`; `RZ_DepositSamplesAction`, `RZ_LabStatusAction` | Cure > Lab prefab | the cure (spawned in a town if none is placed); a second copy is Mobile Lab's field annex for 3 hours |
| `RZ_RewardCrate.et` | a vanilla supply box, filled with loot by hand | — | Missions > Reward crate | mission rewards |
| `RZ_PuzzleObject.et` | a crate, sign or post prop | the generic set | Puzzles > Puzzle object | every puzzle and interaction fallback; missions relabel the actions |
| `RZ_Civilian.et` | a civilian character without AI, or a seated figure prop | the generic set | Puzzles > Civilian | puzzles, unarmed missions, wanderers, leaders and occupants to talk to, the one-choice jobs' stand-in person |
| `RZ_Note.et` | a paper or notebook prop | the clue set | Puzzles > Note | every puzzle mission, scenario clues |
| `RZ_MissionItem.et` | a small carryable item (a box or data drive) | `RZ_MissionItemComponent` | Missions > Mission item | every delivery and choice mission; tagged when it spawns |
| `RZ_Drop_Haven.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction SURVIVORS) | Missions > Haven drop point | the drops those missions name |
| `RZ_Drop_Bastion.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction MILITARY) | Missions > Bastion drop point | the drops those missions name |
| `RZ_Drop_RedHand.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction BANDITS) | Missions > Red Hand drop point | the drops those missions name; also Car Theft's chop shop |
| `RZ_Drop_Halcyon.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction INSTITUTE) | Missions > Halcyon drop point | the drops those missions name |
| (existing prefab) | Bacon Zombies' civilian group, and the repo's unarmed copies of its military groups (Prefabs/ReforgerZ/Zombies/: Bacon's own each have one infected with a pistol) | — | Zombies > Common prefabs (list) | the infected |
| (existing prefab) | Special Zombies, or Bacon heavy, exploding and ranged | — | Zombies > Special prefabs (list) | samples, the cure |
| (existing prefab) | a large Bacon group | — | Zombies > Horde prefabs (list) | hordes |
| (existing prefab) | vanilla AIWaypoint_Move.et | — | Zombies > Move waypoint | hordes toward noise, unarmed missions, hunter squads, supply trucks, favourite spots |
| `RZ_QM_Haven.et` | a civilian character without AI (damage off) | `RZ_QuartermasterComponent` (Faction SURVIVORS); `RZ_ShopOpenAction`, `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), 8 × `RZ_SafehouseAction` (Kind STATUS, DELIVER_FOOD, RADIO, HEAL, DECLARE_BITE, MEDS, BOARD_READ, BOARD_WRITE), 3 × `RZ_FactionServiceAction` (Service AMENDS, SETTLE, ROADS), 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER) | Factions > Haven quartermaster | the Haven shop, leader and safehouse services |
| `RZ_QM_Bastion.et` | a soldier character without AI (damage off) | `RZ_QuartermasterComponent` (Faction MILITARY); `RZ_ShopOpenAction`, `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), 2 × `RZ_FactionServiceAction` (Service AMENDS, SETTLE), `RZ_FactionShopAction` (Faction MILITARY, Required tier CHAMPION, Favor cost 30, Item prefab `RZ_SignalSmoke.et`, Display name Signal smoke), `RZ_FactionServiceAction` (Service CONTACT_BOARD), 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER) | Factions > Bastion quartermaster | the Bastion shop and leader |
| `RZ_QM_RedHand.et` | a civilian character without AI in rough clothes (damage off) | `RZ_QuartermasterComponent` (Faction BANDITS); `RZ_ShopOpenAction`, `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_FactionShopAction` (Faction BANDITS, Required tier KNOWN, Favor cost 12, Item prefab `RZ_CutDose.et`, Display name Cut dose), 6 × `RZ_FactionServiceAction` (Service FENCE, BOUNTY_BOARD, AMENDS, SETTLE, CREW_MAP, FREQUENCIES), 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER) | Factions > Red Hand quartermaster | the Red Hand black market and leader |
| `RZ_QM_Halcyon.et` | a character in a lab coat without AI (damage off) | `RZ_QuartermasterComponent` (Faction INSTITUTE); `RZ_ShopOpenAction`, `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_TrialDoseAction`, `RZ_FactionShopAction` (Faction INSTITUTE, Required tier TRUSTED, Favor cost 60, Item prefab `RZ_Antidote.et`, Display name Antidote); `RZ_FactionShopAction` (Faction INSTITUTE, Required tier KNOWN, Favor cost 40, Item prefab `RZ_Stabiliser.et`, Display name Stabiliser), 6 × `RZ_FactionServiceAction` (Service MEDIC, BOOSTER, AMENDS, SETTLE, RANGE_MAP, CALL_BOX), 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER) | Factions > Halcyon quartermaster | the Halcyon shop, leader, antidotes and the trial dose |
<!-- /manifest:minimum -->

No towns, spawn points or lab need placing: they're generated at start. With only this,
these stay off: hunter squads, Perimeter checkpoints and drones (the line still works with
messages), supply trucks (places count as supplied), territory guards, adaptation traits, the
power station's own control prefab (it falls back), world-interaction loot (nothing lies in
houses until the loot pool is filled), fixable cars, crafting and traps, and the treatments other than the antidote and the trial dose
(stabiliser, low-grade meds).

### The world

<!-- manifest:world -->
| Step | Plugin | Minimum |
|---|---|---|
| Make a world as a sub-scene of the map (File > Load World the map, then File > New World > Sub-scene (of current world)); the repo has Worlds/RZ_Everon (the main world, on Eden) and Worlds/RZ_Arland (the small test world) | manual | yes |
| Place RZ_FactionManager.et (Prefabs/MP/Managers/Factions/: the four factions, the 15 hidden variants and SURV_OUTLAW below) | checks it | yes |
| Place RZ_LoadoutManager.et (Prefabs/MP/Managers/Loadouts/: the 13 loadouts on SURV) | checks it | yes |
| Place the map's AI world (vanilla Prefabs/AI/SCR_AIWorld_Arland.et, _Eden or _Cain) and set its Soldiers and BTRlike navmesh files (on Everon Navmesh_GM_Eden_Soldier.conf and GM_Eden_vehicles.nmn; on Arland worlds/GameMaster/Navmeshes/GM_Arland.nmn and GM_Arland_vehicles.nmn): without it no AI group gets members and there is no road network | manual | yes |
| Place the vanilla world managers every game-mode world has: Prefabs/World/Game/PerceptionManager.et (without it the game crashes in the first frame after the first AI spawns), Prefabs/Systems/Radio/RadioManager.et (radios), Prefabs/MP/ScriptedChatEntity.et (text chat) and Prefabs/World/Game/ItemPreviewManager.et (inventory previews) | manual | yes |
| Place RZ_GameMode.et in the world (one per world) | does it | yes |
| Keep RZ_GameMode.et's spawn logic on SCR_MenuSpawnLogic with Forced faction SURV: our spawn screen (the background picker) opens through its deploy-menu handler, with no faction or group step. GameMode_Plain's SCR_AutoSpawnLogic picks a random faction from all of them and opens no screen | manual | yes |
| Save the world, and point a scenario (mission header) at it to play it | manual | yes |
| Optional: place marker entities (RZ_SettlementMarkerEntity and the others) to override automatic picks | manual | — |
<!-- /manifest:world -->

**The repo's worlds.** The main world is **Everon**: `Worlds/RZ_Everon/RZ_Everon.ent`
(`{4A535A0DE93773EA}`), a sub-scene of Eden, played by `Missions/RZ_Everon.conf`
(`{933D8C91F1C91F38}`). It boots on the dedicated server with no script exceptions, 96 settlements,
14 holdings and, in the cloud's boot of Eden (2026-11-12), 6 road checkpoints, all at roads that end
at the shore ([The Perimeter](DESIGN.md#the-perimeter)). **Arland** stays as the small test world:
`Worlds/RZ_Arland/RZ_Arland.ent`, a sub-scene of Arland. Each holds eight entities: three repo
prefabs and five vanilla ones.

| Entity | Prefab |
|---|---|
| The game mode | `Prefabs/ReforgerZ/Core/RZ_GameMode.et` |
| The FactionManager | `Prefabs/MP/Managers/Factions/RZ_FactionManager.et` ([section 3](#3-factions-and-loadouts)) |
| The LoadoutManager | `Prefabs/MP/Managers/Loadouts/RZ_LoadoutManager.et` ([section 3](#3-factions-and-loadouts)) |
| The AI world | Everon: vanilla `SCR_AIWorld_Eden`, with `Navmesh_GM_Eden_Soldier.conf` and `GM_Eden_vehicles.nmn`. Arland: vanilla `Prefabs/AI/SCR_AIWorld_Arland.et`, with its navmesh files set by hand to Arland's Game Master ones (`worlds/GameMaster/Navmeshes/GM_Arland.nmn` and `GM_Arland_vehicles.nmn`) |
| The perception manager | vanilla `Prefabs/World/Game/PerceptionManager.et` |
| The radio manager | vanilla `Prefabs/Systems/Radio/RadioManager.et` |
| The chat | vanilla `Prefabs/MP/ScriptedChatEntity.et` |
| The item preview manager | vanilla `Prefabs/World/Game/ItemPreviewManager.et` |

Without an AI world no AI or infected spawn, and there's no road network
([the log lines](#7-log-lines-to-check)). Without the perception manager the game crashes in the
first frame after the first AI spawns: an access violation, with no script error in the log. Every
vanilla game-mode world on Arland places a perception manager, and most also place the radio
manager and the chat; the base map, `Arland.ent`, has none of them. The world has no music
manager (vanilla's are `Prefabs/Sounds/Music/MusicManager_*.et`), so there's no music.
Both faction and loadout managers inherit the vanilla ones and already hold
everything in section 3. Each world's scenario is an `SCR_MissionHeader` that points at it, for 32
players: `Missions/RZ_Everon.conf` (`{933D8C91F1C91F38}`) and `Missions/RZ_Arland.conf`
(`{3487FFB701729EC7}`). How to play it is in
[PC_SETUP.md](PC_SETUP.md#6-play-testing).

**On another map:** in Workbench 1.9, **File → Load World** and open the map's world; then **File →
New World → Sub-scene (of current world)**, and **File → Save World** into the project. Drag in the
same three prefabs and the map's AI world (an `SCR_AIWorld` prefab, with the map's navmesh files
set), save, and give it a scenario: an `SCR_MissionHeader` config like `Missions/RZ_Arland.conf`,
with **World** set to the new world. Then check the log lines in
[section 7](#7-log-lines-to-check). If the map has no name labels and few buildings, place a few
`RZ_SettlementMarkerEntity`s. Check the Perimeter rectangle, checkpoints, safehouses, territory,
power and radio station, and whether "Search" spots appear in houses.

## 3. Factions and loadouts

Both live in prefabs, placed in the repo's world ([The world](#the-world)):

| Prefab | Inherits | Holds |
|---|---|---|
| `Prefabs/MP/Managers/Factions/RZ_FactionManager.et` | vanilla `FactionManager_Base.et` | the four factions below, the 15 hidden variants and `SURV_OUTLAW` |
| `Prefabs/MP/Managers/Loadouts/RZ_LoadoutManager.et` | vanilla `LoadoutManager_Base.et` | the 13 loadouts, on `SURV` |

<!-- manifest:factions -->
| Key | In game | Used for |
|---|---|---|
| `SURV` | The Haven | the player faction; its loadout list holds the 13 backgrounds; Haven AI |
| `MIL` | Task Force Bastion | Bastion AI: guards, crews, hunter squads |
| `BAND` | The Red Hand | Red Hand AI: mission groups, guards, crews, hunter squads |
| `INST` | Halcyon | Halcyon AI: guards, crews |
| (the zombie mod's) | The infected | the zombie mod's own faction (Bacon Zombies): hostile to all four |
<!-- /manifest:factions -->

Each faction is built on a vanilla faction config, which gives it its flag and identity: `SURV`
and `INST` on `CIV.conf`, `MIL` on `US.conf`, `BAND` on `FIA.conf`. Only `SURV` is playable. The
colours are the faction accents in [UI/LAYOUTS.md](../UI/LAYOUTS.md). The infected's faction,
`BACON_622120A5448725E3_FACTION`, comes with the base: Bacon Zombies overrides
`FactionManager_Base.et` to add it, hostile to all. Vanilla factions are playable unless told
otherwise, so `RZ_FactionManager.et` sets it not playable; otherwise players could join the infected.

**Characters must name a faction that exists.** A character prefab names its faction twice: its
`SCR_CharacterFactionAffiliationComponent` ("faction affiliation") and its editor UI info's
`m_sFaction`. Vanilla characters say `CIV`, `US` or `FIA`, which don't exist here, and the deploy
menu fails on a loadout whose character names a missing faction (a script exception in
`SCR_LoadoutButton.SetLoadout`). Every ReforgerZ character sets both to its own faction (the
loadouts, wanderers and civilians to `SURV`); do the same for any character you add.

**Relations.** AI shoots individual players through the hidden variants below, so the base
relations are the "nobody wants you dead" case. They're set in each faction's friendly list, both
ways; if you change one, change both:

<!-- manifest:relations -->
| | SURV | MIL | BAND | INST |
|---|---|---|---|---|
| SURV | friendly | friendly | friendly | friendly |
| MIL | friendly |  | hostile | friendly |
| BAND | friendly | hostile |  | hostile |
| INST | friendly | friendly | hostile |  |
<!-- /manifest:relations -->

`SURV`–`BAND` friendly is what makes the Red Hand neutral by default.

**The hidden variant factions.** These let AI shoot one player and not another
([REPUTATION.md](REPUTATION.md#what-makes-ai-shoot-you)). `SURV_H1` to `SURV_H15` are copies of
`SURV` with only the key changed: built on `CIV.conf` like it, with the same name, colour, flag and
identity, so they look like the Haven, but not playable and with no loadouts. The number is a
bitmask of who shoots that player: 1 the Haven, 2 Bastion, 4 the Red Hand, 8 Halcyon, added up.
Their friendly lists are `SURV`'s, but they don't matter: the modded faction check decides from
the key. `SURV_OUTLAW`, only for the Outlaw fallback (**Hostility > Mode**), is the same but with
real relations: friendly with nothing, and in nobody's friendly list. With all of them there, the
log says `Hostility: bitmask mode, all 15 variant factions found.`

Keep the variants in the prefab. The plugin adds missing ones to a placed FactionManager as bare
factions, without a base config, so they have no flag or identity; that's why they were moved
into `RZ_FactionManager.et`. With it placed, the plugin finds them all and adds nothing.

If the player faction key isn't `SURV`, the variants are `<key>_H1`..`<key>_H15` and
`<key>_OUTLAW`. Missing variants only turn the feature off for the players who'd need them.

**Loadouts.** `RZ_LoadoutManager.et` has one loadout per background on `SURV`, named exactly as
below, each pointing at `Prefabs/ReforgerZ/Loadouts/RZ_Loadout_<Name>.et`. Those are characters
affiliated with `SURV`, with `RZ_TreatOtherAction` (so other players can treat you), built on
vanilla civilians, except Soldier (`Character_US_Unarmed`) and Raider (an armed FIA civilian). They
wear and carry what their base character does: the suggested gear below is still to add by hand.
The server won't assign or spawn a locked one (**UI > Lock loadouts**; with it off, a locked pick
keeps its gear but plays as Unemployed). The log warns `No background matches loadout '...'` for a
typo. The perks and flaws are in
[DESIGN.md](DESIGN.md#backgrounds-and-personal-goals).

<!-- manifest:loadouts -->
| Loadout name | Prefab | Unlock | Suggested gear |
|---|---|---|---|
| `Nurse` | `RZ_Loadout_Nurse.et` | — | medical bag, bandages, morphine, saline; no weapon |
| `Mechanic` | `RZ_Loadout_Mechanic.et` | — | wrench, jerrycan; work clothes |
| `Hunter` | `RZ_Loadout_Hunter.et` | — | hunting rifle, 5 rounds, hunting jacket |
| `Police Officer` | `RZ_Loadout_PoliceOfficer.et` | — | pistol, one magazine, police jacket |
| `Ex-Con` | `RZ_Loadout_ExCon.et` | — | knife; plain clothes |
| `Lab Technician` | `RZ_Loadout_LabTechnician.et` | — | lab coat, gloves; no weapon |
| `Radio Operator` | `RZ_Loadout_RadioOperator.et` | — | handheld radio, batteries |
| `Unemployed` | `RZ_Loadout_Unemployed.et` | — | clothes only |
| `Soldier` | `RZ_Loadout_Soldier.et` | Legacy | service rifle, two magazines, body armour, helmet |
| `Doctor` | `RZ_Loadout_Doctor.et` | Legacy | medical bag, morphine, bandages, gloves; no weapon |
| `Raider` | `RZ_Loadout_Raider.et` | Legacy | pistol or sawn-off, knife, red armband or jacket |
| `Elder` | `RZ_Loadout_Elder.et` | Legacy | walking clothes, a little food and water |
| `Drifter` | `RZ_Loadout_Drifter.et` | Legacy | nothing |
<!-- /manifest:loadouts -->

## 4. The prefab manifest

<!-- manifest:counts -->
The code has **286 prefab fields**: 40 on `RZ_GameModeComponent`, 237 in its settings objects, and 9 in list entries and actions (shop item, requisition item, recipe output, the loot table lists). The manifest covers them in **290 entries**: 178 prefabs the plugin creates, 33 slots filled with prefabs you already have (zombie mods, vanilla cars, loot) and 79 list entries set by hand; plus the game mode prefab, 13 loadout characters, 7 additions to existing prefabs and 47 settings objects. 22 of them, counting the game mode, make up the minimum playable subset.
<!-- /manifest:counts -->

Each manifest entry fills one or more fields. **Need:** *minimum* is the
[minimum playable subset](#2-the-minimum-playable-subset); *optional* turns its feature off (or
falls back) when empty. "(existing prefab)" means you
fill the slot with a prefab you already have. All new prefabs go under `Prefabs/ReforgerZ/`. The
tables are generated from the manifest ([1.4](#14-changing-the-manifest)): change the manifest,
not the tables.

**Bases.** The manifest names a base (`basePrefab`) for 152 entries and all 13 loadouts: real
ResourceNames from vanilla 1.9, Bacon Zombies and Special Zombies, picked by reading the game's
resource database. The tables' **Suggested base** is what a prefab should be; `basePrefab` is what
it's built on now, and many are [stand-ins](#stand-in-bases). Two more entries are found by name
(`baseFind`: the spawn point and the move waypoint), the 9 entries set by hand (the hand-set lists
and the UI files) need none, and seven have no base:

| Entry | Why |
|---|---|
| `RZ_Drone.et`, `RZ_WarningShot.et`, Interactions > Crafting > Spear | Vanilla has nothing like them (no drone, no melee weapon) |
| `RZ_InteractionSpot.et`, `RZ_DoorBlocker.et` | To make from an empty entity (the plugin's option) with collision added by hand; not made yet |
| Interactions > General > Dirty water, Clean water | They're the survival mod's water bottles, and Metabolism isn't a dependency yet ([MOD_STACK.md](MOD_STACK.md)) |

<!-- manifest:prefabs -->
#### Core and cure

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_SpawnPoint.et` | a vanilla spawn point (SCR_SpawnPoint), e.g. SpawnPoint_Base.et or SpawnPoint_US.et; confirm the name in Workbench | — | World > Spawn point prefab | **minimum** | spawn points in towns, safehouse spawns |
| `RZ_Sample.et` | a small item (a vial or sample tube) | `RZ_SampleItemComponent` (Value 1) | Cure > Sample | **minimum** | samples from specials |
| `RZ_Antidote.et` | vanilla morphine or saline item (keeps its consumable component) | `RZ_AntidoteItemComponent`; `RZ_InjectAntidoteAction` | Cure > Antidote | **minimum** | antidotes; optionally set the consumable effect to RZ_ConsumableAntidote by hand |
| `RZ_Lab.et` | a field hospital tent or lab container prop | `RZ_LabComponent`; `RZ_DepositSamplesAction`, `RZ_LabStatusAction` | Cure > Lab prefab | **minimum** | the cure (spawned in a town if none is placed); a second copy is Mobile Lab's field annex for 3 hours. Manual: two empty child entities named RZ_CleanRoom (where the final stand is fought, 10 m round it) and RZ_LoadingBay (Take the Lab's hold); without them the finale picks land points near the lab |
| `RZ_RewardCrate.et` | a vanilla supply box, filled with loot by hand | — | Missions > Reward crate | **minimum** | mission rewards. Fill its inventory with loot in the prefab editor |

#### Treatments

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_Stabiliser.et` | an autoinjector item | `RZ_MedItemComponent` (Kind STABILISER); `RZ_UseMedAction` | Survival > Infection > Stabiliser | optional | Halcyon's quartermaster (a Known shop action; sold from the lab's stock), rare in pharmacies. Optionally set its consumable effect to RZ_ConsumableMed with the same kind |
| `RZ_Antibiotics_Expired.et` | a pill strip item | `RZ_MedItemComponent` (Kind ANTIBIOTICS); `RZ_UseMedAction` | Survival > Infection > Antibiotics | optional | searches; every safehouse's "Ask for something for the fever". Optionally set its consumable effect to RZ_ConsumableMed with the same kind |
| `RZ_VetAntiviral.et` | a vial item | `RZ_MedItemComponent` (Kind VET_ANTIVIRAL); `RZ_UseMedAction` | Survival > Infection > Vet antiviral | optional | farms and vet clinics. Optionally set its consumable effect to RZ_ConsumableMed with the same kind |
| `RZ_CutDose.et` | a dirty syringe item | `RZ_MedItemComponent` (Kind CUT_DOSE); `RZ_UseMedAction` | Survival > Infection > Cut dose | optional | the Red Hand quartermaster: a Known shop action, cheap (10-15 Favor). Optionally set its consumable effect to RZ_ConsumableMed with the same kind |
| `RZ_FeverSuppressant.et` | a pill bottle item | `RZ_MedItemComponent` (Kind SUPPRESSANT); `RZ_UseMedAction` | Survival > Infection > Suppressant | optional | pharmacies, some shops. Optionally set its consumable effect to RZ_ConsumableMed with the same kind |
| `RZ_HomeRemedy.et` | a brown bottle item (not named vodka or herb) | `RZ_MedItemComponent` (Kind REMEDY); `RZ_UseMedAction` | Survival > Infection > Remedy | optional | brewed on a stove from alcohol and herbs. Optionally set its consumable effect to RZ_ConsumableMed with the same kind |

#### Puzzles

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_PuzzleObject.et` | a crate, sign or post prop | the generic set | Puzzles > Puzzle object | **minimum** | every puzzle and interaction fallback; missions relabel the actions |
| `RZ_Civilian.et` | a civilian character without AI, or a seated figure prop | the generic set | Puzzles > Civilian | **minimum** | puzzles, unarmed missions, wanderers, leaders and occupants to talk to, the one-choice jobs' stand-in person |
| `RZ_Note.et` | a paper or notebook prop | the clue set | Puzzles > Note | **minimum** | every puzzle mission, scenario clues |
| `RZ_Keypad.et` | a keypad or wall panel prop | `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`; 12 × `RZ_KeypadAction` (Key 0-11), `RZ_ReadClueAction` | Puzzles > Keypad | optional | The Last Experiment (keypad mode), Locked Bunker, the store in The Three Weeks questline (without a terminal); keys 0-9, 10 Clear, 11 Enter |
| `RZ_Terminal.et` | a computer prop | `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`; 12 × `RZ_KeypadAction` (Key 0-11), `RZ_ReadClueAction` | Puzzles > Terminal | optional | The Last Experiment, the store in The Three Weeks questline (both fall back to the keypad); keys 0-9, 10 Clear, 11 Enter |
| `RZ_PuzzleBody.et` | a body prop | the clue set | Puzzles > Body | optional | Paces, Patient Zero's Ledger, Poisoned Well, Last Experiment, The Witness, The Deserter's Map (falls back to the note) |
| `RZ_Well.et` | a well prop | the generic set | Puzzles > Well | optional | look only (falls back to the puzzle object): The Poisoned Well, and The Well Story where the town has no real well |
| `RZ_Bell.et` | a bell prop | the generic set | Puzzles > Bell | optional | look only (falls back to the puzzle object) |
| `RZ_FuseBox.et` | an electrical box prop | the generic set | Puzzles > Fuse box | optional | look only (falls back to the puzzle object) |
| `RZ_RadioMast.et` | an antenna prop | the generic set | Puzzles > Radio mast | optional | look only (falls back to the puzzle object) |
| `RZ_Safe.et` | a safe prop | the generic set | Puzzles > Safe | optional | look only (falls back to the puzzle object) |
| `RZ_ScaleCrate.et` | a crate with an inventory storage | the generic set | Puzzles > Scale crate | optional | The Weighing Vault |
| `RZ_Landmark.et` | a wayside cross or painted mast prop | — | Puzzles > Landmark | optional | towns with no church, tower or well; set Landmark name to match |
| `RZ_MorseLamp.et` | a lamp prop with a light | `RZ_BlinkLightComponent` | Puzzles > Morse light | optional | Lights in the Window (without the component it's spawned per flash) |

#### Shared

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_SeasonMark.et` | a small wayside cross, a cairn or flowers on a post | — | Shared > Season mark prefab | optional | look only: the town log's season marks on the verge (without it they're text only, an approach line) |

#### Epilogue

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_EpilogueBoard.et` | a free-standing board or sign on posts (a notice board, a road sign); confirm the name in Workbench | — | Director > Epilogue > Board prefab | optional | look only: the epilogue's marks (the stand board, the Names board, the price board, notices, posters, graves) on the verge; without it they're text only, an approach line |
| `RZ_EpilogueCandle.et` | a candle or a jar lantern on the ground; confirm the name in Workbench | — | Director > Epilogue > Candle prefab | optional | look only: a town taken back after the cure, the candles on a safehouse step (else the board, else text) |
| `RZ_EpilogueTable.et` | a folding or trestle table (or a crate) with the generic puzzle object's components (RZ_PuzzleObjectComponent, RZ_ClueComponent, an RplComponent) and its read and four choice actions | `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`; `RZ_ReadClueAction`, 4 × `RZ_PuzzleChoiceAction` (Option 0-3) | Director > Epilogue > Table prefab | optional | the tables, boards and crates players use at the epilogue's places (the dose, the Names board, the nail, the book); without it Puzzles > Object stands in |
| `RZ_EpilogueSheet.et` | a bedsheet or tarp hung between two posts; confirm the name in Workbench | — | Director > Epilogue > Sheet on posts | optional | look only: a rescue town's mark (WE GOT THEM OUT) (else the board, else text) |
| `RZ_EpilogueStencil.et` | a painted or stencilled board on a post; confirm the name in Workbench | — | Director > Epilogue > Stencil board | optional | look only: Bastion's marks: a shelled town, the roster board (else the board, else text) |
| `RZ_EpiloguePriceBoard.et` | a chalk board on legs; confirm the name in Workbench | — | Director > Epilogue > Price board | optional | look only: the price board at Rudi's crossroads (else the board, else text) |
| `RZ_EpilogueNotice.et` | a paper notice nailed to a post; confirm the name in Workbench | — | Director > Epilogue > Notice on a post | optional | look only: a lost post's notice, the house rule's notice, the transformer's notice, a faction's season (else the board, else text) |
| `RZ_EpilogueGrave.et` | a wooden cross or grave marker; confirm the name in Workbench | — | Director > Epilogue > Grave marker | optional | look only: graves and the leaders' graves (else the board, else text) |
| `RZ_EpiloguePoster.et` | a poster or paper on a board; confirm the name in Workbench | — | Director > Epilogue > Wanted poster | optional | look only: the wanted posters at holdings (else the board, else text) |
| `RZ_EpilogueTrestle.et` | a trestle or folding table (look only); confirm the name in Workbench | — | Director > Epilogue > Trestle table | optional | look only: the queue's table mark (else the board, else text) |
| `RZ_EpilogueCrate.et` | a wooden crate; confirm the name in Workbench | — | Director > Epilogue > Crate | optional | look only: the open crate, a side story with a crate (else the board, else text) |
| `RZ_EpilogueBunting.et` | a string of flags or cloth; confirm the name in Workbench | — | Director > Epilogue > Bunting | optional | look only: a safehouse that grew this season (else the board, else text) |
| `RZ_EpilogueRibbon.et` | a ribbon or cloth strip on a post; confirm the name in Workbench | — | Director > Epilogue > Ribbon | optional | look only: a name brought home after the cure (else the board, else text) |
| `RZ_EpilogueWreck.et` | a burnt-out truck wreck (static); confirm the name in Workbench | — | Director > Epilogue > Truck wreck | optional | look only: the roads' last four lost trucks (else the board, else text) |
| `RZ_EpilogueTape.et` | barrier tape between two posts; confirm the name in Workbench | — | Director > Epilogue > Tape between posts | optional | look only: a Quarantine Line town's mark (else the board, else text) |
| `RZ_EpilogueHandcart.et` | a handcart or wheelbarrow; confirm the name in Workbench | — | Director > Epilogue > Handcart | optional | look only: the farm's basket, the queue's bags (else the board, else text) |
| `RZ_EpilogueNamesBoard.et` | a large notice board on two legs; confirm the name in Workbench | — | Director > Epilogue > Names board | optional | look only: the Names board at the bend on the north road (else the board, else text) |

#### Traces

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| (existing prefab) | a decal material (`RZ_DoorMarks.emat` in `Prefabs/ReforgerZ/Traces/`) on a 4 × 4 atlas (`RZ_DoorMarks_atlas.edds`): X, slash, post code, C, ATT, CLEARED, bear, patched bear, candle, hatching, ring and dot, arrow, drop, red hand, tally, beak, in that order | — | World > Traces > Kit > Door marks atlas | optional | door marks drawn as decals on each client (docs/TRACES.md, Doors you can see); empty = boards, else text. Manual: drawn in the UI's art direction (hand-made survivor marks); the fields stay text. Settle it with the Traces spike first (VERIFY) |
| (existing prefab) | a second decal material (`RZ_DoorMarks_overlays.emat`) on a 4 × 4 atlas: ring, tick, strike, PAID, scratches outside, scratches inside, smears, thumbprint, grey overpaint | — | World > Traces > Kit > Door marks overlays | optional | the overlays on the door decals (rings, ticks, strikes, the infected's marks, grey paint); empty = not drawn. Manual |
| (existing prefab) | a plywood offcut with the orange X painted on it (`RZ_Trace_DoorBoard_Bastion.et`); nothing with physics, storage or actions | — | World > Traces > Kit > Door board, Bastion | optional | the board path: leaned beside a door whose newest mark is Bastion's. Only when decals fail the spike: never both |
| (existing prefab) | a cardboard sheet with the bear in white (`RZ_Trace_DoorBoard_Haven.et`) | — | World > Traces > Kit > Door board, the Haven | optional | the board path: the Haven's marks. Only when decals fail the spike |
| (existing prefab) | a plank with a red hand (`RZ_Trace_DoorBoard_RedHand.et`) | — | World > Traces > Kit > Door board, the Red Hand | optional | the board path: the Red Hand's marks. Only when decals fail the spike |
| (existing prefab) | a square of white primer with the beak stencilled (`RZ_Trace_DoorBoard_Halcyon.et`) | — | World > Traces > Kit > Door board, Halcyon | optional | the board path: Halcyon's marks. Only when decals fail the spike |
| (existing prefab) | a sheet of paper on the door (`RZ_Trace_DoorBoard_Residents.et`, from `Notice_A4_Vertical_V1.et`) | — | World > Traces > Kit > Door board, residents | optional | the board path: residents' words and the infected's scratches. Only when decals fail the spike |
| (existing prefab) | two fence palings and a mound (`RZ_Trace_Grave.et`): an empty root with an RplComponent, vanilla props as children, no physics | — | World > Traces > Kit > Grave | optional | P3, a grave where a notable character died; without it the grave is its approach line. Check the log for no Dressing: removed line after placing it once |
| (existing prefab) | a black patch decal or mesh, a burnt pallet, a burnt tyre and a post (`RZ_Trace_Pyre.et`) | — | World > Traces > Kit > Pyre | optional | P2, ash where a job burned the bodies. As the grave |
| (existing prefab) | 3-5 jam jars with candles (`RZ_Trace_CandleJars.et`), the lit twin with its light on LIT_ON_SPAWN and no switch action | — | World > Traces > Kit > Candle jars | optional | P3b, the memorial candles on every safehouse's step. As the grave; the lit light is a VERIFY of the Traces spike |
| (existing prefab) | a sheet on two posts (`RZ_SeasonMark_Sheet.et`) | — | World > Traces > Kit > Season mark, rescue | optional | P5, the town log's season mark for people brought in alive. Empty = Shared > Season mark prefab |
| (existing prefab) | a board on a post (`RZ_SeasonMark_Board.et`) | — | World > Traces > Kit > Season mark, burned and other jobs | optional | P5, the season mark for a town burned or a job. Empty = Shared > Season mark prefab |
| (existing prefab) | a stencilled board (`RZ_SeasonMark_Stencil.et`) | — | World > Traces > Kit > Season mark, taken | optional | P5, the season mark for an outpost taken. Empty = Shared > Season mark prefab |
| (existing prefab) | tape between two posts (`RZ_SeasonMark_Tape.et`) | — | World > Traces > Kit > Season mark, sealed | optional | P5, the season mark's look for a sealed place (no town log kind uses it yet). Empty = Shared > Season mark prefab |
| (existing prefab) | flowers tied to a post (`RZ_SeasonMark_Flowers.et`) | — | World > Traces > Kit > Season mark, emptied | optional | P5, the season mark for a safehouse emptied. Empty = Shared > Season mark prefab |
| (existing prefab) | a static civilian car wreck, a hatchback, doors open, a suitcase on the roof; no vehicle component (`RZ_Trace_QueueCar_A.et`) | — | World > Traces > Kit > Column car A | optional | F1, the column's tail: 4-8 cars nose to tail on the verge of the road in from the north barrier; empty = text only. An empty root with an RplComponent and vanilla meshes as children, nothing with physics. Check the log for no Dressing: removed line and that AI aren't trapped on the verge (VERIFY 7) |
| (existing prefab) | a static saloon wreck, doors open (`RZ_Trace_QueueCar_B.et`) | — | World > Traces > Kit > Column car B | optional | F1; empty = car A again. As car A |
| (existing prefab) | a static estate wreck, a coat on the grass (`RZ_Trace_QueueCar_C.et`) | — | World > Traces > Kit > Column car C | optional | F1; empty = car A again. As car A |
| (existing prefab) | three suitcases, a holdall, a folded coat (`RZ_Trace_Suitcases.et`) | — | World > Traces > Kit > Suitcases | optional | F2, where they turned back, 360 m in from the north barrier; empty = text only. As car A |
| (existing prefab) | stacked camp beds, folded blankets, a painted board GYM OPEN (`RZ_Trace_GymDoor.et`) | — | World > Traces > Kit > Gym door | optional | F3, the Day 2 gym; empty = text only. As car A |
| (existing prefab) | a static bus wreck (`RZ_Trace_Bus.et`) | — | World > Traces > Kit > Bus | optional | F3, the bus in the gym's car park; empty = no bus. As car A |
| (existing prefab) | a painted board, a kitchen chair, a folded blanket (`RZ_Trace_ChurchDoor.et`) | — | World > Traces > Kit > Church door | optional | F4, the Day 4 church; the jars are Kit > Candle jars (S8); empty = text only. As car A |
| (existing prefab) | a sandbag half ring (`Sandbag_Barricade_USSR_04.et` or loose sandbags), a barrier pole on the ground, a camp chair, empty tins, a field radio mesh on `CrateWooden_01_plain.et` (`RZ_Trace_Roadblock_D1.et`) | — | World > Traces > Kit > Day 1 roadblock | optional | F5, the cordon's Day 1 roadblock; empty = text only. As car A; the crate with no storage |
| (existing prefab) | a board on two posts with five `Notice_A4_Vertical_V1.et` sheets overlapping (`RZ_Trace_NoticeWall.et`) | — | World > Traces > Kit > Notice wall | optional | F6, the square's notices in one town in five nobody lives in; empty = text only. As car A |
| (existing prefab) | a garage door or shutter mesh on two posts (`RZ_Trace_Shutter.et`) | — | World > Traces > Kit > Rudi's shutter | optional | F7, Rudi's shutter by the crossroads; empty = text only. As car A |
| (existing prefab) | a groundsheet, a ration tin, a fence post (`RZ_Trace_OP.et`) | — | World > Traces > Kit > The hill | optional | F8, the hill over the north road; empty = text only. As car A |
| (existing prefab) | `PicnicTable_01` or a garden table, four chairs, plates, a jug, a radio mesh (`RZ_Trace_LastMeal.et`) | — | World > Traces > Kit > The last meal | optional | V1, a town gone dark; empty = text only. As car A |
| (existing prefab) | planks for windows (static, not the barricade item), one hanging off a frame, casings (`RZ_Trace_HeldOut.et`) | — | World > Traces > Kit > Held out | optional | V2, a house that held out; empty = text only. As car A; never across a doorway |
| (existing prefab) | a kitchen chair, a blanket, a jam jar (`RZ_Trace_EdgeChair.et`) | — | World > Traces > Kit > The chair at the edge | optional | V3, a walk-out; empty = text only. As car A |
| (existing prefab) | sheets pinned to a board, a tin on `CrateWooden_01_plain.et` (`RZ_Trace_IOUs.et`) | — | World > Traces > Kit > The honest shop | optional | V4, IOUs on a shop door; empty = text only. As car A; the crate with no storage |
| (existing prefab) | a closed sample case mesh, a gas mask with a long filter, a bicycle on its side (`RZ_Trace_DroppedRun.et`) | — | World > Traces > Kit > The dropped run | optional | V5, a Halcyon run lost on the road; empty = text only. As car A; no storage, no item |
| (existing prefab) | six plastic chairs in a row, a sign on an easel (`RZ_Trace_WaitingRoom.et`) | — | World > Traces > Kit > The waiting room | optional | V6, outside one clinic in a town nobody holds; empty = text only. As car A |
| (existing prefab) | the roster board from `RZ_Dress_RosterBoard`, a cold `BarrelMetal_01_rust.et`, a few sandbags (`RZ_Trace_FallenPost.et`) | — | World > Traces > Kit > The fallen post | optional | V7, the roll at a lost Bastion outpost; empty = text only. As car A |
| (existing prefab) | buckets and plastic water bottles, meshes, never jerrycans (`RZ_Trace_WaterQueue.et`) | — | World > Traces > Kit > The water queue | optional | V8, a queue at a short house's pump; empty = text only. As car A |
| (existing prefab) | an ash pile and a scorch decal for the wall (`RZ_Trace_Scorch.et`) | — | World > Traces > Kit > Scorch | optional | L2, a building burned; empty = text only. As car A |
| (existing prefab) | a static burnt truck wreck, two empty crates (`RZ_Trace_TruckWreck_Bastion.et`) | — | World > Traces > Kit > Truck wreck, Bastion | optional | L5, a lost Bastion truck on the verge; empty = text only. As car A; never a fixable car |
| (existing prefab) | as the Bastion wreck, the crews' colours (`RZ_Trace_TruckWreck_RedHand.et`) | — | World > Traces > Kit > Truck wreck, the Red Hand | optional | L5; empty = text only. As car A |
| (existing prefab) | as the Bastion wreck, a civilian lorry (`RZ_Trace_TruckWreck_Haven.et`) | — | World > Traces > Kit > Truck wreck, the Haven | optional | L5; empty = text only. As car A |
| (existing prefab) | as the Bastion wreck, a white van (`RZ_Trace_TruckWreck_Halcyon.et`) | — | World > Traces > Kit > Truck wreck, Halcyon | optional | L5; empty = text only. As car A |
| (existing prefab) | sandbags in a half ring, a CHECKPOINT. HALT sign on its side (`RZ_Trace_RoadPostRelic.et`) | — | World > Traces > Kit > Road post relic | optional | L6, where a Bastion road post stood; empty = text only. As car A |
| (existing prefab) | one fence-paling cross (`RZ_Trace_Crosses.et`) | — | World > Traces > Kit > Cross | optional | N1, the crosses outside the warning zone, placed up to 7 times in a row; empty = text only. As car A |
| (existing prefab) | two or three paper sheets on the ground, a decal or a flat mesh (`RZ_Trace_Sheets.et`) | — | World > Traces > Kit > Day 1 sheets | optional | N2, the Day 1 sheets near every barrier; empty = text only. As car A |
| (existing prefab) | `LampKerosene_01_green.et`'s lit twin, its light on LIT_ON_SPAWN, no switch action (`RZ_Trace_Lantern_Lit.et`) | — | World > Traces > Kit > Lantern, lit | optional | S1, a lantern at the lived-in places nearest players after dark; empty = no lanterns. VERIFY 4 of the Traces spike: it lights on clients that stream it in |
| `RZ_Sound_RadioHiss.et` | a new GenericEntity prefab with an RplComponent, a static radio mesh (no radio actions) and a SoundComponent whose project (.acp) has a looped static hiss on the event SOUND_LOOP | `RZ_AmbientLoopComponent` (Event SOUND_LOOP) | World > Traces > Kit > Radio hiss | optional | S3, the hiss at the last meal and the Day 1 roadblock, 2 at most; empty = the broadcast's line only. Manual: the sound itself. VERIFY 6: the loop plays until the entity goes |
| `RZ_Sound_GeneratorHum.et` | a new GenericEntity prefab with an RplComponent and a SoundComponent whose project (.acp) has a looped generator hum on the event SOUND_LOOP; no mesh needed | `RZ_AmbientLoopComponent` (Event SOUND_LOOP) | World > Traces > Kit > Generator hum | optional | S5, at the lab and Halcyon's outposts; empty = silent. Manual: the sound itself. As the radio |
| `RZ_Sound_GeneratorHumNight.et` | a new GenericEntity prefab with an RplComponent and a SoundComponent whose project (.acp) has the generator hum, quieter, looped on the event SOUND_LOOP; no mesh needed | `RZ_AmbientLoopComponent` (Event SOUND_LOOP) | World > Traces > Kit > Generator hum, night | optional | S5 after dark, quieter; empty = the day's hum all night. Manual: the sound itself. As the generator hum |
| (existing prefab) | compositions built like the scenes' own (Prefabs/ReforgerZ/Traces/): one more piece for a scene (a suitcase, a chair, a bucket) and its faded twin, or a faded twin of a scene's own piece (section 4, Traces) | — | World > Traces > Kit > Pieces that age (each entry's Piece and Faded) | optional | scenes that drop a piece as they age, and boards with a fresh and a faded look; empty = each scene is its one composition |
| `RZ_Sound_Dogs.et` | a new GenericEntity prefab with an RplComponent and a SoundComponent whose project (.acp) has a few dogs barking that stop dead on the event SOUND_CALL | `RZ_CallSoundComponent` (Event SOUND_CALL) | World > Traces > Kit > Dogs | optional | S4, 5 minutes before a pack's hunt, at a town by its lair; empty = the Hunter's line only. Manual: the sound itself. As the tribes' call sound |
| `RZ_Sound_Bell.et` | a new GenericEntity prefab with an RplComponent and a SoundComponent whose project (.acp) has one stroke of a church bell (carrying 1.2 km) on the event SOUND_CALL | `RZ_CallSoundComponent` (Event SOUND_CALL) | World > Traces > Kit > Bell | optional | S6, at the Day 4 church at 18:30 after a Remembered name; empty = the line only. Manual: the sound itself. VERIFY 9: it carries to 1.2 km |
| `RZ_Sound_Loudspeaker.et` | a new GenericEntity prefab with an RplComponent and a SoundComponent whose project (.acp) has a loudspeaker's chime and static on the event SOUND_CALL | `RZ_CallSoundComponent` (Event SOUND_CALL) | World > Traces > Kit > Loudspeaker | optional | S7, at Bastion's holdings at 05:30 and 17:30; empty = the subtitle only. Manual: the sound itself. As the tribes' call sound |

#### Missions

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_MissionItem.et` | a small carryable item (a box or data drive) | `RZ_MissionItemComponent` | Missions > Mission item | **minimum** | every delivery and choice mission; tagged when it spawns |
| `RZ_Drop_Haven.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction SURVIVORS) | Missions > Haven drop point | **minimum** | the drops those missions name |
| `RZ_Drop_Bastion.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction MILITARY) | Missions > Bastion drop point | **minimum** | the drops those missions name |
| `RZ_Drop_RedHand.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction BANDITS) | Missions > Red Hand drop point | **minimum** | the drops those missions name; also Car Theft's chop shop |
| `RZ_Drop_Halcyon.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction INSTITUTE) | Missions > Halcyon drop point | **minimum** | the drops those missions name |
| `RZ_SurvivorGroup.et` | a civilian AI group on SURV | — | Missions > Survivor group | optional | Survivors Trapped, Training Day, Harvest Guard, The Bitten Friend, Trader Caravan, Protection Racket, the keeper's people in The Well Story; the fallback escort and bounty target |
| `RZ_Nest.et` | a destructible hive (Special Zombies' ZombieHive_Minor) or a prop with a damage manager | — | Missions > Nest | optional | Burn the Nest, Specimen from the Nest |
| `RZ_BanditDrop.et` | a crate, barrel or car wreck prop | `RZ_SellSamplesToBanditsAction` | Missions > Dead drop | optional | Dead Drop |
| `RZ_HeliWreck.et` | a helicopter wreck prop | the clue set | Missions > Helicopter wreck | optional | Crash Site, Two Bearings; the look of Rescue the Pilot's crash and of The Last Pilot's Day 3 wreck; the last helicopter's crash in The Last Pilot |
| `RZ_CorpsePile.et` | a body bag pile prop | — | Missions > Corpse pile | optional | look only: Burn the Bodies' piles and Memorial's bodies (both fall back to the puzzle object) |
| `RZ_RedHandGroup.et` | an AI group on BAND | — | Missions > Red Hand group | optional | Raid the Red Hand, Trader Caravan, raiders in Lab Defence and Convoy Escort, The Witness, the crews in Learning the Drones and The Pit's Debtor, the guards in Tied Down; in the questlines the thief's friends (The Book), the van's guards and the raided camp (Cargo) and the crew at the well (The Well Story), the toll and the raid (The Census), the crews (The Last Pilot); default Red Hand hunter squad |
| `RZ_FuelDepot.et` | fuel tanks with a damage manager | — | Missions > Fuel depot | optional | Arson; the look of The Last Pilot's Red Hand depot |
| `RZ_Relay.et` | a radio mast with a damage manager | — | Missions > Relay | optional | Sabotage the Relay |
| `RZ_Trader.et` | a character or vehicle (e.g. a Shop System trader) | — | Missions > Trader | optional | Wandering Trader |
| `RZ_Explosion.et` | a mortar or shell impact effect | — | Missions > Explosion | optional | Bastion Shelling; charges in Blow the Bridge, shells in Fire Mission, Gas Station Fire's blast, the burning helicopter in The Last Pilot |
| (existing prefab) | a dead man's things: a kit bag or rucksack with a jacket over it, as one composition (`RZ_ErrandProp_Effects.et`): an empty root with an RplComponent, vanilla props as children, no physics | — | Missions > Errands > Props > Effects | optional | His Effects, The Name on the Tape, Notebook 31 and its fallout, Into the Record: at the step's place while it's open; empty = words only. Shown only while a player is within 150 m (the marks' props pool); nothing is saved |
| (existing prefab) | a small wooden supply crate or a cardboard box of odds and ends (`RZ_ErrandProp_Crate.et`) | — | Missions > Errands > Props > Crate | optional | The Truck Didn't Come, Clean Glasses, Pack for the Queue: at the step's place while it's open; empty = words only. Shown only while a player is within 150 m (the marks' props pool); nothing is saved |
| (existing prefab) | a ledger or a few papers on a stool (`RZ_ErrandProp_Papers.et`; a vanilla notebook or clipboard prop will do) | — | Missions > Errands > Props > Papers | optional | Last Orders' order book at his seat: at the step's place while it's open; empty = words only. Shown only while a player is within 150 m (the marks' props pool); nothing is saved |
| (existing prefab) | a toolbox or a machine part on the ground (`RZ_ErrandProp_Parts.et`; a vanilla toolbox or battery prop will do) | — | Missions > Errands > Props > Parts | optional | Filter Parts' pump housing, Buy It Back's box at the wreck: at the step's place while it's open; empty = words only. Shown only while a player is within 150 m (the marks' props pool); nothing is saved |
| (existing prefab) | a vanilla ration or tin item (an existing food prefab whose name holds one of the food words, e.g. `ration`) | — | Missions > Errands > Props > Pay ration | optional | a Bastion post's pay in kind, one errand in three, in place of 3 Favor; empty = Favor only. Put in the player's pack (or at their feet), tagged as the post's goods: food delivery, the fence and every errand refuse it |
| (existing prefab) | a vanilla rifle magazine item (an existing prefab) | — | Missions > Errands > Props > Pay magazine | optional | a Red Hand camp's pay in kind, the same way; empty = Favor only. Tagged as the camp's goods, as the ration |
| `RZ_ErrandCarried.et` | a small item with an InventoryItemComponent that can't be equipped (a bundle, a satchel, a small box; the mission item prefab will do), plus RZ_MissionItemComponent | `RZ_MissionItemComponent` | Missions > Errands > Props > Carried | optional | a carried way's item: His Effects' effects, The Name on the Tag's ID; only the asker or a helper hands it in; empty = the mission item. Tagged with a negative mission id (no mission's drop takes it) and as the giver's goods |
| (existing prefab) | a vanilla wristwatch item (an existing prefab) | — | Missions > Errands > Props > Watch | optional | The Last Tin's watch, traded for two tins; empty = a common find. Put in the player's pack |

#### Contracts

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_EscortCivilians.et` | an AI group of 2-4 unarmed civilians on SURV | — | Missions > Contracts > Escort civilians | optional | Refugee Column, Bust Out, Desertion Route, the debtors in Learning the Drones and The Pit's Debtor, the brother walked home in The Other Two (falls back to the survivor group, then data) |
| `RZ_EscortScientist.et` | a one-man AI group on INST | — | Missions > Contracts > Escort scientist | optional | Escort the Scientist, Secure the Shipment, Lost Colleague walked in, the field lead in The Three Weeks and the courier in The Other Two |
| `RZ_EscortSoldier.et` | a one-man AI group on MIL | — | Missions > Contracts > Escort soldier | optional | Rescue the Pilot, Struck Off |
| `RZ_Cage.et` | a cage or box-trap prop | the generic set | Missions > Contracts > Cage trap | optional | Live Capture, Private Collection, The Three Weeks questline (falls back to the puzzle object) |
| `RZ_Bus.et` | a drivable bus or truck (keeps its damage manager) | — | Missions > Contracts > Bus | optional | Recover the Bus (falls back to the Haven truck, then car prefabs) |
| `RZ_BountyTarget.et` | an armed AI group or character on SURV (a loner, not a faction's man) | — | Missions > Contracts > Bounty target | optional | Collect the Bounty (falls back to the survivor group) |
| `RZ_BastionSquad.et` | an AI group on MIL | — | Missions > Contracts > Bastion squad | optional | the stockade guards in Bust Out, the arrest squad in Await Relief, Crash Site's recovery team (falls back to the Bastion hunters, then crew) |
| `RZ_DroneWreck.et` | a small wreck prop | — | Missions > Contracts > Crashed drone | optional | Medical Drone (falls back to the puzzle object) |
| `RZ_Smoke.et` | a coloured smoke effect | — | Missions > Contracts > Smoke marker | optional | Fire Mission (none: the message says it) |

#### World events

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_FogEffect.et` | a low mist or fog particle effect | — | Missions > Events > Fog effect | optional | Fog Bank's look (works without) |
| `RZ_Convicts.et` | an AI group on BAND, prison clothes, pistols and bats | — | Missions > Events > Convicts | optional | Prison Break (falls back to the Red Hand group) |
| `RZ_Sniper.et` | a one-man AI group on BAND with a scoped rifle | — | Missions > Events > Sniper | optional | The Sniper (falls back to the bounty target; without either, never appears) |
| `RZ_RivalGroup.et` | an armed AI group on SURV, no faction's colours | — | Missions > Events > Rival group | optional | Rival Survivors (falls back to the survivor group) |
| `RZ_TruckWreck.et` | a military truck wreck prop, not drivable | — | Missions > Events > Truck wreck | optional | Abandoned Convoy (two are placed) |
| `RZ_RemnantCamp.et` | a tent or small camp composition | — | Missions > Events > Camp | optional | Military Remnant Camp |
| `RZ_Hatch.et` | a steel hatch or bunker door in the ground | the generic set | Missions > Events > Hatch | optional | Locked Bunker when no locked building is found |
| (existing prefab) | zombie prefabs in suits and dresses from your zombie mod | — | Missions > Events > Wedding guests (list) | optional | Wedding Party (empty = common zombies) |
| `RZ_Bride.et` | a special infected in a wedding dress (from your zombie mod) | — | Missions > Events > Bride | optional | Wedding Party (falls back to a special) |
| `RZ_Jewellery.et` | a small item; keep "jewel" in its name | — | Missions > Events > Jewellery | optional | Wedding Party; the Trader wanderer takes one instead of Favor |

#### Zombies

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| (existing prefab) | Bacon Zombies' civilian group, and the repo's unarmed copies of its military groups (Prefabs/ReforgerZ/Zombies/: Bacon's own each have one infected with a pistol) | — | Zombies > Common prefabs (list) | **minimum** | the infected |
| (existing prefab) | Special Zombies, or Bacon heavy, exploding and ranged | — | Zombies > Special prefabs (list) | **minimum** | samples, the cure |
| (existing prefab) | a large Bacon group | — | Zombies > Horde prefabs (list) | **minimum** | hordes |
| (existing prefab) | vanilla AIWaypoint_Move.et | — | Zombies > Move waypoint | **minimum** | hordes toward noise, unarmed missions, hunter squads, supply trucks, favourite spots |
| (existing prefab) | Bacon Zombies' Group_Zombies_Base (an infected group with no members of its own) | — | Zombies > Empty group | optional | runtime groups: crawlers and loose ones made from character prefabs (without it they stand inactive) |
| (existing prefab) | Bacon Zombies' civilian characters (the slowest it has: none is slower than about 3.7 m/s) | — | Zombies > Crawlers > Prefabs (list) | optional | the towns' crawlers (empty: Zombies > Common) |
| (existing prefab) | Bacon Zombies' heavy (the fastest Bacon character, about 4.9 m/s; it climbs nothing in the behaviour layer) | — | Zombies > Tribes > Alpha prefabs (list) | optional | a tribe's leader, one-member runtime group (empty: Zombies > Special; both empty: the leader is data only) |
| (existing prefab) | Bacon Zombies' military characters (helmeted ones if the pack has them) | — | Zombies > Tribes > Guard prefabs (list) | optional | the leader's guard at the lair and on the trail (empty: Zombies > Special) |
| (existing prefab) | Bacon Zombies' civilian and military characters | — | Zombies > Tribes > Hunter prefabs (list) | optional | a tribe's grown members (empty: Zombies > Common) |
| (existing prefab) | Bacon Zombies' lightest civilian characters | — | Zombies > Tribes > Scout prefabs (list) | optional | scouts on the ridges (empty: Hunter prefabs) |
| (existing prefab) | Bacon Zombies' civilian characters | — | Zombies > Tribes > Young prefabs (list) | optional | the recently taken and the ones who ran (empty: Hunter prefabs) |
| (existing prefab) | Bacon Zombies' military characters | — | Zombies > Tribes > Military prefabs (list) | optional | a tribe that grew up by a Bastion post or a checkpoint, and the young who were soldiers (empty: no military look) |
| (existing prefab) | work clothes, if the mod stack has them | — | Zombies > Tribes > Work clothes prefabs (list) | optional | a tribe from a farm valley: a barn, a silo, a mill (empty: civilian) |
| (existing prefab) | dust-grey workers, if the mod stack has them | — | Zombies > Tribes > Dust-grey prefabs (list) | optional | a tribe from a quarry (empty: civilian) |
| (existing prefab) | outdoor clothes, if the mod stack has them | — | Zombies > Tribes > Outdoor prefabs (list) | optional | a tribe from the woods (empty: civilian) |
| (existing prefab) | an exploding variant (Bacon's exploder) | — | Zombies > Crawlers > Bloated prefabs (list) | optional | bloated crawlers in dark towns from Overrun |
| `RZ_TribeTag.et` | a small carryable item (a vanilla small prop that can be picked up); display name Halcyon tag | `RZ_TagAction` | Zombies > Tribes > Tag prefab | optional | Plant a Tag (Halcyon) and the Red Hand fence's stolen tag: Tag it on a pack's leader within 8 m. Set the action's duration to 5 s (hold it). Check in Workbench that the action shows on the item in the hand and on the ground |
| `RZ_CallBox.et` | a small radio or speaker item (a vanilla handheld radio); display name Call box | `RZ_CallBoxAction` | Zombies > Tribes > Call box prefab | optional | the call box Halcyon lends from 50% (Trusted, 2 a real day): 90 s of a recorded dusk call as a data noise. Put it down and use it; it's used up when it plays |
| `RZ_IdCard.et` | a small flat carryable item (a card or a wallet); display name ID card | — | Zombies > Tribes > ID card prefab | optional | dropped where a named young is put down (its text is told to whoever is near); cleared like loose items. Bring Them Home uses the mission item instead |
| `RZ_CallSound.et` | a new GenericEntity prefab with an RplComponent and a SoundComponent whose project (.acp) has a dusk call on the event SOUND_CALL (a long rising call, a few voices answering) | `RZ_CallSoundComponent` | Zombies > Tribes > Call sound prefab | optional | a pack's dusk call as a sound at its lair (a player within 1.5 km), the call box and the last call; removed after 40 s. Manual: the sound itself (record or license a call; nothing in the base game sounds right). Check in play that it replicates and plays once (VERIFY) |
| `RZ_GreatCallSound.et` | like the call sound prefab, with a deeper, longer call on SOUND_CALL | `RZ_CallSoundComponent` | Zombies > Tribes > Great call sound prefab | optional | the great one's dusk call (empty: the call sound). Manual: the sound itself |
| (existing prefab) | small vanilla props: a bicycle on its side, a rucksack, a single shoe, a car door (static props, no gore) | — | Zombies > Tribes > Kill-site prefabs (list) | optional | one thing at each kill site while a player is within 200 m. Manual: pick a few props by hand; the lines are told whether or not any are set |
| (existing prefab) | a static prop: flattened bedding, a heap of old coats, or somebody's things at the back of a culvert | — | Zombies > Tribes > Abandoned lair prefab | optional | where a pack lay up before it moved, while a player is within 200 m. Manual: pick or build the prop |
| (existing prefab) | a wooden post or a gatepost with chalk on it (a decal or a sign prop) | — | Zombies > Tribes > Haven mark prefab | optional | the Haven's mark where a road enters a pack's ground (the text is read within 25 m). Manual: the mark's look; its words stay text |
| (existing prefab) | a stencilled metal plate or sign | — | Zombies > Tribes > Bastion mark prefab | optional | Bastion's mark (P-2 · QUARRY · 20+). Manual: the mark's look; its words stay text |
| (existing prefab) | a post or a wall section with a red hand painted on it, a line through it | — | Zombies > Tribes > Red Hand mark prefab | optional | the Red Hand's mark (a crew's name). Manual: the mark's look; its words stay text |
| (existing prefab) | a small plate with the plague doctor stencilled on it | — | Zombies > Tribes > Halcyon mark prefab | optional | Halcyon's mark on a tagged pack (P-02 · I d19). Manual: the mark's look; its words stay text |
| (existing prefab) | runner, helmet or screamer prefabs from your zombie mods | — | Zombies > Adaptation > Runner prefabs (list) | optional | the Collapse trait |
| (existing prefab) | runner, helmet or screamer prefabs from your zombie mods | — | Zombies > Adaptation > Helmet prefabs (list) | optional | the Overrun trait |
| (existing prefab) | runner, helmet or screamer prefabs from your zombie mods | — | Zombies > Adaptation > Screamer prefabs (list) | optional | the Aftermath trait |

#### Faction services and road posts

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_SignalSmoke.et` | a vanilla smoke grenade (still throwable; red smoke if there is one) | `RZ_FireMissionSmokeComponent` | (Bastion shop) | optional | Fire Mission perk: a Bastion Champion throws it at a horde; sold on the Bastion quartermaster's Champion shelf |
| `RZ_RedHandArmband.et` | a red armband or bandana clothing item; keep "armband_red" or "redhand" in its name | — | Factions > Services > Red Hand armband | optional | reputation clue at Red Hand Trusted (also counts toward the Red Hand disguise) |
| `RZ_HavenRibbon.et` | an amber armband or ribbon clothing item (no Red Hand or Bastion keywords in its name) | — | Factions > Services > Haven ribbon | optional | reputation clue at Haven Trusted |
| `RZ_BastionPostBarrier.et` | sandbags and a barrier pole (a small composition) | — | Factions > Checkpoints > Bastion checkpoint barrier | optional | Bastion checkpoints inside the Zone (without it: the crew alone) |
| `RZ_RedHandRoadblock.et` | a car wreck or logs across the road | — | Factions > Checkpoints > Red Hand roadblock | optional | Red Hand roadblocks inside the Zone (without it: the crew alone) |
| `RZ_BastionPostCrew.et` | an AI group of two or three on MIL | — | Factions > Checkpoints > Bastion checkpoint crew | optional | Bastion checkpoints (falls back to Territory > Bastion guard; none = no checkpoints) |
| `RZ_RedHandPostCrew.et` | an AI group of two or three on BAND | — | Factions > Checkpoints > Red Hand roadblock crew | optional | Red Hand roadblocks (falls back to Territory > Red Hand guard, then the Red Hand group; none = no roadblocks) |

#### Quartermasters

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_QM_Haven.et` | a civilian character without AI (damage off) | `RZ_QuartermasterComponent` (Faction SURVIVORS); `RZ_ShopOpenAction`, `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), 8 × `RZ_SafehouseAction` (Kind STATUS, DELIVER_FOOD, RADIO, HEAL, DECLARE_BITE, MEDS, BOARD_READ, BOARD_WRITE), 3 × `RZ_FactionServiceAction` (Service AMENDS, SETTLE, ROADS), 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER) | Factions > Haven quartermaster | **minimum** | the Haven shop, leader and safehouse services. Add RZ_FactionShopAction entries for vanilla stock by hand (suggested stock below) |
| `RZ_QM_Bastion.et` | a soldier character without AI (damage off) | `RZ_QuartermasterComponent` (Faction MILITARY); `RZ_ShopOpenAction`, `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), 2 × `RZ_FactionServiceAction` (Service AMENDS, SETTLE), `RZ_FactionShopAction` (Faction MILITARY, Required tier CHAMPION, Favor cost 30, Item prefab `RZ_SignalSmoke.et`, Display name Signal smoke), `RZ_FactionServiceAction` (Service CONTACT_BOARD), 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER) | Factions > Bastion quartermaster | **minimum** | the Bastion shop and leader. Add one RZ_RequisitionAction per ammo or rations item, and shop actions for vanilla stock, by hand |
| `RZ_QM_RedHand.et` | a civilian character without AI in rough clothes (damage off) | `RZ_QuartermasterComponent` (Faction BANDITS); `RZ_ShopOpenAction`, `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_FactionShopAction` (Faction BANDITS, Required tier KNOWN, Favor cost 12, Item prefab `RZ_CutDose.et`, Display name Cut dose), 6 × `RZ_FactionServiceAction` (Service FENCE, BOUNTY_BOARD, AMENDS, SETTLE, CREW_MAP, FREQUENCIES), 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER) | Factions > Red Hand quartermaster | **minimum** | the Red Hand black market and leader |
| `RZ_QM_Halcyon.et` | a character in a lab coat without AI (damage off) | `RZ_QuartermasterComponent` (Faction INSTITUTE); `RZ_ShopOpenAction`, `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_TrialDoseAction`, `RZ_FactionShopAction` (Faction INSTITUTE, Required tier TRUSTED, Favor cost 60, Item prefab `RZ_Antidote.et`, Display name Antidote); `RZ_FactionShopAction` (Faction INSTITUTE, Required tier KNOWN, Favor cost 40, Item prefab `RZ_Stabiliser.et`, Display name Stabiliser), 6 × `RZ_FactionServiceAction` (Service MEDIC, BOOSTER, AMENDS, SETTLE, RANGE_MAP, CALL_BOX), 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER) | Factions > Halcyon quartermaster | **minimum** | the Halcyon shop, leader, antidotes and the trial dose |

#### Faction consequences

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_BastionHunters.et` | an AI group on MIL | — | Factions > Bastion hunter squad | optional | Bastion hunter squads |
| `RZ_RedHandHunters.et` | an AI group on BAND | — | Factions > Red Hand hunter squad | optional | Red Hand hunter squads, safehouse location leaks (falls back to the Red Hand group) |

#### The Perimeter

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_Drone.et` | a small quadcopter mesh with a hum and a light; no collision or physics | — | Perimeter > Drone prefab | optional | one drone over a player in the warning zone |
| `RZ_WarningShot.et` | a gunshot sound or impact effect | — | Perimeter > Warning shot prefab | optional | the warning shot |
| `RZ_Checkpoint.et` | concrete blocks and a closed barrier (a composition) | — | Perimeter > Barricade prefab | optional | road checkpoints (required for checkpoints) |
| `RZ_CheckpointGuard.et` | a soldier without AI with his rifle (no grenades), or a figure prop | — | Perimeter > Guard prefab | optional | 0-4 per checkpoint |
| `RZ_Smuggler.et` | a civilian without AI (the Red Hand runner) | — | Perimeter > Smuggler prefab | optional | The Blind Spot (falls back to the civilian) |
| `RZ_Package.et` | a small box item | `RZ_MissionItemComponent` | Perimeter > Package prefab | optional | The Blind Spot, The Crossing (falls back to the mission item) |
| `RZ_LineBody.et` | a body prop | the clue set | Perimeter > Line body prefab | optional | Shot at the Line (falls back to the puzzle body) |

#### Haven safehouses and wanderers

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_SafehouseMedic.et` | a civilian without AI | 2 × `RZ_SafehouseAction` (Kind HEAL, DECLARE_BITE) | Haven > Medic prefab | optional | Settlement and up (the quartermaster's actions work without it) |
| `RZ_SafehouseOccupant.et` | a civilian without AI, or a seated prop (no armour or weapons: nothing a shop sells) | the generic set | Haven > Occupant prefabs (list) | optional | 3-6 people shown (empty = the civilian); add more variants to the list by hand |
| `RZ_SafehouseGuard.et` | an armed AI character or group on SURV | — | Haven > Guard prefab | optional | guards; they shoot armed exiles |
| `RZ_Wanderer_Trader.et` | a civilian without AI (stands still while seen), or a one-man AI group on SURV (walks; give it a gun to fight back) | the generic set | Wanderers > Trader prefab | optional | the trader's body (falls back to the civilian) |
| `RZ_Wanderer_Doctor.et` | a civilian without AI (stands still while seen), or a one-man AI group on SURV (walks; give it a gun to fight back) | the generic set | Wanderers > Doctor prefab | optional | the doctor's body (falls back to the civilian) |
| `RZ_Wanderer_Scavenger.et` | a civilian without AI (stands still while seen), or a one-man AI group on SURV (walks; give it a gun to fight back) | the generic set | Wanderers > Scavenger prefab | optional | the scavenger's body (falls back to the civilian) |
| `RZ_Wanderer_Deserter.et` | a civilian without AI (stands still while seen), or a one-man AI group on SURV (walks; give it a gun to fight back) | the generic set | Wanderers > Deserter prefab | optional | the deserter's body (falls back to the civilian) |
| `RZ_Wanderer_Preacher.et` | a civilian without AI (stands still while seen), or a one-man AI group on SURV (walks; give it a gun to fight back) | the generic set | Wanderers > Preacher prefab | optional | the preacher's body (falls back to the civilian) |
| `RZ_Wanderer_Courier.et` | a civilian without AI (stands still while seen), or a one-man AI group on SURV (walks; give it a gun to fight back) | the generic set | Wanderers > Courier prefab | optional | the courier's body (falls back to the civilian) |
| `RZ_Wanderer_Kid.et` | a civilian without AI (stands still while seen), or a one-man AI group on SURV (walks; give it a gun to fight back) | the generic set | Wanderers > Kid prefab | optional | the kid's body (falls back to the civilian) |
| `RZ_Wanderer_Mechanic.et` | a civilian without AI (stands still while seen), or a one-man AI group on SURV (walks; give it a gun to fight back) | the generic set | Wanderers > Mechanic prefab | optional | the mechanic's body (falls back to the civilian) |
| (existing prefab) | any items | — | Wanderers > Trader goods (list) | optional | what the Trader sells |

#### Faction leaders

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_Leader_Haven.et` | a character without AI (damage off) | the generic set | Leaders > Haven leader | optional | optional body next to the quartermaster |
| `RZ_Leader_Bastion.et` | a soldier without AI (damage off) | the generic set | Leaders > Bastion leader | optional | optional body next to the quartermaster |
| `RZ_Leader_RedHand.et` | a character without AI (damage off) | the generic set | Leaders > Red Hand leader | optional | optional body next to the quartermaster |
| `RZ_Leader_Halcyon.et` | a character in a lab coat without AI (damage off) | the generic set | Leaders > Halcyon leader | optional | optional body next to the quartermaster |
| `RZ_CouncilHead.et` | a civilian without AI on SURV, damage on (a council head can die and be replaced) | the generic set | Leaders > Council head | optional | optional body per safehouse |
| `RZ_Outpost_Bastion.et` | a soldier without AI | the generic set | Leaders > Bastion outpost commander | optional | optional outpost leader (falls back to the faction leader, then the civilian); Kill the Officer needs the Bastion one to be able to die |
| `RZ_Outpost_RedHand.et` | a character without AI | the generic set | Leaders > Red Hand crew boss | optional | optional outpost leader (falls back to the faction leader, then the civilian); Kill the Officer needs the Bastion one to be able to die |
| `RZ_Outpost_Halcyon.et` | a character without AI | the generic set | Leaders > Halcyon field lead | optional | optional outpost leader (falls back to the faction leader, then the civilian); Kill the Officer needs the Bastion one to be able to die |

#### Territory and supply trucks

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_Guard_Haven.et` | an armed AI character or small group on SURV | — | Territory > Haven guard | optional | guards at holdings; watched for retaliation |
| `RZ_Guard_Bastion.et` | an armed AI character or small group on MIL | — | Territory > Bastion guard | optional | guards at holdings; watched for retaliation; Bastion's squads at the lab during the final stand in Authors |
| `RZ_Guard_RedHand.et` | an armed AI character or small group on BAND | — | Territory > Red Hand guard | optional | guards at holdings; watched for retaliation |
| `RZ_Guard_Halcyon.et` | an armed AI character or small group on INST | — | Territory > Halcyon guard | optional | guards at holdings; watched for retaliation; the extra guard at the lab gate during the final stand in Authors |
| `RZ_Occupant_Haven.et` | a character without AI in the faction's clothes (no armour or weapons: nothing a shop sells) | the generic set | Territory > Haven occupant | optional | the named people (empty = the civilian) |
| `RZ_Occupant_Bastion.et` | a character without AI in the faction's clothes (no armour or weapons: nothing a shop sells) | the generic set | Territory > Bastion occupant | optional | the named people (empty = the civilian) |
| `RZ_Occupant_RedHand.et` | a character without AI in the faction's clothes (no armour or weapons: nothing a shop sells) | the generic set | Territory > Red Hand occupant | optional | the named people (empty = the civilian) |
| `RZ_Occupant_Halcyon.et` | a character without AI in the faction's clothes (no armour or weapons: nothing a shop sells) | the generic set | Territory > Halcyon occupant | optional | the named people (empty = the civilian) |
| `RZ_Truck_Haven.et` | a drivable vehicle (keeps its damage manager) | — | Supply > Haven truck | optional | the supply truck |
| `RZ_Truck_Bastion.et` | a drivable vehicle (keeps its damage manager) | — | Supply > Bastion truck | optional | the supply truck |
| `RZ_Truck_RedHand.et` | a drivable vehicle (keeps its damage manager) | — | Supply > Red Hand truck | optional | the supply truck; the truck in Tied Down and the van in the Cargo questline (needed for both) |
| `RZ_Truck_Halcyon.et` | a drivable vehicle (keeps its damage manager) | — | Supply > Halcyon truck | optional | the supply truck; Mobile Lab's lab truck |
| `RZ_TruckCrew_Haven.et` | an AI group of 1-2 on SURV; the first one drives | — | Supply > Haven crew | optional | the crew |
| `RZ_TruckCrew_Bastion.et` | an AI group of 1-2 on MIL; the first one drives | — | Supply > Bastion crew | optional | the crew |
| `RZ_TruckCrew_RedHand.et` | an AI group of 1-2 on BAND; the first one drives | — | Supply > Red Hand crew | optional | the crew; the truck crew in Tied Down |
| `RZ_TruckCrew_Halcyon.et` | an AI group of 1-2 on INST; the first one drives | — | Supply > Halcyon crew | optional | the crew |

#### Camp life

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| (existing prefab) | vanilla AIWaypoint_Patrol.et | — | Territory > Camp life > Patrol waypoint | optional | the patrol loop (empty = the move waypoint) |
| (existing prefab) | vanilla AIWaypoint_Cycle.et | — | Territory > Camp life > Cycle waypoint | optional | the patrol repeats (empty = once per shuffle) |
| (existing prefab) | vanilla AIWaypoint_Loiter_CO.et | — | Territory > Camp life > Loiter waypoint | optional | standing about at a piece with no free smart action, gathering (empty = the move waypoint) |
| (existing prefab) | vanilla AIWaypoint_ObservationPost.et | — | Territory > Camp life > Observation waypoint | optional | a sentry with no nest (empty = the loiter waypoint) |
| (existing prefab) | vanilla Group_Base.et (an AI group with no members) | — | Territory > Camp life > Group prefab | optional | lone characters (occupants, single guards) join one so they can take jobs (empty = they stand) |
| (existing prefab) | a new prefab of class SCR_SmartActionWaypoint, saved as Prefabs/ReforgerZ/AI/RZ_WP_SmartAction.et (vanilla ships the class, not a prefab) | — | Territory > Camp life > Smart action waypoint | optional | people use a piece's sit, lean, work and watch points (empty = they loiter beside it) |
| (existing prefab) | the 49 RZ_Dress_* compositions in Prefabs/ReforgerZ/Dressing/ (section 4, Camp life: the compositions and kits) | — | Territory > Camp life > Kits (each piece's prefab) | optional | scenery at holdings, safehouses and road posts (a place with no kit stays bare) |

#### New-player guidance

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_WindupRadio.et` | a small radio item; keep "radio" in its name | `RZ_TuneRadioAction`, `RZ_RadioCallAction` | Guidance > Radio prefab | optional | given to every new life |
| `RZ_CarWreck.et` | a wrecked car prop | — | Guidance > Car wreck prefab | optional | the back-road scenario |
| `RZ_ScratchedDoor.et` | a door or wall panel prop | the clue set | Guidance > Door prefab | optional | the basement scenario |
| `RZ_Flyer.et` | a poster prop | the clue set | Guidance > Flyer prefab | optional | the church scenario, safehouse flyers |
| `RZ_TableRadio.et` | a radio prop | `RZ_ClueComponent`; `RZ_ReadClueAction` (Label Listen) | Guidance > Table radio prefab | optional | the farmhouse scenario; beside the operators in No Reply and Await Relief, and The Census's nightly call |
| `RZ_DeadRaider.et` | a body prop with a red hand on the jacket | the clue set | Guidance > Raider body prefab | optional | the attic scenario |
| `RZ_DeadSoldier.et` | a Bastion soldier body prop | the clue set | Guidance > Soldier body prefab | optional | the barricade scenario |
| `RZ_Barricade.et` | sandbags and a car on its side | — | Guidance > Barricade prefab | optional | barricade scenery |

#### Power grid and radio station

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_PowerControl.et` | a generator or breaker cabinet | the generic set | World > Power grid > Control prefab | optional | the power station (falls back to the fuse box, then the puzzle object) |
| `RZ_RadioConsole.et` | a radio set on a desk | the generic set | World > Radio station > Console prefab | optional | the radio station (falls back to the radio mast, then the puzzle object) |

#### World interactions

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_InteractionSpot.et` | an empty GenericEntity with a small invisible collision box, off the character layer | the generic set | Interactions > General > Spot prefab | optional | every searchable thing, the Unlock/Lock/Force spot, the Clean the bite spot (falls back to the puzzle object, which is visible). Add the small collision box by hand; keep it off the character collision layer |
| `RZ_Key.et` | a small item (a key or key ring) | `RZ_KeyItemComponent`; `RZ_ReadKeyTagAction` | Interactions > Search > Key prefab | optional | keys: they lie in houses next to letters, car keys in their car's town, and come off bodies and from the Mechanic (empty = no keys). It must be a real item that can be picked up: an InventoryItemComponent and vanilla's pick-up action. The notebook base may be a prop; if the key shows no pick-up prompt on the ground, rebase it on a small vanilla item |
| `RZ_DoorBlocker.et` | an empty GenericEntity; add the collision-only box by hand (recipe below) | — | Interactions > Search > Doorway blocker | optional | keeps AI out of locked doorways near players. Needs only RplComponent, a collision-only MeshObject and a static RigidBody: see the doorway blocker recipe |
| (existing prefab) | every item that can lie in a house or turn up in a car boot or on a body (vanilla and mod items) | — | Interactions > Search > Loot pool (list) | optional | nothing lies in houses (and boots and bodies hold nothing) until it's filled; each must be a real item with vanilla's pick-up action |
| (existing prefab) | the repo's parked copies of the Car prefabs (Prefabs/ReforgerZ/Interactions/Cars/), each with SCR_BaseLockComponent m_bIsLocked 1 | — | Interactions > Cars > Parked prefabs (list, the same order as Car prefabs) | optional | a fixable car before it runs: the same car, parked and locked. One per Car prefab, in the same order. A new car needs its own parked copy: inherit it and set its SCR_BaseLockComponent > m_bIsLocked |
| (existing prefab) | vanilla drivable civilian cars | — | Interactions > Cars > Car prefabs (list) | optional | what a fixed car becomes (empty = no fixable cars) |
| `RZ_CarBattery.et` | an item; keep "battery" in its name | — | Interactions > Cars > Battery | optional | car parts |
| `RZ_Tyre.et` | an inventory item with a tyre mesh; keep "tyre" in its name | — | Interactions > Cars > Tyre | optional | car parts. The repo's is vanilla Item_Base with the UAZ tyre's mesh: 9 kg, 2x2 |
| `RZ_SparkPlugs.et` | an item; keep "spark" in its name | — | Interactions > Cars > Spark | optional | car parts |
| `RZ_FuelCan_Full.et` | a full jerrycan item | — | Interactions > Cars > Fuel prefab | optional | siphoning, pumps, fuel, fire |
| `RZ_BoardedDoorway.et` | planks across a door frame, with collision | — | Interactions > Traps > Barricade prefab | optional | the boarded doorway |
| `RZ_TripwireSet.et` | a set tripwire prop | — | Interactions > Traps > Tripwire set | optional | look of a set tripwire (optional) |
| `RZ_SpikeSet.et` | a set spike trap prop | — | Interactions > Traps > Spike trap set | optional | look of a set spike trap (optional) |
| `RZ_FireEffect.et` | a fire and smoke particle | — | Interactions > Traps > Fire effect | optional | a burning building |
| (existing prefab) | the vanilla bandage item | — | Interactions > Crafting > Bandage | optional | the workbench's bandage recipe |
| `RZ_Molotov.et` | a throwable bottle (a vanilla grenade with the explosion removed and a bottle mesh) | `RZ_MolotovComponent` | Interactions > Crafting > Molotov | optional | fire |
| `RZ_TinRattle.et` | a throwable tin item | `RZ_NoisemakerComponent` (Heat 12, On throw on, Delete after off) | Interactions > Crafting > Noisemaker | optional | the workbench's noisemaker recipe |
| (existing prefab) | a melee weapon prefab (vanilla or mod) | — | Interactions > Crafting > Spear | optional | the workbench's spear recipe |
| `RZ_TripwireKit.et` | a small item | `RZ_PlaceableComponent` (Kind TRIPWIRE); `RZ_PlaceAction` | Interactions > Crafting > Tripwire | optional | tripwires |
| `RZ_SpikeTrapKit.et` | a small item | `RZ_PlaceableComponent` (Kind SPIKES); `RZ_PlaceAction` | Interactions > Crafting > Spike trap | optional | spike traps |
| `RZ_Tablets.et` | a small pill tube item | — | Interactions > Crafting > Tablets | optional | water purification |
| (existing prefab) | the survival mod's untreated water bottle | — | Interactions > General > Dirty water | optional | wells, stoves |
| (existing prefab) | the survival mod's safe water bottle | — | Interactions > General > Clean water | optional | wells, stoves |

#### Loot items

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_Planks.et` | an inventory item with a planks mesh | `RZ_PlaceableComponent` (Kind BARRICADE); `RZ_PlaceAction` | (loot or shop) | optional | barricades. The repo's is vanilla Item_Base with the junk planks' mesh: 4 kg, 2x2. Its prefab name has "plank", so crafting uses it too |
| `RZ_PortableGenerator.et` | a generator item | `RZ_PlaceableComponent` (Kind GENERATOR); `RZ_PlaceAction`; `RZ_PlaceAction` (Refuel on) | (loot or shop) | optional | a generator |
| `RZ_Bottle.et` | a vanilla grenade with the explosion removed and a bottle mesh | `RZ_NoisemakerComponent` (Heat 20, On throw on, Delete after on) | (loot or shop) | optional | a thrown noise |
| `RZ_Firecracker.et` | a small item | `RZ_NoisemakerComponent` (Heat 15, Delay 5, Pulses 4, Pulse interval 3, On throw off, Delete after on, Arm label Light the fuse); `RZ_ArmNoisemakerAction` | (loot or shop) | optional | a timed noise |
| `RZ_AlarmClock.et` | a clock prop item | `RZ_NoisemakerComponent` (Heat 10, Delay 30, Pulses 8, Pulse interval 8, On throw off, Delete after off, Arm label Wind the alarm); `RZ_ArmNoisemakerAction` | (loot or shop) | optional | a long timed noise |
| `RZ_FuelCan_Empty.et` | an empty jerrycan item | — | (loot) | optional | siphoning, pumps |

#### Hand-set list entries

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| (existing prefab) | items from the loot pool | — | Interactions > Search > Loot tables (each table's three lists) | optional | exact loot per building type (optional; empty tables are filled from the pool by keywords) |
| (existing prefab) | any item | — | Interactions > Crafting > Extra recipes (each output) | optional | extra workbench recipes (optional) |
| (existing prefab) | vanilla items or crates (never a vehicle or a car part: refused) | — | Each shop action's Item prefab | optional | quartermaster stock beyond what the plugin adds (see the suggested stock) |
| (existing prefab) | vanilla ammo or rations the same quartermaster sells at Trusted or below | — | Each requisition action's Item prefab | optional | Bastion's Requisition (Trusted pays half, once an hour) |

#### UI

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| (existing prefab) | the RZ_Icons imageset that tools/icon_atlas.py builds from UI/Icons/png/128 (UI/LAYOUTS.md) | — | UI > Icon imageset | optional | icons on the HUD, cards and menus (empty = the images set in the layouts) |
| (existing prefab) | the NoticeCard layout (UI/LAYOUTS.md) | — | UI > Notice layout | optional | styled notice cards (empty = the vanilla hint) |
| (existing prefab) | the MissionRow layout (UI/LAYOUTS.md) | — | UI > Mission row layout | optional | journal rows |
| (existing prefab) | the RadioRow layout (UI/LAYOUTS.md) | — | UI > Radio row layout | optional | rows of the notebook's Radio tab |
| (existing prefab) | the MapIcon layout (UI/LAYOUTS.md) | — | UI > Map icon layout | optional | the ReforgerZ icons on the map, per player (empty = vanilla text markers for events and the lab). Also add RZ_MapIconsUI to the map configuration's components (Setting up the UI) |
| (existing prefab) | the SpawnRow layout (UI/LAYOUTS.md) | — | UI > Spawn row layout | optional | our spawn screen, the background picker, in place of vanilla's deploy menu (empty = vanilla's deploy menu). Needs the RZ_SpawnMenu preset in Configs/System/chimeraMenus.conf (Setting up the UI) |
| (existing prefab) | the MapLegend layout (UI/LAYOUTS.md) | — | UI > Map legend layout | optional | the map key in the full-screen map's bottom-left, shown zoomed out (empty = no legend; needs the map icon layout) |
| `RZ_Notebook.et` | a small notebook item (vanilla's carryable notebook); display name Notebook, weight 0.1 kg | `RZ_NotebookGadgetComponent`; `RZ_DeadNameAction`, 2 × `RZ_EpiNotebookAction` (Kind SEASON, ANONYMOUS) | UI > Notebook prefab | optional | given to every new life; taken into the hand it opens the notebook (the pad's way in besides the pause menu). Built as text in the repo; check in Workbench that it can be taken into the hand and put in a quick slot |
| (existing prefab) | the Standing page layout (UI/LAYOUTS.md) | — | UI > Standing page | optional | the notebook's Standing tab (empty = Nothing here yet) |
| (existing prefab) | the Journal page layout (UI/LAYOUTS.md) | — | UI > Journal page | optional | the Journal tab (empty = Nothing here yet) |
| (existing prefab) | the Radio page layout (UI/LAYOUTS.md) | — | UI > Radio page | optional | the Radio tab (empty = Nothing here yet) |
| (existing prefab) | the Hall page layout (UI/LAYOUTS.md) | — | UI > Hall page | optional | the Hall of fame tab (empty = Nothing here yet) |
| (existing prefab) | the Season page layout (UI/LAYOUTS.md) | — | UI > Season page | optional | The season tab: the majors, and in the epilogue the season as the Zone tells it (empty = Nothing here yet) |
| (existing prefab) | the TalkReply layout (UI/LAYOUTS.md) | — | UI > Talk reply layout | optional | the talk screen's reply rows (empty = no talk screen: a person's actions and notices) |
| (existing prefab) | the CarJob layout (UI/LAYOUTS.md) | — | UI > Car job layout | optional | the car repair screen's job rows (empty = no repair screen: the car's actions and notices) |
| (existing prefab) | UI/Textures/ReforgerZ/RZ_CarSide.png, imported as a texture (UI/LAYOUTS.md) | — | UI > Car picture | optional | the side view of the car on the repair screen (empty = the job markers on a plain panel). Import the PNG in the Resource Browser (it becomes RZ_CarSide.edds), then pick it here |
| (existing prefab) | the ShopItem layout (UI/LAYOUTS.md) | — | UI > Shop item layout | optional | the quartermaster's shop screen's item rows (empty = no shop screen: the Buy, Requisition and service actions) |

#### Questlines

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_RedHandLorry.et` | a static civilian lorry or flatbed with no vehicle component (a wreck-style static prop); confirm the name in Workbench | — | Questlines > Majors > Red Hand lorry | optional | look only: the lorry across the road at Rudi's crossroads for the Book's send-off and its last line, near players only (else words only) |
| `RZ_Bonfire.et` | a large campfire or bonfire prop, lit; confirm the name in Workbench | — | Questlines > Majors > Bonfire prop | optional | look only: the fire at Rudi's crossroads when the book is sent off loud, near players only (else words only) |
<!-- /manifest:prefabs -->

### Stand-in bases

Many bases only stand in for the real thing, often because vanilla 1.9 has nothing closer (no lab
coat, armbands, children, bus, drone or melee weapon). What each one means in play, and what's
still to do:

| Prefabs | Built on | What's off, and what to do |
|---|---|---|
| The 23 AI groups | Vanilla FIA and US soldier groups (FIA fire team, sentry team, medical section, sharpshooter team; US sentry and light fire teams). Their faction (`m_faction`) was set by hand, as their names say | Members wear soldiers' kit and carry rifles: the escort civilians are armed FIA riflemen, the convicts carry rifles. Sizes are only roughly right. Give each group its own members |
| Single characters: quartermasters, leaders, occupants, wanderers | Vanilla civilian, US and FIA characters. Their faction affiliation was set afterwards, as their names say | They keep their vanilla kits: set them by hand. Halcyon's people are FIA medics; the kid is an adult |
| `RZ_Antidote.et`; `RZ_Stabiliser.et` and `RZ_CutDose.et` | Saline; morphine, with their consumable effects set to `RZ_ConsumableAntidote` and `RZ_ConsumableMed` | They look like saline and morphine |
| `RZ_Molotov.et`, `RZ_Bottle.et`, `RZ_TinRattle.et` | Smoke grenades with the beer bottle mesh (the tin can mesh for the tin), the smoke cleared, their own names | They throw as grenades do; a real throwable item is the fix |
| `RZ_Explosion.et` | The vanilla mortar effect module (`EffectModule_MortarShell`) | A real HE round: it damages what's near it. **Make it look and sound only, no damage.** The scripts decide every harm themselves, as data (Fire Mission's strike, the infection Bastion Shelling takes off, Gas Station Fire's kills), and the same prefab goes off where players stand by design: the convoy's tripwire at the crates, the bridge charges they set, the burning helicopter they watch, Bastion's guns on the farm. Rebuild it the way `Perimeter/RZ_WarningShot.et` is built: base `Prefabs/Weapons/Warheads/Warhead_Base.et`; a `TimerTriggerComponent` (TIMER 0.05) whose PROJECTILE_EFFECTS hold one `SpawnParticleEffect` with `Particles/Weapon/Blast_Mortar_Dirt.ptc` and the explosion's sound event; a `SoundComponent` with `Sounds/Weapons/Ammo/MortarShells/Weapons_Ammo_MortarShell_8xmm_HE.acp` (use the event name that file's explosion plays); `SignalsManagerComponent` and `RplComponent`; no explosion damage effect and no `HitEffectComponent` damage. Check in play: a character 3 m from one, and a car 5 m from one, are untouched |
| `RZ_Nest.et` | Special Zombies' `ZombieHive_Minor` | It spawns its own infected (a child `Bacon_622120A5448725E3_InfectedSpawnerEntity`, ID 69BD219DDB924140) and blows up when destroyed (`Warhead_Exploder_HiveBlast.et`). **The nest must not spawn:** Burn the Nest and Specimen from the Nest bring their own guards through the spawner, inside the agent budget ([INFECTED.md](INFECTED.md)); the hive's spawner would double them, outside it. Keep the hive's look and that it burns, and drop both: in `RZ_Nest.et` delete the spawner child; if Workbench won't delete an inherited child, build `RZ_Nest.et` as a plain `GenericEntity` instead with the hive's `MeshObject` (`HaystackSmall_01.xob` with its two Hive materials), `RigidBody` (Static 1, ResponseIndex MediumDestructible, ModelGeometry 1), `SCR_DestructionMultiPhaseComponent` (base health 750, Incendiary, Explosive and Fragmentation multipliers 20, Kinetic 0.01; on destruction only the `Blast_huge_Hive.ptc` particle, no `SCR_PrefabSpawnable` warhead), a `SoundComponent` with `Environment_Hive.acp`, and an `RplComponent`. Check in play: 5 minutes beside it bring nothing but the mission's guards, and a grenade destroys it |
| `RZ_Lab.et` | A vanilla US tent | It can be destroyed (1000 HP) |
| The packs' signs | Kill sites: suitcases, a UAZ tyre, a folded blanket; the old lair: a bunk-bed mattress; the marks: a stone verge post (the Haven), the minefield sign (Bastion), the FIA graffiti decal (the Red Hand), a no-entry sign (Halcyon) | Close enough to read; swap in better props when there are any |
| Camp life's compositions | Vanilla props only: the barrel fire is `Particles/Campfire_burning.et`, the cooler a small crate and an autoclave drum, Halcyon's tent `TentSmall_03_white` with a medical table, the civilian tents `TentSmall_02_red` and `TentBig_01_orange` | No new meshes or decals (the red hand, the bear, the plague doctor, chalk, the ball, vegetables, boots, the axe, tubes): those spots are left out or are a plank |
| Halcyon's shop | A clean dressing (Known), a saline bag and a tourniquet (Trusted) stand in for the antiseptic and the stim kit | Vanilla has no gas mask or stim |
| Looks only | Five items share the aspirin bottle (sample, vet antiviral, fever suppressant, home remedy, tablets); six are food packets (antibiotics, car battery, spark plugs, the tripwire and spike trap kits, firecracker); the fog is white smoke; the Red Hand armband and Haven ribbon are knitted hats; the bus is a UAZ van | Real models, from a mod or your own. The manifest's `basePrefab` says what every other one is |

### Add to prefabs you already have

<!-- manifest:additions -->
| Prefab | Add | Why | Need |
|---|---|---|---|
| every player character prefab used by a loadout (the plugin's RZ_Loadout_*.et get it already) | `RZ_TreatOtherAction` | other players can treat you | required |
| any vanilla radio prefab you want crackable | `RZ_TuneRadioAction`, `RZ_RadioCallAction` | secure radio channels, and calling on the open channel with Dialogue > Trees on (the wind-up radio gets both already) | optional |
| quartermasters you placed by hand (not from the manifest) | `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), 2 × `RZ_FactionServiceAction` (Service AMENDS, SETTLE) | standing, the stronghold leader and amends (the plugin's quartermasters get these already; also add the faction's services: Fence and Bounty board on the Red Hand, Medic and Immunity booster on Halcyon) | optional |
| the Bastion quartermaster | `RZ_RequisitionAction` (Favor cost 30, Display name rations) | Requisition: one action per ammo or rations item (set its Item prefab to one this quartermaster sells at Trusted or below; anything else is refused) | optional |
| any quartermaster | `RZ_FactionShopAction` | vanilla stock (suggested stock table); the Item prefab is a vanilla item or crate, never a vehicle or a car part (refused: cars are only found) | optional |
| the player controller prefab the game mode uses (the repo's Prefabs/Characters/Core/RZ_PlayerController.et has it; RZ_GameMode.et's PlayerControllerPrefab points at it) | `RZ_PlayerUIComponent` | the UI's data feed: infection timer, secure radio, noise, standing, journal, backgrounds (docs/UI.md) | optional |
| the player controller prefab the game mode uses (the repo's Prefabs/Characters/Core/RZ_PlayerController.et has it, next to RZ_PlayerUIComponent) | `RZ_PlayerEffectsComponent` | the infected's close moves on the player's own machine: a hold kicked free with Jump, a stagger, a knockdown (docs/INFECTED.md, Attacks); without it those effects stay off and only their lines come | optional |
<!-- /manifest:additions -->

### Notes by system

**The generic puzzle object** is reused by many systems for anything that's read, talked to or
used, so make it early. Missions label the actions ("Seal the well", "Accuse Mirek") and hide the
slots they don't use. **Without replicated labels** (the prefab needs an `RplComponent`), every slot shows its generic
label, and the actions still route correctly. The specific puzzle props only change the look;
missions turn themselves off when a prefab they need is missing.

**The infected** ([INFECTED.md](INFECTED.md)). **Zombies > Empty group** (Bacon's
`Group_Zombies_Base`) is what lets crawlers and loose ones made from character prefabs walk anywhere:
without it they stand on their own and Bacon's stay inactive. **Zombies > Crawlers > Prefabs** takes
characters (each spawn is a group of 1-3 in an empty group) or groups; the repo's game mode has
Bacon's civilian characters there and its exploder as the bloated one. Bacon has no slow prefab, so
crawlers are as fast as the rest (about 3.7 m/s): if a zombie mod in your stack has a slower civilian,
put it here ([MOD_STACK.md](MOD_STACK.md)). Both **Zombies > Crawlers** and **Zombies > Tribes** are on
by default; with both off the spawner is as it was. The first start on a map reads the land grid
(about 5 minutes on Everon, logged), then loads it from the profile.

**The packs' signs and sounds** ([INFECTED.md](INFECTED.md#signs-and-sounds)) are all by hand and all
optional: the plugin can't pick props or record a call. For **Kill-site prefabs**, **Abandoned lair
prefab** and the four **mark prefabs**, pick static props (no inventory item, no physics that rolls)
and put their names in the slots; nothing about them is scripted. They're set on `RZ_GameMode.et`
with stand-ins ([Stand-in bases](#stand-in-bases)). For the two **call sound** prefabs,
make a new entity prefab with an `RplComponent`, a `SoundComponent` with a sound project that has the
call on the event `SOUND_CALL` (the base game has nothing that sounds right: record or license one), and
`RZ_CallSoundComponent` (its **Event** names the event if you used another). The server deletes it after
40 s. Leave any slot empty and that sign is its line only.

**Treatments.** Each turns itself off when its slot is empty. Wound cleaning needs no prefab: it
uses carried items by keyword (alcohol, antiseptic, bandages, first-aid kits) and the interaction
spot. Make sure the loot pool has some alcohol, antiseptic and herbs.

**Missions.** One mission item prefab serves everything (medicine, notes, black box, package):
the mission tags it when it spawns. Missions spawn and remove their own drop points.

**Contracts** (how the blocks work is in [DESIGN.md](DESIGN.md#mission-building-blocks)). An
escort prefab that isn't an AI group (a character or prop) still works: the people then move as
data and stand as that figure when they stop. **To talk to escorted people** (patch up the pilot,
divert the courier, march the deserters to the stronghold), the escort characters (an AI group's
leader) need the generic set, like the wanderers. Without it, standing beside the pilot, the
deserter or the researcher for 10 s still gets them moving.

**World events** also reuse **Missions > Explosion** (Gas Station Fire's blast, optional),
**Interactions > Traps > Fire effect** (every burning building), the power grid, and the generic
puzzle object (Tunnel Rats' culverts, the tape in a Distress Call loop), **Puzzles > Civilian** (the
people in Distress Call, Rival Survivors and Collapse Shelter) and **Missions > Red Hand group**
(Distress Call's crew in the trees, Crash Site's carriers, the watchers at the Abandoned Convoy),
**Puzzles > Keypad** and **Note** or **Body** (Locked Bunker). Gas Station Fire needs **Traps > Fire**
on. The convicts, the sniper, Distress Call's crew and Crash Site's carriers fight through the
hostility system's fights, so they must be on `BAND`. The newer contracts also reuse **Cars > Car prefabs** (Car Theft), **Supply >
Halcyon truck** (Mobile Lab's lab truck, falling back to the bus), **Supply > Haven truck** (the bus
fallback), the four **supply trucks and crews** (Convoy Escort and Hijack the Convoy ride Bastion's
real truck), **Leaders > Bastion outpost commander** (Kill the Officer needs a body that can die)
and Puzzles > Fuse box, Radio mast and Civilian (generators, the relay mast, the people you talk
to). The **unarmed missions** need no prefabs of their own.

**Quartermasters and shops.** Place one quartermaster per faction, or set the four slots and they
spawn by themselves (the Haven's in the safest town, Bastion's in a bigger town, the Red Hand's in
a rougher town, Halcyon's next to the lab). Their positions are saved. The four `RZ_LeaderAction`s
on every quartermaster are how players reach stronghold leaders and council heads.

Each `RZ_FactionShopAction` sells one thing: **Faction**, **Required tier** (Stranger, Known,
Trusted or Champion), **Favor cost**, **Item prefab** (an item or crate, spawned next to the
quartermaster), **Display name** and **Once per life** (off; on: each player can buy it once a life). The action is always shown (or, with the shop screen on, is a
row on it); the server checks tier and Favor when it's used. The four quartermaster prefabs carry
`RZ_ShopOpenAction` (**Trade**, first in the list), which opens the shop screen while **UI > Shop
item layout** is set; then the Buy, Requisition and service actions hide from the action menu. A
quartermaster you build yourself needs it too. The screen sorts the items into tabs by their
prefab path and name ([UI.md](UI.md#the-shop-screen)); give an item a plain name (a medicine under
`/Medicine/`, food under `/Food/`) and it lands in the right one. **Never a vehicle or a car part** (battery, tyre, spark plugs, fuel can, any
vehicle prefab): cars are only ever found, so the server refuses such an action and logs it.
`RZ_RequisitionAction` has the same cost, item and name; the cost is the full price (a Trusted
player pays half, once an hour). Set it up for ammo and rations only, each an item a shop action
on the **same quartermaster** sells at Trusted or below: Requisition is a Trusted perk, so
anything stocked only at Champion (or not stocked there) is refused and logged.
You can also put `RZ_ChoiceDeliveryAction` on a quartermaster so it accepts hand-ins.

- **Antidotes.** A shop action whose item is the **Cure > Antidote** prefab sells from a stock, not
  freely: on the Halcyon quartermaster it sells the lab's doses (from the start; one every 30
  minutes, 2 at most, down to every 10 minutes and 5 at most after 75%), anywhere else (the Red
  Hand) only doses stolen from the lab. The manifest gives Halcyon's quartermaster one (Trusted),
  else the lab's doses can only be stolen.
- **Stabilisers.** A Halcyon shop action whose item is the **Survival > Infection > Stabiliser**
  prefab sells from a second, smaller stock, at double its price until research reaches 50%. The
  manifest makes it Known, 40 Favor. Anywhere else it's an ordinary item.
- **Price food above 2 Favor.** A safehouse pays 2 Haven Favor (and a little Fame) per food item
  handed in, so food the Haven quartermaster sells for 2 or less turns Favor into free Fame.

Suggested starting stock for the shop actions you add by hand (from [REPUTATION.md](REPUTATION.md)):

| Faction | Known | Trusted | Champion |
|---|---|---|---|
| Haven | canned food, water, bandages, crowbar | hunting rifle, ammo, backpack, radio | fortification kit (barricade planks), saline bag |
| Bastion | rations, morphine, flares | service rifle, body armour, helmet, NVGs | machine gun, grenades, signal smoke |
| Red Hand | knife, pistol, cut dose | suppressor, scoped rifle, explosives | rare military gear, a stolen antidote |
| Halcyon | antiseptic, painkillers, stabiliser | antidote, stim kit, gas mask | — |

**Faction consequences.** Gunfire heat needs nothing: characters already have an
`EventHandlerManagerComponent`, and suppressors are detected by attachment names containing
"suppress" or "silenc".

**Faction services and road posts** ([REPUTATION.md](REPUTATION.md#checkpoints-roadblocks-and-camps)).
The plugin's quartermasters get `RZ_FactionServiceAction`: Amends and Settle up on all four, Fence
and Bounty board on the Red Hand, Medic and Immunity booster on Halcyon; a Champion shop action on Bastion
sells the **signal smoke** (`RZ_SignalSmoke.et`: a throwable smoke grenade with
`RZ_FireMissionSmokeComponent`; the shells are **Missions > Explosion**). Check **Services > Fence
keywords** against your loot. The armband's prefab name should keep a Red Hand uniform keyword
("armband_red") so it counts toward the Red Hand's kit; the ribbon's must not carry a faction's.
Check **Hostility > Disguises**' uniform, head-piece and service-weapon keywords against your
faction loadouts and loot: they decide colours (2 pieces), full kit (3, one on the head) and the
wrong-weapon tell. Halcyon's must match the coat and the beak (a hood or mask with a respirator),
and should not match ordinary dust masks. Keep those keywords rare in loot and on the infected's
loadouts: kit marks make a dead man's and the turned's kit fail, but a restart forgets the marks.
For the one clean way in, give the Red Hand quartermaster two `RZ_FactionShopAction`s at Trusted,
40 Favor each, **Once per life** on: a Bastion green set and a Halcyon coat and beak (a bag with the
pieces in it); anything the Red Hand sells is bought fair. Bastion's and Halcyon's own shop kit is signed out to its buyer. Road
posts need territory, the road network and a crew: their own slot, else the territory guard
(else the Red Hand group). Check the **contraband keywords** against your Red Hand stock.

**Noisemakers** aren't slots: put them in loot or shops. A noise only leads infected at 8 heat or
more at once, so keep them at 10 or more.

**The Perimeter** works with no prefabs at all (loudspeaker messages, the warning shot as a
message, the unseen shooter). Checkpoints need the barricade. Keep the drone free of collision and
physics.

**Haven safehouses and wanderers.** Safehouses need only the spawn point and the Haven
quartermaster. Wanderers need nothing: without bodies they use Puzzles > Civilian. A body can be
talked to only if it has the generic set (the manifest gives it to them).

**Faction leaders** need no prefabs: the quartermaster *is* the leader at strongholds and
safehouses, and outpost leaders use Puzzles > Civilian. Turn damage off on leader bodies.

**Territory and supply trucks.** Territory is data: every prefab here only adds bodies near
players. A faction without a truck and crew counts its places as always supplied. Turning on
**Territory > Outpost quartermaster** puts a shop (and Bastion's Requisition) at each outpost,
using the four quartermasters. Supply trucks also need **Zombies > Move waypoint**.

**The Long Table** has no prefabs of its own. Its people are **Puzzles > Civilian** (or the generic
object), its tell on the road is the **Note**, its fights are **Missions > Events > Rival group**
(else **Missions > Survivor group**), Bastion's guns are **Missions > Explosion**, and the jar and
family's bowl are food items from **Search > Loot pool** (by the Haven's food keywords). Without a
person prefab the house is data only (nothing to deal with, no host on the road) and its
questlines don't start; without a fight prefab the guest rule is a narrow escape and "by your own
hand" can only be talked; without food in the pool there are no jars. Check **Long Table > Trade
keywords** against your ammunition and medicine. Its two questlines' stage people need the generic
object's set, like the wanderers.

**New-player guidance, the power grid and the radio station** work with no prefabs: tips use the
people and radios that exist, scenario clues fall back to the note and body, and the control
object and console fall back to the puzzle prefabs. **The wind-up radio's prefab name must
contain "radio"** (or it needs a radio component), or it isn't counted as a radio.

**World interactions.** Everything works without prefabs (spots fall back to the generic object,
which is visible), but **nothing lies in houses until the loot pool is filled**, and each feature
whose output is empty stays quiet. Keep the interaction spot's box small and off the character
collision layer, so it never blocks a door, a doorway or a player. Name parts with "battery",
"tyre" and "spark", and the home remedy not with "vodka" or "herb".

**Door locks need no prefab.** The vanilla door action is modded (`Interactions/RZ_DoorHooks.c`),
so vanilla doors refuse players by themselves. If they don't, see the door rows in
[the VERIFY list](#6-the-verify-list); turning **Search > Locks** off unlocks everything (there's
no lock anywhere else). AI need more (how it works is in [DESIGN.md](DESIGN.md#locks-and-keys)): the
optional `Interactions/RZ_AIDoorHook.c` and the doorway blocker.

**The doorway blocker** (`RZ_DoorBlocker.et`). Players must never see or feel it, and it must
stop characters:
1. A box collider about **1.3 m wide (X) × 2.3 m high (Y) × 0.1 m deep (Z)**, origin at the
   box's centre. The script stands it in the doorway, 0.1 m outside the door leaf, with Z
   pointing out of the wall. Make it as a collision-only model (an FBX with just a `UCX_` box,
   no visual mesh) on a `MeshObject`, plus a static `RigidBody`; no material, no shadow.
2. **Collision layer:** one that blocks characters and counts for the navmesh, but not
   projectiles, fire geometry, view or interaction traces (so bullets pass and the door's
   action and Unlock spot still work through it). Copy the layer preset of a vanilla invisible
   wall or AI-only obstacle if your version has one.
3. `RplComponent`, so clients collide with it too. Nothing else: no actions, no hit zones.
4. Test it: place one in a doorway in the editor. You walk into it, a shot passes through,
   and the door's Open action still shows from both sides.

Leave **Navmesh rebuild** on unless a rebuild proves too slow (watch the server frame time when
a blocker comes or goes); off, AI walk into the blocker and stop there.

**The loot pool** (Search > Loot pool) is one list of every item that can lie in a house or turn
up in a car boot or on a body. Each building type takes the items whose prefab names contain its
keywords (the lists are in `RZ_Search.TypeWords`). To control a type exactly, add a **Loot table**
for it.

**Loose loot: by hand in Workbench.** Nothing new to build: **Interactions > Loot** comes with its
defaults. Check that everything that can lie in a house is a real item that can be picked up (an
`InventoryItemComponent` and vanilla's pick-up action): every loot pool and loot table entry, the
treatments (`Prefabs/ReforgerZ/Cure/`), the car parts and **the key prefab** (`RZ_Key.et`, built on a
notebook prop: if a key lying on a table shows no pick-up prompt, rebase it on a small vanilla item
and keep `RZ_KeyItemComponent` and `RZ_ReadKeyTagAction`). An entry that isn't an item still
spawns, but nobody can pick it up and the building never counts as emptied.

### Camp life: the compositions and kits

How camp life works is in [DESIGN.md](DESIGN.md#camp-life). It needs nothing to run: with no
kits every place stays bare, and with no waypoint slots its people walk to their spots with the
**Zombies > Move waypoint** and stand there. The plugin fills the four vanilla waypoint slots and
the group prefab (section 4, *Camp life*). Everything else is built from one file.

**The source: `tools/camp_kit/pieces.py`.** It holds the 49 camp-life pieces (each piece's props
with their position, turn, height and tilt, and its seats), the six kits (faction, place, pieces in
order with count and ring), where a piece stands by a building (`AT`, written as each kit piece's
**At**; the rest are Auto) and the Traces compositions (section 4, *Traces*). To change a piece,
edit it there and run:

| Command | Does |
|---|---|
| `py tools/camp_kit/build.py` | Writes `Prefabs/ReforgerZ/Dressing/<Faction>/RZ_Dress_<Name>.et` and `Prefabs/ReforgerZ/Traces/<Name>.et` (existing `.meta` files are kept, so GUIDs stay), the kits and the smart-action waypoint on `RZ_GameMode.et` (**Camp life > Kits**, **Smart action waypoint**) and the composition slots of **World > Traces > Kit** |
| `py tools/camp_kit/render.py` | Draws [designs/camp_plans.html](../designs/camp_plans.html): a top-down plan of every piece, a camp concept per faction and the faction bases' schematic plans (from `tools/camp_kit/bases.py`, drawings only) |

A new prop needs its ResourceName: pass `--index` with the resource index the Workbench plugin
**ReforgerZ: Write resource index** writes ([PC_SETUP.md](PC_SETUP.md#how-claude-drives-the-pc)).
`build.py` adds it to `tools/camp_kit/props.txt` as a `GenericEntity`; check its entity class and
whether its prefab already has a `Hierarchy` component (**ReforgerZ: Dump prefabs and classes**) and
correct the line (`Class {GUID}path`, plus ` hier` if it has one: a child can't take a second
`Hierarchy`, and Workbench logs "component Hierarchy cannot be combined" when it does).

**What a piece is.** An empty root entity with an `RplComponent` (so clients see it when it spawns
at run time) and a `Hierarchy`, the props as children, and a **seat** for each person who can use it:
a **direct child of the root** (vanilla looks for smart actions on the entity and its children, one
level down) with `SCR_AISmartActionSentinelComponent`, its **Tags** holding the use's tag
(`RZ_Sit`, `RZ_Work`, `RZ_Watch`, `RZ_Gather`, `RZ_Lean`, `RZ_Cook`, `RZ_Wash`, `RZ_Chop`), **Look
position** where they face and **Loiter animation** the pose; a watch seat looks over 120°. A
piece's front is its +Z. Keep footprints small (under about 4 × 4 m, a large tent 6 × 4 m) and
nothing with physics. Decals (the red hand, the bear stencil, the plague doctor, chalk) and plain
mesh copies of items aren't made yet: those spots use a plank board or are left out.

**Never use** anything from `Prefabs/Props/Military/AmmoBoxes`, `.../SupplyBox`, `.../Arsenal` or
`Prefabs/Props/EmptyStorages/` (`USSR_AmmoBox_empty.et`), vanilla subcompositions with "Ammo",
"Supply", "Fuel" or "Arsenal" in the name (`CamoNet_PalletFuel_US.et`), Conflict base compositions
(they carry services, radios and spawn points), field hospitals or medical tents with a heal
station, fuel tanks, pumps and jerrycans, wells and water pumps (`Well_01.et`), radio stations
and relays with actions (`Table_RadioStation_FIA_01.et`, `RadioStation_ANPRC77_01.et`), the
crafting `Workbench_01.et`, and any ReforgerZ prefab (the notice board, the generator, the
lab). Not the crafting `Props/Industrial/Workbench_01.et` as a bench either. The guard deletes any part with an arsenal, inventory storage, a pick-up item, a support
station, supplies, fuel, a vehicle or vehicle spawner, a spawn point or a ReforgerZ component or
action, and logs `Dressing: removed ...`. A generator or crate whose **root** has one is refused
whole. Check `GeneratorPortable_US_01_Off.et` and the crate: if the log names them, use a plain
mesh copy.

**Test it.** Show the pieces in Workbench with the plugin **ReforgerZ: Show the next composition group** ([PC_SETUP.md](PC_SETUP.md#how-claude-drives-the-pc)), or place each once in a test world, and check the log is silent (no
`Dressing: removed` lines); then walk up to an outpost of each faction in Workbench play (the
play-test checklist, *Camp life*).

**Who takes jobs.** Guards that are AI groups take them as they are. A lone character joins an
empty group from **Camp life > Group prefab**. The occupant and civilian prefabs are characters
without AI today: they keep standing where they spawn until they're based on an AI character.

### Traces: the Workbench spike (phase 0)

Traces ([TRACES.md](TRACES.md)) runs with nothing set: every door's words come when you stand at it.
How a marked door **looks** waits on this spike (decision 1: text first, spike decals, then decals;
boards only if the spike fails). One evening, in this order. The prefab rows are in section 4
(*Traces*); build in `Prefabs/ReforgerZ/Traces/` with camp life's rules (an empty root with an
`RplComponent`, vanilla props as children, nothing with physics, no storage, ammo, supplies, fuel,
wells, pumps or radios with actions).

| # | Check | If it fails |
|---|---|---|
| 1-3 | Decals: one on the right building beside the door, the atlas cell and the age's alpha (steps 3-5) | Boards, else text |
| 4 | A lit lantern or candle-jar prefab spawned at run time lights on clients that stream it in | No lanterns: empty **Kit > Lantern, lit** |
| 5 | Lit windows: a client light 1.5 m inside a door shows at the windows, not through the walls | Leave **Lit windows** off (the default) |
| 6 | A looped sound started on stream-in plays until the prop goes, and stops with it | Empty **Kit > Radio hiss** and **Generator hum** |
| 7 | Static wrecks, sandbags and planks spawned at run time don't trap AI on a verge | Smaller props; empty the column's cars (F1 is text only) |
| 8 | The dressing guard passes every Traces prefab (no `Dressing: removed` line) | A plain mesh copy of the prefab |
| 9 | The bell's one stroke carries to 1.2 km | A shorter sound; the line goes to players within 1.2 km anyway |

1. **A test atlas.** Make `RZ_DoorMarks_atlas.edds` as a 4 × 4 grid of plain shapes (a number in each
   cell is enough for the test), and a decal material `RZ_DoorMarks.emat` on it (copy a vanilla decal
   material, e.g. a bullet hole's, and swap its texture). Set it as **World > Traces > Kit > Door marks
   atlas**. Leave **Door drawing** on *Decals*.
2. **A marked door near you.** Turn **World > Traces > Debug** on, play on the repo's world, walk into a
   town and type `/traces write bear` (or `x`, `hand`, `beak`). The notice names the door
   (`<town>#<slot>`); `/traces` lists it with its grid in the log.
3. **VERIFY 1, the decal.** Walk to the door: within 70 m a decal of cell 6 (the bear) shows on the
   wall beside the door, 1.5 m up, not on the leaf. Walk past 80 m and back: it goes and comes back,
   and nothing piles up (one decal, not many).
4. **VERIFY 2, the building.** Try three buildings of different sizes (a house, a shop, a barn): the
   decal lands on the right building every time. If it lands on nothing (no decal) or on the ground,
   note which: the fallback is to send the wall point and normal only (a code change).
5. **VERIFY 3, the atlas and the alpha.** `/traces write x` on a second door: the X's cell shows, not the
   bear's. Wait a season day (or set **Season speed** 4) and look again: the mark is fainter. If every
   door shows the whole sheet, the material doesn't take `atlasNum`: make one material per symbol
   (a code change) or use boards.
6. **Two clients.** With a second client in the town, both see the same marks on the same doors.
7. **The other kit items, once each.** Place the grave, the pyre and the candle jars (P2, P3, P3b)
   through play (die after 2 hours near a safehouse, finish Burn the Bodies, die as a Haven Champion),
   or spawn each prefab by hand once: the log has no `Dressing: removed` line, nothing on them has a
   prompt, and AI walk round them (VERIFY 8). The lit candle jars light on a second client that
   streams them in (VERIFY 4).
8. **The scenes and the sounds, once each** (the compositions are optional; check the ones you made).
   Walk the north road in from the barrier: the column's cars stand on the verge, 6 m or more off the
   centre line, and AI drive and walk past them (VERIFY 7: no wreck, sandbag or plank cuts the navmesh
   so AI are trapped on a verge). After dark at a holding, the lit lantern lights on a second client
   (VERIFY 4). At the Day 1 roadblock the radio's hiss loops while you stand there and stops when you
   walk off past 120 m (VERIFY 6). With **Lit windows** on, the light inside a safehouse's door shows
   at its windows and not through its walls (VERIFY 5; off again if it bleeds). The bell carries to
   1.2 km (VERIFY 9; spawn `RZ_Sound_Bell.et` by hand and walk away from it).
9. **Decide.** Decals pass: draw the real atlas and the overlays (`RZ_DoorMarks_overlays.emat`: ring,
   tick, strike, PAID, scratches outside, scratches inside, smears, thumbprint, grey overpaint) in the
   UI's art direction. Decals fail: set **Door drawing** to *Boards* and make the five boards. Turn
   **Debug** off again; write the result in [TRACES.md](TRACES.md) and the VERIFY list.

### Setting up the UI

Optional: without it every message is a vanilla hint, as before. How the UI works is in
[UI.md](UI.md); the widget trees and names are in [UI/LAYOUTS.md](../UI/LAYOUTS.md). Steps 1 to 4
and 6 are done in the repo (step 4's menus written as text: check them in Workbench, 4a); 5 and
6's map key aren't. The layouts are in `UI/layouts/ReforgerZ/`.

1. **The feed** (done). `Prefabs/Characters/Core/RZ_PlayerController.et` inherits vanilla
   `DefaultPlayerControllerMP.et` and adds `RZ_PlayerUIComponent` (vanilla controllers already
   have the `RplComponent` it needs). `RZ_GameMode.et`'s **PlayerControllerPrefab** points at it.
2. **The icons** (done). The icon atlas `UI/Textures/ReforgerZ/RZ_Icons.edds` and its imageset
   `UI/Imagesets/ReforgerZ/RZ_Icons.imageset`, made with `tools/icon_atlas.py`
   ([how](../UI/LAYOUTS.md#importing-the-icons)). **UI > Icon imageset** is set to it.
3. **The HUD** (done). `RZ_Hud.layout` and `RZ_NoticeCard.layout` are built. The player
   controller's `SCR_HUDManagerComponent` has an info display of class `RZ_HudDisplay` with the HUD
   layout, and **UI > Notice layout** is set.
4. **The menus** (done as text; not checked in play yet). The notebook's frame
   (`RZ_NotebookMenu.layout`), its five page layouts, the notice board's layout and the two row
   layouts are built, with Buttons for the mouse; **UI > Mission row layout**, **Radio row
   layout** and the five **page** layouts are set on `RZ_GameMode.et`. Two overrides of vanilla
   configs (same GUIDs) in `Configs/System/` add the rest: `chimeraMenus.conf` the presets,
   `chimeraInputCommon.conf` the input actions ([both](../UI/LAYOUTS.md#menus)). **UI > Board
   menus** is still off (the board is a notice and notes are written with `/note`): turn it on
   once the notice board's menu works.

   **4a. Check the notebook in Workbench** (the author's steps; nothing here needs the editor
   unless a check fails):
   1. Open `UI/layouts/ReforgerZ/RZ_NotebookMenu.layout` and the five `RZ_*Page.layout` files in
      the layout editor once, so Workbench registers them (it reads the GUIDs from their `.meta`
      files). Check each page's content sits between the header rule and the footer, inside the
      safe area, and nothing overlaps the tabs. If a page looks squeezed, widen it in the editor:
      the script only looks up the names in UI/LAYOUTS.md.
   2. Open `Prefabs/ReforgerZ/UI/RZ_Notebook.et` (written as text on vanilla's carryable notebook,
      with `RZ_NotebookGadgetComponent`): it should show the component, display name Notebook and
      weight 0.1 kg. **UI > Notebook prefab** points at it.
   3. Open `RZ_ReaderMenu.layout`, `RZ_TalkMenu.layout`, `RZ_TalkReply.layout` and `RZ_Hud.layout`
      (its new `SubtitlePanel`, hidden) the same way. **UI > Talk reply layout** is set, so the
      talk screen is on (**Dialogue > Talk screen**). Workbench may want a 4 px rounded image on the subtitle backing: that's cosmetic.
      Open `RZ_CarMenu.layout` and `RZ_CarJob.layout` too: **UI > Car job layout** is set, so the
      car repair screen is on. The same for `RZ_ShopMenu.layout` and `RZ_ShopItem.layout` (**UI >
      Shop item layout** is set, so the shop screen is on), and open one quartermaster prefab to
      see **Trade** (`RZ_ShopOpenAction`) at the top of its actions.
   4. **The car picture** (done). `UI/Textures/ReforgerZ/RZ_CarSide.png` (a white line drawing on
      transparent, 1,440 × 480) is imported with the vanilla **TextureUIAtlas** preset as
      `RZ_CarSide.edds`, and **UI > Car picture** is set. Without it the repair screen's markers sit
      on a plain dark panel. After changing `src/RZ_CarSide.svg`, run
      `node tools/render_ui.js textures` and import again.
   5. Play: press N; open the pause menu and look for **Notebook** after Field Manual (pick it
      with the pad); after spawning, find the notebook in the inventory and take it in hand. If
      the pause menu shows no button, override vanilla's pause menu layout (same GUID) and add a
      button named `RZ_Notebook` copied from the Field Manual button: the script uses it if
      it's there.
5. **The spawn screen** (to do: written as text). Our own screen replaces vanilla's deploy menu
   ([UI.md](UI.md#the-spawn-screen)). `RZ_SpawnMenu.layout` and `RZ_SpawnRow.layout` are written,
   and `Configs/System/chimeraMenus.conf` has the `RZ_SpawnMenu` preset. Open both layouts in the
   layout editor once (so Workbench registers their GUIDs), then set **UI > Spawn row layout** to
   `RZ_SpawnRow.layout`: from then on the screen opens instead of vanilla's deploy menu. Nothing
   else changes: keep `RZ_GameMode.et`'s `SCR_MenuSpawnLogic` with **Forced faction** `SURV` (its
   deploy-menu handler is what opens the screen; there's no faction or group step), and locked
   loadouts are refused by the server (`UI/RZ_LoadoutLock.c`, **UI > Lock loadouts**). Empty, or
   with the preset missing, vanilla's deploy menu opens as before.
6. **The map** (done; not seen in play yet). `Configs/Map/MapFullscreen.conf` overrides vanilla's
   full-screen map config (same GUID, `{1B8AC767E06A0ACD}`) with `RZ_MapIconsUI` added to its
   `m_aUIComponents`, and **UI > Map icon layout** is set to `RZ_MapIcon.layout`, which turns the
   vanilla text markers for world events and the lab off. **The map key** (to do): open
   `RZ_MapLegend.layout` (written as text) in the layout editor once, then set **UI > Map legend
   layout** to it. It needs no config of its own: `RZ_MapIconsUI` makes it in the map's bottom-left,
   and the `RZ_MapLegend` input (**I**, pad **Y**) is in `chimeraInputCommon.conf`'s `MapContext`.

Also to do: default images for the icons the scripts never swap (the legend icons and the like).
Set each such `Image` from the imageset in the layout editor.

Each step works without the later ones: with steps 1–3 the HUD works (Workbench Play showed the
styled notice cards, the stealth visibility bars and the noise meter). Step 6 stands alone (after
step 1).

### Marker entities (no prefab needed)

Script entities you can place to override the automatic choice. Placing any of a kind turns its
automatic picking off (power and radio station: the first one wins).

| Entity | Overrides |
|---|---|
| `RZ_SettlementMarkerEntity` | settlement detection |
| `RZ_CheckpointMarkerEntity` | road checkpoints (place them on roads, inside the warning zone) |
| `RZ_SafehouseMarkerEntity` | Haven safehouses (optional name, population and food) |
| `RZ_OutpostMarkerEntity` | a fixed outpost for a faction (never dropped or taken) |
| `RZ_PowerStationMarkerEntity` | the power station |
| `RZ_RadioStationMarkerEntity` | the radio station |

## 5. Settings objects

`RZ_GameModeComponent` holds 41 settings objects and two lists. Every value has a working
default, and every default is in the manifest's `settings` (synced from the scripts). This is
what each one covers; defaults are in brackets where they matter for setup.

<!-- manifest:settings -->
| Category | Object | Field | Covers |
|---|---|---|---|
| ReforgerZ | (top level) |  | Tick seconds (5), **Player faction key** (SURV), **Wipe on start**, save seconds (120), **Season speed** (1: about 45 days, from about 2 to 20 regulars with Cure > Pace by play; research, the lab, stages, spread, territory, trucks, upkeep and the Long Table all follow it; [DESIGN.md](DESIGN.md#season-pacing)), **Day length** (4 real hours a game day, x6; 0 = the server's own clock; [DESIGN.md](DESIGN.md#the-clock)), **Keep time of day** (on: saved in state.json and set again at start) |
| ReforgerZ | Season | `m_SeasonSettings` | **Winner rules** (on), **Rule sets** (by name: Winter, Fog, Military Collapse, No Vehicles, or your own in the list), **Wipe** (On cure, the default: only after the cure; Off, or the timed After weeks and first of both, never used unless picked), epilogue (168 h, one week), minimum epilogue (56 h: a late cure pushes an After-weeks wipe back), weeks (10, timed modes only), warning (24 h), **No stand in the last hours** (off: near a timed wipe the stand is held back, [EPILOGUE.md](EPILOGUE.md#edge-cases)), **Story rules** (on: one small rule from each major that ended live, [QUESTLINES.md](QUESTLINES.md#next-season-the-story-rules)), **Hall of fame** (on) ([DESIGN.md](DESIGN.md#seasons)) |
| World | (top level) |  | Cluster cell (300 m), buildings per settlement (8), spawn point prefab, active spawn points (6), minutes between moves (20), **Loose item minutes** (30; 0 = off) |
| World | Town memory | `m_TownMemorySettings` | Loot return (120 min), looted below (0.5), loot floor (0.2), burned share (0.5), journal minutes (10) |
| World | Power grid | `m_PowerSettings` | Control prefab, region (2.5 km), spin-up (90 s), noise (10 heat/min), run hours (4), fuel and patch restore |
| World | Radio station | `m_RadioStationSettings` | Console prefab, takeover (30 s), cooldown (10 min, halved with power) |
| World | Long Table | `m_LongTableSettings` | **Enabled**, **Chance** a season has one (0.75), hunger per hour (6), take hours (3, ÷ season speed), body radius (300 m), the host on the road (on), the **guest rule** (on), **trade keywords** (ammunition and medicine) ([DESIGN.md](DESIGN.md#the-long-table)) |
| World | Traces | `m_TraceSettings` | **Enabled** (on), **Door marks** (on), doors a town (6, 8 in a stronghold's town), layers a door (3), live writes a season day (20, 2 a town), **First days** (on), **Player traces** (on), **First days' scenes** (on: the column, the gym, the church, the roadblock, the squares' notices, Rudi's shutter, the hill, the crosses and the Day 1 sheets), **Vignettes** (on), at once (12) and dealt a season day (3), **Live traces** (on) at once (24), **Sound and light** (on), **Lit windows** (off until the spike), **Door drawing** (Decals: from the kit's atlas, boards without it, else text; Boards; Text), props on the server (40) and a cluster (12), **Debug** (off: logs every door read; lets admins write a mark), and the **Kit** of optional prefabs and materials ([TRACES.md](TRACES.md)) |
| Director | Stages (list) |  | Per-stage overrides; empty = the built-in four (40, 80 and 120 hours played, spread 0.01 to 0.02 a minute; [DESIGN.md](DESIGN.md#the-director-outbreak-stages-and-spread)) |
| Director | (top level) |  | **Broadcast minutes** (25; 0 = only on first join) |
| Director | Environment | `m_EnvironmentSettings` | Night heat spread (0.25), night specials (×1.25), rain and fog thresholds, storm thresholds, storm grounds the helicopter |
| Director | Story arcs | `m_StorySettings` | Minutes between (60), start chance (0.6), repeat hours (4), the dark-side arc's limits |
| Director | Questlines | `m_QuestlineSettings` | **Enabled**, max active (2, one per faction), minutes between (30), start chance (0.5), retry hours (6), once per season (on), **Wait scale** (1; 0.1 for a quick test) ([DESIGN.md](DESIGN.md#questlines)) |
| Director | Majors | `m_MajorSettings` | **Enabled** (on; needs Questlines on), lead points (5), grace sessions (1), hold limit (1), stall points (1), between beats (4), slots (2) ([DESIGN.md](DESIGN.md#the-majors)) |
| Director | Finale | `m_FinaleSettings` | **Enabled** (on; needs Questlines on; off: research runs to 100 and the cure goes to the most season Fame), route worth (0.25), floor (150), ceiling (0: none), soft line (1.1), batch real hours (12) and play hours (2), seal call minutes (60), seal hour (-1: off), take seconds (90), take per point (5), take floor (45), overtime (180 s), clean room radius (10 m), bay seconds (120), **Contest** (Auto, Force, Tally), Coat Run, Take the Lab, pacts (on), groups at the lab (2), fight memory days (2), active lock days (2) and hours (2), Fame leader rule (on) ([FINALE.md](FINALE.md)) |
| Director | Epilogue | `m_EpilogueSettings` | **Enabled** (on), **Names** (Full, Titles only, Off), notables (12), towns (8), since-the-cure beats (20), the reel (on), **The world settles** (on), **Closing work** (on; off below 24 epilogue hours), **Walk Out** (on), empty server ends early (on), archive last entries (off), people per scene (6), **Test scale** (1; 0.008 runs a week in about 81 minutes), and the optional board, candle and table prefabs ([EPILOGUE.md](EPILOGUE.md)) |
| Zombies | Zombie settings | `m_ZombieSettings` | Common, special, horde prefabs, Move waypoint, **Empty group** (runtime groups), per-size spawns (4), activation (600 m), despawn (1.1 km), min distance (120 m), caps (60, 4 per tick), ambient per player (2), kill infection (1.5), horde heat (80), horde gap (600 s) |
| Zombies | Adaptation | `m_AdaptationSettings` | Specials per breakthrough (0.05, cap 0.15), trait prefabs, trait share (0.15), screamers, favourite spots |
| Zombies | Stealth | `m_StealthSettings` | **Enabled** (off: infected perception is never touched), update (1 s), radius (60 m), cap (40 an update), the visibility weights (stance with prone 0.5, speed, light by the clock and from the engine, fog, rain, storm, the surroundings floor 0.55, noise), base factors (common 1, special 1.2, horde 0.9), perception factor (×0.3 hidden, ×1 at normal visibility, up to ×1.2 in plain sight), hidden below (0.35: the HUD and losing them), losing them (8 s, search 45 s, contact 20 m), mission sight (0.6) and notice (0.35) ([DESIGN.md](DESIGN.md#stealth)) |
| Zombies | Tribes | `m_TribeSettings` | **Enabled** (on; off: no land grid, no tribes, and the countryside's roamers and heat hordes as before), **Slots** (0 = from the land), wild km² per slot (5), the **agent budget** (100 living infected, counted in agents; also the crawlers'), cluster share (28 within 600 m), agents per tick (8), **active-AI limit** (160, for all AI; 0 = the engine's 128), land squares read a second (20), the **role prefabs** (alpha, guard, hunter, scout, young; empty lists fall back), **Day party share** (0.2; 0 = none), day party group (2-3), day party bodies near a cluster (6), **Counted night minutes** (20), **Scout sight** (120 m), **Your dead run** (Described), **Life cycle** (on), hunger a counted night (3), split at (24), the looks' prefabs, **People** (on: words, knowing, the packs' contracts and events, the moat, the call box, the stand's waves, the last week), **Name filter** (words), the **ID card**, **tag** and **call box** prefabs, the map of the crews (15 Favor) and the tag frequencies (25 Favor), **Great one** (on: the great one, the Gathering, the Night of Many Calls), **Bright moon** (0.85; 0 = off), and the optional sign prefabs: kill-site things (a list), the abandoned lair, the four marks and the two call sounds (empty: the lines only) ([INFECTED.md](INFECTED.md#tribes), [People](INFECTED.md#people), [The great one](INFECTED.md#the-great-one), [Signs and sounds](INFECTED.md#signs-and-sounds)) |
| Zombies | Behaviour | `m_BehaviourSettings` | **Enabled** (on; off: tribe groups take held waypoints and Bacon's own senses), steer factor (0 = blind), contact radius (8 m), take-back multiple (2.5), sight checks a second (12), **notice rate** (0.15, tune in play), hunter sight (220/190 m), shot hearing (170 m), spacing (3 m), flank radius (140/100 m) and angle (70), stalk distance (115/75 m), **Strategies** (on: rush, stalk, pincer, encircle, flush, bait, herd, ambush), **Own attacks** (on: the hunter's lunge, the young's leap, the alpha's charge), **Client effects** (on: the hold, the stagger and the knockdown; needs `RZ_PlayerEffectsComponent` on the player controller), hold seconds (3), hold kicks (3), **Young sprint** (on), young sprint seconds (7) and young rest seconds (5), **Weak door words** (Shed,Barn,Garage,Wood; empty: no door breaks), order gap (5 s), orders per 5 s (40), special move gap (10 s), **Climb limit** (1.4 m grab height), unstick out of sight (on), LOD bands (60,150,300 m), debug log, **Crawler life** (on), **Sleep lying crawlers** (on), **Crawler speed cap** (0 = Bacon's speed; see the lab's slow-motion note) ([INFECTED.md](INFECTED.md#how-they-move-and-hunt)) |
| Zombies | Crawlers | `m_CrawlerSettings` | **Enabled** (on; off: towns as before), **Prefabs** (Bacon's civilian characters; empty = Common), bloated prefabs and share (0.05, towns at 70+ from Overrun), group size (1-3), kill ledger half-life (30 played minutes), reinforcement distance (200 m), drift radius (300 m) and seconds (90), stir (stirred at 50, -20 an hour, 5 heat a minute up to 60), siege minutes (10), scavengers (2-4) ([INFECTED.md](INFECTED.md#towns-the-crawlers)) |
| Zombies | (top level) |  | Presence heat (0.5/tick), vehicle heat (+4/tick) |
| Backgrounds | Backgrounds (list) |  | Overrides; empty = the built-in 13. **Bite minutes** (60, once the bite takes; hardcore 40, casual 90) |
| Cure | (top level) |  | Lab, sample, antidote prefabs; research per sample (1%), mission research (×0.75), **Cure pace** (0.3: about 45 days), **Pace by play** (12.5 player-hours a day: research ×(12.5 ÷ the last week's player-hours a day), ×0.25–×2.5, so 2 to 20 regulars all take about 45 days; 0 = off), **Lab research per day** (3%, real time; 0 = no limit), **Lab backlog days** (4, a backstop; full: samples refused, mission research lost; 0 = no cap), **Empty lab bonus** (0.25, on what fills the backlog to half a day of work), **Finale fallback** (1% a real day after a week at 90%, only with a finale questline) ([DESIGN.md](DESIGN.md#cure-research-and-the-seasons-end)) |
| Missions | (top level) |  | Reward crate, mission item, drop points and the optional mission prefabs |
| Missions | Events | `m_EventSettings` | Fog Bank radius (700 m), heat (×0.6) and spawn distance (×0.5), mist prefab; Power Surge minutes (10); Gas Station Fire infection drop (8); Burning Town max buildings (8) and drop per building (3); Tunnel Rats infection cap (20); convict, sniper and rival group prefabs; truck wreck, camp and hatch prefabs; Remnant Camp Infamy (10); wedding guests, bride and jewellery prefabs, **jewellery keywords** |
| Missions | Errands | `m_ErrandSettings` | **Enabled** (on), the offer gap (8 min), errands held (2), cards a day (2), pay scale (1), the Fame ceiling (39) and a faction's errand Fame a season day (10), the amends cap (8), Favor an hour (40), lapse (45 min), the fault chance (0.25), call-outs (on), wanderers' asks (on), strangers (on), found hooks (0.2 of job sites), dark cards (on), twist chance (0.2), board asks (on), radio asks (on, 60 min apart a player), the "Most people helped" line (on), telemetry (30 min), debug log (off) |
| Missions | Story | `m_MissionStorySettings` | **Enabled** (the story kit: who decides, who benefits), the **reason lane** (on, 30 min apart, within 3 km of the reason), the one weight clamp (x0.75-1.3) |
| Missions | Contracts | `m_ContractSettings` | **Rough areas** (500 m circle; 0 = exact) and reveal distance (500 m); Max escorts (3), escort leash (150 m), body radius (300 m) and speed (1.3 m/s), escort prefabs; max captures (2), cage prefab, capture radius (4 m), restrain share (0.3), specimen wake minutes (12); bus prefab and **bus keywords**; Blood Drive cost (0.25) and floor (0.7); bounty target, Bastion squad, crashed drone and smoke prefabs |
| Puzzles | (top level) |  | The puzzle prefabs, **Scale counts items** (off), landmark name |
| Perimeter | Perimeter | `m_PerimeterSettings` | **Enabled**, warn (300 m) and kill (100 m) distance, grace (8 s), fit to land and sea margin (200 m), **Override** rectangle, drone height, **Checkpoints**, guards (2), max (6, at most 8), inset, spacing, warn (150 m) and kill (60 m) radius, mission prefabs |
| Haven | Safehouses | `m_SafehouseSettings` | **Enabled**, count (3), start population (4) and food (12), visible radius (300 m), upkeep (180 min), people per food (5), **Food keywords**, Favor per food (2), heal cost (15), house-rule radii and Infamy, caught chance (0.3 a minute in Final), declare multipliers by tier (1.15/1.25/1.4/1.5), meds cost (5), stragglers (4 h), leak chance (0.25), **notice boards** (on, 8 notes, 160 characters, one every 10 min, 3 days, **note filter**), body prefabs |
| Wanderers | Wanderers | `m_WandererSettings` | **Enabled**, min and max count (6, 10), prices, spawn radius (400 m), walk speed, replace minutes (1440), bite chance, Infamy, body prefabs, Trader goods |
| Leaders | Leaders | `m_LeaderSettings` | **Enabled**, quartermaster is leader (on), interim posts (1, territory off only), body radius (200 m), replace minutes (120), job minutes (30), danger scale, discovery, kill Infamy, body prefabs |
| Territory | Territory | `m_TerritorySettings` | **Enabled**, outposts per faction (0 = by map size), held share (0.2), tick (45–75 min, ÷ season speed), pressure minutes (25), visible radius (300 m), guard and occupant counts, outpost quartermaster (on), guard and occupant prefabs, **Haven guard** (on: a Haven group at the lab gate in the stand after The Whole of It) |
| Territory | Supply | `m_SupplySettings` | **Enabled**, truck and crew prefabs, departures (60–90 min, ÷ season speed), unstick radius (1 km), stuck seconds (90), stock (8), Haven food (6), drain (45 min), out-of-stock share (0.3), rewards |
| Territory | Faction life | `m_FactionLifeSettings` | **Enabled**, chatter (10 min), life events (35 min), truces and feuds (on), false rumour chance (0.35) |
| Territory | Camp life | `m_CampLifeSettings` | **Dressing** (on), **Routines** (on), pieces per stronghold / outpost / safehouse / field station / road post (20 / 13 / 8 / 9 / 4), pieces at most (90, spawned 4 per tick), live places (3), shuffle minutes (3), extras per live place / at most (1 / 6), the five waypoint slots and the group prefab, **Kits** ([DESIGN.md](DESIGN.md#camp-life)) |
| Guidance | Guidance | `m_GuidanceSettings` | **Enabled**, meaningful minutes (10), level thresholds (1/3/8), refresher days (30), gentle mult and minutes (0.5, 15), tip gap (90 s), loud heat (45), **Tips** table, radio minutes (12), scenarios, clue minutes (20), flyers, shout minutes (30), wanderer finds, the scenario prefabs |
| Interactions | General | `m_InteractionSettings` | **Enabled**, spot prefab, scan radius (30 m) and seconds (10), caps (10 per player, 70), bells and sirens, water items and keywords, treating others |
| Interactions | Search | `m_SearchSettings` | **Loot pool**, loot tables, chances (find 0.6: each of a building's item slots, and a car boot or body search; rare 0.1, electronics 0.1), car boot and body searches (4 s, 1 heat, a boot empty for 60 min), **Locks** and their shares (on the outside doors only), **Key prefab** (empty = no keys), force seconds (10) and heat (25), relock minutes (180, the reset), crowbar keywords, readable and body chances. Keeping AI out: **AI door hook** (on), **Doorway blocker** prefab, **Navmesh rebuild** (on; off = blockers only), blocker range (150 m), cap (20), cooldown (60 s), bang heat (4) and seconds (15) |
| Interactions | Loot | `m_LootSettings` | **Loose loot** (on; off = nothing lies in houses), put out within 35 m, put away past 60 m (after a minute), **refill** (60 min ÷ the town's loot level), at most 220 items lying on the server, items at most per building (house 2, shop and farm 3, pharmacy, police and garage 4; locked +1), town loot per item taken (0.015) ([DESIGN.md](DESIGN.md#loot-in-houses)) |
| Interactions | Cars | `m_CarSettings` | Car prefabs, parked prefabs, per settlement (0.34), max (6), **parts missing** (2 to 3), parts and keywords, **tool keywords** (wrench, toolkit and the like, to fit parts; empty = no tool), hotwiring (45 s, Mechanic 15 s, 20 heat, fails 0.35, Mechanic 0.1), keys (0.2 a car, 0.1 a search), siphon and pump timings, Mechanic part cost (12) ([DESIGN.md](DESIGN.md#cars-found-rare-and-hard-to-fix)) |
| Interactions | Crafting | `m_CraftSettings` | Workbench keywords, the eight recipes' outputs and inputs, extra recipes |
| Interactions | Traps | `m_TrapSettings` | Barricade, tripwire, spike trap limits and props, generators, fire and molotovs, **Fuel in your hands** (on; off = a full can anywhere on you), seconds to set it going (5) ([DESIGN.md](DESIGN.md#fire)) |
| Interactions | Secure radio | `m_RadioCrackSettings` | Crack seconds (60), listen minutes (20), line minutes (4), range (3 km), interrupts, house radio keywords |
| Survival | (top level) |  | Shot heat (3), suppressed mult (0.3), **Bite chance** (0.1; hardcore 0.15, casual 0.07), bite reach (2.5 m) |
| Survival | Infection | `m_InfectionSettings` | Bite modifiers (surrounded ×2, grabbed ×1.5, tired ×1.5, hurt ×1.5, max 0.6), **protective clothing keywords** and multipliers (jacket 0.8, gloves 0.8, helmet 0.85), **Golden window** (120 s; hardcore 60, casual 180), alcohol and antiseptic keywords, clean chances (0.7, 0.9) and seconds (6), Final stage minutes (10), the six treatment prefabs, stabiliser minutes (20), med cap (30 min), trial dose cure (0.8) and Fame (3), cut dose cure (0.5), antibiotics fail (0.3), herb keywords, brew seconds (20), **Finds** (where treatments turn up; empty = the built-in list) |
| Factions | (top level) |  | Quartermaster and hunter prefabs, hunter minutes (10; 0 = off), bounty minutes (10) |
| Factions | Hostility | `m_HostilitySettings` | **Mode** (Bitmask / Outlaw / Off), each faction's **Shoots from** (Hunted), exile armed only (on) and radius (100 m), fight radius (350 m), wanderers fight back (on) and grudge radius (80 m), **Retaliation** (on, 300 m, 180 s, 3 Infamy, cap 12/h), **Disguises** (on; **uniform**, head-piece and service-weapon **keywords** per faction, 2 pieces, checked every 10 s, blown 60 min of play, 3 catches a life, impersonation 3 Infamy (6 an hour), gate pass 10 min, **Confiscation** (on), the Red Hand's fine 10 and beating 0.2, **Colours** (on), claim in full kit (on), stand radius 1,500 m, **again looks** every 3 min, **roll call** (on), **word gets round** 30 min within 1,500 m, **vouching** (on, 5 Infamy), **the Rogue's price** (20), **kit marks** (on), issued grace 24 real hours), the three AI faction keys |
| Factions | Services | `m_ServiceSettings` | **Fire Mission** (on, 60 min cooldown, 6 infected, 50 m, 30 s, kills at most 25, not within 250 m of people), **Fence** (on, keywords, 3 Favor an item, 8 a sale, 30 an hour), **Bounty board** (on, 60 to post, 40 paid, 3 at once, 2 h), booster (80) and medic (20) prices, **Amends** (15 Favor, ×1.5 each time, 5 Infamy, 15 a life, never below 15, every 30 min), armband and ribbon prefabs |
| Factions | Checkpoints | `m_CheckpointSettings` | **Road posts** (on, 2 at once, every 15 min, 45 min each, within 2.5 km of a holding, bodies within 600 m), barrier and crew prefabs, warn (150 m) and barrier (35 m) radius, search (15 s, **contraband keywords**), run Infamy (3), toll (10), **Surrender** (on, 10 s, 8 Infamy, once an hour, 5 min hold fire), **Camps** (on, warn 150 m, gate 50 m), **Safe zones** (on, 80 m, 2 Infamy) |
| Factions | Reputation | `m_ReputationSettings` | **Names** (on: the 16 names with what each counts as; off = the tier words), **Forgiveness budget** (30 Infamy per faction per life, every route), **Credit rule** (on, from 75 Fame: new Infamy costs the same Fame), **Wildcard at posts** (on: searched at Bastion checkpoints, running it 6 Infamy; Red Hand toll ×2), threats per life per faction (3), change notice (held 30 s, 5 min apart), **Crew names** (on), **Life-stamped memory** (on: earlier lives count at half), **Parley** (on: 2 real hours a faction, closed by a squad within 1 km, alone within 150 m, the hold to 200 m or 90 s), **Settling up** (on: 10 Fame for 5 Infamy, 10 a real day), **Gate barks** and **Others hear** (on) ([REPUTATION.md](REPUTATION.md#reputations)) |
| Voice | Voice | `m_VoiceSettings` | **Season mood** (on: the broadcast and the voice bank read one ladder, and each faction's temper), **Lines** (add a voice bank line, or replace a default by its id), **Disabled** (ids to switch off), the debug **Log** (off) and **Force mood** (off) ([WRITING.md](WRITING.md#the-voice-bank)) |
| UI | UI | `m_UISettings` | **Styled notices** (on; off = the vanilla hint), notice seconds (9), cards at once (2), gap (2 s), wait (45 s), calm after spawn (4 s), queue (12), Radio tab entries (60), infection timer and noise meter (on), resync (60 s), **icon imageset**, the three row layouts, the **spawn row layout** (our spawn screen; empty = vanilla's deploy menu), the **map icon layout** (refresh 20 s) and the **map legend layout** (full while a kilometre is up to 150 units across on screen, gone at 450), **lock loadouts** (on), **board menus** (off; on = the notice board menu), **subtitles** (on, 2.5–8 s, 18 characters a second), **reader** (on, 700 characters a page), the notebook's five **page** layouts and the **notebook prefab**, the talk screen's **reply layout** ([UI.md](UI.md), [UI/LAYOUTS.md](../UI/LAYOUTS.md)) |
| Dialogue | Dialogue | `m_DialogueSettings` | The screen guard that closes the talk screen and the reader in danger: an infected within **Danger near** (6 m) or one closing in within **Danger closing** (30 m), **Open at most** (32 on the server); for talk, **Range** (6 m) and **Idle seconds** (120); **Trees** (off: conversation trees, topics and memory on the talk screen; [DIALOGUE.md](DIALOGUE.md)), **Talk Favor a day** (20 per faction), **Make it right** (on), **Opens** (on: greets that open by themselves), **Radio calls** (on: distress calls and the secure nets), waiting idle (45 s), conversations with one person (6; 16 in a scene), hearing (8 m), the room (30 m), the decide hold (5 s), a promise's hours (2) and real days (3), barks (6 m, 60 s a player, 600 s a speaker), the rumour round's chance for lies (0.15) ([UI.md](UI.md#how-the-scripts-fit)) |
| Shared | Shared | `m_SharedSettings` | **Lanes** (on: unasked contacts one per 4 min, barks 60 s and 10 min a speaker), **Callbacks** (on, 10 a player, a pool of 100, 1 km, an hour of play apart, 3 season days, 24 h in the epilogue), **Marks** (on, spawn 150 m, remove past 200 m after 10 min, pools 40 / 100 / 30 / 30, the verge 6–10 m), the **season budget** lanes (majors ±20 morale and ±8 strength a faction, ±15 / ±5 a major, ±10 / ±5 a call; contracts ±10 / ±4 a season day, loyalty floor 30; finale ±10 / ±5; epilogue ±5), the **set-piece calendar** (quiet 600 m, 2 waits) ([DESIGN.md](DESIGN.md#the-shared-engine)) |
| Map | (top level) |  | **Map markers** (on): vanilla text markers for world events and the lab, used only while the icon layer isn't set up (UI > Map icon layout) |
| Debug | (top level) |  | Status log every few minutes (on) |
<!-- /manifest:settings -->

**Keywords are guesses.** Food, water, fuel, parts, crowbars, medkits, furniture, wrecks,
buildings, uniforms, buses, protective clothing, alcohol, antiseptic and herbs are found by
lower-case words in prefab names. Check them against the mods you run.

**Server.** The mod sets the day's length itself (**Day length**, 4 real hours a game day, x6)
and keeps the time of day across restarts ([DESIGN.md](DESIGN.md#the-clock)). The infected's
design assumes about that: a night of about 85 real minutes ([INFECTED.md](INFECTED.md#hunger-and-the-night)).
Set **Day length** to 0 to keep your own server's acceleration (the engine's default is real
time), and keep it modest and fixed; don't use time-skip or sleep mods. The
defaults fit about 45 real days from about 2 to 20 regulars: **Cure > Pace by play** measures how
much the server plays and scales research to it. For one regular, more than 20, or a longer or
shorter season, set **Season speed** or **Cure > Cure pace** from the table in
[DESIGN.md](DESIGN.md#season-pacing), and check your numbers with `python3 tools/pacing_sim.py
--players N`. For a season wipe by hand, set **Wipe on start** for one restart, or delete
`$profile:ReforgerZ/state.json`. The wipe cycle (**Season > Wipe**, on cure by default, and only
on the cure unless you pick a timed mode) does it for you: when the season is over it
writes `wipe.json`, and the next restart backs `state.json` up to `state_season<N>.json` and starts
fresh ([DESIGN.md](DESIGN.md#seasons)); restart the server some time after players are told. The
last winner is kept in `$profile:ReforgerZ/last_season.json` and the hall of fame in
`hall_of_fame.json`; no wipe touches either. With **Zombies > Tribes** on (the default), the land
grid is cached in `land_<map>_<number>.json` there too; no wipe touches it. The number comes from
the terrain's heights, so a map update that changes only objects (new forests, buildings or roads)
keeps the old file: delete it and the next start reads the map again. For a season with rules, set **Season > Rule sets**
(e.g. `Winter, No Vehicles`) before the fresh season starts. Give admins `/noteclear` for the
notice boards (server admins are whoever the server config lists).

## 6. The VERIFY list

### Compile check without Workbench

`tools/compile_check.sh` compiles every script on the Arma Reforger **Linux dedicated server**.
On first run it installs SteamCMD and the server (anonymous login, about 12 GB). It copies the
repo into a test addon folder, starts the server just long enough to compile, and prints any
ReforgerZ errors and warnings. Exit code 0 means the Game module compiled. Run it before every
commit that touches scripts.

**It leaves Special Zombies out.** On a Linux dedicated server, with Special Zombies as a
dependency, ReforgerZ's scripts don't load at all, without an error (verified with this check;
the cause, how it was found and what to do are in
[MOD_STACK.md](MOD_STACK.md#special-zombies-stops-reforgerz-loading-on-a-linux-server)). So for a
real server, host on Windows or drop Special Zombies until it's resolved. The check removes it
from its throwaway copy of `addon.gproj`, and plants a sentinel script with one known warning: if
the log doesn't show that warning, nothing of ReforgerZ was compiled and the check fails. The
runtime test and the zombie lab drop Special Zombies the same way.

```bash
tools/compile_check.sh
```

Once `addon.gproj` lists Workshop dependencies (step 3 of [1.1](#11-before-you-start-project-and-dependencies)),
set `EXTRA_ADDONS_DIR` to a folder holding them; the check warns when one is missing. On Windows it
runs in Git Bash with `SERVER_DIR` pointing at the Steam server
([PC_SETUP.md](PC_SETUP.md#4-compile-check-and-runtime-test)).

What it can't do:

| Limit | Why |
|---|---|
| The Workbench plugin isn't compiled | The server never loads the WorkbenchGame module. Compile it in Workbench (**F7**) |
| No behaviour is tested | It only compiles. Nothing runs until there's a world with the game mode in it |
| Folder names must be lowercase `scripts/` | The Linux server is case-sensitive and only looks there. Windows doesn't care, so keep it lowercase for Linux-hosted servers |

### Text lint

`tools/voice_lint.py` checks every line of player text in the scripts against the writing rules;
what each check looks for is in [WRITING.md](WRITING.md#checking-text). It reads the list that
`tools/strings_inventory.py` makes. Python 3 only, no server, about a second. Run it before every commit that changes player text; it must print `voice_lint: OK`.

```bash
python3 tools/voice_lint.py                    # new findings only; exit 1 if any
python3 tools/voice_lint.py --all              # every finding, the baseline's too
python3 tools/voice_lint.py --self-test        # plants one bad line per check; each must be caught
python3 tools/strings_inventory.py --summary   # how much text there is, by channel and faction
python3 tools/strings_inventory.py --channel radio
python3 tools/dialogue_pack.py                 # the dialogue lint; writes the packs from dialogue/**/*.rzd
python3 tools/dialogue_pack.py --check         # lint, and fail if the packs or dialogue/shipped.lock are stale
python3 tools/dialogue_pack.py --lock          # add newly shipped conversation and choice ids to the lock
python3 tools/dialogue_sim.py haven.leader.list --bg nurse --tier haven=trusted   # one conversation as one character sees it
python3 tools/voice_sim.py                     # the voice bank: pool sizes and joke shares by mood
python3 tools/voice_sim.py --season speed1     # a scripted season (speed1, speed2, slow): shares by mood, repeats per window
python3 tools/dialogue_sim.py occupant.house.stove_bed --house short --chars "Mara:--tier haven=known" --chars "Tom:--bg nurse" --race
```

**Conversations** are written in `dialogue/**/*.rzd` and packed into
`scripts/Game/ReforgerZ/Dialogue/Pack/` (generated and committed; never edit them). After changing a
`.rzd` file, run `python3 tools/dialogue_pack.py` and commit the packs with it; `tools/compile_check.sh`
runs `--check` first and fails on a lint error or a stale pack. The format, the lint and the
simulator are in [DIALOGUE.md](DIALOGUE.md#the-text-format-rzd).

`tools/voice_lint_baseline.txt` holds today's findings (the check, the file and a hash of the line),
so the lint fails only on new or changed text. Fixing a line leaves a stale entry, which the lint
reports; `--update-baseline` rewrites the file from what's found now. Don't regenerate it to get a
new finding through: fix the line.

| What it shows | What it can't |
|---|---|
| Text that breaks a rule a script can check, in literals the inventory finds | Whether a line is good. Text built from pieces (`"Talk to " + name`) is checked piece by piece. Channels and factions are guessed from the call, the folder and the nearest `case RZ_EFaction.X:` |

### Runtime test without Workbench

`tools/runtime_test/run.sh` runs the game mode on the same server, in a sub-scene of Arland with
vanilla stand-ins for the minimum prefabs and a fake player (a spawned civilian the systems see as
online). It starts every mission type one at a time and ends each. Then it begins every questline
in turn, skips its waits, goes to each stage mission and ends it as won, and follows the questline to
an ending (or reports where it fizzled or got stuck). A fresh test world meets few start conditions,
so the test forces them (each logged as `RZT forced: ...`).

**Missions.** Before each mission the towns go back to a spread of infection (20, 45, 75 and 95 in
turn), whatever earlier missions did to them. A Setup that misses (a random spot in the sea, every
town of its range busy) gets up to 8 goes, as the manager would try again later (`tries=` in the
log line), and each go after the first shifts the spread by one town: on Arland 13 of the 14
settlements are lived in, so the one or two empty towns that most contracts need must pass through
every band. A mission that still doesn't start logs `RZT mission <type> why: ...`: who holds what it
claims, what else runs, how many towns are held, busy and free, the packs and its weight. The test
world sets **Zombies > Tribes > Slots** to 2, so Arland (under the wild-land floor) has a pack for the
packs' contracts; their first go is at the clock as it is, the others in the contract's offer window
(dusk for Not Tonight, dawn for Escort at Dawn; `RZT forced: the clock into ...`). An optional prefab a mission needs gets a vanilla stand-in, set before that mission and
left set: a crate for the nest (Burn the Nest, Specimen from the Nest), the helicopter wreck (Crash
Site), the dead drop, the fuel depot (Arson), the relay, the fallback landmark
(The Smuggler's Ledger) and the scale crate (The Weighing Vault); a civilian for the trader, the
bounty target (Collect the Bounty, The Sniper) and the Bastion outpost commander (Kill the Officer);
an FIA fire team for the rival group; the canteen for Plant a Tag's tag. Convoy Escort and Hijack the Convoy get a Bastion truck and
crew, Mobile Lab a Halcyon truck and quiet towns near the lab, the Locked Bunker a keypad prop.

**Questlines.** Before the first: no town overrun, the season a couple of days old, every
safehouse full and hungry, every seat but the strongholds Independent, a Long Table, a scavenger on
the roads, a Red Hand van and the clock held at dusk. Before each, the flags and state its start
reads: Walk Him Out done, Bastion short of supplies, the Last Helicopter's LZ kept, Patient Zero's
ledger solved, the farm found, the host's offer to the tester (who is made a stranger to the Haven
for it, and put back after: the offer is for someone the Haven doesn't take in), research at 80
percent and, for Authors, the final stand, stopped again after. Before each, too, every seat the
mission phase emptied is filled and every leader a finished mission still holds is let go (a season
fills a seat after **Leaders > Replace minutes**), and The Three Weeks gets a scavenger again. A
questline that doesn't start logs `RZT quest <key> why: ...` (the Red Hand's and Halcyon's seats, the
puzzle prefabs, and what its own start reads). The two majors with no built questline under them
(Every Name, Proof) are left to the majors' phase. The Well Story runs before Cargo
(Cargo's purge ending dismisses the one crew boss it needs). While a questline waits for its stage
mission, every mission that isn't a questline stage is ended, so a contract on the board can't hold
the stage's town for the hour it lasts (a real stage waits hours before it gives up; the test's
waits are scaled down). While The Three Weeks waits for its driver, the tester keeps 300 m from him
(the stage won't start with a player within 250 m). Where a stage mission ended as plain "done"
would loop, the test reports the outcome that moves the story on. The majors are off through the
mission and questline phases (The Book and Await Relief run there as their built questlines, and
nothing starts by itself).

**The cure finale.** Last, after the route walk (the cure stands the line down), the questlines'
**Wait scale** goes to 0.1 and a scripted finale runs: a commit refused below the gate; the slow
finish (the lab-alone track to 10, nobody holding the room), then a fresh finale; the gate opening the
five routes; a route asset on the lock card, dormant until the lock and played once after it; the
dialogue's `route:` and `finale:` conditions before and after the lock; no route paused at the gate;
the tester locked to the Haven and stand-in identities to Bastion and the Red Hand, with
a Bastion–Red Hand pact by tally; the Haven and Bastion racing, the Haven finishing and the others
freezing; the seal and a stand lost, the Haven's remake step making the batch again; the second stand
with a scripted take by the pact's side and the tester in the clean room running it into overtime
(20 s), the stand's HUD line naming the holder's route; the win, the controller, the winner rule, the
route's chunk and `finale.came`; and the release's terms
(`RZT finale check: <name>: OK|FAILED (...)`, then `RZT finale done: N checks OK, N FAILED`). The
five routes are left out of the questlines' phase: they only start at the gate. The stand finds who
stands in the clean room (and at the bay) by their character, and the fake player has no engine id,
so the test's patch tells it which character is his.

**The story checks.** 150 s in, from the errands' tick (`RZT_Story.c`), the mission style's and
errands' data paths run once: a truth kept for its subject, the check results, a record written and
read, a callback queued and the door mark, a fallout dealt to its participant, the dealer's context
cache, a lie found by its owner's tick, a talk scene's three options a node, mission goods' tag, the
reasons the story jobs and the majors answer, and a free player never refused a route asset
(`RZT story check: <name>: OK|FAILED (...)`, then `RZT story done: N checks OK, N FAILED`). An objection
to a hold, a truth kept after a restart, a reason starting a contract and a locked player refused need
a play (§8).

**The epilogue week.** Then, from the finale's cure, **Epilogue > Test scale** goes to 0.02 and the
week runs scripted (`RZT_Epilogue.c`): the commit, the hall's freeze, beats and phase A; phase B;
the tester bitten and dosed at the first outlet, and refused a second; two questlines begun and
listed as loose ends, one closed live by a way and one as data; a named dead person's keepsake taken
and handed in at a house, an epilogue deck card (Take a Name to the Board) true and writing a name
home, and the Last Entry; the chair's grey choice through its hold and a say at
the station; a town with crawlers cleared and taken back, the ridges and the Long Table's week
closed; processing, the broadcast's tally, the tester walking out at the barrier (then a new life);
and the wipe's lines, the names not brought home, `last_season.json`'s beats, every closing item at
its end (`RZT epilogue check:
<name>: OK|FAILED (...)`, then `RZT epilogue done: N checks OK, N FAILED`). Steps the test world can't
supply (no outlet under the terms, no town with crawlers) are logged as skipped.

**The majors.** Then the majors go on (Collapse's second evening forced): each major starts, The
Book's send-off runs live once (its scene is ended as won), and every act is walked through its
default to an ending (`RZT major <key> act <n> ...`, `RZT major <key> ENDED: ending <n>`). Then the
fallback checks: three empty days (a day's worth of questline ticks with nobody online, three times
over) fire nothing; and Proof's Day One with research just short of 25% and a fast pace is squeezed,
sends its last call, and takes its default once a session's grace has passed. Last, what the endings
leave: the story rules' strength cap and Two Tins' dose, the epilogue lines by controller, the season
tab's lines and the stand's groups by Bastion's stance (`RZT majors check:
<name>: OK|FAILED (...)`, then `RZT majors done: N checks OK, N FAILED`).

Last, it walks the player through every
settlement, the safehouses and the lab, into the Perimeter, to death and a second life. It logs
every notice the player is sent, and prints the script exceptions (there should be none), how many
mission types started and which didn't, and how far each questline got. It takes 60 to 90 minutes.
Until the Perimeter test the line leaves the fake player alone: a spot the test moves him to may lie
in the line's band (The Blind Spot's does by design), and a death there would end the life a
questline was following (A Place at the Table fizzles when its guest's life ends). It logs
`RZT perimeter: ...` once per visit instead. If the fake player dies outside the Perimeter test
anyway, it says where and respawns him.

```bash
SERVER_DIR=~/rz_check/server tools/runtime_test/run.sh
```

| What it shows | What it can't |
|---|---|
| Script exceptions on real code paths; startup and save/load; missions' Setup, rewards and cleanup; questline stages, branches and endings (by success and failure only, not every approach); the Perimeter; spawn and death; the text players get | Anything a client does (UI, menus, input, replication), real AI behaviour against a player, your prefabs and their components |

A mission or questline that still doesn't start needs a world state the test doesn't create (an
outpost of the right faction, a spot on the map, the weather). Expected on Arland: **Gas Station
Fire** starts only when the island's petrol station is in a town nobody holds, which depends on the
season's territory roll (about one run in three). The same roll decides how many towns are lived in
(11 to 13 of Arland's 14), so the jobs that need towns well clear of where people live (**Fire
Mission**, **Install Sensors**, **Bastion Shelling**, **Bust Out**, **Lead Them Away**) start in some
runs and not others. Arland's one pack is a fresh one, so the packs' contracts that need a pack's
history or a second pack are expected misses: **Bring Them Home** (a named young who was someone's),
**Break the Habit** (a habit come due), **Cull** (12 or more round a heavy, near a Bastion holding),
**Capture the Index** and **Plant a Tag** (a pack at stage 1 with its heavy; Capture also an alpha
prefab, which the test world doesn't set), **Burn the Lair** (a player who has mapped the lair),
**Stock the Pit** (two young and a Red Hand camp), **Bait the Crew** (two packs that border), **Escort
at Dawn** (the pack between two safehouses 0.8 to 3.5 km apart), and **Not Tonight** and **Track Them
Home** unless the pack is coming back to a house or has a party walking at that moment. The test world has no AI navigation (`No AIWorld`
in the log), so a spawned group has no members, and a mission built on one may end by itself early
(`ended by itself` in the log). None of this is part of the mod: the test files are copied into a throwaway addon folder.

### Zombie lab: Bacon's real AI, measured

`tools/zombie_lab/run.sh` measures what Bacon Zombies' own AI does, on the same server, in a
sub-scene of Everon with Bacon loaded. It finds open fields and wood edges far apart on the map and
runs its experiments at once, each at its own place:

| Experiment | What it measures |
|---|---|
| Runtime groups | An empty Bacon group filled with `AddAIEntityToGroup` (4 members, and 1): faction, waypoints, survival, deleting itself when its members die; a Bacon group prefab for comparison |
| Waypoints | The held waypoint behind `RZ_Util.SendGroupTo`, `RedirectGroup` and `DeleteUnit` (expect one waypoint entity for three completed orders, the same one after a redirect, and none left after `DeleteUnit`); one group for 30 minutes: a long order, 5 minutes idle, a fight on the way to a second waypoint, then one held waypoint moved and re-added for each order |
| Perception | One infected placed 25 to 200 m from a target that is still (standing, crouched, prone) or running: within 30 s, does it see it, count it an enemy, charge? By day, night and fog, in the open and at a wood edge, with perception factors 0.25 to 2; how far a standing man is seen; a target behind a hill as the control |
| Sharing, memory, gunfire | Whether Bacon's shared target brings infected that can't see you (listeners beyond a crest from one that can); how long and where a chaser keeps going after you vanish; who comes to a gunshot (a weapon-fire danger event from script, and a real shot) |
| Speeds | Move-order and chase speeds of the civilian, military, exploder, heavy and bomber prefabs |
| Health | Reading and setting health, regeneration, and what each way of removing an infected leaves (body, loose items, the group, an exploder's blast) |
| Clock | The day's real length, `SetDayDuration`, and (a second short boot) the clock after a restart |
| Frame time | The server's frame time with 0, 60, 100 and 140 agents at AI LOD 0 (half walking), then at their natural LOD |

```bash
SERVER_DIR=~/rz_check/server EXTRA_ADDONS_DIR=<folder holding BaconZombies_622120A5448725E3> tools/zombie_lab/run.sh
```

It takes about 45 minutes (`QUICK=1`: about 12, one trial of each kind; `ONLY=share,perception-core`
runs only the experiments named, listed in `run.sh`; `ONLY=behaviour` runs the behaviour tests below) and prints every result as a table, plus any
script exceptions. The infected are held at AI LOD 0, as near a player (on the 1.9 server they stay
at LOD 0 with no player at all, so this changes nothing yet). Started with `-server`, the server always
listens on UDP 2001 (it ignores `-bindPort`), so on Linux the lab runs it in its own network namespace
(`unshare -n`) and it can sit beside another server; on Windows, stop any other server first.

| What it shows | What it can't |
|---|---|
| Bacon's sight, sharing, hearing, speeds and idle behaviour against a spawned character; group and waypoint plumbing; health; server cost per agent | A real player (a client's movement, noise, camera), Bacon's behaviour under the mod's own spawner and settings, other zombie mods |

None of this is part of the mod: the lab's files are copied into a throwaway addon folder whose
`addon.gproj` gains Bacon Zombies as a dependency. The lab drops Special Zombies from that copy: with
it among the dependencies, the 1.9 server compiles none of the addon's scripts, without an error
(the lab tests Bacon only).

**Behaviour tests (B1-B20).** `ONLY=behaviour` runs, instead of the experiments above, the lab tests
for the infected behaviour layer (our code driving the infected beyond attack range; built on their
verdicts, [INFECTED.md](INFECTED.md#how-they-move-and-hunt)). They run in lanes at places far apart,
about 30 minutes in all (`QUICK=1`: about 16), and the frame-time tests run last, alone.
`ONLY=b1,b6` runs only those.
The tests mod a few of Bacon's scripted nodes and character methods (`RZL_Hooks.c`); each hook only
reads fields a test set on the character, so with none set Bacon behaves as before.

| Test | What it measures |
|---|---|
| B1 | A body spawned after its target is in reach, blind from its spawn frame (perception factor 0, Bacon's target intake closed), at 150/50/15/6 m: does it notice within 30 s? At 6 m it's then released. Control: factor 1 |
| B2 | Every hook's calls, cumulative, each minute |
| B3 | A group of 4 in a file, line and wedge, steered by per-member slots, with and without a held waypoint: how close each body gets to its slot |
| B4 | A new waypoint 150 m away during a fight; how soon they take it after the kill |
| B5 | What keeps a chase alive after the target vanishes, every 2 s; and a take-back 10 s in |
| B6 | Releasing a steered body 30 m from a target: factor back, with and without an injected share event |
| B7 | Speed caps (0.2-1) on a run order and in a chase, per prefab; SPRINT wanted in a chase; walk-and-stop with slots 10-30 m ahead; dynamic stance 0.6/0.7 |
| B8 | Spawned walls 0.85-2.4 m high: Bacon's own climb, then the climb gate at 1.4 and 1.2 m |
| B9 | A tall-walled yard (navmesh not rebuilt, then rebuilt) and a long wall, with the unstick ladder; the cost of a navmesh snap |
| B10, B11 | A target in a house with its doors shut (then opened), and upstairs: does the infected open doors and climb stairs? |
| B12 | AI LOD 2/4/6 and a 1 s think-loop wait on all agents: loops a second and order latency; AI off and on for lying bodies: wake time |
| B13 | 1,000 terrain line checks and 1,000 80 m physics traces among houses, timed |
| B14 | `SetJump`, `SetMeleeAttack` and `IsMeleeAttack` on an AI body; the melee gate; a sprint-and-jump |
| B15 | Server frame time at 0/60/100/140 agents, with half the groups steered in walking formations |
| B16 | Slots across a river and at its edge: does any body enter the water? |
| B17 | The heavy run into destructible wooden fences |
| B18 | A scripted shot at 100/170/230 m, heard by steered bodies (intake closed) and engaged ones |
| B19 | A target inside a house, an infected outside at 10-30 m: does Bacon's sight go through walls? |
| B20 | A fan of 5 and a pincer of two pairs closing on a target, written as tracks (`SVG_DIR=dir` draws them) |

They need no client, so the player effects (a hold, a stagger, a knockdown) stay untested.

### Land survey without Workbench

`tools/land_survey/run.sh` measures the land grid ([INFECTED.md](INFECTED.md#the-land)) on the
base maps, with no Workshop mods. For each of Arland, Everon and Cain it boots a sub-scene of the
map with the game mode, the map's `SCR_AIWorld` (the road network comes with it) and **Zombies >
Tribes > Enabled** on, waits for the grid, prints what it found, then boots the map again to
load the grid from its cache. It takes about 25 minutes for all three (Everon and Cain take 2 to 3
minutes to boot, and 2 to 4 more to read). Name maps to run fewer; `CACHE_RUN=0` skips the second
boots.

```bash
SERVER_DIR=~/rz_check/server tools/land_survey/run.sh            # arland everon cain
SERVER_DIR=~/rz_check/server CACHE_RUN=0 tools/land_survey/run.sh everon
```

It prints, per map: the grid's size, the reading time (the work and the longest second), its
memory, the land and the wild land (and how much each wild-land rule takes), the tribe slots at
1 per 4, 6 and 9 km², rivers, lakes, roads, every inferred bridge or ford with the nearest bridge
prefab on the map (and the bridge prefabs it didn't find), ridge posts, valley lines, forests,
fields, lair candidates and the prefabs it took for barns, sheds and the rest, a tree histogram,
the server's memory before and after, and the cache file. A picture of each grid is saved to
`<WORK_DIR>/land_survey/<map>_map.txt`. On Linux it runs the server in its own network namespace
(`unshare -n`, as root), so it can run beside the runtime test; elsewhere don't run both at once.

| What it shows | What it can't |
|---|---|
| The engine calls the grid makes, on real maps: terrain, water types, river entities, the entity query, the road network; what it costs; the cache written and read back | Maps other than the three, Workshop maps with other prefab names, how the wild land changes as holdings and safehouses do over a season (it reads one fresh start) |

### The markers

```bash
grep -rn "VERIFY" scripts/
```

There are **110** lines with the tag, all in the game scripts (one is the comment in
`Core/RZ_Util.c` that explains them; 16 are in the UI, the talk screen and the server's loadout
lock). The Workbench plugin has none left: its real
run settled them ([1.2](#12-run-reforgerz-create-prefabs)). Every name and signature question is
settled: the game scripts compile on 1.9, and each call was checked against the base game's script
source (its doc comments and how vanilla uses it). Each marker left is a **behaviour** only a
play-test can show: replication, what the AI does, what a prefab carries. The land grid
(`World/RZ_Land.c`) has none: the [land survey](#land-survey-without-workbench) ran each of its
engine calls on Arland, Everon and Cain. They're grouped by risk;
check them in this order. Their paths are under `scripts/Game/ReforgerZ/`. The [UI's](#the-ui-14)
come last.

### High risk: the core loop (2)

No fallback. If the first is wrong, nothing persists; if the second is, loose loot piles up.

| Where | Behaviour to check | If it fails |
|---|---|---|
| `Persistence/RZ_SaveData.c:207` | `JsonSaveContext` and `JsonLoadContext` write and read plain class members and arrays of them | Nothing is saved: after a restart, check `$profile:ReforgerZ/state.json` has real values in it and the world came back |
| `Interactions/RZ_Loot.c:943` | vanilla persistence (if the server runs it) doesn't save items lying in the world | Loose loot left in houses comes back after a restart on top of the new roll: walk into a house, restart, and look. If it does, keep loose loot out of vanilla persistence (or turn **Loot > Loose loot** off) |

### Medium risk: big features, with a fallback (66)

| Where | Behaviour to check | Fallback |
|---|---|---|
| `Factions/RZ_FactionRelations.c:18` | AI perception and target selection pick up a changed `DoCheckIfFactionFriendly` answer quickly (the engine calls it often) | **The biggest unknown.** Test with a Bastion-Hunted player near Bastion AI. If they never fire, set **Hostility > Mode** to *Outlaw* and give `SURV_OUTLAW` real enemies |
| `Factions/RZ_Checkpoints.c:535` | with **Dialogue > Trees** on, the road post's speaker (territory's occupant prefab for the faction, else the puzzle civilian) carries the puzzle components, so "Talk to the toll man" shows on it | No post speaker to talk to: the search and the toll still happen at the barrier as built. Give the occupant prefab `RZ_PuzzleObjectComponent`, `RZ_ClueComponent` and the read action, as the named people at holdings have |
| `Core/RZ_Util.c:749` | `SetAffiliatedFaction` on the server replicates, and AI perception reads it | Same as above. Check that the spawn screen's places, chat, groups and markers stay on `SURV`, and that nothing resets the character (the tick puts it back within 5 s) |
| `Core/RZ_Util.c:894` | `MoveInVehicle` seats an AI at once (it goes through an RPC to the character's owner, the server for AI); `PILOT` is the driver's seat | No truck leaves its yard (the log says nobody drives it). Try the vanilla "get in" waypoint, or a vehicle prefab with its crew already inside |
| `Factions/RZ_SupplyLines.c:980` | a seated AI group drives to a move waypoint, and members on foot follow it | As above. Test one truck end to end first |
| `Core/RZ_Util.c:922` | `SetWorldTransform` moves a physics vehicle with its crew | Unsticking only runs with nobody within 1 km; check truck and crew arrive together |
| `Interactions/RZ_Loot.c:178` | furniture inside buildings is found as static entities | Loose loot lies only on up to five floor points in each building's box |
| `Interactions/RZ_Loot.c:941` | a spawned item lies still where it's put: on a table top or the floor (traced), not falling through or rolling off | Items end up under the floor or on the ground outside: lower `RZ_LootAnchors.TALL` to keep to the floor, or check the item's physics against vanilla's `MRE.et` |
| `Core/RZ_Util.c:678` | `GetVObject().GetResourceName()` names the mesh of map objects that aren't prefab instances | Such objects aren't recognised as landmarks, building types or furniture; prefab instances still are, and puzzles use the fallback landmark |
| `World/RZ_Perimeter.c:264` | `BaseWorld.GetBoundBox()` covers the terrain (it may be larger; `FitToLand` trims it) | Turn on **Override** and type in the corners |
| `World/RZ_Perimeter.c:919`, `:934` | the drone attached to the player follows them on every client; `SetOrigin` on a spawned prop moves it for clients | Moved each tick instead (jumpier) |
| `Interactions/RZ_DoorHooks.c:7` | the vanilla door prefabs use `SCR_DoorUserAction` | Players open locked doors: hook the action in the door prefabs' `ActionsManagerComponent`, or turn **Search > Locks** off (nothing is locked) |
| `Interactions/RZ_DoorHooks.c:28` | the door action's `PerformAction` also runs on the server | The server can't refuse, so a client that skips its check opens the door: try a locked door from outside on a dedicated server |
| `Interactions/RZ_DoorLocks.c:63` | `SetControlValue(0, ...)` on the server shuts the door for every client | Locked doors aren't pulled shut |
| `Interactions/RZ_DoorLocks.c:75`, `:102` | doors are child entities of the building prefab, found with `GetParent()` and `GetChildren()`/`GetSibling()` | No door belongs to a building: nothing is locked and no keys are cut |
| `Interactions/RZ_Interactions.c:332` | a full fuel can held in the hands is seen on the server: as the right-hand item, the held gadget or the item in hands (`RZ_ItemHelper.HeldItem`); the same call on the client hides the prompt | No **Douse the doorway** prompt with the can in your hands: set **Traps > Fuel in your hands** off (a full can anywhere on you will do) |
| `Interactions/RZ_AIDoorHook.c:15` | AI open doors on their path through `SCR_AIOpenDoor` (the vanilla door task) | Blockers and banging still keep AI out. If AI open locked doors, move the check (`RZ_DoorLocks.AiMayOpenNear`) into the task they really use, or delete the file |
| `Core/RZ_Util.c:1046` | `HitZone.SetHealth` on the server replicates and doesn't knock the character out above zero | Blood Drive donors could be knocked out or see stale health; nothing else uses it |
| `Core/RZ_Util.c:1251`, `:1129` | worn kit is read from the loadout slots (`EquipedLoadoutStorageComponent.GetClothFromArea` per area; the head slot is `LoadoutHeadCoverArea`), never from a pack; without the component, child entities are read instead | No kit is ever detected (no claims, no colours): check a character in Bastion green is reported in the log's look lines, or turn **Disguises** off |
| `Factions/RZ_Disguise.c:1067`, `Factions/RZ_DisguiseMarks.c:680`, `Factions/RZ_Hostility.c:137` | `TryDeleteItem` on a worn piece (from `GetClothFromArea`) takes it off the character and deletes it, for every client (a catch's confiscation, and Bastion's search taking a dead man's or someone else's signed kit) | Caught players keep their kit: turn **Hostility > Confiscation** off (held, fined and blown, nothing taken); marked kit stays on at a search |
| `Factions/RZ_DisguiseWatch.c:193` | a character's world transform's forward axis (`mat[2]`) is where its body faces | If it's another axis, guards count as turned to you at the wrong angle: again looks and their noticing line come too often or never. Check by standing behind a Bastion sentry in their kit after a passed look |
| `Factions/RZ_DisguiseWatch.c:206` | a sight trace (`TraceMove`, world and entities) from one of their people's eyes to a player's chest stops on walls and vehicles, and reports the player when it reaches them (again looks within 15 m, seen putting kit on within 30 m, a Red Hand crew seeing Bastion colours in a feud within 150 m) | If it never reaches the player, again looks are only owed and never come (the gate pass, roll call and leaders still limit a stay); if it always does, walls don't hide you. Check with `again look at` in the log |
| `Factions/RZ_DisguiseMarks.c:341`, `:390` | a dead character's worn clothing stays in its loadout slots long enough to be read when retaliation sees one of their people die, and when the spawner counts an infected's kill | No piece is ever marked a dead man's or off the turned (the log's `marked` lines never appear): killed men's and the turned's kit passes as clean. Turn **Hostility > Kit marks** off to make that explicit, and keep faction keywords rare in loot |
| `Factions/RZ_Disguise.c:805` | the server knows a player's sprint (`IsSprinting`) and raised weapon (`IsWeaponRaised`) | The look's "[Running -1]" and "[A gun in your hands -1]" never show (or always do): check the tags in the look's log line |
| `Factions/RZ_DisguiseMarks.c:544` | a worn ribbon's parent is the character, and a packed one's is the pack | Someone else's ribbon in a pack closes Haven doors, or a worn one doesn't: wear and pack one in turn |
| `Factions/RZ_DisguiseMarks.c:208` | a worn piece's root parent is the character wearing it (kit marks saved across a restart) | Marks are never saved across a restart (no `kit mark put back` lines): after a restart a dead man's kit passes as clean, as before |
| `Factions/RZ_DisguiseMarks.c:920` | `TryDeleteItem` through the wearer's inventory manager takes a worn piece off (the last week's homecoming and handing back) | The piece stays on after "He's home. Mark him.": its mark is gone either way, so it passes as clean kit |
| `Core/RZ_Util.c:1215` | `SCR_InventoryStorageManagerComponent.GetItems` walks every storage, so a rifle in a backpack is found (the parley's and the knock's empty hands) | A packed rifle passes a parley: the barrier's search uses the same check, and nobody walks out to meet you |
| `Cure/RZ_Infection.c:1181` | `AddStamina` on the server reaches the player's stamina (vanilla also calls it only on the authority) | The Weakness and Final stages, nausea and the drunk remedy don't tire the player; their noise and the "tired" bite modifier still apply |
| `Zombies/RZ_Stealth.c:253` | the zombie mod's AI uses the vanilla `SCR_AICombatComponent` perception multiplier (vanilla AI applies it in `SCR_AIUtilityComponent`'s update), so a lower factor makes them slower to recognise a player; without that component, or without the vanilla utility AI behind it, the factor goes on the `PerceptionComponent` | The log says once whether the infected take a factor, and once if some never do. If they don't, or nothing changes in play, the senses part does nothing: losing them (by distance), the HUD and the missions' rules still work. Compare with **Stealth** off |
| `Zombies/RZ_Stealth.c:274` | the zombie mod's infected list players as `ENEMY` targets once they recognise them, with the time since seen and the last seen position | Infected never lose anyone they have as a target (only those without perception are judged by distance). Check by going prone out of sight of a chasing group at night |
| `Zombies/RZ_Stealth.c:200` | on the dedicated server, a player's character reports the stance the player is in | Every player counts as standing: speed, light, weather and noise still count. Check the HUD's visibility as you go prone |
| `UI/RZ_LoadoutLock.c:50` | the server asks `SCR_LoadoutManager.CanAssignLoadout_S` before it assigns the loadout a player picks (1.9's `SCR_PlayerLoadoutComponent.Rpc_RequestLoadout_S` does) | A locked pick is assigned; the spawn check below still refuses it |
| `Zombies/RZ_InfectedBodies.c:175` | a physics trace from a player's eyes (`TraceMove`, world and entities) is stopped by buildings, walls and trees, and 16 of them a server tick cost little (crawler spawns use it: no body within 300 m of a player who could see the spot) | If traces never hit, crawlers spawn only behind terrain or beyond 300 m, so towns fill thinner near players (watch `/tribes`: towns well under their base). If they show in the frame time, lower `TRACES_PER_TICK` in `RZ_InfectedBodies` (a spot counts as seen once they're used up) |
| `UI/RZ_LoadoutLock.c:67` | the server asks `SCR_BaseGameMode.CanPlayerSpawn_S` for every spawn request, with the prefab the client asked for (1.9's respawn system does) | A locked loadout can spawn: the player keeps its gear for that life but plays as the default background (`ResolveBackground`, the log line below). Check with a fresh profile: pick Soldier, press spawn (mouse and gamepad); it must refuse |
| `Core/RZ_Util.c:473` | a smart-action waypoint set from script (`SetSmartActionEntity`) sends a ReforgerZ faction group to the composition's tagged sentinel, and vanilla's sit, lean and observation behaviours run for our groups in a mod world | People walk to the piece and stand by it: empty **Camp life > Smart action waypoint** and they loiter beside the pieces instead |
| `Core/RZ_Util.c:555` | an empty group prefab (vanilla `Group_Base.et`) stays alive until a member joins, takes the character's faction, and a spawned lone character joins it and takes its waypoints | Lone occupants and single guards stand where they spawn, as before: empty **Camp life > Group prefab**. Guards that are AI groups still take jobs |
| `Zombies/Behaviour/RZ_CrawlerLife.c:226` | a crawler group's agent switched off with `DeactivateAI` (lying up by day) stays in its group and wakes with `ActivateAI` (lab B12: awake in 0.2 s, moving in 6.5 s), and a sleeping body hit by a player is woken by the player near it | Sleeping crawlers drop out of their groups or don't wake: set **Behaviour > Sleep lying crawlers** off (they stand hunched instead) |
| `Zombies/Behaviour/RZ_Behaviour.c:1184` | a light bleed opened from the server on a player's arm or leg (`AddParticularBleeding`) replicates and bleeds at the usual rate (a hunter's lunge that lands) | The lunge still makes the bite chance x1.5; with no bleed, set **Behaviour > Own attacks** off if it misbehaves |
| `Zombies/RZ_PlayerEffects.c:260` | a speed cap set on the player's own character on their machine (`SetSpeedLimit`) slows them as a hold (0.25) and a stagger (0.6) should, without a server copy, and doesn't look like slow motion (lab B7 saw only AI) | **Zombies > Behaviour > Client effects** off: the lines and the built grab chance stay |
| `Zombies/RZ_PlayerEffects.c:172`, `:173` | a stance change to prone and movement controls off, set on the owner's own character, drop the player with the vanilla animation and hold them still 1.2 s, then give movement back | **Client effects** off |
| `Zombies/RZ_PlayerEffects.c:329` | the vanilla custom hint with its timer shows the hold's prompt over the HUD (on a pad too) | The hold still ends on its own after **Hold seconds**; Jump still kicks free |
| `Zombies/Behaviour/RZ_Doors.c:211` | a Bacon group with a target inside a house follows a move waypoint round to another, open door (another way in, for groups the layer doesn't drive) | They wait at the locked door until the siege cap, as before |
| `Zombies/Behaviour/RZ_Doors.c:281` | `SetControlValue` on the server swings a broken weak door open for every client | The lock is still forced (open and broken), so a player or Bacon can open it |
| `Factions/RZ_CampLife.c:904` | a composition spawned at run time (its root has an `RplComponent`) shows on every client, and its props block AI paths | Pieces only the server sees, or AI walking through tents: check from a client; if AI cut through, keep footprints small and off the paths (a navmesh rebuild as the doorway blockers do is the heavier fix) |
| `Missions/Errands/RZ_Strangers.c:1064` | a physics trace from a player's eyes (the infected bodies' sight test) is stopped by buildings, hedges and trees, so a stranger placed 60–90 m off where nobody sees the spot doesn't visibly pop in | If one visibly pops in, set **Missions > Errands > Strangers** off |
| `Missions/Errands/RZ_Strangers.c:815` | the road network's `GetRoadsInAABB` gives a junction's roads as pieces that end at it (three ends within 25 m), so junctions of used roads count as crossroads | Strangers keep the road posts' crossroads; nothing else changes |
| `Missions/Errands/RZ_Strangers.c:1188` | a dead stranger's inventory (`GetItems` on the body) shrinks when a player takes from it, so robbing one is told | No robbery is ever told; killing one still costs what it did |
| `Missions/Errands/RZ_ErrandsDealer.c:260` | a local's labels set and cleared on its body as a knower comes and goes reach clients (the same labels for everyone near: per-player labels need an engine check) | A local shows to anyone near while a knower is there, as before |
| `Missions/RZ_MissionGoods.c:6` | an item tagged as someone's goods keeps its entity through inventory moves, a body and a car's boot (the list holds entities, like the fence's bought list) | A tagged ration or carried item may count as ordinary food after a move; pay in kind can be left empty (**Props > Pay ration**) |
| `Zombies/RZ_TribeProps.c:68`, `:348` | a call sound prefab spawned on the server replicates, and `RZ_CallSoundComponent` plays its `SoundComponent` event once on each client that streams it in (the great one's call, the packs' dusk calls, the call box) | Nothing is heard: the lines still tell every call. Empty **Zombies > Tribes > Call sound prefab** and **Great call sound prefab** |
| `Zombies/RZ_TribeRest.c:987` | a dead leader's body withdrawn from the garbage system and blacklisted (`SCR_GarbageSystem.Withdraw`, `UpdateBlacklist`) stays where it fell through the vigil, and goes when it's inserted again at dawn | The body goes with the engine's usual cleanup; the vigil, its samples and its lines are unchanged |
| `Zombies/RZ_TribeRest.c:1069` | with **Zombies > Tribes > Rival faction** set, Bacon's infected fight bodies moved onto a faction the faction manager makes hostile to theirs, mid-fight, during a staged Border Night | Nothing is seen fighting: the Border Night is heard, then found, as before. Leave **Rival faction** empty |
| `Cure/RZ_CureFinale.c:1483` | the cure finale's clean room and loading bay: child entities named `RZ_CleanRoom` and `RZ_LoadingBay` on the lab prefab come back by `GetName()` through `GetChildren`/`GetSibling` on the server | Without them a land point 15–25 m from the lab (the bay 25 m on the far side) stands in: check it's somewhere players can stand and fight |
| `Cure/RZ_CureFinale.c:1513` | `CharacterControllerComponent.IsUnconscious()` is true on the server for a downed player, so a downed player doesn't count in the clean room | If it's always false, a downed player holds the room until killed: fine for a first play-test |
| `Cure/RZ_CureFinale.c:2014` | a faction group spawned at the stand fights the players whose hidden faction carries its bit ([REPUTATION.md](REPUTATION.md#what-makes-ai-shoot-you)) | The groups fight only the infected; set **Finale > Groups at the lab** to 0 if they misbehave |
| `UI/RZ_DoorMarkClient.c:77`, `:106` | Traces' door marks as decals: a small sphere query on a client at a door's mark point finds the same static building the server resolved the door from, and `World.CreateDecal` with lifetime 0 projects a static decal from the door atlas (the cell by `atlasNum`, the age by the colour's alpha) onto its wall, removed again with `RemoveDecal` ([the spike](#traces-the-workbench-spike-phase-0), VERIFY 1–3) | Set **World > Traces > Door drawing** to *Boards* and fill the kit's boards, or *Text*: the words come either way |
| `World/Traces/RZ_TraceSounds.c:46` | Traces' loops (the radio's hiss, the generator's hum): `RZ_AmbientLoopComponent` starts its looped event with `SoundComponent.SoundEvent` on each client that streams the prop in, and `Terminate` stops it when the server deletes the prop past 120 m (the spike's VERIFY 6) | Empty **World > Traces > Kit > Radio hiss** and **Generator hum**: the radio's line still comes as a subtitle |
| `UI/RZ_TraceLightsClient.c:56` | Lit windows (S2): a client-side `LightEntity.CreateLight` point light 1.5 m inside a lived-in building's door shows at its windows and doesn't bleed through the walls (the spike's VERIFY 5) | Leave **World > Traces > Lit windows** off (the default); the lanterns (S1) don't need it |
| `Director/Epilogue/RZ_EpilogueClosing.c:957` | the Walk Out: a character killed by the script with the walk-out flag set sends its player to the spawn screen, its body (and the inventory with it) deleted 2 s later, and no death is recorded ([EPILOGUE.md](EPILOGUE.md#the-closing-work)) | A death screen or a body left: set **Epilogue > Walk Out** off; names put down are still read at the wipe |
| `Director/Epilogue/RZ_EpilogueClosing.c:12` | a dynamic sphere query finds a parked vehicle at the queue (the Mechanic's *Leave a car for the queue*) | The choice never shows: the Mechanic's last goal can't finish; the other closing work is untouched |

### Low risk: contained or cosmetic (24)

| Where | Behaviour to check | Fallback |
|---|---|---|
| `Director/Epilogue/RZ_EpilogueRest.c:579` | the Walk Out's fallback barrier with no north-road checkpoint: past the terrain's edge the ground reads as water, so a bearing that runs off the map counts as the land's edge only when it doesn't | An island map gets no Walk Out (as before); a wrong read puts the barrier at the last land point 12 km out |
| `Director/Epilogue/RZ_EpilogueRest.c:677` | scene clearance: `RZ_EpiCarQuery` finds a parked vehicle within 8 m of the queue or the lab gate | Without it the scene stays where it is, beside the car |
| `Core/RZ_Util.c:3` | (the comment that explains VERIFY) | — |
| `Cure/RZ_AntidoteConsumable.c:11`, `Cure/RZ_MedConsumable.c:8` | the consumable's `ApplyEffect` also runs on the server, where the cure happens | Remove the consumable effect from the prefab: the world actions ("Inject antidote", "Take ...") still work |
| `Interactions/RZ_Traps.c:265` | the character's `OnGrenadeThrown` event fires on the server for a thrown molotov, naming the bottle (as the entity or the weapon's owner) | Molotovs never ignite when thrown: douse the doorway with a fuel can instead |
| `Factions/RZ_RepNamesThrows.c:59` | the same `OnGrenadeThrown` event fires on the server for a thrown smoke grenade, naming the grenade | Nobody is ever called Smoke by the crews; every other crew name still works |
| `Puzzles/RZ_BlinkingLight.c:178` | `SetFlags/ClearFlags(EntityFlags.VISIBLE)` hides a light and its mesh, on clients too | Remove `RZ_BlinkLightComponent` from the lamp: it's spawned per flash |
| `Core/RZ_Util.c:864` | `SCR_AIGroup.IsExpandComplete()` turns true once the AI world's spawn queue has given every member slot a living character | Every supply truck waits 90 s in its yard, then goes with whoever is aboard |
| `Core/RZ_Util.c:641` | a weapon's attachments (the suppressor) are child entities of the weapon | Suppressors aren't detected (full gunfire heat) |
| `Core/RZ_Util.c:697` | a prefab's entity source names its entity class, so `Vehicle` and its children are recognised without spawning | A vehicle outside a `/Vehicles/` folder and not in the car settings could be sold: the shop's other checks (the path, the car and part prefabs) still hold |
| `Puzzles/Missions/RZ_PuzzleMechanism.c:425` | `GetTotalWeight()` (weight plus "additional weight") includes a bag's contents | Turn on **Scale counts items** |
| `Missions/RZ_MissionBases.c:261` | the target prop has an `SCR_DamageManagerComponent` | Destroy missions (Arson, Sabotage, Burn the Nest) never finish: check the prop's damage manager |
| `Interactions/RZ_Search.c:824` | a dead infected character from the zombie mod keeps an inventory that `TryInsertItem` fills, and vanilla's inventory opens it as the body's | What the body carries lies loose beside it instead (the fallback the code takes when nothing fits): nothing is lost |
| `Interactions/RZ_KeyItem.c:95` | `GetItems()` also finds a key in a bag inside another bag | Such keys aren't found: carry them in the top-level storages |
| `Core/RZ_Util.c:952` | `GetBounds` is the model's box in the entity's own space and a vehicle's front is its local +Z | A fixable car's prompts float behind its boot instead of in front of its grille: use `boxMin[2] + inset` in `RZ_Util.FrontOf` |
| `Core/RZ_Util.c:939` | `SCR_Global.TeleportPlayer` on the dedicated server moves the player (or their vehicle) for every client | Only after the cure: a player who walks into the kill zone isn't turned back, so the stood-down line can be walked through until the wipe. Check by walking to the edge in the epilogue |
| `Zombies/RZ_Stealth.c:218` | on the dedicated server a player's illumination factor is 0 to 1 and follows the sun, the moon, lamps and their own torch | Set **Stealth > Engine light weight** to 0: the light is the clock's (day, dusk, night) and a torch no longer gives you away |
| `Zombies/RZ_Stealth.c:1172` | the zombie mod's group walks to a move waypoint put in front of its own once it has nobody in sight (vanilla groups do; a mod's own wandering may ignore waypoints) | Infected that lose you don't go and search, and don't go back afterwards; the senses still change |
| `Zombies/Behaviour/RZ_HuntPlan.c:2361` | moving a stuck AI character with `SetWorldTransform` out of every player's sight is picked up by its movement (the unstick ladder's step 4; the lab never needed it, B9) | Set **Behaviour > Unstick out of sight** off: a stuck body gives up and rejoins its group's anchor instead |
| `World/Traces/RZ_DressPlacer.c:47` | the road network's points follow every drivable road near a camp or a scene (the shared placer, camp life's and Traces') | A piece can stand on a road the network doesn't list: move the composition's footprint or drop that piece from the kit |
| `World/Traces/RZ_TraceSounds.c:471` | a camp composition's child props carry their own prefab data, so `GetPrefabName` names them (the table lamp, the generator), and their origin is their world spot | The lit lantern and the hum stand near the place's middle, as before; only a composition named for its piece (`RZ_Dress_Loudspeaker`) is found |
| `Factions/RZ_CampLife.c:2365` | a loiter waypoint (a timed defend) with a 4 m radius holds the group at the spot and uses only the smart actions within it | People wander to other seats or leave early: the next shuffle sends them back. Empty **Loiter waypoint** to use plain move waypoints |
| `Factions/RZ_CampLife.c:2445` | a cycle waypoint made from script loops the patrol for our groups (as vanilla's patrol command does) | The patrol walks its loop once; each shuffle sends it round again |

### The UI (14)

None of these affect the game: the UI only reads, and the talk, repair and shop screens run the
handlers that are already built. If the HUD or a menu misbehaves, turn **UI > Styled notices** off
and every message goes back to the vanilla hint; empty **UI > Talk reply layout** for the old
actions on people, **Car job layout** for a car's actions, **Shop item layout** for a
quartermaster's Buy actions, **Spawn row layout** for vanilla's deploy menu, **UI > Reader** and **Subtitles** off for cards. How the UI is put together is in
[UI.md](UI.md). Check the first five in one Workbench play with a pad plugged in.

| Where | Behaviour to check | If it fails |
|---|---|---|
| `UI/RZ_UIData.c:936` | `WorkspaceWidget.SetFocusedWidget` gives a `Button` in a `ChimeraMenuBase` the pad's focus (the notebook's tab or first row, the talk screen's selected reply, the repair screen's selected job, the reader's close button), so the D-pad moves from it | Pad players can still use LB/RB, up/down, A, X and B, which the menus listen to themselves; only the highlight is off |
| `UI/RZ_Notebook.c:197` | the focus survives a page swap in the notebook (it's set again after every swap) | As above |
| `UI/RZ_PauseNotebook.c:48` | the Notebook button made at run time from vanilla's pause menu button sits after Field Manual in the pad's focus column, labelled Notebook (its siblings are moved behind it with `RemoveChild`/`AddChild`) | Override vanilla's pause menu layout and add a button named `RZ_Notebook` (the script uses it); N and the notebook item still work |
| `UI/RZ_NotebookItem.c:27` | the notebook item's gadget mode change to `IN_HAND` reaches the owning client, which opens the notebook | The item does nothing in the hand: the pause menu's button and N still open it |
| `UI/RZ_Notebook.c:94` | `RemoveHeldGadget` on the owning client puts the item away when the notebook closes | It stays in the hand, harmless: put it away by hand |
| `UI/RZ_UIData.c:959` | the footer prompts (vanilla's `WLib_InputNavigationButton`) switch between key and pad glyphs in our layouts | The static footer text (`PromptsText`) shows instead when they can't be made; if they show but look wrong, hide `Prompts` in the layouts |
| `Dialogue/RZ_Dialogue.c:700` | an owner-only string RPC of up to about 2,000 characters (a talk node) arrives whole | Long greetings don't arrive: lower `MAX_SAID` in `RZ_Dialogue` |
| `UI/RZ_PlayerUIComponent.c:179` | a string RPC argument of a few KB arrives whole (payloads are capped at 6000 characters) | Long journals or standing don't arrive: lower `MAX_PAYLOAD` in `RZ_UIFeed` or trim the briefs |
| `UI/RZ_ShopMenu.c:541` | `ItemPreviewManagerEntity.ResolvePreviewEntityForPrefab` gives the preview world's copy of the item with its `InventoryItemComponent`, so the shop screen shows its description, weight and preview attributes | The details panel shows only the name and price; the picture itself (`SetPreviewItemFromPrefab`) doesn't need it |
| `UI/RZ_ShopMenu.c:590` | `PreviewRenderAttributes.RotateItemCamera` on that copy's attributes turns the item in the shop screen's picture (the inventory turns its character preview the same way), with the right stick (`RZ_TurnPreview`), left/right or a click | The picture doesn't turn; everything else on the screen works |
| `UI/RZ_Menus.c:304` | on a console, the notice board's note box (an `EditBoxWidget`) opens the platform's on-screen keyboard | Nothing breaks: the board's ready-made notes (left and right, then A) pin a note without a keyboard ([UI.md](UI.md#controllers)) |
| `UI/RZ_LoadoutLock.c:16` | with the spawn screen off (**UI > Spawn row layout** empty), what the player sees of a locked loadout: the 1.9 deploy menu leaves unavailable loadouts out when it builds its list, and greys their buttons when it refreshes | Only the fallback's look: the server still refuses to spawn a locked pick |
| `UI/RZ_SpawnMenu.c:19` | the spawn screen's hook: 1.9's `DeployMenuSystem` calls `SCR_PlayerDeployMenuHandlerComponent.Update` every frame on the local player's handler while it's ready, and `CanOpenMenu` is true exactly when vanilla would show its deploy menu (at the first spawn after the welcome screen, and after each death) | The screen doesn't open, or opens late: empty **UI > Spawn row layout** and vanilla's deploy menu comes back while it's fixed |
| `UI/RZ_MapIcons.c:189` | the `RZ_MapLegend` input (`MapContext`, **I** and pad **Y**) fires while the full-screen map is open, and Y there does nothing else (vanilla binds Y in the map only with the D-pad's left, held, to dismiss a hint) | The map key can't be hidden: it still fades as you zoom in. Bind the action to another free pad button |

## 7. Log lines to check

Every line starts with `[ReforgerZ]`. This is what the server printed on Arland with nothing
but `RZ_GameModeComponent` on a game mode (all settings at their defaults), in this order:

```
[ReforgerZ] No common zombie prefabs configured. Set them on RZ_GameModeComponent > Zombies.
[ReforgerZ] Stealth: on. Infected within 60 m see the most visible player near them (perception x0.3 to x1.2); losing them after 8 s hidden, searching 45 s: off (no move waypoint).
[ReforgerZ] Camp life: dressing on, routines on, buildings on, 0 kits, smart actions off, lone characters stand.
[ReforgerZ] No lab prefab set. Samples can't be delivered until one is configured or placed.
[ReforgerZ] Hostility: bitmask mode, only 0 of 15 variant factions (SURV_H1 to SURV_H15) in the FactionManager. ...
[ReforgerZ] Season 1 (fresh). Rules: none. Wipe: 168 h after the cure.
[ReforgerZ] World facts: season seed S, 0 canon facts, 0 season flags.
[ReforgerZ] Settlement scan: 2 named, 12 building clusters, took 83 ms.
[ReforgerZ] Perimeter: x 70 to 3913, z 488 to 3788. Warning zone 300 m, kill zone 100 m.
[ReforgerZ] Perimeter checkpoints: 4 roads come within 350 m of the edge; 0 crossings of the checkpoint line (60 m inside the edge): 0 used, 0 closer than 800 m to another.
[ReforgerZ] Perimeter checkpoints: 148 roads, 10 road ends with the sea within 40 m past them: 0 usable, 7 at a junction, 0 with the barrier in the sea or outside the line, 3 with another road within 75 m of the barrier.
[ReforgerZ] Perimeter checkpoints: 0 usable road ends at the shore: 0 used, 0 closer than 800 m to another, 0 not needed (at most 6 on a map).
[ReforgerZ] No checkpoint barricade prefab set: road checkpoints are off (the roads give 0 spots).
[ReforgerZ] Power grid: the factory outside Beauregard at grid 31-27 (picked), 14 towns in its region, dark, locked up.
[ReforgerZ] Radio station: the radio mast near Beauregard at grid 31-27 (picked).
[ReforgerZ] Haven safehouses: 2 (picked).
[ReforgerZ] Wanderers: 7 on the roads (new roster, seed S).
[ReforgerZ] Faction leaders: 5 (new, seed S). Haven 2, Bastion 1, Red Hand 1, Halcyon 1
[ReforgerZ] Faction life: morale 60/60/60/60, 0 remembered moments.
[ReforgerZ] Territory: 8 holdings in 14 settlements (new, seed S). Haven 1+1, Bastion 1+1, Red Hand 1+1, Halcyon 1+1
[ReforgerZ] Supply trucks: none (no truck or crew prefabs, or no move waypoint).
[ReforgerZ] Long Table: the F farm past T at grid G (new), N of them, open, hunger 45, suspicion 10.
[ReforgerZ] Search: 0 items in the loot pool, 0 tables set by hand.
[ReforgerZ] Locks: on the outside doors; keys are off (no key prefab).
[ReforgerZ] Door blockers: off (no blocker prefab); AI door hook on.
[ReforgerZ] Loose loot: on. Put out within 35 m, put away past 60 m, at most 220 items lying, refill 60 min.
[ReforgerZ] World interactions: on. 0 fixable cars on the map.
[ReforgerZ] Story arcs: on, running none.
[ReforgerZ] Questlines: on, 17 registered (Short Weight, The List, The Price of the Door, The Edge of Town, Every Name, Await Relief, The Census, The Last Pilot, The Book Has to Balance, Cargo, The Well Story, The Three Weeks, The Other Two, Authors, Proof, The House That Went Quiet, A Place at the Table), at most 2 at once, running none.
[ReforgerZ] Started. 14 settlements, stage Early Outbreak, cure 0 percent, state 0.
```

The infected (phase 1, [INFECTED.md](INFECTED.md)) add these, written from the code and not yet seen
in a run: before `Started`, the land grid starting to read the map (from its cache after the first
start) and the active-AI limit; once the clock runs, the day length; about 40 s later on Arland
(5 minutes on Everon) the land's summary and its tribe slots:

```
[ReforgerZ] Land survey: reading the map, 289 squares of 250 m (17 x 17). Tribes stay dormant until it's done.
[ReforgerZ] Tribes: the engine's active-AI limit raised from 128 to 160 (it covers faction AI, wanderers and trucks too).
[ReforgerZ] Clock: a game day takes 240 real minutes; the time of day is 12:00 (saved --:--).
[ReforgerZ] Land: 289 squares of 250 m (17 x 17), 6.3 km2 of land, 0.8 km2 wild; read in 39 s (...). File $profile:ReforgerZ/land_<map>_<number>.json.
[ReforgerZ] Land: the sea, 3 rivers (0.9 km), 0 lakes. ... Ridge posts 3, valley lines 1, forests 3 (N edge points), fields 0.
[ReforgerZ] Land: lair candidates N (forest N, barn N, ...).
[ReforgerZ] Tribes: 0.8 km2 of wild land and N lair candidates: under the floor (1.5 km2 and a lair), so no tribes here, crawlers only. Override: none.
```

On Everon the last line reads `Tribes: 24.4 km2 of wild land, 5 slots (1 kept for a split), 1 clan
possible. Override: none. ...`.

The tribes (phase 2) and the behaviour layer add these. At start: `Behaviour: on. Tribe bodies are
steered blind ...`. Once the land is read (or loaded): `Tribes: 2 tribes seeded (5 slots).` (on a
restart `restored`), then one `Tribes: 1 the woods north of Levie: 12 (leader Rising 1, guard 2,
hunters 6, scouts 1, young 2), hunger 30 fed, BOLD, range 800 m, lair ...` line per tribe. Through the
day: `Tribes: <place> has N groups out by day.`, `Tribes: <place> calls at dusk (...)`, `Tribes:
<place> sets out on N trails (core out ...)`, `Tribes: <place> hunts player N (N in the first wave,
PINCER).`, `Tribes: a scout of <place> called on player N.`, `Tribes: <place> breaks off (lost N of N,
waves N).`, `Tribes: <place> lost player N and goes home.`, `Tribes: <place> took <who> at <grid>
(hunger N, young true).`, `Tribes: someone from <town> ran, and joins <place> at dusk.`, `Tribes:
<place> gains N young at dusk (...)`, `Tribes: a counted night (N so far).`, `Tribes: <place> dawn
count: N of N came home.`, `Tribes: <place> lost its leader; the vigil at <grid> until dawn.`,
`Tribes: <place> has a new voice at first light.`, `Tribes: someone walked into the lair at <place>.`, `Tribes: <place> took nobody three nights running and drops a trail (N left).`, `Tribes: <place>'s hunt picks player N up again.`, `Tribes: <place> moved house (Lead Them Away), to <place>.`, `Behaviour | the infected broke a weak door at <grid>`
Every 5 minutes the status log adds `Status | tribe ...` per tribe, `Status | tribes: phase DAY,
counted nights N, town specials x1 | pack bodies spawned N, hunts N, break-offs N, scout calls N,
packs N`, `Status | samples a player-hour: packs N (guards N), over N player-hours` and `Status | behaviour: steer N, engage N, recover N, by waypoint N; hunts N (started N),
releases N, take-backs N, unsticks N, moves landed N, hand-offs N, leaps N, charges N (stumbles N),
young sprints N, holds N (kicked free N), staggers N, knockdowns N, sound leads N, blood leads N,
shrieks N, doors: went round N, broken N, regroups N, rings asleep N (woken N) |
town life: asleep N agents, slept N, woken N, herded N`.

The small rules add `Status | tribes' small rules: grounds moved to prey N, habits starved N, patrols
taken N, crossings N, starving walkers N, rivals now N`, and events: `Tribes: <place> follows its prey:
a new hunting ground at <grid>.`, `Tribes: <place> had a bad night; N of it are stragglers, home late.`,
`Tribes: one of <place> crossed in the night to <place>.`, `Tribes: <place> loses a habit: nothing comes
past its place any more.`, `Tribes: <place> burned out with N of it dead: N break off.`, `Tribes: the
factions speak of <place>'s new leader.`

The life cycle adds (with **Zombies > Tribes > Life cycle** on): `Status | tribes' life: rovers: lone
N, splinters N; vacuums N, burned N, abandoned lairs N, open taken N | borders N (old N), fights N
(staged N)`, and the tribe lines grow its leader's name and kind, look, disturbance, restless, clan,
weak days, fear places, habits, marks, bonds, drift and history. Events: `Tribes: <place>'s guard
fought at first light; <name> leads now (...), N splintered off.`, `Tribes: N of <place> splinter off
onto the roads.`, `Tribes: a lone one leaves <place> (...).`, `Tribes: the splinter from <place>
rejoins it / joins <place> / dies out.`, `Tribes: N loose ones end up with the slow ones in <town>
(+N infection).`, `Tribes: <place> broke up (...); its slot is free.`, `Tribes: a new pack forms at
<place> (from <town>, gone dark / from the taken / round a lone one): ...`, `Tribes: <place> is
restless (...)`, `Tribes: <place> splits at the next dusk (...)`, `Tribes: <place> split; the daughter
<place> has N.`, `Tribes: <place> and <place> are one clan now.`, `Tribes: one of <place>'s guard will
challenge its old leader at dusk`, `Tribes: the Clash at <place>.`, `Tribes: <place>'s old leader lost
to a challenger / saw off a challenge.`, `Tribes: <place>'s leader died of age.`, `Tribes: <place>
moves to <place> (disturbed / driven off by <faction> / ...).`, `Tribes: <place> is empty now (a vacuum
for 3 season days).`, `Tribes: <place> moves into the barns at the edge of <town> for the winter.`,
`Tribes: <place> is starving: N walk into the nearest town.`, `Tribes: a Border Night at <grid>
between <place> and <place>.`, `Tribes: <place> won a border fight at <grid> against <place> (N lost,
N found dead).`, `Tribes: <place> absorbed / merged <place>; its slot is free.`, `Tribes: <place> fears
<grid> for N counted nights (N of it died there).`, `Tribes: <place> learns a habit: CROSSING at
<grid>.`, `Tribes: <place> marks player N.`, `Tribes: a bonded young of <place> runs reckless.`, and
`Supply: the <faction> truck to <place> was taken by the infected at night, out of sight.`

The people's side adds (with **Zombies > Tribes > People** on): `Status | the packs' people: N
characters know of packs, N on Bastion's board, N tagged, moat at <place>, N named young, frenzy
<place>, Furious <place>, N epilogue beats`, and `/tribes` adds the same line and `for the epilogue:
<beat>` lines. Events: `Tribes: a frenzy night (N tribes, range xN).`, `Tribes: <place> comes to a
mission's noise at <grid>.`, `Tribes: a call box played at <grid> (N parts drawn).`, `Tribes: a take
waits: its road crosses <place> (N of 3).`, `Tribes: a take held up by <place> three times goes through
(they found a way round).`, `Tribes: the walks at the edge of town stopped; <place> goes hungry (N).`,
`Supply: the Haven truck for <place> waits for dawn: its road crosses a hungry pack's ground.`, and
after the cure `Tribes: the cure is out. Nobody new turns, and the packs run down on real hours.`,
`Tribes: <place> is starving in the last week: N lay down.`, `Tribes: <place> lost its leader after
the cure, and ends.`, `Tribes: the last call. Every leader still living calls once, and nothing
answers.`

The great one and the signs ([INFECTED.md](INFECTED.md#the-great-one)) add `Status | the great one:
<name> at <place> (clan of N), clan holding N/8 days, gatherings N, fallen N, clans locked false, many
calls due false | props N standing (N shown so far), call sounds N playing (N so far), moon 0.87`
(`/tribes` too), and events: `Tribes: <place>'s leader <name> is the great one of a clan of N.`,
`Tribes: the Gathering: N packs send a group each by <place> to the wire at <grid>.`, `Tribes: the
great one of <place> fell; its clan breaks up, no clan or great one for 8 season days.`, `Tribes: the
Night of Many Calls: every border reopens for 2 counted nights.`, `Tribes: <place>'s great one has no
clan left to answer it.`, `Tribes: a bright moon tonight (phase 0.92): ranges x1.25, out until
05:30.`

On a restart the same lines say `(save)` or `(saved)` instead of `(picked)` or `(new ...)`, the
world facts keep their seed and count what the season has made so far, and the settlement scan
doesn't run. The Long Table's line is there in a season that rolled a house
(**Long Table > Chance**); otherwise it says `Long Table: none this season (the roll).`, and once
the house is gone, `Long Table: broken up this season.` Arland's 14 settlements: its two cities, the building clusters named
after the nearest local place label (North Head, Angler Bluff, Sandbank, Heath Point) and the rest
by grid reference (`Outskirts 13-29`).

With the repo's world and prefabs (Workbench Play), the zombie, lab and barricade warnings are
gone and these lines change (on Arland, an island, no road crosses the line and its few shore
roads all branch off roads that go on, so it has no road checkpoints):

```
[ReforgerZ] Stealth: on. ... searching 45 s: on.
[ReforgerZ] Perimeter: 0 road checkpoints (from roads).
[ReforgerZ] Perimeter: no road checkpoints on this map (the lines above say why). Place RZ_CheckpointMarkerEntity entities on roads to add them.
[ReforgerZ] Supply trucks: Haven, Bastion, Red Hand, Halcyon.
[ReforgerZ] Search: 30 items in the loot pool, 0 tables set by hand.
[ReforgerZ] Locks: on the outside doors; keys are key items.
[ReforgerZ] World interactions: on. 5 fixable cars on the map.
[ReforgerZ] Hostility: bitmask mode, all 15 variant factions found.
[ReforgerZ] Player 1 asked to spawn with locked loadout 'Soldier': refused
```

Warnings worth acting on:

| Line | Means |
|---|---|
| `GUI (E): Menu preset 'RZ_NotebookMenu' not found!` (or another `RZ_` preset) | An engine line, before the `[ReforgerZ]` ones: the `chimeraMenus.conf` override (`Configs/System/`) wasn't loaded ([the UI](#setting-up-the-ui)). Only the menus are missing |
| `No common zombie prefabs configured. ...` | Set Zombies > Common prefabs (logged as an error: no infected spawn) |
| `Using N hand-placed settlement markers.` | Detection is off because markers are placed |
| `No checkpoint barricade prefab set: road checkpoints are off (the roads give N spots).` | Set Perimeter > Barricade prefab; N is how many checkpoints it would build |
| `Perimeter checkpoints: N roads come within 350 m of the edge; M crossings of the checkpoint line ...` | Roads that cross the line. N = 0 means the edge runs at sea all round (an island): the shore lines below fill in |
| `Perimeter checkpoints: N roads, M road ends with the sea within 40 m past them: K usable, ...` | Roads that end at the shore, and why the rest were left out: an end at a junction, a barrier in the sea or outside the line, or another road within 75 m of the barrier (it would close that road). Everon: about 7 usable |
| `Perimeter checkpoint at grid G: a road ends at the shore (road W m wide), N guards.` (or "a road crosses the line", "a placed marker") | One line per checkpoint built. 0 guards: no Perimeter > Guard prefab, or water behind a shore barrier |
| `Perimeter: the checkpoint barricade prefab didn't spawn at grid G, ...` | The barricade prefab is set but doesn't spawn: check its path |
| `Perimeter: no road checkpoints on this map (the lines above say why). ...` | The `Perimeter checkpoints:` lines say what was found. Place `RZ_CheckpointMarkerEntity`s on roads, or accept a line without checkpoints (Arland) |
| `Perimeter shore: N points along the edge with land within 600 m, M of them on a sea edge.` | Printed once, the first time a Perimeter mission is considered. N near 0 means the edge sits too far out to sea for missions at the line: lower **Sea margin** or set the **Override** rectangle |
| `The Blind Spot: nowhere suitable at the line on this map, so it won't be offered.` (or The Crossing, Shot at the Line) | That mission found no dry spot clear of the checkpoints near the edge, and is off for this map. Same fix |
| `AI (E): No AIWorld is present in the map.` / `Cannot spawn team members of group ..., AIWorld is missing in the world!` | Engine lines: the world has no AI world, so no AI or infected spawn. Place the map's AI world ([The world](#the-world)) |
| `No road network found: place RZ_CheckpointMarkerEntity entities ...` | The road network comes from the AI world: the world has none, or the map has no roads. Without it there are no road checkpoints (unless markers are placed) and no road posts |
| `Hostility: bitmask mode, only N of 15 variant factions ...` | Some `SURV_H` factions are missing |
| `Hostility: outlaw mode, but the FactionManager has no SURV_OUTLAW.` | Add the Outlaw faction |
| `No background matches loadout '...'` | A loadout name doesn't match a background |
| `Player N asked to spawn with locked loadout '...': refused` | The player picked a background they haven't unlocked, and the server refused the spawn (**UI > Lock loadouts**). Normal now and then; often from one player, their client lets them pick it |
| `Player N spawned with locked loadout '...': default background` | A locked loadout got past the spawn check (see `UI/RZ_LoadoutLock.c:67` in the VERIFY list): they keep its gear but play as Unemployed, the default background |
| `Shop action '...' sells a vehicle or a car part (...): refused. Cars are only found` | A quartermaster's shop action is set to a vehicle, a car part or fuel: change its Item prefab |
| `Requisition '...' (...) isn't Trusted stock here ...: refused` | A requisition action hands out something its quartermaster doesn't sell at Trusted or below: point it at a Trusted (or lower) shop item, or add one |
| `Checkpoints: no road points found near players; road posts stay down.` | No roads near where players were |
| `Dressing: no kit for <faction> (<KIND>); those places stay bare.` | Printed once per faction and kind of place, the first time one is dressed: fill **Camp life > Kits** ([the compositions and kits](#camp-life-the-compositions-and-kits)). Road posts only take a kit made for them |
| `Dressing: removed <part> from <prefab> (<why>)` | A composition carries something a player could use (an arsenal, storage, an item, a support station, supplies, fuel, a vehicle or spawner, a spawn point, a ReforgerZ part but the look-only sound components `RZ_AmbientLoopComponent` and `RZ_CallSoundComponent`): the guard deleted it (or the whole piece, if its root has it). Take that prop out of the composition |
| `Camp life: <Faction> <KIND> at <grid> moves into <the school> (<w> x <d> m), <n> road entries.` / `... no building fits, laid out on rings.` | Once per place a run, the first time a player comes near: the building camp life picked ([DESIGN.md](DESIGN.md#camp-life)). Rings where the town has no building of the faction's kind and none big enough; 0 road entries means the gate and barricades stand in front of the building |
| `Camp life: ... smart actions off, lone characters stand.` | Set **Camp life > Smart action waypoint** (people loiter beside the pieces until then) and **Group prefab** (lone occupants and guards don't move until then) |
| `No lab prefab set. Samples can't be delivered ...` | Set Cure > Lab prefab |
| `Puzzle clue prefab has no RZ_ClueComponent ...` / `Puzzle object prefab has no RZ_PuzzleObjectComponent ...` | A puzzle prefab is missing its component |
| `Mission item prefab has no RZ_MissionItemComponent ...` | The mission item is missing its component |
| `Key prefab is empty, has no RZ_KeyItemComponent or didn't spawn: no keys ...` | Set **Search > Key prefab** to an item with `RZ_KeyItemComponent`; until then no keys are given or placed |
| `Loose loot: off.` | Expected only if you turned **Loot > Loose loot** off: nothing lies in houses |
| `Supply: ... truck at ... has nobody to drive it after 90 s (...)` | Printed once per truck, while it waits in the stronghold's yard. Its crew didn't spawn, wasn't within 30 m of the truck, or couldn't take the driver's seat: check the crew prefab (an AI group on the faction) and that the truck has a driver's seat. The truck is lost as late at its deadline |
| `Supply trucks are off. ...` / `Territory is off. ...` / `Long Table: off.` / `Stealth: off (...)` | Expected only if you turned them off |
| `Stealth: on. ... searching 45 s: off (no move waypoint).` | Set Zombies > Move waypoint, or infected that lose a hidden player don't go and search |
| `Stealth: some infected take no perception factor (...)` | Printed once, the first time infected near a player have no `PerceptionComponent`: the zombie mod uses its own senses. Stealth's factor does nothing for them, and losing them goes by distance ([DESIGN.md](DESIGN.md#stealth)) |
| `Stealth: some infected have perception that never took the factor (...)` | Printed once, the first time a group's `PerceptionComponent` refused the factor 5 updates in a row (usually a combat component that is never set up). As above: their senses stay as they are, and losing them goes by distance |
| `Saved state is from another map, starting a fresh outbreak.` | Expected after changing maps |
| `Season wipe carried out (...). A fresh outbreak starts.` | The wipe cycle ran on this start; the old state is in `state_season<N>.json` |
| `Season rule set '...' doesn't exist; ignored.` | A name in **Season > Rule sets** is misspelt |
| `Questlines: the saved questline '...' (or its stage) no longer exists; dropped.` | A questline (or a stage id) was removed or renamed in the code since the save; that one ends |
| `Questlines: two questlines use the key '...'` / `... has no key or no stages; skipped.` | A content mistake in `Missions/Questlines/`: give each questline its own key and at least one stage |
| `Long Table: none this season (no empty place far enough from everyone on this map).` | Every settlement is too infected or within 400 m of a holding, leader seat, safehouse or the lab (the last of the small-map steps): no house this season. Fewer outposts (**Territory > Outposts per faction**) leave room |
| `Requirements: no questline has the key '...' (docs/QUESTLINES.md lists them); the gate never closes.` | A mission's `NotWhileQuest`/`NotAfterQuest`/`NotWithQuest` names a key that is neither registered nor designed: a typo. Use the key from [QUESTLINES.md](QUESTLINES.md#the-questlines) |
| `Save file is corrupt or from an incompatible version, ignoring.` | The save didn't load (or the JSON save behaviour in the VERIFY list is wrong) |
| `Land: ... roads: none found (no road network; no bridges) ...` | The world has no `SCR_AIWorld` (or its road network is empty): place the map's AI world prefab (`SCR_AIWorld_Eden.et` and the like), which the infected need anyway |
| `Zombies > Empty group isn't an AI group: ...` | **Zombies > Empty group** is set to something that isn't an AI group: crawlers and loose ones made from character prefabs stand on their own (Bacon's stay inactive). Set Bacon's `Group_Zombies_Base` |
| `Clock: no time and weather manager in the world; every hour reads as day.` | The world has no time and weather manager: no night, no Day length, nothing to keep across restarts |
| `Land survey: the cached grid ... doesn't read; reading the map again.` / `Land survey: could not write ...` | The cache in `$profile:ReforgerZ/` is damaged or can't be written: the map is read again at each start (a few minutes, tribes dormant meanwhile) |

While running: `Status | ...` lines every few minutes (stage, infection, zombies, missions, move
waypoints in the world (one per AI group with an order at most: a number that keeps climbing is a
leak), cure,
the lab's backlog, whether it's full, whether research waits at the finale's gate, and the pace by
play in percent),
`Cure: the lab's backlog is full; N of N research from a mission was lost.`,
`Cure: research reached the finale's 90 percent gate.` and `Cure: finale research +N (faction N),
now N percent.` (with the finale on), the cure finale's (`Finale: the gate. Five routes open; the
route's worth is N season Fame (leader N).`, `Finale: people seeded (N); the cold store at ...`,
`Finale: <route> locked: N (active N).`, `Finale: <route> +N (<why>), track N.`, `Finale: Last Mile on
<route> (track 8).`, `Finale: <route> proposes a pact with <route>.`, `Finale: pact <route> + <route>
(tally N-N).`, `Finale: <route> finished. Batch hold N real hours and N hours of play.`, `Finale: <why>
paid N season Fame (N full, N half). Season Fame: ...`, `Season Fame: <faction> +N (<why>), now N.`,
`Finale: the batch is ready (<how>). Holder <route>.`, `Finale: the stand starts. Clean room: <route>.`,
`Finale: <faction>'s group stands at the lab.`, `Finale: Take the Lab is offered to the Red Hand
route.`, `Finale: clean room <route> -> <route> (force|theft) with N of the stand left.`, `Finale:
the stand was lost (N this season). Remake from <route> or the backlog.`, `Finale: controller <route>,
how <how>, partner <route>.`, `Finale: controller <route>, how <how>, terms N, deciders N.`, `Finale:
item N (...) for <route> to <ident>.` and its drops, returns, spoils and hand-ins, `Finale: the rota
moved the woman to house N.`, `Finale: a price door paid for <route>.`, `Finale: the Coat Run passed.`
or `... was caught.`, `Finale: <route> broke its pact with <route> (...).`, `Finale: the routes' pitches
went on air at 85 percent.`, `Finale: asset <kind> for <route> to <ident> (<record>).`, `Finale: asset
<kind> played for <route> by <ident>.`, `Finale: <route> spoiled <route>'s case with <person>.`,
`Finale: <route> took <person> into custody from <route>.`, `Finale: <person> talked out of <route>'s
hands.`, `Finale: <route> drained <route>'s source (<source>).`, `Finale: N item(s) of <route> ransomed
back.`, `Finale: item N (...) of <route> spoiled on purpose.`, `Finale: <route>'s plan sold to <route>.`,
`Finale: <route> paused (no active lock for 3 real days).`, `Finale: <person> died on-screen.`,
`Finale: <route> guarded its door.`, `Finale: hand it over: <route>'s item from <name> to <name>.`,
`Finale: the losing routes' marks came down (N left).`, `Questlines: the clean room changed holder (N to N).`),
`Mission N '...' at ...`, `Outbreak stage -> ...`, `Supply: ... truck leaves ...`, `... goes with N of N crew aboard; the rest follow on foot.` and `... lost (...)`,
`Story arc started: ...`, `Questline started: ...`, `Questline ...: stage '...'.` and
`... is on (mission N).`, `Questline ended: ... (ending N, branch N).` / `Questline fizzled: ...`,
the epilogue week's (`Epilogue: the cure is out; the record is taken on the next ticks.`, `Hall of
fame: the records froze at the cure.`, `Epilogue: the record: route N, how N, terms N, faction N, N
players, N towns with crawlers, N packs calling.`, `Epilogue: beat <key> at <place> (weight W)`,
`Epilogue: N dose outlets (row N, terms N, condition N).`, `Epilogue: story built (N beats, N
players, N open items)`, `Epilogue: phase B`, `Epilogue: phase C`, `Epilogue: beat <key> since (<why>)`
and `... dropped (<why>)`, `Epilogue: last call <item>`, `Epilogue: closed <item> (live|default)`,
`Epilogue: a dose at <outlet> (N left).`, `Epilogue: brought home <name> to <place>.`, `Epilogue:
<town> taken back.`, `Epilogue: grey choice N decided (...).`, `Epilogue: walked out (N in all)`,
`Epilogue: a last entry by <name>: ...`, `Epilogue: admin <name>: ...`, `Epilogue: the Unsigned last
day (N beats).`, `Epilogue: the supper reading.`, `Epilogue: phase B holds about N evenings.`, `Story rules: the exile leak at <house>.`, `Epilogue: the 20:30 watch (N strokes).`, `Epilogue:
second airing of <key>`, `Epilogue: last_season.json keeps N beats and the broadcast (...).`,
`Epilogue: <name> went home to <place> by an errand.`, `Cure: the final stand is held back in the
season's last hours (Season > No stand in the last hours).`, `Seasons: a late cure pushes the wipe back to the minimum epilogue.`, `Territory:
<holding> stays as it is (the world settles in the epilogue).`),
the majors' (`Majors: <key> act N (...) opens.`, `Majors: <key> act N squeezed (room N h, need N h). Last call.`
or `... patience. Last call.`, `Majors: <key> act N (...) takes its default (squeezed|patience|nobody qualifies|hard stop).`,
`Majors: <key> act N held its anchor as long as it may; the default comes anyway.`,
`Majors: <key> leads at N percent toward N (research stalled 3 days).`, `Majors: <key> between beats (gap N h).`,
`Majors: <key> is squeezed and takes <key>'s slot.`, `Majors: <key> act N (...) settled, result N.`,
`Majors: <key> act N resolves at the hard stop, at half effect.`, `Majors: a page in the open: ...`,
`Majors: season story for <key>: ending N (...).`, `Majors: <key> <kind> for one player clipped from N to N (the majors' caps)`,
`Majors: <key> stage N went quiet; its act takes its default.`, `Calendar: <key>.<stage> holds evening N`,
`Story rules: N from last season (...).`, `Story rules: the black market starts with N stolen doses.`), the errands' epilogue deck (`Errands: dealt epi_<card> to ...`),
`Bastion checkpoint up near ...` / `Red Hand roadblock up near ...`,
`Long Table: ... never arrived (hunger N, suspicion N, N taken).`, `Long Table: a player heard the rumour.`,
`Long Table: suspicion N (...).`, `Long Table: a fight at the farm (...)` and `... the fight is over (...)`,
`Long Table: found out (to N).`, `Long Table: the farm past ... is moving (quiet for a real day).` and
`... opens again past ...`, `Long Table: broken up (N).`,
`Screen guard: closed talk for player N (danger).` (or `reader`, `car repair`, `shop`; `left`,
`idle`, `gone`: a screen the screen guard shut), `Status | infected N/100 agents (N entries; crawlers N, loose N), clusters N share N [N/N, ...], other
bodies N agents, hordes from towns N, loose ones arrived N | crawlers: ledger N, stirred towns N, out N, kill sites N, held
doors N | active AI N/160 | land: ... | clock ..., day N min` (with Crawlers or Tribes on: agents near the
budget all the time, or the active AI at its limit, means the budget is too high for the server),
`Heat horde: N units out of <town>, from its side of <grid>.`, `Pack horde: N groups of the pack at
<place> sent to the heat at <grid> (day, its day party | night, its hunters).`, `Pack lookers: young
of the pack at <place> sent to look at a busy spot at <grid>.`, `Lookers: N of <town>'s own sent to
look at a busy spot at <grid>.`, `Tribes: R1's rumour of the crew in the woods (...)`, `Crawlers: <town> is stirred (N).`,
`Crawlers: a kill site at <grid> (...); the slow ones from <town> come out to it at night.`,
`Crawlers: N from <town> are walking out to the kill site at <grid>.`, `Tribes dump | ...` (an admin's
`/tribes`: every town with crawlers, a ledger or stir, kill sites, parts, tracked units, last hits),
`Fire Mission perk: player N, M infected killed.`, `Cars: the ... in ... fixed by player N.`, `Stealth: the infected take a perception factor (...)` (once, the
first time infected near a player take it), `Note at ... by ...: ...`, `Season over. Cure goes to ...`,
`Canon: <key> = <value>.` (a fixed fact of the setting, made the first time content asks, once a
season; [DESIGN.md](DESIGN.md#world-facts)), `World change: <key>. <headline>` (for example
`supply.1.called_off` when a truck turns back), `Mission N '...' called off: ...` (the world made it
pointless: the cure, or its truck's post changed hands), `The cure is out: N running mission(s)
called off.`, `Perimeter (stood down): player N turned back from the line.`, and `Season N is
over (...). The wipe happens at the next restart.`

The shared engine and reputation names ([DESIGN.md](DESIGN.md#the-shared-engine),
[REPUTATION.md](REPUTATION.md#reputations)), while running:
`Reputation: <player> <faction> <name> (<tier>)` (every change of name),
`Reputation: <player> <faction> forgave N (<route>), N left to forgive` and `... forgive N refused, 0 left
to forgive (...)` (the forgiveness budget), `Reputation: <player> <faction> credit rule, -N Fame for the
Infamy (...)`, `Reputation read-out: ...` (an admin's `/rep`), `Checks: <player> threatened <person>
(pass|short|refused)`, `Callbacks: queued for ...` and `Callbacks: told ...`, `Note at <safehouse>, unsigned:
...` (a callback whose character died), `Budget: <lane> morale|strength N for <faction> clipped to N (...)`,
`Calendar: <key> holds evening N`, `... waits (...)` and `... released`, `World memory: reason ...` and
`World memory: town ...`; on a restart, `World memory: N reasons, N signals, N town logs, N world
records.` and `Shared: N callbacks waiting, N season records.`

Errands and the story blocks ([ERRANDS.md](ERRANDS.md), [DESIGN.md](DESIGN.md#story-blocks)): at
start, `Errands: on, 36 routines and 70 cards, N players with a ledger.`; while running, `Errands:
dealt <key> to <player> by <giver> (asked|called out)`, `Errands: <key> taken by <player>, way N
(<way>)`, `Errands: <key> done by <player>, way N (<way>), with N helper(s)`, `Errands: <key> for
<player> overtaken`, `Errands: faults at h12 this shift: relief, set`, `Errands: fault <kind> at
<place> fixed`, every 30 minutes `Errands | live N | asked N, called out N | <family>: dealt N,
declined N, lapsed N, overtaken N, done N, Favor N, Fame N (amends N), ways ...`, and `Errands admin
view: ...`; `Story: truth <key>.<subject> = <truth> (kept|new)` and `...; found by <how>`, `Story:
errand.<key> talk pass|short|refused by <player>`, `Story: <key> resolved by hand-in to <party>
(...)`, `Story: reason <key> (<subject>) started N '<title>' at <town>`, `Mirror: <title> hinted to
<faction>`, `Mirror: <title> won by <faction>` (or `won by both, half each`), `Story: <title> hold by
<player> objected to by <player>`.

Borrowed kit's rest: `Disguise: <player> gave <faction> a cover detail (<key>)` (or `... they don't
have (<key>): a lie`), `Disguise: a kit mark put back on <player>'s <faction> after the restart`,
`Disguise: the last week's lines (N caught this season, best-dressed ...)`. Borrowed kit and the parley ([REPUTATION.md](REPUTATION.md#borrowed-kit)): `Disguise: <player> look
by <faction> at <place>: N of D (pass|caught) <tags>`, `Disguise: <player> caught by <faction> (catch N
this life)`, `Disguise: <player> exposed to <faction> (<why>)`, `Story: <title> trick pass|refused by
<identity>`; while passing, `Disguise: again look at <player> by <faction> at <place>`, `Disguise:
<player> owed a look by <faction>: it comes in 10 s`, `Disguise: roll call for <player> by <faction> at
<place>`, `Disguise: <player> seen putting on <faction>'s kit`, `Disguise: <player> walked off while
held: ran the checkpoint`, `Disguise: Rogue <name> asked <player> for his price`, `Disguise: <player>
paid Rogue <name> N Favor (10 minutes at his seat)`, `Disguise: Rogue <name> found out: N payers
named`; `Disguise: word gets round at <holding> (a catch|a kill, new|restarted)`; `Disguise: <player>
signed <player> through for <faction>`, `Disguise: voucher <identity> answers for <player> with
<faction> (<what>)`, `Disguise: <player> pointed out to <faction> (N at this look, at most 2)`;
`Disguise: N pieces of <faction>'s kit marked as <name>'s (a dead man's)`, `Disguise: N pieces of kit
off the turned marked`, `Disguise: N pieces sold by <faction> to <player> (signed out to them|bought
fair)`, `Disguise: Bastion's search took N marked pieces from <player>`, `Disguise: <name> on
<faction>'s list (<place>, day N)`, `Disguise: <name> marked home|kit lost`, `Disguise: the nightly call
read N names`; `Disguise: <player> heard the word at the wire (day N)`, `Disguise: <player> in
<faction>'s kit can't claim: Hunted`, `Disguise: <player>'s pass with <faction> counts for nothing near
the lab during the stand`, `Disguise: <player> in Bastion colours seen by the Red Hand during a feud`,
`Checkpoints: <player> searched with the whole car at <place> (before any look)`; `Parley: <player> at <place> (<faction>) opened`, `Parley: <player> heard by <faction>
(offer: true|false)`, `Parley: <title> reserved for <player>`, `Parley: <player> at <place> broken`,
`Parley: <player> knocked at <house>`; `Reputation: <player> settled up with <faction>: -10 Fame, -N
Infamy`; `Checks: <player> threatened <person> (pass|short|refused)` (The Ledger and Debt Collection
too).

Talk and the voices ([DIALOGUE.md](DIALOGUE.md), [WRITING.md](WRITING.md#the-voice-bank)): at start,
`Dialogue: loaded 157 conversations, 346 nodes`, `Dialogue: memory for N players` (or `Dialogue:
memory from season N dropped (season M)` after a wipe), `Reputation: season record for N players` (or
`Reputation: season record from season N dropped (season M)`) and `Dialogue: trees off` (on with
**Dialogue > Trees**); `Voice: N lines in the bank` the first time anyone speaks from it; while running, `Voice:
season mood <mood> (was <mood>)`, `Dialogue: budget reached <player> <faction>` (talk Favor for the
day), `Reputation: crew name <name> for <player>`, `Dialogue debug: ...` (an admin's `/talkdebug`), and
with **Voice > Log** on, `voice: face <faction> <register> <id> mood=<mood> listener=<identity>`. Several
players: `Dialogue: scene <key> decided <value> by <player>`, `Dialogue: scene <key> hold by <player> objected to
by <player>`, `Dialogue: sway <name> by <player> (now N)`, `Dialogue: <player> vouched for <player>`,
`Dialogue: sway <major>.<vote>.<word> by <player> (voter N)` (a sitting), `Dialogue: session cap at <speaker>`.
The finale's commit: `Dialogue: <player> committed to <route> at <speaker>`.
Promises and barks: `Dialogue: promise <key> made|kept|broken by <player>`, `Dialogue: bark <conv> to <player>`.
Talk's rest: `Dialogue: make it right <player> at <speaker>`, `Dialogue: made right <player> with <speaker>`,
`Dialogue: <speaker> opened on <player> at N m` (a greet that opens by itself), `Dialogue: distress call
answered by <player>, sent to house N (bitten B)`, `Dialogue: claim <speaker>|<claim>.false tested false for
<player>`, `Dialogue: <player> began <questline>`, `Dialogue: <player> saw for themselves: <key>` (a world
predicate), `Dialogue: lie <key> found out: its found effects ran`, `Dialogue: Talk test: N conversations ...`
(an admin's `/talkdebug test`, and the runtime test's `RZT Talk test: ...`), `Dialogue: scene <key> reset by
an admin`. Reputation's voice: `Reputation: gate bark at <place> for <player>`, `Reputation: <player>
laid a pack's leader down`, `Reputation: <player> threw a smoke`, `Reputation: time per cell (player-faction minutes): ...` (every 30 minutes),
`Parley: <player>'s fire paused <player>'s parley`; the runtime test's `RZT Reputation test: ...` and `RZT
voice <mood> faction N: radio [...] face [...]` (Voice > Force mood cycled).
The season record: `Reputation: season record: <player> locked to <route>`. Lies: `Story:
lie <key> told by <player> (<test>)`, `Story: lie <key> found out by <visit|road news|tick|rumour|call-out>`,
`Story: lie <key> caught now by <speaker>`. Givers who lie: `Story: brief <key> leaves something out (version N)` (or `is honest this season`), `Story: brief <key>'s truth found by <player>`, `Story: brief <key>'s lie made public by <player>`. Grief: `Voice: grief for <name>, the <role> at <place>`. Story jobs: `Story: <key> at <subject> is <truth>` (the story
job's kit), `Story: <title> way <n> at <person> by <player>`, `Story: <title> decided <n> at <person> (by
<player>)`, `Story: <title> service <n> at <person> by <player>`, `Story: <title> scene with <person> has no
person (no civilian prefab)`, `Callbacks: <title> at the door for <player>`, `Story: prison bell rung at
<grid>`, `Story: <title> record <key> (<subject>) = <value>`,
`Story: <title> read by <player> (<backgrounds>)`, `Story: distress at <town> is real|loop|bait`,
`Story: colleague <name> is <truth>`, `Story: colleague <name> trail found by <player> (<how>)`,
`Story: <title> left by <player>`, `Story: <title> pays <identity> at their next join (N Fame, N Favor)`,
`Story: <player> hid a bite at house N`, every real hour `Story | per player-hour: Fame N, Favor N | by
job: ... | picks: ... | food on trades N | fallout: started N, paid N Favor`,
`Story: crash at <grid> is <truth>`, `Story: <title> group '<what>' at <grid> (N on the server)`,
`Errands: fallout <card> at <grid> from <title> (cap N Favor, N participant(s) first)`, `Errands: <key>
prop <kind> at <grid>`, `Errands: stranger <kind> shot at by <player>`; the errands' status line ends with
`lies: told N, found or buried N, open N | twists dealt N`. One-choice jobs and work: `Story: <title> asks (<options>)`,
`Story: <title> chose N (by <player>|nobody)`,
`Story: tally N (<town>) = N, mark N (<key>)`. Strangers: `Errands: stranger <kind> for <player> at
<grid> (<spot>)`, `Errands: stranger <kind> done by <player>`, `Errands: stranger <kind> killed by <player>`,
`Errands: found hook <kind> at <grid> after <title>`; `Errands: radio ask to <player> for <name> at <house>`,
`Errands: <key> for <player> has its twist`; locals: `Locals: <name> (<role>) made at <house> by <player>
(pinned|unpinned)`, `Locals: <name> at <house> died with the house's losses`, `Locals: <name> killed (by <player>)`.

Traces ([TRACES.md](TRACES.md)): at start, `Traces: first days seeded (gym <town>, church <town>,
roadblock <grid>, N towns swept)`, `Canon: day2.gym = ...` (and the other new keys, once a season), then
`Traces: N marked doors (N live layers), N town events, drawing decals|boards|text.` (or `Traces: off.`),
`Traces: scene <key> at <grid> (first days)` for each first-days scene, and `Traces: N scenes seeded, N
back from the save.`; while running, `Traces: door <town>#<slot> <author> <symbol> D<day> (<cause>)` for
each live write (`#-1` while the town's doors aren't resolved yet), `Traces: overpaint <town>#<slot> by
<author>`, `Traces: scene grave|pyre at <grid> (a player's)`, `Traces: scene <key> at <grid> (<trigger>)`
for each vignette and live trace (`town gone dark`, `a bite declared`, `a truck lost`, `a death at the
line: N crosses`...), `Traces: scene <key> gone (its days are up|made room)`, `Traces: no anchor for
<kind> in <town>`, `Traces: <kind> keeps off a questline's clue near <grid>`, `Traces: prop budget full
(<grid>)`, `Traces: the young come home to <town>#<slot> (<pack>)`, and once at the cure `Traces: handed
over to the epilogue (N graves down, N wrecks and N fallen posts kept for the week).`; every 5 minutes `Status | Traces: doors N (drawn M), scenes S, props P/40, lights L, loops
O`; `/traces` writes `Traces: <town>#<slot> <key> at <grid>: [<author> <symbol> D<day> <first days|live|old
mark>] | <the line>`, `Traces: scene <key> (<kind>) at <grid>, stage N, props|text | <the line>` and
`Traces: <town> history: Day N <kind> (<faction>) xN`; with **Debug** on, `Traces: player N read
<town>#<slot>`.

## 8. Play-test checklist

What to test first and why is in [ROADMAP.md](ROADMAP.md), "Play-test priorities". On a
session, tick off:

**Start**
- [ ] It compiles; every log line in section 7 appears with sensible numbers.
- [ ] Each background spawns with its loadout and its perk and flaw (try Nurse and Ex-Con).
- [ ] The government broadcast and the session news appear on first spawn.

**Core loop**
- [ ] Infected spawn near you in towns and despawn far away; a shot makes heat, enough heat a horde.
- [ ] Most zombie hits say "Scratched"; a bite opens the golden window and a "Clean the bite" spot
  that follows you. Cleaning with vodka or antiseptic, or disinfectant and a bandage, either stops
  it (told a minute later) or it takes when the 2 minutes are up.
- [ ] With **Bite minutes** at 6: Fever, then Weakness (stamina drains, a friend 10 m away is told
  you look sick), then Final with a countdown; turning starts the Turned Player.
- [ ] An antidote cures it; the trial dose at Halcyon's quartermaster cures or halves the time,
  and refuses a second time; a stabiliser pauses the countdown.
- [ ] Each low-grade med works once and shows its side effect (nausea, coughing heat, fever
  health drain, drunk stumbling); a fever suppressant stops a safehouse catching you; the meds
  never add more than 30 minutes together.
- [ ] Wearing a jacket, gloves and a helmet makes bites rarer (watch the log or count them).
- [ ] A special drops a sample; delivering it at the lab puts it in the backlog: **Check research
  progress** and the journal show it waiting ("Waiting: 1 sample's worth, about 3 hours of work"),
  and research moves within a few minutes. With **Cure > Lab research per day** at 0 it counts at once.
- [ ] The cold store (set **Cure > Lab backlog days** to 0.05, about an hour of work): a second
  delivery is refused ("The cold store is full. ... Come back tomorrow."), the samples stay in your
  pack with no Fame, the action shows "The cold store is full", and a research mission's line says
  part of it was lost. Stop the server for an hour and start it again: on start the backlog is
  down by that hour's work (the lab works in real time), and the cold store takes samples again.
- [ ] Breakthrough news waits for you (set **Cure > Research per sample** to 100 and **Cure > Lab
  research per day** to 240, 10% an hour): deliver one sample on a fresh season and log off at once.
  Come back after three hours: on spawning you hear "Breakthrough: Field antiviral" once, and the
  broadcast speaks of "reports of research".
- [ ] The status line's **pace by play** is 100 percent for the season's first two real days, then
  follows the player-hours played (e.g. about 250 percent for two people playing 2–3 hours a day).
- [ ] A contract is offered by radio, completes, and pays Fame and Favor; a tier change is announced.
- [ ] A quartermaster sells an item at the right tier and refuses one above it. No quartermaster
  offers a vehicle, a car part or a jerrycan. A Bastion Trusted player can requisition rations and
  a rifle magazine (once an hour, half price).
- [ ] The shop screen: **Trade** at the Haven quartermaster opens it, and the Buy actions are gone
  from the action menu. The header says whose stores they are, your standing and your Favor; LB/RB
  (Q/E) switch tabs and skip empty ones; up/down move in the list, which scrolls. A Champion item
  is dim with a lock and "Needs Champion with the Haven"; its picture is darkened. The picture
  shows the selected item, turns with the right stick (left/right, or a click) and shows its
  description and weight. A buys: the item lies by the quartermaster, the Favor drops in the header
  and the status line says "Here's your ...". With too little Favor A says so without asking the
  server. At Bastion as Trusted, the Services tab has Requisition (Sign out) and Make amends (Ask).
  B closes it; walk 6 m off or let an infected close in and it closes. With **UI > Shop item
  layout** empty, Trade is gone and the Buy actions are back.
- [ ] Purification tablets: carry `RZ_Tablets.et` and untreated water; **Purify water** at a
  workbench uses one tablet and gives safe water.
- [ ] Stop and restart the server: stage, infection, research and your standing are back.
- [ ] Restart the server during the final stand with nobody on: it doesn't count as abandoned
  while the server is empty, and the first player to spawn is told the stand is running.
- [ ] Loose items (set **Loose item minutes** to 2 to test): a dropped sample and a bought item
  left on the ground vanish once nobody is within 150 m; the same items in your pack, in a car's
  storage, or lying next to a running mission stay. A vehicle is never removed.
- [ ] Messages that arrive together (the three on spawn) show in the order they were sent.
- [ ] With nobody online for a while, the stage minutes and town infection don't move; with
  **Season speed** at 4, a stage change, a territory tick and a truck come four times as often.

**Stealth** ([DESIGN.md](DESIGN.md#stealth); a group of infected in a quiet town, a friend helps)
- [ ] Stand, crouch and go prone at 30, 60 and 100 m from them, by day and at night: the HUD's
  visibility drops as you go lower (prone and still in the dark: HIDDEN), and they notice you
  later the lower you are. Walking upright at night in a storm never shows HIDDEN. An
  unsuppressed shot shows SEEN or more at once; a few suppressed ones do too. The log says once
  whether the infected take a perception factor.
- [ ] Get chased, break line of sight (a wall, a hedge), go prone and keep still: after about 8 s
  they walk to where they last saw you and look about for 45 s, a new spot every 10–15 s, then go
  back. Stand up in front of one while they search: they come for you. Keep lying there after
  they've given up: they don't come back to the same spot.
- [ ] Walk past an idle group 15 m away without them turning on you, then crouch: they don't
  come to your spot. Fire a suppressed weapon from prone at night near a group that's after you:
  they don't lose you while you shoot.
- [ ] Lie prone while a friend stands next to you: they see the two of you as soon as they'd see
  him alone. Then one of you lies prone 40 m from them and the other stands 90 m off: they see the
  one standing as soon as they would with nobody lying near.
- [ ] Lie in a ditch by the road as The Tribe Moves passes: it is slower to notice you than
  with **Stealth** off. A group with a wander or patrol of its own goes back to it after a search.
- [ ] Hold the lab in the final stand, or a hold mission, lying prone: the waves still find you.
- [ ] All of the above with **Stealth > Enabled** off: no visibility on the HUD, and compare how
  close you get and how long they chase you.

**The infected** ([INFECTED.md](INFECTED.md); the status log's `infected` line and `/tribes` as an admin)
- [ ] At the first start the log says the land is being read and, a few minutes later, its summary
  and the tribe slots (Arland: under the floor). A restart loads it from the cache in milliseconds.
- [ ] By day a town's crawlers stand mostly by doors and in yards; at dusk half are in the streets;
  at night most are in the streets and a few in the fields at the edge. None appears in plain sight
  within 300 m: watch a street from a roof while the town fills.
- [ ] Kill a town's crawlers, walk 1.2 km away and back within half an hour: it's thinner (the
  ledger), and what comes back walks out of the buildings 200 m or more away. `/tribes` shows the
  ledger halving.
- [ ] A thrown bottle or a shot at the edge of a quiet town: the crawlers within 300 m come over,
  stand about a minute and a half, then go back. Not if you're within 250 m of them.
- [ ] Hide in a locked house with crawlers banging on the door: after about 10 minutes the banging
  stops ("The banging stops."), and no new ones come to that door for a while.
- [ ] `/tribes stir` in a town: its crawlers come out and toward the last noise, its square warms
  up, and no horde comes from the stir alone. `/tribes site` 500 m outside a town by day: at
  nightfall 2–4 of its crawlers walk out to it and stand there until dawn.
- [ ] In the countryside, the two infected near you walk somewhere (by day toward a town, at night
  toward the woods); follow one into town. None appears in the open within 300 m of you.
- [ ] Make 80 heat near a town (a long firefight): the horde comes from the town's side, and the
  log names the town. Far from any town, it comes from anywhere, as before.
- [ ] The status line's agents stay at or under 100 with two groups of players far apart (28 each
  at most), and the active AI stays under its limit with trucks and patrols out.
- [ ] A special killed in a group drops its sample where it fell. Burn the Nest's brief reads "The
  slow ones lie up thick in ...".
- [ ] The clock: a game day takes about 4 real hours; restart at night and it's still night.
- [ ] **Zombies > Crawlers** and **Zombies > Tribes** both off: towns, roamers and hordes as before,
  and no `infected` status line.

**The tribes and how they move** ([INFECTED.md](INFECTED.md#tribes); on Everon, or Arland with
**Zombies > Tribes > Slots** 2; the status log's `tribe` lines, `/tribes`, `/tribes hunt`, `/tribes
call`; a Hunter is useful)
- [ ] The log seeds 2 tribes on Everon once the land is read, each with a place (*"the woods north of
  Levie"*); a restart restores them, with the same places.
- [ ] **A solo night crossing** through a range: at 18:45 one line, *"A call from … at dusk, …"*,
  never the words alpha, tribe or crawler. After 20:30, walk the range quietly (no shots, no lights):
  most nights you meet nothing or a group passing in file; if one notices you, it stops and makes a
  sound first. Make noise (30 heat) inside the range: the young come from the trail side, two groups
  from either side out of sight, the big one later. Break the line of sight and get 300 m away: within
  a minute or two they go home. Note how often a careful crossing gets through, and whether any body
  appeared in plain sight within 300 m.
- [ ] **A squad at night** (three players together): `/tribes hunt` or noise in a range. The first
  wave is at most 8; kill a third of what came, or hold out 10 minutes: *"One long call from …, and
  then they pull back."*, and they don't come for the squad again that night. At first light, from
  within 2 km: *"A call from … at first light, and fewer answers than at dusk."*
- [ ] **A day pass by the lair:** by day two or three walk the woods' edges and stop (hop and hold),
  never in a town and never in no man's land; at most 6 near you. Lie flat as a pair passes: they
  should walk on (the notice meter). Walk to within 300 m of the lair: the ring is there, the big one
  in the middle; they don't turn to a shot unless you're close. Within 100 m they all come, the big one
  too. Stand on a ridge in sight of a scout for 5 seconds: *"A shriek from the ridge, and an answer
  from …"*, then at most 4 come from the lair's side; kill the scout within 5 seconds instead and
  nothing comes. A Hunter hears *"The birds just went up off the ridge."* first.
- [ ] **Formations:** bodies keep 3 m apart (watch a pair walk: no two touching), walk-stop by day,
  march in file at night; a pincer's two groups come from two sides about together; a stalking group
  freezes when you turn to face it within 120 m.
- [ ] **Fences and water:** a low garden fence is vaulted, a tall wall gone round; they never wade a
  river or a pond outside a bridge or ford. Climb to an upper floor: they wait below. The big one never
  climbs.
- [ ] **The hunter's lunge:** let a hunter come at you from out of your view: on a hit, sometimes a
  light bleed on an arm or a leg; never more often than once in 10 s.
- [ ] **The taken:** die to a tribe's body in the open after 10 minutes alive: the log says it took you,
  its hunger falls 25, a kill site appears (crawlers come the next night), and the next dusk it gains a
  young. Inside a town you're the town's instead. The next day nobody is out on its paths and its
  dusk is silent (*"Nothing from … tonight."*).
- [ ] **The alpha:** shoot the big one and get away: come back later, its health is as you left it.
  Kill it with a player within 150 m: 2 samples in Prime (none while Rising), the vigil (*"The calling
  from … stops."*), no hunt that night, and *"a voice nobody knows"* at the next dusk.
- [ ] **Tracks** (as a Hunter): along a trail by day, *"Four or five, heading north-east, last night.
  …"*; within 60 m of the lair, the line about no birds.
- [ ] Frame time with a lair, a day party and a town's crawlers near one player, and with two squads at
  night: the status log's agents stay under 100 and the behaviour line's releases close (no strikes
  that stroll in). **Zombies > Behaviour** off: tribe groups walk with waypoints and fight as Bacon's.
- [ ] **The other strategies** (at night, `/tribes hunt`): hide in a wood or a house: one group comes in
  groaning while the rest wait on the far side, and breaking cover brings them all (flush); with a
  cunning tribe, the young come loud from the front while hunters come up behind you (bait); near a
  town, the ring's gap faces the town and the street stirs (herd); near a bridge, a group lying still
  by it (ambush). Lose them: another group of theirs turns up near where you were (hand-off).
- [ ] **The alpha's charge:** stand 20 m from the big one in the open: a roar, then it comes straight;
  sidestep, and it overruns and stumbles for a couple of seconds. Never with 3 or more others round you.
  Let it reach you: you go down prone and can't move for about a second, then it swings (*"It hit you
  like a car door..."*); at most once in 30 s.
- [ ] **The young's sprint:** a young should catch a sprinting player in the open within about 10 s,
  and lose them behind a locked door or over a tall fence. It sprints 6-8 s, then runs 4-6 s; a group
  strings out. Not in your gentle start.
- [ ] **The young's stagger:** let a young leap at you and land a hit: you slow for under a second,
  at most once in 6 s.
- [ ] **The young's stare:** near a pack's young by day in a village, watch one walk to a house's door
  and stand facing it for a few seconds. **The ETA:** in a pincer at night, the nearer group holds
  still until the further one is on its way; both strike together. **Forest edges:** a stalking group's
  shadow point sits just inside the trees on the open side, not deep in the wood.
- [ ] **Hearing:** sprint past a pack's day party at 30 m out of its sight: it turns and walks toward
  where you were (not to you). Crouch-walk past at 10 m: nothing. A noisemaker or a forced lock pulls
  the nearest group to look. Bleeding, walk a field: a group comes along your trail behind you.
- [ ] **Shrieks:** get noticed by a group with young: sometimes a shriek, the line, and another group
  of the same pack comes to look; never a chain across the map.
- [ ] **Reactions:** shoot near a hunting party: the hunters freeze, then come round from the side; the
  young come straight. Kill half a group: it falls back 60-100 m and comes again; kill half again and
  it goes. Wound a hunter: it hangs back, then returns. Drive off: they give up at 30 m (the young at 60).
- [ ] **Doors:** hide in a locked house with a second door open: after about 30 s of banging they come
  round to it. In a locked shed or barn at night with a bold pack's hunt (or the big one) outside: the
  door gives after about 45 s, with a warning first; a house by a holding or a safehouse never does.
- [ ] **A hunt's small rules:** get hunted and log out for 5 minutes: back in, they come on again from
  where they waited; after 10, they've gone home. Lose a hunt, then bang a door near where they lost
  you: they don't come back that night. A hungry pack's morning: the Haven's warning, and walkers at a
  town's edge. Throw a bottle ahead of a hunt with young: the young peel off to it.
- [ ] **The resting ring:** the lair by day from 300 m: the status line counts rings asleep; walk in to
  200 m: woken, and they see you as before.
- [ ] **The ankle grab** (a crawler street, crouched or prone, or a hunched one lying up): a hit holds
  you (slow, *"Something has your ankle."*), the hint says to press Jump; three presses (Space, or the
  pad's jump button) kick you free, and killing it or stepping away frees you too. Never two holds
  within 5 s. Check the tell (a groan) comes before each strike, that the hold doesn't look like slow
  motion, and that it works with a pad.

**The tribes' life cycle** ([INFECTED.md](INFECTED.md#the-life-cycle); `/tribes` shows the leaders'
names and histories, rovers, borders; many of these take season days, so read the log and check each
when it happens)
- [ ] **A succession:** kill the big one at night; at first light from within 2 km: the shrieks, then one
  voice; that morning nobody on its paths and 3-6 of them on a road 1-2 km off; *"a voice nobody knows"*
  at dusk; the headline on the next join.
- [ ] **A lone one:** the next day, a big one alone on the highest ground near the lair; killed with you
  near, 2 samples.
- [ ] **The Clash** (an alpha in Decline, a player within 1 km at its season day's turn): at dusk the
  ring round the lair and the two voices; one walks off alone. Shoot one of them during it.
- [ ] **Borders:** the dusk call adds who answers from across the bridge or the ridge. Hungry
  neighbours at a bridge with you within 800 m: the Border Night's line, both sides' groups at the
  bridge, and, if you stay 150 m off, bodies there afterwards; the next morning the loser answers with
  fewer voices.
- [ ] **Disturb a lair:** loiter within 150 m of it by day, shoot near it, or burn it (a molotov within
  30 m: *"They know you at ..."*): it moves, with the move's line; the old lair holds a line about
  somebody's things; the old place stays silent at dusk (*"Nothing calls from ..."*).
- [ ] **Habits and fear:** feed a tribe at a bridge twice (wanderers or players taken there): at its
  next set-out a group waits at the bridge. Kill 8 of a tribe at one spot in a night: as a Hunter, the
  next day *"They don't come through here any more."*; a few nights later *"They're coming through here
  again."*
- [ ] **Starvation and new tribes:** a tribe left hungry (no takings) eventually sends some into the
  nearest town (its infection +2 each, and the line there); a town at 70+ with no range within 2 km for
  3 season days founds a tribe (the headline) whose first dusk is *"a voice nobody knows"*.
- [ ] **The people's side** ([INFECTED.md](INFECTED.md#people); **Zombies > Tribes > People**): at
  dusk a Haven-Known player hears the Haven name a hungry pack with its place; a Bastion-Known one hears
  a new contact (`Contact Papa-one, ...`). Never "alpha", "tribe" or "crawler". The notebook's Journal
  lists each pack you know of by its place, with the names you've heard and the level (Heard of, Read,
  Mapped); walk to a lair by day and it's Mapped. A new life knows nothing again.
- [ ] **Ways to learn:** the new quartermaster services ("Ask about the roads", "Read the contact board",
  "Buy the map of the crews", "Buy Halcyon's tag frequencies", "Ask for the range map", "Borrow the call
  box"), on the Services tab or as actions, each with its tier and Favor; a Soldier spawns with the board
  in the journal, a Police Officer with the column. A Radio Operator at dusk with two packs calling hears
  the calls pass ridge to ridge.
- [ ] **Marks:** walk the road into a pack's ground near a Haven house: *"Haven chalk on a post: QUARRY LOT.
  NOT AFTER 7. Written on day N."*, and the range is Read.
- [ ] **Your dead run:** die to a pack's body in the open after 10 minutes alive: your group is told, your
  next life's journal says who you run with now, and road news describes you (named only with **Your
  dead run** on Named). The notebook item's action "If I die: say my name, or don't" (and `/mydead
  hidden`) keeps the name out even on Named. Put down that pack's young near you: an ID card line.
- [ ] **The packs' contracts** ([MISSIONS.md](MISSIONS.md#the-packs-contracts)): each is offered only in
  its hour (Not Tonight from 17:00, Contact Report from 17:30, Escort at Dawn and Track Them Home from
  03:30, Plant a Tag in the morning). Run Not Tonight (a noise at the spot before nine), Contact Report
  (two points at dusk), Plant a Tag (the tag from the lab, held 5 s by a sleeping leader by day) and
  Bring Them Home (the card to any house) end to end; the pack doesn't move while one runs.
- [ ] **Every Ridge Calls** at dusk: every pack hunts three trails, its range doubled, straight through its
  fear places, and a border with a frenzied side is fought more often (2 Border Nights at most); **The
  Tribe Moves** when a lair moves out of its range ("On the Move", two groups near you); **Red Dusk** at a
  clear dusk. The towns inside the frenzy hold twice their crawlers until dawn, not just its first 20
  minutes.
- [ ] **Siren Test** near two packs, one with the bells habit: it comes, the other keeps away from
  that town tonight (`keeps away from the siren`). With a pack's group in a town, the crawlers that
  spawn stand by doors or on the far side. In a Winter season a Hunter's track line ends *"Sharp as
  the day they were made, in this snow."* At the cure, `/tribes` lists "Walking with it" beats.
- [ ] **The packs' hordes:** make 80 heat inside a pack's range by day: its day party walks over (`Pack
  horde:`), no town's crawlers come; at night, its hunters on their trails. Stay put an hour at night
  in a range: a young group comes to look (`Pack lookers:`).
- [ ] **Stock the Pit** (Red Hand Known, a camp within 6 km of a pack): lure one young into the cage at
  the lair's edge and drop the door (the second: tie it instead), hold it 30 s, the case to the drop by
  the camp, then the second; the next Fight Pit's brief says *"Fresh hires tonight,
  from …"* and two come out in its last round. **Live Capture** with packs on: the special is one of a
  pack's guard (or young) at its lair's edge, and the pack is one smaller after. **Lead Them Away** near a
  pack's ground: the crowd is in its look, from its side; with a nomad (or half the time) it's the pack
  on the move: kill a few, lead the rest to the quarry, and the log says the pack moved house there and
  `/tribes` shows it smaller. **Blow the Bridge** on Everon: the charges go on
  a real bridge and the crowd comes from the pack's lair. **Missing Patrol** and the **Military Remnant
  Camp** near a military pack: its young.
- [ ] **The great one** (a big map, or `/tribes` to watch the clan's days): a clan of three holding 8
  season days from Overrun gives the headline and a guard of 4; at dusk everyone online hears its call
  instead of their own pack's. Every 4 season days (or on Every Ridge Calls) **The Gathering** is told
  and groups from each clan pack wait at the wire until first light; put 10 down there for a crate at
  dawn. Kill it: every clan pack holds the vigil that night, `/tribes` shows no clans, and the next dusk
  is **The Night of Many Calls**.
- [ ] **Signs and sounds:** with the kill-site, abandoned lair and mark prefabs set, walk up to a kill
  site, an old lair and a mark: each prop appears by 200 m and is gone once you're 250 m off; a
  rewritten mark is a new prop. With a call sound prefab, a pack's dusk call is heard at its lair from
  within 1.5 km, once (VERIFY). On a clear night near a full moon the nightfall line comes and the
  hunt stays out until 05:30.
- [ ] **The journal and the map:** the journal lists your packs Mapped first, then Read, then Heard of,
  each with the range ring and its level where the time goes; on the map each is a yellow ring sized to
  its ground (Heard of: half a kilometre round the rough area), following the zoom.
- [ ] **Tokens:** a line with `{tribe:N.bastion}` or `{alpha:N}` (for example a test rumour) reads
  "Contact Papa-two, the quarry by Morton" and the leader's word; a pack that's gone leaves the token.
- [ ] **The cure:** at 50% borrow the call box (Halcyon Trusted), put it down at night in a pack's
  ground away from people, and play it: its hunters come, never the leader. Refused near a house or
  near a stranger. At 100% the stand's waves come from the lab's neighbour towns and the nearest pack.
- [ ] **The last week:** after the cure, the dusk call reads *"… still calls. There's hardly anyone left
  to come."*; kill a pack's leader and next morning it's gone; with **Epilogue hours** low, the last dusk
  is *"Nothing answers it."* and the packs are gone at dawn.
- [ ] **The small rules** ([INFECTED.md](INFECTED.md); the status log's `tribes' small rules` line):
  stand by a pack's hunting ground three nights and its first trail goes there more often; stand in
  its range away from every ground three nights and at the next season day the log says it `follows
  its prey`. Keep players and trucks off a habit's place a night and the habit weakens. After a bad
  night (kill a quarter of a hunt) a group comes home late, 06:00–07:30, with the stragglers' line. As
  a Hunter, a dusk call adds which way and how far; heard before, it's named by its call. A cunning
  pack's second group comes from behind you, not the side. Go into a house: a scout never calls on you
  there. At dusk the scouts sit above tonight's trail; in fog they sit lower. Kill a leader with you
  near: its samples and body are still there at the end of the vigil (VERIFY). A new leader a season
  day on: the Haven (and Bastion for a board pack) say who leads it now. Burn a lair after killing 4
  there: a splinter breaks off. Walk up to an abandoned lair with someone's things: 1–2 items lie at
  its back.
- [ ] **The behaviour layer's untested levers** (not in the code; each needs a client first): lever
  L6, a forced prone pose for crawlers with `ForceStance` (a prone `SetStanceChange` didn't hold in
  B12): does a crawler stay flat and still fight within 12 m? L16, a scripted `AIDangerEventWeaponFire`
  near an engaged body: does it pull it the way a real shot does? L23, a straight-path cost per kind in
  `SCR_CreateAIExtraMoveParams` (×1 young, ×3 hunters; the hook loads and fires, B2): does a hunter's
  chase curve? Report each here; one that works goes in as a behaviour setting.
- [ ] **Notice rate:** time how long a steered group takes to notice you at each stance and distance of
  phase 0's ladder, set **Zombies > Behaviour > Notice rate** to match, and run `python3
  tools/tribe_sim.py` again with it.
- [ ] **Crawlers' life:** by day a town's crawlers stand hunched and don't turn until you're within 40 m
  or shoot (the status log's `asleep` count); one groan as they wake. At night they drift the streets in
  slow herds; with a pack's group passing through they step aside. **Sleep lying crawlers** off: they
  stand awake instead.

**Seasons and notes**
- [ ] With **Wipe** on *After weeks* (1 week), stop the server and set `seasonStartUnix` in
  `state.json` back 6 days and 20 hours: on start the warning broadcast comes within a minute; set
  it back a full week: `wipe.json` is written and everyone is told. Restart: the backup `state_season<N>.json` exists, the season number went up, and the hall of fame kept
  last season's names.
- [ ] Win a final stand, then restart with **Epilogue hours** at 0: the next season starts with
  the winner's rules (the log and the first-spawn "This season" notice say which).

**The epilogue week** ([EPILOGUE.md](EPILOGUE.md); **Epilogue > Test scale** 0.008 runs it in about
81 minutes, `/epilogue phase B|C` moves it on)
- [ ] Within a minute of the cure everyone online gets *The cure* with their own line and where to
  look; a player who joins later gets it at spawn. The hall of fame stops counting (deliver a sample:
  no record).
- [ ] The lab gate's board, the stand board's names, the queue's sign and table at the north road, a
  safehouse's candles: a prop with **Board prefab** set, else the line when you pass within 40 m. A
  leader on the talk screen has **About the season** and **Where should I look?**; without the screen,
  the quartermaster's *Ask about the season* action.
- [ ] Bitten, ask for the dose at the route's outlet: cured on the spot, the price taken (Bastion: a
  firearm or 15 Favor), a second this life refused, an empty outlet says when the next comes.
- [ ] Fire a shot by the queue with nothing infected near: +2 Infamy and the guards' grudge; kill one
  of its people: +15 Haven Infamy, and the queue's line has the gap until the next step.
- [ ] The chair's *"Whose names do we read tonight?"*: answer *All of them*, have a second player
  object within 5 s (nothing decided), answer again (decided; every board pins the note).
- [ ] A questline running at the cure: its person stands at its place with two or three ways; pick one
  (its ending, Fame 3 and Favor 5); leave another until its last call and default.
- [ ] Take a wanderer's keepsake where they died and hand it in at a house: the Names board lists it;
  die carrying one and it lies where you fell.
- [ ] In processing, walk up the north road unarmed: *"Name?"* instead of being turned back; *Give your
  name* at the barrier returns you to the spawn screen with no death screen (VERIFY) and puts you in the
  hall's *Walked out*; armed, you're turned back as before.
- [ ] `/season` (or the notebook's action, or its The season tab with a pad: LB/RB to the last tab)
  lists the beats you've met, And you and Unfinished; `/anonymous` turns your name into *a survivor*
  on boards and in the hall.
- [ ] The evening calendar: once the Gate has run, at 18:30 the Haven's open channel reads six names;
  once the last call has run, at 20:30 Bastion's says *"Bastion listening. Nothing heard. Bastion
  out."* and the roster board's line counts the strokes; 8 real hours (× Test scale) after a coda, its
  line comes again on its faction's channel.
- [ ] The epilogue deck: at a Haven house *Need a hand?* deals Pack for the Queue, Borrowed, Take a Name
  to the Board or Clear the Way (an admin's **[Admin] Errands near here** says why the others weren't dealt); take Take a Name to the
  Board: the Names board lists the name as home, by you. Before the cure none of them is dealt.
- [ ] At the wipe, `last_season.json` has `broadcast` and up to 10 `beats`; the next season's first
  spawn names "Last season's stories" when a major ended live (Season > Story rules).
- [ ] **No stand in the last hours** on, After weeks set so the wipe is a day away: bring research to
  100: no stand starts and everyone hears the director won't start the synthesis.
- [ ] **Rule sets** `Winter, Fog` on a fresh season: both are named on first spawn; at night more
  specials; Fog Banks come more often. `No Vehicles`: no fixable cars, no trucks, no Recover the Bus.
- [ ] The hall of fame shows on first spawn and on the notebook's Hall of fame tab; a delivered sample and a finished
  mission show up in it.
- [ ] At a safehouse: "Read the notice board" (empty), `/note Water at the church, bring a bucket`,
  read again: your name and the note. A second note at once is refused; away from a safehouse
  it's refused. With a word in **Note filter**, it's blanked out. As an admin, `/noteclear` clears
  it; as a player it's refused. Restart: the notes are still there.
- [ ] Rough areas: a Bastion contract offered to a Stranger names a 500 m circle; walk within
  500 m and you're told the spot; a Radio Operator, or someone Trusted with Bastion, gets the exact
  spot in the offer. A puzzle contract always gives its own positions.

**UI** ([Setting up the UI](#setting-up-the-ui); the spawn screen and map items need its steps 5 and 6)
- [ ] A Haven channel message shows as an amber card with the speaker; with **Styled notices** off
  they're vanilla hints again.
- [ ] Spawn: the background card comes first, then the outbreak and the rest one at a time (two
  cards at most, about 2 s apart); nothing pours in. Get bitten while the welcome cards wait: the
  bite card shows at once. Wait a minute with cards queued: "+N in your notebook" appears under
  them, and the notebook's Radio tab lists them (filter Missions and World).
- [ ] On a dedicated server with two clients, a contract offer to one player never reaches the
  other's log (owner-only notices).
- [ ] Walk up to a Bastion checkpoint: its warning is a subtitle at the bottom centre in olive,
  not a card; a Perimeter loudspeaker too. With **Subtitles** off, they're cards again.
- [ ] Talk to a Trader wanderer: the talk screen opens with their name, what they say and the
  replies; "Buy supplies" shows its price and is dim with what it needs if you're short; pick a
  reply with the D-pad and A, with 1–7, and with a click; B leaves. Their choice actions don't show
  in the action menu while the screen is on. Walk off 6 m, or let an infected come close: it
  closes ("Not now!"). Do the same with a leader's "Talk to whoever's in charge" (Ask for work
  gives the job as a card, the leader's line in the screen) and a safehouse occupant.
- [ ] With **UI > Talk reply layout** empty, Talk and the slots are actions with notices again.
- [ ] **Dialogue > Trees** off (the default): the talk screen is exactly as above, no topics, no
  speaker at road posts, leaders greet as before.
- [ ] **Trees** on ([DIALOGUE.md](DIALOGUE.md)): a council head's list shows the built replies, then
  "Ask about the bear on the door." and the other topics; "Something else." pages when they don't
  fit. In "Ask about the names on the list.", the persuade choice shows "[Persuade - Haven - N of 2]"
  before you pick, and the reply opens with [Persuaded], [Short] or [Not persuaded]; a failed one is
  dim ("you've tried that") next time. `/talkdebug` (admin) lists that speaker's conversations and
  why each shows or not. As a Nurse, the line only Nurses see shows "[Nurse]"; as an Unemployed it
  doesn't show at all.
- [ ] Trees on, at a short safehouse: "Put two tins in the stores." shows "[2 food]", dim with "you
  have 1 food" when you carry fewer; picking it hands in as built (2 Favor an item). Talk Favor stops
  at 20 a faction a day ("That's all I've got today.").
- [ ] Trees on, at a Red Hand roadblock: one of the crew stands at the barrier while you're within
  100 m and goes when you leave. "Ask about the toll." pays the built toll; the threat shows "[Fear N
  of 2 - +3 Infamy]", always costs the Infamy, and the pole goes up on a pass; nobody can threaten
  that toll man again this season. At a Bastion checkpoint, a Soldier's "I know the drill." can wave
  you through without the search. The Long Table's host on the road and the mother at the farm talk,
  and nothing they say describes anything.
- [ ] Trees on, a leader you've met greets you by your standing in their voice; finish their
  personal job and the next greeting is "You're the one who ..."; "What do you make of me?" gives
  the faction's template with your best and worst deeds and the rule in brackets. A Red Hand crew
  name comes with its notice (pay a toll with exactly the Favor you have, while Known).
- [ ] Trees on, lies you're told: ask the Scavenger where's worth looking (the chemist's), then the
  Doctor wanderer about it ("I cleared it myself"); back at the Scavenger, "[Evidence] Picked clean
  on Day 3." shows and opens the honest answer. As a Hunter it shows "[Hunter]" from the start.
- [ ] Trees on, a promise: at a short safehouse "I'll bring two tins. Two hours." gives "(A promise:
  2 hours of your play to keep it.)"; the next Talk there lists it with the play left; handing in two
  tins says "Kept"; letting it run out (or dying) says "Broken", and the house's next greeting says
  so once. The Bastion sentry's "I'll check the road and come back." is kept by talking to them again.
- [ ] Trees on, in Bastion kit you aren't Known in, at a checkpoint's speaker: "Answer as one of
  theirs." shows the look's tags and "[Look - Bastion - N of 2]" before you pick; a pass waves you
  through, a catch takes the kit (and the conversation carries on). "Give the word at the wire."
  offers three words: today's (heard on Bastion's secure net) adds a cover detail to the look; a wrong
  one is caught at once. Change your jacket mid-conversation: it ends ("Didn't you just
  change?"). Named Bastion people greet you with "Where did you get that jacket?".
- [ ] Trees on, walk past a short safehouse's occupants, a Red Hand crewman (with a crew name) and the
  lost Kid: each barks one line as a subtitle, at most one a minute, and the same person not again for
  10 minutes; nothing while you're talking to someone. The log has `Dialogue: bark ...`.
- [ ] Trees on, a stranger: Talk repeats their call-out; their ways are there as before, and "Ask
  who they are" style topics fill in their own words (the house, the town, the name). Walk up to a
  new one: at 10 m the talk screen opens by itself ("Hey. You. Sorry. Have you got a minute?"), once,
  and never twice inside 4 minutes of another unasked contact (the log's `Dialogue: ... opened on`).
- [ ] Trees on, in Bastion kit you aren't Known in, walk toward a checkpoint: at about 35 m its
  speaker's screen opens by itself ("You. Not on the board. Stop there."), with the claim, the word and
  the truth as topics. Leave it and walk on: the built search or turn-back applies at the barrier. In
  kit signed out to someone else, the greeting names them and any claim is caught at once.
- [ ] Trees on, a seat leader at attitude -2 who doesn't hunt you: the hub shows "[Make it right]
  "What would it take?"", which reserves their faction's amends job for an hour. Finish it: the
  leader's "We start again." line, they deal with you again, and the reply is dim for the rest of the
  season. Fail it or let the hour run out: nothing changes.
- [ ] Trees on, a Radio Operator with Bastion's net cracked: the radio's "Call on the open channel"
  also shows "Talk on Bastion's command net." Ask for the word: believed, "(The word at the wire
  today: ...)" and a post's look counts it; caught, +2 Bastion Infamy and the listening ends. A house
  radio's third choice calls only next to a running generator.
- [ ] Trees on, near a Haven house with nobody at its gate: "Answer the call for help." shows on the
  open channel about one hour in two; answering puts a stranger at that house's gate, and its named
  person offers Who's at the Gate as built.
- [ ] Trees on, the routes open: on the open channel, "Speak about the cure." leads to [Point of no
  return]; saying it locks you to the Independent route, a second player within 1.5 km hears it with no
  name, and the roads carry the rumour.
- [ ] Trees on, the epilogue week: every leader has "Ask about the cure." (their own read of who holds
  it, then the built season and where-to-look lines) and "Ask what they'll do now."; occupants greet
  you with the week's line.
- [ ] Trees on: open the notebook's Journal with a promise open: a "Promises" row shows the play left;
  after seeing through the Scavenger's lie, a "What you know" row quotes it.
- [ ] Trees on, the Long Table's host on the road: say they're expecting you back by dark, then stay
  near the road past 20:30: the next meeting opens with "Still expected, are you?".
- [ ] Walk up to a Bastion holding's gate, a Red Hand roadblock and a safehouse door: each says one
  line about you within 25 m, only the first time this life or after your name or best deed with them
  changed; a second player within 30 m hears it too once you're notorious with them (notoriety 3). In
  another faction's colours you earned, the guard remarks on them once.
- [ ] Mid-parley, have a second player fire within 150 m of the gate: the parley pauses 20 s and the
  shooter gets that faction's grudge; no hunter squad of that faction is sent while it runs.
- [ ] Trees on, the open channels for an hour: chatter sometimes ends with a short line; after a truck
  is lost, that faction's channel stays plain for 20 minutes; no joke airs twice inside 6 hours.
- [ ] Trees on, the epilogue week: each leader's first greeting each real day says the ending in
  their voice and where you stood.
- [ ] Trees on, the wind-up radio on the ground: "Call on the open channel" opens the talk screen.
  "Call in what you've seen" puts a rumour on the roads, and a second player within 1.5 km gets "On
  the open channel, someone: ..." with no name.
- [ ] Trees on, Every Name's first sitting seated: a council head's "Talk about the sitting." shows
  the options on the table as persuades; a pass leans that head's vote (the log's `Dialogue: sway
  every_name.first.pot by ...`), and the topic goes for you until the second sitting.
- [ ] After the 90% lock, a leader of your route greets you with an in-group line on the first visit
  of the real day, another faction's with a cool one ("Civilian."); the word in "What do you make of
  me?" is the lock's ("attached", "the other crew"). `season_record.json` is in `$profile:ReforgerZ`
  after the 2-minute save; die and come back, and a leader who never met the new face says "Don't
  know the face. Know the name." once.
- [ ] Die and come back: a leader who remembered you crossing them now counts it at half (the
  attitude words in the talk screen's sub-line), says "There was a ... who crossed me once" once, and
  topics marked once show again. `dialogue.json` is in `$profile:ReforgerZ` after the 2-minute save;
  regard and passed persuades are still there after a restart.
- [ ] The season mood: with **Voice > Force mood** on Bleak, the broadcast says "The perimeter will
  be maintained." (or the Aftermath line), named people's jokes become rare, and the log has
  `Voice: season mood bleak`. Kill a wanderer and talk to people within 1.5 km: no jokes for 10
  minutes.
- [ ] Read a puzzle note, a guidance note at spawn and a letter from a search: each opens on the
  paper; a long one has pages (LB/RB, Q/E or the arrows); B or Esc closes it. Have an infected
  walk up while it's open: it closes by itself. A subtitle that comes while reading waits.
- [ ] A bite shows the golden window counting down, then Fever with the track filling; a
  stabiliser shows PAUSED and the clock stops; the timer matches the stage messages within seconds.
- [ ] Cracking a secure channel shows the indicator until the codes change.
- [ ] N opens the notebook on Journal the first time, then on the tab last used; Q/E and LB/RB go
  through Standing, Journal, Radio, Hall of fame and The season with real data; up/down move the
  journal's selection and scroll the other lists; X or F cycles the Radio filter through six
  names; B or Esc closes. With a pad only: Start, Notebook, A opens it; the notebook item taken in
  hand (inventory or quick slot) opens it and is put away when it closes.
- [ ] A new contract offer shows a count on the Journal tab, a radio message one on Radio; looking
  at the tab clears it.
- [ ] With **Board menus** on (the default), reading a board opens the board menu, and typing in
  its box and pressing Pin it up pins the note (the list refreshes); up/down scroll the notes.
- [ ] **With a gamepad** ([UI.md](UI.md#controllers)): the notice board opens with Pin it up in
  focus; right and left put a ready-made note in the box (not while you type in it with a
  keyboard) and A pins it; B closes. As a server admin, X clears the board ("Cleared (n notes)"),
  as a player X does nothing.
- [ ] With a gamepad, open and close every screen (the notebook from the pause menu and from the
  notebook item, a talk, the reader, the car repair screen, the shop) using only the pad: each has
  a focused widget, footer prompts in pad glyphs, and B closes it.
- [ ] As a server admin, talk to anyone (a wanderer, a safehouse occupant): the **[Admin]** replies
  read their conversations, write the infected to the server log, and read the nearest player's
  standing (yours with nobody within 50 m).
- [ ] On a TV or at 1920×1080 from a couch: every page of the notebook, the notice board, the
  notice cards and the HUD are readable; nothing runs into what's below it (the Standing page's
  labels, the reputation grid's header row).
- [ ] The spawn screen opens at the first spawn (after the welcome screen, if any) and after each
  death, never vanilla's deploy menu or role selection, and asks for no faction or group. With a
  pad only: up/down go through the 13 backgrounds with the panel following (name, perk, flaw, gear,
  standing, Legacy), left/right change where you start, A deploys and you spawn there as that
  background; B opens the pause menu, and closing it brings the screen back. Text reads from a couch.
- [ ] On the spawn screen a locked background shows LOCKED and how to unlock it, and A on it says
  so and sends nothing; the server refuses it anyway (the log says `asked to spawn with locked
  loadout` if a client sends one). After one life as Nurse, its gear lists the Nurse's real gear.
  With a respawn timer, Deploy counts down and then works. With **UI > Spawn row layout** empty,
  vanilla's deploy menu is back.
- [ ] The tabs, close button, journal rows and radio filter take clicks; the lists scroll.
- [ ] Open the map: the lab, the safehouses, the strongholds and a world event show with their
  icons and stay on their spots while you pan and zoom; an outpost appears after you've been near
  it (or once you're Known with its faction); no contract is ever on it.
- [ ] The map key: open the full-screen map zoomed out and the key is in the bottom-left, its icons
  in their colours; zoom in on a town and it fades out, zoom out and it comes back. I (pad Y) hides
  it and shows it again, and it stays hidden the next time you open the map. It covers none of
  vanilla's own map widgets; if the fade starts too early or too late, change **UI > Legend full
  zoom** and **Legend gone zoom**.

**Factions**
- [ ] Hunted by Bastion near Bastion AI: they shoot you, not a friend next to you.
- [ ] Reputation names ([REPUTATION.md](REPUTATION.md#reputations)): do two jobs for the Haven and
  shoot a Haven guard (Fame about 20, Infamy about 18): 30 s later a card says "They call you Mixed
  Bag - counts as Stranger now", once, with the guidance tip; swing back across the line inside
  30 s and nothing is said. The log has `Reputation: ... Mixed Bag (Uneasy)`. The notebook's
  Standing page shows MIXED BAG, "COUNTS AS STRANGER", the Infamy with "TO FORGIVE", and an H in
  the Mixed Bag cell of the grid; with a pad the page opens and scrolls as before.
- [ ] The forgiveness budget (set **Reputation > Forgiveness budget** to 10): at Watched, an amends
  contract takes Infamy off; the next pays Fame instead with "has forgiven you all it will this
  life"; paying at the quartermaster is refused with the same line; Ask what they think of you says
  "0 of 10 left to forgive". A new life starts with the whole budget again.
- [ ] The credit rule: at Champion (75+ Fame), shoot one of theirs: Fame drops by the Infamy gained,
  and a drop to Trusted says "Your credit ran out".
- [ ] The Wildcard tier (about 50 Fame and 45 Infamy with Bastion, then the Red Hand): a Bastion
  checkpoint searches you ("Arms out"); leaving the barrier early costs 6 Infamy. A Red Hand
  roadblock asks 20 Favor.
- [ ] `/rep <your name>` as an admin prints the four factions; as a non-admin it's refused.
- [ ] Known with the Red Hand and no crew name yet, throw a vanilla smoke grenade: the log has
  `Reputation: <you> threw a smoke` and the notice says the crews call you Smoke. Throw a bottle: no
  line (the throw hook, VERIFY).
- [ ] The Standing page: the header has the whole picture after your name; each faction's column
  opens with the word as a sentence ("The crews call you Smoke.") and the nearest leader you've met
  there with what they make of you. Take a parley's amends job: the journal's requirements read
  "Amends: works off Infamy · finishes where the job says".
- [ ] One per approach: walk up to a Bastion post's gate with a name change pending and a tip due: the
  gate's bark first (with a rival-colours remark in the same line, in another faction's armband), the
  change notice 30 s later, then the tip 30 s after that.
- [ ] In Bastion kit while Bastion's AI would shoot you by standing (Watched with **Hostility** set so),
  fail a look at a road post: one urgent line ("Nobody asks you to stand still.") and they open fire;
  no search, kit still on. At a post that wouldn't shoot you, a failed look holds and searches as
  before. After a passed look, stand behind a sentry when the again look is due: no "turns your way"
  line until he faces you.
- [ ] A holding's named person killed: the log has `Voice: grief for ...`, the leader there says no
  joke for 10 minutes, and the next time the holding is shown the role has a new name.
- [ ] A short safehouse (let an upkeep run short): Ask how things are sometimes ends with a soup or
  potato joke; none for 30 minutes after a death in that house.
- [ ] The epilogue week (Trees on): visit a stronghold leader twice in one real day: the second visit
  sometimes opens with a joke for the ending (none with no cure, except the Red Hand's).
- [ ] Givers who lie: take Learning the Drones (or any of the twelve, docs/MISSIONS.md) until the log
  says `leaves something out`; stand 15 s at its spot: the truth comes, and anyone within 30 m hears
  it; finish it: the outcome names it. With Trees on, a Red Hand leader then has "Tell the crew what
  the job left out."; telling costs the Red Hand 1 morale once, and the topic goes.
- [ ] A one-choice job (Checkpoint Duty): the decider's group gets the giver's reaction after the
  outcome; another player who took part but stood apart gets "Someone else made the call..."; let
  the 90 s run out once: "Nobody gave the order, so the standing order stood." Press someone in Fight
  Pit before the choice: "Fight it out" refuses.
- [ ] Callback deeds (Trees on): after a one-choice job, visit a leader on another day: one time in
  three "You're the one who decided ...".
- [ ] Boards of record: after Await Relief's call, a Bastion road post's speaker adds the roster board;
  pay a Red Hand toll and the roadblock's speaker shows the tally; a safehouse's Ask how things are
  shows the council's pinned notes once the house rule is set.
- [ ] Quiet success: Cargo's driver, Threaten him, with 40+ Red Hand Infamy this life: he talks and the
  next stage's camp isn't on guard; with none, the built line ("you can guess who he's running to
  tell").
- [ ] Notice lanes ([DESIGN.md](DESIGN.md#notice-lanes)): walk past two leaders who have work for
  you within 4 minutes: only the first catches your eye; the second does once the 4 minutes are up.
- [ ] Contracts and the season budget: finish several contracts for one faction in one session; the
  log shows `Budget: contracts morale ... clipped` once that faction has moved 10 morale that season
  day, and a failed contract never takes an outpost leader's loyalty below 30.
- [ ] Your spawn screen's places, chat and group stay on the Haven while you're on a variant.
- [ ] An exile with a gun near a safehouse is shot; without one, the door is just shut.
- [ ] Shooting a guard makes nearby guards of that faction fire back for about 3 minutes.
- [ ] Borrowed kit: a Stranger in full Bastion kit at a checkpoint gets the look's tags at 150 m
  ("[Full kit +2] ... 2 of 2"), walks to the barrier and is waved through unsearched. In colours
  only, nothing happens until they talk to the speaker (**Dialogue > Trees**). Caught: held a moment,
  searched, the kit taken (VERIFY), +3 Infamy (never into 15 on its own), blown 60 minutes; the
  third catch says "Your description is on the board". Hunted in kit: no claim, surrender as built.
- [ ] Colours: in Bastion green at a Red Hand roadblock you're turned round whatever your tier; in
  the red armband at a Bastion checkpoint you're searched and the armband taken; a beak at a
  roadblock pays 20. Watched at an outpost gate in its kit: the look lets you in for 10 minutes,
  and a leader of theirs within 15 m catches you. Within 1.5 km of the lab during the stand no
  disguise clears anything. The Red Hand's quartermaster and fence refuse you in Bastion green; a
  Haven safehouse's people and wanderers won't talk to you in red.
- [ ] While passing (disguise phase 2): stay inside an outpost's gate on a pass near a guard. About
  3 minutes in, "Someone's looking at you again" with "[Looked at already -1]", and 10 s later the
  look (`again look at`). Walk out of sight before it's due and claim again within 30 minutes: the
  look comes 10 s after the claim (`owed a look`). At 05:30 or 17:30 inside their wire's 150 m:
  `roll call for`. Put the kit on in front of a guard: the next looks there show "[Seen putting it
  on -1]".
- [ ] Word gets round: get caught at a barrier near a holding; the open channel names the place, and a
  friend arriving within 30 minutes sees "[Word's round -1]". A second catch there only restarts it
  (`restarted`, no second line).
- [ ] Vouching: Known with Bastion in your own green, talk to the checkpoint's corporal with a
  Watched friend within 10 m and pick "Vouch": your friend walks through unsearched (`signed ...
  through`). Your friend then gets caught in Bastion kit or shoots a Bastion guard within 30 minutes:
  you get +5 Bastion Infamy once, and "Vouch" doesn't show for 2 hours.
- [ ] The Rogue's price: passing near a Rogue outpost leader in his faction's kit, "Nice jacket.
  Twenty Favor..."; walk off within 10 s and nothing happens; stay and 20 Favor goes (`paid Rogue`).
  With no Favor, you're caught. Turn him in: the payers take +5 (`found out`).
- [ ] Whose kit it was (phase 3): shoot a Bastion guard and put on his jacket and cap: "The name tape
  says ..." and the look shows "[A dead man's kit]" and fails (`marked as`). Kill a helmeted
  infected and wear its kit: "[Off the turned]". Buy Bastion kit as Known and hand it to a Stranger
  friend: "[Signed out to someone else]", and Bastion's search takes it, telling you it came back.
  Buy kit, drop below Known: it stays your own colours for a real day. At 20:30 the nightly call
  reads the dead man's name, and the corporal's talk reads the board's newest line (no player name).
- [ ] Abuse attempts, each refused with its log line: a Hunted player in full kit (`can't claim:
  Hunted`), a disguised friend in a car with a stranger (`searched with the whole car`), walking off
  while held at a barrier (`walked off while held`), shooting a guard while passing with infected
  10 m away (`exposed to` or `caught by`), relogging during a pass (the owed look still comes),
  swapping to fresh kit after a catch (the look still shows "[Caught before -1]").
- [ ] Borrowed kit's rest: in Bastion kit at a road post, the HUD shows the eye and "PASS AS BASTION:
  2 OF 2" from 150 m (on a pad too, nothing to press), and it goes when you pass, are caught or walk
  200 m off. "Give a name that vouches for you." offers only the names you really have; pick one,
  then the look ("gave Bastion a cover detail"); a second claim line at that speaker isn't offered
  this life. Carry a mission item from a job against Bastion: "[Something of theirs on you -1]". After
  the 90% lock, a look by another route's faction: "[Sworn to another side -1]". Wear marked kit,
  restart the server, log in: "kit mark put back", and the look still fails. The notebook's Standing
  page shows IN THEIR KIT above Bastion's leaders. After the cure, carry a dead man's jacket to his
  post: "He's home. Mark him.", and the four factions' lines on the open channel once.
- [ ] The parley: Hunted by Bastion with 15+ Earned Fame and a best deed, walk to a checkpoint
  empty-handed and alone: the shout at 150 m, no fire while you close, 10 s at the barrier, the
  leader's line and a reserved amends job (Burn the Bodies or Recon) you can take though Hunted.
  Draw a rifle: broken. A friend within 150 m: no shout. Again within 2 real hours (relog too):
  nothing. A Haven exile who's owed: talk to a safehouse's named person, unarmed: the knock.
- [ ] Settle up at a quartermaster: -10 Fame, -5 Infamy, twice a real day at most; refused once
  the forgiveness budget is spent. Group rules: a Known player arriving with a Watched friend is
  searched ("We search the whole car.").
- [ ] Several players at one speaker (**Dialogue > Trees** on, a test `.rzd` with `scene`, `decide`,
  `public` and `check lie`): a deciding pick shows the room its 5 s hold, a second player's Object
  stops it, unobjected it lands once and `jump decided` routes the next visitor. A Trusted friend's
  Vouch adds +1 to your lie's tag; a believed lie is found out within a real day or not at all, and
  costs the voucher too; a Call it out on a true claim costs the caller regard. Overheard lines
  reach a third player within 8 m and not at 9 m.
- [ ] Bounty: a player at Kill on Sight with two factions is killed by another player; only the
  killer is paid, once, by one faction. Die to the infected (or a fall) while at Kill on Sight:
  nobody is paid. Collect the Bounty on a player pays only whoever killed them. With no player
  wanted, the hiding man (Bounty target, on SURV) shoots back at whoever comes near, and killing
  him pays.
- [ ] A Bastion outpost leader dies and is replaced (Replace minutes at 1): when that outpost is
  later abandoned or taken, the new leader leaves with it.
- [ ] A supply truck leaves, is driven by its crew, and arrives. Its whole crew gets in before it
  pulls out of the stronghold's yard, and nobody is left standing there.
- [ ] A leader answers Talk, Work, News and Standing.
- [ ] Hunted by Bastion, stay put through three hunter rounds: never more than two squads after you.
- [ ] Kill a truck crew from 300 m, then drive the truck in: no Fame. (Known gap: a friend who
  didn't shoot can still drive it in for Fame; watch for that pattern.)
- [ ] Retaliation in a crossfire: a Red Hand group attacking a Bastion outpost while you shoot
  infected 300 m away. If you get Bastion's retaliation for their hits, note it.
- [ ] Passing as Bastion, finish a Red Hand job 3 km away: the rival Infamy with Bastion is not
  doubled and nothing is blown (shares never double). Kill a Bastion guard while passing: doubled,
  and caught within 35 m.
- [ ] Start The Last Convoy (or wait for it), restart the server before the truck leaves: it
  still leaves within 10 minutes. Restart while it's on the road: the arc just ends.
- [ ] Questlines (**Wait scale** 0.1, **Minutes between** 5, **Once per season** off): Short Weight starts and its first
  contract is offered; it's in the journal with its notes once you've done something. Read the
  ledger next to the storekeeper (caught), then restart the server during the wait: the questline
  is back at the same stage with the same names. At the handover, warn the runner on one run and
  take the crate back unseen on another: the endings differ (Haven food and morale, headline,
  rumour). A second player who joins late sees it in the journal only once they're offered a stage.
- [ ] The majors (Questlines on, **Wait scale** 0.1): on the season's first evening the Haven's
  family waits at the edge of the house nearest the stronghold, and all four majors are in every
  player's journal with their act. At Collapse's second evening the Red Hand's send-off is offered
  at Rudi's crossroads (a road junction 300–800 m from the boss): ask a crew boss to come, hold the
  crossroads (waves); the back page goes missing and The Book's first stage follows. At the first
  sitting, speak for a rule to a head (one sway at this sitting), table Bastion's cards, wait out
  supper: the vote's rule is in the journal and the Haven's people say a line about it. Raise
  research to 49 percent with a fast pace (a sample run): a squeezed act's last call shows as a
  headline and in the journal, and its default follows a session later. Set **Lead points** 0 and
  stall research at 48: settling day doesn't open until 50. The notebook's The season tab lists all
  four majors with their act. After the reading, Before the stand is offered: sell the pages to the
  boss's man (with `proof.pages` 2), leave the line, and start the stand: the Red Hand's channel reads
  page one and the journal says Two Tins a Copy. With Bastion's stance 3 its group stands on the lab's
  road (150–250 m out), not at the gate.
- [ ] The finale's rest ([FINALE.md](FINALE.md#the-rest-of-the-finale)): between 25 and 50% an unmarked seed
  line comes on a faction's channel now and then; after the gate the readout comes on Halcyon's channel
  at 06:00, 14:00 and 22:00; locked, a disguise look with another route's faction shows *[Another
  route's -1]*; you're told of your route's marks on the verge; the night man at a desk buys a note
  about another route once, hands over what you carry to a rival within 10 m, buys a rival's item for 30;
  standing beside a rival's item on the ground for 20 s spoils it; *Guard the door* once, then a rival's
  sneak there fails about half the time; the Haven's terms one by one at her table; under Bastion,
  Command, a carrier is searched at the lab road's checkpoint for 90 s.
- [ ] The epilogue's rest ([EPILOGUE.md](EPILOGUE.md)): a new face's first spawn in the week gets Petr's
  note and The Key Where It Was; at most two of the deck offered within 3 km; the codas' standing
  versions at the chair's house, the roster board, the lab gate and the crossroads; the supper scene
  18:00–20:00; the queue grows and shuffles at the change of shift; leaders' *The cure*, *This place*,
  *Me* (with your title), *The dead*, *What you didn't tell me* and *Settle accounts*; the ridges' jobs
  below a pack still calling; the season tab's record lines.
- [ ] The cure finale ([FINALE.md](FINALE.md); Questlines on, **Wait scale** 0.1, research brought to
  90 with samples): at 75% a leader's "Ask about the cure." says to ask again; at 80% the stronghold
  leaders and the Doctor give the pitch; at 85% the four pitches and an unsigned one come on air once.
  The gate news and the lab's line come once; the cure shows 90; the five routes'
  opening lines are heard and each route's desk stands by its stronghold leader (the Independent's at
  the lab gate). With **Dialogue > Trees** off: "Write me down" without "What would it mean?" is
  refused; with it, within 30 s, you're locked, and another desk refuses you. With Trees on: the
  desks show no lock choices; a stronghold leader's (or a Loyal seat leader's) "Ask about the cure."
  shows even when they refuse you, its "What would it mean?" shows the lock card tagged [Point of no
  return], and "Write me down." locks you (dimmed with the reason when you can't). Finish a story job
  that leans to a route (the black box into the stove): a "What you'd bring" notice, and the lock card
  lists it. On a sworn stage, a player locked to another route takes "Tell her the truth about them"
  (the next persuade is a step harder for the route) and "Walk her to your own people" (+20 Haven
  Infamy; the stage comes back with "Talk them out of it"). Spoil the captain's freezers: the stage
  comes back as Processing Again. Do the Haven's Act I by the chair: the track readout says
  "The Haven's work is at two" and a tissue hunt is offered. Hunted by Bastion, Bastion's desk refuses
  you until you hand over a firearm at the price door. Two players on different routes: one kills the
  other away from the lab (+3 Infamy with the victim's route faction; a fourth and fifth kill in the
  hour cost nothing more). Finish a route: the others' stage work is called off, the seal calls come at
  60, 30 and 10 minutes, the stand starts with your route holding the room. Stand in the clean room
  (15–25 m from the lab) on a rival route: the "CLEAN ROOM" line under the noise meter counts you up
  (routes, never names; gone 5 s after you leave the lab); take it; let the 15
  minutes run out with a count going (overtime). After the win the release scene at the lab takes the
  terms; the cure's notice names the route and the terms; the hall of fame shows "The cure: ...". Check
  the groups at the lab (two at most) and that they only fire on rival-route players.
- [ ] The List (same settings; finish the contract The List first, or let a safehouse run short; with
  **Director > Majors** off, or as Every Name's act III):
  a free council head asks; the husband's trail ends at him (bring him home: +1 person) or his
  letter. Talk to a Loyal council head while Known (refused), read a house's list in its stores
  after dark next to a lookout (+8 Haven Infamy), solve the copies at the asking house, look round
  the empty farm, and at the chair's table read it out and hold the gate (two waves); on another run,
  carry the copies past a lookout (taken back: Buried).
- [ ] The Price of the Door (a safehouse short of food or that lost people lately, a Red Hand crew
  within 6 km): open the envelope (name and grid), get seen by a lookout (+12 Haven Infamy), deliver
  it; at the barn bring the exile home and speak for him at the vote (+1 person). Another run: leave
  the envelope, then at the gate tell the crew boss no (with the Red Hand group prefab) and hold
  it, and put the barricades up (the town is Barricaded); or take the debt and pay it in the pit.
- [ ] The Edge of Town (finish Walk Him Out first; the lab placed): at dusk take the bitten man to the
  lab (he follows; most of the time he walks out and the house gains one); read the walker's
  notebook beside him (taken back), then ask him instead; read the trial records after dark;
  broker the lab road at the nurse's and carry the Cold Chain batch; on another run refuse both and
  walk the last one out with the walker at dusk.
- [ ] Await Relief (**Wait scale** 0.1): at night an Independent Bastion commander offers the
  operator's tape in person; take it (or copy it, unseen, with a radio), play it at the relay and
  hold 3 min through the horde; walk the runner's three towns; on separate runs give the page to
  the duty officer, the commander, a crew boss's radio man and the chair's clerk: the waits,
  headlines and endings differ. No Reply's tape at an Independent commander's post starts it there.
- [ ] The Census (faction life off, or Bastion short of supplies or security): count a house by
  asking and from the watch point (seen: Haven Infamy), deliver and skim the ration crates, walk
  the curfew gently and, on another run, let the toll in (the crew holds the road until dawn), then
  the contribution, the raid (Red Hand group prefab) and the 20:30 call: as it is, high, low and
  torn up end differently, and Requisition then skips spared houses and goes to refused ones first.
- [ ] The Last Pilot (Perimeter on): finishing Last Helicopter starts it, and no Last Helicopter is
  offered while the helicopter is dry. Fuel it, choose a load, bring the black box from the warning
  zone (never the kill zone), then on separate runs talk him down, report him, help him to the far
  LZ and sell him to the Red Hand: after Grounded there's no Last Helicopter for 2 real hours
  (times **Wait scale**) and the next one names the new pilot; after the line, Bastion's Crash Site
  starts at his wreck near the far LZ (the line's truth, his name in the brief). Failing Rescue the Pilot also grounds the helicopter.
- [ ] The Three Weeks (same settings): solve Patient Zero's Ledger, then it can begin. The driver
  sits at the edge of the town nearest the site, talks to someone Known with the Haven and not
  Trusted with Halcyon (or a Radio Operator), or after two shares of food; the site is well away
  from every town; the three bodies give the code for the terminal (without one, reading all three
  opens the lock); Dr Wolff can be killed, caged (the case to the lab drop) or left while you take
  the notebook unseen. At stage 4 the duty officer turns away someone the director doesn't trust;
  each seat gives its branch (the raid, the field lead walking out, or Bastion's file) and its
  ending's headline.
- [ ] The Other Two (same settings): the clerk refuses a stranger; the ledger only opens at night;
  the courier follows you to the station (or hands you his case); the brother's clock starts when
  you reach the room, and walking him home ends at the sister's safehouse; the sample kit gets him
  up as an infected; selling ends the line; the sister's name choices appear only after the list
  and the man who walked them up; the names go to the director, the chair, or nowhere.
- [ ] Authors (same settings, and Cure > Research per sample raised and Lab research per day 0 so a few samples reach 50%):
  it doesn't start at 90% or more; the letter can be read and copied onto the field lead's desk;
  the cold case goes warm after 20 minutes; bargaining needs Trusted with both; stage 5 waits for
  75%. At the final stand, with Territory > Bastion guard set and the terms agreed, a Bastion squad
  stands at the gate; with the boss paid, a player Known with the Red Hand is offered Take the Lab.
  The ending comes after the stand, and the season's winner in the log is chosen before it.
- [ ] The Book (`Begin("book")`, from the season's second real day): point at the wrong suspect once
  (+5 Red Hand Infamy, the trail still opens); at the barn, talking him out armed is refused. At the
  page, each name is met at their own seat (their own body goes while the stand-in talks); turn one in
  at their stronghold (replaced, morale down) and a second is refused; let a debtor's leader die
  first and they drop off the page. Keep the page on one run: Favor from each faction collected
  from, and +15 Red Hand Infamy a session later.
- [ ] Cargo (Supply > Red Hand truck set): the driver only at dusk; threaten him and the van's guards
  fight at once. Cut the lock and the three follow you to a safehouse (+3); on another run start the
  van (a Mechanic at once) and drive it to the drop or to Bastion. With world flag `table.town` set
  to a settlement index + 1 the drop is below that farm and `cargo.buyer_known` is set at stage 3;
  without it, the crew boss's pit. Tied Down isn't offered while Cargo runs, and after Tied Down's
  shed Cargo sometimes starts as the Haven's.
- [ ] The Well Story: the keeper always refuses; the council head answers only someone Known with the
  Haven; the story only at dusk (an Ex-Con always gets paid). Warn them, then try the well after
  dark: always seen (+15 Haven Infamy). Fake it as a Nurse (harmless) and as someone else
  (sometimes +5 infection); take a sample to Halcyon; stand guard (the crew comes a minute later).
  The boss's verdict a session later takes Trusted; blaming the crew boss replaces them.
- [ ] The Long Table (**Chance** 1, a fresh season): the log names the farm; within 300 m its people
  stand there and talk. Eat (grace reads the last name taken, the boots show once), trade a jar for a
  magazine, declare a bite while infected. Alone, eat at 20:00 and wait past 20:30 by the door: the
  family sets on you once (a fight with the rival group, else a narrow escape); with a friend
  within 100 m, or bitten, nothing happens. Walk within 1.5 km alone: the host steps out once a
  session. Restart: the farm, its people and what it remembers of you are back.
- [ ] The House That Went Quiet (**Wait scale** 0.1; set the world flag `table.suspicion_add` to 30
  from a test, then talk to wanderers until one mentions the missing): the trail's three spots lead
  to the farm gate; at the guest stage decline and lie up on the rise unseen; take the proof at
  supper, then tell a Haven council head (the house is found out: headline, Haven morale −8), and
  walk the family out at dusk unarmed. On another season tell Bastion and get the boy out before
  the guns, or keep it to yourself and talk the mother out.
- [ ] A Place at the Table: eat three times in one life (not Trusted with the Haven): the host makes
  his offer. Do the dead (the traveller in the ditch), send the dying man the wrong way (he's a
  bitten wanderer on the roads after), walk the stray to a safehouse instead (Undercover: The House
  That Went Quiet begins at "Who to tell"). On another life, bring the stray and sit at grace:
  family, then found out (+40 Haven, +20 Bastion Infamy, your name in the headline).
- [ ] With **Post minutes** at 1, a road post goes up on a road ahead of you. At a Bastion
  checkpoint: Known, you're waved through; a stranger is searched at the barrier (a suppressor is
  taken); walking off mid-search costs 3 Infamy and they fire. Unarmed with Bastion Infamy, wait at
  the barrier: you surrender, lose 8 Infamy, and Hunted, they stop shooting for 5 minutes.
  Hunted and unarmed, stand still 100 m out: within about 15 s they shoot again (they hold
  fire only while you walk in; 30 s of hanging back ends it). Walk in slowly from 150 m: they
  hold fire all the way and you surrender. With a hunter squad after you within 1 km, they
  don't hold it at all.
- [ ] At a Red Hand roadblock: Known pays 10 Favor and passes, a stranger is warned then fired on,
  Trusted is waved through.
- [ ] Watched by Bastion, walk at an outpost armed: warned at 150 m, fired on past 50 m. Fire a
  shot inside the lab's safe zone: 2 Halcyon Infamy and the guards turn. Watched by Halcyon and
  bitten (taken), walk up to the lab unarmed: let in for the trial dose, not turned away.
- [ ] As a Bastion Champion, throw the signal smoke at a horde: shells 30 s later; again at once:
  refused (cooldown); next to a safehouse: refused.
- [ ] The fence buys a watch but not an item just bought at a quartermaster; the hourly cap holds.
- [ ] Bounty board: killed by B, post on B; a friend in your group kills B: nothing paid; a
  stranger kills B: they get 40 Red Hand Favor.
- [ ] Halcyon Champion: take the booster, get bitten: it doesn't take, once. The medic heals and
  washes out a fresh bite.
- [ ] Pay amends twice to Bastion: the second costs more; Infamy never goes below 15 by paying.
- [ ] Reach Red Hand Trusted: a red armband appears in your pack, once per life.
- [ ] Let a faction lose an outpost: a wanderer headed there turns aside and talks about it.

**Camp life** ([DESIGN.md](DESIGN.md#camp-life); the kits filled, set the game clock with the
editor's time of day)
- [ ] Walk up to an outpost of each faction: its pieces appear over about 20 s and stand on flat
  ground round it, none on a road, in a building or on the quartermaster; the log has no
  `Dressing: removed` line. Leave (past 350 m) and come back: the same pieces in the same places.
- [ ] It reads as a compound: barricades, the nest and the faction's sign across the approach, the
  fire and tables in the middle, the tents together at the back, the cook pot and the woodpile
  together on one side, the latrine and the line out at the edge.
- [ ] No piece gives anything: no search, water, boil, generator, radio or crafting prompt on
  them (the wash stand, the shower, the radio table included), no inventory, no heal or resupply,
  no vehicle, no respawn on them.
- [ ] Bastion by day: one in the nest, two walking a loop, one at the board, one at the fire. At
  17:30 someone else takes the nest. At 20:30 everyone gathers at the board for about 3 minutes,
  then only the nest and the fire are manned; walk 100 m off and back: the rest are gone (asleep)
  and come back after 05:30.
- [ ] Red Hand by day: two leaning at the barrier, one walks to the road and back. Haven at 18:30:
  everyone at the table but the lookout. Halcyon: a courier walks out and back within 20 minutes.
- [ ] The chores: after 05:30 someone at the wash stand (Bastion's shaving stand, Halcyon's decon
  shower); by day someone at the chopping block (Bastion, Red Hand, Haven); from 17:30 someone at
  the cook pot until the meal.
- [ ] Named people stay within 20 m of the centre; talk to one: they stop, and go back to work a
  minute later. Quartermasters and leaders never move.

**Faction bases on Everon** ([DESIGN.md](DESIGN.md#camp-life); after `build.py` has rewritten the kits)
- [ ] Walk up to each faction's stronghold and one outpost: the log's `Camp life: ... moves into ...`
  line names the building (Bastion a school, town hall or police station on a road; the Red Hand a
  garage or petrol station where roads meet; the Haven the church; Halcyon the lab or a clinic, a
  house for the field station). Note any that fell back to rings or picked an odd building.
- [ ] The people come out in the yard behind it (the Red Hand: its forecourt); the fire, tables
  and boards are there, the cook pot and woodpile together on one side, the line, garden and
  latrine along the walls, no tents. The bear board, the memorial and the decon shower stand by
  the front door.
- [ ] The gate is on the road in from the next town: the nest or the toll barrier, the red hand
  sheet or the plague board beside the road; the other road entries have a barricade beside the
  road. Every road stays drivable (a supply truck and a car get through).
- [ ] Bastion by day: a lookout at a front corner as well as the nest. Under a curfew season, two on
  watch at night. Off shift they walk in at the front door, and come out of it after 05:30.
- [ ] A Haven safehouse dresses its own house; a field station has the researcher and one other,
  and no extra comes out.
- [ ] Shoot near a camp (when Hunted): they fight as before, then go back to their jobs.
- [ ] Do AI walk through the tents (the navmesh VERIFY)? Is one more person per camp and the server
  frame time fine with 3 live places and a stronghold's 20 pieces?

**Traces** ([TRACES.md](TRACES.md); the log's `Traces:` lines, `/traces` and `/traces town <name>` as an
admin; [the spike](#traces-the-workbench-spike-phase-0) first for the decals)
- [ ] The log's first-days line names a gym and a church on the map, and `/traces town <name>` for a town
  near the north road lists X's dated Day 2–5 (never later), each with a section and a found field.
- [ ] Walk into a town with two or more marked doors: one **Doors** notice, the three biggest things,
  once this session (leave and come back: nothing). Stand 2 s at a marked door, on foot, with a pad:
  its line comes once; again at the same door: nothing. Drive past: nothing.
- [ ] The same door read by different characters: a Police Officer reads Bastion's code in full; a
  Hunter gets how many and how old on scratches; an Elder reads names under a candle; anyone else the
  glance. Known with a faction reads that faction's marks.
- [ ] A door with three layers opens the reader (UI > Reader on); B closes it.
- [ ] The notebook's Journal has **Read on doors** with the last lines, newest first; die and it's gone.
- [ ] Finish Protection Racket in a town: a red hand on the shop with one more stroke; one in a town with
  an X on the shop: the X now reads "Under the paint, still readable".
- [ ] A safehouse empties (or `house` dead go up): its bear gets a ring and an arrow; a candle's number
  goes up. Finish Walk Him Out: hatching in that town.
- [ ] Restart: the first-days marks are the same doors with the same words; live marks are still there.
- [ ] Die after a 2-hour life within 3 km of a safehouse: a grave (or its line) where you fell, with your
  description (your name only on **Named**). Die briefly in a town far from any house: no grave.
- [ ] Errands' old mark: its line reads at your level and is in the journal; no old mark in the
  Long Table's ground (no marks at all within 2 km of its farm but residents' Day 1–3 words).
- [ ] The first days' scenes: the log names each `f1`-`f8` it placed. Drive in from the north barrier:
  the column's cars on the verge (or their line once, **On the verge**), spread along the road from
  about 400 m to 1.2 km back from the barrier and clear of junctions, the suitcases nearer the
  barrier, then the crosses; nothing blocks the road. Park on a scene's spot: its prop doesn't come up
  until you've moved off (8 m). As a Police Officer the column's line adds the
  ticket numbers. Stand 2 s by a car on foot: the dashboard sheet opens in the reader (B closes it).
- [ ] The gym, the church, the roadblock (its radio says the broadcast's line as a subtitle within
  12 m), Rudi's shutter, the hill and a square's notices (`/traces` near each lists the scene): one
  line each a session, the paper where there is one. Restart: the same places, the same words.
- [ ] A vignette: let a town nobody holds go dark (or fail Survivors Trapped, declare a bite at a
  safehouse): `Traces: scene v1.N` in the log, its line once within 40 m; no more than 3 dealt a
  season day, never two in a town, none within 300 m of a running job.
- [ ] Live traces: lose a supply truck (its wreck on the verge with the destination's stencil), burn a
  building (scorch at its doorway), let a Bastion road post go (sandbags and the HALT sign). A kill
  site in a town near a Haven house ends: a candle on the nearest door. Help the Digger stranger dig:
  a grave by the road. As a Haven exile, spend 20 minutes in a town, restart: the thumbprint still
  comes 2 hours later. Die in
  the kill zone near a barrier: one more cross outside the warning zone.
- [ ] After dark at a lived-in place: a lit lantern (with the kit's lantern); at 05:30 and 17:30 near a
  Bastion holding the loudspeaker's line; as a Hunter near the town on a pack's first trail before
  its hunt, *"The dogs in ... have stopped."* (not on a night it doesn't set out); after a Haven Champion's death, one bell at 18:30 and the church's jars.
- [ ] Status line: lights and loops count up and down as you come and go; nothing stays up after you
  leave. With **Sound and light** off, nothing at all.
- [ ] **Traces' rest:** the gym's beds and the church's board stand at their buildings' own doors;
  Rudi's red hand is on a garage's shutter near the crossroads (with the kit's Red Hand board); a
  square's notices stand by its monument, bank or station. With **Kit > Pieces that age** entries, a
  scene's extra pieces stand round it and drop as it ages, and a board shows its faded twin when old.
  With **Indoor scenes** on, a town gone dark's last meal is laid inside the house. After dark at a camp
  the lantern sits exactly on camp life's table lamp; a Halcyon station hums from its generator piece
  (quieter at night with the night hum); a Bastion holding's chime comes from its loudspeaker pole. A
  walk-out who's ended leaves no chair; one who walks on does. While a questline's clue is near, no
  scene of its faction is dealt within 1 km (the log line). A Specimen from the Nest job puts a ringed
  beak on a door by the nest as it starts.
- [ ] The cure: `Traces: handed over to the epilogue`; the graves come down; no new scene or mark but
  the epilogue's (a name chalked as a candle at a safehouse, GONE HOME by its door); the epilogue's
  town beats end with the census (*"Four candles on the doors in Brod, and one bear."*).
- [ ] Nothing pays: Fame, Favor and items stay the same through all of the above.

**World**
- [ ] Walking near the map edge: drone and loudspeaker at 300 m, warning shot at 100 m.
- [ ] A safehouse shows occupants; you can hand in food, declare a bite at a Shelter (smaller
  slowdown) and a Town (bigger), and buy expired antibiotics for 5 Favor.
- [ ] As a Haven Champion, walk into a safehouse with a hunter squad on you: it turns away, and
  a bounty notice names only a nearby town. Die as a Haven Champion: the memorial headline goes
  out, and your next character is greeted and treated free at their first safehouse.
- [ ] A stove offers "Brew a home remedy" with alcohol and herbs in your pack.
- [ ] A wanderer can be talked to and remembers you.
- [ ] Bodies: kill a few infected near you. There's no "Search the body" prompt; open a body's
  inventory as you would any body's: now and then something is in it (bandages, ammo, a key with a
  tag), or lying beside the body if it has no room; sometimes a note lies by it ("Read it").
  Kill a big group at once: only the first 8 a minute carry anything. A special still drops its
  sample on the ground.
- [ ] Loose loot: walk into a house and a shop. Up to 2 (3 in a shop) things from the loot pool lie
  on worktops, desks and cupboards or the floor beside furniture, inside the walls, not floating or under the
  floor; nothing in a building offers "Search". Pick one up with F (or from the inventory's nearby
  list). Walk 100 m away for a minute and come back: what you left is still there. Empty a house
  and come back after an hour (**Loot > Refill minutes** at 2 for a quick test): it has things
  again. A pharmacy, police station or garage has up to 4; a locked one, once open, one more.
- [ ] Loose loot and performance: walk into the middle of a town and note the server frame time
  while the buildings around you are put out (three a tick).
- [ ] A letter lying in a house reads with **Read it**; now and then a key with a tag lies next to
  it and opens that kind of building in that town.
- [ ] Car Keys: the Mechanic's keys lie in one of the five houses; picking them up starts the
  car with its keys. Research Data: the drive lies inside the clinic, not outside it.
- [ ] Restart the server with loot lying in a house (vanilla persistence on, if you run it): the
  house shows only one set of loot afterwards.
- [ ] A tyre and a bundle of planks (spawn them, or find them in a garage) lie on the ground looking like
  a tyre and planks, show the pick-up prompt, go into a backpack and show a sensible inventory
  preview. Both were built by text on vanilla `Item_Base` with the prop's mesh. If either has no
  pick-up prompt or falls through the ground, open it in Workbench next to `Prefabs/Items/Food/MRE.et`
  and set its MeshObject and RigidBody up the same way; if that fails, give it a vanilla item's
  mesh instead and keep "tyre" or "plank" in the file name. Planks on the ground still offer
  "Board up the doorway".
- [ ] A fixable car, with **UI > Car job layout** empty: **Look over the car** lists two or three
  parts and, while you carry no tool, says you need a wrench or a toolkit. **Fit parts** refuses
  without one and works with the vanilla
  repair-kit wrench (the Mechanic starts with one); **Fuel it** needs no tool. Hotwiring takes
  about 45 s (15 s as a Mechanic), draws infected, and fails about one time in three (rarely for
  a Mechanic) with "The wires spit..."; trying again can work. Keys are rare. Starting it gives a
  drivable car and the log line `Cars: ... fixed by player N.`
- [ ] The same with the layout set: **Look over the car** opens the repair screen, and the car's
  other prompts are gone. The header names the car with "N of 5 done"; the markers sit on the
  car where the jobs are (the side view once **UI > Car picture** is set), olive done, amber you
  can do, grey you need something; the selected row's marker has a ring. With the D-pad and A:
  fit the battery (the row counts down, "Fitting... 6 s", then turns olive and the status line
  says what was fitted); try a part you don't carry (the row says "None on you" and A does
  nothing); hotwire ("No keys. About 45 s, loud, can fail") and watch it fail or work; then Y
  starts the engine, the screen closes and the line is a card. Do the same with the mouse and with
  Enter and T. B closes it; walk 6 m off with a job under way: it stops ("You moved away"). Let an
  infected close in: the screen closes and the job goes on if you stay.
- [ ] A locked place's outside door says "Locked" from outside and opens from inside (which
  unlocks it); its interior doors and shutting any door always work.
- [ ] At the door: Unlock with the right key (the wrong one says which tag you need), Lock it
  again, Force it with a crowbar (loud; it can't be locked after). The loot inside is better.
- [ ] A key item can be dropped, picked up by someone else and opens the door for them;
  "Read the tag" shows its label on the ground.
- [ ] With **Relock minutes** at 5: the door is locked and pulled shut again once you step away.
- [ ] From a closet or small room behind a door near the outline, the door opens for you.
- [ ] Doorway blockers: with the navmesh debug view on, a locked door near you shows a gap a few
  seconds after you arrive. Lead infected and a hostile AI group at it: they route to another
  door or wait outside; they don't open it. Note the server frame time when blockers come and go.
- [ ] Infected pressed against a locked door make noise (heat), and you're told once.
- [ ] Fire: with a full fuel can in your hands, walk up to a house: **Douse the doorway and set it
  alight** appears at its door within 10 s; a second player without fuel beside you doesn't
  see it; put the can away and it's gone. Five seconds later the house burns and the can is gone.
  No prompt at a safehouse, an outpost or the lab. A thrown molotov still burns the house it
  lands by. If the prompt never shows with the can in your hands, the server doesn't see the held
  can (the VERIFY in `RZ_Interactions.c`): set **Traps > Fuel in your hands** off.
- [ ] World events (raise a weight to start one): Fog Bank muffles heat and brings spawns closer
  only inside its radius, and both reset when it ends. Power Surge marks the town Powered for 10
  minutes (status log, the lights hook, electronics in searches) and dark again after, unless the
  grid powers it. Gas Station Fire goes up after 3 minutes and burns at most three buildings.
  Tunnel Rats' culverts each
  send infected and can be collapsed only with none within 12 m.
- [ ] People events: a Radio Operator gets a private line on both distress calls (live or a tape);
  the fake's two groups appear within 400 m, close in at 60 m and leave a Red Hand Known player
  alone. Prison Break's convicts walk toward the Red Hand camp and fight you, not a Red Hand
  friend. The Sniper shoots everyone near him. Rival Survivors fight a Hunted player only, and
  trade a crate for food with others. Collapse Shelter's people join the safehouse when told the
  way and turn into infected when their time runs out.
- [ ] Loot events: the Abandoned Convoy's tripwire goes off for the first player at the crates
  unless a Mechanic got within 25 m first. The Remnant Camp gives Bastion Infamy once per player
  at its crates while Bastion is around. Locked Bunker: the building's door refuses key and
  crowbar with the keypad message, the right code from the three notes opens it, the crates
  stand inside on the floor (not under it), and the door works normally after. Wedding Party
  drops jewellery, and a Trader wanderer takes it instead of Favor.
- [ ] Car Keys: "Ask about cars" at a Mechanic wanderer offers it; searching each of the five
  houses, only one gives the keys; the car starts with them (or hotwired, for half pay), and
  parking it at the Mechanic pays once you get out and removes it.
- [ ] Mission spots: start a few delivery and puzzle missions in towns with locked buildings.
  No item, note, person or crate is inside a locked building or in its doorway.
- [ ] Nobody gets trapped: unlock, force and the inside latch remove the blocker at once; climb
  in through a window and walk out of the door; stand in the doorway while it relocks. Nothing
  stops you and nothing feels in the way.
- [ ] Without `RZ_AIDoorHook.c` (deleted) and with **Navmesh rebuild** off, blockers alone still
  stop infected at the door.
- [ ] A thrown bottle draws nearby infected; a puzzle mission can be finished.
- [ ] Night: missions that need the dark are offered; the log says "night".

**Errands and the story blocks** ([ERRANDS.md](ERRANDS.md); a safehouse, a Bastion post, a Red
Hand camp and a Halcyon station near each other help; the admin reply **[Admin] Errands near here**
on the talk screen says why something wasn't dealt)
- [ ] A safehouse's first two occupants have names and roles ("Talk to Mara Kovac"); **Anything need
  doing?** on one offers a true errand, its ways become their replies with tags, and a second player
  picking one hears "Give us a minute."
- [ ] The Pot: with the house short of food, hand over two tins; the house's food goes up by 2 and
  the pay is a few Favor and 1 Fame. Ask again at once: it's cooling down.
- [ ] A Bastion post with its relief missing shows one guard fewer; Shift Change's "Stand the post
  yourself" pays after 3 minutes at the gate and the guard is back next visit.
- [ ] Walk past a house or post: a named person waves you over (once, then not again for 8
  minutes); never while infected are within 30 m or someone fired near you.
- [ ] A held errand whose fact stops holding (the truck comes in, the fault is fixed by someone
  else) ends with the giver's line and no pay.
- [ ] Errand Fame stops at 39 with a faction and at 10 a season day; at 15+ Infamy an errand pays
  2 Infamy off instead (8 a life at most, never below 15).
- [ ] A wanderer's **Need a hand?** (the Doctor near dusk: Before Dark; a Kill on Sight player:
  Wanted Poster; otherwise Barter Chain may start).
- [ ] A card (Sleeve Down after a bite near a big house; His Effects after a lost truck) comes once
  this season; the telemetry line every 30 minutes counts it.
- [ ] A choice mission (Crash Site) taken to the other side pays the decider and their group at
  most the base, and pays the others who did the work half the Favor from the giver.
- [ ] A hungry house, a lost truck or a town gone bad starts one answering contract near it, its
  brief opening with the reason's line, at most one every 30 minutes.
- [ ] The journal shows a held errand (the giver, what to do, minutes left), and a selected
  contract's detail adds "Who it went to" once it's decided.

**Contracts on the building blocks** (start one by raising its weight, or wait for the radio)
- [ ] Escort: Refugee Column's group walks when you're within 150 m, stops when you run ahead or
  infected come within 25 m, and joins the safehouse on arrival. With the escort prefab empty they
  move as data and stand as a civilian figure when they stop. A fourth escort contract never starts.
- [ ] Rescue the Pilot: help him up (talk, or stand beside him 10 s) and he follows you; the clock
  stops for a Nurse; killing his AI body fails it.
- [ ] No Reply: the operator's bearing and the mast's cross within 45 m of the source; a Radio
  Operator away from the post gets a bearing every 2 minutes; the log choices appear on the
  operator only after the source is found, and only a player Known with Bastion can pick one.
- [ ] Struck Off (offered from broadcast Day 13, so not on a fresh season's first two days): his
  pack by the post, the trader (or the Deserter wanderer) gives his grid; his KIA date on the roster
  is between Day 11 and two days ago, and always Day 11 on Day 13; talking to him tells the room the
  truth; the stores book by the board reads only after dark (and adds "Report the stores book" when
  the truth is written off or the gap); the five choices are on his kit, "Talk him back" is a Trust
  check; killing him drops a tag that the roster board at the post takes ("Hang his tag on the nail").

**Contracts on the world facts** ([DESIGN.md](DESIGN.md#world-facts), "What uses it")
- [ ] Retake the Town, Burn the Nest and Checkpoint Duty never land in a town with a safehouse, a
  holding or the lab; the Haven's Harvest Guard never in a Bastion or Red Hand holding; Fence the
  Goods' fence and Crash Site's drops sit in their faction's town when one is in reach.
- [ ] Patient Zero twice in one season: both trails end at the same clinic, and the records name
  the same man and date. The Real Orders read the same words every time.
- [ ] Fight Pit and The Pit's Debtor use the same ring all season, and never run at once.
- [ ] Start Await Relief, The Edge of Town or Cargo (`RZ_Questlines.Begin(key)`): The Real Orders,
  Walk Him Out and Tied Down aren't offered on their own while it runs (their questline stages
  still start).
- [ ] Kill the Officer's brief names whoever sits in the chair when you open the journal (kill the
  commander some other way, wait for the new one, read it again).
- [ ] In Aftermath, Bastion's contracts and road posts are rarer; Bastion Shelling and the Fire
  Mission never pick a town within 300 m of a safehouse or holding.

**The story jobs** (wave 3; check the log's `Story:` lines for the truth drawn)
- [ ] Protection Racket: the storekeeper's options change when the receipt (paid twice) or the girl
  (her cut) has been read; Press costs Haven Infamy through the threat; Cover it takes 4 food or 20
  Red Hand Favor and the next racket at that house finds the lie (`Story: lie racket.covered found
  out by tick`); Side with the house waits 5 s and can be objected to; a non-contractor picking a
  deciding option hears "It's not your call". Time up: `Errands: fallout pot ...`, and the house's cook
  offers The Pot to the participant first.
- [ ] Lost Colleague: hiding (choose at him), turned (kill him; the notebook's two choices) and walked
  out (the camp's note, then the new man at the house with five choices); research +2 or +1.
- [ ] Lost Colleague's search: the job's spot is his cold box first; its read leads to his pack,
  whose read moves the spot to him (`Story: colleague <name> trail found by <player> (trail)`). The
  scavenger by the cold box takes a tin for the grid; the Haven trader in a lived-in town gives it
  for nothing. Hiding, "Walk him in": he follows you to the Halcyon station or the lab (with Contracts
  > Escort scientist set, a body you can lose to the infected); at the gate the job ends as done; dead
  on the way, Notebook 31 is dealt. With no escort free, Halcyon's stretcher line, as before.
- [ ] Crash Site: the wreck and cockpit appear at 400 m; the cockpit read tells the room the truth;
  already taken: two Red Hand carriers at a fire 300–600 m off, the box by the fire once they're dead;
  inventory taken to the Red Hand puts the manifest on the open channel. The Last Pilot's line ending
  starts a Crash Site at his wreck. Time up without the box: Buy It Back at a Bastion post within 3 km.
- [ ] Distress Call: answering is told within 800 m (live, a loop, or "Say it again"); a loop springs
  at 60 m; bait has "Cut them loose" (at night, quietly), "They're mine", Buy them, Side with the crew
  (the hold; a Haven witness or Haven-Known player within 150 m: +12 Haven Infamy at once, else a lie
  the next Tied Down finds). People dead: The Name on the Tag at the nearest house.
- [ ] Survivors Trapped: after the hold the mother appears with three choices (a 5 s hold); nobody in
  90 s sends them to the nearest house; a bitten father (one time in four) is seen by a Nurse or
  Doctor and walked out at the door.
- [ ] A Tier 0 job failing (Walk Him Out, The Rat, a leader job such as Debt Collection) sends its fallout
  line after the fail text; the next one offered in that town says in its brief how the last went.
  A player it wasn't offered to picking one of its choices hears "It's not your call".
- [ ] Walk Him Out, let walk on: at dusk the next season day, near a holding's approach, the stranger
  Still Walking may ask the way to the lab.
- [ ] A talk scene (any of the twenty kit jobs, e.g. The Cache's scavenger): its person appears at 80 m;
  Talk says the first node's line to you and anyone within 8 m; an ask opens the second node; a
  second player picking a check while the first holds the floor hears "... is talking to them";
  "Leave it" goes back to the first node; a Decide runs the 5 s hold, and the use slots read Talk,
  Object, Stand with while it runs; Object cancels it and gives the objector the floor. With a pad:
  every slot reachable, no mouse. With **Missions > Story > Numbered scenes** on, the line lists the
  options and the slots read Option 1 to Option 4.
- [ ] A service (a Nurse at The Cache's scavenger, a Mechanic at the pilot's farmer): stay within 4 m
  for its seconds and the way passes; walk off and it ends.
- [ ] Errand pay: at a Bastion post with **Props > Pay ration** set, one errand in three puts a ration in
  your pack; handing it in as food at a house says *"These are ...'s."*; a house errand at a fed house
  pays supper (some health back); with Bastion short of supplies, a post's pay waits for its truck
  (`Errands: ...'s pay waits for the truck`), and comes when it arrives.
- [ ] His Effects: the effects go in your pack at the wreck; a friend picking them up spoils it (+3
  Infamy); selling them at the fence pays 3 and 10 Red Hand Favor; the card comes again for another
  dead man, at most 3 a life.
- [ ] Strangers: one appears 60–90 m off, out of sight (no pop-in), and goes once you can't see them;
  a junction of two busy roads deals one; robbing a dead one's pockets (within 3 m) is +8 Haven Infamy.
- [ ] A local you helped walks up as you pass their house (once a real day); their labels show only
  while you (or someone who knows them) are within 100 m.
- [ ] A lie found out: a Paranoid leader of that faction offers you no in-person job for 2 hours of
  play; on your next life, that faction's next errand opens with "The last one who said that lied".
- [ ] A major's act opening logs `Story: reason major.<key> posted`, and its margin card opens with the
  act's line.
- [ ] Tricks: a trick within 25 m of a leader's seat is refused; in a story job, a hidden participant
  within 25 m of someone in the giver's kit breaks their look.
- [ ] Each kit job once: its truth in the log, a read for the right background at its place, the tins
  way and the check way at its person, the end question's options by truth, and its record. A
  callback at the door (The Cache's family, the boy from Bust Out) turns up as a person beside you
  some hours of play later, then goes.
- [ ] Protection Racket: a Short persuade, then the next collection at that house: the brief mentions
  the promise. Collected: a note at the house comes back some hours of play later.
- [ ] The bed by the stove (Dialogue > Trees on, a short house): someone Known with the Haven decides;
  a Stranger sees the choices dimmed ("only someone ... trusts"); the room hears it. Leave a talk open
  with another player standing beside the person: it closes after 45 s.

**Phase 8's errands** (the admin reply's "why not" list names each card's reason when it isn't dealt)
- [ ] Each new card is offered only while its reason holds and lapses when it stops (`Errands: <key> for
  <player> overtaken`): Masks Off after a bite at a house, After the Noise after The Tribe Moves, The New
  Number after a cure milestone, Names on the Call after people went to Bastion's post, The Chalk on the
  Door after a work job's tally, Last Orders after a leader died, The Market after a Wedding Party, His
  Lighter after a death near a crew camp, The Receiver with a tagged pack near a station.
- [ ] The margin cards: Clean Glasses while The Book Has to Balance waits for its settling day, never
  16:00–21:00; Count for the Sitting while Every Name waits for a sitting, before 17:00. The lock card
  (finale on, past 90%): lock to Bastion, help a local at a house, then ask there: *They Say You're With
  Bastion*. Hand to Hand only while unlocked.
- [ ] Lost Colleague failed while he hid: Notebook 31 is dealt at a Halcyon station within 3 km,
  participants first.

**Strangers** (Missions > Errands > Strangers; the admin reply names the spot kinds where you stand)
- [ ] Walk a road you've used a lot (by day), toward a safehouse's approach, past a road post, along a
  wanderer's way, or to a town edge at dusk or in fog: a stranger may stand 150–350 m ahead
  (`Errands: stranger <kind> ... (<spot>)`). Crossing empty country, standing still, in a car, with
  another player outside your group within 300 m, near a lair, or within 2 km of the Long Table's
  farm: nobody, however long you wait.
- [ ] Stop your car 20 s by day near a wreck or on a used road and get out: someone walks up 60–90 m
  off (Two in the Back, The Funeral for the Car). Boil water on a stove after dark: Warm Hands may
  come to the fire; let him sit 5 real minutes for one true thing.
- [ ] Up on the Roof: "Talk them down" refuses with infected within 25 m; done, +1 at the nearest house
  and 4 Haven Favor. The Last Tin: a tin for a true tip, and he greets you at his house's gate later
  (a local on an occupant: **Talk to {name}**, "You gave me a tin on the road").
- [ ] Ways that send you somewhere (walk her past the post, dig with her) say where; the stranger waits
  10 minutes while you do it, then gives up. Only you or your group can answer them.
- [ ] Kill a stranger: +15 Infamy with their people, and no stranger finds you for a real day.
- [ ] They call out once at 40 m and are gone 5 minutes later or once you're 150 m away; the next one
  comes 15–25 minutes later at the earliest, never at the same spot within 2 hours.
- [ ] Found hooks: finish a few jobs; at about one site in five a stranger is there afterwards, or an
  old mark is read out as you pass within 30 m. A bitten stranger sent on waits at the house's gate:
  **Who's at the Gate** there.
- [ ] Locals: a local dies with their house's losses now and then, and only players who knew them hear.
- [ ] Board asks: read a safehouse board while something is true there: up to two "{house} asks" lines
  above the notes. Radio asks: walk 300 m–1.5 km from such a house: one personal line on the Haven's
  channel, at most once an hour.
- [ ] A few cards end to end: Vouch at a Haven house (Known), Borrowed Green after declaring a bite at a
  house with a Bastion post toward the lab, Name Day at a Red Hand camp (no crew name yet), Shoes in a
  Row by day near a lair. A lie (Fresh Chalk without looking) is found later and costs a little Infamy.
- [ ] Dress Rehearsal (Known with the Red Hand, a Bastion post within 2 km, disguises on): at the
  barrier not passing as Bastion, it's over and the open channel laughs.
- [ ] The disguise side jobs: kill nothing, let a Bastion guard near a post die to the infected, and
  a Bastion named person (Known) offers The Name on the Tape; bring the tag home and the nightly call
  reads the name with "jacket home". The player who shot that guard is never offered it. Two
  different players caught at a Bastion post: Spot the Tells. At night near a Bastion post: Paint
  (the trick ends at the barrier if you're not passing; plain sight brings the post's fire). A
  Halcyon death: Notebook 31; keep the notebook and the next Halcyon look shows "[Notebook N +1]"
  once. Laundry Day ends if you walk within 150 m of the roadblock.
- [ ] Live Capture: lure the special into the cage and drop the door within 4 m; it vanishes without
  a sample, the cage area gets loud, after 30 s the case appears; deliver it at the lab drop before
  12 minutes. Killing it fails; wounding it to a third and standing over it ties it.
- [ ] Vehicles: Recover the Bus counts the spawned bus and any truck or van driven into the safehouse
  zone, and pays once nobody is sitting in it; Car Theft counts only its own car.
- [ ] Convoys: Convoy Escort and Hijack the Convoy ask for Bastion's next truck within two minutes;
  the breakdown stops and restarts the crew; robbing it completes the hijack, its arrival the escort.
- [ ] Blood Drive takes a quarter of your health (check it shows), refuses under 70% and a second
  time in the same life.
- [ ] Patient Zero: each town's body appears only when you're near; a Lab Tech reads a grid in it.
- [ ] Take the Lab during a final stand: 90 s in the clean room ends the stand, research shows 90%,
  and the Red Hand antidote stock goes up by two.
- [ ] Mobile Lab: the field annex takes samples and disappears after 3 hours.
- [ ] The List: the safehouse's people and list appear when you come near; its last clue (cairn,
  bedroll, bag or person) appears only after the list and the council member.

**Multiplayer and griefing** (two clients at least; the pass 2 sweep's open risks)
- [ ] Two players race one choice delivery: the first hand-in wins, the second is told it isn't
  wanted. A player who isn't eligible delivers someone else's contract: only eligible people at
  the drop are paid. Take a choice item to the other side: it pays everyone with you whom that
  side doesn't hunt, and the giver's leader neither remembers it as help nor scales the pay.
- [ ] Mirror offers (set **Missions > Story > Mirror chance** to 1): The Cache and The Armory tell
  the second side 5 minutes in; The Cache goes to the side with more holders at the end (a draw is
  half each), The Armory to whoever hands the case in, and dropping the case down the well pays
  nobody and costs both sides. With **PvE** on, no mirror.
- [ ] One-choice jobs (two clients): finish Harvest Guard or Lab Defence; a stand-in person appears
  with the options (`Story: <title> asks`). The second client, not offered the job, is told "It's
  not your call."; picking starts the 5-second hold the other may object to once; nobody for 90 s
  takes the default. Fight Pit asks in round 3, Kill the Officer within 40 m of him (taking the
  stores ends it paid by Bastion at half Favor). With no civilian prefab, the default is taken at once.
- [ ] The worked examples' remaining ways: Crash Site's sneak lift (hidden within 8 m in the first
  90 s), the recovery team at minute 15 (Trusted / Known / Watched; with Contracts > Bastion squad or
  the hunter squad set, a real section climbs down on the road and walks to the wreck:
  `Story: Crash Site group 'recovery team' at <grid> (N on the server)`; gone when the job ends), the carriers' fire (persuade, pay,
  "That's ours, lads" in Bastion kit) and the open channel at the radio station (10 s with the box);
  Lost Colleague's Sold truth (the fence's table at night, unseen) and the Haven medic; Distress
  Call's Sneak out the back and the three destinations; Struck Off's Press and Trade at a Haven door,
  the Hunter's tracks and the Soldier's shortcut, and let go becoming the Deserter; Protection Racket's
  runner (Off the book) with a Rogue crew boss near. Each new outcome logs `Story: <title> record`.
- [ ] Leave: at a one-choice job's stand-in, "Leave it to them" says you're out; you aren't paid when
  it ends and can't pick its options. A Watched player who pressed or tricked in an amends job is paid
  Fame, not amends. A story job finished while one who took part (5 minutes there) is offline: at their
  next join, "Finished while you were away" with the base pay (`Story: <title> pays ...`).
- [ ] The four tips, once each: the first tagged options (an errand's ways), the first truth found, the
  first background read, the first hold someone near you starts.
- [ ] "Hid a bite": as a Nurse, dress the hidden bite in Survivors Trapped or Distress Call; that
  house's errands then tag their talk one step harder for you (e.g. "[Trusted - Haven]" where it was
  "[Known - Haven]"), and Protection Racket's persuade at that house is one harder.
- [ ] The new cards: with Await Relief waiting for Effective Strength (or the muster), a Bastion post's
  people offer Weather, Write It Down (or Clean Rifles), never 18:00–22:00; with Proof waiting for the
  reading, a Halcyon station offers Clean Paper and Chairs for the Reading, never around a change of
  shift. A Champion with Bastion, the Red Hand or Halcyon is offered The Kid's Inspection, A Story at the
  Fire or The Visitors' Log; none of them pays.
- [ ] Chore slots: with one player online, a second routine at the same house isn't dealt while one is
  offered or held there ("every chore here is taken" in the admin view); a player never holds two
  routines at one place.
- [ ] Threads: do The Pot, then ask the lookout at the same house: the offer opens *"<the cook> says you
  brought something for the pot."*, once. A callback: bring Boarded Up's family out; some hours of play
  later near the house, the line arrives.
- [ ] Strangers: fire a shot beside a stranger with no infected near: `Errands: stranger <kind> shot at
  by <player>`, +4 Infamy, and they're gone. A new character on a first life gets no call-outs until 10
  minutes after reaching a safehouse. A radio ask never comes within 5 minutes of a contract offer.
- [ ] Errand props: with **Missions > Errands > Props** set, His Effects' wreck shows the effects,
  The Truck Didn't Come's wreck a crate, Last Orders' seat the papers and Filter Parts' pump house the
  part while the step is open (`Errands: <key> prop <kind> at <grid>`); each goes when the step is done
  or the errand ends. Empty slots: the same errands, words only.
- [ ] Work rows: finish Retake the Town in a town twice; the second brief there reads the town tally
  ("cleared 1 times" is never said: "once"). A failed work job says its fallout line after the fail text.
- [ ] Route assets (finale on, past 90%): lock one player to Bastion, then pick Steal the Antidotes'
  Haven medic: refused with "You've picked your side. This isn't it."; an unlocked player is not.
- [ ] The merged jobs: Take Back the Town once Bastion is gone (Overrun+), Burn the Bodies at the
  lab from Halcyon, Checkpoint Duty at an infested town pausing its spread, Medicine Run's infected
  drawn off by a noise, and the Courier's letters through an infested town paying ×1.25 when quiet.
- [ ] A personal notice (a mission brief, a tier change) shows only on its player's screen: with
  a second client watching its radio log, nothing of yours appears there. Broadcasts still reach
  both.
- [ ] Carry a delivery item (Courier letters, a Bastion case or the capture case), then
  disconnect: the others are told and the item is back on the ground
  where you were. Die with it and wait for the body to go: same.
  Check the log for an item that comes back twice from one disconnect.
- [ ] The Courier's letters taken off the one who asked and delivered by someone else pay nobody.
- [ ] Escort: the player the pilot or scientist follows disconnects or dies; the nearest player
  within 20 m gets "Now they follow you", and the job goes on.
- [ ] Locks near missions: start a puzzle or delivery mission in a town with locked buildings;
  buildings within 25 m of its props open (the vanilla Open works), and lock again (doors pulled
  shut once nobody stands in them) when it ends. The Locked Bunker's building stays locked.
  Watch the log and frame time with many missions running (the spot list is rebuilt once a tick).
- [ ] Keys: search bodies and readables in a town with no locked buildings: no key turns up.
  Every key found names a building that really is locked in that town.
- [ ] Spam Lock and Unlock on one door: after one turn it says "The lock's stiff" for 5 s; the
  blocker comes back no sooner than its cooldown.
- [ ] Blood Drive while the bite detector is on: no "bitten" message from the health drop.
- [ ] Disconnect while bitten and have a friend look at your "Clean the bite" spot: no new
  `local_` profile appears in `state.json`. Old saves' `local_` entries with nothing in them
  are dropped after one save.
- [ ] A Nurse slows a friend's bite, restart the server, try again in the same life: refused.
- [ ] Shoot near a wanderer while infected are on them and let them die: nobody is blamed. Shoot
  one yourself: you are (the log says who).
- [ ] Drive and boat into the Perimeter's warning zone: the drone follows over the vehicle (not
  stuck where you entered), the kill zone still shoots, and leaving removes the drone.
- [ ] The Missing Courier's radio offer is titled "The Missing Courier", not "Lost Colleague".
- [ ] Ring bells, sirens and read clues repeatedly: one bell or siren per 15 minutes, no extra
  spawns or rewards from rereading.
- [ ] 1 player and a full server: spot cap (70) and escort (3) and capture (2) caps hold; with
  everyone in one town some players get no interaction spots until others leave (expected).
