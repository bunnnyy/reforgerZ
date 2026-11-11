# Designs

**Status: designed, not built.** Approved by the author on 2026-11-11.

This folder holds the approved designs for the rest of the game: who you are, what you do, how it sounds,
the four majors, the cure's finale, the epilogue week and the infected. Only phase 1 of the plan (the shared engine
and reputation's names) is in code; it lives in `docs/`.

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
| [TONE.md](TONE.md) | Grey choices and dark humour: the text standard, the season mood, temper, grief and joke memory, one voice bank and one text lint. Becomes `docs/WRITING.md` | Step 0 (the guide and the lint); phase 2 (the voice bank); phase 5 (running jokes); phase 7 (ending variants) |
| [REPUTATION_NAMES.md](REPUTATION_NAMES.md) | Each faction's word for you, the voices, the parley and the Knock, settling up, the season record (the 16 names, the credit rule and one forgiveness budget are built) | Phase 1 built; phase 2 (voice); phase 4 (the quiet threat, parley, settling up); phase 7 (season record, titles) |
| [DIALOGUE.md](DIALOGUE.md) | Conversation trees on the talk screen's `RZ_Dialogue`: hubs, checks and tags, regard, claims, lies, memory, multiplayer scenes | Phase 2 (trees, about 30 conversations); phase 4 (overheard lines, decide scenes, vouch); phase 5 (sitting scenes); phase 8 (strangers, radio speakers) |
| [MISSION_STYLE.md](MISSION_STYLE.md) | The mission style: the story kit, who benefits, who decides, truths, lies and witnesses, failure as a branch, the merges and drops | Phase 3 (wave 1); phase 4 (wave 3: worked examples, merges, drops); phase 5 (wave 4); phase 8 (wave 5) |
| [ERRANDS.md](ERRANDS.md) | Small side jobs from what's true right now: routines and cards, givers, the dealer, strangers only where one would find you | Phase 3 (core); phase 4 (strangers, locals, twists); phase 5 (margin cards); phase 7 (the epilogue deck); phase 8 (to about 60 cards) |
| [DISGUISE.md](DISGUISE.md) | Borrowed kit and faction colours: looks, caught and held, tricks, the side jobs | Phase 4 (disguise phase 1); phase 8 (phases 2 and 3) |
| [INFECTED.md](INFECTED.md) | Tribes in the wild and crawlers in the towns: lairs, ranges, alphas, day parties, hunts, the land, the last week | Steps 0.2 and 0.3; tribes phases 1–5, in parallel with phases 1–5 |
| [BEHAVIOUR.md](BEHAVIOUR.md) | How the infected move and hunt, on top of Bacon's AI: our code drives them except at striking range; gait and senses per kind, formations and flanking, stalking in bush, vaulting low fences, navigation, hunt strategies, attacks per kind | Tribes phases 1–2 |
| [MAJORS.md](MAJORS.md) | One season-long story per faction, climaxes on cure milestones, dynamic fallbacks from what's happening on the server | Phase 5 |
| [FINALE.md](FINALE.md) | The fight for control of the cure at 90%: five routes, the lock, the race, the stand as a control fight, terms | Phase 6 |
| [EPILOGUE.md](EPILOGUE.md) | The week after the cure (168 hours): the Zone remembers the season, and closing work lets every story finish | Phase 7 (records from phase 3) |
