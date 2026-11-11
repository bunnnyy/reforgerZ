# Errands

**Status: designed; phases 3, 4 and 8 built.** Approved by the author on 2026-11-11.
Built and described in [docs/ERRANDS.md](../docs/ERRANDS.md): the givers, the dealer's asked and
called-out offers, soft holds and overtaking, the faults table, the small-pay ledger, 34 routines, the
wanderers' asks, 63 cards (with the first margin cards, the lock card and the disguise side jobs), fallout errands, strangers
at every spot kind (18 stranger routines and found hooks), locals, twists, and board and radio asks;
the lie store and the witness rule are in [docs/DESIGN.md](../docs/DESIGN.md#lies-and-witnesses). This
file keeps what's still to build: the epilogue cards, the other majors' margin cards, promises,
callbacks and threads from errands, His Effects in full, chore slots, and the parts of the built pieces
listed as still to build below.

## What changed in review

Conflict numbers are the plan's, counted from 1 (as in [FINALE.md](FINALE.md) and
[INFECTED.md](INFECTED.md)). Decisions are numbered as in the plan's "Decisions for you". The
alternatives that were not chosen are gone.

- **Strangers find you only where it makes sense** (decision 14, changed by the author). A stranger is
  never dealt on a timer and never in the empty middle of nowhere. The game finds the places a stranger
  would really be (a road or crossroads people use, a wanderer's route, a holding's approach, a
  campfire, a stopped car, a fresh event or a body, a town edge at dusk) from world facts, and deals
  one only there. The 15–25 minute gap and the interruption budget stay, as a ceiling only. See
  [the spots](../docs/ERRANDS.md#strangers) (built).
- **One side-quest system** (the New Vegas plan, section 5). Errands now hold every small job:
  missions' 25 odd jobs become routines; missions' found hooks and fallout jobs, reputation's 11
  seeds and disguise's 10 side jobs become cards; dialogue's asks and strangers are how they're
  spoken, and its stranger kinds become stranger routines; the epilogue's last errands are the epilogue
  deck, run by this dealer ([EPILOGUE.md](EPILOGUE.md) owns their content). Missions' reasons are the
  Why. The plan's duplicates are merged and its premise collisions cut or re-angled
  ([The catalogue](#the-catalogue)).
- **Folded-in jobs pay by errands' rules**: Fame at most 2 an errand, Favor 3–10, the hourly ledger.
  Their source designs' bigger numbers (+3 to +5 Fame) are cut to fit. Their hidden percentage rolls
  ("35% it comes out") are now lie records found by events.
- **Words** (plan §5): the hand-written layer is **cards** (was "stories"); remembered people are
  **locals** (was "faces"); "face" now means only recognition.
- **Bastion lines** (conflict 38): the boxes are kept *"for when the gates open"*, never "for when relief
  comes"; the mother hears *"He's on the roster as KIA, or struck off."*
- **The finale's reads** (conflict 39): `finale.route` 0 is nobody (handed over), the lab alone is
  `finale.finisher` 0, and every lock is read with `RouteOf(identity)`.
- **One dose-price table** (conflict 40): What the Cure Costs Here reads the epilogue's outlet table; its
  fixed "15 RH Favor" is gone.
- **Hand to Hand** (conflict 22) eases only the eight's yes and shortens the Independent's lab-gate
  sit-in. It never eases a stronghold leader and never moves a track.
- **Tribes replace Grounds** (conflicts 42 and 43): every ground card now reads tribes
  ([INFECTED.md](INFECTED.md)), with each faction's own words in player text.
- **The epilogue lasts one week** (decision 12). The ending contract runs for the week, and errands feed
  the closing work that lets everything finish inside it
  ([After the cure: the epilogue week](#after-the-cure-the-epilogue-week)).
- **Dialogue is off by default** (decision 13), so the first version is the action-menu fallback.
- **After the lock** (decision 8): errands for the finale's open acts (I and III) are dealt to anyone not
  locked elsewhere.
- **Shared pieces replace errands' own copies:** the owner-only personal payload replaces the "talk lane
  in `NotifyPlayer`" (and fixes the built leak); follow-ups go into the one callback queue; lies go into
  the one lie store; "if seen" is the one witness helper; vouching is the one vouch; amends go through
  `Forgive()`; disguises follow [REPUTATION.md](../docs/REPUTATION.md#borrowed-kit).
- **The Long Table:** The Jar Run's joke becomes silence; no stranger within 2 km of the farm;
  Processing ends at the queue, never at the barrier.
- **Lore fixes:** a dead Red Hand man's kit goes to the fence (conflict 33); crew names are what the crew
  holds on you (conflict 31); Bastion's tribe list is "the contact board" (conflict 48).
- **Leaders never give errands**, so the dialogue design's council-head, commander and crew-boss asks
  now come from named people.
- **Groups and strangers:** a stranger is dealt when no player *outside your vanilla group* is within
  300 m, so friends travelling together still meet them.
- **Card names match the sibling designs:** Notebook 31 (with His Notebook and Into the Record), Unknown
  Face, A Debt Gone Quiet (Collections, re-angled), Civvies (re-angled to the clothes only), Paint (with
  Name Day's Brush), as in [REPUTATION.md](../docs/REPUTATION.md#borrowed-kit) and [REPUTATION_NAMES.md](REPUTATION_NAMES.md). The
  Bastion private's card is **Off the Report**, because "the board" is the roster and names never leave
  it.

---

## Contents

1. [The idea](#1-the-idea)
2. [Two layers: routines and cards](#2-two-layers-routines-and-cards)
3. [Card parts](#3-card-parts)
4. [Where errands come from](#4-where-errands-come-from)
5. [Givers](#5-givers)
6. [The dealer](#6-the-dealer)
7. [Facts, reasons and signals](#7-facts-reasons-and-signals)
8. [Keeping them true](#8-keeping-them-true)
9. [Size, distance and clocks](#9-size-distance-and-clocks)
10. [Ways, checks and talk](#10-ways-checks-and-talk)
11. [Givers who lie or hide things](#11-givers-who-lie-or-hide-things)
12. [Lies, witnesses and knowing](#12-lies-witnesses-and-knowing)
13. [Failure is a branch](#13-failure-is-a-branch)
14. [Pay: nothing for free](#14-pay-nothing-for-free)
15. [Who asks, and who gets asked](#15-who-asks-and-who-gets-asked)
16. [Disguises](#16-disguises)
17. [Chores at holdings](#17-chores-at-holdings)
18. [Strangers](#18-strangers)
19. [Locals, callbacks and threads](#19-locals-callbacks-and-threads)
20. [The big stories, the finale and the lock](#20-the-big-stories-the-finale-and-the-lock)
21. [The tribes](#21-the-tribes)
22. [After the cure: the epilogue week](#after-the-cure-the-epilogue-week)
23. [Multiplayer, death and griefing](#23-multiplayer-death-and-griefing)
24. [Cheap](#24-cheap)
25. [The catalogue](#the-catalogue)
26. [One evening in Levie](#one-evening-in-levie)
27. [Settings, telemetry, writing, phases](#27-settings-telemetry-writing-phases)
28. [Engine needs](#28-engine-needs)
29. [Risks](#29-risks)

---

Code and docs call them **errands**. The code lives in `Missions/Errands/`:
- `RZ_Errands.c`, the dealer;
- `RZ_Errand.c`, the base class;
- `RZ_ErrandRoutines.c`, the routine templates;
- `RZ_StrangerSpots.c`, the stranger-place finder;
- `Cards/<Family>/`, the cards;
- `RZ_ErrandRegistry.c`.

Settings go under **Missions > Errands**. DESIGN.md gets an "Errands" section (how it works), and
MISSIONS.md an "Errands" table (every routine and card, with ✅). Nobody in the Zone calls them errands:

| Faction | Calls one |
|---|---|
| The Haven | "do us a turn" |
| Bastion | "a detail" |
| The Red Hand | "a little something" |
| Halcyon | "a small request" |
| A stranger | "Got a minute?" |

## 1. The idea

- **Contracts** are the factions' work, put out on the radio ([MISSION_STYLE.md](MISSION_STYLE.md)).
- **Questlines, majors and the finale** are the season's long stories.
- **Errands** are everything in between: small, personal, offered in person, and true.

Each errand exists because something in the world is true right now, here, for you. It ends the moment
that stops being true. Most take 5–15 minutes and have two or three ways through. Many leave someone
behind who remembers you.

| | Contract | Leader job | Questline / major / finale | **Errand** |
|---|---|---|---|---|
| Offered by | the faction, by radio or quartermaster | an Independent or Rogue leader | a stage | **one named person, in person** (never a leader) |
| Offered to | everyone who qualifies | who the leader likes | everyone who qualifies | **one player, and their helpers** |
| Size | 10–40 min | 10–30 min | many sessions | **5–15 min, 1–3 steps, within 1.5 km** |
| Exists because | weights, needs, reasons | traits, loyalty | the story | **one live fact** |
| Pays | Fame, Favor, crates | Favor, Fame | stage pay | **a little Favor or an effect in kind; Fame up to a ceiling; memory** |
| Changes | strength, territory | loyalty, food | endings | **one small, capped thing** |

Errands are never stages and never on the map. Every one has a way through without a gun. Errands
spawn no AI groups of their own except a few infected through `SpawnGuards`, no crates and no new
items.

## 2. Two layers: routines and cards

The season runs about 45 days. Seven regulars doing 4–6 errands a session is about 1,300–1,900
errands a season. Hand-written cards with season counts can't supply that, so there are two layers.

| | **Routines** | **Cards** |
|---|---|---|
| What | data-driven templates: holdings' chores, strangers' small asks, house asks on boards, wanderers' asks | hand-written, each with its own scene, ways and twist |
| Repeat | yes, on cooldowns: per player per place 1 real day; per player anywhere 3 real hours; per place 20 min | **once per player per season**, keyed to the player, not the character |
| Variety | slots (who, what, where, why variant, twist variant) and three line variants per beat | written per card |
| Count | 30 templates in the first phase, about 600 versions with slots, about 40 with every source folded in | 16 in the first phase, 40 in the second, about 60 in the third |
| Share | about 3 of every 4 errands | at most 2 a player a real day; the dealer prefers one whenever its Why holds |

**Supply check** (7 regulars, 45 days):
- Cards: 60, each player qualifying for about 40 of them, at 2 a day at most. They last 20+ days per
  player, and the deck turns with the stages, so new families keep appearing.
- Routines: about 12 holdings × up to 3 chore slots × 20-minute place cooldowns, plus strangers wherever
  a stranger makes sense. That's several hundred true offers a day. Whys gate them; supply doesn't.

"When nothing is true near you, nobody asks" still holds. Routines are true too: the pot really is thin,
and the relief really is missing.

## 3. Card parts

Every routine and card declares the same parts:

| Part | Meaning | Example: The Pot |
|---|---|---|
| **Why** | live facts that must hold, usually a reason ([MISSION_STYLE.md](MISSION_STYLE.md)) | `house.hungry`: `House(idx)` food under one upkeep; no Haven truck for it due within 20 real min |
| **Where** | places found from the Why, all within 1.5 km of the offer | the kitchen; a farm building in the nearest empty village (≤ 1.2 km) |
| **Who** | a giver kind (§5), or a stranger and the spot kinds it fits (§18) | the house's cook |
| **For whom** | standing, background, title halves, life state | not exiled; under 15 Haven Infamy for Fame (else amends) |
| **Premise** | subjects and people it shares with bigger content (§8) | `house:idx` food, routine |
| **Still** | re-checked every 30 s and at each step | still short; the cook's body seat still exists; still the Haven's house |
| **Ways** | 2–4 ways through | |
| **Without you** | what the world does without you: never more than the owning system already does | nothing extra |
| **Twists** | optional, varying per deal (§11) | none |
| **Size** | the furthest point, the longest wait, escort or not | 1.2 km, no wait, no escort |

**Real faults, not dice.** Some routines need a small fault: the relief sentry missing, the radio set
low, the freezer fuel low, a sensor out, the tag receiver down. These live in a **faults table** on the
holding, as real state other systems show:
- a low set makes that post's nightly numbers a hiss on Bastion's net for everyone;
- a sensor or receiver out is a line on Halcyon's secure channel;
- a missing relief shows as one guard fewer at the gate.

A fault is rolled at most once per shift (05:30/17:30) per holding, and only while a player is within
2 km. It clears itself at the next shift. **A fault never drains real stock:** if nobody helps, the line
says so and nothing is lost.

## 4. Where errands come from

| Source | How it reaches you | Notes |
|---|---|---|
| **Holdings** | "Anything need doing?" on a quartermaster or any named body there | Answered by the person who needs it (§17). It counts as asked |
| **Strangers** | a passer-by calls out, only at a place a stranger would be (§18) | inside the ceiling and the interruption budget |
| **Wanderers** | "Need a hand?" in their talk | role and memory of you |
| **Locals** | someone you met before, at their spot (§19) | callbacks ×2 |
| **Board asks** | a separate strip of up to two asks on each safehouse board, signed with the house's name | not in the board's 8 player notes |
| **Radio asks** | one personal line to the one player the dealer picked: *"If you're near Levie, Mrs Dvorak at the gate could use a hand."* | never broadcast, so it can't bait an ambush |
| **Traces** | another player's journal line, notice, barricade or robbery (§15) | names never shown |
| **Found hooks** | something at the site of a job (from missions) | one site in five; per kind per player every 4 hours of play; one server-wide every 20 min |
| **Fallout** | a story job failed at a subject ([MISSION_STYLE.md](MISSION_STYLE.md)) | the job's participants first for 20 minutes, then anyone; at most 30% of the failed job's base |
| **Margin and lock cards** | around the majors' set pieces and the finale's routes (§20) | never on a set-piece evening at its place |
| **The epilogue deck** | after the cure (§22) | content owned by [EPILOGUE.md](EPILOGUE.md) |

**Leaders never give errands.** Asked for work, a council head, crew boss, sergeant or field lead says
who to see: *"Ask Marta at the stove. She's been on at me about the pot."* Where the quartermaster *is*
the leader, "Anything need doing?" on it answers the same way, and the errand belongs to the named
person.

## 5. Givers

Built ([docs/ERRANDS.md](../docs/ERRANDS.md#givers), locals: [Locals](../docs/ERRANDS.md#locals)). Still to
build: leaders *saying* a line read from a thread (*"The cook says you brought turnips."*).

## 6. The dealer

Built for holdings, wanderers, strangers (with the place factor) and radio asks
([docs/ERRANDS.md](../docs/ERRANDS.md#the-dealer)). Still to build: the per-player context cache (the
route lock), the freshness and nearness factors, locals coming to you, radio contract offers taking
priority over a radio ask, and a 10-minute first-life wait after Find the Haven's first leg.

## 7. Facts, reasons and signals

**Reasons** ([MISSION_STYLE.md](MISSION_STYLE.md)) are errands' Why: `house.hungry`, `post.pressure`,
`tribe.hungry`, `wanderer.bitten` and the rest, each with a subject and a place.

**Read the facts that already exist first:**
- `Changed` day flags with `NeedFlagWithin`;
- `House(idx)`: `m_iDied`, `m_iLost`, food;
- leaders and wanderers (and their routes); headlines;
- the Long Table's never-arrived;
- journals of the dead;
- the tribes' flags (`alpha.down`, `tribe.hungry`, a fed tribe, a day party out);
- world records ([MISSION_STYLE.md](MISSION_STYLE.md)).

The **signal ring** in `RZ_WorldMemory` holds only facts that have no home elsewhere: bites and deaths
on roads (where and how, never who), Fire Missions and shelling impacts, trial dose outcomes, burned
buildings, a truck robbed or lost (with position). `Signal(kind, pos, faction, subject, value, cause)`
and `Recent(kind, near, radius, minutes)`.

- **Per-kind quotas, so rare signals survive:** deaths and bites 16; shells 6; trucks 6; trial doses 6;
  fires 6; others 4. Rare kinds are kept at least 2 real days.
- **Flood guard:** one player's own deaths and bites add at most one signal per 10 minutes.
- **The cause** (a player id, server-side only, never shown) feeds the arsonist rule.
- Saved, about 2 KB.

**The arsonist rule.** A card is never dealt to the player, or to their vanilla group, whose act made its
fact within the card's window: robbed the truck, killed the crewman, called the shells, put the man out.
Some cards have a *suspect* variant instead. The giver asks *"Where were you Tuesday?"*, and fit drops
×0.5.

## 8. Keeping them true

**Guards on every errand,** checked at deal, every 30 s while offered or held, and at every step:

| Guard | Rule |
|---|---|
| The person | every named person, wanderer, local or alpha named is alive and free (`WandererFree`, the local alive, the alpha alive) |
| The place | still that faction's (`TownOwner`); the house has the people and food the text says; not burned out |
| Stock | hunger cards need the house short; pay in kind needs the stock; "the truck's late" needs a truck on the road or lost |
| The hour | §9 |
| The season | stage, cure and epilogue windows; research errands stop with the cure (Halcyon's by default); line errands only while `PerimeterSealed()`; Bastion errands only above their Bastion presence; No Vehicles removes vehicle ways |
| Holds | below |
| The giver's will | the faction's rules: exiles get no Haven work except the exile deck; a faction that hunts you gives you nothing |
| The lock | §20 |
| Events | each card lists what it can't run under (Every Ridge Calls, Storm, Fog Bank, the final stand) and what it needs |

**Soft holds.** An errand holds its subject with a **soft hold**, `SoftHold(subject, errandId)`, kept
apart from claims:
- It never makes a town busy for anyone.
- It stops another errand from using the same subject.
- When a mission, arc, questline, major or the finale claims that subject, every errand softly holding
  it is **Overtaken** at the next tick.

Errands check claims this way:
- **A mission's claim** (`ClaimedBy` starts with `mission:`) blocks every errand on that subject: a
  stage is happening there now.
- **A story's long hold** (`quest:`, `arc:`) blocks only cards whose Premise lists that subject.
  Routines go on: people cook while the council sits.
- No errand is dealt within 600 m of a running set piece, or on a set-piece evening at the set piece's
  place (the majors' one calendar, [MAJORS.md](MAJORS.md)).

**The premise rule.** Two pieces of content share a premise when they use the same subject (house,
person, item, tribe, place) for the same question. A card that shares a premise with bigger content is
**cut or re-angled**, never just shortened and paid in Favor. `NotWithQuest(key)` is used only for
short questlines; a major runs all season, so its premise is avoided by design instead.

| Card | Shared with | Done |
|---|---|---|
| The Second Bowl (put a mouth out) | Every Name, Shut the Door | cut |
| What He Was Owed, Settling Up | The Book Has to Balance | cut |
| His Boots' loud send-off | the book's send-off | send-off removed; the lighter goes home quietly |
| The Volunteer's Number | The Other Two | cut |
| Volunteers (an orderly wants blood donors) | Blood Drive, The Other Two | cut |
| The Empty Nail (tag, nail, nightly call) | Bastion's lost posts; Await Relief's call | re-angled: The Locker, no call reading |
| Fresh Chalk's Bastion and Halcyon versions | Contact Report, Track Them Home | cut; Haven chalk only |
| Contact (a sentry watches a moving tribe) | Contact Report | cut |
| Halcyon's "read the marks at dusk and bring the count" ask | Contact Report | cut |
| Tag One | Plant a Tag (it's the same job) | cut |
| The Tag (an index unread) | Plant a Tag, Capture the Index | re-angled: The Receiver, the station's receiver fault; never near the alpha or its tag |
| Is She Gone? keepsake | Bring Them Home | re-angled: see whether the tribe still gathers at the lair, no keepsake |
| Walking With; dialogue's "is {name} walking with them?" ask | Bring Them Home; player names | cut and merged into Somebody Should Tell Them: read the marks and hand off, no keepsake, no player names |
| The Market's keepsake variant | Bring Them Home | cut; The Market keeps rings and satchels |
| Not Past the Chapel | Not Tonight | re-angled: bring him in or board him in, never turning a hunting party |
| The Notebook Goes Home | leader job The Notebook | cut |
| The Book (a collector reminds a wanderer) | The Book Has to Balance, Debt Collection | cut |
| Collections | The Book Has to Balance, Debt Collection | re-angled: A Debt Gone Quiet, finding out why a man stopped paying, never collecting |
| Civvies (a deserter's clothes and tag) | Struck Off, Desertion Route | re-angled: only the clothes; his tag is Struck Off's story |
| Rudi Liked It Loud | the book's set piece | cut; a margin card on a non-set-piece evening instead |
| Batteries for the Call | Await Relief's nightly call | re-angled: the set's fault (§3), never the call's content |

**Overtaken.** When a Still fails, the errand ends at the next tick, like `Withdraw`. The giver says one
line: *"Never mind. The truck came in after all."* No pay and no penalty. The "+1, you tried" memory is
given **only when someone other than the asker or their group changed the world**, so a player can't
make their own errand fail for memory.

**Text.** Every name and place is a token filled when shown (`Fill`, `{leader:id}`, `{local:id}`,
`{tribe:id}`, `{alpha:id}`). **A player's platform name never appears** in any errand line, road news or
local's line. Dead characters are their background and home: *"the nurse who came in from Morton"*.
Names follow the epilogue's and tone's policy: frozen at the deed, passed through the note filter,
`/anonymous` and the admin strike honoured, never a joke's butt.

## 9. Size, distance and clocks

- **Distance.** Every place an errand uses is within 1.5 km of the offer point, most within 800 m.
  Checked at deal, on the real route length.
- **Escorts.** At most 600 m, through `RZ_Escort`, ending at a place, never a follower. Errands use
  **one** escort slot server-wide, and only while two of the three are free. Otherwise the escort way
  isn't offered.
- **Waits.** No wait over 10 real minutes, except follow-ups, which wait for you.
- **Clocks.** Every time window is turned into real minutes from the server's measured time rate
  (`RZ_GameTime.RealSecondsUntil`, `RealSecondsFor`), at deal:
  - a window must last at least 10 real minutes, and start within 20, or the card isn't dealt;
  - on accelerated servers some timed cards simply don't fit, and are skipped;
  - with the clock uncalibrated, or no time manager, timed cards are off;
  - text gives clock times in game time (*"before half six"*), and the journal shows the real minutes
    left.
- **Lapse.** A held errand lapses 45 real minutes after you take it (20 if you go offline), or on its own
  clock.
- **The giver can't wait** (a stranger): pay on the spot. **Can wait** (a holding): come back to them.

## 10. Ways, checks and talk

| Way | What it is | Built on |
|---|---|---|
| **Do** | carry, fetch, visit, search, fix, stand a post, board a door, show the way, put down one infected the world already has there | delivery, visit points, search spots, barricades, a short hold |
| **Talk** | a reply that needs a check | the persuade score (`RZ_Checks`; fixed, no roll) |
| **Pay** | Favor, or an item from your pack at the price table's values | `TryBribe`, item hand-in |
| **Sneak** | done unseen and unheard | `m_Quiet`, built stealth |
| **Press** | a threat | Fear, with its limits ([MISSION_STYLE.md](MISSION_STYLE.md)): +3 Infamy, once per person by anyone, three a life per faction. Never at a safehouse or in a safe zone |
| **Force** | a few infected | `SpawnGuards` (4 at most) |
| **Claim** | you're one of theirs | the look ([REPUTATION.md](../docs/REPUTATION.md#borrowed-kit)) |
| **Lie** | say it's done, or say something false | the lie store (§12) |
| **Betray** | take it to someone else | drops |
| **Walk away** | say no, or leave | Without you |

**Checks are fixed, visible tags** on a reply: [Known: Haven], [★ Nurse], [2 food], [8 Favor], [Lie],
[Feared]. Tags show costs and requirements, never outcomes. The same character always gets the same
answer. A failed check says what would work: *"Not from you. Maybe from someone the house knows."*

**Talk** (dialogue is off by default, decision 13):
- **The first version is the action-menu fallback.** It uses the generic puzzle person's four choice
  actions: at most three levels, and at most **four replies a node, hidden ones included**. A hidden
  reply takes one of the four slots, and the writer drops a reply to make room. `CanBeShownScript` shows
  each reply only to the player it's for (the client's journal cache knows the errand's asker and
  helpers), so other players at the same cook see only "Anything need doing?" (VERIFY: per-player action
  visibility from the client cache). If that fails, the replies arrive as a notice with numbered keys.
- **On the dialogue screen**, once it's built and turned on ([DIALOGUE.md](DIALOGUE.md)), the same lines
  and replies show there.
- **One conversation per giver at a time.** Anyone else hears *"Give us a minute."*
- **Every personal line rides the owner-only personal payload:** offers, replies, results, the budget
  left. It's shown at once, ahead of the notice queue, never behind broadcasts or tier changes. That
  also fixes the built leak, where `NotifyPlayer` sends every player's text to every client.

## 11. Givers who lie or hide things

| Twist | What's really going on | Found by |
|---|---|---|
| The wrong reason | they want it for something else | ★ Nurse/Doctor; the right question |
| The wrong person | the "sister" is someone they mean to sell | ★ Police/Ex-Con; Known with the Red Hand; a clue at the place |
| Already gone | the thing isn't there, and what is, is something else | going there |
| Someone else wants it | a second party turns up | sneaking first; ★ Radio Op |
| It's a test | the faction is checking you | doing it straight |
| The giver's own | the giver did the thing they ask you to fix | the right question |

The budget below, the same replies either way and the truth-deck draws are built
([docs/ERRANDS.md](../docs/ERRANDS.md#twists)); the twist kinds in this table are for the cards still to
write.

- **Twists vary per deal.** A card's twist is true only some of the time: Sleeve Down's occupant is
  bitten 6 deals in 10, and otherwise it's a fever. My Sister at Morton's man is a crew lad 1 in 2. The
  wiki can say "it might be a trap", never "it's always a trap".
- **The same replies show whether or not the twist is true.** The call-out reply (*"That's not a
  cough."*, *"Your sister has a lot of brothers."*) is always offered on cards that can have a twist.
- **Calling it out falsely costs:** the giver's regard is crossed, and a line: *"Calling a grieving man a
  liar. Nice."*
- **Budget:** cards with optional twists roll at 20% (a setting). One twist in three errands per player
  at most, never two in a row. Every twist can be found before the decisive choice by at least one way.
  A twist never costs more than the errand's own stakes.

## 12. Lies, witnesses and knowing

**Lies** go into the one lie store, shared with missions and dialogue: per player per season, errands'
share 6 at most (the oldest *unresolvable* one goes first), saved. Each record holds its exposing
condition and its cost:
- the tribe's lair or alpha changed;
- she reached the post;
- the hunting party hit the toll;
- the house counted.

They're **resolved as data, by events first**, whenever the condition is checked, by anyone or by the
owning system: a Haven wanderer reads the mark, the next upkeep, the next night at the toll, a visit by
a player of that faction. The liar needn't be near. Only a claim with nothing in the world to test it
uses the rumour tick.

**Found out:**
- the giver's regard records *lied*;
- 2–5 Infamy with the giver's faction, on the life it was told in, if that life is still alive;
- the local says so next time.

If the life is gone, the lie is buried with it: no Infamy. The next errand that player gets from that
giver kind comes with *"The last one who said that lied."* and fit ×0.8 for a real day.

**Lies never pay more than the truth.** A lie pays at most ×0.6 Favor and no Fame (one rule with
missions), and never changes the data it lies about. Chalking a date without looking marks the prop,
never the Haven's reading. A made-up count is read out as the giver's own guess (*"about forty"*), never
as data any channel repeats. So a griefer can't spoil anyone's intel.

**The witness rule** (`RZ_Witness.Seen(act, pos, faction)`, shared). "If seen" always means one rule. A
witness is any of these within its distance, at the moment of the act:

| Witness | Distance |
|---|---|
| a body the act's faction shows (guard, occupant, lookout, named person, wanderer) | 40 m, or 60 m for posts |
| a local | 40 m |
| the giver | always |

Bodies use built stealth (`RZ_Stealth` visibility); a hidden player isn't seen. Where no bodies exist
(the act is data), a witness is rolled: 25% within 300 m of a lived-in place, else 5%. Witnesses name
the uniform, never the player, in gossip. The Infamy goes to the player only, never to the faction whose
kit they wore.

**Knowing (you).** As with the Long Table, dark acts count against you only when you knew: the tell was
named to you, or the reply says it plainly (*"Sell her to the toll."*). Being tricked by a twist you
never saw costs nothing.

**Vouching** is the one vouch mechanic (dialogue's, disguise's sign-in and the Vouch card): the voucher
pays +5 Infamy once per vouch if the one they vouched for does what the vouch covered, can't vouch again
for 2 hours, and loses 1 regard with the giver if a lie was told under the vouch.

## 13. Failure is a branch

| What went wrong | What it becomes |
|---|---|
| **Too late** | the card's late branch: a grave, a keepsake, a name at supper; half pay or none |
| **Seen** | witness rule: the giver's regard crossed, the card's Infamy, heard about later |
| **Broke it** | bring what's left: half pay, and the giver says so |
| **Said no** | Without you; the giver may mention it (*"You said no. Fair. Most do."*) |
| **Never came back** | lapses: Without you, and a line next time; no Infamy |
| **A promise broken** | the promise lapses (dialogue's clock: 2 hours of your play, at most 3 real days), and the giver says so |
| **A lie found out** | §12 |
| **The world changed** | Overtaken |
| **You died** | the errand lapses quietly: *"Heard what happened. We'll manage."* No penalty |
| **Spoiled by someone else** | Overtaken; the spoiler pays what the act costs anywhere |

## 14. Pay: nothing for free

The ledger is built ([docs/ERRANDS.md](../docs/ERRANDS.md#pay-nothing-for-free)). Still to build:
- **Tagged mission-paid items:** a post's ration or magazine paid in kind, from its stock and never while
  short, tagged so quartermasters, food delivery, the fence and every errand refuse it for pay;
- **a meal as pay:** a fed house above one upkeep pays −1 food, a little healing and 2 Favor;
- **"when the truck comes":** deferred pay at the next delivery there, to the life that earned it;
- **items an errand has you carry** (a letter, a tag, a cut, a cold box) as mission items that can't be
  equipped, pay only when the asker or a helper hands them in, and are refused at drops not the card's
  own; taken by another player, the errand is spoiled and the thief takes the card's Infamy;
- **a stranger's gift:** one common House or Shop find from their own pocket;
- the world changes beyond food, faults and morale (±1 person, ±3 infection, a local, a rumour), and at
  most 2 errand headlines a real day.

## 15. Who asks, and who gets asked

- **The giver's attitude** decides, as for wanderers. Strangers read what they can see (the amber
  ribbon, the red armband, a uniform, the bounty notice's face) **and the Haven's house rules**: a
  Haven-anchored stranger refuses an exile, as Haven wanderers do.
- **The mixed picture.** The title's two halves (what you're known for, how you do it) colour lines, and
  some cards need one half: Wildcard Money wants someone the crews fear and owe. Errands never set the
  title. Errands give no nicknames; the Red Hand's crew names are reputation's.
- **The bad-standing deck.** A bad name opens other doors:
  - Haven exiles are asked by the Red Hand's "little something" people and by Rogue leaders' people;
  - a Bastion Watched player gets amends details at the wire;
  - a Kill on Sight player is sometimes stopped by the one stranger who wants something done by "the
    face from the notice", or recognised by a wanderer who'd sell where you sleep;
  - the desperate don't care who you are.
- **Champions.** People come because of your name: *"You're the one who held Levie."* There's a Champion
  card per faction, and the hall of fame is read for a line (*"My brother saw your name on the list at
  the lab."*).
- **Backgrounds.** Every background has at least two cards that read it ([The catalogue](#the-catalogue)):
  Nurse, Doctor, Mechanic, Hunter, Police, Ex-Con, Lab Tech, Radio Op, Unemployed, Soldier, Raider, Elder
  and Drifter. The Drifter is the go-between.
- **Players' traces.** All data already, never naming anyone:
  - a dead player's journal line (The Name on the Tag's satchel variant);
  - a bounty notice (a stranger asks if you've seen the face);
  - a player's note on the board (a house ask: *"Someone wrote that Levie's clear. Is it?"*);
  - a player's barricade (a family behind it wants to thank whoever boarded it);
  - a truck a player robbed (The Truck Didn't Come, under the arsonist rule).

## 16. Disguises

[REPUTATION.md](../docs/REPUTATION.md#borrowed-kit) owns what a disguise is. For errands:
1. **Who's disguised** is the disguise design's rule: a faction's kit while you're below Known with it
   (mixed tiers by access). At Known or better it's just your gear ("own colours"), and you're treated by
   your real standing. The Haven has no disguise.
2. **A disguise changes who stops you, never what you're allowed.** It passes no errand check, opens no
   chores and counts as no standing in talk. **It never clears an escorted person or an errand item at a
   post:** they're searched or turned back on their own terms, whatever you wear.
3. **Asking their people for work in their kit is a claim**, looked at by the disguise rules (difficulty
   2 at their holding). A pass still opens nothing (*"Nice jacket. Still not on my list."*); a fail is
   the usual catch (+3 Infamy, the kit taken, one of three catches a life).
4. **A stranger who takes you for that faction** asks that faction's kind of question (a mother asks
   about the stockade). Playing along is a lie (§12) with its normal cost.
5. **Acts in disguise** are reported by the uniform (*"a soldier did it"*) but cost only the player. No
   faction's standing, morale or retaliation moves for it.
6. **Cards never pay kit.** A few cards offer a claim as one way (Dress Rehearsal, Borrowed Green,
   Paint's trick, Shift Change's stand-in). The look decides, and you see its tags before you commit.

## 17. Chores at holdings

24 routines are built ([docs/ERRANDS.md](../docs/ERRANDS.md#the-routines)); chore slots per holding (1 + 1
per 5 players online, 3 at most) and one chore per player per holding are still to build. The routines
still to build are in [The catalogue](#the-catalogue).

## 18. Strangers

Built ([docs/ERRANDS.md](../docs/ERRANDS.md#strangers)): the spots (every kind, the footfall grid),
the never rules, the ceiling, how they behave, the 18 stranger routines and found hooks. Still to build:
- **Out of sight:** spawned at a point 60–90 m from the player that they can't see (a building door, a
  hedge line, a road bend), removed out of sight when passed by (VERIFY: the out-of-sight spawn).
- **Every crossroads:** junctions of two used roads from the road network, not only road posts.
- **Players' own campfires** as a fire spot, once players can build one.
- **Harm beyond killing:** shot at +4, robbed +8 Haven Infamy (the wanderer house rules), and robbing
  one stops strangers for that player for a real day.
- **Their own place** for more routines (the house behind a player's barricade).
- **The run-ups the routines left as words:** Still Walking's board note at his house when he got the
  dose; The Last Tin's watch as a real watch.

## 19. Locals, callbacks and threads

**Locals** are built ([docs/ERRANDS.md](../docs/ERRANDS.md#locals)). Still to build for them: shown only
while a player who knows them is within 100 m; three lines a memo; a local's spot used as a place
(the gate at dusk, the stove at supper); Hand to Hand's promises; their epilogue scenes. The rest of
this section is the design they follow.

**Locals** are the people the season remembers (the earlier draft called them faces).
- **What's kept:** a seeded name and role; one line; a **home that's always a place** (a safehouse,
  holding or town); a **spot** (the stove, the gate, the bench, the barrier); alive or not; and
  per-player memory (helped, saved, lied to, crossed, sold).
- **When one is made:** when an errand ends with a person alive and placed. Nobody is made a local
  without a home.
- **Pinned locals** carry a season flag or a big act: saved, walked to the line, sold, given the cure by
  hand, promised by hand. Up to 30 a server, 4 per house. They're kept to the epilogue, dead or alive.
- **Unpinned locals:** up to 20. The oldest that nobody has met for 3 real days goes first.
- **Shown:** one of the house's occupant bodies wears the local's name, line and spot while a player who
  knows them is within 100 m. A local counts inside the house's population and can be among those it
  loses, with news told only to players who knew them. One dressing owner per body: leaders keep their
  own bodies, and locals go only on unclaimed occupants.
- **Across lives:** locals remember players for the season (the ending needs that), but greet a new life
  as a stranger, with one sideways line: *"You've a look of someone."* A Haven Champion's kin is greeted
  as family (existing rule).
- **Privacy:** a local's news about you is a personal line to you only, never road news.

**Callbacks** (the one callback queue, shared with missions and dialogue): errands' follow-ups go into
it. Words only, on played time, near the subject; 10 per player and a system pool of 100; each expires
after 3 season days; at death a waiting one becomes a board note. Dialogue's promises are a held-errand
kind with their own clock (2 hours of the promiser's play, at most 3 real days).

**Threads:** up to 12 per player-season tags that lines read (*"The cook says you brought turnips."*).

**Season flags:** only where other content reads them.

## 20. The big stories, the finale and the lock

1. **Errands are never stages.** They never advance, decide or skip a stage, never set a questline's,
   major's or finale's flags, and never write leader memory, regard, loyalty or votes.
2. They feed only what those stories already read from anyone: a rumour; the Long Table's built triggers
   (a rumour heard, a boot, `table.suspicion_add`); a local. **Majors ignore errand memory** (the
   default; an option allows +1 per head per season, through the clerk's word).
3. **Margin cards.** 2–4 per major, in its voice. Never on its set-piece evenings, never inside its
   600 m, and they change only a giver's memory and a rumour. They fill the gaps between beats
   ([MAJORS.md](MAJORS.md)).
4. **Route work** means anything touching the lab, the cure, the stand, a route's subjects or a route's
   set pieces. Only route work is limited by the lock. House, post, crew, station and stranger errands
   stay open to everyone for the last 10%.
5. **Before 90%** nothing an errand does can close a route. Errands never touch stronghold leaders, and
   routes are joined through them.
6. **After the lock** nothing asks a locked player to do route work for another route, and nothing an
   errand does moves a track:
   - route errands for the finale's **open acts** (I and III) are dealt to that route's locked players
     and to anyone not locked elsewhere (decision 8);
   - route errands for its **sworn acts** (II and IV) only to that route's locked players
     ([FINALE.md](FINALE.md), §5).
7. **Lock cards** are built (one card, titled by your route: [docs/ERRANDS.md](../docs/ERRANDS.md#the-cards)).
8. **Hand to Hand** is built as a card ([docs/ERRANDS.md](../docs/ERRANDS.md#the-cards)). In the finale a
   promise does only two things ([FINALE.md](FINALE.md), §2):
   - a promised local living in the same house as one of the eight makes that person's yes one step
     easier;
   - each promised local present at the Independent's lab-gate sit-in takes a minute off it.

   It never eases a stronghold leader's check (the director's last persuade counts the locals standing at
   the gate, promised or not), and never moves a track. It also feeds the Independent ending's in-world
   scenes.
9. **Final stand:** no errands within 1 km of the lab; elsewhere people keep living.
10. **Reads,** read-only: `finale.route` (the controller: 0 nobody, handed over; 1–4 a faction, the
    faction + 1; 5 Independent), `finale.finisher` (the same values; 0 is the lab alone),
    `finale.terms`, and `RZ_Finale.RouteOf(identity)`.

## 21. The tribes

Tribes and crawlers ([INFECTED.md](INFECTED.md)) replace the Grounds design's town firsts.
- **Reads:** the nearest tribe whose range is within 1.5 km: its alpha (the Haven's name for it where the
  Haven knows who it was, alive, stage), its temperament and hunger, its lair, whether its day party is
  out, whether it fed last night, each faction's reading day (the faction marks where roads enter the
  range), its named young.
- **Writes, only these:** the Haven's reading day, and only by a real reading (30 s unnoticed at the
  range's edge, judged by built stealth); kills through the tribe's ledger like any kill.
- **Errands never** turn a hunting party, move a lair, name a young or take a tribe's samples beyond the
  built rules. Those are contracts' work.
- **By day,** a card whose place is inside a range is never dealt within 300 m of a day party's data
  position. Cards that send you to a range by day say whether it fed (*"Go the day after they've fed.
  Nobody's out then."*).
- **At night,** no card sends you into a range. Reading cards stay at the edge (★ Hunter or Radio Op read
  a tribe by its call from 300 m at dusk).
- **Words.** Player text never says "alpha", "tribe" or "crawler". Each speaker uses its own word:
  `{tribe:N}`, `{alpha:N}` and `{young:N}` resolve to the Haven's "the Quarry lot", Bastion's "Contact
  Papa-two" on the contact board, the Red Hand's "the Postmen", Halcyon's "P-02".
- **Without tribes,** tribe cards are off and lines say "the infected".

## After the cure: the epilogue week

The epilogue lasts **one week: 168 real hours** from the cure to the wipe (decision 12).
[EPILOGUE.md](EPILOGUE.md) owns the week: its phases, its calendar, the epilogue deck's cards (the last
errands) and the closing work that lets every story reach an ending inside it. Errands supply the
machinery and their own season's loose ends.

**The epilogue deck** replaces research and line errands, through this dealer:
- its cards read the route, the majors' endings, locals and errand flags;
- the cards are once per identity per beat, picked per player from beats within 3 km, the player's
  standing and the phase, up to 2 offered at once (EPILOGUE.md's rules);
- they aren't limited by the season's "2 cards a day", so a player can finish the closing work in a
  week of normal play;
- the small-pay ledger still applies, and nothing paid carries into next season.

**Errand pieces in the closing work** (named as in EPILOGUE.md):

| Closing work | What errands give it |
|---|---|
| **Bring Them Home** | The season's **His Effects** cards (tags, lighters, a driver's papers) and **The Name on the Tag** leave their carried items and world records for it. A tag that reached the nail before the walk goes into Sedlak's box ([MAJORS.md](MAJORS.md)) |
| **Settle Accounts** | The amends family's last card, **Come In**; Vouch's open liabilities; promises still open (each gets a closing line: kept, or *"You meant it. That counts for something."*) |
| **The Walk Out** | **Processing**: a pinned local waits in the queue and goes with the Walk Out when it opens |
| **Take Back a Town** | Errands never run inside a town being taken back, but a cleared town's houses get their asks again |
| **Personal Goals, The Last Entry** | The hall of fame's "Most people helped" line, frozen at the cure; the radio counts line below |
| **Loose Ends** | An errand is never a loose end: a held errand at the cure finishes normally or lapses quietly, and route errands are called off with no pay and no penalty |

**The ending contract** is what the world shows for the whole week, built only when a player is near:

| Where | What's shown | From |
|---|---|---|
| **The house at supper** (18:30–20:30) | pinned locals at their spots, alive, by name; one line each about what you did (*"You brought the letter."*), to you only | pinned locals with `helped`/`saved` |
| **The gate** | the locals who waited there (the Kid at dusk); the dead's names read by the head | locals, `m_iDied` |
| **The board** | the house's last asks, struck through as "done" or "never came"; one line per pinned local who died | board asks, locals |
| **The north road** | the letter under its stone (`errand.letter_at_line`); locals in the queue, 150–250 m short of the barrier, until the Walk Out takes them | flags, locals |
| **Posts and camps** | the boxes by the radio, labelled *"for when the gates open"*; the crews' last tolls | threads, The Locker |
| **The road** | under route 5, the locals you promised, waiting at their houses for the hand-to-hand dose | `errand.hands` |
| **Radio** | one line per house about the season's errands (*"Levie: forty-one turns done for us this season. Thank you, whoever you were."*), which The Last Broadcast can read | the server's errand counters |

At most 8 local scenes per house, pinned first. In the epilogue a callback expires after 24 real hours,
and one still waiting on the last day becomes a board note. Unresolved lie records go to the epilogue's
"The truth comes out" on the last day. Nothing persists past the wipe.

## 23. Multiplayer, death and griefing

- An errand is offered to one player. Their helpers can choose and are paid as §14. Errands never reveal
  another player's name or position.
- **Killing a local** costs as killing a wanderer (+15 Haven Infamy). The local's ending scene becomes
  "killed on the Levie road", so the spoiler adds a grave rather than erasing the story.
- **Bringing a horde onto a lived-in place** through an errand way (bells, sirens, noise) costs the house
  rules' Infamy, as firing near it does.
- **Death:** held errands lapse quietly. Callbacks, lie records and threads stay with the player for the
  season. Deferred pay goes to the life that earned it, or is lost.
- **Disconnect:** the errand waits 20 minutes. An escort waits beside the nearest group member within
  20 m, else walks home as data, and the errand lapses without penalty.
- **Starving the system** is impossible: one errand escort, scene caps, and a 45-minute lapse.
- **Scene overflow:** live errands up to 12, errand scenes up to 10, strangers up to 6 (the one body
  budget). If no scene slot is free when the asker comes within 250 m, the oldest scene with nobody
  within 250 m is put back to data. A holding giver can always speak through the holding's existing
  bodies. A stranger who can't spawn isn't offered.

## 24. Cheap

- An errand is one record of about 120 bytes.
- Scenes are built at 250 m and removed at 400 m: one person and two props at most, existing prefabs
  (puzzle civilian and object, notes, the mission item, keys, barricades), through the one scene-people
  spawner.
- Infected only through `SpawnGuards` (4), inside the spawner's budget. At most one card in eight uses an
  AI group.
- No map scans: the context, the facts, the ring and the stranger spots, with 8 candidates per player per
  90 s. Steps on the 5 s tick; the dealer, Still and the footfall grid on the 30 s tick; nothing per
  frame.
- **Saved:**
  - per player-season, about 0.5 KB (card bitset 8 bytes, 20 heard-line ids, 12 threads, 6 lies, the
    ledger): about 50 KB for 100 players;
  - locals about 10 KB (50 × 0.2 KB); named people about 5 KB; the ring about 2 KB; counters about 2 KB;
  - the footfall grid isn't saved.

## The catalogue

Every routine and card still to build, with where it came from. "Pays" is within §14's rules
everywhere: Fame at most 2, Favor 3–10, or amends. The built ones, 34 routines and 63 cards, are in
[docs/ERRANDS.md](../docs/ERRANDS.md#the-routines).

**The dead**
- **His Effects in full** (a simpler Locker and Wreck's Name on a lost truck, His Lighter as a routine,
  and The Name on the Tape for Bastion's dead near a post are built). Still to build: the effects as a
  mission item; the jacket and the photo frame at the fence (3 and 10 RH Favor, *"A name the army can't check twice"*) under the witness rule; a kit piece
  off a man you killed refused (*"You'd know. You were there."*, +3 Infamy with his faction); once per
  dead man, 3 times a life. *After the cure: The Tags* is part of Bring Them Home ([EPILOGUE.md](EPILOGUE.md)).
- **The Name on the Tag**'s named body at a job's site (the satchel variant is built). Carry the ID to
  the house or post it names: 4 Favor from that faction. Red Hand crews don't bury their dead, so it's
  thanks only.

**Margin cards** (2–4 per major, in its voice; never on its set-piece evenings or inside its 600 m).
Clean Glasses (The Book Has to Balance), Count for the Sitting (Every Name) and Hand to Hand (the
Independent) are built. Await Relief and Proof each get 2–4, written with their remaining build
([MAJORS.md](MAJORS.md)). Hand to Hand's promises in the finale (§20) are the finale's to read.

**The epilogue** (also listed in [EPILOGUE.md](EPILOGUE.md)'s deck)
- **Processing** (a pinned local). Why: the cure is out, and a checkpoint is within 1.5 km. Walk them to
  the queue at the north road, 150–250 m short of the barrier: *"Processing. I've been processed since Day
  2."* They sit in the queue, and when the Walk Out opens they go with it, and your line says so. Or talk
  them into supper, where they are the next night. Or leave them sitting by the concrete every time anyone
  passes, until the Walk Out or the wipe. Never at the barrier itself.
- **What the Cure Costs Here** (a sick local). Why: the epilogue, and the terms keep this local from a
  dose. The epilogue's one outlet table decides whether the terms reach them and at what price, and the
  card offers only the way that table allows: register them at a post's list (Bastion's roster terms;
  they comply, and it's sad) or [Trusted: Bastion] vouch; pay their dose at the table's price, or put it
  on their line in the book (the Red Hand's terms) or [Trusted: RH] haggle; or, where no outlet serves
  them, sit with them. Where the terms reach them, it's a thank-you instead. They tell people who brought
  it.
- *After the cure:* **Come In** (the amends family's last card, [EPILOGUE.md](EPILOGUE.md)).

## One evening in Levie

How errands stay relevant: one evening, every errand true tonight.

**The player:** a Mechanic, Known with the Haven, not Watched by Bastion, who rides into Levie at 18:00.

**What the world says right now** (the reasons):
- Levie is on half portions (`house.hungry`).
- Its generator is dry (`house.dark`).
- The Quarry lot are hungry 1.2 km away (`tribe.hungry`).
- Bastion's crossroads post lost a truck last night (`post.truck_lost`).

**The evening:**
- **Lights.** At 15 m the storekeeper waves him over: *"Generator's dry. One can, before the lamps."*
  He's a Mechanic, so a car battery will do: *"There's one in the Škoda by the church."* Eight minutes, 5
  Haven Favor, and the record `lit.levie`.
- **The Pot.** The board says *"Short this week. Ask Mara at the stove."* Mara, the cook, sends him to the
  root cellar at a farm outside town: two tins Levie kept back from the other houses. At the cellar he
  chooses whether they go to Levie, or to Morton, which is hungrier ("borrowed").
  - He takes them to Morton. Morton thanks him (a local).
  - Levie's cook sulks for 2 hours of his play, in words only.
- **A found hook.** At the cellar there's a body with a Morton man's ID: The Name on the Tag. He carries
  it to Morton, where the name is read at supper. That writes a world record with an epilogue line.
- **The Wire.** Passing the crossroads post, the sergeant calls out: *"Contact Papa-two will be on the
  north road tonight. Two barricades, fifteen minutes."* 6 Bastion Favor before diminishing returns.

**The total:** about 40 minutes and four small errands, all from what's true tonight. About 11 Favor
across two factions after diminishing returns (5, thanks only, 3, then 3), 2 Fame, and no loot.

**On the way home** he takes the Levie road at dusk, a supply route into the house's approach: a stranger
could find him there, if one has a reason to. On the forest track he took that morning, nobody could.

**The callback:** two hours of his play later, Morton's board reads: *"Tomas came home on a door. Thank
you for bringing his name. The Novaks."*

**If he'd been Watched by the Haven:** the gate guard would only have offered Make It Right (keep the gate
at dusk, 2 Infamy off). The storekeeper wouldn't have waved.

**If he'd been a Nurse:** Sit With Him, at the house with a bitten resident, would have come first.

## 27. Settings, telemetry, writing, phases

**Missions > Errands** is built with the core's and phase 4's settings ([docs/ERRANDS.md](../docs/ERRANDS.md)).
Still to add: Stranger gap, Strangers on the server, Footfall half-life and Pinned / unpinned locals as
settings (they're fixed today: 15–25 min, 6, 6 real hours, 30 / 20), and Majors read errand memory (off).
Telemetry still to add: lies told against lies found, and twists.

**Writing a card.** One class per card, `Cards/<Family>/RZ_Errand_<Name>.c` (built: the base
`RZ_ErrandCard`). Three variants of every line, in the giver's voice ([../docs/WRITING.md](../docs/WRITING.md)),
written for a figure standing still. Add it to docs/ERRANDS.md and run `tools/compile_check.sh`. The runtime
test is to force each Why true then false and check the Overtake, and place one stranger at each spot kind.

**Phases** (the plan's numbering):
- **Phase 3, the core:** built.
- **Phase 4, people:** built.
- **Phase 5:** Await Relief's and Proof's margin cards (the others are built).
- **Phase 6:** the finale reading Hand to Hand's promises (the lock card and Hand to Hand are built).
- **Phase 7:** the epilogue deck and the ending contract, with [EPILOGUE.md](EPILOGUE.md).
- **Phase 8:** built (to 63 cards, with the disguise side jobs); dialogue's strangers and radio speakers in full are dialogue's.

## 28. Engine needs

The dealer, the routines, named people, soft holds, the faults table, clock conversion and the
errand ledger are built ([docs/ERRANDS.md](../docs/ERRANDS.md)). Still to build:

- **The signal ring** with per-kind quotas, a per-player flood guard and a server-only cause.
- **A per-player context cache,** refreshed every 30 s: settlement and owner, holding within 600 m,
  tribe's range within 1.5 km, road, vehicle (moving or stopped), fight, near a mission or set piece,
  near the farm, minutes into the life, the nearest plausible spot, tiers, title halves, route lock.
- **Size checks** at deal: route length ≤ 1.5 km from the offer point; escorts ≤ 600 m through
  `RZ_Escort`, one errand escort server-wide and only while two of its three slots are free; waits ≤ 10
  real min.
- **Talk fallback:** four replies a node including hidden ones, on the generic puzzle person's choice
  actions; `CanBeShownScript` filters per player from the client journal cache (VERIFY); one conversation
  per giver; every line on the owner-only payload.
- **Pay in kind as effects:** house food −1 with a meal heal; post stock −1 never while short; deferred
  "when the truck comes" pay to the earning life. Tagged mission-paid items refused for pay at
  quartermasters, food delivery, the fence and errands (extend the fence's sale memory). A never-pay
  list: disguise keywords, vehicles and keys, Trusted and Champion shelves, rare finds.
- **Group rules:** choices only by the asker and helpers (vanilla group, within 50 m, present for a step
  or 2 min); helpers get half Favor, three paid at most; in kind to the asker only.
- **Disguise reads** from [REPUTATION.md](../docs/REPUTATION.md#borrowed-kit): own colours, `IsPassing`, the look for claims.
  Disguises never clear escorted people or errand items at posts.
- **Finale and majors reads,** read-only: `finale.route`, `finale.finisher`, `finale.terms`,
  `RZ_Finale.RouteOf(identity)`, the open and sworn acts, and the majors' set-piece calendar. A
  route-work tag on cards. The flag `errand.hands`, capped at 6 per player.
- **Tribe hooks:** the nearest tribe, its alpha, temperament, hunger, lair, day party, a fed night,
  reading days and named young. The only writes are the Haven's reading day, by a real 30 s reading, and
  kills through the ledger. Tribe cards are off without tribes.
- **The epilogue ending contract:** a builder that, near a player, places pinned locals at their spots at
  supper and the gate, board asks struck through, the letter under its stone, the boxes by the radio,
  locals in the queue, route-5 promises and the radio counts line, for the whole week. At most 8 local
  scenes per house; nothing saved past the wipe.
- **Scene budget:** live errands ≤ 12, errand scenes ≤ 10, strangers ≤ 6. Overflow puts back to data the
  oldest scene with nobody within 250 m; holding givers fall back to the holding's bodies; a stranger who
  can't spawn isn't offered.
- **UI:** an "Errand" journal row kind (giver as "by", the exact place, real minutes left, what you said
  you'd do, known ways and tells) and a `mission_errand` icon (SVG and 64/128 PNG). Never a map marker.
  Docs in UI.md and UI/LAYOUTS.md.
- **Runtime test:** force each Why true then false, and assert the Overtake; place one stranger at each
  spot kind, and assert none in open country.
- **Docs in the same commit:** DESIGN.md "Errands" and its Contents line; MISSIONS.md "Errands" (routines
  and cards with ✅) and the counts; REPUTATION.md (errand Fame ceiling, amends, the witness rule for
  selling people); UI.md; WORKBENCH_SETUP.md (actions, settings, VERIFY items, log lines); ROADMAP (the
  cut cards go nowhere: they were never built); README feature list.

## 29. Risks

- **Writing volume and voice.** 60 cards and 30 routines with three variants of every line is a lot of
  writing, and weak lines would feel generated. Mitigation: routines are data with slots; ship 16 strong
  cards first; review each against the writing guide; telemetry shows which cards players decline.
- **Contradiction bugs:** naming the dead, food at a fed house, a truck that came. Mitigation: central
  guards at every step, tokens filled when shown, soft holds that give way, and the runtime test flipping
  each Why.
- **Annoyance.** Mitigation: strangers only at plausible spots, the shared interruption budget, silence
  in fights, vehicles, near missions and set pieces, a free "Not now", settings, and telemetry on
  declines.
- **Strangers too rare.** On a small server few roads gather footfall, so strangers may feel scarce.
  Mitigation: supply and wanderer routes count as used roads whatever the footfall, approaches and town
  edges always count, and telemetry shows strangers dealt by spot kind.
- **Farming.** Mitigation: per-player (not per-life) cooldowns and caps, the Fame ceiling, in kind as
  effects, tagged items refused everywhere, group pay that needs presence, Overtaken memory only when
  someone else changed the world, chore slots that scale with players, and "the same spot twice" barred
  for strangers.
- **Economy drift from in-kind effects.** Mitigation: never pay from a short place, ±2 food per errand,
  no stock drain from neglect, and every in-kind change logged.
- **Griefing through misinformation or spoiling.** Mitigation: lies never write data; witnesses and lie
  records resolve as data; killing a local costs a wanderer's Infamy and leaves a grave in the ending; no
  position leaks (one group per stranger, personal radio asks, no "someone's helping her").
- **Performance on busy servers.** Mitigation: round-robin dealing, 8 candidates, the anchor index, the
  spot finder on the existing 30 s tick, 10 scenes and 6 strangers at most, and a 20-player measurement.
- **Engine unknowns:** the out-of-sight stranger spawn, per-player action visibility on a shared entity,
  reading lit fires and stopped cars cheaply. Mitigation: each is a VERIFY, with fallbacks: a road-bend
  spawn; a reply list as a notice with number keys; fires and cars as spot kinds turned off.
- **Dependencies on unbuilt designs** (tribes, the dialogue screen, the reputation title, the finale's
  route keys and lock, the majors, the epilogue week). Mitigation: every family that needs one is off
  until it exists; phase 3 (chores, wanderers, aftermath cards) stands alone.
- **Named people on bodies may clash** with leader bodies or other content dressing occupants.
  Mitigation: one dressing owner per body, leaders keep their own bodies, and locals only on unclaimed
  occupants.
- **Pinned locals could crowd the epilogue** on big servers. Mitigation: 30 pinned, 4 per house, 8 local
  scenes per house, pinned first.
- **Overlap with contracts and leader jobs.** Mitigation: errands stay small, in person, never offered on
  the radio as work; the premise rule; and their own MISSIONS.md section.
