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
They're isolated in small helpers, mostly in `Core/RZ_Util.c`.

## 3. Prefabs to make

| Prefab | Base | Add |
|---|---|---|
| `RZ_GameMode.et` | vanilla `GameMode_Plain` (or any `SCR_BaseGameMode` prefab) | `RZ_GameModeComponent`; fill in its prefab slots |
| `RZ_SpawnPoint.et` | vanilla `SpawnPoint` prefab | none |
| `RZ_Lab.et` | any building or composition (field hospital tent, lab container) | `RZ_LabComponent`; in `ActionsManagerComponent` add `RZ_DepositSamplesAction` and `RZ_LabStatusAction` |
| `RZ_Sample.et` | a small vanilla item (e.g. a medical vial or bandage) | `RZ_SampleItemComponent` |
| `RZ_Antidote.et` | morphine or saline item | (phase 2: a use action that calls `CurePlayer`) |
| `RZ_RewardCrate.et` | vanilla supply box | Fill its storage with loot |

## 4. Factions and backgrounds

In the world's `FactionManager`:

- `SURV`: the player faction. Its **loadout list** has one loadout per background,
  with names matching exactly: `Nurse`, `Mechanic`, `Hunter`, `Police Officer`,
  `Ex-Con`, `Lab Technician`, `Radio Operator`, `Unemployed`.
  Each loadout is a civilian character prefab with that background's starting gear.
- `MIL`, `BAND`, `INST`: AI factions for later phases.
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
  `$profile:ReforgerZ/state.json`.
