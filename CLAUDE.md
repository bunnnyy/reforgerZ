# ReforgerZ: project rules

A zombie survival game mode for Arma Reforger (Enfusion engine, Enforce Script),
set in a sealed government Quarantine Zone. Scripts live in `scripts/Game/ReforgerZ/`,
and design and setup docs in `docs/`.

**On the author's PC:** when asked to set up, compile, test or play-test there, follow
`docs/PC_SETUP.md` (also `/pc-setup`), starting with its rules.

## Docs must always match the project

Docs are part of every change, not an afterthought. **No commit that changes
behaviour, design, content or setup is complete until the docs match it.**

### On every change
1. **Update the docs in the same commit** as the code or design change:
   - New, changed or removed **system** → design doc + roadmap
   - New, changed or removed **mission or event** → `docs/MISSIONS.md` (✅ only when it's in code; note any simplification)
   - New **prefab, component, action or attribute** the user must set up → `docs/WORKBENCH_SETUP.md`
   - New or changed **screen, icon or widget name** a script looks up → `docs/UI.md` and `UI/LAYOUTS.md`
   - New, changed or removed **conversation** (`dialogue/**/*.rzd`) → `docs/DIALOGUE.md`'s list of conversations
   - New, changed or removed **door mark, scene, trace sound or light** → `docs/TRACES.md`
   - Change to **the week after the cure** (its phases, beats, carriers, closing work) → `docs/EPILOGUE.md`
   - New, changed or removed **Workshop dependency** → `docs/MOD_STACK.md`
   - Change to **setting or factions** → `docs/LORE.md` / `docs/REPUTATION.md`; to **tone or how text is written** → `docs/WRITING.md`
   - New or changed **tool, test script, Steam app, install step, folder or anything else the
     author's PC needs** → `docs/PC_SETUP.md` (and settle its "Not yet confirmed" rows when a session confirms them)
   - Anything that changes the **big picture** (features, status, doc list) → `README.md`
2. **Remove outdated content; don't mark it as outdated.** If a design was dropped
   (e.g. supply drops), delete it everywhere. Mention it only in the roadmap's
   "Dropped" list, with one line of reason.
3. **One source of truth per topic.** Don't explain the same thing in two docs.
   Link to the doc that owns the topic instead.

### Designs (`designs/`)
Approved designs that aren't built yet live in `designs/`, with an index in `designs/README.md` that also
gives the order of work. The designs that wait are listed there; these rules apply to every
one. Designs are not docs:
- A design file says "designed, not built" and never describes built behaviour.
- When a design (or part of one) is built, its content moves into the `docs/` file that owns the topic
  (Doc map below) or a new doc, and the design file is trimmed or deleted, **in the same commit as the code**.
- `docs/` may link to a design for what's planned, but never restates it.

### You may (and should) restructure docs
- **Add** a doc when a topic outgrows its section.
- **Merge** docs that overlap or are too small to stand alone.
- **Split** a doc that covers unrelated topics.
- **Delete** a doc whose content is gone or has moved. Fix every link to it.
- **Rename** for clarity. Fix every link.

When you restructure, update the **Doc map** below and the README's doc list in the same commit.

### Before committing, check
- [ ] A new or changed task meets `docs/MISSION_RULES.md` (its checklist); a rule's status there is current;
      `python3 tools/mission_rules.py --strict` ends `mission_rules: OK`.
- [ ] Every doc statement you touched matches the code. Docs say "planned" for anything not built.
- [ ] No doc mentions a dropped idea except the roadmap's "Dropped" list.
- [ ] No broken relative links (`grep -o '([^)]*\.md)' -r README.md docs/ designs/`).
- [ ] The Doc map below lists exactly the files in `docs/`.
- [ ] `VERIFY` count and prefab list in `docs/WORKBENCH_SETUP.md` are current.
- [ ] `docs/PC_SETUP.md` still matches the tools, scripts and setup steps you touched.
- [ ] Everything you added works with a gamepad (`docs/UI.md`, "Controllers").

## Doc map

The Doc map covers `docs/` only. Approved but unbuilt designs are in `designs/` (see above).

| File | Owns |
|---|---|
| `README.md` | What the mod is, feature summary, status, the doc list, the code tree |
| `docs/LORE.md` | Setting (the sealed Zone), the factions as people, endings |
| `docs/DESIGN.md` | How every system works, organised by system (simplifications marked inline) |
| `docs/DIALOGUE.md` | Talking to people: the talk screen and its engine, conversation trees, checks and tags, effects, memory and `dialogue.json`, conditions, the `.rzd` format, its lint and simulator, the conversations |
| `docs/INFECTED.md` | The infected: the spawner, the towns' crawlers, loose ones, heat hordes, infection adapts, the land grid, bodies and the agent budget, the tribes (phases 1–5 built), the behaviour layer |
| `docs/TRACES.md` | Traces, the world's environmental storytelling: door marks (the grammar, the symbols, reading by level, where marks come from, ageing, doors on the map), doors you can see (decals, boards), player traces, the scenes (the first days and the line, vignettes, live traces), sound and light, the cure and the epilogue, errands' old mark, the Long Table's absence, caps, its decisions |
| `docs/PLACES.md` | Task places and things: the mainland (no island gets anything), a place is what its words say (the name rule and the words by what's there), the needs (place flags, the things a task's words name and their slots), the needs table, how pickers use them (`RZ_PlaceFind`), the live check and its log line, the place audit (`run.sh tasks`, its log lines and drawing), results, what isn't covered |
| `docs/SITES.md` | Camp sites: the site grid, real footprints (`footprints.json`, the measuring plugin), drop don't squeeze, the surroundings and the threat sectors, the site audit (its checks, score and log lines, `run.sh sites`), `render.py --placed` and the fly-to plugin, the world-building kit (`world_kit.txt`), hand-built sites (the site builder, the saves, role spots and the checklist, pinning, `tools/hand_built.py`) |
| `docs/ROADMAP.md` | Built, next, play-test priorities, and the Dropped list |
| `docs/REPUTATION.md` | Factions, Fame/Infamy/Favor, tiers, shops, perks, leaders, AI hostility, morale/needs/truces |
| `docs/MISSION_RULES.md` | The rules every task follows (place, people, doing it, talk and hand-overs, objectives and the tracker), each with the doc that owns it, how it's kept and its status; the checklist for a new task |
| `docs/MISSIONS.md` | Every contract, world event, puzzle, leader and arc mission, with requirements and ✅ status |
| `docs/ERRANDS.md` | Errands: the givers (named people, the wanderers' asks), the dealer, keeping them true, ways and steps, pay, the faults table, every routine and card with ✅, the duties (always there to be asked for), the trust ladder, the sorting duties played at their place and the duties' people, telemetry |
| `docs/QUESTLINES.md` | The four majors (acts, defaults, endings) and the long questlines (three per faction, two for the Long Table): stages, approaches, endings, crossings |
| `docs/FINALE.md` | The cure's finale: the gate, the lock and price doors, the five routes, live tissue, pacts and kills between routes, the finish, the batch and the seal, the stand as a fight for the clean room, Take the Lab and the Coat Run, terms, the route Fame, fallbacks, items and people |
| `docs/EPILOGUE.md` | The epilogue week after the cure: its phases, the record and the hall's freeze, beats and carriers, the dose outlets, places and scenes, the grey choices, the closing work (the closure set), names, edge cases, admin tools |
| `docs/MOD_STACK.md` | Workshop dependencies and why |
| `docs/PC_SETUP.md` | The author's Windows PC, run by Claude Code: rules, Steam apps and paths, the repo, opening Workbench, local compile check and runtime test, play-testing, logs |
| `docs/UI.md` | Art direction, the icon set, the screens, the UI scripts and data feeds, built vs planned (layout trees: `UI/LAYOUTS.md`) |
| `docs/WORKBENCH_SETUP.md` | Turning the scripts into a playable mod: dependencies, factions, prefabs, settings, VERIFY list, the compile check, text lint and test tools, log lines, play-test checklist |
| `docs/WRITING.md` | How player text is written: tone, the voices, by channel, the season mood and the voice bank, conversation lines, grey choices, phrasing, the humour line, template rules, and what the text lint checks |

## Writing style for docs
- Plain, clear English, written for the mod author. Short sentences, tables for lists of things.
- Player-facing text (broadcasts, notes, mission briefs) follows `docs/WRITING.md`.
- Say honestly what's built, what's simplified and what's only designed.

## Code rules (Enforce Script)

The game scripts compile on the dedicated server (below); the Workbench plugin only in Workbench.
Write conservatively:
- No ternary operator, no C-style casts (use `Type.Cast(x)`, `RZ_Util.RoundToInt`).
- `out` is a reserved word, so don't use it as a name.
- Don't reuse a local variable name twice in one function, even in sibling blocks.
- Empty virtual methods get `{}`, not `;`. `string.Format` supports `%1`–`%9` only.
- `ref` for owned objects; engine entities (`IEntity`) as weak references.
- Never give an `[Attribute]` field an initialiser: it overrides the value set in Workbench (tested).
  The attribute's default string is the only default. To create an attribute class in code, use
  `RZ_Util.NewWithDefaults("ClassName")`, not `new` (which leaves every field at zero).
- Mark any engine API you aren't sure of with `// VERIFY: ...` and keep engine calls in small helpers.
- Server-authoritative. Poll on the existing tick, not per frame. Spawn AI and props only near players.
- Works on any map: place things relative to settlements and landmarks, never fixed coordinates.
  The one exception: hand-built sites, which the author builds in Game Master and saves in a map's
  own coordinates (`World/HandBuilt/`, docs/SITES.md). Code never depends on them: a site without
  one, and every map without saves, is laid out as before.
- Optional prefabs are attributes on `RZ_GameModeComponent`; features that need one turn themselves off when it's empty.
- Run `tools/compile_check.sh` before committing script changes: it compiles everything on the
  dedicated server (Linux in the cloud; on the author's PC in Git Bash, see `docs/PC_SETUP.md`) and
  must exit 0 with no ReforgerZ errors or warnings.
- Script folders are lowercase `scripts/` (the Linux server is case-sensitive).
- Conversations are `.rzd` files in `dialogue/`: after changing one, run `python3 tools/dialogue_pack.py`
  (the dialogue lint; it writes the generated packs in `Dialogue/Pack/`, never edited by hand) and
  `--lock` when new ids ship (`docs/DIALOGUE.md`).
- Run `python3 tools/mission_rules.py --strict` after adding or changing a task: it must end
  `mission_rules: OK` (what it checks: `docs/WORKBENCH_SETUP.md`, "Mission rules check").
- Run `python3 tools/voice_lint.py` after changing player text: it must print `voice_lint: OK`
  (the text lint: what it checks is in `docs/WRITING.md`, how to run it in `docs/WORKBENCH_SETUP.md`).
- After changes to missions or cross-system logic, run `tools/runtime_test/run.sh` (60–90 min)
  and fix any script exception it reports.

## Design guardrails (agreed with the author)
- **Sealed Zone:** no supply drops, no outside aid, no visible wall. The perimeter is drones,
  loudspeakers, unseen snipers and fully sealed road checkpoints.
- **No sleep and no time skip.**
- **Nothing overpowered:** faction strongholds never fall, effects are capped and recover,
  adaptation is light, nobody gets free loot or free Fame.
- **Performance first:** simulate as data; only spawn bodies near players; no persistent physical traces.
- Most of the map is abandoned. Factions hold only a few towns that make sense for them.
- **Default first, then menus:** use as much of the game's own stuff as possible (vanilla's inventory,
  looting, healing, doors, fuel, vehicles, radios), and menus as much as possible, even ones we have to
  make; a single world prompt only for what's done quickly in a rush. The rule and the audit: `docs/UI.md`,
  "Default first, then menus".
- **Games are played in the world:** anything that is a game (sorting the dead, a patrol's find, a
  watch) is done in person, with your hands on things that stand at the place, never as a menu. A
  menu may be one prop of the game when it's itself something to handle (a clipboard, a ledger), never
  the whole game. The rule: `docs/UI.md`, "Default first, then menus".
- **Console and controller, for the whole mod:** everything works on console (Xbox, PlayStation)
  and with a gamepad, not only with keyboard and mouse. Every input action has a gamepad binding;
  every screen opens, moves and closes without a mouse; nothing needs hover, typing or a chat
  command to reach it; text stays readable on a TV. The rules in full: `docs/UI.md`, "Controllers".

## Git
- Work on the branch you're given. Commit in logical chunks. Never force-push.
