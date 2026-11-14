//! The rest of the cure finale (docs/FINALE.md): what the first pass left out, in one place, owned
//! by the controller (RZ_CureFinale.GetRest) and saved with it.
//! - Before the gate: seeds from 25 to 50 percent research (unmarked lines on the channels), the
//!   lab's readout at each change of shift on the open channel, and rumours of who is close.
//! - The lock: Hand to Hand (promises ease the eight's yes and shorten the sit-in; the look's -1 is
//!   RZ_DisguiseClaims.OtherRoute's), route marks on the verge (an equal share per
//!   route with an active lock; the losing routes' come down at the first dusk of the week), notes
//!   about other routes sold at any route's desk.
//! - The moves: drain at a container (20 s beside a route item on the ground), guard the door (a
//!   route moves its door once: rivals' sneaks and tricks there one step harder, talk needs a pact),
//!   hand it over (a carried item to a rival, logged), Last Mile's contracts at the desks (Buy It,
//!   Sign It In, Nobody's, Keep It Overnight), the deal at the gate during the batch hold.
//! - Each route's remaining ways, as choices on its scenes (MoreChoices): the Haven's terms one by
//!   one and moving him; Bastion's ledgers, the clipboard's guard and the carries; the Red Hand's
//!   Who Came Back, the Mechanic's coil, the volunteer by kind and The Bottles; Halcyon's canisters'
//!   riddle, the vials and the ways down; the Independent's carry on air.
//! - Live tissue's rules: within reach of the lab (5 km, straight line x 1.3), the same pack never
//!   twice in 6 season days, a great one's leader +3.
//! - The stand: Bastion's checkpoint on the lab road under Bastion, Command (a rival's carry is
//!   searched: a look, or a 90 s wait).
//! - The majors' remaining inputs, read where each check is made (PageEase and the route files).
//! Server only. Nothing polls per frame.

//------------------------------------------------------------------------------------------------
class RZ_FinaleRestSave
{
	int seedDay = -1;			// the season day of the last seed
	int seedNext;				// played seconds to the next seed
	int shiftKey = -1;			// day * 3 + shift of the last readout
	ref array<string> marks = {};		// "route|x|z": route marks on the verge
	bool marksDown;				// the losing routes' marks came down
	ref array<string> notes = {};		// "ident|route|unix": what a player knows about another route
	ref array<string> sold = {};		// "ident|route": notes sold (once a route)
	ref array<int> guarded = {};		// routes that guarded their door (once a season)
	ref array<string> drained = {};		// item ids drained at the container
	ref array<string> tissueTribes = {};	// "tribe|season day": tissue given
	ref array<string> checked = {};		// "ident": searched at Bastion's lab-road checkpoint this stand
	ref array<string> overnight = {};	// "item id|unix": kept overnight in the cold
	bool gateDeal;				// the deal at the gate was made
	ref array<string> dealIdents = {};	// "route|ident" agreeing at the gate
}

//------------------------------------------------------------------------------------------------
class RZ_FinaleRest
{
	protected static const int HOUR = 3600;
	protected static const int DAY = 86400;
	protected static const float DRAIN_SECONDS = 20;
	protected static const float DRAIN_NEAR = 6;
	protected static const float CHECKPOINT_RADIUS = 30;
	protected static const float CHECKPOINT_WAIT = 90;
	protected static const int MARKS_PER_ROUTE = 2;
	protected static const float TISSUE_REACH = 5000;
	protected static const int TISSUE_DAYS = 6;

	protected RZ_GameModeComponent m_GM;
	protected RZ_CureFinale m_Finale;
	protected ref RZ_FinaleRestSave m_S = new RZ_FinaleRestSave();
	protected float m_fCheck;
	protected ref map<string, float> m_mDrain = new map<string, float>();	// "item|ident" -> seconds
	protected ref map<string, float> m_mHeld = new map<string, float>();		// ident -> seconds at the checkpoint
	protected ref map<string, int> m_mMarkTold = new map<string, int>();	// "ident|mark" -> unix

	//------------------------------------------------------------------------------------------------
	void RZ_FinaleRest(RZ_GameModeComponent gm, RZ_CureFinale finale)
	{
		m_GM = gm;
		m_Finale = finale;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_FinaleRestSave data)
	{
		if (data)
			m_S = data;
	}

	//------------------------------------------------------------------------------------------------
	RZ_FinaleRestSave Save()
	{
		return m_S;
	}

	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! Every controller tick with players online: seeds before the gate; after it, the readout, the
	//! marks, the drain, the checkpoint.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fCheck += dt;
		if (m_fCheck < 5)
			return;

		float step = m_fCheck;
		m_fCheck = 0;
		RZ_CureResearch cure = m_GM.GetCure();
		ReleaseCall();
		if (!m_Finale.IsOpen())
		{
			if (cure)
				Seeds(step, cure.ShownProgress());

			return;
		}

		Readout();
		Marks(ids, players);
		Drains(step, ids, players);
		Checkpoint(step, ids, players);
		Overnight();
	}

	// ================================================================ Before the gate
	//------------------------------------------------------------------------------------------------
	//! Seeds from 25 to 50 percent research: unmarked, one every 90 played minutes or so, about one
	//! line in fifteen of the channel's own. Never says what they mean.
	protected void Seeds(float step, float progress)
	{
		if (progress < 25 || progress > 50 || !m_GM.GetFactionLife())
			return;

		m_S.seedNext = m_S.seedNext - RZ_Util.RoundToInt(step);
		if (m_S.seedNext > 0)
			return;

		m_S.seedNext = 5400;
		array<string> lines = {};
		array<int> factions = {};
		lines.Insert("Bowl out at the back door again tonight. For the cat. We haven't got a cat.");
		factions.Insert(RZ_EFaction.SURVIVORS);
		lines.Insert("Medic here. We had one who didn't take. Fever went down on its own. Nobody knows why.");
		factions.Insert(RZ_EFaction.SURVIVORS);
		lines.Insert("Day 6 trial, discharged eight. One did not present.");
		factions.Insert(RZ_EFaction.INSTITUTE);
		lines.Insert("Stores, detached, nightly. Temperature up a degree again. Diesel 20 L: stores.");
		factions.Insert(RZ_EFaction.MILITARY);
		lines.Insert("Smoke past the still's town again last night. Somebody's cooking something that isn't soup.");
		factions.Insert(RZ_EFaction.BANDITS);
		lines.Insert("Reference stock still wanted. Any sample predating Day 1, to the duty officer.");
		factions.Insert(RZ_EFaction.INSTITUTE);
		lines.Insert("The vet's invoice came round again. Nitrogen, one tank. Paid by nobody.");
		factions.Insert(RZ_EFaction.SURVIVORS);
		int pick = Math.AbsInt(RZ_WorldFacts.Get().Day() * 7 + m_S.seedDay) % lines.Count();
		m_S.seedDay = m_S.seedDay + 1;
		m_GM.GetFactionLife().OpenChannelLine(factions[pick], lines[pick]);
	}

	//------------------------------------------------------------------------------------------------
	//! The lab's readout at each change of shift (06:00, 14:00, 22:00) on Halcyon's open channel,
	//! and a rumour of who is close on the Haven's, a third of the time wrong.
	protected void Readout()
	{
		if (!RZ_GameTime.IsCalibrated() || !m_GM.GetFactionLife())
			return;

		float hour = RZ_GameTime.GetHour();
		int shift = -1;
		if (hour >= 6 && hour < 6.5)
			shift = 0;
		else if (hour >= 14 && hour < 14.5)
			shift = 1;
		else if (hour >= 22 && hour < 22.5)
			shift = 2;

		int key = RZ_WorldFacts.Get().Day() * 3 + shift;
		if (shift < 0 || key == m_S.shiftKey)
			return;

		m_S.shiftKey = key;
		m_GM.GetFactionLife().OpenChannelLine(RZ_EFaction.INSTITUTE, "Change of shift. " + m_Finale.Readout());
		int lead = RZ_CureFinale.NONE;
		float best = 0;
		for (int r = 0; r < RZ_CureFinale.ROUTES; r++)
		{
			if (m_Finale.Track(r) > best)
			{
				best = m_Finale.Track(r);
				lead = r;
			}
		}

		if (lead == RZ_CureFinale.NONE)
			return;

		// The rumour: a third of the time it names the wrong route (the one after the leader)
		if (Math.AbsInt(key * 31) % 3 == 0)
			lead = (lead + 1) % RZ_CureFinale.ROUTES;

		m_GM.GetFactionLife().OpenChannelLine(RZ_EFaction.SURVIVORS, string.Format("Word at the gate is %1 are close. Word at the gate is usually wrong.", RZ_CureFinale.RouteName(lead)));
	}

	// ================================================================ The lock
	//------------------------------------------------------------------------------------------------
	//! Hand to Hand: a living promised local in the same house (within 400 m of `at`) as one of the
	//! eight, through the errands' own query (RZ_Errands.PromisedAt).
	bool Promised(string ident, vector at)
	{
		RZ_Errands errands = m_GM.GetErrands();
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!errands || !houses)
			return false;

		int house = houses.FindNearest(at, 400);
		return house >= 0 && errands.PromisedAt(house, ident) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Hand to Hand at the Independent's sit-in: each promise the route's locked players made takes
	//! a minute off its 10 (6 at most).
	float SitInSeconds()
	{
		int promises = 0;
		array<RZ_PlayerProfile> profiles = {};
		m_GM.GetAllProfiles(profiles);
		foreach (RZ_PlayerProfile p : profiles)
		{
			if (!p.m_sIdentity.IsEmpty() && m_Finale.RouteOf(p.m_sIdentity) == RZ_CureFinale.INDEP)
				promises += RZ_WorldFacts.Get().Flag("errand.hands." + p.m_sIdentity);
		}

		return 600 - 60 * Math.Min(6, promises);
	}

	//------------------------------------------------------------------------------------------------
	//! Route marks from the gate: an equal share of verge places (two) for every route with an
	//! active lock, near its seat, private (finale.marks_public 0): a route's locked players are
	//! told of their own route's marks as they pass (40 m), once a real day.
	protected void Marks(array<int> ids, array<IEntity> players)
	{
		RZ_WorldFacts.Get().SetFlag("finale.marks_public", 0);
		for (int r = 0; r < RZ_CureFinale.ROUTES; r++)
		{
			if (m_Finale.ActiveLocks(r) <= 0 || HasMarks(r))
				continue;

			vector seat = SeatOf(r);
			if (seat == vector.Zero)
				continue;

			for (int i = 0; i < MARKS_PER_ROUTE; i++)
			{
				vector p = RZ_Mission.LandPointNear(seat, 300 + 250 * i, 500 + 250 * i);
				m_S.marks.Insert(string.Format("%1|%2|%3", r, p[0], p[2]));
			}
		}

		int now = RZ_SeasonClock.Now();
		foreach (int k, IEntity ent : players)
		{
			int r2 = m_Finale.RouteOfPlayer(ids[k]);
			if (r2 == RZ_CureFinale.NONE)
				continue;

			foreach (int m, string mark : m_S.marks)
			{
				array<string> parts = {};
				mark.Split("|", parts, false);
				if (parts.Count() < 3 || parts[0].ToInt() != r2)
					continue;

				if (RZ_Util.DistXZ(ent.GetOrigin(), Vector(parts[1].ToFloat(), 0, parts[2].ToFloat())) > 40)
					continue;

				string key = string.Format("%1|%2", ids[k], m);
				int told = 0;
				m_mMarkTold.Find(key, told);
				if (now - told < DAY)
					continue;

				m_mMarkTold.Set(key, now);
				m_GM.NotifyPlayer(ids[k], RZ_CureFinale.RouteTitle(r2), MarkText(r2));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasMarks(int r)
	{
		string head = r.ToString() + "|";
		foreach (string m : m_S.marks)
		{
			if (m.StartsWith(head))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static string MarkText(int r)
	{
		switch (r)
		{
			case RZ_CureFinale.HAVEN: return "A strip of sheet tied to a fence post on the verge, knotted twice. The Haven's road.";
			case RZ_CureFinale.BASTION: return "A stencilled arrow on the verge marker: HFC. Bastion's road.";
			case RZ_CureFinale.REDHAND: return "A red hand on a road sign, small, at knee height. The crews' road.";
			case RZ_CureFinale.HALCYON: return "A numbered tag wired to the verge post. Halcyon's road.";
		}

		return "Chalk on the verge post: a dash. Nobody's road. Everyone's.";
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue's first dusk: the losing routes' marks come down; the winner's are made final.
	void MarksAtDusk(int winner)
	{
		if (m_S.marksDown)
			return;

		m_S.marksDown = true;
		for (int i = m_S.marks.Count() - 1; i >= 0; i--)
		{
			array<string> parts = {};
			m_S.marks[i].Split("|", parts, false);
			if (parts.IsEmpty() || parts[0].ToInt() != winner)
				m_S.marks.Remove(i);
		}

		RZ_Log.Info(string.Format("Finale: the losing routes' marks came down (%1 left).", m_S.marks.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! The winner's marks, for the epilogue's places ("" for none).
	string WinnerMarkText(int winner)
	{
		if (!HasMarks(winner))
			return "";

		return MarkText(winner);
	}

	//------------------------------------------------------------------------------------------------
	protected vector SeatOf(int r)
	{
		if (r == RZ_CureFinale.INDEP)
		{
			if (!m_GM.HasLab())
				return vector.Zero;

			return m_GM.GetLabPosition();
		}

		RZ_Leader l = RZ_MajorHelp.Top(m_GM, r);
		if (!l)
			return vector.Zero;

		return l.m_vPos;
	}

	// ================================================================ Notes
	//------------------------------------------------------------------------------------------------
	//! A player took part in another route's scene: they know something about it (a note).
	void Noted(int playerId, int route)
	{
		string ident = Ident(playerId);
		int mine = m_Finale.RouteOfPlayer(playerId);
		if (ident.IsEmpty() || mine == route)
			return;

		string head = string.Format("%1|%2|", ident, route);
		for (int i = m_S.notes.Count() - 1; i >= 0; i--)
		{
			if (m_S.notes[i].StartsWith(head))
				m_S.notes.Remove(i);
		}

		m_S.notes.Insert(head + RZ_SeasonClock.Now().ToString());
	}

	// ================================================================ The desks
	//------------------------------------------------------------------------------------------------
	//! The night man's labels at every route's desk (RZ_Mission_FinaleDesk, spot 4).
	static string DeskLabels()
	{
		return "Sell what you know of a route [15 Favor]|Hand over what you carry|Buy what a rival carries [30 Favor]|Sign a found item in for its route";
	}

	//------------------------------------------------------------------------------------------------
	void DeskChoice(int route, int playerId, int option)
	{
		switch (option)
		{
			case 0: SellNote(route, playerId); break;
			case 1: HandOver(route, playerId); break;
			case 2: BuyIt(route, playerId); break;
			case 3: SignItIn(route, playerId); break;
			case 4: GateDeal(route, playerId); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Notes about other routes, sold to this desk's faction for 15 Favor (once a route). That's
	//! crossing the route sold: +10 Infamy with its faction if a lookout sees (half the time), and its
	//! leader remembers. A note older than two real days is found out by events: nothing paid, 5
	//! Infamy with the buyer.
	protected void SellNote(int route, int playerId)
	{
		string ident = Ident(playerId);
		string title = RZ_CureFinale.RouteTitle(route);
		int faction = RZ_CureFinale.PayFaction(route);
		foreach (string note : m_S.notes)
		{
			array<string> parts = {};
			note.Split("|", parts, false);
			if (parts.Count() < 3 || parts[0] != ident)
				continue;

			int about = parts[1].ToInt();
			string key = string.Format("%1|%2", ident, about);
			if (about == route || m_S.sold.Contains(key))
				continue;

			m_S.sold.Insert(key);
			if (RZ_SeasonClock.Now() - parts[2].ToInt() > 2 * DAY * m_GM.GetQuestlines().WaitScale())
			{
				if (RZ_Factions.IsValid(faction))
					m_GM.AddInfamy(playerId, faction, 5, "sold a stale note", 0, "sold a note that turned out false");

				m_GM.NotifyPlayer(playerId, title, "\"That was true last week.\" The clerk tears it in half. \"Things moved. You'd know, if you'd looked.\"");
				return;
			}

			if (RZ_Factions.IsValid(faction))
				m_GM.AddFavor(playerId, faction, 15);

			int theirs = RZ_CureFinale.RouteFaction(about);
			if (RZ_Factions.IsValid(theirs) && Math.AbsInt((ident + about.ToString()).Hash()) % 2 == 0)
			{
				m_GM.AddInfamy(playerId, theirs, 10, "sold another route's business", 0, "sold a note about their route");
				m_Finale.Hostile(m_Finale.RouteOfPlayer(playerId), about);
			}

			m_GM.NotifyPlayer(playerId, title, string.Format("The clerk writes it down word for word: what you saw of %1's route. \"Fifteen. And we never met.\"", RZ_CureFinale.RouteName(about)));
			return;
		}

		m_GM.NotifyPlayer(playerId, title, "You know nothing about anyone's route that we don't. Come back when you do.");
	}

	//------------------------------------------------------------------------------------------------
	//! Hand it over: what you carry for your route goes to a rival route's locked player within
	//! 10 m, logged; +15 Infamy with your own route's faction.
	protected void HandOver(int route, int playerId)
	{
		string ident = Ident(playerId);
		int mine = m_Finale.RouteOfPlayer(playerId);
		string title = RZ_CureFinale.RouteTitle(route);
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me || mine == RZ_CureFinale.NONE || m_Finale.GetItems().CarriesRoute(ident, mine) <= 0)
		{
			m_GM.NotifyPlayer(playerId, title, "You're not carrying anything of your route's.");
			return;
		}

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			int theirs = m_Finale.RouteOfPlayer(ids[i]);
			if (ids[i] == playerId || theirs == RZ_CureFinale.NONE || theirs == mine || RZ_Util.DistXZ(p.GetOrigin(), me.GetOrigin()) > 10)
				continue;

			if (m_Finale.GetItems().Pass(ident, Ident(ids[i]), mine) <= 0)
				return;

			int faction = RZ_CureFinale.RouteFaction(mine);
			if (RZ_Factions.IsValid(faction))
				m_GM.AddInfamy(playerId, faction, 15, "handed the route's item to a rival", 0, "handed over what the route trusted them with");

			RZ_Log.Info(string.Format("Finale: hand it over: %1's item from %2 to %3.", RZ_CureFinale.RouteName(mine), RZ_Util.PlayerName(playerId), RZ_Util.PlayerName(ids[i])));
			m_GM.NotifyPlayer(playerId, title, "You hand it over. Somebody writes down the time.");
			m_GM.NotifyPlayer(ids[i], title, string.Format("%1 hands you what they were carrying for %2.", RZ_Util.PlayerName(playerId), RZ_CureFinale.RouteName(mine)));
			return;
		}

		m_GM.NotifyPlayer(playerId, title, "Nobody from another route is close enough to take it.");
	}

	//------------------------------------------------------------------------------------------------
	//! Last Mile's Buy It: 30 Favor with the carrier's route faction, and what a rival carries for
	//! another route is now this desk's route's (the carrier must be within 30 m and agree by
	//! standing still: as data, the nearest carrier).
	protected void BuyIt(int route, int playerId)
	{
		string title = RZ_CureFinale.RouteTitle(route);
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		int mine = m_Finale.RouteOfPlayer(playerId);
		if (!me || mine != route)
		{
			m_GM.NotifyPlayer(playerId, title, "Buy it for whom? Sign with us first.");
			return;
		}

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			int theirs = m_Finale.RouteOfPlayer(ids[i]);
			if (theirs == RZ_CureFinale.NONE || theirs == mine || RZ_Util.DistXZ(p.GetOrigin(), me.GetOrigin()) > 30)
				continue;

			string seller = Ident(ids[i]);
			if (m_Finale.GetItems().CarriesRoute(seller, theirs) <= 0)
				continue;

			int f = RZ_CureFinale.PayFaction(theirs);
			if (!RZ_Factions.IsValid(f) || !m_GM.SpendFavor(playerId, f, 30))
			{
				m_GM.NotifyPlayer(playerId, title, "Thirty. With their people, not ours.");
				return;
			}

			m_Finale.GetItems().Pass(seller, Ident(playerId), theirs);
			m_GM.NotifyPlayer(playerId, title, "Bought, off the road, for thirty. It's yours to carry now.");
			m_GM.NotifyPlayer(ids[i], title, string.Format("%1 bought what you carried. Thirty, paid to your own people.", RZ_Util.PlayerName(playerId)));
			return;
		}

		m_GM.NotifyPlayer(playerId, title, "Nobody close by is carrying anything for another route.");
	}

	//------------------------------------------------------------------------------------------------
	//! Last Mile's Sign It In: a found item (any route's, carried by you) handed to this desk for its
	//! own route; 5 Fame with that route's faction, and Nobody's: the Independent's desk takes any of
	//! them for everyone.
	protected void SignItIn(int route, int playerId)
	{
		string ident = Ident(playerId);
		string title = RZ_CureFinale.RouteTitle(route);
		int handed = m_Finale.GetItems().GiveBack(ident, route);
		if (handed <= 0 && route == RZ_CureFinale.INDEP)
			handed = m_Finale.GetItems().HandInAny(ident, RZ_CureFinale.INDEP);

		if (handed <= 0)
		{
			m_GM.NotifyPlayer(playerId, title, "Nothing of ours on you.");
			return;
		}

		int f = RZ_CureFinale.PayFaction(route);
		if (RZ_Factions.IsValid(f))
			m_Finale.PayIdent(ident, f, 5, 0, 0, "signed a found item in");

		m_GM.NotifyPlayer(playerId, title, "Signed in, with the time and your name. \"Thank you. That one had us worried.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The deal at the gate: during the batch hold, one locked player from the holder's route and one
	//! from the partner's agree at the lab gate (within 60 m of it). Both: the deal is made.
	protected void GateDeal(int route, int playerId)
	{
		string title = RZ_CureFinale.RouteTitle(route);
		int mine = m_Finale.RouteOfPlayer(playerId);
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (m_S.gateDeal || !m_Finale.IsBatchReady() && m_Finale.Finisher() < 0)
		{
			m_GM.NotifyPlayer(playerId, title, "A deal at the gate? Not now. Only while a batch is in the hold.");
			return;
		}

		if (!me || !m_GM.HasLab() || RZ_Util.DistXZ(me.GetOrigin(), m_GM.GetLabPosition()) > 200 || mine == RZ_CureFinale.NONE)
		{
			m_GM.NotifyPlayer(playerId, title, "At the lab gate. Both of you. In person.");
			return;
		}

		string key = string.Format("%1|%2", mine, Ident(playerId));
		if (!m_S.dealIdents.Contains(key))
			m_S.dealIdents.Insert(key);

		int holder = m_Finale.Holder();
		int partner = m_Finale.PactOf(holder);
		bool holderIn = false;
		bool partnerIn = false;
		foreach (string d : m_S.dealIdents)
		{
			if (d.StartsWith(holder.ToString() + "|"))
				holderIn = true;

			if (partner >= 0 && d.StartsWith(partner.ToString() + "|"))
				partnerIn = true;
		}

		if (!holderIn || !partnerIn)
		{
			m_GM.NotifyPlayer(playerId, title, "Your word's down. Now theirs.");
			return;
		}

		m_S.gateDeal = true;
		RZ_WorldFacts.Get().SetFlag("finale.gate_deal", 1);
		m_GM.NotifyPlayer(playerId, title, "Two names on one sheet at the lab gate. The duty officer witnesses it.");
	}

	//------------------------------------------------------------------------------------------------
	//! The deal at the gate was made in person (the controller's deal reads it).
	bool GateDealMade()
	{
		return m_S.gateDeal;
	}

	// ================================================================ The moves
	//------------------------------------------------------------------------------------------------
	//! Drain at a container: a rival locked player standing beside a route item on the ground (3 to 6
	//! m, not on it) for 20 s spoils it, once per container. Seen (another route's player within 40
	//! m): +5 Infamy with its route's faction, +15 if that route's own.
	protected void Drains(float step, array<int> ids, array<IEntity> players)
	{
		RZ_FinaleItems items = m_Finale.GetItems();
		array<RZ_FinaleItemSave> lying = {};
		items.Lying(lying);
		foreach (RZ_FinaleItemSave it : lying)
		{
			if (m_S.drained.Contains(it.id.ToString()))
				continue;

			vector at = Vector(it.x, 0, it.z);
			foreach (int i, IEntity p : players)
			{
				int theirs = m_Finale.RouteOfPlayer(ids[i]);
				float d = RZ_Util.DistXZ(p.GetOrigin(), at);
				if (theirs == RZ_CureFinale.NONE || theirs == it.route || d > DRAIN_NEAR)
					continue;

				string key = string.Format("%1|%2", it.id, ids[i]);
				float had = 0;
				m_mDrain.Find(key, had);
				had += step;
				m_mDrain.Set(key, had);
				if (had < DRAIN_SECONDS)
					continue;

				m_S.drained.Insert(it.id.ToString());
				items.Drain(it.id);
				DrainSeen(ids[i], it.route, at, ids, players);
				m_GM.NotifyPlayer(ids[i], RZ_CureFinale.RouteTitle(it.route), string.Format("Twenty seconds with the lid off. %1 is no good to anyone now.", it.name));
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DrainSeen(int drainer, int route, vector at, array<int> ids, array<IEntity> players)
	{
		int f = RZ_CureFinale.RouteFaction(route);
		if (!RZ_Factions.IsValid(f))
			return;

		int infamy = 0;
		foreach (int i, IEntity p : players)
		{
			int theirs = m_Finale.RouteOfPlayer(ids[i]);
			if (ids[i] == drainer || theirs == RZ_CureFinale.NONE || RZ_Util.DistXZ(p.GetOrigin(), at) > 40)
				continue;

			if (theirs == route)
				infamy = 15;
			else if (infamy == 0)
				infamy = 5;
		}

		if (infamy > 0)
		{
			m_GM.AddInfamy(drainer, f, infamy, "seen draining a route's container", 0, "drained a container on the road");
			m_Finale.Hostile(m_Finale.RouteOfPlayer(drainer), route);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Guard the door: the door's own route moves it once a season (the clipboard, 114, the trays,
	//! his boy's line).
	bool GuardDoor(int route)
	{
		if (m_S.guarded.Contains(route))
			return false;

		m_S.guarded.Insert(route);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool Guarded(int route)
	{
		return m_S.guarded.Contains(route);
	}

	//------------------------------------------------------------------------------------------------
	//! A rival move at a guarded door ("" if allowed): talk needs a pact; a sneak or a trick there is
	//! one step harder (half of them fail).
	string GuardedRefusal(int route, int playerId, bool sneakOrTrick)
	{
		if (!Guarded(route))
			return "";

		int mine = m_Finale.RouteOfPlayer(playerId);
		if (!sneakOrTrick)
		{
			if (m_Finale.PactOf(route) != mine)
				return "The door's being watched. Without a pact, nobody's talking to you here.";

			return "";
		}

		if (Math.RandomFloat01() < 0.5)
			return "Somebody's on the door tonight, and awake. Not this time.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Last Mile's Keep It Overnight: a route item kept in the cold from dusk to dawn by its carrier
	//! gains back an hour of its cold clock (once an item).
	protected void Overnight()
	{
		if (!RZ_GameTime.IsCalibrated())
			return;

		float hour = RZ_GameTime.GetHour();
		if (hour < 5 || hour >= 6)
			return;

		m_Finale.GetItems().Overnight(m_S.overnight);
	}

	// ================================================================ The stand's checkpoint
	//------------------------------------------------------------------------------------------------
	//! Under Bastion, Command (orders.stance 3), during the stand: Bastion's checkpoint on the lab's
	//! main road. A rival route's carrier within 30 m is searched: passing as Bastion is a look; else
	//! a 90 s wait while the item's cold clock runs. Leaving early: +10 Bastion Infamy.
	protected void Checkpoint(float step, array<int> ids, array<IEntity> players)
	{
		// Bastion, Command; or the Red Hand's book in Bastion's hands (book.last 5): Bastion stops the cook
		bool command = RZ_MajorHelp.World(m_GM, "orders.stance") == 3;
		bool cook = RZ_MajorHelp.World(m_GM, "book.last") == 5;
		if (!m_Finale.IsStandOn() || (!command && !cook) || !m_GM.HasLab())
			return;

		vector post = CheckpointPos();
		RZ_Hostility hostility = m_GM.GetHostility();
		foreach (int i, IEntity p : players)
		{
			string ident = Ident(ids[i]);
			int theirs = m_Finale.RouteOfPlayer(ids[i]);
			if (theirs == RZ_CureFinale.NONE || theirs == RZ_CureFinale.BASTION || m_S.checked.Contains(ident) || m_Finale.GetItems().CarriesRoute(ident, theirs) <= 0)
				continue;

			if (!command && theirs != RZ_CureFinale.REDHAND)
				continue;

			float d = RZ_Util.DistXZ(p.GetOrigin(), post);
			bool isHeld = m_mHeld.Contains(ident);
			float held = 0;
			if (isHeld)
				held = m_mHeld.Get(ident);

			if (!isHeld)
			{
				if (d > CHECKPOINT_RADIUS)
					continue;

				if (hostility && hostility.GetDisguise() && hostility.GetDisguise().IsPassing(ids[i], RZ_EFaction.MILITARY))
				{
					m_S.checked.Insert(ident);
					m_GM.NotifyPlayer(ids[i], "Bastion's checkpoint", "\"Go on through.\" A Bastion look and a nod. Nobody opens anything.");
					continue;
				}

				m_mHeld.Set(ident, 0);
				m_GM.NotifyPlayer(ids[i], "Bastion's checkpoint", "\"Search. Ninety seconds. Stand there.\" Whatever you carry stays out of the cold while they look.");
				continue;
			}

			if (d > CHECKPOINT_RADIUS * 2)
			{
				m_mHeld.Remove(ident);
				m_S.checked.Insert(ident);
				m_GM.AddInfamy(ids[i], RZ_EFaction.MILITARY, 10, "ran Bastion's checkpoint", 0, "ran the checkpoint on the lab road");
				continue;
			}

			held += step;
			m_mHeld.Set(ident, held);
			if (held >= CHECKPOINT_WAIT)
			{
				m_mHeld.Remove(ident);
				m_S.checked.Insert(ident);
				m_GM.NotifyPlayer(ids[i], "Bastion's checkpoint", "\"Logged. Go on.\" Ninety seconds you won't get back.");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! On the lab's main road, 200 m out.
	vector CheckpointPos()
	{
		vector lab = m_GM.GetLabPosition();
		return RZ_Mission.LandPointNear(lab + Vector(0, 0, 200), 0, 20);
	}

	// ================================================================ Live tissue
	//------------------------------------------------------------------------------------------------
	//! A pack may give tissue now: within reach of the lab (5 km by road; the straight line x 1.3
	//! stands in), and not within 6 season days of the last time it gave.
	bool TissueFrom(RZ_Tribe t)
	{
		if (!t || !m_GM.HasLab())
			return false;

		if (RZ_Util.DistXZ(t.m_vLair, m_GM.GetLabPosition()) * 1.3 > TISSUE_REACH)
			return false;

		int day = RZ_SeasonClock.Day();
		string head = t.m_iId.ToString() + "|";
		foreach (string g : m_S.tissueTribes)
		{
			if (g.StartsWith(head) && day - g.Substring(head.Length(), g.Length() - head.Length()).ToInt() < TISSUE_DAYS)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void TissueGiven(RZ_Tribe t)
	{
		if (t)
			m_S.tissueTribes.Insert(string.Format("%1|%2", t.m_iId, RZ_SeasonClock.Day()));
	}

	//------------------------------------------------------------------------------------------------
	//! The unsided deliverer chooses: their best standing among the routes that still need it.
	int UnsidedChoice(int playerId, int offeredFor)
	{
		int best = offeredFor;
		int bestFame = -1;
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		for (int r = 0; r < RZ_CureFinale.INDEP; r++)
		{
			if (m_Finale.TissueState(r) != 0)
				continue;

			int fame = p.GetFame(r);
			if (fame > bestFame)
			{
				bestFame = fame;
				best = r;
			}
		}

		return best;
	}

	// ================================================================ The majors' inputs
	//------------------------------------------------------------------------------------------------
	//! Each page in the open makes one finale check one step easier, once: a page not yet used.
	bool PageLeft()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		return facts.Flag("majors.pages_open") > facts.Flag("finale.pages_used");
	}

	//------------------------------------------------------------------------------------------------
	void UsePage()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		facts.SetFlag("finale.pages_used", facts.Flag("finale.pages_used") + 1);
	}

	// ================================================================ Each route's remaining ways
	//------------------------------------------------------------------------------------------------
	//! More choices on a route's stage scene (RZ_FinaleRoute.BuildScene): the ways the first pass
	//! left out. Codes "do:rest:<code>" come back to CanDo and DoIt below.
	void MoreChoices(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene, int route)
	{
		// Every route: guard the door, once a season (its own locked players: the door is sworn work)
		if (stage == RZ_FinaleRoute.S_DOOR && !Guarded(route))
		{
			int door = scene.Person(RZ_Mission.LandPointNear(scene.m_vPos, 6, 12), "the door, after dark", "Look at the door", "Whoever holds this door could move it once. After that, everybody knows where it went.");
			scene.Choice(door, "Guard the door [once a season]").Sets("do:rest:guard").Says("The door moves tonight: the clipboard, the key, the trays or the line, somewhere new. Sneaks and tricks get harder here.", "guarded the route's door");
		}

		switch (route)
		{
			case RZ_CureFinale.HAVEN: HavenWays(gm, run, stage, scene); break;
			case RZ_CureFinale.BASTION: BastionWays(gm, run, stage, scene); break;
			case RZ_CureFinale.REDHAND: RedHandWays(gm, run, stage, scene); break;
			case RZ_CureFinale.HALCYON: HalcyonWays(gm, run, stage, scene); break;
			case RZ_CureFinale.INDEP: IndependentWays(gm, run, stage, scene); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven: his four terms one by one (his name read at supper costs Haven morale 5), moving him
	//! once a real day, the hungry house selling the spot, the annex's centrifuge, the quiet draw,
	//! "One more", the truck's cab, betraying him on the last mile.
	protected void HavenWays(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		RZ_FinalePeople people = m_Finale.GetPeople();
		if (!people.IsAlive("woman"))
			return;

		vector house = people.Pos("woman");
		if (stage == RZ_FinaleRoute.S_DOOR)
		{
			int terms = scene.Person(RZ_Mission.LandPointNear(house, 3, 6), "him, at the table", "Sit with him", "One at a time. Say each one and mean it. Leave the gun twenty metres off.");
			scene.Choice(terms, "Promise: never up that hill").Sets("term1").Says("\"Not up the hill. Good.\" He ticks a finger.", "promised him the first term");
			scene.Choice(terms, "Promise: nobody first for guns").Only("term1").Sets("term2").Says("\"Nobody first because they're armed.\" Two fingers.", "promised him the second term");
			scene.Choice(terms, "Promise his name read at supper").Only("term2").Sets("do:rest:name").Says("Out loud. At the table. Where I can hear it.", "promised his name at supper");
			scene.Choice(terms, "Promise his boy goes free").Only("term3").Sets("do:rest:yes").Says("\"And my boy.\" He holds your eye. \"Then yes.\"", "kept all four of his terms");
			int chair = scene.Person(RZ_MajorHelp.NearSeat(SeatOf(RZ_CureFinale.HAVEN)), "the chair's runner", "Talk to the runner", "He moves at dusk. Never the same two nights. A locked hand can move him once a day.");
			scene.Choice(chair, "Move his tonight [the Haven's route]").Sets("do:rest:move").Says("A different house tonight, a different bowl. Whoever was watching the last one can keep watching it.", "moved him to another house");
			int hungry = scene.Person(RZ_Mission.LandPointNear(house, 30, 60), "the hungry house next door", "Talk to them", "We're short. We know which house has his tonight. Everyone's short.");
			// Under the Far House (haven.rule 2) his house is known and sold more readily: one food
			int spotPrice = 2;
			if (RZ_MajorHelp.World(m_GM, "haven.rule") == 2)
				spotPrice = 1;

			scene.Choice(hungry, string.Format("Buy tonight's spot [%1 food]", spotPrice)).Way(RZ_EQuestApproach.BETRAY).Trade(RZ_EErrandItem.FOOD, spotPrice).Sets("do:rest:spot").Says("They tell you which house. They don't look at you while they do.", "bought his spot from a hungry house");
		}
		else if (stage == RZ_FinaleRoute.S_PIECE)
		{
			int annex = scene.Person(RZ_Mission.LandPointNear(house, 40, 80), "the annex with the generator", "Look at the annex", "A generator, a pharmacy fridge, and somewhere to lie down. The vet's centrifuge is at the crossroads.");
			scene.Choice(annex, "Hire the vet's centrifuge [25 Favor]").Bribe(RZ_EFaction.BANDITS, 25).Sets("do:draw").Says("The centrifuge comes in a crew's van and leaves the same way. The draw goes cleaner.", "drew a bag at the annex");
			scene.Choice(annex, "Draw it quietly [unseen]").Sneak().Sets("do:draw").Says("No lamp, no queue at the door. One bag, and nobody saw it go.", "drew a bag quietly");
			scene.Choice(annex, "Ask him for one more").Only("spared").Persuade(RZ_EFaction.SURVIVORS, 3, "nurse,doctor").Sets("do:draw").Says("One more. Then that's all of me you get.", "asked him for one more").Refused("I said two. I meant two.");
		}
		else if (stage == RZ_FinaleRoute.S_ACT4)
		{
			int cab = scene.Person(RZ_Mission.LandPointNear(house, 20, 40), "a truck with its engine running", "Look at the truck", "A cab with room for one more. The roadblocks wave trucks through.");
			scene.Choice(cab, "Drive him up in the cab").Sets("walked").Says("He sits in the cab with his hands in his lap. The roadblocks wave you through.", "drove him up in a truck's cab");
			scene.Choice(cab, "Turn him over on the last mile").Way(RZ_EQuestApproach.BETRAY).Sets("do:rest:betray").Says("You stop short of the lab and somebody else's people take him the rest of the way. He doesn't fight.", "betrayed him on the last mile");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion: the four ledgers one by one, no names at all, the 20:30 call reading the release (the
	//! tick), Act IV's carries (the stage truck, the LZ under pilot.stays, on foot quietly, the box
	//! handed in at a checkpoint through Forgive()).
	protected void BastionWays(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector seat = SeatOf(RZ_CureFinale.BASTION);
		if (stage == RZ_FinaleRoute.S_DOOR)
		{
			int books = scene.Person(RZ_MajorHelp.NearSeat(seat), "the four ledgers", "Look at the ledgers", "The post's, the quartermaster's, the medical store's and the bus company's. Any three name the tubes.");
			scene.Choice(books, "Read the post's ledger").Sets("ledger1").Says("Day 2: a column of names and a bus number.", "read the post's ledger");
			scene.Choice(books, "Read the quartermaster's ledger").Only("ledger1").Sets("ledger2").Says("Diesel, twenty litres, stores. Every week since Day 2.", "read the quartermaster's ledger");
			scene.Choice(books, "Read the medical store's ledger").Only("ledger2").Decides(RZ_FinaleRoute.OUT_DOOR).Says("R1 to R4, a name beside each, in pencil. Three ledgers agree.", "named the tubes from three ledgers");
			scene.Choice(books, "Name nobody at all").Persuade(RZ_EFaction.MILITARY, 2, "soldier").Decides(RZ_FinaleRoute.OUT_OTHER).Says("\"No names. Numbers only.\" The captain signs for numbers.", "had the tubes released by number").Refused("The captain wants names.");
		}
		else if (stage == RZ_FinaleRoute.S_PIECE && RZ_MajorHelp.World(m_GM, "orders.stance") == 2)
		{
			// All Stations: by need at Known, and the captain one step easier
			int need = scene.Person(RZ_Mission.LandPointNear(scene.m_vPos, 6, 12), "the captain, after All Stations", "Talk to the captain", "Civilians first, the CO said on the net. Then this box is civilians'. Ask me properly.");
			scene.Choice(need, "Ask for the box, by need [Known]").Needs(RZ_EFaction.MILITARY, 1).Persuade(RZ_EFaction.MILITARY, 1, "nurse,doctor").Decides(RZ_FinaleRoute.OUT_PIECE).Says("By need. Fine. Sign here, and keep it cold.", "had the box released by need").Refused("Need isn't a word. Show me.");
		}
		else if (stage == RZ_FinaleRoute.S_ACT4)
		{
			int carry = scene.Person(RZ_Mission.LandPointNear(seat, 20, 40), "the box and its escort", "Look at the box", "The box goes to the lab cold. How it gets there is up to whoever carries it.");
			scene.Choice(carry, "Load it on the stage truck").Sets("carry").Says("The stage truck, two road posts' worth of escort, and the box strapped down in the back.", "loaded the box on the stage truck");
			scene.Choice(carry, "Fly it from the LZ").Only("world:pilot.stays").Sets("carry").Says("The pilot who stayed takes it up off the pad. Twelve minutes in the air.", "flew the box from the LZ");
			scene.Choice(carry, "Walk it in quietly [unseen]").Sneak().Sets("carry").Says("On foot, off the roads, in the cold bag. Nobody sees a box.", "walked the box in quietly");
			scene.Choice(carry, "Hand it in at a checkpoint").Sets("do:rest:checkpoint").Says("The checkpoint signs for the box and takes it the rest of the way. Bastion writes your name down kindly.", "handed the box in at a checkpoint");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand: Who Came Back in full (lines bought at the Haven and the lab, a false answer +10
	//! Red Hand Infamy), telling the boss someone's asking, the Mechanic's coil, Halcyon copying the
	//! cook's notes (cook.lab_copy).
	protected void RedHandWays(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		if (stage == RZ_FinaleRoute.S_ACT1)
		{
			int haven = scene.Person(RZ_MajorHelp.NearSeat(SeatOf(RZ_CureFinale.HAVEN)), "a Haven cook who knew him", "Talk to the cook", "Who came back from his pan? I'll tell you for a price. Everyone's got one this week.");
			scene.Choice(haven, "Buy his line at the Haven [10 Favor]").Bribe(RZ_EFaction.SURVIVORS, 10).Sets("line_h").Says("Four came back. One didn't, and he knows his name.", "bought a line at the Haven");
			int lab = scene.Person(RZ_MajorHelp.NearSeat(SeatOf(RZ_CureFinale.HALCYON)), "a lab orderly", "Talk to the orderly", "The cook's patients? We logged them. For a price.");
			scene.Choice(lab, "Buy the lab's line [10 Favor]").Only("line_h").Bribe(RZ_EFaction.INSTITUTE, 10).Decides(RZ_FinaleRoute.OUT_ACT1).Says("Three lines that agree: who came back, and from which pan.", "had Who Came Back from three lines");
			scene.Choice(lab, "Make up who came back").Way(RZ_EQuestApproach.BETRAY).Sets("do:rest:false").Says("You give the boss a list. Some of it might even be true.", "gave the boss a false answer");
			int boss = scene.Person(RZ_MajorHelp.NearSeat(SeatOf(RZ_CureFinale.REDHAND)), "the boss's man", "Talk to the boss's man", "Somebody's asking about the cook. The boss likes to know who.");
			scene.Choice(boss, "Tell the boss who's asking [Known]").Needs(RZ_EFaction.BANDITS, 1).Sets("told_boss").Says("Good. The boss remembers who tells him things.", "told the boss someone was asking");
		}
		else if (stage == RZ_FinaleRoute.S_PIECE)
		{
			int still = scene.Person(RZ_Mission.LandPointNear(scene.m_vPos, 10, 20), "the still's coil", "Look at the coil", "The coil is burnt through. A Mechanic could fix it in the dark.");
			scene.Choice(still, "Fix the coil in the dark").Persuade(RZ_EFaction.BANDITS, 2, "mechanic").Decides(RZ_FinaleRoute.OUT_PIECE).Says("New copper, bent by hand. The still runs all night without a cough.", "fixed the still's coil").Refused("It needs hands that know engines.");
			scene.Choice(still, "Copy his notes for the lab [unseen]").Sneak().Sets("world:cook.lab_copy").Says("His chalk on the boards, copied into a notebook. Halcyon will want it.", "copied the cook's notes for the lab");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon: the canisters' riddle (the bracelet and the date; a wrong canister costs a vial), the
	//! Haven trading silence about the Day 3 page, Act IV's ways down, the family at dusk.
	protected void HalcyonWays(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		if (stage == RZ_FinaleRoute.S_ACT1)
		{
			int haven = scene.Person(RZ_MajorHelp.NearSeat(SeatOf(RZ_CureFinale.HAVEN)), "a council head who read the Day 3 page", "Talk to the council head", "We read the Day 3 page. We could forget we did. For the right price.");
			scene.Choice(haven, "Trade the Haven's silence [Known]").Needs(RZ_EFaction.SURVIVORS, 1).Sets("world:ref.haven_silence").Says("\"Forgotten. For now.\" The council head doesn't smile.", "traded for the Haven's silence");
		}
		else if (stage == RZ_FinaleRoute.S_DOOR)
		{
			// Expended: a Rogue commander sells 114; Under Seal: 114 at Known with Bastion
			if (RZ_MajorHelp.World(m_GM, "orders.stance") == 4)
			{
				int rogue = scene.Person(RZ_Mission.LandPointNear(SeatOf(RZ_CureFinale.BASTION), 60, 120), "a commander who stopped answering the net", "Talk to the commander", "Expended, they said. Then I'm selling. Item one-one-four, thirty.");
				scene.Choice(rogue, "Buy 114 off the commander [30 Favor]").Bribe(RZ_EFaction.MILITARY, 30).Decides(RZ_FinaleRoute.OUT_DOOR).Says("The key comes out of a sock. \"Don't tell me what it opens.\"", "bought item 114 off a Rogue commander");
			}

			if (RZ_MajorHelp.World(m_GM, "proof.first_page") == 4)
			{
				int seal = scene.Person(RZ_MajorHelp.NearSeat(SeatOf(RZ_CureFinale.BASTION)), "the adjutant, under seal", "Talk to the adjutant", "Under seal means under our seal. One-one-four goes to people we know.");
				scene.Choice(seal, "Ask for 114 under seal [Bastion: Known]").Needs(RZ_EFaction.MILITARY, 1).Decides(RZ_FinaleRoute.OUT_DOOR).Says("The adjutant signs it out to you and keeps the carbon.", "had item 114 under seal");
			}
		}
		else if (stage == RZ_FinaleRoute.S_PIECE)
		{
			int bracelet = scene.Person(RZ_Mission.LandPointNear(scene.m_vPos, 4, 8), "the four canisters", "Look at the canisters", "Four canisters, no labels. A hospital bracelet taped to the dewar, with a date in biro.");
			scene.Choice(bracelet, "Read the bracelet's date").Sets("bracelet").Says("Day 3, 04:10. One canister has the same time scratched on its lid.", "read the bracelet's date");
			scene.Choice(bracelet, "Open the canister with the time").Only("bracelet").Decides(RZ_FinaleRoute.OUT_PIECE).Says("The right one. The vials are where the bracelet said.", "opened the right canister");
			scene.Choice(bracelet, "Open a canister at random").Sets("do:rest:wrong").Says("Wrong canister. The cold goes out of it in a breath, and a vial with it.", "opened the wrong canister");
		}
		else if (stage == RZ_FinaleRoute.S_ACT4)
		{
			int down = scene.Person(RZ_Mission.LandPointNear(scene.m_vPos, 30, 60), "the road down from the site", "Look at the road", "Rival routes know roughly where the site is. Not which way you'll come down.");
			scene.Choice(down, "Walk it down by night [unseen]").Sneak().Sets("down").Says("Down by night, no lights, the cold bag on your back.", "walked the reference down by night");
			scene.Choice(down, "Send a decoy down first").Sets("decoy").Says("A second cold bag goes down the main road first, with a lot of noise.", "sent a decoy down first");
			scene.Choice(down, "Take the Haven's road [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Sets("down").Says("Through the houses' back gardens, a lamp in each window to show the way.", "took the Haven's road down");
			scene.Choice(down, "Ask the family to hold it at dusk").Sets("world:ref.haven_holds").Says("A family at the edge of town keeps it in their cellar until dusk. They don't ask what it is.", "had a family hold the reference at dusk");
			// The Whole of It lends the transport case: the carry keeps cold the whole way
			if (RZ_MajorHelp.World(m_GM, "proof.first_page") == 2)
				scene.Choice(scene.Person(RZ_MajorHelp.NearSeat(m_GM.GetLabPosition()), "the paper's transport case", "Look at the case", "Latched, insulated, lent for an hour by the people who will carry the paper out."), "Carry it in the paper's case").Sets("down").Says("The transport case the paper will go out in, lent for an hour. Cold the whole way.", "carried the reference in the transport case");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Independent: the carry on air (the Doctor's broadcast) or quiet, and the duty officer's vouch.
	protected void IndependentWays(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		if (stage != RZ_FinaleRoute.S_ACT4)
			return;

		int air = scene.Person(RZ_Mission.LandPointNear(scene.m_vPos, 20, 40), "the Doctor with a handset", "Talk to the Doctor", "If they hear it on air, nobody can say it was anyone's. Or we do it quietly and hope.");
		scene.Choice(air, "Have the Doctor read it on air").Sets("do:rest:onair").Says("\"This is the Doctor. Released to: nobody. That means you.\" Every channel at once.", "had the release read on air");
		scene.Choice(air, "Carry it quietly [unseen]").Sneak().Sets("quiet").Says("Nobody hears. Nobody comes. That's the idea.", "carried it in quietly");
		scene.Choice(air, "Ask the duty officer to vouch").Persuade(RZ_EFaction.INSTITUTE, 2, "labtech").Sets("vouched").Says("I'll say it came in the front door. Because it did.", "had the duty officer vouch for the carry").Refused("Vouch for nobody? I'd rather not.");
	}

	//------------------------------------------------------------------------------------------------
	//! A rest code's checks ("" if it can happen now).
	string CanDo(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int route, notnull array<string> parts)
	{
		if (parts.Count() < 2)
			return "";

		string code = parts[1];
		int mine = m_Finale.RouteOfPlayer(playerId);
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		switch (code)
		{
			case "guard":
			{
				if (mine != route)
					return "Only the route that holds the door moves it.";

				if (Guarded(route))
					return "It's been moved once. Everyone knows where now.";

				break;
			}
			case "yes":
			{
				if (me && RZ_Util.IsArmed(me))
					return "Twenty metres off. The gun.";

				break;
			}
			case "move":
			{
				if (mine != RZ_CureFinale.HAVEN)
					return "Only the Haven's people move him.";

				if (RZ_SeasonClock.Now() - run.Flag("moved_unix") < DAY * m_GM.GetQuestlines().WaitScale())
					return "He's moved once today. Twice and the houses talk.";

				break;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A rest code's effect, once its checks passed. An outcome, or -1.
	int DoIt(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int route, notnull array<string> parts)
	{
		if (parts.Count() < 2)
			return -1;

		string code = parts[1];
		RZ_FinalePeople people = m_Finale.GetPeople();
		switch (code)
		{
			case "guard":
			{
				GuardDoor(route);
				RZ_Log.Info(string.Format("Finale: %1 guarded its door.", RZ_CureFinale.RouteName(route)));
				break;
			}
			case "name":
			{
				run.SetFlag("term3", 1);
				RZ_Majors.MoraleThroughBudget(RZ_EFaction.SURVIVORS, -5);
				break;
			}
			case "yes":
			{
				RZ_FinaleRoute line = RZ_FinaleRoute.Cast(run.m_Line);
				if (line)
					return line.DoIt(gm, run, playerId, "yes");

				break;
			}
			case "move":
			{
				run.SetFlag("moved_unix", RZ_SeasonClock.Now());
				people.MoveWoman();
				break;
			}
			case "spot":
			{
				int buyer = m_Finale.RouteOfPlayer(playerId);
				if (buyer != RZ_CureFinale.NONE && buyer != RZ_CureFinale.HAVEN)
					m_Finale.Hostile(buyer, RZ_CureFinale.HAVEN);

				Noted(playerId, RZ_CureFinale.HAVEN);
				break;
			}
			case "betray":
			{
				people.Betray("woman");
				m_GM.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 20, "handed him over on the last mile", 0, "betrayed him on the last mile");
				break;
			}
			case "checkpoint":
			{
				int off = m_GM.Forgive(playerId, RZ_EFaction.MILITARY, Math.Min(10, m_GM.GetProfile(playerId).GetInfamy(RZ_EFaction.MILITARY)), "handed the box in at a checkpoint");
				run.SetFlag("carry", 1);
				if (off > 0)
					m_GM.NotifyPlayer(playerId, RZ_CureFinale.RouteTitle(route), string.Format("Bastion takes %1 Infamy off your name for it.", off));

				break;
			}
			case "false":
			{
				m_GM.AddInfamy(playerId, RZ_EFaction.BANDITS, 10, "gave the boss a false answer", 0, "lied to the boss about who came back");
				break;
			}
			case "wrong":
			{
				run.SetFlag("vials_lost", run.Flag("vials_lost") + 1);
				break;
			}
			case "onair":
			{
				RZ_FactionLife life = m_GM.GetFactionLife();
				for (int f = 0; life && f < RZ_Factions.COUNT; f++)
				{
					life.OpenChannelLine(f, "This is the Doctor. The batch sheet says: released to nobody. That means you. Come to the lab gate.");
				}

				run.SetFlag("onair", 1);
				break;
			}
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's 20:30 call reads the release once its terms are written (the controller's route).
	void ReleaseCall()
	{
		if (m_Finale.Controller() != RZ_CureFinale.BASTION || m_Finale.Terms() <= 0 || !m_GM.GetFactionLife() || !RZ_GameTime.IsCalibrated())
			return;

		float hour = RZ_GameTime.GetHour();
		if (hour < 20.5 || hour >= 21 || RZ_WorldFacts.Get().Flag("finale.release_called") != 0)
			return;

		RZ_WorldFacts.Get().SetFlag("finale.release_called", 1);
		m_GM.GetFactionLife().OpenChannelLine(RZ_EFaction.MILITARY, "Command, Bastion. Stores held for collection: released. Minus nineteen. Receipt held. Over.");
	}
}
