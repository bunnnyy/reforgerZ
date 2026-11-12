# Errands: what's left

**Status: designed, not built.** Approved by the author on 2026-11-11. The rest of errands is built and
described in [docs/ERRANDS.md](../docs/ERRANDS.md); the lie store and the witness rule are in
[docs/DESIGN.md](../docs/DESIGN.md#lies-and-witnesses), and the epilogue deck (20 cards) in
[docs/ERRANDS.md](../docs/ERRANDS.md#the-epilogue-deck). This file keeps only the parts not in code yet.

## Strangers

- **Out of sight:** spawned 60–90 m off where the player can't see (a door, a hedge line, a bend) and
  removed out of sight when passed. VERIFY: the out-of-sight spawn.
- **Every crossroads:** junctions of two used roads from the road network, not only road posts. VERIFY:
  reading junctions from the road network.
- **Robbed:** +8 Haven Infamy, and no stranger for that player for a real day. VERIFY: telling that a
  body's inventory was taken.
- **The run-ups left as words:** Still Walking's board note at his house when he got the dose; The Last
  Tin's watch as a real watch.

<<<<<<< HEAD
=======
## Locals

- Shown only while a player who knows them is within 100 m. VERIFY: per-player action labels (today a
  label is the same for everyone).
- A local's spot used as a place (the gate at dusk, the stove at supper).

## Cards on traces

- **Anchored on Traces' scenes** (from the Traces design, whose own part is built:
  [docs/TRACES.md](../docs/TRACES.md)): Boarded Up on V2, Is She Gone? near L1, His Effects on L5 or V7
  (a driver's papers, a soldier's tags); a stranger's fresh event at a live trace's place. Traces never
  deals an errand: the dealer finds the scene with `RZ_Scenes.SceneNear(kinds, pos, radius, at, key)`.

## The catalogue

- **His Effects in full:** the effects as a carried item; the jacket and the photo frame at the fence (3
  and 10 Red Hand Favor) under the witness rule; a kit piece off a man you killed refused (*"You'd know.
  You were there."*, +3 Infamy with his faction); once per dead man, 3 times a life.
- **The Name on the Tag's named body** at a job's site: carry the ID to the house or post it names, 4
  Favor from that faction. Red Hand crews don't bury their dead, so it's thanks only.

>>>>>>> claude/sleepy-volta-nwm433
## The rest

- The `mission_errand` icon for the journal row (the journal uses the unarmed icon today).
