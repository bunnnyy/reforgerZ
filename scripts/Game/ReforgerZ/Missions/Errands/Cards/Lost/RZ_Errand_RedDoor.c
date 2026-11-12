//! Behind the Red Door (docs/ERRANDS.md, the lost): the Kid lives at this house now, and you walked
//! him there or you're Known with the Haven. He draws a house with a red door. Find his mother's note
//! there, then tell him the truth (sideways), bring him something from his room, or tell him she's
//! coming: then he waits at the gate at dusk, every dusk you pass, to the end (a pinned local).
class RZ_Errand_RedDoor : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "red_door";
		m_sTitle = "Behind the Red Door";
		m_sFamily = "lost";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("The boy who lives with us now keeps drawing the same house: red door, a tree, a window with a cat in it. It's in {town}, I think, from the church he draws behind it. His mother isn't with him. Would you go and look?",
			"He's drawn the red door again. Same house every time, in {town}. Somebody should go and see what's there, and it shouldn't be him.");
		Over("He's stopped drawing it. Don't ask him about it.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Find the house, then tell him the truth";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 15;
		w.m_fRadius = 60;
		w.m_sStep = "Find the house with the red door in {place} and search what's left inside (15 s there), then go back to the boy at {here}.";
		w.Work(RZ_EWork.SEARCH, "Search what's left");
		w.Later("The boy sat on the step all evening after you told him. Then he came in for supper. That's something.");
		w.m_iFavor = 4;
		w.Done("You told him she went to another house. True, nearly. He nodded like a grown-up, which is worse.");

		w = Way("do");
		w.m_sLabel = "Find it, bring him a thing from his room";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 25;
		w.m_fRadius = 60;
		w.m_sStep = "Find the house with the red door in {place} and bring something from the boy's room (25 s there), then go back to him at {here}.";
		w.Work(RZ_EWork.SEARCH, "Search the boy's room");
		w.m_iFavor = 4;
		w.Done("A stuffed rabbit with one ear. He didn't say thank you. He hasn't let go of it either.");

		w = Way("do");
		w.m_sLabel = "Find it, tell him she's coming";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 15;
		w.m_fRadius = 60;
		w.m_sStep = "Find the house with the red door in {place} (15 s there), then go back to the boy at {here}.";
		w.Done("You told him she's coming. He asked when, and you said soon. He's gone to stand at the gate. He'll be there at dusk.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (!wanderers)
			return false;

		array<int> kids = {};
		wanderers.GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.KID), kids);
		string ident = gm.GetProfile(e.m_iAsker).m_sIdentity;
		foreach (int id : kids)
		{
			RZ_Wanderer w = wanderers.FindById(id);
			if (!w || !w.m_bSettled || w.m_iDestSafehouse != e.m_iHouse)
				continue;

			bool known = RZ_Reputation.IsAtLeast(gm.GetTier(e.m_iAsker, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN);
			if (!known && !w.Remembers(ident, RZ_Wanderers.MEM_HELPED) && !w.Remembers(ident, RZ_Wanderers.MEM_SAVED))
				return false;

			e.m_iValue = id;
			e.m_sSubject = "wanderer:" + id.ToString() + ".door";
			vector town = dealer.PlaceFor(e, RZ_EErrandPlace.NEXT_TOWN);
			if (town == vector.Zero)
				return false;

			RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(town));
			if (s)
				e.SetText("town", s.m_sName);

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 2)
			return;

		// He waits at the gate at dusk from now on: a pinned local, kept to the end
		RZ_Local l = dealer.GetLocals().Make(playerId, "the boy at the gate", "Is she coming today? You said soon. It's been soon for ages.", e.m_iHouse, "gate", true, "promised");
		if (l)
			said.Insert("He'll be at the gate when the light goes now, every evening. Every time you pass, he'll ask.");
	}
}
