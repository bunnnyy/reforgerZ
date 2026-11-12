# Designs

**Status: designed; phases 1 to 4, phases 5, 6, 7 and 8 in part and tribes phases 1 to 5 built.** Approved by the author on 2026-11-11.

This folder holds the approved designs for the rest of the game: who you are, what you do, how it sounds,
the four majors, the cure's finale, the epilogue week and the infected. Phases 1 to 4 of the plan (the shared engine,
reputation's names; talk: the trees, the voices, the season mood and the voice bank; small things everywhere:
errands' core, the mission style's framework, the world memory's writers; the mission style in full), phase 5 in part (the four majors' engine and stories), phase 8 in part (borrowed kit's phases 2 and 3) and tribes phases 1 to 5 (the crawlers, the land
grid in use, the agent budget, which is also the one body budget; the tribes and the behaviour layer; their life cycle;
the people's side: words, knowing, the packs' contracts and events, the cure's hooks, the last week; the great one, signs and
sounds; the behaviour layer's rest and the close moves on the player's machine) are in code; they
live in `docs/`. So are the last pieces of talk, reputation's names and tone: their three files keep only
what waits for an engine check or another session's code.

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
| [TONE.md](TONE.md) | Only what needs another session's code: grief for a holding's named people, a short house's soup jokes, look lines outside a conversation, the epilogue week's jokes, and the grey-choice mechanics in missions (the payout spread, givers who lie, failure as a branch, weighted deeds, the shared server). Everything else is built ([docs/WRITING.md](../docs/WRITING.md)) | Built; the rest waits on the missions, disguise and epilogue sessions |
| [REPUTATION_NAMES.md](REPUTATION_NAMES.md) | Only what needs an engine check or another session's code: the quiet success on questlines' FORCE stages, Smoke (a throw hook), missions' deed phrases, the standing page's header and leaders' lines, the notices' priority, a parley job's journal line. Everything else is built ([docs/REPUTATION.md](../docs/REPUTATION.md)) | Built; the rest waits on the missions and UI sessions, and Smoke on the engine |
| [DIALOGUE.md](DIALOGUE.md) | Only the text source read from the addon (with `$profile` overrides), which waits for an engine check (`FileIO` on addon files). Everything else of dialogue is built ([docs/DIALOGUE.md](../docs/DIALOGUE.md)) | Built; the text source waits for the PC |
| [MISSION_STYLE.md](MISSION_STYLE.md) | What's left of the mission style (waves 1 to 3, the Tier 0 audit, the worked examples' remaining ways, the one-choice jobs, the work rows and route assets with the lock are built: [docs/DESIGN.md](../docs/DESIGN.md#story-blocks), [docs/MISSIONS.md](../docs/MISSIONS.md)): the other missions' route assets, talk scenes, tricks and trade, the other story jobs | Phases 3, 4 and 8 built; phase 5 (the other story jobs) |
| [ERRANDS.md](ERRANDS.md) | What's left of errands (the core, phase 4 and phase 8's 63 cards, with the first margin cards, the lock card and the disguise side jobs, are built: [docs/ERRANDS.md](../docs/ERRANDS.md)): promises, callbacks and threads from errands, His Effects in full, the other majors' margin cards, the epilogue deck's last six cards (13 are built) | Phases 3, 4 and 8 built; phase 5 (the other margin cards); phase 7 in part (the epilogue deck) |
| [DISGUISE.md](DISGUISE.md) | What's left of borrowed kit (disguise phases 1 to 3 and the side jobs are built: [docs/REPUTATION.md](../docs/REPUTATION.md#borrowed-kit), [docs/ERRANDS.md](../docs/ERRANDS.md#the-cards)): cover details and the other kit kinds in conversations, the majors' and the finale's tricks, the HUD, the last week | Phase 4 built (disguise phase 1); phase 8 built in part (disguise phases 2 and 3); phases 5 to 7 (the majors, the finale, the last week) |
| [INFECTED.md](INFECTED.md) | What's left of the tribes and crawlers (built: [docs/INFECTED.md](../docs/INFECTED.md)): the model's small rules, the call's and the hunt's rest, the towns' giving way and the packs' hordes, scouts' posts, the jobs' rest and questline hooks, the last week's extra beat, the rename table | Tribes phases 1 to 5 and their rest built; these not yet scheduled (the questline hooks with their questlines, live tissue with the finale) |
| [BEHAVIOUR.md](BEHAVIOUR.md) | What's left of how the infected move and hunt (the layer is built: [docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): a few small rules: the notice rate's calibration, the formations' re-plan and ETA, forest edge points, untested levers | Tribes phases 1–3 and the layer's rest built; these wait for a play-test or a lab test |
| [MAJORS.md](MAJORS.md) | What's left of the four majors (the engine, the four stories' acts, defaults, endings and codas, the story rules, the stand's inputs, the hall's lines, the season tab and Before the stand are built: [docs/QUESTLINES.md](../docs/QUESTLINES.md#the-majors)): the codas' standing versions and second airings as scenes, leader stand-ins, channels and profiles, the story rules' last pieces, the remaining approaches and crossings | Phase 5 built in part |
| [FINALE.md](FINALE.md) | What's left of the cure's finale (the controller, the lock and its dialogue commit node, pitches, route assets, the five routes with their ways and branches, live tissue, pacts and the moves, the stand as a control fight with its HUD line, Take the Lab, the Coat Run and terms are built: [docs/FINALE.md](../docs/FINALE.md)): what waits on other systems, seeds, the lock's other pieces, each route's remaining ways and carries, live tissue's remaining rules, the remaining moves and notes, the stand's and the majors' remaining pieces, the epilogue's carriers | Phase 6 built in part |
| [EPILOGUE.md](EPILOGUE.md) | What's left of the epilogue week (the record, beats, carriers, the outlets, the queue and lab gate, the grey choices and the closure set are built: [docs/EPILOGUE.md](../docs/EPILOGUE.md)): the fuller record and titles, beats' slots, route marks, the evening's own set pieces, scenes that move, the truth, each closing item's rest, the epilogue deck's last six cards (13 are built), the supper reading as a scene and the standing versions | Phase 7 built in part |
| [TRACES.md](TRACES.md) | What's left of Traces, the world's environmental storytelling (phases 1 to 7 are built: [docs/TRACES.md](../docs/TRACES.md)): one placer for scenery, pieces that age, scenes on their buildings, indoor scenes, camp life's pieces, finer triggers, the Digger, errands on traces, one kit with the epilogue | Built in part; the rest waits on other systems |
