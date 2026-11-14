//! The Book (the Red Hand; R1 in docs/QUESTLINES.md, built as in docs/MISSIONS.md, "Questlines").
//! Somebody tore a page out of the book: the one with who outside the Red Hand owes it, and for
//! what. The boss wants it back, and the man who tore it out in the pit.
//!
//! 1. Who tore it (mission): three of the crew at the boss's camp, exactly one lying (as The Rat).
//!    ★ Police/Ex-Con: the night book clears one of them. Wrong: the wrong man goes in the pit
//!    (+5 Red Hand Infamy) and the real one runs; the story goes on either way.
//! 2. Find him (mission): his trail through two towns (a step chain) to a barn. Talk him out
//!    (unarmed; Known with the Red Hand, or ★ Ex-Con), take him (his two friends fight, with the
//!    Red Hand group prefab; else at gunpoint) or pay him to run (20 Red Hand Favor). He says why:
//!    his brother's house is on the page.
//! 3. Read the page (mission): one name per faction (a Bastion outpost commander, a Halcyon field
//!    lead, a Haven council head: Rogue ones first, else Independent, else any), each met at
//!    their own seat. Collect (talk 60%, lean on them, or pay it yourself), let it lie, or turn
//!    one in at their own stronghold. A name whose leader is out of the chair drops off the page.
//! 4. The page (mission): give it back to the boss (Trusted; Known with B1's orders.mocked), keep it
//!    or burn it at the barn's fire, or hand it to Bastion's commanding officer.
//! 5. The thief (mission, skipped if he ran): at the pit. Let it happen, take his place (four rounds
//!    in the ring), pay his debt (30 Red Hand Favor) or get him out to a safehouse (seen near the
//!    pit: +10 Red Hand Infamy).
//! 6. The boss hears (wait, only after the page was kept): a session.
//!
//! Endings: The book balances, Held over them, Bastion's file, Burnt.
class RZ_Quest_TheBook : RZ_Questline
{
	static const int S_WHO = 0;
	static const int S_FIND = 1;
	static const int S_READ = 2;
	static const int S_PAGE = 3;
	static const int S_THIEF = 4;
	static const int S_HEARS = 5;

	static const int OUT_NAMED = 10;		// stage 1: the right man
	static const int OUT_WRONG = 11;		// stage 1: the wrong man went in the pit
	static const int OUT_CAUGHT = 12;		// stage 2: brought in
	static const int OUT_RAN = 13;			// stage 2: paid to run
	static const int OUT_READ = 14;			// stage 3: the names seen to
	static const int OUT_RETURNED = 15;		// stage 4
	static const int OUT_KEPT = 16;
	static const int OUT_BURNT = 17;
	static const int OUT_BASTION = 18;
	static const int OUT_PIT = 19;			// stage 5: he went in
	static const int OUT_PLACE = 20;		// someone took his place
	static const int OUT_PAID = 21;			// his debt was paid
	static const int OUT_FREED = 22;		// walked to a safehouse

	static const int E_BALANCES = 0;
	static const int E_HELD = 1;
	static const int E_BASTION = 2;
	static const int E_BURNT = 3;

	// A name on the page (flag d<i>_state)
	static const int D_OPEN = 0;
	static const int D_TALKED = 1;			// collected, by talking
	static const int D_SQUEEZED = 2;		// collected, by leaning on them
	static const int D_PAID = 3;			// paid off by a player
	static const int D_LEFT = 4;			// let lie
	static const int D_PENDING = 5;			// on its way to their own stronghold
	static const int D_TURNED = 6;			// turned in
	static const int D_GONE = 7;			// out of the chair: the debt was theirs, and it went with them

	// The thief's fate (world flag book.thief)
	static const int T_PIT = 1;
	static const int T_PLACE = 2;
	static const int T_PAID = 3;
	static const int T_FREED = 4;
	static const int T_RAN = 5;

	static const int MAX_NAMES = 3;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info("book", "The Book", RZ_EFaction.BANDITS);

		RZ_QuestStage who = MissionStage(S_WHO, "Who tore it", "Somebody tore a page out of the Red Hand's book: the one with who outside the Red Hand owes it, and for what. {boss} wants it back, and the man who tore it out. Three of the crew were on the night it went. One of them is lying.");
		who.GiveUp(4, "Nobody found out who tore the page out of the book. {boss} has stopped asking, which is worse.");
		who.Visits(2);

		RZ_QuestStage find = MissionStage(S_FIND, "Find {thief_short}", "{thief} tore the page out and ran. Follow his trail, and bring him back with the page.");
		find.GiveUp(4, "{thief} got away with the page. Somewhere out there, people the Red Hand owns are sleeping better.");
		find.Visits(2);

		RZ_QuestStage read = MissionStage(S_READ, "Read the page", "The page is short: a few names outside the Red Hand, and what they owe. Go round them. Collect, let it lie, or turn one of them in to their own people. Nobody rats, they say.");
		read.GiveUp(4, "Nobody went round the names on the page. It's still folded in somebody's coat.");
		read.Visits(2);

		RZ_QuestStage page = MissionStage(S_PAGE, "The page", "Now the page itself. Back in the book at {boss_place}, into your pocket, into the fire at the barn where {thief_short} hid, or onto the desk of Bastion's commanding officer.");
		page.GiveUp(4, "The page stayed folded in somebody's coat, and {boss} stopped waiting for it.");
		page.Visits(2);

		RZ_QuestStage thief = MissionStage(S_THIEF, "The thief", "{boss} wants {thief} in the pit at {pit}. Let it happen, take his place, pay what he owes, or get him out.");
		thief.GiveUp(8, "Nobody came for {thief}, and the pit didn't wait. The page is settled. He isn't.");

		RZ_QuestStage hears = WaitStage(S_HEARS, "The boss hears", "The page is in someone's pocket, and the people on it know. Somebody always talks.");
		hears.Wait(120);
	}

	//------------------------------------------------------------------------------------------------
	//! The boss, one name per other faction (two at least), the thief and his brother. From the
	//! second real day of the season.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || !gm.GetLeaders())
			return false;

		if (gm.GetSeasons() && gm.GetSeasons().GetRealDays() == 0)
			return false;

		RZ_Leader boss = gm.GetLeaders().GetStronghold(RZ_EFaction.BANDITS);
		if (!boss || !boss.IsPresent())
			return false;

		array<int> factions = {RZ_EFaction.MILITARY, RZ_EFaction.INSTITUTE, RZ_EFaction.SURVIVORS};
		int count = 0;
		foreach (int f : factions)
		{
			RZ_Leader d = RZ_RedHandQuest.PickLeader(gm, f, 0);
			if (!d)
				continue;

			string key = Key(count);
			RZ_RedHandQuest.Keep(run, key, d);
			run.SetFlag(key + "_faction", f);
			run.SetFlag(key + "_state", D_OPEN);
			run.SetText(key + "_what", DebtText(f));
			if (f == RZ_EFaction.SURVIVORS)
				run.SetFlag("haven", count);

			count++;
		}

		if (count < 2)
			return false;

		run.SetFlag("names", count);
		RZ_RedHandQuest.Keep(run, "boss", boss);
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		array<string> names = {};
		RZ_RedHandQuest.MenNames(rng, 3, names);
		if (names.Count() < 3)
			return false;

		run.SetText("thief", names[0] + " " + RZ_NoteText.Surname(rng));
		run.SetText("thief_short", names[0]);
		run.SetText("inno1", names[1]);
		run.SetText("inno2", names[2]);
		run.SetText("sister", RZ_NoteText.ManName(rng));
		run.SetFlag("turned", -1);
		CountCutDose(gm, run);
		run.SetPlace(boss.m_vPos, boss.m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! H3's cut dose (walk.cut_dose): the Haven owes for it too, on the Haven name's line (once).
	static void CountCutDose(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int haven = run.Flag("haven", -1);
		if (haven < 0 || run.Flag("cut_dose") == 1 || !RZ_RedHandQuest.HasWorld(gm, "walk.cut_dose"))
			return;

		string key = Key(haven);
		run.SetFlag("cut_dose", 1);
		run.SetText(key + "_what", run.Text(key + "_what") + ", and the Haven's bill for the cut dose");
	}

	//------------------------------------------------------------------------------------------------
	static string Key(int i)
	{
		return "d" + i.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! What a faction's leader owes the Red Hand, for the page.
	static string DebtText(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "eight rifles: twelve paid for in diesel, four delivered";
			case RZ_EFaction.INSTITUTE: return "two live ones, left at the back door of a field station and never paid for";
			case RZ_EFaction.SURVIVORS: return "three months of the house's quiet arrangement";
		}

		return "something they'd rather not say out loud";
	}

	//------------------------------------------------------------------------------------------------
	static int State(RZ_QuestRun run, int i)
	{
		return run.Flag(Key(i) + "_state", D_GONE);
	}

	//------------------------------------------------------------------------------------------------
	static void SetState(RZ_QuestRun run, int i, int state)
	{
		run.SetFlag(Key(i) + "_state", state);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsCollected(int state)
	{
		return state == D_TALKED || state == D_SQUEEZED;
	}

	//------------------------------------------------------------------------------------------------
	//! A name still open (or on its way to their stronghold) whose leader is out of the chair drops
	//! off the page: a debt is a debt, but it was theirs. Returns how many names are still open.
	static int UpdateNames(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int open = 0;
		for (int i = 0; i < run.Flag("names"); i++)
		{
			int state = State(run, i);
			if (state != D_OPEN && state != D_PENDING)
				continue;

			string key = Key(i);
			if (RZ_RedHandQuest.Same(gm, run, key))
			{
				open++;
				continue;
			}

			SetState(run, i, D_GONE);
			run.Note(string.Format("%1 isn't in the chair in %2 any more. Whatever they owed was theirs, and it went with them.", run.Text(key), run.Text(key + "_place")));
		}

		return open;
	}

	//------------------------------------------------------------------------------------------------
	//! "Major Weber in Morton: eight rifles (collected)" lines for the journal.
	static string PageLines(RZ_QuestRun run)
	{
		string text = "";
		for (int i = 0; i < run.Flag("names"); i++)
		{
			string key = Key(i);
			text += string.Format("\n%1 in %2: %3 (%4).", run.Text(key), run.Text(key + "_place"), run.Text(key + "_what"), StateText(State(run, i)));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	static string StateText(int state)
	{
		switch (state)
		{
			case D_OPEN: return "open";
			case D_TALKED: return "collected";
			case D_SQUEEZED: return "collected, the hard way";
			case D_PAID: return "paid, by someone else";
			case D_LEFT: return "let lie";
			case D_PENDING: return "on its way to their own people";
			case D_TURNED: return "turned in";
		}

		return "off the page";
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, "The Red Hand's book came up a page short this week. The crews are checking each other's pockets.", false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_WHO:
			{
				run.Note("A page is missing from the Red Hand's book: the one about people outside it. {boss} keeps the book at {boss_place}.");
				break;
			}
			case S_FIND:
			{
				PickTrail(gm, run);
				break;
			}
			case S_READ:
			{
				CountCutDose(gm, run);
				UpdateNames(gm, run);
				break;
			}
			case S_PAGE:
			{
				RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
				if (boss)
					run.SetPlace(boss.m_vPos, boss.m_sPlace);

				break;
			}
			case S_THIEF:
			{
				PickPit(gm, run);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Two towns for his trail, from the boss's camp, and a barn outside the second.
	protected bool PickTrail(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.HasFlag("t2"))
			return true;

		RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
		if (!boss)
			return false;

		array<int> towns = {};
		if (!RZ_StepChain.PickTowns(gm, boss.m_vPos, 2, 600, 2500, 0, 95, towns))
			return false;

		RZ_Settlement first = gm.GetSettlement(towns[0]);
		RZ_Settlement second = gm.GetSettlement(towns[1]);
		run.SetFlag("t1", towns[0]);
		run.SetFlag("t2", towns[1]);
		run.SetText("town1", first.m_sName);
		run.SetText("town2", second.m_sName);
		vector barn = RZ_Mission.LandPointNear(second.m_vCenter, second.m_fRadius + 150, second.m_fRadius + 400);
		RZ_RedHandQuest.KeepPos(run, "barn", barn);
		run.SetText("barn_grid", RZ_SettlementScanner.GridReference(barn));
		run.SetPlace(first.m_vCenter, first.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The boss's own pit, behind the garages at his camp (a stronghold never falls).
	protected bool PickPit(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (RZ_RedHandQuest.HasPos(run, "pit"))
			return true;

		RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
		if (!boss)
			return false;

		vector pit = RZ_Mission.LandPointNear(boss.m_vPos, 60, 150);
		string pitName = string.Format("the garages behind %1's camp in %2", boss.m_sShort, boss.m_sPlace);
		RZ_RedHandQuest.KeepPos(run, "pit", pit);
		run.SetText("pit", pitName);
		run.SetPlace(pit, pitName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_FIND)
			return PickTrail(gm, run);

		if (stage == S_THIEF)
			return PickPit(gm, run);

		if (stage == S_READ)
			UpdateNames(gm, run);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_WHO: return new RZ_QM_BookWho();
			case S_FIND: return new RZ_QM_BookFind();
			case S_READ: return new RZ_QM_BookRead();
			case S_PAGE: return new RZ_QM_BookPage();
			case S_THIEF: return new RZ_QM_BookThief();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_WHO:
			{
				if (outcome == OUT_NAMED)
				{
					run.Note("It was {thief}. By the time anyone looked round for him, he was over the fence with the page.");
					PickTrail(gm, run);
					return S_FIND;
				}

				if (outcome == OUT_WRONG)
				{
					run.Note("{wrong} went in the pit for it, and it wasn't him. That night {thief} went over the fence with the page.");
					PickTrail(gm, run);
					return S_FIND;
				}

				run.Note("Nobody pointed at anyone. {boss} will ask again, less politely.");
				return S_WHO;
			}
			case S_FIND:
			{
				if (outcome != OUT_CAUGHT && outcome != OUT_RAN)
				{
					run.Note("{thief}'s trail went cold. Somebody will have to pick it up again.");
					return S_FIND;
				}

				if (outcome == OUT_RAN)
				{
					run.SetFlag("thief_ran", 1);
					RZ_RedHandQuest.SetWorld(gm, "book.thief", T_RAN);
					run.Note("{thief} gave up the page and ran, with somebody's Favor in his pocket.");
				}
				else
				{
					run.Note("{thief} is back at {boss_place} with his hands tied, and the page is out of his boot.");
				}

				int open = UpdateNames(gm, run);
				run.Note(SisterLine(gm, run));
				if (open == 0)
				{
					run.Note("Every name on the page is out of the chair already. Only the page is left.");
					return S_PAGE;
				}

				return S_READ;
			}
			case S_READ:
			{
				if (outcome != OUT_READ && !AnyHandled(run))
				{
					run.Note("Nobody went round the names. The page is still folded in somebody's coat.");
					return S_READ;
				}

				for (int i = 0; i < run.Flag("names"); i++)
				{
					int state = State(run, i);
					if (state == D_OPEN || state == D_PENDING)
						SetState(run, i, D_LEFT);
				}

				run.Note("The names are seen to, one way or another:" + PageLines(run));
				return S_PAGE;
			}
			case S_PAGE:
				return AfterPage(gm, run, outcome);
			case S_THIEF:
			{
				int fate = T_PIT;
				string line = "{thief} went in the pit at {pit}. He lasted two rounds.";
				if (outcome == OUT_PLACE)
				{
					fate = T_PLACE;
					line = "Somebody took {thief}'s place in the pit and walked out of it. {boss} let him go, because a crowd is a crowd.";
				}
				else if (outcome == OUT_PAID)
				{
					fate = T_PAID;
					line = "Somebody paid {thief}'s debt out of their own pocket. His name is crossed out, and he's still breathing.";
				}
				else if (outcome == OUT_FREED)
				{
					fate = T_FREED;
					line = "{thief} was walked out of the pit and into the Haven at {refuge}.";
				}

				run.SetFlag("thief", fate);
				RZ_RedHandQuest.SetWorld(gm, "book.thief", fate);
				run.Note(line);
				if (run.m_iBranch == OUT_KEPT)
					return S_HEARS;

				return Ending(run);
			}
			case S_HEARS:
				return Finish(run, E_HELD, "Held over them. The page is still in a pocket, and the people on it pay to keep it there. The boss has heard whose pocket.");
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterPage(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome != OUT_RETURNED && outcome != OUT_KEPT && outcome != OUT_BURNT && outcome != OUT_BASTION)
		{
			run.Note("Nobody did anything with the page. It's still in somebody's coat.");
			return S_PAGE;
		}

		run.SetBranch(outcome);
		if (outcome == OUT_RETURNED)
			run.Note("The page went back into the book at {boss_place}, and the book balances.");
		else if (outcome == OUT_KEPT)
			run.Note("The page is in a pocket now, and the people on it are paying to keep it there.");
		else if (outcome == OUT_BURNT)
			run.Note("The page went in the fire at {town2}. The debts on it went with it.");
		else
			run.Note("The whole page is on the desk of Bastion's commanding officer.");

		if (run.Flag("thief_ran") == 1 || !PickPit(gm, run))
		{
			if (outcome == OUT_KEPT)
				return S_HEARS;

			return Ending(run);
		}

		return S_THIEF;
	}

	//------------------------------------------------------------------------------------------------
	protected int Ending(RZ_QuestRun run)
	{
		if (run.m_iBranch == OUT_RETURNED)
			return Finish(run, E_BALANCES, "The book balances. The page is back where it belongs, and {boss} knows who brought it.");

		if (run.m_iBranch == OUT_BASTION)
			return Finish(run, E_BASTION, "Bastion's file. The Red Hand's page is on the commanding officer's desk, and the Red Hand knows who put it there.");

		return Finish(run, E_BURNT, "Burnt. The page is ash, and the debts on it with it. The Red Hand says a debt is a debt, page or no page.");
	}

	//------------------------------------------------------------------------------------------------
	static bool AnyHandled(RZ_QuestRun run)
	{
		for (int i = 0; i < run.Flag("names"); i++)
		{
			int state = State(run, i);
			if (state != D_OPEN && state != D_GONE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Why he tore it out: his brother's house is on the page (said as it stands now: a council head
	//! who has left the chair since is spoken of in the past).
	static string SisterLine(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!run)
			return "";

		int haven = run.Flag("haven", -1);
		if (haven >= 0)
		{
			string key = Key(haven);
			if (!Name(gm, run, haven))
				return string.Format("He says why. His brother {sister} lives at the Haven house in %1, and its council head was on the page. He wanted one house in the Zone to owe nobody.", run.Text(key + "_place"));

			return string.Format("He says why. His brother {sister} lives at the Haven house in %1, and %2, its council head, is on the page. He wanted one house in the Zone to owe nobody.", run.Text(key + "_place"), run.Text(key));
		}

		return "He says why. His brother {sister} owes too, through somebody on the page, and he wanted one family in the Zone to owe nobody.";
	}

	//------------------------------------------------------------------------------------------------
	//! A name on the page whose leader is still the same person, or null.
	static RZ_Leader Name(RZ_GameModeComponent gm, RZ_QuestRun run, int i)
	{
		return RZ_RedHandQuest.Same(gm, run, Key(i));
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		int count = run.Flag("names");
		bool thiefLived = run.Flag("thief") == T_PLACE || run.Flag("thief") == T_PAID || run.Flag("thief") == T_FREED || run.Flag("thief_ran") == 1;
		if (ending == E_BALANCES)
		{
			NudgeMorale(gm, RZ_EFaction.BANDITS, 5);
			ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0);

			// Every leader somebody collected from remembers who
			for (int i = 0; i < count; i++)
			{
				RZ_Leader l = Name(gm, run, i);
				if (l && IsCollected(State(run, i)))
					RZ_RedHandQuest.Remembers(l, run.Text(Key(i) + "_by"), RZ_Leaders.MEM_CROSSED);
			}

			Headline(gm, run.Fill("The Red Hand's book balances again. The crews have started calling {returner} \"{crewname}\"."));
			SpreadRumour(gm, run.Fill("The Red Hand's got its page back. Ask for {crewname} if you want to know how."), 8);
			return;
		}

		if (ending == E_HELD)
		{
			// The boss has heard; the one squeezed hardest has stopped answering anyone's calls
			ChangeStanding(gm, run, RZ_EFaction.BANDITS, 0, 15, RZ_EQuestApproach.BETRAY);

			// The one leaned on, else the first one collected from
			int hardest = run.Flag("squeezed", -1);
			for (int k = 0; k < count && hardest < 0; k++)
			{
				if (IsCollected(State(run, k)))
					hardest = k;
			}

			RZ_Leader squeezed = Name(gm, run, hardest);
			if (squeezed)
				RZ_RedHandQuest.SwingLoyalty(gm, squeezed, -RZ_RedHandQuest.SWING);

			Headline(gm, "Somebody outside the Red Hand is holding a page of its book, the crews say. Nobody will say whose names are on it.", false);
			SpreadRumour(gm, "The boss is asking who's got the page. Nobody's answering, which is an answer.", 6);
			return;
		}

		if (ending == E_BASTION)
		{
			NudgeStrength(gm, RZ_EFaction.BANDITS, -3);
			ChangeStanding(gm, run, RZ_EFaction.BANDITS, 0, 10, RZ_EQuestApproach.SIDE);
			RZ_Leader commander = Name(gm, run, BastionName(run));
			if (commander)
			{
				RZ_RedHandQuest.SwingLoyalty(gm, commander, RZ_RedHandQuest.SWING);
				Headline(gm, string.Format("A page of the Red Hand's book is on Bastion's commanding officer's desk. %1 in %2 has started saluting again.", commander.m_sName, commander.m_sPlace));
			}
			else
			{
				Headline(gm, "A page of the Red Hand's book is on Bastion's commanding officer's desk, and it's being read slowly.");
			}

			SpreadRumour(gm, "Bastion's got a page of the Red Hand's book. The Red Hand's got a list of who gave it to them.", 8);
			return;
		}

		// Burnt
		NudgeMorale(gm, RZ_EFaction.BANDITS, -5);
		ChangeStanding(gm, run, RZ_EFaction.BANDITS, 0, 10, RZ_EQuestApproach.BETRAY);
		if (thiefLived)
			ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 5, 0);

		for (int j = 0; j < count; j++)
		{
			RZ_Leader saved = Name(gm, run, j);
			if (saved)
				RZ_RedHandQuest.RemembersAll(saved, run, RZ_Leaders.MEM_SAVED);
		}

		Headline(gm, "Somebody burned a page of the Red Hand's book. The boss says a debt is a debt, page or no page.");
		SpreadRumour(gm, "A page of the Red Hand's book went in a fire. Three people slept properly that night, and none of them were Red Hand.", 8);
	}

	//------------------------------------------------------------------------------------------------
	//! The index of the Bastion name on the page, -1 if none.
	static int BastionName(RZ_QuestRun run)
	{
		for (int i = 0; i < run.Flag("names"); i++)
		{
			if (run.Flag(Key(i) + "_faction", -1) == RZ_EFaction.MILITARY)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		string text = super.JournalText(gm, run);
		if (run.m_iStage == S_FIND && run.HasFlag("t2"))
			text += run.Fill(" He was seen in {town1}, heading for {town2}.");

		if (run.m_iStage == S_READ || run.m_iStage == S_PAGE)
		{
			UpdateNames(gm, run);
			text += run.Fill("\nThe page:" + PageLines(run));
		}

		if (run.m_iStage == S_READ && RZ_RedHandQuest.HasWorld(gm, "door.debt_yours"))
			text += "\nAnd a fourth line in newer ink: a safehouse's debt at the door, taken on by a stranger who said it could be theirs.";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage >= S_FIND && run.m_iStage <= S_PAGE)
			return "Somebody tore a page out of the Red Hand's book. Half the crews are checking their pockets, and the other half are checking their friends'.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The people on the page get nervous while it's loose; Red Hand leaders talk about it.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (!l)
			return "";

		if (run.m_iStage >= S_READ && run.m_iStage <= S_PAGE)
		{
			for (int i = 0; i < run.Flag("names"); i++)
			{
				RZ_Leader d = Name(gm, run, i);
				if (d && d.m_iId == leaderId && run.PlayerTookPart(gm, playerId))
					return string.Format("%1 looks at your coat pocket, then at you. \"If you've got something with my name on it, I'd like it back. I can be grateful.\"", l.m_sShort);
			}
		}

		if (l.m_eFaction == RZ_EFaction.BANDITS && run.m_iStage <= S_FIND)
			return string.Format("%1 taps the side of their nose. \"The boss's book is a page short. Whoever's got it had better have a good reason, or a long way to run.\"", l.m_sShort);

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! The Book, stage 1 (Red Hand contract, Known): three of the crew at the boss's camp, exactly
//! one lying, as in The Rat (the statements are checked by brute force so exactly one fits).
//! The liar is the thief. ★ Police/Ex-Con: the night book clears one of the other two. Only
//! someone the Red Hand knows may point; wrong: +5 Red Hand Infamy and the real one runs.
class RZ_QM_BookWho : RZ_QuestMission
{
	protected int m_iRat = -1;
	protected int m_iCleared = -1;
	protected int m_iKeeperClue = -1;
	protected ref array<string> m_aNames = {};
	protected ref array<int> m_aClues = {};

	protected static const int STMT_ACCUSE = 0;		// "X did it"
	protected static const int STMT_CLEAR = 1;		// "X didn't"
	protected static const int STMT_SELF = 2;		// "I didn't"
	protected static const int WRONG_INFAMY = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 12, 15);
		m_Req.Bonus("police");
		m_Req.Bonus("excon");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsTrue(int type, int speaker, int target, int liar)
	{
		if (type == STMT_ACCUSE)
			return target == liar;

		if (type == STMT_CLEAR)
			return target != liar;

		return speaker != liar;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader boss = RZ_RedHandQuest.Same(gm, run, "boss");
		if (!boss)
			return false;

		m_vPos = LandPointNear(boss.m_vPos, 15, 30);
		m_iSettlement = gm.FindNearestSettlement(boss.m_vPos);

		// Statements until exactly one of the three can be the only liar
		array<int> types = {0, 0, 0};
		array<int> targets = {0, 0, 0};
		bool found = false;
		for (int attempt = 0; attempt < 300 && !found; attempt++)
		{
			for (int i = 0; i < 3; i++)
			{
				types[i] = m_Rng.Int(0, 3);
				targets[i] = (i + m_Rng.Int(1, 3)) % 3;
			}

			int fits = 0;
			int fitLiar = -1;
			for (int c = 0; c < 3; c++)
			{
				bool consistent = true;
				for (int k = 0; k < 3; k++)
				{
					if (IsTrue(types[k], k, targets[k], c) != (k != c))
						consistent = false;
				}

				if (consistent)
				{
					fits++;
					fitLiar = c;
				}
			}

			if (fits == 1)
			{
				found = true;
				m_iRat = fitLiar;
			}
		}

		if (!found)
			return false;

		// The liar is the thief; the other two are innocent
		array<string> innocents = {};
		innocents.Insert(QText(gm, "inno1"));
		innocents.Insert(QText(gm, "inno2"));
		int next = 0;
		for (int n = 0; n < 3; n++)
		{
			if (n == m_iRat)
			{
				m_aNames.Insert(QText(gm, "thief_short"));
				continue;
			}

			m_aNames.Insert(innocents[next]);
			next++;
		}

		m_iCleared = (m_iRat + 1) % 3;
		m_iKeeperClue = AddClue("The book-keeper", QFill(gm, "The book lies open on a crate, with a ragged edge where a page used to be. \"Three of them were on the night it went. One of them's lying to my face. Only one. Hear them out, then point, and {boss} will do the rest.\""));
		for (int j = 0; j < 3; j++)
		{
			m_aClues.Insert(AddClue(m_aNames[j], Statement(types[j], j, targets[j])));
		}

		m_sTitle = "The Book: who tore it";
		m_sBrief = QFill(gm, "Somebody tore a page out of my book. Not just any page: the one with who outside the Red Hand owes us, and for what. Three of the crew were on the night it went. Come to {boss_place}, hear them out and point. I'll do the rest.");
		m_sObjective = QFill(gm, "Find out which of the three at {boss_place} tore the page from the book.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string Statement(int type, int speaker, int target)
	{
		string other = m_aNames[target];
		if (type == STMT_ACCUSE)
			return string.Format("It was %1. Ask %1 why he kept going back to the book-keeper's tent. Go on, ask.", other);

		if (type == STMT_CLEAR)
			return string.Format("%1 was on the gate with me all night, moaning about his feet. It wasn't %1.", other);

		return "I can't even read the book. Why would I want a page of it? It's not me.";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string choices = string.Format("Point at %1|Point at %2|Point at %3", m_aNames[0], m_aNames[1], m_aNames[2]);
		if (!SpawnPerson(gm, m_vPos, 0, m_iKeeperClue, "Talk to the book-keeper", choices))
			return false;

		for (int j = 0; j < 3; j++)
		{
			SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, RZ_Util.RandomPointInRing(m_vPos, 4, 8), m_aClues[j], "Listen to " + m_aNames[j]);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! ★ Police/Ex-Con: the night book clears one of the innocent two.
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iKeeperClue && m_iCleared >= 0 && HasBonus(gm, playerId))
			text += string.Format("\nYou know how a night book is kept. %1 signed on at the gate at dusk and off at dawn, and two others signed beside him. It wasn't %1.", m_aNames[m_iCleared]);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || option < 0 || option > 2)
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The book-keeper doesn't look up. \"The boss asked for people we know.\"");
			return;
		}

		if (option == m_iRat)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 goes white, and then he goes for the fence. He's over it before anyone moves. The page went with him.", m_aNames[m_iRat]));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheBook.OUT_NAMED, "pointed at the thief");
			return;
		}

		RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.BANDITS, 0, WRONG_INFAMY);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetText("wrong", m_aNames[option]);

		RZ_RedHandQuest.SetWorld(gm, "book.wrong_man", 1);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("They take %1 to the pit that night. In the morning the book-keeper finds you. \"It wasn't him. %2 went over the fence while we were all watching %1. That's on you.\" (+%3 Red Hand Infamy)", m_aNames[option], m_aNames[m_iRat], WRONG_INFAMY));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheBook.OUT_WRONG, "pointed at the wrong man", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Book: nobody pointed at anyone. The page is still missing.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Book, stage 2 (Red Hand contract, Known): his trail through two towns (RZ_StepChain), then
//! the barn outside the second. At the barn: talk him out (unarmed, persuade 1 with the Red Hand,
//! ★ Ex-Con +1), take him (his two friends, an AI group with the Red Hand group prefab, fight;
//! without it, at gunpoint) or pay him to run (20 Red Hand Favor, half pay).
class RZ_QM_BookFind : RZ_QuestMission
{
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected ref RZ_UnitWatch m_Friends = new RZ_UnitWatch();
	protected vector m_vBarn;
	protected bool m_bBarnUp;
	protected bool m_bFight;
	protected int m_iTaker = -1;
	protected int m_iThiefTalk = -1;

	protected static const int RUN_FAVOR = 20;
	protected static const float RUN_SCALE = 0.5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 15, 20);
		m_Req.Bonus("excon");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Chain.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		RZ_Settlement first = gm.GetSettlement(run.Flag("t1", -1));
		RZ_Settlement second = gm.GetSettlement(run.Flag("t2", -1));
		if (!first || !second || !RZ_RedHandQuest.HasPos(run, "barn"))
			return false;

		m_vBarn = RZ_RedHandQuest.PosOf(run, "barn");
		m_Chain.SetMission(m_iId);
		RZ_ChainStep jacket = m_Chain.AddStep(gm, run.Flag("t1"), LandPointNear(first.m_vCenter, first.m_fRadius * 0.5, first.m_fRadius + 60), 6, 0);
		jacket.m_iClue = AddClue("A jacket on a fence", QFill(gm, "A Red Hand jacket hung on a fence at the edge of {town1}, the hand on the back painted over in brown. In the pocket, a bus ticket from before, and on the back of it in pencil: \"{town2}. Don't tell {sister}.\""));
		jacket.m_sReadLabel = "Go through the jacket";

		RZ_ChainStep fire = m_Chain.AddStep(gm, run.Flag("t2"), LandPointNear(second.m_vCenter, second.m_fRadius * 0.4, second.m_fRadius), 6, 0);
		fire.m_iClue = AddClue("A cold fire", QFill(gm, "Somebody slept here and left before it was light. Two sets of big boots and one smaller pair that paced all night. The prints head out of {town2} toward a barn at grid {barn_grid}."));
		fire.m_sReadLabel = "Look round the fire";
		fire.m_iGuards = 2;

		m_iThiefTalk = AddClue(QText(gm, "thief"), QFill(gm, "{thief_short} has the page down his boot and his back to the wall. \"Did {boss} send you? Course they did. Go on, then. Do what you came for.\""));
		m_vPos = jacket.m_vPos;
		m_iSettlement = run.Flag("t1");
		m_sTitle = QFill(gm, "The Book: find {thief_short}");
		m_sBrief = QFill(gm, "{thief_short} tore my page out and ran. He was seen in {town1}. Follow him. Bring him back with the page, and I don't much mind how.");
		m_sObjective = QFill(gm, "Follow {thief_short} from {town1} and bring {thief_short} back with the page.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
		if (idx == m_iThiefTalk)
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "found him in the barn");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Chain.IsDone())
		{
			int done = m_Chain.Update(gm, dt, ids, players, m_aParticipants);
			RZ_ChainStep next = m_Chain.GetCurrent();
			if (next)
				m_vPos = next.m_vPos;
			else
				m_vPos = m_vBarn;

			if (done >= 0)
				NotifyParticipants(gm, m_sTitle, "The trail goes on.");

			return;
		}

		m_vPos = m_vBarn;
		if (!m_bBarnUp && RZ_ContractHelp.AnyNear(players, m_vBarn, 300))
			ShowBarn(gm);

		if (m_bResolved || !m_Friends.IsDead(dt))
			return;

		// His friends are down: he gives up to whoever is closest
		int taker = m_iTaker;
		if (taker <= 0)
			taker = RZ_EscortHelp.PlayerBeside(ids, players, m_vBarn, 150);

		if (taker <= 0)
			return;

		NotifyParticipants(gm, m_sTitle, QFill(gm, "{thief_short} puts his hands up and takes the page out of his boot. \"All right. All right.\" " + RZ_Quest_TheBook.SisterLine(gm, Run(gm))));
		Resolve(gm, taker, RZ_EQuestApproach.FORCE, RZ_Quest_TheBook.OUT_CAUGHT, "took him by force");
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowBarn(RZ_GameModeComponent gm)
	{
		m_bBarnUp = true;
		string choices = string.Format("Talk him out|Take him|Pay him to run (%1 Red Hand Favor)", RUN_FAVOR);
		if (!SpawnPerson(gm, m_vBarn, 0, m_iThiefTalk, "Talk to " + QText(gm, "thief_short"), choices))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		// His two friends, if the Red Hand group prefab is set
		ResourceName group = gm.GetRedHandGroupPrefab();
		if (!group.IsEmpty() && m_Friends.Spawn(group, LandPointNear(m_vBarn, 6, 12)))
		{
			TrackProp(m_Friends.m_Unit);
			if (gm.GetHostility())
				gm.GetHostility().WatchUnit(m_Friends.m_Unit, RZ_EFaction.BANDITS);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_bResolved)
			return;

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		bool armed = user && RZ_Util.IsArmed(user);
		if (option == 0)
		{
			if (armed)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "Put that away first. I'm not coming out to a gun.");
				return;
			}

			if (TryPersuade(gm, playerId, RZ_EFaction.BANDITS, 1, "excon"))
			{
				string text = "You sit down on a bale with your hands where he can see them, and talk about the book, and the pit, and what happens to people who run for ever. He listens.";
				if (HasBonus(gm, playerId))
					text = "He sees the tattoos on your hands and stops being afraid of you. \"You've done time. You know how this goes.\" He does too.";

				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, text + " " + RZ_Quest_TheBook.SisterLine(gm, Run(gm))));
				Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_TheBook.OUT_CAUGHT, "talked him out");
				return;
			}

			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "tried to talk him out");
			gm.NotifyPlayer(playerId, m_sTitle, "You're the boss's errand boy. I don't come out for errand boys.");
			return;
		}

		if (option == 1)
		{
			if (RZ_Util.CountAlive(m_Friends.m_Unit) > 0)
			{
				if (m_bFight)
					return;

				m_bFight = true;
				m_iTaker = playerId;
				Remember(gm, playerId, RZ_EQuestApproach.FORCE, "went in for him");
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Friends.m_Unit, 100);
				gm.GetHeat().AddHeat(m_vBarn, 10);
				RZ_ContractHelp.NotifyNear(gm, m_vBarn, 150, m_sTitle, "His two friends have other ideas.");
				return;
			}

			if (!armed)
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{thief_short} looks at your empty hands and laughs. \"Take me with what?\""));
				return;
			}

			gm.GetHeat().AddHeat(m_vBarn, 10);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "At gunpoint he comes, and the page comes out of his boot. " + RZ_Quest_TheBook.SisterLine(gm, Run(gm))));
			Resolve(gm, playerId, RZ_EQuestApproach.FORCE, RZ_Quest_TheBook.OUT_CAUGHT, "took him at gunpoint");
			return;
		}

		if (option == 2 && TryBribe(gm, playerId, RZ_EFaction.BANDITS, RUN_FAVOR))
		{
			m_fRewardScale = RUN_SCALE;
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He stares at what you're offering, then hands over the page and goes out of the back of the barn at a run. " + RZ_Quest_TheBook.SisterLine(gm, Run(gm))));
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_TheBook.OUT_RAN, "paid him to run");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The Book: you have the page.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Book: {thief_short}'s trail went cold.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Chain.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Book, stage 3 (Red Hand contract, Known): the names on the page, each at their own seat (the
//! leader is held and a stand-in speaks for them). Ask for what's owed (60%, once), lean on them
//! (+3 Infamy with their faction), pay it yourself (15 Red Hand Favor) or let it lie; or take their
//! line to their own stronghold (at most one name): the leader is replaced, their faction's morale
//! -3, Fame +10 with that faction and +15 Red Hand Infamy, because nobody rats. Done when every
//! name is seen to; time up with any seen to: the rest are let lie. Pay scales with what was
//! collected.
class RZ_QM_BookRead : RZ_RedHandQuestMission
{
	protected ref array<int> m_aSpotName = {};		// spot -> name on the page (-1: the hand-in at a stronghold)
	protected ref array<int> m_aTalk = {};			// name -> its talk clue
	protected ref array<bool> m_aAsked = {};		// name -> the 60% was tried
	protected int m_iHandSpot = -1;
	protected int m_iLast = -1;
	protected int m_iNames;

	protected static const int OBJ_HAND = 10;
	protected static const int PAY_FAVOR = 15;
	protected static const int LEAN_INFAMY = 3;
	protected static const float ASK_CHANCE = 0.6;
	protected static const int RAT_FAME = 10;
	protected static const int RAT_INFAMY = 15;
	protected static const float RAT_MORALE = -3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 12, 30);
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		RZ_Quest_TheBook.UpdateNames(gm, run);
		m_iNames = run.Flag("names");
		string lines = "";
		for (int i = 0; i < m_iNames; i++)
		{
			string key = RZ_Quest_TheBook.Key(i);
			m_aTalk.Insert(AddClue(run.Text(key), TalkText(run, i)));
			m_aAsked.Insert(false);
			RZ_Leader l = RZ_Quest_TheBook.Name(gm, run, i);
			if (!l || RZ_Quest_TheBook.State(run, i) != RZ_Quest_TheBook.D_OPEN)
				continue;

			AddSpot(LandPointNear(l.m_vPos, 3, 6), string.Format("%1 in %2", l.m_sName, l.m_sPlace));
			m_aSpotName.Insert(i);
			lines += string.Format(" %1 in %2 owes %3.", l.m_sName, l.m_sPlace, run.Text(key + "_what"));
		}

		RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
		if (!m_aSpots.IsEmpty())
			m_vPos = m_aSpots[0];
		else if (boss)
			m_vPos = boss.m_vPos;

		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_sTitle = "The Book: read the page";
		m_sBrief = "Here's the page. Read it." + lines + " Go round them and collect. If one of them won't pay, lean on them, or pay it yourself and they owe you instead. Your choice. Just don't rat. Nobody rats.";
		m_sObjective = "Collect what each name on the torn page owes the book.";
		// The quiet threat (Reputation > Quiet success): Fear first, so they don't tell their own
		m_Ways.m_iMenaceDifficulty = RZ_ReputationSettings.QuietDifficulty();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string TalkText(RZ_QuestRun run, int i)
	{
		string key = RZ_Quest_TheBook.Key(i);
		switch (run.Flag(key + "_faction", -1))
		{
			case RZ_EFaction.MILITARY: return run.Fill(string.Format("{%1} keeps their voice down and their back to their own soldiers. \"Eight rifles. I know. The diesel's gone, and so is the inventory officer who'd have noticed. Tell the boss I'm good for it.\"", key));
			case RZ_EFaction.INSTITUTE: return run.Fill(string.Format("{%1} doesn't look up from a sample case. \"Two live ones. I wrote them up as caught in the field. If the director finds out where they came from, there's no field station and no me.\"", key));
			case RZ_EFaction.SURVIVORS: return run.Fill(string.Format("{%1} has flour on their hands and doesn't wipe it off. \"Three months. We haven't had it. We've thirty mouths here, and every one of them eats before the Red Hand does.\"", key));
		}

		return "I know why you're here.";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsSpotOpen(int spot)
	{
		if (spot == m_iHandSpot)
			return PendingName() >= 0;

		RZ_QuestRun run = Run(GetGM());
		if (!run || spot < 0 || spot >= m_aSpotName.Count())
			return false;

		return RZ_Quest_TheBook.State(run, m_aSpotName[spot]) == RZ_Quest_TheBook.D_OPEN;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_GameModeComponent GetGM()
	{
		return RZ_GameModeComponent.GetInstance();
	}

	//------------------------------------------------------------------------------------------------
	//! The name on its way to its own stronghold, -1 if none.
	protected int PendingName()
	{
		RZ_QuestRun run = Run(GetGM());
		if (!run)
			return -1;

		int turned = run.Flag("turned", -1);
		if (turned >= 0 && RZ_Quest_TheBook.State(run, turned) == RZ_Quest_TheBook.D_PENDING)
			return turned;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SpawnSpot(RZ_GameModeComponent gm, int spot)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return;

		if (spot == m_iHandSpot)
		{
			int pending = PendingName();
			if (pending < 0)
				return;

			string who = run.Text(RZ_Quest_TheBook.Key(pending));
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_HAND, "Hand over the line about " + who, "", -1, "");
			return;
		}

		int i = m_aSpotName[spot];
		RZ_Leader l = RZ_Quest_TheBook.Name(gm, run, i);
		if (!l)
			return;

		if (l.m_iHeldBy < 0)
			HoldLeader(gm, l.m_iId);

		string choices = string.Format("Ask for what's owed|Lean on them|Pay it yourself (%1 Red Hand Favor)|Let it lie", PAY_FAVOR);
		string useLabel = "";
		vector seat;
		string seatName;
		if (run.Flag("turned", -1) < 0 && RZ_RedHandQuest.FactionSeat(gm, run.Flag(RZ_Quest_TheBook.Key(i) + "_faction", -1), l.m_vPos, seat, seatName) && RZ_Util.DistXZ(seat, l.m_vPos) > 50)
			useLabel = "Take this line to their own people in " + seatName;

		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], i, useLabel, choices, m_aTalk[i], "Talk to " + l.m_sShort);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || m_bResolved)
			return;

		int open = RZ_Quest_TheBook.UpdateNames(gm, run);
		TickSpots(gm, players);
		int next = NearestOpenSpot(m_vPos);
		if (next >= 0)
			m_vPos = m_aSpots[next];

		if (open > 0)
			return;

		Done(gm, m_iLast);
	}

	//------------------------------------------------------------------------------------------------
	//! Every name is seen to (or time is up): the stage goes on, paid by what was collected.
	protected void Done(RZ_GameModeComponent gm, int playerId)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return;

		int live = 0;
		int collected = 0;
		for (int i = 0; i < m_iNames; i++)
		{
			int state = RZ_Quest_TheBook.State(run, i);
			if (state == RZ_Quest_TheBook.D_GONE)
				continue;

			live++;
			if (RZ_Quest_TheBook.IsCollected(state) || state == RZ_Quest_TheBook.D_PAID)
				collected++;
		}

		float share = collected;
		if (live > 0)
			m_fRewardScale = share / live;
		else
			m_fRewardScale = 0;

		Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_TheBook.OUT_READ, "went round the names", collected > 0);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (run && RZ_Quest_TheBook.AnyHandled(run))
		{
			Done(gm, m_iLast);
			return;
		}

		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || objIdx < 0 || objIdx >= m_iNames || RZ_Quest_TheBook.State(run, objIdx) != RZ_Quest_TheBook.D_OPEN)
			return;

		RZ_Leader l = RZ_Quest_TheBook.Name(gm, run, objIdx);
		if (!l)
			return;

		string key = RZ_Quest_TheBook.Key(objIdx);
		int faction = run.Flag(key + "_faction", -1);
		if (option == 0)
		{
			if (m_aAsked[objIdx])
			{
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 has said no once. Asking nicely again won't change it.", l.m_sShort));
				return;
			}

			m_aAsked[objIdx] = true;
			if (Math.RandomFloat01() >= ASK_CHANCE)
			{
				Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked " + l.m_sShort + " for it");
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 hears you out and shakes their head. \"Not this week.\" They won't pay for asking.", l.m_sShort));
				return;
			}

			Settle(gm, run, playerId, objIdx, RZ_Quest_TheBook.D_TALKED, RZ_EQuestApproach.TALK, "collected from " + l.m_sShort);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 pays, slowly, and watches you write it down. \"Tell the boss we're square.\"", l.m_sShort));
			return;
		}

		if (option == 1)
		{
			if (run.Flag("squeezed", -1) < 0)
				run.SetFlag("squeezed", objIdx);

			// Quiet success: they pay and tell nobody, so their people don't hold it against you
			if (TryMenace(gm, playerId, "book." + key, faction, false))
			{
				Settle(gm, run, playerId, objIdx, RZ_Quest_TheBook.D_SQUEEZED, RZ_EQuestApproach.FORCE, "leaned on " + l.m_sShort + " quietly");
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("You say it once, quietly. %1 finds what's owed and doesn't tell a soul who came for it. That's worth something to both of you.", l.m_sShort));
				return;
			}

			RZ_QuestApproach.SideWith(gm, playerId, -1, faction, 0, LEAN_INFAMY);

			Settle(gm, run, playerId, objIdx, RZ_Quest_TheBook.D_SQUEEZED, RZ_EQuestApproach.FORCE, "leaned on " + l.m_sShort);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You say it quietly, close up. %1 finds what's owed. They'll remember who came for it. (+%2 Infamy with %3)", l.m_sShort, LEAN_INFAMY, RZ_Factions.GetNameInText(faction)));
			return;
		}

		if (option == 2)
		{
			if (!TryBribe(gm, playerId, RZ_EFaction.BANDITS, PAY_FAVOR))
				return;

			RZ_RedHandQuest.Remembers(l, gm.GetProfile(playerId).m_sIdentity, RZ_Leaders.MEM_HELPED);
			Settle(gm, run, playerId, objIdx, RZ_Quest_TheBook.D_PAID, RZ_EQuestApproach.BRIBE, "paid what " + l.m_sShort + " owed");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You square it out of your own pocket. %1 looks at you a long time. \"I owe you, then. Not them.\"", l.m_sShort));
			return;
		}

		if (option == 3)
		{
			Settle(gm, run, playerId, objIdx, RZ_Quest_TheBook.D_LEFT, RZ_EQuestApproach.NONE, "let " + l.m_sShort + "'s debt lie");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You fold the page again. %1 lets out a breath they've been holding since you arrived.", l.m_sShort));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A name is seen to: its stand-in has nothing more to offer, and the stage may be done.
	protected void Settle(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int i, int state, int approach, string what)
	{
		RZ_Quest_TheBook.SetState(run, i, state);
		if (RZ_Quest_TheBook.IsCollected(state))
			run.SetText(RZ_Quest_TheBook.Key(i) + "_by", gm.GetProfile(playerId).m_sIdentity);

		Remember(gm, playerId, approach, what);
		m_iLast = playerId;
		SetObjectLabels(i, "", "");
		SetClueText(m_aTalk[i], run.Fill(string.Format("{%1} has nothing more to say to you. On the page: %2.", RZ_Quest_TheBook.Key(i), RZ_Quest_TheBook.StateText(state))));
	}

	//------------------------------------------------------------------------------------------------
	//! "Take this line to their own people": the line goes to their stronghold (at most one name).
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return;

		if (objIdx == OBJ_HAND)
		{
			HandIn(gm, run, playerId);
			return;
		}

		if (objIdx < 0 || objIdx >= m_iNames || value != 0 || RZ_Quest_TheBook.State(run, objIdx) != RZ_Quest_TheBook.D_OPEN)
			return;

		if (run.Flag("turned", -1) >= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "One line has gone to its own people already. Nobody in the Red Hand rats twice and keeps their teeth.");
			return;
		}

		RZ_Leader l = RZ_Quest_TheBook.Name(gm, run, objIdx);
		vector seat;
		string seatName;
		if (!l || !RZ_RedHandQuest.FactionSeat(gm, run.Flag(RZ_Quest_TheBook.Key(objIdx) + "_faction", -1), l.m_vPos, seat, seatName))
			return;

		run.SetFlag("turned", objIdx);
		RZ_Quest_TheBook.SetState(run, objIdx, RZ_Quest_TheBook.D_PENDING);
		SetObjectLabels(objIdx, "", "");
		Remember(gm, playerId, RZ_EQuestApproach.SIDE, "took " + l.m_sShort + "'s line to their own people");
		m_iLast = playerId;
		m_iHandSpot = AddSpot(LandPointNear(seat, 6, 12), "Their own people in " + seatName);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You tear %1's line off the page and pocket it. %1 sees you do it. Take it to their own people in %2.", l.m_sShort, seatName));
	}

	//------------------------------------------------------------------------------------------------
	protected void HandIn(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId)
	{
		int i = PendingName();
		if (i < 0)
			return;

		string key = RZ_Quest_TheBook.Key(i);
		int faction = run.Flag(key + "_faction", -1);
		RZ_Leader l = RZ_Quest_TheBook.Name(gm, run, i);
		RZ_Quest_TheBook.SetState(run, i, RZ_Quest_TheBook.D_TURNED);
		RZ_RedHandQuest.SetWorld(gm, "book.turned_in", faction + 1);
		RZ_QuestApproach.SideWith(gm, playerId, faction, RZ_EFaction.BANDITS, RAT_FAME, RAT_INFAMY);
		RZ_Questline.NudgeMorale(gm, faction, RAT_MORALE);
		Remember(gm, playerId, RZ_EQuestApproach.SIDE, "turned " + run.Text(key) + " in");
		m_iLast = playerId;

		string text = string.Format("They read the line twice. Nobody thanks you. (+%1 Fame with %2, +%3 Red Hand Infamy: nobody rats.)", RAT_FAME, RZ_Factions.GetNameInText(faction), RAT_INFAMY);
		if (l)
		{
			ReleaseLeader(gm, l.m_iId);
			string headline = string.Format("%1 has been put out of the chair in %2. Someone showed %3 what they owed the Red Hand.", l.m_sName, l.m_sPlace, RZ_Factions.GetNameInText(faction));
			if (!RZ_Questline.ReplaceLeader(gm, l.m_iId, headline))
				text += string.Format(" %1 keeps the chair for now: somebody above them still needs them.", l.m_sShort);
		}

		gm.NotifyPlayer(playerId, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The Book: the names on the page are seen to.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Book: the names on the page were seen to, and nothing came back for the book.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Book, stage 4 (Red Hand contract, Known): what happens to the page. The book-keeper at the
//! boss's camp takes it back from someone Trusted (Known with B1's orders.mocked). At the barn's
//! fire: keep it (each faction whose leader was collected from pays the keeper 20 Favor to keep it
//! quiet) or burn it (+15 Red Hand Infamy). Bastion's duty officer takes the whole page for the
//! commanding officer (+10 Bastion Fame twice over, +15 Red Hand Infamy).
class RZ_QM_BookPage : RZ_RedHandQuestMission
{
	protected int m_iBossSpot = -1;
	protected int m_iFireSpot = -1;
	protected int m_iBastionSpot = -1;

	protected static const int OBJ_BOSS = 0;
	protected static const int OBJ_FIRE = 1;
	protected static const int OBJ_BASTION = 2;
	protected static const int RETURN_FAVOR = 20;
	protected static const int HUSH_FAVOR = 20;
	protected static const int BURN_INFAMY = 15;
	protected static const int BASTION_FAME = 10;
	protected static const int BASTION_INFAMY = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 10, 20);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
		if (!run || !boss)
			return false;

		m_iBossSpot = AddSpot(LandPointNear(boss.m_vPos, 8, 16), "The book-keeper at " + boss.m_sPlace);
		if (RZ_RedHandQuest.HasPos(run, "barn"))
			m_iFireSpot = AddSpot(RZ_RedHandQuest.PosOf(run, "barn"), "The fire at the barn outside " + run.Text("town2"));

		vector bastion;
		string bastionName;
		if (RZ_RedHandQuest.FactionSeat(gm, RZ_EFaction.MILITARY, boss.m_vPos, bastion, bastionName))
			m_iBastionSpot = AddSpot(LandPointNear(bastion, 8, 16), "Bastion's duty officer at " + bastionName);

		m_vPos = m_aSpots[m_iBossSpot];
		m_iSettlement = gm.FindNearestSettlement(boss.m_vPos);
		m_sTitle = "The Book: the page";
		m_sBrief = QFill(gm, "Bring my page back to {boss_place}. My book-keeper will take it from you, if you're someone I trust. If you're thinking of doing anything else with it, think hard.");
		m_sObjective = QFill(gm, "Bring the page back to the book-keeper at {boss_place}.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SpawnSpot(RZ_GameModeComponent gm, int spot)
	{
		if (spot == m_iBossSpot)
		{
			int keeper = AddClue("The book-keeper", "The book is open at the ragged edge. \"Well? Have you got it?\"");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_BOSS, "", "Give the page back", keeper, "Talk to the book-keeper");
			return;
		}

		if (spot == m_iFireSpot)
		{
			int fire = AddClue("The fire", "An oil drum with a fire still going in it, where the thief kept warm. Nobody else is here.");
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_aSpots[spot], OBJ_FIRE, "", "Burn the page|Keep the page", fire, "Look at the fire");
			return;
		}

		if (spot == m_iBastionSpot)
		{
			int officer = AddClue("The duty officer", "\"Name and business.\" He doesn't write either down until you say the words Red Hand. Then he does.");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_BASTION, "", "Hand over the whole page", officer, "Talk to the duty officer");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		TickSpots(gm, players);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || m_bResolved)
			return;

		if (objIdx == OBJ_BOSS && option == 0)
		{
			GiveBack(gm, run, playerId);
			return;
		}

		if (objIdx == OBJ_FIRE && option == 0)
		{
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.BANDITS, 0, BURN_INFAMY);
			RZ_RedHandQuest.SetWorld(gm, "book.burnt", 1);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The page curls, goes brown, and goes. Somewhere a bastion commander, a field lead and a council head owe nobody anything. The Red Hand will see it differently. (+%1 Red Hand Infamy)", BURN_INFAMY));
			Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_TheBook.OUT_BURNT, "burned the page", false);
			return;
		}

		if (objIdx == OBJ_FIRE && option == 1)
		{
			Keep(gm, run, playerId);
			return;
		}

		if (objIdx == OBJ_BASTION && option == 0)
		{
			int commander = RZ_Quest_TheBook.BastionName(run);
			RZ_Leader l = RZ_Quest_TheBook.Name(gm, run, commander);
			int value = 0;
			if (l)
				value = l.m_iId + 1;

			RZ_RedHandQuest.SetWorld(gm, "book.bastion", value);
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, BASTION_FAME, BASTION_INFAMY);
			run.Reward(gm, playerId, RZ_EFaction.MILITARY, BASTION_FAME, 0, "The commanding officer reads the whole page, twice, and remembers who brought it.");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The duty officer takes the page with two fingers, like it's wet. \"The commanding officer will want to thank you. Quietly.\" (+%1 Red Hand Infamy)", BASTION_INFAMY));
			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_TheBook.OUT_BASTION, "gave the page to Bastion", false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Trusted with the Red Hand, or Known while the boss owes for B1's open channel (orders.mocked).
	protected void GiveBack(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId)
	{
		RZ_ETier tier = gm.GetTier(playerId, RZ_EFaction.BANDITS);
		bool owed = RZ_RedHandQuest.HasWorld(gm, "orders.mocked") && RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN);
		if (!RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED) && !owed)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The book-keeper shakes his head. \"The boss takes it back from people the boss trusts. Not from you. Not yet.\"");
			return;
		}

		array<string> names = {"Bookends", "the Clerk", "Margins", "Receipts", "Tally", "the Accountant", "Ink", "Carbon Copy"};
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		run.SetText("crewname", rng.Pick(names));
		run.SetText("returner", RZ_RedHandQuest.NameOf(playerId));
		RZ_RedHandQuest.SetWorld(gm, "book.returned", 1);
		run.Reward(gm, playerId, RZ_EFaction.BANDITS, 0, RETURN_FAVOR, "The boss counts the page back into the book in person.");

		string text = "The book-keeper smooths the page flat and glues it back where it belongs. The book balances.";
		if (owed && !RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED))
			text = "The book-keeper hesitates, then remembers who read Bastion's orders out on the open channel. \"The boss owes you for that one.\" He glues the page back. The book balances.";

		gm.NotifyPlayer(playerId, m_sTitle, text);
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheBook.OUT_RETURNED, "gave the page back");
	}

	//------------------------------------------------------------------------------------------------
	//! The keeper is paid, once, by each faction whose leader was collected from.
	protected void Keep(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId)
	{
		run.SetText("keeper", gm.GetProfile(playerId).m_sIdentity);
		RZ_RedHandQuest.SetWorld(gm, "book.kept", 1);
		for (int i = 0; i < run.Flag("names"); i++)
		{
			string key = RZ_Quest_TheBook.Key(i);
			if (!RZ_Quest_TheBook.IsCollected(RZ_Quest_TheBook.State(run, i)) || !RZ_Quest_TheBook.Name(gm, run, i))
				continue;

			run.Reward(gm, playerId, run.Flag(key + "_faction", -1), 0, HUSH_FAVOR, run.Fill(string.Format("{%1} hears who has the page now, and pays to keep their line quiet.", key)));
		}

		gm.NotifyPlayer(playerId, m_sTitle, "You fold the page small and put it somewhere nobody will look. The people on it will pay to keep it there. The boss will hear. Not yet.");
		Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_TheBook.OUT_KEPT, "kept the page", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The Book: the page is back in the book.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Book: the page didn't go back into the book.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Book, stage 5 (Red Hand contract, Known): the thief at the pit. Let it happen; take his
//! place (stand in the ring for RING_SECONDS against ROUNDS rounds); pay his debt (30 Red Hand
//! Favor); or get him out (he follows you to the Haven; while he's within SPOT_RADIUS of the pit
//! the crowd may see: +10 Red Hand Infamy at once, and the pit crew fights if the Red Hand group
//! prefab is set). Nobody chooses in time: he goes in.
class RZ_QM_BookThief : RZ_QuestMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vPit;
	protected vector m_vRefuge;
	protected string m_sRefuge;
	protected IEntity m_Thief;
	protected IEntity m_Crew;
	protected int m_iTalk = -1;
	protected int m_iPath;
	protected int m_iSecond = -1;
	protected float m_fHeld;
	protected int m_iRounds;
	protected float m_fSpotTimer;
	protected bool m_bSpotted;
	protected bool m_bClear;

	protected static const int PATH_NONE = 0;
	protected static const int PATH_RING = 1;
	protected static const int PATH_OUT = 2;

	protected static const int DEBT_FAVOR = 30;
	protected static const float RING_SECONDS = 240;
	protected static const float RING_RADIUS = 25;
	protected static const int ROUNDS = 4;
	protected static const float FIRST_ROUND = 20;
	protected static const float ROUND_GAP = 55;
	protected static const int ROUND_SIZE = 3;
	protected static const float SPOT_RADIUS = 60;
	protected static const float SPOT_EVERY = 5;
	protected static const float SPOT_DAY = 0.3;
	protected static const float SPOT_NIGHT = 0.12;
	protected static const int SPOTTED_INFAMY = 10;
	protected static const int HAVEN_FAME = 5;
	protected static const int HAVEN_FAVOR = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 10, 20);
		m_Req.Bonus("excon");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
		m_fSceneRadius = 300;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !RZ_RedHandQuest.HasPos(run, "pit"))
			return false;

		m_vPit = RZ_RedHandQuest.PosOf(run, "pit");
		if (!RZ_RedHandEdges.HavenRefuge(gm, m_vPit, m_vRefuge, m_sRefuge))
			return false;

		run.SetText("refuge", m_sRefuge);
		m_vPos = m_vPit;
		m_iSettlement = gm.FindNearestSettlement(m_vPit);
		m_iTalk = AddClue(QText(gm, "thief"), QFill(gm, "{thief_short} has his hands tied in front of him and his eyes on the pens. \"I'd do it again. Tell {sister} I'd do it again.\""));
		m_Escort.m_sName = QText(gm, "thief_short");
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.m_bHide = false;
		m_sTitle = "The Book: the thief";
		m_sBrief = QFill(gm, "The pit's on tonight at {pit}, and {thief_short} is going in it. Come and watch, so everyone knows what happens to people who tear pages out of my book. Or do something stupid. Everyone will know that too.");
		m_sObjective = QFill(gm, "Be at the pit at {pit} tonight when {thief_short} goes in.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string choices = string.Format("Let it happen|Take his place|Pay his debt (%1 Red Hand Favor)|Get him out of here", DEBT_FAVOR);
		m_Thief = SpawnPerson(gm, m_vPit, 0, m_iTalk, "Talk to " + QText(gm, "thief_short"), choices);
		if (!m_Thief)
			return false;

		m_Crew = RZ_RedHandEdges.SpawnCrew(gm, gm.GetRedHandGroupPrefab(), m_vPit, 10, 20);
		TrackProp(m_Crew);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		if (m_iPath == PATH_OUT)
			text += string.Format("\nTake him to: %1 (%2)", m_sRefuge, RZ_Util.DirectionText(from, m_vRefuge));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_iPath != PATH_NONE || m_bResolved)
			return;

		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You step back into the crowd. They open the pens. {thief_short} doesn't shout, which is the worst of it."));
			Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_TheBook.OUT_PIT, "let it happen");
			return;
		}

		if (option == 1)
		{
			m_iPath = PATH_RING;
			m_iSecond = playerId;
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "took his place in the pit");
			RZ_Util.DeleteUnit(m_Thief);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You climb into the ring yourself. The crowd loves it. {boss} shrugs: a show is a show. Stay standing for four rounds and {thief_short} walks."));
			return;
		}

		if (option == 2)
		{
			if (!TryBribe(gm, playerId, RZ_EFaction.BANDITS, DEBT_FAVOR))
				return;

			m_fRewardScale = 0.5;
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You pay it out of your own Favor. The book-keeper crosses {thief_short} out, and the crowd boos. A debt is a debt, and it's been paid."));
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_TheBook.OUT_PAID, "paid his debt");
			return;
		}

		if (option != 3)
			return;

		if (!m_Escort.Begin(gm, m_vPit))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{thief_short} shakes his head. \"Not with this many watching. Give it a few minutes.\""));
			return;
		}

		m_iPath = PATH_OUT;
		m_iSecond = playerId;
		Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "walked him out of the pit");
		RZ_Util.DeleteUnit(m_Thief);
		m_Escort.Follow(playerId);
		m_Escort.Go();
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You take {thief_short} by the arm and walk him out like you've every right to. Don't run. Take him to {refuge}."));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (m_iPath == PATH_RING)
		{
			UpdateRing(gm, dt, ids, players);
			return;
		}

		if (m_iPath != PATH_OUT)
			return;

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_Escort.IsDead())
		{
			NotifyParticipants(gm, m_sTitle, QFill(gm, "{thief_short} never got clear of the pit."));
			Resolve(gm, m_iSecond, RZ_EQuestApproach.SNEAK, RZ_Quest_TheBook.OUT_PIT, "tried to get him out", false);
			return;
		}

		UpdateSpotting(gm, dt);
		if (!m_Escort.IsNear(m_vRefuge, 25))
			return;

		m_Escort.Arrive();
		RZ_Questline.ChangePopulation(gm, RZ_RedHandQuest.HouseAt(gm, m_vRefuge), 1);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, m_iSecond, RZ_EFaction.SURVIVORS, HAVEN_FAME, HAVEN_FAVOR, QFill(gm, "The Haven takes {thief_short} in and doesn't ask what he owed."));

		NotifyParticipants(gm, m_sTitle, QFill(gm, "The Haven gives {thief_short} a blanket and a bowl. He asks after {sister} before he eats."));
		Resolve(gm, m_iSecond, RZ_EQuestApproach.SNEAK, RZ_Quest_TheBook.OUT_FREED, "got him to the Haven", false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRing(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!RZ_ContractHelp.AnyNear(players, m_vPit, RING_RADIUS))
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vPit, RING_RADIUS, m_aParticipants);
		m_fHeld += dt;
		if (m_iRounds < ROUNDS && m_fHeld >= FIRST_ROUND + m_iRounds * ROUND_GAP)
		{
			m_iRounds++;
			SpawnGuards(gm, m_vPit, ROUND_SIZE, 20, 35);
			NotifyParticipants(gm, m_sTitle, string.Format("Round %1. The crowd has stopped betting on whether you'll last and started betting on how.", m_iRounds));
		}

		if (m_fHeld < RING_SECONDS)
			return;

		NotifyParticipants(gm, m_sTitle, QFill(gm, "Last bell. You're still standing. {boss} keeps the word the whole crowd heard given: {thief_short} walks."));
		Resolve(gm, m_iSecond, RZ_EQuestApproach.FORCE, RZ_Quest_TheBook.OUT_PLACE, "took his place and walked out");
	}

	//------------------------------------------------------------------------------------------------
	//! Walking him out: while he's near the pit, the crowd may notice (once).
	protected void UpdateSpotting(RZ_GameModeComponent gm, float dt)
	{
		if (m_bSpotted || m_bClear)
			return;

		if (!m_Escort.IsNear(m_vPit, SPOT_RADIUS))
		{
			m_bClear = true;
			gm.NotifyPlayer(m_iSecond, m_sTitle, "Nobody shouts. The crowd is watching the pens.");
			return;
		}

		m_fSpotTimer += dt;
		if (m_fSpotTimer < SPOT_EVERY)
			return;

		m_fSpotTimer = 0;
		float chance = SPOT_DAY;
		if (RZ_GameTime.IsNight())
			chance = SPOT_NIGHT;

		if (HasBonus(gm, m_iSecond))
			chance = chance * 0.5;

		if (Math.RandomFloat01() >= chance)
			return;

		m_bSpotted = true;
		RZ_QuestApproach.SideWith(gm, m_iSecond, -1, RZ_EFaction.BANDITS, 0, SPOTTED_INFAMY);
		string text = string.Format("\"Oi! Where's he going?\" The crowd has seen. (+%1 Red Hand Infamy)", SPOTTED_INFAMY);
		if (RZ_Util.CountAlive(m_Crew) > 0)
		{
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew, 150);
			text += " The pit crew is coming after you.";
		}

		RZ_ContractHelp.NotifyNear(gm, m_vPit, 150, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody did anything in time: he goes in.
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetOutcome(RZ_Quest_TheBook.OUT_PIT);

		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The Book: the thief is settled.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Book: {thief_short} went in the pit.");
	}

	//------------------------------------------------------------------------------------------------
	//! The pit; walking him out, the refuge.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_iPath == PATH_OUT && !m_Escort.IsArrived())
			return RZ_ObjPlace.Spot(m_vRefuge, 25);

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}
