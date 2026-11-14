# Persistence: what still waits

**Designed, not built** (these parts only). Approved by the author on 2026-11-14. The character who
lasts is built: [DESIGN.md](../docs/DESIGN.md#a-character-who-lasts) (deaths, quest items, coming
round at a camp, the kit, Reset character, the wipe) and
[REPUTATION.md](../docs/REPUTATION.md#characters-and-carry-over) (what carries to a new character).
What follows waits.

| Part | What it does | Why it waits |
|---|---|---|
| A mission's own item, held by reference | A job that keeps its objective item as an entity (a retrieve job's black box, a live capture, Await Relief's package) follows the item that comes back in the pack instead of counting the first one lost | Each of those jobs holds its own reference; one shared hook (the item given back tells its job) is needed |
| Coming round at a hand-built site's spots | At a hand-built camp, you come round at its sleeping place or medic's tent (SITES.md, "Role spots") instead of the yard | The spots are found by type; coming round reads only the camp's home today |
| A bandage and a bottle given | When the background's loadout has no bandage or water bottle, coming round gives one | Needs two optional prefab slots and their manifest entries |
| An escort waits | An escort or a walking contact waits 5 minutes where you fell, then goes home | Each escort block decides on its own today |
| Bounties | A bounty is paid once a death; the same target can't be posted again for 30 real minutes | The board doesn't keep a per-target cooldown |
| The wall's numbers | The epilogue's names on the wall carry each character's deaths as a number on their line | The record keeps names, not deaths |
| A goal after a goal | When a character's personal goal is done, a new one is given (one at a time per character) | Today the first one stays done until a reset |

## Order of work

1. A mission's own item (the shared hook), then the escorts' wait.
2. The bandage and bottle slots; coming round at a hand-built site's spots.
3. Bounties' cooldown, the wall's numbers, a goal after a goal.
