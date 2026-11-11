# Questlines

Long stories that run over many sessions, New Vegas style: three per faction, and two for the
Long Table. Each stage can be done several ways. Choices set flags that later stages, other
questlines and the ending read. Every ending pays the player and moves the world a little,
always through systems that already exist.

**Status: designed, not built.** How a questline runs (stages, waits, conditions, flags, who
takes part, saving) is in [DESIGN.md](DESIGN.md), "Questlines". Who the people are is in
[LORE.md](LORE.md); standing, tiers and leaders are in [REPUTATION.md](REPUTATION.md); the
missions the stages reuse are in [MISSIONS.md](MISSIONS.md).

## The questlines

| # | Questline | Giver | Needs | Spans | Crosses |
|---|---|---|---|---|---|
| H1 | [The List](#h1-the-list) | Haven | Known; Trusted by the end | 6 stages, 3–5 sessions | B2, R2, C1 |
| H2 | [The Price of the Door](#h2-the-price-of-the-door) | Haven or Red Hand | Known with either | 5 stages, 3–4 sessions | R1, R2, R3, B2, C1 |
| H3 | [The Edge of Town](#h3-the-edge-of-town) | Haven | Known | 5 stages, 3–5 sessions | HC1, HC2, R1 |
| B1 | [No Reply](#b1-no-reply) | Bastion | Known; Trusted for the stove | 5 stages, 3–5 sessions | R1, B2, H2 |
| B2 | [The Census](#b2-the-census) | Bastion | Known | 6 stages, 4–6 sessions | H1, H2, R3 |
| B3 | [The Last Pilot](#b3-the-last-pilot) | Bastion | Known; Trusted to talk him down | 5 stages, 3–5 sessions | HC3, R1 |
| R1 | [The Book](#r1-the-book) | Red Hand | Known; Trusted to return it | 5 stages, 3–5 sessions | H2, H3, B1 |
| R2 | [Cargo](#r2-cargo) | Red Hand (or Haven) | Known with either | 5 stages, 3–5 sessions | H1, H2, C1 |
| R3 | [The Well Story](#r3-the-well-story) | Red Hand | Known; Trusted at the end | 5 stages, 3–4 sessions | H2, B2 |
| HC1 | [The Three Weeks](#hc1-the-three-weeks) | Halcyon | Known | 5 stages, 3–5 sessions | H3, HC3 |
| HC2 | [The Other Two](#hc2-the-other-two) | Halcyon or Haven | Known with either | 5 stages, 3–4 sessions | H3 |
| HC3 | [Authors](#hc3-authors) | Halcyon | Trusted, cure 50%+ | 6 stages, to the final stand | B3, HC1, H3 |
| C1 | [The House That Went Quiet](#c1-the-house-that-went-quiet) | Nobody (the world) | Any | 5 stages, 2–4 sessions | H1, H2, R2 |
| C2 | [A Place at the Table](#c2-a-place-at-the-table) | The host | Ate with them twice | 4 stages, 2–3 sessions | C1 |

A session is a few hours of play. A questline usually takes 2–5 real days, because of its
waits and because some stages only open at dusk, at night or at a cure percentage.

## How to read a questline

- **Premise:** who asks, and the hard edge it's about ([LORE.md](LORE.md)). People are named by
  role, because leaders, wanderers and named people are generated each season.
- **Needs:** tier with the giver, other standing, ★ backgrounds that get an easier or better
  approach, 🔒 locks.
- **Stages:** each one offers one mission, or waits for time or a world condition. Waits count
  real time with players online. Nothing is ever skipped or slept through.
- **Approaches** in each stage:

  | Approach | What it means |
  |---|---|
  | **Talk** | Speak to a leader, a wanderer or a named person. Some lines need a standing check (a tier with someone) or a background ★ |
  | **Sneak** | Do it unseen: lookouts, a heat limit, the night shift |
  | **Fight** | Kill what the stage spawned. Usually costs Infamy with someone |
  | **Bribe** | Pay in a faction's Favor |
  | **Betray** | Take the job, then hand the result to someone else |
  | **Side with** | Bring another faction in: Fame with it, Infamy with the giver |

- **Sets** a flag in `code style`. Flags are the server's for the season. Where a choice is the
  player's own (they'll be remembered for it), it says *(you)*: that belongs to the character
  and ends with the life, like reputation.
- **Uses** names the existing mission or building block a stage reuses. A stage that needs an
  optional prefab offers that approach only when the prefab is set, as missions do.
- **Endings:** what the player gets, and what the world gets.

### Limits every questline keeps

These are the design's targets; where the engine sets its own caps, its caps win
([DESIGN.md](DESIGN.md), "Questlines").

| Consequence | At most, per ending |
|---|---|
| Fame / Infamy / Favor | 25 Fame, 40 Infamy, 60 Favor with any one faction |
| Items | a reward crate, or one item from the giver's shelf at your tier. No new items, no new perks |
| Faction morale | ±10 per faction, recovering as usual |
| Faction strength | ±5 per faction, drifting back as usual |
| Leaders | one outpost or safehouse leader replaced, or one loyalty swing. Stronghold leaders never change |
| Territory | one outpost changes hands, by the usual strength rules. Never a stronghold or the lab |
| Towns | ±15 infection in one town; one town flag (Barricaded, Burned) |
| Safehouses | ±4 people and ±4 food at one safehouse; never below the upkeep floor of 2 people |
| Wanderers | up to two added (an existing role) or one gone |
| Truces and feuds | one started, under the usual rules |
| Research | +3 over the whole questline (scaled like any mission's research) |
| Contract weights | ×0.8–1.2 for one contract, for the season |
| Words | headlines, rumours, open-channel lines, leaders' lines: as many as fit |

And always:

- **The Zone stays sealed.** Command never answers. Nothing and nobody crosses the line, either
  way. The gates open only with the cure. A questline can find out what the outside did *before*
  the gates closed; it never makes the outside act.
- **No faction wins or falls through a questline.** Its Fame counts toward the season's winner
  like any other Fame. Strongholds never fall.
- **People are data until a player is near.** Everyone a stage needs is spawned only within a
  few hundred metres of a player; nothing a questline leaves is a physical trace.

### Crossing lines

Siding against a faction matters in that faction's own questlines:

- **Personal:** every ending records *(you)* which faction you worked against, if any. That
  faction's questline Talk checks need one tier more from you this life, and its leaders open
  with it ("You're the one who sold the List.").
- **The world:** the flags in [Crossings](#crossings) open, close or bend stages in other lines.

---

## The Haven

### H1. The List

Every Haven house keeps a list of names, and some are quietly crossed out. A woman at a safehouse
found her husband's name crossed out while he was still out on a supply run. He never came
back. Her council head wants to know who crosses names out, and why, without the chair of the
council finding out who's asking. The answer is the Haven's oldest shame. In the first hungry
week the council sent its "mouths" to a farmhouse past the last village that said it could feed
them. The farmhouse went quiet, and the council crossed the whole house off.

**Built** ([MISSIONS.md](MISSIONS.md#the-list-the-haven)). The contract The List is one crossed-out
name ([MISSIONS.md](MISSIONS.md#the-haven), #21); this questline asks whose pencil it was. Changes
from the design: it also opens once that contract has been done this season, and "lost people to
upkeep" is read as "ran short, or lost people lately"; the council head asks on the radio, in their
voice; the asking house's own list is the first copy (fewer copies on a map without the houses);
the reading is held at the gate on the road in. In a season with the Long Table, stage 4's farm is
its house (unless it moved before H1 began), and getting there is the finding: lived in, the house
is found (`list.found_table`); gone, the table is left laid, or burnt. There's nothing to look round
there, and the meal is the house's own. The names read raises the Long Table's suspicion through
`list.read`; Sold doesn't change the exile-leak chance (`list.to_redhand` is published instead).
*(you)* choices are season-wide flags and the record of who took part.

**Needs:** Known with the Haven; Trusted (or ★ Elder) to win the last stage by talk. Not while
exiled. ★ Radio Op, ★ Police, ★ Elder.
**Starts:** an Independent or Rogue council head, once the contract The List has been done this
season or any safehouse has run short or lost people lately.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Find the husband** | **Follow his trail**: a note, his pack, a third clue. Half the time he's alive: he walked off when he saw his name crossed out, and comes home (+1 person, `list.husband_alive`). **Ask the wanderers**: a Scavenger sells his last sighting for 1 food. ★ Radio Op: hears his last call on the Haven channel, grid included | Trail of Clues |
| 2 | **Get three copies of the List** | **Talk** to council heads: a Merciful one hands it over; a Loyal one asks the chair first and refuses unless you're Trusted; a Paranoid one won't. **Sneak**: read it in the stores at night past two lookouts (seen: +8 Haven Infamy, `list.stolen` *(you)*). **Bribe**: deliver 3 food to that house. **Side with the Red Hand**: their list of strays counts as one copy, for 15 Red Hand Favor | A Quiet Arrangement's lookouts; delivery |
| 3 | **Read the pattern** | **Work it out** at a safehouse: the names crossed out in the same week on all three copies, against the ration notes, point to one farmhouse. Wrong: locked 5 min; twice: you only learn "a farm out that way" (`list.half_truth`). ★ Elder or Police: the method spelled out. **Ask the chair** (Trusted): she tells you half and asks you to stop (`list.chair_asked`) | Patient Zero's Ledger (a deduction) |
| 4 | **Go and look** | If the Long Table lives there this season, it's their house: the host meets you at the gate and sets a place (C1 starts at stage 2, `list.found_table`). If not, an empty farm: nine graves, and one too few. **Look round** (three points, 10 s each). **Sit down and eat** (the guest rule applies, [REPUTATION.md](REPUTATION.md#the-long-table)). **Leave and report** | Visit points |
| 5 | **Wait** | One session (2 real hours with players online). Gossip in the news: *"Someone's been asking about the List."* Then the chair of the council asks to see you | Wait |
| 6 | **The chair's decision** | **Talk: read it out**, every house, crossed names too (Trusted, ★ Elder, or Known with `list.husband_alive` as proof): then hold the reading at the stronghold safehouse while the bells bring the infected (3 min, 2 waves). **Keep it quiet**: give the copies to the chair (`list.buried`). **Side with Bastion**: deliver them to a Bastion seat for its census (`list.to_bastion`). **Side with the Red Hand**: sell them to a crew boss (`list.to_redhand`). Either delivery must pass two Haven lookouts unseen | Hold; delivery |

**Branches:** stage 1 decides whether you have proof at stage 6. Stage 2's Red Hand copy makes
`list.to_redhand` cheaper to reach (the crew boss already trusts you). Stage 4 starts C1 when the
Long Table is there.

| Ending | You get | The world |
|---|---|---|
| **The names read** | Haven Fame +20, 30 Favor; the council head who asked remembers you as having *saved* them | Haven morale −8, recovering; the safehouse leader who crossed out the most names is replaced; the Long Table's suspicion +30. Headline: *"Every Haven house read the List at supper last night, crossed names and all."* |
| **Buried** | Haven Fame +10, 20 Favor; the chair remembers you helped | Nothing changes. If the Long Table is found later, the Haven deals with it quietly (C1 leans to walking them out) |
| **The census** | Bastion Fame +15, 30 Bastion Favor; Haven Infamy +15 | Haven morale −10; players the Haven exiled take +5 Bastion Infamy (as Names for Bastion); B2 starts at stage 3 |
| **Sold** | Red Hand Fame +10, 40 Red Hand Favor; 20% it's found out: Haven Infamy +20 | A hungry safehouse sells an exile's location 35% of the time instead of 25%, for the season; R2 reads `list.to_redhand` |

### H2. The Price of the Door

A hungry safehouse pays the Red Hand to be left alone. This month the crew boss doesn't want
tins. He wants a stranger: a man the house put out at night for stealing from the stores, and
where he sleeps now. The council head would rather not know what she's selling. Someone has to
carry the envelope.

**Built** ([MISSIONS.md](MISSIONS.md#the-price-of-the-door-the-haven-or-the-red-hand)). Stage 1 is
A Quiet Arrangement's walk, with its lookouts. Changes from the design: "short" means hungry, or it
has lost people lately, and never the chair's house or one at the Preacher's prayers; stage 4 is a
wait, then the vote; Bastion's checkpoint for the night is in words, and Under Bastion doesn't make
road posts likelier (`door.bastion` is published); Paid in tins is −4 food once and `door.tins`,
not a cost at every second upkeep; the cook takes the chair in words (the leaders' usual successor
fills the seat). `cargo.exile` is now `door.exile_sold`, set whenever he's walked to the crew; in a
season with a Long Table he's one of the people who never arrive.

**Needs:** Known with the Haven or with the Red Hand (each side gives its own version). ★ Ex-Con,
★ Elder.
**Starts:** an Independent or Rogue council head of a short safehouse; or the crew boss at the
nearest Red Hand outpost.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Carry the envelope** | **Deliver it** to the crew's drop past two lookouts (`door.paid`). ★ Ex-Con: lookouts spot you only at 25 m. **Open it**: a grid and a name (`door.knows` *(you)*). **Refuse**: the house finds someone else; go to stage 3 (`door.refused`) | A Quiet Arrangement |
| 2 | **The man at the grid** | He's a Haven exile, sleeping in a barn. **Talk: warn him**: he moves on as a wanderer elsewhere (`exile.warned`). **Bring him back to plead** (escort to the safehouse; he needs someone Trusted with the Haven, or ★ Elder, to speak for him at stage 4) (`exile.home`). **Betray: hand him over**: walk him to the crew's drop (+15 Haven Infamy if a Haven wanderer or lookout sees, `exile.sold` *(you)*). **Leave it**: the crew finds him in a day | Escort |
| 3 | **The crew comes to collect** | Only if the price wasn't paid. **Fight**: hold the safehouse gate against a Red Hand raid, 3 min (`door.held`). **Bribe**: 40 Haven Favor or 25 Red Hand Favor to the crew boss (`door.bought`). **Talk him down** (Trusted with the Red Hand, or ★ Ex-Con): a debt is a debt, but it can be yours (`door.debt_yours` *(you)*). **Side with Bastion**: bread to the nearest Bastion post for a patrol, and a Bastion checkpoint goes up on the road for the night (`door.bastion`) | Trader Caravan's raid; Over the Fence |
| 4 | **Wait, then the vote** | One session. The house votes on its council head. **Speak for her** (Known). **Speak against her** (Trusted, or ★ Elder): the cook takes the chair. **Speak for the exile** (with `exile.home`; Trusted or ★ Elder): he's let back in (+1 person) | Wait; talk |
| 5 | **The house stands, or pays** | `door.held` or `door.bastion`: put the barricades up (Fortify the Safehouse). `door.paid` or `exile.sold`: carry next month's envelope, which is tins again. `door.debt_yours`: pay the first instalment, in the crew's pit (House Rules) or 20 Red Hand Favor | Fortify the Safehouse; House Rules |

| Ending | You get | The world |
|---|---|---|
| **Stands alone** | Haven Fame +20; Red Hand Infamy +10 | The town counts as Barricaded (until the next restart, as Fortify); the council head is replaced if the vote went that way; the crew boss remembers you crossed him; `door.shut` |
| **Under Bastion** | Bastion Fame +12, Haven Fame +8 | Bastion checkpoints are twice as likely on the roads near that town for the season (within the road-post caps); the house is on Bastion's census (`door.bastion`) |
| **Paid in tins** | Red Hand Fame +10, 30 Red Hand Favor; Haven Fame +5 | The house loses 1 food at every second upkeep for the season and is never raided by that crew. With `exile.sold` and the Long Table active: the crew sells him on (`door.exile_sold`), and *"never arrived"* is in the news |
| **The debt is yours** | Red Hand Fame +15, Haven Fame +15 | Nobody was sold; R1's page has your name on it |

### H3. The Edge of Town

At one house, someone who declares a bite is walked to the edge of town at dusk, and the walker
comes back alone. He has walked eleven people out. A nurse at another house says eight in ten of
them could have walked out of Halcyon's lab instead. The walker says the lab writes down the
other two, and at least at the edge of town somebody holds your hand. Both of them are right.
The houses want someone to settle it.

**Built** ([MISSIONS.md](MISSIONS.md#the-edge-of-town-the-haven)). Stages 1 and 5 ("the walks
stay") are the contract Walk Him Out at the walker's house; the lab road is the contract Cold
Chain. Changes from the design: the nurse's house has a medic when one does (else any other
house); Cold Chain here has no cure threshold; the cut dose's "a session later" is a sixth stage (a
wait); the walker isn't a named person the world keeps (a headline and `walk.walks_stay`). Brokering
the lab road needs Known with Halcyon and Trusted with the Haven: a saved man makes it Known,
`weeks.public` makes it Trusted again.

**Needs:** Known with the Haven. Talking to Halcyon at stage 3 needs Trusted with Halcyon.
★ Nurse, ★ Doctor, ★ Elder, ★ Lab Tech.
**Starts:** the nurse (a named person at a safehouse with a medic), after any player has
finished Walk Him Out.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **One more walk** | A bitten member waits at the edge of town at dusk. **End it** or **let him walk on**, as in the contract (`walk.walked`). **Take him to the lab** for the trial dose before he turns (escort; 8 in 10 he walks out and joins a safehouse, `walk.lab_saved`; else `walk.lab_lost`) | Walk Him Out; escort |
| 2 | **The walker's tally** | The graves at the edge of town (three points) and his notebook. **Talk** (Known): he says every name. **Sneak**: read the notebook while he's on watch. ★ Elder: he asks you to walk the next one with him | Visit points |
| 3 | **The lab's numbers** | What happens to "the other two". **Talk** to a field lead (Trusted with Halcyon, or ★ Lab Tech/Doctor). **Sneak** into the records on the night shift when the lights go dim (a heat limit; seen: +8 Halcyon Infamy). **Bribe** a courier: 20 Halcyon Favor. Sets `walk.knows_other_two` *(you)*: HC2 opens at stage 2 | Silent Courier; talk |
| 4 | **Wait; two offers** | One session. The research director offers, on the secure channel, to take every bitten Haven member and sell doses to Haven clinics at Known prices (Radio Operators hear it; others hear it from the nurse). The Red Hand offers the cut dose, cheap, for every house. **Broker the lab road** (Known with Halcyon). **Refuse both**. **Side with the Red Hand** (Trusted with the Red Hand) | Wait; talk |
| 5 | **Make it real** | Lab road: an antidote batch to a Haven clinic before it spoils. The walks stay: the walker's last walk, with you. The cut dose: a crate from a Red Hand drop to a safehouse, unseen by Halcyon's courier | Cold Chain; Walk Him Out; delivery |

**Branches:** a saved man at stage 1 makes the lab road easier (Known is enough at stage 4). If
HC1 ended with the three weeks public (`weeks.public`), the houses don't trust the lab: brokering
it needs Trusted with the Haven.

| Ending | You get | The world |
|---|---|---|
| **The lab road** | Haven Fame +15, Halcyon Fame +15 | Research +2; Haven morale −4 when the first of the other two are written up (a headline names them, or numbers them if HC2 ended that way); a Haven–Halcyon truce starts if the rules allow; `walk.lab_road` |
| **The walks stay** | Haven Fame +15; the walker and the nurse both remember you helped | The walker becomes a named person at his house; Halcyon morale −3 |
| **The cut dose** | Red Hand Fame +12, 30 Red Hand Favor; a session later, Haven Infamy +10 | One safehouse loses 2 people; R1 counts the Haven's debt. Headline: *"Half of them walked out. The Red Hand calls that a good rate."* |

---

## Task Force Bastion

### B1. No Reply

Every night the operator reads the numbers to Command and writes "No reply". At one outpost the
operator wrote something else, then crossed it out. He swears he heard an answer. His commander
wants it dealt with before the stronghold hears about it. The truth is smaller and worse. The
relay on the hill still rebroadcasts the Day 3 signal on a loop: the one too broken for voice,
that a runner set off with and never delivered. The runner is still out there, and so is the
decoded page in his pocket.

Command never answers. The "reply" is Bastion's own Day 3 signal, coming back.

**Needs:** Known with Bastion; Trusted (or ★ Soldier) to take the page to the commanding officer.
★ Radio Op, ★ Soldier.
**Starts:** an Independent outpost commander, in person, after the nightly call (20:30).

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **The operator's tape** | Take it to the relay on high ground and keep the relay running 3 min while it's compared (a horde after the first minute). ★ Radio Op: hears it's a loop at once (half the hold). **Sneak**: copy the tape rather than take it (`noreply.copy` *(you)*) | Radio Relay |
| 2 | **The runner** | The relay's log gives the hour. Follow his trail across three towns (his pack, his fire, his body), past turned soldiers at the last. His page is water-stained, legible in parts: *"...hold the line... no relief will... authorised to... expended."* | Step chain (Hunt the Deserter's trail); Missing Patrol |
| 3 | **Who reads it** | **To the commanding officer** (Trusted, or ★ Soldier): it goes in the stove (`orders.stove`). **To the commander** who sent you: he reads it to his men at the shift change (`orders.read`). **Side with the Red Hand**: a crew boss reads it on the open channel, for 40 Red Hand Favor (`orders.mocked`). **Side with the Haven**: the council chair keeps it, as proof of what Bastion was told (`orders.haven`) | Delivery |
| 4 | **Wait** | One session. Read: two men walk off the post. Mocked: Bastion's open channel is silent for a night while the Red Hand laughs every hour. Stove: the operator is moved to the stronghold. The Haven: the chair says nothing, yet | Wait |
| 5 | **The reckoning** | *Read:* the commanding officer sends a squad to arrest the commander. **Fight** for him (+20 Bastion Infamy; he goes Rogue and keeps his post, and Rein In follows), **arrest him yourself** (escort to the stronghold), or **talk the CO down** (Champion with Bastion, or Trusted and ★ Soldier: loyalty +25). *Mocked:* find the operator who leaked it, and bring him in or walk him to a safehouse. *Stove:* a copy, if you made one, can still go to anyone in stage 3, once. *The Haven:* Bastion rations to the chair's house, as her price for keeping it | Escort; Hunt the Deserter; Desertion Route; Hearts and Minds |

| Ending | You get | The world |
|---|---|---|
| **Into the stove** | Bastion Fame +20, 30 Favor; the CO remembers you helped | Bastion morale +5; the outpost commander is replaced by a Loyal one; the log still says "No reply" |
| **The men were told** | Bastion Fame +8; Haven Fame +8 if the deserters reached a safehouse | Bastion morale −8; that commander goes Rogue or is replaced; up to two Deserter wanderers; the safehouse +2 people |
| **On the open channel** | Red Hand Fame +15, 40 Red Hand Favor; Bastion Infamy +20 | Bastion morale −10; a Bastion–Red Hand feud starts; `orders.mocked` |
| **The Haven's leverage** | Haven Fame +15, Bastion Fame +5 | Bastion's Requisition never picks the chair's safehouse this season; Haven morale +5 |

### B2. The Census

The nightly call reads "civilians sheltered" to Command, and the numbers have to add up when
relief comes. The commanding officer wants a real count: every safehouse, every name, for ration
cards, a curfew and a "contribution". The Haven won't give names. It remembers what Bastion's
lists were for on Day 2. A civilian who can walk into both is useful to everyone.

**Needs:** Known with Bastion. 🔒 not Elder for stage 4's collection (as Requisition). ★ Police,
★ Radio Op.
**Starts:** the commanding officer's radio contract, while Bastion's need is Supplies or Security.
With `list.to_bastion` it starts at stage 3: the count is done.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Count a safehouse** | **Talk** to its council head (Known with the Haven): an honest count. **Sneak**: count from outside, unnoticed. ★ Police: the count is exact either way. With `door.bastion` (H2), that house counts itself | Quiet Count |
| 2 | **Ration cards** | Rations to two houses, on the books. **Skim** a crate: 15 Bastion Favor for you, 25% the quartermaster notices (+10 Bastion Infamy). **Give extra** from your own pack: Haven Fame | Hearts and Minds |
| 3 | **The curfew** | Patrol a Haven town at night. **Enforce it**: Bastion Fame, +3 Haven Infamy. **Patrol gently** (★ Police, or Known with the Haven): no Infamy, half the Fame. **Betray**: let a Red Hand toll in (Known with the Red Hand): a roadblock at the town for the night, Red Hand Fame | Curfew Patrol |
| 4 | **The contribution** | Collect 4 food from a counted house. **Bribe**: pay it yourself, 15 Bastion Favor. **Give it back** to another house. **Falsify the ledger** (Trusted, or ★ Police): it shows collected; 25% found out at stage 6 | Requisition |
| 5 | **Wait; the Red Hand hears** | One session. A crew hears the cards mean full stores and raids a counted house. **Fight**: defend it. **Side with the Red Hand**: hold its stores 90 s while the crew loads up. **Let it happen** | Trader Caravan's raid; Pharmacy Heist |
| 6 | **The nightly call** | Stand by the radio at the stronghold at 20:30 and give the count: **true**, **forged high** (more rations for the houses) or **forged low** (fewer collections). Forged: 25% found out. Command doesn't answer | Visit point |

| Ending | You get | The world |
|---|---|---|
| **Counted** | Bastion Fame +20, 30 Favor; Haven Infamy +5 | Counted houses get +2 food once from Bastion's next truck; Haven morale −5. Headline: *"Bastion read two hundred and eleven names to Command last night. No reply."* |
| **Forged** | Haven Fame +15, Bastion Fame +10. Found out: +15 Bastion Infamy instead of the Bastion Fame | High: two counted houses +2 food once. Low: Requisition never picks them this season |
| **Refused** | Haven Fame +15; Bastion Infamy +10 | Requisition picks those houses more often (×1.2); `census.refused` |
| **The Red Hand's count** | Red Hand Fame +15, 30 Red Hand Favor; Bastion Infamy +15, Haven Infamy +10 | The raided house −3 food; Red Hand strength +3 |

### B3. The Last Pilot

Of the helicopters Bastion came in with, only one still flies, and only one pilot is left who can
fly it. Everyone wants a flight: Halcyon for a sample run, the Haven for its wounded, the Red Hand
for the helicopter itself. On Day 3 he was wingman to the pilot who flew the wounded for the line
and was shot down short of it. Lately he talks about the line at the shift change, and asks
people what it looks like from the ground.

**Needs:** Known with Bastion; Trusted (or ★ Soldier) to talk him down. ★ Mechanic, ★ Radio Op,
★ Soldier. No storm for the flights.
**Starts:** the pilot, after any player finishes Last Helicopter or Clear the Airfield.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Fuel** | The helicopter is dry. **Bring 2 jerrycans** to the LZ. **Sneak**: take fuel from a Red Hand depot (+8 Red Hand Infamy if seen). **Bribe**: buy it from a crew, 25 Red Hand Favor (the boss hears Bastion is buying) | Delivery; Arson's depot |
| 2 | **Whose flight** | Keep the LZ clear until it lifts off, and choose the load: **Halcyon's samples** (Halcyon Fame +6, research +1), **the Haven's wounded** (Haven Fame +6, +2 people at a safehouse) or **Bastion's ammunition** (Bastion Fame +6, strength +2) (`pilot.flew_for`) | Last Helicopter; choice |
| 3 | **His friend's wreck** | Find the Day 3 wreck near the line, at the edge of the warning zone (never in the kill zone), and bring him the black box. ★ Mechanic: twice as fast. ★ Radio Op: you can play it there, on a radio | Crash Site |
| 4 | **He decides** | He wants to fly for the line. **Talk him down** (Trusted, or ★ Soldier; Known is enough if he's heard the black box: the line shooting his friend) (`pilot.stays`). **Report him** to the CO (`pilot.grounded`). **Help him**: fuel to a far LZ (`pilot.line`). **Side with the Red Hand** (Trusted with them): sell the plan, so a crew can take the helicopter on the ground (`pilot.redhand`) | Talk; delivery |
| 5 | **The last of it** | Stays: one more run, with the LZ kept clear. Grounded: walk him to the stronghold; he goes quietly. The line: hold the far LZ while he lifts off. The Red Hand: hold the LZ for the crew 90 s. They can't fly it, so they burn it | Last Helicopter; escort; hold |

| Ending | You get | The world |
|---|---|---|
| **He stays** | Bastion Fame +20, 30 Favor | Last Helicopter runs twice as often for 2 hours (as Clear the Airfield); Bastion morale +5 |
| **Grounded** | Bastion Fame +10 | No Last Helicopter for 2 real hours, until a new pilot (a named person) is found |
| **He flew for the line** | Nothing | No Last Helicopter for 2 real days, until Bastion's mechanics patch up a grounded airframe; Bastion morale −10; a Crashed Helicopter event at the edge of the warning zone. Headline: *"Bastion's last pilot flew for the line at dawn. The line shot him down short of it, like the one before."* |
| **Burnt for the Red Hand** | Red Hand Fame +15, 40 Red Hand Favor; Bastion Infamy +25 | No Last Helicopter for 2 real days; Bastion morale −10, Red Hand morale +5; a Bastion–Red Hand feud |

---

## The Red Hand

### R1. The Book

Everything owed is in the book, and the book has to balance. Last week somebody tore a page out:
the one nobody is meant to read, with who *outside* the Red Hand owes it and for what. A Bastion
outpost commander for rifles that fell off the inventory, a Halcyon field lead for live ones, a
Haven council head for her quiet arrangement. Whoever holds that page can ruin someone in every
faction. The boss wants it back, and the man who tore it out put in the pit.

**Needs:** Known with the Red Hand; Trusted to return the page (Known with `orders.mocked`: the
boss owes you). ★ Ex-Con, ★ Police.
**Starts:** the boss's radio contract, from the second real day of the season. The page names
the faction's Rogue leaders where there are any, else Independent ones.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Who tore it** | Three crew members, one of them lying. ★ Police or Ex-Con: more to go on. Wrong: the wrong man goes in the pit and you're told so (+5 Red Hand Infamy, `book.wrong_man` *(you)*) | The Rat |
| 2 | **Find him** | His trail through two towns to a barn. **Talk** him out (unarmed; ★ Ex-Con, he knows you're one of his). **Fight** (two friends with him). **Bribe**: 20 Red Hand Favor and he runs (`book.thief_ran`). He tells you why: his sister's house is on the page | Step chain; Hunt the Deserter |
| 3 | **Read the page** | Three names. For each: **collect** the debt (talk 60%; threaten, +3 Infamy with their faction; or pay it, 15 Red Hand Favor), **leave it**, or **betray: turn them in** to their own stronghold (at most one name: that leader is replaced, their faction's morale −3, Fame +10 with that faction and +15 Red Hand Infamy, because nobody rats). With `door.debt_yours` (H2), your name is on it too: a fourth debt, yours | Debt Collection |
| 4 | **The page** | **Return it** to the boss (`book.returned`). **Keep it** (`book.kept` *(you)*). **Burn it** (`book.burnt`). **Side with Bastion**: the whole page to the commanding officer (`book.bastion`) | Delivery |
| 5 | **The thief** | The boss wants him in the pit. **Let it happen**. **Fight**: take his place (4 min of waves). **Bribe**: pay his debt, 30 Red Hand Favor. **Get him out**: walk him to a safehouse (+10 Red Hand Infamy if seen) | Fight Pit; Desertion Route |

| Ending | You get | The world |
|---|---|---|
| **The book balances** | Red Hand Fame +20, 40 Favor; the crew gives you a name, used in headlines and by Red Hand leaders this life | Red Hand morale +5; every leader you collected from remembers you crossed them |
| **Held over them** | 20 Favor, once, from each faction whose leader you squeezed; when the boss hears (a session later), Red Hand Infamy +15 | The leader you squeezed hardest swings toward Rogue |
| **Bastion's file** | Bastion Fame +20; Red Hand Infamy +25 | The Bastion commander on the page is pulled into line (loyalty +25); Red Hand strength −3; Bastion keeps Red Hand prisoners longer (Bust Out a little more often, ×1.2) |
| **Burnt** | Haven Fame +5 if the thief lived; Red Hand Infamy +25 (a debt is a debt) | The debts die with the page; Red Hand morale −5; the three leaders remember you saved them |

### R2. Cargo

Some trucks carry people, not tins, and where to, nobody says. The driver of one of them wants
out. He has carried strays the Haven turned out, debtors from the book, a deserter or two. Most
went to the pit, or to the line, to learn the drones. Lately one crew boss sends a load somewhere
else: a farm past the last village, where a woman pays in jars. The driver has stopped eating meat.

**Needs:** Known with the Red Hand; or Known with the Haven, for the Haven's version (a Haven
wanderer saw the van). ★ Ex-Con, ★ Mechanic, ★ Police.
**Starts:** the driver (a Red Hand named person) at an outpost, at dusk. Without the Long Table
this season, the van goes to the pit, and stage 3 ends there.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Meet the driver** | At a crossroads at dusk. **Talk** (★ Ex-Con: he trusts you sooner). **Threaten** him: he talks, then tells his crew (stage 2's camp is on guard). He shows you the van, padlocked | Talk |
| 2 | **The load** | Three people in the back of the van at a crew camp. **Let them out**: cut the lock and walk them to a safehouse (+3 people; +10 Red Hand Infamy, `cargo.freed`). **Drive it** to where it's going (`cargo.delivered` *(you)*). **Side with Bastion**: drive it to a Bastion seat: three "civilians sheltered", the driver arrested (`cargo.bastion`). ★ Mechanic: the van starts first time | Bust Out; vehicle zone |
| 3 | **Who buys** | **Sneak**: watch the drop at night from 150 m, 2 min unseen: the host comes out with jars. **Talk** to the fence (Trusted): he says "pork" and laughs. **Bribe** the driver, 10 Red Hand Favor: he names the crew boss. Sets `cargo.buyer_known`; C1's proof stage counts as done | Visit point; talk |
| 4 | **Tell someone** | **The boss** (Trusted with the Red Hand): the code has no rule about this, and the boss says it doesn't need one (`cargo.purge`). **Keep quiet** for a cut (`cargo.cut` *(you)*). **Side with Bastion**: the crew's camp to Bastion, and the farm's grid with it (`cargo.bastion_raid`). **Side with the Haven**: the farm's grid to the chair (C1 starts at stage 4) | Talk; Sell Out a Rival Crew |
| 5 | **The driver** | **Get him out**: walk him to a safehouse; he joins it. **Leave him driving**. Purged: the boss wants the driver in the pit too: take his place or let it be. Bastion's raid: assault the crew's camp | Desertion Route; Fight Pit; Raid the Red Hand |

| Ending | You get | The world |
|---|---|---|
| **Purged** | Red Hand Fame +20, 30 Favor | The crew boss is replaced; the Long Table loses its buyer and takes people half as often (`table.supplier_cut`); Red Hand morale −3 |
| **The cut** | 20 Red Hand Favor a session for three sessions; if the Long Table is exposed while you're on it, Haven Infamy +20 | The vans keep running; one more *"never arrived"* in the news |
| **Bastion's raid** | Bastion Fame +20; Red Hand Infamy +25 | The crew's outpost goes to Bastion if Bastion is strong enough by the usual margin, else the Red Hand drops it; Bastion knows the farm (`table.bastion_knows`) |
| **Freed and told** | Haven Fame +15; Red Hand Infamy +10 | The safehouse +3 people, and the driver; C1 starts with the Haven |

### R3. The Well Story

Nobody in the Red Hand says what happens to a place that doesn't pay. They tell the story about
the well, and let you finish it yourself. A crew boss with a village that has stopped paying
wants the story told, and if that doesn't work, he wants it to stop being a story. The boss says
the story works best untold: you don't kill the cow you milk. Both of them think you're on their
side.

**Needs:** Known with the Red Hand; Trusted when the boss sends for you. 🔒 not Elder (as
Protection Racket). ★ Ex-Con, ★ Lab Tech, ★ Nurse.
**Starts:** a crew boss at a Red Hand outpost (Ruthless or Greedy ones more often). The village
is the nearest Haven safehouse town, or a quiet village under 30 infection. With `door.shut`
(H2) or a house counted by Bastion (B2), it's that house: it has someone else now.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Collect** | The keeper won't pay | Protection Racket |
| 2 | **Why they stopped** | **Talk** to the council head (Known with the Haven): they're short, or they pay Bastion now. **Bribe**: pay their dues yourself, 20 Red Hand Favor. The crew boss doesn't care who paid: the story ends at "The story worked" | Talk |
| 3 | **Tell the story** | At the well, at dusk, to whoever's there. **Talk** (★ Ex-Con: they believe you): 60% they pay (`well.paid`). **Warn them instead**: they put two people on the well (`well.warned`, Haven Fame +5) | Talk at a point |
| 4 | **The crew boss's order** | If they didn't pay: put something in the well. **Do it**, at night, unseen (`well.done` *(you)*). **Fake it**: something dead in the bucket; ★ Lab Tech or Nurse knows what's harmless (`well.faked`). **Side with Halcyon**: get the water tested and the well sealed (`well.tested`). **Fight**: the crew comes to do it themselves; hold the well | Visit point; The Poisoned Well; Trader Caravan's raid |
| 5 | **The boss hears** | A session later the boss sends for you. **Own it**. **Blame the crew boss**: he goes in the pit. **Say nothing** | Wait; talk |

| Ending | You get | The world |
|---|---|---|
| **The story worked** | Red Hand Fame +20, 30 Favor; the boss remembers you helped | The village −2 food once; nobody hurt; the well becomes a rumour |
| **The well was done** | Red Hand Fame +5; the boss is angry: Red Hand Infamy +10; if anyone saw, Haven Infamy +30 | Town infection +15 (Fever Town may start there); the safehouse −3 people; the crew boss is replaced for a bad deal. Headline: *"Nobody in [the village] is drinking from the well."* |
| **Faked** | Red Hand Fame +10; Haven Fame +8 if you told the council after; 25% the crew boss finds out: Red Hand Infamy +10 | The village −1 food once; the story spreads anyway |
| **Stood up** | Haven Fame +15; Halcyon Fame +8 if tested; Red Hand Infamy +10 | Infection −10 there if tested; the crew boss's loyalty swings toward Rogue; `well.stood` |

---

## Halcyon

### HC1. The Three Weeks

The first case was three weeks before the gates closed, and the Institute was told. Someone
wrote "notify the Institute" in the margin and underlined it twice. A young field lead has found
the clinic's copy and wants to know what the lab did with those three weeks. The research
director wants the lab to stay the one way out, which means nobody asks. The only person who
knows is the driver who came back alone from the site on Day 3, and he won't say what he saw.

This line is about the past. It never makes the outside act: the Institute's last call on Day 2
is still the last.

**Needs:** Known with Halcyon. ★ Lab Tech, ★ Doctor, ★ Radio Op.
**Starts:** an Independent or Rogue field lead at the field station, after any player has
finished Patient Zero or Patient Zero's Ledger.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **The driver** | A Scavenger wanderer who won't talk to anyone in a lab coat. **Talk** (Known with the Haven, and not Trusted with Halcyon). **Share food** twice. **Threaten** him (+3 Haven Infamy). He names the site, and says *"she stayed because she wanted to."* | Wanderer talk |
| 2 | **The site** | The closed site on the highest ground away from towns. A keypad; the code is on three bodies from the team that went up with her. ★ Lab Tech: the first digit. Three wrong: locked 2 min, loud | Locked Bunker |
| 3 | **The team lead** | Her notebook is inside, and so is she: she turned a long time ago. **Mercy**: end it, take the notebook. **Live capture**: bring her to the lab (`weeks.captured`, research +2). **Sneak**: take the notebook and leave her | Live Capture |
| 4 | **Who reads it** | The notebook doesn't say what the site was for. It says the Institute was told on the first day of the three weeks, and wrote back one word: *"Continue."* **To the director** (Trusted): it goes in the record; the pages about the director go in the stove (`weeks.director`). **To the field lead**: she wants it out (`weeks.public`). **Side with Bastion**: the commanding officer was right about Halcyon (`weeks.bastion`). **Side with the Haven**: the chair reads it at supper (`weeks.public`). **Side with the Red Hand**: 40 Red Hand Favor, and they read it on the open channel (`weeks.public`) | Delivery |
| 5 | **The lab answers** | One session. Public: the Red Hand thinks the lab has more to hide and raids it (hold the lab). In the record: the field lead walks out; walk her to a safehouse, or bring her back to the lab. Bastion: the CO's demand goes to the director (HC3 opens with it) | Wait; Lab Defence; escort |

| Ending | You get | The world |
|---|---|---|
| **In the record** | Halcyon Fame +20, 30 Favor; research +1 | Halcyon morale +3; the field lead is replaced (she leaves: a life event) |
| **Out on the open channel** | Fame +15 with whoever you gave it to; Halcyon Infamy +10 | Halcyon morale −10; *"Halcyon made this"* is a common rumour for the season; H3's lab road gets harder |
| **Bastion's file** | Bastion Fame +20; Halcyon Infamy +15 | Bastion strength +3; HC3 starts with Bastion's terms already on the table |
| *(with any ending)* **She's at the lab** | Research +2 at stage 3 | The director speaks for two minutes on the open channel, about her |

### HC2. The Other Two

Eight in ten walk out of the trial dose, and the lab writes down what happens to the other two.
A Haven woman's brother volunteered and didn't come back, and the lab won't tell her more than a
number. The answer is at the field station. The other two leave with half the time they came in
with, and some of them are kept, in a back room, so that someone with a clipboard can be there at
the moment of turning.

**Needs:** Known with the Haven or with Halcyon. ★ Doctor, ★ Nurse, ★ Lab Tech.
**Starts:** the sister (a Haven named person) at a safehouse. With `walk.knows_other_two` (H3),
at stage 2.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **His number** | **Talk** at the lab (Known with Halcyon): a number and a date. **Sneak**: the volunteers' ledger on the night shift (a heat limit; seen: +8 Halcyon Infamy). **Bribe** a courier: 20 Halcyon Favor. ★ Lab Tech reads the ledger's codes | Silent Courier; talk |
| 2 | **The back room** | **Get in as a courier**: walk a cold case there with Halcyon's courier. **Talk**: the field lead's leave (Trusted with Halcyon). **Sneak**: at night, unseen. Inside: three people in the Final stage, and a chair with a clipboard by each | Escort; sneak |
| 3 | **Her brother** | He has a few minutes left. **Sit with him** (`brother.sat`). **Take him to his sister** before he turns (`brother.home`). **Take the sample** at the moment of turning (research +1, `brother.sample` *(you)*). **End it** | Patients Not Samples; escort |
| 4 | **The field lead** | **Talk** (Trusted with Halcyon, or ★ Doctor): she closes the room (`ward.closed`). **Report her** to the director: she's replaced and the room moves to the lab (`ward.lab`). **Keep quiet**: 25 Halcyon Favor (`ward.kept`). **Side with the Red Hand** (Trusted with them): one of the other two goes to a crew as a "live one", for 40 Red Hand Favor (`ward.sold` *(you)*) | Talk |
| 5 | **Names** | The paper lists the volunteers by number. **Bring the names** to the director (Trusted, or Known with the sister beside you after `brother.home`). **Side with the Haven**: the chair reads them at supper instead. **Leave the numbers** | Delivery; talk |

| Ending | You get | The world |
|---|---|---|
| **Named** | Haven Fame +15, Halcyon Fame +15 | A Haven–Halcyon truce if the rules allow; Haven morale +5. Headline: *"The lab put fourteen names on its paper last night. The Haven read them at supper."* |
| **Brought home** | Haven Fame +20 | Halcyon morale −3; the room closes or stays, by stage 4 |
| **The work goes on** | Halcyon Fame +15, 25 Favor; research +2 | `other_two.numbers` (H3's headline uses numbers); Haven morale −3 |
| **Sold** | Red Hand Fame +15, 40 Red Hand Favor; 35% found out: Haven Infamy +25 | A headline about something new in the Red Hand's pit |

### HC3. Authors

At half research, the cure stops being a hope and becomes something a person can own. The
research director would trade what the lab knows for Bastion's guns at the final stand. A field
lead would rather give the method to every Haven clinic for nothing. The Red Hand would buy the
batch outright. Whose names go on the paper decides who's remembered for the cure. Where the cure
goes is still the season's Fame; this line is about who is standing at the lab when it's made.

**Needs:** Trusted with Halcyon; cure 50% or more; the final stand not yet won. ★ Lab Tech,
★ Doctor, ★ Soldier.
**Starts:** the research director's radio contract when research reaches 50%. With
`weeks.bastion` (HC1), stage 1 is done: Bastion already has terms.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **The director's letter** | Carry her terms to the commanding officer. **Read it first** (`authors.read` *(you)*): the method for Bastion alone, in return for guards. **Sneak** a copy to the field lead | Delivery |
| 2 | **The field lead's plan** | She wants the method at a Haven clinic first. **Carry it**, cold, before it's missed (`authors.haven_copy`). **Report her** to the director (`authors.reported`). **Side with the Red Hand**: sell it (Sell the Formula) | Cold Chain; Sell the Formula |
| 3 | **The CO's answer** | One session. He wants a locked crate, a key and a guard on the clean room. **Agree** for the director. **Bargain** (Trusted with both): two guards at the gate, no key. **Refuse** for the lab (+5 Bastion Infamy) | Wait; talk |
| 4 | **The Red Hand's offer** | The boss offers to buy the batch, and to leave the lab alone during the stand. **Take it to the director**: she refuses. **Betray**: take the money (60 Red Hand Favor); Take the Lab is offered during the stand. **Side with Bastion**: tell the CO, and a Bastion checkpoint goes up on the lab's road | Talk |
| 5 | **Ready the lab** | One of Power the Lab, Decontamination or Lab Defence, picked by the director from your flags | Power the Lab; Decontamination; Lab Defence |
| 6 | **The final stand** | The stand itself. With `pilot.flew_for` samples (B3), Halcyon's people remember who flew for them, and one Halcyon guard more stands at the gate | Final stand |

| Ending | You get | The world |
|---|---|---|
| **Under Bastion's guard** | Bastion Fame +15, Halcyon Fame +10 | One Bastion guard group at the lab gate through the final stand (it fights the waves; the stand's rules don't change); Bastion strength +3; Haven morale −3 |
| **Open doors** | Halcyon Fame +15, Haven Fame +10 | Infection −3 in every safehouse town (as Leak the Data); the field lead's name goes on the paper first |
| **The Red Hand's batch** | Red Hand Fame +25, 60 Red Hand Favor; Halcyon Infamy +30 | Take the Lab runs during the stand, with its usual result; Halcyon morale −10 |
| **The lab alone** | Halcyon Fame +20 | Nothing more: the lab holds or doesn't, on its own |

---

## The Long Table

Who they are is in [LORE.md](LORE.md#the-long-table); how they work and how players deal with them
is in [DESIGN.md](DESIGN.md#the-long-table) and [REPUTATION.md](REPUTATION.md#the-long-table).
Both lines exist only in a season that has a Long Table, and never run together (one slot).
Both are built; their stages, numbers and what each approach does are in
[MISSIONS.md](MISSIONS.md#questlines).

### C1. The House That Went Quiet

Past the last village there's a farmhouse with smoke in the chimney, in a valley where nothing
else burns. Wanderers who take the road past it don't always arrive. A Haven courier's satchel
turned up at a Red Hand fence. The man you meet on that road is the friendliest person in the
Zone. Nobody gives this job. Somebody just has to notice.

**Needs:** Any. ★ Hunter, ★ Police, ★ Nurse, ★ Elder.
**Starts:** when the house's suspicion reaches 30 and a player hears a *"never arrived"*
rumour; or from H1 (at stage 2), R2 (at stage 4) or C2 (at stage 4).

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Never arrived** | Follow a missing wanderer's last road: a boot on the verge, a satchel at a fence, the gate of the farm. ★ Hunter or Police: grid references | Trail of Clues |
| 2 | **Guest** | The host meets you on the road. **Talk: accept** and eat: in daylight, in company or bitten, it's only a meal (`table.guest` *(you)*). **Sneak: decline** and watch from outside, unseen. **Declare your bite** if you have one: they bless you and send you on | Talk; visit points |
| 3 | **Proof** | Three of: the guestbook and its small crosses, the boots by the door, a dead courier's letters in a drawer, the cellar (locked: the mother's key, or a crowbar, which is loud). **Sneak** in during supper (18:30). **Talk** to the boy, away from the house, unarmed (★ Nurse or Elder): he says one thing. **Fight** your way in (before the cellar is open, shooting them costs Haven Infamy, [REPUTATION.md](REPUTATION.md#the-long-table)). With `cargo.buyer_known` (R2) or `place.undercover` (C2): done | Searching; door locks; clues |
| 4 | **Who to tell** | **The Haven**: horror and shame; the chair wants them brought home and walked out (`table.haven`). **Bastion**: containment (`table.bastion`). **Halcyon**: the director wants one of them alive, and the mother's kitchen notebook: why they never eat the bitten (`table.halcyon`). **The Red Hand**: the boss pays the house to go away (`table.redhand`). **Nobody**: deal with it yourself | Talk; delivery |
| 5 | **The end of the house** | *Haven:* walk the family to the edge of town at dusk, unarmed. They go quietly, except the quiet son. *Bastion:* the guns, after five minutes' warning; get the boy out first (talk him down, nothing within 25 m), or don't. *Halcyon:* walk the mother to the lab as a "volunteer". *Red Hand:* hold the farm 90 s while a crew moves them on. *Yourself:* **fight** (three or four of them), or **talk** the last ones out once the mother is gone | Walk Him Out; Bastion Shelling; escort; hold |

| Ending | You get | The world |
|---|---|---|
| **Walked out** | Haven Fame +20, 30 Favor | Haven morale −8, then +5 when it's done; the boy joins a safehouse (+1) or becomes the Kid wanderer. Headline: *"Every Haven house lit a candle last night. Nobody said who for."* |
| **Burnt** | Bastion Fame +20, 30 Favor | The farm's town is Burned (town memory); Bastion morale +3. Headline: *"Bastion reports an infected cluster cleared past the last village."* If the boy wasn't got out, nobody reads his name |
| **Studied** | Halcyon Fame +20; research +2 | The mother is at the lab; the rest scatter. The host becomes a Scavenger wanderer who never asks anyone to dinner again |
| **Paid to go** | Red Hand Fame +15, 40 Red Hand Favor | The house moves 2–5 km and is quiet for a real day, then takes people half as often; C1 can start again, once |
**By your own hand** is no ending of its own: the house is gone for the season, the boy lives if you
got him out, and the proof, taken to a faction after, pays half that faction's Fame.

If nobody ends it and it's never found out, the house goes on until the season ends, and next
season there may be a long table somewhere else. Found out and not ended within a real day,
Bastion burns it ([DESIGN.md](DESIGN.md#the-long-table)).

**Built** ✅ (`Missions/Questlines/LongTable/RZ_Quest_HouseWentQuiet.c`). Changes from the design:
- A first stage routes it: `list.found_table` to stage 2; `cargo.buyer_known`, `place.undercover`
  or `place.proof`, or the house already found out, to stage 4.
- *Guest:* accepting walks you up to the farm, and the meal is at the table. Declining points you
  to a rise behind the farm: a minute there unseen is the watch; seen, they're wary at stage 3.
- *Proof:* the boy's one thing is also where the key hangs (without it, the cellar needs a
  crowbar). Outside supper, any of them within 20 m sees you (30 m once they're wary). Caught, they
  set on you (a fight) or, without the fight prefab, shut the house for the day. A fight that
  finishes the house ends the line by your own hand.
- *Who to tell* is any leader of that faction, talked to by someone who took part (by anyone, when
  the proof came from another line); Halcyon needs the lab, Bastion its stronghold. The house is
  found out then, and the teller gets the faction's Fame then (+10, the Red Hand +4).
- *The end of the house* the factions' way is that faction's contract on the radio, so an ending's
  Fame is the telling's plus the contract's (10 Fame and 30 Favor; Halcyon 10 and 20; the Red Hand
  11 and 40). The Haven's walk is between 16:00 and 21:30, and the family waits for you at the edge
  of town rather than walking with you; the quiet son is talked round (persuade) or let go.
  Halcyon's volunteer is the mother, or one of them alive without her. By your own hand: fight them
  (the fight prefab), or tell the mother you know (persuade with the Haven, difficulty 2; ★ Elder
  or Nurse) and she walks out, then tell the rest to go; a failed word and they set on you. "Taken
  to a faction after" is a last wait of two hours of play or 12 real hours.
- Its endings also publish what happened (`table.walked_out` and the rest, in [Crossings](#crossings)).

### C2. A Place at the Table

The host doesn't only invite people to dinner. To someone who has eaten with him twice and come
back, he makes a small offer. The dead don't need what they have. Tell the family where the dead
are, before anyone else finds them, and the family will feed you and tell you what the roads are
doing. It starts with the dead. Their story started that way too.

This is the darkest line in the mod. Most players take it to get proof for C1. A player is never
the one taken, and nothing here is described: it's all said sideways ([LORE.md](LORE.md#the-long-table)).

**Needs:** ate with the Long Table twice this life; not Trusted with the Haven (the host doesn't
ask the Haven's friends). ★ Police: anything you learn counts as proof.
**Starts:** the host, on your third visit.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **The dead** | Stand where a wanderer or a truck crew died to the infected in the last hour, then tell the host. A bowl and true road news (where the next truck is going) (`place.dead` *(you)*) | Visit point; talk |
| 2 | **The dying** | A bitten wanderer *"who won't make it."* **Tell him where** (the wanderer never arrives, `place.dying` *(you)*). **Lie**: send him the wrong way, and he's cooler with you. **Save the wanderer**: give them your antidote | Talk; wanderer |
| 3 | **A stray** | He wants a living one: an exile the Red Hand keeps track of. **Betray: bring him** (escort to the farm, `place.stray` *(you)*). **Pretend**: walk him to a safehouse or a Bastion seat instead (`place.undercover`). **Refuse**: you're not a guest any more, and the guest rule holds for you in daylight too | Escort |
| 4 | **Grace** | The mother asks you to sit at the long table and say a name. **Sit and say it** (`place.family` *(you)*). **Get up and go**, with the proof (C1, stage 4). **Fight**, at the table (C1, stage 5, by your own hand) | Talk |

| Ending | You get | The world |
|---|---|---|
| **Family** | A bowl when you visit (one food item a real day) and true road news; no Fame anywhere | The guest rule never applies to you. When the house is exposed while you're family, the Haven and Bastion hold it against you ([REPUTATION.md](REPUTATION.md#the-long-table)), and your name is in the headline |
| **Undercover** | C1's proof; +5 Fame with whoever you take it to, on top of C1's | The exile lives, with the Haven or Bastion |
| **Walked away** | Nothing | The host stops meeting you on the road |

**Built** ✅ (`Missions/Questlines/LongTable/RZ_Quest_PlaceAtTable.c`). Changes from the design:
- It's personal: only the character the offer was made to does its stages, and only in that life.
  The host makes the offer on the third meal of a life.
- Stages 2 to 4 wait 20 minutes of play or 6 real hours first; Grace is in the evening (17:00 to
  22:00).
- *The dead:* a wanderer or a truck crew who died to the infected in the last hour, within 4 km;
  if there isn't one, a traveller in a ditch, laid out by the stage, with two infected about.
  Telling the host also feeds the house a little (fewer taken).
- *The dying* is a wanderer from the roster, held by the stage (a stranger if nobody is free). Sent
  the wrong way, he's the usual bitten wanderer; your antidote saves him (+8 Haven Fame).
- *A stray:* refusing is said to the host at the farm gate. A safehouse takes the exile in (+1
  person); Bastion logs him as sheltered.
- Walking away from the host at stage 1, and Grace's "get up and go", end it Walked away; Grace's,
  or a ★ Police guest's walk, carries the proof (`place.proof`): C1 starts at stage 4.
- The fight at the table finishes the house there: the line ends By your own hand (C1's outcome,
  reached here).

---

## Crossings

The flags other lines read.

| Flag | Set in | Read in | What it does there |
|---|---|---|---|
| `list.to_bastion` | H1 | B2 | B2 starts at stage 3: the count is done |
| `list.to_redhand` | H1 | R2 | The driver's strays came off the List; exiles are sold more often |
| `list.found_table` | H1 | C1 | The Haven found the farm lived in: C1 starts at stage 2 |
| `door.debt_yours` | H2 | R1 | Your name is on the page |
| `door.shut`, `door.bastion` | H2 | R3, B2 | That house is the one that won't pay; it's on Bastion's census |
| `door.exile_sold` | H2 | R2, the Long Table | The exile was walked to the crew: he is one of the people who never arrived |
| `walk.knows_other_two` | H3 | HC2 | HC2 starts at stage 2 |
| `walk.lab_road` | H3 | HC2 | The back room holds Haven people: stage 1's talk needs only Known with the Haven |
| `walk.cut_dose` | H3 | R1 | The Haven owes the Red Hand for the cut dose |
| `list.contract`, `walk.contract` | The contracts The List and Walk Him Out, when done | H1, H3 | They can start (`list.contract` holds the safehouse, + 1) |
| `list.read` | H1 | the Long Table | The List was read at every house: suspicion +30 |
| `list.buried`, `list.sold_known`, `list.husband_home`, `list.head_replaced` | H1 | (none yet) | What happened, for contracts, events and rumours: the List was put back in the drawer; who sold it is known; the husband came home; a council head stepped down |
| `door.paid`, `door.knows`, `door.tins`, `door.exile_home`, `door.exile_warned`, `door.head_replaced` | H2 | (none yet) | The envelope was carried, or opened; the house pays in tins; the exile came home, or took to the roads; the house voted its council head out |
| `walk.lab_saved`, `walk.walks_stay`, `walk.truce` | H3 | (none yet) | A bitten man walked out of the lab; the walks stay; the lab road began with a truce |
| `other_two.numbers` | HC2 | H3 | The lab road's headline gives numbers, not names |
| `orders.mocked` | B1 | R1 | The boss owes you: returning the page needs only Known |
| `orders.haven` | B1 | B2 | The chair's house is never asked for a contribution |
| `census.refused` | B2 | H1 | The chair trusts you: the last stage's talk needs only Known |
| `pilot.flew_for` | B3 | HC3 | Samples: one more Halcyon guard at the final stand |
| `book.bastion` | R1 | B1, B2 | Bastion knows which of its commanders sold rifles: B1's outpost commander is that one, if he's still in the chair |
| `cargo.buyer_known`, `table.supplier_cut` | R2 | C1 | C1's proof is done; the house takes people half as often |
| `well.stood` | R3 | H2 | The house stands: H2's "Stands alone" needs no barricades |
| `weeks.public` | HC1 | H3 | Brokering the lab road needs Trusted with the Haven |
| `weeks.bastion` | HC1 | HC3 | HC3 starts at stage 2 |
| `place.undercover` | C2 | C1 | C1's proof is done: it starts at stage 4; the undercover guest gets +5 Fame on top when they tell |
| `place.proof` | C2 | C1 | Got up from grace (or a ★ Police guest walked away) with the proof: C1 starts at stage 4 |
| `place.family`, `place.by_hand` | C2 | any | Someone sat at the long table and said a name; the house was finished at its own table |
| `table.exists`, `table.town` | the Long Table | R2, any | There's a house this season, past this settlement (its index; -1 none). Without one, R2's van goes to the pit |
| `table.exposed`, `table.exposed_to` | the Long Table | R2, any | The house was found out (by the faction told; -1 everyone). R2's "The cut" holds it against its player |
| `table.broken`, `table.burnt`, `table.moved`, `table.suspicion` | the Long Table | any | Gone for the season; burnt by Bastion; moved once; how suspected (0–100) |
| `table.suspicion_add` | any line | the Long Table | Added to its suspicion, then cleared |
| `table.bastion_knows` | R2 | the Long Table | Bastion knows the farm: the house is found out, to Bastion |
| `table.haven`, `table.bastion`, `table.halcyon`, `table.redhand` | C1 | any | Who was told |
| `table.walked_out`, `table.studied`, `table.host_wanders`, `table.paid`, `table.by_hand`, `table.boy_lost` | C1 (`table.by_hand` also C2) | any | How the house ended: walked out; the mother at the lab; the host on the roads as a Scavenger; paid to move; by your own hand; the boy not got out |

The Long Table also accepts the setter's own spelling for flags the table names under `table.`
(`cargo.supplier_cut`, `cargo.bastion_knows`).
