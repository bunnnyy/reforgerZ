# The opening: the Case

**Designed, not built** (the rest of it). Approved by the author on 2026-11-14 (hook B, the case;
the fixer is Red Hand). **Built:** beat 1 (the door asks who you were, a habit), beat 2 (the favour by
background and its other way: [DESIGN.md](../docs/DESIGN.md#openings-how-every-life-starts),
[habits](../docs/DESIGN.md#habits)), beat 3 (the line and its short form:
[DESIGN.md](../docs/DESIGN.md#the-line)) and beat 4's case and the main story's door: the runner, the
case, its lead and its hand-overs, Proof's door ([DESIGN.md](../docs/DESIGN.md#the-case)). What follows
is what waits: beat 5 (the fixer) and the rest of the shorter forms.

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
| 5 | **The fixer comes to town** | Back in town, a Red Hand crew is asking who picked it up. You pick a side, several ways (below) | sides, standing, grey choices, the town's people |

Then the lead to the station and the main story: built ([DESIGN.md](../docs/DESIGN.md#the-case)).

## Beat 5: the fixer

A Red Hand crew of three comes into the start town 2 to 4 minutes after you return, led by
**Ferda Kroupa**, who collects for the book. Polite, patient, never in a hurry. He knows the runner
was carrying something, and that someone from this street picked it up. He asks the contact first.

| Way | How | Standing (small, through the caps) |
|---|---|---|
| **Hand it over** | He pays well (tins, a favour in the book). The street thinks less of you | Red Hand +, Haven − |
| **Talk him off** | A check on your background or the liar habit: *"Bastion's patrol is two streets away."* He goes, for now | none; he remembers |
| **Rally the street** | Three neighbours each need one thing first: a door boarded (a real door), a rifle cleaned (a work site), a lookout posted (a watch). Then you stand together and he backs off; if anyone fires, it's a fight | Haven +, Red Hand − |
| **Slip away** | Out the back with the case. The street takes his visit | Haven −; he remembers |
| **Sell it, then warn the street** | Take his tins, then tell the street he's coming back for more | Red Hand + then −, Haven ± |

Every way is a reply, a work site or a fight in the world, never a menu choice. The radio mentions
what you did once, that evening.

**Selling to him.** "Hand it over" is the built sale (a Red Hand person buys it today, [DESIGN.md](../docs/DESIGN.md#the-case)) moved to Ferda: Get It Back then takes it from his lock-up.

**Ferda afterwards.** He's the opening's thread into the season: a note on your door, his men at a
toll asking after you, his voice once on the crews' channel. He wants the case, or what it was worth.
He never hunts a player (no spawned squad follows you); he turns up where the Red Hand already is.

## Shorter for those who know it

| Level | The opening |
|---|---|
| 1, learning | All five beats |
| 2, familiar | The door (no questions), the favour, the line's short form (built), the case, Ferda as one visit |
| 3, veteran | The door, the line's short form (built), the case, Ferda's note on the door; about 5 minutes |

## Many players

Each life's opening is its own: its own contact, favour, runner and case, near that player only.
Ferda's crew comes for one player at a time; two players' openings in one town share his visit.
Cases are capped like every sample (one live case per player per season counts as tissue).

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
4. Beat 5: Ferda's visit and its five ways; his later turns.
5. The shorter forms and the server checks (a fresh life reaches the case; each favour's thing is
   there; each way through Ferda ends). The case's own check is built (`RZT_Case.c`).
