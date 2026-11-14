# Persistence: a character who lasts

**Designed, not built.** Approved by the author on 2026-11-14. It changes what death means, so the
other designs in this folder are written for it ([README.md](README.md), the order of work).

The author: *"When you die you don't lose progress with quests, etc. But loot is lost. And you are
locked into the person you are, unless you reset character. You spawn at the nearest base or camp
with people you are Stranger or above with. Otherwise, random."*

Today a death starts a new character: a new background, reputation from the background's start,
every quest and errand gone, the opening again (DESIGN.md, "Backgrounds and personal goals";
REPUTATION.md, "Lives and carry-over"). After this design a **character** lasts until the player
resets it, and a death costs what you carried and a little time.

## What lasts and what doesn't

| Lasts across deaths (until you reset) | Lost at a death |
|---|---|
| Your name, background (locked), perk and flaw | Everything you carry, worn or held: it stays on your body |
| Fame, Infamy and Favor with every faction, and their tiers | An heirloom: it stays on your body ([HEIRLOOMS.md](HEIRLOOMS.md)) |
| The trust ladder, duties' rungs, deeds, "what people call you" | A bite and its infection (the body that died had it) |
| Questline and major participation, their stages and choices | A work site's hold or a carry under way |
| Errands and jobs held (their own timers keep running) | A duty run under way (ended, unpaid) |
| Leaders' and wanderers' memory of you, regard, lies told | |
| Habits ([HABITS.md](HABITS.md)), the locker ([LOCKER.md](LOCKER.md)) | |
| **Quest items** (below): they come round with you | |
| The journal, the tracker, Legacy, unlocks, your companion's loyalty ([COMPANION.md](COMPANION.md)) | |

The spawn screen no longer offers a background after your first: it shows the character, where
you'll come round and **Reset character** ([below](#reset-character)).

## Death

- **Your body** stays as vanilla's lootable corpse for **30 real minutes** (setting), or until
  emptied. Anyone can loot it, as today: the Zone has no rules about the dead.
- **Quest items come round with you.** A quest item is any carried item a running job needs: a
  mission's, an errand's, a duty's, a questline stage's or an opening step's. The Halcyon case, His
  Effects' bundle, a carried errand item, a key or papers a job gave you. Heirlooms are not quest
  items ([HEIRLOOMS.md](HEIRLOOMS.md)), and neither is anything you looted on the way.
  - **How:** at the death the server takes each quest item off the body and holds it as data
    (prefab, its job, its state such as a case's warmth); at the respawn it's put back in your
    inventory. The body never shows it, so nobody can loot it.
  - **Only while you're alive** can someone take a quest item from you (a robbery, a surrender, a
    toll that asks for it, a trade), as today.
  - **Warming pauses** while you're dead and coming round: the case's clock is held with it.
  - **A job that ends while you're dead** (its timer, the world moved on): its quest item goes when
    the job goes, and the death brief says so (*"The job's over. They took their case back."*).
- **Timed jobs** keep their timers through a death. A job that runs out while you're coming round
  fails as it does today.
- **An escort or a walking contact** waits 5 minutes where you fell, then goes home (data).

## Coming round

You come round at the **nearest camp that will take you in**, nearest to where you fell.

| Counts as a camp | Takes you in when |
|---|---|
| A holding (stronghold or outpost) | your tier with its faction is **Stranger or better** by access (Stranger, Uneasy, Known, Tolerated, Wildcard, Trusted, Respected, Champion: REPUTATION.md, "Standing tiers") |
| A Haven safehouse | the same with the Haven, and you're not exiled from that house (REPUTATION.md, "Haven house rules and exile") |
| Halcyon's lab | Known or better with Halcyon (it doesn't open its door to strangers) |
| A road post, the Long Table | never: a post isn't a camp, and the Long Table takes nobody in |

- **Not taken in:** you died bitten (*"Nobody carries in a bitten body"*), the camp is under
  attack or its site is in a stand, or it took you in three times in the last real hour (*"We're not
  your hospital."*). Then the next camp that will; with none, a **random start** by the openings'
  place rules (DESIGN.md, "Openings: how every life starts"), without the opening.
- **Where:** at the camp's sick bay or its fire (a hand-built site's sleep spot or medic's tent:
  SITES.md, "Role spots"), never in a wall, under the respawn timer as today.
- **The line:** someone there says how you got back: *"Found you on the road. You owe us."*

### What you come round with

Your background's **clothes**, a bandage and a water bottle. **No weapon, no ammunition, no
tools, no medical supplies** beyond the bandage. Loot is what you go out and get again; the clothes
are there so nobody walks out of a camp naked. The Nurse keeps his one dressing; the
Mechanic his wrench (a background's signature tool, never a weapon). The locker is where you keep
what you can't bear to lose ([LOCKER.md](LOCKER.md)).

### What a death costs

| Cost | How much | Why |
|---|---|---|
| Loot | everything carried, on the body (quest items come back with you) | the whole cost: coming round is free (the author, 2026-11-14) |

No other cost: no debuff, no Favor owed, no longer timer than vanilla's. No Fame or Infamy changes for dying.

## Reset character

**Reset character** is on the spawn screen and in the notebook's Standing page, behind two
confirms (*"This person is gone. Their name stays on the wall."*), with a pad as everywhere.

| A reset | |
|---|---|
| Starts | a new character: pick a background, the opening runs again |
| Resets | reputation, Favor, the trust ladder, quests and errands held, habits, the locker, the companion (they go home) |
| Keeps (by identity) | Legacy and its unlocks, "Word gets around", the season record and your titles (REPUTATION.md, "The season record"), the hall of fame |
| Carries as the name | with any faction your last character was **Hunted or worse** with, the new one starts **Watched**: they don't know the face, they know the name. A debt in the Red Hand's book stays in it |
| Limit | one reset a real day |

A reset is the only way a character ends. A death never does.

## The season and the wipe

**A wipe resets every character** (the Zone starts over: new outbreak, new people). Legacy, unlocks
and the hall of fame carry, as today. The epilogue's names on the wall are the characters that
ended this season (reset or wiped), and their deaths are a number on their line.

## What changes elsewhere

Today "a life" is a death-to-death span, numbered by `RZ_PlayerProfile.m_iLives`, and many systems
stamp things with it. After this design they stamp with a **character number** (`m_iCharacter`,
+1 at a reset or a wipe), and deaths are counted apart (`m_iDeaths`). A save from before loads its
current life as character 1.

| System (code) | Today | After |
|---|---|---|
| Profile (`Backgrounds/RZ_PlayerProfile.c`, `StartNewLife`) | resets background, reputation, meds, deeds at every spawn | splits in two: **at a death** only the bite, meds' once-a-bite flags, the life's numbers; **at a reset** the rest |
| Game mode (`Core/RZ_GameModeComponent.c`) | calls `StartNewLife`, `RZ_Shared.OnNewLife`, `RZ_MissionManager.OnNewLife` at each spawn | calls them at a reset only; a death calls a new `OnDeathRespawn` |
| Questline participation (`Director/RZ_Questlines.c`, `PlayerTookPartThisLife`; 18c87da) | per life | **per character** |
| Running jobs (`RZ_MissionManager.OnNewLife`; 18c87da) | a new life drops every job | at a reset only; a death keeps them |
| Quests tab, UI feed (`UI/RZ_Quests.c`, `UI/RZ_UIFeed.c`), the tracker | a new life is a new character | per character; the tracker keeps its job across a death |
| Errands (`RZ_Errand.m_iLife`, owed pay `RZ_ErrandsPay`, `m_iPerLife`) | belong to the life | belong to the character; `m_iPerLife` reads per character |
| Callbacks (`Core/RZ_Callbacks.c`) | belong to the life they were made in | to the character |
| Leaders' and wanderers' memory (`RZ_Leaders`, `RZ_Wanderers`, `RZ_Locals`, `RZ_DialogueMemory`) | life-stamped, half for earlier lives (REPUTATION.md, "Face and name") | character-stamped, same rule |
| Lies (`World/RZ_Lies.c`), Paranoid givers (`RZ_StoryKit`) | on the life they were told in | on the character |
| Disguise claims (`RZ_DisguiseClaims`), "met" (`RZ_DialogueTrees`) | per life | per character |
| Long Table (`World/RZ_LongTable.c`, `RZ_Quest_PlaceAtTable`) | memory and the offer belong to the life | to the character |
| Tribes' people and memory (`RZ_TribeData.m_iLife`, `RZ_TribePeople`, `RZ_TribeWays`) | a new life is a new character to them | per character: a tribe knows you across deaths |
| The season record (`RZ_RepNamesSeason`), gates' stamps (`RZ_RepNamesGates`), titles | by identity, best life | by identity, best character |
| Bastion's roster and rank (`RZ_MajorRest`), Bastion's "Counted" | per life | per character |
| Openings (`Guidance/RZ_Openings.c`) | every life, shorter with counted lives | **once a character**; a death never runs one |
| Guidance levels (`RZ_Guidance`, `m_iGuideLives`) | counted lives | characters and hours played (a death counts nothing) |
| Personal goals (`RZ_Objectives`) | one per life | one at a time per character; a new one when it's done |
| Legacy | personal goals, over all lives | the same, over all characters |
| Background locks, the spawn screen | a background each life | at a reset only |
| Word gets around, Remembered (the Haven Champion's kin) | at the next life | at the next character (a Haven Champion who resets) |
| Infection (`RZ_Medical`, meds once a life, trial dose once a life, `RZ_HealthCost`) | per life | the bite and meds per bite (a death clears it); the trial dose and health givers once **per character** |
| The Turned Player (DESIGN.md, "Infection") | a turned player roams with their gear | the same: the body that turned roams with what it carried; you come round at a random start (nobody carries in a bitten body) |
| Bounties (the Red Hand's board) | paid when you die; the next life is fresh | paid once a death; the same target can't be posted again for 30 real minutes |
| Epilogue (`RZ_Epilogue`, `RZ_EpilogueRecord`: names, Wanted, walked out; outlets' one dose a life) | per life | per character; one outlet dose per character |
| Quartermaster once-a-life items (`RZ_Quartermaster`) | once a life | once a character |
| The death brief (`UI/RZ_DeathBrief.c`), the HUD and radio log (`RZ_HudDisplay`, `RZ_PlayerUIComponent`) | "Life 4" | "Novak, background, death 4"; the radio log keeps across a death |
| Docs | UI.md "a new life is a new character", REPUTATION.md "Lives and carry-over", "Face and name", DESIGN.md backgrounds and openings | rewritten for characters when built |

## Exploits and balance

| Worry | Answer |
|---|---|
| Dying to get home fast | You lose everything you carry; with nothing on you it's a free trip, which the author accepts |
| Dying to save a quest item from a robbery | It can only be taken while you're alive anyway; dying doesn't change who has it |
| Dying to clear a bite | You lose your kit, and the bitten aren't carried in: a random start |
| Dying in a stand or the finale | You come round at a camp that takes you in, never inside the stand's area; the stand's own rules stand |
| PvP farming a bounty | Paid once a death, then 30 minutes before the same target can be posted |
| Resetting to dodge Infamy | The new character starts Watched where the old was Hunted or worse; the book's debts stay; one reset a day |
| A save from before this | Its current life becomes character 1; past lives' records stay as they are |

## Order of work

1. The character number and the death count; the profile split (death vs reset); a save migrates.
2. Coming round at a camp (the table above), the kit, the costs, the line.
3. Every system in "What changes elsewhere" moved to the character, with the runtime test's
   fresh-life checks rewritten as fresh-character and death checks.
4. Reset character on the spawn screen and the Standing page; the name carried; the wipe.
5. Docs rewritten (DESIGN, REPUTATION, UI, MISSION_RULES rule 24), this file trimmed.

## Settled with the author (2026-11-14)

- A wipe resets every character; Legacy carries.
- Coming round is free: losing what you carry is the whole cost.
