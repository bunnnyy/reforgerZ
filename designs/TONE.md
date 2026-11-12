# Grey choices and dark humour

**Status: designed, not built.** Approved by the author on 2026-11-11. The text standard, the season mood,
the voice bank with its open-channel tails and joke memories, grief, the running jokes, the race and the lock,
the wanderers' and the epilogue week's lines, earned colours and the rules for every channel are built and
live in [docs/WRITING.md](../docs/WRITING.md). This file keeps only what needs another session's code, with
the reason. The section numbers other designs cite are kept.

## 3.3 Grief and other local overrides

- **A holding's named people.** Grief after one of them dies, like a leader's. *Why it waits:* a holding's
  named people are the holding's guards and occupants, respawned as data; nothing records one of them dying
  as a person (Territory would need to).
- **A short house jokes about the soup more**, never about who eats, and not at all in the 30 minutes after a
  death there. *Why it waits:* the bank's `person` lines are said by holdings' people, which don't know a
  safehouse's state; the safehouse's own residents' lines are `Director/Epilogue` and `World/RZ_Safehouses.c`
  text, outside the bank.

## 3.9 Look lines outside a conversation

A failed look at a hostile post goes straight to the guns, and a noticing line plays only while the guard
turns to watch you. *Why it waits:* the barrier's look is borrowed kit's (`Factions/RZ_Disguise*.c`, the
disguise session's), not a line the bank picks.

## 3.12 The epilogue week's jokes

Each stronghold and seat leader's plain lines for every ending are built from parts (docs/WRITING.md); the
**jokes** for the week (about five per faction per ending) are still to write, with the epilogue's own lines
in `Director/Epilogue/RZ_EpilogueText.c` (the epilogue session's).

## 4. Grey choices in missions

The mechanics behind the grey test, all in `Missions/` (the mission-style session's), with
[MISSION_STYLE.md](MISSION_STYLE.md):

- **4.2 The payout spread:** the giver's way pays in full; the kindest option the least, with a world change
  and a deed; the option that hurts a third party more with one faction and a cost elsewhere; the clever way
  best on paper, with a 15–35% discovery chance and a worse result when found out.
- **4.3 Givers who lie:** at most one omission per brief, two or three per lying brief plus an honest version,
  picked once a season (`CanonPick`); a way to the truth open to everyone before the choice; the outcome names
  it; lying by trait; making a caught lie public costs 1 morale (once per faction per real day, once per lie
  per season), an outpost leader caught 5 loyalty. About 12 briefs: Learning the Drones, A Quiet Arrangement,
  Requisition, Names for Bastion, Off-Book Requisition, Rifles for Sale, The Real Orders, Unsanctioned Trial,
  Private Collection, Sell the Formula, The Pit's Debtor, Debt Collection.
- **4.4 Failure as a branch:** every failure (time, the fight, nobody choosing) with its own outcome text and
  world effect.
- **4.5 Callbacks and deeds:** twelve weighted callback slots per life in `RZ_Deeds` (heavy, medium, light;
  light at most four, never pushing out a heavy one), written by missions' choices that involve a person, a
  house or a leader, brought up plainly at most once a visit.
- **4.6 Two columns** for the mission-style table in docs/DESIGN.md: who pays, and what to watch for.
- **4.8 The shared server:** who can choose (the participation rule), the clever way closing the honest one
  for the run, everyone found out together; boards of record in their in-season mode (the epilogue's props).
- **Phases 4 and 5 of the writing pass:** the grey test on every choice mission, and the lying briefs.
- **`RZ_Outcome.Tell`:** the three voiced beats, the reaction from the bank, the system line, and "someone else
  made the call" for the others who took part.
