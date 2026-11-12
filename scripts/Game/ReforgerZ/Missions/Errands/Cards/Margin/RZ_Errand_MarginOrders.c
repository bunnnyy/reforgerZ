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
		Offer("Log wants a weather reading from the ridge. Wind, cloud, visibility. The CO writes 'weather' against anything that went wrong, so it may as well be accurate.",
			"I need eyes on the weather from up {place}. For the log. Everything in the log is weather, lately.");
		Over("Never mind the reading. The CO's written 'weather' already. He usually does.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Take the reading on the ridge";
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_sStep = "Go up {place} and take the wind, the cloud and how far you can see (15 s there), then report to {giver}.";
		w.m_iFavor = 4;
		w.Done("\"Wind westerly, cloud broken, visibility good.\" He writes it down. \"So nothing that happens today is the weather. Interesting.\"",
			"He logs it word for word. \"Weather. Written down.\"");

		w = Way("talk");
		w.m_sLabel = "Tell him to write \"weather\" and skip it";
		w.Done("\"Ha. Weather. Write it down.\" He does. Nobody pays you for a joke the CO already made.");
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
		Offer("Rifles clean for the call, the CO says. We've the rifles. We've no kits. The police station in {place} had a cupboard of them, once.",
			"Cleaning kits. The station in {place}. The CO wants us looking like a unit at the call, and we'd like to oblige him while he can still tell.");
		Over("Kits came up with the truck. Somebody's thinking of us after all.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch the cleaning kits";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 20;
		w.m_fRadius = 80;
		w.m_sStep = "Search the police station in {place} for cleaning kits (20 s there), then bring them to {giver}.";
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
		w.m_sStep = "Strip and clean a rifle with the new lads at {here} (a minute), slowly enough to follow.";
		w.m_iFavor = 3;
		w.m_iFame = 1;
		w.Done("They've got it. Mostly. One of them put the spring back the right way on his own.");
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
