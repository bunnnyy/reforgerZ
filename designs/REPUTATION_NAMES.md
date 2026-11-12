# Reputation names: the mixed picture

**Status: designed, not built.** Approved by the author on 2026-11-11. The 16 names, the rules underneath, the
voices, the Fear check, the parley and the knock, settling up, gates and the company you keep, gate barks and
others hear, the word and crew names, deeds, the whole picture, face and name, the lock and the season record,
Make it right and the last week's hooks are built and live in [docs/REPUTATION.md](../docs/REPUTATION.md). This
file keeps only what needs an engine check or another session's code, with the reason. The section numbers
other files cite are kept.

## 6.1 The quiet success on the questlines' FORCE stages

Each built FORCE stage (Read the Page, Collect, The Driver, Meet the Driver, and any stage written with a
threat) sets `m_iMenaceDifficulty` in its Setup and writes its quiet outcome: the stage's better outcome
(nobody tells the crew, the camp isn't on guard); below it, the built outcome, still recorded as FORCE. The
helper (`RZ_Ways.TryMenace`) is built. *Why it waits:* the stages are `Missions/Questlines/` code (the
mission-style session's). With them: **Reputation > Quiet success** as a setting.

## 9. Smoke, a crew name

**Smoke**: threw a signal smoke. *Why it waits (engine check):* nothing tells the server that a player threw a
smoke grenade (VERIFY: a throw or a smoke-grenade event the server sees).

## 10. Deed phrases from missions

Phrases (`m_sDeed`) for the contract classes that still use *"did a job for us near {town}"*, and a few words
for each questline's and major's endings. *Why it waits:* they're set on each mission and stage, in
`Missions/` (the mission-style session's). War deeds between locked players are the finale's (`NoteWar` from its
kill hook, [FINALE.md](FINALE.md)).

## 17. The standing page and the notices

- **The standing page's header** carries the whole picture (`H` in the feed), and **`LeadersN`'s first two
  lines** are the word and the nearest met leader's verdict ("The crews call you Lucky."). The feed sends the
  word and the owed note already. *Why it waits:* the page is the notebook's client code (`UI/RZ_Notebook.c`, the
  UI session's).
- **Notices, one per approach:** the gate's own line first (with the bark folded in), then a change notice, then
  a tip; anything lower waits until 30 s after the last, and is dropped past 2 minutes. *Why it waits:* it's a
  rule of the shared notice lanes (`UI/RZ_NoticeLanes.c`, the UI session's). Until then a gate bark is its own
  subtitle in the approach lane, once per place per session.
- **The journal's line for a parley's job:** "Amends: works off Infamy · finishes where the job says". *Why it
  waits:* a reserved job's journal text is the mission's (`Missions/`).
