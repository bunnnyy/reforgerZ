//! Await Relief's margin cards (docs/ERRANDS.md, margin cards): dealt at Bastion's posts while the major
//! waits between its beats, never on the nightly call's evening hours (the muster is a set piece at the
//! call), and changing only the giver's memory. They share no premise with the major: the count and the
//! call stay its own.

//------------------------------------------------------------------------------------------------
//! The major "orders" is at one of these waits now.
class RZ_MarginOrders
{
	//------------------------------------------------------------------------------------------------
	static bool Waiting(RZ_Errands dealer, int stage)
	{
		RZ_Questlines q = dealer.GetGM().GetQuestlines();
		if (!q)
			return false;

		RZ_QuestRun run = q.FindRunByKey("orders");
		return run && run.m_iStage == stage;
	}

	//------------------------------------------------------------------------------------------------
	//! Not in the call's evening hours, when the muster would be on.
	static bool NotCallHours()
	{
		float hour = RZ_GameTime.GetHour();
		return hour < 0 || hour < 18 || hour > 22;
	}
}

//------------------------------------------------------------------------------------------------
//! Weather, Write It Down (margin: Await Relief, before Effective Strength): the radio operator wants
//! a weather reading from the ridge for the log, because the CO writes "weather" against everything.
class RZ_Errand_WeatherWriteItDown : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "weather_write_it_down";
		m_sTitle = "Weather, Write It Down";
		m_sFamily = "bastion";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_sRoles = "radio operator,operator";
		m_bTimed = true;
		Offer("The log wants a weather reading from the ridge before the call: wind, cloud, visibility. The CO writes 'weather' against anything that went wrong this season, the helicopter included, so it may as well be accurate.",
			"I need someone up on the ridge to read the weather for the log. I can't leave the radio before the call. Everything in the log is weather, lately.");
		Over("Never mind the reading. The CO's written 'weather' in the log already. The CO usually does.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Take the reading on the ridge";
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_sStep = "Climb the ridge {place} and note the wind, the cloud and how far you can see (15 s there), then report to {giver}.";
		w.m_iFavor = 4;
		w.Done("Wind westerly, cloud broken, visibility good. I've written it down. So nothing that goes wrong today is the weather. Interesting.",
			"Logged, word for word. Weather. Written down. The CO will be pleased, or the CO will be something.");

		w = Way("talk");
		w.m_sLabel = "Tell him to just write 'weather'";
		w.Done("Ha. Weather. Write it down. All right, I have. Nobody pays for a joke the CO already made, mind.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_MarginOrders.Waiting(dealer, RZ_Major_Orders.S_COUNT_WAIT) || !RZ_MarginOrders.NotCallHours())
			return false;

		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".weather";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Clean Rifles (margin: Await Relief, before the muster): the post wants its rifles clean for the
//! call, and the cleaning kits are in the next town's police station. Or a Soldier shows the new lads.
class RZ_Errand_CleanRifles : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "clean_rifles";
		m_sTitle = "Clean Rifles";
		m_sFamily = "bastion";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_bTimed = true;
		Offer("The CO wants rifles clean for the nightly call, a unit that looks like a unit. We've the rifles. We've no cleaning kits left. The police station in the next town had a cupboard of them, once.",
			"Cleaning kits, from the police station in the next town. The CO wants us looking like a unit at the call, and we'd like to oblige while the CO can still tell the difference.");
		Over("The kits came up with the truck from the stronghold. Somebody's thinking of us after all.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch the cleaning kits";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 20;
		w.m_fRadius = 80;
		w.m_sStep = "Search the police station in {place} for cleaning kits (20 s there), then bring them to {giver}.";
		w.Work(RZ_EWork.SEARCH, "Search for cleaning kits");
		w.m_eProp = RZ_EErrandProp.CRATE;
		w.m_iFavor = 4;
		w.Done("Oil, rods, patches. Like Christmas, if Christmas was regulation.",
			"That'll do. We'll shine for the call. Shining is what's left.");

		w = Way("do");
		w.m_sLabel = "Show the new lads how it's done";
		w.m_sBackground = "soldier";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 15;
		w.m_sStep = "Strip and clean a rifle in front of the new lads at {here} (a minute), slowly enough for them to follow.";
		w.Work(RZ_EWork.BENCH, "Strip and clean the rifle", 15);
		w.m_iFavor = 3;
		w.m_iFame = 1;
		w.Done("They've got it. Mostly. One of them even put the spring back the right way on his own.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_MarginOrders.Waiting(dealer, RZ_Major_Orders.S_MUSTER_WAIT) || !RZ_MarginOrders.NotCallHours())
			return false;

		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".rifles";
		return true;
	}
}
