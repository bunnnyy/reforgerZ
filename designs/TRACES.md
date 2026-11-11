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
| **The column in full** | F1's cars spread from 400 m to 1.2 km back from the barrier, at least 25 m from a junction | The placer above |
| **Scenes on their buildings** | The gym's beds and Rudi's red hand on the building's own door (F7 as a door layer on a garage's shutter, the board only where no garage stands within 60 m); F6 at a square's landmark (*monument*, *bank*, *station*) | Door slots for named buildings outside towns' resolve radius |
| **Indoor scenes** (decision 10) | A table laid inside, cots in the gym, on loot's furniture spots, behind a setting | A play-test of the outdoor ones |
| **Camp life's pieces** | S1 as camp life's table lamp swapped for its lit twin, S5's hum on its Halcyon generator piece (quieter at night), S7 from a loudspeaker pole piece (`RZ_Dress_Loudspeaker`) in Bastion's kit | The author's camp life compositions |
| **Placement checks** | Keep 1 km off an active questline clue of the same faction; don't spawn a scene's props while a player or a vehicle is within 8 m of its spot | A clue registry the questlines expose |
| **Finer triggers** | V3 only on Walk Him Out's *walk on*; V5 also when a pack takes a runner within 3 km of the lab or a field station; L1 on the nearest door within 60 m, and for a pack on Bastion's board; L8's beaks at a nest under study; S4's dogs at a town on the night's first trail | Those facts exposed by their owners (the mission's outcome, the tribes' takes and trails, Halcyon's study list) |
| **The Digger** | The Digger stranger finishing makes a grave (P3) | The Digger stranger |
| **Errands on traces** | Designed cards anchored on traces: Boarded Up on V2, Is She Gone? near L1, His Effects on L5 or V7 (a driver's papers, a soldier's tags); a stranger's fresh event at a live trace's place. Traces never deals an errand | Those cards |
| **One kit with the epilogue** | The epilogue's optional props (sign board, sheet on posts, stencil board, candle, candle in a jar, grave marker, tape between posts, truck wreck) read from Traces' kit; V7's fallen posts passed to the epilogue's outpost boards at the cure | The epilogue's later phases ([EPILOGUE.md](EPILOGUE.md)) |
| **The thumbprint, kept** | The exiles' minutes in a town saved across a restart; the Long Table's exile counted | A saved exile record |
