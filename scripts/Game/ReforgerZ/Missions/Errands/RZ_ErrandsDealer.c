//! The dealer's rest (docs/ERRANDS.md, "The dealer" and "Locals"): a context cache per player,
//! refreshed every 30 s, that the score, the call-outs and the admin view read; freshness and nearness
//! in the score; locals coming to you near their house; locals shown only near someone who knows them;
//! a local's spot as a place; and "the last one who said that lied". Server only.

//------------------------------------------------------------------------------------------------
//! What the dealer knows about one player right now (30 s old at most).
class RZ_ErrandContext
{
	float m_fAt;				// the dealer's clock when it was made
	vector m_vPos;
	int m_iSettlement = -1;		// the nearest settlement within its radius + 100 m
	int m_iOwner = -1;			// who holds that town (RZ_EFaction), -1 nobody
	int m_iHolding = -1;		// a holding within 600 m (its id), -1 none
	int m_iHoldingFaction = -1;
	bool m_bPackNear;			// inside a pack's range within 1.5 km
	bool m_bRoad;				// on a road people use (the footfall)
	bool m_bVehicle;
	bool m_bFight;				// a shot within 60 m in the last minute
	bool m_bNearJob;			// within 300 m of a running mission's spot
	float m_fLifeMinutes;
	ref array<int> m_aTiers = {};	// RZ_ETier per faction
	int m_iRoute = -1;			// the finale route they're locked to, -1 none
}

//------------------------------------------------------------------------------------------------
class RZ_ErrandsDealer
{
	static const float REFRESH = 30;
	static const float KNOWER_RANGE = 100;
	static const float VISIT_MIN = 80;			// a local walks up to you 80-250 m from their house
	static const float VISIT_MAX = 250;
	static const int VISIT_GAP = 86400;			// once a real day per local per player
	static const int LIED_SECONDS = 86400;

	protected RZ_Errands m_Dealer;
	protected ref map<int, ref RZ_ErrandContext> m_mContext = new map<int, ref RZ_ErrandContext>();
	protected ref map<int, bool> m_mShown = new map<int, bool>();		// local id -> tagged now
	protected ref map<string, int> m_mVisited = new map<string, int>();	// "ident|local" -> unix
	protected IEntity m_Visitor;
	protected float m_fVisitorAge;
	protected float m_fTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_ErrandsDealer(RZ_Errands dealer)
	{
		m_Dealer = dealer;
	}

	//------------------------------------------------------------------------------------------------
	//! On the errands' tick: every 30 s the cache, locals' labels and a visit at most.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (m_Visitor)
		{
			m_fVisitorAge += dt;
			if (RZ_DoorCallers.GoneUnseen(m_Visitor, m_fVisitorAge, RZ_DoorCallers.STAY_SECONDS))
			{
				RZ_Util.DeleteUnit(m_Visitor);
				m_Visitor = null;
			}
		}

		m_fTimer += dt;
		if (m_fTimer < REFRESH)
			return;

		m_fTimer = 0;
		m_mContext.Clear();
		foreach (int i, IEntity p : players)
		{
			m_mContext.Set(ids[i], Make(ids[i], p));
		}

		ShowLocals(ids, players);
		Visit(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! The player's context (made now if the cache has none).
	RZ_ErrandContext Context(int playerId)
	{
		RZ_ErrandContext c;
		if (m_mContext.Find(playerId, c))
			return c;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me)
			return null;

		c = Make(playerId, me);
		m_mContext.Set(playerId, c);
		return c;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_ErrandContext Make(int playerId, IEntity me)
	{
		RZ_GameModeComponent gm = m_Dealer.GetGM();
		RZ_ErrandContext c = new RZ_ErrandContext();
		vector pos = me.GetOrigin();
		c.m_vPos = pos;
		int town = gm.FindNearestSettlement(pos);
		RZ_Settlement s = gm.GetSettlement(town);
		if (s && RZ_Util.DistXZ(s.m_vCenter, pos) <= s.m_fRadius + 100)
		{
			c.m_iSettlement = town;
			if (gm.GetFacts())
				c.m_iOwner = gm.GetFacts().TownOwner(town).m_iFaction;
		}

		RZ_Territory territory = gm.GetTerritory();
		if (territory && territory.IsActive())
		{
			for (int f = 0; f < 4; f++)
			{
				RZ_Holding h = territory.FindHolding(f, pos, 600);
				if (h)
				{
					c.m_iHolding = h.m_iId;
					c.m_iHoldingFaction = f;
					break;
				}
			}
		}

		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		c.m_bPackNear = tribes && tribes.NearestTribe(pos, 1500) != null;
		c.m_bRoad = m_Dealer.GetStrangers().Footfall(pos) >= 20;
		c.m_bVehicle = RZ_Util.IsInVehicle(me);
		c.m_bFight = m_Dealer.RecentShot(pos, 60, 60);
		c.m_bNearJob = gm.GetMissions() && !m_Dealer.QuietHere(pos);
		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		c.m_fLifeMinutes = profile.m_fLifeSeconds / 60;
		for (int t = 0; t < 4; t++)
		{
			c.m_aTiers.Insert(profile.GetTier(t));
		}

		if (gm.GetFinale())
			c.m_iRoute = gm.GetFinale().RouteOfPlayer(playerId);

		c.m_fAt = m_Dealer.GetClock();
		return c;
	}

	//------------------------------------------------------------------------------------------------
	//! Nearness and freshness (x the score): the errand's own place near you now x1.2, far x0.85;
	//! something fresh at its subject (a death, a bite, a fire, shells in the last 2 played hours) x1.2.
	float Factor(RZ_Errand e)
	{
		float mult = 1;
		RZ_ErrandContext c = Context(e.m_iAsker);
		vector place = e.m_vSubject;
		if (place == vector.Zero)
			place = e.m_vGiver;

		if (c)
		{
			float d = RZ_Util.DistXZ(c.m_vPos, place);
			if (d <= 600)
				mult *= 1.2;
			else if (d > 1200)
				mult *= 0.85;
		}

		if (e.m_vSubject != vector.Zero && RZ_WorldFacts.Get())
		{
			array<RZ_Signal> fresh = {};
			float days = 2 * 60 / Math.Max(1, RZ_SeasonClock.DayMinutes());
			RZ_WorldFacts.Get().Memory().RecentSignals("", e.m_vSubject, 400, days, fresh);
			if (!fresh.IsEmpty())
				mult *= 1.2;
		}

		// A margin card answers its major's act: x1.3 while the act's reason is posted
		if (MajorReason(e) != null)
			mult *= 1.3;

		return mult;
	}

	//------------------------------------------------------------------------------------------------
	//! The major a margin card belongs to ("" none).
	static string MajorOf(string cardKey)
	{
		if (cardKey == "weather_write_it_down" || cardKey == "clean_rifles")
			return "orders";

		if (cardKey == "clean_paper" || cardKey == "chairs_for_the_reading")
			return "proof";

		if (cardKey == "count_sitting")
			return "every_name";

		if (cardKey == "clean_glasses")
			return "book";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The posted `major.<key>` reason a margin card answers, null if none.
	static RZ_Reason MajorReason(RZ_Errand e)
	{
		string major = MajorOf(e.m_Def.m_sKey);
		if (major.IsEmpty() || !RZ_WorldFacts.Get())
			return null;

		return RZ_WorldFacts.Get().Memory().FindReason("major." + major);
	}

	//------------------------------------------------------------------------------------------------
	//! Dealt: a margin card answering its major's act opens with the act's line.
	void OpenWithReason(RZ_Errand e)
	{
		RZ_Reason r = MajorReason(e);
		if (r)
			m_Dealer.GetGM().NotifyPlayerAs(e.m_iAsker, e.m_sGiverTitle, r.text, RZ_ENoticeTag.MISSION);
	}

	// ================================================================ "The last one who said that lied"
	//------------------------------------------------------------------------------------------------
	//! A lie found out on a dead life: that giver kind's next errand for this identity opens with the
	//! line, its fit x0.8 for a real day.
	void LiedOnDeadLife(string ident, int faction)
	{
		RZ_ErrandLedger ledger = m_Dealer.Ledger(ident);
		ledger.liedFaction = faction;
		ledger.liedUntil = RZ_SeasonClock.Now() + LIED_SECONDS;
		ledger.liedSaid = false;
		RZ_Log.Info(string.Format("Errands: %1's dead life lied to %2", ident, RZ_Factions.GetName(faction)));
	}

	//------------------------------------------------------------------------------------------------
	//! x0.8 while that's fresh, for errands from the faction lied to.
	static float LiedFactor(RZ_ErrandLedger ledger, int faction)
	{
		if (ledger.liedUntil > RZ_SeasonClock.Now() && ledger.liedFaction == faction)
			return 0.8;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Dealt: the giver opens with it, once.
	void OpenWithLie(RZ_Errand e)
	{
		RZ_ErrandLedger ledger = m_Dealer.Ledger(e.m_sIdent);
		if (ledger.liedSaid || ledger.liedUntil <= RZ_SeasonClock.Now() || ledger.liedFaction != e.m_iFaction)
			return;

		ledger.liedSaid = true;
		m_Dealer.GetGM().NotifyPlayerAs(e.m_iAsker, e.m_sGiverTitle, "The last one who said that lied. You've the same look. Go on, then.", RZ_ENoticeTag.MISSION);
	}

	// ================================================================ Locals
	//------------------------------------------------------------------------------------------------
	//! A local is shown (Talk to, Ask how they are) only while someone who knows them is within 100 m;
	//! with nobody who does, the body is a plain occupant. VERIFY: per-player action labels (today a
	//! label is the same for everyone near).
	protected void ShowLocals(array<int> ids, array<IEntity> players)
	{
		RZ_GameModeComponent gm = m_Dealer.GetGM();
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return;

		for (int h = 0; h < houses.Count(); h++)
		{
			array<RZ_Local> here = {};
			m_Dealer.GetLocals().AtHouse(h, here);
			foreach (RZ_Local l : here)
			{
				IEntity body = m_Dealer.GetLocals().BodyOf(l.id);
				if (!body)
					continue;

				bool show = KnowerNear(gm, l, body.GetOrigin(), ids, players);
				bool was;
				if (m_mShown.Find(l.id, was) && was == show)
					continue;

				m_mShown.Set(l.id, show);
				if (show)
				{
					RZ_PuzzleObjectComponent.Tag(body, RZ_Locals.TAG, l.id, "", "Ask how they are");
					RZ_ClueComponent.Tag(body, RZ_Locals.TAG, l.id, "Talk to " + l.name);
				}
				else
				{
					RZ_PuzzleObjectComponent.Tag(body, RZ_Locals.TAG, l.id, "", "");
					RZ_ClueComponent.Tag(body, RZ_Locals.TAG, l.id, "");
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool KnowerNear(RZ_GameModeComponent gm, RZ_Local l, vector at, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), at) <= KNOWER_RANGE && l.idents.Contains(gm.GetProfile(ids[i]).m_sIdentity))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Locals coming to you: a local you helped walks up when you pass 80-250 m from their house (never
	//! in a fight, a car or near a job), once a real day each, one on the server at a time.
	protected void Visit(array<int> ids, array<IEntity> players)
	{
		RZ_GameModeComponent gm = m_Dealer.GetGM();
		RZ_Safehouses houses = gm.GetSafehouses();
		ResourceName prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (m_Visitor || !houses || prefab.IsEmpty())
			return;

		int now = RZ_SeasonClock.Now();
		foreach (int i, IEntity p : players)
		{
			RZ_ErrandContext c = Context(ids[i]);
			if (!c || c.m_bFight || c.m_bVehicle || c.m_bNearJob)
				continue;

			int idx = houses.FindNearest(p.GetOrigin(), VISIT_MAX);
			if (idx < 0 || RZ_Util.DistXZ(houses.Get(idx).m_vPos, p.GetOrigin()) < VISIT_MIN)
				continue;

			string ident = gm.GetProfile(ids[i]).m_sIdentity;
			array<RZ_Local> here = {};
			m_Dealer.GetLocals().AtHouse(idx, here);
			foreach (RZ_Local l : here)
			{
				string memo = m_Dealer.GetLocals().MemoOf(l, ident);
				if (memo != "helped" && memo != "saved")
					continue;

				string key = ident + "|" + l.id.ToString();
				int last;
				if (m_mVisited.Find(key, last) && now - last < VISIT_GAP)
					continue;

				// Out of sight 15-25 m off, inside the body budget (an errand's person); else not this time
				vector spot;
				RZ_Shared shared = RZ_Shared.Get();
				if (!RZ_InfectedBodies.FindHiddenSpot(p.GetOrigin(), RZ_DoorCallers.NEAR_MIN, RZ_DoorCallers.NEAR_MAX, players, spot))
					return;

				if (shared && !shared.CanSpawnBody(RZ_BodyBudget.ERRAND, spot))
					return;

				m_mVisited.Set(key, now);
				m_Visitor = RZ_Util.SpawnPrefab(prefab, spot, Math.RandomFloat(0, 360));
				m_fVisitorAge = 0;
				if (shared && m_Visitor)
					shared.AddBody(RZ_BodyBudget.ERRAND, m_Visitor);

				gm.NotifyPlayerAs(ids[i], string.Format("%1, %2", l.name, l.role), string.Format("It's me, %1, from the house. %2 We could use a hand there, if you've the time.", l.name, l.line), RZ_ENoticeTag.AUTO);
				RZ_Log.Info(string.Format("Errands: local %1 walked up to %2", l.name, RZ_Util.PlayerName(ids[i])));
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A local's spot as a place: at a house, a "here" step for someone who knows a local there is that
	//! local's spot at its hour (the gate at dusk, the stove at supper). False if none fits now.
	bool LocalSpot(RZ_Errand e, out vector at, out string name)
	{
		at = vector.Zero;
		name = "";
		if (e.m_iHouse < 0)
			return false;

		float hour = RZ_GameTime.GetHour();
		array<RZ_Local> here = {};
		m_Dealer.GetLocals().AtHouse(e.m_iHouse, here);
		foreach (RZ_Local l : here)
		{
			if (!l.idents.Contains(e.m_sIdent))
				continue;

			bool dusk = hour >= 17.5 && hour <= 21;
			bool supper = hour >= 18 && hour <= 20.5;
			if (l.spot == "gate" && dusk)
				at = RZ_Mission.LandPointNear(e.m_vGiver, 15, 25);
			else if ((l.spot == "stove" || l.spot == "bench") && supper)
				at = RZ_Mission.LandPointNear(e.m_vGiver, 2, 6);

			if (at != vector.Zero)
			{
				name = l.name;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The admin view's line for this player's context.
	string AdminLine(int playerId)
	{
		RZ_ErrandContext c = Context(playerId);
		if (!c)
			return "";

		return string.Format("context: town %1 (owner %2), holding %3, pack %4, road %5, car %6, fight %7, near a job %8, life %9 min",
			c.m_iSettlement, c.m_iOwner, c.m_iHolding, c.m_bPackNear, c.m_bRoad, c.m_bVehicle, c.m_bFight, c.m_bNearJob, RZ_Util.RoundToInt(c.m_fLifeMinutes));
	}
}
