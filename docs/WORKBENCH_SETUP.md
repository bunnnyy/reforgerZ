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
They're isolated in small helpers, mostly in `Core/RZ_Util.c`. There are 73 of them
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

The riskiest ones in the Perimeter (`World/RZ_Perimeter.c`):

- **Map edge** (`BaseWorld.GetBoundBox()`): if the logged rectangle is wrong, turn on
  **Override** in the Perimeter settings and type in the corners.
- **Road network** (`SCR_AIWorld.GetRoadNetworkManager()`, `RoadNetworkManager.GetRoadsInAABB()`,
  `BaseRoad.GetPoints()`/`GetWidth()`): finds where roads reach the edge. If these don't compile,
  empty the body of `FindRoadExits()` and place `RZ_CheckpointMarkerEntity` entities instead.
- **Drone attachment** (`IEntity.AddChild()`): the drone is attached to the player so it follows
  them. If that fails it's moved over them on each tick instead (`SetOrigin()`, jumpier).
- **Untouchable guards** (`EnableDamageHandling(false)`): guards stand outside the playable area
  anyway; if this fails they can be shot from inside, so use a prop or a character without AI.

The riskiest ones in the safehouses and wanderers (`World/RZ_Safehouses.c`, `World/RZ_Wanderers.c`):

- **Food in the inventory** (`RZ_FoodHelper.TakeFood`: `GetItems()` and `TryDeleteItem()`, the same
  calls the radio check and the antidote use). If food isn't found, check the food keywords
  against your food prefabs' names first.
- **Healing** (`RZ_Util.HealCharacter`: `SCR_CharacterDamageManagerComponent.FullHeal()`), used by the
  safehouse medic and the Doctor wanderer. If it doesn't exist, the Favor is still taken, so fix it before release.
- Not a new VERIFY, but worth testing: wanderer and occupant actions reuse the puzzle civilian's
  replicated labels (`[RplProp]`, above). Without them every slot shows with its generic label,
  and "Choose" options 0, 1 and 3 still do the right thing.

The riskiest ones for faction leaders (`Factions/RZ_Leaders.c`): none are new, but three
existing calls matter more now:

- **Untouchable stronghold leaders** (`RZ_Util.MakeUntouchable`: `EnableDamageHandling(false)`,
  moved there from the Perimeter, which now calls it). If it fails, a stronghold leader's body can be
  shot: they're "wounded" and back later (they never die), but test it.
- **Labels on a tagged quartermaster or leader body** reuse the puzzle object's `[RplProp]` labels
  (above). The `RZ_LeaderAction`s on the quartermaster don't depend on them, so add those.
- **Drops told apart by entity**: leader jobs with two drops of the same faction (Our Own First,
  The Real Orders, The Notebook, the stronghold "turn it in" drop) match the drop entity the
  `RZ_ChoiceDeliveryAction` was used on. That's plain script, but play-test it.

The riskiest ones for per-player AI hostility (`Factions/RZ_FactionRelations.c`,
`Factions/RZ_Hostility.c`, the faction helpers in `Core/RZ_Util.c`). These decide whether AI
really shoots individual players, so test them first ([REPUTATION.md](REPUTATION.md), "What makes AI shoot you"):

- **The faction check** (`modded SCR_Faction`, `DoCheckIfFactionFriendly(Faction)`): the
  biggest unknown. It has to be the check AI perception and target selection use, and the
  answer mustn't be cached for long. If the override doesn't compile (other name, `protected`),
  fix the signature. Test: make a player Hunted by Bastion, stand near a Bastion AI group, see
  if it fires. If it never does, set **Hostility > Mode** to *Outlaw* and give `SURV_OUTLAW`
  real enemies in the FactionManager (section 4), which only uses vanilla relations.
- **Moving the character** (`RZ_Util.SetCharacterFaction`: `FactionAffiliationComponent.SetAffiliatedFaction`,
  `GetAffiliatedFaction`): server side, and it must replicate and be read by AI. The player
  controller's faction is a separate component (`SCR_PlayerFactionAffiliationComponent`), which is
  what the deploy menu, chat channels, groups and map markers read as far as I know, so they
  should stay on `SURV`; check each once. Also check that nothing resets the character to
  `SURV` (the tick puts it back within 5 s if something does), and what name tags, vehicle
  locks and team-kill penalties do with a character on a variant.
- **Armed check** (`RZ_Util.IsArmed`: `BaseWeaponManagerComponent.GetWeaponsList`,
  `BaseWeaponComponent.GetWeaponType`, the `EWeaponType` names): if it's always false, armed
  exiles are never shot and wanderers never fight back; if always true, unarmed exiles are shot.

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
| `RZ_Trader.et` | a character or vehicle, e.g. with a Shop System trader | | Missions > Trader | Wandering Trader |
| `RZ_Explosion.et` | a vanilla mortar or artillery impact effect | | Missions > Explosion | Bastion Shelling |

### Quartermasters (one per faction)

Each is a character or a table with a sign, plus actions. Place one per faction in the world,
or set the four prefab slots under **Factions** and they spawn by themselves: the Haven's in the
safest town, Bastion's in a bigger town, the Red Hand's in a rougher town, Halcyon's next to the
lab. Their positions are saved.

| Prefab | Add | Slot |
|---|---|---|
| `RZ_QM_Haven.et` | `RZ_QuartermasterComponent` (faction SURVIVORS); actions: `RZ_StandingStatusAction`, several `RZ_FactionShopAction` | Factions > Haven quartermaster |
| `RZ_QM_Bastion.et` | same with faction MILITARY, plus one `RZ_RequisitionAction` per requisition item (ammo, rations) | Factions > Bastion quartermaster |
| `RZ_QM_RedHand.et` | same with faction BANDITS | Factions > Red Hand quartermaster |
| `RZ_QM_Halcyon.et` | same with faction INSTITUTE | Factions > Halcyon quartermaster |

Each `RZ_FactionShopAction` sells one thing. Set:

- **Faction**: the quartermaster's faction
- **Required tier**: Stranger, Known, Trusted or Champion (only these four make sense)
- **Favor cost**
- **Item prefab**: an item, a crate or even a vehicle; it spawns next to the quartermaster
- **Display name**: shown as "Buy <name> (<cost> Favor, <tier>)"

The action is always shown. The server checks tier and Favor when it's used and tells the
player why if they can't buy it.

`RZ_RequisitionAction` (Bastion's Requisition perk) has the same **Favor cost**, **Item prefab**
and **Display name** attributes. The cost is the full price: a Trusted player pays half (rounded
up), once an hour across all requisition actions. Set it up for ammo and rations only. `RZ_StandingStatusAction` shows the player's standing
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
| `RZ_BastionHunters.et` | an AI group on faction `MIL` | Factions > Bastion hunter squad |
| `RZ_RedHandHunters.et` | an AI group on faction `BAND` | Factions > Red Hand hunter squad (falls back to the Red Hand group) |

Hunter squads spawn 400 m from a Hunted player and get a move waypoint (the zombie
move waypoint prefab) to where the player was. They're hostile to that player within 800 m.
Set **Hunter squad minutes** to 0 to turn them off. **Bounty minutes** sets how often
Kill-on-Sight bounties are announced.

**Hostility** (one settings object under **Factions**; defaults in brackets) decides which AI
shoots which player ([REPUTATION.md](REPUTATION.md), "What makes AI shoot you"). It needs the
hidden factions from section 4.

- **Mode** (Bitmask): *Bitmask* uses `SURV_H1`..`SURV_H15`; *Outlaw* uses one `SURV_OUTLAW` for
  anyone any faction would shoot (fallback if AI ignores the modded faction check); *Off* = vanilla.
- **Haven / Bastion / Red Hand / Halcyon shoots from** (Hunted for all four): Hunted, Watched,
  Strangers (anyone below Known) or Never.
- **Exile: armed only** (on) and **Exile radius** (100 m): the Haven only shoots a player it
  wants dead while they carry a firearm this close to a safehouse. Off = everywhere.
- **Fight radius** (350 m): players this close to a mission's Red Hand group are hostile to it.
- **Wanderers fight back** (on) and **Grudge radius** (80 m).
- **Bastion / Red Hand / Halcyon key** (MIL, BAND, INST): their FactionManager keys. The Haven's
  is the game mode's **Player faction key** (SURV).

The log says `Hostility: bitmask mode, all 15 variant factions found.`, or how many are missing.

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
| `RZ_Civilian.et` | a civilian character (no AI needed) or a seated figure prop | as the generic object | Puzzles > Civilian (*generic*) | The Rat (boss and suspects), The Witness; Walk Him Out, The Ledger, the lookouts in A Quiet Arrangement; fallback for safehouse occupants and wanderers |
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

### The Perimeter

The edge of the map is the quarantine line ([LORE.md](LORE.md), "The Perimeter"). It works
with no prefabs at all (loudspeaker messages, warning shot as a message, the unseen shooter),
and each prefab below only adds to it. Settings are on `RZ_GameModeComponent` under
**Perimeter** (one settings object). Nothing is built along the edge except the checkpoints.

| Prefab | Base | Add | Setting | Used for |
|---|---|---|---|---|
| `RZ_Drone.et` | a small quadcopter mesh with a hum sound and a light; **no collision or physics** | `RplComponent` | Drone prefab | hovers over a player in the warning zone, one per player |
| `RZ_WarningShot.et` | a gunshot sound or bullet impact effect | | Warning shot prefab | the warning shot (deleted after 20 s) |
| `RZ_Checkpoint.et` | a small composition: concrete blocks and a closed barrier across a road | | Barricade prefab | **required for checkpoints**; they're off without it |
| `RZ_CheckpointGuard.et` | a soldier character without AI (or a figure prop) | | Guard prefab | 0-4 per checkpoint, outside the playable area, damage turned off |
| `RZ_Smuggler.et` | a civilian character without AI, or a figure prop | | Smuggler prefab (falls back to Puzzles > Civilian) | The Blind Spot |
| `RZ_Package.et` | a small sealed box item | `RZ_MissionItemComponent` | Package prefab (falls back to Missions > Mission item) | The Blind Spot, The Crossing |
| `RZ_LineBody.et` | a body or body bag prop | `RZ_ClueComponent`, `RZ_ReadClueAction` | Line body prefab (falls back to Puzzles > Body, then the note) | Shot at the Line |

The Blind Spot also needs the **Red Hand drop point**, and The Crossing the **Haven drop point**.

**Settings** (defaults in brackets): **Enabled** (on); **Warn distance** (300 m) and
**Kill distance** (100 m) from the edge; **Grace seconds** (8) between the warning shot and the
fatal one, rounded up to the server tick; **Fit to land** (on) and **Sea margin** (200 m): the
edge is pulled in to the outermost dry land plus this margin, so on island maps the line sits
offshore and only the outer edge counts (raise the margin above the warning distance to keep
every beach quiet; The Crossing then only uses land edges); **Override** with **Override min/max**
(x and z of two corners) if the detected rectangle is wrong; **Drone height** (25 m);
**Checkpoints** (on), **Guards per checkpoint** (2), **Max checkpoints** (6, at most 8),
**Checkpoint inset** (60 m inside the edge), **Checkpoint spacing** (800 m),
**Checkpoint warn radius** (150 m) and **Checkpoint kill radius** (60 m).

**Checkpoints** are spawned once at start where roads cross the checkpoint line, widest roads
first. Island maps often have no road reaching the edge, so no checkpoints. To choose them
yourself, place `RZ_CheckpointMarkerEntity` entities (a script entity, no prefab needed) on the
roads, inside the warning zone; placing any turns road detection off. The log says how many
were built and from what.

Missions, spawn points and quartermasters keep out of the warning zone and away from checkpoints.

### Haven safehouses

Settings are on `RZ_GameModeComponent` under **Haven** (one settings object). Safehouses work
with no prefabs of their own: they need the **Spawn point** prefab for their spawn points and
the **Haven quartermaster** prefab for their services. Everything else only adds bodies.

| Prefab | Base | Add | Setting | Used for |
|---|---|---|---|---|
| `RZ_QM_Haven.et` (the existing one) | | also 5 × `RZ_SafehouseAction`: STATUS, DELIVER_FOOD, RADIO, HEAL, DECLARE_BITE | Factions > Haven quartermaster | every safehouse's services. One is spawned at each safehouse while a player is near (the first safehouse sits at the main one) |
| `RZ_SafehouseMedic.et` | a civilian character without AI | 2 × `RZ_SafehouseAction`: HEAL, DECLARE_BITE | Medic prefab | optional; Settlement and up. Without it the quartermaster's HEAL/DECLARE_BITE still work |
| `RZ_SafehouseOccupant*.et` | civilian characters without AI or seated figure props; give them the generic puzzle object components to be talked to | | Occupant prefabs (a list; empty = Puzzles > Civilian) | 3-6 people shown while a player is within 300 m |
| `RZ_SafehouseGuard.et` | an armed Haven AI character or group on faction `SURV` | | Guard prefab | 1 guard at Community and Settlement, 2 at Town; they shoot armed exiles |

`RZ_SafehouseAction` has one attribute, **Kind**. It works at whichever safehouse it stands in
(within 150 m) and says there's none elsewhere.

**Settings** (defaults in brackets): **Enabled** (on); **Count** (3, at most one per 8
settlements); **Start population** (4) and **Start food** (12); **Visible radius** (300 m);
**Upkeep minutes** (180, real time with players online); **People per food** (5); **Food keywords**
(empty = food, canned, ration, tushonka, bread, beans: lower-case words matched against item prefab
names; set your food mod's words here); **Favor per food** (2); **Heal cost** (15); **No-fire radius** (50 m),
**Defend radius** (40 m), **Fire Infamy** (3); **Caught Infamy** (15), **Caught chance** (0.3 a
minute); **Leak chance** (0.25 every 10 min while a safehouse is short of food).

**Placing safehouses yourself:** put `RZ_SafehouseMarkerEntity` entities (a script entity, no prefab
needed) where the Haven should be, with an optional name, starting population and food. Placing any
turns the automatic picking off; saved population and food are kept for a marker within 100 m of
a saved safehouse. Without markers, safehouses are picked once per season and saved. The log says
`Haven safehouses: N (picked | save | markers).`

### Wanderers

Settings are under **Wanderers**. Wanderers need no prefabs: without them they use
**Puzzles > Civilian** (and that falls back to the generic puzzle object). A body can only be
talked to if it has the generic puzzle object components (`RZ_PuzzleObjectComponent`,
`RZ_ClueComponent`, `RZ_ReadClueAction`, 4 × `RZ_PuzzleChoiceAction`); the wanderer labels them.

| Prefab | Base | Setting |
|---|---|---|
| `RZ_Wanderer_Trader.et`, `_Doctor`, `_Scavenger`, `_Deserter`, `_Preacher`, `_Courier`, `_Kid` | a civilian character without AI (stands still while seen), or a one-man AI group on `SURV` whose character has the components above (walks while seen). Give an AI group's character a gun if it should fight back (the Deserter at least) | one slot per role |
| Trader goods | any items | **Trader goods** (a list); empty = the Trader only talks |

**Settings** (defaults in brackets): **Enabled** (on); **Min count** (6) and **Max count** (10);
**Trade cost** (15), **Rumour cost** (10), **Treat cost** (20) in Haven Favor; **Spawn radius**
(400 m); **Walk speed** (1.3 m/s); **Replace minutes** (1440); **Bite chance** (0.05 per 30 s
with an infected within 12 m); **Kill Infamy** (15), **Shot Infamy** (4), **Rob Infamy** (8).
The log says `Wanderers: N on the roads (new roster, seed S)` or `(saved)`.

The Courier's job needs **Missions > Mission item** and **Missions > Haven drop point**; A Quiet
Arrangement needs the mission item and the **Red Hand drop point**.

### Faction leaders

Settings are under **Leaders**. Leaders need no prefabs of their own: at a stronghold or a
safehouse the quartermaster is the leader, and outpost leaders use **Puzzles > Civilian**.

| Prefab or action | Base | Add | Setting | Used for |
|---|---|---|---|---|
| `RZ_LeaderAction` on every quartermaster prefab (the existing four) | | 4 × `RZ_LeaderAction`: TALK, WORK, NEWS, STANDING | | talking to the leader whose seat this is (within 60 m). **Add these**: they're how players reach stronghold leaders and council heads |
| `RZ_Leader_Haven.et`, `_Bastion`, `_RedHand`, `_Halcyon` | a character without AI (or a figure prop) | the generic puzzle object components (to be talked to), or the `RZ_LeaderAction`s | Haven / Bastion / Red Hand / Halcyon leader prefab | optional: a body for the stronghold leader next to the quartermaster. Damage is turned off |
| `RZ_CouncilHead.et` | a civilian character without AI | as above | Council head prefab | optional: a body for each safehouse's council head |
| `RZ_Outpost_Bastion.et`, `_RedHand`, `_Halcyon` | a character without AI | as above | Bastion / Red Hand / Halcyon outpost prefab (fall back to the faction leader, then the puzzle civilian) | optional: outpost leaders |

**Settings** (defaults in brackets): **Enabled** (on); **Quartermaster is leader** (on; off = a puzzle
civilian stands in at strongholds and safehouses without a leader prefab); **Auto outposts** (1:
interim leader posts per faction for Bastion and the Red Hand, Halcyon at most 1); **Body radius**
(200 m); **Replace minutes** (120); **Job minutes** (30, between one leader's personal jobs);
**Danger scale** (1; 0 = outpost and safehouse leaders only die if killed); **Discovery Infamy** (10),
**Discovery chance** (0.15), **Seen chance** (+0.35); **Kill Infamy** (15).

**Placing outposts yourself:** put `RZ_OutpostMarkerEntity` entities (a script entity, no prefab
needed) with a faction and an optional place name. Placing any for a faction turns its interim
posts off. Leader jobs use the mission item, the drop points and the puzzle civilian, generic
object and well; each job is skipped while what it needs is missing. Training Day uses
**Missions > Survivor group** if set. The log says
`Faction leaders: N (new, seed S). Haven 2, Bastion 2, Red Hand 2, Halcyon 2` or `(saved)`.

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

**Relations.** AI shoots individual players through the hidden factions below, so the base
relations are the "nobody wants you dead" case:

| | SURV | MIL | BAND | INST |
|---|---|---|---|---|
| SURV | friendly | friendly | friendly | friendly |
| MIL | friendly | | hostile | friendly |
| BAND | friendly | hostile | | hostile |
| INST | friendly | friendly | hostile | |

Set them in each faction's friendly list (both ways). `SURV`–`BAND` friendly is what makes the
Red Hand neutral by default; its mission groups still fight the players near them.

**Hidden player factions (15, plus 1 optional).** In the `FactionManager`'s faction list:

1. Duplicate the `SURV` entry. Change only its **Faction key** to `SURV_H1`.
2. Keep its name, colour, flag and identity the same, so anything that shows it looks like the
   Haven. Turn **Is Playable** off, so it never appears in the deploy menu's faction list,
   and empty its loadout list if it has one (spawning always uses `SURV`).
3. Repeat for `SURV_H2` up to `SURV_H15` (the number is the bitmask: 1 Haven, 2 Bastion,
   4 Red Hand, 8 Halcyon, added up).
4. Their friendly lists don't matter: the modded faction check decides their relations from
   the key. Copying `SURV`'s is fine.
5. Optional, for **Hostility > Mode** *Outlaw*: one more copy keyed `SURV_OUTLAW`. Since the
   Outlaw mode is the fallback for when the modded check doesn't reach the AI, give it real
   relations: friendly with nothing (or only itself), and remove it from the friendly lists of
   `SURV`, `MIL`, `BAND` and `INST`.

If the player faction key isn't `SURV`, the variants are `<key>_H1`..`<key>_H15` and `<key>_OUTLAW`.
Missing variants only switch the feature off for the players who'd need them (logged once).

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
[ReforgerZ] Perimeter: x A to B, z C to D. Warning zone 300 m, kill zone 100 m.
[ReforgerZ] Perimeter: N road checkpoints (from roads).
[ReforgerZ] Haven safehouses: N (picked).
[ReforgerZ] Wanderers: N on the roads (new roster, seed S).
[ReforgerZ] Faction leaders: N (new, seed S). Haven 2, Bastion 2, Red Hand 2, Halcyon 2
[ReforgerZ] Hostility: bitmask mode, all 15 variant factions found.
[ReforgerZ] Started. N settlements, stage Early Outbreak, cure 0 percent ...
```

For another map, make another sub-scene with the same contents.

## 7. Server config

- **Broadcast minutes** (Director, default 25): how often everyone hears the government
  broadcast ([LORE.md](LORE.md)). Every player also hears it on their first spawn. 0 = only then.
- Keep time acceleration modest and fixed. Don't use time-skip or sleep mods.
- Season wipe: set `m_bWipeOnStart` for one restart, or delete
  `$profile:ReforgerZ/state.json`. The last season's winner is kept in
  `$profile:ReforgerZ/last_season.json`, which a wipe doesn't touch.
- Old saves load: profiles from before the reputation rework start with no Fame or Infamy.
