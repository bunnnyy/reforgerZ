//! The finale's items (docs/FINALE.md, "Items and people"): his bags, the box, a tray, the pan's
//! case, the vials, the specimen case and the like, carried as data by one identity. Each keeps its
//! route, its holder or the place it lies, its cold clock (seconds out of the cold) and its chain of
//! carriers (saved). They drop where their carrier dies (the robbed-truck rule) and are picked up by
//! whoever walks over them (3 m): a rival who takes one keeps it, gives it back at the route's desk or
//! lets it spoil. A dropped item nobody comes near goes back to its seat after 2 hours of play or a real day. They are
//! never cleaned up as loose items: there's no entity to clean.
//! Cold places: within 100 m of the lab, 60 m of a Halcyon holding (a field station) or of Bastion's
//! stronghold (its medical store).

//------------------------------------------------------------------------------------------------
class RZ_FinaleItemSave
{
	int id;
	string kind;
	string name;
	int route;
	string carrier;
	float x;
	float z;
	float seatX;
	float seatZ;
	float cold = -1;
	float idle;
	int dropUnix;
	string chain;
}

//------------------------------------------------------------------------------------------------
class RZ_FinaleItemsSave
{
	int nextId = 1;
	ref array<ref RZ_FinaleItemSave> items = {};
}

//------------------------------------------------------------------------------------------------
class RZ_FinaleItems
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_CureFinale m_Finale;
	protected ref RZ_FinaleItemsSave m_S = new RZ_FinaleItemsSave();

	protected static const float PICKUP = 3;
	protected static const float NEAR = 150;
	protected static const float RETURN_PLAY = 7200;
	protected static const int RETURN_REAL = 86400;

	//------------------------------------------------------------------------------------------------
	void RZ_FinaleItems(RZ_GameModeComponent gm, RZ_CureFinale finale)
	{
		m_GM = gm;
		m_Finale = finale;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_FinaleItemsSave data)
	{
		if (data)
			m_S = data;
	}

	//------------------------------------------------------------------------------------------------
	RZ_FinaleItemsSave Save()
	{
		return m_S;
	}

	//------------------------------------------------------------------------------------------------
	//! A new item in `ident`'s hands ("" = lying at `seat`). cold: seconds it keeps out of the cold
	//! (-1: no clock). Returns its id.
	int Give(string kind, string name, int route, string ident, vector seat, float cold)
	{
		RZ_FinaleItemSave it = new RZ_FinaleItemSave();
		it.id = m_S.nextId;
		m_S.nextId = m_S.nextId + 1;
		it.kind = kind;
		it.name = name;
		it.route = route;
		it.carrier = ident;
		it.x = seat[0];
		it.z = seat[2];
		it.seatX = seat[0];
		it.seatZ = seat[2];
		it.cold = cold;
		it.chain = Short(ident);
		m_S.items.Insert(it);
		RZ_Log.Info(string.Format("Finale: item %1 (%2) for %3 to %4.", it.id, name, RZ_CureFinale.RouteName(route), ident));
		return it.id;
	}

	//------------------------------------------------------------------------------------------------
	protected string Short(string ident)
	{
		if (ident.IsEmpty())
			return "";

		int pid = m_GM.GetQuestlines().OnlineId(ident);
		if (pid > 0)
			return RZ_Names.Public(pid);

		return "someone";
	}

	//------------------------------------------------------------------------------------------------
	//! How many items of `kind` for `route` this identity carries (route -1: any route).
	int Count(string ident, string kind, int route)
	{
		int n = 0;
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier == ident && it.kind == kind && (route < 0 || it.route == route))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Items of `kind` for `route` anywhere (carried or lying).
	int Exists(string kind, int route)
	{
		int n = 0;
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.kind == kind && it.route == route)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Hands in up to `count` of this identity's `kind` items. anyRoute: a crossing (his tube for the
	//! Haven, any one unit for the Independent): an item of any route counts. Returns how many.
	int HandIn(string ident, string kind, int route, int count, bool anyRoute = false)
	{
		int taken = 0;
		for (int i = m_S.items.Count() - 1; i >= 0 && taken < count; i--)
		{
			RZ_FinaleItemSave it = m_S.items[i];
			if (it.carrier != ident || it.kind != kind || (!anyRoute && it.route != route))
				continue;

			// Handing in for another route than the item's own: "handed over"
			if (it.route != route)
				m_Finale.Hostile(route, it.route);

			RZ_Log.Info(string.Format("Finale: item %1 (%2) handed in for %3; carriers %4.", it.id, it.name, RZ_CureFinale.RouteName(route), it.chain));
			m_S.items.Remove(i);
			taken++;
		}

		return taken;
	}

	//------------------------------------------------------------------------------------------------
	//! One unit from another route's subject, for route r (the Independent): a carried item that
	//! isn't a tray and isn't r's own bag (another route's bag, tube, box, case or vial, or a vial
	//! pocketed for nobody).
	protected bool IsUnitFor(RZ_FinaleItemSave it, int r)
	{
		return it.kind != "tray" && (it.route != r || it.kind != "bag");
	}

	//------------------------------------------------------------------------------------------------
	int CountAny(string ident, int r)
	{
		int n = 0;
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier == ident && IsUnitFor(it, r))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Hands in one such unit for route r. Another route's item is "handed over": the two routes
	//! are fighting. Returns how many (0 or 1).
	int HandInAny(string ident, int r)
	{
		for (int i = m_S.items.Count() - 1; i >= 0; i--)
		{
			RZ_FinaleItemSave it = m_S.items[i];
			if (it.carrier != ident || !IsUnitFor(it, r))
				continue;

			if (it.route != r)
				m_Finale.Hostile(r, it.route);

			RZ_Log.Info(string.Format("Finale: item %1 (%2) handed in for %3 as one unit; carriers %4.", it.id, it.name, RZ_CureFinale.RouteName(r), it.chain));
			m_S.items.Remove(i);
			return 1;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Ambush a carry: an item of another route this identity carries is spoiled on purpose.
	//! Returns its route, or NONE when there was none.
	int Spoil(string ident, int mine)
	{
		for (int i = m_S.items.Count() - 1; i >= 0; i--)
		{
			RZ_FinaleItemSave it = m_S.items[i];
			if (it.carrier != ident || it.route == mine)
				continue;

			int r = it.route;
			RZ_Log.Info(string.Format("Finale: item %1 (%2) of %3 spoiled on purpose.", it.id, it.name, RZ_CureFinale.RouteName(r)));
			Tell(it, string.Format("%1 was spoiled on purpose. That step has to be done again.", it.name));
			m_S.items.Remove(i);
			return r;
		}

		return RZ_CureFinale.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! The first route other than `mine` this identity carries an item of, NONE if none.
	int CarriesOther(string ident, int mine)
	{
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier == ident && it.route != mine)
				return it.route;
		}

		return RZ_CureFinale.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! "Found it": this identity's items of `route` go back to their seats. Returns how many.
	int GiveBack(string ident, int route)
	{
		int n = 0;
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier != ident || it.route != route)
				continue;

			it.carrier = "";
			it.x = it.seatX;
			it.z = it.seatZ;
			it.idle = 0;
			it.dropUnix = RZ_SeasonClock.Now();
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Every item of `route` is gone (a frozen race, the cure).
	void ClearRoute(int route)
	{
		for (int i = m_S.items.Count() - 1; i >= 0; i--)
		{
			if (m_S.items[i].route == route)
				m_S.items.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The carrier died: what they carried drops where they fell.
	void OnCarrierDied(string ident, vector at, string killer)
	{
		if (ident.IsEmpty())
			return;

		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier != ident)
				continue;

			it.carrier = "";
			it.x = at[0];
			it.z = at[2];
			it.idle = 0;
			it.dropUnix = RZ_SeasonClock.Now();
			RZ_Log.Info(string.Format("Finale: item %1 (%2) dropped where its carrier fell.", it.id, it.name));
			int kr = m_Finale.RouteOf(killer);
			if (kr >= 0)
				m_Finale.Hostile(kr, it.route);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where a carried item of `route` is now (its carrier online), zero if none is on the move.
	vector CarriedPos(int route)
	{
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.route != route || it.carrier.IsEmpty())
				continue;

			int pid = m_GM.GetQuestlines().OnlineId(it.carrier);
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
			if (ent)
				return ent.GetOrigin();
		}

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! "a bag of his blood, the box" for what this identity carries (route -1: any).
	string CarriedText(string ident, int route)
	{
		string s = "";
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier != ident || (route >= 0 && it.route != route))
				continue;

			if (!s.IsEmpty())
				s += ", ";

			s += it.name;
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! A route's items for its own players: what, and carried or where it lies.
	string RouteText(int route)
	{
		string s = "";
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.route != route)
				continue;

			if (!s.IsEmpty())
				s += "; ";

			if (!it.carrier.IsEmpty())
			{
				s += it.name + " (carried)";
				continue;
			}

			string where = "on the ground";
			RZ_Settlement st = m_GM.GetSettlement(m_GM.FindNearestSettlement(Vector(it.x, 0, it.z)));
			if (st)
				where = "on the ground near " + st.m_sName;

			s += it.name + " (" + where + ")";
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! In the cold: near the lab, a Halcyon holding or Bastion's stronghold.
	protected bool InCold(vector pos)
	{
		if (m_GM.HasLab() && RZ_Util.DistXZ(pos, m_GM.GetLabPosition()) <= 100)
			return true;

		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return false;

		foreach (RZ_Holding h : territory.GetHoldings())
		{
			bool cold = h.m_eFaction == RZ_EFaction.INSTITUTE || (h.m_eFaction == RZ_EFaction.MILITARY && h.m_bStronghold);
			if (cold && RZ_Util.DistXZ(pos, h.m_vPos) <= 60)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		for (int i = m_S.items.Count() - 1; i >= 0; i--)
		{
			RZ_FinaleItemSave it = m_S.items[i];
			if (it.carrier.IsEmpty())
				TickLying(it, dt, ids, players);
			else
				TickCarried(it, dt);

			if (it.cold == 0)
			{
				RZ_Log.Info(string.Format("Finale: item %1 (%2) spoiled out of the cold.", it.id, it.name));
				Tell(it, string.Format("%1 has spoiled out of the cold. That step has to be done again.", it.name));
				m_S.items.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TickCarried(RZ_FinaleItemSave it, float dt)
	{
		int pid = m_GM.GetQuestlines().OnlineId(it.carrier);
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
		if (!ent)
			return;

		vector pos = ent.GetOrigin();
		it.x = pos[0];
		it.z = pos[2];
		if (it.cold > 0 && !InCold(pos))
			it.cold = Math.Max(0, it.cold - dt);
	}

	//------------------------------------------------------------------------------------------------
	protected void TickLying(RZ_FinaleItemSave it, float dt, array<int> ids, array<IEntity> players)
	{
		vector at = Vector(it.x, 0, it.z);
		if (it.cold > 0 && !InCold(at))
			it.cold = Math.Max(0, it.cold - dt);

		bool near = false;
		foreach (int i, IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), at);
			if (d <= NEAR)
				near = true;

			if (d <= PICKUP)
			{
				PickUp(it, ids[i]);
				return;
			}
		}

		if (near)
			return;

		it.idle = it.idle + dt;
		if (it.idle >= RETURN_PLAY * WaitScale() || RZ_SeasonClock.Now() - it.dropUnix >= RETURN_REAL * WaitScale())
		{
			it.x = it.seatX;
			it.z = it.seatZ;
			it.idle = 0;
			it.dropUnix = RZ_SeasonClock.Now();
			RZ_Log.Info(string.Format("Finale: item %1 (%2) went back to its seat.", it.id, it.name));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float WaitScale()
	{
		return m_GM.GetQuestlines().WaitScale();
	}

	//------------------------------------------------------------------------------------------------
	protected void PickUp(RZ_FinaleItemSave it, int playerId)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		it.carrier = ident;
		it.idle = 0;
		string name = RZ_Names.Public(playerId);
		if (!it.chain.Contains(name))
			it.chain = it.chain + " > " + name;

		int mine = m_Finale.RouteOfPlayer(playerId);
		if (mine != RZ_CureFinale.NONE && mine != it.route)
			m_Finale.Hostile(mine, it.route);

		m_GM.NotifyPlayer(playerId, RZ_CureFinale.RouteTitle(it.route), string.Format("You pick up %1. It's %2's.", it.name, RZ_CureFinale.RouteName(it.route)));
	}

	//------------------------------------------------------------------------------------------------
	//! Tell the route's online locked players.
	protected void Tell(RZ_FinaleItemSave it, string text)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int pid : all)
		{
			if (m_Finale.RouteOfPlayer(pid) == it.route || m_GM.GetProfile(pid).m_sIdentity == it.carrier)
				m_GM.NotifyPlayer(pid, RZ_CureFinale.RouteTitle(it.route), text);
		}
	}

	// ================================================================ The rest (RZ_FinaleRest)
	//------------------------------------------------------------------------------------------------
	//! How many items of `route` this identity carries.
	int CarriesRoute(string ident, int route)
	{
		int n = 0;
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier == ident && it.route == route)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Hand it over and Buy It: every item of `route` this identity carries goes to `to`, logged in
	//! its chain. Returns how many.
	int Pass(string from, string to, int route)
	{
		int n = 0;
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier != from || it.route != route)
				continue;

			it.carrier = to;
			it.chain = it.chain + " > (handed over)";
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Every item lying on the ground (for the drain at a container).
	void Lying(notnull array<RZ_FinaleItemSave> result)
	{
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.carrier.IsEmpty())
				result.Insert(it);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Drained at the container: it spoils on the next tick, as out of the cold.
	void Drain(int id)
	{
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			if (it.id == id)
				it.cold = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keep It Overnight: at dawn, an item carried in the cold gets an hour of its cold clock back,
	//! once an item (`done` holds the ids already given it).
	void Overnight(notnull array<string> done)
	{
		foreach (RZ_FinaleItemSave it : m_S.items)
		{
			string key = it.id.ToString();
			if (it.carrier.IsEmpty() || it.cold < 0 || done.Contains(key) || !InCold(Vector(it.x, 0, it.z)))
				continue;

			done.Insert(key);
			it.cold = it.cold + 3600;
			Tell(it, string.Format("%1 kept overnight in the cold. It will last an hour longer.", it.name));
		}
	}
}
