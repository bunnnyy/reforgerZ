# Writing

How every line of player text in ReforgerZ is written: the tone, the voices, the rules each line
follows, and the lint that checks them. Who the factions are as people, and the setting they speak
from, is in [LORE.md](LORE.md). The epilogue week's lines (route and terms, the lab, the queue,
the leaders' verdicts, residents, the road, the open channels) are in
`Director/Epilogue/RZ_EpilogueText.c` ([EPILOGUE.md](EPILOGUE.md)). The running jokes on the channels,
the race lines and pitches on air, the radio's pools and joke memory, grief's other sources and the rest
of the epilogue's variants are designed in
[designs/TONE.md](../designs/TONE.md) and not built yet.

## Contents

- [Tone](#tone)
- [Everyone is right about something](#everyone-is-right-about-something)
- [The voices](#the-voices)
- [By channel](#by-channel)
- [The government broadcast](#the-government-broadcast)
- [Information comes first](#information-comes-first)
- [The season mood](#the-season-mood)
- [The voice bank](#the-voice-bank)
- [Conversations](#conversations)
- [The race and the lock](#the-race-and-the-lock)
- [Faction kit and disguises](#faction-kit-and-disguises)
- [Grey choices](#grey-choices)
- [Phrasing choices and outcomes](#phrasing-choices-and-outcomes)
- [Dark humour, not edginess](#dark-humour-not-edginess)
- [Template rules](#template-rules)
- [Names in shared text](#names-in-shared-text)
- [Writing the infected](#writing-the-infected)
- [Checking text](#checking-text)

## Tone

- **Bleak but human.** The world is cruel. People are what make it worth surviving.
- **No heroes from outside.** No one is coming. The broadcasts are lies or useless.
- **Ordinary people.** You're a nurse, a mechanic, a kid out of work. Not a commando.
- **Small stories.** A note on a fridge, a radio message cut off, a body near the line.
- **Nobody is a villain in their own story.** Every faction does something ugly for a reason you can understand.

## Everyone is right about something

A grey choice puts one side's truth against another's. If one side of a choice has nothing it's
right about, the choice is a virtue test, not a grey choice. Fix it, or make it a plain job. The
hard edges are each faction's in [LORE.md](LORE.md#the-factions-as-people).

| Faction | Right about | Wrong about (its hard edges) | What it jokes about instead |
|---|---|---|---|
| The Haven | Nobody is useless. People are the point | The List, the mouths, walking people out, quiet arrangements, "borrowing" | Food, chores, the house rules, itself |
| Bastion | They're all they have: feed your own, look for your own, bring your own home. And someone has to count, queue and hold, or people get trampled | Containment, "contribution", "signed correct", the stove | Forms, Command, the roster, rank |
| The Red Hand | Nobody was coming, and nothing taken ever has to be given back. Everyone would sell you, so keep something on everyone | Rudi, Cargo, the pit, the cut dose, the well, learning the drones | Everyone, Bastion most; prices, the book, the toll |
| Halcyon | The cure is the only way out, and the numbers matter | The three weeks, the other two, samples first | Rarely on purpose: exact words that land wrong |
| The Long Table | Nothing that can be said aloud | Everything | Nothing, ever |

## The voices

- **Broadcasts** are short, official and slightly wrong:
  *"Day 14. Residents are reminded that the perimeter is for your protection."*
- **Notes** are personal and specific: names, places in the town they're found in, and mistakes.
- **Faction voices:**
  - **Bastion:** clipped, procedural, tired; close with its own.
  - **Haven:** warm, worried, practical, sometimes darkly funny.
  - **Red Hand:** mocking, casual, always counting.
  - **Halcyon:** precise, a bit cold, sometimes guilty.
  - **The Long Table:** homely, attentive, a little too interested in you.
- **Open channels** on the radio are people, not institutions. The Haven's is a tired woman:
  *"If you can hear this, Morton is open. Bring water if you can. Bring yourself either way."*
  Bastion's net still calls Command, which never answers. The Red Hand uses the open channel
  to laugh at Bastion: *"Still waiting for relief? We'll keep the kettle on."*
- **Faction chatter** on the open channels is shift talk with something real in it: a truck
  leaving in twenty minutes, a toll until dusk, the night watch at 20:30. People say more on the
  radio than they should.
- **Rumours** are gossip, often wrong and worst about rivals: *"I heard Bastion got thrown out
  of Levie. Good riddance."* Never mark which ones are true.
- **Things found in drawers and on bodies** (ID cards, letters, receipts, photos) are one person's
  life in a sentence or two: a name, the town they're found in, what they bought or meant to say.
  *"A till receipt from Morton, Day 2: tinned peaches, batteries (all of them), bleach. Underneath,
  in pencil: 'enough for a week?'"* Some point somewhere: a key with a tag, a grid on the back.
- **Secure channels** are what a faction says when it thinks nobody's listening: blunter than its
  open channel, with numbers in it. *"All stations: Morton rotates the watch at 20:30. Two on the
  gate. Ten minutes where nobody's looking."*
- **Headlines** about the factions are one plain sentence with one small detail: *"Bastion buried
  Corporal Weber outside Morton. No salute: they're saving ammunition."*
- **Haven gossip** is about food, never blame: *"The safehouse in Levie has gone to half portions.
  The children eat first."* When a hungry house takes from a fed one, it's *"borrowed"*, in quotes.
  The Haven's dark side is always said sideways, never announced.
- **Players on the radio station** pick a preset, and it reads like a person: their name, one real
  place, one request. *"This is Mara. There's a horde moving around Morton. Stay clear of it."*
- **Leaders** speak in their faction's voice, coloured by their traits. One short line before a
  brief and one after, never a speech:

| Trait | Humour | Example of the register |
|---|---|---|
| Greedy | About money and the price of the job; talks about pay first | *"Pay first, then the job."* |
| Paranoid | None. Asks who saw you | *"Close the door. Who saw you come in?"* |
| Merciful | Gentle, about food and rest; tells you to sit down and eat | *"The soup doesn't mind either way."* |
| Ruthless | Dry and short, aimed at the job or the institution, never the victim | *"The form was short."* |
| Idealist | None, earnest. May laugh at themselves once | |
| Pragmatist | Wry about the deal | *"I like starving less."* |
| Veteran | Gallows humour about their own dead, fondly | *"They itch."* |
| Talker | The most jokes, and they leak | |

**Stronghold leaders:** Bastion's commanding officer almost never jokes. The chair jokes once in a
while, about food. The boss always jokes. The director never jokes on purpose.

## By channel

| Channel | Who speaks | Register | Jokes | Never |
|---|---|---|---|---|
| Government broadcast | The outside, through a machine | Official and deadpan, with one wrong word | None on purpose | Names a cause, apologises, acts, answers anyone |
| Open channels | People on shift | The faction's voice | About 1 line in 4 | A joke instead of the information ([below](#information-comes-first)). A joke about the bitten on air |
| Secure channels | The faction when nobody's listening | Blunt, with numbers | About 1 in 10, bleak only | A joke at its own people |
| Rumours, road news | The teller | Gossip, biased, worst about rivals | About 1 in 3, aimed at rivals | Saying which are true |
| Headlines | The roads | One plain sentence and one detail | The detail may be dry | A verdict. A player's name ([exceptions](#names-in-shared-text)) |
| Notices and boards | The printed text, then whoever wrote on it | The printed line, then handwriting | The handwriting may answer back | Reusing a questline clue's text |
| Traces: door marks, graves, the season marks ([TRACES.md](TRACES.md)) | What anyone sees, then the writer's meaning, then one true detail ([levels](TRACES.md#reading-a-door)) | One plain sentence and one detail; the writer's voice briefly at Read and Know | None. The mark's own words may be dry | A gore detail, a child hurt, a pronoun for a seeded person, a hint at the Long Table, a joke at a victim. The Haven never writes "taken" |
| Leader openers and closers | A named person | Faction voice and traits ([above](#the-voices)) | Set by trait | A speech |
| Mission briefs | The giver | The giver's voice | Haven and Red Hand givers may open with one. Bastion is procedural, Halcyon clinical. **Mercy jobs never** | Telling the player what's right |
| Choice labels | The player | Verb and object, then the cost or check in brackets | None | Judgement words. Rewards or consequences |
| Outcomes | What happened, then one reaction | Three beats ([below](#phrasing-choices-and-outcomes)) | Only in the reaction | A verdict. Numbers inside the voiced beats |
| Guidance | A voiced speaker, or nobody | Voiced tips: the speaker's register. Plain hints: plain | Voiced tips: the speaker's. Plain hints, veterans' short lines, standing and pay notices: none | Burying a critical instruction: it comes first and can't be misread |
| The Long Table | The family | Homely | None, ever | Describing anything |

**Shared text is the same for everyone who hears it.** The broadcast, the open and secure channels,
headlines, rumours on the roads, notices and boards never depend on one listener: not their
standing, their disguise or what they did.

**A voiced tip may be callous in the speaker's way.** The Red Hand's tips stay as they are:
*"Pour the vodka on it, not in you. Quick, before it takes."* and *"Bitten? Shame. Our cut dose
works half the time..."* The target is the price, the dose and the drinking, not the bitten
person's fear or pain.

**Swearing:** mild and rare, only in Red Hand and Bastion voices, and never the punchline.

## The government broadcast

Until the cure the broadcast never changes its message, but its tone follows the season mood
([below](#the-season-mood); why it's there: [LORE.md](LORE.md#the-outside-world-silent-watching);
how it runs: [DESIGN.md](DESIGN.md#the-government-broadcast)). The first match wins:

| State | Broadcast tone |
|---|---|
| The cure is out | The loop stops. Anyone who tunes in hears the new order: *"The quarantine order for this area has been lifted. Remain where you are. Processing will begin at the road checkpoints."* |
| Hope (the breakthrough window) | *"Reports of research inside the zone are being evaluated."* |
| At the finale gate (research stops by design) | *"The status of this area is under review. Remain where you are."* |
| Bleak: the long stall | *"The perimeter will be maintained indefinitely."* |
| Bleak: Aftermath, cure under 50% | *"Residents are thanked for remaining indoors. Residents are thanked for remaining."* |
| Bleak: Overrun, cure under 50% | *"The perimeter will be maintained."* |
| Grim | *"Residents are reminded that the perimeter is for your protection."* |
| Early | *"Remain indoors. Aid is on its way."* |

With **Voice > Season mood** off, the broadcast keeps its built table: the same lines, the gate's
included, without the Aftermath line and without the mood's holding rules.

## The season mood

✅ Code: `Director/RZ_SeasonMood.c` (`RZ_WorldFacts.SeasonMood()`, `MoodFor(faction)`). One
server-wide value every voice and the broadcast read. It runs on stages and cure percent, not days,
so it works at any season speed, and adds no simulation. The ladder, first match wins:

| Mood | When | Joke share face to face |
|---|---|---|
| **Epilogue** | after the cure | 1 in 3 |
| **Hope** | the broadcast's breakthrough window: 90 online minutes from when players heard the news | 1 in 3 |
| **Bleak** | the long stall (3 h with no research), or Overrun or Aftermath with the cure under 50% | 1 in 10 |
| **Grim** | Collapse or later, average infection 45+, or the 45-minute stall | 1 in 5 |
| **Early** | anything else | 1 in 3 |

- **At the gate** research stops by design: the stall doesn't count, and the mood reads one step
  lighter (Grim as Early, Bleak as Grim), because everyone can see the end.
- **Holding.** Hope and the Epilogue start at once. The others change only after two checks in a row
  (a check every 5 online minutes) and never before the mood has held 60 online minutes. Runtime
  only: a restart starts from the ladder.
- **Temper.** Each faction's morale moves its own lines one step: bleaker below 30 (back above 38),
  lighter above 75 (back below 67). Steps run Hope/Early, Grim, Bleak; the Epilogue isn't shifted.
- **Grief** (`RZ_ToneGuard`): after a leader's or wanderer's death, no wry or joke line is said face
  to face within 1.5 km for 10 minutes; after a death in a safehouse, 30 minutes. The faction's
  radio quiet is kept too, for when the radio's pools move into the bank.
- Debug: **Voice > Force mood**; the log line `Voice: season mood <mood> (was <mood>)`.

## The voice bank

✅ Code: `Factions/RZ_VoiceBank.c`, its defaults in `Factions/RZ_VoiceDefaults.c` (one `Line()` a
line). The single-line pools people say face to face, as data: leaders' greetings (`greeting`),
their openings by manner (`opener`, said with Dialogue > Trees on), named people at holdings
(`person`) and their routine asides at the meal, the night watch and shift change (`aside_meal`,
`aside_night`, `aside_shift`), and each faction's lines about the packs (`infected`, by manner: `warn`,
`roads`, `furious` and `moved` for the Haven, `contact` and `cull` for Bastion, `crew` for the Red Hand,
`index`, `sorry` and `lost` for Halcyon; `{place}` is the speaker's name for the pack with its place,
[INFECTED.md](INFECTED.md#voices)). Each line has a stable id, a faction, a kind, a manner, a register
(PLAIN, WRY, JOKE, TAUNT, BLEAK, SILENCE), the moods it's said in, the leader traits it suits, an
optional condition in the conversation syntax (`cure>=50`), a weight and the text (with `{place}`).

- **A pick** reads the speaker faction's mood, grief near the speaker, and the listener's **joke
  memory**: the last 24 jokes (WRY, JOKE or TAUNT) that player heard, with the real time, kept on
  the profile across lives. A joke heard in the last 24 real hours isn't said again; a joke is said
  only at the mood's share; when there's no joke to say, a plain line; when there's no line at all,
  the built one.
- **Voice > Lines** adds a line or replaces a default with the same id; **Voice > Disabled** switches
  ids off. **Voice > Log** writes every pick: `voice: face <faction> <register> <id> mood=<mood>
  listener=<identity>`.
- `python3 tools/voice_sim.py` replays the picker offline: pool sizes and joke shares per mood, and
  which pools are too small for a veteran's day (a pool's jokes should number at least 1.5 × visits
  × share; under that, the pool falls back to plain lines rather than repeat a joke).
- **Running jokes on the majors' flags** (`RunningJokes` in the defaults): people's lines that run
  while a state stands and stop when it changes, never on a day. The Red Hand's kettle for Bastion
  while `orders.stance` is unset or 1, and "expended" at 4; Bastion's call at 1, "civilians first" at
  2, Command answering at 3; the Haven's rule (`haven.rule` 1–4, the arrangement said sideways); the
  Red Hand's book jokes until `book.debts_dead`, then a plain line; Halcyon noticing once a page is
  in the open (`majors.pages_open`). They're ordinary `person` lines with a condition, so the joke
  memory and the mood's share apply.
- **More kinds:** `route` (face to face after the 90% lock, [below](#the-race-and-the-lock)),
  `look_pass` and `look_caught` ([below](#faction-kit-and-disguises)), `change` (the change notice in
  the faction's voice, by manner), and the openers' `owed` manner and set-word lines.
- **Feud or truce** (`truce:<f>`, `feud:<f>` on people's lines): a feud sharpens the Red Hand's taunts
  and Bastion's lists; a truce allows one joke across the line: *"We shared a fire with Bastion last
  night. Both sides counted the matches."* An opener is filled for the listener too: `{word}`, `{best}`, `{worst}`,
  `{crew}` (reputation's tokens).
- **Nobody jokes with a player their leader refuses:** at a seat whose leader holds you at −2 or
  below, the bank picks only plain lines.
- *Not built yet:* the radio's pools, rumours and chatter in the bank, the shared channels' joke
  memory, and the running jokes on the open channels (the nightly kettle line on air)
  ([designs/TONE.md](../designs/TONE.md)).

## Conversations

Conversation lines ([DIALOGUE.md](DIALOGUE.md)) follow everything on this page, and these:

- **Lines.** One to three short lines a node, in the faction's voice, coloured by the speaker's
  traits. The speaker talks in the first person; no he, she, him or her for a seeded speaker.
- **Choices** are things a person would say or do: speech in quotes, actions as plain words, 40
  characters or fewer. No choice is labelled good or evil, and every option costs something real;
  the kind one usually pays least. A wry line is never the only way through, and never the only
  kind way.
- **Tags are the engine's.** Writers never type "[Persuade]" or "(20 Favor)": the tag comes from
  the check or the price, so it can't promise what isn't tested.
- **The player's voice by background:** Ex-Con and Raider wry and quick; Unemployed and Drifter
  plain, sometimes funny; Soldier and Police procedural; Nurse, Doctor and Lab Technician clinical
  and kind; Hunter, Mechanic and Radio Operator practical, few words; the Elder slow, and knows
  everyone.
- **Failure is a branch:** a failed check names someone else, raises the price or gives half the
  truth. Never a dead end.
- **The word and crew names** ([REPUTATION.md](REPUTATION.md#what-people-call-you)) are said face to
  face only, never on air and never by the narrator. The name itself is said aloud only when they
  tell you what they make of you. A crew name is said the way the crews would: cheerfully, in front
  of people, as a reminder of what they know. It mocks luck, nerve or skill, never a kindness or a
  victim.
- **Rudi** is the Red Hand's official lie, said flat: *"Rudi was bitten."* Nothing after it. The
  boss's line stays LORE's: *"He let people pay later. You can't run a road like that."*
- **The Long Table is never described.** Its people are homely and attentive; they ask where you're
  going and whether anyone expects you. No line anywhere else hints at it.
- **A dry line** (`dry` on a choice) is a node's wry way of saying what a plain choice beside it
  says: the same result, only the reply changes. It's never the only kind option or the only way to
  say no, and the lint holds it to its plain twin.
- **Lies you're told.** A giver or passer-by lies in their own voice, plausibly, and once: one lie
  a conversation. The truth is always findable before it matters, by someone anyone can ask (the
  Doctor wanderer knows the chemist's) or by a background line, and the [Evidence] answer names it:
  *"Picked clean on Day 3. I asked."* Believing a lie is never punished; the truth is a better
  door. No tag ever gives a lie away.
- **Promises** are said in the player's words, with the time in them: *"I'll bring two tins. Two
  hours."* The thank-you and the disappointment are one line each, said plainly, and the
  disappointment blames nobody but the speaker's own hope: *"You said you'd bring tins. I told the
  children."*
- **Barks** are one line, said in passing: never a question that needs an answer, never a player's
  name in someone else's kit, never anything the player couldn't see for themselves.
- **Strangers** speak with one personal detail (a marrow, a pram of pans, a husband across the
  line), in their own words. Small means small: a pinch, not a tragedy. Nobody leans on them.
- **On the open channel** nobody is named, ever: no `{player}`, `{word}` or crew name on air.
- The lint for conversations is `tools/dialogue_pack.py` ([DIALOGUE.md](DIALOGUE.md#tools)); the
  checks below also read every `.rzd` line.

## The race and the lock

✅ Face to face, the voice bank's `route` lines (`Factions/RZ_VoiceDefaults.c`, `Routes`): a leader's
first visit from you each real day, once the routes are open or you've locked, said instead of the
opening ([REPUTATION.md](REPUTATION.md#what-people-call-you)). Shared channels never single out
anyone's route.

| The listener is | What they hear |
|---|---|
| **In-group** (`route:in`, locked to this faction's route) | Gallows humour *with* them, as one of their own: *"Attached. Your name's on the board in pencil. Don't make us rub it out."* |
| **Out-group** (`route:out`) | Cool for two real days (`lockdays<2`): no in-jokes, their word not their name; Bastion says *"Civilian."* Then civil. The Haven stays kind; the Red Hand jokes *at* them, as at everyone; Halcyon is exact and disappointed |
| **Independent** | Everyone is a little afraid of them: *"An uncontrolled variable. You understand why I'd rather you stood over there."* |
| **Unsided** (`route:unsided`, routes open) | Courted once a visit: the Red Hand an offer, Bastion a form, the Haven a bowl and a question, Halcyon a pen |
| **After the cure** | The losing route's own leaders give one plain line, no blame (`route:lost & route:haven`); the winners say theirs to a loser (`flag:finale.route`): the Haven gracious, Bastion files them, the Red Hand *"No hard feelings. Some soft ones."*, Halcyon objections on page two. Never on air, never with a name |

The lock ends at the cure: `route:in`, `out` and `independent` are the locked weeks only. The
pitches, the admissions and the race lines on air are the finale's and the channels'
([FINALE.md](FINALE.md), [designs/TONE.md](../designs/TONE.md)).

## Faction kit and disguises

✅ The voice follows the look's result ([REPUTATION.md](REPUTATION.md#borrowed-kit)), with no fixed
count. A pass in a conversation goes without a word at least one time in three (the server's pick),
else a `look_pass` line (*"Didn't recognise you. Must be the light."*); a catch gets a `look_caught`
line, said only while the look is being made (*"Nice uniform. Whose was it?"*). People react to kit
without any claim in their greetings (*"Where did you get that jacket? Don't answer. I'd have to
write it down."*), saying only what they can see. A disguise never changes the word anyone uses for
you once they know who you are.

## Information comes first

Some lines carry information: chatter with a truck time, toll hours, a watch rotation, the lab's
count. Their information clause is fixed, as a token the code fills in. A joke may ride on top, never
in its place, so players never learn that jokes are filler they can skip. The lint checks that these
lines keep their token.

## Grey choices

**The grey test.** Every choice passes all seven:

1. **Two options someone would defend.** Someone in the world would argue for each.
2. **Every option changes the world, and the text says how.** A cost is real only if something
   changes that the world or a later line reads: food at a house, people at a safehouse, a person's
   fate, a town's infection, a leader's memory, a season flag, the player's Favor or antidote. A
   sentence with nothing behind it fails the test.
3. **No option is best on every axis.** The giver's way pays the giver in full; the kindest way pays
   least; the way that hurts a third party costs somewhere else.
4. **Labelled as actions, not judgements** ([below](#phrasing-choices-and-outcomes)).
5. **The giver's framing is one side.** At least one option needs something the giver didn't say.
6. **Something comes back later, at least sometimes.**
7. **Doing nothing and failing are outcomes too.**

**Labels show costs and requirements, and hide rewards and consequences.** *"Pay their dues
yourself (20 Red Hand Favor)"* is right; *"(+10 Haven Fame)"* is not.

## Phrasing choices and outcomes

- **Choice labels:**
  - Verb and object, from the player's point of view, 40 characters or fewer, with no judgement words.
  - Then the check or cost in short brackets: `[Police or Known]`, `(20 Favor)`.
  - The whole label is 64 characters or fewer, so the action menu doesn't cut it.
  - Good: "Walk him to the line", "Get him away", "Pay what he owes". Not: "Save him", "Do the right
    thing", "[Evil]".
- **Outcomes come in three beats**, then a separate system line:
  1. What happened, in one concrete sentence.
  2. What it cost, and to whom: one detail in the world.
  3. One reaction, from a voice that would have one. If there's a joke, it goes here.

  Then **the system line**: standing and pay as numbers. Older outcomes that put
  *"(+%1 Red Hand Infamy)"* inside the voiced text have it moved out as each one is touched.
- **No narrator verdicts.** Never *"You did the right thing"* or *"Monster."*
- **Use the faction's own word:** "contribution", "borrowed", "walked out", "struck off" (Bastion's
  deserter only), "paid" (the book), "expended", "the other two". The euphemism is the grey.

## Dark humour, not edginess

### Three tests

1. **Who's laughing?** Someone in the world who would. Never the narrator, the system or a plain hint.
2. **At what?**
   - Up: the broadcast, Command, the forms, the book, prices, the lab's euphemisms.
   - Sideways: rivals, in gossip.
   - At themselves.
   - Never down at someone with less power in the scene: the bitten, the volunteers, exiles, the
     people who never arrived, children, or a named dead person. The one exception is the dead
     person's own people, fondly.
3. **What's underneath?** A true detail: a number the world holds, a name, a habit, a shortage. Take
   the joke out and something real is still there; usually the next line says it.

**Callous isn't mocking.** A cruel speaker may be callous about price, odds or count, because that's
who they are (the Red Hand's tips, a crew with a stopwatch). But the target is never the victim's
fear, pain or body; the line is never the narrator's; and the next beat shows the cost, in the same
outcome.

### Never, in any voice

- Sexual violence, in any form. It isn't mentioned at all.
- Torture or suffering as the punchline; gore described for effect.
- A child hurt, or a joke at a child's expense (the Kid, the class trip).
- Suicide as a joke.
- **Rudi's end as a joke.** Rudi was put out of his own garage at dusk on Day 7. The Red Hand's lie
  about it (*"He was bitten"*) is said flatly, and nobody laughs after it. The boss's line stays as
  LORE has it: *"He let people pay later. You can't run a road like that."*
- Slurs, or jokes about real-world groups, ethnicities, faiths as such, disability, or illness as
  identity. The Preacher is a person and the church is a building.
- Real countries, governments, parties, real pandemics, or their politics. The broadcast mocks
  official language in general.
- **The Long Table.** Describing it, or the family or the narrator joking about it. "Pork" belongs
  to the Red Hand, only at the moments already written (when the house is found out, in Cargo and in
  C1), and silence always follows it. **No ambient line ever hints at it**: no meat of unknown
  origin, no "don't ask what it was", no "a place set".
- A player's name as the butt of a joke.
- Memes, internet talk, pop culture, fourth-wall jokes, modern slang.
- Praising cruelty. Only a faction that would laugh does, and the next beat shows the cost.
- A joke that replaces information ([above](#information-comes-first)).
- A joke about anyone who went through the barrier.
- **The wrong words for the infected.** Player text never says "alpha", "tribe" or
  "crawler", and "strays" means only the people the Haven turned out, never the infected.

### Signs a line has turned edgy

- The joke would work in any setting: there's no detail in it.
- It's louder than the thing it's about.
- It needs a swear word to land.
- The victim has no name and no line, and is the punchline.
- The player is made to feel stupid for choosing kindness.
- It repeats: one punchline belongs to one line.

### Each faction's humour

| Voice | Jokes about | How | Goes quiet about |
|---|---|---|---|
| The Haven | Food, chores, house rules, itself, other houses' cooking | Warm, practical, sideways | The List, the mouths, the walks, the Long Table; anyone's dead but its own |
| Bastion | Forms, Command, the roster, rank, rations | Dry, procedural, deadpan, often in the passive voice | Day 2 at the north road, the pilot, "expended", its deserters; its orders once read |
| The Red Hand | Everyone, Bastion most; prices, the book, the toll; itself | Mocking and casual. The joke is how it avoids saying something | Rudi; Cargo; the mothers and sisters in safehouses; the drones' count (the joke dies on air) |
| Halcyon | Rarely on purpose. Tired understatement; words that are exact and terrible | Precise, short | The other two by name, the three weeks, the volunteers |
| The broadcast | Never on purpose | One wrong word | Everything |
| Wanderers | By role: the Trader prices, the Mechanic cars, the Deserter Bastion (bitterly), the Doctor gallows humour, the Scavenger his own rumours, the Courier the post | Their own way | The Preacher almost never jokes. The Kid never does, and nobody jokes about him |

### Before and after

| Edgy | Fixed | Why |
|---|---|---|
| Red Hand, about a Haven death: *"Another do-gooder down. More tins for us."* | *"Heard {subject_town} buried someone. We'll keep the noise down on that road tonight. Don't tell anyone we're nice."* | The joke is now at the Red Hand's own expense, and the death stays real |
| Halcyon, after a trial: *"Volunteer seven popped. Science!"* | *"Volunteer seven didn't walk out. The notes are thorough. The director asks that someone from the Haven come for his coat."* | Exact and guilty, and the coat makes him a person |
| Walk Him Out brief: *"He's going to turn. Bring popcorn."* | *"He declared it himself, at supper. He'd like it done before dark, by someone who isn't family."* | Mercy jobs carry no joke |
| Bastion: *"Shoot first. The paperwork's easier if they're dead."* | *"The form for a civilian casualty is four pages. For an infected it's one. The sergeant checks every one-pager twice."* | It shows the pressure, and the man resisting it |
| Red Hand meal aside: *"Dinner's on. Don't ask what it was."* | *"Dinner's on. Tins. Someone's written prices on them, so don't eat the expensive ones."* | The old line hinted at the Long Table every evening. The new joke is about prices, which is the Red Hand |
| Halcyon: *"It's done. I keep checking the numbers anyway. Habit."* | *"It's done. I keep checking the numbers anyway. Nobody has asked me to stop."* | The Haven also ends on "Habit." One punchline, one owner |
| A crewman about Rudi: *"Rudi? Bit, wasn't he. Bit thick, too."* | *"Rudi was bitten."* Nothing after it. The crewman looks at the shutter, then at you | The lie is said flatly, and the silence carries it |

## Template rules

- **Only real tokens.** A line may use `%1`–`%9` (filled by `string.Format`) and the `Fill` tokens
  ([DESIGN.md](DESIGN.md#world-facts)): `{day}`, `{day_text}`, `{days_since_command}`,
  `{days_since_command_words}`, `{cure_state}`, `{cure_percent}`, `{stage}`, `{stage_tone}`,
  `{time_of_day}`, `{lab_town}`, `{stolen_doses}`, `{town:N}`, `{owner:N}`, `{flag:key}`,
  `{canon:key}`, `{canon_town:key}`, `{leader:id}`, `{leader_short:id}`, `{leader_place:id}`,
  `{tribe:N}`, `{alpha:N}`, `{young:N}` (the packs, in a speaker's word, [INFECTED.md](INFECTED.md#the-words)). A
  questline's text also uses its own keys (`{keeper}`, `{house}`...), set with `run.SetText`.
- **No invented numbers.** If the world doesn't hold a count, use a token or say it without the number.
- **No pronouns for seeded people** (leaders, questline people like `{keeper}`, seeded names). The
  seed stores no gender, so use the name or the title.
- **Ambient notices never reuse a questline clue's text.** The Census's curfew sheet stays a clue.
- **Seeded names never collide with LORE's named people.** Halas and Tomas (the Long Table), Alena
  and Ivo (Bastion's dead and its signaller) and the like are kept out of the seed lists.

## Names in shared text

Headlines name no players. These exceptions stand:
- the Turned Player event's title (its point is whose gear and whose friend);
- the Long Table's headlines that name online family members ([REPUTATION.md](REPUTATION.md#the-long-table));
- players' own radio station presets (they sign their own).

## Writing the infected

The setting is in [LORE.md](LORE.md#what-the-infected-do); these are the rules for their lines (the
call, tracks, the life cycle's lines and headlines, [INFECTED.md](INFECTED.md)).
- **Never explain how it works.** Never "alpha", "tribe" or "crawler" (the lint checks): the speaker's
  word, and always the place (*"the quarry by Morton"*), so players can tell four names mean one pack.
- **Show them through ordinary things:** calls at dusk, birds gone quiet in a wood, crows going up off
  a hedge at noon, a gate swinging, tracks across a field all going one way, a path worn through the
  bracken, two voices at each other, one call where there used to be two.
- **Kill sites and lairs are understated:** a bicycle on its side, a rucksack with its zip open, one
  shoe, somebody's things at the back of a culvert. No gore beyond what a note would say.
- **Inspired by "alpha infected" films, with nothing copied:** no named film characters, no signature
  kills, no trophies, no rituals with bodies. A pack is just people who still go out together at dusk.
- **The young are someone.** Use their names gently; a player's character is described in public text
  unless the server allows the name. Leaders' names are seeded: no pronouns for them (the template
  rules), so a line uses the name again.
- Only the Red Hand jokes about them. Halcyon never says "him" or "her". The Haven never puts "walked
  out" and "taken" in the same sentence. At the dusk meal the Haven reads its dead, and at the same
  hour the leaders call theirs: writers can use that, but never say it.

## Checking text

Two tools check the scripts' text; how to run them is in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#text-lint).

- **`tools/strings_inventory.py`** lists every literal that can reach a player: notices, headlines,
  rumours, briefs, labels and choices, clues, journal lines, outcomes, the guidance tips, the voice
  pools and the voice bank, and every line of the conversation files (`dialogue/**/*.rzd`: says as
  talk-screen lines, labels as choices; the generated packs are skipped). For each: its file and line, the channel (guessed from the call, the assignment or the
  function), the faction (from the folder, or the nearest `case RZ_EFaction.X:`), a register guess
  (PLAIN, WRY, JOKE, TAUNT, BLEAK, SILENCE) and how often it repeats.
- **`tools/voice_lint.py`** checks that list against the rules on this page:

| Check | The rule |
|---|---|
| `length` | Radio, hints, leader lines, rumours, headlines, the broadcast, loudspeakers, talk-screen lines and Traces' lines (channels `trace` and `door`): 200 characters or fewer. Labels and choices: the action part 40 or fewer, the whole 64 or fewer. Briefs over 500: a warning only |
| `bang` | `!` only in Red Hand lines, notice titles and loudspeakers (alarms) |
| `format` | `%1`–`%9` only |
| `token` | Every `{token}` is a `Fill` token or a questline's key (a conversation's tokens in `.rzd` files, `{place}` in the voice bank) |
| `never` | The never list: sexual violence, slurs, real countries and pandemics, internet talk |
| `long-table` | No ambient line hints at the Long Table |
| `infected` | No "alpha", "tribe" or "crawler"; "strays" never for the infected |
| `pronoun` | No he, she, him, her or his after a seeded person's token |
| `info-token` | Information lines (truck times, shifts, tolls, the lab's count) keep a token |
| `clue-echo` | An ambient line never repeats a clue's text |
| `seed-name` | Seeded name lists never use a name LORE.md gives a person |
| `punchline` | A short last sentence ends one line only |

`tools/voice_lint_baseline.txt` holds the findings in today's text (the check, the file and a hash of
the line), so the lint fails only on new or changed text. Each writing pass fixes lines and shrinks
it. The lint checks words and limits, not tone: the three tests above are still the author's.
