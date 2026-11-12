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

**From the cloud's missions and errands pass of 2026-11-12 (all optional; WBS §4, "Missions")**
- 2026-11-12 3e8f53a: prop `RZ_ErrandProp_Effects.et`, slot **Missions > Errands > Props > Effects** (WBS §4).
- 2026-11-12 3e8f53a: prop `RZ_ErrandProp_Crate.et`, slot **Missions > Errands > Props > Crate** (WBS §4).
- 2026-11-12 3e8f53a: prop `RZ_ErrandProp_Papers.et`, slot **Missions > Errands > Props > Papers** (WBS §4).
- 2026-11-12 3e8f53a: prop `RZ_ErrandProp_Parts.et`, slot **Missions > Errands > Props > Parts** (WBS §4).
- 2026-11-12 3e8f53a: play-test Crash Site's recovery team (a real squad from **Missions > Contracts > Bastion squad**) and Lost Colleague's search and walk-in (WBS §8, the story jobs).
- 2026-11-12 985b210: item `RZ_ErrandCarried.et` (a small bundle that can't be equipped, with `RZ_MissionItemComponent`), slot **Missions > Errands > Props > Carried** (WBS §4; empty = the mission item).
- 2026-11-12 985b210: slot **Missions > Errands > Props > Pay ration**: a vanilla ration item (its name holding a food word) (WBS §4).
- 2026-11-12 985b210: slot **Missions > Errands > Props > Pay magazine**: a vanilla rifle magazine (WBS §4).
- 2026-11-12 985b210: slot **Missions > Errands > Props > Watch**: a vanilla wristwatch item (WBS §4).
- 2026-11-12 985b210: the new Errands settings **Pay in kind**, **Meals as pay**, **When the truck comes** (on) and **Headlines a day** (2) appear on the game mode (WBS §5): check the defaults came through.
- 2026-11-12 6fd2709: the new setting **Missions > Story > Numbered scenes** (off) appears (WBS §5); play a talk scene with it off, and on if the slots' labels don't show (WBS §8).
- 2026-11-12 104e08d: the icon atlas holds `mission_errand` now (`UI/Textures/ReforgerZ/RZ_Icons.png`, 63 icons): import it again and run `py tools/icon_atlas.py imageset` (UI/LAYOUTS.md, "Importing the icons").
- 2026-11-12 104e08d: play-test the story jobs' talk scenes, errand pay, His Effects, strangers out of sight and the new tricks (WBS §8), and check the five new VERIFY markers (WBS §6: strangers' sight, junctions, a robbed body, locals' labels, mission goods' tag).

**From the cloud's finale, majors and epilogue rest (2026-11-12)**
- 2026-11-12 a122e00: `World/RZ_EpilogueSheet.et`, slot **Director > Epilogue > Sheet on posts** (WBS §4, Epilogue; all optional, look only).
- 2026-11-12 a122e00: `World/RZ_EpilogueStencil.et`, slot **Director > Epilogue > Stencil board** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpiloguePriceBoard.et`, slot **Director > Epilogue > Price board** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueNotice.et`, slot **Director > Epilogue > Notice on a post** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueGrave.et`, slot **Director > Epilogue > Grave marker** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpiloguePoster.et`, slot **Director > Epilogue > Wanted poster** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueTrestle.et`, slot **Director > Epilogue > Trestle table** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueCrate.et`, slot **Director > Epilogue > Crate** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueBunting.et`, slot **Director > Epilogue > Bunting** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueRibbon.et`, slot **Director > Epilogue > Ribbon** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueWreck.et`, slot **Director > Epilogue > Truck wreck** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueTape.et`, slot **Director > Epilogue > Tape between posts** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueHandcart.et`, slot **Director > Epilogue > Handcart** (WBS §4).
- 2026-11-12 a122e00: `World/RZ_EpilogueNamesBoard.et`, slot **Director > Epilogue > Names board** (WBS §4).
- 2026-11-12 a122e00: the new Epilogue settings **Marks per cluster** (8), **Scenes that last** (2), **Evening scenes** (1) appear on the game mode (WBS §5): check the defaults came through.
- 2026-11-12 17a4c83: `Questlines/RZ_RedHandLorry.et`, slot **Questlines > Majors > Red Hand lorry** (WBS §4, Questlines; optional, look only).
- 2026-11-12 17a4c83: `Questlines/RZ_Bonfire.et`, slot **Questlines > Majors > Bonfire prop** (WBS §4, Questlines; optional, look only).
- 2026-11-12 17a4c83: on the lab prefab, two empty child entities named `RZ_CleanRoom` (where the stand is fought) and `RZ_LoadingBay` (Take the Lab's hold), and check in play that the finale finds them (WBS §6, `Cure/RZ_CureFinale.c:1483`).

**From the cloud's faction bases (2026-11-12)**
- 2026-11-12 (cloud, docs pass): `designs/camp_plans.html` moved to `tools/camp_kit/camp_plans.html`, now `render.py`'s default output (`py tools/camp_kit/render.py`); `designs/` is empty. Nothing to do but pull.
- 2026-11-12 61d3d8e: re-run `py tools/camp_kit/build.py`: it now writes each kit piece's **At** (where it stands by a building, from `pieces.py`'s `AT`) and the new barricade counts (Bastion 3, Red Hand 2, Haven 2) onto `RZ_GameMode.et`; check the new setting **Territory > Camp life > Buildings** (on) came through (WBS §5).
- 2026-11-12 61d3d8e: play-test each faction's base on Everon (WBS §8, *Faction bases on Everon*): its stronghold and one outpost, a Haven safehouse and Halcyon's field station. Report each `Camp life: ... moves into ...` log line (or `no building fits`), any building that looks wrong for the faction, pieces inside or on top of the building, and whether the gate and barricades sit at the real road entries.

**From the cloud's infected and Traces rest (2026-11-12; all optional)**
- 2026-11-12 8039776: create `RZ_Sound_GeneratorHumNight.et` (the plugin makes it; add the quieter hum), slot **World > Traces > Kit > Generator hum, night** (WBS §4, Traces).
- 2026-11-12 8039776: build `Dressing/Bastion/RZ_Dress_Loudspeaker.et` (a loudspeaker on a pole, camp life's composition rules) and add it to Bastion's kits (**Territory > Camp life > Kits**), so S7's chime comes from it (WBS §4, Camp life).
- 2026-11-12 8039776: **World > Traces > Kit > Pieces that age**: entries for a scene's extra pieces and faded twins, if wanted (WBS §4, Traces; TRACES.md, How scenes age).
- 2026-11-12 8039776: if wanted, a second infected faction in the faction manager, hostile to the infected's own, and its key in **Zombies > Tribes > Rival faction** (empty = off; VERIFY at `Zombies/RZ_TribeRest.c:1069`).
- 2026-11-12 8039776: check the new settings came through on the game mode: **Zombies > Tribes > Prime after Halcyon** (off), **Rival faction** (empty), **World > Traces > Indoor scenes** (off) (WBS §5).
- 2026-11-12 8039776: play-test the behaviour layer's untested levers L6, L16 and L23 and calibrate **Notice rate** (WBS §8, "The tribes' life cycle"; not in the code until a client shows them working).
**From the cloud's reputation-names and tone pass (2026-11-12)**
- 2026-11-12 7d85307: the new Reputation settings **Quiet success** (on) and **Quiet success: the Fear difficulty** (2) appear on the game mode (WBS §5): check the defaults came through.
- 2026-11-12 7d85307: Known with the Red Hand and no crew name, throw a vanilla smoke grenade: the log says `Reputation: <you> threw a smoke` and the crews call you Smoke (VERIFY `Factions/RZ_RepNamesThrows.c:59`, WBS §6 and §8).
- 2026-11-12 a290ce2: in Bastion kit after a passed look, stand behind a sentry when the again look is due: no noticing line until he faces you (VERIFY `Factions/RZ_DisguiseWatch.c:193`, WBS §8).

**Manual entries (WBS §4)**
- 2026-11-11 76b52d0: optional lists: loot tables, extra recipes, shop items, Requisition items (WBS §4, "Hand-set list entries").
- 2026-11-12 5b6814d: rebuild `RZ_Explosion.et` as look and sound only, no damage, on `Warhead_Base.et` like `RZ_WarningShot.et`; and `RZ_Nest.et` without the hive's infected spawner child or its blast warhead (WBS §4, "Stand-in bases": the exact components and the two checks in play).

**Engine checks**
- 2026-11-12 8039776: check all 110 VERIFY markers (WBS §6, in risk order) and report each as "confirmed" or "wrong, the real API is …". New since the last count: Traces' decals, loops and lit windows, the client effects, the disguise phases, the epilogue's and the majors' leftovers, the lab's named children, `IsUnconscious()` in the clean room, the Walk Out's land-edge fallback and the scenes' car clearance; and now a leader's corpse kept through the vigil, the rival faction's fight and a camp composition's child prefab names; the crews' Smoke throw hook (`OnGrenadeThrown` for a smoke) and a guard's facing for the noticing line.
- 2026-11-12 681b52a: after the next push, re-run the runtime test on Windows (`tools/runtime_test/run.sh`, WBS §6) and report its summary here. The cloud fixed the epilogue's closing list and the harness (the stand's overtime, the line leaving the tester alone, Arland's busy towns, the packs, the majors-only lines); on Linux: 117 of 131 missions, 15 of 15 questlines, finale and epilogue all OK. Misses WBS lists as expected on Arland need no report.
- 2026-11-11 76b52d0: the epilogue's Walk Out kills and deletes the character. Check that the player lands back on the spawn screen cleanly (new VERIFY).
- 2026-11-12 7c2f323: can the dedicated server open a file inside the addon with `FileIO` (for example `dialogue/radio/open.rzd`, read-only)? Report "yes" or "no, <the error>": it decides whether conversations are read from `.rzd` files at start (docs/DIALOGUE.md, "Simplified").
- 2026-11-12 b0b0090: measure a player's real run and sprint speed in m/s (the code assumes about 4 and 5.5: VERIFY) and Bacon's run and sprint speed; the young's sprint is tuned to about 10% faster than a sprinting player (WBS §6).

## For the cloud

- 2026-11-12 (PC): **Traces kit built as text** (36 compositions in `Prefabs/ReforgerZ/Traces/`, camp life's rules: empty root + RplComponent + Hierarchy, vanilla props as children; all load in Workbench) and **World > Traces > Kit** filled for every composition slot on `RZ_GameMode.et` (`m_TraceSettings` with `m_Kit`; Story rules and the rest left at their defaults). Stand-ins: the column cars are the S105, S1203 and blue UAZ static wrecks; the bus is the UAZ452 van wreck (vanilla has no bus); the trucks are the M923A1 (Bastion), Ural (Red Hand, Haven) and UAZ452 (Halcyon) wrecks; crosses and grave are a fence pole with a plank; the season marks are poles, planks, a blanket, notices and a flowerpot; no decals or plain mesh copies (scorch is a litter pile and logs; no casings, gas mask or bicycle). Not done: the door marks atlas and overlays and the five boards (the spike), and the five sound prefabs (they need recorded sounds). Please note in WBS (Traces).
- 2026-11-12 (PC): **spawn screen works in Workbench play on Everon** (keyboard only: Down to pick, Enter to deploy): "Who you were" with the 13 backgrounds (5 locked), the panel, the start town picker and Deploy; deploying as the Mechanic spawned at the back-road car wreck. 0 script exceptions. **UI > Spawn row layout**, **Map legend layout** and **Season page** set; `m_sMapKeyPage` removed (`m_UISettings` was there all along). `RZ_RadioCallAction` added to the wind-up radio; `RZ_PlayerEffectsComponent` was already on the controller.
- 2026-11-12 (PC): **map legend works**: bottom-left, fades when zoomed in, I hides it, and the map's ReforgerZ icons show. **It overlaps vanilla's bottom-left widgets** (the map's key hints column and the scale bar at about x 0-330, y the bottom 120 px): please move it up or right of them.
- 2026-11-12 (PC): Everon checkpoints confirmed in play: `Perimeter: 6 road checkpoints (from roads).` with six `Perimeter checkpoint at grid ...` lines (101-13, 114-109, 39-107, 74-95, 95-53, 46-113). Barriers not looked at yet.
- 2026-11-12 (author's decision, via PC): **no vanilla deploy/loadout screen at all.** The author: "Shouldn't have the default load out screen at all. Should match our UI plan." Build our own spawn screen in the ReforgerZ style (the background picker: the 13 backgrounds as the list, the panel's name, perk, flaw, gear, starting standing, locked/available and Legacy, a Deploy button; pad first, no mouse needed) that opens instead of vanilla's `RoleSelectionMenu` and spawns through the vanilla respawn requests, so `RZ_LoadoutLock` still applies. Update `UI/Mockups/background_picker.html` (drop the "vanilla deploy menu (stand-in)" half), UI.md, UI/LAYOUTS.md and WBS. Faction is already forced to SURV (`m_sForcedFaction`), so no faction or group step is needed. This replaces the deploy-panel layout-override item. The PC will check it in play when it's pushed.
- 2026-11-12 (author's decision, via PC): **move the map key onto the map and drop the notebook's Map key tab.** A compact legend in the full-screen map's bottom-left, shown when zoomed out and fading as you zoom in, with one show/hide input that has a gamepad binding (CLAUDE.md, Controllers). Remove the Map key tab (`RZ_MapKeyPage.layout`, `RZ_MapKeyPage`, **UI > Map key page**, the notebook's tab row: four tabs) and update UI.md, UI/LAYOUTS.md, WBS and the manifest. The PC adds the legend's slot to the game mode and checks it in play once it's built.
- 2026-11-12 (PC): **script fixes (one-liners, compile check OK):** negative seeds indexed arrays. `RZ_Tribes.c` CallPattern and AlphaWho, `RZ_TribePeople.c` CrewName now use `Math.AbsInt(...) % Count()` (each threw "Index out of bounds" in Workbench play on Everon); `RZ_SupplyLines.c` out-of-stock hash too (a negative seed made every item out of stock). Please scan for the same pattern in new code.
- 2026-11-12 212e575 (cloud): Everon boots with 0 road checkpoints; cloud to investigate
- 2026-11-12 cc30ef4 (cloud, missions, for the finale's owner): Hand to Hand's promises are queryable: `gm.GetErrands().PromisedAt(house, ident)` (living promised locals at a safehouse; "" = by anyone) and `PromisedNear(pos, radius, ident)`. Wire them where designs/FINALE.md says (one of the eight's yes one step easier with a promised local in their house; a minute off the Independent's lab-gate sit-in per promised local present). One-choice jobs now tag their route assets (allies, docs/MISSIONS.md "Route assets"); the finale reads them through `RZ_FinaleAssets.Find` as before.


- 2026-11-12 6c66be6 (cloud, missions, for the UI owner): order per player is a query: `RZ_MissionReasons.OrderScore(gm, mission, playerId, playerPos)` (higher first; near, reason-started, a leader they helped). The journal's and the quartermaster's lists can sort by it (DESIGN "Story blocks" says they don't yet).


## Claims

(none)

## Status

- Cloud, 2026-11-12 104e08d: the mission style's and errands' designs are built in full (the twenty story jobs, talk scenes, errand pay, the dealer's rest, strangers' last rules, the packs' jobs, cards on traces) and their design files removed; their prefab slots are under "For the PC".
- Cloud, 2026-11-12 8039776: the infected's and Traces' designs are built in full (simplified as INFECTED.md and TRACES.md say) and their design files removed; their cross-owner leftovers are in MISSION_STYLE.md and ERRANDS.md.
- Cloud, 2026-11-12 3393db6: talk's, reputation names' and tone's design files are built and gone (designs/DIALOGUE.md, REPUTATION_NAMES.md, TONE.md); the `.rzd` addon reader waits for the FileIO check above (docs/DIALOGUE.md, "Simplified").

- Cloud, 2026-11-12 6924ec7: the spawn screen (in place of vanilla's deploy menu) and the map key on the map are built as code and text; both need their slots set and a play (under "For the PC").
- Cloud, 2026-11-12 17a4c83: the finale's, the majors' and the epilogue's designs are built (simplified as each doc says) and their design files removed; the epilogue deck has 20 cards.
- Cloud, 2026-11-12: merged the finale/majors/epilogue leftovers onto the missions/errands ones. The look's −1 after the lock counts once (`RZ_DisguiseClaims.OtherRoute`; the branch's second copy in `RZ_FinaleRest` removed), and the finale's Hand to Hand reads `RZ_Errands.PromisedAt`.
- Cloud, 2026-11-12 0fbc1e2: all the build branches are merged and pushed (finale, missions and errands, dialogue and reputation, disguise, tribes and attacks, Traces phases 1 to 7, majors and epilogue); their prefab work is under "For the PC". Docs match the PC's builds of 2026-11-12.
- PC, 2026-11-12: Everon world built; QM and notebook actions added (text, from the manifest); plugin made SeasonMark, TribeTag, CallBox, IdCard and filled 15 slots. Next: the remaining missing prefabs, camp life, Workbench Play test.
