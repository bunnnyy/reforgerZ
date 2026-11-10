# Workbench setup

The scripts in this repo are plain Enforce Script. Prefabs, configs and the
`.gproj` are created in Workbench, because they need GUIDs that Workbench generates.

## 1. Create the addon

1. Subscribe to the dependencies in the game ([MOD_STACK.md](MOD_STACK.md)), at minimum **Bacon Zombies**.
2. Arma Reforger Tools → **Create new project** named `ReforgerZ`.
3. Put this repo's contents in the project's folder, so `Scripts/` sits next to `addon.gproj`
   (or clone the repo there first and point the new project at that folder).
4. Project Settings → Dependencies: add Arma Reforger and Bacon Zombies, plus whatever else you use.
5. Open the Script Editor and compile (**F7**). Commit `addon.gproj` once it exists.

## 2. Fix the `VERIFY` calls

```bash
grep -rn "VERIFY" Scripts/
```

Each one is an engine API whose exact name or signature I couldn't confirm
for the current game version. Usually the fix is a rename. Look up the class in
the Script Editor or in the [Script API docs](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/).
They're isolated in small helpers, mostly in `Core/RZ_Util.c`. There are 56 of them
(one is the comment in `RZ_Util.c` that explains them).

The riskiest ones in the puzzle missions (`Puzzles/`):

- **`[RplProp]` on a `ScriptComponent`** (`RZ_ClueComponent`, `RZ_PuzzleObjectComponent`,
  `RZ_BlinkLightComponent`): replicated action labels, keypad display and Morse light state.
  The prefab needs an `RplComponent`. If labels don't replicate, the actions fall back to
  the labels set on the prefab and all slots stay visible, so puzzles still work, just with
  generic labels ("Use", "Choose").
- **Time of day** (`RZ_GameTime.GetHour()`: `ChimeraWorld.GetTimeAndWeatherManager().GetTimeOfTheDay()`).
  If it returns -1, the three night/clock missions never appear.
- **Landmark names** (`RZ_Util.GetPrefabName()`): the prefab name, else the mesh name, of map
  buildings. If neither works, puzzles use the fallback landmark prop.
- **Scale weight** (`RZ_ScaleHelper`: `BaseInventoryStorageComponent.GetAll()` and
  `InventoryItemComponent.GetTotalWeight()`). If the weight is wrong, turn on
  **Scale counts items** on the game mode.
- **Horde redirect** (`RZ_Util.RedirectGroup()`: `GetWaypoints`/`RemoveWaypoint`) for The Clockwork Horde.
- **Light visibility** (`SetFlags`/`ClearFlags(EntityFlags.VISIBLE)`): if it doesn't hide the
  lamp, remove `RZ_BlinkLightComponent` from the lamp prefab; the light is then spawned and
  deleted for each flash instead (slower Morse).

Two files can simply be deleted if they won't compile, because nothing else depends on them:

- `Cure/RZ_AntidoteConsumable.c` (antidote used from the inventory; the world action still works)
- the bodies of `Add()` and `Remove()` in `Core/RZ_MapMarkers.c` (map markers), or turn
  markers off with **Map markers** on the game mode component

## 3. Prefabs to make

Set every prefab you make in the matching slot on `RZ_GameModeComponent`. A mission or
feature whose prefab slot is empty is simply switched off, so you can start with the
required ones and add the rest over time.

### Required

| Prefab | Base | Add | Slot |
|---|---|---|---|
| `RZ_GameMode.et` | vanilla `GameMode_Plain` (or any `SCR_BaseGameMode` prefab) | `RZ_GameModeComponent`; fill in its prefab slots | |
| `RZ_SpawnPoint.et` | vanilla `SpawnPoint` prefab | none | World > Spawn point prefab |
| `RZ_Lab.et` | any building or composition (field hospital tent, lab container) | `RZ_LabComponent`; in `ActionsManagerComponent` add `RZ_DepositSamplesAction` and `RZ_LabStatusAction` | Cure > Lab prefab |
| `RZ_Sample.et` | a small vanilla item (e.g. a medical vial or bandage) | `RZ_SampleItemComponent` | Cure > Sample |
| `RZ_Antidote.et` | morphine or saline item | `RZ_AntidoteItemComponent`; `RZ_InjectAntidoteAction` in its `ActionsManagerComponent`; optionally set its consumable effect to `RZ_ConsumableAntidote` | Cure > Antidote |
| `RZ_RewardCrate.et` | vanilla supply box | Fill its storage with loot | Missions > Reward crate |

### Mission items and drop points (delivery and choice missions)

| Prefab | Base | Add | Slot |
|---|---|---|---|
| `RZ_MissionItem.et` | a small carryable item (a radio, a case, a hard drive) | `RZ_MissionItemComponent` | Missions > Mission item |
| `RZ_Drop_Haven.et` | a crate, table or mailbox | `RZ_ChoiceDeliveryAction` in its `ActionsManagerComponent`, faction **SURVIVORS** | Missions > Haven drop point |
| `RZ_Drop_Bastion.et` | same, inherit from the one above | same action, faction **MILITARY** | Missions > Bastion drop point |
| `RZ_Drop_RedHand.et` | same | faction **BANDITS** | Missions > Red Hand drop point |
| `RZ_Drop_Halcyon.et` | same | faction **INSTITUTE** | Missions > Halcyon drop point |

One mission item prefab is used for everything (medicine, notes, black box, data drive,
package); the mission tags it with its id when it spawns. Missions spawn their own drop
points and remove them afterwards. You can also put `RZ_ChoiceDeliveryAction` on a
quartermaster prefab so it accepts hand-ins too.

Enables: Medicine Run, Missing Scout (Haven drop); Fence the Goods, Smuggler's Run (Red Hand drop);
Crash Site (Bastion + Red Hand drops, plus the wreck); Requisition (Bastion + Haven);
Research Data (Halcyon + Bastion).

### Optional mission prefabs

| Prefab | Base | Needs | Slot | Enables |
|---|---|---|---|---|
| `RZ_SurvivorGroup.et` | civilian AI group on faction `SURV` | | Missions > Survivor group | real survivors in Survivors Trapped; The Bitten Friend; Trader Caravan |
| `RZ_Nest.et` | Zombie Hive prefab, or any destructible prop | a damage manager | Missions > Nest | Burn the Nest |
| `RZ_BanditDrop.et` | crate, barrel or car wreck | `RZ_SellSamplesToBanditsAction` | Missions > Dead drop | Dead Drop |
| `RZ_HeliWreck.et` | a helicopter wreck prop | | Missions > Helicopter wreck | Crash Site, Crashed Helicopter |
| `RZ_CorpsePile.et` | a body bag pile or debris prop | | Missions > Corpse pile | visual only for Burn the Bodies and Decontamination |
| `RZ_RedHandGroup.et` | an AI group on faction `BAND` (e.g. FIA or guerrilla units) | | Missions > Red Hand group | Raid the Red Hand, Trader Caravan, raiders in Lab Defence; default Red Hand hunter squad |
| `RZ_FuelDepot.et` | fuel tanks or a fuel truck | a damage manager | Missions > Fuel depot | Arson |
| `RZ_Relay.et` | a radio mast or antenna | a damage manager | Missions > Relay | Sabotage the Relay |
| `RZ_Airdrop.et` | a supply crate with a parachute or smoke | | Missions > Airdrop | nicer Airdrop (the reward crate is used otherwise) |
| `RZ_Trader.et` | a character or vehicle, e.g. with a Shop System trader | | Missions > Trader | Wandering Trader |
| `RZ_Explosion.et` | a vanilla explosion / artillery impact effect | | Missions > Explosion | Bombing Run |

### Quartermasters (one per faction)

Each is a character or a table with a sign, plus actions. Place one per faction in the world,
or set the four prefab slots under **Factions** and they spawn by themselves: the Haven's in the
safest town, Bastion's in a bigger town, the Red Hand's in a rougher town, Halcyon's next to the
lab. Their positions are saved.

| Prefab | Add | Slot |
|---|---|---|
| `RZ_QM_Haven.et` | `RZ_QuartermasterComponent` (faction SURVIVORS); actions: `RZ_StandingStatusAction`, several `RZ_FactionShopAction` | Factions > Haven quartermaster |
| `RZ_QM_Bastion.et` | same with faction MILITARY, plus `RZ_RequisitionAction` | Factions > Bastion quartermaster |
| `RZ_QM_RedHand.et` | same with faction BANDITS | Factions > Red Hand quartermaster |
| `RZ_QM_Halcyon.et` | same with faction INSTITUTE | Factions > Halcyon quartermaster |

Each `RZ_FactionShopAction` sells one thing. Set:

- **Faction**: the quartermaster's faction
- **Required tier**: Stranger, Known, Trusted or Champion (only these four make sense)
- **Favor cost**
- **Item prefab**: an item, a crate or even a vehicle; it spawns next to the quartermaster
- **Display name**: shown as "Buy <name> (<cost> Favor, <tier>)"

The action is always shown. The server checks tier and Favor when it's used and tells the
player why if they can't buy it. `RZ_StandingStatusAction` shows the player's standing
with every faction and the contracts this quartermaster has for them.

A starting shop list, from REPUTATION.md:

| Faction | Known | Trusted | Champion |
|---|---|---|---|
| Haven | canned food, water, bandages, crowbar | hunting rifle, ammo, backpack, radio, jerrycan | pickup, van, fortification kit, good medkit |
| Bastion | rations, morphine, flares | service rifle, body armour, helmet, NVGs | machine gun, grenades, military truck |
| Red Hand | knife, pistol | suppressor, scoped rifle, explosives | rare military gear, a stolen antidote |
| Halcyon | antiseptic, painkillers | antidote, stim kit, gas mask | (immunity booster: not in code yet) |

### Faction perks and consequences

| Prefab | Base | Slot |
|---|---|---|
| `RZ_RequisitionCrate.et` | a military supply box with loot | Factions > Requisition crate (the reward crate is used if empty) |
| `RZ_BastionHunters.et` | an AI group on faction `MIL` | Factions > Bastion hunter squad |
| `RZ_RedHandHunters.et` | an AI group on faction `BAND` | Factions > Red Hand hunter squad (falls back to the Red Hand group) |

Hunter squads spawn 400 m from a Hunted player and get a move waypoint (the zombie
move waypoint prefab) to where the player was. Set **Hunter squad minutes** to 0 to turn
them off. **Bounty minutes** sets how often Kill-on-Sight bounties are announced.

Characters already have an `EventHandlerManagerComponent`, and ReforgerZ listens to
its `OnProjectileShot` event for gunfire heat. Nothing to add. Suppressors are detected by
attachment prefab names containing "suppress" or "silenc"; **Suppressed heat mult** sets how
much quieter they are.

### Puzzle prefabs

These enable the 17 puzzle missions ([MISSIONS.md](MISSIONS.md), "Puzzle missions"). Slots are
under **Puzzles** on `RZ_GameModeComponent`. Every prefab with an action list needs an
`ActionsManagerComponent`, and every prefab with an `RZ_` component needs an `RplComponent`.

The **generic puzzle object** is the workhorse. Missions label its actions ("Seal the well",
"Accuse Mirek", "Flip switch B2") and hide the slots they don't use, so one prefab can be a
well, a bell, a fuse box, a radio mast, a safe, a person or a decision. The specific slots are
optional and only change the look: give them the same components and actions.

| Prefab | Base | Add | Slot | Needed by |
|---|---|---|---|---|
| `RZ_Note.et` | a paper or notebook prop | `RZ_ClueComponent`, `RZ_ReadClueAction` | Puzzles > Note | every puzzle except Hot and Cold (needs no prefab), The Rat, The Clockwork Horde and Time-Lock Safe |
| `RZ_PuzzleObject.et` | a crate, sign or post | `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`, `RZ_ReadClueAction`, 4 × `RZ_PuzzleUseAction` (values 0-3), 4 × `RZ_PuzzleChoiceAction` (options 0-3) | Puzzles > Puzzle object | most puzzles (fallback for the rows below marked *generic*) |
| `RZ_Keypad.et` | a keypad or wall panel | `RZ_PuzzleObjectComponent`, `RZ_ClueComponent`, `RZ_ReadClueAction`, 12 × `RZ_KeypadAction` (keys 0-9, 10 = Clear, 11 = Enter) | Puzzles > Keypad | The Last Experiment (keypad mode) |
| `RZ_Terminal.et` | a computer prop | same as the keypad | Puzzles > Terminal (falls back to the keypad) | The Last Experiment |
| `RZ_PuzzleBody.et` | a body or body bag prop | `RZ_ClueComponent`, `RZ_ReadClueAction` | Puzzles > Body (falls back to the note) | Paces, Patient Zero's Ledger, Poisoned Well, Last Experiment, The Witness |
| `RZ_Well.et` | a well | as the generic object | Puzzles > Well (*generic*) | The Poisoned Well |
| `RZ_Bell.et` | a bell on a frame | as the generic object | Puzzles > Bell (*generic*) | The Bell Order |
| `RZ_FuseBox.et` | an electrical box | as the generic object | Puzzles > Fuse box (*generic*) | Junction Boxes |
| `RZ_RadioMast.et` | an antenna mast | as the generic object | Puzzles > Radio mast (*generic*) | The Numbers Station |
| `RZ_Safe.et` | a safe | as the generic object | Puzzles > Safe (*generic*) | Time-Lock Safe |
| `RZ_Civilian.et` | a civilian character (no AI needed) or a seated figure prop | as the generic object | Puzzles > Civilian (*generic*) | The Rat (boss and suspects), The Witness |
| `RZ_ScaleCrate.et` | a crate with an inventory storage | as the generic object, plus the storage | Puzzles > Scale crate | The Weighing Vault |
| `RZ_Landmark.et` | a wayside cross, shrine or painted mast | none | Puzzles > Landmark, plus **Landmark name** (default "the old cross") | fallback when a town has no church, tower or well |
| `RZ_MorseLamp.et` | a lamp with a light | `RZ_BlinkLightComponent` (or nothing: the lamp is then spawned per flash) | Puzzles > Morse light | Lights in the Window |

Also used if set: **Missions > Helicopter wreck** (Two Bearings tags it as a clue if it has
`RZ_ClueComponent` + `RZ_ReadClueAction`; otherwise a note lies next to it) and
**Missions > Red Hand group** (the camp in The Witness). The Clockwork Horde needs the zombie
**Move waypoint** set.

**Scale counts items** (Puzzles): The Weighing Vault counts items instead of weighing them.

#### Noisemakers

Not a slot: make these items and put them in loot or shops. Each has
`RZ_NoisemakerComponent` (heat per burst, delay, number of bursts, interval, goes off when
thrown, deleted after) and, for timed ones, `RZ_ArmNoisemakerAction`. The server checks them on
its tick, so the noise comes up to one tick after the event.

| Prefab | Base | Settings |
|---|---|---|
| `RZ_Bottle.et` | a vanilla grenade, with the explosion removed and a bottle mesh, so it can be thrown | heat 20, on throw, deleted after |
| `RZ_Firecracker.et` | a small item | heat 15, delay 5 s, 4 bursts 3 s apart, arm action "Light the fuse", deleted after |
| `RZ_AlarmClock.et` | a clock prop item | heat 10, delay 30 s, 8 bursts 8 s apart, arm action "Wind the alarm", not on throw, kept |

A thrown noisemaker counts when it was carried at one tick and lies at least 4 m from where
the carrier stood at the next. Other mods (car alarms...) can call
`RZ_GameModeComponent.ReportNoise(pos, heat)`.

## 4. Factions and backgrounds

In the world's `FactionManager`:

- `SURV`: the player faction. Its **loadout list** has one loadout per background,
  with names matching exactly: `Nurse`, `Mechanic`, `Hunter`, `Police Officer`,
  `Ex-Con`, `Lab Technician`, `Radio Operator`, `Unemployed`, and the five unlockable ones:
  `Soldier`, `Doctor`, `Raider`, `Elder`, `Drifter`.
  Each loadout is a civilian character prefab with that background's starting gear.
  Players who pick a locked background keep its gear for that life but play as Unemployed
  (perks, flaws and standing) and are told how to unlock it. Hiding locked loadouts in the
  deploy menu needs a custom menu (roadmap).
- `MIL`, `BAND`, `INST`: AI factions for hunter squads, Red Hand groups and survivors.
  In game they're called Task Force Bastion, the Red Hand and Halcyon; `SURV` is the Haven.
- The Bacon Zombies faction is hostile to all of the above.

Players then pick their background in the vanilla deploy menu. That's the
"choose it in the spawn menu" part, with no new UI yet.

## 5. Zombie settings

On `RZ_GameModeComponent > Zombies`:

- **Common prefabs**: Bacon Zombies civilian and military **group** prefabs
- **Special prefabs**: Special Zombies or Bacon heavy, exploding and ranged variants
- **Horde prefabs**: a large Bacon group
- **Move waypoint**: vanilla `AIWaypoint_Move.et`

## 6. World

Create a new world as a **sub-scene** of Arland (start small), and place the
`RZ_GameMode` prefab plus a `FactionManager` and `LoadoutManager`. No spawn
points, towns or lab are needed, because they're generated at start. Check the log for:

```
[ReforgerZ] Settlement scan: N named, M building clusters, took X ms.
[ReforgerZ] Started. N settlements, stage Early Outbreak, cure 0 percent ...
```

For another map, make another sub-scene with the same contents.

## 7. Server config

- Keep time acceleration modest and fixed. Don't use time-skip or sleep mods.
- Season wipe: set `m_bWipeOnStart` for one restart, or delete
  `$profile:ReforgerZ/state.json`. The last season's winner is kept in
  `$profile:ReforgerZ/last_season.json`, which a wipe doesn't touch.
- Old saves load: profiles from before the reputation rework start with no Fame or Infamy.
