# Handoff board

The working list between the cloud session (code, tools, docs, manifest) and the PC session
(Workbench, prefabs, worlds, configs, engine tests). Each entry is one or two lines, starts with the
date and a short commit hash, and is **deleted** once it's done. Design goes in the docs, not here.
"WBS" = `docs/WORKBENCH_SETUP.md`.

## For the PC

**Minimum playable subset first (WBS §2)**
- 2026-11-11 61b31e1: `RZ_QM_Haven.et`, `RZ_QM_Bastion.et`, `RZ_QM_RedHand.et`, `RZ_QM_Halcyon.et` each lack 4 × `RZ_LeaderAction` (Kind SEASON, LOOK, ANSWER_FIRST, ANSWER_OTHER). Add them by hand (the plugin skips existing prefabs). WBS §2 table.
- 2026-11-11 10f83ce: the same quartermasters lack `RZ_FactionServiceAction`s. Haven needs SETTLE and ROADS; Bastion needs SETTLE and CONTACT_BOARD; Red Hand and Halcyon need 3 more each (check the Service values against WBS §2).
- 2026-11-11 76b52d0: play the minimum subset on `Missions/RZ_Arland.conf`. Check the log lines in WBS §7, report every script exception and missing log line here, and run the play-test checklist in WBS §8.

**UI (WBS §4, "Setting up the UI")**
- 2026-11-11 98c9c51: check the notebook, talk, shop, car and reader menus in play (step 4a); import the car picture and set **UI > Car picture** (4a.4).
- 2026-11-11 98c9c51: step 5, the deploy panel (`RZ_BackgroundPanel.layout`); step 6, the map (set **UI > Map icon layout** to `RZ_MapIcon.layout`). Then turn **UI > Board menus** on once the board's menu works.

**Prefabs missing from `Prefabs/` (run "ReforgerZ: Create prefabs", then finish by hand; WBS §4)**
- 2026-11-11 9584a6a: `World/RZ_SeasonMark.et`.
- 2026-11-11 61b31e1: `World/RZ_EpilogueBoard.et`, `World/RZ_EpilogueCandle.et`, `World/RZ_EpilogueTable.et` (WBS §4, Epilogue).
- 2026-11-11 10f83ce: `Zombies/RZ_TribeTag.et`, `Zombies/RZ_CallBox.et`, `Zombies/RZ_IdCard.et`.
- 2026-11-11 94217f5: `Zombies/RZ_CallSound.et`, `Zombies/RZ_GreatCallSound.et`.
- 2026-11-11 61b31e1: `UI/RZ_Notebook.et` lacks `RZ_DeadNameAction` (10f83ce) and 2 × `RZ_EpiNotebookAction` (61b31e1).

**Manual entries (WBS §4)**
- 2026-11-11 55f071f: camp life. Make the smart-action waypoint `AI/RZ_WP_SmartAction.et`, the 49 compositions and the kits (WBS §4, "Camp life: the compositions and kits"). The 21 original pieces come first.
- 2026-11-11 94217f5: tribe props: the kill-site props, the old lair, and the four faction marks on pack ground (WBS §4, Zombies).
- 2026-11-11 76b52d0: optional lists: loot tables, extra recipes, shop items, Requisition items (WBS §4, "Hand-set list entries").
- 2026-11-11 76b52d0: replace the stand-in bases, worst first: drugs' consumable effects, `RZ_Explosion.et` (a real HE round), `RZ_Nest.et` (spawns its own), the carryable generator and alarm clock, the molotov, bottle and tin (WBS §4, "Stand-in bases").

**Engine checks**
- 2026-11-11 76b52d0: check all 79 VERIFY markers (WBS §6, in risk order) and report each as "confirmed" or "wrong, the real API is …".
- 2026-11-11 76b52d0: Workbench's own script compile, the Windows compile check, then `tools/runtime_test/run.sh`. The last Linux run had 0 exceptions, 125/143 missions started and 10/15 questlines ended. On Linux, Special Zombies' `Scripts/` hides ours, so Windows is the real test (MOD_STACK).
- 2026-11-11 76b52d0: the epilogue's Walk Out kills and deletes the character. Check that the player lands back in the deploy menu cleanly (new VERIFY).
- 2026-11-11 76b52d0: measure a player's real run and sprint speed (m/s) and Bacon's run and sprint speed. Young infected are being tuned to about 10% faster than a sprinting player.

## For the cloud

(empty)

## Claims

(none)

## Status

- Cloud, 2026-11-11 76b52d0: merging 7 build branches still running: finale, missions and errands, dialogue and reputation, disguise, tribes and attacks, traces phases 1/2/5, majors and epilogue. Each one adds manifest entries, and those land here when merged.
- PC: (not started)
