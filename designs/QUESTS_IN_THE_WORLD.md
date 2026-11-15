# Quests in the world: every stage saved, every way chosen by doing it

**Designed, not built.** Approved by the author 2026-11-15 ("go ahead, build both"), from play-testing:

- *"Every stage of a quest/errand should save its progress."* A death, a logout, a crash or a server
  restart never costs a quest, an errand or a job, nor where you were in it.
- *"Shouldn't those decisions be made in game rather than just dialogue? Like the errand had 3 sub
  optional objectives like Fallout: New Vegas and you can choose to do either one. This should be for
  all quests where there's multiple clear objectives/ways to do things."*
- *"The dialogue and the text here is really confusing and not clear what the context is and what the
  decisions I do affect."* (Local Law's well: three buttons with one line of context.)
- *"I just ran 3 km for no interaction really, just to press a button. That isn't fun or interesting
  or interactive or dynamic."*
- *"Every quest or errand should have some meaningful interaction with some event or something, with
  lots of context that is clear! A lot of the dialogue is written in a strange confusing way."*

## 1. Every stage saved

| What | Saved (in `state.json`) | Coming back |
|---|---|---|
| A held errand | Its def key, owner (identity and character), giver (key, name, title, faction, spot), way (or *none yet*), every place, its text slots, step and first-step progress, pay modifiers, time left (in real seconds, not the dealer's clock) | Rebuilt on load and bound to its owner when they join; its work site, prop and carry re-opened where they stood; a duty's game ends unpaid (as at a death) |
| The opening | Its plan, stage and step, the favour and its place, whether the background's way was taken, the habit, the end faction and place, the contact's name and words | Resumed, never restarted: a contact at a door near where you come back says where you left off ("You're back. The battery?"), and the current stage's step is handed again; the line's scene comes back as its short form |
| A mission or contract you took | Its type, giver, who took it (identities), its place and town, time left, and how far its objectives got | Offered again to you alone by its giver if it can't be rebuilt where it stood; a questline stage already comes back this way |
| The tracker | The tracked job (already saved) | Points at the rebuilt job |

A death keeps everything above too (it already keeps held errands): the opening goes on after you come
round, from its stage.

## 2. Ways chosen by doing them

A job with more than one clear way lists **every way at once** as optional objectives, New Vegas
style, all live in the world; **the first one you finish settles it** and the others close.

- **Tracker and Quests tab:** the ways as optional lines under the job (*◇ Wake the private asleep in a
  car, 360 m SE*, *◇ Stand his post yourself at the gate*, *◇ Report him to the corporal*), each
  with its place, each with its arrow when tracked.
- **Taking a job is one reply** (*[Errand] Shift Change*, then *I'll see to it.*): the giver says what's
  wrong and what each way means for whom, then every way opens.
- **Ways done in talk stay talk** (persuade, lie, pay, report to someone): they are a reply at the
  person the way names, open from the start, shown in the tracker like the others (*◇ Tell the
  corporal he was asleep*).
- **Costs move to the doing:** items, Favor and checks are asked when you do that way, not when you
  take the job.
- **The opening:** the favour and the background's own way are both open from the start; whichever
  you finish first counts.
- **Mission choices** (`RZ_OneChoice`, the well, every "A or B" ending): not a menu. The sides are
  people standing there, and you settle it by what you do with your hands or a reply to one of them
  (hand the rota to the Novaks, nail the new rota on the board, tell both to share): each a way as
  above.

## 3. Every choice says what it's about and what it changes

- **Context first:** before any choice, the people involved say who they are and what they want, in
  their own voices (the Novaks: their children, the Vogels: their cows), and the giver says why
  Bastion cares.
- **Stakes shown:** each way names whom it pleases and whom it angers, in plain words and with the
  factions' marks (*The Novaks will remember. The Vogels won't forgive it.*, *Haven +, the street −*).
- **Never a lone menu at the end of a run:** a place a job sends you to has something to do or someone
  to deal with there (the families at the well, the pump, the rota on the board). A job with only a
  button at the far end gets people and a scene, or its place comes nearer.

## 4. Every job has an event at its place

- **Something happens where it sends you:** every errand and quest has at least one meaningful
  interaction at its place: people with a stake, something that goes on while you're there (an
  argument that heats up, infected drawn by the noise, a rival who arrives), or a find that changes
  the job. A job whose place has nothing gets one; a step that is only a button gets a reason.
- **The rest of the writing pass:** the errand cards, contract and quest pitches and briefs,
  questline stages and the givers' conversations, rewritten to the rule in
  [WRITING.md](../docs/WRITING.md#plain-words) (the opening, every job's choice, the errand routines
  and the duties are done).

## Order of building

1. Held errands saved and rebuilt (`RZ_ErrandsSave.held`, rebinding owners on join).
2. The opening saved and resumed from its stage (also after a death).
3. Errands' ways all open at once, settled in the world; talk ways as replies at their person.
4. The opening's favour and background way open together.
5. Missions' choices (`RZ_OneChoice` and the talk scenes' DECIDE) as ways at people, with context and
   stakes.
6. Missions and contracts saved (offered again to their taker when they can't be rebuilt).
7. Events at every job's place, and the rest of the writing pass.
