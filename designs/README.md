# Designs

**Status: designed; phases 1 to 4, phases 5, 6 and 7 in part and tribes phases 1 to 5 built.** Approved by the author on 2026-11-11.

This folder holds the approved designs for the rest of the game: who you are, what you do, how it sounds,
the four majors, the cure's finale, the epilogue week and the infected. Phases 1 to 4 of the plan (the shared engine,
reputation's names; talk: the trees, the voices, the season mood and the voice bank; small things everywhere:
errands' core, the mission style's framework, the world memory's writers; the mission style in full), phase 5 in part (the four majors' engine and stories) and tribes phases 1 to 5 (the crawlers, the land
grid in use, the agent budget, which is also the one body budget; the tribes and the behaviour layer; their life cycle;
the people's side: words, knowing, the packs' contracts and events, the cure's hooks, the last week; the great one, signs and
sounds) are in code; they
live in `docs/`.

- **Designs are not docs.** Built behaviour is always described in `docs/` ([the Doc map](../CLAUDE.md#doc-map)).
  A design file only says what's designed, and never describes built behaviour.
- **When a part is built,** its content moves into the `docs/` file that owns the topic (or a new doc), in the
  same commit as the code. The design file is then trimmed, or deleted once nothing in it is left to build.
  [PLAN.md](PLAN.md#docs) says where each design lands.
- **The order of the work** is [PLAN.md](PLAN.md#the-build-order). The roadmap's "Next" points here rather than
  restating it.

## Index

| File | What it covers | Build step ([PLAN.md](PLAN.md#the-build-order)) |
|---|---|---|
| [PLAN.md](PLAN.md) | The one combined plan: the shape of a season, the epilogue week, one owner per thing, the shared engine, the build order, the author's decisions, the 50 conflicts, risks and where each design lands in `docs/` | All steps; step 0 runs now |
| [TONE.md](TONE.md) | What's left of tone to build: the radio in the voice bank, grief's other sources, running jokes, lies and callbacks in choices, the state variants. The text standard, the lint, the season mood and the voice bank are built ([WRITING.md](../docs/WRITING.md)) | Phase 2 built; phase 5 (running jokes); phase 7 (ending variants) |
| [REPUTATION_NAMES.md](REPUTATION_NAMES.md) | The season record and titles, what's left of the parley, settling up and the company you keep (the 16 names, the budget, the voices, the word, crew names and phase 3 are built, [docs/REPUTATION.md](../docs/REPUTATION.md)) | Phases 1 to 4 built; phase 7 (season record, titles) |
| [DIALOGUE.md](DIALOGUE.md) | What's left of dialogue (the trees, the player's lies and the multiplayer layer with `who` are built, [docs/DIALOGUE.md](../docs/DIALOGUE.md)): lies you're told, promises, claims in kit as lines, strangers' conversations, radio speakers, barks | Phases 2 and 4 built; phase 3 (asks as errands, promises, callbacks, barks); phase 5 (sitting scenes); phase 8 (strangers' talk, radio speakers) |
| [MISSION_STYLE.md](MISSION_STYLE.md) | What's left of the mission style (waves 1 to 3 and the Tier 0 audit are built: [docs/DESIGN.md](../docs/DESIGN.md#story-blocks), [docs/MISSIONS.md](../docs/MISSIONS.md)): route assets, talk scenes, tricks and trade, the six worked examples' remaining ways, the other story jobs | Phases 3 and 4 built (waves 1 to 3); phase 5 (the other story jobs); phase 8 (one-choice and work rows) |
| [ERRANDS.md](ERRANDS.md) | What's left of errands (the core and phase 4 are built: [docs/ERRANDS.md](../docs/ERRANDS.md)): promises, callbacks and threads from errands, the remaining routines and cards, the margin, lock and epilogue cards | Phases 3 and 4 built; phase 5 (margin cards); phase 6 (lock cards); phase 7 (the epilogue deck); phase 8 (to about 60 cards) |
| [DISGUISE.md](DISGUISE.md) | Borrowed kit and faction colours: what's left after phase 1 (built: [docs/REPUTATION.md](../docs/REPUTATION.md#borrowed-kit)): the side jobs still to build, phases 2 and 3 | Phase 4 built (disguise phase 1); phase 8 (phases 2 and 3) |
| [INFECTED.md](INFECTED.md) | Tribes in the wild and crawlers in the towns: lairs, ranges, alphas, day parties, hunts, the land, the last week | Steps 0.2 and 0.3; tribes phases 1 to 5 built ([docs/INFECTED.md](../docs/INFECTED.md)) but for the small rules each section lists, the people's rest and the finale's live tissue (with phase 6) |
| [BEHAVIOUR.md](BEHAVIOUR.md) | What's left of how the infected move and hunt (the layer is built: [docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): hearing beyond shots, blood, shrieks, doors, the reactions table, the regroup, the ankle grab and the client effects, the small rules | Tribes phases 1–3 built; the rest not yet scheduled, the client effects after a client test |
| [MAJORS.md](MAJORS.md) | What's left of the four majors (the engine and the four stories' acts, defaults, endings and codas are built: [docs/QUESTLINES.md](../docs/QUESTLINES.md#the-majors)): story rules, hall lines, the season tab, the stand's inputs, Before the stand, the remaining approaches and crossings | Phase 5 built in part |
| [FINALE.md](FINALE.md) | What's left of the cure's finale (the controller, the lock and its dialogue commit node, pitches, route assets, the five routes with their ways and branches, live tissue, pacts and the moves, the stand as a control fight with its HUD line, Take the Lab, the Coat Run and terms are built: [docs/FINALE.md](../docs/FINALE.md)): what waits on other systems, seeds, the lock's other pieces, each route's remaining ways and carries, live tissue's remaining rules, the remaining moves and notes, the stand's and the majors' remaining pieces, the epilogue's carriers | Phase 6 built in part |
| [EPILOGUE.md](EPILOGUE.md) | What's left of the epilogue week (the record, beats, carriers, the outlets, the queue and lab gate, the grey choices and the closure set are built: [docs/EPILOGUE.md](../docs/EPILOGUE.md)): the fuller record and titles, beats' slots, route marks, the evening's own set pieces, scenes that move, the truth, each closing item's rest, the epilogue deck | Phase 7 built in part |
| [TRACES.md](TRACES.md) | What's left of Traces, the world's environmental storytelling (phases 1 to 7 are built: [docs/TRACES.md](../docs/TRACES.md)): one placer for scenery, pieces that age, scenes on their buildings, indoor scenes, camp life's pieces, finer triggers, the Digger, errands on traces, one kit with the epilogue | Built in part; the rest waits on other systems |
