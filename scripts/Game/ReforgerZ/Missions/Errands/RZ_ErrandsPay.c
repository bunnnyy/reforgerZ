//! Errand pay's other kinds (docs/ERRANDS.md, "Pay: nothing for free"): in kind from a post's stock,
//! a meal at a fed house, "when the truck comes", and carried items. Every one pays little, never
//! locks a route and never hands out doses, crates, kit or keys. Server only.

//------------------------------------------------------------------------------------------------
//! Pay waiting for the next truck at a post (to the life that earned it). Saved (RZ_ErrandsSave).
class RZ_ErrandDeferred
{
	string ident;
	int life;
	int faction;
	int favor;
	string title;
	float atX;			// the post's giver
	float atZ;
	int madeUnix;
}

//------------------------------------------------------------------------------------------------
class RZ_ErrandsPay
{
	static const float TRUCK_RANGE = 150;
	static const int DEFER_HOURS = 24;		// real hours a deferred pay waits for its truck
	static const int KIND_CUT = 3;			// Favor a ration or magazine replaces
	static const int MEAL_FAVOR = 2;
	static const float MEAL_HEAL = 0.15;	// of max health
	static const int THIEF_INFAMY = 3;

	protected RZ_Errands m_Dealer;
	protected ref array<ref RZ_ErrandDeferred> m_aWaiting = {};
	protected int m_iHeadlineDay = -1;
	protected int m_iHeadlines;

	//------------------------------------------------------------------------------------------------
	void RZ_ErrandsPay(RZ_Errands dealer)
	{
		m_Dealer = dealer;
	}

	// ================================================================ A meal at a fed house
	//------------------------------------------------------------------------------------------------
	//! A house errand done at a house fed above one upkeep pays supper: the house -1 food, a little
	//! healing and 2 Favor, in place of a way's Favor of 4 or less. The line, "" if not a meal.
	string Meal(RZ_Errand e, RZ_ErrandWay w, int playerId)
	{
		RZ_GameModeComponent gm = m_Dealer.GetGM();
		RZ_ErrandSettings s = RZ_ErrandSettings.Get();
		if (!s.m_bMeals || e.m_iHouse < 0 || w.m_iFavor > 4 || !gm.GetSafehouses())
			return "";

		RZ_Safehouse h = gm.GetSafehouses().Get(e.m_iHouse);
		if (!h || h.m_iFood <= h.FoodNeed(gm.GetSafehouses().PeoplePerFood()))
			return "";

		gm.GetSafehouses().AddFood(e.m_iHouse, -1);
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (me)
		{
			float max = RZ_Util.GetCharacterMaxHealth(me);
			RZ_Util.SetCharacterHealth(me, Math.Min(max, RZ_Util.GetCharacterHealth(me) + max * MEAL_HEAL));
		}

		m_Dealer.SmallFavor(playerId, RZ_EFaction.SURVIVORS, MEAL_FAVOR);
		RZ_Log.Info(string.Format("Errands: %1 paid in a meal at %2", e.m_Def.m_sKey, h.m_sName));
		return string.Format("They sit you down at the table in %1 and put a bowl in front of you. Supper is the pay.", h.m_sName);
	}

	// ================================================================ In kind, from a post's stock
	//------------------------------------------------------------------------------------------------
	//! At a Bastion post, one errand in three pays a ration from its stock (never while Bastion is
	//! short of food or supplies) in place of 3 of its Favor, tagged as the post's so nobody else takes
	//! it for pay. A Red Hand camp pays a magazine the same way. The Favor it replaced, 0 if none.
	int InKind(RZ_Errand e, RZ_ErrandWay w, int playerId, out string line)
	{
		line = "";
		RZ_GameModeComponent gm = m_Dealer.GetGM();
		RZ_ErrandSettings s = RZ_ErrandSettings.Get();
		if (!s.m_bPayInKind || e.m_iHolding < 0 || w.m_iFavor < KIND_CUT + 1 || !s.m_Props)
			return 0;

		ResourceName prefab;
		if (e.m_iFaction == RZ_EFaction.MILITARY)
			prefab = s.m_Props.m_sPayRation;
		else if (e.m_iFaction == RZ_EFaction.BANDITS)
			prefab = s.m_Props.m_sPayMagazine;

		if (prefab.IsEmpty() || RZ_Util.PickIndex(e.m_iId + playerId, 3) != 0)
			return 0;

		RZ_FactionLife life = gm.GetFactionLife();
		if (life)
		{
			int need = life.GetNeed(e.m_iFaction);
			if (need == RZ_ENeed.FOOD || need == RZ_ENeed.SUPPLIES)
				return 0;
		}

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		string owner = e.m_sGiverName;
		if (!RZ_MissionGoods.Give(me, prefab, owner))
			return 0;

		line = "Part of it in kind, off the post's own shelf, with a name on it in marker.";
		if (e.m_iFaction == RZ_EFaction.BANDITS)
			line = "Part of it in kind: a magazine, with a red thumbprint on the base.";

		RZ_Log.Info(string.Format("Errands: %1 paid in kind to %2", e.m_Def.m_sKey, RZ_Util.PlayerName(playerId)));
		return KIND_CUT;
	}

	// ================================================================ When the truck comes
	//------------------------------------------------------------------------------------------------
	//! A post short of supplies pays at its next truck: the Favor waits (a real day at most) for the life
	//! that earned it. True if it was deferred.
	bool Defer(RZ_Errand e, int playerId, int faction, int favor)
	{
		RZ_GameModeComponent gm = m_Dealer.GetGM();
		RZ_ErrandSettings s = RZ_ErrandSettings.Get();
		if (!s.m_bTruckPay || e.m_iHolding < 0 || favor <= 0 || !gm.GetFactionLife() || gm.GetFactionLife().GetNeed(faction) != RZ_ENeed.SUPPLIES)
			return false;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		RZ_ErrandDeferred d = new RZ_ErrandDeferred();
		d.ident = p.m_sIdentity;
		d.life = p.m_iLives;
		d.faction = faction;
		d.favor = favor;
		d.title = e.m_sGiverTitle;
		d.atX = e.m_vGiver[0];
		d.atZ = e.m_vGiver[2];
		d.madeUnix = RZ_SeasonClock.Now();
		m_aWaiting.Insert(d);
		RZ_Log.Info(string.Format("Errands: %1's pay waits for the truck (%2 Favor)", RZ_Util.PlayerName(playerId), favor));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A truck came in: the pay waiting at that post goes to the lives that earned it (online now).
	void OnTruck(int faction, vector at)
	{
		RZ_GameModeComponent gm = m_Dealer.GetGM();
		int now = RZ_SeasonClock.Now();
		for (int i = m_aWaiting.Count() - 1; i >= 0; i--)
		{
			RZ_ErrandDeferred d = m_aWaiting[i];
			if (now - d.madeUnix > DEFER_HOURS * 3600)
			{
				m_aWaiting.Remove(i);
				continue;
			}

			if (d.faction != faction || RZ_Util.DistXZ(Vector(d.atX, 0, d.atZ), at) > TRUCK_RANGE)
				continue;

			int playerId = gm.FindPlayerByIdentity(d.ident);
			m_aWaiting.Remove(i);
			if (playerId <= 0 || gm.GetProfile(playerId).m_iLives != d.life)
				continue;

			gm.AddFavor(playerId, faction, d.favor);
			gm.NotifyPlayerAs(playerId, d.title, string.Format("The truck's in, so here's what the post owed you. +%1 %2 Favor.", d.favor, RZ_Factions.GetName(faction)), RZ_ENoticeTag.MISSION);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The pay still waiting for a truck, for the save (real time, so a restart doesn't lose it).
	void Save(notnull array<ref RZ_ErrandDeferred> into)
	{
		int now = RZ_SeasonClock.Now();
		foreach (RZ_ErrandDeferred d : m_aWaiting)
		{
			if (d && now - d.madeUnix <= DEFER_HOURS * 3600)
				into.Insert(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Restore(array<ref RZ_ErrandDeferred> from)
	{
		if (!from)
			return;

		foreach (RZ_ErrandDeferred d : from)
		{
			if (d && !d.ident.IsEmpty())
				m_aWaiting.Insert(d);
		}
	}

	// ================================================================ Carried items
	//------------------------------------------------------------------------------------------------
	//! A carried way's step is done: its item (Missions > Errands > Props > Carried, else the mission
	//! item) goes in the asker's pack. It can't be equipped, no mission's drop takes it, and it counts
	//! for no trade.
	void GiveCarried(RZ_Errand e, IEntity me)
	{
		RZ_GameModeComponent gm = m_Dealer.GetGM();
		RZ_ErrandSettings s = RZ_ErrandSettings.Get();
		ResourceName prefab = gm.GetMissionItemPrefab();
		if (s.m_Props && !s.m_Props.m_sCarried.IsEmpty())
			prefab = s.m_Props.m_sCarried;

		e.m_Carried = RZ_MissionGoods.Give(me, prefab, e.m_sGiverName);
		e.m_bCarried = e.m_Carried != null;
		if (e.m_Carried && e.m_Carried.FindComponent(RZ_MissionItemComponent))
			RZ_MissionItemComponent.Tag(e.m_Carried, -1000 - e.m_iId);
	}

	//------------------------------------------------------------------------------------------------
	//! Its carried item was given (and may since have been lost).
	bool HadCarried(RZ_Errand e)
	{
		return e.m_bCarried;
	}

	//------------------------------------------------------------------------------------------------
	//! Who carries the errand's item now: the player id, 0 on the ground, -1 gone.
	static int Carrier(RZ_Errand e)
	{
		if (!e.m_Carried)
			return -1;

		return Math.Max(0, RZ_Util.GetCarrierPlayerId(e.m_Carried));
	}

	//------------------------------------------------------------------------------------------------
	//! Taken by someone who isn't the asker or a helper: spoiled, and the thief takes the card's
	//! Infamy with the giver's people. True if it was.
	bool Stolen(RZ_Errand e, int carrier)
	{
		if (carrier <= 0 || carrier == e.m_iAsker || e.m_aHelpers.Contains(carrier))
			return false;

		RZ_GameModeComponent gm = m_Dealer.GetGM();
		if (RZ_Factions.IsValid(e.m_iFaction))
			gm.AddInfamy(carrier, e.m_iFaction, THIEF_INFAMY, "took an errand's carried things");

		gm.NotifyPlayer(e.m_iAsker, "Errand: " + e.Fill(e.m_Def.m_sTitle), "Somebody else has it now. It's spoiled: nobody will take it from them, or pay for it.");
		RZ_Log.Info(string.Format("Errands: %1's carried item taken by %2", e.m_Def.m_sKey, RZ_Util.PlayerName(carrier)));
		return true;
	}

	// ================================================================ Headlines
	//------------------------------------------------------------------------------------------------
	//! At most two errand headlines a real day (Missions > Errands > Headlines a day). True if this one
	//! may go out.
	bool MayHeadline()
	{
		int day = RZ_SeasonClock.Now() / 86400;
		if (day != m_iHeadlineDay)
		{
			m_iHeadlineDay = day;
			m_iHeadlines = 0;
		}

		if (m_iHeadlines >= RZ_ErrandSettings.Get().m_iHeadlinesPerDay)
			return false;

		m_iHeadlines++;
		return true;
	}
}
