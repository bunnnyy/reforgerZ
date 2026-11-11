# Designs

**Status: designed; phases 1 to 4, phases 5 and 6 in part and tribes phases 1 to 5 built.** Approved by the author on 2026-11-11.

This folder holds the approved designs for the rest of the game: who you are, what you do, how it sounds,
the four majors, the cure's finale, the epilogue week and the infected. Phases 1 to 4 of the plan (the shared engine,
reputation's names; talk: the trees, the voices, the season mood and the voice bank; small things everywhere:
errands' core, the mission style's framework, the world memory's writers; the mission style in full), phase 5 in part (the four majors' engine and stories) and tribes phases 1 to 5 (the crawlers, the land
grid in use, the agent budget, which is also the one body budget; the tribes and the behaviour layer; their life cycle;
the people's side: words, knowing, the packs' contracts and events, the cure's hooks, the last week; the great one, signs and
sounds; the behaviour layer's rest and the close moves on the player's machine) are in code; they
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
| [INFECTED.md](INFECTED.md) | What's left of the tribes and crawlers (built: [docs/INFECTED.md](../docs/INFECTED.md)): the model's small rules, the call's and the hunt's rest, the towns' giving way and the packs' hordes, scouts' posts, the jobs' rest and questline hooks, the last week's extra beat, the rename table | Tribes phases 1 to 5 and their rest built; these not yet scheduled (the questline hooks with their questlines, live tissue with the finale) |
| [BEHAVIOUR.md](BEHAVIOUR.md) | What's left of how the infected move and hunt (the layer is built: [docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): a few small rules: the notice rate's calibration, the formations' re-plan and ETA, forest edge points, untested levers | Tribes phases 1–3 and the layer's rest built; these wait for a play-test or a lab test |
| [MAJORS.md](MAJORS.md) | What's left of the four majors (the engine and the four stories' acts, defaults, endings and codas are built: [docs/QUESTLINES.md](../docs/QUESTLINES.md#the-majors)): story rules, hall lines, the season tab, the stand's inputs, Before the stand, the remaining approaches and crossings | Phase 5 built in part |
| [FINALE.md](FINALE.md) | What's left of the cure's finale (the controller, the lock, the five routes' acts, live tissue, pacts, the stand as a control fight, Take the Lab, the Coat Run and terms are built: [docs/FINALE.md](../docs/FINALE.md)): route assets, pitches and seeds, the lock's other pieces, each route's remaining ways, the subjects' branches, the moves and notes, the majors' remaining inputs, the epilogue's carriers | Phase 6 built in part |
| [EPILOGUE.md](EPILOGUE.md) | The week after the cure (168 hours): the Zone remembers the season, and closing work lets every story finish | Phase 7 (records from phase 3) |
