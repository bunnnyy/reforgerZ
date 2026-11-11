//! The dose outlets (docs/EPILOGUE.md, The cure in hand): one table, read from the finale's route
//! and terms (finale.route, finale.terms, the condition). Every outlet gives the dose on the spot
//! (never an item), only to the bitten (the antidote's own check), one outlet dose per life from
//! all outlets together, from a small stock that refills in real time. Outlets read standing,
//! never clothes. Players use them at the outlet's place (RZ_EpiSpots: "Ask for the dose").

//------------------------------------------------------------------------------------------------
class RZ_EpiOutlet
{
	string key;			// "house.3", "queue", "post.5", "board", "lab", "batch.2", "pan"
	int faction = -1;	// whose outlet (-1: the Independent's crate)
	vector pos;
	string place;
	int stockMax = 3;
	float refillMinutes = 30;
	int house = -1;		// a safehouse outlet's index
	string label = "Ask for the dose";
}

//------------------------------------------------------------------------------------------------
class RZ_EpiOutlets
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Epilogue m_Epi;
	protected ref array<ref RZ_EpiOutlet> m_aOutlets = {};
	protected int m_iRoute = -9;		// the row in force: the route (or the faction it fell to) and terms
	protected int m_iFaction = -1;
	protected int m_iTerms;
	protected bool m_bCondition;		// the Independent's one side: that faction's outlet, with its condition

	//------------------------------------------------------------------------------------------------
	void RZ_EpiOutlets(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		m_GM = gm;
		m_Epi = epi;
	}

	//------------------------------------------------------------------------------------------------
	array<ref RZ_EpiOutlet> GetList()
	{
		if (m_aOutlets.IsEmpty() && m_Epi.IsBuilt())
			Setup();

		return m_aOutlets;
	}

	//------------------------------------------------------------------------------------------------
	//! The row: the controller's route and terms; nobody (handed over, the slow cure) is the Fame
	//! leader's faction under its stronghold leader's terms; the Independent's one side is that
	//! faction's row with its condition.
	protected void PickRow()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		m_iRoute = m_Epi.Route();
		m_iTerms = Math.MaxInt(1, s.terms);
		m_iFaction = m_iRoute;
		m_bCondition = false;
		if (m_iRoute == RZ_CureFinale.INDEP)
		{
			m_iFaction = -1;
			if (m_iTerms == 2 && s.condition > 0)
			{
				m_iFaction = s.condition - 1;
				m_iRoute = m_iFaction;
				m_iTerms = DefaultTerms(m_iFaction);
				m_bCondition = true;
			}
		}
		else if (m_iRoute < 0 || m_iRoute > RZ_CureFinale.INDEP)
		{
			m_iFaction = s.faction;
			m_iRoute = m_iFaction;
			m_iTerms = DefaultTerms(m_iFaction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The terms a stronghold leader's traits pick when nobody chose (docs/FINALE.md, the routes).
	protected int DefaultTerms(int faction)
	{
		if (faction == RZ_EFaction.SURVIVORS && (RZ_MajorHelp.TopHas(m_GM, faction, RZ_ELeaderTrait.MERCIFUL) || RZ_MajorHelp.TopHas(m_GM, faction, RZ_ELeaderTrait.IDEALIST)))
			return 3;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The outlets for the row in force (at the commit, after a restart).
	void Setup()
	{
		m_aOutlets.Clear();
		PickRow();
		RZ_EpiSpots spots = m_Epi.GetSpots();
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (m_iRoute == RZ_CureFinale.INDEP && m_iFaction < 0)
		{
			if (m_iTerms == 1 || m_iTerms == 3)
				AddOutlet("queue", -1, spots.QueueTablePos(), "the north road", 3, 60, -1, "Take one from the crate");

			Log();
			return;
		}

		switch (m_iRoute)
		{
			case RZ_EFaction.SURVIVORS:
			{
				float refill = 60;
				if (m_iTerms == 4)
					refill = 90;

				for (int i = 0; houses && i < houses.Count(); i++)
				{
					RZ_Safehouse h = houses.Get(i);
					if (h.GetTier() >= RZ_ESafehouseTier.COMMUNITY)
						AddOutlet("house." + i.ToString(), RZ_EFaction.SURVIVORS, RZ_Mission.LandPointNear(h.m_vPos, 6, 10), h.m_sName, 2, refill, i, "Ask for the dose");
				}

				break;
			}
			case RZ_EFaction.MILITARY:
			{
				float every = 30;
				if (m_iTerms == 2)
					every = 45;

				AddOutlet("queue", RZ_EFaction.MILITARY, spots.QueueTablePos(), "the north road", 3, every, -1, "Sign for treatment");
				RZ_Territory territory = m_GM.GetTerritory();
				if (territory && territory.IsActive())
				{
					foreach (RZ_Holding hold : territory.GetHoldings())
					{
						if (hold.m_eFaction == RZ_EFaction.MILITARY)
							AddOutlet("post." + hold.m_iId.ToString(), RZ_EFaction.MILITARY, RZ_Mission.LandPointNear(hold.m_vPos, 20, 30), hold.m_sName, 3, every, -1, "Sign for treatment");
					}
				}

				break;
			}
			case RZ_EFaction.BANDITS:
			{
				AddOutlet("board", RZ_EFaction.BANDITS, spots.CrossroadsPos(), "Rudi's crossroads", 4, 20, -1, "Pay the board for a dose");
				if (m_iTerms == 3 && houses)
				{
					int pan = houses.FindNearest(spots.CrossroadsPos());
					RZ_Safehouse ph = houses.Get(pan);
					if (ph)
						AddOutlet("pan", RZ_EFaction.SURVIVORS, RZ_Mission.LandPointNear(ph.m_vPos, 6, 10), ph.m_sName, 2, 60, pan, "Ask for the dose");
				}

				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (m_GM.HasLab())
					AddOutlet("lab", RZ_EFaction.INSTITUTE, spots.LabGatePos(), "the lab", 3, 30, -1, "Ask for a dose");

				bool batches = m_iTerms == 4 || m_bCondition;
				for (int j = 0; batches && houses && j < houses.Count(); j++)
				{
					RZ_Safehouse bh = houses.Get(j);
					if (bh.HasMedic() || m_bCondition)
						AddOutlet("batch." + j.ToString(), RZ_EFaction.SURVIVORS, RZ_Mission.LandPointNear(bh.m_vPos, 6, 10), bh.m_sName, 2, 60, j, "Ask for a home batch");
				}

				break;
			}
		}

		Log();
	}

	//------------------------------------------------------------------------------------------------
	protected void Log()
	{
		RZ_Log.Info(string.Format("Epilogue: %1 dose outlets (row %2, terms %3, condition %4).", m_aOutlets.Count(), m_iRoute, m_iTerms, m_bCondition));
	}

	//------------------------------------------------------------------------------------------------
	protected void AddOutlet(string key, int faction, vector pos, string place, int stock, float refill, int house, string label)
	{
		if (pos == vector.Zero)
			return;

		RZ_EpiOutlet o = new RZ_EpiOutlet();
		o.key = key;
		o.faction = faction;
		o.pos = pos;
		o.place = place;
		o.stockMax = stock;
		o.refillMinutes = refill;
		o.house = house;
		o.label = label;
		m_aOutlets.Insert(o);

		RZ_EpilogueSave s = m_Epi.Data();
		if (s.outletKeys.Find(key) < 0)
		{
			s.outletKeys.Insert(key);
			s.outletStock.Insert(stock);
			s.outletUnix.Insert(RZ_SeasonClock.Now());
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_EpiOutlet Find(string key)
	{
		foreach (RZ_EpiOutlet o : GetList())
		{
			if (o.key == key)
				return o;
		}

		return null;
	}

	// ================================================================ Stock
	//------------------------------------------------------------------------------------------------
	//! Refills: one every refill minutes (x Test scale), never past the stock.
	void Tick()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		int now = RZ_SeasonClock.Now();
		foreach (RZ_EpiOutlet o : GetList())
		{
			int i = s.outletKeys.Find(o.key);
			if (i < 0)
				continue;

			int every = Math.MaxInt(1, RZ_Util.RoundToInt(o.refillMinutes * 60 * m_Epi.Scale()));
			if (s.outletStock[i] >= o.stockMax)
			{
				s.outletUnix[i] = now;
				continue;
			}

			int n = (now - s.outletUnix[i]) / every;
			if (n <= 0)
				continue;

			s.outletStock[i] = Math.MinInt(o.stockMax, s.outletStock[i] + n);
			s.outletUnix[i] = s.outletUnix[i] + n * every;
		}
	}

	//------------------------------------------------------------------------------------------------
	int Stock(string key)
	{
		RZ_EpilogueSave s = m_Epi.Data();
		int i = s.outletKeys.Find(key);
		if (i < 0)
			return 0;

		return s.outletStock[i];
	}

	//------------------------------------------------------------------------------------------------
	//! Real minutes until the next dose at an empty outlet.
	protected int MinutesToNext(RZ_EpiOutlet o)
	{
		RZ_EpilogueSave s = m_Epi.Data();
		int i = s.outletKeys.Find(o.key);
		if (i < 0)
			return 0;

		float every = o.refillMinutes * 60 * m_Epi.Scale();
		float left = every - (RZ_SeasonClock.Now() - s.outletUnix[i]);
		return Math.MaxInt(1, RZ_Util.RoundToInt(left / 60));
	}

	// ================================================================ Giving it
	//------------------------------------------------------------------------------------------------
	//! "Ask for the dose" at outlet `key`: the checks, the price, the dose on the spot.
	void Ask(int playerId, string key, IEntity user)
	{
		RZ_EpiOutlet o = Find(key);
		if (!o || !user)
			return;

		string title = string.Format("The dose, at %1", o.place);
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		RZ_EpilogueSave s = m_Epi.Data();
		if (!m_GM.GetInfection().SeemsInfected(playerId))
		{
			m_GM.NotifyPlayer(playerId, title, "\"You're not bitten. It's for the bitten. Come back if that changes, and hope it doesn't.\"");
			return;
		}

		string lifeKey = string.Format("%1|%2", p.m_sIdentity, p.m_iLives);
		if (s.dosed.Contains(lifeKey))
		{
			m_GM.NotifyPlayer(playerId, title, "\"You've had yours this life. One each. The old ways work as well as they ever did.\"");
			return;
		}

		string refuse = Refusal(o, playerId);
		if (!refuse.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, title, refuse);
			return;
		}

		if (Stock(key) <= 0)
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("\"None left. Come back in about %1 minutes.\"", MinutesToNext(o)));
			return;
		}

		// The open crate: once per identity for the whole week
		if (o.faction < 0 && m_iTerms == 1 && s.crate.Contains(p.m_sIdentity))
		{
			m_GM.NotifyPlayer(playerId, title, "The crate says ONE EACH. Somebody chalked your name on the lid already.");
			return;
		}

		string paid = "";
		if (!Pay(o, playerId, user, paid))
			return;

		int i = s.outletKeys.Find(key);
		s.outletStock[i] = s.outletStock[i] - 1;
		s.dosed.Insert(lifeKey);
		if (o.faction < 0 && m_iTerms == 1)
			s.crate.Insert(p.m_sIdentity);

		bool cured = true;
		if (o.key.StartsWith("batch.") && !m_bCondition && Math.RandomFloat01() >= 0.8)
			cured = false;

		if (o.key.StartsWith("batch.") && m_bCondition && Math.RandomFloat01() >= 0.8)
			cured = false;

		if (cured)
		{
			m_GM.GetInfection().UseAntidote(playerId);
		}
		else
		{
			p.m_fInfectionLeft = p.m_fInfectionLeft * 0.5;
			m_GM.NotifyPlayer(playerId, title, "The home batch goes in. It doesn't take, not fully. The fever eases, then comes back. You've bought some time. Half of what you had, they say.");
		}

		if (!paid.IsEmpty())
			m_GM.NotifyPlayer(playerId, title, paid);

		RZ_Log.Info(string.Format("Epilogue: a dose at %1 (%2 left).", o.key, s.outletStock[i]));
		m_Epi.GetClosing().OnDose(playerId, o.pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Who an outlet refuses: those its faction hunts (the Haven's exiles unless the chair chose all
	//! of them, the bite book or the condition), "" if served.
	protected string Refusal(RZ_EpiOutlet o, int playerId)
	{
		if (o.faction < 0)
			return "";

		RZ_ETier tier = m_GM.GetTier(playerId, o.faction);
		if (!RZ_Reputation.IsHunted(tier))
			return "";

		RZ_EpilogueSave s = m_Epi.Data();
		if (o.faction == RZ_EFaction.SURVIVORS)
		{
			bool all = s.choices[0] == 2 || m_bCondition || (m_iRoute == RZ_EFaction.SURVIVORS && m_iTerms == 3);
			if (all)
				return "";

			return "\"We put you out. That hasn't changed because the lab finished something.\"";
		}

		if (o.faction == RZ_EFaction.MILITARY && m_iTerms == 3)
			return "";

		return string.Format("\"Not you.\" %1 doesn't treat people it's hunting.", RZ_Util.Capitalize(RZ_Factions.GetNameInText(o.faction)));
	}

	//------------------------------------------------------------------------------------------------
	//! The price, from the row. False (and told why) if the player can't pay it.
	protected bool Pay(RZ_EpiOutlet o, int playerId, IEntity user, out string paid)
	{
		paid = "";
		int phase = m_Epi.Phase();
		string title = string.Format("The dose, at %1", o.place);
		RZ_EpilogueSave s = m_Epi.Data();
		string ident = m_GM.GetProfile(playerId).m_sIdentity;

		// The Independent: open is free; sold is 30 Favor of the buyer's best faction, going nowhere
		if (o.faction < 0)
		{
			if (m_iTerms != 3)
				return true;

			int best = BestFavor(playerId);
			if (best < 0 || !m_GM.SpendFavor(playerId, best, 30))
			{
				m_GM.NotifyPlayer(playerId, title, "The lid says 30 FAVOR. Anyone's will do. You don't have it with anyone.");
				return false;
			}

			paid = string.Format("You pay 30 %1 Favor into a tin. Nobody collects it.", RZ_Factions.GetShortName(best));
			return true;
		}

		switch (o.faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				// A Halcyon home batch at a house: 10 of the house's faction (free under the condition)
				if (o.key.StartsWith("batch."))
				{
					if (m_bCondition)
						return true;

					return Favor(playerId, RZ_EFaction.SURVIVORS, 10, title, "\"Ten for the batch. The method's free. The hands aren't.\"", paid);
				}

				// The cook's sister's house under the first pan
				if (o.key == "pan")
					return true;

				if (m_iTerms == 1 && o.house == RZ_HavenQuest.ChairHouse(m_GM))
					return true;

				if (m_iTerms == 2 && o.house == FarHouse())
					return true;

				if (m_iTerms == 3 && m_GM.GetProfile(playerId).m_aFavor[RZ_EFaction.SURVIVORS] < 5)
					return true;

				return Favor(playerId, RZ_EFaction.SURVIVORS, 5, title, "\"Five for the pot. If you've got none, ask at the big house.\"", paid);
			}
			case RZ_EFaction.MILITARY:
			{
				if (m_iTerms == 3)
					return true;

				if (!m_bCondition && RZ_Mission_FinaleDesk.TakeFirearm(user))
				{
					paid = "\"Weapon. Thank you. Name. Sign. No, here. Arm.\" Civilian, attached.";
					return true;
				}

				return Favor(playerId, RZ_EFaction.MILITARY, 15, title, "\"One firearm, or fifteen on the books. You've neither.\"", paid);
			}
			case RZ_EFaction.BANDITS:
			{
				if (phase >= 3 && m_iTerms == 1)
				{
					paid = "\"Free. We're not carrying it out.\"";
					return true;
				}

				if (m_iTerms == 4)
				{
					if (!s.owed.Contains(ident))
						s.owed.Insert(ident);

					paid = "\"Free. Thirty on your line in the book. You'll pay it. Everyone does.\"";
					return true;
				}

				int price = 30;
				if (m_iTerms == 2 || m_bCondition || s.choices[2] == 2)
					price = 15;

				if (!m_bCondition && !RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
					price = RZ_Util.RoundToInt(price * 1.5);

				if (!Favor(playerId, RZ_EFaction.BANDITS, price, title, string.Format("\"%1 Favor. The board's always right.\"", price), paid))
					return false;

				if (m_iTerms == 2 && !s.owed.Contains(ident))
				{
					s.owed.Insert(ident);
					paid = paid + " And one tin on your line in the book.";
				}

				return true;
			}
			case RZ_EFaction.INSTITUTE:
			{
				RZ_PlayerProfile p = m_GM.GetProfile(playerId);
				if (m_iTerms == 1 && RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE), RZ_ETier.TRUSTED))
					return true;

				if (m_iTerms == 2 && p.m_bTrialDoseTaken)
					return true;

				if (m_iTerms == 3 && p.m_fInfectionTotal - p.m_fInfectionLeft <= 1200)
					return true;

				return Favor(playerId, RZ_EFaction.INSTITUTE, 10, title, "\"Ten Favor. The order is on the door.\"", paid);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Favor(int playerId, int faction, int amount, string title, string refused, out string paid)
	{
		paid = "";
		if (!m_GM.SpendFavor(playerId, faction, amount))
		{
			m_GM.NotifyPlayer(playerId, title, refused);
			return false;
		}

		paid = string.Format("(%1 %2 Favor.)", amount, RZ_Factions.GetShortName(faction));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected int BestFavor(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		int best = -1;
		int most = 29;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (f < p.m_aFavor.Count() && p.m_aFavor[f] > most)
			{
				most = p.m_aFavor[f];
				best = f;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The house farthest from the chair's (the far house first).
	protected int FarHouse()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		int chair = RZ_HavenQuest.ChairHouse(m_GM);
		if (!houses || chair < 0)
			return -1;

		vector from = houses.Get(chair).m_vPos;
		int best = -1;
		float bestDist = 0;
		for (int i = 0; i < houses.Count(); i++)
		{
			float d = RZ_Util.DistXZ(houses.Get(i).m_vPos, from);
			if (d > bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	// ================================================================ Words
	//------------------------------------------------------------------------------------------------
	//! A safehouse door's notice: how the dose reaches this house, "" without one.
	string HouseNotice(int house)
	{
		foreach (RZ_EpiOutlet o : GetList())
		{
			if (o.house != house)
				continue;

			if (o.key.StartsWith("batch."))
				return "On the door: HOME BATCH HERE. ASK THE MEDIC.";

			return "On the door: THE DOSE IS HERE. ASK INSIDE.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest outlet, for road news: "The nearest dose is at Levie."
	string NearestText(vector pos)
	{
		RZ_EpiOutlet best;
		float bestDist = float.MAX;
		foreach (RZ_EpiOutlet o : GetList())
		{
			float d = RZ_Util.DistXZ(pos, o.pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = o;
			}
		}

		if (!best)
			return "Nobody's giving it out near here.";

		return string.Format("The nearest dose is at %1.", best.place);
	}
}
