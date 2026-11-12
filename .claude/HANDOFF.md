# Handoff board

The working list between the cloud session (code, tools, docs, manifest) and the PC session
(Workbench, prefabs, worlds, configs, engine tests). Each entry is one or two lines, starts with the
date and a short commit hash, and is **deleted** once it's done. Design goes in the docs, not here.
"WBS" = `docs/WORKBENCH_SETUP.md`.

## For the PC

**Minimum playable subset first (WBS §2)**
- 2026-11-11 76b52d0: play the minimum subset on `Missions/RZ_Arland.conf`. Check the log lines in WBS §7, report every script exception and missing log line here, and run the play-test checklist in WBS §8.

**UI (WBS §4, "Setting up the UI")**
- 2026-11-12 91fcf47: open `UI/layouts/ReforgerZ/RZ_Hud.layout` (its new `LookPanel` with `LookIcon` and `LookText`, `RZ_Hud_Look`, written as text) in Workbench, fix what it reports, and see the look line in play in Bastion kit at a road post, with a pad and on a TV (UI.md, the HUD; UI/LAYOUTS.md).
- 2026-11-11 98c9c51: see the map's icon layer in play (step 6), and turn **UI > Board menus** on once the board's menu works.

**Prefabs missing from `Prefabs/` (run "ReforgerZ: Create prefabs", then finish by hand; WBS §4)**
- 2026-11-11 94217f5: `Zombies/RZ_CallSound.et`, `Zombies/RZ_GreatCallSound.et`.

**Traces' kit, all optional (WBS §4, Traces; do the spike first, WBS "Traces: the Workbench spike")**
- 2026-11-12 ee5f1f6: create `RZ_Sound_RadioHiss.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Radio hiss** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_GeneratorHum.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Generator hum** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Dogs.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Dogs** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Bell.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Bell** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Loudspeaker.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Loudspeaker** (WBS §4, Traces)

**From the cloud's merges of 2026-11-12**
- 2026-11-12 2990171: open the notebook's sixth tab, `UI/layouts/ReforgerZ/RZ_SeasonPage.layout` (written as text), in Workbench, fix what it reports and set **UI > Season page** (WBS §4, UI; "Setting up the UI").
- 2026-11-12 34ab931: Traces' Workbench spike, phase 0: the test atlas, decals on a door, the boards' fallback, the lit jars (WBS §4, "Traces: the Workbench spike (phase 0)"). Do it before the kit below.
- 2026-11-12 34ab931: **World > Traces > Kit > Door marks atlas** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door marks overlays** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, Bastion** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, the Haven** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, the Red Hand** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, Halcyon** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, residents** (WBS §4, Traces).

**From the cloud's spawn screen and map key (2026-11-12)**

**Manual entries (WBS §4)**
- 2026-11-11 76b52d0: optional lists: loot tables, extra recipes, shop items, Requisition items (WBS §4, "Hand-set list entries").
- 2026-11-11 76b52d0: stand-in bases still left: `RZ_Explosion.et` (a real HE round) and `RZ_Nest.et` (spawns its own) — both need a design call from the cloud (is shelling meant to hurt? should the nest spawn?).

**Engine checks**
- 2026-11-12 2990171: check all 99 VERIFY markers (WBS §6, in risk order) and report each as "confirmed" or "wrong, the real API is …". New since the last count: Traces' decals, loops and lit windows, the client effects, the disguise phases, the epilogue's and the majors' leftovers.
- 2026-11-11 76b52d0: the epilogue's Walk Out kills and deletes the character. Check that the player lands back on the spawn screen cleanly (new VERIFY).
- 2026-11-12 b0b0090: measure a player's real run and sprint speed in m/s (the code assumes about 4 and 5.5: VERIFY) and Bacon's run and sprint speed; the young's sprint is tuned to about 10% faster than a sprinting player (WBS §6).

## For the cloud

- 2026-11-12 (PC): **Traces kit built as text** (36 compositions in `Prefabs/ReforgerZ/Traces/`, camp life's rules: empty root + RplComponent + Hierarchy, vanilla props as children; all load in Workbench) and **World > Traces > Kit** filled for every composition slot on `RZ_GameMode.et` (`m_TraceSettings` with `m_Kit`; Story rules and the rest left at their defaults). Stand-ins: the column cars are the S105, S1203 and blue UAZ static wrecks; the bus is the UAZ452 van wreck (vanilla has no bus); the trucks are the M923A1 (Bastion), Ural (Red Hand, Haven) and UAZ452 (Halcyon) wrecks; crosses and grave are a fence pole with a plank; the season marks are poles, planks, a blanket, notices and a flowerpot; no decals or plain mesh copies (scorch is a litter pile and logs; no casings, gas mask or bicycle). Not done: the door marks atlas and overlays and the five boards (the spike), and the five sound prefabs (they need recorded sounds). Please note in WBS (Traces).
- 2026-11-12 (PC): **spawn screen works in Workbench play on Everon** (keyboard only: Down to pick, Enter to deploy): "Who you were" with the 13 backgrounds (5 locked), the panel, the start town picker and Deploy; deploying as the Mechanic spawned at the back-road car wreck. 0 script exceptions. **UI > Spawn row layout**, **Map legend layout** and **Season page** set; `m_sMapKeyPage` removed (`m_UISettings` was there all along). `RZ_RadioCallAction` added to the wind-up radio; `RZ_PlayerEffectsComponent` was already on the controller.
- 2026-11-12 (PC): **map legend works**: bottom-left, fades when zoomed in, I hides it, and the map's ReforgerZ icons show. **It overlaps vanilla's bottom-left widgets** (the map's key hints column and the scale bar at about x 0-330, y the bottom 120 px): please move it up or right of them.
- 2026-11-12 (PC): Everon checkpoints confirmed in play: `Perimeter: 6 road checkpoints (from roads).` with six `Perimeter checkpoint at grid ...` lines (101-13, 114-109, 39-107, 74-95, 95-53, 46-113). Barriers not looked at yet.
- 2026-11-12 (PC): **Windows runtime test** (scripts as of 0ce47f3, before the seed fixes; Special Zombies loaded): 0 script exceptions. Missions: 106 of 131 started, 46 marked won. Did not start (25): BurnTheNest, Memorial, ClearSettlement, CheckpointDuty, WeaponsCache, BlowTheBridge, FireMission, ResearchData, InstallSensors, SpecimenFromTheNest, NotTonight, BringThemHome, EscortAtDawn, ContactReport, ClearTheTrail, Cull, BurnTheLair, BreakTheHabit, BaitTheCrew, PlantATag, TrackThemHome, CaptureTheIndex, StockThePit, Event RedDusk, Event BastionShelling. Questlines: 11 of 17 ended (every_name, well, cargo, weeks, proof: CanStart false; place fizzled at stage 3). Majors 7/7 OK. Finale 26 OK, 1 FAILED ("the stand is won after overtime": no overtime ran). Epilogue 19 OK, 2 FAILED ("a say at the station": no say; "the Last Broadcast is decided before processing": open). The fake player died to the Perimeter once. Full output on the PC: `~/rz_runtime_out.txt`.

## Claims

(none)

## Status

- Cloud, 2026-11-12 6924ec7: the spawn screen (in place of vanilla's deploy menu) and the map key on the map are built as code and text; both need their slots set and a play (under "For the PC").
- Cloud, 2026-11-12 0fbc1e2: all the build branches are merged and pushed (finale, missions and errands, dialogue and reputation, disguise, tribes and attacks, Traces phases 1 to 7, majors and epilogue); their prefab work is under "For the PC". Docs match the PC's builds of 2026-11-12.
- PC, 2026-11-12: Everon world built; QM and notebook actions added (text, from the manifest); plugin made SeasonMark, TribeTag, CallBox, IdCard and filled 15 slots. Next: the remaining missing prefabs, camp life, Workbench Play test.
