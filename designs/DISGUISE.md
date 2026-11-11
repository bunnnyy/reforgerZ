# Borrowed kit: disguises and faction colours

**Status: designed; phase 1 built.** Approved by the author on 2026-11-11. Phase 1 (kit from the loadout
slots, colours at posts and camps, claims and the fixed look at barriers and gates, caught and held, `PassAs`
and TRICK, Bust Out and Double Agent, the stand's radius) is built and lives in
[REPUTATION.md](../docs/REPUTATION.md#borrowed-kit); this file keeps the rest.

Disguises and faction colours that buy a moment, never a living. The base is New Vegas's rules: the rank and file go by
your clothes; leaders see through you, and so does everyone inside a stronghold or the lab; the Hunted can't pass at
all; attacking them breaks it; their enemies react to your colours.

On top of that sits one check, the **look**. It's scored like the persuade check, and you see the result before you
commit, the way New Vegas shows [Speech 50]: "[Full kit +2] [Watched −1] 1 of 2". Wearing someone's kit does nothing
until you **claim** to be one of them: at a barrier, at a gate, in a conversation or in a mission's trick. A pass buys
passage and a moment. It never buys goods, standing, services or the cure. Getting caught costs the kit, +3 Infamy and
one of three catches per faction per life.

## What changed in review

- **One look for every claim** (the plan's section 5). Dialogue's random challenge odds and missions' trick score fold
  into this fixed look, shown before you commit. It's the only judge of a disguise anywhere: barriers, gates,
  conversations, missions, the majors and the finale. Dialogue's and errands' own definitions of a disguise are gone.
- **Cover details** (+1 at the look): Bastion's word at the wire (heard on the secure net, or told by a friend over
  voice; **a wrong word fails at once**), a real name you've heard (a leader's, or a dead man's from the nightly call),
  a crew name, a notebook number, or a mission's paper.
- **Kit kinds:** own, **issued** (a real day's grace when your standing slips below Known), in their kit (looted),
  signed for, a dead man's, off the turned.
- **One cost when caught:** +3 Infamy (never taking you into Watched on its own), the kit taken, one of 3 catches per
  life per faction. Missions' fixed +10 for a failed uniform trick is dropped.
- **One vouch** with one liability everywhere: +5 Infamy for the voucher once per vouch, 2 hours without vouching, and
  regard −1 when a lie was told under the vouch.
- **One `AddInfamy` with flags:** the disguise's `act` parameter is the ACT flag; rival shares carry SHARE and never
  double ([REPUTATION_NAMES.md](REPUTATION_NAMES.md)).
- **Point-outs and call-outs** by other players are −1 each at the look, two at most. The finale's Coat Run uses the
  same rule.
- **"They know your face"** (−1) also covers having talked to their people undisguised this life (dialogue's face),
  never stacking with Watched or Uneasy.
- **Old lore fixed** (conflict 33). A dead Red Hand man's kit passes the Red Hand's look only if it was bought from the
  fence (*"Bought fair. It's in the book."*); taken off the body, it fails. The Civvies card's man is struck off as KIA
  with his name still on the board, and the callback is the deserter handing his tag to the sergeant.
- **The Coat Run** (conflict 34, decision 7). This file keeps only its look: difficulty 3, the lock's −1, and call-outs
  making it harder. What it does (re-labelling the sealed batch inside the lab, during another route's batch hold) is
  [FINALE.md](FINALE.md#the-coat-run-independent-once-a-season)'s. The batch never leaves the cold.
- **Halcyon's kit is the beaks** (conflict 35). Halcyon's full kit is a coat and a hood or mask with a respirator. The
  respirator and gas-mask keywords join the attribute's default string (never an initialiser). The mask is never a tell:
  Halcyon's people always wear them outside the clean room.
- **No disguise near the lab during the stand** (conflict 13): within 1.5 km of the lab, `RZ_Disguise` clears no faction
  bits, with one `IsFinalStand` check like stealth's. At the stand, at most two of the majors' groups come (the finisher's
  faction first) plus Halcyon's guard (conflict 26).
- **Tribes replace Grounds** (conflicts 42 and 43). "Off the turned" is kit off a military tribe, founded at a lost
  Bastion post; its young are still on Bastion's roster, so Bastion's line asks where he is.
- **The names policy.** The Red Hand's open channel never names a caught player. The hall of fame keeps "Caught in
  someone else's kit" as a count only; "Never caught" is an honour line, with `/anonymous` honoured.
- **Side jobs are errand cards,** dealt by [ERRANDS.md](ERRANDS.md) under its rules. Their pay follows the small-pay
  ledger: Fame at most 2 a card. The Name on the Tape is Bastion's variant of the "bring their effects home" family;
  Notebook 31 is merged with His Notebook and Into the Record; Paint takes in Name Day's Brush; Civvies is re-angled away
  from Struck Off and Desertion Route.
- **Checkpoints:** "search the whole car" applies before any look; the Wildcard toll and the lab-coat toll take the
  higher of the two (20), never both.
- **The epilogue lasts one week** (decision 12). Locks end at the cure, so the lock's −1 ends with them. Borrowed kit can
  be given back and dead men's kit carried home ([The last week](#the-last-week)). Nothing carries into the next season
  but hall-of-fame lines.
- **Small fixes:** the Masks Off coat is number 17 (31 is the dead researcher's notebook). The options left open in the
  draft are decided ([Options decided](#options-decided)).

**Contents**
- [1–8. Built in phase 1, and what's left of them](#18-built-in-phase-1-and-whats-left-of-them)
- [9. Word gets round, and vouching](#9-word-gets-round-and-vouching)
- [10. Lines in someone else's kit](#10-lines-in-someone-elses-kit)
- [11. Missions, the majors and the finale](#11-missions-the-majors-and-the-finale)
- [12. Cards for the errands deck](#12-cards-for-the-errands-deck)
- [13. The world tells it](#13-the-world-tells-it)
- [14. Why it can't be farmed](#14-why-it-cant-be-farmed)
- [15. UI and fallback](#15-ui-and-fallback)
- [16. Build in phases](#16-build-in-phases)
- [17. Code shape](#17-code-shape)
- [Examples](#examples)
- [Engine needs](#engine-needs)
- [Options decided](#options-decided)
- [Risks](#risks)

---

## 1–8. Built in phase 1, and what's left of them

**Built** ([REPUTATION.md](../docs/REPUTATION.md#borrowed-kit)): kit read from the loadout slots (colours,
full kit with a head piece, Halcyon's beak, own colours, in their kit, the two tells), claims at road posts
(full kit at the barrier, or the speaker's talk) and at outpost gates while Watched, the fixed look with its
tags shown first (kit, background, one cover detail from the word at the wire heard on the secure net, a crew
name, a leader's name you've met or a mission's paper, dark or fog; minus a known face, an earlier catch, the
notices, looks passed at the place, tells, point-outs, wounds, the infection), passes (cleared at a post, 10
minutes at a gate), caught and held (the one cost; Bastion's search, the Red Hand's fine or beating,
Halcyon's polite walk-off, the open channel without a name), exposure by firing, doubling only for an act
seen while passing, leaders within 15 m and the wire, the stand's radius, colours at posts and camps (the
whole-car search before any look, the beak's toll), surrender first, `PassAs` and TRICK.

**Still to build from these sections:**
- **Kit kinds:** issued kit (a real day's grace below Known); kit that remembers whose it was (a dead man's,
  off the turned, signed for, with their put-on lines and instant fails); Bastion's search taking a dead
  man's or someone else's signed kit; the Red Hand's Trusted shelf sets; a red armband the Red Hand didn't
  give *you* as a tell; the Haven's ribbon refusing talk ("Where did you get that ribbon?").
- **Colours:** Red Hand crews within 150 m giving the grudge on sight during a feud; the Red Hand
  quartermaster and fence refusing Bastion colours ("Take the costume off, soldier."), Bastion's refusing
  Red Hand colours; the Deserter refusing Bastion's jacket; council heads and Haven wanderers within 60 m
  refusing talk in Red Hand colours; under a strict stance, holding fire beyond 35 m without a claim.
- **Looks:** the *again* looks (every 3 minutes within 15 m of one of theirs who can see you, with the
  sight trace, and the notice 10 s ahead; a look owed for 30 minutes), roll call at the shift change, the
  Red Hand's dusk meal, choosing the word at the wire from three (a wrong word fails at once), a notebook
  number, "running" and "a firearm in your hands" (VERIFY), something of theirs from a mission on you,
  someone who saw you put the kit on there.
- **Breakers:** a hit or a kill on their people (retaliation's hook, with only the 3 m infected excuse); a
  sneak approach that spots you.
- **Caught:** Bastion's board description; the reputation name face to face ("You're that Merciful Thug
  from Levie."); walking off while held counted as running a barrier.
## 9. Word gets round, and vouching

**Word gets round**
- **Triggers:** a catch, or a kill of one of their people that retaliation counts. Hits and gunfire never trigger it, so a
  griefer pays for every trigger with a catch or a kill.
- **Scope:** that faction's nearest holding within 1.5 km, and its road posts, so it scales with the map. There's one per
  holding. Another trigger restarts its 30 minutes and never queues, so there's nothing to flush.
- **Effect:** −1 at every look there, for everyone. Known players and wave-throughs are untouched.
- **Lines**, by faction and trigger:

| | A catch | A kill |
|---|---|---|
| Bastion | *"All stations: civilian in our kit at Morton. Check faces. Bastion out."* | *"All stations: Morton has a man down. Faces checked at the wire. Bastion out."* |
| The Red Hand | *"Someone's wearing hands we didn't paint. Ask names at the toll."* | *"Somebody dropped one of ours by Levie. Nobody walks through on a smile till dusk."* |
| Halcyon | *"Security notice: a coat was presented at the station by someone not in the record. Please check numbers."* | *"A member of staff was killed near the station. Please stay inside the wire."* |

The secure channel names the grid, so Radio Operators hear it. Leader news and wanderer road news carry it for an hour:
*"Heard someone walked into Morton in a dead man's jacket."*

**Vouching** is the cooperative way in, and a use for your own colours. It's the one vouch mechanic, shared with
dialogue's vouch and the errands' Vouch card.
- **Sign them in** (Bastion, Known), **Vouch for them** (the Red Hand, Trusted), **Sign a visitor in** (Halcyon, Known, at
  the field station or the lab's gate). It needs your face and their colours.
- At a barrier or gate, you can vouch for one other player within 10 m, and they go through without a look:
  - Bastion waves them through unsearched;
  - the Red Hand charges them the Known toll instead of turning them back;
  - Halcyon lets a Watched visitor in for 10 minutes.
- **The voucher answers for them.** If the person you vouched for is caught in that faction's kit, or hits or kills its
  people, within 30 minutes and 1 km, you take +5 Infamy with that faction (once per vouch) and can't vouch for 2 hours.
  If they told a lie there under your vouch, you also take regard −1 with that speaker.
- Without a vouch, everyone is checked on their own. Walking behind a friend gets you nothing.

## 10. Lines in someone else's kit

Claim lines go on the conversations of their people: named people, the gate sentry, road post crews (through the post
speaker at the barrier) and a mission's people ([DIALOGUE.md](DIALOGUE.md#12-claims-in-someone-elses-kit)). There are no
companions. Without Dialogue on, the **Pass as one of them** action does the same job, and the answer comes as an
owner-only notice.

- Each choice shows its tags: *"[Bastion kit +2] [Name: Sgt Hruby +1] [Watched −1] 'Private Dusek, Third Section.' 2 of 2"*.
- Each speaker hears one claim line from you per life.
- A pass gets you through, or one line they'd tell their own (never more than their open channel says), or the mission's
  trick outcome. Never anything that pays.

**Failure lines** (grey, darkly funny, in each faction's voice):
- **Bastion:** *"Corporal Dusek? Dusek went on the nail on Day 12. Hands on the barrier."*
- **The Red Hand:** *"Nobody calls you anything. That's the problem. Wallet or teeth?"*
- **Halcyon:** *"Your notebook number, please. ... That's a phone number. Please take the coat off. It's numbered."*

**The Rogue leader** is the one exception to "leaders see through it": *"Nice jacket. Twenty Favor and I never saw it."*
- Pay 20 of his faction's Favor and he doesn't turn you in. You're passing at his seat for 10 minutes, and he writes it
  down.
- If his rogue work is discovered later, everyone who paid him is named in that faction's records: +5 Infamy each.
- Loyal and Independent leaders never play along.

**Names are real.** Every name in these lines comes from the season, never from a fixed string:
- **Bastion's nail list:** a seeded name (`RZ_FactionVoice.PersonName`) for every Bastion guard, crew member or named
  person who dies. His tag goes on the nail behind the radio, and his name stays on the roster with a KIA. The last 30 are
  kept and saved, and they're read on the nightly call. A struck-off man's nail stays empty; Private Marek Fiala, the
  finale's canon deserter, is never generated.
- **Halcyon's numbered coats and notebooks.**
- **The Red Hand's crew names.**

The nail list also feeds the boards, the lines and the epilogue. One record serves the nail list, the roster and "struck
off": cards add names to it; only the majors and the built nightly call read names on the air.

## 11. Missions, the majors and the finale

**The trick approach** is built (`RZ_Ways.PassAs`, `RZ_Disguise.AddWatcher`, TRICK,
[DESIGN.md](../docs/DESIGN.md#checks-and-ways)). Still to build: a stage that offers a trick sets `<key>.disguised`
*(you)* for later stages; a wanderer who remembers you, a life record about you and hunter squads always
seeing through it (the mission passes `seenThrough` today); keeping trick objectives 25 m from a leader's seat.

**Changes to built missions:** built (Bust Out's claim and second look, Double Agent's colours rule,
[MISSIONS.md](../docs/MISSIONS.md)).

**The majors** ([MAJORS.md](MAJORS.md))
- **Await Relief, Act IV.** A player below Known with Bastion (for example, whoever read the page on the open channel in
  Act II) can read a post's roster at night in Bastion kit, as a trick alongside the sneak.
  - The duty board is in the radio room, at least 25 m from the commander's seat.
  - Two looks: the gate sentry (difficulty 2), and one more inside at the board (difficulty 3). That takes a Stranger
    with full kit, the night and a name, or a Soldier or Police background for someone Watched.
  - The cover detail is a name off the nail. A dead man's name works anywhere except at his own post.
  - Caught: the act's +8 doubles, within the questline cap of 15, plus the +3 for the impersonation. That post stays
    padded.
  - Callback at the muster: *"And whoever signed the roster as {dead_name}: he's still dead."* That player's persuade at
    the muster is one step harder.
- **The Book's** supper theft happens at the boss's table in the stronghold, so no disguise works there, as the majors
  already assume.
- **Proof's records room** (HC2's night scene) can be done in a Halcyon coat and beak at night.
  - The duty officer is the watcher (difficulty 3).
  - The room is at least 25 m from the director's seat.
  - A real notebook number is the cover detail. Notebook 31 gives one ([section 12](#12-cards-for-the-errands-deck)).

**The finale** ([FINALE.md](FINALE.md)) owns the routes, the lock and the stand. The disguise rules it keeps:
- Before 90% nothing is locked, and disguises work as above.
- After the 90% lock, every look with every faction whose route you didn't choose is −1. For the Independent route,
  that's every faction.
  - It follows your identity, so a new life can't wash it off.
  - A late joiner who hasn't locked has no −1 until they do.
  - It ends at the cure, with the locks.
- The final stand's groups are fights, and during the stand no disguise works within 1.5 km of the lab: `RZ_Disguise`
  clears no faction bits there.
- At the stand, at most two of the majors' groups come (the finisher's faction first, then by active locks at the lab),
  plus Halcyon's guard, which fights only the infected. Your route's colours mark you as theirs to your route's group.
- **The Coat Run** (the Independent's once-a-season trick) uses this look: **difficulty 3, with the lock's −1**. It takes
  Halcyon's full kit (the coat, and a hood or mask with a respirator), a real notebook number and the night, or a ★ Lab
  Technician's background. Each holder-route player standing at the clean room door who calls the player out makes it one
  step harder (two at most), as any point-out does. What a pass does (re-labelling the sealed batch inside the lab, during
  another route's batch hold) and what a catch tells the batch's route are in
  [FINALE.md](FINALE.md#the-coat-run-independent-once-a-season). A catch costs the usual: +3 Halcyon Infamy, the coat
  taken, one of three catches.
- **Take the Lab** (the Red Hand route's theft) is a hold at the loading bay, not a disguise.

## 12. Cards for the errands deck

These small jobs are **errand cards**, dealt by [ERRANDS.md](ERRANDS.md) under its rules: a live Why, the dealer, the
interruption budget, and the small-pay ledger (errand Fame at most 2 a card, never above 39 with a faction, 10 a faction
a season day; Favor inside the hourly ledger; amends instead at 15+ Infamy). Leaders never give them: a named person
does. Each is offered only when the season's state makes it relevant, by someone who would really ask. The conditions
count events over time and across different players, so one friend's cheap catch can't open a card for the whole group.

| Card | Giver, and when | What happens |
|---|---|---|
| **The Name on the Tape** (Bastion's variant of the "bring their effects home" family) | A Bastion named person, for players Known with Bastion. One of Bastion's people died near a post this season day (to the infected or in a raid), and his body is still out there | The card puts his body where he fell (a prop, removed afterwards). Bring his tag and jacket to that post's quartermaster: 4 Favor, +1 Fame, his tag on the nail, and his name on the nightly call. Or sell the jacket to the Red Hand's fence for 10 Red Hand Favor (*"A name the army can't check twice"*), and the post's board lists him as "kit lost". It refuses anything a quartermaster sold, and kit off a man you killed (*"You'd know. You were there."*, +3 Bastion Infamy). Once per dead man, 3 times a life |
| **Spot the Tells** | A Bastion named person, for Known. Two or more different players were caught in Bastion kit near a post in the last 2 season days | A Red Hand scout in Bastion kit stands among the post's three named people. Talk to each and find him by his tells: the wrong boots, an AK, no cap. Right: +2 Fame, 5 Favor, and he goes to the stockade (Bust Out runs a little more often). Wrong: *"Private Hollis spends the night in the stockade for wearing the wrong boots. He's furious. He's also, technically, out of uniform."* No pay. Or take the scout's 10 Red Hand Favor to point at Hollis (35% it comes out: +5 Bastion Infamy) |
| **Paint** (with Name Day's Brush) | A crew boss's man, for Known with the Red Hand. A Bastion checkpoint stands at night within 2.5 km of a Red Hand camp | *"New faces paint a red hand on."* Paint one on the barrier or on a Bastion sign in sight of the checkpoint: 20 s at the barrier prop. The post comes down within 45 minutes, so nothing stays. Trick: in Bastion kit, ask the crew for a light. Sneak: from the back, unseen. Force: it's a fight. Pays a crew name for the life (with the majors on, the same `book.named` the pit gives), picked from whatever went wrong, and +2 Fame. Bastion's board: *"Civilian with a paint tin. Day 14."* |
| **Masks Off** | A Halcyon named person, for Known. A safehouse took a bite declaration in the last season day | A beak (a man in a coat, hood and respirator) has been taking "samples" from the bitten. He's a Haven orderly selling them to the Red Hand for his daughter's antidote. Turn him in: +2 Halcyon Fame, and the coat goes back in the record. Let him go: +2 Haven Fame. Or take his coat: *"It's a good coat. It's also number 17, and number 17 is signed out to Dr Brandt."* It's useless at Halcyon's looks, and there's a 35% chance the lab finds out: +3 Halcyon Infamy |
| **Notebook 31** (merged with His Notebook and Into the Record) | A Halcyon named person, for anyone. A Halcyon courier or researcher died this season (The Missing Courier, Lost Colleague, a field lead's death) | His numbered coat and notebook are where he fell (a prop). Return both: +2 Halcyon Fame, the notebook read into the record, and his name on the authors' list at the change of shift. Sell the notebook to the fence: 15 Red Hand Favor (35% it's found out: +5 Halcyon Infamy). Keep the notebook: its number is a real cover detail for one Halcyon look. The coat is a dead man's coat, so it's no good at Halcyon's looks |
| **Wrong Way Round** | A crew boss's man, for Trusted with the Red Hand. A Haven house sits on a road the Red Hand doesn't hold | The house's lads take tolls under painted red hands to keep raiders off, and they painted them the wrong way round. Tell the boss: the lads are "settled", the house loses 1 person, +2 Red Hand Fame, +5 Haven Infamy. Take a cut: 6 Haven Favor from the lads' takings, once (35% the house finds out: +5 Haven Infamy). Warn them off: +2 Haven Fame. Or recruit them: +2 Red Hand Fame, and the house loses 1 person |
| **Civvies** (re-angled) | A Haven occupant, for anyone. A deserter struck off Bastion's strength as KIA, his name still on the board, is sheltering at a safehouse (a Deserter wanderer resting there, or a man generated for the card; never the finale's canon deserter) | The disguise in reverse: he wants to stop looking like a soldier. Bring him civilian clothes (nothing with a faction's keywords): +2 Haven Fame, and he stays as one of the house. What happens to him after that is Struck Off's story, not this card's: if a player takes his tag to a Bastion post, Struck Off opens with him as its subject, and Bastion's way ends with him handing his tag to the sergeant and answering "Here" at the roll. If you sell where he sleeps to the Red Hand (10 Red Hand Favor), he's gone by morning (a headline). It never overlaps Desertion Route, which is about the men a commander lets go |
| **Laundry Day** | A Haven occupant, for Known. A Haven and Bastion truce, a house near a Bastion post, and a Red Hand roadblock in between | The house washes Bastion's uniforms for bread. Get the sack (a mission item, not wearable) past the roadblock. Wear a uniform from it on top: that's Bastion colours, so you're turned round, and during a feud you get the grudge. Or go round, more than 150 m from the roadblock. Or pay the toll twice (20 Red Hand Favor). Or sell the sack to the fence for 15 Red Hand Favor: the house loses its bread (−2 food), and there's a 35% chance it's found out (+3 Haven Infamy). Delivered: +2 Haven Fame, +1 Bastion Fame, and the house gets +2 food |

**Built:** Dress Rehearsal and Borrowed Green ([docs/ERRANDS.md](../docs/ERRANDS.md#the-cards)), and The
Officer's Coat (the stockade mother, for a player disguised in Bastion kit). **Next:** The Name on the Tape and
Spot the Tells, then the rest.

## 13. The world tells it

All of this is text on props and voices that already exist. Nothing physical stays.

**During the season**
- **Bastion's board** at each holding lists the season's impostors, built from each catch: the kit, the tells and the
  name given (never the player's). It's read at the quartermaster or the sentry.
- **The nightly call** reads the names that went on the nail, and the jackets that came back.
- **The Red Hand's chatter** keeps a running joke going: *"That's three this week in Bastion green. We should open a
  tailor's."*
- **Halcyon** reads out the coat count at the change of shift: *"Coats returned: 7 of 9."*
- **Haven gossip** is sideways and about food: *"Somebody came to Morton in a soldier's coat asking for bread. We gave
  him bread. The coat didn't fit."*

### The last week

After the cure the epilogue lasts **one week** (168 hours; decision 12). [EPILOGUE.md](EPILOGUE.md) owns the closure set;
the disguise system adds these, all finishable within the week:
- **The lock's −1 ends** with the locks at the cure, and every grudge the contest started stands down once. Looks go
  back to their season rules.
- **Bring Them Home.** A dead man's tag, jacket or notebook carried back to his post or station is taken in: Bastion puts
  the tag on the nail and reads his name at the roll; Halcyon reads the notebook into the record and adds him to the
  authors; the Red Hand writes his line in the book as "sent off". Kit off the turned from a military tribe goes back to
  the lost post's roster: *"He's home. Mark him."* Pay is the epilogue deck's, never anything that carries on.
- **Give back borrowed kit** (part of Settle Accounts). Signed-for kit worn by someone else can be handed in at its
  buyer's quartermaster: no catch, no Infamy, and the buyer hears it came back (*"Your jacket came home. Clean, mostly."*).
  Clean kit from the Red Hand's shelf can be sold back to the fence at half.
- **Lines**, built from the season's counts:
  - Bastion, at each holding: *"Eleven jackets on the wire at Morton. None of them fit anyone."*
  - The Red Hand, from the lorry at the crossroads: *"Best-dressed of the season: {player}, through nine of Bastion's
    checkpoints in green and never caught. Drinks are on Bastion."* It's an honour line: the name is frozen and
    filtered, and an `/anonymous` player is "somebody".
  - Halcyon, with the paper: *"Coats returned: 31 of 34. The rest are listed as authors."*
  - The Haven: *"The ribbon only ever went on the people it was given to. Mostly."*
- **The hall of fame:** "Caught in someone else's kit" (a count, with no names) and "Never caught" (the most looks passed
  without a catch, as an honour line).

## 14. Why it can't be farmed

| Attempt | Why it fails |
|---|---|
| Walk in and help yourself | Nothing a claim buys pays. Holdings have no stores. Shops, services, leaders and hand-ins check your real standing. The wire catches you |
| Kill one of them and wear his kit | It's a dead man's kit. You're told when you put it on, and it fails at their look. The Red Hand's passes only if bought from the fence |
| Loot uniforms off military infected | It's kit off the turned, and it fails at their look |
| A friend buys kit for you | It's signed for, so it fails at their look and is taken at a Bastion search |
| Guess the word at the wire | A guess is right 1 time in 3; a wrong word is an instant catch |
| Farm standing | No Fame or Favor ever comes from a disguise. Infamy is always yours. Amends and surrender need your face |
| Retry until it works | The check is fixed and shown first. Lines are once per speaker per life. Catches count: three per life |
| Stay inside | A gate pass is 10 minutes. Each *again* look is one step harder. Roll call comes at the shift change |
| Friends take turns | Word gets round at the holding, for everyone |
| Grief a friend's plan | Only catches and kills spread word, and each one costs the griefer. There's one per holding, restarted, never queued. A point-out is −1 at most twice, and a wrong one costs the pointer |
| Shoot guards from inside a horde | A hit always exposes you. Only an infected within 3 m of the victim excuses it |
| A Hunted team's opening volley from 36 m | The Hunted can't claim. Like Vilified in New Vegas, they're seen through |
| Log out before a look | Passes, owed looks and the looks you've passed are kept by identity for 30 minutes after a disconnect. Catches and blown time are saved with the life |
| Swap kits to reset | Blown time, catches and word round belong to you and the place, not the kit |
| Walk behind a friend at a barrier | Everyone is checked on their own unless vouched for, and the voucher answers for you |
| Hide in a searched car | "We search the whole car" comes before any look |
| Get intel from lines | Lines never say more than the open channel. A stage's facts come only through that stage's own trick, with its own pay and callback |
| Farm the cards | The Name on the Tape refuses sold kit and kit you killed for, pays once per dead man and at most 3 times a life. Dress Rehearsal is once per life and needs you below Known with Bastion. Card pay is the errands' small-pay ledger. Conditions count events over time and across different players |
| Use colours as a licence to kill | Colours are refusals and timed grudges, never standing bits, so retaliation still counts every kill |
| Hide Infamy behind Fame | Tolerated and Wildcard players aren't disguised. They're recognised and treated by their real standing, as built |
| Use it on the cure | No disguise works within 1.5 km of the lab during the stand. After the lock your face counts against you with every route you didn't choose. The Coat Run is once a season, at difficulty 3 |

## 15. UI and fallback

- **HUD.** No meter. While a look is coming, one eye (the Watched eye icon) appears with the score, "2 of 2", in a new
  widget, `RZ_Hud_Look` (to add to docs/UI.md and UI/LAYOUTS.md). `RZ_UIFeed` sends it owner-only, and only when it
  changes.
- **Standing screen**, per faction:
  - *Your face:* unknown / known / caught 1 of 3 / blown for 40 more minutes.
  - *Their kit:* their tells ("Bastion carries service rifles and wears a helmet or cap"; "Halcyon wears the beak").
  - *Their colours:* what their enemies will do.
- **The first time you wear their kit**, one notice gives the rule of thumb: *"In Bastion green, Bastion's people look
  at your face up close. Don't run, sling your rifle, keep away from officers and never go inside the wire. The Red Hand
  will treat you as Bastion."*
- **Fallback** without layouts: the notices and the **Pass as one of them** action.

## 16. Build in phases

It touches about a dozen built systems, so it ships in phases, each gated by its VERIFY tests and playable on its own.

| Disguise phase | Plan phase | What | Gate |
|---|---|---|---|
| 0 | 0 | A Workbench test, with no code merged: worn clothing read from the loadout slots; worn told apart from carried; head pieces told apart from body pieces; a worn piece removed; sprint and weapon-raised state; a sight trace. Also, in phase 0's fixes: the SHARE flag (doubling stops hitting rival shares) and the trial-dose gate fix | If worn kit can't be read from the slots, **Disguises** stays off |
| 1 | 4 | **Built** ([REPUTATION.md](../docs/REPUTATION.md#borrowed-kit)); the leftovers are listed in sections 1–8 above. Cards: Dress Rehearsal and Borrowed Green built ([ERRANDS.md](../docs/ERRANDS.md#the-cards)) | `tools/compile_check.sh`, then the runtime test |
| — | 5 | Await Relief's Act IV trick | The same |
| — | 6 | The lock's −1, the route colours at the stand, the Coat Run's look | The same |
| — | 7 | The last week's lines, Bring Them Home and giving kit back, the hall of fame | The same |
| 2 | 8 | Claim lines in conversations (or the action); *again* looks with the trace; roll call; word gets round and its lines; vouching; point-outs; the Rogue's price; the board. Cards: Spot the Tells, Paint, Wrong Way Round, Laundry Day, Civvies | The same |
| 3 | 8 | Dead man's kit, kit off the turned and signed-for kit; issued kit; confiscation; the nail list. Cards: The Name on the Tape, Notebook 31, Masks Off | The same |

**One fix that doesn't wait** (phase 0). A bitten, unarmed player is always let through the lab's gate for the trial dose,
whatever their standing short of Hunted and whatever they wear (`RZ_Checkpoints.CampContact`). Today a Watched player is
turned away, but the trial dose is owed to everyone.

## 17. Code shape

**Files**
- `Factions/RZ_Disguise.c`: reading kit from the slots (colours, full kit, head piece, tells, marks), and each player's
  state per faction (own colours, issued, in their kit, passing, blown).
- `Factions/RZ_DisguiseLooks.c`: looks, claims, passes, *again* looks, breakers, caught and held, word gets round,
  vouching. The score itself is `RZ_Checks.Look`.
- The marks (dead man's, off the turned, signed for) and the nail list live in `RZ_Disguise` until they outgrow it.

**Hooks into built systems**
- **`RZ_Hostility`:**
  - `Covers` now means passing (or in their kit under a strict stance, beyond 35 m), not blown, none of that faction's
    fights on you, and not within 1.5 km of the lab during the stand.
  - A shared query, `NearestPeople(faction, pos, radius)`, over the units already tracked: retaliation's watched units,
    road post crews, holding guards and named people, leaders' bodies and mission watchers.
- **`RZ_Checkpoints`:** claims and looks at barriers and gates; the group search before any look; being held; colours at
  posts and camps (the higher toll, never both); surrender first; vouching; the trial-dose gate fix; the armband taken
  only when worn.
- **`RZ_Retaliation`:** marks kit at death, with the nail name and the killer; the kill trigger for word gets round; hits
  that expose (the 3 m rule).
- **`RZ_ZombieSpawner`** (and the tribes' bodies): marks kit off the turned when it counts a kill.
- **`RZ_GameModeComponent.AddInfamy`:** the ACT and SHARE flags. It doubles only for ACT while passing, within 150 m of
  their people or holdings. The four share callers pass SHARE.
- **`RZ_QuartermasterHelper.MarkBought`:** adds the buyer's identity.
- **`RZ_Leaders`:** leaders see through you within 15 m; the Rogue's price and its discovery; talk refused while you wear
  rival colours.
- **`RZ_FactionVoice` and `RZ_FactionLife`:** the lines, the board, the channels, rumours and the coat count, all through
  the names policy.
- **`RZ_PlayerProfile`:** catches[4] and blown-until[4], in played seconds, saved with the life.
- **`RZ_UIFeed`:** the coming look, owner-only.
- **Missions and errands:** `RZ_QuestApproach.PassAs` (in `RZ_Ways`), `RZ_EQuestApproach.TRICK`, `RZ_Disguise.AddWatcher`,
  Bust Out, Double Agent and the cards.
- **Dialogue:** claim choices run `RZ_Checks.Look`; `pass:` and `catch:` effects; point-outs.

**State**
- **Runtime, kept by identity for 30 minutes after a disconnect:** passes, owed looks, looks passed per place, who saw
  you put kit on, point-outs at the current look, and which speakers have heard a line.
- **Per holding:** word gets round.
- **Saved, about 2 KB:** the nail list (30 per faction) and the season's counts (catches per holding, jackets back, coats
  returned, the best impostor).

**Settings** on Hostility, as attributes with no initialisers (defaults only in the attribute strings):
- head-piece and service-weapon keywords per faction, with Halcyon's respirator and gas-mask keywords in the default
  string;
- Halcyon's long-gun rule;
- blown minutes (60) and catches per life (3);
- impersonation Infamy (3) and its hourly cap (6);
- word-gets-round minutes (30) and holding radius (1,500 m);
- gate pass minutes (10);
- confiscation (on);
- the Red Hand's fine (10) and beating share (0.2);
- Colours (on);
- show the score (on);
- claim in full kit when there's no barrier prop (on);
- vouching (on); point-outs (on, at most 2 at a look);
- the Rogue's price (20);
- the Red Hand shelf's clean sets (on, 40 Favor);
- the stand's no-disguise radius (1,500 m).

The built settings (pieces 2, check seconds 10, the keywords) stay.

**Performance**
- The kit is read every 10 s from a few slots, as now.
- Looks happen only at claims and for passing players, with at most one sight trace per *again* look.
- Marks are weak references, capped at 200.
- Nothing is spawned except the cards' own props, which go when the card ends.

**Docs when built**
- **REPUTATION.md:** rewrite the Disguises section (kit kinds, colours, looks, caught, vouching), and update the
  consequences line, the shooting table and the checkpoints section.
- **DESIGN.md:** the building-block row (`PassAs`, the trick approach).
- **MISSIONS.md:** Bust Out, Double Agent and the cards.
- **QUESTLINES.md:** the majors' trick stages, when they're built.
- **WORKBENCH_SETUP.md:** the settings row, the keywords (with the respirator), the barrier and occupant actions, the
  VERIFY lines, and replace the two disguise checklist lines.
- **UI.md and UI/LAYOUTS.md:** `RZ_Hud_Look` and the standing lines.
- **ROADMAP.md:** updated.

---

## Examples

**Bust Out, done properly.** An Ex-Con, Watched by Bastion at 25 Infamy, has clean Bastion green and a cap from the Red
Hand's Trusted shelf. He heard the post commander's name, Sgt Hruby, in Bastion's news. Cut the lock shows "[Full kit +2]
[Name: Sgt Hruby +1] [Watched −1] 2 of 2". A guard asks "Who sent you?", takes the name and looks away, and the 8 s cut is
done before the second look. A Stranger who isn't an Ex-Con passes the first look at 3, and the second, at 15 s ("What
are you doing with those?" "Orders. They're being moved."), at 2. If either of them had worn the jacket off the truck
driver he shot that afternoon, putting it on would have said "The name tape says DUSEK", and the first look would have
failed: the alarm, as now.

**The barrier, and the word.** A Soldier who has gone bad with Bastion (Watched) reaches a checkpoint at dusk in full
kit. At 150 m the loudhailer calls him in, and his notice reads "[Full kit +2] [Soldier +1] [Dark +1] [Watched −1] Pass
as one of them: 3 of 2." At the barrier: "Not on the board. Word." His friend, a Radio Operator, heard the word on the
secure net that morning and told him over voice. He gives it: 4 of 2. He keeps the suppressor a search would have taken,
and asks the sentry about the truck: "Truck for Levie at 20:30. Don't tell the Red Hand." Bastion's open channel said the
same an hour ago. It's the only claim line this sentry will hear from him this life. Had he guessed and got the word
wrong, he'd have been caught on the spot.

**Seeing it coming.** A Watched player passes an outpost's gate at night (full kit +2, dark +1, Watched −1: 2 of 2) to
find a prisoner's name on the stockade board for a Haven job. Her 10 minutes run out while she's still inside. Ten
seconds before the sentry's next look, her notice reads "[Full kit +2] [Dark +1] [Watched −1] [Looked at once −1] 1 of
2." She walks out of the gate. She's lost nothing, but if she claims there again within 30 minutes, the sentry looks at
her straight away.

**Friends taking turns.** The first friend, a Stranger at 13 Bastion Infamy, fails a look at a barrier near Morton. He's
held for 15 s and searched, and his jacket and cap are taken. The impersonation +3 stops at 14, so he's still a Stranger.
Word gets round at Morton and its road posts for 30 minutes. The second friend arrives five minutes later in different
clean kit. At 150 m her notice reads "[Full kit +2] [Word's round −1] 1 of 2." She turns back.

**Hunted in kit.** A player at 45 Bastion Infamy walks to a checkpoint in full Bastion kit with no firearm. The Hunted
can't claim ("They know a Hunted face"), so the kit buys him nothing. Surrender comes first: he's searched, the kit is
taken as army property, and he gets −8 Bastion Infamy (through the forgiveness budget) and five minutes' ceasefire. No
catch, and no word round. If he'd come armed, they'd have shot him, kit or no kit.

**A leader.** Inside an outpost on a gate pass, a player walks to within 15 m of the commander's seat: "I know every face
under my command, and yours isn't one of them." She's caught: held, kit taken, +3 Infamy, and word gets round. At a Rogue
crew boss's camp, the same walk gets "Nice jacket. Twenty Favor and I never saw it." She pays and has 10 minutes at his
seat. A week later his skimming is discovered, and everyone who paid him is named in the book: +5 Red Hand Infamy each.

**Red Hand liars pay.** A Stranger in a red bandana and an armband he took off a dead raider walks up to a roadblock. His
notice said "[Their colours +1] [Not your armband −1] 0 of 2", and he claims anyway. "What do they call you, and who
vouches?" He has no crew name. "Nobody calls you anything. That's the problem. Wallet or teeth?" He has 4 Red Hand Favor,
not 10, so it's teeth: 20% of his health, never below the floor. The armband and the bandana are taken, and the open
channel laughs about a man who tried to pay with a salute, without his name.

**A dead crewman's jacket.** A player buys a dead crewman's leather jacket from the fence the night of his send-off. At
the next roadblock: "Bought fair. It's in the book." It counts as clean kit. Another player who took the same kind of
jacket off a crewman's body on the hill road is told at the look: "That's his jacket. It hasn't been through the market."
Caught.

**Halcyon and the bitten.** A player in the Weakness stage, Watched by Halcyon, puts on a coat, hood and respirator to
talk her way into the field station. Showing the infection is an instant fail at a Halcyon look: "They see bites for a
living." She's held, the coat and beak are taken ("It's numbered"), she gets +3 Halcyon Infamy, and she's walked off
politely. She can still walk into the lab for the trial dose, because a bitten, unarmed player is always let through the
lab's gate for it.

**Colours cost friends too.** A Red Hand Trusted player wears the Bastion helmet and vest that his Bastion-friendly friend
Mara bought for him. At the next Red Hand roadblock, where he'd normally be waved through, he's turned round at 150 m: "We
don't sell passes to the army." He takes the vest off behind a barn, walks through, and puts it back on: a hassle, by
design. At a Bastion checkpoint he's searched as a Stranger, and the vest goes in the crate: "Army property, signed out to
Mara Novak. She'll be hearing from us."

**Infamy from far away.** Passing as Bastion in green, a player finishes a Red Hand job 3 km from any Bastion post. The
Bastion share of its Infamy is flagged SHARE, so it isn't doubled and nothing is blown. If he had set the job's charge
within 150 m of a Bastion patrol, that act's Infamy (ACT) would have doubled, and with the patrol 30 m away he'd have been
caught.

**A callback.** In Await Relief's Act IV, a Stranger with Bastion reads a post's roster at night in Bastion kit. He gives
the sentry the name "Corporal Dusek", which he heard off the nail at a different post. At the gate: full kit +2, dark +1,
name +1 = 4 of 2. At the duty board inside: 4 − 1 = 3 of 3. That sets `orders.disguised` (you). At the muster the CO says,
"And whoever signed the roster as Corporal Dusek: he's still dead." That player's persuade at the muster is one step
harder. If he'd used the name at Dusek's own post: "Dusek went on the nail on Day 12. We buried him outside."

**A card from the season's state.** A Bastion guard, Private Dusek, died to the infected on the Levie road this season day.
The Morton sentry deals The Name on the Tape to players Known with Bastion, and the card puts Dusek where he fell. One
player brings the tag and jacket back for 4 Favor and +1 Fame; the tag goes on the nail, and that night's call reads
"Dusek" with the numbers. On another server, a player gets there first and sells the jacket to the Red Hand's fence for 10
Favor. Morton's board lists "Dusek, kit lost".

**Vouching.** A Bastion Known player in her own green signs her Watched friend in at a checkpoint: no look, no search.
Twenty minutes later and 600 m down the road, the friend shoots a Bastion truck's driver. The voucher takes +5 Bastion
Infamy and can't sign anyone in for 2 hours. Without the vouch, the friend would have been searched like any Watched
civilian.

**Pointed out.** A Stranger in clean Bastion green claims at a checkpoint: 2 of 2. A player Trusted with Bastion is
standing at the barrier and picks "[Point them out] 'That one's not yours.'" The look becomes 1 of 2 before it's made,
and the Stranger's notice says so. He walks away instead of claiming. Had the pointer been wrong (his own kit), the pointer
would have taken regard *slander* with the sentry.

**No reset by logging out.** A player on a gate pass logs out with one look passed and 2 minutes left on the pass. She
rejoins 12 minutes later, still inside the wire. The pass has run out while she was away, and the look she passed is still
on record, so the sentry's look comes 10 s after she's back, at 1 of 2. She walks out in time, and the look stays owed for
30 minutes.

**Off the turned.** A player kills a helmeted infected from a military tribe and takes its jacket. Putting it on, the
notice says "Bastion green, burnt at the cuff. Off one of the turned." At a Bastion look: "That's off one of ours. Where
is he?" It's an instant fail, so the cheapest Bastion kit on the map is worthless for a claim and only good as armour. In
the last week, the same jacket carried to the lost post's roster is taken in: "He's home. Mark him."

**The Coat Run.** During Bastion's batch hold, an Independent-locked Lab Technician walks to the clean room at the 05:30
shift change in a clean coat, hood and respirator, with notebook 31's number. Her notice reads "[Full kit +2] [Lab
Technician +1] [Notebook 31 +1] [Dark +1] [Locked elsewhere −1] 4 of 3." Two Bastion-locked players are standing at the
clean room door and both call her out: 2 of 3. She doesn't go in. What a pass would have done is the finale's
([FINALE.md](FINALE.md#the-coat-run-independent-once-a-season)).

---

## Engine needs

- **VERIFY (phase 0, before any code):** read worn clothing from the character's loadout slots (the character inventory
  storage or loadout storage, and the cloth component's area type to tell head pieces from body pieces), not from child
  entities. The built `RZ_Util.GetWornNames` (`RZ_Util.c:845`) walks child entities, so it may count items carried in a
  backpack, and its 40-name cap makes the result depend on order. No fallback: if worn kit can't be read from the slots,
  Disguises stays off.
- **VERIFY:** removing a worn item from its slot through the inventory manager, then deleting it, for confiscation.
  `RZ_FoodHelper.TakeFood` covers storage, not worn slots. Fallback: the confiscation setting is off, so you're held,
  fined and blown, and nothing is taken.
- **VERIFY:** sprint state (`CharacterControllerComponent`'s sprint query) and weapon raised (the controller's
  weapon-raised state, or the weapon manager's current weapon). Fallback: the "running" and "firearm in your hands" tags
  are dropped. Position changes on the 5 s tick can't detect running.
- **VERIFY:** a sight trace (the world's `TraceMove` from one of their people's heads to the player's chest, excluding
  both), at most one per *again* look and per put-on check. Fallback: *again* looks only within 10 m, and the put-on check
  by distance (30 m) at both kit reads.
- **VERIFY:** an inventory event when a piece goes into a loadout slot (the storage manager's item-added invoker), for
  exact put-on timing. Fallback: the 10 s kit read, counted as seen only if one of their people was within 30 m at both
  the last read and this one.
- **VERIFY:** worn items stay on a corpse (as its loadout) long enough to be marked when retaliation sees a faction person
  die, and when the spawner counts the kill of an infected whose loadout matches a faction's keywords. Fallback: kill
  memory instead. If you killed one of their people this life within 2 km and in the last 2 hours of play, your first look
  near there fails.
- **VERIFY** (an existing per-player hostility item): how fast AI reacts when a player's bit flips. That matters for
  passes, for holds (the built `StandDown` for 25 s, without forgiving) and for catches (the built `AddGrudge`).
- **The light level:** the look's "dark" reads `RZ_Stealth.GetIllumination` (built, already marked VERIFY for the
  dedicated server), falling back to the clock and powered places (`IsPoweredAt`) when it returns −1. Fog comes from the
  world facts.
- A shared nearness query, `RZ_Hostility.NearestPeople(faction, pos, radius)`, over the units already tracked:
  retaliation's watched units, road post crews, holding guards and named people, leaders' bodies, and mission watchers
  registered with `RZ_Disguise.AddWatcher`.
- `RZ_GameModeComponent.AddInfamy(player, faction, amount, why, flags, deed)`: ACT doubles only while the player is passing
  within 150 m of that faction's people or holdings. Four callers pass SHARE: `AddFame`'s rival share,
  `RZ_MissionManager`'s rival Infamy on completion, `RZ_Questline.ChangeStanding` and `RZ_QuestApproach.SideWith`. Offline
  profiles never double.
- `RZ_QuartermasterHelper.MarkBought` gains the buyer's identity (signed-for and issued kit). Its sale memory already
  exists (`s_aBought`, weak references). The fence's sales of dead crewmen's kit are marked "bought fair" the same way.
- `RZ_PlayerProfile`: catches[4] and blown-until[4] in played seconds, saved with the life. Everything else is runtime,
  kept by identity for 30 minutes after a disconnect.
- **The look in `RZ_Checks`:** score, tags, cover details (the word at the wire with its wrong-word fail, names, crew names,
  notebook numbers, a mission's paper), instant fails, point-outs (at most 2).
- **Conversations:** claim lines are choices on the speakers in [DIALOGUE.md](DIALOGUE.md) (named people, the gate sentry,
  the post speaker at the barrier, a mission's people). Fallback: a "Pass as one of them" `ScriptedUserAction` on the
  barrier prop and on the occupant prefabs (`RZ_Occupant_Bastion.et`, `RZ_Occupant_RedHand.et`,
  `RZ_Occupant_Halcyon.et`), answered with an owner-only notice. AI crew bodies carry no actions, so they're reached
  through the barrier prop. At a post with no barrier prop, walking up in full kit is the claim.
- `RZ_UIFeed` pushes the coming look (score and difficulty) to its owner only when it changes. A new HUD widget,
  `RZ_Hud_Look`, using the Watched eye icon, is built in Workbench. The notices are the fallback.
- `RZ_EQuestApproach.TRICK`, appended once with PRESS, TRADE and LEAVE so saved runs keep their numbers.
  `RZ_QuestApproach.PassAs` works out the look score once per player per mission, like `TryPersuade`.
- `RZ_HealthCost` for the Red Hand's beating, using its own key per catch number so its once-per-life rule doesn't refuse a
  second beating, and keeping its floor.
- The nail list: a seeded name (`RZ_FactionVoice.PersonName`) for each Bastion guard, crew member or named person who
  dies, the last 30 per faction, saved, never the finale's canon names. Plus Halcyon's coat and notebook numbers. It feeds
  the lines, the board, the nightly call and the epilogue.
- World facts already cover night, dusk, powered places (`IsPoweredAt`), fog, town infection and the routine times (05:30,
  17:30, 18:30). The weapon keyword lists use the same weapons list as `RZ_Util.IsArmed` (low risk).
- `RZ_Checkpoints.CampContact`: a bitten, unarmed player who hasn't had the trial dose this life is let through the lab's
  gate whatever their standing short of Hunted, and whatever they wear.
- `RZ_Disguise`: one `IsFinalStand` check, so no faction bit is cleared within 1.5 km of the lab during the stand.
- For [FINALE.md](FINALE.md): a fight zone's spare rule that also spares route-locked players wearing that route's colours,
  alongside the built "spare Known" rule; and the Coat Run's look through `PassAs` at difficulty 3.

## Options decided

| Question | Decided |
|---|---|
| Showing the check | The tags and the result, "2 of 2", like New Vegas's [Speech 50] |
| Who can claim | Everyone below Known except the Hunted and Kill on Sight, who are seen through |
| Confiscation when you're held | Take the faction's kit pieces |
| Who pays for colours | Everyone, friends of the faction included (New Vegas's faction-armour rule) |
| Bastion's shop armour | Keeps Bastion's keywords, so you buy the colours with it |
| Severity | Blown for 60 minutes of play, 3 catches per faction per life |
| The Red Hand's toll for liars | 10 Favor or a beating |
| Passing at a Red Hand roadblock | Waved through like crew: a friend pays; a liar walks through, until he doesn't |
| The Rogue leader | Plays along for 20 Favor, and everyone who paid is named in that faction's records if he's found out |
| Vouching | On, with the one vouch's liability |
| Signed-for kit found at a Bastion search | Taken |
| A bonus for your own colours on persuade | None |
| After the 90% lock | −1 at every look with every faction whose route you didn't choose, following the identity, until the cure |
| The finale's trick | The Coat Run, re-labelling inside the lab ([FINALE.md](FINALE.md)) |
| Clean kit from the Red Hand | One set of Bastion green and one Halcyon set on the Trusted shelf, 40 Favor each, once per life |
| Halcyon's mask | The respirator is Halcyon's head piece and never a tell: the beaks always wear them outside the clean room |
| Naming players | Never on air. The Red Hand's channel laughs without a name; the epilogue's best-dressed line and "Never caught" are honour lines, through the names policy |
| Which cards to build first | Borrowed Green and Dress Rehearsal, then The Name on the Tape and Spot the Tells, then Paint, Masks Off, Notebook 31, Wrong Way Round, Civvies and Laundry Day |
| An impostor bounty ("Wanted: one civilian in Bastion green") | Not built: the griefing risk is too high |

## Risks

- **Phase 0** may show that worn kit can't be read reliably from the slots, or can't be told apart from carried items.
  Then Disguises stays off, and Bust Out runs only as a fight, as it does today without a uniform.
- **The sight trace** may cost too much or behave badly near buildings. The fallback, *again* looks only within 10 m,
  makes loitering inside a holding easier, though the 10-minute gate pass, roll call and leaders still limit it.
- **Hostility flips per player, not per AI.** A group that has already started firing may not stop when a hold starts,
  and a pass may lag a tick behind. This depends on the existing per-player hostility VERIFY. If it fails, the Outlaw
  fallback makes every catch a catch by everyone, which is too blunt.
- **Showing the check makes it predictable.** That's intended, as in New Vegas: the challenge is preparation (clean kit,
  a cover detail, a background, the night), and the caps keep it from ever paying. A play-test should confirm that
  players still find it worth trying, rather than seeing it as homework.
- **The word at the wire can be shared.** A group with a Radio Operator passes the word round. It's only +1, at a look
  that still reads the kit, the face and the night; a wrong word is a catch. Play-test whether posts feel toothless.
- **Kit supply decides how rare disguises are.** If the loot pool often drops uniform pieces that match Bastion's keywords
  (the US gear common on Arland and Everon), clean kit gets cheap. Keep those keywords rare in searches, and check the
  keyword lists against the loot pool and the zombie loadouts. The respirator keywords must not match ordinary dust masks
  if the loot pool has them.
- **Doubled questline Infamy** (Act IV: up to 15, plus 3 for the impersonation) can take a Watched player to Hunted in one
  catch. That's by design, but it needs a play-test so the step feels earned rather than sudden.
- **Vouching can be griefed:** a vouched friend gets caught on purpose to dump Infamy on the voucher. The cost is capped
  (+5 once per vouch, then 2 hours without vouching), and the voucher chooses whom to vouch for.
- **Point-outs can be griefed:** a rival points at every claimant. It's −1 at most twice, a wrong one costs the pointer
  regard, and it's once per pair per hour.
- **Runtime-only marks:** a restart forgets which jackets were dead men's or came off the turned, so a freshly looted one
  becomes clean after a restart. The nail list and the season's counts are saved, and catches and blown time survive
  with the life. This is a minor gap.
- **Readability:** there are many possible tags. The first-wear notice and the standing screen must carry the rule of
  thumb. A play-test should check whether a new player can predict a look before seeing the result.
- **Servers with strict stances** (Watched or Strangers) bring back cover at a distance without a claim. The
  opening-volley abuse there is no worse than what default stances already allow without any disguise, but it should be
  play-tested on such a server.
- **The trial-dose gate fix** changes camp behaviour for the bitten. Check that it can't be used to get into the lab for
  anything else: only the trial dose, with services still going by real standing.
- **Military tribes:** if Bacon Zombies' military corpses don't keep lootable clothing, the "off the turned" marks do
  nothing. That's harmless. If they do keep it and the marks fail (VERIFY), military tribes become a cheap source of
  disguises.
