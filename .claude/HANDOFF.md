# Handoff board

The working list between the cloud session (code, tools, docs, manifest) and the PC session
(Workbench, prefabs, worlds, configs, engine tests). Each entry is one or two lines, starts with the
date and a short commit hash, and is **deleted** once it's done. Design goes in the docs, not here.
"WBS" = `docs/WORKBENCH_SETUP.md`.

## For the PC

**Minimum playable subset first (WBS §2)**
- 2026-11-11 76b52d0: play the minimum subset on `Missions/RZ_Arland.conf`. Check the log lines in WBS §7, report every script exception and missing log line here, and run the play-test checklist in WBS §8.

**UI (WBS §4, "Setting up the UI")**
- 2026-11-12 bafc9ab: the object menu (UI.md, "The object menu"): add `MenuPreset RZ_ObjectMenu` to `Configs/System/chimeraMenus.conf` (layout `RZ_ObjectMenu.layout`, class `RZ_ObjectMenu`, like the `RZ_CraftMenu` entry), set **UI > Object row layout** to `RZ_ObjectRow.layout` on `RZ_GameMode.et`, open both layouts (written as text) in the layout editor and fix what it reports (WBS §4, Setting up the UI, step 4a.5).
- 2026-11-12 bafc9ab: play the object menu with a pad and on a TV (WBS §8, "The object menu"): a stove, a generator, a house radio, a puzzle object and a keypad, the Haven quartermaster (one **Ask the quartermaster**) and the medic, the lab, the notebook item on the ground; bells, sirens, doors, Clean the bite and Douse stay single prompts. No prefab action changes are needed: the actions keep their class names (they now derive from `RZ_GroupedAction`).
- 2026-11-12 bafc9ab: build `Prefabs/ReforgerZ/Interactions/RZ_FuelPumpStation.et` (an empty entity with an RplComponent, `SCR_FuelManagerComponent` with one 60 L fuel node that provides fuel, and `SCR_FuelSupportStationComponent` from `FuelSupportStation_FuelPump.ct`, as the FuelTank_02 pump has) and set **Interactions > Cars > Fuel pump station**; then at a petrol pump a held vanilla jerrycan offers **Refuel canister** (WBS §8, the petrol pump line; VERIFY `Interactions/RZ_FuelPumps.c:172`). Check the new Cars settings came through: **Pump litres** (60), **Car fuel litres** (5) (WBS §5).
- 2026-11-12 2e9f6f9: the talk screen's pages: open `RZ_TalkMenu.layout` in Workbench (new `TalkPage` with `PagePrev`, `PageText`, `PageNext`, written as text) and fix what it reports; then talk to a council head with Trees on and check "1/2" and MORE > on a TV, LB/RB with a pad, Q/E and a click on the arrows turn the page without saying a reply, and the right stick scrolls a long line the right way (new `RZ_TalkScroll` in `chimeraInputCommon.conf`, VERIFY `UI/RZ_TalkMenu.c:288`; WBS §8, "Pages").
- 2026-11-12 103bd52: the hubs, New Vegas style (DIALOGUE.md, "Hubs: work comes out of talk"): with Trees on, play a leader of each faction, a quartermaster's "Talk to whoever's in charge", a safehouse's named cook, a holding's named person and a wanderer: no "Ask for work / Heard anything? / What do you make of me?" menu, the job, the news and the verdict reached through their own topics, and the built replies back with errands off (WBS §8, the Trees lines and "What's going on").
- 2026-11-12 4288689: re-check spawning in play: **one card, then quiet** (WBS §8, "Spawn: one card, then quiet"). Respawn a few times, first spawn and later ones: count the cards (one: the background and where you wake), see "+N in your notebook" and the Radio tab's World entries, one card at most every 45 s for five minutes. Report any other card in the first 30 s and its title.
- 2026-11-12 4288689: the new settings **UI > Quiet start** (30), **First minutes** (300), **First minutes gap** (45) and **Guidance > Tips wait after spawn** (90) appear on the game mode (WBS §5): check the defaults came through.
- 2026-11-12 4288689: **UI > Quests page** on `RZ_GameMode.et` is `RZ_QuestsPage.layout` (the cloud renamed the old `m_sHallPage` line; the layout kept the hall page's GUID `{FCD5A2E4F7512EFF}`): open the game mode and check the slot is set, then open `RZ_QuestsPage.layout`, `RZ_SeasonPage.layout` and `RZ_Hud.layout` (its new `TrackerPanel`) in the layout editor and fix what they report (UI/LAYOUTS.md).
- 2026-11-12 4288689: with a pad, go through all five tabs (Standing, Journal, Radio, Quests, The season): the Journal has only notes, The season scrolls down to the hall of fame, and on Quests **Y** tracks the selected job (T on a keyboard; the TRACK button with the mouse): `RZ_TrackJob` is new in `chimeraInputCommon.conf`'s `MenuContext`; check Y does nothing else in the notebook (WBS §8, "Quests tab").
- 2026-11-12 4288689: play the quest tracker (WBS §8, "The quest tracker"): take a contract, see the small box top left with a real icon (no flat square), the objective with a rough area (no metres), the other jobs as dim lines, the ring on the map; relog and it's still tracked.
- 2026-11-12 3e921af: your `TrackPanel` (fc7cb37) is replaced by the cloud's `TrackerPanel` and its journal Track button by the Quests tab's; the map's you-are-here stays. Nothing to do but pull; the atlas with `marker_self` still needs importing for the map's arrow (that missing texture was the flat yellow square in the author's screenshot).
- 2026-11-12 (cloud, docs pass): `UI/layouts/Menus/DeployMenu/RespawnMenu.layout` (the override of vanilla's deploy menu) is deleted: it embedded the removed `RZ_BackgroundPanel.layout`, and our spawn screen replaces that menu. Nothing to do but pull; with **UI > Spawn row layout** empty, vanilla's own deploy menu opens.
- 2026-11-12 (cloud, docs pass): new in play: the journal's *The finale* row while the cure's routes are open, the jobs in each player's order, and a sound on each notice card (vanilla's hint sound): check them in Workbench play (UI.md).
- 2026-11-12 (cloud, docs pass): `chimeraInputCommon.conf` has pad bindings for the notebook (View + RB, in play) and the talk screen's replies (LT + D-pad up/right/down/left = 1-4, RT + D-pad up/right/down = 5-7): open the conf in Workbench, then check both with a pad, and that View alone still opens the map and RB alone does what it did (UI.md, Controllers).
- 2026-11-12 4288689: `Prefabs/ReforgerZ/UI/RZ_Notebook.et:40`, the item's Description (the tabs are now Standing, Journal, Radio, Quests, The season). Set it to exactly: `Your notebook: standing, what you know, what you heard, your jobs and the season with its hall of fame. Take it in hand to read it.`
- 2026-11-12 (cloud, docs pass): the map key moved up to offset (96, -190) in `RZ_MapLegend.layout`, clear of vanilla's key hints and scale bar: check in play that it covers none of vanilla's map widgets (WBS §8, the map key).
- 2026-11-12 07f80de: the crafting screen (UI.md, "The crafting screen"): in Workbench add `MenuPreset RZ_CraftMenu` to `Configs/System/chimeraMenus.conf` (layout `RZ_CraftMenu.layout`, class `RZ_CraftMenu`, like the `RZ_ShopMenu` entry), and set **UI > Craft row layout** to `RZ_CraftRow.layout` on `RZ_GameMode.et` (WBS §4, Setting up the UI, step 4a.4). Until then every server start logs `GUI (E): Menu preset 'RZ_CraftMenu' not found!` (harmless: the actions stay).
- 2026-11-12 07f80de: play the crafting screen with a pad, the mouse and on a TV at a workbench, with a few recipes' outputs set: open, move, short and ready recipes, Craft, the countdown, the item in your pack, B, danger closing it, and the slot emptied (WBS §8, "The crafting screen"); and the new VERIFY `UI/RZ_ItemPreview.c:65` (the output's description and weight).
- 2026-11-12 91fcf47: open `UI/layouts/ReforgerZ/RZ_Hud.layout` (its new `LookPanel` with `LookIcon` and `LookText`, `RZ_Hud_Look`, written as text) in Workbench, fix what it reports, and see the look line in play in Bastion kit at a road post, with a pad and on a TV (UI.md, the HUD; UI/LAYOUTS.md).
- 2026-11-11 98c9c51: read a safehouse board in play and check its menu (**UI > Board menus** is on by default; turn it off only if the menu fails).

**Prefabs missing from `Prefabs/` (run "ReforgerZ: Create prefabs", then finish by hand; WBS §4)**
- 2026-11-11 94217f5: `Zombies/RZ_CallSound.et`, `Zombies/RZ_GreatCallSound.et`.

**Traces' kit, all optional (WBS §4, Traces; do the spike first, WBS "Traces: the Workbench spike")**
- 2026-11-12 ee5f1f6: create `RZ_Sound_RadioHiss.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Radio hiss** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_GeneratorHum.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Generator hum** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Dogs.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Dogs** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Bell.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Bell** (WBS §4, Traces)
- 2026-11-12 ee5f1f6: create `RZ_Sound_Loudspeaker.et` (the plugin makes it; add the sound), slot **World > Traces > Kit > Loudspeaker** (WBS §4, Traces)

**From the cloud's merges of 2026-11-12**
- 2026-11-12 34ab931: Traces' Workbench spike, phase 0: the test atlas, decals on a door, the boards' fallback, the lit jars (WBS §4, "Traces: the Workbench spike (phase 0)"). Do it before the kit below.
- 2026-11-12 34ab931: **World > Traces > Kit > Door marks atlas** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door marks overlays** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, Bastion** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, the Haven** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, the Red Hand** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, Halcyon** (WBS §4, Traces).
- 2026-11-12 34ab931: **World > Traces > Kit > Door board, residents** (WBS §4, Traces).

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
- 2026-11-12 (cloud, camp spots off walls): re-check the Morton lean in play: camp life now keeps every loiter and watch spot 0.4 m off walls and out of the building, and pushes pieces 1.4 m (lean pieces 2.3 m) off walls and fences, by flat traces (WBS §8, Faction bases; new VERIFY `Factions/RZ_CampLife.c:2533`). Report any head still in a wall, and where.
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
- 2026-11-12 8039776: check all 127 VERIFY markers (WBS §6, in risk order) and report each as "confirmed" or "wrong, the real API is …". New since the last count: Traces' decals, loops and lit windows, the client effects, the disguise phases, the epilogue's and the majors' leftovers, the lab's named children, `IsUnconscious()` in the clean room, the Walk Out's land-edge fallback and the scenes' car clearance; and now a leader's corpse kept through the vigil, the rival faction's fight and a camp composition's child prefab names; the crews' Smoke throw hook (`OnGrenadeThrown` for a smoke) and a guard's facing for the noticing line; the Walk Out landing the player back on the spawn screen cleanly; and a player's real run and sprint speed in m/s (`Zombies/RZ_Stealth.c:352` assumes about 4 and 5.5; measure Bacon's too: the young's sprint is tuned about 10% faster than a sprinting player); the crafting screen's preview copy of its output (`UI/RZ_ItemPreview.c:65`); and camp life's flat wall traces (`Factions/RZ_CampLife.c:2533`).
- 2026-11-12 681b52a: after the next push, re-run the runtime test on Windows (`tools/runtime_test/run.sh`, WBS §6) and report its summary here. The cloud fixed the epilogue's closing list and the harness (the stand's overtime, the line leaving the tester alone, Arland's busy towns, the packs, the majors-only lines); on Linux: 117 of 131 missions, 15 of 15 questlines, finale and epilogue all OK. Misses WBS lists as expected on Arland need no report.
- 2026-11-12 7c2f323: can the dedicated server open a file inside the addon with `FileIO` (for example `dialogue/radio/open.rzd`, read-only)? Report "yes" or "no, <the error>": it decides whether conversations are read from `.rzd` files at start (docs/DIALOGUE.md, "Simplified").

- 2026-11-12 f0ffcfa: the author asks how to enter Game Master in Workbench play. Check whether `Worlds/RZ_Everon` (and RZ_Arland) let the host open Game Master (vanilla's default key is Y). If not, add what vanilla GM worlds use, so the author can open it in Workbench play, and tell them the steps. Wells: our own "Fill a bottle" is off by default (**Interactions > Well spots**), so check each well now shows one action.

**From the cloud's play-test fixes: treating, spots and loot (2026-11-12)**
- 2026-11-12 2e47cb9: remove `RZ_TreatOtherAction` from each `Prefabs/ReforgerZ/Loadouts/RZ_Loadout_*.et` (13, ActionsManagerComponent > additionalActions): the class is gone (treating is vanilla's heal now), so until then each logs an unknown class. Then as a Nurse treat a bitten player with a bandage, vanilla's way: both get the bite line (WBS §8; VERIFY `Interactions/RZ_Medical.c:15`).
- 2026-11-12 b5a2859: the stove's, a workbench's, a generator's and a house radio's prompt sit on the prop's front, not on the wall beside it (WBS §8; VERIFY `Interactions/RZ_Interactions.c:177`). Report any prop whose prompt is still off, and where.
- 2026-11-12 2c43c17: walk into a house in a fresh town: items on the floor and shelves, by building type, none in walls or on roofs, nothing new appearing while you stand inside; a looted town thin; no loot at strongholds and safehouses (WBS §8, loose loot). Check the new Loot settings came through: **Fresh roll distance** (20), **Slot chance** (0.7), **Sure items** (1), and the new defaults (45 m, 70 m, 350 items, 4/5/6 per building, 0.01 per item) (WBS §5). Report the `Search:` and `Loose loot:` log lines.

## For the cloud

- 2026-11-12 (PC): merged from the author's play-test: the talk screen New Vegas style (compact box, 5 replies, [Admin] sub-list, grey when heard, keys 1-5), map icons 48 px, the Standing page as G2 (rows of icons, 9 new icons, no Fame/Infamy grid), 96 people looks with RZ_Looks.c and PickLook at every faction spawn, camp life laid out by plans (kits are plans now: m_vPlan, m_fPlanYaw, footprints; counters; SetGroupSmartAction finds the sentinel), the notebook item opened by the HUD's held-gadget check, Metabolism water (RZ_Water_Dirty). Claims cleared. Not yet seen in play.

## Claims

(none)

## Status

- Cloud, 2026-11-12 104e08d: the mission style's and errands' designs are built in full (the twenty story jobs, talk scenes, errand pay, the dealer's rest, strangers' last rules, the packs' jobs, cards on traces) and their design files removed; their prefab slots are under "For the PC".
- Cloud, 2026-11-12 8039776: the infected's and Traces' designs are built in full (simplified as INFECTED.md and TRACES.md say) and their design files removed; their cross-owner leftovers are in docs/MISSIONS.md and docs/ERRANDS.md.
- Cloud, 2026-11-12 3393db6: talk's, reputation names' and tone's design files are built and gone (designs/DIALOGUE.md, REPUTATION_NAMES.md, TONE.md); the `.rzd` addon reader waits for the FileIO check above (docs/DIALOGUE.md, "Simplified").
- Cloud, 2026-11-12 17a4c83: the finale's, the majors' and the epilogue's designs are built (simplified as each doc says) and their design files removed; the epilogue deck has 20 cards.
- Cloud, 2026-11-12: merged the finale/majors/epilogue leftovers onto the missions/errands ones. The look's −1 after the lock counts once (`RZ_DisguiseClaims.OtherRoute`; the branch's second copy in `RZ_FinaleRest` removed), and the finale's Hand to Hand reads `RZ_Errands.PromisedAt`.
- Cloud, 2026-11-12 0fbc1e2: all the build branches are merged and pushed (finale, missions and errands, dialogue and reputation, disguise, tribes and attacks, Traces phases 1 to 7, majors and epilogue); their prefab work is under "For the PC". Docs match the PC's builds of 2026-11-12.
- PC, 2026-11-12: Everon world built; QM and notebook actions added (text, from the manifest); plugin made SeasonMark, TribeTag, CallBox, IdCard and filled 15 slots. Next: the remaining missing prefabs, camp life, Workbench Play test.
