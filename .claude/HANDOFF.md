# Handoff board

The working list between the cloud session (code, tools, docs, manifest) and the PC session
(Workbench, prefabs, worlds, configs, engine tests). Each entry is one or two lines, starts with the
date and a short commit hash, and is **deleted** once it's done. Design goes in the docs, not here.
"WBS" = `docs/WORKBENCH_SETUP.md`.

## For the PC

**Minimum playable subset first (WBS §2)**
- 2026-11-11 76b52d0: play the minimum subset on `Missions/RZ_Arland.conf`. Check the log lines in WBS §7, report every script exception and missing log line here, and run the play-test checklist in WBS §8.

**UI (WBS §4, "Setting up the UI")**
- 2026-11-11 98c9c51: check the notebook, talk, shop, car and reader menus in play (step 4a); import the car picture and set **UI > Car picture** (4a.4).
- 2026-11-11 98c9c51: step 5, the deploy panel (`RZ_BackgroundPanel.layout`); step 6, the map (set **UI > Map icon layout** to `RZ_MapIcon.layout`). Then turn **UI > Board menus** on once the board's menu works.

**Prefabs missing from `Prefabs/` (run "ReforgerZ: Create prefabs", then finish by hand; WBS §4)**
- 2026-11-11 94217f5: `Zombies/RZ_CallSound.et`, `Zombies/RZ_GreatCallSound.et`.

**Manual entries (WBS §4)**
- 2026-11-11 55f071f: camp life. Make the smart-action waypoint `AI/RZ_WP_SmartAction.et`, the 49 compositions and the kits (WBS §4, "Camp life: the compositions and kits"). The 21 original pieces come first.
- 2026-11-11 76b52d0: optional lists: loot tables, extra recipes, shop items, Requisition items (WBS §4, "Hand-set list entries").
- 2026-11-11 76b52d0: replace the stand-in bases, worst first: drugs' consumable effects, `RZ_Explosion.et` (a real HE round), `RZ_Nest.et` (spawns its own), the carryable generator and alarm clock, the molotov, bottle and tin (WBS §4, "Stand-in bases").

**Engine checks**
- 2026-11-11 76b52d0: check all 79 VERIFY markers (WBS §6, in risk order) and report each as "confirmed" or "wrong, the real API is …".
- 2026-11-11 76b52d0: Workbench's own script compile, the Windows compile check, then `tools/runtime_test/run.sh`. The last Linux run had 0 exceptions, 125/143 missions started and 10/15 questlines ended. On Linux, Special Zombies' `Scripts/` hides ours, so Windows is the real test (MOD_STACK).
- 2026-11-11 76b52d0: the epilogue's Walk Out kills and deletes the character. Check that the player lands back in the deploy menu cleanly (new VERIFY).
- 2026-11-11 76b52d0: measure a player's real run and sprint speed (m/s) and Bacon's run and sprint speed. Young infected are being tuned to about 10% faster than a sprinting player.

## For the cloud

- 2026-11-12 (PC): set `basePrefab` in the manifest for EpilogueBoard (MessageBoard_01_base), EpilogueCandle (CandleHolderDecorative_01) and EpilogueTable (Table_01); the plugin built them and filled the slots. Tribe props set on RZ_GameMode.et (stand-ins): kill sites = suitcases, a UAZ tyre, a folded blanket; old lair = a bunk-bed mattress; marks = a stone verge post (Haven), the minefield sign (Bastion), the FIA graffiti decal (Red Hand), a no-entry sign (Halcyon). Please note these in WBS.
- 2026-11-12 (PC): new PC-side Workbench plugin `scripts/WorkbenchGame/ReforgerZ/RZ_ResourceIndexPlugin.c` ("ReforgerZ: Write resource index": every prefab/sound/layout/texture ResourceName to `$profile:RZ_ResourceIndex.txt`). Please list it in PC_SETUP / WBS tools.

- 2026-11-12 (PC): `tools/compile_check.sh` picked the Store's `python3` shortcut on Windows and failed the dialogue lint; patched it to probe python3/python/py like the runtime test (one-line fix, pushed). Windows compile check: OK, 0 ReforgerZ warnings.
- 2026-11-12 (PC): the author wants the build on **Everon** now. PC is making `Worlds/RZ_Everon/RZ_Everon.ent` (sub-scene of Eden, `SCR_AIWorld_Eden`) and `Missions/RZ_Everon.conf`; Arland stays as the small test world. Done on the PC: world `{4A535A0DE93773EA}Worlds/RZ_Everon/RZ_Everon.ent` (sub-scene of Eden; AI world `SCR_AIWorld_Eden` with `Navmesh_GM_Eden_Soldier.conf` + `GM_Eden_vehicles.nmn`, the same managers as Arland), scenario `{933D8C91F1C91F38}Missions/RZ_Everon.conf`. Loads cleanly in Workbench and boots on the dedicated server (0 exceptions; 96 settlements, 14 holdings, 0 road checkpoints). Please update the docs (WBS "The world", README, PC_SETUP) to name Everon as the main world.

## Claims

(none)

## Status

- Cloud, 2026-11-11 76b52d0: merging 7 build branches still running: finale, missions and errands, dialogue and reputation, disguise, tribes and attacks, traces phases 1/2/5, majors and epilogue. Each one adds manifest entries, and those land here when merged.
- PC, 2026-11-12: Everon world built; QM and notebook actions added (text, from the manifest); plugin made SeasonMark, TribeTag, CallBox, IdCard and filled 15 slots. Next: the remaining missing prefabs, camp life, Workbench Play test.
