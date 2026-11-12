# Dialogue

**Status: designed, not built; waits for an engine check.** Approved by the author on 2026-11-11. Everything
else of dialogue is built and lives in [docs/DIALOGUE.md](../docs/DIALOGUE.md); this file keeps only the one
piece that needs the engine checked first.

## The text source from the addon

The server reading `.rzd` files from the addon at start ("plan A"), and server-only
`$profile:ReforgerZ/dialogue/*.rzd` overrides by conversation id, so a text fix needs no mod update and
twists can stay off clients' disks.

**Why it waits:** it needs `FileIO` to open files inside the addon on the dedicated server (VERIFY). If it
can't, the generated packs stay the one text source (as built), and only the `$profile` overrides would be
worth a runtime `.rzd` reader. Check it on the PC, then build the reader (the packer's parser, ported) with
the overrides on top.
