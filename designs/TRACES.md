# Environmental storytelling: what's left of Traces

**Status: designed, not built.** Approved by the author on 2026-11-11 with all 16 of its recommended
decisions. Traces' phases 1 to 7 are built and described in [docs/TRACES.md](../docs/TRACES.md) (door
marks, doors you can see, player traces, the scenes, sound and light, the epilogue's handover), with
its decisions and what was simplified. This file holds only the pieces that wait on other systems or
on a play-test. Code goes in `scripts/Game/ReforgerZ/World/Traces/`.

## What's left

| Piece | What it adds | Waits on |
|---|---|---|
| **One placer for scenery** | Camp life's placement (rings by role, flat ground, 2 m out of footprints, apart from each other, 6 m off roads, 4 m off a quartermaster or medic) pulled out as `RZ_DressPlacer` in `Factions/` and used by both camp life and the scenes, so a scene can be several pieces round its centre | A camp life refactor; nothing else needs it |
| **Pieces that age** | Multi-piece scenes that drop a piece as they age (a suitcase gone, a chair fallen), and a fresh and a faded variant of a board when the author sets both | The placer above, and the author's variants |
| **Scenes on their buildings** | The gym's beds and Rudi's red hand on the building's own door (F7 as a door layer on a garage's shutter, the board only where no garage stands within 60 m); F6 at a square's landmark (*monument*, *bank*, *station*) | Door slots for named buildings outside towns' resolve radius |
| **Indoor scenes** (decision 10) | A table laid inside, cots in the gym, on loot's furniture spots, behind a setting | A play-test of the outdoor ones |
| **Camp life's pieces** | S1 as camp life's table lamp swapped for its lit twin, S5's hum on its Halcyon generator piece (quieter at night), S7 from a loudspeaker pole piece (`RZ_Dress_Loudspeaker`) in Bastion's kit | Camp life's pieces sit inside its compositions, where no script reaches one: a lit twin of each composition, or the pieces as their own prefabs, from the PC |
| **Placement checks** | Keep 1 km off an active questline clue of the same faction (the 8 m check for players and vehicles is built) | A clue registry the questlines expose |
| **Finer triggers** | V3 only on Walk Him Out's *walk on*; L1 for a pack on Bastion's board; L8's beaks at a nest under study (V5's runner, L1's door and S4's first trail are built) | Those facts exposed by their owners (the mission's outcome, Bastion's board by pack, Halcyon's study list) |
| **Errands on traces** | Designed cards anchored on traces: Boarded Up on V2, Is She Gone? near L1, His Effects on L5 or V7 (a driver's papers, a soldier's tags); a stranger's fresh event at a live trace's place. Traces never deals an errand | The cards are built; the dealer anchoring them on traces is errands' work (`Missions/Errands/`) |
| **One kit with the epilogue** | The epilogue's optional props (sign board, sheet on posts, stencil board, candle, candle in a jar, grave marker, tape between posts, truck wreck) read from Traces' kit; V7's fallen posts passed to the epilogue's outpost boards at the cure | The epilogue's later phases ([EPILOGUE.md](EPILOGUE.md)) |
| **The thumbprint's other exile** | The Long Table's exile counted (the exiles' minutes are saved across a restart) | The Long Table making a player its exile |
