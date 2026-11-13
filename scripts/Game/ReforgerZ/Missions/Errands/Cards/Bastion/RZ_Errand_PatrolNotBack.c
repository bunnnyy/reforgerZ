//! The Patrol That Didn't Come Back (docs/ERRANDS.md, Bastion): a town near a Bastion post is half
//! taken (infection 50+), and the patrol sent into it didn't come back. Go in, find where they fell,
//! and carry one soldier's identity card home (Props > Soldier's ID): his name goes on the call. Or,
//! Known with the Red Hand, sell the card to the fence, for whom a dead man's name is a door.
class RZ_Errand_PatrolNotBack : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "patrol_not_back";
		m_sTitle = "The Patrol That Didn't Come Back";
		m_sFamily = "bastion";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("We sent four into {town} at first light, Corporal {dead}'s section. The radio stopped at eleven. Nobody's going in after them today. I'm not asking for the men. I'm asking for one card, so we can write the letters.",
			"{dead}'s patrol went into {town} and didn't come out. The town's more theirs than ours now. Bring me back one identity card from where they stopped. One. So the return isn't a guess.");
		Over("The section's been posted missing. Someone else went in. Leave it.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring a card home";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Go into {town}, {where_place}, find where {dead}'s patrol stopped and take an identity card from them (10 s there), then carry it to {giver} yourself.";
		w.Work(RZ_EWork.TAKE, "Take the identity card");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_bCarry = true;
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("That's {dead}'s face. Younger than I remember it. The letters can go now. Thank you for going in where we wouldn't.",
			"One card, and four letters. I'll read the names on the call tonight.");

		w = Way("do");
		w.m_sLabel = "Sell the card to the fence";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Go into {town}, {where_place}, take a card from {dead}'s patrol (10 s there), and sell it to the Red Hand's fence, who'll give it a new face. Then tell {giver} there was nothing to bring.";
		w.Work(RZ_EWork.TAKE, "Take the identity card");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFavor = 10;
		w.Done("Nothing? Then they're missing, and missing is what the return will say. Missing, until somebody walks up to a gate with one of their cards.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Settlement town = gm.GetSettlement(gm.FindNearestSettlement(e.m_vGiver));
		if (!town || town.m_fInfection < 50 || RZ_Util.DistXZ(town.m_vCenter, e.m_vGiver) > 1500)
			return false;

		Place(e, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(town.m_vCenter, 40, 120)), "");
		e.m_sSubject = "patrol:" + town.m_sName;
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 53);
		e.SetText("dead", RZ_NoteText.Surname(rng));
		e.SetText("town", town.m_sName);
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override ResourceName CarriedPrefab(RZ_Errand e)
	{
		return RZ_ErrandSettings.Get().Props().m_sSoldiersID;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		array<int> who = {playerId};
		string fate = "home";
		if (w.m_iIndex != 0)
			fate = "sold";

		RZ_WorldFacts.Get().Memory().Record("errand.patrol_not_back", e.m_sSubject, fate, RZ_WorldMemory.SOURCE_ERRAND, true, who);
	}
}
