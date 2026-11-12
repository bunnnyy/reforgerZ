//! Names on the Call (docs/ERRANDS.md, Bastion): civilians were sheltered at Bastion's posts this season
//! (the season count `bastion.sheltered`) and a Haven house is within 1.5 km. The operator reads the
//! number every night; nobody at the houses hears the names. Carry the list to the house.
class RZ_Errand_NamesOnTheCall : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "names_call";
		m_sTitle = "Names on the Call";
		m_sFamily = "bastion";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_sRoles = "radio operator,operator";
		Offer("Every night on the call I read Command 'civilians sheltered, {count}'. A number. I have the names behind it, every one, in a book nobody asked for. The Haven house {subject_place} is missing people. Take them the list.",
			"I've {count} names of civilians this post has sheltered, on a list nobody asked for. The house {subject_place} has people they've stopped expecting. Do the arithmetic. Take it to them.");
		Over("Never mind the list. Someone from the house came and read it at the gate themselves.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry the list to the house";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Pin the list of sheltered civilians on the board at {subject_place} (5 s at the door), then tell {giver}.";
		w.Work(RZ_EWork.WRITE, "Pin the list up", 5);
		w.m_iFavor = 4;
		w.Done("Somebody found a name on it? Good. That's the first time the number's been for anything but Command.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		int sheltered = gm.GetFacts().Flag("bastion.sheltered");
		if (sheltered <= 0)
			return false;

		int house = HouseNear(gm, e.m_vGiver, 1500, -1, 0);
		if (house < 0)
			return false;

		RZ_Safehouse h = gm.GetSafehouses().Get(house);
		Place(e, h.m_vPos, h.m_sName);
		e.SetText("count", sheltered.ToString());
		e.m_sSubject = RZ_WorldFacts.Safehouse(house) + ".names";
		return true;
	}
}
