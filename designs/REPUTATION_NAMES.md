# Reputation names: the mixed picture

**Status: designed, phases 1 and 2 built.** Approved by the author on 2026-11-11. Phase 2 (the voice: manners,
the opening line, what a leader makes of you in four voices, the faction's word for you, Red Hand crew names,
the whole picture, deed phrases and local deeds, life-stamped memory) is built and lives in
[REPUTATION.md](../docs/REPUTATION.md#what-people-call-you). Phase 1 (the 16 names and
"counts as", Earned Fame and Feared, the flags of `AddInfamy` and the credit rule, one forgiveness budget across
every built route, the debounced change notice, the card, the grid and the fallback text, the Wildcard search and
toll, the admin read-out) is built and lives in [REPUTATION.md](../docs/REPUTATION.md#reputations); this file keeps
only what's still to build.

Each of the 16 cells of the existing Fame × Infamy grid gets a name, as in New Vegas. The 7 plain cells keep their
tier names. The 9 mixed cells get Mixed Bag, Useful Trouble, Favourite Rascal, Bad News, Wildcard, Necessary Evil,
Merciful Thug, Soft-Hearted Devil and Wild Child.

The tier is still the only rule word. **A name opens nothing.** It may pick lines, and which small story is
offered; options vary by tier or by a number on the card, never by a name or a manner. Underneath, two fixes close
the holes the names would expose: the credit runs out at Fame 75+, and there is one forgiveness budget. On top come
lines, the shared Fear check, the parley and the knock, settling up, the Wildcard tier at posts, the company you
keep, the faction's word for you, the whole picture, face and name, the lock, and the season record the ending
reads.

## What changed in review

- **One threat score** (the plan's section 5). Reputation's quiet threat, missions' Press and dialogue's `threaten`
  are one **Fear** check: the higher of your Feared band and your backing (at most 2), +1 for a listed background,
  no bonus for being armed, +3 Infamy, once per person by anyone, three per life per faction, a failed threat barred
  for a real day, never on stronghold leaders. [MISSION_STYLE.md](MISSION_STYLE.md#checks-persuade-and-fear) owns it;
  this file keeps the reputation numbers it reads and the quiet success on built FORCE stages. `PRESS`, `TRADE`,
  `TRICK` and `LEAVE` are appended to the enum once; built threat stages keep recording FORCE
  ([The Fear check](#the-fear-check-one-threat-score)).
- **Tags show costs and requirements, never outcomes.** The threat tag keeps its cost and the Hunted-cliff warning.
  "He'll tell the post" and "he knows about the stockade" are gone.
- **One persuade score** (decision 11): your access rank, mixed tiers one lower; the built `PersuadeScore` stays.
  It's dialogue's Trust check ([DIALOGUE.md](DIALOGUE.md#8-checks-and-tags-new-vegas-style)). No check reads a name.
- **One `AddInfamy` with flags:** SHARE (a rival share: not direct, never doubled), ACT (done in sight: may double
  while passing in disguise), an optional deed text, and the credit rule.
- **One forgiveness budget, for everything** that works Infamy off: it now also covers errand amends, dialogue's
  Make it right, the epilogue's Come In and the finale's Hand it in (conflict 23). Price doors forgive nothing.
- **"Moods" are "manners"** (the plan's renames: "mood" means only tone's season mood now). Manners are a writing
  tool; no rule reads one.
- **Crew names follow the new lore** (conflict 31). A crew name is what the crew holds on you, picked after
  something you'd rather they forgot, not an honour. Bailiff's trigger is "laid down a tribe's leader". Nanny is cut
  (it mocked a kindness). The code is quoted as LORE.md has it now.
- **Tribes replace Grounds** (conflicts 42 and 43): deeds, crew names and conditions read tribes and their leaders,
  in each faction's own word.
- **The Knock also hosts the Haven's price door,** "In pencil" (conflict 21), so the two no longer share a name.
- **The lock** follows [FINALE.md](FINALE.md#4-the-lock): the price doors are named, "fighting" has the finale's
  meaning, the Independent's overlay is "nobody's", and killing locked players costs +3 Infamy outside the stand,
  12 an hour at most, and nothing at the stand between locked sides (decision 9).
- **The epilogue lasts one week** (decision 12). Reputation gives the closure set's Settle Accounts its hooks
  ([The last week](#14-the-last-week-settling-accounts)). Nothing carries into the next season except hall-of-fame
  lines and the majors' words-only crew-name greeting.
- **Side-quest seeds are errand cards** dealt by [ERRANDS.md](ERRANDS.md) under its rules and small-pay ledger. The
  two Vouch cards are one vouch; Clean Slate joins the amends family; Name Day's Brush joins Paint; Collections is
  re-angled away from Debt Collection; "Who's Asking" is renamed "Unknown Face" (errands has its own). The
  Collections worked example is gone.
- **Disguise is owned by [DISGUISE.md](DISGUISE.md).** This file only lists what the look reads from reputation.
  Checkpoints: "search the whole car" applies before any look, and the Wildcard toll and the lab-coat toll take the
  higher of the two (20), never both.
- **The names policy** applies: names are frozen at the deed and filtered; `/anonymous` and the admin strike are
  honoured; gate guards may name a player face to face; a name is never a joke's butt or on air.
- **Small fixes:** the knock example's dead council head is renamed (Tomas is the Long Table's grace name).

**Contents**
- [0. What this design changes](#0-what-this-design-changes)
- [1. The idea](#1-the-idea)
- [2. The sixteen reputations](#2-the-sixteen-reputations)
- [3. Three rules underneath](#3-three-rules-underneath)
- [4. Manners (for writing lines only)](#4-manners-for-writing-lines-only)
- [5. What people say](#5-what-people-say)
- [6. Talk: Fear, the parley and the knock](#6-talk-fear-the-parley-and-the-knock)
- [7. Settling up](#7-settling-up)
- [8. Gates, roads and the company you keep](#8-gates-roads-and-the-company-you-keep)
- [9. The faction's word for you](#9-the-factions-word-for-you)
- [10. Deeds and tallies](#10-deeds-and-tallies)
- [11. The whole picture](#11-the-whole-picture)
- [12. Face and name: one scope rule](#12-face-and-name-one-scope-rule)
- [13. The lock, the cure and the season record](#13-the-lock-the-cure-and-the-season-record)
- [14. The last week: settling accounts](#14-the-last-week-settling-accounts)
- [15. Side quests: what reputation gives errands](#15-side-quests-what-reputation-gives-errands)
- [16. What the disguise look reads](#16-what-the-disguise-look-reads)
- [17. In the UI](#17-in-the-ui)
- [18. Multiplayer, saving, performance](#18-multiplayer-saving-performance)
- [19. Build order](#19-build-order)
- [20. Guardrails](#20-guardrails)
- [21. Fits with the majors and the tribes](#21-fits-with-the-majors-and-the-tribes)
- [22. Doc changes when it's built](#22-doc-changes-when-its-built)
- [Examples](#examples)
- [Engine needs](#engine-needs)
- [Options decided](#options-decided)
- [Risks](#risks)

---

## 0. What this design changes

- **Manners are a writing tool only.** Every rule reads a tier or a number the standing card shows, and REPUTATION.md
  states it as a rule. Players learn two things: the tier (what you can do) and the name (what they call you).
- **Built in phase 1:** the credit rule at Fame 75+, one forgiveness budget, and Earned this life
  ([REPUTATION.md](../docs/REPUTATION.md#reputations)). The later routes join the budget ([§3.3](#3-three-rules-underneath)).
- **One Fear check** for every threat in the game, with the quiet success on built FORCE stages
  ([§6.1](#the-fear-check-one-threat-score)). It's tagged honestly, cliff included.
- **The parley** for the Hunted who are owed: local by its conditions, closed while a squad or fight of that faction
  has you, empty hands checked in every pocket, nobody walks out to meet you, the job finishes where it finishes,
  and only your own group's fire breaks it. The Haven's version is a knock ([§6.2](#62-the-parley-for-the-owed)).
  The same rules close the hole in the built Bastion approach.
- **Settling up:** banked Fame for Infamy, 2 for 1, from the budget ([§7](#7-settling-up)).
- **Cut for good:** danger money on shop prices; leader job weights by manner; Call it square; background crew
  names. Disguise exposure rules belong to [DISGUISE.md](DISGUISE.md).
- **Crew names** follow LORE: what the crew holds on you, given for something you'd rather they forgot
  ([§9](#9-the-factions-word-for-you)).
- **Face and name** ([§12](#12-face-and-name-one-scope-rule)): this life versus this season. Leader and wanderer
  memory is life-stamped.
- **New:** the 90% lock and the season record ([§13](#13-the-lock-the-cure-and-the-season-record)); the last week's
  hooks ([§14](#14-the-last-week-settling-accounts)); the company you keep ([§8](#8-gates-roads-and-the-company-you-keep));
  local deeds at a seat and the cards for errands ([§15](#15-side-quests-what-reputation-gives-errands)); an admin
  read-out ([§17](#17-in-the-ui)).

## 1. The idea

Each faction keeps your **Fame** and **Infamy** and turns them into a **tier** (REPUTATION.md, "Standing tiers").
The tier stays the only rule word for:
- shelves, perks and contract requirements;
- persuade ranks;
- the hostility bitmask (`SURV_H1–15`) and the **Shoots from** stance.

A **reputation** is the same cell of the same grid, with its own name: what people call you. **A name opens
nothing.** The few rules this design adds read either a tier (Wildcard, Hunted) or a number shown on the card (the
Infamy bands, the budget left). REPUTATION.md lists all of them.

A mixed reputation is the story of this life. Infamy still never fades by itself. It can be worked off only from one
capped budget ([§3.3](../docs/REPUTATION.md#working-off-infamy)). The next character starts fresh, as today: background standing,
Word gets around, Remembered.

## 2. The sixteen reputations

**Built:** the names, how they're shown ("counts as"), and notoriety are in
[REPUTATION.md](../docs/REPUTATION.md#reputations). Still to build here: the voices that say them ([§5](#5-what-people-say)).

## 3. Three rules underneath

**Built:** Earned this life, the credit rule (Fame 75+) and the one forgiveness budget with `Forgive()` across the
built routes (amends contracts, paying, surrender) are in [REPUTATION.md](../docs/REPUTATION.md#working-off-infamy).
Being **owed** (the parley) will read Earned Fame 15+ and a recorded best deed.

### 3.3 The routes still to join the budget

Each of these goes through `Forgive()` when it's built, inside the same 30 a life. The finale's price doors open a
route and **forgive nothing**.

| Route | Who | Takes off | Limits besides the budget |
|---|---|---|---|
| Settling up in Fame ([§7](#7-settling-up)) | Anyone the quartermaster serves | 5 for 10 Fame | 10 a real day |
| Hand it in (the finale's Bastion route: a thief surrenders with the box) | As surrender | 8 | As surrender. With the budget spent, the box is still taken in and no Infamy comes off |
| Parley amends ([§6.2](#62-the-parley-for-the-owed)) | The owed, while Hunted | The contract's Fame | One offer per parley; no extra Merciful count |
| Make it right ([DIALOGUE.md](DIALOGUE.md#make-it-right)) | A player a leader holds at −2 or below | The amends contract's Fame | Clears that leader's built crossed bit once a season |
| Errand amends ([ERRANDS.md](ERRANDS.md)) | 15+ Infamy with that faction | 2 at a time | 8 a life; never below 15 |
| Come In (the epilogue's amends, [EPILOGUE.md](EPILOGUE.md)) | As the epilogue sets | Its own amount | — |

Settling up greys out at the budget's end with the built line: "Bastion has forgiven you all it will this life."

## 4. Manners (for writing lines only)

**Built:** the manners and `VoiceManner` (Pragmatist, Independent and Rogue leaders)
([REPUTATION.md](../docs/REPUTATION.md#what-people-call-you)). Still to build: the **Owed** manner (Hunted or
Kill on Sight with Earned Fame 15+ and a best deed) with the parley; wanderers reading the Haven's manner and
then their memory, and the Deserter reading Bastion's reversed, when wanderer greetings move into the voice bank.
**Weighting:** the runtime test logs time spent per cell, so the pools are written heaviest for the cells players
live in (Stranger, Known, Watched, Mixed Bag).

## 5. What people say

**Built:** the opening chain (a local deed at this seat, memory from this life, the name from an earlier life
once per leader per life, the greeting by manner from the voice bank, the built greeting), Ask what they think in
four voices with four variants and the rule line, and the writing rule (the name said aloud only when they say
what they make of you) ([REPUTATION.md](../docs/REPUTATION.md#what-people-call-you)).

Still to build:
- **Step 1 of the opening, the crossing opener:** a questline or major's deed line ("You're the one who gave
  Bastion the book.").
- **Kin and the Long Table in the opening:** kin of the fallen and the Long Table's guest, family or crossed.
- **Who you work for:** the faction you gave the most Fame this life ("You run for Bastion."), and a tally
  standing in for the worst deed in the openings ("You've put {kills} of ours down.").
- **One time in three a deed callback** in the reputation greeting.
- **The change notice in voice:** the line from the nearest person of that faction (else its channel), with the
  rule in brackets: "(Hunted: their people shoot you. They still owe you a hearing at a gate.)"
- **The owed greetings** (with the parley): "Halt. Hands. You {best}. You also {worst}. Both go in the log. Talk."

## 6. Talk: Fear, the parley and the knock

### The Fear check: one threat score

**Built:** the score (Feared band or backing, +1 for a listed background, nothing for being armed), unaffiliated
listeners (your highest earned Infamy, at most 2, charged to the Haven), Pass / Short / Refused, the cost (+3, up to
8, and the life record "pressed"), the limits (once per person by anyone, three per life per faction, a failed threat
barred a real day, never a stronghold leader), the enum (PRESS, TRADE, TRICK and LEAVE appended once) and the quiet
success's helper (`m_iMenaceDifficulty`, `TryMenace`): [DESIGN.md](../docs/DESIGN.md#checks-and-ways).

Still to build:
- **Trait modifiers** on the difficulty: a Ruthless listener one harder (`harder:ruthless`), a Paranoid one one
  easier (`easier:paranoid`); a faction you worked against in a questline ending adds 1 (the crossing record).
- **The quiet success on the built FORCE stages** (Read the Page, Collect, Protection Racket, The Driver, Meet the
  Driver, Debt Collection, The Ledger, and any stage written with a threat): each sets `m_iMenaceDifficulty` and
  writes its quiet outcome (the stage's better outcome: nobody tells the crew and the camp isn't on guard). Below it,
  the built outcome, still recorded as FORCE.
- **Where threats are offered:** the stronghold leader's option greyed ("The boss doesn't scare."); never during a
  parley; free-standing talk threats never in safe zones (strongholds, the lab) or safehouses (a threat a mission
  writes into its own scene stays). After one threat by anyone: "He's already scared. Of someone else."
- **Memory:** a threatened leader or wanderer remembers being crossed, life-stamped ([§12](#12-face-and-name-one-scope-rule));
  dialogue adds regard *threatened*.
- **The tag** in this form: "[Threaten · +3 Bastion Infamy · Fear 2 of 2]", "[Threaten · +3 Bastion Infamy · this
  makes you Hunted]" (the built `FearTag` gives the cost and the Hunted warning).

Short Weight's "Lean on him" stays what it is: a persuade with ★ Police, Ex-Con or Elder. Only threats use Fear.

### 6.2 The parley (for the owed)

**Who is owed:** Hunted or Kill on Sight with a faction, Earned Fame 15+ with it ([§3.1](../docs/REPUTATION.md#reputations)), and a
recorded best deed with it this life. In the grid that's Bad News, Merciful Thug and Soft-Hearted Devil, but only for
someone who earned the Fame this life.

**Where:**

| Faction | Where |
|---|---|
| Bastion | A checkpoint barrier or a camp gate. For the owed, the built surrender becomes the parley: the search, then the offer |
| The Red Hand | A roadblock barrier or a camp gate |
| Halcyon | The lab gate or a field station gate |
| The Haven | **The knock** (below) |

**Closed (no shout at all, so they shoot as usual) when any of these is true:**
- a hunter squad of that faction is within 1 km of you;
- any mission fight, hostile zone or retaliation grudge of that faction has you;
- another player is within 150 m of the gate ("Just you.");
- this identity parleyed with that faction in the last 2 real hours (saved as server time, so a reconnect or restart
  doesn't reset it);
- after the 90% lock, that faction's route is **fighting** yours ([§13](#13-the-lock-the-cure-and-the-season-record)).

**How it runs:**
1. **The shout.** At 150 m: "Stop there. Empty hands, or not at all."
2. **Walking in.** You must be **empty-handed**: no firearm in any slot *or* in storage, and no grenade or charge (new
   `RZ_Util.IsEmptyHanded`, every tick). They hold fire only while your distance is falling, 90 s at most. The hold is
   the existing `StandDown`, refreshed 5 s at a time, and only while you're within 200 m of the gate with every
   closing condition still false. *Simplified:* Reforger's hostility is per faction, so the hold is made local by its
   conditions. It can only exist at the gate, with nothing else of that faction on you.
3. **At the barrier** (35 m) for 10 s you're searched. A weapon found ends it, with the post's grudge. Nobody walks out
   to meet you. The seat's leader speaks as a notice ("over the radio from inside"). At a road post, the nearest
   outpost's leader speaks; with no leader, the gate's voice. You get one deed callback and **one amends offer**.
4. **Walking away.** The hold lasts while your distance is rising, until 200 m or 90 s, whichever comes first. Stop,
   turn back or arm yourself, and it ends.

**What breaks it, and what only pauses it:**
- **Breaks:** you arm yourself or fire; you go 20 m past the barrier; your own vanilla group fires on the post or that
  faction's people.
- **Pauses:** anyone else's fire. The talk timer stops, your hold stays, and the post takes a grudge against the
  shooter. A third party can't get you killed this way.
- **Another player arriving within 150 m** also pauses it. "Back off. We're talking." An armed newcomer who stays 20 s
  gets the post's grudge. Player-against-player damage is unchanged, so a bounty hunter can still shoot a Kill on
  Sight parleyer. That's the risk of being Kill on Sight.
- **Hunter squads.** Sent squads keep hunting. A squad coming within 1 km breaks the parley: "They've got friends
  coming. So have we." While a parley runs (at most about 4 minutes), that faction's next squad round skips you, so a
  squad sent mid-parley doesn't spawn on top of you. Bounties and the bounty board carry on.

**The Haven's knock.** The Haven only shoots an exile who carries a firearm within 100 m of a safehouse, so there's no
hold and no shout.
- Stand empty-handed at a safehouse door (10 m), under the bear stencilled on it, and use **Knock**.
- The council head answers through the door, which stays shut. You get one callback and the offer.
- The same 2-hour limit and "just you" rule apply.
- Hostile Haven lines are a shut door, never gunpoint.
- **In pencil** (the finale's Haven price door, [FINALE.md](FINALE.md#price-doors)) uses the same Knock. After the 90%
  gate, at dusk, an exile who **isn't** owed can offer 4 food for the pot and a name to read (one of your own dead
  characters, or a crossed-out name you found). A council head (never the chair) writes you on the List in pencil.
  That opens the Haven's route, not the door, and forgives nothing.

**The amends offer.** One of that faction's listed amends contracts, reserved for you for 60 minutes and offered
although you're Hunted:

| Faction | Contracts |
|---|---|
| Haven | Medicine Run, else Survivors Trapped |
| Bastion | Burn the Bodies, else Recon |
| Red Hand | Fence the Goods |
| Halcyon | Field Samples |

- Blood Drive is left out: it needs Known and is once per life.
- If none can find a place now: "Come back later." The 2 hours aren't used.
- **The job finishes by its own rules, where it says.** Nothing is handed in at the gate, so there's no return trip and
  no free second parley.
- It works off its Fame as Infamy, from the budget ([§3.3](../docs/REPUTATION.md#working-off-infamy)).
- That faction's AI still shoots you elsewhere. The jobs are away from its people, and Medicine Run ends at a
  safehouse drop, where an unarmed exile isn't shot.

**A fix to built code (`RZ_Checkpoints.BastionContact`).** Today a Hunted, unarmed player within 150 m of a Bastion
checkpoint gets a 15 s `StandDown` renewed every tick, with no limit. That's a safe zone from every Bastion AI, squads
included. It gets the same rules as the parley walk-in:
- only while distance is falling, at most 90 s;
- closed while a Bastion squad is within 1 km or a zone has you;
- the approach uses the surrender hour.

Surrender's own 5-minute hold (built) ends early if you arm yourself within 200 m of the post or fire.

### 6.3 Tags and conditions for talk

**Tags, as in New Vegas:** [Trusted], [★ Nurse], [Threaten · …], [Pay 15 Favor], [Settle up]. A tier or background
check you don't meet shows greyed, with what it needs. Threaten always shows its cost and score, never its outcome.
Parley isn't a dialogue option: it's a gate.

**Conditions** that dialogue and errands may read. They're all on the card, or obvious:
- `tier(f)`, `rep(f)`, `fameBand(f)`, `infamyBand(f)`;
- `earnedFameBand(f)`, `fearedBand(f)`, `notoriety(f)`, `owed(f)`;
- `word(f)`, `hasDeed(f)`, `kills(f)`, `jobs(f)`, `budgetLeft(f)`;
- `picture()`, `pictureKind()`, `localDeed(seat)`;
- `sidedWith()` and `routeAtLock()` after the lock;
- the background.

There's no `manner()`: lines may vary by manner, but options may not.

## 7. Settling up

The built **Make amends** action at a faction's quartermaster gains a second price, **Settle up in Fame**:
- **2 Fame for 1 Infamy**, in steps of 10 Fame for 5 Infamy;
- at most 10 Infamy a real day per faction, from the shared budget;
- the stronghold's temper applies to the Infamy taken off, within the budget.

**Why it can't launder:**
- **Spent Fame is gone.** It can drop your Fame band and your tier. That's the price.
- No contract is involved, so nothing dodges rival shares. That Fame was earned with its shares already.
- With [§3.2](../docs/REPUTATION.md#standing-tiers), Fame above 75 isn't dead weight: it's what your next mistake costs.
  Spending it is a real choice.
- Hopping across a band edge costs Fame on both axes, and the Infamy doesn't come back for free. It's once a real day,
  and within 30 a life.
- At the budget's end it greys out, with the notice. Nothing is silently lost.

**What it does in practice.** Necessary Evil (Fame 81, Infamy 46) settles 10, ending at 61/36: Useful Trouble, which
counts as Known. That's the same access, a lighter name, and no more Wildcard search. The Trusted shelf needs work, not
settling.

## 8. Gates, roads and the company you keep

**The Wildcard tier** is built: searched at Bastion checkpoints (running that search +6) and the toll ×2 at Red
Hand roadblocks ([REPUTATION.md](../docs/REPUTATION.md#checkpoints-roadblocks-and-camps)). Still to build with disguise:
if you also wear Halcyon colours, the lab-coat toll and this one take the higher of the two (20), never both. There's
no danger money on shop prices.

**The company you keep.** A vanilla group that arrives together (members within 50 m of each other) is treated
together:

| Place | What happens |
|---|---|
| Bastion checkpoint | If one member would be searched, everyone there is: "We search the whole car." This applies **before any disguise look**: a disguised member of a searched car gets the look at the search. Running is still per player |
| Red Hand roadblock | The wave-through covers only Trusted members. Everyone else pays their own toll |
| Camps | Built per-player turn-aways, with a line: "Your friend waits outside." |
| Parley | Only alone ([§6.2](#62-the-parley-for-the-owed)) |

**Barks** (a setting):
- **Where from:** the gate or door point of a holding, safehouse or road post, never from a body. So a bark never gives
  away a lookout you can't see.
- **When:** within 25 m of that point, and only on a change: your first visit this life, or a new cell or deed since
  your last bark there.
- **Never:** while you're claiming in someone's kit, during a stealth stage you're in, or for the Hostile manner.
- **One notice per approach.** The bark becomes the first sentence of the post's own line, never a second notice. It's
  a rule of the shared notice lanes (approach lines: once per place per session).

**Others hear** (a setting, on by default). At notoriety 3 in a non-hostile manner, undisguised, other players within
30 m of the gate get the line too: "A guard nods at Mara. 'That's the one who held the lab.'" Once per 10 minutes per
place, for all players together. That's how late joiners learn who's who. It follows the names policy: the name is
frozen at the deed and filtered, an `/anonymous` player is "that one", and the admin strike removes it. Gate guards may
name a player face to face the same way.

**Bounty notices** add the name and the reputation, and a dry note about the faction for the owed: "Wanted by the Red
Hand: Mara, Merciful Thug. The book says she's owed, too."

**Unchanged:** hostility stances and the bitmask, hunter squads, retaliation, mission fights, the Haven's armed-exile
rule, Safe Harbour.

## 9. The faction's word for you

**Built:** the word per faction and cell, set words (`SetWord`), kin as "family", and Red Hand crew names from
Lucky, Snack, Legs, Skint, Pockets and Sorry, given at Known or better and said as leverage
([REPUTATION.md](../docs/REPUTATION.md#what-people-call-you)).

Still to build:
- **Owed** words: "put out, prayed for", "on the list, with a note", "in the book, both pages", "excluded, with
  thanks" (with the parley); **after the lock** a one-word overlay ([§13](#13-the-lock-the-cure-and-the-season-record)).
- **Crew names** from **Bailiff** (laid down a tribe's leader: with the tribes) and **Smoke** (threw a signal
  smoke: needs a throw hook), from a major (the pit, The book balances) and from a crew boss's dare (the Name Day
  card, [§15](#15-side-quests-what-reputation-gives-errands)).
- **Next season:** a crew name carries over only through the majors' "The book balances", as words only.

## 10. Deeds and tallies

**Built:** best and worst per faction with the kill-tally rule, kills and jobs, `m_sDeed` on the commonest
contracts (37) with "did a job for us near {town}" for the rest, leaders' and checkpoints' phrases
([REPUTATION.md](../docs/REPUTATION.md#reputations), [DESIGN.md](../docs/DESIGN.md#deeds)).

Still to build: phrases for the remaining contract classes; questline and major endings' few words; house rules
("fired at the door in {town}"); wanderer kills ("shot {name}, who ran {town}"); laying down a tribe's leader in
each faction's word ("laid the big one down at its lair", "put the heavy down", "evicted the boss up the
quarry", "brought in the index"); the Long Table's (you) flags; war deeds between locked players
([§13](#13-the-lock-the-cure-and-the-season-record)).

## 11. The whole picture

**Built:** `Picture` and `PictureKind` (nine kinds), and wanderers saying it one time in four after the road
news ([REPUTATION.md](../docs/REPUTATION.md#what-people-call-you)).

Still to build: the standing screen's header (`H` in the feed); the Deserter reading Bastion's side ("Bastion's
got you as a Necessary Evil. Means they'll use you till you're used up. Ask me how I know."); the Long Table's
host to an exile ("Put out, were you?"); the epilogue's picture from the season record.

## 12. Face and name: one scope rule

**Built:** life-stamped leader and wanderer memory (earlier lives at half, met doesn't carry, old saves as an
earlier life) and local deeds ([REPUTATION.md](../docs/REPUTATION.md#face-and-name)).

**Name (this season)**, still to build: the season record ([§13](#13-the-lock-the-cure-and-the-season-record)),
a crew name carried by the book, the route at the lock, all keyed by identity and wiped with the season.

## 13. The lock, the cure and the season record

[FINALE.md](FINALE.md#4-the-lock) owns the routes and the lock. Reputation supplies who may side, and what changes in
what people call you.

**Before 90%, reputation locks no finale route.** `CanSideWith(identity, f)` answers:
- **yes:** f doesn't hunt you;
- **by parley:** you're owed. A parley at f's gate at the lock opens its door: "You're owed one conversation. This is
  it.";
- **by price:** the route's price door ([FINALE.md](FINALE.md#price-doors)): In pencil at the Haven's knock; Sign on,
  attached at a Bastion checkpoint; a line in the Red Hand's book; the volunteers' book at a Halcyon field station.
  Each opens the route only and forgives nothing;
- **Independent:** always open, with no standing needed.

**At the lock** each player's choice is recorded, and the season record takes a snapshot.

**After the lock** the word gains a one-word overlay. Names don't change.

| You sided | The Haven | Bastion | The Red Hand | Halcyon |
|---|---|---|---|---|
| With them | with the house | attached | in on it | signed on |
| With a rival route | the other lot's | hostile element | the other crew | a confound |
| Independent | nobody's | unaffiliated | freelance | an uncontrolled variable |

- **Parley and settling up** stay open with your route's faction. They're closed with a faction whose route is
  **fighting** yours: either route has done a hostile route act against the other in the last 2 real days, or it's the
  stand ([FINALE.md](FINALE.md#fighting-routes)). Independents keep them with any faction not fighting them right now.
- **Kills between route players** (decision 9, [FINALE.md](FINALE.md#kills-between-route-players)):
  - outside the stand, killing a player locked to a faction route costs +3 Infamy with that faction, at most 12 an
    hour; killing an Independent-locked player costs +3 with the Haven;
  - at the stand, within 150 m of the lab, fire between locked players of different sides costs nothing;
  - it's ordinary Infamy, with credit erosion. Each is tagged *war* in the season record, so the ending can tell a war
    from a crime. Nothing new is paid for these kills; the built bounties are unchanged.
- **At the cure** all locks end, and every grudge and squad the contest started stands down once. Parley reopens with
  everyone: "It's over. Talk."

**The season record** (saved in `RZ_SaveData`, by identity, in the shared deeds store's season scope, wiped with the
season):
- per faction: the best and worst cell reached (by access rank), the cell at the lock, the biggest best and worst deeds
  across all lives, kill and job tallies, and Infamy worked off;
- the route at the lock, and one war deed;
- a crew name, if "The book balances" carried it;
- whether the season ended square with that faction ([§14](#14-the-last-week-settling-accounts)).

**What the in-world ending reads:** `SeasonPicture(ident)`, `SeasonDeed(ident, f)`, `SeasonCell(ident, f)`,
`RouteAtLock(ident)`, `CrewName(ident)` and `TitleFor(ident, f)` (the name from the best life). It speaks about the
**name**, so a player who dies in the epilogue isn't greeted as a stranger: "Don't know the face. Know the name."

## 14. The last week: settling accounts

After the cure the epilogue lasts **one week** (decision 12). [EPILOGUE.md](EPILOGUE.md) owns its closure set. Its
**Settle Accounts** (clear bounties, pay what you owe the book, give back borrowed kit, make amends) runs on these hooks,
and nothing in it is free:

| Account | How it's settled in the week | What reputation does |
|---|---|---|
| Hunted by a faction | The parley, open with everyone from the cure ("It's over. Talk."), once per 2 real hours per faction, with its closures except the lock | Its amends offer works off Infamy through `Forgive()`, inside the 30 a life |
| A Kill on Sight bounty | Leave Kill on Sight, through the parley's amends or settling up | The bounty comes down as built when you leave Kill on Sight |
| Infamy with a faction you can still visit | Amends, paying, settling up, the knock, Come In | All through `Forgive()`. The budget doesn't reset for the week |
| A line in the Red Hand's book (the price door's 60 Favor, a crew name's debt) | Pay it in Red Hand Favor at the boss's table | The book marks it paid. Nothing else changes |
| Borrowed kit (signed out to someone else) | Return it to its buyer's quartermaster ([DISGUISE.md](DISGUISE.md#13-the-world-tells-it)) | No catch, no Infamy |
| A leader at −2 | Make it right | Through `Forgive()`, once a season per leader |

**Square.** At the end of the week, a player is **square** with a faction if their Infamy with it is under 15 and they
have no unpaid line in the book. The season record keeps it, and the hall of fame can carry one line: "Left the Zone
square with all four", with the names policy. It changes nothing in the next season.

## 15. Side quests: what reputation gives errands

**Hooks:**
- the conditions in [§6.3](#63-tags-and-conditions-for-talk);
- `FillFor` tokens: {rep:f}, {word:f}, {best:f}, {worst:f}, {kills:f}, {jobs:f}, {picture}, {name}, {best_place:f};
- **local deeds at a seat.**

**Local deeds.** `RZ_Leaders.SetLocalDeed(seat, playerId, phrase)` stores one phrase per player per seat, life-stamped,
with the latest winning: "fixed the pump", "carried the Weber girl in". It's written by outpost and safehouse tasks and
by helping people there, and greeted second in the opening chain. So helping one place is remembered at that place,
with no new numbers. These are words a leader says ("threads"), never attitude: errands never write leader memory or
loyalty.

**Cards for the errands deck.** These are errand cards, dealt by [ERRANDS.md](ERRANDS.md) under its rules: a live Why,
the dealer, the interruption budget, and the small-pay ledger (errand Fame at most 2 a card and never above 39 with a
faction; Favor inside the hourly ledger; amends instead at 15+ Infamy). Leaders never give them: the leader says who to
see. They're weighted toward the cells players live in, each has a grey side, and none pays free Fame.

| Card | Offered to | What happens | The grey part |
|---|---|---|---|
| **Unknown Face** | Stranger with a faction, by one of its named people | A guard needs a letter carried into a rival's camp, because nobody there knows your face | Read it first. Deliver it, swap it, or sell it to the camp |
| **Vouch** (the one vouch card) | Known with the Haven, at a safehouse door | A newcomer needs someone Known to speak for them. Vouch, and you answer for them: +5 Haven Infamy once if they steal within a real day, then 2 hours without vouching | They will, a bit. Their kid's hungry |
| **Clean Slate** (in the amends family) | Watched, at that faction's outpost | The dirtiest local job (latrines, the burial pit, scrubbing cages) as an amends job, from the budget | "Nobody watches the man with the shovel." It puts you next to what they hide |
| **Both Hands** | Mixed Bag, Useful Trouble or Favourite Rascal | Someone needs you *because* you're liked and feared: talk a brother out of a crew, or an informer out of a post | He doesn't want to come, and his crew doesn't want him to go |
| **Name Day** | Known with the Red Hand, no crew name; once a real day | A crew boss offers two dares: fetch the clock from the infested church tower (Belfry), or one round in the pit (Ringside). The third dare, painting a red hand in sight of a checkpoint, is the Paint card ([DISGUISE.md](DISGUISE.md#12-cards-for-the-errands-deck)) | Stupid on purpose. The name you get is whatever went wrong |
| **A Debt Gone Quiet** (Collections, re-angled) | Feared band 2+ with anyone, from a Red Hand crew boss | Not a collection: a man in the book has stopped paying and stopped answering. Find out why before the crew does. Threaten (Fear), pay it yourself, or tell the boss he's dead | He's alive and paying for medicine. The book finds the lie later (a callback) |
| **Keep Me Off the Board** | Known or better with Bastion, with any Bastion Infamy | A private wants his name off a duty report | The report's true |
| **A Word for Me** | Champion with the Haven | Speak for someone at the council. The head listens because it's you | Two families want one room |
| **The Other Half** | Owed by one faction, Known with its rival | Carry a message to the people who want you dead. The parley is the delivery | The message is a threat |
| **Wanted Poster** | Kill on Sight with anyone | A wanderer recognises you from the bounty and offers to sell your location: to them, or to you for more | Pay him, threaten him, or let him sell it and wait for the squad |
| **Character Reference** | Known with two factions | One asks what the other is really like. Your answer becomes a rumour their leaders repeat | Lie, and the callback finds you |

## 16. What the disguise look reads

[DISGUISE.md](DISGUISE.md) owns every detection rule. The look reads these from reputation:
- your **tier** with the faction: Known or better in its kit is your own colours, never a disguise; Hunted or Kill on
  Sight can't claim at all;
- **Watched or Uneasy** (−1: they know your face), and `KnowsFace(identity, f, pos)` for having talked to their people
  undisguised this life;
- being on the **bounty notices** (Kill on Sight with anyone: −1);
- `RouteAtLock` for the lock's −1 with every faction whose route you didn't choose;
- the **crew name**, as a cover detail at the Red Hand's look;
- `AddInfamy`'s ACT flag, which may double an act done in sight while passing; shares (SHARE) never double.

`KnowsFace` is true within 20 m of anyone who met you **this life** (life-stamped Met, a questline contact this life),
or at notoriety 3 within 30 m. Barks and others-hear never name a player who is claiming in someone's kit.

## 17. In the UI

**Built** ([REPUTATION.md](../docs/REPUTATION.md#reputations), [UI.md](../docs/UI.md#the-notebook)): the card's name and
note, the budget on `InfamyValueN`, the code-filled grid (`RepCell_<fame>_<infamy>`), the feed's new `F` fields, the
fallback text, the first-mixed-name tip, the `/rep` read-out and the log line on every cell change.

Still to build:
- `TierNoteN` for the owed: "HUNTED · THEY OWE YOU A HEARING" (the feed's owed field is sent as 0 until the parley).
- The word, and the nearest met leader's verdict, as the first two lines of `LeadersN`: "The crews call you Lucky."
  (the feed's word field is sent empty until the word exists).
- `Header`: the whole picture, carried in `H`.
- **Notices, one per approach.** Priority: the gate's own line (with the bark folded in), then a change notice, then
  a tip. Anything lower waits until 30 s after the last notice from this system, and is dropped if older than 2
  minutes; a rule of the shared notice lanes.
- **Journal:** a parley's job reads "Amends: works off Infamy · finishes where the job says".

## 18. Multiplayer, saving, performance

**Per player:**
- Everything is per player. Nothing new is replicated except the owner's standing feed.
- In a group, each player is greeted and judged alone, except the checkpoint group rules ([§8](#8-gates-roads-and-the-company-you-keep)).
- A parley holds fire only for the parleyer.

**Keyed by identity** (a reconnect resets nothing):
- the parley's 2 hours and the settle day, as saved server time;
- Fear's once per person (on the person, not the player) and its three a life;
- bark cooldowns, which are in memory, so a restart resets them (documented).

**Saved, per life:**
- Fame gained and direct Infamy gained, the budget used, and the last settle time (×4 factions);
- the word or set word, best and worst deeds, kills and jobs (×4);
- the crew name and its event flags;
- the last parley time per faction;
- Fear threats used per faction.

**Saved, per season:** the season record. Memory entries gain a life number.

**Performance:**
- The name is a lookup on two bands, cached in `CheckTierChange`.
- Parley, barks and group rules run on the existing checkpoint and hostility contact ticks, over bodies already spawned
  near players.
- `IsEmptyHanded` scans the inventory only for a player who is mid-parley.
- No new entities, no new hostility variants, no Workbench prefabs, nothing left in the world.

## 19. Build order

Reputation lands in four steps across the plan's build order.

| Plan phase | Builds | Why then |
|---|---|---|
| 0. Fixes | The `BastionContact` hold fix; amends paid uncapped on the Hunted path in `RewardPlayer`; the SHARE flag so disguise doubling stops hitting rival shares | Built bugs |
| 1. Shared foundations | **Built** ([REPUTATION.md](../docs/REPUTATION.md#reputations)) | — |
| 2. Talk | **Built** ([REPUTATION.md](../docs/REPUTATION.md#what-people-call-you)) | — |
| 4. The mission style | The Fear check's trait modifiers, tags and quiet successes (the check itself is built); the parley and the knock; `IsEmptyHanded`; amends reservations; settling up; the company you keep | New rules, each small and capped |
| 6–7. Finale and ending | `CanSideWith`, the lock overlay, parley closed with fighting routes, war deeds; the season record and its APIs; barks and others-hear; the last week's hooks and "square" | Needs the finale and the epilogue |

## 20. Guardrails

| Guardrail | How this keeps it |
|---|---|
| Nothing overpowered | Names open nothing. Credit runs out at 75+. One 30-a-life budget for every route, and money never goes below 15. Settling costs 2 Fame for each Infamy. A threat always costs +3, works three times a life per faction, and its quiet success needs Infamy earned this life. The parley gives one conversation and one job, with no services. Price doors forgive nothing. No free loot, Fame or Favor |
| Strongholds and leaders | Stronghold leaders are never threatened. Strongholds never fall. Nobody is replaced |
| Performance first | Data only; only bodies already near players are read; no new entities |
| Works on any map | Everything is relative to holdings, posts, safehouses and named settlements |
| Sealed Zone | Not touched. Nothing hints that the outside answers |
| No sleep, no time skip | Limits run on real time: 2 hours, a real day |
| Multiplayer | Per player, keyed by identity; group rules are explicit; late joiners need nothing |

## 21. Fits with the majors and the tribes

**The majors** ([MAJORS.md](MAJORS.md)):
- Tier and persuade checks are unchanged.
- "Seen: +8 Infamy" is direct Infamy, so it feeds Feared and the worst deed.
- Honours go through `SetWord` ("Sergeant {name}").
- Crew names from the pit and from "The book balances" are the same crew names as [§9](#9-the-factions-word-for-you).
  "One step easier with a crew name" at settling day stays meaningful, because names are rare and mean the crews have
  something on you.
- "The book balances" carries crew names to the next season through the season record, as a words-only greeting.
- "You're the one who gave Bastion the book" is step 1 of the opening chain.
- The majors' own "Red Hand Talk checks need one tier more" is unchanged.

**The tribes** ([INFECTED.md](INFECTED.md)):
- Laying down a tribe's leader is a deed, in each faction's word ([§10](#10-deeds-and-tallies)). For the Red Hand it
  gives Bailiff.
- "Strays" stays the crews' word for the Haven's exiles.
- Player text never says "alpha", "tribe" or "crawler".

## 22. Doc changes when it's built

| Doc | Change |
|---|---|
| REPUTATION.md | Done in phase 1: the credit rule, the names, the budget table, the Wildcard rows, "How it maps to code". Still: in "Reputations", the faction's word, the picture, face and name; the checkpoints' groups, the parley and the knock; Disguises: links to DISGUISE's look; Leaders: life-stamped memory; the lock and the season record |
| WRITING.md (the writing guide, moved out of LORE.md) | How each faction says a name; the word; crew names "after something you'd rather they forgot" |
| DESIGN.md | Questlines: the Fear check and the quiet success (`m_iMenaceDifficulty`); wanderers' picture line; leaders' local deeds |
| MISSIONS.md | Amends: the parley offer and the budget; `m_sDeed` in "Writing a contract" |
| QUESTLINES.md | Threat stages that have a quiet success |
| UI.md and UI/LAYOUTS.md | Done in phase 1: the `RepCell_*` grid, the note. Still: `LeadersN` first lines, notice priority |
| WORKBENCH_SETUP.md | Factions > Reputation settings, log lines, the play-test checklist |
| ROADMAP.md and README | Planned → Built per step; "Dropped" (danger money on prices is there since phase 1): manner job weights, Call it square, background crew names, each with one line of reason |

---

## Examples

**Two jobs and a gate.** A Nurse (Haven +10 at start) has fed Morton twice, so Haven Fame is 46. She fired at Levie's
door during a raid, and a Red Hand job's share fell on the Haven, so Haven Infamy is 22.
- With the Haven she's **Useful Trouble · counts as Known**.
- At Morton, on her first visit this life, the gate's bark is folded into the safehouse's welcome: "Here she is. Mind
  the windows."
- She asks the council head what they think of her: "People have started calling you Useful Trouble. You fed Morton on
  Day 14. You also fired at the door in Levie on Day 16. I'd rather they didn't have to."
- Then the rule line: "(The Haven: Useful Trouble. Counts as Known. Fame 46, Infamy 22. 30 of 30 left to forgive.)"
- Prices are as they were. There's no danger money.

**Wildcard at the barrier.** Bastion Fame 52 and Bastion Infamy 44, all of it her own doing (Bust Out, and two stockade
guards).
- She's in the Wildcard tier, so she's searched although she counts as Known: "Command would have you shot or decorated.
  Command isn't answering. Arms out."
- At a stage of The Census, the commander won't open the ledger. Persuade is Short: rank 1 against difficulty 2.
- The threat's tag reads "[Threaten · +3 Bastion Infamy · Fear 2 of 2]". Her Feared band is 2, which meets the stage's
  menace difficulty of 2, so the built FORCE threat works quietly and nobody warns the post.
- Bastion Infamy goes to 47. Her Fame is under 75, so there's no erosion.
- The commander remembers, for this life, that she crossed him. It's her first of three threats with Bastion this life.

**The credit runs out.** A Bastion Wild Child (Fame 78, Infamy 76) shoots two stockade guards. Each retaliation kill is
+3 Infamy and, at Fame 75+, also −3 Fame. She ends at Infamy 82 and Fame 72: **Soft-Hearted Devil · Hunted**.
- The tier notice: "Bastion: Soft-Hearted Devil. (Hunted: their soldiers shoot you. You'd run up a lot of credit. It's
  gone.)"
- Hunter squads start coming. Before this fix she could have gone on killing at Infamy 100 and never been hunted.

**A Merciful Thug parleys with the Red Hand.** Red Hand Fame 20, all earned this life (her best deed: "fenced the goods
in Levie on Day 9"). Infamy 82, with a kill tally of 9: Kill on Sight.
- **Getting there:** she shakes the last squad at the river and waits until no squad is within 1 km. She drops her
  rifle and grenades 300 m out, and her friend backs off past 150 m.
- **The shout:** "Stop there. Empty hands, or not at all." They hold fire while she walks in.
- **At the barrier:** searched for 10 s. Then the crew boss, over the radio from inside: "You put nine of ours down, and
  you fenced the goods in Levie on Day 9. The book's got you on both pages. It has to balance. Here's the bill."
- **The offer:** Fence the Goods, reserved for her for an hour. She walks away, and the hold lasts until she's 200 m
  out.
- **The job:** done two towns over. It works off 10 Infamy: 82 → 72 (budget 20 left).
- **The result:** she's now **Bad News · Hunted**. The Kill on Sight bounty comes down; the squads don't.

**The Haven's knock.** An exile has Haven Fame 48, all earned ("carried a family into Levie on Day 11"), and Infamy 77
("shot Karel Novak, who ran the Levie house"). She stands empty-handed under the bear on the Levie door and knocks.
- The new council head, through the shut door: "You carried a family in on Day 11, and you shot Karel Novak. The door
  stays shut. I'm still talking to you, aren't I? The pharmacy in Morton. Bring the boxes to the step."
- That's Medicine Run as amends: about 10 Fame, ×1.2 under the Haven's Merciful stronghold leader, so 12 off. 77 → 65
  (budget 18 left).
- She's **Wildcard**: no longer exiled. She gets the house back, and a seat by the door where they can see her.

**Settling up.** Halcyon has her as Necessary Evil (Fame 81, Infamy 46): "We have you as Necessary Evil. It's a
category, not a judgement. Contribution: brought in eleven samples by Day 19. Incident: shot a courier outside Levie on
Day 20."
- At the lab stores she settles 10 Infamy for 20 Fame: 61/36, **Useful Trouble · counts as Known**. Same access, a
  lighter name, and no Wildcard search at Bastion's checkpoints either.
- The antidote shelf stays shut. Two Field Samples later (Fame 81, Infamy 36) she's **Favourite Rascal · counts as
  Trusted**, and it opens. Work did that, not settling.
- Her budget shows "20 TO FORGIVE". Tomorrow she could settle another 10.

**A crew name.** An Ex-Con starts Known with the Red Hand. The roadblock calls her "new face". Nothing in her background
counts.
- On Day 3 the cut dose works on her. That evening: "Lucky! Heard the dose took. Nobody's that lucky twice. Toll's ten."
  The crews have something on her now: she took the cut dose and lived.
- Her card reads: "The crews call you Lucky."
- Her friend, also Known with the Red Hand, has done nothing stupid yet. A crew boss deals him Name Day. He takes
  Ringside, is knocked flat in the first ten seconds and survives the round. He's "Ringside" for the rest of the life,
  and every crew knows why.

**Unknown Face, for a Stranger.** A Bastion corporal at Morton outpost: "Nobody at the Red Hand camp knows your face.
Lucky you. Take this to the man with the limp."
- She reads it on the way. It's a price for a name on Bastion's list.
- She can deliver it, swap it for a blank page, or sell it to the camp's boss. Whatever she does, it pays the usual
  small errand amount.
- Her card doesn't move much, but the corporal's local deed now reads "carried the letter", and he greets her with it
  next time.

**The company you keep.** Three players in one vanilla group drive up to a Bastion checkpoint. Two are Known with
Bastion; the third has Bastion Infamy 41 and is a Wildcard.
- "We search the whole car." All three are searched, before anyone could claim to be one of them. The Wildcard's
  suppressor goes into the crate.
- At the Red Hand roadblock down the road, the one Trusted with the Red Hand is waved through. The other two pay their
  own tolls.

**The lock.** At 90%:
- Mara is Known with Bastion and locks to Bastion's route. Bastion's word for her becomes "civilian, cleared, attached".
  The Red Hand's route ambushes a Bastion carry the next night, so the two routes are fighting: the Red Hand now calls
  her "the other crew", and parley with the Red Hand is closed for 2 real days after the last hostile act, or until the
  cure.
- Pavel is Hunted by the Red Hand and owed nothing. He can't side with the Red Hand by standing. He can pay its price
  door (a line in the book), or go Independent. He goes Independent on the radio. The Haven calls him "nobody's", and
  Halcyon "an uncontrolled variable".
- Outside the stand, a Red Hand-locked player shoots Mara on the lab road: +3 Bastion Infamy for him, tagged *war*.
- After the cure, Pavel knocks at the Red Hand's gate: "It's over. Talk."

**Face and name.** Mara dies on the epilogue's first evening and spawns as a Nurse.
- At Morton the guard doesn't know the face: no bark, a Stranger's greeting.
- The notice board does know the name, from the season record: "Don't know the face. Know the name. Mara fed us on Day
  14."
- The Levie council head, crossed by her last life, says it once: "There was a Mara who fired at my door. You're not
  her. Are you?" His attitude counts it at −1, not −3.
- Before the week ends she parleys with Bastion, works off the last of her Infamy through an amends job, and pays her
  line in the Red Hand's book. The hall of fame says she left the Zone square with all four.

---

## Engine needs

Built in phase 1 and not repeated here: `RZ_ERep` and the names (`Factions/RZ_RepNames.c`), the profile's gains,
budget and threats, `AddInfamy`/`AddFame` with their flags and the credit rule, `Forgive()` on the built routes, the
debounced change notice, `RZ_Checks` (Trust, Fear, threats) and `RZ_Ways`, and the Wildcard tier at posts
([REPUTATION.md](../docs/REPUTATION.md#how-it-maps-to-code), [DESIGN.md](../docs/DESIGN.md#checks-and-ways)).

- **`RZ_RepNames`** additions: `enum RZ_EPictureKind` (9, in match order); `GetManner`, for writing only.
- **`RZ_PlayerProfile`** per life: `m_aSettleUnix[4]` and `m_aParleyUnix[4]` (server time).
- **Call sites marked `ACT`** one by one (today a direct gain without SHARE counts as done in sight), and offline
  profiles never doubled.
- **`Forgive()` callers still to come:** parley amends, settling up, Make it right, errand amends, the epilogue's Come
  In and the finale's Hand it in.
- **`RZ_Checks`:** the trait modifiers; refused during a parley and for free-standing talk in safe zones and
  safehouses; crossed memory written, life-stamped.
- **`RZ_Checkpoints`:**
  - the lab-coat toll, and the Wildcard toll taking the higher of the two;
  - group search before any look, per-member tolls, the camp line (vanilla group, 50 m);
  - surrender's 5-minute hold ends if the player arms within 200 m or fires.
- **The parley** in `RZ_Checkpoints` (posts, camps, the lab and field station gates):
  - the closing conditions: a squad of that faction within 1 km, any zone or grudge, another player within 150 m, the
    2-hour limit by identity, the finale's fighting flag;
  - the 150 m shout only when open;
  - the walk-in hold: `StandDown`, 5 s at a time, only while distance falls and within 200 m, at most 90 s;
  - a 10 s search at the barrier;
  - the leader's voice as a notice;
  - the walk-away hold to 200 m or 90 s;
  - breaks on the parleyer's own group's fire; pauses on others' fire, with a grudge on the shooter;
  - `RZ_FactionPerks` skips that faction's squad round for the parleyer while a parley runs.
- **The Haven's knock:** a `Knock` action on the safehouse door or quartermaster for owed exiles, empty-handed within
  10 m, with the same limits; after the gate, also In pencil for exiles who aren't owed (the finale's price door).
- **Amends reservations:** an `RZ_Mission` reservation offers the contract to one player for 60 min although Hunted, and
  it finishes by its own rules. If none can be placed, the 2 hours aren't used.
- **`RZ_Util.IsEmptyHanded(character)`:** no firearm in weapon slots *or* storage, and no grenade or charge (a keyword
  list in settings). It's checked every tick during a parley or a knock. VERIFY: the inventory iteration API (reuse
  the item walk `RZ_FoodHelper.TakeFood` uses).
- **Settling up:** a second price on the built Make amends action in `RZ_FactionServices` (10 Fame for 5 Infamy,
  through `Forgive`, 10 a real day), greyed out with a notice at the budget's end.
- **`RZ_RepVoice`**, built in phase 2 ([REPUTATION.md](../docs/REPUTATION.md#what-people-call-you)). Still: bark,
  parley, knock and change lines; the crossing opener; `OpinionText` and the Long Table host routed through it; the
  wanderers' greetings by manner; per-faction tokens (`{rep:f}`, `{word:f}`, `{best_place:f}`) for lines about
  another faction; deed phrases for the remaining contracts, questline and major endings, house rules, wanderer
  kills, tribe leaders and the Long Table's flags; Bailiff and Smoke, and crew names from a major or Name Day.
- **UI:** `RZ_UIFeed.BuildStanding` appends to the `F` record (rep id, notoriety, word, note, budget left, owed) and puts
  the picture in `H`; `RZ_StandingMenu` fills `TierNameN`, a short `TierNoteN`, the budget in `InfamyValueN`, the first
  two lines of `LeadersN`, and the 16 `RepCell_<fame>_<infamy>` widgets (the name plus faction initials);
  `DescribeStanding` fallback text; a one-time guidance tip.
- **Barks and others-hear:** from gate and door points on the existing contact ticks; first visit or change only;
  suppressed while claiming in someone's kit (`RZ_Disguise`), during a stealth stage the player is in, and for Hostile;
  folded into the post's line; others-hear at notoriety 3, non-hostile, undisguised, 30 m, once per 10 min per place,
  through `RZ_Names`. Wanderers' picture line after road news, one time in four; the Deserter's Bastion read; the Long
  Table host's line for exiles; bounty notices with the name and the owed note.
- **The season and the lock:**
  - **The `RZ_SaveData` season record by identity:** best and worst cell per faction, the cell at the lock, the season's
    biggest deeds, tallies, Infamy worked off, the route at the lock, one war deed, a carried crew name, and "square".
    Wiped with the season.
  - **APIs for the finale and the epilogue:** `CanSideWith(identity, f)` (yes / by parley / by price / no),
    `SnapshotAtLock()`, `SeasonPicture(ident)`, `SeasonDeed(ident, f)`, `SeasonCell(ident, f)`, `RouteAtLock(ident)`,
    `CrewName(ident)`, `TitleFor(ident, f)`, `IsSquare(ident, f)`.
  - **After the lock:** the word overlay, and parley and settling closed with factions on a fighting route; war deeds;
    at the cure, locks end and parley reopens with everyone.
- **For the disguise look:** `Notoriety(identity, f)` and `KnowsFace(identity, f, pos)`.
- **Settings** still to add to the built `RZ_ReputationSettings` (Factions > Reputation): the quiet success on; parley on, with 120 min, 1 km squad radius, 150 m alone radius,
  200 m hold radius and 90 s walk; settling on, at 2 Fame per Infamy and 10 a day; group rules on; crew names on; barks
  on and others hear on, with 25 m / 30 m and 10 min.
- **Tests:** `tools/compile_check.sh` after each step. A runtime-test path walks a fake player through the cells with
  `AddFame` and `AddInfamy`, and checks: the debounced change notice and its log line; credit erosion at 75+; that the
  budget stops every route at 30 and paying at 15 and 15; Earned Fame and Feared ignoring background and SHARE; Fear's
  score, once-per-person and three-a-life rules; parley closure with a fake squad within 1 km and its hold radius, the
  pause on third-party fire, the 2 hours by identity across a reconnect; settling's daily cap; `FillFor` with missing
  deeds; `PictureKind` covering every combination; a death and respawn near a leader who remembers the previous life.
  Log time spent per cell, to weight the line pools.

## Options decided

| Question | Decided |
|---|---|
| Names for the nine mixed cells | Six Zone names plus New Vegas's Merciful Thug, Soft-Hearted Devil and Wild Child |
| Plain cells | Keep the tier names, so the rules and the people share one word |
| The Fame 75+ corner | The credit runs out: at Fame 75+, every Infamy gain also costs the same Fame |
| The forgiveness budget | 30 per faction per life, shared by every route; paying keeps its floor of 15 and sub-cap of 15 |
| Settling up | 2 Fame per Infamy, 10 a real day |
| Parley | Bastion, the Red Hand and Halcyon gates plus the Haven's knock, once per 2 real hours per faction |
| Threats | One Fear check everywhere, with the quiet success on built FORCE stages, three a life per faction |
| Persuade | Your access rank, mixed tiers one lower; the built `PersuadeScore` stays (decision 11). Each member of a group has their own one try per job, so a group brings its talker |
| The Wildcard tier at posts | Searched at Bastion checkpoints (running it +6) and a double Red Hand toll, never stacked with the lab-coat toll |
| The company you keep | Group search before any look, and per-member tolls |
| Barks and others-hear | On, on a change only, from gate points, through the names policy |
| Red Hand crew names | Given at Known for something you'd rather they forgot, by the majors, the deed list or Name Day; "new face" until then |
| Memory scope | Life-stamped, with earlier lives at half and voiced as the name |
| After the 90% lock | The word overlay, with parley and settling closed with fighting routes until the cure |
| A Hunted player who isn't owed, at the lock | Can side by the route's price door, or go Independent |

## Risks

- **Two vocabularies.** Tiers are the rule words and names are the people's words. Mitigations: plain cells share the
  tier word, the card always shows "counts as", requirements never use a name, and no rule reads a manner. Play-tests
  should check that players still understand "needs Trusted" when they're Favourite Rascal.
- **The forgiveness budget changes built behaviour.** Watched amends contracts used to be uncapped with no floor, and
  surrender was uncapped. A long-lived loyalist's rival-share Infamy now piles up beyond what 30 can fix, so a Bastion
  lifer will end up Hunted by the Red Hand. That's the New Vegas way, and the lock's price door and the Independent
  route keep it from locking the finale. Play-test whether 30 feels fair. The setting can move it.
- **The credit rule changes built behaviour too:** a Champion's accidental Infamy now costs Fame. The amounts are small,
  but REPUTATION.md must say it plainly, and the tier notice must explain a Champion → Trusted drop.
- **The parley's hold is still faction-wide in the engine.** It's made local only by its conditions: within 200 m of the
  gate, nothing else of that faction on you, a squad within 1 km closes or breaks it, and it runs only while you're
  walking. A far-off crew of the same faction that wanders within range during those 90 s would also hold fire. It's
  marked *Simplified*; watch it in play-tests.
- **`IsEmptyHanded` depends on walking the inventory.** That needs VERIFY on the engine API and a keyword list for
  grenades and charges. If modded weapons don't match, someone could carry one into a parley. Mitigations: the barrier
  search runs the same check, and nobody walks out to meet you, so there's no one to ambush but the gate crew.
- **Parley still leaks a little intel**: guard counts at a gate, from 35 m. It's no more than binoculars give from
  300 m. "Just you" within 150 m, the 2-hour limit and the walking-only hold keep it from staging an attack.
- **A bounty hunter can still shoot an unarmed Kill on Sight parleyer.** Player-against-player damage is unchanged on
  purpose. That's the price of being Kill on Sight, and the notice says so.
- **Content volume.** About 200 short lines: greetings, Ask-what-they-think variants, barks, parley and knock lines,
  words, picture lines and overlays. Every pool falls back to a generic line per manner, so it can ship with gaps.
  Weight the writing by the runtime test's time per cell; Stranger, Known, Watched and Mixed Bag first.
- **Deed phrases.** About 100 contract classes each need an `m_sDeed`. The fallback ("did a job for us near {town}") is
  safe but dull, and a bad phrase repeats for a whole life. Write phrases for the commonest contracts first.
- **Life-stamped memory touches saved leader and wanderer data**, plus `Attitude`, prices, rogue jobs and the Deserter. A
  migration bug could reset or double memory. Old entries must load as an earlier life, and the runtime test covers a
  death and respawn near a leader who remembers the previous life.
- **The quiet success needs stage writers** to set `m_iMenaceDifficulty` and write a quiet outcome. Stages that don't
  stay exactly as built, which is safe, but the feature only shows where someone wrote it.
- **Fear's three-a-life cap** may feel arbitrary to a Raider who lives by threats. It's a setting; play-test it.
- **Group rules can feel unfair** to a clean player searched because of a friend. That's the intent ("the company you
  keep"). It's a setting, and the line says why.
- **Interfaces across the pieces.** This design supplies the condition API and `FillFor` tokens, `Picture`, `Notoriety`
  and `KnowsFace`, `SetWord` and `SetLocalDeed`, Fear, `Forgive`, `CanSideWith`, the season record and the lock hooks.
  The review of every API name before each phase must check that no other piece adds its own Fame-and-Infamy labels.
- **Others-hear lines name players to people within 30 m**, only at gates and only at notoriety 3. Some communities may
  dislike it, so it's a setting, and `/anonymous` turns it into "that one".
