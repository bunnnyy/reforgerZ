# Writing

How every line of player text in ReforgerZ is written: the tone, the voices, how people talk to you
(the house rules for talk, below), the rules each line follows, and the lint that checks them. Who the factions are as people, and the setting they speak
from, is in [LORE.md](LORE.md). The epilogue week's lines (route and terms, the lab, the queue,
the leaders' verdicts, residents, the road, the open channels) are in
`Director/Epilogue/RZ_EpilogueText.c` ([EPILOGUE.md](EPILOGUE.md)); a leader's opener in the week is
built from parts here (below).

## Contents

- [Tone](#tone)
- [Everyone is right about something](#everyone-is-right-about-something)
- [The voices](#the-voices)
- [By channel](#by-channel)
- [Talk](#talk)
- [The government broadcast](#the-government-broadcast)
- [Information comes first](#information-comes-first)
- [The season mood](#the-season-mood)
- [The voice bank](#the-voice-bank)
- [Conversations](#conversations)
- [The race and the lock](#the-race-and-the-lock)
- [The epilogue week](#the-epilogue-week)
- [Faction kit and disguises](#faction-kit-and-disguises)
- [Grey choices](#grey-choices)
- [Phrasing choices and outcomes](#phrasing-choices-and-outcomes)
- [Objective lines](#objective-lines)
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
  The children eat first."* When a hungry house takes from a fed one, it's *'borrowed'*, in single quotes.
  The Haven's dark side is always said sideways, never announced.
- **Whoever's outside an opening's door** ([DESIGN.md](DESIGN.md#openings-how-every-life-starts))
  talks like a neighbour on a doorstep: plain, relieved, a little embarrassed, never an institution.
  Nobody is new, so they never say you're new and never explain the Zone: the story comes one piece a
  step, in their own life (*"My brother tried the river road. The loudspeaker said his name. Then it
  didn't."*), and the ask comes in their own need, never as a lesson.
  - **Someone from before** knew you through your background: one shared detail from before (the
    stitches, the van, the apple every morning). Each background has a line for each reason: in
    the hiding place they'd stopped knocking; with your group gone they're the one who came back
    (*"I came back for my watch, and here you are."*); after the fever they wrote on your door.
  - **A stranger** says what they saw and why they stayed, nothing about you they couldn't know
    (*"That was my chalk on your door. I heard you through it for three nights."*).
  - **A wanderer from an earlier life** opens with *"You look like someone I knew."* and reads what
    they remember (a kindness, a wrong, only the face) without naming the old life or its deeds.
  - **What you read by the door** is a note in someone's hand, with one domestic detail (*"Back by
    dark. Don't eat the peaches."*); the veteran's version is its one line. The fever changes nothing
    in the rules, so its words never promise immunity.
  - **The step 2s** are a person's own trouble (a boy with a fever, a van with no fuel, two families
    and one well), with no joke in a bite or a fever: the Clock lost is said plainly and nothing
    follows it.
  - No oaths in this voice: mild swearing stays with the Red Hand and Bastion.
- **Players on the radio station** pick a preset, and it reads like a person: their name, one real
  place, one request. *"This is Mara. There's a horde moving around Morton. Stay clear of it."*
- **Leaders** speak in their faction's voice, coloured by their traits. One short line before a
  brief and one after, never a speech; on the talk screen the brief is followed by where, the pay and
  the danger in their words, and you say yes or no ([Talk](#talk), rule 5):

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
| Replies on the talk screen | The player | What you say, word for word ([Talk](#talk)) | Only the player's own wry replies (`dry`) | A menu label. Judgement words. Rewards or consequences |
| Action labels (world prompts, the object menu) | The player | Verb and object, then the cost or check in brackets | None | Judgement words. Rewards or consequences |
| Status tiles and lock tags ([UI.md](UI.md#the-status-strip-and-locked-rows)) | The screen | Capitals, one word (two at most): the state (ON, COLD, ~25 MIN) or what's missing, in the thing's terms (NEEDS POWER, OPERATOR ONLY, SHORT 1). The full reason is the sentence the action already says | None | A judgement or a reward. A tag that says something the reason doesn't |
| Objectives (the quest tracker, the Quests tab, the map) | The screen | One plain imperative line: who, what and where ([below](#objective-lines)) | None | A pronoun without a name. Leaning on the brief |
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

## Talk

The house rules for everything a person says to you: lines on the talk screen, the replies you
pick, job offers, subtitles, and radio and loudspeaker lines. The model is Fallout: New Vegas: their
name sits at the top right, their line in the band, your replies in the box, and both read like two
people talking. They apply to the conversation files (`dialogue/**/*.rzd`) and to every line and
reply the scripts build; the text lint checks what it can ([below](#checking-text)). Examples on
this page are in italics; the quotation marks some of them carry elsewhere on the page are the
page's, never the line's.

1. **They talk to you, straight.** Their line is spoken to the player in the first person, as
   someone standing in front of you would say it. No quotation marks around it, no speaker label
   ("The guard:", "Sgt Hale says"), no narration or stage directions ("He looks away."), and never a
   line that's only "...". The name is already on screen. A quote inside a line takes single quotes:
   *We call it 'borrowed'. Nobody's fooled.*
2. **Every line carries context.** Someone new to the Zone must be able to follow it: who's talking
   (their job, side, what they care about), what's going on, where (a real place from the tokens),
   and why it matters to you. Build the world: the seal and the Perimeter, the broadcast, the
   factions and their people, the infected, the season's days, shortages, the dead. Concrete details
   (names, places, numbers, objects, smells) beat mood. Two to four sentences is normal. The screen
   pages a node's line in boxes of about three rows; keep a node under about 420 characters and split
   a longer thought into a chain of nodes with a reply like "Go on."
3. **Depth over filler.** Every line tells you something new, gives you a choice that matters, or
   shows who this person is. Cut "Stay safe.", "Hmm.", "Something else?", or give them a reason to
   exist. Keep each faction's voice ([The voices](#the-voices)) and the season mood, the humour line
   ([below](#dark-humour-not-edginess)) and the swearing rule (mild, Red Hand and Bastion only).
4. **Replies are what you say, word for word.** First person, natural, short (under about 70
   characters), never a menu label, and varied by speaker and situation. A check keeps the engine's
   tag in front of the words ([Persuade - Haven - 1 of 2] *You don't want this on your conscience.*):
   writers never type the tag ([DIALOGUE.md](DIALOGUE.md#checks-and-tags)).
5. **A job is explained before you take it.** The giver says what, where, who it's for, why it
   matters to them, what it pays and the danger in it, in their own voice. Then your replies are
   what you'd say, and the yes makes clear you're agreeing: *I'll do it.*, *Count me in. Where do I
   start?*, *What's in it for me?*, *Not today.*
6. **Ids are the engine's.** Node ids, reply ids (`@id`), conditions, effects, checks and tokens
   stay as they are unless a change needs them; the dialogue lint must pass ([DIALOGUE.md](DIALOGUE.md#tools)).
7. **Radio and loudspeakers are speech too.** No quotation marks; a sign-off like *Bastion out.* is
   part of the speech, not a label. The channel's name is the title.

**No filler greeting.** When you walk up and talk, the screen opens on your replies: nobody says a
default hello. They speak first only when they have something to say: a first meeting (`say? !met
:: ...`), a callback or promise, news, an event, their state, a refusal, a job waiting, or because
they called you over. How the engine does it: [DIALOGUE.md](DIALOGUE.md#no-filler-greeting).

| | Not this | This |
|---|---|---|
| Their line | *"Stores are open, such as they are."* | (nothing: the screen opens on your replies) |
| Their line | The Haven: "We're managing." | We're managing, just about. Morton's had soup three nights running and nobody's complained yet, which worries me. |
| Their line | He holds up an empty sack. "Got a tin?" | Got a tin? Just the one. {house} is on half portions and I've a girl there. Look, the sack's empty. |
| Radio | "Bastion to Command. Over." A long hiss. | Bastion to Command. Requesting status of relief. Over. ... Bastion out. |
| Reply | Ask for work | [Job] Recover the Bus (each job a reply, its title with its kind in front); with none, *Got any work for me? (Nothing today. Ask again later)* locked |
| Reply | Trade | Show me what you've got. (the Red Hand), I need to draw from stores. (Bastion) |
| Reply | Something else. / Leave. | Let me ask you something else. / I'll let you get on. |
| Reply | Check standing | (gone: the notebook's Standing tab says it) |
| Reply | Give them your antidote | Here. Take my antidote. Quickly. |
| Job offer | "Nothing for you today." / the job as a card | The leader's intro, the brief and their sign-off, then *It's at Morton, 1.2 km north-east. Pay is 15 Favor on completion, and 10 Fame on the roster.* and the danger; then *[Job] I'll do it.* / *Not today.* |
| Locked reply | the job missing, or "You can't do that." after you pick it | *[Quest] The Other Two: his number (Needs Known with Haven or Halcyon)* |
| Start card | "Errand: The Pump" | *Errand started: The Pump* / From Mara, the cook at Morton. / Fetch the pump's spare valve. Out past the church, north-east. |

**Built replies.** A reply the scripts build from an action's name ("Ask for work", "Trade", "Share
some food", a stranger's "Give him a tin") keeps that name for the world prompt and the object menu;
the talk screen says it in the words in `Dialogue/RZ_TalkWords.c`, by the speaker's faction. A new
built reply gets an entry there (the text lint holds every one to it). An errand's ways on offer are
said as what you agree to: "Fetch the cleaning kits" is *I'll fetch the cleaning kits.*, its "Not now"
*Not now. Ask me again another time.* The way out varies by who you're talking to (*I'll let you get
on.* at the Haven, *That's all for now.* to Bastion).

**Tag words.** A reply that starts a job, or opens its pitch, has its kind in front, in brackets:
**[Quest]** (a questline's or a major's stage), **[Job]** (a contract, a leader's own job, a wanderer's
job), **[Errand]**. The same word starts its cards: *Quest started: ...*, *Job done: ...*, *Errand
abandoned: ...*. A locked reply's need is short and plain, a fact, not a voice: *Needs Known with
Halcyon*, *Needs a Lab Technician*, *Needs 2 food*, *Needs the page on you*, *After dark*, *Not
while Hunted with Bastion*, *Nothing today. Ask again later*; a giver's refusal may be its own line
(*You've your hands full already. Finish what you've got.*). No "heard" tag on anything.

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
- **Grief** (`RZ_ToneGuard`): after a leader's or wanderer's death, one of a holding's named people
  killed, a lost truck crew, a fallen Haven Champion (Remembered) or a player who turned, no wry or
  joke line is said face to face within 1.5 km for 10 minutes; after a death in a safehouse, 30
  minutes. The faction's own open channel goes plain for 20 minutes too, the same for everyone (the
  radio's tails read it). A holding's named person is a person: their body is watched while it's
  shown (every 5 s), and when one is killed a newcomer takes the role under a new name
  (`RZ_Territory.ReplacePerson`; log line `Voice: grief for <name>, the <role> at <place>`).
  *Simplified:* what the dead one remembered of you (dialogue's regard on that person's key) carries
  to the newcomer.
- **A short house** (its last upkeep ran short): a resident's "Ask how things are" can end with a joke
  about the soup, the potatoes or the chores (`house_short`, at twice the mood's share), never about
  who eats, and none in the 30 minutes after a death there (the house's grief). Never in the epilogue
  week's resident line.
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
  `look_pass`, `look_caught`, `look_notice` and `look_guns` ([below](#faction-kit-and-disguises)),
  `change` (the change notice in the faction's voice, by manner), `house_short` (above), `week` (the
  epilogue week's jokes, [below](#the-epilogue-week)), and the openers' `owed` manner and set-word
  lines.
- **Feud or truce** (`truce:<f>`, `feud:<f>` on people's lines): a feud sharpens the Red Hand's taunts
  and Bastion's lists; a truce allows one joke across the line: *"We shared a fire with Bastion last
  night. Both sides counted the matches."* An opener is filled for the listener too: `{word}`, `{best}`, `{worst}`,
  `{crew}` (reputation's tokens).
- **Nobody jokes with a player their leader refuses:** at a seat whose leader holds you at −2 or
  below, the bank picks only plain lines.
- **The open channels** (`radio_tail`, `RZ_VoiceBank.PickShared`): a faction's chatter can end with
  one short line from the bank, **picked once for everyone who hears it** (no listener, no personal
  memory). Its joke share is the lower of the channel's (about 1 in 4) and the faction's mood after
  temper; the Red Hand's never drops below 1 in 5, except in grief; a channel in grief is plain for
  20 minutes. **The channel's own joke memory:** a joke it aired in the last 6 real hours isn't aired
  again (runtime only). Feud and truce lines on the channels read `truce:` and `feud:` (*"We're friends
  with Bastion this week. Hide the good tins."*).
- **Running jokes on the open channels:** the Red Hand's kettle for Bastion (*"Still waiting for
  relief? We'll keep the kettle on."*) runs while `orders.stance` is unset or 1 and is retired once
  Bastion stops waiting; in the epilogue, if the CO closed the net, *"Bastion's net's gone quiet.
  Nobody left to laugh at."* The toll sign is in a new hand once the book burns or its debts die, and
  its joke stops (*"A new sign, in someone else's hand: TOLL 10. No joke under it this time."*).
  Bastion's *"No reply."* on the nightly call is plain, always: it's a fact.
- **The wanderers' greeting** (`wanderer`, Trees on): what they remember of you this life first
  (saved, helped, wronged: a wronged one never jokes with you), else the manner of your Haven standing;
  the Deserter reads Bastion's reversed (`wanderer_deserter`: who Bastion hunts, he warms to).
- **A new life on a locked route:** a leader of your route says *"Somebody said you're with us."* (a
  `route` line with `fresh`: this life under an hour of play).
- **The epilogue week's opener** is built from parts so every ending has plenty of plain lines: what
  the ending is to the speaker's faction (eight endings: each route, the Independent, the slow cure,
  handed over, no cure), then where you stood (on the route that won, on another, on none), then, one
  time in two, the place's own beat. Plain, never a joke; the first visit each real day
  (`RZ_RepVoice.EpilogueOpener`).
- **Earned colours say whose you are:** at a gate, a guard who sees a rival's colours you earned (the
  amber ribbon, the red armband, armour bought at Trusted or Champion) makes one plain remark, once a
  visit, saying only what their hostility already does (*"Red Hand's colours at a Bastion gate. Bold.
  You're on the list. So are the colours."*).

## Conversations

Conversation lines ([DIALOGUE.md](DIALOGUE.md)) follow everything on this page, and these:

- **Lines** follow [Talk](#talk): one to three says a node, spoken straight to you, in the
  faction's voice, coloured by the speaker's traits; no he, she, him or her for a seeded speaker.
- **A first meeting** is a `say? !met :: ...` on the greet: `met` is true once you've talked to that
  speaker this life, for every speaker ([DIALOGUE.md](DIALOGUE.md#no-filler-greeting)). A greet's plain
  `say` is never shown when the screen opens on its own; only its `say?` lines that hold are.
- **Choices** are what you say ([Talk](#talk), rule 4), without quotation marks, 40 characters or
  fewer where the lint allows (labels 60). No choice is labelled good or evil, and every option costs
  something real; the kind one usually pays least. A wry line is never the only way through, and
  never the only kind way.
- **Tags are the engine's.** Writers never type "[Persuade]" or "(20 Favor)": the tag comes from
  the check or the price, so it can't promise what isn't tested, and the screen shows it in front of
  your words.
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
  door. No tag ever gives a lie away. Each faction lies its own way: Bastion by leaving things out
  (*"The patrol saw nothing."*), the Red Hand with a joke (*"Late? Nobody's late. Some people are
  slow learners."*), Halcyon with a true number that isn't the whole number (*"Two of three
  volunteers recovered."*), the Haven with kindness (*"We didn't want to worry you."*), the Long
  Table by being too helpful (*"Take the north road, love. Nobody walks it."*).
- **Promises** are said in the player's words, with the time in them: *"I'll bring two tins. Two
  hours."* The thank-you and the disappointment are one line each, said plainly, and the
  disappointment blames nobody but the speaker's own hope: *"You said you'd bring tins. I told the
  children."*
- **Barks** are one line, said in passing: never a question that needs an answer, never a player's
  name in someone else's kit, never anything the player couldn't see for themselves.
- **Strangers** speak with one personal detail (a marrow, a pram of pans, a husband across the
  line), in their own words. Small means small: a pinch, not a tragedy. Nobody leans on them.
- **On the open channel** nobody is named, ever: no `{player}`, `{word}` or crew name on air.
- **Hubs** ([DIALOGUE.md](DIALOGUE.md#hubs-work-comes-out-of-talk)) are conversations, not menus:
  - **Each person's own way in.** A topic label is something you'd say to *this* person ("Ask why
    the bowls are half full.", "Ask what's worth the fuel.", "Ask what's for supper."), never a
    generic "Any work?" or "Heard anything?". The same job or news from two factions gets two labels.
  - **Work comes out of talk.** They bring up what's wrong first, in their voice; you can ask about
    it, dig (who, why, what's in it for them), push back, bargain or take a check; only then comes the
    choice that offers the job ("Show me.", "Anything need doing?"). The talk sets up what's on their
    mind, never the job's details: the built reply says what the job is, and it may be something else
    or nothing today.
  - **What's going on.** Openings change with the stage, the cure, the lock and the epilogue, the
    town, the faction's need, the news near them, the time and the weather; and with you (your kit,
    a bite, your record, your route). Each speaker type has several openings per state (traits for
    leaders, `persona:1`–`3` for everyone else), so two of one job don't sound alike.
  - **Gossip is specific and sourced.** A real place or person from the news (`{bind:...}`), who
    said it ("Someone's cousin saw the flag change."), and the speaker's own take, worst about
    rivals: Haven gossip stays about food. Some of it is wrong in the faction's own way (Bastion leaves
    things out, the Red Hand jokes, the Haven is kind about it), and someone else knows better.
  - **"What do you make of me"** is a reaction, not a readout: what they can see first (colours, a
    bite), then what you did with them (`{best}`, `{worst}`, `{kills}`, a crew name, a title), then
    your standing. The built verdict is one choice inside it.
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
pitches, the admissions and the race lines on air are the finale's ([FINALE.md](FINALE.md)). The
Independent's commit on air is said without a name: *"This is nobody. The cure goes to everyone."*

## The epilogue week

The epilogue's places, props and closing work are [EPILOGUE.md](EPILOGUE.md)'s; these are the rules for
their lines.

- **Shared text is picked once, on the server, for everyone** (the broadcast, the channels, headlines,
  boards, marks): two players standing together hear the same words, and it never depends on one
  listener's route, standing, kit or jokes heard. Face-to-face text is picked per player.
- **One line per place**, in its owner's voice, pointing at something from this season, not only at
  the winner; at most one wry line per place. The Red Hand is still the funniest. A Long Table that
  lasted the season unfound gets no line: its farm is just quiet.
- **Never contradict the facts.** Until processing the barriers stay closed and nobody has gone home;
  in processing people walk out at the north road one at a time, and nothing past the barrier is ever
  shown or said. What the outside says after the cure is only the loudspeaker's new order, *"Leave it
  on the table. Step back. It will be processed with you."* at the paper's table, and *"Name?"* at the
  north road. Never a reply to anyone.
- **Every ending has lines:** each route that controlled the cure, the slow cure (relief with an
  edge: everyone claims a share, nobody fought for it), handed over (the slow cure's lines with that
  faction's name), and no cure (no wry lines but the Red Hand's last; the Haven reads names).
- **Players are named only in honour lines**, from the hall's records or the frozen record, in the
  name as it was at the deed; a name the filter would change is "a stranger", never asterisks;
  `/anonymous` and the admin strike are honoured.
- **The closing work speaks in the same voices.** A rushed or bitter ending is said plainly, never as a
  punchline; a walk-out is a goodbye.
- **A stronghold leader never jokes about their own page.** Once a page is in the open, rivals may
  gossip about it plainly, never as a joke.
- **The week's jokes** (the bank's `week` lines, `Week` in the defaults): four or five a faction for
  each ending (`cure_held:<who>`: each route, everyone for the Independent's open terms, slow, nobody
  for handed over, spoken as onlookers since any faction may be the one handed it), and for no cure
  (`cure_held:none`) only the Red Hand's last three. A leader says one only after the day's plain
  opener has been said, at the Epilogue's share (1 in 3), through grief and joke memory, never to a
  player they're hostile to or owe a hearing; a roll that finds no joke leaves the built greeting.
  *"Distribution is orderly. The sergeant cried once. It's been logged as weather."*

## Faction kit and disguises

✅ The voice follows the look's result ([REPUTATION.md](REPUTATION.md#borrowed-kit)), with no fixed
count. A pass in a conversation goes without a word at least one time in three (the server's pick),
else a `look_pass` line (*"Didn't recognise you. Must be the light."*); a catch gets a `look_caught`
line, said only while the look is being made (*"Nice uniform. Whose was it?"*). People react to kit
without any claim in their greetings (*"Where did you get that jacket? Don't answer. I'd have to
write it down."*), saying only what they can see. A disguise never changes the word anyone uses for
you once they know who you are.

**Outside a conversation** (borrowed kit's own looks, [REPUTATION.md](REPUTATION.md#borrowed-kit)):
- **A noticing line** (`look_notice`) leads an again look's 10-second notice (*"The sentry stops
  talking and watches you walk."*), and plays only while one of theirs within 15 m can see you and is
  turned toward you (within 60° of where they face, `RZ_DisguiseWatch.Facing`, VERIFY). Turned away
  when it's due, the look waits (owed, as for nobody seeing).
- **Straight to the guns:** a failed look at a post whose AI would shoot you by your standing alone
  (only the kit held them) has no hold, no search and no kit taken: one urgent line (`look_guns`,
  *"'That's not our armband.' Nobody asks twice."*) and their grudge, with the catch's Infamy as
  usual. Elsewhere a catch is held, searched and told as before.

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

The mechanics behind tests 3, 6 and 7 (the reaction beat, nobody chose, someone else made the call,
the clever way closing the honest one, givers who lie, callback deeds, boards of record) are
[DESIGN.md](DESIGN.md#story-blocks)'s. **What each kind of job offers** (at least two grey options a
row):

| Kind | Grey options | Who pays | Watch for |
|---|---|---|---|
| Delivery | deliver; deliver where it's needed more; open it first; skim; sell to the rival | the receiver, or the place that went without | opening it tells you something worth knowing |
| Collection | collect it all; half and cover the rest; from someone who can afford it; warn and refuse; give it back | the house (food), you (Favor), the giver (memory) | no option where everyone is happy |
| Rescue and escort | where the giver wants; where *they* want; where they're safest; hand them over; leave the one who can't make it | the giver's numbers, a house's food, the person's freedom | always ask the person what they want |
| Hold and defend | hold; hold for a higher price, raised mid-fight; get the people out instead; let the rival in | the place, the people inside, your standing | the grey comes before the fight or at a beat in it |
| Hunt a person | kill; bring back alive; let go and claim the kill (proof needed); hear them out; hand them on | the target, the giver's trust, a later callback | every target gets a line; for Bastion, bringing a man home is its own way |
| Retrieve and search | return; read first; copy; burn; sell; give it to the person it's about | whoever the information hurts | reading it adds a journal note |
| Research | the lab's way; the person's way; the Red Hand's price; wait for consent and lose the sample | the volunteer, research, the Haven's trust | the numbers never stand in for the person |
| Mercy | end it; let them walk; to the lab; your antidote; sit with them | you, the town, the person | no jokes, ever; the bitten person chooses if they can |
| Perimeter | do it; send someone else; take the package and leave the person; get them away from the line | whoever walks toward the line | the line never opens; the grey is who gets sent |
| Investigation and puzzles | tell the giver; the family; the rival; keep it quiet; tell everyone | whoever the truth hurts | the answer is a reason, not a villain |
| Trade | deal; cheat (can be found out); take a cut; switch sides; turn the leader in | the other side, the leader, the stores | both sides have a good reason to trade |
| World events | who gets the built loot; protect or rob; help the strangers or not | other players, the stranger | say so when the cost falls on another player |
| Leader personal jobs | do it; do it and turn them in; do it badly on purpose; tell their people | the leader, the stronghold, whoever the rules bend for | the leader's reason is human, and they say it if asked |
| Small side quests | the favour; the favour at someone else's cost; your way; say no kindly or unkindly | small and local: a tin, a lie, a rule bent | [ERRANDS.md](ERRANDS.md) |
| Questline and major stages | all of the above, plus siding | as above | the choice echoes the faction's hard edge |
| The finale | before 90%, any route; after, the grey moves inside the route | everyone | no route reads as the good ending |
| The epilogue's closing work | finish it properly; rushed; leave it to close; someone else's way | mostly the people it's about | a rushed or bitter ending is still an ending, said plainly |

## Phrasing choices and outcomes

- **Action labels** (world prompts, the object menu; a reply on the talk screen is speech instead,
  [Talk](#talk)):
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

## Objective lines

An objective is the line the quest tracker, the Quests tab and the map's mark show for a job: a
mission's `m_sObjective`, a questline stage's or finale route's objective, an errand's or an
opening's step and its "go back" line, and the stage titles listed as done
([MISSIONS.md](MISSIONS.md#objectives)). A player reads it on the move, often days after the brief,
so it never leans on the brief.

- **Who, what and where, in one line that stands on its own.** Name the person by their name or
  token, the thing, and the town, house or grid.
- **One thing at one place.** Steps in a row are separate objectives, never one line chained with
  "then" (a duty's scene names its steps in `RZ_DutyGoals.c`; the way back is always its own line).
- **Short and imperative:** a verb first, one line, no second sentence. The brief keeps the voice;
  the objective is plain.
- **No pronoun without a named person in the same line.** "Find out his number" fails; so does "Go
  back to her". A pronoun is fine once the line names who it is, but a name or "the pilot" reads better.
- **Use the tokens the job already has:** `{giver}` and `{here}` (errands), a questline's keys
  (`{brother_full}`, `{lead_place}`), `{lab_town}` and `{leader:id}` (filled when shown), or the town
  in `string.Format`. Add a token only when the name is in the data but no string can reach it.
- **A stage title is a heading, not a pronoun:** "{brother}'s number", not "His number".

| Not | But |
|---|---|
| Find out his number. | Find Emil Novak's trial number at the lab in Morton. |
| Search the houses over the way. | Search the chemist's down the road, quietly. Then go on to Ruth, at Ruth's house in Morton. |
| Search the houses over the way. | Search the houses across the road from Lucie Brandt's, quietly. |
| Secure it and hold the area for a minute. | Hold the cellar cache in Levie for a minute. |
| Find him. | Find the downed pilot outside Morton and walk the pilot to Levie. |

The text lint's `objective` check holds the pronoun and length rules, and that every mission with a
brief names its objective ([Checking text](#checking-text)).

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
- **The players' tokens** are filled per listener and never go on air: reputation's `{rep}`, `{word}`,
  `{best}`, `{worst}`, `{crew}`, `{name}` and their `:<faction>` forms, and in conversations `{cover}`,
  `{kit_owner}`, `{claim:...}`. The packs' tokens (`{tribe:N}`, `{alpha:N}`, `{young:N}`, and `{pack}` in
  conversations) say each speaker's own word.
- **The Walk Out is a goodbye, never a joke.** Nothing past the barrier is shown or said; *"Name?"*
  is all the outside says.

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
- **Each speaker's word, always with the place:** the Haven names a pack's leader by who it was, or
  "the big one" when nobody knew; Bastion says "contact" and "the heavy"; the Red Hand a crew
  nickname for the pack and "the boss" for its leader; Halcyon a pack number and "the index"
  (`{pack}` and `{pack_leader}` in conversations).
- Only the Red Hand jokes about them. Halcyon never says "him" or "her". The Haven never puts "walked
  out" and "taken" in the same sentence. At the dusk meal the Haven reads its dead, and at the same
  hour the leaders call theirs: writers can use that, but never say it.

## Checking text

Two tools check the scripts' text; how to run them is in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#text-lint).

- **`tools/strings_inventory.py`** lists every literal that can reach a player: notices, headlines,
  rumours, briefs, objective lines (`objective`), labels and choices, clues, journal lines, outcomes,
  the guidance tips, the voice pools and the voice bank, the replies the talk screen's code builds (`reply`: `Reply`, `OwnRow`, a
  scene's `Verb`, and what `RZ_TalkWords` returns), and every line of the conversation files
  (`dialogue/**/*.rzd`: says as talk-screen lines, labels as choices; the generated packs are skipped). For each: its file and line, the channel (guessed from the call, the assignment or the
  function), the faction (from the folder, or the nearest `case RZ_EFaction.X:`), a register guess
  (PLAIN, WRY, JOKE, TAUNT, BLEAK, SILENCE) and how often it repeats.
- **`tools/voice_lint.py`** checks that list against the rules on this page:

| Check | The rule |
|---|---|
| `length` | Radio, hints, leader lines, rumours, headlines, the broadcast, loudspeakers and Traces' lines (channels `trace` and `door`): 200 characters or fewer. Labels and choices: the action part 40 or fewer, the whole 64 or fewer. Briefs over 500: a warning only |
| `quote` | No double quotation marks in a spoken line: talk-screen lines, the radio, loudspeakers, the broadcast, the voice pools, leaders' lines, replies built in code, and a conversation's says and choices ([Talk](#talk), rules 1 and 7) |
| `speaker` | A talk-screen line doesn't start with a speaker's label ("The guard: ", "Sgt Hale: "). A conversation's `say (role)` lines are the engine's and pass, and so do a voice's own heading words (Bastion's "Report: ..." and "Entry: ...", Halcyon's "Subject: ..."; the list is `VOICE_HEADINGS` in the lint) |
| `reply` | A reply (a conversation's choice, or one built in code) reads as speech: not a bare label ("Trade", "Leave.", "Something else."), not an action ("Ask for work", "Tell them you've been bitten"), and it ends like speech (`.` `?` `!`). Check tags in brackets in front or behind are fine; a built label with an entry in `Dialogue/RZ_TalkWords.c` passes, since the screen says it in those words |
| `split` | A talk-screen line built in code, or a conversation node's says together (a `say?` counts as the say it stands in for), is 420 characters or fewer |
| `ellipsis` | No spoken line that's only "..." |
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
| `objective` | An objective line ([Objective lines](#objective-lines): `m_sObjective`, `ObjSteps`, an errand's or opening's `m_sStep`, a finale route's `StageObjective`, the lines `UI/RZ_Quests.c` and the step chain build) has no he, she, him, her, his, they, them or their without a person named in the same line (a capitalised name, a person's `{token}` or a `%1`), and three words or more. A stage title never leans on a pronoun ("His number"). Every class that sets a mission's brief (`m_sBrief`, or `scene.m_sBrief`) also sets its `m_sObjective` |

`tools/voice_lint_baseline.txt` holds the findings in today's text (the check, the file and a hash of
the line), so the lint fails only on new or changed text. Each writing pass fixes lines and shrinks
it: most of it today is the talk pass's `quote` and `reply` findings in the conversation files and
the errands' cards, which the rewrite drives to zero. The lint checks words and limits, not tone or
narration: the three tests above, and whether a line gives context ([Talk](#talk)), are still the
author's.
