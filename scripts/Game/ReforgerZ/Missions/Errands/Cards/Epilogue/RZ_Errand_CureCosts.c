//! What the Cure Costs Here (the epilogue deck, errands' card 41): a sick local the terms keep from a
//! dose (no outlet of the house's own and no Haven or open-crate outlet within 2 km). The epilogue's
//! one outlet table decides the way, by whose rules the cure goes out:
//!   Bastion's roster: register them at a post's list, or [Trusted: Bastion] vouch for them;
//!   the Red Hand's book: pay their dose (15 Favor), put it on their line in the book, or
//!     [Trusted: Red Hand] haggle (8 Favor);
//!   anyone else's, or no outlet at all: sit with them.
//! Where the terms reach them there's no card (it's a thank-you instead). They tell people who
//! brought it.
class RZ_Errand_CureCosts : RZ_EpiCard
{
	protected static const int W_REGISTER = 0;
	protected static const int W_VOUCH = 1;
	protected static const int W_PAY = 2;
	protected static const int W_BOOK = 3;
	protected static const int W_HAGGLE = 4;
	protected static const int W_SIT = 5;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_cure_costs";
		m_sTitle = "What the Cure Costs Here";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("{local} has the fever, and the cure's going out by somebody else's rules: Bastion's roster, the Red Hand's book, the lab's list. Nobody's coming to this house with a dose. Go and find out what it costs, here, for {local}.",
			"{local} is sick, and the nearest dose isn't ours to give. Whoever holds it has terms. Find out what they cost, and whether any of us can pay them.");
		Over("Never mind. A dose came for {local} this morning.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Register {local} at the post's list";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 30;
		w.m_sStep = "Put {local}'s name on Bastion's roster at {subject_place} (20 s there), then tell {giver}.";
		w.m_iFavor = 3;
		w.Done("On the roster. They'll come down the list to {local}. It's a long list. They'll come.");

		w = Way("talk");
		w.m_sLabel = "Vouch for {local}";
		w.m_iMinTier = RZ_ETier.TRUSTED;
		w.m_iTierFaction = RZ_EFaction.MILITARY;
		w.Done("Your word on the list? Then the army medic comes tonight. Thank you.");

		w = Way("pay");
		w.m_sLabel = "Pay for {local}'s dose";
		w.m_iFavorCost = 15;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.Done("Paid? In full? I'm holding your hand too long, I know. We won't forget who paid.");

		w = Way("talk");
		w.m_sLabel = "Put it on {local}'s line in the book";
		w.Done("A line in the Red Hand's book, for a dose. It's a fair trade. It's still a line, and {local} will be paying it after the gates open.");

		w = Way("pay");
		w.m_sLabel = "Haggle for {local}'s dose";
		w.m_iFavorCost = 8;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_iMinTier = RZ_ETier.TRUSTED;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.Done("Eight? You got it for eight? That's the first laugh this house has had all week.");

		w = Way("do");
		w.m_sLabel = "Sit with {local}";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 15;
		w.m_sStep = "Sit with {local} at the house a while (a minute), then tell {giver}.";
		w.m_iFavor = 2;
		w.Done("Nobody had a dose for {local}. You had an hour, and you gave it. That'll be remembered here.");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi || epi.Phase() >= 3 || Reached(epi, e.m_vGiver, e.m_iHouse))
			return false;

		RZ_Locals locals = dealer.GetLocals();
		if (!locals)
			return false;

		array<RZ_Local> here = {};
		locals.AtHouse(e.m_iHouse, here);
		foreach (RZ_Local l : here)
		{
			if (!l.alive || l.memos.Contains("dosed"))
				continue;

			e.m_iValue = l.id;
			e.m_sSubject = "epi.cost:" + l.id.ToString();
			e.SetText("local", l.name);
			RZ_EpiOutlet post = Nearest(epi, RZ_EFaction.MILITARY, e.m_vGiver);
			if (post)
			{
				e.m_vSubject = post.pos;
				e.SetText("subject_place", post.place);
			}

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The terms reach this house: an outlet of its own, or a Haven or open-crate one within 2 km.
	protected static bool Reached(RZ_Epilogue epi, vector house, int idx)
	{
		foreach (RZ_EpiOutlet o : epi.GetOutlets().GetList())
		{
			if (o.house == idx && idx >= 0)
				return true;

			if ((o.faction == RZ_EFaction.SURVIVORS || o.faction < 0) && RZ_Util.DistXZ(o.pos, house) <= 2000)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_EpiOutlet Nearest(RZ_Epilogue epi, int faction, vector from)
	{
		RZ_EpiOutlet best;
		float bestDist = 3000;
		foreach (RZ_EpiOutlet o : epi.GetOutlets().GetList())
		{
			float d = RZ_Util.DistXZ(o.pos, from);
			if (o.faction == faction && d < bestDist)
			{
				bestDist = d;
				best = o;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Only the way the table allows: by whose rules the cure goes out.
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		RZ_Epilogue epi = Week();
		if (!epi)
			return false;

		int rules = epi.Faction();
		switch (w.m_iIndex)
		{
			case W_REGISTER: return rules == RZ_EFaction.MILITARY && e.m_vSubject != vector.Zero;
			case W_VOUCH: return rules == RZ_EFaction.MILITARY;
			case W_PAY: return rules == RZ_EFaction.BANDITS;
			case W_BOOK: return rules == RZ_EFaction.BANDITS;
			case W_HAGGLE: return rules == RZ_EFaction.BANDITS;
		}

		return rules != RZ_EFaction.MILITARY && rules != RZ_EFaction.BANDITS;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_Local l = dealer.GetLocals().Get(e.m_iValue);
		if (!l)
			return;

		if (w.m_iIndex == W_SIT)
		{
			dealer.GetLocals().Remember(l, playerId, "helped");
			return;
		}

		dealer.GetLocals().Remember(l, playerId, "dosed");
		if (w.m_iIndex == W_BOOK)
		{
			RZ_Epilogue epi = Week();
			if (epi && !epi.Data().owed.Contains(e.m_sIdent))
				epi.Data().owed.Insert(e.m_sIdent);
		}

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("epi.cost", l.name, "dosed", RZ_WorldMemory.SOURCE_ERRAND, true, who,
			e.Fill("{local} tells everyone who brought the dose."), "voice");
	}
}
