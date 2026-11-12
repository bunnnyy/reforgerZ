# The mission style: what's left

**Status: designed, not built.** Approved by the author on 2026-11-11. Everything else of the mission
style is built and described in [docs/DESIGN.md](../docs/DESIGN.md#story-blocks) (the story kit, ways,
checks, who decides, who benefits, truths, records, lies, fallout, reasons, route assets and the lock)
and [docs/MISSIONS.md](../docs/MISSIONS.md) (the six story jobs, the Tier 0 audit, the one-choice jobs
and the work rows). This file keeps only the parts not in code yet.

## Reasons

- **The majors' reasons** (`major.<act>`), posted by the majors as an act opens, for their stage missions
  and the margin cards to answer.

## The tribes and the conversion

- **The tribes' contracts on the kit:** Bring Them Home as a story job (find out who the named young was,
  then the ID card or keepsake to the family, or the fence buys it); Cull's choice (where the displaced
  pack goes, through the "where the horde drifts" block); Bastion's lost posts as a one-choice job (their
  tags to the nail, or left walking).
- **`walk.out` read by the tribes:** a man walked on feeds the nearest pack within 2 km as a named young.
- **Tier 0:** check tags on the audited choices that have none.

## Tricks (from the disguise design, whose own part is built)

- **A breaker:** a mission's sneak approach that spots someone passing as one of them is a catch.
- **A trick that lasts:** a stage that offers a trick sets `<key>.disguised` *(you)* for its later
  stages, and trick objectives sit at least 25 m from a leader's seat. A stage's paper is handed to the
  look with `RZ_DisguiseClaims.SetPaper` (built: [docs/REPUTATION.md](../docs/REPUTATION.md#borrowed-kit)).

## Log lines and tests

- Runtime test additions: each check result, each truth revealed and one kept after a restart, each
  beneficiary with an objection, a fallout errand, a record written and read, a callback, a reason
  starting a contract, a lie found out by each kind of event, a locked player refused another route's
  option.
