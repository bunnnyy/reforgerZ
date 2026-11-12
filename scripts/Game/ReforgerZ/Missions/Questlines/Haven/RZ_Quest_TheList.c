//! The List (the Haven): questline H1 (docs/QUESTLINES.md, "H1. The List"). Every Haven house keeps
//! a list of names, and some are quietly crossed out. Who crosses them out, and why: the Haven's
//! oldest shame, a farm past the last village that the council sent its "mouths" to in the first
//! hungry week, and then crossed off. One crossed-out name is the contract The List
//! (Missions/Haven/RZ_HavenLoreMissions.c); this questline opens once that's been done this season
//! (world flag "list.contract"), or once a safehouse has gone hungry, from an Independent or Rogue
//! council head.
//!
//! 1. Find the husband (mission): his name was crossed out while he was still out on a supply run.
//!    His trail (a note, his pack, a third clue): half the time he's alive and comes home. A
//!    Scavenger sells his last sighting for one food item; ★ Radio Op hears his last call.
//! 2. Three copies of the List (mission): the asking house's own list is the first. Ask other
//!    council heads (Merciful: yes; Loyal: only if you're Trusted; Paranoid: no; the rest: Known),
//!    read a house's list in its stores after dark past two lookouts, pay a house's stores 3 food,
//!    or buy the Red Hand's list of strays (15 Red Hand Favor).
//! 3. Read the pattern (mission): names crossed out on the same day in every house, against the
//!    ration notes, point to one farm. Twice wrong: only "a farm out that way". Or ask the chair
//!    (Trusted): half the story, and a request to stop.
//! 4. Go and look (mission): an empty farm, nine graves and one too few. Look round, or leave and
//!    report. In a season with the Long Table its house is the farm (FindLongTable): somebody
//!    lives there (list.found_table: The House That Went Quiet from its guest stage), or the
//!    family has gone.
//! 5. Someone's been asking (wait): two hours of play; gossip in the news.
//! 6. The chair's decision (mission): read it out at every house (Trusted, ★ Elder, or Known with
//!    the husband home as proof; Known with census.refused) and hold the reading while the bells
//!    bring the infected; give the copies to the chair; or take them, past two lookouts, to Bastion
//!    or to the Red Hand.
//!
//! Endings: The names read, Buried, The census, Sold.
class RZ_Quest_TheList : RZ_Questline
{
	static const int S_HUSBAND = 0;
	static const int S_COPIES = 1;
	static const int S_PATTERN = 2;
	static const int S_FARM = 3;
	static const int S_ASKING = 4;
	static const int S_CHAIR = 5;

	static const int OUT_ALIVE = 10;		// 1: he's alive and came home
	static const int OUT_DEAD = 11;			// 1: his last letter
	static const int OUT_COPIES = 12;		// 2: enough copies
	static const int OUT_FARM = 13;			// 3: the pattern worked out
	static const int OUT_HALF = 14;			// 3: only "a farm out that way"
	static const int OUT_CHAIR_HALF = 15;	// 3: the chair told half and asked you to stop
	static const int OUT_LOOKED = 16;		// 4: looked round the farm
	static const int OUT_REPORTED = 17;		// 4: left and reported
	static const int OUT_READ = 18;			// 6: read out, and the reading held
	static const int OUT_BURIED = 19;		// 6: the copies to the chair
	static const int OUT_BASTION = 20;		// 6: to Bastion's census
	static const int OUT_REDHAND = 21;		// 6: sold to the Red Hand
	static const int OUT_CAUGHT = 22;		// 6: the lookouts took the copies back
	static const int OUT_LIVED_IN = 23;		// 4: the Long Table lives there (list.found_table)
	static const int OUT_GONE = 24;			// 4: the Long Table lived there, and has gone

	static const int E_READ = 0;			// The names read
	static const int E_BURIED = 1;			// Buried
	static const int E_CENSUS = 2;			// The census
	static const int E_SOLD = 3;			// Sold

	static const int COPIES_NEEDED = 3;		// copies to lay side by side (fewer on a map without the houses)

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info("list", "The List", RZ_EFaction.SURVIVORS);

		RZ_QuestStage husband = MissionStage(S_HUSBAND, "Find the husband", "{wife} found {husband}'s name crossed off the list at the safehouse in {askerhouse} while he was still out on a supply run. He never came back. {asker} wants to know who crosses names out, and why, without the chair, {chair}, hearing who's asking. Start with {husband}.");
		husband.GiveUp(4, "{why}");
		husband.Visits(2);

		RZ_QuestStage copies = MissionStage(S_COPIES, "Three copies of the List", "Every house keeps a list, and {askerhouse}'s is the first copy. Get the others: ask their council heads, read one in a house's stores after dark without the lookouts seeing, or pay a house's stores in food. The Red Hand keeps a list of its own.");
		copies.GiveUp(4, "{why}");
		copies.Visits(2);

		RZ_QuestStage pattern = MissionStage(S_PATTERN, "Read the pattern", "Lay the copies side by side at the safehouse in {askerhouse}, with the ration notes. A name crossed out in one house is a death. The same day crossed out in every house is something else. Or ask {chair}, if you're Trusted.");
		pattern.GiveUp(4, "{why}");
		pattern.Visits(2);

		RZ_QuestStage farm = MissionStage(S_FARM, "Go and look", "The lists point to {farmwhere}. Go and see what's there.");
		farm.GiveUp(4, "{why}");
		farm.Visits(2);

		RZ_QuestStage asking = WaitStage(S_ASKING, "Someone's been asking", "Word goes round the houses that someone's been asking about the List. Sooner or later {chair} will want to see whoever it is.");
		asking.Wait(120);
		asking.GiveUp(3, "{why}");

		RZ_QuestStage chair = MissionStage(S_CHAIR, "The chair's decision", "{chair} has asked to see you at {chair_place}, with the copies. Read the List out at every house, give the copies to {chair}, or take them to someone who'll use them.");
		chair.GiveUp(4, "{why}");
		chair.Visits(2);
	}

	//------------------------------------------------------------------------------------------------
	//! Retired while the majors run: its stages are Every Name's act III (RZ_Major_EveryName). It
	//! stays registered so saved runs load.
	override bool StartsOnItsOwn()
	{
		return !RZ_Majors.IsOn();
	}

	//------------------------------------------------------------------------------------------------
	//! Needs two safehouses, the chair, an Independent or Rogue council head, a farm out past the
	//! last village and the note, the object and the civilian. Opens after the contract The List,
	//! or once a safehouse has gone hungry or lost people lately.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return false;

		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() < 2 || !gm.GetLeaders() || !gm.GetQuestlines())
			return false;

		RZ_Leader chair = gm.GetLeaders().GetStronghold(RZ_EFaction.SURVIVORS);
		if (!chair)
			return false;

		int contractHouse = gm.GetQuestlines().WorldFlag("list.contract", 0) - 1;
		if (contractHouse < 0 && !RZ_HavenQuest.AnyHungry(gm))
			return false;

		array<RZ_Leader> heads = {};
		RZ_HavenQuest.FreeHeads(gm, 2, heads);
		if (heads.IsEmpty())
			return false;

		// The council head of the house where one name was looked into, if they're free to ask
		RZ_Leader asker = heads.GetRandomElement();
		foreach (RZ_Leader head : heads)
		{
			if (head.m_iSafehouse == contractHouse)
				asker = head;
		}

		// The farm the mouths went to: the Long Table's, in a season with one (it was Ada's farm)
		RZ_Safehouse h = houses.Get(asker.m_iSafehouse);
		int farm = PickFarm(gm, h.m_vPos);
		int mouthsFarm = -1;
		if (gm.GetLongTable())
			mouthsFarm = gm.GetLongTable().MouthsFarm();

		if (mouthsFarm >= 0 && !houses.IsSafehouseTown(mouthsFarm))
		{
			farm = mouthsFarm;
			run.SetFlag("table_farm", 1);
		}

		RZ_Settlement fs = gm.GetSettlement(farm);
		if (!fs)
			return false;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		run.SetFlag("house", asker.m_iSafehouse);
		RZ_HavenQuest.KeepLeader(run, "asker", asker);
		RZ_HavenQuest.KeepLeader(run, "chair", chair);
		run.SetFlag("chairhouse", RZ_HavenQuest.ChairHouse(gm));
		run.SetText("askerhouse", h.m_sName);
		run.SetFlag("farm", farm);
		run.SetText("farm", fs.m_sName);
		run.SetText("farmdir", RZ_Compass.DirectionWord(RZ_Compass.Bearing(h.m_vPos, fs.m_vCenter)));
		run.SetText("farmwhere", "a farm out past the last village");
		run.SetText("wife", RZ_HavenQuest.WomanName(rng));
		run.SetText("husband", RZ_HavenQuest.ManName(rng));
		run.SetFlag("seed", rng.Int(1, 1000000));
		if (rng.Chance(0.5))
			run.SetFlag("alive", 1);

		if (asker.m_iSafehouse == contractHouse)
			run.SetFlag("from_contract", 1);

		run.SetText("why", run.Fill("Nobody asked about the List again. The safehouse in {askerhouse} put it back in its drawer."));
		run.SetPlace(h.m_vPos, h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The farm the mouths were sent to: a small place past the last village, as far from every
	//! safehouse as can be, with no faction holding it. 1.2-6 km from the asking house (then
	//! anywhere 0.8-10 km). -1 if none.
	protected int PickFarm(RZ_GameModeComponent gm, vector from)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		int best = -1;
		float bestScore = -1;
		for (int pass = 0; pass < 2 && best < 0; pass++)
		{
			float minDist = 1200;
			float maxDist = 6000;
			if (pass == 1)
			{
				minDist = 800;
				maxDist = 10000;
			}

			foreach (int i, RZ_Settlement s : gm.GetSettlements())
			{
				float d = RZ_Util.DistXZ(from, s.m_vCenter);
				if (d < minDist || d > maxDist || houses.IsSafehouseTown(i) || i == gm.GetLabSettlement())
					continue;

				if (RZ_Perimeter.IsNearLineAt(s.m_vCenter))
					continue;

				if (pass == 0 && gm.GetTerritory() && gm.GetTerritory().IsActive() && gm.GetTerritory().FindInTown(i))
					continue;

				float nearest = 100000;
				for (int k = 0; k < houses.Count(); k++)
				{
					nearest = Math.Min(nearest, RZ_Util.DistXZ(houses.Get(k).m_vPos, s.m_vCenter));
				}

				float score = nearest;
				if (s.m_iSize <= 1)
					score += 5000;

				if (score > bestScore)
				{
					bestScore = score;
					best = i;
				}
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the Long Table's house is, if it's the farm outside settlement `farm` and somebody
	//! still lives there: stage 4's other branch (the house found, list.found_table: C1 from its
	//! guest stage). False in a season without one, or once the family has gone.
	static bool FindLongTable(RZ_GameModeComponent gm, int farm, out vector pos)
	{
		pos = vector.Zero;
		RZ_LongTable t = gm.GetLongTable();
		if (!t || !t.LivesAt(farm))
			return false;

		pos = t.GetHousePos();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, run.Fill("The safehouse in {askerhouse} counted the names on its list twice this week, and got two different numbers."), false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		CheckAsker(gm, run);
		RZ_Settlement farm = gm.GetSettlement(run.Flag("farm", -1));
		RZ_Leader chair = RZ_HavenQuest.SeatHolder(gm, run.Flag("chair", -1));
		if (stage == S_FARM && farm && run.Flag("half_truth") != 1)
			run.SetPlace(farm.m_vCenter, farm.m_sName);
		else if (stage == S_CHAIR && chair)
			run.SetPlace(chair.m_vPos, chair.m_sPlace);
		else if (stage != S_FARM)
		{
			RZ_Safehouse home = RZ_HavenQuest.LiveHouse(gm, run.Flag("house", -1));
			if (home)
				run.SetPlace(home.m_vPos, home.m_sName);
		}

		if (stage == S_HUSBAND && run.Visits(S_HUSBAND) <= 1)
		{
			if (run.Flag("from_contract") == 1)
				run.Note("{asker} heard someone found out why one name was crossed off the list in {askerhouse}. \"There's more than one. There's a pencil somewhere that does it, and I want to know whose hand it's in.\"");

			run.Note("{husband} went out on a supply run. When he was late, {wife} found his name already crossed off. Nobody would say who did it.");
			return;
		}

		if (stage == S_ASKING)
		{
			SpreadRumour(gm, "Someone's been going round the houses asking about the List. With copies of it, I heard. Bold.", 4);
			Headline(gm, "Someone's been asking about the List.", false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The council head who asked has left the chair (died, replaced): {wife} carries it on, and the
	//! texts name her from now on.
	protected void CheckAsker(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.Flag("asker_gone") == 1 || RZ_HavenQuest.KeptLeader(gm, run, "asker"))
			return;

		run.SetFlag("asker_gone", 1);
		run.Note("{asker} isn't council head in {askerhouse} any more. {wife} carries on asking, on her own.");
		run.SetText("asker", run.Text("wife"));
	}

	//------------------------------------------------------------------------------------------------
	//! The asking house must still have people in it.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (RZ_HavenQuest.LiveHouse(gm, run.Flag("house", -1)))
			return true;

		run.SetText("why", run.Fill("The safehouse in {askerhouse} stands empty now. There's nobody left there to ask about the List."));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_HUSBAND: return new RZ_QM_ListHusband();
			case S_COPIES: return new RZ_QM_ListCopies();
			case S_PATTERN: return new RZ_QM_ListPattern();
			case S_FARM: return new RZ_QM_ListFarm();
			case S_CHAIR: return new RZ_QM_ListChair();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_HUSBAND: return AfterHusband(gm, run, outcome);
			case S_COPIES:
			{
				if (outcome == OUT_COPIES || outcome == OUTCOME_DONE)
				{
					run.Note("Copies of the List from more than one house. Every one has lines through it, in the same council pencil.");
					return S_PATTERN;
				}

				if (run.Visits(S_COPIES) < 2)
				{
					run.Note("Not enough copies yet. The ones you got are safe with {asker}.");
					return S_COPIES;
				}

				run.Note("Fewer copies than you wanted. They'll have to do.");
				return S_PATTERN;
			}
			case S_PATTERN: return AfterPattern(run, outcome);
			case S_FARM:
			{
				// The Long Table's farm: somebody lives there, or the family has gone
				if (run.Flag("table_farm") == 1 && (outcome == OUT_LIVED_IN || outcome == OUT_GONE || outcome == OUTCOME_DONE))
					return AfterTableFarm(gm, run, outcome);

				if (outcome == OUT_LOOKED || outcome == OUTCOME_DONE)
				{
					run.SetFlag("looked", 1);
					run.Note("The farm is empty. Nine graves behind the barn, dug with care, and ten names on the lists. Haven ration crates in the barn, stacked neatly, all empty.");
					return S_ASKING;
				}

				if (outcome == OUT_REPORTED)
				{
					run.Note("Someone went as far as the farm gate, saw it was empty, and came back to say so.");
					return S_ASKING;
				}

				if (run.Visits(S_FARM) < 2)
					return S_FARM;

				run.Note("Nobody went out to the farm. The lists will have to speak for themselves.");
				return S_ASKING;
			}
			case S_ASKING:
				return S_CHAIR;
			case S_CHAIR: return AfterChair(run, outcome);
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterHusband(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_ALIVE)
		{
			run.SetFlag("husband_home", 1);
			gm.GetQuestlines().SetWorldFlag("list.husband_home", 1);
			run.Note("{husband} is alive. He came back from the run, saw his name already crossed out on the gate, and walked away. He's home in {askerhouse} now, and he'll say so to anyone.");
			return S_COPIES;
		}

		if (outcome == OUT_DEAD)
		{
			run.Note("{husband} is dead. His last letter says they'd cross him off if he was late. They crossed him off before anyone knew.");
			return S_COPIES;
		}

		if (outcome == OUTCOME_DONE)
		{
			run.Note("Someone followed {husband}'s trail to its end. {asker} still wants to know whose pencil it was.");
			return S_COPIES;
		}

		if (run.Visits(S_HUSBAND) < 2)
		{
			run.Note("Nobody found {husband}'s trail. {asker} will ask again.");
			return S_HUSBAND;
		}

		run.Note("Nobody found {husband}. {wife} has stopped waiting at the gate. {asker} still wants to know whose pencil it was.");
		return S_COPIES;
	}

	//------------------------------------------------------------------------------------------------
	//! Stage 4 at the Long Table's farm (docs/REPUTATION.md, "The Long Table"): lived in, the house
	//! is found (list.found_table, set by the farm mission: The House That Went Quiet from its guest
	//! stage); or the family has gone, burnt out or moved on.
	protected int AfterTableFarm(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		vector house;
		bool livedIn = outcome == OUT_LIVED_IN;
		if (outcome == OUTCOME_DONE)
			livedIn = FindLongTable(gm, run.Flag("farm", -1), house);

		run.SetFlag("looked", 1);
		if (livedIn)
		{
			run.SetFlag("lived_in", 1);
			gm.GetQuestlines().SetWorldFlag("list.found_table", 1);
			run.Note("The farm isn't empty. A family lives there: smoke in the chimney, washing on the line, and a place set at supper for whoever comes. The council crossed the whole house off in the first hungry week, and nobody went to look.");
			return S_ASKING;
		}

		RZ_LongTable t = gm.GetLongTable();
		if (t && t.WasBurnt() && t.GetSettlement() == run.Flag("farm", -1))
			run.Note("The farm outside {farm} is a burnt shell. Under the fallen beams in the kitchen, the long table, burnt through. Ten names on the lists, no graves, and nobody left to ask.");
		else
			run.Note("The farm outside {farm} is empty now: the stove cold, the long table still laid, a bowl at every place. Whoever lived there has gone. Ten names on the lists, and no graves.");

		return S_ASKING;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterPattern(RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_FARM || outcome == OUTCOME_DONE || outcome == OUT_CHAIR_HALF)
		{
			run.SetText("farmwhere", run.Fill("a farmhouse outside {farm}, past the last village"));
			if (outcome == OUT_CHAIR_HALF)
			{
				run.SetFlag("chair_asked", 1);
				run.Note("{chair} told you half of it: in the first hungry week the council sent people to a farm outside {farm}. Then the farm went quiet. \"Leave it there,\" {chair} said. \"Please.\"");
			}
			else
			{
				run.Note("The same day is crossed out in every house: the day the ration notes send ten portions' worth of people to a farm outside {farm}. Mouths, the council called them.");
			}

			return S_FARM;
		}

		if (outcome == OUT_HALF || run.Visits(S_PATTERN) >= 2)
		{
			run.SetFlag("half_truth", 1);
			run.SetText("farmwhere", run.Fill("a farm out {farmdir}, past the last village. That's all the lists will say"));
			run.Note("The lists won't give up more than this: a lot of names went in the first hungry week, toward a farm out {farmdir}.");
			return S_FARM;
		}

		run.Note("Nobody made sense of the copies. They're still on the table in {askerhouse}.");
		return S_PATTERN;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterChair(RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_READ)
			return Finish(run, E_READ, "The names read. Every Haven house heard the List at supper, crossed names and all. Nobody ate much.");

		if (outcome == OUT_BASTION)
			return Finish(run, E_CENSUS, "The census. Bastion has the Haven's lists now, every house and every name.");

		if (outcome == OUT_REDHAND)
			return Finish(run, E_SOLD, "Sold. The Red Hand has the Haven's lists: who's in which house, and who was put out.");

		if (outcome == OUT_BURIED || outcome == OUT_CAUGHT || outcome == OUTCOME_DONE)
			return Finish(run, E_BURIED, "Buried. {chair} has the copies, and the List is back in the council's drawer.");

		if (run.Visits(S_CHAIR) < 2)
			return S_CHAIR;

		return Finish(run, E_BURIED, "Buried. Nobody settled it, so {chair} took the copies back into the council's drawer, and that was that.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		RZ_Questlines q = gm.GetQuestlines();
		switch (ending)
		{
			case E_READ:
			{
				q.SetWorldFlag("list.read", 1);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -8);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				RZ_HavenQuest.LeaderRemembers(gm, run, RZ_HavenQuest.KeptLeader(gm, run, "asker"), RZ_Leaders.MEM_SAVED);

				// The council head whose house crossed out the most names leaves the chair (if it's
				// still the same person: the successor didn't hold the pencil)
				RZ_Leader worst = RZ_HavenQuest.KeptLeader(gm, run, "worst");
				if (worst && ReplaceLeader(gm, worst.m_iId, string.Format("%1 has stepped down as council head in %2 after the List was read. Nobody argued.", worst.m_sName, worst.m_sPlace)))
					q.SetWorldFlag("list.head_replaced", 1);

				Headline(gm, "Every Haven house read the List at supper last night, crossed names and all.");
				SpreadRumour(gm, "They read the List out. All of it. There was a farm, in the first week. Nobody's saying what happened there, only that the council knew.", 8);
				return;
			}
			case E_BURIED:
			{
				q.SetWorldFlag("list.buried", 1);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 5, 0);
				RZ_HavenQuest.LeaderRemembers(gm, run, RZ_HavenQuest.KeptLeader(gm, run, "chair"), RZ_Leaders.MEM_HELPED);
				Headline(gm, run.Fill("The council in {chair_place} has its List back in the drawer. Nobody asks about it at supper."), false);
				SpreadRumour(gm, "Somebody was asking about the List. Not any more. The council had a word.", 6);
				return;
			}
			case E_CENSUS:
			{
				q.SetWorldFlag("list.to_bastion", 1);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -10);
				Headline(gm, "Bastion has a census of the Haven now, house by house and name by name. Somebody handed them the lists.");
				SpreadRumour(gm, "Bastion knows how many of us there are in every house. They wrote it on a board. Somebody gave them our own lists to do it.", 8);
				return;
			}
			case E_SOLD:
			{
				q.SetWorldFlag("list.to_redhand", 1);
				if (run.Flag("sold_known") == 1)
				{
					q.SetWorldFlag("list.sold_known", 1);
					Headline(gm, "The Haven's lists are in Red Hand hands. The houses know who sold them.");
				}
				else
				{
					Headline(gm, "A Red Hand crew has been asking after Haven people by name. Names it shouldn't know.", false);
				}

				SpreadRumour(gm, "The Red Hand knows who the Haven turned out, and where they went. They've got it in writing now.", 8);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage == S_COPIES || run.m_iStage == S_PATTERN)
			return "Every house keeps a list. Names in pencil. Some of them have a line through, and nobody died.";

		if (run.m_iStage == S_FARM)
			return "There was a farm, the first week. They sent the ones who couldn't work. Nobody went back to check. That's all I'm saying.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Council heads and the chair are asked for copies (stage 2) and the chair for the truth
	//! (stage 3): the running stage mission answers.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		RZ_Mission m = gm.GetMissions().FindById(run.m_iMissionId);
		RZ_QM_ListCopies copies = RZ_QM_ListCopies.Cast(m);
		if (copies)
			return copies.LeaderTalk(gm, playerId, leaderId);

		RZ_QM_ListPattern pattern = RZ_QM_ListPattern.Cast(m);
		if (pattern)
			return pattern.LeaderTalk(gm, playerId, leaderId);

		if (run.m_iStage == S_ASKING && run.PlayerTookPart(gm, playerId) && leaderId == run.Flag("chair", -1))
			return "You're the one who's been going house to house about the List. Come and see me, and bring the copies you've made. I'd rather read them before anyone else does.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A Scavenger sells the husband's last sighting (stage 1).
	override string OnWandererTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int wandererId)
	{
		RZ_QM_ListHusband m = RZ_QM_ListHusband.Cast(gm.GetMissions().FindById(run.m_iMissionId));
		if (!m)
			return "";

		return m.WandererTalk(gm, playerId, wandererId);
	}
}

//------------------------------------------------------------------------------------------------
//! The List, stage 1 (Haven contract, Known): find the husband. {wife} at the safehouse, his note in
//! the town he was sent to, his pack in a second town, and in a third either him (alive: bring him
//! home, +1 person) or his last letter. Each place shows its clue once the clue before it has been
//! read. A Scavenger wanderer sells his last sighting for one food item, and ★ Radio Op hears his
//! last call on the Haven channel (talking to {wife}): either gives the end of the trail at once.
//! ★ Police: grids on the trail.
class RZ_QM_ListHusband : RZ_HavenQM
{
	protected int m_iHouse = -1;
	protected ref array<vector> m_aSpots = {};
	protected ref array<string> m_aTowns = {};
	protected ref array<bool> m_aShown = {};
	protected bool m_bAlive;
	protected int m_iStep;				// clues read along the trail: 0, 1 (the note), 2 (the pack)
	protected bool m_bEndKnown;			// a sighting or his last call gave the end's grid
	protected int m_iWifeTalk = -1;
	protected int m_iNote = -1;
	protected int m_iPack = -1;
	protected int m_iEnd = -1;

	protected static const int OBJ_WIFE = 0;
	protected static const int OBJ_HUSBAND = 1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 10, 10);
		m_Req.Bonus("police");
		m_Req.Bonus("radioham");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house", -1);
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, m_iHouse);
		if (!h)
			return false;

		// The run he went on, the road past it, and where it ended
		array<int> used = {h.m_iSettlement};
		vector from = h.m_vPos;
		for (int i = 0; i < 3; i++)
		{
			array<int> options = {};
			CollectSettlements(gm, from, 600, 1800, used, options);
			if (options.IsEmpty())
				CollectSettlements(gm, from, 300, 3500, used, options);

			if (options.IsEmpty())
				return false;

			RZ_Settlement s = gm.GetSettlement(options[0]);
			used.Insert(options[0]);
			m_aSpots.Insert(LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.6));
			m_aTowns.Insert(s.m_sName);
			m_aShown.Insert(false);
			from = s.m_vCenter;
		}

		m_bAlive = QFlag(gm, "alive") == 1;
		m_iSettlement = h.m_iSettlement;
		m_vPos = LandPointNear(h.m_vPos, 4, 10);
		int day = 9;
		if (gm.GetBroadcast())
			day = Math.Max(6, gm.GetBroadcast().GetDay() - 3);

		m_iWifeTalk = AddClue(QText(gm, "wife"), QFill(gm, string.Format("\"He went to %1 for the run. Three days, he said. On the fourth day his name had a line through it, in the council's pencil, on the list by the gate.\" {wife} holds her own copy of the list as if it might bite. \"I don't want to know who. I want to know where he is.\"", m_aTowns[0])));
		m_iNote = AddClue("A note in his hand", QFill(gm, string.Format("Pinned inside a shop door: \"{husband}, Day %1. %2 picked clean. Going on to %3 for the rest. Back a day late. Tell {wife} not to let them put a line through me.\"", day, m_aTowns[0], m_aTowns[1])));
		if (m_bAlive)
		{
			m_iPack = AddClue("His pack", QFill(gm, string.Format("{husband}'s pack, tins still in it, pushed under a hedge outside %1. On the flap, in pencil: \"Went home Day %2. Saw the list on the gate. I'm already through, so I'll go where crossed-out people go. %3, for now. Don't come after me.\"", m_aTowns[1], day + 3, m_aTowns[2])));
			m_iEnd = AddClue(QText(gm, "husband"), QFill(gm, "\"She sent you.\" {husband} is thinner than his name. \"I came back on the fourth day and I was already crossed out. Before I was even late, by the look of the pencil. I didn't go in. Where do you go when you're already dead?\""));
		}
		else
		{
			m_iPack = AddClue("His pack", QFill(gm, string.Format("{husband}'s pack, torn open and empty, in a ditch outside %1. Blood on one strap. A scrap of paper in the side pocket: \"Day %2. Scratched, not bitten. Going on to %3, somebody said there's a medic.\"", m_aTowns[1], day + 2, m_aTowns[2])));
			m_iEnd = AddClue("His last letter", QFill(gm, "On him, folded small: \"{wife}. If they crossed me out before I was late, then it was never about being late. Ask who holds the pencil. Don't let it be you next.\""));
		}

		VoiceBy(gm, "asker");
		m_sTitle = "The List: the husband";
		m_sBrief = QFill(gm, string.Format("{wife}'s husband {husband} went on a supply run to %1. While he was still out, someone crossed his name off our list. Nobody will say who. Find him, or find what happened to him. Start with {wife}, at the safehouse in {askerhouse}.", m_aTowns[0]));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		return SpawnPerson(gm, m_vPos, OBJ_WIFE, m_iWifeTalk, "Talk to " + QText(gm, "wife"), "") != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The trail's places show their clue once it's their turn and a player comes near.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, vector spot : m_aSpots)
		{
			if (m_aShown[i] || !RZ_ContractHelp.AnyNear(players, spot, 300))
				continue;

			bool open = m_iStep >= i;
			if (i == 2 && m_bEndKnown)
				open = true;

			if (!open)
				continue;

			m_aShown[i] = true;
			ShowSpot(gm, i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowSpot(RZ_GameModeComponent gm, int i)
	{
		if (i == 0)
		{
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_aSpots[0], m_iNote, "Read the note");
			return;
		}

		if (i == 1)
		{
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_aSpots[1], m_iPack, "Look in the pack");
			return;
		}

		if (m_bAlive)
		{
			SpawnPerson(gm, m_aSpots[2], OBJ_HUSBAND, m_iEnd, "Talk to " + QText(gm, "husband"), "Bring him home");
			return;
		}

		SpawnClue(gm, RZ_EPuzzleProp.BODY, m_aSpots[2], m_iEnd, "Search the body");
		SpawnGuards(gm, m_aSpots[2], 2, 25, 60);
	}

	//------------------------------------------------------------------------------------------------
	//! "Ruben, grid 047-112" for a Police reader; the town alone for others.
	protected string Where(RZ_GameModeComponent gm, int playerId, int i)
	{
		string text = m_aTowns[i];
		if (RZ_QuestApproach.HasBackground(gm, playerId, "police"))
			text += " (grid " + RZ_HavenQuest.Grid(m_aSpots[i]) + ")";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iWifeTalk && RZ_QuestApproach.HasBackground(gm, playerId, "radioham"))
			text += QFill(gm, string.Format("\n(You know his call sign. You heard it on the Haven channel the night he went quiet: \"...not coming in, tell {wife}...\" and a grid, %1.)", RZ_HavenQuest.Grid(m_aSpots[2])));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == m_iWifeTalk)
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "talked to " + QText(gm, "wife"));
			if (RZ_QuestApproach.HasBackground(gm, playerId, "radioham"))
				RevealEnd(gm, playerId, "heard his last call");
			else
				gm.NotifyPlayer(playerId, m_sTitle, "Start where he went: " + Where(gm, playerId, 0) + ".");

			return;
		}

		if (idx == m_iNote && m_iStep == 0)
		{
			m_iStep = 1;
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "followed his trail");
			gm.NotifyPlayer(playerId, m_sTitle, "Next: " + Where(gm, playerId, 1) + ".");
			return;
		}

		if (idx == m_iPack && m_iStep == 1)
		{
			m_iStep = 2;
			gm.NotifyPlayer(playerId, m_sTitle, "Next: " + Where(gm, playerId, 2) + ".");
			return;
		}

		if (idx == m_iEnd && !m_bAlive)
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_DEAD, "found his last letter");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_HUSBAND || option != 0 || !m_bAlive)
			return;

		if (!MayDecide(gm, playerId, QText(gm, "husband")))
			return;

		RZ_Questline.ChangePopulation(gm, m_iHouse, 1);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He's quiet for a long time. Then he picks up his coat. \"If she still wants me.\" By supper he's back in {askerhouse}, and somebody has rubbed the line off his name with a thumb."));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_ALIVE, "brought him home");
	}

	//------------------------------------------------------------------------------------------------
	//! The end of the trail is known: a Scavenger's sighting or his last call.
	protected void RevealEnd(RZ_GameModeComponent gm, int playerId, string how)
	{
		if (m_bEndKnown)
			return;

		m_bEndKnown = true;
		Remember(gm, playerId, RZ_EQuestApproach.TALK, how);
		NotifyParticipants(gm, m_sTitle, string.Format("Where the trail ends: %1, grid %2.", m_aTowns[2], RZ_HavenQuest.Grid(m_aSpots[2])));
	}

	//------------------------------------------------------------------------------------------------
	//! From the questline: a Scavenger is talked to while the husband is missing. One food item
	//! buys his last sighting.
	string WandererTalk(RZ_GameModeComponent gm, int playerId, int wandererId)
	{
		if (m_eState != RZ_EMissionState.ACTIVE || m_bEndKnown || !gm.GetWanderers())
			return "";

		array<int> scavengers = {};
		gm.GetWanderers().GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER), scavengers);
		if (!scavengers.Contains(wandererId))
			return "";

		AddParticipant(playerId);
		if (!RZ_HavenQuest.TakeFood(gm, PlayerEnt(playerId), 1))
			return "\"A Haven man on his own, asking nobody anything? Might have seen one. I remember better on a full stomach.\" (One food item.)";

		RevealEnd(gm, playerId, "bought a Scavenger's sighting");
		return string.Format("The tin goes into a coat pocket. \"Your man with the Haven armband? Two days back, outside %1, walking like he had nowhere to be. Grid %2.\"", m_aTowns[2], RZ_HavenQuest.Grid(m_aSpots[2]));
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		int last = Math.MinInt(m_iStep, 2);
		text += "\nThe trail: " + m_aTowns[last] + " (" + RZ_Util.DirectionText(from, m_aSpots[last]) + ")";
		if (m_bEndKnown && last < 2)
			text += "\nWhere it ends: grid " + RZ_HavenQuest.Grid(m_aSpots[2]);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		foreach (vector p : m_aSpots)
		{
			result.Insert(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The List: you know what happened to {husband}.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The List: nobody found {husband}'s trail. {wife} is still waiting at the gate.");
	}
}

//------------------------------------------------------------------------------------------------
//! The List, stage 2 (Haven contract, Known): copies of the List. The asking house's own is the
//! first. At each other safehouse: its council head (talked to: Merciful hands it over, Loyal only
//! to someone Trusted or a Known ★ Elder, Paranoid never, anyone else to someone Known; the chair gives
//! nothing and now knows someone's asking), and its stores, with two lookouts (read the list there
//! after dark: seen, +8 Haven Infamy and "stolen"; or leave 3 food for the stores). At the nearest
//! Red Hand seat, its tally man sells the crew's list of strays for 15 Red Hand Favor. Copies are
//! kept on the run, so a retry (or a restart) keeps the ones already got. Three are enough (fewer
//! on a map without enough houses).
class RZ_QM_ListCopies : RZ_HavenQM
{
	protected ref array<int> m_aHouses = {};
	protected ref array<vector> m_aStores = {};
	protected ref array<int> m_aStoresClue = {};
	protected ref array<ref RZ_HavenLookouts> m_aLookouts = {};
	protected ref array<ref RZ_QuestStealth> m_aQuiet = {};
	protected int m_iRedHandSite = -1;
	protected vector m_vRedHand;
	protected string m_sRedHandPlace;
	protected int m_iTallyTalk = -1;
	protected int m_iNeed = 3;

	protected static const int OBJ_TALLY = 50;
	protected static const int FOOD_COST = 3;
	protected static const int STRAYS_FAVOR = 15;
	protected static const int SEEN_INFAMY = 8;
	protected static const float SEEN_RADIUS = 18;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 12, 12);
		m_Req.Bonus("elder");
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		int own = QFlag(gm, "house", -1);
		RZ_Safehouse home = RZ_HavenQuest.LiveHouse(gm, own);
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!home || !houses)
			return false;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, i);
			if (i == own || !h)
				continue;

			vector stores = LandPointNear(h.m_vPos, 8, 16);
			m_aHouses.Insert(i);
			m_aStores.Insert(stores);
			m_aStoresClue.Insert(AddClue("The stores", ""));
			m_aLookouts.Insert(new RZ_HavenLookouts());
			RZ_QuestStealth quiet = new RZ_QuestStealth();
			quiet.Watch(stores, 60);
			m_aQuiet.Insert(quiet);
			AddSite(stores);
		}

		RZ_Leader boss = RZ_HavenQuest.NearestSeat(gm, RZ_EFaction.BANDITS, home.m_vPos, 6000);
		if (boss)
		{
			m_vRedHand = LandPointNear(boss.m_vPos, 8, 14);
			m_sRedHandPlace = boss.m_sPlace;
			m_iTallyTalk = AddClue("The tally man", "A Red Hand man with a schoolboy's exercise book and a pencil behind his ear. \"Strays. People the Haven turned out, and where they went after. It's a list like any other, friend. Costs like any other.\"");
			m_iRedHandSite = AddSite(m_vRedHand);
		}

		int sources = m_aHouses.Count();
		if (boss)
			sources++;

		m_iNeed = Math.MinInt(RZ_Quest_TheList.COPIES_NEEDED, 1 + sources);
		if (m_iNeed < 2)
			return false;

		m_iSettlement = home.m_iSettlement;
		m_vPos = home.m_vPos;
		VoiceBy(gm, "asker");
		m_sTitle = "The List: the copies";
		m_sBrief = QFill(gm, string.Format("Here's our own list. I need %1 copies to lay side by side, ours and the other houses'. Ask their council heads, or read them in their stores after dark if they won't. Food opens doors too. Don't tell {chair} who's asking.", m_iNeed));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnSiteNear(RZ_GameModeComponent gm, int site)
	{
		if (site == m_iRedHandSite)
		{
			SpawnPerson(gm, m_vRedHand, OBJ_TALLY, m_iTallyTalk, "Talk to the tally man", string.Format("Buy the list of strays (%1 Red Hand Favor)", STRAYS_FAVOR));
			return;
		}

		if (site < 0 || site >= m_aHouses.Count())
			return;

		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_aStores[site], site, "", string.Format("Leave %1 food for the stores", FOOD_COST), m_aStoresClue[site], "Read the list in the stores");
		m_aLookouts[site].m_sWho = "One of the house's lookouts";
		m_aLookouts[site].SpawnAround(gm, m_aStores[site], 12, 22, m_aHouses[site], 2);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		TickSites(gm, players);
		foreach (RZ_QuestStealth quiet : m_aQuiet)
		{
			quiet.Track(gm, ids, players, dt);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerShot(RZ_GameModeComponent gm, int playerId, vector pos)
	{
		super.OnPlayerShot(gm, playerId, pos);
		foreach (RZ_QuestStealth quiet : m_aQuiet)
		{
			quiet.OnShot(pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string HouseName(RZ_GameModeComponent gm, int slot)
	{
		RZ_Safehouse h = gm.GetSafehouses().Get(m_aHouses[slot]);
		if (!h)
			return "the house";

		return h.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasCopy(RZ_GameModeComponent gm, int house)
	{
		return QFlag(gm, "copy_" + house.ToString()) == 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Copies so far: the asking house's own, plus every one got.
	protected int CountCopies(RZ_GameModeComponent gm)
	{
		int n = 1;
		foreach (int house : m_aHouses)
		{
			if (HasCopy(gm, house))
				n++;
		}

		if (QFlag(gm, "copy_rh") == 1)
			n++;

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! One more copy: from house `house` (or the Red Hand, -1). Enough of them ends the stage.
	protected void GotCopy(RZ_GameModeComponent gm, int playerId, int house, int approach, string what)
	{
		if (house >= 0)
			SetQFlag(gm, "copy_" + house.ToString(), 1);
		else
			SetQFlag(gm, "copy_rh", 1);

		int have = CountCopies(gm);
		if (have >= m_iNeed)
		{
			Resolve(gm, playerId, approach, RZ_Quest_TheList.OUT_COPIES, what);
			return;
		}

		Remember(gm, playerId, approach, what);
		NotifyParticipants(gm, m_sTitle, string.Format("Copies of the List: %1 of %2.", have, m_iNeed));
	}

	//------------------------------------------------------------------------------------------------
	//! From the questline: a council head (or the chair) is talked to.
	string LeaderTalk(RZ_GameModeComponent gm, int playerId, int leaderId)
	{
		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (m_eState != RZ_EMissionState.ACTIVE || !l || l.m_eFaction != RZ_EFaction.SURVIVORS)
			return "";

		AddParticipant(playerId);
		if (l.IsStronghold())
		{
			SetQFlag(gm, "chair_heard", 1);
			return string.Format("%1 looks at you a moment too long. \"Lists are the council's business. Who's asking?\"", l.m_sShort);
		}

		if (l.m_iSafehouse == QFlag(gm, "house", -1))
			return "You've got ours. Get the others.";

		int slot = m_aHouses.Find(l.m_iSafehouse);
		if (slot < 0)
			return "";

		string town = HouseName(gm, slot);
		if (HasCopy(gm, l.m_iSafehouse))
			return "You've had our copy. Don't wave it about.";

		if (!IsEligible(gm, playerId))
			return string.Format("%1 shakes their head. \"The list? Not for you.\"", l.m_sShort);

		if (l.HasTrait(RZ_ELeaderTrait.PARANOID))
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "was turned away in " + town);
			return string.Format("%1 lowers their voice. \"Who sent you? No. The list stays in this house, and so do the names on it.\"", l.m_sShort);
		}

		bool yes = l.HasTrait(RZ_ELeaderTrait.MERCIFUL);
		string answer = string.Format("%1 takes the list down from its nail and lets you copy it. \"Whatever you're looking for, I hope it's not there.\"", l.m_sShort);
		if (!yes && l.m_eLoyalty == RZ_ELoyalty.LOYAL)
		{
			yes = RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.SURVIVORS, 2, "elder");
			if (!yes)
			{
				Remember(gm, playerId, RZ_EQuestApproach.TALK, "was sent to the chair in " + town);
				return string.Format("\"I'd have to ask the chair first,\" %1 says, and doesn't move. \"And the chair would say no.\"", l.m_sShort);
			}

			answer = string.Format("\"I should ask the chair first.\" %1 looks at you for a while. \"I know you, though.\" The list comes down off its nail.", l.m_sShort);
		}

		if (!yes)
			yes = RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.SURVIVORS, 1, "elder");

		if (!yes)
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "was turned away in " + town);
			return string.Format("%1 shrugs. \"I don't know you. The list doesn't leave the house.\"", l.m_sShort);
		}

		GotCopy(gm, playerId, l.m_iSafehouse, RZ_EQuestApproach.TALK, "was given the copy in " + town);
		return answer;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		int slot = m_aStoresClue.Find(idx);
		if (slot < 0)
			return super.GetClueText(gm, playerId, idx);

		if (!RZ_HavenQuest.IsDark())
			return "Shelves, tins and a ledger. The list hangs on a nail inside the door, but somebody's always in here by day, counting. Come back after dark.";

		return string.Format("The list of the house in %1, by torchlight: everyone who sleeps here, in pencil. Some names have a line through them and a day beside it. You copy it down, line for line.", HouseName(gm, slot));
	}

	//------------------------------------------------------------------------------------------------
	//! Reading a house's list in its stores, after dark: the lookouts see whoever is close.
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		int slot = m_aStoresClue.Find(idx);
		if (slot < 0 || !RZ_HavenQuest.IsDark())
			return;

		int house = m_aHouses[slot];
		if (HasCopy(gm, house))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've copied this house's list already.");
			return;
		}

		IEntity ent = PlayerEnt(playerId);
		string who = "";
		if (ent)
			who = m_aLookouts[slot].SeenBy(gm, ent.GetOrigin(), SEEN_RADIUS, false, playerId);

		if (who.IsEmpty() && !m_aQuiet[slot].IsQuiet(gm, playerId))
			who = "Somebody in the yard";

		if (!who.IsEmpty())
		{
			SetQFlag(gm, "stolen", 1);
			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, SEEN_INFAMY);
			gm.NotifyPlayer(playerId, "The Haven", string.Format("%1 sees you with the house's list in your hand. By morning the council in %2 knows someone was in the stores. (+%3 Haven Infamy)", who, HouseName(gm, slot), SEEN_INFAMY));
			GotCopy(gm, playerId, house, RZ_EQuestApproach.SNEAK, "was seen reading the list in " + HouseName(gm, slot));
			return;
		}

		GotCopy(gm, playerId, house, RZ_EQuestApproach.SNEAK, "copied the list in " + HouseName(gm, slot) + " unseen");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option != 0 || !MayDecide(gm, playerId, "The council"))
			return;

		if (objIdx == OBJ_TALLY)
		{
			if (QFlag(gm, "copy_rh") == 1)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "You've got it. Pleasure doing business.");
				return;
			}

			if (!TryBribe(gm, playerId, RZ_EFaction.BANDITS, STRAYS_FAVOR))
				return;

			SetQFlag(gm, "redhand_copy", 1);
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, -1, 3, 0);
			gm.NotifyPlayer(playerId, m_sTitle, "The tally man tears the pages out neatly. \"Every stray the Haven put out, and where they slept after. The boss says you're welcome back.\"");
			GotCopy(gm, playerId, -1, RZ_EQuestApproach.SIDE, "bought the Red Hand's list of strays");
			return;
		}

		if (objIdx < 0 || objIdx >= m_aHouses.Count())
			return;

		int house = m_aHouses[objIdx];
		if (HasCopy(gm, house))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've copied this house's list already.");
			return;
		}

		IEntity user = PlayerEnt(playerId);
		if (!RZ_HavenQuest.TakeFood(gm, user, FOOD_COST))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("That takes %1 food items. You have %2.", FOOD_COST, RZ_HavenQuest.CountFood(gm, user)));
			return;
		}

		gm.GetSafehouses().AddFood(house, FOOD_COST);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("The storekeeper in %1 counts the tins onto the shelf, then looks away while you copy the list off its nail.", HouseName(gm, objIdx)));
		GotCopy(gm, playerId, house, RZ_EQuestApproach.BRIBE, "paid the stores in " + HouseName(gm, objIdx));
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from) + string.Format("\nCopies: %1 of %2.", CountCopies(gm), m_iNeed);
		foreach (int i, int house : m_aHouses)
		{
			if (!HasCopy(gm, house))
				text += string.Format("\nThe safehouse in %1: grid %2", HouseName(gm, i), RZ_HavenQuest.Grid(m_aStores[i]));
		}

		if (m_iRedHandSite >= 0 && QFlag(gm, "copy_rh") != 1)
			text += string.Format("\nThe Red Hand's tally man, %1: grid %2", m_sRedHandPlace, RZ_HavenQuest.Grid(m_vRedHand));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The List: the copies are on the table, side by side.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The List: not enough copies yet. The ones you got are safe with {asker}.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		foreach (RZ_HavenLookouts lookouts : m_aLookouts)
		{
			lookouts.Cleanup();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! The List, stage 3 (Haven contract, Known): read the pattern. At the asking house, the copies side
//! by side and the house's ration notes: the one day crossed out on every copy, and the portions
//! that went that day, point to the farm. Three answers; wrong is a five-minute wait, twice wrong
//! is only "a farm out that way". ★ Elder/Police: the method spelled out. Talk to the chair while
//! Trusted with the Haven: half the story (the farm), and a request to stop. With fewer than three
//! copies the pattern doesn't close: reading them is the half truth.
class RZ_QM_ListPattern : RZ_HavenQM
{
	protected int m_iCopiesClue = -1;
	protected int m_iNotesClue = -1;
	protected int m_iAnswer;				// which option is the farm
	protected bool m_bComplete;				// three copies to compare
	protected string m_sChoices;

	protected static const int OBJ_TABLE = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 12, 10);
		m_Req.Bonus("elder");
		m_Req.Bonus("police");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, QFlag(gm, "house", -1));
		if (!run || !h)
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vPos = LandPointNear(h.m_vPos, 3, 8);
		if (!Generate(gm, run, h))
			return false;

		VoiceBy(gm, "asker");
		m_sTitle = "The List: the pattern";
		m_sBrief = QFill(gm, "The copies are on my table in {askerhouse}, with our ration notes from the first weeks. A line through one name in one house is a death. Find me what isn't.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The copies and the notes, from the run's seed (the same after a restart). On every copy, some
	//! names crossed out on the farm's day (ten in all); other days on one or two copies only.
	protected bool Generate(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_Safehouse home)
	{
		RZ_Rng rng = new RZ_Rng(run.Flag("seed", 1) * 31 + 7);
		array<string> houseNames = {home.m_sName};
		array<int> houseSeats = {-1};
		RZ_Safehouses houses = gm.GetSafehouses();
		for (int i = 0; i < houses.Count() && houseNames.Count() < 3; i++)
		{
			if (run.Flag("copy_" + i.ToString()) != 1 || !houses.Get(i))
				continue;

			houseNames.Insert(houses.Get(i).m_sName);
			RZ_Leader head = RZ_HavenQuest.CouncilHead(gm, i);
			int seat = -1;
			if (head && !head.IsStronghold())
				seat = head.m_iId;

			houseSeats.Insert(seat);
		}

		if (run.Flag("copy_rh") == 1 && houseNames.Count() < 3)
		{
			houseNames.Insert("the Red Hand's strays");
			houseSeats.Insert(-1);
		}

		m_bComplete = houseNames.Count() >= 3;
		int farmDay = rng.Int(5, 9);
		array<int> days = {farmDay - 3, farmDay - 1, farmDay + 2};

		array<string> names = {};
		RZ_NoteText.DistinctFirstNames(rng, 20, names);
		while (names.Count() < 20)
		{
			names.Insert("Dana");
		}

		// The farm's day on every copy, adding up to ten; one or two others per copy
		array<int> farmCounts = {4, 3, 3};
		int nameAt = 0;
		int worstSeat = -1;
		int worstCount = 0;
		string copies = "";
		foreach (int c, string houseName : houseNames)
		{
			string lines = "";
			int crossed = 0;
			for (int f = 0; f < farmCounts[c]; f++)
			{
				lines += string.Format("%1, Day %2. ", names[nameAt], farmDay);
				nameAt++;
				crossed++;
			}

			int extra = rng.Int(1, 3);
			for (int e = 0; e < extra; e++)
			{
				lines += string.Format("%1, Day %2. ", names[nameAt], days[(c + e) % 3]);
				nameAt++;
				crossed++;
			}

			copies += string.Format("\n%1: crossed out: %2", RZ_Util.Capitalize(houseName), lines.Trim());
			if (houseSeats[c] >= 0 && crossed > worstCount)
			{
				worstCount = crossed;
				worstSeat = houseSeats[c];
			}
		}

		// Whose house crossed out the most (the one who leaves the chair if the List is read)
		RZ_Leader worst = RZ_HavenQuest.SeatHolder(gm, worstSeat);
		if (worst)
			RZ_HavenQuest.KeepLeader(run, "worst", worst);

		// The decoys: two other places in the notes
		array<string> places = {QText(gm, "farm")};
		foreach (int si, RZ_Settlement s : gm.GetSettlements())
		{
			if (places.Count() >= 3)
				break;

			if (si != run.Flag("farm", -1) && si != home.m_iSettlement && !houses.IsSafehouseTown(si))
				places.Insert(s.m_sName);
		}

		while (places.Count() < 3)
		{
			places.Insert("the next valley");
		}

		string notes = string.Format("Ration notes, %1, in the council's hand:\nDay %2: four portions fewer. Sent to the school at %3.\nDay %4: ten portions fewer. Sent past the last village, to the farm outside %5. \"She says she can feed them.\"\nDay %6: two portions fewer. Sent to the mill at %7.",
			home.m_sName, days[0], places[1], farmDay, places[0], days[2], places[2]);

		m_iCopiesClue = AddClue("The copies of the List", "The lists, side by side." + copies);
		m_iNotesClue = AddClue("The ration notes", notes);

		// The answer among three, in a seeded order
		array<int> order = {0, 1, 2};
		rng.ShuffleInts(order);
		string choices = "";
		foreach (int k, int placeIdx : order)
		{
			if (k > 0)
				choices += "|";

			choices += "It's " + places[placeIdx];
			if (placeIdx == 0)
				m_iAnswer = k;
		}

		m_sChoices = choices;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string choices = "";
		if (m_bComplete)
			choices = m_sChoices;

		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vPos, OBJ_TABLE, "", choices, m_iCopiesClue, "Read the copies side by side"))
			return false;

		return SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(m_vPos, 1, 3), m_iNotesClue, "Read the ration notes") != null;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx != m_iCopiesClue)
			return text;

		if (!m_bComplete)
			return text + QFill(gm, "\n\nThere aren't enough copies to make a pattern. The most you can say is that a lot of names went in the first hungry week, toward a farm out {farmdir}.");

		if (HasBonus(gm, playerId))
			text += "\n(You've read enough lists to see it. One day is crossed out in every house: that isn't a death, it's a decision. Count those names, then find the day in the ration notes.)";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx != m_iCopiesClue)
			return;

		Remember(gm, playerId, RZ_EQuestApproach.TALK, "read the copies side by side");
		if (!m_bComplete && IsEligible(gm, playerId))
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_HALF, "made what sense of the copies they could");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_TABLE || !m_bComplete || option < 0 || option > 2 || !MayDecide(gm, playerId, QText(gm, "asker")))
			return;

		if (option == m_iAnswer)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "Ten names, every house, one day. The ration notes say where they went: the farm outside {farm}. Nobody wrote down what happened next. The council crossed out the whole house."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_FARM, "worked out the pattern");
			return;
		}

		m_iWrong++;
		if (m_iWrong >= 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "It doesn't add up, and by now you're not sure it ever will. A lot of names went in the first hungry week, toward a farm out {farmdir}. That's all the lists will give you."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_HALF, "got half of it");
			return;
		}

		LockOut(300);
		gm.NotifyPlayer(playerId, m_sTitle, "The days don't match the portions. Go over it again. You'll have it in five minutes, or you won't.");
	}

	//------------------------------------------------------------------------------------------------
	//! From the questline: the chair, asked by someone Trusted, tells half and asks them to stop.
	string LeaderTalk(RZ_GameModeComponent gm, int playerId, int leaderId)
	{
		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (m_eState != RZ_EMissionState.ACTIVE || !l || !l.IsStronghold() || l.m_eFaction != RZ_EFaction.SURVIVORS)
			return "";

		AddParticipant(playerId);
		if (!IsEligible(gm, playerId) || !RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.TRUSTED))
			return "Lists are the council's business. Every house keeps one, and every house keeps it to itself. That's all I'll say.";

		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_CHAIR_HALF, "asked the chair");
		return QFill(gm, "There was a farm past the last village, outside {farm}. The first week we sent it the ones who couldn't work. It said it could feed them. Then it went quiet. Leave it there. Please.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The List: the lists point to a farm.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The List: the copies are still on the table in {askerhouse}, unread.");
	}
}

//------------------------------------------------------------------------------------------------
//! The List, stage 4 (Haven contract, Known): go and look. The farm outside {farm}, empty: three
//! places to look round (stand at each for 10 s: the graves, the kitchen, the barn), or leave from
//! the gate and report. Infected about the place. With only half the truth the farm is a rough
//! area, not a spot. At the Long Table's farm (run flag table_farm) getting there is the finding:
//! within 200 m of a lived-in house (list.found_table), or 60 m of one the family has left.
class RZ_QM_ListFarm : RZ_HavenQM
{
	protected vector m_vHouse;
	protected ref array<vector> m_aLook = {};
	protected ref array<float> m_aLooked = {};
	protected ref array<int> m_aLookClue = {};
	protected int m_iLookedAt;
	protected bool m_bHalf;
	protected int m_iTable;				// the Long Table's farm: TABLE_LIVED_IN or TABLE_GONE

	protected static const int OBJ_GATE = 0;
	protected static const float LOOK_SECONDS = 10;
	protected static const float LOOK_RADIUS = 6;
	protected static const int TABLE_NONE = 0;
	protected static const int TABLE_LIVED_IN = 1;
	protected static const int TABLE_GONE = 2;
	protected static const float LIVED_IN_RADIUS = 200;	// smoke and washing, seen from the lane
	protected static const float GONE_RADIUS = 60;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 12, 10);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_fSceneRadius = 350;
	}

	//------------------------------------------------------------------------------------------------
	//! Half the truth: a rough area only.
	override bool NeedsExactLocation()
	{
		return !m_bHalf;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(QFlag(gm, "farm", -1));
		RZ_Safehouse home = RZ_HavenQuest.LiveHouse(gm, QFlag(gm, "house", -1));
		if (!s || !home)
			return false;

		m_bHalf = QFlag(gm, "half_truth") == 1;
		m_iSettlement = QFlag(gm, "farm", -1);
		float away = RZ_Compass.Bearing(home.m_vPos, s.m_vCenter);
		m_vHouse = LandPointNear(RZ_Compass.PointAt(s.m_vCenter, away, s.m_fRadius * 0.6), 0, 20);
		m_vPos = m_vHouse;
		m_sTitle = "The List: the farm";
		m_sBrief = QFill(gm, "The lists point to {farmwhere}. Go and look. Come back and tell me what's there, whatever it is.");
		VoiceBy(gm, "asker");

		// The Long Table's farm (World/RZ_LongTable.c): the house itself, lived in or left. Nothing
		// to look round: getting there is the finding
		if (QFlag(gm, "table_farm") == 1)
		{
			vector lived;
			m_iTable = TABLE_GONE;
			if (RZ_Quest_TheList.FindLongTable(gm, m_iSettlement, lived))
				m_iTable = TABLE_LIVED_IN;

			RZ_LongTable t = gm.GetLongTable();
			if (t && t.GetSettlement() == m_iSettlement)
				m_vHouse = t.GetHousePos();

			m_vPos = m_vHouse;
			return true;
		}

		m_aLook.Insert(LandPointNear(m_vHouse, 14, 22));
		m_aLook.Insert(LandPointNear(m_vHouse, 2, 5));
		m_aLook.Insert(LandPointNear(m_vHouse, 18, 30));
		m_aLookClue.Insert(AddClue("Behind the barn", "Graves in a row, the earth long settled, each with a stick for a cross. Somebody dug them carefully, and kept them straight. Nine of them. The lists sent ten."));
		m_aLookClue.Insert(AddClue("The kitchen", "A long table set for twenty, the plates still out, a skin of dust on everything. On the dresser, a child's drawing: a house, a lot of people, a sun."));
		m_aLookClue.Insert(AddClue("The barn", "Haven ration crates, stencilled with the council's mark, empty and stacked neatly. Somebody kept them. Under the last one, a tally in a careful hand: ten names, and nine of them crossed through."));
		for (int i = 0; i < m_aLook.Count(); i++)
		{
			m_aLooked.Insert(0);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		// The Long Table's farm: its own people (or nobody); no props, no infected about the place
		if (m_iTable != TABLE_NONE)
			return true;

		RZ_Safehouse home = RZ_HavenQuest.LiveHouse(gm, QFlag(gm, "house", -1));
		vector gate = LandPointNear(m_vHouse, 22, 30);
		if (home)
			gate = LandPointNear(RZ_Compass.PointAt(m_vHouse, RZ_Compass.Bearing(m_vHouse, home.m_vPos), 26), 0, 4);

		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, gate, OBJ_GATE, "", "Leave, and report what you saw", -1, ""))
			return false;

		foreach (int i, vector p : m_aLook)
		{
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, p, m_aLookClue[i], "Look");
		}

		int guards = 3;
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s && s.m_fInfection >= 50)
			guards = 5;

		SpawnGuards(gm, m_vHouse, guards, 30, 80);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Looking round: someone stands at each of the three places for ten seconds.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSceneUp || m_bResolved)
			return;

		if (m_iTable != TABLE_NONE)
		{
			float radius = GONE_RADIUS;
			if (m_iTable == TABLE_LIVED_IN)
				radius = LIVED_IN_RADIUS;

			array<int> close = {};
			GetPlayersNear(ids, players, m_vHouse, radius, close);
			if (!close.IsEmpty())
				ArriveAtTable(gm, close);

			return;
		}

		foreach (int i, vector p : m_aLook)
		{
			if (m_aLooked[i] >= LOOK_SECONDS)
				continue;

			array<int> near = {};
			GetPlayersNear(ids, players, p, LOOK_RADIUS, near);
			if (near.IsEmpty())
				continue;

			m_aLooked[i] = m_aLooked[i] + dt;
			if (m_aLooked[i] < LOOK_SECONDS)
				continue;

			m_iLookedAt++;
			foreach (int playerId : near)
			{
				AddParticipant(playerId);
				gm.NotifyPlayer(playerId, m_sTitle, GetClueText(gm, playerId, m_aLookClue[i]));
			}

			if (m_iLookedAt >= m_aLook.Count())
				Resolve(gm, near[0], RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_LOOKED, "looked round the farm");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At the Long Table's farm. Lived in (checked again now): the house is found, and The House
	//! That Went Quiet can begin at its guest stage (list.found_table; the family notices whoever
	//! came). Gone: burnt out, or left with the table still laid.
	protected void ArriveAtTable(RZ_GameModeComponent gm, array<int> close)
	{
		RZ_LongTable t = gm.GetLongTable();
		vector house;
		if (m_iTable == TABLE_LIVED_IN && t && RZ_Quest_TheList.FindLongTable(gm, m_iSettlement, house))
		{
			foreach (int seen : close)
			{
				AddParticipant(seen);
				t.Notice(seen);
				gm.NotifyPlayer(seen, m_sTitle, QFill(gm, "Smoke from the chimney, washing on the line, a kitchen garden dug in straight rows. Somebody lives at the farm outside {farm}, and nobody crossed them off anything. You've seen enough to tell {asker}."));
			}

			gm.GetQuestlines().SetWorldFlag("list.found_table", 1);
			t.TryBeginC1();
			Resolve(gm, close[0], RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_LIVED_IN, "found people living at the farm");
			return;
		}

		string text = "Nobody's here. The door's open, the stove is cold, and the long table is still laid, a bowl at every place. Whoever lived here has gone, and not long ago.";
		if (t && t.WasBurnt() && t.GetSettlement() == m_iSettlement)
			text = "The farmhouse is a burnt shell, the roof fallen in. Under the beams in the kitchen, a long table, burnt through, and a lot of bowls. Nobody lives here now.";

		foreach (int came : close)
		{
			AddParticipant(came);
			gm.NotifyPlayer(came, m_sTitle, text);
		}

		Resolve(gm, close[0], RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_GONE, "found the farm empty");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_GATE || option != 0 || !MayDecide(gm, playerId, QText(gm, "asker")))
			return;

		gm.NotifyPlayer(playerId, m_sTitle, "You've seen enough from the gate: no smoke, no tracks, a door left open. Whoever lived here is gone. You go back to say so.");
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_REPORTED, "reported the farm from its gate");
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		foreach (vector p : m_aLook)
		{
			result.Insert(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The List: someone has been out to the farm.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The List: nobody went out to the farm.";
	}
}

//------------------------------------------------------------------------------------------------
//! The List, stage 6 (Haven contract, Known): the chair's decision, at the chair's seat. The copies
//! on a table, four choices: read it out at every house (Trusted, ★ Elder, or Known with the
//! husband home as proof; Known with census.refused from B2), then hold the gate on the road in
//! (60-80 m out) for 3 minutes while the bells bring two waves; give the copies to the chair; or carry them
//! to the nearest Bastion seat or Red Hand crew, past two Haven lookouts (seen: the copies go back
//! to the chair, +8 Haven Infamy). The Red Hand copy from stage 2 brings a runner to the edge of
//! town instead, past one lookout.
class RZ_QM_ListChair : RZ_HavenQM
{
	protected vector m_vTable;
	protected vector m_vGate;
	protected vector m_vBastion;
	protected vector m_vRedHand;
	protected string m_sBastionPlace;
	protected string m_sRedHandPlace;
	protected bool m_bBastion;
	protected bool m_bRedHand;
	protected int m_iTo = -1;				// where the carrier is going: OBJ_BASTION or OBJ_REDHAND
	protected bool m_bReading;
	protected int m_iReader = -1;
	protected bool m_bDropUp;
	protected ref RZ_HavenLookouts m_Lookouts = new RZ_HavenLookouts();

	protected static const int OBJ_TABLE = 0;
	protected static const int OBJ_BASTION = 1;
	protected static const int OBJ_REDHAND = 2;
	protected static const float SEEN_RADIUS = 35;
	protected static const int SEEN_INFAMY = 8;
	protected static const float READ_SECONDS = 180;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 10, 10);
		m_Req.Bonus("elder");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader chair = RZ_HavenQuest.SeatHolder(gm, QFlag(gm, "chair", -1));
		if (!chair)
			return false;

		vector seat = chair.m_vPos;
		RZ_Safehouse chairHouse = RZ_HavenQuest.LiveHouse(gm, QFlag(gm, "chairhouse", -1));
		if (chairHouse)
			seat = chairHouse.m_vPos;

		m_vTable = LandPointNear(seat, 5, 10);
		m_vGate = LandPointNear(seat, 60, 80);
		m_vPos = m_vTable;
		m_iSettlement = gm.FindNearestSettlement(seat);

		RZ_Leader post = RZ_HavenQuest.NearestSeat(gm, RZ_EFaction.MILITARY, seat, 6000);
		if (post)
		{
			m_bBastion = true;
			m_vBastion = LandPointNear(post.m_vPos, 6, 12);
			m_sBastionPlace = string.Format("%1, %2", post.m_sShort, post.m_sPlace);
		}

		RZ_Leader boss = RZ_HavenQuest.NearestSeat(gm, RZ_EFaction.BANDITS, seat, 6000);
		if (QFlag(gm, "redhand_copy") == 1)
		{
			// The crew already trusts you: a runner meets you just outside town
			m_bRedHand = true;
			m_vRedHand = LandPointNear(seat, 300, 450);
			m_sRedHandPlace = "a Red Hand runner outside town";
		}
		else if (boss)
		{
			m_bRedHand = true;
			m_vRedHand = LandPointNear(boss.m_vPos, 6, 12);
			m_sRedHandPlace = string.Format("%1, %2", boss.m_sShort, boss.m_sPlace);
		}

		VoiceBy(gm, "chair");
		m_sTitle = "The List: the chair's decision";
		m_sBrief = QFill(gm, "You've been asking about the List. So have I, for longer than you. Bring the copies to {chair_place} and we'll decide what's done with them. You'll have a say. So will I.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string bastion = "";
		if (m_bBastion)
			bastion = "Take them to Bastion's census";

		string redHand = "";
		if (m_bRedHand)
			redHand = "Sell them to the Red Hand";

		int talk = AddClue("The copies", QFill(gm, "The copies of the List on the council's table, weighted down with a mug. {chair} doesn't touch them. \"Every house reads its dead at supper. These aren't all dead. Read them, and the houses will know what the council did. Or give them to me. Or take them to people who'll put them to use. I know which I'd choose. It isn't my choice today.\""));
		string choices = string.Format("Read it out at supper, every name|Give the copies to %1|%2|%3", QText(gm, "chair"), bastion, redHand);
		return SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vTable, OBJ_TABLE, "", choices, talk, "Look at the copies") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (m_bReading)
		{
			// The bells at supper: the infected hear them too. The reading is inside; the gate is held
			gm.GetHeat().AddHeat(m_vTable, 0.5 * dt);
			if (TickHold(gm, dt, ids, players, m_vGate, 30, READ_SECONDS, 60, 2))
			{
				RZ_QuestRun run = Run(gm);
				if (run && m_iReader > 0)
					run.Reward(gm, m_iReader, RZ_EFaction.SURVIVORS, 10, 30, QFill(gm, "{asker} won't forget who read the List out."));

				NotifyParticipants(gm, m_sTitle, "The last name is read. Nobody says anything for a while. Then somebody puts the kettle on.");
				Resolve(gm, m_iReader, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_READ, "read the List out");
			}

			return;
		}

		if (m_iTo < 0)
			return;

		int carrier = Carrier();
		if (carrier < 0)
		{
			m_iTo = -1;
			m_iCarrier = -1;
			NotifyParticipants(gm, m_sTitle, QFill(gm, "Whoever carried the copies is gone. They're back on the table at {chair_place}."));
			return;
		}

		// The drop at the other end, once the carrier is near it
		vector dest = m_vBastion;
		if (m_iTo == OBJ_REDHAND)
			dest = m_vRedHand;

		IEntity ent = PlayerEnt(carrier);
		if (!m_bDropUp && RZ_Util.DistXZ(ent.GetOrigin(), dest) <= 300)
		{
			m_bDropUp = true;
			string label = "Hand over the copies";
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, dest, m_iTo, "", label, -1, "");
		}

		// Leaving the house itself is fine; past it, the lookouts are watching the road
		if (RZ_Util.DistXZ(ent.GetOrigin(), m_vTable) < 40)
			return;

		string who = m_Lookouts.SeenBy(gm, ent.GetOrigin(), SEEN_RADIUS, true, carrier);
		if (who.IsEmpty())
			return;

		gm.AddInfamy(carrier, RZ_EFaction.SURVIVORS, SEEN_INFAMY);
		gm.NotifyPlayer(carrier, "The Haven", string.Format("%1 sees the council's copies under your arm and takes them back off you, not gently. (+%2 Haven Infamy)", who, SEEN_INFAMY));
		Resolve(gm, carrier, RZ_EQuestApproach.SIDE, RZ_Quest_TheList.OUT_CAUGHT, "was caught carrying the copies off", false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx == OBJ_TABLE)
		{
			TableChoice(gm, playerId, option);
			return;
		}

		if ((objIdx == OBJ_BASTION || objIdx == OBJ_REDHAND) && option == 0)
			HandOver(gm, playerId, objIdx);
	}

	//------------------------------------------------------------------------------------------------
	protected void TableChoice(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (m_bReading || m_iTo >= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "That's already decided. See it through.");
			return;
		}

		if (!MayDecide(gm, playerId, QText(gm, "chair")))
			return;

		if (option == 0)
		{
			if (!MayRead(gm, playerId))
			{
				Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "wanted to read it out");
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"And who are you, to read the council's List to every house?\" {chair} isn't angry. \"Somebody the houses trust might. Or somebody with proof.\""));
				return;
			}

			m_bReading = true;
			m_iReader = playerId;
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "began the reading");
			gm.GetHeat().AddHeat(m_vTable, 20);
			NotifyParticipants(gm, m_sTitle, string.Format("The bells go for supper, and the house listens. Every name, crossed lines and all. It takes three minutes, and the bells carry. Hold the gate on the road in while it's read: grid %1.", RZ_HavenQuest.Grid(m_vGate)));
			return;
		}

		RZ_QuestRun run = Run(gm);
		if (option == 1)
		{
			if (run)
				run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, 5, 20, QFill(gm, "{chair} remembers who brought the copies back."));

			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{chair} gathers the copies, squares the edges and puts them in a drawer that locks. \"Thank you. Some things are kept so the houses can go on.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_TheList.OUT_BURIED, "gave the copies to the chair");
			return;
		}

		if ((option == 2 && m_bBastion) || (option == 3 && m_bRedHand))
		{
			m_iTo = OBJ_BASTION;
			vector dest = m_vBastion;
			string place = m_sBastionPlace;
			int lookouts = 2;
			if (option == 3)
			{
				m_iTo = OBJ_REDHAND;
				dest = m_vRedHand;
				place = m_sRedHandPlace;
				if (QFlag(gm, "redhand_copy") == 1)
					lookouts = 1;
			}

			GiveTo(playerId);
			m_Lookouts.SpawnAlong(gm, m_vTable, dest, QFlag(gm, "chairhouse", -1), lookouts);
			Remember(gm, playerId, RZ_EQuestApproach.SIDE, "took the copies away");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You put the copies under your coat. %1 says nothing at all. To %2: grid %3. The house's lookouts are on the road.", QText(gm, "chair"), place, RZ_HavenQuest.Grid(dest)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Who may read it out: Trusted with the Haven or a favoured Known (★ Elder); Known with the
	//! husband home as proof, or once the chair trusts you (census.refused, from B2).
	protected bool MayRead(RZ_GameModeComponent gm, int playerId)
	{
		int difficulty = 2;
		if (QFlag(gm, "husband_home") == 1 || gm.GetQuestlines().WorldFlag("census.refused") == 1)
			difficulty = 1;

		return RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.SURVIVORS, difficulty, "elder");
	}

	//------------------------------------------------------------------------------------------------
	//! The carrier hands the copies to Bastion or the Red Hand.
	protected void HandOver(RZ_GameModeComponent gm, int playerId, int to)
	{
		if (playerId != m_iCarrier || to != m_iTo)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Not from you.");
			return;
		}

		RZ_QuestRun run = Run(gm);
		if (to == OBJ_BASTION)
		{
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.MILITARY, RZ_EFaction.SURVIVORS, 10, 15);
			if (run)
				run.Reward(gm, playerId, RZ_EFaction.MILITARY, 5, 30, "Bastion has its census. Somebody in a clean uniform writes your name down too, in the other column.");

			// The people the Haven put out are on Bastion's board now (as Names for Bastion)
			array<int> all = {};
			GetGame().GetPlayerManager().GetPlayers(all);
			foreach (int other : all)
			{
				if (other == playerId || !gm.GetSafehouses() || !gm.GetSafehouses().IsExiled(other))
					continue;

				gm.AddInfamy(other, RZ_EFaction.MILITARY, 5);
				gm.NotifyPlayer(other, RZ_Factions.GetName(RZ_EFaction.MILITARY), "Bastion has your name now. It was on a Haven list, with a line through it. (+5 Infamy)");
			}

			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_TheList.OUT_BASTION, "gave the copies to Bastion", false);
			return;
		}

		if (run)
			run.Reward(gm, playerId, RZ_EFaction.BANDITS, 10, 40, "The Red Hand pays well for names. It always has.");

		// One time in five the houses find out who sold them
		if (Math.RandomFloat01() < 0.2)
		{
			SetQFlag(gm, "sold_known", 1);
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.SURVIVORS, 0, 15);
			gm.NotifyPlayer(playerId, "The Haven", "Somebody saw who carried the council's copies to the Red Hand. Every house will know your face by supper. (+15 Haven Infamy)");
		}

		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_TheList.OUT_REDHAND, "sold the copies to the Red Hand", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		if (m_bReading)
			text += "\nThe gate to hold: " + RZ_Util.DirectionText(from, m_vGate);

		if (m_iTo == OBJ_BASTION)
			text += string.Format("\nBastion's census: %1, grid %2", m_sBastionPlace, RZ_HavenQuest.Grid(m_vBastion));
		else if (m_iTo == OBJ_REDHAND)
			text += string.Format("\nThe Red Hand: %1, grid %2", m_sRedHandPlace, RZ_HavenQuest.Grid(m_vRedHand));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The List: it's decided.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bReading)
			return "The List: the reading broke up before the last name. The bells brought too many.";

		return QFill(gm, "The List: {chair} is still waiting with the copies.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Lookouts.Cleanup();
	}
}
