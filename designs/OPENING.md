# The opening: the Case

**Designed, not built** (the rest of it). Approved by the author on 2026-11-14 (hook B, the case;
the fixer is Red Hand). **Built:** beat 1 (the door asks who you were, a habit), beat 2 (the favour by
background and its other way: [DESIGN.md](../docs/DESIGN.md#openings-how-every-life-starts),
[habits](../docs/DESIGN.md#habits)), beat 3 (the line and its short form:
[DESIGN.md](../docs/DESIGN.md#the-line)), beat 4's case and the main story's door: the runner, the
case, its lead and its hand-overs, Proof's door ([DESIGN.md](../docs/DESIGN.md#the-case)), and beat 5,
the fixer, with his own shorter forms ([DESIGN.md](../docs/DESIGN.md#the-fixer)). What follows is
what waits: the shorter forms of the first beats.

The author's ask: the start should guide you into what's happening, one reason after another, and
end by putting you on the main story. It takes the structure of a good RPG opening (a personal hook,
favours that teach, a reveal, a side you pick, a lead with a name), never its characters, places or
words.

## The shape

Five beats, 25 to 35 minutes the first time, every one in the world and every one done for a person.
Each follows [MISSION_RULES.md](../docs/MISSION_RULES.md): the thing named stands there, real doors,
hand-overs by reply, one objective per step, the next step said in words.

| # | Beat | What happens | It teaches |
|---|---|---|---|
| 1 | **The door** | Built ([DESIGN.md](../docs/DESIGN.md#openings-how-every-life-starts)) | the talk screen, the tracker, replies |
| 2 | **The favour** | Built ([DESIGN.md](../docs/DESIGN.md#openings-how-every-life-starts)) | searching, stealth, door marks, the infected, your background's own skill |
| 3 | **The line** | Built ([DESIGN.md](../docs/DESIGN.md#the-line)) | the perimeter, the season's premise: nobody gets out |
| 4 | **The case** | Built ([DESIGN.md](../docs/DESIGN.md#the-case)) | the main story's thing, the factions by name |
| 5 | **The fixer comes to town** | Built ([DESIGN.md](../docs/DESIGN.md#the-fixer)) | sides, standing, grey choices, the town's people |

Then the lead to the station and the main story: built ([DESIGN.md](../docs/DESIGN.md#the-case)).

## Shorter for those who know it

| Level | The opening |
|---|---|
| 1, learning | All five beats |
| 2, familiar | The door (no questions), the favour, the line's short form (built), the case, Ferda as one visit (built) |
| 3, veteran | The door, the line's short form (built), the case, Ferda's note on the door (built); about 5 minutes |

## Many players

Each life's opening is its own: its own contact, favour, runner and case, near that player only.
Cases are capped like every sample (one live case per player per season counts as tissue). Ferda's
visit is built ([DESIGN.md](../docs/DESIGN.md#the-fixer)).

## What it uses

The openings' engine and the case as built, the walking contact, work sites and carrying, the dialogue engine's checks and
tags, the perimeter as built, the Red Hand's crews and the book, reputation's caps, the majors' Proof
and its participation by life, the finale's live tissue, the tracker.

## Order of work

1. Built: the case and the lead, the item and its tag, the station's reply, Proof's door (The Date
   on the Tag, Get It Back, The Case at the Post).
2. Built: beats 1 and 2, the questions, the habits, the favours by background with their ways
   through (`RZT_Favour.c` checks them).
3. Built: beat 3, the viewpoint, the runner shot in the warning zone, the short form (`RZT_Line.c`
   checks it).
4. Built: beat 5, Ferda's visit and its five ways, his later turns, his own shorter forms
   (`RZT_Ferda.c` checks them).
5. The shorter forms and the server checks (a fresh life reaches the case; each favour's thing is
   there). The case's own check is built (`RZT_Case.c`), and each way through Ferda's (`RZT_Ferda.c`).
