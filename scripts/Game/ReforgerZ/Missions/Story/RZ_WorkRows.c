//! The work rows (docs/MISSIONS.md, "Work"): the jobs that are plain work keep their play and get one
//! row each here: the reasons they answer (the reason lane, RZ_MissionManager.ReasonLane), whether
//! they write the town tally, and a fallout line said when they fail. The town tally is one per town
//! in the season flags: how often work was done there and whose mark is on the wall now (Haven
//! chalk, a Bastion stencil, a painted red hand, Halcyon's tape). The next work job offered in that
//! town reads it into its brief. Server only.
class RZ_WorkRow
{
	string m_sKey;			// saved in the tally's log line only
	string m_sReasons;		// "town.infested,horde.passed"
	bool m_bTally;
	string m_sFallout;
}

//------------------------------------------------------------------------------------------------
class RZ_WorkRows
{
	protected static ref map<string, ref RZ_WorkRow> s_mRows;

	//------------------------------------------------------------------------------------------------
	protected static void Add(typename t, string key, string reasons, bool tally, string fallout)
	{
		RZ_WorkRow r = new RZ_WorkRow();
		r.m_sKey = key;
		r.m_sReasons = reasons;
		r.m_bTally = tally;
		r.m_sFallout = fallout;
		s_mRows.Set(t.ToString(), r);
	}

	//------------------------------------------------------------------------------------------------
	//! The row for type `t`, null if it isn't a work job.
	static RZ_WorkRow Row(typename t)
	{
		if (!s_mRows)
			Build();

		RZ_WorkRow r;
		s_mRows.Find(t.ToString(), r);
		return r;
	}

	//------------------------------------------------------------------------------------------------
	static int Count()
	{
		if (!s_mRows)
			Build();

		return s_mRows.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected static void Build()
	{
		s_mRows = new map<string, ref RZ_WorkRow>();

		// The Haven
		Add(RZ_Mission_RecoverTheBus, "recover_bus", "town.infested", true, "The bus stays where it stopped. Kids play in it now, which nobody likes.");
		Add(RZ_Mission_Memorial, "memorial", "house.lost_to", true, "The names stay in the book, not on a wall. The book's getting full.");

		// Bastion
		Add(RZ_Mission_ClearSettlement, "retake_town", "town.infested,horde.passed", true, "The town stays theirs.");
		Add(RZ_Mission_LastHelicopter, "last_heli", "", false, "The wreck stays where it came down. The crows have it now.");
		Add(RZ_Mission_BurnTheBodies, "burn_bodies", "town.shelled", true, "The bodies stay where they fell, and the town smells of it for days.");
		Add(RZ_Mission_Recon, "recon", "town.infested", false, "The map keeps a blank where that town should be. Bastion fills it in with a guess.");
		Add(RZ_Mission_FireMission, "fire_mission", "town.infested", true, "The guns stay quiet. Whatever was there is still there.");

		// The Red Hand
		Add(RZ_Mission_FenceTheGoods, "fence_goods", "post.truck_lost", false, "The goods sit in a shed. The fence marks you down as slow.");
		Add(RZ_Mission_CarTheft, "car_theft", "", false, "The car's still where it was. So is its owner, and he's heard.");
		Add(RZ_Mission_PharmacyHeist, "pharmacy_heist", "wanderer.bitten", true, "The shelves stay full for somebody else to empty.");
		Add(RZ_Mission_Arson, "arson", "town.shelled", false, "The depot stands. Bastion's trucks run on time, and they know someone tried.");
		Add(RZ_Mission_SabotageRelay, "sabotage_relay", "", false, "The relay keeps talking. Bastion keeps listening.");
		Add(RZ_Mission_CollectTheBounty, "bounty", "", false, "The name stays on the notice. The price goes up.");

		// Halcyon
		Add(RZ_Mission_SampleHunt, "lone_one", "cure.*", false, "It walks on alone. The lab's board says it's still out there.");
		Add(RZ_Mission_FieldSamples, "field_samples", "cure.*,wanderer.bitten", false, "The racks stay half empty. The work slows by a day.");
		Add(RZ_Mission_PowerTheLab, "power_lab", "cure.*", false, "The lab runs on its batteries tonight. The freezers hum a little lower.");
		Add(RZ_Mission_InstallSensors, "sensors", "town.infested", true, "The map stays dark there. Halcyon draws a question mark on it.");
		Add(RZ_Mission_BloodDrive, "blood_drive", "house.bitten,cure.*", false, "The vials go back in the box empty. Someone will be asked again.");
		Add(RZ_Mission_SpecimenFromTheNest, "specimen_nest", "town.infested", true, "The nest keeps what the lab wanted, and grows.");
		Add(RZ_Mission_HuntScreamer, "screamer", "town.infested", true, "The screaming goes on at night. People sleep in the cellars now.");
		Add(RZ_Mission_MedicalDrone, "drone", "", false, "The drone's still out there in the trees, blinking. Somebody else will find it.");
		Add(RZ_Mission_PatientZero, "patient_zero", "cure.*", false, "Patient Zero stays a guess. The lab writes 'unknown' and underlines it.");
		Add(RZ_Mission_LiveCapture, "live_capture", "cure.*", false, "The cage goes back empty. The lab works with what it has.");
		Add(RZ_Mission_MobileLab, "mobile_lab", "house.bitten", false, "The van stays parked. The bitten queue at the lab's gate instead.");

		// World events
		Add(RZ_Event_AbandonedConvoy, "abandoned_convoy", "", false, "Somebody else emptied it. The tyres went last.");
		Add(RZ_Event_TheSniper, "sniper", "", false, "He's still up there. People cross that road at a run now.");
		Add(RZ_Event_StashNote, "stash_note", "", false, "The stash stays hidden, or somebody else read the note first.");
		Add(RZ_Event_MassGrave, "mass_grave", "", true, "Nobody counted them. The Haven writes 'many' in the book.");
		Add(RZ_Event_LockedBunker, "bunker", "", false, "The door stays shut. Whatever's behind it waits for someone with more patience.");
		Add(RZ_Event_TunnelRats, "tunnel_rats", "", true, "The tunnels stay theirs. They come up somewhere new every night.");
		Add(RZ_Event_RemnantCamp, "remnant_camp", "", false, "The camp's men stay out there, with their own orders.");
		Add(RZ_Event_WeddingParty, "wedding", "", false, "The party goes on without you. Nobody there will stop dancing now.");

		// Puzzles
		Add(RZ_Mission_NumbersStation, "numbers", "", false, "The numbers go on every night at nine. Nobody writes them down now.");
		Add(RZ_Mission_TwoBearings, "two_bearings", "", false, "The signal fades. Whoever was sending it stops, or can't.");
		Add(RZ_Mission_PatientZeroLedger, "pz_ledger", "", false, "The ledger stays closed. Halcyon files it as missing.");
		Add(RZ_Mission_PoisonedWell, "poisoned_well", "", true, "The well stays bad. The town carries its water from further off.");
		Add(RZ_Mission_SmugglersLedger, "smugglers_ledger", "", false, "The crew's books still don't balance, and somebody pays for it.");
		Add(RZ_Mission_ClockworkHorde, "clockwork", "town.infested", true, "They keep their hours. The town learns them, and hides on time.");
		Add(RZ_Mission_TimeLockSafe, "time_lock", "", false, "The safe keeps what's in it. The lock keeps counting.");
		Add(RZ_Mission_BellOrder, "bell_order", "", false, "The bells ring wrong. Something answers them anyway.");
		Add(RZ_Mission_LightsInTheWindow, "lights_window", "", false, "The light goes on at dusk and off at dawn. Nobody goes to see who.");
		Add(RZ_Mission_JunctionBoxes, "junction_boxes", "", true, "The grid stays down there. The street stays dark.");
		Add(RZ_Mission_WeighingVault, "weighing_vault", "", false, "The vault keeps its weights. Nobody's counted them right yet.");
		Add(RZ_Mission_LastExperiment, "last_experiment", "", false, "The last experiment stays the last. Its notes go in a drawer.");

		// Unarmed and the perimeter
		Add(RZ_Mission_Distraction, "distraction", "town.infested", false, "They never looked away. The other job went loud.");
		Add(RZ_Mission_QuietCount, "quiet_count", "town.infested", false, "The count stays a guess, and the guess is low.");
		Add(RZ_Mission_BlindSpot, "blind_spot", "", false, "The gap in the drones' lines stays a rumour.");
		Add(RZ_Mission_ShotAtTheLine, "shot_line", "", false, "The line stays as it is. Somebody else will try it.");

		// Leader jobs (in person only: they answer no reasons)
		Add(RZ_Job_OurOwnFirst, "our_own_first", "", false, "The tins go where they were going. Both houses notice.");
		Add(RZ_Job_BringThemHome, "fetch_family", "", false, "The family stays in the town. Somebody leaves a light on for them.");
		Add(RZ_Job_OverTheFence, "over_fence", "", false, "The trade doesn't happen. The medicine stays on Bastion's side of the fence.");
		Add(RZ_Job_RationRun, "ration_run", "", false, "The sack stays in the store. The pills stay with the crews.");
		Add(RZ_Job_NamesForBastion, "names_bastion", "", false, "The list stays in the drawer. Bastion asks again, less politely.");
		Add(RZ_Job_TrainingDay, "training_day", "", false, "The recruit stays a civilian, and alive. The sergeant counts the roster again.");
		Add(RZ_Job_HiddenStores, "hidden_stores", "", false, "The counter finds the shelves as full as they are. Somebody has a hard week.");
		Add(RZ_Job_HeartsAndMinds, "hearts_minds", "", true, "The town keeps its doors shut when the patrol passes.");
		Add(RZ_Job_RiflesForSale, "rifles_sale", "", false, "The rifles stay on Bastion's books. The crews buy elsewhere.");
		Add(RZ_Job_BurnTheLedger, "burn_ledger", "", false, "The ledger survives. So does what's written in it.");
		Add(RZ_Job_SkimTheToll, "skim_toll", "", false, "The toll comes in to the penny. The boss notices that too.");
		Add(RZ_Job_SoftTouch, "soft_touch", "", false, "The man pays in full, the hard way.");
		Add(RZ_Job_SellOutRivalCrew, "sell_out_crew", "", false, "The other crew hears it was nearly them. They remember who nearly did it.");
		Add(RZ_Job_SecondOpinion, "second_opinion", "", false, "The first opinion stands. The director signs it.");
		Add(RZ_Job_UnsanctionedTrial, "unsanctioned_trial", "", false, "The trial stays on paper. The patient waits in the corridor.");
		Add(RZ_Job_LeakTheData, "leak_data", "", false, "The data stays inside the lab. So do the questions.");
		Add(RZ_Job_SellTheFormula, "sell_formula", "", false, "The formula stays in the safe. The buyer finds another seller.");

		// Story arcs
		Add(RZ_Mission_DeserterMap, "deserter_map", "", false, "The map stays in pieces. The bunker keeps its secret.");
		Add(RZ_Mission_FeverWater, "fever_water", "", false, "Nobody tested the water. The fevers carry on.");
		Add(RZ_Mission_FeverEvacuation, "fever_out", "", true, "The gate didn't hold long enough. The house is smaller now.");
	}

	//------------------------------------------------------------------------------------------------
	//! The reasons a work job answers.
	static void Reasons(typename t, notnull array<string> result)
	{
		RZ_WorkRow r = Row(t);
		if (!r || r.m_sReasons.IsEmpty())
			return;

		array<string> keys = {};
		r.m_sReasons.Split(",", keys, true);
		foreach (string k : keys)
		{
			if (!result.Contains(k))
				result.Insert(k);
		}
	}

	//------------------------------------------------------------------------------------------------
	static string Fallout(typename t)
	{
		RZ_WorkRow r = Row(t);
		if (!r)
			return "";

		return r.m_sFallout;
	}

	// ================================================================ The town tally
	//------------------------------------------------------------------------------------------------
	protected static string TallyKey(int settlement, string what)
	{
		return string.Format("tally.%1.%2", settlement, what);
	}

	//------------------------------------------------------------------------------------------------
	//! A work job done in a town: one more on its tally, and the giver's mark is the one on the wall.
	static void Ended(RZ_Mission m, bool success)
	{
		RZ_WorkRow r = Row(m.Type());
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (!r || !r.m_bTally || !success || m.m_bWithdrawn || m.m_iSettlement < 0 || !facts)
			return;

		int by = 0;
		if (m.m_bHasGiver)
			by = m.m_eGiver + 1;

		int n = facts.Flag(TallyKey(m.m_iSettlement, "n")) + 1;
		facts.SetFlag(TallyKey(m.m_iSettlement, "n"), n);
		facts.SetFlag(TallyKey(m.m_iSettlement, "by"), by);
		RZ_Log.Info(string.Format("Story: tally %1 (%2) = %3, mark %4 (%5)", m.m_iSettlement, m.GetLocationName(RZ_GameModeComponent.GetInstance()), n, by, r.m_sKey));
	}

	//------------------------------------------------------------------------------------------------
	//! How often work was done in this town, 0 if never.
	static int Tally(int settlement)
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (!facts || settlement < 0)
			return 0;

		return facts.Flag(TallyKey(settlement, "n"));
	}

	//------------------------------------------------------------------------------------------------
	//! A work job offered in a town with a tally: its brief says whose mark is on the wall.
	static void Read(RZ_Mission m)
	{
		if (!Row(m.Type()) || m.m_iSettlement < 0)
			return;

		int n = Tally(m.m_iSettlement);
		if (n <= 0)
			return;

		int by = RZ_WorldFacts.Get().Flag(TallyKey(m.m_iSettlement, "by")) - 1;
		m.m_sBrief += " " + TallyLine(by, n);
	}

	//------------------------------------------------------------------------------------------------
	//! "Haven chalk on the church door counts 3."
	static string TallyLine(int by, int n)
	{
		string times = string.Format("%1 times", n);
		if (n == 1)
			times = "once";

		switch (by)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("Haven chalk on the church door says it's been done here %1. The last line is the newest.", times);
			case RZ_EFaction.MILITARY: return string.Format("A Bastion stencil on the post office wall: cleared %1. Dated, initialled, straight.", times);
			case RZ_EFaction.BANDITS: return string.Format("A red hand painted over the last mark on the wall. Done here %1, and the crews want it known.", times);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon tape on a lamp post, numbered: %1 here. The latest strip is still sticky.", times);
		}

		return string.Format("Somebody's chalk on a wall says it's been done here %1.", times);
	}
}
