# Handoff board

The working list between the cloud session (code, tools, docs, manifest) and the PC session
(Workbench, prefabs, worlds, configs, engine tests). Each entry is one or two lines, starts with the
date and a short commit hash, and is **deleted** once it's done. Design goes in the docs, not here.
"WBS" = `docs/WORKBENCH_SETUP.md`.

## For the PC

**Minimum playable subset first (WBS §2)**
- 2026-11-11 76b52d0: play the minimum subset on `Missions/RZ_Arland.conf`. Check the log lines in WBS §7, report every script exception and missing log line here, and run the play-test checklist in WBS §8.

**UI (WBS §4, "Setting up the UI")**
- 2026-11-11 98c9c51: see the map's icon layer in play (step 6), and turn **UI > Board menus** on once the board's menu works.

**Prefabs missing from `Prefabs/` (run "ReforgerZ: Create prefabs", then finish by hand; WBS §4)**
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

**From the cloud's merges of 2026-11-12**
- 2026-11-12 07730f4: `RZ_RadioCallAction` ("Call") on `RZ_WindupRadio.et`, and on any vanilla radio you made crackable (WBS §4, New-player guidance; "Add to prefabs you already have").
- 2026-11-12 b0b0090: `RZ_PlayerEffectsComponent` on `Prefabs/Characters/Core/RZ_PlayerController.et`, next to `RZ_PlayerUIComponent` (WBS §4, "Add to prefabs you already have").
- 2026-11-12 2990171: open the notebook's sixth tab, `UI/layouts/ReforgerZ/RZ_SeasonPage.layout` (written as text), in Workbench, fix what it reports and set **UI > Season page** (WBS §4, UI; "Setting up the UI").
- 2026-11-12 2990171: `Prefabs/ReforgerZ/Core/RZ_GameMode.et` has no `m_UISettings` block since f034d6b (the layouts, the icon set, the notebook prefab were set there): check it's meant, and set them again if not (WBS §4, "Setting up the UI").
- 2026-11-12 2990171: **Season > Story rules** (on by default) on the game mode's season settings (WBS §5).
- 2026-11-12 34ab931: the settings object **World > Traces** (`m_TraceSettings`) on the game mode (WBS §5).
- 2026-11-12 34ab931: Traces' Workbench spike, phase 0: the test atlas, decals on a door, the boards' fallback, the lit jars (WBS §4, "Traces: the Workbench spike (phase 0)"). Do it before the kit below.
- 2026-11-12 34ab931: **World > Traces > Kit > Door marks atlas** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door marks overlays** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, Bastion** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, the Haven** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, the Red Hand** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, Halcyon** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, residents** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Grave** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Pyre** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Candle jars** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Season mark, rescue** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Season mark, burned and other jobs** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Season mark, taken** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Season mark, sealed** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Season mark, emptied** (WBS §4, Traces).

**Manual entries (WBS §4)**
- 2026-11-11 76b52d0: optional lists: loot tables, extra recipes, shop items, Requisition items (WBS §4, "Hand-set list entries").
- 2026-11-11 76b52d0: stand-in bases still left: `RZ_Explosion.et` (a real HE round) and `RZ_Nest.et` (spawns its own) — both need a design call from the cloud (is shelling meant to hurt? should the nest spawn?).

**Engine checks**
- 2026-11-12 2990171: check all 95 VERIFY markers (WBS §6, in risk order) and report each as "confirmed" or "wrong, the real API is …". New since the last count: Traces' decals, loops and lit windows, the client effects, the disguise phases, the epilogue's and the majors' leftovers.
- 2026-11-11 76b52d0: Done (results under For the cloud).
- 2026-11-11 76b52d0: the epilogue's Walk Out kills and deletes the character. Check that the player lands back in the deploy menu cleanly (new VERIFY).
- 2026-11-12 78dad90: re-check Everon's checkpoint log lines in Workbench play on `Missions/RZ_Everon.conf`: `Perimeter: N road checkpoints (from roads).` should say 6 (the cloud's boot of Eden did), after the `Perimeter checkpoints:` lines and one `Perimeter checkpoint at grid ...` line each; look at two barriers in play (on the road, facing the sea, guards toward the water) and report the lines here if it's still 0 (WBS §7).
- 2026-11-12 b0b0090: measure a player's real run and sprint speed in m/s (the code assumes about 4 and 5.5: VERIFY) and Bacon's run and sprint speed; the young's sprint is tuned to about 10% faster than a sprinting player (WBS §6).

## For the cloud

- 2026-11-12 (PC): **Windows runtime test** (scripts as of 0ce47f3, before the seed fixes; Special Zombies loaded): 0 script exceptions. Missions: 106 of 131 started, 46 marked won. Did not start (25): BurnTheNest, Memorial, ClearSettlement, CheckpointDuty, WeaponsCache, BlowTheBridge, FireMission, ResearchData, InstallSensors, SpecimenFromTheNest, NotTonight, BringThemHome, EscortAtDawn, ContactReport, ClearTheTrail, Cull, BurnTheLair, BreakTheHabit, BaitTheCrew, PlantATag, TrackThemHome, CaptureTheIndex, StockThePit, Event RedDusk, Event BastionShelling. Questlines: 11 of 17 ended (every_name, well, cargo, weeks, proof: CanStart false; place fizzled at stage 3). Majors 7/7 OK. Finale 26 OK, 1 FAILED ("the stand is won after overtime": no overtime ran). Epilogue 19 OK, 2 FAILED ("a say at the station": no say; "the Last Broadcast is decided before processing": open). The fake player died to the Perimeter once. Full output on the PC: `~/rz_runtime_out.txt`.
- 2026-11-12 (author's decision, via PC): **no vanilla deploy/loadout screen at all.** The author: "Shouldn't have the default load out screen at all. Should match our UI plan." Build our own spawn screen in the ReforgerZ style (the background picker: the 13 backgrounds as the list, the panel's name, perk, flaw, gear, starting standing, locked/available and Legacy, a Deploy button; pad first, no mouse needed) that opens instead of vanilla's `RoleSelectionMenu` and spawns through the vanilla respawn requests, so `RZ_LoadoutLock` still applies. Update `UI/Mockups/background_picker.html` (drop the "vanilla deploy menu (stand-in)" half), UI.md, UI/LAYOUTS.md and WBS. Faction is already forced to SURV (`m_sForcedFaction`), so no faction or group step is needed. This replaces the deploy-panel layout-override item. The PC will check it in play when it's pushed.
- 2026-11-12 (author's decision, via PC): **move the map key onto the map and drop the notebook's Map key tab.** A compact legend in the full-screen map's bottom-left, shown when zoomed out and fading as you zoom in, with one show/hide input that has a gamepad binding (CLAUDE.md, Controllers). Remove the Map key tab (`RZ_MapKeyPage.layout`, `RZ_MapKeyPage`, **UI > Map key page**, the notebook's tab row: four tabs) and update UI.md, UI/LAYOUTS.md, WBS and the manifest. The PC adds the legend's slot to the game mode and checks it in play once it's built.

## Claims

(none)

## Status

- Cloud, 2026-11-12 0fbc1e2: all the build branches are merged and pushed (finale, missions and errands, dialogue and reputation, disguise, tribes and attacks, Traces phases 1 to 7, majors and epilogue); their prefab work is under "For the PC". Docs match the PC's builds of 2026-11-12.
- PC, 2026-11-12: Everon world built; QM and notebook actions added (text, from the manifest); plugin made SeasonMark, TribeTag, CallBox, IdCard and filled 15 slots. Next: the remaining missing prefabs, camp life, Workbench Play test.
