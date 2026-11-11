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
- 2026-11-11 61b31e1: `World/RZ_EpilogueBoard.et`, `World/RZ_EpilogueCandle.et`, `World/RZ_EpilogueTable.et` (WBS §4, Epilogue).
- 2026-11-11 94217f5: `Zombies/RZ_CallSound.et`, `Zombies/RZ_GreatCallSound.et`.

**Traces' kit, all optional (WBS §4, Traces; do the spike first, WBS "Traces: the Workbench spike")**
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_QueueCar_A.et`, slot **World > Traces > Kit > Column car A** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_QueueCar_B.et`, slot **World > Traces > Kit > Column car B** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_QueueCar_C.et`, slot **World > Traces > Kit > Column car C** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_Suitcases.et`, slot **World > Traces > Kit > Suitcases** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_GymDoor.et`, slot **World > Traces > Kit > Gym door** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_Bus.et`, slot **World > Traces > Kit > Bus** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_ChurchDoor.et`, slot **World > Traces > Kit > Church door** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_Roadblock_D1.et`, slot **World > Traces > Kit > Day 1 roadblock** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_NoticeWall.et`, slot **World > Traces > Kit > Notice wall** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_Shutter.et`, slot **World > Traces > Kit > Rudi's shutter** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_OP.et`, slot **World > Traces > Kit > The hill** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_LastMeal.et`, slot **World > Traces > Kit > The last meal** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_HeldOut.et`, slot **World > Traces > Kit > Held out** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_EdgeChair.et`, slot **World > Traces > Kit > The chair at the edge** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_IOUs.et`, slot **World > Traces > Kit > The honest shop** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_DroppedRun.et`, slot **World > Traces > Kit > The dropped run** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_WaitingRoom.et`, slot **World > Traces > Kit > The waiting room** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_FallenPost.et`, slot **World > Traces > Kit > The fallen post** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_WaterQueue.et`, slot **World > Traces > Kit > The water queue** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_Scorch.et`, slot **World > Traces > Kit > Scorch** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_TruckWreck_Bastion.et`, slot **World > Traces > Kit > Truck wreck, Bastion** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_TruckWreck_RedHand.et`, slot **World > Traces > Kit > Truck wreck, the Red Hand** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_TruckWreck_Haven.et`, slot **World > Traces > Kit > Truck wreck, the Haven** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_TruckWreck_Halcyon.et`, slot **World > Traces > Kit > Truck wreck, Halcyon** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_RoadPostRelic.et`, slot **World > Traces > Kit > Road post relic** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_Crosses.et`, slot **World > Traces > Kit > Cross** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_Sheets.et`, slot **World > Traces > Kit > Day 1 sheets** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: composition `Traces/RZ_Trace_Lantern_Lit.et`, slot **World > Traces > Kit > Lantern, lit** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_RadioHiss.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Radio hiss** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_GeneratorHum.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Generator hum** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Dogs.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Dogs** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Bell.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Bell** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Loudspeaker.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Loudspeaker** (WBS §4, Traces)

**Manual entries (WBS §4)**
- 2026-11-11 55f071f: camp life. Make the smart-action waypoint `AI/RZ_WP_SmartAction.et`, the 49 compositions and the kits (WBS §4, "Camp life: the compositions and kits"). The 21 original pieces come first.
- 2026-11-11 94217f5: tribe props: the kill-site props, the old lair, and the four faction marks on pack ground (WBS §4, Zombies).
- 2026-11-11 76b52d0: optional lists: loot tables, extra recipes, shop items, Requisition items (WBS §4, "Hand-set list entries").
- 2026-11-11 76b52d0: replace the stand-in bases, worst first: drugs' consumable effects, `RZ_Explosion.et` (a real HE round), `RZ_Nest.et` (spawns its own), the carryable generator and alarm clock, the molotov, bottle and tin (WBS §4, "Stand-in bases").

**Engine checks**
- 2026-11-11 76b52d0: check all 79 VERIFY markers (WBS §6, in risk order) and report each as "confirmed" or "wrong, the real API is …".
- 2026-11-12 ee5f1f6: the VERIFY list is 83 now: the new ones are Traces' loop (`World/Traces/RZ_TraceSounds.c:46`) and lit windows (`UI/RZ_TraceLightsClient.c:56`), and the door-mark decals (`UI/RZ_DoorMarkClient.c:77`, `:106`) from the merge (WBS §6).
- 2026-11-11 76b52d0: Workbench's own script compile, the Windows compile check, then `tools/runtime_test/run.sh`. The last Linux run had 0 exceptions, 125/143 missions started and 10/15 questlines ended. On Linux, Special Zombies' `Scripts/` hides ours, so Windows is the real test (MOD_STACK).
- 2026-11-11 76b52d0: the epilogue's Walk Out kills and deletes the character. Check that the player lands back in the deploy menu cleanly (new VERIFY).
- 2026-11-11 76b52d0: measure a player's real run and sprint speed (m/s) and Bacon's run and sprint speed. Young infected are being tuned to about 10% faster than a sprinting player.

## For the cloud

- 2026-11-12 (PC): `tools/compile_check.sh` picked the Store's `python3` shortcut on Windows and failed the dialogue lint; patched it to probe python3/python/py like the runtime test (one-line fix, pushed). Windows compile check: OK, 0 ReforgerZ warnings.
- 2026-11-12 (PC): the author wants the build on **Everon** now. PC is making `Worlds/RZ_Everon/RZ_Everon.ent` (sub-scene of Eden, `SCR_AIWorld_Eden`) and `Missions/RZ_Everon.conf`; Arland stays as the small test world. Done on the PC: world `{4A535A0DE93773EA}Worlds/RZ_Everon/RZ_Everon.ent` (sub-scene of Eden; AI world `SCR_AIWorld_Eden` with `Navmesh_GM_Eden_Soldier.conf` + `GM_Eden_vehicles.nmn`, the same managers as Arland), scenario `{933D8C91F1C91F38}Missions/RZ_Everon.conf`. Loads cleanly in Workbench and boots on the dedicated server (0 exceptions; 96 settlements, 14 holdings, 0 road checkpoints). Please update the docs (WBS "The world", README, PC_SETUP) to name Everon as the main world.

## Claims

(none)

## Status

- Cloud, 2026-11-11 76b52d0: merging 7 build branches still running: finale, missions and errands, dialogue and reputation, disguise, tribes and attacks, traces phases 1/2/5, majors and epilogue. Each one adds manifest entries, and those land here when merged.
- PC, 2026-11-12: Everon world built; QM and notebook actions added (text, from the manifest); plugin made SeasonMark, TribeTag, CallBox, IdCard and filled 15 slots. Next: the remaining missing prefabs, camp life, Workbench Play test.
