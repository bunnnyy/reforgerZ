//! Cargo (the Red Hand, or the Haven; R2 in docs/QUESTLINES.md, built as in docs/MISSIONS.md,
//! "Questlines"). Some trucks carry people, not tins. The driver of one of them wants out. The
//! long version of the Haven's Tied Down contract (Missions/RedHand/RZ_RedHandLoreMissions.c),
//! sharing its van and padlock (RZ_CargoVan). The Haven's version starts only after Tied Down has
//! reached its shed this season (world flag cargo.truck_seen): its stage missions are Haven
//! contracts; otherwise they're the Red Hand's.
//!
//! Where the loads go: the Long Table's farm when the house publishes where it is (world flag
//! table.town, its settlement index + 1), else the pit behind the crew boss's own camp.
//!
//! 1. Meet the driver (mission, dusk): at a crossroads near his crew's camp. Hear him out (stay with
//!    him a while; ★ Ex-Con: sooner) or threaten him (he talks, then tells his crew: the camp is on
//!    guard).
//! 2. The load (mission): the van at the camp, three people padlocked in the back, its guards.
//!    Let them out (cut the lock, walk them to a safehouse: +3 people, +10 Red Hand Infamy); start
//!    it (★ Mechanic: first time) and drive it to the drop (cargo.delivered) or to a Bastion seat
//!    (cargo.bastion: the driver is arrested), or to the Haven with them still in the back.
//! 3. Who buys (mission; skipped if you drove it to the drop yourself): watch the drop after dark
//!    from 60-150 m for 2 minutes unseen (★ Police: 1 minute), ask the fence (Trusted with the Red
//!    Hand) or pay the driver (10 Red Hand Favor) to name the crew boss. At the farm: cargo.buyer_known.
//! 4. Tell someone (mission): the boss (Trusted with the Red Hand: a purge), the crew boss (keep
//!    quiet for a cut), Bastion (the camp, and the farm's grid) or the Haven's chair.
//! 5. The driver (mission): get him out to a safehouse, or leave him driving; after a purge, take his
//!    place in the pit or let it be; after Bastion's raid, assault the camp.
//! 6-8. The cut (waits, only after the cut): one session each, 20 Red Hand Favor a session.
//!
//! Endings: Purged, The cut, Bastion's raid, Told (Freed and told, in the design).
class RZ_Quest_Cargo : RZ_Questline
{
	static const int S_DRIVER = 0;
	static const int S_LOAD = 1;
	static const int S_BUYS = 2;
	static const int S_TELL = 3;
	static const int S_FATE = 4;
	static const int S_CUT1 = 5;
	static const int S_CUT2 = 6;
	static const int S_CUT3 = 7;

	static const int OUT_MET = 10;			// stage 1: heard him out
	static const int OUT_THREATENED = 11;	// stage 1: threatened him (the camp is on guard)
	static const int OUT_FREED = 12;		// stage 2
	static const int OUT_DELIVERED = 13;
	static const int OUT_BASTION = 14;
	static const int OUT_KNOWN = 15;		// stage 3: who buys
	static const int OUT_PURGE = 16;		// stage 4
	static const int OUT_CUT = 17;
	static const int OUT_RAID = 18;
	static const int OUT_HAVEN = 19;
	static const int OUT_DRIVER_OUT = 20;	// stage 5
	static const int OUT_DRIVER_STAYS = 21;
	static const int OUT_DRIVER_PIT = 22;
	static const int OUT_DRIVER_PLACE = 23;
	static const int OUT_RAIDED = 24;

	static const int E_PURGED = 0;
	static const int E_CUT = 1;
	static const int E_RAID = 2;
	static const int E_TOLD = 3;

	// The driver's fate (world flag cargo.driver)
	static const int DRIVER_OUT = 1;
	static const int DRIVER_DRIVING = 2;
	static const int DRIVER_PIT = 3;
	static const int DRIVER_ARRESTED = 4;

	static const int CUT_FAVOR = 20;
	static const int EXPOSED_INFAMY = 15;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info("cargo", "Cargo", RZ_EFaction.BANDITS);

		RZ_QuestStage driver = MissionStage(S_DRIVER, "Meet the driver", "{intro}");
		driver.GiveUp(12, "{driver_short} stopped waiting at the crossroads. The van with the canvas tied down still goes by, and nobody stops it.");
		driver.Visits(2);

		RZ_QuestStage load = MissionStage(S_LOAD, "The load", "{driver_short} showed you the van at {crew_short}'s camp outside {crew_place}: three people in the back, padlocked in. Let them out, or drive it where it's going, to {drop}{bastion_line}.");
		load.GiveUp(4, "The van stopped coming by {crew_short}'s camp. Wherever it goes now, it goes without anyone watching.");
		load.Visits(2);

		RZ_QuestStage buys = MissionStage(S_BUYS, "Who buys", "Somebody buys what's in those vans. Watch the drop at {drop} after dark, ask the Red Hand's fence, or pay {driver_short} to say who.");
		buys.GiveUp(4, "Nobody found out who buys what's in the vans. They keep running.");
		buys.Visits(2);

		RZ_QuestStage tell = MissionStage(S_TELL, "Tell someone", "{buyer_line} Now somebody has to hear it: the boss, {crew_short} (for a cut), Bastion, or the Haven's chair.");
		tell.GiveUp(4, "Nobody told anyone what's in the vans. They keep running.");
		tell.Visits(2);

		RZ_QuestStage fate = MissionStage(S_FATE, "The driver", "{fate_line}");
		fate.GiveUp(8, "Nobody went back for {driver_short}. He drives the van, and doesn't eat meat.");

		RZ_QuestStage cut1 = WaitStage(S_CUT1, "The cut", "The cut comes once a session, as long as the vans run.");
		cut1.Wait(120);
		RZ_QuestStage cut2 = WaitStage(S_CUT2, "The cut", "The cut comes once a session, as long as the vans run.");
		cut2.Wait(120);
		RZ_QuestStage cut3 = WaitStage(S_CUT3, "The cut", "The cut comes once a session, as long as the vans run.");
		cut3.Wait(120);
	}

	//------------------------------------------------------------------------------------------------
	//! A crew boss's camp (an outpost, not the stronghold), the van, a crossroads, the driver. The
	//! Haven's version only after Tied Down reached its shed this season.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (RZ_CargoVan.VanPrefab(gm).IsEmpty() || RZ_Seasons.Mods().m_bNoVehicles || !gm.GetLeaders())
			return false;

		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return false;

		RZ_Leader boss = gm.GetLeaders().GetStronghold(RZ_EFaction.BANDITS);
		RZ_Leader crew = RZ_RedHandQuest.PickLeader(gm, RZ_EFaction.BANDITS, 0);
		if (!boss || !crew)
			return false;

		RZ_RedHandQuest.Keep(run, "boss", boss);
		RZ_RedHandQuest.Keep(run, "crew", crew);
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		string first = RZ_RedHandEdges.DebtorName(rng);
		run.SetText("driver", first + " " + RZ_NoteText.Surname(rng));
		run.SetText("driver_short", first);

		// The crossroads: a few hundred metres out from the camp, toward the Haven
		vector toward;
		string ignored;
		if (!RZ_RedHandEdges.HavenRefuge(gm, crew.m_vPos, toward, ignored))
			return false;

		float dist = RZ_Util.DistXZ(crew.m_vPos, toward);
		float share = 0.5;
		if (dist > 800)
			share = 400 / dist;

		RZ_RedHandQuest.KeepPos(run, "cross", RZ_ContractHelp.Between(crew.m_vPos, toward, share));
		RZ_RedHandQuest.KeepPos(run, "camp", crew.m_vPos);

		bool haven = RZ_RedHandQuest.HasWorld(gm, "cargo.truck_seen") && Math.RandomFloat01() < 0.5;
		if (haven)
		{
			run.SetFlag("haven", 1);
			string witness = "";
			if (gm.GetWanderers())
				witness = gm.GetWanderers().AnyName(RZ_Wanderers.RoleBit(RZ_EWandererRole.TRADER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.COURIER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER));

			if (witness.IsEmpty())
				witness = "A Haven lookout";

			run.SetText("witness", witness);
			run.SetText("intro", run.Fill("{witness} saw the van with the canvas tied down again, and its driver, {driver}, asked them to pass a message to the Haven: he wants out. He'll be at a crossroads outside {crew_place}, at dusk."));
		}
		else
		{
			run.SetText("intro", run.Fill("The driver of one of the Red Hand's canvas-backed vans, {driver}, wants out. He'll meet someone at a crossroads outside {crew_place}, at dusk. He didn't say what he carries."));
		}

		run.SetPlace(RZ_RedHandQuest.PosOf(run, "cross"), crew.m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven's version: the stage missions are Haven contracts.
	static bool IsHaven(RZ_QuestRun run)
	{
		return run && run.Flag("haven") == 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's version is the driver's own business: no leader voices or remembers it. The
	//! Haven's is voiced by the nearest Haven leader, as usual.
	static void Voice(RZ_Mission m, RZ_QuestRun run)
	{
		if (m && !IsHaven(run))
			m.m_iLeaderId = RZ_RedHandQuest.NO_VOICE;
	}

	//------------------------------------------------------------------------------------------------
	//! Whose contract a stage is.
	static int Giver(RZ_QuestRun run)
	{
		if (IsHaven(run))
			return RZ_EFaction.SURVIVORS;

		return RZ_EFaction.BANDITS;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the loads go: the Long Table's farm while it publishes its town, else the pit behind the
	//! crew boss's camp. Picked again whenever it may have changed (the house moved, or is gone).
	static void PickDrop(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int farmTown = RZ_RedHandQuest.World(gm, "table.town") - 1;
		RZ_Settlement farm = gm.GetSettlement(farmTown);
		bool wasFarm = run.Flag("farm") == 1;
		if (farm)
		{
			if (wasFarm && run.Flag("farm_town", -1) == farmTown)
				return;

			if (wasFarm)
				run.Note("The farm has moved. The van goes somewhere else past the last village now.");

			run.SetFlag("farm", 1);
			run.SetFlag("farm_town", farmTown);
			vector layby = RZ_Mission.LandPointNear(farm.m_vCenter, farm.m_fRadius + 250, farm.m_fRadius + 450);
			RZ_RedHandQuest.KeepPos(run, "drop", layby);
			run.SetText("drop", "a lay-by below a farm past " + farm.m_sName);
			run.SetText("farm_grid", RZ_SettlementScanner.GridReference(farm.m_vCenter));
			return;
		}

		if (!wasFarm && RZ_RedHandQuest.HasPos(run, "drop"))
			return;

		if (wasFarm)
			run.Note("Whatever was at the farm past the last village, it's gone. The vans go to the pit now.");

		run.SetFlag("farm", 0);
		vector camp = RZ_RedHandQuest.PosOf(run, "camp");
		RZ_RedHandQuest.KeepPos(run, "drop", RZ_Mission.LandPointNear(camp, 250, 400));
		run.SetText("drop", run.Fill("the pit behind {crew_short}'s camp"));
	}

	//------------------------------------------------------------------------------------------------
	static bool IsFarm(RZ_QuestRun run)
	{
		return run && run.Flag("farm") == 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The crew boss who sells the loads: the same one, or whoever has the chair now. False if the
	//! camp itself is gone.
	static bool CheckCrew(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Leader seat = RZ_RedHandQuest.Seat(gm, run, "crew");
		if (!seat)
			return false;

		string old = run.Text("crew");
		RZ_Leader fresh = RZ_RedHandQuest.Successor(gm, run, "crew");
		if (!fresh)
			return true;

		run.Note(string.Format("%1 is out of the chair in %2. %3 has it now, and the vans still run.", old, fresh.m_sPlace, fresh.m_sName));
		if (!IsFarm(run) && RZ_RedHandQuest.HasPos(run, "drop"))
			run.SetText("drop", run.Fill("the pit behind {crew_short}'s camp"));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, "Another one never arrived: a man who left a safehouse for the next town over. His family has stopped asking the Red Hand.", false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		CheckCrew(gm, run);
		if (stage == S_DRIVER)
		{
			run.Note("{driver} drives one of the vans with the canvas tied down, for {crew_short}'s crew outside {crew_place}. He wants out.");
			return;
		}

		if (stage == S_LOAD)
		{
			PickDrop(gm, run);
			string line = "";
			vector seat;
			string seatName;
			if (RZ_RedHandQuest.FactionSeat(gm, RZ_EFaction.MILITARY, RZ_RedHandQuest.PosOf(run, "camp"), seat, seatName))
				line = ", or take it to Bastion at " + seatName;

			run.SetText("bastion_line", line);
			run.SetPlace(RZ_RedHandQuest.PosOf(run, "camp"), run.Text("crew_place"));
			return;
		}

		if (stage == S_BUYS)
		{
			PickDrop(gm, run);
			run.SetPlace(RZ_RedHandQuest.PosOf(run, "drop"), run.Text("drop"));
			return;
		}

		if (stage == S_TELL)
		{
			PickDrop(gm, run);
			run.SetText("buyer_line", BuyerLine(run));
			return;
		}

		if (stage == S_FATE)
			run.SetText("fate_line", FateLine(run));
	}

	//------------------------------------------------------------------------------------------------
	static string BuyerLine(RZ_QuestRun run)
	{
		if (IsFarm(run))
			return run.Fill("{crew_short} sells the loads to a farm past the last village, at grid {farm_grid}. A man there pays in jars.");

		return run.Fill("{crew_short} sells the loads to the crew's own pit: strays who owe nothing go in with the debtors, and the crew boss keeps the bets.");
	}

	//------------------------------------------------------------------------------------------------
	static string FateLine(RZ_QuestRun run)
	{
		switch (run.m_iBranch)
		{
			case OUT_PURGE: return run.Fill("The boss wants {driver_short} in the pit too, at {drop_pit}. Take his place, get him out, or let it be.");
			case OUT_RAID: return run.Fill("Bastion goes into {crew_short}'s camp outside {crew_place} at dawn, and wants it quiet when it gets there. Go in first.");
		}

		return run.Fill("{driver_short} is at the crossroads outside {crew_place}, waiting to see what you did. Get him out to the Haven, or tell him to keep driving.");
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (!CheckCrew(gm, run))
			return false;

		if (stage == S_LOAD || stage == S_BUYS || stage == S_TELL)
			PickDrop(gm, run);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_DRIVER:
			{
				// He only meets at dusk
				if (!RZ_RedHandQuest.IsDusk())
					return null;

				return new RZ_QM_CargoDriver();
			}
			case S_LOAD: return new RZ_QM_CargoLoad();
			case S_BUYS: return new RZ_QM_CargoBuys();
			case S_TELL: return new RZ_QM_CargoTell();
			case S_FATE: return new RZ_QM_CargoFate();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		if (stage < S_CUT1 && !CheckCrew(gm, run))
			return Fizzle(run, "The crew in {crew_place} is gone, and its vans with it. Whoever was in the back of the last one, nobody says.");

		switch (stage)
		{
			case S_DRIVER:
			{
				if (outcome == OUT_MET)
				{
					run.Note(DriverStory(gm, run, false));
					return S_LOAD;
				}

				if (outcome == OUT_THREATENED)
				{
					run.SetFlag("on_guard", 1);
					run.Note(DriverStory(gm, run, true));
					return S_LOAD;
				}

				run.Note("Nobody came to the crossroads. {driver_short} will try once more.");
				return S_DRIVER;
			}
			case S_LOAD:
				return AfterLoad(gm, run, outcome);
			case S_BUYS:
			{
				if (outcome != OUT_KNOWN)
				{
					run.Note("Nobody found out who buys. The vans keep running.");
					return S_BUYS;
				}

				BuyerKnown(gm, run);
				return S_TELL;
			}
			case S_TELL:
				return AfterTell(gm, run, outcome);
			case S_FATE:
				return AfterFate(gm, run, outcome);
			case S_CUT1:
			{
				PayCut(gm, run);
				if (run.Flag("exposed") == 1)
					return EndCut(run);

				return S_CUT2;
			}
			case S_CUT2:
			{
				PayCut(gm, run);
				if (run.Flag("exposed") == 1)
					return EndCut(run);

				return S_CUT3;
			}
			case S_CUT3:
			{
				PayCut(gm, run);
				return EndCut(run);
			}
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	//! What he tells you at the crossroads, with the List (H1) if the Haven's strays came off it.
	protected string DriverStory(RZ_GameModeComponent gm, RZ_QuestRun run, bool threatened)
	{
		PickDrop(gm, run);
		string text = "He's carried strays the Haven turned out, debtors from the book, a deserter or two. Most went to the pit, or to the line to learn the drones.";
		if (IsFarm(run))
			text += " Lately {crew_short} sends a load somewhere else: a farm past the last village, where a man pays in jars. The driver has stopped eating meat.";
		else
			text += " Lately {crew_short} sends the ones who owe nothing to the crew's own pit as well, and keeps the bets.";

		if (RZ_RedHandQuest.HasWorld(gm, "list.to_redhand"))
			text += " Half of this month's strays came off a Haven list somebody sold.";

		if (threatened)
			text += " He told you all of it, and then he told his crew someone was asking. The camp will be watching.";

		return text + " There's a van at the camp now, padlocked.";
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterLoad(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_FREED)
		{
			run.Note("The three from the van are at the Haven now. {crew_short} is short a load, and knows it.");
			return S_BUYS;
		}

		if (outcome == OUT_BASTION)
		{
			run.SetFlag("arrested", 1);
			RZ_RedHandQuest.SetWorld(gm, "cargo.driver", DRIVER_ARRESTED);
			run.Note("Bastion has the van, the three from the back on its books as civilians sheltered, and {driver_short} in a cell.");
			return S_BUYS;
		}

		if (outcome == OUT_DELIVERED)
		{
			// Whoever drove it there saw who took them
			run.Note("The van went where it was going, and whoever drove it saw who took the load off the back.");
			BuyerKnown(gm, run);
			return S_TELL;
		}

		run.Note("The van left the camp without anyone stopping it. {crew_short} will load another.");
		return S_LOAD;
	}

	//------------------------------------------------------------------------------------------------
	//! The buyer is known. At the farm that's the Long Table's proof (C1 reads cargo.buyer_known).
	protected void BuyerKnown(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		PickDrop(gm, run);
		if (IsFarm(run))
		{
			RZ_RedHandQuest.SetWorld(gm, "cargo.buyer_known", 1);
			RZ_RedHandQuest.SetWorld(gm, "cargo.buyer", 1);
			string line = "The buyer is the farm past the last village. The fence calls it pork.";
			if (RZ_RedHandQuest.HasWorld(gm, "door.exile_sold"))
				line += " The exile the safehouse sold was one of the ones who never arrived.";

			run.Note(line);
			return;
		}

		RZ_RedHandQuest.SetWorld(gm, "cargo.buyer", 2);
		run.Note("The buyer is {crew_short}'s own pit. People who owe the book nothing go in with the debtors, and the bets go in {crew_short}'s pocket.");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterTell(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome != OUT_PURGE && outcome != OUT_CUT && outcome != OUT_RAID && outcome != OUT_HAVEN)
		{
			run.Note("Nobody told anyone. The vans keep running.");
			return S_TELL;
		}

		run.SetBranch(outcome);
		if (outcome == OUT_PURGE)
			run.Note("The boss heard it. The code has no rule about this, and the boss says it doesn't need one.");
		else if (outcome == OUT_CUT)
			run.Note("{crew_short} pays for quiet, once a session.");
		else if (outcome == OUT_RAID)
			run.Note("Bastion has the camp's grid, and the farm's, if there is one.");
		else
			run.Note("The Haven's chair knows where the strays go now.");

		// The driver's stage: nothing to do for a man already in a Bastion cell, and Bastion's raid
		// needs the crew there (the Red Hand group prefab)
		if (outcome == OUT_RAID)
		{
			if (gm.GetRedHandGroupPrefab().IsEmpty())
			{
				run.Note("Bastion went into the camp at dawn without waiting for anyone.");
				return EndBranch(run);
			}

			return S_FATE;
		}

		if (run.Flag("arrested") == 1)
			return EndBranch(run);

		if (outcome == OUT_PURGE)
			run.SetText("drop_pit", run.Fill("the boss's pit"));

		return S_FATE;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterFate(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		int fate = DRIVER_DRIVING;
		string line = "{driver_short} is still driving.";
		if (outcome == OUT_DRIVER_OUT)
		{
			fate = DRIVER_OUT;
			line = "{driver_short} is at the Haven now. He eats the bread and leaves the soup.";
		}
		else if (outcome == OUT_DRIVER_PIT)
		{
			fate = DRIVER_PIT;
			line = "{driver_short} went in the pit. The boss calls it tidying up.";
		}
		else if (outcome == OUT_DRIVER_PLACE)
		{
			line = "Somebody took {driver_short}'s place in the pit and walked out of it. He drives for another crew now.";
		}
		else if (outcome == OUT_RAIDED)
		{
			fate = DRIVER_ARRESTED;
			line = "Somebody cleared the camp before Bastion got there. {driver_short} is in a Bastion cell, alive.";
		}
		else if (run.m_iBranch == OUT_PURGE)
		{
			fate = DRIVER_PIT;
			line = "Nobody came for {driver_short}, and the boss's pit didn't wait.";
		}
		else if (run.m_iBranch == OUT_RAID)
		{
			fate = DRIVER_ARRESTED;
			line = "Bastion went into the camp without anyone else. {driver_short} is in a Bastion cell.";
		}

		run.SetFlag("driver_fate", fate);
		RZ_RedHandQuest.SetWorld(gm, "cargo.driver", fate);
		run.Note(line);
		return EndBranch(run);
	}

	//------------------------------------------------------------------------------------------------
	protected int EndBranch(RZ_QuestRun run)
	{
		switch (run.m_iBranch)
		{
			case OUT_PURGE: return Finish(run, E_PURGED, "Purged. The boss has dealt with {crew_short}'s crew, and the vans with the canvas tied down have stopped. For now.");
			case OUT_CUT: return S_CUT1;
			case OUT_RAID: return Finish(run, E_RAID, "Bastion's raid. {crew_short}'s camp is broken up, and Bastion knows what the vans were for.");
		}

		return Finish(run, E_TOLD, "Told. The Haven knows where its strays went, and it will remember who told it.");
	}

	//------------------------------------------------------------------------------------------------
	//! A session's cut, for whoever took it, if they're on (it waits for nobody). While the Long
	//! Table is exposed (table.exposed, C1) the Haven holds it against them, once.
	protected void PayCut(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.Flag("exposed") != 1 && IsFarm(run) && RZ_RedHandQuest.HasWorld(gm, "table.exposed"))
		{
			run.SetFlag("exposed", 1);
			ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 0, EXPOSED_INFAMY, RZ_EQuestApproach.BETRAY);
			run.Note("The farm past the last village is out in the open now, and so is who was paid to keep quiet about it.");
			return;
		}

		int playerId = -1;
		if (gm.GetQuestlines())
			playerId = gm.GetQuestlines().OnlineId(run.Text("cut_ident"));

		if (playerId <= 0)
		{
			run.Note("A session's cut went unpaid: nobody was there to take it.");
			return;
		}

		run.Reward(gm, playerId, RZ_EFaction.BANDITS, 0, CUT_FAVOR, run.Fill("{crew_short}'s man finds you, as agreed, and counts out your cut."));
	}

	//------------------------------------------------------------------------------------------------
	protected int EndCut(RZ_QuestRun run)
	{
		if (run.Flag("exposed") == 1)
			return Finish(run, E_CUT, "The cut stopped coming when the farm was found out. The people on the vans stopped too. Nobody thanks you.");

		return Finish(run, E_CUT, "The cut. Three sessions paid, and the vans with the canvas tied down still run.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		if (ending == E_PURGED)
		{
			NudgeMorale(gm, RZ_EFaction.BANDITS, -3);
			ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0);
			RZ_Leader crew = RZ_RedHandQuest.Seat(gm, run, "crew");
			bool replaced = false;
			if (crew && crew.IsPresent())
				replaced = ReplaceLeader(gm, crew.m_iId, string.Format("The boss has purged %1's crew in %2. Nobody says what for, and nobody needs to. The Red Hand says %3 was bitten.", crew.m_sName, crew.m_sPlace, crew.m_sShort));

			if (!replaced)
				Headline(gm, run.Fill("The boss has had words with the crew outside {crew_place}. Its vans have stopped running."));

			if (IsFarm(run))
			{
				RZ_RedHandQuest.SetWorld(gm, "table.supplier_cut", 1);
				SpreadRumour(gm, "There's a farm past the last village that's lost its butcher. That's what the Red Hand says. Then it laughs.", 8);
			}
			else if (replaced)
			{
				SpreadRumour(gm, "The boss emptied a crew boss's pit last night, and put the crew boss in it.", 8);
			}

			return;
		}

		if (ending == E_CUT)
		{
			RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
			Headline(gm, string.Format("%1 left a safehouse for the next town over three days ago. %1 never arrived.", RZ_NoteText.FirstName(rng)));
			return;
		}

		if (ending == E_RAID)
		{
			ChangeStanding(gm, run, RZ_EFaction.MILITARY, 10, 0, RZ_EQuestApproach.SIDE);
			ChangeStanding(gm, run, RZ_EFaction.BANDITS, 0, 10, RZ_EQuestApproach.SIDE);
			if (!HandCamp(gm, run))
				NudgeStrength(gm, RZ_EFaction.BANDITS, -3);

			Headline(gm, run.Fill("Bastion raided the Red Hand camp outside {crew_place}. The report says vehicles seized. It doesn't say what was in them."));
			return;
		}

		// Told
		ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
		Headline(gm, "Every Haven house lit a candle at supper for the ones who never arrived. Somebody told them where they went.");
		SpreadRumour(gm, "The Haven knows where the strays went. It's stopped putting people out at night.", 8);
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's raid: the crew's outpost goes to Bastion if Bastion is stronger by the territory's
	//! usual margin (and the usual rules allow it). False if it doesn't change hands.
	protected bool HandCamp(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Territory territory = gm.GetTerritory();
		if (!territory || !territory.IsActive())
			return false;

		RZ_Holding h = territory.FindHolding(RZ_EFaction.BANDITS, RZ_RedHandQuest.PosOf(run, "camp"), 250, true);
		if (!h || territory.GetStrength(RZ_EFaction.MILITARY) < territory.GetStrength(RZ_EFaction.BANDITS) + 10)
			return false;

		return HandOutpost(gm, h.m_iId, RZ_EFaction.MILITARY);
	}

	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		string text = super.JournalText(gm, run);
		if (run.m_iStage == S_BUYS && run.Flag("arrested") == 1)
			text += run.Fill(" {driver_short} is in a Bastion cell; he won't be naming anyone.");

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage <= S_TELL)
			return "There's a Red Hand van with the canvas tied down. It never stops for tins.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Haven council heads have lost people on the roads; the crew boss wonders where his driver goes.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (!l || run.m_iStage > S_TELL)
			return "";

		if (l.m_eFaction == RZ_EFaction.SURVIVORS && l.m_eRole == RZ_ELeaderRole.SAFEHOUSE)
			return string.Format("%1 counts the bowls on the shelf. \"Two of ours left for the next town last week. Neither of them got there. The Red Hand says it hasn't seen them.\"", l.m_sShort);

		RZ_Leader crew = RZ_RedHandQuest.Same(gm, run, "crew");
		if (crew && crew.m_iId == leaderId)
			return run.Fill(string.Format("%1 watches the road. \"{driver_short}'s been quiet lately. Quiet drivers worry me.\"", l.m_sShort));

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! Cargo, stage 1 (a Red Hand or, in the Haven's version, a Haven contract, Known; only at dusk):
//! the driver at a crossroads. Hear him out (stay within LISTEN_RADIUS of him for LISTEN_SECONDS,
//! ★ Ex-Con BONUS_SECONDS) or threaten him (at once: the camp is on guard at stage 2).
class RZ_QM_CargoDriver : RZ_QuestMission
{
	protected int m_iTalk = -1;
	protected int m_iListener = -1;
	protected float m_fListened;
	protected vector m_vDriver;

	protected static const float LISTEN_RADIUS = 8;
	protected static const float LISTEN_SECONDS = 40;
	protected static const float BONUS_SECONDS = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_Quest_Cargo.Giver(Run(gm)), RZ_ETier.KNOWN, 6, 10);
		RZ_Quest_Cargo.Voice(this, Run(gm));
		m_Req.Bonus("excon");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
		m_fSceneRadius = 300;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !RZ_RedHandQuest.HasPos(run, "cross"))
			return false;

		m_vDriver = RZ_RedHandQuest.PosOf(run, "cross");
		m_vPos = m_vDriver;
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_iTalk = AddClue(QText(gm, "driver"), QFill(gm, "{driver_short} keeps his eyes on the road while he talks, and his hands in his pockets. \"Not here. Not where they can see. Walk with me a bit.\""));
		m_sTitle = "Cargo: the driver";
		m_sObjective = QFill(gm, "Meet {driver_short} at the crossroads outside {crew_place} at dusk.");
		if (RZ_Quest_Cargo.IsHaven(run))
			m_sBrief = QFill(gm, "{witness} passed us a message from a Red Hand driver, {driver_short}. He drives one of the vans with the canvas tied down, and he says he wants out. He'll be at the crossroads outside {crew_place} at dusk. Hear him out. Be careful.");
		else
			m_sBrief = QFill(gm, "One of {crew_short}'s drivers, {driver_short}, wants a word with someone who isn't {crew_short}. Crossroads outside {crew_place}, dusk. Listen to him. Whatever he's carrying, find out.");

		// The quiet threat (Reputation > Quiet success): Fear first, before he runs to tell
		m_Ways.m_iMenaceDifficulty = RZ_ReputationSettings.QuietDifficulty();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		return SpawnPerson(gm, m_vDriver, 0, m_iTalk, "Talk to " + QText(gm, "driver_short"), "Hear him out|Threaten him") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_bResolved)
			return;

		if (option == 0)
		{
			if (m_iListener == playerId)
				return;

			m_iListener = playerId;
			m_fListened = 0;
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "heard the driver out");
			if (HasBonus(gm, playerId))
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{driver_short} sees the prison ink on your wrists and starts talking before you've asked. Stay with him."));
			else
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{driver_short} starts slowly, watching the road the whole time. Stay with him while he gets to it."));

			return;
		}

		if (option == 1)
		{
			// Quiet success: he's more afraid of you than of them, so nobody tells the crew
			if (TryMenace(gm, playerId, "cargo.driver", RZ_EFaction.BANDITS, false))
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You say it once, quietly. {driver_short} tells you all of it: the vans, the strays, where they go. Then he goes back to his van as if nothing happened. He won't be telling anyone. He's more scared of you than of them."));
				Resolve(gm, playerId, RZ_EQuestApproach.FORCE, RZ_Quest_Cargo.OUT_MET, "threatened the driver quietly");
				return;
			}

			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You don't give him time to be careful. It all comes out at once: the vans, the strays, where they go. Then he's gone into the dusk, and you can guess who he's running to tell."));
			Resolve(gm, playerId, RZ_EQuestApproach.FORCE, RZ_Quest_Cargo.OUT_THREATENED, "threatened the driver");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iListener <= 0 || m_bResolved)
			return;

		IEntity listener = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iListener);
		if (!listener || RZ_Util.DistXZ(listener.GetOrigin(), m_vDriver) > LISTEN_RADIUS)
		{
			m_iListener = -1;
			m_fListened = 0;
			return;
		}

		m_fListened += dt;
		float needed = LISTEN_SECONDS;
		if (HasBonus(gm, m_iListener))
			needed = BONUS_SECONDS;

		if (m_fListened < needed)
			return;

		gm.NotifyPlayer(m_iListener, m_sTitle, QFill(gm, "He tells you all of it: who rides in the back, and where they get off. \"There's a van at the camp now. Padlocked. I can't. Somebody has to.\""));
		Resolve(gm, m_iListener, RZ_EQuestApproach.TALK, RZ_Quest_Cargo.OUT_MET, "heard the driver out");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Cargo: nobody met {driver_short} at the crossroads.");
	}
}

//------------------------------------------------------------------------------------------------
//! Cargo, stage 2 (contract, Known): the van at the crew's camp (RZ_CargoVan: Supply > Red Hand
//! truck, no fuel, padlocked, guarded; on guard if the driver was threatened). Cut the lock and the
//! three come out and follow the cutter (RZ_Escort) to a safehouse: +3 people, +10 Red Hand Infamy.
//! Or get the engine going at the bonnet (START_SECONDS within reach; ★ Mechanic at once; the
//! guards hear it) and drive the van, still locked, to the drop (delivered), to a Bastion seat
//! (Bastion: the driver is arrested) or to the safehouse (freed).
class RZ_QM_CargoLoad : RZ_QuestMission
{
	protected ref RZ_CargoVan m_Load = new RZ_CargoVan();
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected ref RZ_VehicleZone m_DropZone = new RZ_VehicleZone();
	protected ref RZ_VehicleZone m_BastionZone = new RZ_VehicleZone();
	protected vector m_vVan;
	protected vector m_vDrop;
	protected vector m_vBastion;
	protected vector m_vRefuge;
	protected string m_sRefuge;
	protected string m_sBastion;
	protected bool m_bHasBastion;
	protected bool m_bShown;
	protected bool m_bStarted;
	protected bool m_bOut;
	protected int m_iStarter = -1;
	protected float m_fStart;
	protected int m_iCutter = -1;
	protected int m_iDoorTalk = -1;
	protected int m_iBonnetTalk = -1;

	protected static const int OBJ_DOOR = 0;
	protected static const int OBJ_BONNET = 1;
	protected static const float START_SECONDS = 30;
	protected static const float REACH = 4;
	protected static const int FREE_INFAMY = 10;
	protected static const int SIDE_FAME = 8;
	protected static const int SIDE_INFAMY = 10;
	protected static const int PEOPLE = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_Quest_Cargo.Giver(Run(gm)), RZ_ETier.KNOWN, 14, 20);
		RZ_Quest_Cargo.Voice(this, Run(gm));
		m_Req.Bonus("mechanic");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_fSceneRadius = 300;
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsVehicle()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		if (m_Load.m_Van)
			result.Insert(m_Load.m_Van.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !RZ_RedHandQuest.HasPos(run, "camp") || !RZ_RedHandQuest.HasPos(run, "drop"))
			return false;

		vector camp = RZ_RedHandQuest.PosOf(run, "camp");
		// The van is parked well clear of where it's going (the crew's own pit can be close by)
		m_vDrop = RZ_RedHandQuest.PosOf(run, "drop");
		m_vVan = LandPointNear(camp, 60, 140);
		for (int tries = 0; tries < 8 && RZ_Util.DistXZ(m_vVan, m_vDrop) < 150; tries++)
		{
			m_vVan = LandPointNear(camp, 60, 140);
		}
		if (!RZ_RedHandEdges.HavenRefuge(gm, m_vVan, m_vRefuge, m_sRefuge))
			return false;

		m_bHasBastion = RZ_RedHandQuest.FactionSeat(gm, RZ_EFaction.MILITARY, camp, m_vBastion, m_sBastion);
		m_iDoorTalk = AddClue("Through the canvas", "Three voices, one of them a child's. \"Is it a checkpoint? Are we there? Nobody tells us anything.\"");
		m_iBonnetTalk = AddClue("The van", "Old, patched, and the starter motor's on its last legs. The keys are in it. Nobody steals from the Red Hand.");
		m_Escort.m_sName = "the three from the van";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.m_bHide = true;
		if (!m_Escort.Begin(gm, m_vVan))
			return false;

		m_vPos = m_vVan;
		m_iSettlement = gm.FindNearestSettlement(camp);
		m_sTitle = "Cargo: the load";
		m_sObjective = QFill(gm, "Reach the van with three people in it at {crew_short}'s camp outside {crew_place}.");
		if (RZ_Quest_Cargo.IsHaven(run))
			m_sBrief = QFill(gm, "The van is at {crew_short}'s camp outside {crew_place}, padlocked, with three people in the back. Get them out and bring them to us. Please don't let them see you coming.");
		else
			m_sBrief = QFill(gm, "There's a van at {crew_short}'s camp outside {crew_place} with three people in the back. It's meant to go to {drop}. What happens to it now is up to whoever gets there first.");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		if (m_bOut)
			return text + string.Format("\nWalk them to: %1 (%2)", m_sRefuge, RZ_Util.DirectionText(from, m_vRefuge));

		if (!m_bStarted)
			return text;

		text += string.Format("\nThe drop: %1 (%2)", QText(gm, "drop"), RZ_Util.DirectionText(from, m_vDrop));
		text += string.Format("\nThe Haven: %1 (%2)", m_sRefuge, RZ_Util.DirectionText(from, m_vRefuge));
		if (m_bHasBastion)
			text += string.Format("\nBastion: %1 (%2)", m_sBastion, RZ_Util.DirectionText(from, m_vBastion));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		m_bShown = true;
		RZ_QuestRun run = Run(gm);
		bool onGuard = run && run.Flag("on_guard") == 1;
		if (!m_Load.Park(gm, m_vVan, Math.RandomFloat(0, 360), onGuard))
			return false;

		TrackProp(m_Load.m_Guards);
		m_Load.m_Zone.SetZone(m_vRefuge, 30);
		m_DropZone.SetVehicle(m_Load.m_Van);
		m_DropZone.SetZone(m_vDrop, 30);
		m_BastionZone.SetVehicle(m_Load.m_Van);
		if (m_bHasBastion)
			m_BastionZone.SetZone(m_vBastion, 40);

		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_Load.m_vDoor, OBJ_DOOR, "", "Cut the lock", m_iDoorTalk, "Listen at the canvas");
		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, LandPointNear(m_vVan, 2, 4), OBJ_BONNET, "Get the engine going", "", m_iBonnetTalk, "Look at the van");
		if (onGuard)
			RZ_ContractHelp.NotifyNear(gm, m_vVan, 300, m_sTitle, "The camp is awake and watching the road. Somebody told them to expect you.");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_DOOR || option != 0 || m_bResolved || m_bOut)
			return;

		string text = m_Load.StartCut(playerId);
		if (text.IsEmpty())
			return;

		m_iCutter = playerId;
		Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "went for the padlock");
		gm.NotifyPlayer(playerId, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_BONNET || value != 0 || m_bResolved || m_bOut || m_bStarted)
			return;

		if (HasBonus(gm, playerId))
		{
			m_iStarter = playerId;
			Started(gm, "You know this engine. Two wires, a tap with the spanner, and it catches first time.");
			return;
		}

		if (m_iStarter == playerId)
			return;

		m_iStarter = playerId;
		m_fStart = 0;
		gm.NotifyPlayer(playerId, m_sTitle, "The starter grinds and grinds. Keep at it.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Started(RZ_GameModeComponent gm, string text)
	{
		m_bStarted = true;
		m_Load.Fuel(0.4);
		m_Load.Alarm();
		Remember(gm, m_iStarter, RZ_EQuestApproach.NONE, "got the van going");
		NotifyParticipants(gm, m_sTitle, text + " The camp heard that. Drive.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved || !m_bShown)
			return;

		if (m_bOut)
		{
			UpdateWalk(gm, dt, ids, players);
			return;
		}

		if (m_Load.IsLost())
		{
			NotifyParticipants(gm, m_sTitle, "The van is finished, and nobody in the back made it out.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_Load.UpdateCut(dt))
		{
			LetOut(gm);
			return;
		}

		UpdateStarting(gm, dt);
		if (!m_bStarted)
			return;

		m_vPos = m_Load.m_Van.GetOrigin();
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 8, m_aParticipants);
		if (m_Load.m_Zone.Check(ids, players))
		{
			Freed(gm, m_Load.m_Zone.m_iDriver, "drove them to the Haven");
			return;
		}

		if (m_DropZone.Check(ids, players))
		{
			Delivered(gm, m_DropZone.m_iDriver);
			return;
		}

		if (m_bHasBastion && m_BastionZone.Check(ids, players))
			ToBastion(gm, m_BastionZone.m_iDriver);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateStarting(RZ_GameModeComponent gm, float dt)
	{
		if (m_bStarted || m_iStarter <= 0)
			return;

		IEntity starter = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iStarter);
		if (!starter || RZ_Util.DistXZ(starter.GetOrigin(), m_vVan) > REACH + 3)
		{
			m_iStarter = -1;
			m_fStart = 0;
			return;
		}

		m_fStart += dt;
		if (m_fStart >= START_SECONDS)
			Started(gm, "The engine coughs, dies, coughs, and catches.");
	}

	//------------------------------------------------------------------------------------------------
	//! The lock gives: they come out and follow the cutter to the Haven.
	protected void LetOut(RZ_GameModeComponent gm)
	{
		m_bOut = true;
		m_iCutter = m_Load.m_iCutter;
		m_Escort.Follow(m_iCutter);
		m_Escort.Go();
		RZ_ContractHelp.NotifyNear(gm, m_vVan, 80, m_sTitle, string.Format("The padlock gives. Two men and a boy climb down, blinking, holding on to each other. Walk them to %1. They'll keep up.", m_sRefuge));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateWalk(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_Escort.IsDead())
		{
			NotifyParticipants(gm, m_sTitle, "They didn't make it to the Haven.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_Escort.IsNear(m_vRefuge, 25))
			return;

		m_Escort.Arrive();
		int who = m_Escort.GetFollowed();
		if (who <= 0)
			who = m_iCutter;

		Freed(gm, who, "walked them to the Haven");
	}

	//------------------------------------------------------------------------------------------------
	//! At the Haven: +3 people at that safehouse, +10 Red Hand Infamy for whoever brought them.
	protected void Freed(RZ_GameModeComponent gm, int playerId, string what)
	{
		RZ_Questline.ChangePopulation(gm, RZ_RedHandQuest.HouseAt(gm, m_vRefuge), PEOPLE);
		RZ_RedHandQuest.SetWorld(gm, "cargo.freed", 1);
		RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.BANDITS, 0, FREE_INFAMY);
		NotifyParticipants(gm, m_sTitle, string.Format("The Haven's people come out with blankets. The boy won't let go of his father's coat. (+%1 Red Hand Infamy for whoever brought them)", FREE_INFAMY));
		RZ_QuestRun run = Run(gm);
		bool haven = RZ_Quest_Cargo.IsHaven(run);
		if (!haven && run)
			run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, 5, 10, "The Haven heard who brought three people in off a Red Hand van.");

		Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_Cargo.OUT_FREED, what, haven);
	}

	//------------------------------------------------------------------------------------------------
	//! At the drop: whoever drove it saw who took them.
	protected void Delivered(RZ_GameModeComponent gm, int playerId)
	{
		RZ_QuestRun run = Run(gm);
		RZ_RedHandQuest.SetWorld(gm, "cargo.delivered", 1);
		string seen = "A man comes down from the farm with a lantern and a crate that clinks: jars. He counts the three off the back like he's done it before. He has.";
		if (!RZ_Quest_Cargo.IsFarm(run))
			seen = "The pit boss counts the three off the back into the pens and pays the crew boss's man out of the bets. Nobody in the pens owes the book a thing.";

		bool haven = RZ_Quest_Cargo.IsHaven(run);
		if (haven)
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, RZ_EFaction.SURVIVORS, SIDE_FAME, SIDE_INFAMY);

		gm.NotifyPlayer(playerId, m_sTitle, "You drove them where they were going. " + seen);
		Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_Cargo.OUT_DELIVERED, "drove the van to the drop", !haven);
	}

	//------------------------------------------------------------------------------------------------
	//! At a Bastion seat: three civilians sheltered, and the driver arrested.
	protected void ToBastion(RZ_GameModeComponent gm, int playerId)
	{
		RZ_RedHandQuest.SetWorld(gm, "cargo.bastion", 1);
		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, SIDE_FAME, SIDE_INFAMY);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "A sergeant opens the back, looks for a long time, and writes three lines in his book: civilians sheltered. Then he sends a section for {driver_short}."));
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_Cargo.OUT_BASTION, "drove the van to Bastion", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Cargo: the van left the camp with its load.";
	}

	//------------------------------------------------------------------------------------------------
	//! The van at the camp; once it runs (or they're out on foot), where the asker wants them: the
	//! Haven's refuge, else the drop it was meant for.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_bOut)
			return RZ_ObjPlace.Spot(m_vRefuge, 25);

		if (m_bStarted && m_Load.m_Van)
		{
			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!me || RZ_Util.GetVehicleOf(me) != m_Load.m_Van)
				return RZ_ObjPlace.Spot(m_Load.m_Van.GetOrigin());

			if (RZ_Quest_Cargo.IsHaven(Run(gm)))
				return RZ_ObjPlace.Spot(m_vRefuge, 30);

			return RZ_ObjPlace.Spot(m_vDrop, m_DropZone.m_fRadius);
		}

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
		m_Load.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! Cargo, stage 3 (contract, Known): who buys. Watch the drop after dark from between WATCH_NEAR
//! and WATCH_FAR for WATCH_SECONDS, unseen (★ Police: BONUS_WATCH); ask the fence at the Red Hand
//! stronghold (persuade 2 with the Red Hand: Trusted); or pay the driver (10 Red Hand Favor) to
//! name his crew boss (not while he's in a Bastion cell).
class RZ_QM_CargoBuys : RZ_RedHandQuestMission
{
	protected vector m_vDrop;
	protected int m_iDropSpot = -1;
	protected int m_iFenceSpot = -1;
	protected int m_iDriverSpot = -1;
	protected int m_iWatcher = -1;
	protected float m_fWatched;
	protected bool m_bWarned;

	protected static const int OBJ_FENCE = 0;
	protected static const int OBJ_DRIVER = 1;
	protected static const float WATCH_NEAR = 60;
	protected static const float WATCH_FAR = 150;
	protected static const float WATCH_SECONDS = 120;
	protected static const float BONUS_WATCH = 60;
	protected static const int DRIVER_FAVOR = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_Quest_Cargo.Giver(Run(gm)), RZ_ETier.KNOWN, 10, 15);
		RZ_Quest_Cargo.Voice(this, Run(gm));
		m_Req.Bonus("police");
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !RZ_RedHandQuest.HasPos(run, "drop"))
			return false;

		m_vDrop = RZ_RedHandQuest.PosOf(run, "drop");
		m_iDropSpot = AddSpot(m_vDrop, "The drop, " + run.Text("drop") + " (watch from 60-150 m, after dark)");
		RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
		if (boss)
			m_iFenceSpot = AddSpot(LandPointNear(boss.m_vPos, 8, 16), "The fence at " + boss.m_sPlace);

		if (run.Flag("arrested") != 1 && RZ_RedHandQuest.HasPos(run, "cross"))
			m_iDriverSpot = AddSpot(RZ_RedHandQuest.PosOf(run, "cross"), run.Fill("{driver_short} at the crossroads"));

		m_Quiet.Watch(m_vDrop, WATCH_FAR);
		m_vPos = m_vDrop;
		m_iSettlement = gm.FindNearestSettlement(m_vDrop);
		m_sTitle = "Cargo: who buys";
		m_sBrief = QFill(gm, "Somebody buys what's in those vans. Watch the drop at {drop} after dark without being seen, ask the Red Hand's fence, or pay {driver_short} to say who.");
		m_sObjective = QFill(gm, "Find out who buys the vans' cargo at the drop at {drop}.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SpawnSpot(RZ_GameModeComponent gm, int spot)
	{
		if (spot == m_iFenceSpot)
		{
			int fence = AddClue("The fence", "He's weighing watches on a kitchen scale. \"Buying or selling? Or just asking? Asking costs the most.\"");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_FENCE, "", "Ask who buys cargo", fence, "Talk to the fence");
			return;
		}

		if (spot == m_iDriverSpot)
		{
			int driver = AddClue(QText(gm, "driver"), QFill(gm, "{driver_short} won't look at you. \"I drive. I don't ask who pays.\" He's lying, and he wants you to know it."));
			string choices = string.Format("Pay him to say who (%1 Red Hand Favor)", DRIVER_FAVOR);
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_DRIVER, "", choices, driver, "Talk to " + QText(gm, "driver_short"));
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		TickSpots(gm, players);
		UpdateWatch(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! After dark, someone between WATCH_NEAR and WATCH_FAR of the drop who hasn't been noticed.
	protected void UpdateWatch(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!RZ_RedHandQuest.IsDark())
			return;

		int watcher = -1;
		foreach (int i, IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), m_vDrop);
			if (d > WATCH_FAR)
				continue;

			if (d < WATCH_NEAR)
			{
				m_Quiet.Spot(ids[i]);
				if (!m_bWarned)
				{
					m_bWarned = true;
					gm.NotifyPlayer(ids[i], m_sTitle, "Too close: somebody at the drop has seen a shape where there shouldn't be one. Nothing will happen here tonight with you watching.");
				}

				continue;
			}

			if (m_Quiet.IsQuiet(gm, ids[i]))
			{
				watcher = ids[i];
				break;
			}
		}

		if (watcher <= 0)
			return;

		if (watcher != m_iWatcher)
		{
			m_iWatcher = watcher;
			Remember(gm, watcher, RZ_EQuestApproach.SNEAK, "watched the drop");
		}

		m_fWatched += dt;
		float needed = WATCH_SECONDS;
		if (HasBonus(gm, watcher))
			needed = BONUS_WATCH;

		if (m_fWatched < needed)
			return;

		string seen = "The van comes in after midnight. A man walks down from the farm with a lantern and a crate that clinks: jars. The back of the van is empty when it leaves.";
		if (!RZ_Quest_Cargo.IsFarm(Run(gm)))
			seen = "The van comes in after midnight. The pit boss counts the load off the back into the pens, and pays the crew boss's man out of a tin. None of them is in the book.";

		gm.NotifyPlayer(watcher, m_sTitle, seen);
		Resolve(gm, watcher, RZ_EQuestApproach.SNEAK, RZ_Quest_Cargo.OUT_KNOWN, "watched the drop unseen");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_bResolved || option != 0)
			return;

		RZ_QuestRun run = Run(gm);
		bool farm = RZ_Quest_Cargo.IsFarm(run);
		if (objIdx == OBJ_FENCE)
		{
			if (!TryPersuade(gm, playerId, RZ_EFaction.BANDITS, 2, ""))
			{
				Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked the fence");
				gm.NotifyPlayer(playerId, m_sTitle, "Who buys what's none of your business, friend. Come back when you're somebody.");
				return;
			}

			string said = "He says \"pork\" and laughs, and keeps laughing longer than it's funny.";
			if (!farm)
				said = "He says \"pit fodder\" and laughs. \"Ask who keeps the bets. Go on.\"";

			gm.NotifyPlayer(playerId, m_sTitle, said);
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Cargo.OUT_KNOWN, "got it out of the fence");
			return;
		}

		if (objIdx == OBJ_DRIVER)
		{
			if (!TryBribe(gm, playerId, RZ_EFaction.BANDITS, DRIVER_FAVOR))
				return;

			string named = QFill(gm, "\"{crew_short}. It's always {crew_short}. They go to a farm past the last village, and the man there pays in jars.\" {driver_short} wipes his mouth like he's tasted something.");
			if (!farm)
				named = QFill(gm, "{crew_short}. The ones who owe nothing go in the crew's own pit, and the bets go in {crew_short}'s tin.");

			gm.NotifyPlayer(playerId, m_sTitle, named);
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_Cargo.OUT_KNOWN, "paid the driver to name names");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Cargo: nobody found out who buys.";
	}
}

//------------------------------------------------------------------------------------------------
//! Cargo, stage 4 (contract, Known): who hears it. The boss's second at the stronghold (Trusted with
//! the Red Hand): a purge. The crew boss (held while his stand-in talks): keep quiet for a cut.
//! Bastion's duty officer: the camp (and the farm's grid). The Haven chair's table: where the strays
//! go. Whose contract it is decides who pays (the Red Hand: only the purge; the Haven: all but the cut).
class RZ_QM_CargoTell : RZ_RedHandQuestMission
{
	protected int m_iBossSpot = -1;
	protected int m_iCrewSpot = -1;
	protected int m_iBastionSpot = -1;
	protected int m_iHavenSpot = -1;

	protected static const int OBJ_BOSS = 0;
	protected static const int OBJ_CREW = 1;
	protected static const int OBJ_BASTION = 2;
	protected static const int OBJ_HAVEN = 3;
	protected static const int PURGE_FAVOR = 15;
	protected static const int RAID_FAME = 10;
	protected static const int RAID_INFAMY = 15;
	protected static const int HAVEN_FAME = 5;
	protected static const int HAVEN_INFAMY = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_Quest_Cargo.Giver(Run(gm)), RZ_ETier.KNOWN, 10, 15);
		RZ_Quest_Cargo.Voice(this, Run(gm));
		m_fTimeLeft = 4200;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader crew = RZ_RedHandQuest.Same(gm, run, "crew");
		if (!run || !crew)
			return false;

		RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
		if (boss)
			m_iBossSpot = AddSpot(LandPointNear(boss.m_vPos, 8, 16), "The boss's second at " + boss.m_sPlace);

		m_iCrewSpot = AddSpot(LandPointNear(crew.m_vPos, 3, 6), string.Format("%1 at %2", crew.m_sName, crew.m_sPlace));
		vector seat;
		string seatName;
		if (RZ_RedHandQuest.FactionSeat(gm, RZ_EFaction.MILITARY, crew.m_vPos, seat, seatName))
			m_iBastionSpot = AddSpot(LandPointNear(seat, 8, 16), "Bastion's duty officer at " + seatName);

		RZ_Leader chair = gm.GetLeaders().GetStronghold(RZ_EFaction.SURVIVORS);
		if (chair && chair.IsPresent())
			m_iHavenSpot = AddSpot(LandPointNear(chair.m_vPos, 8, 16), "The Haven chair's table at " + chair.m_sPlace);

		m_vPos = m_aSpots[m_iCrewSpot];
		m_iSettlement = gm.FindNearestSettlement(crew.m_vPos);
		m_sTitle = "Cargo: tell someone";
		m_sBrief = QFill(gm, "{buyer_line} Somebody has to hear it. The boss, if you can get a hearing. {crew_short}, if you want paying to forget it. Bastion. Or the Haven's chair.");
		m_sObjective = QFill(gm, "Tell someone who buys the cargo: the boss, {crew_short}, Bastion or the Haven's chair.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SpawnSpot(RZ_GameModeComponent gm, int spot)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return;

		bool farm = RZ_Quest_Cargo.IsFarm(run);
		if (spot == m_iBossSpot)
		{
			int second = AddClue("The boss's second", "The boss is busy. Whatever it is, tell me, and I'll decide if it's worth the boss's time.");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_BOSS, "", run.Fill("Tell the boss what {crew_short} sells"), second, "Talk to the boss's second");
			return;
		}

		if (spot == m_iCrewSpot)
		{
			RZ_Leader crew = RZ_RedHandQuest.Same(gm, run, "crew");
			if (!crew)
				return;

			if (crew.m_iHeldBy < 0)
				HoldLeader(gm, crew.m_iId);

			int talk = AddClue(crew.m_sName, run.Fill("{crew_short} doesn't get up. \"I hear you've been following my van. That's a hobby that costs. Or pays, if you're sensible.\""));
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_CREW, "", "Keep quiet, for a cut", talk, "Talk to " + crew.m_sShort);
			return;
		}

		if (spot == m_iBastionSpot)
		{
			string label = "Give them the camp";
			if (farm)
				label = "Give them the camp, and the farm";

			int officer = AddClue("The duty officer", "\"Name and business.\" He stops writing when you say the word van.");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_BASTION, "", label, officer, "Talk to the duty officer");
			return;
		}

		if (spot == m_iHavenSpot)
		{
			int clerk = AddClue("The chair's table", "A man is writing the names of the missing on the back of a ration list. The list is long.");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_HAVEN, "", "Tell the chair where the strays go", clerk, "Talk to the man at the table");
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
		if (!run || m_bResolved || option != 0)
			return;

		bool haven = RZ_Quest_Cargo.IsHaven(run);
		bool farm = RZ_Quest_Cargo.IsFarm(run);
		if (objIdx == OBJ_BOSS)
		{
			if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "\"The boss doesn't hear crew business from strangers.\" He turns his back.");
				return;
			}

			RZ_RedHandQuest.SetWorld(gm, "cargo.purge", 1);
			run.Reward(gm, playerId, RZ_EFaction.BANDITS, 0, PURGE_FAVOR, "The boss heard it from you first.");
			gm.NotifyPlayer(playerId, m_sTitle, "The second goes inside. When he comes back, he's not smiling. \"The code has five rules and none of them is about this. The boss says it doesn't need one.\"");
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Cargo.OUT_PURGE, "told the boss", true);
			return;
		}

		if (objIdx == OBJ_CREW)
		{
			run.SetText("cut_ident", gm.GetProfile(playerId).m_sIdentity);
			RZ_RedHandQuest.SetWorld(gm, "cargo.cut", 1);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{crew_short} smiles, not all the way. \"Sensible. My man will find you once a session.\" You walk away, and the van goes past you on the road."));
			Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_Cargo.OUT_CUT, "took a cut to keep quiet", false);
			return;
		}

		if (objIdx == OBJ_BASTION)
		{
			RZ_RedHandQuest.SetWorld(gm, "cargo.bastion_raid", 1);
			if (farm)
				RZ_RedHandQuest.SetWorld(gm, "table.bastion_knows", 1);

			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RAID_FAME, RAID_INFAMY);
			string given = "He writes down the camp's grid twice.";
			if (farm)
				given = "He writes down the camp's grid, and the farm's, and underlines the farm.";

			gm.NotifyPlayer(playerId, m_sTitle, given + string.Format(" \"Dawn,\" is all he says. (+%1 Red Hand Infamy)", RAID_INFAMY));
			if (haven)
				m_fRewardScale = 0.5;

			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_Cargo.OUT_RAID, "gave the camp to Bastion", haven);
			return;
		}

		if (objIdx == OBJ_HAVEN)
		{
			RZ_RedHandQuest.SetWorld(gm, "cargo.told_haven", 1);
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.SURVIVORS, RZ_EFaction.BANDITS, HAVEN_FAME, HAVEN_INFAMY);
			string told = "He stops writing and listens to all of it. Then he writes one more line, at the top.";
			if (farm)
				told = "He stops writing when you say the farm's grid. \"That was one of ours. On Day 3 we sent it our mouths.\" He puts the pen down.";

			gm.NotifyPlayer(playerId, m_sTitle, told + string.Format(" (+%1 Red Hand Infamy)", HAVEN_INFAMY));
			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_Cargo.OUT_HAVEN, "told the Haven", haven);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Cargo: nobody told anyone what's in the vans.";
	}
}

//------------------------------------------------------------------------------------------------
//! Cargo, stage 5 (contract, Known): the driver. Usually at the crossroads: get him out (he follows
//! you to the Haven: +1 person) or tell him to keep driving. After a purge he's at the boss's pit:
//! take his place (RING_SECONDS in the ring, ROUNDS rounds), get him out, or let it be. After
//! Bastion's raid: assault the crew's camp with Bastion (its Red Hand group must all die).
class RZ_QM_CargoFate : RZ_QuestMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected ref RZ_UnitWatch m_Crew = new RZ_UnitWatch();
	protected vector m_vDriver;
	protected vector m_vRefuge;
	protected string m_sRefuge;
	protected IEntity m_Driver;
	protected int m_iBranch;
	protected int m_iPath;
	protected int m_iSecond = -1;
	protected float m_fHeld;
	protected int m_iRounds;
	protected int m_iTalk = -1;

	protected static const int PATH_NONE = 0;
	protected static const int PATH_OUT = 1;
	protected static const int PATH_RING = 2;
	protected static const float RING_SECONDS = 240;
	protected static const float RING_RADIUS = 25;
	protected static const int ROUNDS = 4;
	protected static const float FIRST_ROUND = 20;
	protected static const float ROUND_GAP = 55;
	protected static const int ROUND_SIZE = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_Quest_Cargo.Giver(Run(gm)), RZ_ETier.KNOWN, 8, 10);
		RZ_Quest_Cargo.Voice(this, Run(gm));
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
		m_fSceneRadius = 300;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		m_iBranch = run.m_iBranch;
		if (m_iBranch == RZ_Quest_Cargo.OUT_RAID)
		{
			m_vDriver = LandPointNear(RZ_RedHandQuest.PosOf(run, "camp"), 20, 40);
			m_sTitle = "Cargo: the raid";
			m_sBrief = QFill(gm, "Bastion goes into {crew_short}'s camp outside {crew_place} at dawn, and wants it quiet when it gets there. Go in first. Everyone in that crew who's holding a gun is a problem.");
			m_sObjective = QFill(gm, "Clear the guns out of {crew_short}'s camp outside {crew_place} before Bastion's dawn raid.");
		}
		else if (m_iBranch == RZ_Quest_Cargo.OUT_PURGE)
		{
			RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
			if (!boss)
				return false;

			m_vDriver = LandPointNear(boss.m_vPos, 60, 150);
			run.SetText("drop_pit", string.Format("the garages behind %1's camp in %2", boss.m_sShort, boss.m_sPlace));
			m_sTitle = "Cargo: the driver";
			m_sBrief = QFill(gm, "The boss is tidying up, and {driver_short} is part of the mess. He goes in the pit at {drop_pit} tonight. Take his place, get him out, or let it be.");
			m_sObjective = QFill(gm, "Get {driver_short} out of the pit at {drop_pit} tonight, or take the driver's place.");
		}
		else
		{
			m_vDriver = RZ_RedHandQuest.PosOf(run, "cross");
			m_sTitle = "Cargo: the driver";
			m_sBrief = QFill(gm, "{driver_short} is at the crossroads outside {crew_place}, waiting to hear what happens to him now. Get him out to the Haven, or tell him to keep driving.");
			m_sObjective = QFill(gm, "Get {driver_short} from the crossroads outside {crew_place} to the Haven, or send the driver on.");
		}

		if (m_vDriver == vector.Zero)
			return false;

		if (m_iBranch != RZ_Quest_Cargo.OUT_RAID && !RZ_RedHandEdges.HavenRefuge(gm, m_vDriver, m_vRefuge, m_sRefuge))
			return false;

		m_vPos = m_vDriver;
		m_iSettlement = gm.FindNearestSettlement(m_vDriver);
		m_iTalk = AddClue(QText(gm, "driver"), QFill(gm, "{driver_short} has a bag packed, and he keeps looking at it. \"Well?\""));
		m_Escort.m_sName = QText(gm, "driver_short");
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.m_bHide = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (m_iBranch == RZ_Quest_Cargo.OUT_RAID)
		{
			if (!m_Crew.Spawn(gm.GetRedHandGroupPrefab(), LandPointNear(m_vDriver, 5, 15)))
				return false;

			TrackProp(m_Crew.m_Unit);
			if (gm.GetHostility())
				gm.GetHostility().WatchUnit(m_Crew.m_Unit, RZ_EFaction.BANDITS);

			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew.m_Unit, 200);
			return true;
		}

		string choices = "Get him out of here|Tell him to keep driving";
		if (m_iBranch == RZ_Quest_Cargo.OUT_PURGE)
			choices = "Get him out of here|Let it be|Take his place in the pit";

		m_Driver = SpawnPerson(gm, m_vDriver, 0, m_iTalk, "Talk to " + QText(gm, "driver_short"), choices);
		return m_Driver != null;
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
			if (!m_Escort.Begin(gm, m_vDriver))
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{driver_short} shakes his head. \"Not with this many eyes on the road. A few minutes.\""));
				return;
			}

			m_iPath = PATH_OUT;
			m_iSecond = playerId;
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "walked the driver out");
			RZ_Util.DeleteUnit(m_Driver);
			m_Escort.Follow(playerId);
			m_Escort.Go();
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, string.Format("{driver_short} picks up the bag. \"Don't take the road. They know the road.\" Take him to %1.", m_sRefuge)));
			return;
		}

		if (m_iBranch == RZ_Quest_Cargo.OUT_PURGE)
		{
			if (option == 1)
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You let it be. {driver_short} goes in with his hands empty and his eyes shut."));
				Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_Cargo.OUT_DRIVER_PIT, "let it be");
				return;
			}

			if (option == 2)
			{
				m_iPath = PATH_RING;
				m_iSecond = playerId;
				Remember(gm, playerId, RZ_EQuestApproach.FORCE, "took the driver's place in the pit");
				RZ_Util.DeleteUnit(m_Driver);
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You climb into the ring instead of him. The crowd likes that better. Four rounds, and {driver_short} walks."));
			}

			return;
		}

		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{driver_short} looks at the bag, then at the van. \"Right.\" He drives off toward the camp without looking back."));
			Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_Cargo.OUT_DRIVER_STAYS, "told him to keep driving");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (m_iBranch == RZ_Quest_Cargo.OUT_RAID)
		{
			UpdateRaid(gm, dt, ids, players);
			return;
		}

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
			NotifyParticipants(gm, m_sTitle, QFill(gm, "{driver_short} didn't make it to the Haven."));
			Resolve(gm, m_iSecond, RZ_EQuestApproach.SNEAK, RZ_Quest_Cargo.OUT_DRIVER_PIT, "tried to get him out", false);
			return;
		}

		if (!m_Escort.IsNear(m_vRefuge, 25))
			return;

		m_Escort.Arrive();
		RZ_Questline.ChangePopulation(gm, RZ_RedHandQuest.HouseAt(gm, m_vRefuge), 1);
		NotifyParticipants(gm, m_sTitle, QFill(gm, "The Haven takes {driver_short} in. He eats the bread and leaves the soup."));
		bool paid = RZ_Quest_Cargo.IsHaven(Run(gm));
		Resolve(gm, m_iSecond, RZ_EQuestApproach.SNEAK, RZ_Quest_Cargo.OUT_DRIVER_OUT, "got the driver to the Haven", paid);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRaid(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_ContractHelp.AddNear(ids, players, m_vDriver, 150, m_aParticipants);
		if (!m_Crew.IsDead(dt))
			return;

		int who = RZ_EscortHelp.PlayerBeside(ids, players, m_vDriver, 200);
		if (who <= 0)
			return;

		foreach (int playerId : m_aParticipants)
		{
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "went into the camp ahead of Bastion");
		}

		NotifyParticipants(gm, m_sTitle, QFill(gm, "The camp is quiet. Bastion's section walks in behind you and starts counting. They find {driver_short} in the cab of the van, hands up, and put him in the back of their own truck."));
		Resolve(gm, who, RZ_EQuestApproach.FORCE, RZ_Quest_Cargo.OUT_RAIDED, "went into the camp ahead of Bastion", RZ_Quest_Cargo.IsHaven(Run(gm)));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRing(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!RZ_ContractHelp.AnyNear(players, m_vDriver, RING_RADIUS))
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vDriver, RING_RADIUS, m_aParticipants);
		m_fHeld += dt;
		if (m_iRounds < ROUNDS && m_fHeld >= FIRST_ROUND + m_iRounds * ROUND_GAP)
		{
			m_iRounds++;
			SpawnGuards(gm, m_vDriver, ROUND_SIZE, 20, 35);
			NotifyParticipants(gm, m_sTitle, string.Format("Round %1. The boss is watching from the garage door, not smiling.", m_iRounds));
		}

		if (m_fHeld < RING_SECONDS)
			return;

		NotifyParticipants(gm, m_sTitle, QFill(gm, "Last bell. The boss's word was given in front of the crowd: {driver_short} walks. He doesn't thank you. He can't, yet."));
		Resolve(gm, m_iSecond, RZ_EQuestApproach.FORCE, RZ_Quest_Cargo.OUT_DRIVER_PLACE, "took the driver's place and walked out");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Cargo: nobody went back for {driver_short}.");
	}

	//------------------------------------------------------------------------------------------------
	//! The driver by the pit (the ring and the raid are there too); walking him out, the refuge.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_iPath == PATH_OUT && !m_Escort.IsArrived())
			return RZ_ObjPlace.Spot(m_vRefuge, 25);

		if (m_iPath == PATH_RING || m_iBranch == RZ_Quest_Cargo.OUT_RAID)
			return RZ_ObjPlace.Spot(m_vDriver, 25);

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}
