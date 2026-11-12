# Dialogue

**Status: designed, not built.** Approved by the author on 2026-11-11. Everything else of dialogue is built
and lives in [docs/DIALOGUE.md](../docs/DIALOGUE.md); this file keeps only what's still to build.

## The rest of the format

- **Keywords:** scene scopes `run` and `house`; `bind <name> = <picker>` (a value picked once in a session,
  such as `town:thick_within(1500)` or `house:nearest_short`, kept with claims, promises and callbacks) and
  its tokens `{bind:name}` and `{town:@name}`; `use <conv id>` (one conversation's nodes in another's);
  `say (<role>)` for someone else in the scene; the node option `shared`; the choice options `tell always`
  (a lie that's always caught, for a joke lie) and `found <effects>` (run when a lie is found out, only on
  things still running: a passage revoked, a sway taken out; never on Fame paid or a finished mission).
- **Tokens:** `{claim:key}` (what you told this speaker).
- **World predicates:** a lie's truth checked when the player gets there (`town:thick=@target` is true the
  moment they walk into the bound town while its crawlers are thick).
- **Lint:** a `truth` that reads what the player can't know; a lie without `truth` or `tell always`; an
  unscoped season flag in a conversation that serves many speakers; a flag outside its owner's namespace;
  Halcyon's long trees without numbers.
- **Simulator:** `--chars ... --race` for several characters in a scene (holds, objections, tallies).
- **Runtime test:** open every conversation's start node with a fake player, and run each lie, promise and
  callback once. `/talkreset <scene>` (admins), with an [Admin] reply for the pad.
- **The rumour tick** (15%) as a setting.

## The text source (needs an engine check)

The server reading `.rzd` files from the addon ("plan A") and server-only `$profile:ReforgerZ/dialogue/*.rzd`
overrides by conversation id, so a text fix needs no mod update and twists can stay off clients' disks.
**Engine check first:** `FileIO` on addon files on the dedicated server (VERIFY). Until then the generated
packs are the one text source.
