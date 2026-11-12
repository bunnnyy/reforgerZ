//! C1. The House That Went Quiet (the Long Table; docs/QUESTLINES.md, docs/MISSIONS.md
//! "Questlines"). Nobody gives this job: somebody just has to notice. Only in a season with a
//! Long Table (World/RZ_LongTable.c), and never described: the writing stays on the boots, the
//! names, the jars.
//!
//! 0. Somebody noticed (wait, none): routes in. list.found_table (H1): to stage 2.
//!    cargo.buyer_known (R2), place.undercover or place.proof (C2): the proof is done, stage 4.
//!    The house found out by everyone: stage 4.
//! 1. Never arrived: the last road of someone who never arrived (a boot on the verge, a satchel
//!    at a fence, the farm gate; a step chain). ★ Hunter or Police: grid references.
//! 2. Guest: the host meets you on the road. Talk: accept and eat (table.guest, yours; the guest
//!    rule watches the rest of the visit). Sneak: decline and watch the house unseen. Declare a
//!    bite: blessed and sent on.
//! 3. Proof: three of the guestbook, the boots, the letters in a drawer, the cellar (the mother's
//!    key, or a crowbar, which is loud). Sneak in at supper (18:30), talk to the boy away from the
//!    house, unarmed (★ Nurse or Elder: he says one thing, and where the key is), or fight.
//! 4. Who to tell: any leader of the Haven, Bastion, Halcyon or the Red Hand (table.haven,
//!    table.bastion, table.halcyon, table.redhand): the house is found out. Or the gate: yourself.
//! 5. The end of the house: the Haven walks them out at dusk, unarmed; Bastion's guns after five
//!    minutes (get the boy out first, or don't); Halcyon takes the mother as a "volunteer";
//!    a Red Hand crew moves them on while you hold the farm 90 s; or by your own hand.
//! 6. The proof (wait, after "by your own hand"): taken to a faction now, half its Fame.
//!
//! Endings: Walked out, Burnt, Studied, Paid to go, By your own hand.
class RZ_Quest_HouseWentQuiet : RZ_Questline
{
	static const int S_OPEN = 0;
	static const int S_TRAIL = 1;
	static const int S_GUEST = 2;
	static const int S_PROOF = 3;
	static const int S_TELL = 4;
	static const int S_END = 5;
	static const int S_AFTER = 6;

	static const int OUT_FOUND = 10;		// the trail reached the farm gate
	static const int OUT_ATE = 11;			// a guest: ate with them
	static const int OUT_WATCHED = 12;		// declined, and watched the house unseen
	static const int OUT_SEEN = 13;			// declined, watched, and was seen
	static const int OUT_BLESSED = 14;		// declared a bite: blessed and sent on
	static const int OUT_PROVEN = 15;		// three things found
	static const int OUT_CAUGHT = 16;		// caught at it: the house is shut
	static const int OUT_TOLD = 20;			// + the faction told (20 Haven .. 23 Halcyon)
	static const int OUT_SELF = 24;			// nobody told: deal with it yourself
	static const int OUT_ENDED = 30;		// the end of the house happened, the branch's way
	static const int OUT_BY_HAND = 31;		// broken up by your own hand

	static const int BRANCH_SELF = 4;		// branches 0-3 are the faction told

	static const int E_WALKED = 0;
	static const int E_BURNT = 1;
	static const int E_STUDIED = 2;
	static const int E_PAID = 3;			// RZ_LongTable.C1_ENDING_PAID: the house moves, and C1 can come back once
	static const int E_BY_HAND = 4;

	protected static const int TELL_FAME = 10;
	protected static const int TELL_FAME_RED_HAND = 4;
	protected static const int UNDERCOVER_FAME = 5;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(RZ_LongTable.C1_KEY, "The House That Went Quiet", -1);
		Slot(RZ_LongTable.SLOT);

		RZ_QuestStage open = WaitStage(S_OPEN, "Somebody noticed", "Wanderers who take the road past {town} don't always arrive. Nobody gives this job. Somebody just has to notice.");
		open.GiveUp(2, "{fizzle}");

		RZ_QuestStage trail = MissionStage(S_TRAIL, "Never arrived", "{lost} left {from} for {to} and never got there. Their last road runs past {town}. Follow it: something on the verge, something at a fence, a gate at the end.");
		trail.GiveUp(4, "{fizzle}");
		trail.Visits(2);

		RZ_QuestStage guest = MissionStage(S_GUEST, "Guest", "There's a farm past {town} with smoke in the chimney, and a family that sets a place for whoever comes. {host} keeps the road there. Go and meet him. Eat with them, or decline and watch the house from outside, unseen.");
		guest.GiveUp(4, "{fizzle}");
		guest.Visits(2);

		RZ_QuestStage proof = MissionStage(S_PROOF, "Proof", "Find three things at the {family} farm that say what the house is: the guestbook, the boots by the door, a drawer, the cellar. Go in at supper, half past six, when they're all at the table. Ask the boy, away from the house, with nothing in your hands. Or force your way in.");
		proof.GiveUp(4, "{fizzle}");
		proof.Visits(3);

		RZ_QuestStage tell = MissionStage(S_TELL, "Who to tell", "You know what the {family} farm is. Take it to someone who'll end it: a Haven council head, a Bastion commander, a Halcyon researcher, a Red Hand crew boss. Or go back to the farm gate and deal with it yourself.");
		tell.GiveUp(6, "{fizzle}");
		tell.Visits(2);

		RZ_QuestStage end = MissionStage(S_END, "The end of the house", "{end}");
		end.GiveUp(6, "{fizzle}");
		end.Visits(2);

		RZ_QuestStage after = WaitStage(S_AFTER, "What you know", "The {family} farm is gone. What you found there is still worth something to whoever you tell: any faction's leader will listen.");
		after.Wait(120, 12);
	}

	// ================================================================ The house
	//------------------------------------------------------------------------------------------------
	protected static RZ_LongTable Table(RZ_GameModeComponent gm)
	{
		return gm.GetLongTable();
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody is at the farm any more (broken up, or none).
	protected static bool Gone(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		return !t || !t.Exists();
	}

	//------------------------------------------------------------------------------------------------
	//! Proof that reached the story from another line (H1's look, R2's buyer, C2's undercover or
	//! hand-off): the proof stage is done.
	static bool ProofFromCrossing(RZ_GameModeComponent gm)
	{
		RZ_Questlines q = gm.GetQuestlines();
		return q.WorldFlag("cargo.buyer_known") > 0 || q.WorldFlag("place.undercover") > 0 || q.WorldFlag("place.proof") > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The names and places as they are now: whoever meets the road, the boy if he's still there,
	//! the farm's town (it may have moved). And the line it ends with if it ends now.
	protected void RefreshTexts(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_LongTable t = Table(gm);
		if (!t)
			return;

		if (t.Exists())
		{
			run.SetText("town", t.GetTownName());
			run.SetText("family", t.GetSurname());
			run.SetPlace(t.GetHousePos(), t.FarmTitle());
		}

		string mother = t.NameOf(RZ_ETableRole.MOTHER);
		if (mother.IsEmpty())
			mother = "the mother";

		run.SetText("mother", mother);
		run.SetText("host", t.GreeterName());
		string son = t.NameOf(RZ_ETableRole.SON);
		if (son.IsEmpty())
			son = "the quiet son";

		run.SetText("son", son);
		string boy = t.NameOf(RZ_ETableRole.BOY);
		if (boy.IsEmpty())
			boy = "the boy";

		run.SetText("boy", boy);
		if (run.Text("volunteer").IsEmpty())
			run.SetText("volunteer", mother);

		// Filled now, so the stage text that is only "{end}" needs no second pass
		run.SetText("end", run.Fill(EndText(gm, run)));
		run.SetText("fizzle", FizzleText(gm, run));
	}

	//------------------------------------------------------------------------------------------------
	protected string FizzleText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		string town = run.Text("town");
		if (Gone(gm))
			return string.Format("There's nobody at the farm past %1 any more. Whatever was going on there, it's over.", town);

		RZ_LongTable t = Table(gm);
		if (t.GetState() == RZ_ETableState.QUIET)
			return string.Format("The farm past %1 has gone dark, and nobody followed it to wherever it went.", town);

		return string.Format("Nobody followed it up. The smoke still goes up past %1 most evenings, and there's always a place set.", town);
	}

	//------------------------------------------------------------------------------------------------
	//! The last stage's brief, by who was told, naming only who is still at the farm.
	static string EndText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_LongTable t = Table(gm);
		bool boy = t && t.IsAlive(RZ_ETableRole.BOY);
		bool son = t && t.IsAlive(RZ_ETableRole.SON);
		bool mother = t && t.IsAlive(RZ_ETableRole.MOTHER);
		string text;
		switch (run.m_iBranch)
		{
			case RZ_EFaction.SURVIVORS:
			{
				text = "The Haven's chair wants them brought home and walked out, the way the Haven does it: at dusk, to the edge of town, unarmed. They'll go quietly.";
				if (son)
					text += " All but {son}.";

				return text;
			}
			case RZ_EFaction.MILITARY:
			{
				text = "Bastion has the {family} farm down as an infected cluster. The mortars open up five minutes after the warning, and Bastion doesn't check for survivors.";
				if (boy)
					text += " {boy} is in the yard. Get him out first, or don't.";

				return text;
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (mother)
					return "Halcyon's director wants one of them alive, and {mother}'s kitchen notebook: why they never eat the bitten. Walk her to the lab as a volunteer.";

				return "Halcyon's director wants one of them alive, and the kitchen notebook: why they never eat the bitten. Walk one of them to the lab as a volunteer.";
			}
			case RZ_EFaction.BANDITS:
				return "The Red Hand's boss is paying the {family} farm to go away. Hold the farm ninety seconds while a crew loads them up and moves them on. Don't make it a story.";
			case BRANCH_SELF:
			{
				text = "You haven't told anyone. Go back to the {family} farm and end it: fight them";
				if (mother)
					text += ", or talk the last of them out once {mother} is gone.";
				else
					text += ", or talk the last of them out.";

				if (boy)
					text += " Get {boy} out first if you can.";

				return text;
			}
		}

		return "Somebody will end the {family} farm. Be there when they do.";
	}

	// ================================================================ The questline
	//------------------------------------------------------------------------------------------------
	//! A season with a Long Table, the house open, a person, a note and an object to use; and somebody has
	//! noticed: suspicion 30 and a rumour heard, a crossing's proof, or the house found out.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen())
			return false;

		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return false;

		RZ_Questlines q = gm.GetQuestlines();
		bool noticed = t.GetSuspicion() >= 30 && t.WasHeard();
		bool crossing = q.WorldFlag("list.found_table") > 0 || ProofFromCrossing(gm);
		if (!noticed && !crossing && !t.IsExposed())
			return false;

		// Who never arrived: the house's last, or (none yet, or H2's exile, who came by van with
		// nothing, so left no trail) somebody the road remembers
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		string lost = t.GetLostName();
		string from = t.GetLostFrom();
		string dest = t.GetLostTo();
		if (lost.IsEmpty() || t.GetLostRole() == RZ_LongTable.ROLE_NO_BAG)
		{
			lost = "A Haven runner called " + RZ_NoteText.FirstName(rng);
			from = NearOther(gm, t.GetHousePos(), t.GetSettlement());
			dest = "a safehouse that waited up for them";
		}

		run.SetText("lost", RZ_Util.Capitalize(lost));
		run.SetText("from", from);
		run.SetText("to", dest);
		run.SetText("watch", RZ_NoteText.Surname(rng));
		RefreshTexts(gm, run);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A settlement other than `skip`, the nearest to pos: where someone came from.
	protected static string NearOther(RZ_GameModeComponent gm, vector pos, int skip)
	{
		string best = "the last village";
		float bestDist = float.MAX;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i == skip)
				continue;

			float d = RZ_Util.DistXZ(pos, s.m_vCenter);
			if (d < bestDist)
			{
				bestDist = d;
				best = s.m_sName;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Questlines q = gm.GetQuestlines();
		bool routed = ProofFromCrossing(gm) || Table(gm).IsExposed() || q.WorldFlag("list.found_table") > 0;
		if (!routed)
			run.Note("{lost} left {from} for {to} and never got there. Their road runs past a farm past {town}, the one with smoke in the chimney.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RefreshTexts(gm, run);
		if (stage == S_GUEST && run.Flag("from_haven") == 1)
			run.Note("The Haven's List led to a farm past {town}: in the first hungry week the council crossed the whole house off. There's smoke in the chimney.");

		if (stage == S_TELL && run.Flag("proof_crossing") == 1)
			run.Note("Someone else found out what the {family} farm is. Now somebody has to say it to someone who'll act.");

		if (stage == S_END)
			run.Note(EndText(gm, run));
	}

	//------------------------------------------------------------------------------------------------
	//! The farm has to have someone at it (not moving); the Haven's walk waits for dusk.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RefreshTexts(gm, run);
		if (stage == S_AFTER)
			return true;

		// Gone: the stage goes ahead at once and Next() says so
		if (Gone(gm))
			return true;

		RZ_LongTable t = Table(gm);
		if (!t.IsOpen())
			return false;

		if (stage == S_GUEST && !t.Greeter() && !t.IsAlive(RZ_ETableRole.MOTHER))
			return false;

		if (stage == S_END && run.m_iBranch == RZ_EFaction.SURVIVORS)
		{
			float hour = RZ_GameTime.GetHour();
			if (hour >= 0 && !RZ_GameTime.IsWithin(hour, 16, 5.5))
				return false;
		}

		if (stage == S_END && run.m_iBranch == RZ_EFaction.INSTITUTE && !gm.HasLab())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (Gone(gm))
			return new RZ_QM_TableGone();

		switch (stage)
		{
			case S_TRAIL: return new RZ_QM_TableTrail();
			case S_GUEST: return new RZ_QM_TableGuest();
			case S_PROOF: return new RZ_QM_TableProof();
			case S_TELL: return new RZ_QM_TableTell();
			case S_END:
			{
				switch (run.m_iBranch)
				{
					case RZ_EFaction.SURVIVORS: return new RZ_QM_TableWalkOut();
					case RZ_EFaction.MILITARY: return new RZ_QM_TableGuns();
					case RZ_EFaction.INSTITUTE: return new RZ_QM_TableVolunteer();
					case RZ_EFaction.BANDITS: return new RZ_QM_TableMoveOn();
				}

				return new RZ_QM_TableByHand();
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		RefreshTexts(gm, run);
		switch (stage)
		{
			case S_END:
				return NextFromEnd(gm, run, outcome);
			case S_AFTER:
				return Finish(run, E_BY_HAND, "By your own hand. The {family} farm is an ordinary house again, and nobody sets a place there.");
		}

		// The house is gone, and not by this story's hand (or by its fight)
		if (Gone(gm))
		{
			if (run.Flag("fought") == 1)
				return S_AFTER;

			return Fizzle(run, "{fizzle}");
		}

		RZ_LongTable t = Table(gm);
		switch (stage)
		{
			case S_OPEN:
			{
				if (ProofFromCrossing(gm) || t.IsExposed())
				{
					run.SetFlag("proof_crossing", 1);
					return S_TELL;
				}

				RZ_Questlines q = gm.GetQuestlines();
				if (q.WorldFlag("list.found_table") > 0)
				{
					run.SetFlag("from_haven", 1);
					return S_GUEST;
				}

				return S_TRAIL;
			}
			case S_TRAIL:
			{
				if (outcome == OUT_FOUND || outcome == OUTCOME_DONE)
				{
					run.Note("{lost}'s road ends at a farm gate past {town}: a boot on the verge, a satchel at a fence, then the gate, oiled and standing open.");
					return Skip(gm, run, S_GUEST);
				}

				run.Note("Nobody got to the end of {lost}'s road. Somebody might try again.");
				return S_TRAIL;
			}
			case S_GUEST:
			{
				if (outcome == OUT_ATE || outcome == OUTCOME_DONE)
					run.Note("You ate at the long table. Grace was a name. There are boots by the door in too many sizes.");
				else if (outcome == OUT_WATCHED)
					run.Note("From the rise you watched them carry something wrapped in a sheet to the cellar door. Nobody said a word.");
				else if (outcome == OUT_SEEN)
					run.Note("{host} saw you watching the house. He waved. They'll be careful now.");
				else if (outcome == OUT_BLESSED)
					run.Note("They blessed your bite and sent you on, kindly, like people who learned the hard way what the bitten do to a house.");
				else
				{
					run.Note("Nobody went to meet {host}. The road past {town} is still kept.");
					return S_GUEST;
				}

				if (outcome == OUT_SEEN)
					run.SetFlag("wary", 1);

				return Skip(gm, run, S_PROOF);
			}
			case S_PROOF:
			{
				if (outcome == OUT_PROVEN || outcome == OUTCOME_DONE)
				{
					run.Note("Three things at the farm, and they all say the same thing. You know what the {family} farm is.");
					if (run.Flag("cellar") == 1)
						run.Note("In the cellar: jars, each with a day written on the lid, and nothing else.");

					return S_TELL;
				}

				if (outcome == OUT_CAUGHT)
				{
					run.SetFlag("wary", 1);
					run.Note("They caught you at it, and shut the house. Next time, go in at supper.");
				}

				return S_PROOF;
			}
			case S_TELL:
			{
				// Without a choice made (a test, an ending forced), it's the Haven's
				int faction = outcome - OUT_TOLD;
				if (outcome == OUTCOME_DONE)
					faction = RZ_EFaction.SURVIVORS;

				if (outcome == OUT_SELF)
				{
					run.SetBranch(BRANCH_SELF);
					return S_END;
				}

				if (RZ_Factions.IsValid(faction))
				{
					run.SetBranch(faction);
					run.SetFlag(TellFlag(faction), 1);
					gm.GetQuestlines().SetWorldFlag(RZ_LongTable.C1_KEY + "." + TellFlag(faction), 1);
					if (outcome == OUTCOME_DONE)
						t.Expose(faction, -1);

					return S_END;
				}

				run.Note("Nobody has been told yet.");
				return S_TELL;
			}
		}

		return Fizzle(run, "{fizzle}");
	}

	//------------------------------------------------------------------------------------------------
	//! The proof from another line was found meanwhile: past the stages it makes moot.
	protected int Skip(RZ_GameModeComponent gm, RZ_QuestRun run, int next)
	{
		if (ProofFromCrossing(gm) || Table(gm).IsExposed())
		{
			run.SetFlag("proof_crossing", 1);
			return S_TELL;
		}

		return next;
	}

	//------------------------------------------------------------------------------------------------
	protected int NextFromEnd(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_BY_HAND || (outcome == OUTCOME_DONE && run.m_iBranch == BRANCH_SELF))
		{
			if (!Gone(gm))
				Table(gm).BreakUp(RZ_ETableEnd.BY_HAND, run.Flag("boy_out") == 1, false, run.Fill("The lamp at the {family} farm past {town} didn't come on last night. Nobody has been up to see why."));

			run.Note("The {family} farm is finished, by your own hand.");
			return S_AFTER;
		}

		if (outcome == OUT_ENDED || outcome == OUTCOME_DONE)
		{
			switch (run.m_iBranch)
			{
				case RZ_EFaction.SURVIVORS:
				{
					if (run.Flag("boy_out") == 1)
						return Finish(run, E_WALKED, "Walked out. The {family} family walked to the edge of town in their good coats, and on toward the trees. {boy} stayed.");

					return Finish(run, E_WALKED, "Walked out. The {family} family walked to the edge of town in their good coats, and on toward the trees.");
				}
				case RZ_EFaction.MILITARY: return Finish(run, E_BURNT, "Burnt. Bastion's mortars took the {family} farm past {town}, and Bastion wrote it up as an infected cluster.");
				case RZ_EFaction.INSTITUTE: return Finish(run, E_STUDIED, "Studied. {volunteer} is at the lab, with the kitchen notebook. The rest of them went their own ways in the night.");
				case RZ_EFaction.BANDITS: return Finish(run, E_PAID, "Paid to go. A Red Hand cart took the {family} family away down the back lane. Somewhere else, there's a place set.");
			}
		}

		// The house broke up some other way meanwhile
		if (Gone(gm))
			return Fizzle(run, "{fizzle}");

		run.Note("The end of the {family} farm didn't happen this time. It still sets its table.");
		return S_END;
	}

	//------------------------------------------------------------------------------------------------
	static string TellFlag(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "haven";
			case RZ_EFaction.MILITARY: return "bastion";
			case RZ_EFaction.BANDITS: return "redhand";
			case RZ_EFaction.INSTITUTE: return "halcyon";
		}

		return "nobody";
	}

	//------------------------------------------------------------------------------------------------
	//! What each ending changes, through the existing systems (the house: World/RZ_LongTable.c).
	//! Each also publishes a fact as a world flag (table.walked_out ...) for other content.
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		RZ_LongTable t = Table(gm);
		RZ_Questlines q = gm.GetQuestlines();
		bool boyOut = run.Flag("boy_out") == 1;
		switch (ending)
		{
			case E_WALKED:
			{
				if (t)
					t.BreakUp(RZ_ETableEnd.WALKED_OUT, true, false, "Every Haven house lit a candle last night. Nobody said who for.");

				NudgeMorale(gm, RZ_EFaction.SURVIVORS, 5);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 4, 0);
				SpreadRumour(gm, "\"Every house lit a candle last night. Don't ask me who for. We don't say.\"", 12);
				q.SetWorldFlag("table.walked_out", 1);
				return;
			}
			case E_BURNT:
			{
				if (t)
					t.BreakUp(RZ_ETableEnd.BURNT, boyOut, false, run.Fill("Bastion reports an infected cluster cleared past {town}."));

				NudgeMorale(gm, RZ_EFaction.MILITARY, 3);
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 4, 0);
				if (!boyOut)
					q.SetWorldFlag("table.boy_lost", 1);

				return;
			}
			case E_STUDIED:
			{
				bool host = t && t.IsAlive(RZ_ETableRole.HOST);
				gm.GetCure().AddResearch(2, "Halcyon has a new volunteer, and a kitchen notebook it reads with the door shut.");
				if (t)
					t.BreakUp(RZ_ETableEnd.STUDIED, true, true, run.Fill("Halcyon has a volunteer from the {family} farm. The rest of the family went their own ways in the night."));

				ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 4, 0);
				q.SetWorldFlag("table.studied", 1);
				if (host)
					q.SetWorldFlag("table.host_wanders", 1);

				return;
			}
			case E_PAID:
			{
				if (t)
					t.StartMove(true, run.Fill("The {family} farm past {town} is empty. Nobody saw them go, and for once the table was cleared."));

				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 4, 0);
				SpreadRumour(gm, "\"The Red Hand moved a family on last night, cart and all. Long table on its side in the back. Nobody's saying where to.\"", 8);
				q.SetWorldFlag("table.paid", 1);
				return;
			}
			case E_BY_HAND:
			{
				SpreadRumour(gm, run.Fill("\"Somebody went up to the farm past {town} on their own. The lamp hasn't come on since.\""), 8);
				q.SetWorldFlag("table.by_hand", 1);
				if (!boyOut)
					q.SetWorldFlag("table.boy_lost", 1);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RefreshTexts(gm, run);
		if (Gone(gm) && run.m_iStage != S_AFTER)
			return run.Text("fizzle");

		RZ_LongTable t = Table(gm);
		if (t && t.GetState() == RZ_ETableState.QUIET)
			return run.Fill("The farm past {town} has gone dark. Nobody's at the table. Wherever they've gone, they'll set a place there too.");

		return super.JournalText(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (Gone(gm))
			return "";

		if (run.m_iStage <= S_PROOF)
			return "\"There's a farm past {town} that still sets a place at supper. In times like these. Makes you wonder what they've got to spare.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Who to tell: a leader of the faction you tell, talked to by someone who knows. After "by
	//! your own hand": the proof is still worth half that faction's Fame, once. Before that, the
	//! Haven's people say one thing about the house that went quiet.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (!l)
			return "";

		bool knows = run.PlayerTookPart(gm, playerId) || run.Flag("proof_crossing") == 1;
		if (run.m_iStage == S_TELL && knows)
		{
			RZ_QM_TableTell m = RZ_QM_TableTell.Cast(gm.GetMissions().FindById(run.m_iMissionId));
			if (m)
				return m.Told(gm, playerId, l);

			return "";
		}

		if (run.m_iStage == S_AFTER && run.PlayerTookPart(gm, playerId))
		{
			string key = "after_" + gm.GetProfile(playerId).m_sIdentity;
			if (run.Flag(key) == 1)
				return "";

			run.SetFlag(key, 1);
			int fame = TELL_FAME / 2;
			if (l.m_eFaction == RZ_EFaction.BANDITS)
				fame = TELL_FAME_RED_HAND / 2;

			run.Reward(gm, playerId, l.m_eFaction, fame, 0, run.Fill(string.Format("%1 listens to what you found at the {family} farm, and writes some of it down.", l.m_sShort)));
			return string.Format("%1 hears you out. \"It's done, then.\" A long pause. \"Good.\"", l.m_sShort);
		}

		if (run.m_iStage < S_TELL && l.m_eFaction == RZ_EFaction.SURVIVORS && run.PlayerTookPart(gm, playerId))
			return string.Format("%1 goes still when you mention the farm. \"Past {town}? That was one of ours, once. The house that went quiet.\" That's all they'll say.", l.m_sShort);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The fame for taking the proof to `faction` (the Red Hand pays a little).
	static int TellFame(int faction)
	{
		if (faction == RZ_EFaction.BANDITS)
			return TELL_FAME_RED_HAND;

		return TELL_FAME;
	}

	//------------------------------------------------------------------------------------------------
	static int UndercoverFame()
	{
		return UNDERCOVER_FAME;
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 1, Never arrived: the last road of someone who never arrived, as a step chain: a boot
//! on the verge, a satchel at a fence, the farm gate. ★ Hunter or Police: each clue gives the
//! next one's grid.
class RZ_QM_TableTrail : RZ_QM_TableBase
{
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected ref array<vector> m_aSpots = {};
	protected int m_iBoot = -1;
	protected int m_iSatchel = -1;
	protected int m_iGate = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(3600);
		m_Req.Bonus("hunter");
		m_Req.Bonus("police");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen())
			return false;

		vector house = t.GetHousePos();
		vector lost = t.GetLostPos();
		float away = RZ_Util.DistXZ(lost, house);
		if (lost == vector.Zero || away < 600 || away > 3000)
			lost = LandPointNear(house, 1000, 1400);

		vector boot = LandPointNear(lost, 0, 15);
		vector mid = boot + (house - boot) * 0.55;
		vector satchel = LandPointNear(mid, 0, 50);
		vector gate = LandPointNear(house, 45, 70);
		m_aSpots.Insert(boot);
		m_aSpots.Insert(satchel);
		m_aSpots.Insert(gate);

		m_iBoot = AddClue("A boot on the verge", QFill(gm, "A walking boot on the verge, good leather, laced tight. {lost} had boots like that, people say. Nobody walks on with one boot. The grass is pressed flat toward {town}."));
		m_iSatchel = AddClue("A satchel at a fence", QFill(gm, "A satchel hooked on a fence post, as if someone meant to come back for it. A hairbrush, a tin of tea, a letter addressed in {from} and never posted. The fence runs on toward a farm with smoke in the chimney."));
		m_iGate = AddClue("The farm gate", QFill(gm, "The gate is oiled and standing open. Beyond it a yard swept clean, washing on the line, and through the window a long table laid for twenty. Somebody inside is singing."));

		AddStep(gm, boot, m_iBoot, "Look at the boot on the verge");
		AddStep(gm, satchel, m_iSatchel, "Look at the satchel on the fence");
		AddStep(gm, gate, m_iGate, "Look at the gate");
		m_Chain.SetMission(m_iId);

		m_vPos = boot;
		m_iSettlement = gm.FindNearestSettlement(boot);
		m_sTitle = "Never arrived";
		m_sBrief = QFill(gm, "{lost} left {from} for {to} and never got there. Their last road runs past {town}. Start where they were last seen, and see where it goes.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddStep(RZ_GameModeComponent gm, vector pos, int clue, string label)
	{
		RZ_ChainStep st = m_Chain.AddStep(gm, gm.FindNearestSettlement(pos), pos, 20, 0);
		st.m_iClue = clue;
		st.m_sReadLabel = label;
	}

	//------------------------------------------------------------------------------------------------
	//! ★ Hunter or Police: where the next thing is, as a grid.
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (!HasBonus(gm, playerId))
			return text;

		if (idx == m_iBoot)
			return text + string.Format("\nYou read the verge like a page: the trail goes on to grid %1.", RZ_SettlementScanner.GridReference(m_aSpots[1]));

		if (idx == m_iSatchel)
			return text + string.Format("\nThe fence line runs to grid %1.", RZ_SettlementScanner.GridReference(m_aSpots[2]));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		OfferAround(gm, dt, true);
		int done = m_Chain.Update(gm, dt, ids, players, m_aParticipants);
		if (done < 0)
			return;

		array<int> walkers = {};
		walkers.Copy(m_aParticipants);
		foreach (int playerId : walkers)
		{
			Remember(gm, playerId, RZ_EQuestApproach.NONE, "followed the last road");
		}

		if (m_Chain.IsDone())
		{
			int finder = PlayerNear(ids, players, m_aSpots[2], 40);
			if (finder <= 0 && !m_aParticipants.IsEmpty())
				finder = m_aParticipants[0];

			Resolve(gm, finder, RZ_EQuestApproach.NONE, RZ_Quest_HouseWentQuiet.OUT_FOUND, "followed the last road to the farm gate");
			return;
		}

		// On to the next thing on the road (the journal follows it)
		m_vPos = m_aSpots[m_Chain.GetCurrentIndex()];
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
		RZ_LongTable t = Table(gm);
		if (t)
			t.Notice(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Chain.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Chain.Cleanup();
		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Never arrived: {lost}'s road ends at a farm gate past {town}.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Never arrived: nobody followed {lost}'s road to the end.");
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 2, Guest: the host (or whoever meets the road now) on the lane to the farm. Accept and
//! eat (the meal at the house; the guest rule watches the rest of the visit), decline and watch
//! the house from a rise unseen, or declare a bite.
class RZ_QM_TableGuest : RZ_QM_TableBase
{
	protected vector m_vHouse;
	protected vector m_vMeet;
	protected vector m_vWatch;
	protected int m_iGreeterRole = -1;
	protected int m_iTalk = -1;
	protected int m_iGuest = -1;		// accepted: walking up to the table
	protected ref array<int> m_aDeclined = {};
	protected ref map<int, float> m_mWatched = new map<int, float>();

	protected static const int OBJ_HOST = 0;
	protected static const float WATCH_SECONDS = 60;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(3600);
		m_Req.Bonus("nurse");
		m_Req.Bonus("elder");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen())
			return false;

		RZ_TableMember greeter = t.Greeter();
		if (!greeter)
			return false;

		m_iGreeterRole = greeter.m_eRole;
		if (!HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		RZ_Settlement s = gm.GetSettlement(t.GetSettlement());
		vector toward = m_vHouse;
		if (s && RZ_Util.DistXZ(s.m_vCenter, m_vHouse) > 50)
			toward = s.m_vCenter;
		else
			toward = RZ_Util.RandomPointInRing(m_vHouse, 300, 400);

		float bearing = RZ_Compass.Bearing(m_vHouse, toward);
		m_vMeet = LandPointNear(RZ_Compass.PointAt(m_vHouse, bearing, 160), 0, 20);
		m_vWatch = HighPointNear(RZ_Compass.PointAt(m_vHouse, bearing + 180, 110), 0, 30);
		m_iTalk = AddClue(greeter.m_sName, QFill(gm, "{host} is leaning on a gatepost with a flask. \"Cold out. Here.\" Tea, sweet, still hot. \"Lost someone? Everybody has.\" He doesn't ask how you came to be on this road. \"There's always a place set at ours. Supper's at half six. Will you eat with us?\""));

		m_vPos = m_vMeet;
		m_iSettlement = t.GetSettlement();
		m_Quiet.Watch(m_vHouse, 160);
		m_sTitle = "Guest";
		m_sBrief = QFill(gm, "There's a farm past {town} with smoke in the chimney. {host} keeps the road to it, with a flask of something hot and an invitation. Eat with them, or decline and watch the house unseen.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		return SpawnMember(gm, m_iGreeterRole, m_vMeet, OBJ_HOST, m_iTalk, "Accept, and eat with them|Thank him and decline|Tell him about your bite") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_HOST)
			return;

		RZ_LongTable t = Table(gm);
		t.Notice(playerId);
		if (option == 0)
		{
			m_iGuest = playerId;
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "accepted the invitation");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"Good. Mother will be pleased.\" He walks you up the lane to the farm and holds the door. The table's laid. Go in and sit down."));
			m_vPos = m_vHouse;
			return;
		}

		if (option == 1)
		{
			if (!m_aDeclined.Contains(playerId))
				m_aDeclined.Insert(playerId);

			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "declined the invitation");
			IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			string dir = "behind the farm";
			if (user)
				dir = RZ_Util.DirectionText(user.GetOrigin(), m_vWatch);

			gm.NotifyPlayer(playerId, m_sTitle, string.Format("He doesn't push. \"Another time. The door's always open.\" When you look back from the bend in the lane, he's still at the gate, watching the road.\nIf you want to see what the house does when it thinks nobody's looking, there's a rise %1. Lie up there out of sight for a minute and watch.", dir));
			return;
		}

		if (option != 2)
			return;

		if (!gm.GetProfile(playerId).IsInfected())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"No bite? Good. Then you'll eat with us.\" He asks it kindly, the way every Haven house does.");
			return;
		}

		t.Remember(playerId, RZ_LongTable.MEM_BLESSED);
		SetQFlag(gm, "boots_seen", 1);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{host} looks at your arm, and something in his face goes careful. \"Thank you for saying.\" He takes you as far as the yard, where {mother} blesses you with her hand on your head and gives you bread in a cloth. By the door there's a row of boots, in too many sizes. \"Go on, now, while it's light.\""));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_HouseWentQuiet.OUT_BLESSED, "declared a bite, and was blessed and sent on");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		OfferAround(gm, dt, true);
		CheckFamilyBodies(gm);
		if (m_bResolved)
			return;

		RZ_LongTable t = Table(gm);
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];

			// The guest at the table: the meal
			if (playerId == m_iGuest && RZ_Arrive.InArea(p, m_vHouse, 0))	// at the house (RZ_Arrive: a building)
			{
				t.RecordMeal(playerId);
				gm.NotifyPlayer(playerId, t.FarmTitle(), QFill(gm, "{mother} puts a bowl in front of you before you've sat down. \"Sit, sit. You're thin.\" Stew, thick, hot, more than you've seen in a week. The long table has places for twenty, and most of them are set. " + t.Grace() + " By the door there's a row of boots, in too many sizes."));
				SetQFlag(gm, "boots_seen", 1);
				Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_HouseWentQuiet.OUT_ATE, "ate at the long table");
				return;
			}

			// Declined, and lying up on the rise: a minute unseen
			if (!m_aDeclined.Contains(playerId) || RZ_Util.DistXZ(p.GetOrigin(), m_vWatch) > 25)
				continue;

			if (!m_Quiet.IsQuiet(gm, playerId))
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{host} is standing at the gate, looking straight up at where you're lying. After a while he lifts a hand, the way you'd wave to a neighbour."));
				Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_HouseWentQuiet.OUT_SEEN, "was seen watching the house", false);
				return;
			}

			float watched;
			m_mWatched.Find(playerId, watched);
			watched += dt;
			m_mWatched.Set(playerId, watched);
			if (watched < WATCH_SECONDS)
				continue;

			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "From the rise you watch the yard for a long time. {boy} fetches water. Then {son} carries something long, wrapped in a sheet, from the barn to the cellar door, and {mother} holds the door for him, and neither of them says a word. A bell rings for supper. Everyone goes in, smiling."));
			Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_HouseWentQuiet.OUT_WATCHED, "watched the house unseen");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMemberKilled(RZ_GameModeComponent gm, int role, int killer)
	{
		if (m_bResolved)
			return;

		NotifyParticipants(gm, m_sTitle, "Somebody shot the man with the flask. The farm's door is bolted now, and the lamp is out.");
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Guest: you've been to the farm past {town}.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Guest: nobody went up to the farm past {town} this time.");
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 3, Proof: three of the guestbook, the boots, the letters, the cellar, or the boy's one
//! thing. At supper the house is at the table and nobody sees you; otherwise a family member
//! within 20 m, shots or the infected's notice and you're caught: they set on you (a fight with
//! the fight prefab) or shut the house. The cellar: the mother's key (the boy says where it is)
//! or a crowbar, which they hear. A fight you win opens the house.
class RZ_QM_TableProof : RZ_QM_TableBase
{
	protected vector m_vHouse;
	protected vector m_vBoy;
	protected int m_iBook = -1;
	protected int m_iBoots = -1;
	protected int m_iLetters = -1;
	protected int m_iCellarLook = -1;
	protected int m_iBoyTalk = -1;
	protected int m_iFound;			// bitmask of the pieces
	protected bool m_bKeyKnown;
	protected bool m_bCellarOpen;
	protected bool m_bOpened;		// the house is open: they're dead, gone, or fought off
	protected bool m_bFighting;
	protected bool m_bMenOut;
	protected int m_iLastApproach = RZ_EQuestApproach.SNEAK;

	protected static const int OBJ_CELLAR = 0;
	protected static const int OBJ_MOTHER = 1;
	protected static const int OBJ_BOY = 2;
	protected static const int OBJ_HOST = 3;
	protected static const int OBJ_SON = 4;
	protected static const int P_BOOK = 1;
	protected static const int P_BOOTS = 2;
	protected static const int P_LETTERS = 4;
	protected static const int P_CELLAR = 8;
	protected static const int P_BOY = 16;
	protected static const float SEEN_RADIUS = 20;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(3600);
		m_Req.Bonus("nurse");
		m_Req.Bonus("elder");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		m_vBoy = LandPointNear(m_vHouse, 160, 220);
		string last = "the last one is from three days ago";
		if (!t.GetLostName().IsEmpty() && t.GetLostRole() != RZ_LongTable.ROLE_NO_BAG)
			last = "the last one is " + t.ShortLost();

		m_iBook = AddClue("The guestbook", QFill(gm, "The guestbook, on a shelf by the door. Every guest has signed it, in pencil, in ink, in a child's capitals. Some names have a date beside them and a small cross, the way the Haven's List does it. ") + RZ_Util.Capitalize(last) + ".");
		m_iBoots = AddClue("The boots", "A row of boots by the door, polished and paired, in too many sizes: work boots, a woman's walking boots, a child's wellingtons, a soldier's boots with the laces still in. Nobody in this house has feet that size.");
		m_iLetters = AddClue("The dresser drawer", QFill(gm, "In the dresser drawer, under the tea towels: letters, a dozen of them, tied with string. All addressed, all stamped by hand at the Haven's post table, none of them delivered. The top one is from {from}."));
		m_iCellarLook = AddClue("The cellar door", "A cellar door with a new padlock. The key isn't in it. It's cold, even through the wood.");
		m_iBoyTalk = AddClue(QText(gm, "boy"), QFill(gm, "{boy} is skimming stones at the stream, badly. He doesn't look round."));
		if (QFlag(gm, "boots_seen") == 1)
			m_iFound = P_BOOTS;

		int wary = QFlag(gm, "wary");
		m_Quiet.Watch(m_vHouse, 120);
		m_vPos = m_vHouse;
		m_iSettlement = t.GetSettlement();
		m_sTitle = "Proof";
		m_sBrief = QFill(gm, "Find three things at the {family} farm that say what it is: the guestbook, the boots by the door, a drawer, the cellar. At supper, half past six, they're all at the table. {boy} is at the stream, away from the house.");
		if (wary == 1)
			m_sBrief += " They've seen you before. They'll be watching.";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (!SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(m_vHouse, 2, 4), m_iBook, "Read the guestbook"))
			return false;

		if ((m_iFound & P_BOOTS) == 0)
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(m_vHouse, 3, 5), m_iBoots, "Look at the boots by the door");

		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(m_vHouse, 1, 3), m_iLetters, "Open the dresser drawer");
		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, LandPointNear(m_vHouse, 5, 9), OBJ_CELLAR, "Open it with the mother's key|Force it with a crowbar", "", m_iCellarLook, "Look at the cellar door");
		SpawnMember(gm, RZ_ETableRole.MOTHER, LandPointNear(m_vHouse, 2, 4), OBJ_MOTHER, -1, "");
		SpawnMember(gm, RZ_ETableRole.BOY, m_vBoy, OBJ_BOY, m_iBoyTalk, "Ask him about the house");
		UpdateMen(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! At supper the men are at the table (no bodies outside); otherwise in the yard.
	protected void UpdateMen(RZ_GameModeComponent gm)
	{
		bool want = !IsSupper() && !m_bOpened && !m_bFighting;
		if (want == m_bMenOut)
			return;

		m_bMenOut = want;
		if (!want)
		{
			RemoveMembers(RZ_ETableRole.HOST);
			RemoveMembers(RZ_ETableRole.SON);
			return;
		}

		SpawnMember(gm, RZ_ETableRole.HOST, LandPointNear(m_vHouse, 8, 15), OBJ_HOST, -1, "");
		SpawnMember(gm, RZ_ETableRole.SON, LandPointNear(m_vHouse, 8, 18), OBJ_SON, -1, "");
	}

	//------------------------------------------------------------------------------------------------
	//! Seen: not at supper, one of them within 20 m; or shots, heat, the infected's notice.
	protected bool IsSeen(RZ_GameModeComponent gm, int playerId)
	{
		if (m_bOpened)
			return false;

		if (!m_Quiet.IsQuiet(gm, playerId))
			return true;

		if (IsSupper())
			return false;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent)
			return false;

		float radius = SEEN_RADIUS;
		if (QFlag(gm, "wary") == 1)
			radius = SEEN_RADIUS * 1.5;

		radius = RZ_Stealth.SightRadius(playerId, radius);
		foreach (IEntity e : m_aFamilyBodies)
		{
			if (e && RZ_Util.DistXZ(e.GetOrigin(), ent.GetOrigin()) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		int piece = 0;
		if (idx == m_iBook)
			piece = P_BOOK;
		else if (idx == m_iBoots)
			piece = P_BOOTS;
		else if (idx == m_iLetters)
			piece = P_LETTERS;

		if (piece == 0 || (m_iFound & piece) != 0 || m_bResolved)
			return;

		if (IsSeen(gm, playerId))
		{
			Caught(gm, playerId);
			return;
		}

		Found(gm, playerId, piece, RZ_EQuestApproach.SNEAK, "searched the house unseen");
	}

	//------------------------------------------------------------------------------------------------
	protected void Found(RZ_GameModeComponent gm, int playerId, int piece, int approach, string what)
	{
		m_iFound = m_iFound | piece;
		if (approach != RZ_EQuestApproach.SNEAK || m_iLastApproach == RZ_EQuestApproach.SNEAK)
			m_iLastApproach = approach;

		Remember(gm, playerId, approach, what);
		Table(gm).Sanction(playerId);
		int count = 0;
		array<int> bits = {P_BOOK, P_BOOTS, P_LETTERS, P_CELLAR, P_BOY};
		foreach (int b : bits)
		{
			if ((m_iFound & b) != 0)
				count++;
		}

		if (count < 3)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("That's %1 of three.", count));
			return;
		}

		foreach (int pid : m_aParticipants)
		{
			Table(gm).Sanction(pid);
		}

		if (m_bFighting || m_bOpened)
			SetQFlag(gm, "fought", 1);

		Resolve(gm, playerId, m_iLastApproach, RZ_Quest_HouseWentQuiet.OUT_PROVEN, "found what the house is");
	}

	//------------------------------------------------------------------------------------------------
	//! Caught at it: they set on you (a fight, if there's the prefab for it), else the house shuts.
	protected void Caught(RZ_GameModeComponent gm, int playerId)
	{
		RZ_LongTable t = Table(gm);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{son} is in the doorway. He doesn't shout. He looks at what's in your hands, and then at you, and {mother} says, from the kitchen, \"Shut the door, love. It's letting the cold in.\""));
		Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was caught at it");
		t.Remember(playerId, RZ_LongTable.MEM_CROSSED);
		array<int> targets = {playerId};
		if (!m_bFighting && t.StartFight(targets, m_vHouse, true))
		{
			m_bFighting = true;
			RemoveMembers(RZ_ETableRole.HOST);
			RemoveMembers(RZ_ETableRole.SON);
			m_bMenOut = false;
			return;
		}

		if (m_bFighting)
		{
			t.JoinFight(playerId);
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "The door shuts, and the bolt goes across. Nobody comes out again while you're there. You won't get in today.");
		Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_HouseWentQuiet.OUT_CAUGHT, "was caught, and the house was shut", false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_BOY || option != 0 || m_bResolved)
			return;

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (user && RZ_Util.IsArmed(user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{boy} looks at the gun, not at you, and goes back up to the house."));
			return;
		}

		if (!RZ_QuestApproach.HasBackground(gm, playerId, "nurse,elder"))
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "tried to talk to the boy");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{boy} looks at you and says nothing. Then: \"I'm not supposed to talk to guests.\" He skims another stone."));
			return;
		}

		if ((m_iFound & P_BOY) != 0)
			return;

		m_bKeyKnown = true;
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You sit by the water and don't ask anything for a while. He talks to the stream, not to you. \"Mr {watch} had a watch. {mother} wears it now.\" A stone. \"The key for downstairs is on the nail inside the stove door.\" He goes back to the house without looking round."));
		Found(gm, playerId, P_BOY, RZ_EQuestApproach.TALK, "talked to the boy");
	}

	//------------------------------------------------------------------------------------------------
	//! The cellar: the key quietly (only once you know where it hangs), or a crowbar, loud.
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_CELLAR || m_bCellarOpen || m_bResolved)
			return;

		if (value == 0)
		{
			if (!m_bKeyKnown && !m_bOpened)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "The key isn't in the lock, and it isn't anywhere you can see. Somebody in the house knows where it hangs.");
				return;
			}

			if (IsSeen(gm, playerId))
			{
				Caught(gm, playerId);
				return;
			}

			OpenCellar(gm, playerId, RZ_EQuestApproach.SNEAK);
			return;
		}

		string words = "crowbar";
		RZ_Interactions hub = RZ_Interactions.Get();
		if (hub && hub.GetSearch())
			words = hub.GetSearch().GetCrowbarWords();

		if (!user || !RZ_ItemHelper.Has(user, words))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You'd need a crowbar for that padlock.");
			return;
		}

		// Loud: the house hears it unless there's nobody left to hear
		gm.GetHeat().AddHeat(m_vHouse, 25);
		OpenCellar(gm, playerId, RZ_EQuestApproach.FORCE);
		if (!m_bOpened && !m_bResolved)
			Caught(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenCellar(RZ_GameModeComponent gm, int playerId, int approach)
	{
		m_bCellarOpen = true;
		gm.NotifyPlayer(playerId, "The cellar", "Steps down into the cold. Shelves, and jars on the shelves, each with a day written on the lid in the same careful hand, and nothing else. You count the days back to when the Zone was sealed. You stop counting.");
		Table(gm).Sanction(playerId);
		SetQFlag(gm, "cellar", 1);
		Found(gm, playerId, P_CELLAR, approach, "opened the cellar");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		OfferAround(gm, dt, true);
		CheckFamilyBodies(gm);
		if (m_bResolved)
			return;

		if (m_bSceneUp)
			UpdateMen(gm);

		RZ_LongTable t = Table(gm);

		// The house is gone (a fight broke it): whatever was found is enough
		if (!t.Exists())
		{
			SetQFlag(gm, "fought", 1);
			int any = PlayerNear(ids, players, m_vHouse, 300);
			if (any <= 0 && !m_aParticipants.IsEmpty())
				any = m_aParticipants[0];

			Resolve(gm, any, RZ_EQuestApproach.FORCE, RZ_Quest_HouseWentQuiet.OUT_PROVEN, "broke the house open");
			return;
		}

		// Forcing their way in: shots at the house start the fight (they didn't set on you)
		if (!m_bFighting && !m_bOpened && m_bSceneUp && m_Quiet.WasLoud())
		{
			array<int> near = {};
			GetPlayersNear(ids, players, m_vHouse, 150, near);
			if (!near.IsEmpty() && t.StartFight(near, m_vHouse, false))
			{
				m_bFighting = true;
				RemoveMembers(RZ_ETableRole.HOST);
				RemoveMembers(RZ_ETableRole.SON);
				m_bMenOut = false;
				foreach (int fighter : near)
				{
					Remember(gm, fighter, RZ_EQuestApproach.FORCE, "forced their way in");
				}

				NotifyParticipants(gm, m_sTitle, "The lamp goes out, and the door bangs shut, and then it opens again.");
			}
		}

		if (m_bFighting && !t.IsFighting())
		{
			m_bFighting = false;
			if (t.FightResult() == 1)
			{
				m_bOpened = true;
				m_iLastApproach = RZ_EQuestApproach.FORCE;
				SetQFlag(gm, "fought", 1);
				NotifyParticipants(gm, m_sTitle, "Nobody is standing between you and the house any more. Take your time.");
			}
			else
			{
				Resolve(gm, PlayerNear(ids, players, m_vHouse, 2000), RZ_EQuestApproach.FORCE, RZ_Quest_HouseWentQuiet.OUT_CAUGHT, "was driven off the farm", false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMemberKilled(RZ_GameModeComponent gm, int role, int killer)
	{
		if (role == RZ_ETableRole.MOTHER)
			m_bOpened = true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Proof: you know what the {family} farm is.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Proof: the {family} farm keeps its secrets, for now.");
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 4, Who to tell: anyone who knows talks to a leader of the faction they choose
//! (RZ_Quest_HouseWentQuiet.OnLeaderTalk calls Told), or goes back to the farm gate to deal with
//! it themselves (needs the fight prefab or the talk: by your own hand).
class RZ_QM_TableTell : RZ_QM_TableBase
{
	protected int m_iGateTalk = -1;

	protected static const int OBJ_GATE = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(7200);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen())
			return false;

		m_vPos = LandPointNear(t.GetHousePos(), 50, 70);
		m_iSettlement = t.GetSettlement();
		m_iGateTalk = AddClue("The farm gate", QFill(gm, "The gate of the {family} farm, oiled and standing open. The smoke goes up from the chimney the same as always. Nobody's told anyone yet."));
		m_sTitle = "Who to tell";
		m_sBrief = QFill(gm, "You know what the {family} farm past {town} is. Tell a leader who'll end it: a Haven council head, a Bastion commander, a Halcyon researcher, a Red Hand crew boss. Each will end it their own way. Or come back to the farm gate and do it yourself.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		return SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vPos, OBJ_GATE, "", "Deal with it yourself", m_iGateTalk, "Look at the gate") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		OfferAround(gm, dt, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_GATE || option != 0)
			return;

		RZ_QuestRun run = Run(gm);
		if (run && !run.PlayerTookPart(gm, playerId) && run.Flag("proof_crossing") != 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "It's an ordinary farm gate. You don't know anything about this house that would make you do that.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You don't tell anyone. You'll come back to the {family} farm yourself, and end it."));
		Resolve(gm, playerId, RZ_EQuestApproach.FORCE, RZ_Quest_HouseWentQuiet.OUT_SELF, "kept it to themselves, to end it by their own hand");
	}

	//------------------------------------------------------------------------------------------------
	//! A leader of `l`'s faction hears the proof from this player: the house is found out, the
	//! teller gets that faction's Fame (the Red Hand's, a little; +5 more for C2's undercover
	//! guest). Returns the leader's answer, "" if this leader can't take it.
	string Told(RZ_GameModeComponent gm, int playerId, RZ_Leader l)
	{
		if (m_bResolved || !l || !RZ_Factions.IsValid(l.m_eFaction))
			return "";

		int f = l.m_eFaction;
		if (f == RZ_EFaction.INSTITUTE && !gm.HasLab())
			return string.Format("%1 listens to the end. \"We'd want one of them alive. There's nowhere to keep her.\" They won't take it further.", l.m_sShort);

		if (f == RZ_EFaction.MILITARY && !Table(gm).BastionAround())
			return "";

		RZ_LongTable t = Table(gm);
		AddParticipant(playerId);
		t.Remember(playerId, RZ_LongTable.MEM_TOLD | RZ_LongTable.MEM_CROSSED);
		t.Expose(f, playerId);

		int fame = RZ_Quest_HouseWentQuiet.TellFame(f);
		if (t.Remembers(playerId, RZ_LongTable.MEM_UNDERCOVER))
			fame += RZ_Quest_HouseWentQuiet.UndercoverFame();

		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, f, fame, 0, QFill(gm, string.Format("%1 knows about the {family} farm now, because of you.", RZ_Factions.GetName(f))));

		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_HouseWentQuiet.OUT_TOLD + f, "told " + RZ_Factions.GetNameInText(f));
		return QFill(gm, TellLine(f, l.m_sShort));
	}

	//------------------------------------------------------------------------------------------------
	protected string TellLine(int f, string who)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("%1 sits down without meaning to. \"We sent them our mouths.\" A long time later: \"Bring them home. All of them. And then walk them out, at dusk, the way we do it.\"", who);
			case RZ_EFaction.MILITARY: return string.Format("%1 writes it in the margin of something else: infected cluster, past {town}. \"We'll clear it. Five minutes' warning, like always. If there's anyone there you'd want out, get them out first.\"", who);
			case RZ_EFaction.INSTITUTE: return string.Format("%1 is quiet for a long time, and then ashamed of the first question. \"Do they eat the bitten?\" Another silence. \"Bring the mother here. As a volunteer. And her kitchen notebook.\"", who);
			case RZ_EFaction.BANDITS: return string.Format("%1 laughs until he stops. \"Pork.\" Then, not laughing: \"The boss'll pay them to move on. Hold the farm while a crew shifts them. Don't make it a story.\"", who);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Who to tell: the {family} farm's secret is out of the house.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Who to tell: nobody has been told about the {family} farm yet.");
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 5, the Haven's way (a Haven contract): at dusk, unarmed, the family walked to the edge
//! of town. {son} won't go: talk him round (persuade, Haven, difficulty 2; ★ Elder or Nurse +1) or
//! let him run. Then they wait at the edge for you (they don't walk with you), and walk on.
class RZ_QM_TableWalkOut : RZ_QM_TableBase
{
	protected vector m_vHouse;
	protected vector m_vEdge;
	protected int m_iMotherTalk = -1;
	protected int m_iSonTalk = -1;
	protected int m_iEdgeTalk = -1;
	protected bool m_bSonDone;
	protected bool m_bWalking;

	protected static const int OBJ_MOTHER = 0;
	protected static const int OBJ_SON = 1;
	protected static const int OBJ_EDGE = 2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureFor(RZ_EFaction.SURVIVORS, 10, 30, 3600);
		m_Req.Bonus("elder");
		m_Req.Bonus("nurse");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !t.IsAlive(RZ_ETableRole.MOTHER) || !HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		RZ_Settlement s = gm.GetSettlement(t.GetSettlement());
		float bearing = Math.RandomFloat(0, 360);
		if (s && RZ_Util.DistXZ(s.m_vCenter, m_vHouse) > 30)
			bearing = RZ_Compass.Bearing(s.m_vCenter, m_vHouse);

		m_vEdge = LandPointNear(RZ_Compass.PointAt(m_vHouse, bearing, 220), 0, 30);
		m_bSonDone = !t.IsAlive(RZ_ETableRole.SON);
		m_iMotherTalk = AddClue(QText(gm, "mother"), QFill(gm, "{mother} has her good coat on already. \"The Haven sent for us, then.\" She doesn't ask who told. \"We'll walk. We know how it's done. Not with a gun at our backs, mind.\""));
		m_iSonTalk = AddClue(QText(gm, "son"), QFill(gm, "{son} is at the woodpile with the axe, not splitting anything. \"I'm not walking anywhere.\""));
		m_iEdgeTalk = AddClue(QText(gm, "mother"), QFill(gm, "They're waiting at the edge of town in their good coats, {boy} in front, the way they were told."));
		m_vPos = m_vHouse;
		m_iSettlement = t.GetSettlement();
		m_sTitle = "The end of the house";
		m_sBrief = QFill(gm, RZ_Quest_HouseWentQuiet.EndText(gm, Run(gm)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (!SpawnMember(gm, RZ_ETableRole.MOTHER, LandPointNear(m_vHouse, 2, 4), OBJ_MOTHER, m_iMotherTalk, "It's time"))
			return false;

		if (!m_bSonDone)
			SpawnMember(gm, RZ_ETableRole.SON, LandPointNear(m_vHouse, 10, 18), OBJ_SON, m_iSonTalk, "Talk him round|Let him go");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_bResolved)
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"The Haven didn't send you.\" Nobody here will go anywhere on your word.");
			return;
		}

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (objIdx == OBJ_SON)
		{
			SonChoice(gm, playerId, option);
			return;
		}

		if (objIdx == OBJ_MOTHER && option == 0)
		{
			if (user && RZ_Util.IsArmed(user))
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{mother} looks at the gun until you put it away. \"Not like that. Not with that.\""));
				return;
			}

			if (!m_bSonDone)
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"Not without {son}.\" She looks at the woodpile, where he's standing with his back to you."));
				return;
			}

			// They go ahead and wait at the edge of town (they don't walk with you)
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "brought them out of the house");
			m_bWalking = true;
			RemoveMembers(RZ_ETableRole.MOTHER);
			SpawnMember(gm, RZ_ETableRole.MOTHER, m_vEdge, OBJ_EDGE, m_iEdgeTalk, "Let them walk on");
			m_vPos = m_vEdge;
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "They come out in their good coats, {boy} in front, and walk ahead of you down the lane without being told. At the edge of town they stop, and wait for you. Come unarmed."));
			return;
		}

		if (objIdx != OBJ_EDGE || option != 0)
			return;

		if (user && RZ_Util.IsArmed(user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{mother} won't look at you while you're carrying that."));
			return;
		}

		if (Table(gm).IsAlive(RZ_ETableRole.BOY))
		{
			SetQFlag(gm, "boy_out", 1);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{mother} turns once, and nods to you as if you'd done her a kindness. Then they walk on toward the trees, all of them but {boy}, who stays at your side and doesn't watch them go."));
		}
		else
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{mother} turns once, and nods to you as if you'd done her a kindness. Then they walk on toward the trees, and nobody looks back."));
		}

		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_HouseWentQuiet.OUT_ENDED, "walked them out");
	}

	//------------------------------------------------------------------------------------------------
	protected void SonChoice(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (m_bSonDone)
			return;

		if (option == 0)
		{
			if (TryPersuade(gm, playerId, RZ_EFaction.SURVIVORS, 2, "elder,nurse"))
			{
				m_bSonDone = true;
				RemoveMembers(RZ_ETableRole.SON);
				Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "talked the quiet son round");
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{son} puts the axe down, very carefully, as if it might break. \"For Mother, then.\" He goes in to get his coat."));
				return;
			}

			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "couldn't talk the quiet son round");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"No.\" {son} doesn't raise his voice. You won't move him with words."));
			return;
		}

		if (option != 1)
			return;

		m_bSonDone = true;
		RemoveMembers(RZ_ETableRole.SON);
		Table(gm).MemberGone(RZ_ETableRole.SON);
		Remember(gm, playerId, RZ_EQuestApproach.TALK, "let the quiet son go");
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{son} goes over the back fence and into the trees, and doesn't look back. Nobody calls after him."));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		CheckFamilyBodies(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMemberKilled(RZ_GameModeComponent gm, int role, int killer)
	{
		if (role == RZ_ETableRole.SON)
		{
			m_bSonDone = true;
			return;
		}

		if (role == RZ_ETableRole.MOTHER && !m_bResolved)
		{
			NotifyParticipants(gm, m_sTitle, "Somebody shot her. That isn't how the Haven does it.");
			m_eState = RZ_EMissionState.FAILED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The end of the house: the {family} family were walked out.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The end of the house: nobody walked the {family} family out tonight.");
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 5, Bastion's way (a Bastion contract): the mortars, five minutes after the warning
//! (given when someone comes within 600 m, or after 15 minutes regardless). Get the boy out first:
//! talk him away, with no infected within 25 m of him. Bastion doesn't check for survivors.
class RZ_QM_TableGuns : RZ_QM_TableBase
{
	protected vector m_vHouse;
	protected int m_iBoyTalk = -1;
	protected float m_fCountdown = -1;
	protected float m_fAge;
	protected bool m_bMinute;

	protected static const int OBJ_BOY = 0;
	protected static const float WARNING = 300;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureFor(RZ_EFaction.MILITARY, 10, 30, 1800);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		m_iBoyTalk = AddClue(QText(gm, "boy"), QFill(gm, "{boy} is sitting on the step, lacing a pair of boots that are much too big for him. \"Are they coming?\""));
		m_vPos = m_vHouse;
		m_iSettlement = t.GetSettlement();
		m_sTitle = "The end of the house";
		m_sBrief = QFill(gm, RZ_Quest_HouseWentQuiet.EndText(gm, Run(gm)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		SpawnMember(gm, RZ_ETableRole.BOY, LandPointNear(m_vHouse, 8, 16), OBJ_BOY, m_iBoyTalk, "Come away from the house");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_BOY || option != 0 || QFlag(gm, "boy_out") == 1)
			return;

		if (InfectedNear(gm, source.GetOrigin(), 25))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "He won't move with them that close. Clear them off first.");
			return;
		}

		SetQFlag(gm, "boy_out", 1);
		RemoveMembers(RZ_ETableRole.BOY);
		Remember(gm, playerId, RZ_EQuestApproach.TALK, "got the boy out");
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{boy} looks at the house, and at you, and comes. He doesn't look back. You leave him with a corporal at the road, who gives him a biscuit and doesn't ask his name."));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		CheckFamilyBodies(gm);
		if (m_bResolved)
			return;

		m_fAge += dt;
		if (m_fCountdown < 0)
		{
			if (CountPlayersNear(players, m_vHouse, 600) == 0 && m_fAge < 900)
				return;

			m_fCountdown = WARNING;
			Warn(gm, players, ids, "Bastion's mortars are laid on the {family} farm past {town}. Five minutes. They don't check for survivors.");
		}

		m_fCountdown -= dt;
		if (!m_bMinute && m_fCountdown <= 60)
		{
			m_bMinute = true;
			Warn(gm, players, ids, "One minute until the mortars open up on the farm past {town}.");
		}

		if (m_fCountdown > 0)
			return;

		// The guns: for anyone watching; as data if nobody is
		array<int> near = {};
		GetPlayersNear(ids, players, m_vHouse, 1000, near);
		if (CountPlayersNear(players, m_vHouse, 600) > 0)
			Table(gm).Shell();

		foreach (int playerId : near)
		{
			AddParticipant(playerId);
		}

		int by = -1;
		if (!near.IsEmpty())
			by = near[0];
		else if (!m_aParticipants.IsEmpty())
			by = m_aParticipants[0];

		NotifyParticipants(gm, m_sTitle, "The first round lands short, in the orchard. Then the rest of them.");
		Resolve(gm, by, RZ_EQuestApproach.SIDE, RZ_Quest_HouseWentQuiet.OUT_ENDED, "was there when the guns came");
	}

	//------------------------------------------------------------------------------------------------
	protected void Warn(RZ_GameModeComponent gm, array<IEntity> players, array<int> ids, string text)
	{
		string filled = QFill(gm, text);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vHouse) <= 1500 || m_aOfferedTo.Contains(ids[i]))
				gm.NotifyPlayer(ids[i], "Bastion", filled);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The end of the house: Bastion cleared the farm past {town}.");
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 5, Halcyon's way (a Halcyon contract): walk the mother (or, without her, one of them
//! alive) to the lab as a "volunteer" (an escort that follows you). The rest scatter.
class RZ_QM_TableVolunteer : RZ_QM_TableBase
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vHouse;
	protected int m_iRole = -1;
	protected int m_iTalk = -1;
	protected bool m_bFollowing;

	protected static const int OBJ_VOLUNTEER = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureFor(RZ_EFaction.INSTITUTE, 10, 20, 3600);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !gm.HasLab() || !RZ_Escort.HasRoom(gm))
			return false;

		m_iRole = RZ_ETableRole.MOTHER;
		if (!t.IsAlive(m_iRole))
		{
			RZ_TableMember other = t.Greeter();
			if (!other)
				return false;

			m_iRole = other.m_eRole;
		}

		if (!HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		string name = t.NameOf(m_iRole);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetText("volunteer", name);

		m_iTalk = AddClue(name, QFill(gm, string.Format("%1 looks at you for a long time. \"They want to know if we were right about the bitten.\" A notebook comes out of the dresser: a kitchen book, recipes in the front. \"I'll come. Nobody else.\"", name)));
		m_Escort.m_sName = name;
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.SetTalk(m_iId, OBJ_VOLUNTEER, "Come with me", m_iTalk, "Talk to " + name);
		if (!m_Escort.Begin(gm, LandPointNear(m_vHouse, 2, 5)))
			return false;

		m_vPos = m_vHouse;
		m_iSettlement = t.GetSettlement();
		m_sTitle = "The end of the house";
		m_sBrief = QFill(gm, string.Format("Halcyon's director wants one of the {family} family alive, and the kitchen notebook: why they never eat the bitten. Walk %1 to the lab as a volunteer.", name));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_VOLUNTEER || option != 0 || m_bResolved)
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"I'll go with the lab's people. Not with you.\"");
			return;
		}

		m_bFollowing = true;
		m_Escort.Follow(playerId);
		m_Escort.Go();
		AddParticipant(playerId);
		Remember(gm, playerId, RZ_EQuestApproach.SIDE, "walked a volunteer to the lab");
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 puts the notebook in a coat pocket and follows you, a few steps behind. The rest of them don't come to the door. The lab is %2.", m_Escort.m_sName, RZ_Util.DirectionText(source.GetOrigin(), gm.GetLabPosition())));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Escort.Update(gm, dt, ids, players);
		if (m_bResolved)
			return;

		if (m_Escort.IsDead())
		{
			Table(gm).MemberKilled(m_iRole, -1, m_Escort.m_vPos);
			NotifyParticipants(gm, m_sTitle, m_Escort.m_sName + " didn't reach the lab.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_bFollowing || !m_Escort.IsNear(gm.GetLabPosition(), 40))
			return;

		m_Escort.Arrive();
		int by = m_Escort.GetFollowed();
		gm.NotifyPlayer(by, m_sTitle, string.Format("At the lab door a researcher in a mask takes the notebook first, then %1's arm. \"Volunteer,\" she says, writing it down. %1 says thank you, and goes in.", m_Escort.m_sName));
		Resolve(gm, by, RZ_EQuestApproach.SIDE, RZ_Quest_HouseWentQuiet.OUT_ENDED, "brought a volunteer to the lab");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Escort.Cleanup();
		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The end of the house: Halcyon has its volunteer from the {family} farm.");
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 5, the Red Hand's way (a Red Hand contract): hold the farm (within 50 m) for 90 s,
//! the infected coming twice, while a crew loads the family up and moves them on.
class RZ_QM_TableMoveOn : RZ_QM_TableBase
{
	protected vector m_vHouse;
	protected float m_fHeld;
	protected int m_iWaves;
	protected bool m_bTold;

	protected static const float HOLD_SECONDS = 90;
	protected static const float HOLD_RADIUS = 50;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureFor(RZ_EFaction.BANDITS, 11, 40, 2400);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		m_vPos = m_vHouse;
		m_iSettlement = t.GetSettlement();
		m_sTitle = "The end of the house";
		m_sBrief = QFill(gm, "The boss is paying the {family} farm to go away. A crew is coming with a cart. Hold the farm ninety seconds while they load up, and keep the infected off. Don't make it a story.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		array<int> near = {};
		GetPlayersNear(ids, players, m_vHouse, HOLD_RADIUS, near);
		if (near.IsEmpty())
			return;

		foreach (int playerId : near)
		{
			AddParticipant(playerId);
		}

		if (!m_bTold)
		{
			m_bTold = true;
			NotifyParticipants(gm, m_sTitle, "The cart comes up the back lane. The crew don't look at the family, and the family don't look at them. Ninety seconds.");
		}

		m_fHeld += dt;
		if (m_iWaves < 2 && m_fHeld >= 25 + m_iWaves * 30)
		{
			m_iWaves++;
			SpawnGuards(gm, m_vHouse, 3, 60, 110);
		}

		if (m_fHeld < HOLD_SECONDS)
			return;

		NotifyParticipants(gm, m_sTitle, QFill(gm, "The cart goes off down the back lane, piled with bedding and a long table on its side. {host} lifts a hand to you from the back of it, the way you'd wave to a neighbour."));
		Resolve(gm, near[0], RZ_EQuestApproach.SIDE, RZ_Quest_HouseWentQuiet.OUT_ENDED, "held the farm while the crew moved them on");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The end of the house: the {family} family have been moved on.");
	}
}

//------------------------------------------------------------------------------------------------
//! C1 stage 5, by your own hand (nobody told): fight them (three or four, the fight prefab), or
//! talk the last of them out once the mother is gone: tell her you know (persuade, Haven,
//! difficulty 2; ★ Elder or Nurse +1) and she walks out; then tell the rest to go. A failed word
//! and she calls them: they set on you. Get the boy out first if you can.
class RZ_QM_TableByHand : RZ_QM_TableBase
{
	protected vector m_vHouse;
	protected int m_iMotherTalk = -1;
	protected int m_iHostTalk = -1;
	protected int m_iBoyTalk = -1;
	protected bool m_bMotherGone;
	protected bool m_bFighting;

	protected static const int OBJ_MOTHER = 0;
	protected static const int OBJ_HOST = 1;
	protected static const int OBJ_BOY = 2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(3600);
		m_Req.Bonus("elder");
		m_Req.Bonus("nurse");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		m_bMotherGone = !t.IsAlive(RZ_ETableRole.MOTHER);
		m_iMotherTalk = AddClue(QText(gm, "mother"), QFill(gm, "{mother} is laying the table. She sets a place for you without asking. \"You're thin.\""));
		m_iHostTalk = AddClue(QText(gm, "host"), QFill(gm, "{host} is by the door, where the boots are. He isn't smiling."));
		m_iBoyTalk = AddClue(QText(gm, "boy"), QFill(gm, "{boy} is on the step with a bowl he isn't eating."));
		m_Quiet.Watch(m_vHouse, 120);
		m_vPos = m_vHouse;
		m_iSettlement = t.GetSettlement();
		m_sTitle = "The end of the house";
		m_sBrief = QFill(gm, RZ_Quest_HouseWentQuiet.EndText(gm, Run(gm)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (!m_bMotherGone)
			SpawnMember(gm, RZ_ETableRole.MOTHER, LandPointNear(m_vHouse, 2, 4), OBJ_MOTHER, m_iMotherTalk, "Tell her you know");

		RZ_TableMember greeter = Table(gm).Greeter();
		if (greeter)
			SpawnMember(gm, greeter.m_eRole, LandPointNear(m_vHouse, 5, 10), OBJ_HOST, m_iHostTalk, "Tell the rest of them to go");

		SpawnMember(gm, RZ_ETableRole.BOY, LandPointNear(m_vHouse, 10, 18), OBJ_BOY, m_iBoyTalk, "Get him away from the house");

		// Those who found the proof know what they're shooting at
		foreach (int playerId : m_aParticipants)
		{
			Table(gm).Sanction(playerId);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_bResolved || option != 0)
			return;

		RZ_LongTable t = Table(gm);
		RZ_QuestRun run = Run(gm);
		if (run && run.PlayerTookPart(gm, playerId))
			t.Sanction(playerId);

		if (objIdx == OBJ_BOY)
		{
			if (QFlag(gm, "boy_out") == 1)
				return;

			if (InfectedNear(gm, source.GetOrigin(), 25))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "He won't move with them that close.");
				return;
			}

			SetQFlag(gm, "boy_out", 1);
			RemoveMembers(RZ_ETableRole.BOY);
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "got the boy out");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{boy} puts the bowl down and comes. You leave him at the bend in the lane, out of sight of the house, and tell him to wait. He does."));
			return;
		}

		if (objIdx == OBJ_MOTHER)
		{
			if (TryPersuade(gm, playerId, RZ_EFaction.SURVIVORS, 2, "elder,nurse"))
			{
				m_bMotherGone = true;
				RemoveMembers(RZ_ETableRole.MOTHER);
				t.MemberGone(RZ_ETableRole.MOTHER);
				Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "told the mother they knew");
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{mother} listens to the end without stopping you. Then she takes off her apron, folds it, lays it on the table, and walks out of the gate toward the trees. She doesn't look back. The others watch her go."));
				return;
			}

			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "told the mother, and was set on");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"You've eaten at our table,\" says {mother}, and calls for {son}."));
			StartFight(gm, playerId, true);
			return;
		}

		if (objIdx != OBJ_HOST)
			return;

		if (!m_bMotherGone)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{host} looks past you at the kitchen, where {mother} is. Nobody here goes anywhere while she's in it."));
			return;
		}

		Remember(gm, playerId, RZ_EQuestApproach.TALK, "talked the last of them out");
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{host} looks at you for a long time. Then he takes his coat from the peg. By the time you've counted the boots by the door, the house is empty."));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_HouseWentQuiet.OUT_BY_HAND, "talked the last of them out");
	}

	//------------------------------------------------------------------------------------------------
	protected void StartFight(RZ_GameModeComponent gm, int playerId, bool setOn)
	{
		if (m_bFighting)
		{
			Table(gm).JoinFight(playerId);
			return;
		}

		array<int> targets = {playerId};
		if (!Table(gm).StartFight(targets, m_vHouse, setOn))
			return;

		m_bFighting = true;
		RemoveMembers(RZ_ETableRole.HOST);
		RemoveMembers(RZ_ETableRole.SON);
		RemoveMembers(RZ_ETableRole.OTHER);
		Remember(gm, playerId, RZ_EQuestApproach.FORCE, "fought them");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		CheckFamilyBodies(gm);
		if (m_bResolved)
			return;

		RZ_LongTable t = Table(gm);

		// Shots at the house: the fight
		if (!m_bFighting && m_bSceneUp && m_Quiet.WasLoud())
		{
			int shooter = PlayerNear(ids, players, m_vHouse, 150);
			if (shooter > 0)
				StartFight(gm, shooter, false);
		}

		if (!m_bFighting || t.IsFighting())
			return;

		m_bFighting = false;
		if (t.FightResult() == 1 || !t.Exists())
		{
			int by = PlayerNear(ids, players, m_vHouse, 400);
			if (by <= 0 && !m_aParticipants.IsEmpty())
				by = m_aParticipants[0];

			Resolve(gm, by, RZ_EQuestApproach.FORCE, RZ_Quest_HouseWentQuiet.OUT_BY_HAND, "fought them");
			return;
		}

		// Driven off: the house is shut against you for today
		Resolve(gm, PlayerNear(ids, players, m_vHouse, 2000), RZ_EQuestApproach.FORCE, RZ_Questline.OUTCOME_FAILED, "was driven off", false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMemberKilled(RZ_GameModeComponent gm, int role, int killer)
	{
		if (role == RZ_ETableRole.MOTHER)
			m_bMotherGone = true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The end of the house: the {family} farm is finished.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The end of the house: the {family} farm is still standing tonight.");
	}
}
