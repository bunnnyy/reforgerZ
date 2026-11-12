# Designs

**Status: designed; phases 1 to 8, the infected, Traces, errands and the mission style built.** Approved by the author on 2026-11-11.

This folder holds the approved designs for the rest of the game: who you are, what you do, how it sounds,
and what's left of talk, reputation's names and tone. Phases 1 to 4 of the plan (the shared engine,
reputation's names; talk: the trees, the voices, the season mood and the voice bank; small things everywhere:
errands' core, the mission style's framework, the world memory's writers; the mission style in full), phases 5 to 7 (the four majors, the cure's finale and the epilogue week: their design files are done and gone), phase 8 in part (borrowed kit's phases 2 and 3) and tribes phases 1 to 5 (the crawlers, the land
grid in use, the agent budget, which is also the one body budget; the tribes and the behaviour layer; their life cycle;
the people's side: words, knowing, the packs' contracts and events, the cure's hooks, the last week; the great one, signs and
sounds; the behaviour layer's rest and the close moves on the player's machine; the small rules) and
Traces, errands and the mission style in full (the twenty story jobs, talk scenes, errand pay, the dealer's
rest, strangers' last rules) are in code; they live in `docs/`, and their design files are gone. So are talk,
reputation's names and tone: their design files are done and gone.

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
| [camp_plans.html](camp_plans.html) | Drawings, not a design: every camp-life piece from above, a camp concept per faction and the faction bases' schematic plans, drawn by `tools/camp_kit/render.py`. The faction bases are built ([docs/DESIGN.md](../docs/DESIGN.md#camp-life)); the Workshop mods researched for their look are in [docs/MOD_STACK.md](../docs/MOD_STACK.md) | Drawings only |
