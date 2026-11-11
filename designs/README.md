# Designs

**Status: designed; phases 1, 2 and 3 and tribes phases 1 to 3 built.** Approved by the author on 2026-11-11.

This folder holds the approved designs for the rest of the game: who you are, what you do, how it sounds,
the four majors, the cure's finale, the epilogue week and the infected. Phases 1 to 3 of the plan (the shared engine,
reputation's names; talk: the trees, the voices, the season mood and the voice bank; small things everywhere:
errands' core, the mission style's framework, the world memory's writers) and tribes phases 1 to 3 (the crawlers, the land
grid in use, the agent budget, which is also the one body budget; the tribes and the behaviour layer; their life cycle)
are in code; they live in `docs/`.

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
| [REPUTATION_NAMES.md](REPUTATION_NAMES.md) | The parley and the Knock, settling up, the company you keep, the season record (the 16 names, the budget, the voices, the word and crew names are built) | Phases 1 and 2 built; phase 4 (the quiet threat, parley, settling up); phase 7 (season record, titles) |
| [DIALOGUE.md](DIALOGUE.md) | What's left of dialogue (the trees are built, [docs/DIALOGUE.md](../docs/DIALOGUE.md)): lies, promises, claims in kit, multiplayer scenes, strangers, radio speakers, barks | Phase 2 built; phase 3 (asks as errands, promises, callbacks, barks); phase 4 (lies, scenes, vouch); phase 5 (sitting scenes); phase 8 (strangers, radio speakers) |
| [MISSION_STYLE.md](MISSION_STYLE.md) | What's left of the mission style (wave 1, the framework, is built: [docs/DESIGN.md](../docs/DESIGN.md#story-blocks)): route assets, talk scenes, tricks, lies and witnesses, failure as a branch, the merges and drops | Phase 3 built (wave 1); phase 4 (the pilot: worked examples, merges, drops); phase 5 (the other story jobs); phase 8 (one-choice and work rows) |
| [ERRANDS.md](ERRANDS.md) | What's left of errands (the core is built: [docs/ERRANDS.md](../docs/ERRANDS.md)): strangers only where one would find you, locals, twists, board and radio asks, the rest of the routines and cards | Phase 3 built (core); phase 4 (strangers, locals, twists); phase 5 (margin cards); phase 7 (the epilogue deck); phase 8 (to about 60 cards) |
| [DISGUISE.md](DISGUISE.md) | Borrowed kit and faction colours: looks, caught and held, tricks, the side jobs | Phase 4 (disguise phase 1); phase 8 (phases 2 and 3) |
| [INFECTED.md](INFECTED.md) | Tribes in the wild and crawlers in the towns: lairs, ranges, alphas, day parties, hunts, the land, the last week | Steps 0.2 and 0.3; tribes phases 1 to 3 built ([docs/INFECTED.md](../docs/INFECTED.md)); tribes phases 4–5, in parallel with phases 4–5 |
| [BEHAVIOUR.md](BEHAVIOUR.md) | How the infected move and hunt, on top of Bacon's AI: our code drives them except at striking range; gait and senses per kind, formations and flanking, stalking in bush, vaulting low fences, navigation, hunt strategies, attacks per kind | Tribes phases 1–3 (built but for the reactions table, doors, shrieks, blood and the client effects: [docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)) |
| [MAJORS.md](MAJORS.md) | One season-long story per faction, climaxes on cure milestones, dynamic fallbacks from what's happening on the server | Phase 5 |
| [FINALE.md](FINALE.md) | The fight for control of the cure at 90%: five routes, the lock, the race, the stand as a control fight, terms | Phase 6 |
| [EPILOGUE.md](EPILOGUE.md) | The week after the cure (168 hours): the Zone remembers the season, and closing work lets every story finish | Phase 7 (records from phase 3) |
