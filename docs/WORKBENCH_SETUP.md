# Workbench setup

How to turn the scripts in this repo into a playable mod. The scripts are plain Enforce
Script; prefabs, configs and the `.gproj` are made in Workbench, because they need GUIDs that
Workbench generates. **Nothing has been compiled yet.** Expect to fix the `VERIFY` list first.

Work through the sections in order:

1. [Project and dependencies](#1-project-and-dependencies)
2. [The minimum for a playable loop on Arland](#2-the-minimum-for-a-playable-loop-on-arland)
3. [Factions](#3-factions)
4. [Prefabs by system](#4-prefabs-by-system)
5. [Settings objects](#5-settings-objects)
6. [The VERIFY list](#6-the-verify-list)
7. [Log lines to check](#7-log-lines-to-check)
8. [Play-test checklist](#8-play-test-checklist)

How each system behaves is in [DESIGN.md](DESIGN.md); this doc only says how to set it up.

## 1. Project and dependencies

1. Subscribe to the dependencies in the game ([MOD_STACK.md](MOD_STACK.md)): at minimum
   **Bacon Zombies**; Metabolism and Special Zombies are recommended.
2. Arma Reforger Tools → **Create new project** named `ReforgerZ`.
3. Put this repo's contents in the project's folder, so `Scripts/` sits next to `addon.gproj`
   (or clone the repo there first and point the new project at that folder).
4. Project Settings → Dependencies: add Arma Reforger, Bacon Zombies and whatever else you use.
5. Open the Script Editor and compile (**F7**). Fix errors using [the VERIFY list](#6-the-verify-list).
   Commit `addon.gproj` once it exists.

Look up classes in the Script Editor or the
[Script API docs](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/).

## 2. The minimum for a playable loop on Arland

Every optional prefab is an attribute on `RZ_GameModeComponent` (or one of its settings
objects). **A feature whose prefab is empty turns itself off**, so you can start with this list
and add the rest from [section 4](#4-prefabs-by-system) over time.

| Step | What | Gets you |
|---|---|---|
| 1 | A world as a **sub-scene of Arland**, with a `FactionManager` and a `LoadoutManager` | — |
| 2 | The factions `SURV`, `MIL`, `BAND`, `INST` and the zombie faction ([section 3](#3-factions)); the 15 hidden variants can wait | Spawning |
| 3 | 13 loadouts on `SURV`, named exactly as the backgrounds ([section 3](#3-factions)) | Backgrounds |
| 4 | `RZ_GameMode.et`: vanilla `GameMode_Plain` (or any `SCR_BaseGameMode` prefab) with `RZ_GameModeComponent`, placed in the world | The mode |
| 5 | **World > Spawn point prefab** (`RZ_SpawnPoint.et`, a vanilla spawn point) | Spawn points in towns, safehouse spawns |
| 6 | **Zombies:** Bacon common, special and horde group prefabs, and the vanilla `AIWaypoint_Move.et` as **Move waypoint** | The infected, hordes, many missions |
| 7 | **Cure:** lab, sample and antidote prefabs | The cure |
| 8 | **Missions > Reward crate** | Mission rewards |
| 9 | The four **quartermasters** with a few shop actions, `RZ_StandingStatusAction` and the four `RZ_LeaderAction`s; the Haven one also with the five `RZ_SafehouseAction`s | Shops, leaders, safehouse services |
| 10 | **Missions > Mission item** and the four **drop points** | Delivery and choice missions |
| 11 | **Puzzles > Note**, **Puzzles > Puzzle object** and **Puzzles > Civilian** | Puzzles, unarmed missions, wanderers, leaders and occupants to talk to, interaction spots |

No towns, spawn points or lab need placing: they're generated at start. With only this,
these stay off: hunter squads, Perimeter checkpoints and drones (the line still works with
messages), supply trucks (places count as supplied), territory guards, adaptation traits, the
power station's own control prefab (it falls back), world-interaction loot (searches find
nothing until the loot pool is filled), fixable cars, crafting and traps.

**On another map:** make another sub-scene with the same contents, then check the log lines in
[section 7](#7-log-lines-to-check). If the map has no name labels and few buildings, place a few
`RZ_SettlementMarkerEntity`s. Check the Perimeter rectangle, checkpoints, safehouses, territory,
power and radio station, and whether "Search" spots appear in houses.

## 3. Factions

In the world's `FactionManager`:

| Key | In game | Used for |
|---|---|---|
| `SURV` | The Haven, and every player | The player faction. Its loadout list holds the backgrounds |
| `MIL` | Task Force Bastion | Bastion AI: guards, crews, hunter squads |
| `BAND` | The Red Hand | Red Hand AI: mission groups, guards, crews, hunter squads |
| `INST` | Halcyon | Halcyon AI: guards, crews |
| Bacon Zombies' faction | The infected | Hostile to all of the above |

**Loadouts.** On `SURV`, one loadout per background, named exactly: `Nurse`, `Mechanic`,
`Hunter`, `Police Officer`, `Ex-Con`, `Lab Technician`, `Radio Operator`, `Unemployed`, and the
five unlockable ones `Soldier`, `Doctor`, `Raider`, `Elder`, `Drifter`. Each is a civilian
character prefab with that background's starting gear. Players who pick a locked one keep its
gear but play as Unemployed. The log warns `No background matches loadout '...'` for a typo.

**Relations.** AI shoots individual players through the hidden variants below, so the base
relations are the "nobody wants you dead" case. Set them in each faction's friendly list, both ways:

| | SURV | MIL | BAND | INST |
|---|---|---|---|---|
| SURV | friendly | friendly | friendly | friendly |
| MIL | friendly | | hostile | friendly |
| BAND | friendly | hostile | | hostile |
| INST | friendly | friendly | hostile | |

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

## 4. Prefabs by system

The code has **127 prefab fields**: 40 on `RZ_GameModeComponent`, 81 in its settings objects, and
6 in list entries and actions (shop item, requisition item, recipe output, loot table lists).
Every prefab with an action list needs an `ActionsManagerComponent`; every prefab with an `RZ_`
component needs an `RplComponent`. Slot names are the attribute's category, then its label.

### The generic puzzle object (used everywhere)

Many systems reuse one prefab for anything that's read, talked to or used. Make it early.

| Prefab | Add | Slot |
|---|---|---|
| `RZ_PuzzleObject.et` (a crate, sign or post) | `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`, `RZ_ReadClueAction`, 4 × `RZ_PuzzleUseAction` (values 0–3), 4 × `RZ_PuzzleChoiceAction` (options 0–3) | Puzzles > Puzzle object |
| `RZ_Civilian.et` (a civilian without AI, or a seated figure prop) | the same components | Puzzles > Civilian |
| `RZ_Note.et` (a paper or notebook prop) | `RZ_ClueComponent`, `RZ_ReadClueAction` | Puzzles > Note |

Missions label the actions ("Seal the well", "Accuse Mirek") and hide the slots they don't use.
**Without replicated labels** (a VERIFY), every slot shows its generic label, and the actions
still route correctly.

### Core and cure

| Prefab | Base | Add | Slot | | Enables |
|---|---|---|---|---|---|
| `RZ_GameMode.et` | `GameMode_Plain` | `RZ_GameModeComponent` | (placed) | **required** | everything |
| `RZ_SpawnPoint.et` | vanilla spawn point | | World > Spawn point prefab | **required** | spawning |
| `RZ_Lab.et` | a field hospital tent or lab container | `RZ_LabComponent`; `RZ_DepositSamplesAction`, `RZ_LabStatusAction` | Cure > Lab prefab | **required** | the cure (spawned in a town if none is placed) |
| `RZ_Sample.et` | a small item (a vial) | `RZ_SampleItemComponent` | Cure > Sample | **required** | samples from specials |
| `RZ_Antidote.et` | morphine or saline item | `RZ_AntidoteItemComponent`, `RZ_InjectAntidoteAction`; optionally the consumable effect `RZ_ConsumableAntidote` | Cure > Antidote | **required** | antidotes |
| `RZ_RewardCrate.et` | a vanilla supply box, filled with loot | | Missions > Reward crate | **required** | mission rewards |

### Zombies

| Prefab | Slot | | Enables |
|---|---|---|---|
| Bacon civilian and military groups | Zombies > Common prefabs | **required** | the infected |
| Special Zombies or Bacon heavy, exploding and ranged | Zombies > Special prefabs | **required** | samples, the cure |
| A large Bacon group | Zombies > Horde prefabs | **required** | hordes |
| `AIWaypoint_Move.et` | Zombies > Move waypoint | **required** | hordes toward noise, unarmed missions, hunter squads, supply trucks, favourite spots |
| Runner, helmet and screamer prefabs | Zombies > Adaptation | optional | the stage traits |

### Missions

| Prefab | Base | Add | Slot | Enables |
|---|---|---|---|---|
| `RZ_MissionItem.et` | a small carryable item | `RZ_MissionItemComponent` | Missions > Mission item | every delivery and choice mission |
| `RZ_Drop_Haven.et` / `_Bastion` / `_RedHand` / `_Halcyon` | a crate, table or mailbox | `RZ_ChoiceDeliveryAction` with faction SURVIVORS / MILITARY / BANDITS / INSTITUTE | Missions > Haven / Bastion / Red Hand / Halcyon drop point | the drops those missions name (Medicine Run, Fence the Goods, Crash Site, Requisition, Research Data, the Perimeter and leader deliveries) |
| `RZ_SurvivorGroup.et` | a civilian AI group on `SURV` | | Missions > Survivor group | real survivors in Survivors Trapped and Training Day; The Bitten Friend; Trader Caravan |
| `RZ_Nest.et` | a Zombie Hive prefab or destructible prop | a damage manager | Missions > Nest | Burn the Nest |
| `RZ_BanditDrop.et` | crate, barrel or car wreck | `RZ_SellSamplesToBanditsAction` | Missions > Dead drop | Dead Drop |
| `RZ_HeliWreck.et` | a helicopter wreck prop | optionally `RZ_ClueComponent` + `RZ_ReadClueAction` | Missions > Helicopter wreck | Crash Site, Crashed Helicopter, Two Bearings |
| `RZ_CorpsePile.et` | a body bag pile | | Missions > Corpse pile | visual only |
| `RZ_RedHandGroup.et` | an AI group on `BAND` | | Missions > Red Hand group | Raid the Red Hand, Trader Caravan, raiders in Lab Defence, the camp in The Witness; default Red Hand hunter squad |
| `RZ_FuelDepot.et` | fuel tanks | a damage manager | Missions > Fuel depot | Arson |
| `RZ_Relay.et` | a radio mast | a damage manager | Missions > Relay | Sabotage the Relay |
| `RZ_Trader.et` | a character or vehicle (e.g. a Shop System trader) | | Missions > Trader | Wandering Trader |
| `RZ_Explosion.et` | a mortar impact effect | | Missions > Explosion | Bastion Shelling |

One mission item prefab serves everything (medicine, notes, black box, package): the mission
tags it when it spawns. Missions spawn and remove their own drop points.

### Quartermasters and shops

Place one quartermaster per faction, or set the four slots and they spawn by themselves (the
Haven's in the safest town, Bastion's in a bigger town, the Red Hand's in a rougher town,
Halcyon's next to the lab). Their positions are saved.

| Prefab | Add | Slot |
|---|---|---|
| `RZ_QM_Haven.et` | `RZ_QuartermasterComponent` (SURVIVORS); `RZ_StandingStatusAction`; several `RZ_FactionShopAction`; 4 × `RZ_LeaderAction` (TALK, WORK, NEWS, STANDING); 5 × `RZ_SafehouseAction` (STATUS, DELIVER_FOOD, RADIO, HEAL, DECLARE_BITE) | Factions > Haven quartermaster |
| `RZ_QM_Bastion.et` | the same with MILITARY (no safehouse actions), plus one `RZ_RequisitionAction` per requisition item | Factions > Bastion quartermaster |
| `RZ_QM_RedHand.et` | the same with BANDITS | Factions > Red Hand quartermaster |
| `RZ_QM_Halcyon.et` | the same with INSTITUTE | Factions > Halcyon quartermaster |

Each `RZ_FactionShopAction` sells one thing: **Faction**, **Required tier** (Stranger, Known,
Trusted or Champion), **Favor cost**, **Item prefab** (an item, crate or vehicle, spawned next
to the quartermaster) and **Display name**. The action is always shown; the server checks tier
and Favor when it's used. `RZ_RequisitionAction` has the same cost, item and name; the cost is
the full price (a Trusted player pays half, once an hour). Set it up for ammo and rations only.
You can also put `RZ_ChoiceDeliveryAction` on a quartermaster so it accepts hand-ins.

Suggested starting stock (from [REPUTATION.md](REPUTATION.md)):

| Faction | Known | Trusted | Champion |
|---|---|---|---|
| Haven | canned food, water, bandages, crowbar, car parts | hunting rifle, ammo, backpack, radio, jerrycan | pickup, van, fortification kit, good medkit |
| Bastion | rations, morphine, flares | service rifle, body armour, helmet, NVGs | machine gun, grenades, military truck |
| Red Hand | knife, pistol | suppressor, scoped rifle, explosives | rare military gear, a stolen antidote |
| Halcyon | antiseptic, painkillers | antidote, stim kit, gas mask | — |

### Faction consequences

| Prefab | Base | Slot | Enables |
|---|---|---|---|
| `RZ_BastionHunters.et` | an AI group on `MIL` | Factions > Bastion hunter squad | Bastion hunter squads |
| `RZ_RedHandHunters.et` | an AI group on `BAND` | Factions > Red Hand hunter squad (falls back to the Red Hand group) | Red Hand hunter squads, safehouse location leaks |

Gunfire heat needs nothing: characters already have an `EventHandlerManagerComponent`, and
suppressors are detected by attachment names containing "suppress" or "silenc".

### Puzzles

Slots under **Puzzles**. The specific ones only change the look; give them the generic object's
components. Missions turn themselves off when a prefab they need is missing.

| Prefab | Add | Slot | Needed by |
|---|---|---|---|
| `RZ_Keypad.et` | `RZ_PuzzleObjectComponent`, 12 × `RZ_KeypadAction` (0–9, 10 Clear, 11 Enter), optionally clue components | Puzzles > Keypad | The Last Experiment (keypad mode) |
| `RZ_Terminal.et` | as the keypad | Puzzles > Terminal (falls back to the keypad) | The Last Experiment |
| `RZ_PuzzleBody.et` | `RZ_ClueComponent`, `RZ_ReadClueAction` | Puzzles > Body (falls back to the note) | Paces, Patient Zero's Ledger, Poisoned Well, Last Experiment, The Witness, The Deserter's Map |
| `RZ_Well.et`, `RZ_Bell.et`, `RZ_FuseBox.et`, `RZ_RadioMast.et`, `RZ_Safe.et` | as the generic object | Puzzles > Well / Bell / Fuse box / Radio mast / Safe | look only |
| `RZ_ScaleCrate.et` | as the generic object, plus an inventory storage | Puzzles > Scale crate | The Weighing Vault |
| `RZ_Landmark.et` | none | Puzzles > Landmark, plus **Landmark name** ("the old cross") | towns with no church, tower or well |
| `RZ_MorseLamp.et` | `RZ_BlinkLightComponent` (or nothing: then spawned per flash) | Puzzles > Morse light | Lights in the Window |

**Noisemakers** aren't slots: make these items and put them in loot or shops. Each has
`RZ_NoisemakerComponent`, and timed ones `RZ_ArmNoisemakerAction`. A noise only leads infected at
8 heat or more at once, so keep them at 10 or more.

| Prefab | Base | Settings |
|---|---|---|
| `RZ_Bottle.et` | a vanilla grenade with the explosion removed and a bottle mesh | heat 20, on throw, deleted after |
| `RZ_Firecracker.et` | a small item | heat 15, delay 5 s, 4 bursts 3 s apart, "Light the fuse", deleted after |
| `RZ_AlarmClock.et` | a clock prop item | heat 10, delay 30 s, 8 bursts 8 s apart, "Wind the alarm", kept |

The **unarmed missions** need no prefabs of their own: they reuse the mission item, the Haven
drop point, the note, civilian, generic object and the Move waypoint.

### The Perimeter

Works with no prefabs at all (loudspeaker messages, the warning shot as a message, the unseen
shooter). Slots are in **Perimeter**.

| Prefab | Base | Slot | | Used for |
|---|---|---|---|---|
| `RZ_Drone.et` | a small quadcopter mesh with a hum and a light; **no collision or physics**; `RplComponent` | Drone prefab | optional | one drone over a player in the warning zone |
| `RZ_WarningShot.et` | a gunshot sound or impact effect | Warning shot prefab | optional | the warning shot |
| `RZ_Checkpoint.et` | concrete blocks and a closed barrier | Barricade prefab | **required for checkpoints** | road checkpoints |
| `RZ_CheckpointGuard.et` | a soldier without AI, or a figure prop | Guard prefab | optional | 0–4 per checkpoint |
| `RZ_Smuggler.et` | a civilian without AI | Smuggler prefab (falls back to Puzzles > Civilian) | optional | The Blind Spot |
| `RZ_Package.et` | a small box item with `RZ_MissionItemComponent` | Package prefab (falls back to the mission item) | optional | The Blind Spot, The Crossing |
| `RZ_LineBody.et` | a body with `RZ_ClueComponent`, `RZ_ReadClueAction` | Line body prefab (falls back to Puzzles > Body) | optional | Shot at the Line |

### Haven safehouses and wanderers

Safehouses need only the spawn point and the Haven quartermaster. Wanderers need nothing:
without bodies they use Puzzles > Civilian. A body can be talked to only if it has the generic
object's components.

| Prefab | Base | Slot | Used for |
|---|---|---|---|
| `RZ_SafehouseMedic.et` | a civilian without AI with 2 × `RZ_SafehouseAction` (HEAL, DECLARE_BITE) | Haven > Medic prefab | Settlement and up (the quartermaster's actions work without it) |
| `RZ_SafehouseOccupant*.et` | civilians without AI or seated props, with the generic components | Haven > Occupant prefabs (a list; empty = Puzzles > Civilian) | 3–6 people shown |
| `RZ_SafehouseGuard.et` | an armed AI character or group on `SURV` | Haven > Guard prefab | guards; they shoot armed exiles |
| `RZ_Wanderer_Trader.et`, `_Doctor`, `_Scavenger`, `_Deserter`, `_Preacher`, `_Courier`, `_Kid`, `_Mechanic` | a civilian without AI (stands still while seen), or a one-man AI group on `SURV` (walks while seen; give it a gun to fight back) | Wanderers > one slot per role | the roster's bodies |
| Trader goods | any items | Wanderers > Trader goods | what the Trader sells |

### Faction leaders

Leaders need no prefabs: the quartermaster *is* the leader at strongholds and safehouses, and
outpost leaders use Puzzles > Civilian. **Add the four `RZ_LeaderAction`s to every quartermaster**;
they're how players reach stronghold leaders and council heads.

| Prefab | Slot | Used for |
|---|---|---|
| `RZ_Leader_Haven.et`, `_Bastion`, `_RedHand`, `_Halcyon` (a character without AI) | Leaders > Haven / Bastion / Red Hand / Halcyon leader | optional body next to the quartermaster; damage off |
| `RZ_CouncilHead.et` | Leaders > Council head | optional body per safehouse |
| `RZ_Outpost_Bastion.et`, `_RedHand`, `_Halcyon` | Leaders > outpost prefabs (fall back to the faction leader, then the civilian) | optional outpost leaders |

### Territory and supply trucks

Territory is data: every prefab here only adds bodies near players. A faction without a truck
and crew counts its places as always supplied.

| Prefab | Base | Slot | Used for |
|---|---|---|---|
| `RZ_Guard_Haven.et`, `_Bastion`, `_RedHand`, `_Halcyon` | an armed AI character or small group on `SURV` / `MIL` / `BAND` / `INST` | Territory > ... guard | guards at holdings; watched for retaliation |
| `RZ_Occupant_Haven.et`, `_Bastion`, `_RedHand`, `_Halcyon` | a character without AI in the faction's clothes, with the generic components | Territory > ... occupant (empty = Puzzles > Civilian) | the named people |
| the four quartermasters | (existing) | Territory > **Outpost quartermaster** on | a shop at each outpost; Bastion's Requisition |
| `RZ_Truck_Haven.et`, `_Bastion`, `_RedHand`, `_Halcyon` | a drivable vehicle with a damage manager | Supply > ... truck | the supply truck |
| `RZ_TruckCrew_Haven.et`, `_Bastion`, `_RedHand`, `_Halcyon` | an AI group of 1–2 on the faction; the first drives | Supply > ... crew | the crew |

Supply trucks also need **Zombies > Move waypoint**.

### New-player guidance

Works with no prefabs: tips use the people and radios that exist, and scenario clues fall back to
the note and body. Slots under **Guidance**.

| Prefab | Base | Slot | Used for |
|---|---|---|---|
| `RZ_WindupRadio.et` | a small radio item; **its prefab name must contain "radio"** (or it needs a radio component); add `RZ_TuneRadioAction` | Radio prefab | given to every new life; empty = none |
| `RZ_CarWreck.et` | a wrecked car prop | Car wreck prefab | the back-road scenario |
| `RZ_ScratchedDoor.et` | a door panel with clue components | Door prefab | the basement scenario |
| `RZ_Flyer.et` | a poster with clue components | Flyer prefab | the church scenario, safehouse flyers |
| `RZ_TableRadio.et` | a radio prop with clue components (label "Listen") | Table radio prefab | the farmhouse scenario |
| `RZ_DeadRaider.et`, `RZ_DeadSoldier.et` | body props with clue components | Raider / Soldier body prefab | the attic and barricade scenarios |
| `RZ_Barricade.et` | sandbags and a car on its side | Barricade prefab | barricade scenery |

### Environment, the power grid and the radio station

All work without prefabs: the control object and console fall back to the puzzle prefabs.

| Prefab | Base | Slot | Used for |
|---|---|---|---|
| `RZ_PowerControl.et` | a generator or breaker cabinet with the generic components | World > Power grid > Control prefab (falls back to Puzzles > Fuse box, then the generic object) | the power station |
| `RZ_RadioConsole.et` | a radio set on a desk with the generic components | World > Radio station > Console prefab (falls back to Puzzles > Radio mast, then the generic object) | the radio station |

### World interactions

Everything works without prefabs (spots fall back to the generic object, which is visible), but
**searches find nothing until the loot pool is filled**, and each feature whose output is empty
stays quiet. Slots under **Interactions**.

| Prefab | Base | Slot | Used for |
|---|---|---|---|
| `RZ_InteractionSpot.et` | a `GenericEntity` with a small invisible collision box, `RplComponent`, `ActionsManagerComponent` and the generic components | General > Spot prefab | every searchable thing near players |
| `RZ_CarProject.et` | a non-drivable car prop with the same components | Cars > Project prefab | a fixable car before it runs |
| Drivable cars | vanilla civilian cars | Cars > Car prefabs (empty = no fixable cars) | what a fixed car becomes |
| `RZ_CarBattery.et`, `RZ_Tyre.et`, `RZ_SparkPlugs.et` | items named with "battery", "tyre", "spark" | Cars > Battery / Tyre / Spark | parts |
| `RZ_FuelCan_Full.et`, `RZ_FuelCan_Empty.et` | jerrycans | Cars > Fuel prefab | siphoning, pumps, fuel, fire |
| `RZ_Planks.et` | a planks item with `RZ_PlaceableComponent` (Barricade), `RZ_PlaceAction` | (loot) | barricades |
| `RZ_BoardedDoorway.et` | planks across a door frame, with collision | Traps > Barricade prefab | the boarded doorway |
| `RZ_TripwireKit.et`, `RZ_SpikeTrapKit.et` | items with `RZ_PlaceableComponent`, `RZ_PlaceAction` | Crafting > Tripwire / Spike trap | traps (optional "set" props in Traps) |
| `RZ_PortableGenerator.et` | an item with `RZ_PlaceableComponent` (Generator) and two `RZ_PlaceAction`s (one with Refuel) | (loot or shop) | a generator |
| `RZ_Molotov.et` | a throwable bottle with `RZ_MolotovComponent` | Crafting > Molotov | fire |
| `RZ_FireEffect.et` | a fire and smoke particle | Traps > Fire effect | a burning building |
| Recipe outputs | bandage, tin rattle (`RZ_NoisemakerComponent`), spear, tablets | Crafting > Bandage / Noisemaker / Spear / Tablets | the workbench |
| Water | untreated and safe bottles (the survival mod's if it has both) | General > Dirty / Clean water | wells, stoves |

**The loot pool** (Search > Loot pool) is one list of every item a search can find. Each building
type takes the items whose prefab names contain its keywords (the lists are in
`RZ_Search.TypeWords`). To control a type exactly, add a **Loot table** for it.

### Add to existing prefabs

- **The player character** (every loadout): `RZ_TreatOtherAction`, so others can treat you.
- **The wind-up radio and any crackable radio:** `RZ_TuneRadioAction`.
- **The Haven quartermaster:** `RZ_FactionShopAction`s for the car parts (Known), if wanted.

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

`RZ_GameModeComponent` holds 22 settings objects and two lists. Every value has a working
default; this is what each one covers. Defaults are in brackets where they matter for setup.

| Category | Object | Covers |
|---|---|---|
| ReforgerZ | (top level) | Tick seconds (5), **Player faction key** (SURV), **Wipe on start**, save seconds (120) |
| World | (top level) | Cluster cell (300 m), buildings per settlement (8), spawn point prefab, active spawn points (6), minutes between moves (20) |
| World | Town memory | Loot return (120 min), looted below (0.5), loot floor (0.2), burned share (0.5), journal minutes (10) |
| World | Power grid | Control prefab, region (2.5 km), spin-up (90 s), noise (10 heat/min), run hours (4), fuel and patch restore |
| World | Radio station | Console prefab, takeover (30 s), cooldown (10 min, halved with power) |
| Director | Stages (list) | Per-stage overrides; empty = the built-in four |
| Director | (top level) | **Broadcast minutes** (25; 0 = only on first join) |
| Director | Environment | Night heat spread (0.25), night specials (×1.25), rain and fog thresholds, storm thresholds, storm grounds the helicopter |
| Director | Story arcs | Minutes between (60), start chance (0.6), repeat hours (4), the dark-side arc's limits |
| Zombies | Zombie settings | Common, special, horde prefabs, Move waypoint, per-size spawns (4), activation (600 m), despawn (1.1 km), min distance (120 m), caps (60, 4 per tick), ambient per player (2), kill infection (1.5), horde heat (80), horde gap (600 s) |
| Zombies | Adaptation | Specials per breakthrough (0.05, cap 0.15), trait prefabs, trait share (0.15), screamers, favourite spots |
| Zombies | (top level) | Presence heat (0.5/tick), vehicle heat (+4/tick) |
| Backgrounds | Backgrounds (list) | Overrides; empty = the built-in 13. **Bite minutes** (20) |
| Cure | (top level) | Lab, sample, antidote prefabs; research per sample (2%) |
| Missions | (top level) | Reward crate, mission item, drop points and the optional mission prefabs |
| Puzzles | (top level) | The puzzle prefabs, **Scale counts items** (off), landmark name |
| Perimeter | Perimeter | **Enabled**, warn (300 m) and kill (100 m) distance, grace (8 s), fit to land and sea margin (200 m), **Override** rectangle, drone height, **Checkpoints**, guards (2), max (6, at most 8), inset, spacing, warn (150 m) and kill (60 m) radius, mission prefabs |
| Haven | Safehouses | **Enabled**, count (3), start population (4) and food (12), visible radius (300 m), upkeep (180 min), people per food (5), **Food keywords**, Favor per food (2), heal cost (15), house-rule radii and Infamy, leak chance (0.25), body prefabs |
| Wanderers | Wanderers | **Enabled**, min and max count (6, 10), prices, spawn radius (400 m), walk speed, replace minutes (1440), bite chance, Infamy, body prefabs, Trader goods |
| Leaders | Leaders | **Enabled**, quartermaster is leader (on), interim posts (1, territory off only), body radius (200 m), replace minutes (120), job minutes (30), danger scale, discovery, kill Infamy, body prefabs |
| Territory | Territory | **Enabled**, outposts per faction (0 = by map size), held share (0.2), tick (20–30 min), pressure minutes (25), visible radius (300 m), guard and occupant counts, outpost quartermaster (on), guard and occupant prefabs |
| Territory | Supply | **Enabled**, truck and crew prefabs, departures (40–60 min), unstick radius (1 km), stuck seconds (90), stock (8), Haven food (6), drain (45 min), out-of-stock share (0.3), rewards |
| Territory | Faction life | **Enabled**, chatter (10 min), life events (35 min), truces and feuds (on), false rumour chance (0.35) |
| Guidance | Guidance | **Enabled**, meaningful minutes (10), level thresholds (1/3/8), refresher days (30), gentle mult and minutes (0.5, 15), tip gap (90 s), loud heat (45), **Tips** table, radio minutes (12), scenarios, clue minutes (20), flyers, shout minutes (30), wanderer finds, the scenario prefabs |
| Interactions | General | **Enabled**, spot prefab, scan radius (30 m) and seconds (10), caps (10 per player, 70), bells and sirens, water items and keywords, treating others |
| Interactions | Search | **Loot pool**, loot tables, chances (find 0.6, rare 0.1, electronics 0.1), locks and their shares, crowbar keywords, readable and body chances |
| Interactions | Cars | Car prefabs, project prefab, per settlement (0.34), max (6), parts and keywords, hotwire and siphon timings, Mechanic part cost (12) |
| Interactions | Crafting | Workbench keywords, the eight recipes' outputs and inputs, extra recipes |
| Interactions | Traps | Barricade, tripwire, spike trap limits and props, generators, fire and molotovs |
| Interactions | Secure radio | Crack seconds (60), listen minutes (20), line minutes (4), range (3 km), interrupts, house radio keywords |
| Survival | (top level) | Shot heat (3), suppressed mult (0.3), bite chance (0.3), bite reach (2.5 m) |
| Factions | (top level) | Quartermaster and hunter prefabs, hunter minutes (10; 0 = off), bounty minutes (10) |
| Factions | Hostility | **Mode** (Bitmask / Outlaw / Off), each faction's **Shoots from** (Hunted), exile armed only (on) and radius (100 m), fight radius (350 m), wanderers fight back (on) and grudge radius (80 m), **Retaliation** (on, 300 m, 180 s, 3 Infamy, cap 12/h), the three AI faction keys |
| Map | (top level) | **Map markers** (on) |
| Debug | (top level) | Status log every few minutes (on) |

**Keywords are guesses.** Food, water, fuel, parts, crowbars, medkits, furniture, wrecks and
buildings are found by lower-case words in prefab names. Check them against the mods you run.

**Server.** Keep time acceleration modest and fixed; don't use time-skip or sleep mods. For a
season wipe, set **Wipe on start** for one restart, or delete `$profile:ReforgerZ/state.json`.
The last winner is kept in `$profile:ReforgerZ/last_season.json`, which a wipe doesn't touch.

## 6. The VERIFY list

```bash
grep -rn "VERIFY" Scripts/
```

There are **93** markers (one is the comment in `Core/RZ_Util.c` that explains them). Each is an
engine API whose exact name or signature couldn't be confirmed. Usually the fix is a rename, and
most are isolated in small helpers in `Core/RZ_Util.c`. They're grouped by risk; fix them in this
order. Paths are under `Scripts/Game/ReforgerZ/`.

### High risk: the core loop (25)

No fallback. If these are wrong, the basic game doesn't work.

| Where | Call | If it fails |
|---|---|---|
| `Core/RZ_GameModeComponent.c:536` | `GetOnPlayerSpawned()` invoker | Nothing per-player runs on spawn (backgrounds, goals, radio) |
| `Core/RZ_GameModeComponent.c:831` | `GetBackendApi().GetPlayerIdentityId` | Profiles can't follow a player across lives |
| `Core/RZ_GameModeComponent.c:900` | `SCR_LoadoutComponent.GetLoadout()` | Everyone plays as the default background |
| `Core/RZ_GameModeComponent.c:1032` | `OnProjectileShot` event and signature | No gunfire heat |
| `Core/RZ_GameModeComponent.c:1696`, `:1697` | `SetFactionKey`, `SetSpawnPointName` on spawn points | Spawn points unusable or unnamed |
| `Core/RZ_GameModeComponent.c:1707` | `GetGame().GetWorldFile()` | Saves can't tell maps apart |
| `Core/RZ_GameModeComponent.c:1923`, `Cure/RZ_LabComponents.c:253` | `SCR_HintManagerComponent.ShowCustomHint` | Players see no messages at all |
| `Persistence/RZ_SaveData.c:117` | JSON contexts serialise plain members | Nothing is saved |
| `World/RZ_SettlementScanner.c:59`, `:69` | building base class; `GetBaseType()` and `EMapDescriptorType` | No settlements: place `RZ_SettlementMarkerEntity`s |
| `Core/RZ_Util.c:369`, `:377` | `GetPrefabData().GetPrefabName()`, `GetVObject().GetResourceName()` | Landmarks, building types, furniture and territory names fail; puzzles use the fallback landmark |
| `Core/RZ_Util.c:614`, `Zombies/RZ_BiteDetector.c:74` | total health accessor (maybe `GetDefaultHitZone().GetHealth()`) | No bites; retaliation only sees kills |
| `Core/RZ_Util.c:171` | `Kill()` signature | The infection timer and kill zone can't kill |
| `Cure/RZ_LabComponents.c:72`, `Missions/RZ_MissionItems.c:49`, `Cure/RZ_AntidoteItem.c:23` | `IsMatch` predicate signature | Samples, mission items and antidotes aren't found in inventories |
| `Cure/RZ_LabComponents.c:156`, `Missions/RZ_MissionItems.c:75`, `Cure/RZ_AntidoteItem.c:47` | `TryDeleteItem()` | Items aren't taken when handed in |
| `Puzzles/RZ_RadioStealth.c:36` | `GetItems()` lists every carried storage | Radio checks fail (the same call is used for food and keywords) |
| `Puzzles/RZ_PuzzleActions.c:15` | `ScriptedUserAction.GetOwner()` | No clue can be read |

### Medium risk: big features, with a fallback (39)

| Where | Call | Fallback |
|---|---|---|
| `Factions/RZ_FactionRelations.c:16` | modded `SCR_Faction.DoCheckIfFactionFriendly(Faction)` is what AI uses | **The biggest unknown.** Test with a Bastion-Hunted player near Bastion AI. If it never fires, set **Hostility > Mode** to *Outlaw* and give `SURV_OUTLAW` real enemies |
| `Core/RZ_Util.c:403`, `:428` | `GetAffiliatedFaction`, `SetAffiliatedFaction` (server side, replicated, read by AI) | Same as above. Check that the deploy menu, chat, groups and markers stay on `SURV`, and that nothing resets the character (the tick puts it back within 5 s) |
| `Core/RZ_Util.c:475`, `:482` | `GetWeaponsList`, `GetWeaponType` and `EWeaponType` names | Always false: armed exiles never shot, wanderers never fight back. Always true: unarmed exiles shot |
| `Core/RZ_Util.c:546` | `MoveInVehicle(vehicle, ECompartmentType)` seats AI, `PILOT` drives | No truck ever arrives. Try the vanilla "get in" waypoint, or a vehicle prefab with its crew already inside |
| `Factions/RZ_SupplyLines.c:816` | a seated AI group drives to a move waypoint | As above. Test one truck end to end first |
| `Core/RZ_Util.c:560` | vehicle `SCR_DamageManagerComponent.GetState()` | A burnt-out truck counts as late instead |
| `Core/RZ_Util.c:574`, `:578` | `SetWorldTransform`, `Physics.SetVelocity` | Unsticking only runs with nobody within 1 km; check truck and crew arrive together |
| `Interactions/RZ_Interactions.c:635` | furniture inside buildings is found as static entities | Every building still gets one "Search the house" spot |
| `Interactions/RZ_Interactions.c:703` | `SCR_DestructibleBuildingEntity` as the building class | Buildings aren't recognised; furniture still is |
| `Interactions/RZ_Interactions.c:248` | `GetItems()` for keyword items | Crowbars, parts, fuel, water and medkits not found |
| `Puzzles/RZ_Landmarks.c:55` | `QueryEntitiesBySphere(center, radius, cb, filter, flags)` | No landmarks: the fallback landmark prop |
| `Puzzles/RZ_PuzzleComponents.c:80`, `:142`, `Puzzles/RZ_BlinkingLight.c:161` | `[RplProp]` on a `ScriptComponent` | Labels don't replicate: generic labels, all slots visible, actions still work |
| `Puzzles/RZ_GameTime.c:21`, `:25`, `:29` | `ChimeraWorld.CastFrom`, `GetTimeAndWeatherManager`, `GetTimeOfTheDay` | -1: night missions never appear, no night effects |
| `Director/RZ_Environment.c:67`, `:82`, `:93`, `:104` | world cast; rain, fog and wind accessors | Make the helper return -1: only night and the Storm event apply |
| `World/RZ_Perimeter.c:226` | `BaseWorld.GetBoundBox()` | Turn on **Override** and type in the corners |
| `World/RZ_Perimeter.c:725`, `:732`, `:733`, `:778`, `:782` | road network: `GetAIWorld`, `GetRoadNetworkManager`, `GetRoadsInAABB`, `GetPoints`, `GetWidth` | Empty `FindRoadExits()` and place `RZ_CheckpointMarkerEntity`s |
| `World/RZ_Perimeter.c:633`, `:647` | `AddChild` keeps the drone on the player; `SetOrigin` replicates | Moved each tick instead (jumpier) |
| `Core/RZ_Util.c:196` | `EnableDamageHandling(false)` | Checkpoint guards and stronghold leaders can be shot (leaders are "wounded" and come back) |
| `Core/RZ_Util.c:184` | `FullHeal()` | Medics and Doctors take Favor but don't heal: fix before release |
| `Core/RZ_Util.c:257` | `TryInsertItem` | Items (the wind-up radio) land at the player's feet |
| `Core/RZ_Util.c:301` | `AIGroup.GetWaypoints()` / `RemoveWaypoint()` | Hordes can't be redirected (The Clockwork Horde, unarmed missions) |
| `World/RZ_Safehouses.c:336`, `:346` | `GetItems`, `TryDeleteItem` for food | Food isn't found or taken; check the food keywords first |
| `Core/RZ_Util.c:594` | fuel nodes: `GetFuelNodesList`, `GetMaxFuel`, `SetFuel` | Empty the helper: a fixed car has its prefab's own fuel |

### Low risk: contained or cosmetic (29)

| Where | Call | Fallback |
|---|---|---|
| `Core/RZ_Util.c:3` | (the comment that explains VERIFY) | — |
| `Core/RZ_MapMarkers.c:14`, `:19`, `:20`, `:21`, `:22`, `:44`, `:46` | map marker manager, type, position, text, insert, remove | Empty `Add()` and `Remove()`, or turn **Map markers** off |
| `Cure/RZ_AntidoteConsumable.c:8`, `:15` | the consumable effect class and `ApplyEffect` signature | Delete the file: the world action still works |
| `Guidance/RZ_Guidance.c:172` | `System.GetUnixTime()` | Return 0: the 30-day refresher never fires |
| `Puzzles/RZ_BlinkingLight.c:55`, `:72`; `Puzzles/RZ_Landmarks.c:65`; `Core/RZ_Util.c:347` | `ToUpper`, `Trim`, `ToLower` in place or as copies | Swap to the other form |
| `Puzzles/RZ_BlinkingLight.c:178` | `SetFlags/ClearFlags(EntityFlags.VISIBLE)` | Remove `RZ_BlinkLightComponent` from the lamp: it's spawned per flash |
| `Core/RZ_Util.c:340`, `:343` | `GetChildren()`/`GetSibling()`, attachment prefab names | Suppressors aren't detected (full gunfire heat) |
| `Core/RZ_Util.c:203` | `IsInCompartment()` | Driving or stealing a supply truck isn't recognised; interaction scans don't skip players in vehicles |
| `Core/RZ_Util.c:267`, `Puzzles/RZ_Noisemaker.c:130`, `Interactions/RZ_Traps.c:245`, `:412` | `GetRootParent()`: carried items are children of the character | Thrown noisemakers, molotovs and picked-up kits aren't noticed; a picked-up clue could be cleaned up |
| `Interactions/RZ_Traps.c:605` | `IEntity.GetAngles()` | Barricades face the wrong way |
| `Interactions/RZ_Traps.c:666` | returning false stops a query early | The callback runs to the end; nothing breaks |
| `Puzzles/Missions/RZ_PuzzleMechanism.c:408`, `:425` | `GetAll()`, `GetTotalWeight()` | Turn on **Scale counts items** |
| `Puzzles/RZ_RadioStealth.c:42` | `BaseRadioComponent` class name | Radios are found by name ("radio") only |
| `Missions/RZ_MissionBases.c:244` | damage manager `GetState()` on a target prop | Destroy missions (Arson, Sabotage, Burn the Nest) never finish: check the prop's damage manager |

## 7. Log lines to check

Every line starts with `[ReforgerZ]`. On start you should see, in roughly this order:

```
[ReforgerZ] Settlement scan: N named, M building clusters, took X ms.
[ReforgerZ] Perimeter: x A to B, z C to D. Warning zone 300 m, kill zone 100 m.
[ReforgerZ] Perimeter: N road checkpoints (from roads).
[ReforgerZ] Haven safehouses: N (picked).
[ReforgerZ] Wanderers: N on the roads (new roster, seed S).
[ReforgerZ] Faction leaders: N (new, seed S). Haven 2, Bastion 2, Red Hand 2, Halcyon 2
[ReforgerZ] Territory: 8 holdings in 14 settlements (new, seed S). Haven 1+1, Bastion 1+1, ...
[ReforgerZ] Supply trucks: Haven, Bastion.
[ReforgerZ] Faction life: morale 60/60/60/60, 0 remembered moments.
[ReforgerZ] Power grid: the power station outside Morton at grid 047-112 (picked), 3 towns in its region, dark.
[ReforgerZ] Radio station: the radio mast near Levie at grid 052-118 (picked).
[ReforgerZ] Story arcs: on, running none.
[ReforgerZ] Search: N items in the loot pool, M tables set by hand.
[ReforgerZ] World interactions: on. N fixable cars on the map.
[ReforgerZ] Hostility: bitmask mode, all 15 variant factions found.
[ReforgerZ] Started. N settlements, stage Early Outbreak, cure 0 percent, state ...
```

Warnings worth acting on:

| Line | Means |
|---|---|
| `Using N hand-placed settlement markers.` | Detection is off because markers are placed |
| `No checkpoint barricade prefab set: road checkpoints are off.` | Set Perimeter > Barricade prefab |
| `No road network found: place RZ_CheckpointMarkerEntity entities ...` | The road VERIFYs failed or the map has no roads |
| `Hostility: bitmask mode, only N of 15 variant factions ...` | Some `SURV_H` factions are missing |
| `Hostility: outlaw mode, but the FactionManager has no SURV_OUTLAW.` | Add the Outlaw faction |
| `No background matches loadout '...'` | A loadout name doesn't match a background |
| `No lab prefab set. Samples can't be delivered ...` | Set Cure > Lab prefab |
| `Puzzle clue prefab has no RZ_ClueComponent ...` / `Puzzle object prefab has no RZ_PuzzleObjectComponent ...` | A puzzle prefab is missing its component |
| `Mission item prefab has no RZ_MissionItemComponent ...` | The mission item is missing its component |
| `Supply trucks are off. ...` / `Territory is off. ...` | Expected only if you turned them off |
| `Saved state is from another map, starting a fresh outbreak.` | Expected after changing maps |
| `Save file is corrupt or from an incompatible version, ignoring.` | The save didn't load (or the JSON VERIFY failed) |

While running: `Status | ...` lines every few minutes (stage, infection, zombies, missions, cure),
`Mission N '...' at ...`, `Outbreak stage -> ...`, `Supply: ... truck leaves ...` and `... lost (...)`,
`Story arc started: ...`, and `Season over. Cure goes to ...`.

## 8. Play-test checklist

What to test first and why is in [ROADMAP.md](ROADMAP.md), "Play-test priorities". On a
session, tick off:

**Start**
- [ ] It compiles; every log line in section 7 appears with sensible numbers.
- [ ] Each background spawns with its loadout and its perk and flaw (try Nurse and Ex-Con).
- [ ] The government broadcast and the session news appear on first spawn.

**Core loop**
- [ ] Infected spawn near you in towns and despawn far away; a shot makes heat, enough heat a horde.
- [ ] A bite starts the infection timer; an antidote cures it; turning starts the Turned Player.
- [ ] A special drops a sample; delivering it at the lab moves research.
- [ ] A contract is offered by radio, completes, and pays Fame and Favor; a tier change is announced.
- [ ] A quartermaster sells an item at the right tier and refuses one above it.
- [ ] Stop and restart the server: stage, infection, research and your standing are back.

**Factions**
- [ ] Hunted by Bastion near Bastion AI: they shoot you, not a friend next to you.
- [ ] Your deploy menu, chat and group stay on the Haven while you're on a variant.
- [ ] An exile with a gun near a safehouse is shot; without one, the door is just shut.
- [ ] Shooting a guard makes nearby guards of that faction fire back for about 3 minutes.
- [ ] A supply truck leaves, is driven by its crew, and arrives.
- [ ] A leader answers Talk, Work, News and Standing.

**World**
- [ ] Walking near the map edge: drone and loudspeaker at 300 m, warning shot at 100 m.
- [ ] A safehouse shows occupants; you can hand in food and declare a bite.
- [ ] A wanderer can be talked to and remembers you.
- [ ] "Search" spots appear in a house; a search finds something from the loot pool.
- [ ] A thrown bottle draws nearby infected; a puzzle mission can be finished.
- [ ] Night: missions that need the dark are offered; the log says "night".
