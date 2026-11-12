// Test-only (not part of the mod): the epilogue week's phase of the runtime test (docs/EPILOGUE.md;
// the plan's "epilogue week at Test scale 0.02 with every closing item reaching its end"). It runs
// after the cure finale, whose cure starts the week. With Epilogue > Test scale at 0.02:
// 1. the story commits: the hall froze, beats were picked, phase A;
// 2. phase B; the tester is bitten and uses a dose outlet (one per life, its price);
// 3. two questlines are begun and listed as loose ends: one closes live (a way), one as data;
// 4. Bring Them Home: a named dead person's keepsake is taken and handed in at a house; an
//    epilogue deck card (Take a Name to the Board) is true and writes a name home;
// 5. the Last Entry; the chair's grey choice through the decide hold; a say at the station;
// 6. a town with crawlers is cleared (taken back); the ridges and the Long Table's week close;
// 7. processing (phase C): the broadcast's tally closes; the tester walks out at the barrier;
// 8. the wipe's lines: names not brought home, the season block's lines, last_season.json's beats;
//    every closing item ended.
// Lines: "RZT epilogue check: <name>: OK|FAILED (...)", "RZT epilogue: ..." and
// "RZT epilogue done: N checks OK, N FAILED".

modded class RZ_Epilogue
{
	void RZT_Scale(float scale)
	{
		m_Settings.m_fTestScale = scale;
	}
}

modded class RZ_EpiClosing
{
	//! A running questline listed as a loose end now (the test begins it after the commit).
	void RZT_AddLoose(string key)
	{
		RZ_QuestRun run = m_GM.GetQuestlines().FindRunByKey(key);
		if (!run || Item("loose." + key))
			return;

		string giver;
		string call;
		LooseCall(key, giver, call);
		AddItem("loose." + key, run.m_Line.m_sTitle, run.m_Line.m_iFaction, PlaceOf(run), 1, giver, call);
	}

	void RZT_Loose(int playerId, string key, int way)
	{
		LooseWay(playerId, key, way);
	}

	int RZT_State(string key)
	{
		RZ_EpiItem it = Item(key);
		if (!it)
			return -1;

		return it.state;
	}

	bool RZT_AllEnded(out string open)
	{
		open = "";
		foreach (RZ_EpiItem it : S().items)
		{
			if (it.state < 2)
				open += it.key + " ";
		}

		return open.IsEmpty();
	}

	void RZT_Tick()
	{
		Decide();
		Towns();
		Ridges();
		TableWeek(new array<IEntity>());
	}
}

modded class RZ_GameModeComponent
{
	//! The fake player's new life after the walk-out.
	void RZT_Spawned(int playerId, IEntity e)
	{
		HandlePlayerSpawned(playerId, e);
	}
}

class RZT_Epilogue
{
	protected static int s_iStep;
	protected static int s_iSec;
	protected static int s_iOk;
	protected static int s_iFailed;
	protected static ref array<string> s_aLoose = {};

	//------------------------------------------------------------------------------------------------
	protected static void Check(string name, bool ok, string detail)
	{
		if (ok)
		{
			s_iOk++;
			Print(string.Format("RZT epilogue check: %1: OK", name));
			return;
		}

		s_iFailed++;
		Print(string.Format("RZT epilogue check: %1: FAILED (%2)", name, detail));
	}

	//------------------------------------------------------------------------------------------------
	protected static void Next()
	{
		s_iStep++;
		s_iSec = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Every second of the epilogue's phase. True when it's done.
	static bool Tick(RZ_GameModeComponent gm)
	{
		RZ_Epilogue epi = gm.GetEpilogue();
		if (!epi || !RZ_Epilogue.IsOn())
		{
			Print("RZT epilogue: the epilogue is off, skipped");
			return true;
		}

		s_iSec++;
		switch (s_iStep)
		{
			case 0: return Start(gm, epi);
			case 1: return Built(gm, epi);
			case 2: return Dose(gm, epi);
			case 3: return Loose(gm, epi);
			case 4: return Home(gm, epi);
			case 5: return Choices(gm, epi);
			case 6: return Decided(gm, epi);
			case 7: return Places(gm, epi);
			case 8: return Processing(gm, epi);
			case 9: return Wipe(gm, epi);
		}

		Print(string.Format("RZT epilogue done: %1 checks OK, %2 FAILED", s_iOk, s_iFailed));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! 1. Test scale 0.02; the cure (from the finale's phase) builds the story over a few ticks.
	protected static bool Start(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		epi.RZT_Scale(0.02);
		if (!gm.GetCure().IsCured())
		{
			Print("RZT epilogue: no cure (the finale's phase didn't end with one), skipped");
			return true;
		}

		Print("RZT epilogue: phase starts (Test scale 0.02)");
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Built(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		if (!epi.IsBuilt())
		{
			if (s_iSec > 60)
			{
				Check("the story commits", false, "not built after 60 s");
				return true;
			}

			return false;
		}

		Check("the story commits", true, "");
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && RZ_CureFinale.IsOn() && finale.Controller() >= 0)
			Check("the record has the release's terms", epi.Data().terms > 0 && epi.Data().terms == finale.Terms(), string.Format("record %1, finale %2", epi.Data().terms, finale.Terms()));

		Check("the hall of fame froze", !gm.GetHall() || gm.GetHall().IsFrozen(), "not frozen");
		Check("beats were picked", epi.GetBeats().Count() > 0, "no beats");
		Check("phase A", epi.Phase() == 1, string.Format("phase %1", epi.Phase()));
		Print(string.Format("RZT epilogue: %1 beats, %2 open items, %3 outlets", epi.GetBeats().Count(), epi.GetClosing().OpenCount(), epi.GetOutlets().GetList().Count()));
		epi.ForcePhase(2);
		Check("phase B", epi.Phase() == 2, string.Format("phase %1", epi.Phase()));
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 2. A dose: the tester is bitten, has the Favor, and asks at the first outlet.
	protected static bool Dose(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		array<ref RZ_EpiOutlet> outlets = epi.GetOutlets().GetList();
		if (outlets.IsEmpty())
		{
			Print("RZT epilogue: no outlet under these terms (kept back), the dose skipped");
			Next();
			return false;
		}

		RZ_EpiOutlet o = outlets[0];
		RZ_PlayerProfile p = gm.GetProfile(RZT_Fake.ID);
		p.m_fInfectionTotal = 1200;
		p.m_fInfectionLeft = 1100;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			gm.AddFavor(RZT_Fake.ID, f, 50);
		}

		RZT_Fake.MoveTo(o.pos);
		epi.GetOutlets().Ask(RZT_Fake.ID, o.key, RZT_Fake.s_Ent);
		Check("a dose at an outlet cures on the spot", !p.IsInfected(), string.Format("outlet %1, still infected", o.key));
		p.m_fInfectionTotal = 1200;
		p.m_fInfectionLeft = 1100;
		epi.GetOutlets().Ask(RZT_Fake.ID, o.key, RZT_Fake.s_Ent);
		Check("one outlet dose per life", p.IsInfected(), "a second dose was given");
		gm.GetInfection().UseAntidote(RZT_Fake.ID);
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 3. Loose Ends: two questlines begun now are listed; one ends live, one as data.
	protected static bool Loose(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		RZ_Questlines q = gm.GetQuestlines();
		array<string> keys = {"door", "census", "walk", "pilot", "well", "cargo", "other_two", "haven_short_weight"};
		foreach (string key : keys)
		{
			if (s_aLoose.Count() >= 2)
				break;

			RZT_Prep.BeforeQuest(gm, key);
			if (q.IsRunning(key) || q.Begin(key))
			{
				epi.GetClosing().RZT_AddLoose(key);
				s_aLoose.Insert(key);
			}
		}

		if (s_aLoose.Count() < 2)
		{
			Print(string.Format("RZT epilogue: only %1 questlines could begin, loose ends partly skipped", s_aLoose.Count()));
		}

		if (s_aLoose.Count() >= 1)
		{
			epi.GetClosing().RZT_Loose(RZT_Fake.ID, s_aLoose[0], 0);
			Check("a loose end closes live", epi.GetClosing().RZT_State("loose." + s_aLoose[0]) == 2 && !q.IsRunning(s_aLoose[0]), s_aLoose[0]);
		}

		if (s_aLoose.Count() >= 2)
		{
			epi.GetClosing().CloseNow("loose." + s_aLoose[1]);
			Check("a loose end closes as data", epi.GetClosing().RZT_State("loose." + s_aLoose[1]) == 3 && !q.IsRunning(s_aLoose[1]), s_aLoose[1]);
		}

		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 4. Bring Them Home: a name's keepsake taken where it lies, handed in at a house.
	protected static bool Home(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() == 0)
		{
			Print("RZT epilogue: no safehouse, Bring Them Home skipped");
			Next();
			return false;
		}

		RZ_Safehouse h = houses.Get(0);
		vector at = RZ_Mission.LandPointNear(h.m_vPos, 300, 500);
		epi.AddNamed("Pavel Novak", "his ID card", -1, at, "", "", "the trader");
		int idx = epi.Data().names.Count() - 1;
		RZ_EpiSpot lies = new RZ_EpiSpot();
		lies.key = "keep.test";
		lies.pos = at;
		lies.title = "Something lying here";
		epi.GetClosing().Verb(RZT_Fake.ID, "take:" + idx.ToString(), lies, RZT_Fake.s_Ent);
		Check("a keepsake is taken", epi.Data().names[idx].state == 1, "not carried");
		RZ_EpiSpot house = new RZ_EpiSpot();
		house.key = "house.0";
		house.pos = h.m_vPos;
		house.title = "Safehouse: " + h.m_sName;
		epi.GetClosing().Verb(RZT_Fake.ID, "home", house, RZT_Fake.s_Ent);
		Check("a name is brought home to the wall", epi.Data().names[idx].state == 2 && epi.GetClosing().WallCount() >= 1, string.Format("state %1", epi.Data().names[idx].state));
		epi.GetClosing().WriteLine(RZT_Fake.ID, "");
		Check("the Last Entry", epi.Data().entries.Count() >= 1, "no line");

		// The epilogue deck: Take a Name to the Board is true for a name still lying out, and writes it home
		epi.AddNamed("Jana Kralova", "her ID card", -1, RZ_Mission.LandPointNear(h.m_vPos, 200, 400), "", "", "the nurse");
		int jana = epi.Data().names.Count() - 1;
		RZ_Errand_NameToTheBoard card = new RZ_Errand_NameToTheBoard();
		card.Define();
		RZ_Errand e = new RZ_Errand();
		e.m_Def = card;
		e.m_iAsker = RZT_Fake.ID;
		e.m_vGiver = h.m_vPos;
		e.m_iHouse = 0;
		e.m_iFaction = RZ_EFaction.SURVIVORS;
		bool why = gm.GetErrands() && card.Why(gm.GetErrands(), e);
		Check("an epilogue card is true in the week", why && card.m_bAfterCure, "not true");
		if (why)
		{
			array<string> said = {};
			card.Resolve(gm.GetErrands(), e, card.m_aWays[0], RZT_Fake.ID, said);
			RZ_EpiName named = RZ_EpiCard.NameCalled(e.m_sValue);
			Check("the card writes the name home", named && named.state == 2, e.m_sValue);
		}

		if (epi.Data().names[jana].state == 0)
			epi.Data().names[jana].state = 2;

		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 5. The chair's question through the decide hold; a say at the station.
	protected static bool Choices(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (chair)
		{
			RZT_Fake.MoveTo(chair.m_vPos);
			epi.GetClosing().ChoicePick(chair, RZT_Fake.ID, false);
		}
		else
		{
			Print("RZT epilogue: no chair, the grey choice skipped");
		}

		RZ_EpiSpot station = new RZ_EpiSpot();
		station.key = "station";
		station.title = "The radio station";
		epi.GetClosing().Verb(RZT_Fake.ID, "say:0", station, RZT_Fake.s_Ent);
		Check("a say at the station", epi.Data().sayIdent.Count() >= 1, "no say");
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Decided(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		epi.GetClosing().RZT_Tick();
		if (s_iSec < 8)
			return false;

		if (RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS))
			Check("a grey choice decided after the hold", epi.Data().choices[0] == 2, string.Format("choice %1", epi.Data().choices[0]));

		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 6. A town with crawlers at the cure is cleared and taken back; the ridges and the farm close.
	protected static bool Places(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		RZ_EpilogueSave s = epi.Data();
		if (s_iSec == 1)
		{
			if (!s.townIdx.IsEmpty())
			{
				RZ_Settlement st = gm.GetSettlement(s.townIdx[0]);
				if (st)
					st.m_fLedger = st.m_fLedger + 10000;
			}

			epi.GetClosing().CloseNow("ridges");
			return false;
		}

		epi.GetClosing().RZT_Tick();
		if (!s.townIdx.IsEmpty())
			Check("a town is taken back", s.townTaken[0] > 0, "still has crawlers");
		else
			Print("RZT epilogue: no town had crawlers at the cure, Take Back a Town skipped");

		if (epi.GetClosing().RZT_State("ridges") >= 0)
			Check("the ridges went quiet", epi.GetClosing().RZT_State("ridges") >= 2, "open");

		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 7. Processing: the broadcast's tally closes, the farm packs and leaves, the tester walks out.
	protected static bool Processing(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		if (s_iSec == 1)
		{
			epi.ForcePhase(3);
			Check("phase C", epi.Phase() == 3, string.Format("phase %1", epi.Phase()));
			Check("the Last Broadcast is decided before processing", epi.Data().broadcast >= 0, "open");
			if (epi.GetClosing().RZT_State("table") >= 0)
			{
				epi.GetClosing().CloseNow("table");
				Check("the Long Table's week ends", epi.GetClosing().RZT_State("table") >= 2, "open");
			}

			// The tester: three hours into a life, unarmed, at the barrier
			gm.GetProfile(RZT_Fake.ID).m_fLifeSeconds = 3 * 3600;
			RZ_EpiSpot barrier = new RZ_EpiSpot();
			barrier.key = "barrier";
			barrier.title = "The barrier";
			epi.GetClosing().Verb(RZT_Fake.ID, "walk", barrier, RZT_Fake.s_Ent);
			Check("the tester walks out", epi.Data().walked.Count() >= 1, "not walked");
			return false;
		}

		if (s_iSec < 6)
			return false;

		Check("a walk-out isn't a death", !epi.IsWalking(RZT_Fake.ID) || !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent), "still walking");
		IEntity e = RZT_Fake.Spawn(gm.GetLabPosition());
		gm.RZT_Spawned(RZT_Fake.ID, e);
		Next();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 8. The wipe: the week's lines go into the season block; every closing item has ended.
	protected static bool Wipe(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		epi.AddNamed("Vera Hruba", "her ID card", -1, gm.GetLabPosition(), "", "", "the mechanic");
		epi.OnWipeDue();
		RZ_EpilogueSave s = epi.Data();
		Check("names nobody carried are NOT BROUGHT HOME", s.names[s.names.Count() - 1].state == 3, "still lying out");
		Check("the season block has the week's lines", !s.hallLines.IsEmpty(), "no lines");
		RZ_SeasonRecord rec = RZ_Persistence.LoadSeason();
		Check("last_season.json keeps the week's beats", rec && rec.beats.Count() > 0, "no beats");
		string open;
		Check("every closing item reached its end", epi.GetClosing().RZT_AllEnded(open), "open: " + open);
		int since = 0;
		int dropped = 0;
		foreach (RZ_EpiBeat b : s.beats)
		{
			if (b.sinceOn)
				since++;

			if (b.dropped)
				dropped++;
		}

		Print(string.Format("RZT epilogue: beats %1 (%2 since, %3 dropped), wall %4, walked %5, hall lines %6", s.beats.Count(), since, dropped, epi.GetClosing().WallCount(), s.walked.Count(), s.hallLines.Count()));
		foreach (string line : s.hallLines)
		{
			Print("RZT epilogue: hall line: " + line);
		}

		Next();
		return false;
	}
}
