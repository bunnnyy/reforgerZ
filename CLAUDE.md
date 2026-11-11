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
   - New, changed or removed **Workshop dependency** → `docs/MOD_STACK.md`
   - Change to **setting, tone or factions** → `docs/LORE.md` / `docs/REPUTATION.md`
   - New or changed **tool, test script, Steam app, install step, folder or anything else the
     author's PC needs** → `docs/PC_SETUP.md` (and settle its "Not yet confirmed" rows when a session confirms them)
   - Anything that changes the **big picture** (features, status, doc list) → `README.md`
2. **Remove outdated content; don't mark it as outdated.** If a design was dropped
   (e.g. supply drops), delete it everywhere. Mention it only in the roadmap's
   "Dropped" list, with one line of reason.
3. **One source of truth per topic.** Don't explain the same thing in two docs.
   Link to the doc that owns the topic instead.

### Designs (`designs/`)
Approved designs that aren't built yet live in `designs/` (index and build steps: `designs/README.md`,
order of work: `designs/PLAN.md`). They are not docs:
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
| `docs/LORE.md` | Setting (the sealed Zone), the factions as people, tone, writing guide, endings |
| `docs/DESIGN.md` | How every system works, organised by system (simplifications marked inline) |
| `docs/ROADMAP.md` | Built, next, play-test priorities, and the Dropped list |
| `docs/REPUTATION.md` | Factions, Fame/Infamy/Favor, tiers, shops, perks, leaders, AI hostility, morale/needs/truces |
| `docs/MISSIONS.md` | Every contract, world event, puzzle, leader and arc mission, with requirements and ✅ status |
| `docs/QUESTLINES.md` | The long questlines (three per faction, two for the Long Table): stages, approaches, endings, crossings |
| `docs/MOD_STACK.md` | Workshop dependencies and why |
| `docs/PC_SETUP.md` | The author's Windows PC, run by Claude Code: rules, Steam apps and paths, the repo, opening Workbench, local compile check and runtime test, play-testing, logs |
| `docs/UI.md` | Art direction, the icon set, the screens, the UI scripts and data feeds, built vs planned (layout trees: `UI/LAYOUTS.md`) |
| `docs/WORKBENCH_SETUP.md` | Turning the scripts into a playable mod: dependencies, factions, prefabs, settings, VERIFY list, log lines, play-test checklist |

## Writing style for docs
- Plain, clear English, written for the mod author. Short sentences, tables for lists of things.
- Player-facing text (broadcasts, notes, mission briefs) follows the writing guide in `docs/LORE.md`.
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
- Optional prefabs are attributes on `RZ_GameModeComponent`; features that need one turn themselves off when it's empty.
- Run `tools/compile_check.sh` before committing script changes: it compiles everything on the
  dedicated server (Linux in the cloud; on the author's PC in Git Bash, see `docs/PC_SETUP.md`) and
  must exit 0 with no ReforgerZ errors or warnings.
- Script folders are lowercase `scripts/` (the Linux server is case-sensitive).
- Run `python3 tools/voice_lint.py` after changing player text: it must print `voice_lint: OK`
  (the text lint; see `docs/WORKBENCH_SETUP.md`, "Text lint").
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
- **Console and controller, for the whole mod:** everything works on console (Xbox, PlayStation)
  and with a gamepad, not only with keyboard and mouse. Every input action has a gamepad binding;
  every screen opens, moves and closes without a mouse; nothing needs hover, typing or a chat
  command to reach it; text stays readable on a TV. The rules in full: `docs/UI.md`, "Controllers".

## Git
- Work on the branch you're given. Commit in logical chunks. Never force-push.
