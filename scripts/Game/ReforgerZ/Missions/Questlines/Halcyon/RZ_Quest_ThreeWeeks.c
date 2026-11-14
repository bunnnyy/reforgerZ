//! The Three Weeks (Halcyon; HC1 in docs/QUESTLINES.md, built in docs/MISSIONS.md, "Questlines").
//! The first case was three weeks before the gates closed, and the Institute was told. A field
//! lead found the clinic's copy and wants to know what the lab did with those three weeks. This
//! line is about the past: the outside never acts.
//!
//! 1. The driver (mission): the Scavenger who drove Dr Wolff up to the closed site on Day 3 sits at
//!    the edge of the town nearest it. Talk (Known with the Haven and not Trusted with Halcyon, or a
//!    Radio Operator, who heard his call that day), share food twice, or threaten him (armed, +3
//!    Haven Infamy). He names the site: "He stayed because he wanted to."
//! 2. The site (mission): a remote spot on high ground between two towns (a mast, a factory or a
//!    barracks there if there is one). Three of his team lie around it, each with two digits of the
//!    store's six. Lab Tech: the first digit. Three wrong: the keypad is dead for 2 minutes, loud.
//! 3. The team lead (mission): Dr Wolff is inside, turned long ago. Mercy (kill him, take the
//!    notebook), live capture (RZ_LiveCapture: the cage, then the lab, research +2) or sneak (take
//!    the notebook unseen and leave him).
//! 4. Who reads it (mission): the notebook says the Institute was told on the first day and wrote
//!    back one word, "Continue." To the director (Trusted, or Known as a Lab Tech or Doctor), the
//!    field lead, Bastion's commanding officer, the Haven's chair or the Red Hand's boss (40 Favor).
//! 5. The lab answers (wait, one session). Public: the Red Hand raids the lab (Lab Defence). In the
//!    record: the field lead walks out (walk him to a safehouse, or back to the lab). Bastion: no more.
//!
//! Endings: In the record, Out on the open channel, Bastion's file; and with any of them, if he was
//! brought in alive, the director speaks about him on the open channel. Starts once Patient Zero or
//! Patient Zero's Ledger has been solved this season (they set FLAG_RECORDS), with an Independent or
//! Rogue field lead, a Scavenger wanderer and a site to be found.
class RZ_Quest_ThreeWeeks : RZ_Questline
{
	static const string KEY = "weeks";
	static const string FLAG_RECORDS = "weeks.records";		// read: the clinic records were found (Patient Zero, its Ledger)
	static const string FLAG_PUBLIC = "weeks.public";		// set: the notebook was read out (H3: the lab road is harder)
	static const string FLAG_BASTION = "weeks.bastion";		// set: Bastion has it (HC3 starts with its terms on the table)
	static const string FLAG_RECORD = "weeks.record";		// set: it went into the lab's record
	static const string FLAG_CAPTURED = "weeks.captured";	// set: Dr Wolff is in the lab's isolation room
	static const string FLAG_LEAD_LEFT = "weeks.lead_left";	// set: the field lead left his station

	static const int S_DRIVER = 0;
	static const int S_SITE = 1;
	static const int S_LEAD = 2;
	static const int S_READERS = 3;
	static const int S_ANSWER = 4;
	static const int S_RAID = 5;
	static const int S_WALKOUT = 6;

	static const int OUT_NAMED = 10;		// the driver named the site
	static const int OUT_OPEN = 11;			// the store is open
	static const int OUT_MERCY = 12;		// he was ended; the notebook taken
	static const int OUT_CAPTURED = 13;		// he's at the lab, alive
	static const int OUT_SNEAK = 14;		// the notebook taken, he was left
	static const int OUT_LOOSE = 15;		// caught, then lost on the way: he's loose
	static const int OUT_DIRECTOR = 16;
	static const int OUT_LEAD = 17;
	static const int OUT_BASTION = 18;
	static const int OUT_HAVEN = 19;
	static const int OUT_REDHAND = 20;
	static const int OUT_TO_HOUSE = 21;
	static const int OUT_TO_LAB = 22;
	static const int OUT_LEAD_DIED = 23;

	static const int BR_RECORD = 0;
	static const int BR_PUBLIC = 1;
	static const int BR_BASTION = 2;

	static const int E_RECORD = 0;
	static const int E_PUBLIC = 1;
	static const int E_BASTION = 2;

	static const string WOLFF = "Dr Gregor Wolff";

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(KEY, "The Three Weeks", RZ_EFaction.INSTITUTE);

		RZ_QuestStage driver = MissionStage(S_DRIVER, "The driver", "{lead} at the field station in {lead_place} found the clinic's copy: the first case, three weeks before the gates closed, and \"notify the Institute\" underlined twice. The only one who knows what happened at the site on Day 3 is {driver}, the driver who came back alone. He won't talk to anyone in a lab coat.");
		driver.GiveUp(4, "{driver} never said what was up at the site on Day 3, and nobody will hear it now.");
		driver.Visits(2);

		RZ_QuestStage site = MissionStage(S_SITE, "The site", "{driver} named the closed site: {site_place}, grid {site_grid}. Dr Wolff went back up on Day 3 with his team, and the store there has a keypad.");
		site.GiveUp(4, "Nobody went back up to the site near {site_town}. Whatever is in the store stays in it.");
		site.Visits(2);

		RZ_QuestStage lead = MissionStage(S_LEAD, "The team lead", "The store at the site near {site_town} is open. Dr Wolff's notebook is inside, and so is he. He turned a long time ago.");
		lead.GiveUp(4, "Nobody went into the store at the site. Dr Wolff is still in there with his notebook.");
		lead.Visits(2);

		RZ_QuestStage readers = MissionStage(S_READERS, "Who reads it", "The notebook doesn't say what the site was for. It says the Institute was told on the first day of the three weeks, and wrote back one word: \"Continue.\" Somebody has to decide who reads it.");
		readers.GiveUp(4, "Dr Wolff's notebook stayed in somebody's pack. Nobody has read it but whoever carries it.");
		readers.Visits(2);

		RZ_QuestStage answer = WaitStage(S_ANSWER, "The lab answers", "Wolff's notebook has gone where it went. The lab will answer, in a day or so.");
		answer.Wait(120, 24);
		answer.GiveUp(48, "Whatever the lab meant to say about the notebook, it never said it.");

		RZ_QuestStage raid = MissionStage(S_RAID, "The raid", "Since the notebook was read out, the Red Hand is sure the lab has more to hide, and a crew is coming to look. Hold the lab.");
		raid.GiveUp(4, "The Red Hand never came for the lab after all. The notebook is still all anyone talks about.");

		RZ_QuestStage walkout = MissionStage(S_WALKOUT, "The field lead walks out", "{lead} heard the notebook went into the record without the pages about the director. He's leaving {lead_place}. Walk him to a safehouse, or bring him back to the lab.");
		walkout.GiveUp(3, "{lead} left {lead_place} alone one night, and the lab hasn't said where to.");
	}

	//------------------------------------------------------------------------------------------------
	//! Patient Zero and Patient Zero's Ledger call this when they're solved: the clinic records are
	//! out, and with them "notify the Institute" in the margin.
	static void NoteClinicRecords(RZ_GameModeComponent gm)
	{
		if (gm && gm.GetQuestlines())
			gm.GetQuestlines().SetWorldFlag(FLAG_RECORDS, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! An inner line of Proof (Halcyon's major) while the majors run: only Proof starts it.
	override bool StartsOnItsOwn()
	{
		return !RZ_Majors.IsOn();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!gm.GetQuestlines() || gm.GetQuestlines().WorldFlag(FLAG_RECORDS) != 1)
			return false;

		if (!gm.HasLab() || !gm.GetLeaders() || !gm.GetWanderers() || gm.GetSettlements().Count() < 4)
			return false;

		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return false;

		RZ_Leader lead = RZ_QM_HalcyonScene.PickFieldLead(gm, false);
		if (!lead)
			return false;

		array<int> drivers = {};
		gm.GetWanderers().GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER), drivers);
		if (drivers.IsEmpty())
			return false;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		vector site;
		int anchor = -1;
		string place;
		if (!PickSite(gm, rng, site, anchor, place))
			return false;

		RZ_Settlement town = gm.GetSettlement(anchor);
		RZ_QM_HalcyonScene.StoreLeader(run, "lead", lead);
		run.SetFlag("driver", drivers[0]);
		run.SetText("driver", gm.GetWanderers().GetName(drivers[0]));
		RZ_QM_HalcyonScene.StorePos(run, "site", site);
		run.SetFlag("site_town", anchor);
		run.SetText("site_town", town.m_sName);
		run.SetText("site_grid", RZ_SettlementScanner.GridReference(site));
		string dir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(town.m_vCenter, site));
		if (place.IsEmpty())
			run.SetText("site_place", string.Format("the closed site on the high ground %1 of %2", dir, town.m_sName));
		else
			run.SetText("site_place", string.Format("%1 on the high ground %2 of %3", place, dir, town.m_sName));

		run.SetText("director", RZ_QM_HalcyonScene.TopName(gm, RZ_EFaction.INSTITUTE, "the director"));
		run.SetPlace(lead.m_vPos, lead.m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The season's closed site (the canon fact `origin.site`, the same one Patient Zero's records and
	//! the finale point to; it has a fallback, so a small map has one too), else one found here.
	static bool PickSite(RZ_GameModeComponent gm, RZ_Rng rng, out vector site, out int anchor, out string place)
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (facts)
		{
			vector canon = facts.CanonPos("origin.site");
			if (canon != vector.Zero)
			{
				int town = facts.CanonSettlement("origin.site.town");
				if (town < 0)
					town = gm.FindNearestSettlement(canon);

				if (town >= 0)
				{
					array<string> marks = {"military", "industrial", "radio", "power", "tower"};
					RZ_Landmark l = RZ_LandmarkFinder.FindNearest(canon, 30, marks);
					site = canon;
					anchor = town;
					place = "";
					if (l)
						place = l.m_sName;

					return true;
				}
			}
		}

		return FindSite(gm, rng, site, anchor, place);
	}

	//------------------------------------------------------------------------------------------------
	//! The closed site: a remote spot on high ground between two towns, well clear of every
	//! settlement and the lab, at a real mast, factory or barracks there if there is one. A small
	//! map gets looser tries at the end. anchor = the nearest town; place = the landmark's name, or "".
	static bool FindSite(RZ_GameModeComponent gm, RZ_Rng rng, out vector site, out int anchor, out string place)
	{
		array<ref RZ_Settlement> all = gm.GetSettlements();
		int n = all.Count();
		array<string> kinds = {"military", "industrial", "radio", "power", "tower"};
		for (int t = 0; t < 40; t++)
		{
			float need = 450;
			float minGap = 1400;
			if (t >= 20)
				need = 300;

			if (t >= 30)
			{
				need = 180;
				minGap = 800;
			}

			int ia = rng.Int(0, n);
			int ib = rng.Int(0, n);
			if (ia == ib)
				continue;

			vector ca = all[ia].m_vCenter;
			vector cb = all[ib].m_vCenter;
			float gap = RZ_Util.DistXZ(ca, cb);
			if (gap < minGap || gap > 6000)
				continue;

			vector line = cb - ca;
			vector side = Vector(-line[2], 0, line[0]) * (1 / gap);
			vector mid = ca + line * rng.Range(0.4, 0.6) + side * rng.Range(-gap * 0.2, gap * 0.2);
			vector p = RZ_Mission.HighPointNear(RZ_Util.GroundPos(mid), 0, 200, 12);
			if (!IsRemote(gm, p, need))
				continue;

			string found = "";
			RZ_Landmark l = RZ_LandmarkFinder.FindNearest(p, 400, kinds);
			if (l && IsRemote(gm, l.m_vPos, need - 100))
			{
				p = RZ_Mission.LandPointNear(l.m_vPos, 4, 10);
				found = l.m_sName;
			}

			int nearest = gm.FindNearestSettlement(p);
			if (nearest < 0 || nearest == gm.GetLabSettlement())
				continue;

			site = p;
			anchor = nearest;
			place = found;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Dry, inside the line, at least `gap` past every settlement's edge and 1.5 km from the lab.
	protected static bool IsRemote(RZ_GameModeComponent gm, vector p, float gap)
	{
		if (RZ_Util.IsWater(p) || RZ_Perimeter.IsNearLineAt(p) || RZ_LockedSpots.IsBlocked(p))
			return false;

		if (gm.HasLab() && RZ_Util.DistXZ(p, gm.GetLabPosition()) < 1500)
			return false;

		foreach (RZ_Settlement s : gm.GetSettlements())
		{
			if (RZ_Util.DistXZ(p, s.m_vCenter) - s.m_fRadius < gap)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The driver is alive and no other mission is using him.
	static bool DriverFree(RZ_GameModeComponent gm, int driver)
	{
		if (!gm.GetWanderers() || driver < 0)
			return false;

		array<int> living = {};
		gm.GetWanderers().GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER), living);
		return living.Contains(driver);
	}

	//------------------------------------------------------------------------------------------------
	//! The field lead voices the stage's brief and pays it, while he's still the one in his chair.
	static void Voice(RZ_GameModeComponent gm, RZ_Mission m)
	{
		if (!gm.GetQuestlines())
			return;

		RZ_Leader l = RZ_QM_HalcyonScene.SameLeader(gm, gm.GetQuestlines().FindRun(m.m_iQuestRun), "lead");
		if (l)
			m.m_iLeaderId = l.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, run.Fill("{lead_short} at the field station in {lead_place} has been asking the lab's drivers where they were on Day 3."), false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_DRIVER)
			run.Note("{lead} found the clinic's copy of the first admission: three weeks before the gates closed. Someone wrote \"notify the Institute\" in the margin and underlined it twice.");
	}

	//------------------------------------------------------------------------------------------------
	//! The driver has to be alive and free. The lab answers after a final stand, not during one (so
	//! the story knows whether the cure is out); the raid never comes during a stand or after the cure.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_DRIVER)
			return DriverFree(gm, run.Flag("driver", -1));

		if (stage == S_ANSWER)
			return !gm.GetCure().IsFinalStand();

		if (stage == S_RAID)
			return !gm.GetCure().IsFinalStand() && !gm.GetCure().IsCured();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_DRIVER: return new RZ_QM_WeeksDriver();
			case S_SITE: return new RZ_QM_WeeksSite();
			case S_LEAD: return new RZ_QM_WeeksLead();
			case S_READERS: return new RZ_QM_WeeksReaders();
			case S_RAID: return new RZ_QM_WeeksRaid();
			case S_WALKOUT: return new RZ_QM_WeeksWalkout();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_DRIVER:
			{
				if (outcome == OUTCOME_FAILED)
				{
					run.Note("Nobody got {driver} to talk. {lead_short} will ask someone else to try.");
					return S_DRIVER;
				}

				run.Note("{driver} named the site: {site_place}, grid {site_grid}. Then, to nobody: \"He stayed because he wanted to.\"");
				run.SetPlace(RZ_QM_HalcyonScene.LoadPos(run, "site"), run.Text("site_place"));
				return S_SITE;
			}
			case S_SITE:
			{
				if (outcome == OUTCOME_FAILED)
				{
					run.Note("The store at the site is still shut.");
					return S_SITE;
				}

				run.Note("Three of Dr Wolff's team are still lying around the site. Their pages gave the store's code, and the store is open.");
				return S_LEAD;
			}
			case S_LEAD:
				return AfterLead(run, outcome);
			case S_READERS:
				return AfterReaders(gm, run, outcome);
			case S_ANSWER:
				return AfterAnswer(gm, run);
			case S_RAID:
			{
				if (outcome == OUTCOME_FAILED)
				{
					run.SetFlag("raid", 2);
					run.Note("The lab didn't hold. The crew went through the store room looking for more pages.");
				}
				else
				{
					run.SetFlag("raid", 1);
					run.Note("The lab held. The crew went home with nothing but the story.");
				}

				return Finish(run, E_PUBLIC, "Out on the open channel. The whole Zone has heard what the Institute wrote back on the first day of the three weeks: \"Continue.\"");
			}
			case S_WALKOUT:
			{
				if (outcome == OUT_TO_HOUSE)
					run.SetFlag("lead_to", 1);
				else if (outcome == OUT_TO_LAB)
					run.SetFlag("lead_to", 2);
				else if (outcome == OUT_LEAD_DIED)
					run.SetFlag("lead_to", 3);

				return Finish(run, E_RECORD, "In the record. Dr Wolff's notebook is in the lab's record now, most of it. The pages about the director went in the stove.");
			}
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterLead(RZ_QuestRun run, int outcome)
	{
		if (outcome == OUTCOME_FAILED)
		{
			run.Note("Nobody came out of the store with the notebook.");
			return S_LEAD;
		}

		if (outcome == OUT_CAPTURED)
		{
			run.SetFlag("captured", 1);
			run.Note("Dr Wolff is in the lab's isolation room, alive in the way the infected are. His notebook came down with him.");
		}
		else if (outcome == OUT_SNEAK)
		{
			run.Note("Somebody took the notebook and left him in the store. He didn't look up.");
		}
		else if (outcome == OUT_LOOSE)
		{
			run.Note("He woke on the way down and broke out of the case. He's out there now. The notebook is safe.");
		}
		else
		{
			run.Note("Dr Wolff is at rest. His notebook was in his coat.");
		}

		return S_READERS;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterReaders(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		switch (outcome)
		{
			case OUT_BASTION:
			{
				run.SetBranch(BR_BASTION);
				run.Note("The notebook is in Bastion's file now. The commanding officer was right about Halcyon, and knows it.");
				return S_ANSWER;
			}
			case OUT_LEAD:
			{
				run.SetBranch(BR_PUBLIC);
				run.SetFlag("readers", RZ_EFaction.INSTITUTE);
				run.Note("{lead} has the notebook. He read it on the open channel, every page.");
				return S_ANSWER;
			}
			case OUT_HAVEN:
			{
				run.SetBranch(BR_PUBLIC);
				run.SetFlag("readers", RZ_EFaction.SURVIVORS);
				run.Note("The chair of the Haven council read the notebook at supper, and every house heard it by morning.");
				return S_ANSWER;
			}
			case OUT_REDHAND:
			{
				run.SetBranch(BR_PUBLIC);
				run.SetFlag("readers", RZ_EFaction.BANDITS);
				run.Note("The Red Hand bought the notebook and read it on the open channel, laughing between the pages.");
				return S_ANSWER;
			}
			case OUTCOME_FAILED:
			{
				run.Note("The notebook still hasn't gone to anyone.");
				return S_READERS;
			}
		}

		run.SetBranch(BR_RECORD);
		run.Note("The director has the notebook. It goes in the record. The pages about the director go in the stove.");
		return S_ANSWER;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterAnswer(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iBranch == BR_BASTION)
			return Finish(run, E_BASTION, "Bastion's file. The commanding officer has Dr Wolff's notebook, and a reason to ask the lab for more.");

		if (run.m_iBranch == BR_PUBLIC)
		{
			// After the cure nobody raids the lab for its secrets: the story ends with the reading
			if (gm.GetCure().IsCured())
				return Finish(run, E_PUBLIC, "Out on the open channel. The whole Zone has heard what the Institute wrote back on the first day of the three weeks: \"Continue.\"");

			return S_RAID;
		}

		// In the record: the field lead walks out, if he's still the one in his chair
		if (RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
			return S_WALKOUT;

		return Finish(run, E_RECORD, "In the record. Dr Wolff's notebook is in the lab's record now, most of it. The pages about the director went in the stove.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		RZ_Questlines q = gm.GetQuestlines();
		if (run.Flag("captured") == 1)
		{
			q.SetWorldFlag(FLAG_CAPTURED, 1);
			RZ_Leader director = RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.INSTITUTE);
			if (director)
				Headline(gm, string.Format("On the open channel last night, %1 spoke for two minutes about %2, who is in the lab's isolation room. Then the numbers, as usual.", director.m_sName, WOLFF));
		}

		if (ending == E_RECORD)
		{
			q.SetWorldFlag(FLAG_RECORD, 1);
			NudgeMorale(gm, RZ_EFaction.INSTITUTE, 3);
			ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 4, 0);
			gm.GetCure().AddResearch(1, "Dr Wolff's notebook went into the lab's record.");
			LeadLeaves(gm, run);
			SpreadRumour(gm, "Halcyon burned notebook pages in the stove the other night. Halcyon never burns paper.", 8);
			return;
		}

		if (ending == E_PUBLIC)
		{
			q.SetWorldFlag(FLAG_PUBLIC, 1);
			NudgeMorale(gm, RZ_EFaction.INSTITUTE, -10);
			Headline(gm, run.Fill(PublicHeadline(run)));
			if (run.Flag("raid") == 2)
				Headline(gm, "The Red Hand went through the lab's store room looking for more pages, and found nothing they could read.", false);

			SpreadRumour(gm, "Halcyon made this. Ask them about the three weeks and watch their faces.", 48);
			return;
		}

		q.SetWorldFlag(FLAG_BASTION, 1);
		NudgeStrength(gm, RZ_EFaction.MILITARY, 3);
		Headline(gm, string.Format("%1 has a dead researcher's notebook in Bastion's file now, and questions for the lab.", RZ_QM_HalcyonScene.TopName(gm, RZ_EFaction.MILITARY, "Bastion's commanding officer")));
		SpreadRumour(gm, "Bastion's got something on the lab. A notebook. The commanding officer reads it at night, they say.", 12);
	}

	//------------------------------------------------------------------------------------------------
	protected string PublicHeadline(RZ_QuestRun run)
	{
		int readers = run.Flag("readers", RZ_EFaction.SURVIVORS);
		if (readers == RZ_EFaction.BANDITS)
			return "The Red Hand read a dead researcher's notebook on the open channel last night, page by page, laughing. The Institute knew three weeks before the gates closed, and wrote back \"Continue.\"";

		if (readers == RZ_EFaction.INSTITUTE)
			return "{lead} read Dr Wolff's notebook on the open channel last night, every page. The Institute knew three weeks before the gates closed, and wrote back \"Continue.\"";

		return "Every Haven house heard Dr Wolff's notebook read at supper last night. The Institute knew three weeks before the gates closed, and wrote back one word: \"Continue.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! In the record: he leaves his station (a life event), if he's still in it.
	protected void LeadLeaves(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Leader l = RZ_QM_HalcyonScene.SameLeader(gm, run, "lead");
		if (!l)
			return;

		int to = run.Flag("lead_to");
		string headline = run.Fill("{lead} has left the field station in {lead_place}. Nobody at the lab says where he went.");
		if (to == 2)
			headline = run.Fill("{lead} has left the field station in {lead_place} for a desk at the lab, where the director can watch the work.");

		int house = run.Flag("lead_house", -1);
		RZ_Safehouse h = GetHouse(gm, house);
		if (to == 1 && h)
			headline = run.Fill("{lead} has left Halcyon. He sleeps at the safehouse in " + h.m_sName + " now, and he has stopped keeping a notebook.");

		if (!ReplaceLeader(gm, l.m_iId, headline))
			return;

		gm.GetQuestlines().SetWorldFlag(FLAG_LEAD_LEFT, 1);
		if (to == 1 && h)
			ChangePopulation(gm, house, 1);
	}

	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage == S_DRIVER && gm.GetWanderers() && !gm.GetWanderers().IsAlive(run.Flag("driver", -1)))
			return run.Fill("{driver}, the driver who came back alone from the site on Day 3, is dead. Whatever he saw went with him.");

		if (run.m_iStage == S_ANSWER)
		{
			if (run.m_iBranch == BR_BASTION)
				return run.Fill("The notebook is in Bastion's file. The commanding officer will want something for it. Wait for the lab's answer.");

			if (run.m_iBranch == BR_PUBLIC)
				return run.Fill("Everyone has heard the notebook now. The lab will answer, one way or another.");

			if (RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
				return run.Fill("The notebook is in the lab's record, most of it. {lead} will hear which pages went in the stove.");

			return run.Fill("The notebook is in the lab's record, most of it.");
		}

		if (run.m_iStage == S_WALKOUT && !RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
			return run.Fill("{lead} is no longer at the field station in {lead_place}.");

		return super.JournalText(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage == S_SITE || run.m_iStage == S_LEAD)
			return "Somebody's been up at the old site past {site_town}. Halcyon's people go quiet when you ask what they kept up there.";

		if (run.m_iStage == S_READERS)
			return "There's a dead man's notebook going round, and everybody wants to read it first.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The field lead and the director each have a line while it runs.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		if (run.m_iStage > S_READERS)
			return "";

		RZ_Leader lead = RZ_QM_HalcyonScene.SameLeader(gm, run, "lead");
		if (lead && lead.m_iId == leaderId)
			return string.Format("%1 keeps his voice down. \"I don't want to know what we wrote down about those three weeks. I want to know what we did.\"", lead.m_sShort);

		RZ_Leader director = RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.INSTITUTE);
		if (director && director.m_iId == leaderId && run.m_iStage >= S_SITE)
			return string.Format("%1 doesn't look up from the readings. \"If you've been up the hill past {site_town}, I'd rather hear it from you than from the open channel.\"", director.m_sShort);

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! The Three Weeks, stage 1 (Halcyon contract, Known): the driver, held by the mission at the edge
//! of the town nearest the site. Talk (the Haven's people and Radio Operators only), share food
//! twice, or threaten him.
class RZ_QM_WeeksDriver : RZ_QM_HalcyonScene
{
	//------------------------------------------------------------------------------------------------
	//! Shown before you pick: he talks to someone the Haven knows and the lab doesn't trust, or to a
	//! Radio Operator who heard his call.
	override string ChoiceLock(RZ_GameModeComponent gm, int playerId, int objIdx, bool choice, int slot)
	{
		string need = super.ChoiceLock(gm, playerId, objIdx, choice, slot);
		if (!need.IsEmpty() || !choice || objIdx != OBJ_DRIVER || slot != 0 || RZ_QuestApproach.HasBackground(gm, playerId, "radioham"))
			return need;

		if (Rank(gm, playerId, RZ_EFaction.INSTITUTE) >= 2)
			return "Not while the lab trusts you";

		return RankNeed(gm, playerId, RZ_EFaction.SURVIVORS, 1);
	}

	protected int m_iDriver = -1;
	protected bool m_bHeld;
	protected int m_iTalk = -1;
	protected int m_iShared;

	protected static const int OBJ_DRIVER = 0;
	protected static const int THREAT_INFAMY = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 10, 10);
		m_Req.Bonus("radioham");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		RZ_Quest_ThreeWeeks.Voice(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Wanderers wanderers = gm.GetWanderers();
		RZ_Settlement town;
		if (run)
			town = gm.GetSettlement(run.Flag("site_town", -1));

		if (!run || !wanderers || !town)
			return false;

		m_iDriver = run.Flag("driver", -1);
		if (!RZ_Quest_ThreeWeeks.DriverFree(gm, m_iDriver))
			return false;

		// Not while somebody is standing next to him (his body would vanish in front of them)
		vector at;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (wanderers.GetPosition(m_iDriver, at) && RZ_Util.NearestDistance(at, players) < 250)
			return false;

		// He sits where he left the van on Day 3: the edge of town, on the side toward the site
		vector site = LoadPos(run, "site");
		float toSite = RZ_Compass.Bearing(town.m_vCenter, site);
		m_vPos = LandPointNear(RZ_Compass.PointAt(town.m_vCenter, toSite, town.m_fRadius + 30), 0, 15);
		m_iSettlement = run.Flag("site_town", -1);
		wanderers.Hold(m_iDriver, m_iId);
		m_bHeld = true;

		m_iTalk = AddClue(QText(gm, "driver"), "");
		string choices = "Ask about Day 3|Share your food with him|Threaten him";
		AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_DRIVER, m_iTalk, "Talk to " + QText(gm, "driver"), choices, QFill(gm, "{driver}, at the edge of {site_town}"));
		m_sTitle = "The Three Weeks: the driver";
		m_sBrief = QFill(gm, "On Day 3 {driver} drove Dr Wolff up to the closed site and came back alone. He sits where he left the van, at the edge of {site_town}, and he won't talk to anyone in a lab coat. Get him to say where the site is, and what he saw.");
		m_sObjective = QFill(gm, "Get {driver} at the edge of {site_town} to say where the closed site is.");
		// The quiet threat (Reputation > Quiet success): Fear first, so he doesn't tell the houses
		m_Ways.m_iMenaceDifficulty = RZ_ReputationSettings.QuietDifficulty();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! He reads you before you say anything.
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		if (idx != m_iTalk)
			return super.GetClueText(gm, playerId, idx);

		if (Rank(gm, playerId, RZ_EFaction.INSTITUTE) >= 2)
			return QFill(gm, "{driver} sees the lab on you from twenty metres. \"No. Not you lot. Not again.\" He's holding a tin with nothing in it.");

		if (Rank(gm, playerId, RZ_EFaction.SURVIVORS) >= 1 || RZ_QuestApproach.HasBackground(gm, playerId, "radioham"))
			return QFill(gm, "{driver} moves over on the step to make room. \"You're not lab. Lab hands are clean.\" He's holding a tin with nothing in it.");

		return QFill(gm, "{driver} looks at your hands before your face. \"Don't know you.\" He's holding a tin with nothing in it, and he's thin under the coat.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_DRIVER || !MayAct(gm, playerId))
			return;

		if (option == 0)
		{
			Ask(gm, playerId);
			return;
		}

		if (option == 1)
		{
			Share(gm, playerId);
			return;
		}

		if (option == 2)
			Threaten(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Known with the Haven and not Trusted with the lab, or a Radio Operator, who heard his call.
	protected void Ask(RZ_GameModeComponent gm, int playerId)
	{
		bool radio = RZ_QuestApproach.HasBackground(gm, playerId, "radioham");
		if (!radio && Rank(gm, playerId, RZ_EFaction.INSTITUTE) >= 2)
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked him and got nothing");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"You've got their badge in your voice.\" {driver} turns away from you, and that's the end of it."));
			return;
		}

		if (!radio && Rank(gm, playerId, RZ_EFaction.SURVIVORS) < 1)
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked him and got nothing");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"Why would I tell you?\" {driver} doesn't know you. Someone the Haven knows might do better."));
			return;
		}

		string opener = QFill(gm, "He talks to the road, not to you.");
		if (radio)
			opener = QFill(gm, "\"I know your voice. You were on the net on Day 3.\" {driver} nods slowly. \"Then you heard me call it in.\"");

		Named(gm, playerId, opener, RZ_EQuestApproach.TALK, "got him talking");
	}

	//------------------------------------------------------------------------------------------------
	protected void Share(RZ_GameModeComponent gm, int playerId)
	{
		array<string> words = {};
		RZ_FoodHelper.DefaultKeywords(words);
		if (RZ_FoodHelper.TakeFood(PlayerEntity(playerId), words, 1) <= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've nothing to share.");
			return;
		}

		m_iShared++;
		if (gm.GetWanderers())
			gm.GetWanderers().RememberPlayer(m_iDriver, playerId, RZ_Wanderers.MEM_HELPED);

		if (m_iShared < 2)
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "shared food with him");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{driver} eats it slowly, the way people do when they've learned to. \"One tin doesn't buy Day 3.\" But he hasn't told you to go."));
			return;
		}

		Named(gm, playerId, QFill(gm, "He eats the second one more slowly than the first. Then he wipes his mouth and starts talking."), RZ_EQuestApproach.TALK, "shared food with him until he talked");
	}

	//------------------------------------------------------------------------------------------------
	protected void Threaten(RZ_GameModeComponent gm, int playerId)
	{
		IEntity user = PlayerEntity(playerId);
		if (!user || !RZ_Util.IsArmed(user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{driver} looks at your empty hands. \"With what?\""));
			return;
		}

		// Quiet success: said low, with nobody about; he won't tell the houses who leaned on him
		if (TryMenace(gm, playerId, "weeks.driver", RZ_EFaction.SURVIVORS, false))
		{
			Named(gm, playerId, QFill(gm, "You say it low, with nobody about. {driver} nods once. He won't be telling the houses about this."), RZ_EQuestApproach.FORCE, "threatened him quietly");
			return;
		}

		RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.SURVIVORS, 0, THREAT_INFAMY);
		if (gm.GetWanderers())
			gm.GetWanderers().RememberPlayer(m_iDriver, playerId, RZ_Wanderers.MEM_WRONGED);

		gm.NotifyPlayer(playerId, "The Haven", string.Format("A Haven wanderer will tell the houses who put a gun on him. (+%1 Haven Infamy)", THREAT_INFAMY));
		Named(gm, playerId, QFill(gm, "{driver} doesn't argue with the gun. He's argued with worse."), RZ_EQuestApproach.FORCE, "threatened him");
	}

	//------------------------------------------------------------------------------------------------
	protected void Named(RZ_GameModeComponent gm, int playerId, string opener, int approach, string what)
	{
		gm.NotifyPlayer(playerId, m_sTitle, opener + " " + QFill(gm, "\"The site's {site_place}. Grid {site_grid}. I drove him up on Day 3 and he told me to wait till dark. His team went up with him. I waited.\" A long breath. \"He stayed because he wanted to. That's what nobody at the lab will say.\""));
		Resolve(gm, playerId, approach, RZ_Quest_ThreeWeeks.OUT_NAMED, what);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseDriver(RZ_GameModeComponent gm)
	{
		if (m_bHeld && gm.GetWanderers())
			gm.GetWanderers().Release(m_iDriver, m_vPos);

		m_bHeld = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		ReleaseDriver(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		ReleaseDriver(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		ReleaseDriver(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Three Weeks: {driver} named the site.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Three Weeks: {driver} said nothing about Day 3.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Three Weeks, stage 2 (Halcyon contract, Known, Lab Tech): the site. Three of his team on the
//! ground around it, each with two of the store's six digits; the keypad (or terminal) at the store.
class RZ_QM_WeeksSite : RZ_QM_HalcyonScene
{
	protected string m_sCode;
	protected int m_iScreen = -1;
	protected int m_iTries;
	protected vector m_vDoor;
	protected bool m_bKeypad;
	protected ref array<int> m_aPages = {};					// the team's clue indices
	protected ref map<int, int> m_mPagesRead = new map<int, int>();	// player -> bitmask of pages read

	protected static const int OBJ_DOOR = 0;
	protected static const int BODIES = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 15, 15);
		m_Req.Bonus("labtech");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_fSpotRadius = 250;
		RZ_Quest_ThreeWeeks.Voice(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		vector site = LoadPos(run, "site");
		m_iSettlement = run.Flag("site_town", -1);
		m_vPos = site;
		m_vDoor = LandPointNear(site, 0, 4);
		int code = m_Rng.Int(100000, 1000000);
		m_sCode = code.ToString();

		// The keypad (or terminal); without either, a door that opens for someone with all six figures
		m_bKeypad = HasPrefab(gm, RZ_EPuzzleProp.TERMINAL);
		if (m_bKeypad)
		{
			m_iScreen = AddClue("The store door", "INSTITUTE FIELD SITE. STORE 2. Six figures, then Enter.\nThe paint around the keypad is worn through where somebody pressed the same keys every day.");
			AddSpot(m_vDoor, RZ_EPuzzleProp.TERMINAL, OBJ_DOOR, m_iScreen, "Read the door", "", "the store at the site");
		}
		else
		{
			m_iScreen = AddClue("The store door", "INSTITUTE FIELD SITE. STORE 2. A combination lock, six wheels, and the wheels are stiff with cold.");
			AddSpot(m_vDoor, RZ_EPuzzleProp.OBJECT, OBJ_DOOR, m_iScreen, "Read the door", "", "the store at the site", "Turn the wheels to the code");
		}

		// His team: three bodies around the site, each with two of the digits
		array<vector> used = {};
		for (int i = 0; i < BODIES; i++)
		{
			vector pos = LandPointNear(site, 30, 110);
			for (int tries = 0; tries < 8 && NearAny(pos, used, 30); tries++)
			{
				pos = LandPointNear(site, 30, 110);
			}

			used.Insert(pos);
			string who = RZ_NoteText.FullName(m_Rng);
			string digits = string.Format("%1 and %2", m_sCode.Substring(i * 2, 1), m_sCode.Substring(i * 2 + 1, 1));
			int clue = AddClue(who, string.Format("An Institute field card, %1, and a page folded into it. \"Store 2. My part of the six, numbers %2 and %3: %4. Gregor has all of them. He would.\" Under it, later, in a worse hand: \"Day 3. He's gone in to finish it. We're not to follow.\"",
				who, i * 2 + 1, i * 2 + 2, digits));
			AddSpot(pos, RZ_EPuzzleProp.BODY, -1, clue, "Search the body", "", "");
			m_aPages.Insert(clue);
		}

		// The site's pack is held still, and its bodies keep 150 m from the store (docs/QUESTLINES.md)
		RZ_Tribe pack = RZ_TribePeople.HoldSite(gm, site, m_iId);
		if (pack)
			ClaimSubject(gm, RZ_TribeContract.Claim(pack.m_iId));

		m_sItemName = "the code";
		m_sTitle = "The Three Weeks: the site";
		m_sBrief = QFill(gm, "The closed site is {site_place}, grid {site_grid}. Dr Wolff went into the store there on Day 3 and his team stayed outside. The store has a six-figure lock. His team will have had the code between them.");
		m_sObjective = QFill(gm, "Open the store at the closed site, grid {site_grid}, with the team's code.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool NearAny(vector pos, array<vector> others, float dist)
	{
		foreach (vector o : others)
		{
			if (RZ_Util.DistXZ(o, pos) < dist)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The site has its infected: three by the store, one with each of the team.
	override protected void OnSpotUp(RZ_GameModeComponent gm, RZ_HalcyonSpot s)
	{
		if (s.m_iObj == OBJ_DOOR)
		{
			SpawnGuards(gm, s.m_vPos, 3, 10, 40);
			return;
		}

		SpawnGuards(gm, s.m_vPos, 1, 6, 20);
	}

	//------------------------------------------------------------------------------------------------
	//! Lab Tech: the Institute numbers a store's code after its site.
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iScreen && HasBonus(gm, playerId))
			text += string.Format("\n(You know the Institute's habits: a store's code starts with the number of its site. This is Site %1.)", m_sCode.Substring(0, 1));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Which of the team's pages each player has read (for the lock without a keypad).
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		int page = m_aPages.Find(idx);
		if (page < 0)
			return;

		int mask;
		m_mPagesRead.Find(playerId, mask);
		m_mPagesRead.Set(playerId, mask | (1 << page));
	}

	//------------------------------------------------------------------------------------------------
	//! The lock without a keypad: whoever has read all three pages has the six figures.
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_DOOR || m_bKeypad)
			return;

		int mask;
		m_mPagesRead.Find(playerId, mask);
		if (mask != (1 << BODIES) - 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You don't have all six figures. His team had them between them, and they're still lying around the site.");
			return;
		}

		Open(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Open(RZ_GameModeComponent gm, int playerId)
	{
		gm.NotifyPlayer(playerId, m_sTitle, "The lock gives with a sound like a held breath. Inside it's dark, and cold, and something moves at the back.");
		Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_ThreeWeeks.OUT_OPEN, "opened the store");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCodeEntered(RZ_GameModeComponent gm, int playerId, int objIdx, string code, IEntity source)
	{
		if (objIdx != OBJ_DOOR)
			return;

		if (code == m_sCode)
		{
			Open(gm, playerId);
			return;
		}

		m_iTries++;
		if (m_iTries < 3)
		{
			SoundAlarm(gm, m_vDoor, 10, false);
			gm.NotifyPlayer(playerId, m_sTitle, "A flat buzz. Wrong.");
			return;
		}

		// Three wrong: loud, and the keypad goes dead for two minutes. It never wipes: the code is paper.
		m_iTries = 0;
		SoundAlarm(gm, m_vDoor, 25, true);
		LockOut(120);
		gm.NotifyPlayer(playerId, m_sTitle, "A long, loud buzz, and the keypad goes dark for two minutes. Somewhere past the fence, something answers it.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Three Weeks: the store at the site is still shut.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Three Weeks, stage 3 (Halcyon contract, Known): Dr Wolff in the store, turned (a special).
//! Kill him and take the notebook (mercy), catch him (the cage, RZ_LiveCapture) and bring him to the
//! lab (research +2), or take the notebook unseen and leave him (sneak). Without room for a capture
//! (or the mission item or the lab drop), there's no cage, and he's spawned by the mission.
class RZ_QM_WeeksLead : RZ_QM_HalcyonScene
{
	protected ref RZ_LiveCapture m_Capture = new RZ_LiveCapture();
	protected bool m_bCage;
	protected bool m_bSpawned;
	protected bool m_bCrated;
	protected IEntity m_Drop;
	protected vector m_vLair;
	protected int m_iNoteSpot = -1;
	protected int m_iPage = -1;

	protected static const int OBJ_CAGE = 0;
	protected static const int OBJ_NOTEBOOK = 1;
	protected static const float RESEARCH = 2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 15, 15);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_fSpotRadius = 250;
		RZ_Quest_ThreeWeeks.Voice(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		vector site = LoadPos(run, "site");
		m_iSettlement = run.Flag("site_town", -1);
		m_vPos = site;
		m_vLair = LandPointNear(site, 1, 4);
		m_iPage = AddClue("Dr Wolff's notebook", QFill(gm, "Numbered, like every Halcyon notebook. The first day of the three weeks: \"Clinic in town reports a fever. One of ours, a site worker. Institute informed, 14:10.\" The next morning, one word on a line of its own, copied out and underlined: \"Continue.\" The pages after that are about the work, and about {director}, and some of them are about why he came back up the hill."));
		m_iNoteSpot = AddSpot(LandPointNear(site, 2, 5), RZ_EPuzzleProp.OBJECT, OBJ_NOTEBOOK, m_iPage, "Read the open page", "", "the store at the site", "Take his notebook");
		m_Quiet.Watch(site, 60);

		// The cage, if a capture can run and there's somewhere at the lab to hand him in
		if (RZ_LiveCapture.HasRoom(gm) && !gm.GetDropPointPrefab(RZ_EFaction.INSTITUTE).IsEmpty())
		{
			m_Drop = RZ_HalcyonContractHelp.SpawnLabDrop(gm);
			if (m_Drop)
			{
				TrackProp(m_Drop);
				m_bCage = m_Capture.Begin(gm, m_iId, m_iSettlement, LandPointNear(site, 15, 30), m_vLair, OBJ_CAGE);
			}
		}

		RZ_Tribe pack = RZ_TribePeople.HoldSite(gm, site, m_iId);
		if (pack)
			ClaimSubject(gm, RZ_TribeContract.Claim(pack.m_iId));

		m_sItemName = "the notebook";
		m_sTitle = "The Three Weeks: the team lead";
		m_sBrief = QFill(gm, "Dr Wolff's notebook is in the store at the site near {site_town}, and so is he. He turned a long time ago. Bring the notebook down. What you do about him is up to you.");
		m_sObjective = QFill(gm, "Bring Dr Wolff's notebook down from the store at the site near {site_town}.");
		if (m_bCage)
			m_sBrief += " The lab has left a cage trap outside, if you'd rather bring him too.";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bCage)
		{
			if (!m_bSpawned && RZ_Util.NearestDistance(m_vLair, players) <= 250)
				m_bSpawned = gm.GetSpawner().SpawnZombie(m_vLair, true, m_iSettlement, m_iId) != null;

			return;
		}

		m_Capture.Update(gm, dt, ids, players);
		if (m_Capture.m_eState == RZ_ECaptureState.CAUGHT)
			RZ_ContractHelp.AddNear(ids, players, m_Capture.m_vHeld, 30, m_aParticipants);

		if (m_Capture.m_eState == RZ_ECaptureState.CRATED && !m_bCrated)
		{
			m_bCrated = true;
			SetSpotOn(m_iNoteSpot, false);
			NotifyParticipants(gm, m_sTitle, string.Format("His notebook goes into the case with him. The lab's drop is at grid %1. The sedative won't last long.", RZ_SettlementScanner.GridReference(m_Drop.GetOrigin())));
		}

		// Caught, then lost on the way: he's loose, and the notebook is still yours
		if (m_Capture.IsLost() && m_bCrated)
		{
			int near = RZ_EscortHelp.PlayerBeside(ids, players, m_Capture.m_vHeld, 100000);
			Resolve(gm, near, RZ_EQuestApproach.FORCE, RZ_Quest_ThreeWeeks.OUT_LOOSE, "caught him and lost him again");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx == OBJ_CAGE && option == 0 && m_bCage)
			m_Capture.TryCloseCage(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Take the notebook and leave him: only unseen, and only while he's still in there.
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_NOTEBOOK || !MayAct(gm, playerId))
			return;

		if (m_bCage && m_Capture.m_eState != RZ_ECaptureState.BAIT)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "It's going with him.");
			return;
		}

		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			string why = m_Quiet.Why(gm, playerId);
			m_Quiet.Spot(playerId);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was heard at the notebook");
			gm.NotifyPlayer(playerId, m_sTitle, why + " His head comes up. He knows you're there now.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "You lift the notebook off the bench without a sound. He doesn't turn round. You leave him where he chose to be.");
		Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_ThreeWeeks.OUT_SNEAK, "took the notebook and left him");
	}

	//------------------------------------------------------------------------------------------------
	//! Mercy: he's dead, and the notebook is in his coat.
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId != m_iId || !special || m_bCrated || m_bResolved)
			return;

		if (m_bCage)
			m_Capture.OnSpecialKilled(gm, missionId, special);

		// Whoever is closest gets the credit (nobody near: the stage is still over)
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		int killer = RZ_EscortHelp.PlayerBeside(ids, players, pos, 400);
		NotifyParticipants(gm, m_sTitle, "He goes down for the last time. His notebook is in his coat pocket, dry.");
		Resolve(gm, killer, RZ_EQuestApproach.FORCE, RZ_Quest_ThreeWeeks.OUT_MERCY, "ended it");
	}

	//------------------------------------------------------------------------------------------------
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		if (!m_bCage || faction != RZ_EFaction.INSTITUTE || drop != m_Drop || !m_Capture.Deliver())
			return false;

		AddParticipant(playerId);
		gm.GetCure().AddResearch(RESEARCH, "Dr Gregor Wolff is in the lab's isolation room. Halcyon is writing everything down.");
		Resolve(gm, playerId, RZ_EQuestApproach.FORCE, RZ_Quest_ThreeWeeks.OUT_CAPTURED, "brought him to the lab alive");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		if (m_bCage)
			m_Capture.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		if (m_bCrated && m_Drop)
			text += string.Format("\nThe lab's drop: grid %1 (%2)", RZ_SettlementScanner.GridReference(m_Drop.GetOrigin()), RZ_Util.DirectionText(from, m_Drop.GetOrigin()));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Three Weeks: nobody came out of the store with the notebook.";
	}

	//------------------------------------------------------------------------------------------------
	//! With the cage: the cage, the held special, the case, then the lab's drop (RZ_LiveCapture).
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_bCage)
		{
			vector drop = vector.Zero;
			if (m_Drop)
				drop = m_Drop.GetOrigin();

			RZ_ObjPlace at = m_Capture.ObjPlace(drop);
			if (at)
				return at;
		}

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Capture.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Three Weeks, stage 4 (Halcyon contract, Known): who reads the notebook. The duty officer at
//! the lab (for the director: Trusted, or Known as a Lab Tech or Doctor), the field lead at him
//! station (held while this runs), and the other factions' strongholds: Bastion's adjutant, the
//! Haven's council clerk, the Red Hand's boss's man. Only a seat that's there is offered.
class RZ_QM_WeeksReaders : RZ_QM_HalcyonScene
{
	protected int m_iLead = -1;

	protected static const int OBJ_DIRECTOR = 0;
	protected static const int OBJ_LEAD = 1;
	protected static const int OBJ_BASTION = 2;
	protected static const int OBJ_HAVEN = 3;
	protected static const int OBJ_REDHAND = 4;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 12, 20);
		m_Req.Bonus("labtech");
		m_Req.Bonus("doctor");
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
		RZ_Quest_ThreeWeeks.Voice(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		vector lab = gm.GetLabPosition();
		m_vPos = LandPointNear(lab, 6, 12);
		m_iSettlement = gm.GetLabSettlement();
		int duty = AddClue("The duty officer", QFill(gm, "The duty officer at the lab has a clipboard and a cold. \"If it's for {director}, it goes through me. He'll want to know who brought it.\""));
		AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_DIRECTOR, duty, "Talk to the duty officer", "Give it to the director", "the duty officer at the lab");

		// His seat is offered unless another mission has his just now
		RZ_Leader lead = SameLeader(gm, run, "lead");
		if (lead && lead.m_iHeldBy < 0 && gm.GetLeaders())
		{
			m_iLead = lead.m_iId;
			gm.GetLeaders().Hold(m_iLead, m_iId);
			int talk = AddClue(lead.m_sName, string.Format("%1 has cleared a space on his desk for it. \"If it says what I think it says, people have a right to hear it. All of them.\"", lead.m_sShort));
			AddSpot(NearSeat(lead.m_vPos), RZ_EPuzzleProp.CIVILIAN, OBJ_LEAD, talk, "Talk to " + lead.m_sShort, "Give it to him", string.Format("%1 at the field station in %2", lead.m_sShort, lead.m_sPlace));
		}

		AddSeat(gm, RZ_EFaction.MILITARY, OBJ_BASTION, "The adjutant", "Talk to the adjutant", "The commanding officer's adjutant reads the first page standing up, then sits down. \"The CO will want this. The CO has always said the lab knew.\"", "Give it to Bastion", "the commanding officer's adjutant");
		AddSeat(gm, RZ_EFaction.SURVIVORS, OBJ_HAVEN, "The council clerk", "Talk to the council clerk", "The Haven council's clerk wipes his hands before he touches it. \"The chair reads things like this at supper, to everyone. That's the rule.\"", "Give it to the chair", "the council clerk at the Haven");
		AddSeat(gm, RZ_EFaction.BANDITS, OBJ_REDHAND, "The boss's man", "Talk to the boss's man", "The boss's man doesn't open it. \"A dead scientist's diary? We'll read it on the open channel, slowly. Forty Favor, and the boss owes you a laugh.\"", "Sell it to the Red Hand (40 Favor)", "the boss's man at the Red Hand's camp");

		m_sItemName = "the notebook";
		m_sTitle = "The Three Weeks: who reads it";
		m_sBrief = QFill(gm, "Dr Wolff's notebook says the Institute was told on the first day of the three weeks, and wrote back \"Continue.\" {director} wants it in the record. {lead} wants it out. Bastion, the Haven and the Red Hand would all read it. Take it to one of them.");
		m_sObjective = QFill(gm, "Take Dr Wolff's notebook to one reader: {director}, Bastion, the Haven or the Red Hand.");
		if (m_iLead < 0)
			m_sBrief = QFill(gm, "Dr Wolff's notebook says the Institute was told on the first day of the three weeks, and wrote back \"Continue.\" {director} wants it in the record. Bastion, the Haven and the Red Hand would all read it. Take it to one of them.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddSeat(RZ_GameModeComponent gm, int faction, int objIdx, string who, string readLabel, string talk, string choice, string name)
	{
		RZ_Leader top = Top(gm, faction);
		if (!top)
			return;

		int clue = AddClue(who, talk);
		AddSpot(NearSeat(top.m_vPos), RZ_EPuzzleProp.CIVILIAN, objIdx, clue, readLabel, choice, name);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option != 0 || !MayAct(gm, playerId))
			return;

		RZ_QuestRun run = Run(gm);
		switch (objIdx)
		{
			case OBJ_DIRECTOR:
			{
				if (!TryPersuade(gm, playerId, RZ_EFaction.INSTITUTE, 2, "labtech,doctor"))
				{
					Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "was turned away at the lab");
					gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"{director} doesn't see people he doesn't know. Leave it with me.\" You don't. (The director takes it from someone he trusts, or from a Lab Tech or Doctor he knows.)"));
					return;
				}

				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{director} comes down himself. He reads the first page and the last, closes it, and thanks you without a flicker. \"It goes in the record. All of it that matters.\""));
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.INSTITUTE, 8, 10, "The director signed for it himself.");

				Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_ThreeWeeks.OUT_DIRECTOR, "took it to the director");
				return;
			}
			case OBJ_LEAD:
			{
				if (!SameLeader(gm, run, "lead"))
					return;

				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{lead_short} reads it at the desk, coat still on. At the one word there's a laugh, once, and then nothing. \"Tonight. On the open channel.\""));
				RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.INSTITUTE, 0, 10);
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.INSTITUTE, 3, 0, QFill(gm, "{lead_short} won't forget who brought it. The director won't either. (+10 Halcyon Infamy)"));

				Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_ThreeWeeks.OUT_LEAD, "gave it to the field lead");
				return;
			}
			case OBJ_BASTION:
			{
				RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.MILITARY, RZ_EFaction.INSTITUTE, 10, 15);
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.MILITARY, 10, 0, "Bastion's file has a new page, and your name is on the receipt. (+15 Halcyon Infamy)");

				Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_ThreeWeeks.OUT_BASTION, "gave it to Bastion", false);
				return;
			}
			case OBJ_HAVEN:
			{
				RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, 10, 10);
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, 5, 0, "The chair will read it at supper. The lab will hear who brought it. (+10 Halcyon Infamy)");

				Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_ThreeWeeks.OUT_HAVEN, "gave it to the Haven", false);
				return;
			}
			case OBJ_REDHAND:
			{
				RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE, 10, 10);
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.BANDITS, 5, 40, "\"Pleasure doing business.\" The lab will hear who sold it. (+10 Halcyon Infamy)");

				Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_ThreeWeeks.OUT_REDHAND, "sold it to the Red Hand", false);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseLead(RZ_GameModeComponent gm)
	{
		if (m_iLead >= 0 && gm.GetLeaders())
			gm.GetLeaders().Release(m_iLead);

		m_iLead = -1;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		ReleaseLead(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		ReleaseLead(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		ReleaseLead(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Three Weeks: the notebook hasn't gone to anyone yet.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Three Weeks, stage 5 when it was read out (Halcyon contract, Known): Lab Defence, because the
//! Red Hand thinks the lab has more to hide. Held: the crew goes home. Not held: the store room.
class RZ_QM_WeeksRaid : RZ_Mission_LabDefence
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 20, 20);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!super.Setup(gm))
			return false;

		m_sTitle = "The Three Weeks: the raid";
		m_sBrief = "Since the notebook was read out, the Red Hand is sure the lab has more to hide, and a crew is coming to look for it with the infected for cover. Hold the lab for four minutes.";
		m_sObjective = "Hold the lab in {lab_town} for four minutes.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Three Weeks: the lab didn't hold. The crew went through the store room.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Three Weeks, stage 5 in the record (Halcyon contract, Known): the field lead walks out. He
//! follows whoever offers (RZ_Escort) to the nearest safehouse or back to the lab. With no room for
//! another escort he goes on his own, wherever he's told. Killed on the way: he's dead.
class RZ_QM_WeeksWalkout : RZ_QM_HalcyonScene
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected bool m_bEscort;
	protected int m_iLead = -1;
	protected int m_iHouse = -1;
	protected int m_iDest;
	protected vector m_vDest;

	protected static const int OBJ_LEAD = 0;
	protected static const int DEST_HOUSE = 1;
	protected static const int DEST_LAB = 2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 10, 10);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		// Not while another mission has him (its own stand-in is out there)
		RZ_QuestRun run = Run(gm);
		RZ_Leader l = SameLeader(gm, run, "lead");
		if (!l || l.m_iHeldBy >= 0)
			return false;

		m_iLead = l.m_iId;
		m_iHouse = RZ_LeaderJob.SafehouseNear(gm, l.m_vPos, 0, 100000, -1);
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		m_vPos = NearSeat(l.m_vPos);
		m_iSettlement = gm.FindNearestSettlement(l.m_vPos);
		gm.GetLeaders().Hold(m_iLead, m_iId);

		string choices = "|Take him back to the lab";
		if (h)
			choices = string.Format("Walk him to the safehouse in %1|Take him back to the lab", h.m_sName);

		int talk = AddClue(l.m_sName, string.Format("%1 has one bag, and the notebook isn't in it. \"The record. Of course. Minus the pages that matter.\" He buttons his coat. \"I'm not staying. Where I go is the only thing I've got left to decide, and I'd rather not walk it alone.\"", l.m_sShort));
		m_Escort.m_sName = l.m_sShort;
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 1);
		m_Escort.SetTalk(m_iId, OBJ_LEAD, choices, talk, "Talk to " + l.m_sShort);
		m_bEscort = m_Escort.Begin(gm, m_vPos);
		if (!m_bEscort)
			AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_LEAD, talk, "Talk to " + l.m_sShort, choices, string.Format("%1 at the field station in %2", l.m_sShort, l.m_sPlace));

		m_sTitle = "The Three Weeks: the field lead walks out";
		m_sBrief = string.Format("%1 has heard the notebook went into the record, without the pages about the director, and he's leaving the field station in %2. He shouldn't walk alone. Take him to a safehouse, or back to the lab.", l.m_sName, l.m_sPlace);
		m_sObjective = string.Format("Walk %1 from the field station in %2 to a safehouse or the lab.", l.m_sName, l.m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bEscort)
			return;

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		if (m_Escort.IsDead())
		{
			Died(gm, ids, players);
			return;
		}

		if (m_iDest == 0 || m_Escort.GetFollowed() <= 0)
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (!m_Escort.IsNear(m_vDest, 25))
			return;

		m_Escort.Arrive();
		int playerId = m_Escort.GetFollowed();
		Arrived(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_LEAD || m_iDest != 0 || !MayAct(gm, playerId))
			return;

		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		if (option == 0 && h)
		{
			m_iDest = DEST_HOUSE;
			m_vDest = h.m_vPos;
		}
		else if (option == 1)
		{
			m_iDest = DEST_LAB;
			m_vDest = gm.GetLabPosition();
		}
		else
		{
			return;
		}

		RZ_QuestRun run = Run(gm);
		if (run && m_iDest == DEST_HOUSE)
			run.SetFlag("lead_house", m_iHouse);

		// No room for another escort: he goes on ahead on his own
		if (!m_bEscort)
		{
			Arrived(gm, playerId);
			return;
		}

		m_Escort.Follow(playerId);
		m_Escort.Go();
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("He falls in behind you without a word. %1.", RZ_Util.Capitalize(RZ_Util.DirectionText(m_vPos, m_vDest))));
	}

	//------------------------------------------------------------------------------------------------
	protected void Arrived(RZ_GameModeComponent gm, int playerId)
	{
		RZ_Leader l = gm.GetLeaders().Get(m_iLead);
		string name = "He";
		if (l)
			name = l.m_sShort;

		if (m_iDest == DEST_HOUSE)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 stops at the safehouse gate long enough to take his lab badge off. Then he goes in, and somebody gives his soup.", name));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_ThreeWeeks.OUT_TO_HOUSE, "walked him to a safehouse");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 walks into the lab as if he'd never left. The director will give him a desk where he can see it.", name));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_ThreeWeeks.OUT_TO_LAB, "brought him back to the lab");
	}

	//------------------------------------------------------------------------------------------------
	protected void Died(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		int near = RZ_EscortHelp.PlayerBeside(ids, players, m_vPos, 300);
		RZ_QuestRun run = Run(gm);
		string place = "";
		if (run)
			place = run.Text("lead_place");

		ReleaseLead(gm);
		gm.GetLeaders().KillOffscreen(m_iLead, "was killed on the road from " + place);
		Resolve(gm, near, RZ_EQuestApproach.NONE, RZ_Quest_ThreeWeeks.OUT_LEAD_DIED, "was with him on the road", false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseLead(RZ_GameModeComponent gm)
	{
		if (m_iLead >= 0 && gm.GetLeaders())
			gm.GetLeaders().Release(m_iLead);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		ReleaseLead(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		ReleaseLead(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		if (!m_bEscort)
			return super.GetWhereText(gm, from);

		string text = "He is " + RZ_Util.DirectionText(from, m_vPos);
		if (m_iDest != 0)
			text += "\nWhere he's going: " + RZ_Util.DirectionText(from, m_vDest);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Three Weeks: the field lead left on his own.";
	}

	//------------------------------------------------------------------------------------------------
	//! Him at the field station, then (he follows you) where he goes.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_iDest != 0 && m_Escort.GetFollowed() > 0 && !m_Escort.IsArrived())
			return RZ_ObjPlace.Spot(m_vDest, 25);

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
		ReleaseLead(gm);
	}
}
