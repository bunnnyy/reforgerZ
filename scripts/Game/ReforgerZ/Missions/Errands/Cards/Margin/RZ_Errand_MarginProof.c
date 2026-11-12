//! Proof's margin cards (docs/ERRANDS.md, margin cards): dealt at Halcyon's stations and the lab while
//! the major waits for the reading, never around the change of shift (the reading is a set piece at
//! the lab's change of shift), and changing only the giver's memory. Neither touches the first page.

//------------------------------------------------------------------------------------------------
//! The major "proof" waits for the reading, and it isn't the change of shift.
class RZ_MarginProof
{
	//------------------------------------------------------------------------------------------------
	static bool BeforeReading(RZ_Errands dealer)
	{
		RZ_Questlines q = dealer.GetGM().GetQuestlines();
		if (!q)
			return false;

		RZ_QuestRun run = q.FindRunByKey(RZ_Major_Proof.KEY);
		if (!run || run.m_iStage != RZ_Major_Proof.S_READING_WAIT)
			return false;

		float hour = RZ_GameTime.GetHour();
		if (hour < 0)
			return true;

		return !((hour >= 4.5 && hour <= 6.5) || (hour >= 16.5 && hour <= 18.5));
	}
}

//------------------------------------------------------------------------------------------------
//! Clean Paper (margin: Proof, before the reading): the director wants the minutes on clean paper,
//! and all of Halcyon's is written on the back of something else.
class RZ_Errand_CleanPaper : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "clean_paper";
		m_sTitle = "Clean Paper";
		m_sFamily = "halcyon";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		m_bTimed = true;
		Offer("The director wants the reading of the results minuted on clean paper. Every sheet we have has readings, or a body count, on the back. The school in the next town had reams of it, once.",
			"A small request. Paper, unused, from the school in the next town, for the minutes of the reading. Ninety-four percent of ours is already somebody's data.");
		Over("We found a ream in the stores, labelled 'reserve'. Nobody remembers reserving it.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch a ream from the school";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 20;
		w.m_fRadius = 80;
		w.m_sStep = "Search the school in {place} for paper nobody has written on (20 s there), then bring it to {giver}.";
		w.m_eProp = RZ_EErrandProp.PAPERS;
		w.m_iFavor = 4;
		w.Done("Clean, both sides. Five hundred sheets. The reading will use three. I'll keep the rest somewhere nobody will write on them.",
			"Thank you. It's a small thing. The director notices small things. That's the problem.");

		w = Way("do");
		w.m_sLabel = "Rule the margins for the minutes";
		w.m_sBackground = "labtech";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 30;
		w.m_fRadius = 15;
		w.m_sStep = "Rule the margins on the clean sheets there are at {here} (30 s), straight enough for the director.";
		w.m_iFavor = 3;
		w.m_iFame = 1;
		w.Done("Two centimetres, every one. You've done this before.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_MarginProof.BeforeReading(dealer))
			return false;

		e.m_sSubject = "proof.paper";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Chairs for the Reading (margin: Proof, before the reading): people should sit down for numbers
//! like these, the technician says. Or tell her they'll stand.
class RZ_Errand_ChairsForTheReading : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "chairs_for_the_reading";
		m_sTitle = "Chairs for the Reading";
		m_sFamily = "halcyon";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		m_sRoles = "technician,lab assistant";
		m_bTimed = true;
		Offer("Chairs, for the reading of the results. People should be sitting down for numbers like these. There's a village hall nearby with stacking ones nobody's used since spring.",
			"We need chairs from the village hall nearby, for the reading. Eleven, ideally. Twelve if anyone faints, which is a real figure from last time.");
		Over("The director says people will stand. The director says it's more honest.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry the chairs over";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 30;
		w.m_fRadius = 80;
		w.m_sStep = "Stack the chairs at the village hall in {place} and bring them back (30 s there), then tell {giver}.";
		w.m_iFavor = 4;
		w.Done("Twelve. Good. Someone will need the twelfth.",
			"In rows, please. Rows make people listen.");

		w = Way("talk");
		w.m_sLabel = "Tell her they can stand";
		w.Done("Standing, then. I'll put that in the minutes. 'Attendance: standing.'");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_MarginProof.BeforeReading(dealer))
			return false;

		e.m_sSubject = "proof.chairs";
		return true;
	}
}
