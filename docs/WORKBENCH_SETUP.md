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

## 1. Create the prefabs

Run **ReforgerZ: Create prefabs** (or follow the manual checklist). First:

### 1.1 Before you start: project and dependencies

1. Subscribe to the dependencies in the game ([MOD_STACK.md](MOD_STACK.md)): at minimum
   **Bacon Zombies**; Metabolism and Special Zombies are recommended.
2. Arma Reforger Tools → **Add existing project** → pick this repo's `addon.gproj`.
3. Project Settings → Dependencies: Arma Reforger is already there; add Bacon Zombies and
   whatever else you use. Commit the changed `addon.gproj`.
4. Open the Script Editor and compile (**F7**). The game scripts compile cleanly on 1.9; [the VERIFY list](#6-the-verify-list)
   is what to watch in play.
5. Make a world as a **sub-scene of Arland** and open it in the World Editor
   ([world steps](#the-world)).

Look up classes in the Script Editor or the
[Script API docs](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/).

### 1.2 Run "ReforgerZ: Create prefabs"

The plugin in `scripts/WorkbenchGame/ReforgerZ/RZ_CreatePrefabsPlugin.c` reads the manifest and
does as much of sections 2 to 4 as it can. **It has never been run**: its editor calls were checked
against the 1.9 Workbench script declarations, and what only a run can show is marked `VERIFY`
([the list](#the-workbench-plugin-9)). Expect a round of fixes the first time, and use the [manual checklist](#13-by-hand-the-manual-checklist) for
whatever it can't do.

1. Compile the scripts (the plugin is in the Workbench script module, so `RZ_` classes must exist).
2. Open your Arland sub-scene in the World Editor.
3. **Plugins → ReforgerZ: Create prefabs.** The options dialog opens with **Dry run** on. Run it
   once like that: it changes nothing and lists what it would do in the log (search for
   `[ReforgerZ]`).
4. Turn **Dry run** off and run it again. Save the world afterwards.
5. Read the summary: *done*, *skipped* (already there), *failed* (with a reason) and *manual steps
   left*. Fix what failed, do the manual steps, and run it again: it never touches what exists.

**What it does, in order:**

| Step | What | Idempotent because |
|---|---|---|
| Prefabs | Creates each prefab at its path from its base, adds the components and actions with their values, and an `RplComponent` and `ActionsManagerComponent` (with a context) where needed. `@Antidote`-style values become that prefab's ResourceName | a prefab whose file exists is skipped |
| Loadouts | Creates the 13 loadout characters, each with `RZ_TreatOtherAction` | as above |
| Game mode | Creates `RZ_GameMode.et`, creates its settings objects and fills every slot it has a prefab for | only empty slots are filled; lists only when empty |
| World | Places the game mode in the open world; checks for a FactionManager and LoadoutManager | skipped if the world already has one |
| Factions | Adds `SURV_H1`..`SURV_H15` and `SURV_OUTLAW` (with `SURV`'s colour and name, not playable) and the missing friendly keys; reports hostile pairs that are friendly | existing keys and friends are left alone |
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
search for `baseFind` (a vanilla file name), else it leaves the prefab as a manual step. Only a
few entries have a `baseFind` (`GameMode_Plain.et`, `SpawnPoint_Base.et`, `AIWaypoint_Move.et`),
because vanilla names change between versions. For the rest, put the base you want in
`basePrefab` and run it again; that's the quickest way to make many prefabs at once.

**The fallback.** **Plugins → ReforgerZ: Write prefab checklist** writes the whole manifest as a
checklist to `PrefabChecklist.md` in the project folder (git ignores it), with what already
exists ticked. If even that fails, the tables in this doc are the same checklist.

**Known limitations:**
- Never run: names of editor calls may be wrong; each is in one helper in `RZ_WB`, so a fix is one edit.
- It can't choose meshes, fill inventories (loadout gear, the reward crate), set collision layers
  (the interaction spot, the doorway blocker), make AI groups' members or pick vanilla bases.
- Faction variants copy only the colour and name (with their own key): check the flag and identity
  by hand. If the factions live in a referenced `.conf` rather than on the FactionManager entity, it
  can't edit them. If it can't read a friendly list, it may replace it with only the new key, so check
  the four friendly lists after the first real run.
- It doesn't overwrite anything: to redo a prefab, delete it first.
- Shop stock beyond the antidote, stabiliser and cut dose, requisition items and loot tables are by hand.

### 1.3 By hand: the manual checklist

Work down the tables: [the minimum playable subset](#2-the-minimum-playable-subset) first, then
[factions and loadouts](#3-factions-and-loadouts), then the rest of [the manifest](#4-the-prefab-manifest)
system by system. For each prefab:

1. Find the suggested base in the Resource Browser (vanilla paths and names differ between game
   versions, so search by description) and **Create inherited prefab** at the path in the table
   (all under `Prefabs/ReforgerZ/`).
2. Add the components and actions in the **Add** column, with the attribute values shown.
   **The generic set** is `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`, `RZ_ReadClueAction`,
   4 × `RZ_PuzzleUseAction` (Value 0–3) and 4 × `RZ_PuzzleChoiceAction` (Option 0–3).
   **The clue set** is `RZ_ClueComponent` and `RZ_ReadClueAction`.
3. Every prefab with an `RZ_` component needs an `RplComponent`; every prefab with an action
   needs an `ActionsManagerComponent` with at least one action context, and each action's
   parent context set to it.
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

To use a prefab you already have (or a confirmed vanilla base), put its ResourceName
(`{GUID}path.et`) in the entry's `basePrefab`. Lists (zombies, cars, the loot pool) take several,
comma-separated.

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
| `RZ_Civilian.et` | a civilian character without AI, or a seated figure prop | the generic set | Puzzles > Civilian | puzzles, unarmed missions, wanderers, leaders and occupants to talk to |
| `RZ_Note.et` | a paper or notebook prop | the clue set | Puzzles > Note | every puzzle mission, scenario clues |
| `RZ_MissionItem.et` | a small carryable item (a box or data drive) | `RZ_MissionItemComponent` | Missions > Mission item | every delivery and choice mission; tagged when it spawns |
| `RZ_Drop_Haven.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction SURVIVORS) | Missions > Haven drop point | the drops those missions name |
| `RZ_Drop_Bastion.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction MILITARY) | Missions > Bastion drop point | the drops those missions name |
| `RZ_Drop_RedHand.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction BANDITS) | Missions > Red Hand drop point | the drops those missions name; also Car Theft's chop shop |
| `RZ_Drop_Halcyon.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction INSTITUTE) | Missions > Halcyon drop point | the drops those missions name |
| (existing prefab) | Bacon Zombies civilian and military groups | — | Zombies > Common prefabs (list) | the infected |
| (existing prefab) | Special Zombies, or Bacon heavy, exploding and ranged | — | Zombies > Special prefabs (list) | samples, the cure |
| (existing prefab) | a large Bacon group | — | Zombies > Horde prefabs (list) | hordes |
| (existing prefab) | vanilla AIWaypoint_Move.et | — | Zombies > Move waypoint | hordes toward noise, unarmed missions, hunter squads, supply trucks, favourite spots |
| `RZ_QM_Haven.et` | a civilian character without AI (damage off) | `RZ_QuartermasterComponent` (Faction SURVIVORS); `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), 8 × `RZ_SafehouseAction` (Kind STATUS, DELIVER_FOOD, RADIO, HEAL, DECLARE_BITE, MEDS, BOARD_READ, BOARD_WRITE), `RZ_FactionServiceAction` (Service AMENDS) | Factions > Haven quartermaster | the Haven shop, leader and safehouse services |
| `RZ_QM_Bastion.et` | a soldier character without AI (damage off) | `RZ_QuartermasterComponent` (Faction MILITARY); `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_FactionServiceAction` (Service AMENDS), `RZ_FactionShopAction` (Faction MILITARY, Required tier CHAMPION, Favor cost 30, Item prefab `RZ_SignalSmoke.et`, Display name Signal smoke) | Factions > Bastion quartermaster | the Bastion shop and leader |
| `RZ_QM_RedHand.et` | a civilian character without AI in rough clothes (damage off) | `RZ_QuartermasterComponent` (Faction BANDITS); `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_FactionShopAction` (Faction BANDITS, Required tier KNOWN, Favor cost 12, Item prefab `RZ_CutDose.et`, Display name Cut dose), 3 × `RZ_FactionServiceAction` (Service FENCE, BOUNTY_BOARD, AMENDS) | Factions > Red Hand quartermaster | the Red Hand black market and leader |
| `RZ_QM_Halcyon.et` | a character in a lab coat without AI (damage off) | `RZ_QuartermasterComponent` (Faction INSTITUTE); `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_TrialDoseAction`, `RZ_FactionShopAction` (Faction INSTITUTE, Required tier TRUSTED, Favor cost 60, Item prefab `RZ_Antidote.et`, Display name Antidote); `RZ_FactionShopAction` (Faction INSTITUTE, Required tier KNOWN, Favor cost 40, Item prefab `RZ_Stabiliser.et`, Display name Stabiliser), 3 × `RZ_FactionServiceAction` (Service MEDIC, BOOSTER, AMENDS) | Factions > Halcyon quartermaster | the Halcyon shop, leader, antidotes and the trial dose |
<!-- /manifest:minimum -->

No towns, spawn points or lab need placing: they're generated at start. With only this,
these stay off: hunter squads, Perimeter checkpoints and drones (the line still works with
messages), supply trucks (places count as supplied), territory guards, adaptation traits, the
power station's own control prefab (it falls back), world-interaction loot (searches find
nothing until the loot pool is filled), fixable cars, crafting and traps, and the treatments other than the antidote and the trial dose
(stabiliser, low-grade meds).

**On another map:** make another sub-scene with the same contents, then check the log lines in
[section 7](#7-log-lines-to-check). If the map has no name labels and few buildings, place a few
`RZ_SettlementMarkerEntity`s. Check the Perimeter rectangle, checkpoints, safehouses, territory,
power and radio station, and whether "Search" spots appear in houses.

### The world

<!-- manifest:world -->
| Step | Plugin | Minimum |
|---|---|---|
| Make a world as a sub-scene of Arland (a new world with Arland as its parent; the menu name varies by version) | manual | yes |
| Place a FactionManager (a vanilla FactionManager prefab) and fill in the factions below | checks it | yes |
| Place a LoadoutManager (a vanilla LoadoutManager prefab) with the 13 loadouts on SURV | checks it | yes |
| Place RZ_GameMode.et in the world (one per world) | does it | yes |
| Save the world, and point a scenario (mission header) at it to play it | manual | yes |
| Optional: place marker entities (RZ_SettlementMarkerEntity and the others) to override automatic picks | manual | — |
<!-- /manifest:world -->

## 3. Factions and loadouts

In the world's `FactionManager`:

<!-- manifest:factions -->
| Key | In game | Used for |
|---|---|---|
| `SURV` | The Haven | the player faction; its loadout list holds the 13 backgrounds; Haven AI |
| `MIL` | Task Force Bastion | Bastion AI: guards, crews, hunter squads |
| `BAND` | The Red Hand | Red Hand AI: mission groups, guards, crews, hunter squads |
| `INST` | Halcyon | Halcyon AI: guards, crews |
| (the zombie mod's) | The infected | the zombie mod's own faction (Bacon Zombies): hostile to all four |
<!-- /manifest:factions -->

**Relations.** AI shoots individual players through the hidden variants below, so the base
relations are the "nobody wants you dead" case. Set them in each faction's friendly list, both ways:

<!-- manifest:relations -->
| | SURV | MIL | BAND | INST |
|---|---|---|---|---|
| SURV | friendly | friendly | friendly | friendly |
| MIL | friendly |  | hostile | friendly |
| BAND | friendly | hostile |  | hostile |
| INST | friendly | friendly | hostile |  |
<!-- /manifest:relations -->

`SURV`–`BAND` friendly is what makes the Red Hand neutral by default.

**The 15 hidden variant factions.** These let AI shoot one player and not another
([REPUTATION.md](REPUTATION.md), "What makes AI shoot you"):

1. Duplicate the `SURV` entry. Change only its **Faction key** to `SURV_H1`.
2. Keep its name, colour, flag and identity the same, so it looks like the Haven. Turn
   **Is Playable** off, and empty its loadout list.
3. Repeat for `SURV_H2` up to `SURV_H15`. The number is a bitmask of who shoots that player:
   1 the Haven, 2 Bastion, 4 the Red Hand, 8 Halcyon, added up.
4. Their friendly lists don't matter: the modded faction check decides from the key. Copying
   `SURV`'s is fine.
5. **Optional, for the Outlaw fallback:** one more copy keyed `SURV_OUTLAW`, with real relations:
   friendly with nothing (or only itself), and removed from the friendly lists of `SURV`, `MIL`,
   `BAND` and `INST`.

If the player faction key isn't `SURV`, the variants are `<key>_H1`..`<key>_H15` and
`<key>_OUTLAW`. Missing variants only turn the feature off for the players who'd need them.

**Loadouts.** In the world's `LoadoutManager`, on `SURV`, one loadout per background, named
exactly as below. Each is a civilian character prefab with that background's starting gear and
`RZ_TreatOtherAction` (so other players can treat you). Players who pick a locked one keep its
gear but play as Unemployed. The log warns `No background matches loadout '...'` for a typo.
The gear is a suggestion; the perks and flaws are in [DESIGN.md](DESIGN.md#backgrounds-and-personal-goals).

<!-- manifest:loadouts -->
| Loadout name | Prefab | Unlock | Suggested gear |
|---|---|---|---|
| `Nurse` | `RZ_Loadout_Nurse.et` | — | medical bag, bandages, morphine, saline; no weapon |
| `Mechanic` | `RZ_Loadout_Mechanic.et` | — | toolbox, jerrycan, wrench; work clothes |
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
The code has **165 prefab fields**: 40 on `RZ_GameModeComponent`, 119 in its settings objects, and 6 in list entries and actions (shop item, requisition item, recipe output, the loot table lists). The manifest covers them in **170 entries**: 146 prefabs the plugin creates, 15 slots filled with prefabs you already have (zombie mods, vanilla cars, loot) and 9 list entries set by hand; plus the game mode prefab, 13 loadout characters, 7 additions to existing prefabs and 29 settings objects. 22 of them, counting the game mode, make up the minimum playable subset.
<!-- /manifest:counts -->

Each manifest entry fills one or more fields. **Need:** *minimum* is the
[minimum playable subset](#2-the-minimum-playable-subset); *optional* turns its feature off (or
falls back) when empty. "(existing prefab)" means you
fill the slot with a prefab you already have. All new prefabs go under `Prefabs/ReforgerZ/`.

<!-- manifest:prefabs -->
#### Core and cure

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_SpawnPoint.et` | a vanilla spawn point (SCR_SpawnPoint), e.g. SpawnPoint_Base.et or SpawnPoint_US.et; confirm the name in Workbench | — | World > Spawn point prefab | **minimum** | spawn points in towns, safehouse spawns |
| `RZ_Sample.et` | a small item (a vial or sample tube) | `RZ_SampleItemComponent` (Value 1) | Cure > Sample | **minimum** | samples from specials |
| `RZ_Antidote.et` | vanilla morphine or saline item (keeps its consumable component) | `RZ_AntidoteItemComponent`; `RZ_InjectAntidoteAction` | Cure > Antidote | **minimum** | antidotes; optionally set the consumable effect to RZ_ConsumableAntidote by hand |
| `RZ_Lab.et` | a field hospital tent or lab container prop | `RZ_LabComponent`; `RZ_DepositSamplesAction`, `RZ_LabStatusAction` | Cure > Lab prefab | **minimum** | the cure (spawned in a town if none is placed); a second copy is Mobile Lab's field annex for 3 hours |
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
| `RZ_Civilian.et` | a civilian character without AI, or a seated figure prop | the generic set | Puzzles > Civilian | **minimum** | puzzles, unarmed missions, wanderers, leaders and occupants to talk to |
| `RZ_Note.et` | a paper or notebook prop | the clue set | Puzzles > Note | **minimum** | every puzzle mission, scenario clues |
| `RZ_Keypad.et` | a keypad or wall panel prop | `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`; 12 × `RZ_KeypadAction` (Key 0-11), `RZ_ReadClueAction` | Puzzles > Keypad | optional | The Last Experiment (keypad mode), Locked Bunker; keys 0-9, 10 Clear, 11 Enter |
| `RZ_Terminal.et` | a computer prop | `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`; 12 × `RZ_KeypadAction` (Key 0-11), `RZ_ReadClueAction` | Puzzles > Terminal | optional | The Last Experiment (falls back to the keypad); keys 0-9, 10 Clear, 11 Enter |
| `RZ_PuzzleBody.et` | a body prop | the clue set | Puzzles > Body | optional | Paces, Patient Zero's Ledger, Poisoned Well, Last Experiment, The Witness, The Deserter's Map (falls back to the note) |
| `RZ_Well.et` | a well prop | the generic set | Puzzles > Well | optional | look only (falls back to the puzzle object) |
| `RZ_Bell.et` | a bell prop | the generic set | Puzzles > Bell | optional | look only (falls back to the puzzle object) |
| `RZ_FuseBox.et` | an electrical box prop | the generic set | Puzzles > Fuse box | optional | look only (falls back to the puzzle object) |
| `RZ_RadioMast.et` | an antenna prop | the generic set | Puzzles > Radio mast | optional | look only (falls back to the puzzle object) |
| `RZ_Safe.et` | a safe prop | the generic set | Puzzles > Safe | optional | look only (falls back to the puzzle object) |
| `RZ_ScaleCrate.et` | a crate with an inventory storage | the generic set | Puzzles > Scale crate | optional | The Weighing Vault |
| `RZ_Landmark.et` | a wayside cross or painted mast prop | — | Puzzles > Landmark | optional | towns with no church, tower or well; set Landmark name to match |
| `RZ_MorseLamp.et` | a lamp prop with a light | `RZ_BlinkLightComponent` | Puzzles > Morse light | optional | Lights in the Window (without the component it's spawned per flash) |

#### Missions

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_MissionItem.et` | a small carryable item (a box or data drive) | `RZ_MissionItemComponent` | Missions > Mission item | **minimum** | every delivery and choice mission; tagged when it spawns |
| `RZ_Drop_Haven.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction SURVIVORS) | Missions > Haven drop point | **minimum** | the drops those missions name |
| `RZ_Drop_Bastion.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction MILITARY) | Missions > Bastion drop point | **minimum** | the drops those missions name |
| `RZ_Drop_RedHand.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction BANDITS) | Missions > Red Hand drop point | **minimum** | the drops those missions name; also Car Theft's chop shop |
| `RZ_Drop_Halcyon.et` | a crate, table or mailbox prop | `RZ_ChoiceDeliveryAction` (Faction INSTITUTE) | Missions > Halcyon drop point | **minimum** | the drops those missions name |
| `RZ_SurvivorGroup.et` | a civilian AI group on SURV | — | Missions > Survivor group | optional | Survivors Trapped, Training Day, Harvest Guard, The Bitten Friend, Trader Caravan, Protection Racket; the fallback escort and bounty target |
| `RZ_Nest.et` | a Zombie Hive prefab or a destructible prop with a damage manager | — | Missions > Nest | optional | Burn the Nest, Specimen from the Nest |
| `RZ_BanditDrop.et` | a crate, barrel or car wreck prop | `RZ_SellSamplesToBanditsAction` | Missions > Dead drop | optional | Dead Drop |
| `RZ_HeliWreck.et` | a helicopter wreck prop | the clue set | Missions > Helicopter wreck | optional | Crash Site, Crashed Helicopter, Two Bearings; the look of Rescue the Pilot's crash |
| `RZ_CorpsePile.et` | a body bag pile prop | — | Missions > Corpse pile | optional | look only: Burn the Bodies' piles and Memorial's bodies (both fall back to the puzzle object) |
| `RZ_RedHandGroup.et` | an AI group on BAND | — | Missions > Red Hand group | optional | Raid the Red Hand, Trader Caravan, raiders in Lab Defence and Convoy Escort, The Witness; default Red Hand hunter squad |
| `RZ_FuelDepot.et` | fuel tanks with a damage manager | — | Missions > Fuel depot | optional | Arson |
| `RZ_Relay.et` | a radio mast with a damage manager | — | Missions > Relay | optional | Sabotage the Relay |
| `RZ_Trader.et` | a character or vehicle (e.g. a Shop System trader) | — | Missions > Trader | optional | Wandering Trader |
| `RZ_Explosion.et` | a mortar or shell impact effect | — | Missions > Explosion | optional | Bastion Shelling; charges in Blow the Bridge, shells in Fire Mission, Gas Station Fire's blast |

#### Contracts

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_EscortCivilians.et` | an AI group of 2-4 unarmed civilians on SURV | — | Missions > Contracts > Escort civilians | optional | Refugee Column, Bust Out, Desertion Route (falls back to the survivor group, then data) |
| `RZ_EscortScientist.et` | a one-man AI group on INST | — | Missions > Contracts > Escort scientist | optional | Escort the Scientist, Secure the Shipment |
| `RZ_EscortSoldier.et` | a one-man AI group on MIL | — | Missions > Contracts > Escort soldier | optional | Rescue the Pilot, Hunt the Deserter |
| `RZ_Cage.et` | a cage or box-trap prop | the generic set | Missions > Contracts > Cage trap | optional | Live Capture, Private Collection (falls back to the puzzle object) |
| `RZ_Bus.et` | a drivable bus or truck (keeps its damage manager) | — | Missions > Contracts > Bus | optional | Recover the Bus (falls back to the Haven truck, then car prefabs) |
| `RZ_BountyTarget.et` | an armed AI group or character | — | Missions > Contracts > Bounty target | optional | Collect the Bounty (falls back to the survivor group) |
| `RZ_BastionSquad.et` | an AI group on MIL | — | Missions > Contracts > Bastion squad | optional | the stockade guards in Bust Out (falls back to the Bastion hunters, then crew) |
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
| (existing prefab) | Bacon Zombies civilian and military groups | — | Zombies > Common prefabs (list) | **minimum** | the infected |
| (existing prefab) | Special Zombies, or Bacon heavy, exploding and ranged | — | Zombies > Special prefabs (list) | **minimum** | samples, the cure |
| (existing prefab) | a large Bacon group | — | Zombies > Horde prefabs (list) | **minimum** | hordes |
| (existing prefab) | vanilla AIWaypoint_Move.et | — | Zombies > Move waypoint | **minimum** | hordes toward noise, unarmed missions, hunter squads, supply trucks, favourite spots |
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
| `RZ_QM_Haven.et` | a civilian character without AI (damage off) | `RZ_QuartermasterComponent` (Faction SURVIVORS); `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), 8 × `RZ_SafehouseAction` (Kind STATUS, DELIVER_FOOD, RADIO, HEAL, DECLARE_BITE, MEDS, BOARD_READ, BOARD_WRITE), `RZ_FactionServiceAction` (Service AMENDS) | Factions > Haven quartermaster | **minimum** | the Haven shop, leader and safehouse services. Add RZ_FactionShopAction entries for vanilla stock by hand (suggested stock below) |
| `RZ_QM_Bastion.et` | a soldier character without AI (damage off) | `RZ_QuartermasterComponent` (Faction MILITARY); `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_FactionServiceAction` (Service AMENDS), `RZ_FactionShopAction` (Faction MILITARY, Required tier CHAMPION, Favor cost 30, Item prefab `RZ_SignalSmoke.et`, Display name Signal smoke) | Factions > Bastion quartermaster | **minimum** | the Bastion shop and leader. Add one RZ_RequisitionAction per ammo or rations item, and shop actions for vanilla stock, by hand |
| `RZ_QM_RedHand.et` | a civilian character without AI in rough clothes (damage off) | `RZ_QuartermasterComponent` (Faction BANDITS); `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_FactionShopAction` (Faction BANDITS, Required tier KNOWN, Favor cost 12, Item prefab `RZ_CutDose.et`, Display name Cut dose), 3 × `RZ_FactionServiceAction` (Service FENCE, BOUNTY_BOARD, AMENDS) | Factions > Red Hand quartermaster | **minimum** | the Red Hand black market and leader |
| `RZ_QM_Halcyon.et` | a character in a lab coat without AI (damage off) | `RZ_QuartermasterComponent` (Faction INSTITUTE); `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_TrialDoseAction`, `RZ_FactionShopAction` (Faction INSTITUTE, Required tier TRUSTED, Favor cost 60, Item prefab `RZ_Antidote.et`, Display name Antidote); `RZ_FactionShopAction` (Faction INSTITUTE, Required tier KNOWN, Favor cost 40, Item prefab `RZ_Stabiliser.et`, Display name Stabiliser), 3 × `RZ_FactionServiceAction` (Service MEDIC, BOOSTER, AMENDS) | Factions > Halcyon quartermaster | **minimum** | the Halcyon shop, leader, antidotes and the trial dose |

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
| `RZ_CheckpointGuard.et` | a soldier without AI, or a figure prop | — | Perimeter > Guard prefab | optional | 0-4 per checkpoint |
| `RZ_Smuggler.et` | a civilian without AI (the Red Hand runner) | — | Perimeter > Smuggler prefab | optional | The Blind Spot (falls back to the civilian) |
| `RZ_Package.et` | a small box item | `RZ_MissionItemComponent` | Perimeter > Package prefab | optional | The Blind Spot, The Crossing (falls back to the mission item) |
| `RZ_LineBody.et` | a body prop | the clue set | Perimeter > Line body prefab | optional | Shot at the Line (falls back to the puzzle body) |

#### Haven safehouses and wanderers

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_SafehouseMedic.et` | a civilian without AI | 2 × `RZ_SafehouseAction` (Kind HEAL, DECLARE_BITE) | Haven > Medic prefab | optional | Settlement and up (the quartermaster's actions work without it) |
| `RZ_SafehouseOccupant.et` | a civilian without AI, or a seated prop | the generic set | Haven > Occupant prefabs (list) | optional | 3-6 people shown (empty = the civilian); add more variants to the list by hand |
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
| `RZ_CouncilHead.et` | a civilian without AI (damage off) | the generic set | Leaders > Council head | optional | optional body per safehouse |
| `RZ_Outpost_Bastion.et` | a soldier without AI | the generic set | Leaders > Bastion outpost commander | optional | optional outpost leader (falls back to the faction leader, then the civilian); Kill the Officer needs the Bastion one to be able to die |
| `RZ_Outpost_RedHand.et` | a character without AI | the generic set | Leaders > Red Hand crew boss | optional | optional outpost leader (falls back to the faction leader, then the civilian); Kill the Officer needs the Bastion one to be able to die |
| `RZ_Outpost_Halcyon.et` | a character without AI | the generic set | Leaders > Halcyon field lead | optional | optional outpost leader (falls back to the faction leader, then the civilian); Kill the Officer needs the Bastion one to be able to die |

#### Territory and supply trucks

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_Guard_Haven.et` | an armed AI character or small group on SURV | — | Territory > Haven guard | optional | guards at holdings; watched for retaliation |
| `RZ_Guard_Bastion.et` | an armed AI character or small group on MIL | — | Territory > Bastion guard | optional | guards at holdings; watched for retaliation |
| `RZ_Guard_RedHand.et` | an armed AI character or small group on BAND | — | Territory > Red Hand guard | optional | guards at holdings; watched for retaliation |
| `RZ_Guard_Halcyon.et` | an armed AI character or small group on INST | — | Territory > Halcyon guard | optional | guards at holdings; watched for retaliation |
| `RZ_Occupant_Haven.et` | a character without AI in the faction's clothes | the generic set | Territory > Haven occupant | optional | the named people (empty = the civilian) |
| `RZ_Occupant_Bastion.et` | a character without AI in the faction's clothes | the generic set | Territory > Bastion occupant | optional | the named people (empty = the civilian) |
| `RZ_Occupant_RedHand.et` | a character without AI in the faction's clothes | the generic set | Territory > Red Hand occupant | optional | the named people (empty = the civilian) |
| `RZ_Occupant_Halcyon.et` | a character without AI in the faction's clothes | the generic set | Territory > Halcyon occupant | optional | the named people (empty = the civilian) |
| `RZ_Truck_Haven.et` | a drivable vehicle (keeps its damage manager) | — | Supply > Haven truck | optional | the supply truck |
| `RZ_Truck_Bastion.et` | a drivable vehicle (keeps its damage manager) | — | Supply > Bastion truck | optional | the supply truck |
| `RZ_Truck_RedHand.et` | a drivable vehicle (keeps its damage manager) | — | Supply > Red Hand truck | optional | the supply truck |
| `RZ_Truck_Halcyon.et` | a drivable vehicle (keeps its damage manager) | — | Supply > Halcyon truck | optional | the supply truck; Mobile Lab's lab truck |
| `RZ_TruckCrew_Haven.et` | an AI group of 1-2 on SURV; the first one drives | — | Supply > Haven crew | optional | the crew |
| `RZ_TruckCrew_Bastion.et` | an AI group of 1-2 on MIL; the first one drives | — | Supply > Bastion crew | optional | the crew |
| `RZ_TruckCrew_RedHand.et` | an AI group of 1-2 on BAND; the first one drives | — | Supply > Red Hand crew | optional | the crew |
| `RZ_TruckCrew_Halcyon.et` | an AI group of 1-2 on INST; the first one drives | — | Supply > Halcyon crew | optional | the crew |

#### New-player guidance

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| `RZ_WindupRadio.et` | a small radio item; keep "radio" in its name | `RZ_TuneRadioAction` | Guidance > Radio prefab | optional | given to every new life |
| `RZ_CarWreck.et` | a wrecked car prop | — | Guidance > Car wreck prefab | optional | the back-road scenario |
| `RZ_ScratchedDoor.et` | a door or wall panel prop | the clue set | Guidance > Door prefab | optional | the basement scenario |
| `RZ_Flyer.et` | a poster prop | the clue set | Guidance > Flyer prefab | optional | the church scenario, safehouse flyers |
| `RZ_TableRadio.et` | a radio prop | `RZ_ClueComponent`; `RZ_ReadClueAction` (Label Listen) | Guidance > Table radio prefab | optional | the farmhouse scenario |
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
| `RZ_Key.et` | a small item (a key or key ring) | `RZ_KeyItemComponent`; `RZ_ReadKeyTagAction` | Interactions > Search > Key prefab | optional | keys as real items (empty = keys are data for one life) |
| `RZ_DoorBlocker.et` | an empty GenericEntity; add the collision-only box by hand (recipe below) | — | Interactions > Search > Doorway blocker | optional | keeps AI out of locked doorways near players. Needs only RplComponent, a collision-only MeshObject and a static RigidBody: see the doorway blocker recipe |
| (existing prefab) | every item a search can find (vanilla and mod items) | — | Interactions > Search > Loot pool (list) | optional | searches find nothing until it's filled |
| `RZ_CarProject.et` | a non-drivable car prop | the generic set | Interactions > Cars > Project prefab | optional | a fixable car before it runs |
| (existing prefab) | vanilla drivable civilian cars | — | Interactions > Cars > Car prefabs (list) | optional | what a fixed car becomes (empty = no fixable cars) |
| `RZ_CarBattery.et` | an item; keep "battery" in its name | — | Interactions > Cars > Battery | optional | car parts |
| `RZ_Tyre.et` | an item; keep "tyre" in its name | — | Interactions > Cars > Tyre | optional | car parts |
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
| `RZ_Planks.et` | a planks item | `RZ_PlaceableComponent` (Kind BARRICADE); `RZ_PlaceAction` | (loot or shop) | optional | barricades |
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
| (existing prefab) | vanilla items, crates or vehicles | — | Each shop action's Item prefab | optional | quartermaster stock beyond what the plugin adds (see the suggested stock) |
| (existing prefab) | vanilla ammo or rations | — | Each requisition action's Item prefab | optional | Bastion's Requisition (Trusted pays half, once an hour) |

#### UI

| Prefab | Suggested base | Add | Slot | Need | Used for |
|---|---|---|---|---|---|
| (existing prefab) | an imageset built from UI/Icons/png/64 (UI/LAYOUTS.md) | — | UI > Icon imageset | optional | icons on the HUD, cards and menus (empty = the images set in the layouts) |
| (existing prefab) | the NoticeCard layout (UI/LAYOUTS.md) | — | UI > Notice layout | optional | styled notice cards (empty = the vanilla hint) |
| (existing prefab) | the MissionRow layout (UI/LAYOUTS.md) | — | UI > Mission row layout | optional | journal rows |
| (existing prefab) | the RadioRow layout (UI/LAYOUTS.md) | — | UI > Radio row layout | optional | radio log rows |
| (existing prefab) | the MapIcon layout (UI/LAYOUTS.md) | — | UI > Map icon layout | optional | the ReforgerZ icons on the map, per player (empty = vanilla text markers for events and the lab). Also add RZ_MapIconsUI to the map configuration's components (Setting up the UI) |
<!-- /manifest:prefabs -->

### Add to prefabs you already have

<!-- manifest:additions -->
| Prefab | Add | Why | Need |
|---|---|---|---|
| every player character prefab used by a loadout (the plugin's RZ_Loadout_*.et get it already) | `RZ_TreatOtherAction` | other players can treat you | required |
| any vanilla radio prefab you want crackable | `RZ_TuneRadioAction` | secure radio channels (the wind-up radio gets it already) | optional |
| quartermasters you placed by hand (not from the manifest) | `RZ_StandingStatusAction`, 4 × `RZ_LeaderAction` (Kind TALK, WORK, NEWS, STANDING), `RZ_FactionServiceAction` (Service AMENDS) | standing, the stronghold leader and amends (the plugin's quartermasters get these already; also add the faction's services: Fence and Bounty board on the Red Hand, Medic and Immunity booster on Halcyon) | optional |
| the Haven quartermaster | `RZ_FactionShopAction` (Faction SURVIVORS, Required tier KNOWN, Favor cost 8, Item prefab `RZ_CarBattery.et`, Display name Car battery) | car parts for sale (Known), if wanted; same for tyre and spark plugs | optional |
| the Bastion quartermaster | `RZ_RequisitionAction` (Favor cost 30, Display name rations) | Requisition: one action per ammo or rations item (set its Item prefab) | optional |
| any quartermaster | `RZ_FactionShopAction` | vanilla stock (suggested stock table); the Item prefab is a vanilla item, crate or vehicle | optional |
| the player controller prefab the game mode uses (vanilla DefaultPlayerController.et or your copy; confirm in Workbench) | `RZ_PlayerUIComponent` | the UI's data feed: infection timer, secure radio, noise, standing, journal, backgrounds (docs/UI.md) | optional |
<!-- /manifest:additions -->

### Notes by system

**The generic puzzle object** is reused by many systems for anything that's read, talked to or
used, so make it early. Missions label the actions ("Seal the well", "Accuse Mirek") and hide the
slots they don't use. **Without replicated labels** (the prefab needs an `RplComponent`), every slot shows its generic
label, and the actions still route correctly. The specific puzzle props only change the look;
missions turn themselves off when a prefab they need is missing.

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
puzzle object (Tunnel Rats' culverts, the tape in Fake Distress Call), **Puzzles > Civilian** (the
people in Distress Call, Rival Survivors and Collapse Shelter) and **Missions > Red Hand group**
(Fake Distress Call, the watchers at the Abandoned Convoy), **Puzzles > Keypad** and **Note** or
**Body** (Locked Bunker). Gas Station Fire and Burning Town need **Traps > Fire** on. The convicts,
the sniper and the Fake Distress Call's ambush fight through the hostility system's fights, so they
must be on `BAND`. The newer contracts also reuse **Cars > Car prefabs** (Car Theft), **Supply >
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
Trusted or Champion), **Favor cost**, **Item prefab** (an item, crate or vehicle, spawned next
to the quartermaster) and **Display name**. The action is always shown; the server checks tier
and Favor when it's used. `RZ_RequisitionAction` has the same cost, item and name; the cost is
the full price (a Trusted player pays half, once an hour). Set it up for ammo and rations only.
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
| Haven | canned food, water, bandages, crowbar, car parts | hunting rifle, ammo, backpack, radio, jerrycan | pickup, van, fortification kit, good medkit |
| Bastion | rations, morphine, flares | service rifle, body armour, helmet, NVGs | machine gun, grenades, military truck |
| Red Hand | knife, pistol, cut dose | suppressor, scoped rifle, explosives | rare military gear, a stolen antidote |
| Halcyon | antiseptic, painkillers, stabiliser | antidote, stim kit, gas mask | — |

**Faction consequences.** Gunfire heat needs nothing: characters already have an
`EventHandlerManagerComponent`, and suppressors are detected by attachment names containing
"suppress" or "silenc".

**Faction services and road posts** ([REPUTATION.md](REPUTATION.md#checkpoints-roadblocks-and-camps)).
The plugin's quartermasters get `RZ_FactionServiceAction`: Amends on all four, Fence and Bounty
board on the Red Hand, Medic and Immunity booster on Halcyon; a Champion shop action on Bastion
sells the **signal smoke** (`RZ_SignalSmoke.et`: a throwable smoke grenade with
`RZ_FireMissionSmokeComponent`; the shells are **Missions > Explosion**). Check **Services > Fence
keywords** against your loot. The armband's prefab name should keep a Red Hand uniform keyword
("armband_red") so it counts toward the disguise; the ribbon's must not carry a faction's. Road
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

**New-player guidance, the power grid and the radio station** work with no prefabs: tips use the
people and radios that exist, scenario clues fall back to the note and body, and the control
object and console fall back to the puzzle prefabs. **The wind-up radio's prefab name must
contain "radio"** (or it needs a radio component), or it isn't counted as a radio.

**World interactions.** Everything works without prefabs (spots fall back to the generic object,
which is visible), but **searches find nothing until the loot pool is filled**, and each feature
whose output is empty stays quiet. Keep the interaction spot's box small and off the character
collision layer, so it never blocks a door, a doorway or a player. Name parts with "battery",
"tyre" and "spark", and the home remedy not with "vodka" or "herb".

**Door locks need no prefab.** The vanilla door action is modded (`Interactions/RZ_DoorHooks.c`),
so vanilla doors refuse players by themselves. If they don't, see the door rows in
[the VERIFY list](#6-the-verify-list); turning **Search > Door locks** off puts every lock
back on the loot. AI need more (how it works is in [DESIGN.md](DESIGN.md#locks-and-keys)): the
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

**The loot pool** (Search > Loot pool) is one list of every item a search can find. Each building
type takes the items whose prefab names contain its keywords (the lists are in
`RZ_Search.TypeWords`). To control a type exactly, add a **Loot table** for it.

### Setting up the UI

Optional: without it every message is a vanilla hint, as before. How the UI works is in
[UI.md](UI.md); the widget trees and names are in [UI/LAYOUTS.md](../UI/LAYOUTS.md).

1. **The feed.** Add `RZ_PlayerUIComponent` to the player controller prefab the game mode uses
   (the addition in the table above). Vanilla controllers already have the `RplComponent` it needs.
2. **The icons.** Import `UI/Icons/png/128/` as textures and build the `RZ_Icons` imageset, image
   names = file names ([LAYOUTS.md](../UI/LAYOUTS.md#importing-the-icons)). Set **UI > Icon imageset**.
3. **The HUD.** Build `RZ_Hud.layout` and `RZ_NoticeCard.layout`. On the player controller's
   `SCR_HUDManagerComponent`, add an info display of class `RZ_HudDisplay` with the HUD layout.
   Set **UI > Notice layout**.
4. **The menus.** Build the six menu layouts and the two row layouts; set **UI > Mission row
   layout** and **Radio row layout**. Make the tabs, the close button, the journal row's root,
   the radio filter chip and the notice board's post button Buttons with the names in LAYOUTS.md,
   for the mouse. Override `chimeraMenus.conf` with the six presets (`RZ_StandingMenu`,
   `RZ_JournalMenu`, `RZ_RadioLogMenu`, `RZ_MapKeyMenu`, `RZ_NoticeBoardMenu`,
   `RZ_HallOfFameMenu`) and the input config with `RZ_OpenStanding`, `RZ_OpenJournal`,
   `RZ_OpenRadio`, `RZ_OpenHall` and `RZ_RadioFilter`. Once the notice board's layout works, turn
   **UI > Board menus** on (off, the board is a notice and notes are written with `/note`).
5. **The deploy panel.** Override the deploy menu's layout, add `RZ_BackgroundPanel.layout` on the
   right, and put `RZ_BackgroundPanelComponent` on its root. Locked loadouts are refused by
   `UI/RZ_LoadoutLock.c` with nothing to set up (**UI > Lock loadouts**).
6. **The map.** Build `RZ_MapIcon.layout` and set **UI > Map icon layout**. Override the full-screen
   map's configuration (search the Resource Browser for the map config the game mode uses) and add
   `RZ_MapIconsUI` to its components. With the layout set, the vanilla text markers for world
   events and the lab are off.

Each step works without the later ones: with only step 1, nothing shows; with steps 1–3, the HUD
and styled cards work and the keys do nothing. Step 6 stands alone (after step 1).

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

`RZ_GameModeComponent` holds 29 settings objects and two lists. Every value has a working
default, and every default is in the manifest's `settings` (synced from the scripts). This is
what each one covers; defaults are in brackets where they matter for setup.

<!-- manifest:settings -->
| Category | Object | Field | Covers |
|---|---|---|---|
| ReforgerZ | (top level) |  | Tick seconds (5), **Player faction key** (SURV), **Wipe on start**, save seconds (120), **Season speed** (1) |
| ReforgerZ | Season | `m_SeasonSettings` | **Winner rules** (on), **Rule sets** (by name: Winter, Fog, Military Collapse, No Vehicles, or your own in the list), **Wipe** (On cure; Off, After weeks, or the first of both), epilogue (48 h), weeks (4), warning (24 h), **Hall of fame** (on) ([DESIGN.md](DESIGN.md#seasons)) |
| World | (top level) |  | Cluster cell (300 m), buildings per settlement (8), spawn point prefab, active spawn points (6), minutes between moves (20), **Loose item minutes** (30; 0 = off) |
| World | Town memory | `m_TownMemorySettings` | Loot return (120 min), looted below (0.5), loot floor (0.2), burned share (0.5), journal minutes (10) |
| World | Power grid | `m_PowerSettings` | Control prefab, region (2.5 km), spin-up (90 s), noise (10 heat/min), run hours (4), fuel and patch restore |
| World | Radio station | `m_RadioStationSettings` | Console prefab, takeover (30 s), cooldown (10 min, halved with power) |
| Director | Stages (list) |  | Per-stage overrides; empty = the built-in four (8, 16 and 24 hours played; [DESIGN.md](DESIGN.md#season-pacing)) |
| Director | (top level) |  | **Broadcast minutes** (25; 0 = only on first join) |
| Director | Environment | `m_EnvironmentSettings` | Night heat spread (0.25), night specials (×1.25), rain and fog thresholds, storm thresholds, storm grounds the helicopter |
| Director | Story arcs | `m_StorySettings` | Minutes between (60), start chance (0.6), repeat hours (4), the dark-side arc's limits |
| Zombies | Zombie settings | `m_ZombieSettings` | Common, special, horde prefabs, Move waypoint, per-size spawns (4), activation (600 m), despawn (1.1 km), min distance (120 m), caps (60, 4 per tick), ambient per player (2), kill infection (1.5), horde heat (80), horde gap (600 s) |
| Zombies | Adaptation | `m_AdaptationSettings` | Specials per breakthrough (0.05, cap 0.15), trait prefabs, trait share (0.15), screamers, favourite spots |
| Zombies | (top level) |  | Presence heat (0.5/tick), vehicle heat (+4/tick) |
| Backgrounds | Backgrounds (list) |  | Overrides; empty = the built-in 13. **Bite minutes** (60, once the bite takes; hardcore 40, casual 90) |
| Cure | (top level) |  | Lab, sample, antidote prefabs; research per sample (1%), mission research (×0.75) |
| Missions | (top level) |  | Reward crate, mission item, drop points and the optional mission prefabs |
| Missions | Events | `m_EventSettings` | Fog Bank radius (700 m), heat (×0.6) and spawn distance (×0.5), mist prefab; Power Surge minutes (10); Gas Station Fire infection drop (8); Burning Town max buildings (8) and drop per building (3); Tunnel Rats infection cap (20); convict, sniper and rival group prefabs; truck wreck, camp and hatch prefabs; Remnant Camp Infamy (10); wedding guests, bride and jewellery prefabs, **jewellery keywords** |
| Missions | Contracts | `m_ContractSettings` | **Rough areas** (500 m circle; 0 = exact) and reveal distance (500 m); Max escorts (3), escort leash (150 m), body radius (300 m) and speed (1.3 m/s), escort prefabs; max captures (2), cage prefab, capture radius (4 m), restrain share (0.3), specimen wake minutes (12); bus prefab and **bus keywords**; Blood Drive cost (0.25) and floor (0.7); bounty target, Bastion squad, crashed drone and smoke prefabs |
| Puzzles | (top level) |  | The puzzle prefabs, **Scale counts items** (off), landmark name |
| Perimeter | Perimeter | `m_PerimeterSettings` | **Enabled**, warn (300 m) and kill (100 m) distance, grace (8 s), fit to land and sea margin (200 m), **Override** rectangle, drone height, **Checkpoints**, guards (2), max (6, at most 8), inset, spacing, warn (150 m) and kill (60 m) radius, mission prefabs |
| Haven | Safehouses | `m_SafehouseSettings` | **Enabled**, count (3), start population (4) and food (12), visible radius (300 m), upkeep (180 min), people per food (5), **Food keywords**, Favor per food (2), heal cost (15), house-rule radii and Infamy, caught chance (0.3 a minute in Final), declare multipliers by tier (1.15/1.25/1.4/1.5), meds cost (5), stragglers (4 h), leak chance (0.25), **notice boards** (on, 8 notes, 160 characters, one every 10 min, 3 days, **note filter**), body prefabs |
| Wanderers | Wanderers | `m_WandererSettings` | **Enabled**, min and max count (6, 10), prices, spawn radius (400 m), walk speed, replace minutes (1440), bite chance, Infamy, body prefabs, Trader goods |
| Leaders | Leaders | `m_LeaderSettings` | **Enabled**, quartermaster is leader (on), interim posts (1, territory off only), body radius (200 m), replace minutes (120), job minutes (30), danger scale, discovery, kill Infamy, body prefabs |
| Territory | Territory | `m_TerritorySettings` | **Enabled**, outposts per faction (0 = by map size), held share (0.2), tick (45–75 min, ÷ season speed), pressure minutes (25), visible radius (300 m), guard and occupant counts, outpost quartermaster (on), guard and occupant prefabs |
| Territory | Supply | `m_SupplySettings` | **Enabled**, truck and crew prefabs, departures (60–90 min, ÷ season speed), unstick radius (1 km), stuck seconds (90), stock (8), Haven food (6), drain (45 min), out-of-stock share (0.3), rewards |
| Territory | Faction life | `m_FactionLifeSettings` | **Enabled**, chatter (10 min), life events (35 min), truces and feuds (on), false rumour chance (0.35) |
| Guidance | Guidance | `m_GuidanceSettings` | **Enabled**, meaningful minutes (10), level thresholds (1/3/8), refresher days (30), gentle mult and minutes (0.5, 15), tip gap (90 s), loud heat (45), **Tips** table, radio minutes (12), scenarios, clue minutes (20), flyers, shout minutes (30), wanderer finds, the scenario prefabs |
| Interactions | General | `m_InteractionSettings` | **Enabled**, spot prefab, scan radius (30 m) and seconds (10), caps (10 per player, 70), bells and sirens, water items and keywords, treating others |
| Interactions | Search | `m_SearchSettings` | **Loot pool**, loot tables, chances (find 0.6, rare 0.1, electronics 0.1), locks and their shares, **Door locks** (on; off = the lock on the loot), **Key prefab**, force seconds (10) and heat (25), relock minutes (180, the reset), crowbar keywords, readable and body chances. Keeping AI out: **AI door hook** (on), **Doorway blocker** prefab, **Navmesh rebuild** (on; off = blockers only), blocker range (150 m), cap (20), cooldown (60 s), bang heat (4) and seconds (15) |
| Interactions | Cars | `m_CarSettings` | Car prefabs, project prefab, per settlement (0.34), max (6), parts and keywords, hotwire and siphon timings, Mechanic part cost (12) |
| Interactions | Crafting | `m_CraftSettings` | Workbench keywords, the eight recipes' outputs and inputs, extra recipes |
| Interactions | Traps | `m_TrapSettings` | Barricade, tripwire, spike trap limits and props, generators, fire and molotovs |
| Interactions | Secure radio | `m_RadioCrackSettings` | Crack seconds (60), listen minutes (20), line minutes (4), range (3 km), interrupts, house radio keywords |
| Survival | (top level) |  | Shot heat (3), suppressed mult (0.3), **Bite chance** (0.1; hardcore 0.15, casual 0.07), bite reach (2.5 m) |
| Survival | Infection | `m_InfectionSettings` | Bite modifiers (surrounded ×2, grabbed ×1.5, tired ×1.5, hurt ×1.5, max 0.6), **protective clothing keywords** and multipliers (jacket 0.8, gloves 0.8, helmet 0.85), **Golden window** (120 s; hardcore 60, casual 180), alcohol and antiseptic keywords, clean chances (0.7, 0.9) and seconds (6), Final stage minutes (10), the six treatment prefabs, stabiliser minutes (20), med cap (30 min), trial dose cure (0.8) and Fame (3), cut dose cure (0.5), antibiotics fail (0.3), herb keywords, brew seconds (20), **Finds** (where treatments turn up; empty = the built-in list) |
| Factions | (top level) |  | Quartermaster and hunter prefabs, hunter minutes (10; 0 = off), bounty minutes (10) |
| Factions | Hostility | `m_HostilitySettings` | **Mode** (Bitmask / Outlaw / Off), each faction's **Shoots from** (Hunted), exile armed only (on) and radius (100 m), fight radius (350 m), wanderers fight back (on) and grudge radius (80 m), **Retaliation** (on, 300 m, 180 s, 3 Infamy, cap 12/h), **Disguises** (on; **uniform keywords** per faction, 2 pieces, checked every 10 s, blown for 15 min), the three AI faction keys |
| Factions | Services | `m_ServiceSettings` | **Fire Mission** (on, 60 min cooldown, 6 infected, 50 m, 30 s, kills at most 25, not within 250 m of people), **Fence** (on, keywords, 3 Favor an item, 8 a sale, 30 an hour), **Bounty board** (on, 60 to post, 40 paid, 3 at once, 2 h), booster (80) and medic (20) prices, **Amends** (15 Favor, ×1.5 each time, 5 Infamy, 15 a life, never below 15, every 30 min), armband and ribbon prefabs |
| Factions | Checkpoints | `m_CheckpointSettings` | **Road posts** (on, 2 at once, every 15 min, 45 min each, within 2.5 km of a holding, bodies within 600 m), barrier and crew prefabs, warn (150 m) and barrier (35 m) radius, search (15 s, **contraband keywords**), run Infamy (3), toll (10), **Surrender** (on, 10 s, 8 Infamy, once an hour, 5 min hold fire), **Camps** (on, warn 150 m, gate 50 m), **Safe zones** (on, 80 m, 2 Infamy) |
| UI | UI | `m_UISettings` | **Styled notices** (on; off = the vanilla hint), notice seconds (9), cards at once (3), queue (8), radio log size (40), infection timer and noise meter (on), resync (60 s), **icon imageset**, the three row layouts and the **map icon layout** (refresh 20 s), **lock loadouts** (on), **board menus** (off; on = the notice board menu) ([UI/LAYOUTS.md](../UI/LAYOUTS.md)) |
| Map | (top level) |  | **Map markers** (on): vanilla text markers for world events and the lab, used only while the icon layer isn't set up (UI > Map icon layout) |
| Debug | (top level) |  | Status log every few minutes (on) |
<!-- /manifest:settings -->

**Keywords are guesses.** Food, water, fuel, parts, crowbars, medkits, furniture, wrecks,
buildings, uniforms, buses, protective clothing, alcohol, antiseptic and herbs are found by
lower-case words in prefab names. Check them against the mods you run.

**Server.** Keep time acceleration modest and fixed; don't use time-skip or sleep mods. Set
**Season speed** for the server's size: 1 for 5–10 regulars (about 10 days), 1.5–2 for 2–4,
0.5–0.7 for 11–20, 0.3–0.5 above that; ×1.4 for a one-week season ([DESIGN.md](DESIGN.md#season-pacing)). For a
season wipe by hand, set **Wipe on start** for one restart, or delete `$profile:ReforgerZ/state.json`.
The wipe cycle (**Season > Wipe**, on cure by default) does it for you: when the season is over it
writes `wipe.json`, and the next restart backs `state.json` up to `state_season<N>.json` and starts
fresh ([DESIGN.md](DESIGN.md#seasons)); restart the server some time after players are told. The
last winner is kept in `$profile:ReforgerZ/last_season.json` and the hall of fame in
`hall_of_fame.json`; no wipe touches either. For a season with rules, set **Season > Rule sets**
(e.g. `Winter, No Vehicles`) before the fresh season starts. Give admins `/noteclear` for the
notice boards (server admins are whoever the server config lists).

## 6. The VERIFY list

### Compile check without Workbench

`tools/compile_check.sh` compiles every script on the Arma Reforger **Linux dedicated server**.
On first run it installs SteamCMD and the server (anonymous login, about 12 GB). It copies the
repo into a test addon folder, starts the server just long enough to compile, and prints any
ReforgerZ errors and warnings. Exit code 0 means the Game module compiled. Run it before every
commit that touches scripts.

```bash
tools/compile_check.sh
```

What it can't do:

| Limit | Why |
|---|---|
| The Workbench plugin isn't compiled | The server never loads the WorkbenchGame module. Compile it in Workbench (**F7**) |
| No behaviour is tested | It only compiles. Nothing runs until there's a world with the game mode in it |
| Folder names must be lowercase `scripts/` | The Linux server is case-sensitive and only looks there. Windows doesn't care, so keep it lowercase for Linux-hosted servers |

### The markers

```bash
grep -rn "VERIFY" scripts/
```

There are **40** lines with the tag: 30 in the game scripts (one is the comment in `Core/RZ_Util.c`
that explains them; 2 are in the UI) and 10 in the Workbench plugin (one is its header comment).
Every name and signature question is settled: the game scripts compile on 1.9, and each call was
checked against the base game's script source (its doc comments and how vanilla uses it). Each
marker left is a **behaviour** only a play-test can show (for the plugin, a run in Workbench):
replication, what the AI does, what a prefab carries. The game scripts' markers are grouped by
risk; check them in this order. Their paths are under `scripts/Game/ReforgerZ/`. The
[UI's](#the-ui-2) and the [plugin's](#the-workbench-plugin-9) come last.

### High risk: the core loop (1)

No fallback. If this is wrong, nothing persists.

| Where | Behaviour to check | If it fails |
|---|---|---|
| `Persistence/RZ_SaveData.c:131` | `JsonSaveContext` and `JsonLoadContext` write and read plain class members and arrays of them | Nothing is saved: after a restart, check `$profile:ReforgerZ/state.json` has real values in it and the world came back |

### Medium risk: big features, with a fallback (19)

| Where | Behaviour to check | Fallback |
|---|---|---|
| `Factions/RZ_FactionRelations.c:18` | AI perception and target selection pick up a changed `DoCheckIfFactionFriendly` answer quickly (the engine calls it often) | **The biggest unknown.** Test with a Bastion-Hunted player near Bastion AI. If they never fire, set **Hostility > Mode** to *Outlaw* and give `SURV_OUTLAW` real enemies |
| `Core/RZ_Util.c:488` | `SetAffiliatedFaction` on the server replicates, and AI perception reads it | Same as above. Check that the deploy menu, chat, groups and markers stay on `SURV`, and that nothing resets the character (the tick puts it back within 5 s) |
| `Core/RZ_Util.c:606` | `MoveInVehicle` seats an AI at once (it goes through an RPC to the character's owner, the server for AI); `PILOT` is the driver's seat | No truck ever arrives. Try the vanilla "get in" waypoint, or a vehicle prefab with its crew already inside |
| `Factions/RZ_SupplyLines.c:843` | a seated AI group drives to a move waypoint | As above. Test one truck end to end first |
| `Core/RZ_Util.c:634` | `SetWorldTransform` moves a physics vehicle with its crew | Unsticking only runs with nobody within 1 km; check truck and crew arrive together |
| `Interactions/RZ_Interactions.c:653` | furniture inside buildings is found as static entities | Every building still gets one "Search the house" spot |
| `Core/RZ_Util.c:437` | `GetVObject().GetResourceName()` names the mesh of map objects that aren't prefab instances | Such objects aren't recognised as landmarks, building types or furniture; prefab instances still are, and puzzles use the fallback landmark |
| `World/RZ_Perimeter.c:226` | `BaseWorld.GetBoundBox()` covers the terrain (it may be larger; `FitToLand` trims it) | Turn on **Override** and type in the corners |
| `World/RZ_Perimeter.c:676`, `:691` | the drone attached to the player follows them on every client; `SetOrigin` on a spawned prop moves it for clients | Moved each tick instead (jumpier) |
| `Interactions/RZ_DoorHooks.c:7` | the vanilla door prefabs use `SCR_DoorUserAction` | Players open locked doors: hook the action in the door prefabs' `ActionsManagerComponent`, or turn **Search > Door locks** off (locks go back on the loot) |
| `Interactions/RZ_DoorHooks.c:28` | the door action's `PerformAction` also runs on the server | The server can't refuse, so a client that skips its check opens the door: try a locked door from outside on a dedicated server |
| `Interactions/RZ_DoorLocks.c:63` | `SetControlValue(0, ...)` on the server shuts the door for every client | Locked doors aren't pulled shut |
| `Interactions/RZ_DoorLocks.c:75`, `:102` | doors are child entities of the building prefab, found with `GetParent()` and `GetChildren()`/`GetSibling()` | No door belongs to a building: no door locks, the loot lock everywhere |
| `Interactions/RZ_AIDoorHook.c:15` | AI open doors on their path through `SCR_AIOpenDoor` (the vanilla door task) | Blockers and banging still keep AI out. If AI open locked doors, move the check (`RZ_DoorLocks.AiMayOpenNear`) into the task they really use, or delete the file |
| `Core/RZ_Util.c:714` | `HitZone.SetHealth` on the server replicates and doesn't knock the character out above zero | Blood Drive donors could be knocked out or see stale health; nothing else uses it |
| `Core/RZ_Util.c:800` | worn clothing is attached to the character as child entities | No disguise is ever detected: read the loadout storage's slots in `GetWornNames`, or turn **Disguises** off |
| `Cure/RZ_Infection.c:1106` | `AddStamina` on the server reaches the player's stamina (vanilla also calls it only on the authority) | The Weakness and Final stages, nausea and the drunk remedy don't tire the player; their noise and the "tired" bite modifier still apply |

### Low risk: contained or cosmetic (8)

| Where | Behaviour to check | Fallback |
|---|---|---|
| `Core/RZ_Util.c:3` | (the comment that explains VERIFY) | — |
| `Cure/RZ_AntidoteConsumable.c:8`, `Cure/RZ_MedConsumable.c:5` | the consumable's `ApplyEffect` also runs on the server, where the cure happens | Remove the consumable effect from the prefab: the world actions ("Inject antidote", "Take ...") still work |
| `Puzzles/RZ_BlinkingLight.c:178` | `SetFlags/ClearFlags(EntityFlags.VISIBLE)` hides a light and its mesh, on clients too | Remove `RZ_BlinkLightComponent` from the lamp: it's spawned per flash |
| `Core/RZ_Util.c:400` | a weapon's attachments (the suppressor) are child entities of the weapon | Suppressors aren't detected (full gunfire heat) |
| `Puzzles/Missions/RZ_PuzzleMechanism.c:425` | `GetTotalWeight()` (weight plus "additional weight") includes a bag's contents | Turn on **Scale counts items** |
| `Missions/RZ_MissionBases.c:261` | the target prop has an `SCR_DamageManagerComponent` | Destroy missions (Arson, Sabotage, Burn the Nest) never finish: check the prop's damage manager |
| `Interactions/RZ_KeyItem.c:81` | `GetItems()` also finds a key in a bag inside another bag | Such keys aren't found: carry them in the top-level storages, or leave **Key prefab** empty (data keys) |

### The UI (2)

None of these affect the game: the UI only reads. If the HUD or a menu misbehaves, turn **UI >
Styled notices** off and every message goes back to the vanilla hint. How the UI is put together
is in [UI.md](UI.md).

| Where | Behaviour to check | If it fails |
|---|---|---|
| `UI/RZ_PlayerUIComponent.c:149` | a string RPC argument of a few KB arrives whole (payloads are capped at 6000 characters) | Long journals or standing don't arrive: lower `MAX_PAYLOAD` in `RZ_UIFeed` or trim the briefs |
| `UI/RZ_LoadoutLock.c:9` | what the player sees of a locked loadout: the 1.9 deploy menu leaves unavailable loadouts out when it builds its list, and greys their buttons when it refreshes | The panel can't show a locked background's unlock hint if the menu hides it. The server still spawns a locked pick as the default background |

### The Workbench plugin (9)

Paths are under `scripts/WorkbenchGame/ReforgerZ/`. The plugin isn't compiled by the server check,
but its editor calls were checked against the 1.9 Workbench script declarations. What's left is
what only a run in Workbench shows. None of these affect the game: if the plugin fails, make the
prefabs by hand. Riskiest first.

| Where | Behaviour to check | If it fails |
|---|---|---|
| `RZ_CreatePrefabsPlugin.c:363` | `CreateEntityTemplate(src, absPath)` saves a prefab that inherits from the entity's prefab and writes its `.meta` | No prefab is saved: make prefabs by hand |
| `RZ_CreatePrefabsPlugin.c:934` | API edits on a prefab loaded from its file, not only on world entities | Slots aren't wired: wire a placed instance and apply it to the prefab, or fill the slots by hand |
| `RZ_CreatePrefabsPlugin.c:214` | The editor names: `ActionContexts`, `additionalActions`, `ParentContextList`, `UIInfo`, `Factions`, `FactionKey`, `m_bIsPlayable`, `m_aFriendlyFactionsIds`, `m_aPlayerLoadouts` and the rest | Actions without a context, factions or loadouts not found: compare with a vanilla `.et`/`.conf` and fix the constants |
| `RZ_CreatePrefabsPlugin.c:536` | JSON load of nested classes and arrays, ignoring unknown keys | "Couldn't read the manifest": use the checklist from this doc |
| `RZ_CreatePrefabsPlugin.c:492` | `BaseContainer.Get(key, string)` reads a string, ResourceName, number or colour as text | Filled slots look empty (it never overwrites, but may report wrongly); faction keys unreadable |
| `RZ_CreatePrefabsPlugin.c:502` | `BaseContainer.Get(key, array<string>)` reads a string or ResourceName array | Lists look empty; friendly lists are rewritten with only the new key: check them |
| `RZ_CreatePrefabsPlugin.c:1158` | factions inline on the FactionManager | Variants can't be added: do it by hand |
| `RZ_CreatePrefabsPlugin.c:1360` | loadouts inline on the LoadoutManager | Loadout entries can't be added: do it by hand |
| `RZ_CreatePrefabsPlugin.c:260` | `FileIO.MakeDirectory` on absolute paths and existing folders | Saving into a new folder fails: make the `Prefabs/ReforgerZ/...` folders by hand |

## 7. Log lines to check

Every line starts with `[ReforgerZ]`. This is what the server printed on Arland with nothing
but `RZ_GameModeComponent` on a game mode (all settings at their defaults), in this order:

```
[ReforgerZ] No common zombie prefabs configured. Set them on RZ_GameModeComponent > Zombies.
[ReforgerZ] No lab prefab set. Samples can't be delivered until one is configured or placed.
[ReforgerZ] Hostility: bitmask mode, only 0 of 15 variant factions (SURV_H1 to SURV_H15) in the FactionManager. ...
[ReforgerZ] Season 1 (fresh). Rules: none. Wipe: 48 h after the cure.
[ReforgerZ] Settlement scan: 2 named, 12 building clusters, took 83 ms.
[ReforgerZ] Perimeter: x 70 to 3913, z 488 to 3788. Warning zone 300 m, kill zone 100 m.
[ReforgerZ] No checkpoint barricade prefab set: road checkpoints are off.
[ReforgerZ] Power grid: the factory outside Beauregard at grid 31-27 (picked), 14 towns in its region, dark, locked up.
[ReforgerZ] Radio station: the radio mast near Beauregard at grid 31-27 (picked).
[ReforgerZ] Haven safehouses: 2 (picked).
[ReforgerZ] Wanderers: 7 on the roads (new roster, seed S).
[ReforgerZ] Faction leaders: 5 (new, seed S). Haven 2, Bastion 1, Red Hand 1, Halcyon 1
[ReforgerZ] Faction life: morale 60/60/60/60, 0 remembered moments.
[ReforgerZ] Territory: 8 holdings in 14 settlements (new, seed S). Haven 1+1, Bastion 1+1, Red Hand 1+1, Halcyon 1+1
[ReforgerZ] Supply trucks: none (no truck or crew prefabs, or no move waypoint).
[ReforgerZ] Search: 0 items in the loot pool, 0 tables set by hand.
[ReforgerZ] Locks: on the doors (the loot where no doors are found); keys are data for one life.
[ReforgerZ] Door blockers: off (no blocker prefab); AI door hook on.
[ReforgerZ] World interactions: on. 0 fixable cars on the map.
[ReforgerZ] Story arcs: on, running none.
[ReforgerZ] Started. 14 settlements, stage Early Outbreak, cure 0 percent, state 0.
```

On a restart the same lines say `(save)` or `(saved)` instead of `(picked)` or `(new ...)`, and the
settlement scan doesn't run. Arland's 14 settlements: its two cities, the building clusters named
after the nearest local place label (North Head, Angler Bluff, Sandbank, Heath Point) and the rest
by grid reference (`Outskirts 13-29`).

Warnings worth acting on:

| Line | Means |
|---|---|
| `GUI (E): Menu preset 'RZ_StandingMenu' not found!` (and the other five) | An engine line, before the `[ReforgerZ]` ones: the `chimeraMenus.conf` override isn't there yet ([the UI](#setting-up-the-ui)). Harmless until then |
| `No common zombie prefabs configured. ...` | Set Zombies > Common prefabs (logged as an error: no infected spawn) |
| `Using N hand-placed settlement markers.` | Detection is off because markers are placed |
| `No checkpoint barricade prefab set: road checkpoints are off.` | Set Perimeter > Barricade prefab |
| `No road network found: place RZ_CheckpointMarkerEntity entities ...` | The road network calls found no roads, or the map has none |
| `Hostility: bitmask mode, only N of 15 variant factions ...` | Some `SURV_H` factions are missing |
| `Hostility: outlaw mode, but the FactionManager has no SURV_OUTLAW.` | Add the Outlaw faction |
| `No background matches loadout '...'` | A loadout name doesn't match a background |
| `Checkpoints: no road points found near players; road posts stay down.` | No roads near where players were |
| `No lab prefab set. Samples can't be delivered ...` | Set Cure > Lab prefab |
| `Puzzle clue prefab has no RZ_ClueComponent ...` / `Puzzle object prefab has no RZ_PuzzleObjectComponent ...` | A puzzle prefab is missing its component |
| `Mission item prefab has no RZ_MissionItemComponent ...` | The mission item is missing its component |
| `Key prefab has no RZ_KeyItemComponent (or didn't spawn) ...` | Add the component to the key prefab; until then keys are data |
| `Supply trucks are off. ...` / `Territory is off. ...` | Expected only if you turned them off |
| `Saved state is from another map, starting a fresh outbreak.` | Expected after changing maps |
| `Season wipe carried out (...). A fresh outbreak starts.` | The wipe cycle ran on this start; the old state is in `state_season<N>.json` |
| `Season rule set '...' doesn't exist; ignored.` | A name in **Season > Rule sets** is misspelt |
| `Save file is corrupt or from an incompatible version, ignoring.` | The save didn't load (or the JSON save behaviour in the VERIFY list is wrong) |

While running: `Status | ...` lines every few minutes (stage, infection, zombies, missions, cure),
`Mission N '...' at ...`, `Outbreak stage -> ...`, `Supply: ... truck leaves ...` and `... lost (...)`,
`Story arc started: ...`, `Bastion checkpoint up near ...` / `Red Hand roadblock up near ...`,
`Fire Mission perk: player N, M infected killed.`, `Note at ... by ...: ...`, `Season over. Cure goes to ...`,
and `Season N is over (...). The wipe happens at the next restart.`

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
- [ ] A special drops a sample; delivering it at the lab moves research.
- [ ] A contract is offered by radio, completes, and pays Fame and Favor; a tier change is announced.
- [ ] A quartermaster sells an item at the right tier and refuses one above it.
- [ ] Stop and restart the server: stage, infection, research and your standing are back.
- [ ] Restart the server during the final stand with nobody on: it doesn't count as abandoned
  while the server is empty, and the first player to spawn is told the stand is running.
- [ ] Loose items (set **Loose item minutes** to 2 to test): a dropped sample and a bought item
  left on the ground vanish once nobody is within 150 m; the same items in your pack, in a car's
  storage, or lying next to a running mission stay. A bought vehicle is never removed.
- [ ] Messages that arrive together (the three on spawn) show in the order they were sent.
- [ ] With nobody online for a while, the stage minutes and town infection don't move; with
  **Season speed** at 4, a stage change, a territory tick and a truck come four times as often.

**Seasons and notes**
- [ ] With **Wipe** on *After weeks* (1 week), stop the server and set `seasonStartUnix` in
  `state.json` back 6 days and 20 hours: on start the warning broadcast comes within a minute; set
  it back a full week: `wipe.json` is written and everyone is told. Restart: the backup `state_season<N>.json` exists, the season number went up, and the hall of fame kept
  last season's names.
- [ ] Win a final stand, then restart with **Epilogue hours** at 0: the next season starts with
  the winner's rules (the log and the first-spawn "This season" notice say which).
- [ ] **Rule sets** `Winter, Fog` on a fresh season: both are named on first spawn; at night more
  specials; Fog Banks come more often. `No Vehicles`: no fixable cars, no trucks, no Recover the Bus.
- [ ] The hall of fame shows on first spawn and with `/hall`; a delivered sample and a finished
  mission show up in it.
- [ ] At a safehouse: "Read the notice board" (empty), `/note Water at the church, bring a bucket`,
  read again: your name and the note. A second note at once is refused; away from a safehouse
  it's refused. With a word in **Note filter**, it's blanked out. As an admin, `/noteclear` clears
  it; as a player it's refused. Restart: the notes are still there.
- [ ] Rough areas: a Bastion contract offered to a Stranger names a 500 m circle; walk within
  500 m and you're told the spot; a Radio Operator, or someone Trusted with Bastion, gets the exact
  spot in the offer. A puzzle contract always gives its own positions.

**UI** (once the layouts exist; [Setting up the UI](#setting-up-the-ui))
- [ ] A Haven channel message shows as an amber card with the speaker; three at once stack, the
  rest queue; with **Styled notices** off they're vanilla hints again.
- [ ] A bite shows the golden window counting down, then Fever with the track filling; a
  stabiliser shows PAUSED and the clock stops; the timer matches the stage messages within seconds.
- [ ] Cracking a secure channel shows the indicator until the codes change.
- [ ] K, J and U open Standing, Journal and Radio with real data; Q/E switch tabs; Esc closes.
- [ ] O opens the hall of fame; with **Board menus** on, reading a board opens the board menu, and
  typing in its box and pressing Post pins the note (the list refreshes).
- [ ] In the deploy menu, a locked background shows LOCKED and how to unlock it, and the menu won't
  spawn it; after one life as Nurse the panel lists the Nurse's real gear.
- [ ] The tabs, close button, journal rows and radio filter take clicks; the lists scroll.
- [ ] Open the map: the lab, the safehouses, the strongholds and a world event show with their
  icons and stay on their spots while you pan and zoom; an outpost appears after you've been near
  it (or once you're Known with its faction); no contract is ever on it.

**Factions**
- [ ] Hunted by Bastion near Bastion AI: they shoot you, not a friend next to you.
- [ ] Your deploy menu, chat and group stay on the Haven while you're on a variant.
- [ ] An exile with a gun near a safehouse is shot; without one, the door is just shut.
- [ ] Shooting a guard makes nearby guards of that faction fire back for about 3 minutes.
- [ ] Disguise: with Bastion set to shoot from Watched, a Watched player in two Bastion uniform
  pieces walks past Bastion AI and is told so; Infamy with Bastion while wearing it is doubled.
- [ ] Bounty: a player at Kill on Sight with two factions is killed by another player; only the
  killer is paid, once, by one faction. Die to the infected (or a fall) while at Kill on Sight:
  nobody is paid. Collect the Bounty on a player pays only whoever killed them.
- [ ] A Bastion outpost leader dies and is replaced (Replace minutes at 1): when that outpost is
  later abandoned or taken, the new leader leaves with it.
- [ ] A supply truck leaves, is driven by its crew, and arrives.
- [ ] A leader answers Talk, Work, News and Standing.
- [ ] Hunted by Bastion, stay put through three hunter rounds: never more than two squads after you.
- [ ] Kill a truck crew from 300 m, then drive the truck in: no Fame. (Known gap: a friend who
  didn't shoot can still drive it in for Fame; watch for that pattern.)
- [ ] Retaliation in a crossfire: a Red Hand group attacking a Bastion outpost while you shoot
  infected 300 m away. If you get Bastion's retaliation for their hits, note it.
- [ ] Wear a Bastion uniform and finish a Red Hand job: the rival Infamy with Bastion is doubled
  and the disguise is blown. Decide whether that's wanted.
- [ ] Start The Last Convoy (or wait for it), restart the server before the truck leaves: it
  still leaves within 10 minutes. Restart while it's on the road: the arc just ends.
- [ ] With **Post minutes** at 1, a road post goes up on a road ahead of you. At a Bastion
  checkpoint: Known, you're waved through; a stranger is searched at the barrier (a suppressor is
  taken); walking off mid-search costs 3 Infamy and they fire. Unarmed with Bastion Infamy, wait at
  the barrier: you surrender, lose 8 Infamy, and Hunted, they stop shooting for 5 minutes.
- [ ] At a Red Hand roadblock: Known pays 10 Favor and passes, a stranger is warned then fired on,
  Trusted is waved through.
- [ ] Watched by Bastion, walk at an outpost armed: warned at 150 m, fired on past 50 m. Fire a
  shot inside the lab's safe zone: 2 Halcyon Infamy and the guards turn.
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

**World**
- [ ] Walking near the map edge: drone and loudspeaker at 300 m, warning shot at 100 m.
- [ ] A safehouse shows occupants; you can hand in food, declare a bite at a Shelter (smaller
  slowdown) and a Town (bigger), and buy expired antibiotics for 5 Favor.
- [ ] As a Haven Champion, walk into a safehouse with a hunter squad on you: it turns away, and
  a bounty notice names only a nearby town. Die as a Haven Champion: the memorial headline goes
  out, and your next character is greeted and treated free at their first safehouse.
- [ ] A stove offers "Brew a home remedy" with alcohol and herbs in your pack.
- [ ] A wanderer can be talked to and remembers you.
- [ ] "Search" spots appear in a house; a search finds something from the loot pool.
- [ ] A locked place's outside door says "Locked" from outside and opens from inside (which
  unlocks it); its interior doors and shutting any door always work.
- [ ] At the door: Unlock with the right key (the wrong one says which tag you need), Lock it
  again, Force it with a crowbar (loud; it can't be locked after). Search inside follows.
- [ ] A key item can be dropped, picked up by someone else and opens the door for them;
  "Read the tag" shows its label on the ground.
- [ ] With **Relock minutes** at 5: the door is locked and pulled shut again once you step away.
- [ ] From a closet or small room behind a door near the outline, the door opens for you.
- [ ] Doorway blockers: with the navmesh debug view on, a locked door near you shows a gap a few
  seconds after you arrive. Lead infected and a hostile AI group at it: they route to another
  door or wait outside; they don't open it. Note the server frame time when blockers come and go.
- [ ] Infected pressed against a locked door make noise (heat), and you're told once.
- [ ] World events (raise a weight to start one): Fog Bank muffles heat and brings spawns closer
  only inside its radius, and both reset when it ends. Power Surge marks the town Powered for 10
  minutes (status log, the lights hook, electronics in searches) and dark again after, unless the
  grid powers it. Gas Station Fire goes up after 3 minutes and burns at most three buildings;
  Burning Town burns one building at a time and stops at its cap. Tunnel Rats' culverts each
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

**Contracts on the building blocks** (start one by raising its weight, or wait for the radio)
- [ ] Escort: Refugee Column's group walks when you're within 150 m, stops when you run ahead or
  infected come within 25 m, and joins the safehouse on arrival. With the escort prefab empty they
  move as data and stand as a civilian figure when they stop. A fourth escort contract never starts.
- [ ] Rescue the Pilot: help him up (talk, or stand beside him 10 s) and he follows you; the clock
  stops for a Nurse; killing his AI body fails it.
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

**Multiplayer and griefing** (two clients at least; the pass 2 sweep's open risks)
- [ ] Two players race one choice delivery: the first hand-in wins, the second is told it isn't
  wanted. A player who isn't eligible delivers someone else's contract: only eligible people at
  the drop are paid.
- [ ] Carry a delivery item (Courier letters, a Bastion case, the capture case, Water Purifier or
  Restore Comms parts), then disconnect: the others are told and the item is back on the ground
  where you were (or, for parts, at its first spot). Die with it and wait for the body to go: same.
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
