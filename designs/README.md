# Designs

**Status: designed; phases 1 to 4, phases 5, 6, 7 and 8 in part and tribes phases 1 to 5 built.** Approved by the author on 2026-11-11.

This folder holds the approved designs for the rest of the game: who you are, what you do, how it sounds,
the four majors, the cure's finale, the epilogue week and the infected. Phases 1 to 4 of the plan (the shared engine,
reputation's names; talk: the trees, the voices, the season mood and the voice bank; small things everywhere:
errands' core, the mission style's framework, the world memory's writers; the mission style in full), phase 5 in part (the four majors' engine and stories), phase 8 in part (borrowed kit's phases 2 and 3) and tribes phases 1 to 5 (the crawlers, the land
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
| [TONE.md](TONE.md) | What's left of tone to build: the radio in the voice bank, grief's other sources, running jokes on the channels, race lines and pitches, givers who lie in briefs, the epilogue's variants. The text standard, the lint, the season mood, the voice bank, route and look lines face to face and dry lines are built ([WRITING.md](../docs/WRITING.md)) | Phase 2 built, and phase 6's face-to-face variants; phase 5 (running jokes); phase 7 (ending variants) |
| [REPUTATION_NAMES.md](REPUTATION_NAMES.md) | What's left of the voices (the crossing opener, the change notice in voice, gate barks and others-hear), the parley's and settling's closures after the lock, the last week's hooks and square, the remaining crew names and deed phrases, the UI pieces (the 16 names, the budget, the voices, the word, crew names, phase 3, the season record and titles are built, [docs/REPUTATION.md](../docs/REPUTATION.md)) | Phases 1 to 4 built, and the season record; phase 7 (the last week's hooks) |
| [DIALOGUE.md](DIALOGUE.md) | What's left of dialogue (the trees, lies both ways, promises and callbacks, claims in kit, the multiplayer layer and sittings, barks, strangers and the open channel, and the finale's commit are built, [docs/DIALOGUE.md](../docs/DIALOGUE.md)): asks as errands, the rest of lies and promises, cover details as choices, Make it right, the Independent's commit on air and the last week's conversations, distress calls and the secure nets, the rest of the format | Phases 2, 4, 6 and 8's talk built, 3 and 5 in part; phase 6 (the commit on air); phase 7 (the last week) |
| [MISSION_STYLE.md](MISSION_STYLE.md) | What's left of the mission style (waves 1 to 3, the Tier 0 audit, the worked examples' remaining ways, the one-choice jobs, the work rows and route assets with the lock are built: [docs/DESIGN.md](../docs/DESIGN.md#story-blocks), [docs/MISSIONS.md](../docs/MISSIONS.md)): the other missions' route assets, talk scenes, tricks and trade, the other story jobs | Phases 3, 4 and 8 built; phase 5 (the other story jobs) |
| [ERRANDS.md](ERRANDS.md) | What's left of errands (the core, phase 4 and phase 8's 63 cards, with the first margin cards, the lock card and the disguise side jobs, are built: [docs/ERRANDS.md](../docs/ERRANDS.md)): promises, callbacks and threads from errands, His Effects in full, the other majors' margin cards, the epilogue deck's last six cards (13 are built) | Phases 3, 4 and 8 built; phase 5 (the other margin cards); phase 7 in part (the epilogue deck) |
| [INFECTED.md](INFECTED.md) | What's left of the tribes and crawlers (built: [docs/INFECTED.md](../docs/INFECTED.md)): the model's small rules, the call's and the hunt's rest, the towns' giving way, scouts' posts, the jobs' rest, the behaviour layer's untested levers, the last week's extra beat, the rename table | Tribes phases 1 to 5, their rest, the questline hooks and the behaviour layer's small rules built; these not yet scheduled (live tissue with the finale; the levers after a client check) |
| [MAJORS.md](MAJORS.md) | What's left of the four majors (the engine, the four stories' acts, defaults, endings and codas, the story rules, the stand's inputs, the hall's lines, the season tab and Before the stand are built: [docs/QUESTLINES.md](../docs/QUESTLINES.md#the-majors)): the codas' standing versions and second airings as scenes, leader stand-ins, channels and profiles, the story rules' last pieces, the remaining approaches and crossings | Phase 5 built in part |
| [FINALE.md](FINALE.md) | What's left of the cure's finale (the controller, the lock and its dialogue commit node, pitches, route assets, the five routes with their ways and branches, live tissue, pacts and the moves, the stand as a control fight with its HUD line, Take the Lab, the Coat Run and terms are built: [docs/FINALE.md](../docs/FINALE.md)): what waits on other systems, seeds, the lock's other pieces, each route's remaining ways and carries, live tissue's remaining rules, the remaining moves and notes, the stand's and the majors' remaining pieces, the epilogue's carriers | Phase 6 built in part |
| [EPILOGUE.md](EPILOGUE.md) | What's left of the epilogue week (the record, beats, carriers, the outlets, the queue and lab gate, the grey choices and the closure set are built: [docs/EPILOGUE.md](../docs/EPILOGUE.md)): the fuller record and titles, beats' slots, route marks, the evening's own set pieces, scenes that move, the truth, each closing item's rest, the epilogue deck's last six cards (13 are built), the supper reading as a scene and the standing versions | Phase 7 built in part |
| [TRACES.md](TRACES.md) | What's left of Traces, the world's environmental storytelling (phases 1 to 7 are built: [docs/TRACES.md](../docs/TRACES.md)): one placer for scenery, pieces that age, scenes on their buildings, indoor scenes, camp life's pieces, finer triggers, the Digger, errands on traces, one kit with the epilogue | Built in part; the rest waits on other systems |
