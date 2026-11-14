//! The finale's people (docs/FINALE.md, "Items and people"): the man who didn't turn and his son,
//! Twice (Private Marek Fiala), the captain at the cold store, the cook and his brother, the sergeant
//! with the second key, the vet, the eight from the Day 6 trial and the radio ham. Each is data:
//! alive, a place, who holds them (custody), the route their yes went to, draws and betrayals
//! (saved). Bodies come only with the route's scenes, within 300-400 m of a player. Nobody is
//! killed off-screen, and consent is a check in every draw and dose: a person refuses without
//! their own yes. Claims: "person:<key>" (RZ_FinalePeople.Subject).
//!
//! Places are season canon (World facts): unturned.house (a Haven safehouse; the rota moves him every
//! 3 real days at dusk), held.store (the cold store in the hills), redhand.still, cook.brother,
//! origin.site.lower, ham.house, day6.cohort.<n>; held.count (280-340), unturned.name.

//------------------------------------------------------------------------------------------------
class RZ_FinalePersonSave
{
	string key;
	string name;
	string role;
	float x;
	float z;
	bool alive = true;
	int holder = -1;		// a route holding them in custody, -1 none
	int yes = -1;			// the route their yes went to, -1 none
	int draws;
	int betrayals;
	int faction = -1;		// whose people they live with (the eight), -1 none
}

//------------------------------------------------------------------------------------------------
class RZ_FinalePeopleSave
{
	bool seeded;
	int rotaUnix;
	int rotaHouse = -1;
	ref array<ref RZ_FinalePersonSave> people = {};
}

//------------------------------------------------------------------------------------------------
class RZ_FinalePeople
{
	static const int MAX_DRAWS = 5;		// he gives five at most, never a sixth

	protected RZ_GameModeComponent m_GM;
	protected RZ_CureFinale m_Finale;
	protected ref RZ_FinalePeopleSave m_S = new RZ_FinalePeopleSave();

	protected static const int ROTA_SECONDS = 259200;	// every 3 real days

	//------------------------------------------------------------------------------------------------
	void RZ_FinalePeople(RZ_GameModeComponent gm, RZ_CureFinale finale)
	{
		m_GM = gm;
		m_Finale = finale;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_FinalePeopleSave data)
	{
		if (data)
			m_S = data;
	}

	//------------------------------------------------------------------------------------------------
	RZ_FinalePeopleSave Save()
	{
		return m_S;
	}

	//------------------------------------------------------------------------------------------------
	//! A person's claim subject, beside the tribes' Tribe(id).
	static string Subject(string key)
	{
		return "person:" + key;
	}

	//------------------------------------------------------------------------------------------------
	RZ_FinalePersonSave Get(string key)
	{
		foreach (RZ_FinalePersonSave p : m_S.people)
		{
			if (p.key == key)
				return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	string Name(string key)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p)
			return "";

		return p.name;
	}

	//------------------------------------------------------------------------------------------------
	vector Pos(string key)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p)
			return vector.Zero;

		return RZ_Util.GroundPos(Vector(p.x, 0, p.z));
	}

	//------------------------------------------------------------------------------------------------
	bool IsAlive(string key)
	{
		RZ_FinalePersonSave p = Get(key);
		return p && p.alive;
	}

	//------------------------------------------------------------------------------------------------
	//! Their yes for route r: given now if they haven't given it to another route (one route at a
	//! time), and they're free (nobody holds them, or r does). False: they refuse.
	bool Yes(string key, int r)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p || !p.alive)
			return false;

		if (p.holder >= 0 && p.holder != r)
			return false;

		if (p.yes >= 0 && p.yes != r)
			return false;

		p.yes = r;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The route their yes went to, -1 none.
	int YesTo(string key)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p)
			return -1;

		return p.yes;
	}

	//------------------------------------------------------------------------------------------------
	//! One draw (needs their yes for r). False when they've given all they will.
	bool Draw(string key, int r, int most)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p || !p.alive || p.yes != r || p.draws >= most)
			return false;

		p.draws = p.draws + 1;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A bag of theirs was spoiled on purpose: they give again (one draw back).
	void GiveAgain(string key)
	{
		RZ_FinalePersonSave p = Get(key);
		if (p && p.draws > 0)
			p.draws = p.draws - 1;
	}

	//------------------------------------------------------------------------------------------------
	int Draws(string key)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p)
			return 0;

		return p.draws;
	}

	//------------------------------------------------------------------------------------------------
	//! Custody (a betrayal): a route walks the person to its own place. They give that holder
	//! nothing. Their third betrayal and they walk off alone.
	void Custody(string key, int r, vector to)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p || !p.alive)
			return;

		p.holder = r;
		p.x = to[0];
		p.z = to[2];
		Betray(key);
	}

	//------------------------------------------------------------------------------------------------
	//! A betrayal (custody, a diverted bag, a broken term). The third: they walk off.
	void Betray(string key)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p)
			return;

		p.betrayals = p.betrayals + 1;
		if (p.betrayals >= 3 && p.alive)
		{
			p.alive = false;
			RZ_Log.Info(string.Format("Finale: %1 walked off alone after a third betrayal.", p.key));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Killed on-screen (a scene's body): +20 Infamy with their faction for the killer, and the
	//! route moves to its branch.
	void Killed(string key, int killer)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p || !p.alive)
			return;

		p.alive = false;
		if (killer > 0 && p.faction >= 0)
			m_GM.AddInfamy(killer, p.faction, 20, "killed one of the finale's people", RZ_RepFlag.ACT);

		RZ_Log.Info(string.Format("Finale: %1 died on-screen.", p.key));
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_FinalePersonSave Add(string key, string name, string role, vector pos, int faction)
	{
		RZ_FinalePersonSave p = Get(key);
		if (!p)
		{
			p = new RZ_FinalePersonSave();
			p.key = key;
			m_S.people.Insert(p);
		}

		p.name = name;
		p.role = role;
		p.x = pos[0];
		p.z = pos[2];
		p.faction = faction;
		return p;
	}

	// ================================================================ The seed
	//------------------------------------------------------------------------------------------------
	//! At the gate: every person and place, as season canon, once.
	void Seed()
	{
		if (m_S.seeded)
			return;

		m_S.seeded = true;
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		RZ_Rng rng = new RZ_Rng(facts.Flag("finale.seed", 0) + RZ_SeasonClock.Now());

		// The man, in a Haven house; his son with the Red Hand crew nearest it
		int house = WomanHouse(rng);
		vector womanAt = HousePos(house);
		string woman = facts.CanonText("unturned.name");
		if (woman.IsEmpty())
		{
			woman = RZ_NoteText.FirstName(rng) + " " + RZ_NoteText.Surname(rng);
			facts.SetCanon("unturned.name", woman);
		}

		m_S.rotaHouse = house;
		m_S.rotaUnix = RZ_SeasonClock.Now();
		Add("woman", woman, "the lodger", womanAt, RZ_EFaction.SURVIVORS);
		vector crew = NearestSeat(RZ_EFaction.BANDITS, womanAt);
		Add("son", RZ_NoteText.FirstName(rng) + " " + RZ_NoteText.Surname(rng), "his son", crew, RZ_EFaction.BANDITS);

		// The cold store in the hills, and its captain
		vector store = StorePos(rng);
		facts.SetCanon("held.store", RZ_WorldFacts.PosText(store));
		facts.CanonInt("held.count", 280, 340);
		Add("captain", "Captain " + RZ_NoteText.Surname(rng), "the medical officer", store, RZ_EFaction.MILITARY);

		// Twice: Private Marek Fiala, with the crew nearest the store
		Add("twice", "Private Marek Fiala", "Twice", NearestSeat(RZ_EFaction.BANDITS, store), RZ_EFaction.BANDITS);

		// The cook at the Red Hand's stronghold, the still, and his brother at a Haven house
		vector rh = StrongholdPos(RZ_EFaction.BANDITS);
		Add("cook", RZ_NoteText.FirstName(rng) + " " + RZ_NoteText.Surname(rng), "the cook", rh, RZ_EFaction.BANDITS);
		vector still = StillPos(rng, rh);
		facts.SetCanon("redhand.still", RZ_WorldFacts.PosText(still));
		int sisterHouse = NearestHouse(rh);
		Add("sister", RZ_NoteText.FirstName(rng) + " " + RZ_NoteText.Surname(rng), "the cook's brother", HousePos(sisterHouse), RZ_EFaction.SURVIVORS);

		// The lower store at the closed site, the sergeant with the second key, the vet's tank
		vector site = facts.CanonPos("origin.site");
		vector lower = RZ_Mission.LandPointNear(site, 40, 150);
		if (site == vector.Zero)
			lower = RZ_Mission.LandPointNear(m_GM.GetLabPosition(), 1500, 3000);

		facts.SetCanon("origin.site.lower", RZ_WorldFacts.PosText(lower));
		Add("sergeant", "Sergeant " + RZ_NoteText.Surname(rng), "the sergeant", RZ_Mission.LandPointNear(lower, 800, 1600), RZ_EFaction.MILITARY);
		Add("vet", RZ_NoteText.FirstName(rng) + " " + RZ_NoteText.Surname(rng), "the vet", HousePos(NearestHouse(lower)), RZ_EFaction.SURVIVORS);

		// The eight: most at Haven houses, one on a Bastion roster, one in a crew's book, one at a
		// field station, one or two dead. And the ham in an empty village.
		SeedEight(rng);
		vector ham = HamPos(rng);
		facts.SetCanon("ham.house", RZ_WorldFacts.PosText(ham));
		Add("ham", RZ_NoteText.FirstName(rng) + " " + RZ_NoteText.Surname(rng), "the radio ham", ham, -1);
		RZ_Log.Info(string.Format("Finale: people seeded (%1); the cold store at %2.", m_S.people.Count(), RZ_WorldFacts.PosText(store)));
	}

	//------------------------------------------------------------------------------------------------
	protected void SeedEight(RZ_Rng rng)
	{
		array<int> where = {0, 0, 0, 1, 2, 3, -2, -2};
		int dead = rng.Int(1, 3);
		for (int i = 0; i < 8; i++)
		{
			int f = where[i];
			vector pos;
			if (f == RZ_EFaction.SURVIVORS || f < 0)
				pos = HousePos(rng.Int(0, Math.MaxInt(1, HouseCount())));
			else
				pos = NearestSeat(f, m_GM.GetLabPosition());

			string key = string.Format("eight%1", i + 1);
			RZ_FinalePersonSave p = Add(key, RZ_NoteText.FirstName(rng) + " " + RZ_NoteText.Surname(rng), "one of the eight", pos, Math.MaxInt(f, RZ_EFaction.SURVIVORS));
			if (f < 0 && dead > 0)
			{
				p.alive = false;
				dead--;
			}

			if (f < 0 && p.alive)
				p.faction = RZ_EFaction.SURVIVORS;

			RZ_WorldFacts.Get().SetCanon("day6.cohort." + (i + 1).ToString(), RZ_WorldFacts.PosText(pos));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The eight still alive, in seed order.
	void LivingEight(notnull array<string> result)
	{
		for (int i = 1; i <= 8; i++)
		{
			string key = string.Format("eight%1", i);
			if (IsAlive(key))
				result.Insert(key);
		}
	}

	// ================================================================ Places
	//------------------------------------------------------------------------------------------------
	protected int HouseCount()
	{
		if (!m_GM.GetSafehouses())
			return 0;

		return m_GM.GetSafehouses().Count();
	}

	//------------------------------------------------------------------------------------------------
	protected vector HousePos(int idx)
	{
		if (idx >= 0 && idx < HouseCount())
			return m_GM.GetSafehouses().Get(idx).m_vPos;

		return StrongholdPos(RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	protected int NearestHouse(vector pos)
	{
		int best = -1;
		float bestD = 100000000;
		for (int i = 0; i < HouseCount(); i++)
		{
			float d = RZ_Util.DistXZ(m_GM.GetSafehouses().Get(i).m_vPos, pos);
			if (d < bestD)
			{
				bestD = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A Haven house no other story holds, away from the chair's house while there's another.
	protected int WomanHouse(RZ_Rng rng)
	{
		int chair = RZ_HavenQuest.ChairHouse(m_GM);
		array<int> free = {};
		for (int i = 0; i < HouseCount(); i++)
		{
			if (i != chair && !RZ_WorldFacts.Get().IsClaimed(RZ_WorldFacts.Safehouse(i)))
				free.Insert(i);
		}

		if (free.IsEmpty())
			return chair;

		return free[rng.Int(0, free.Count())];
	}

	//------------------------------------------------------------------------------------------------
	protected vector StrongholdPos(int f)
	{
		RZ_Leader l = RZ_MajorHelp.Top(m_GM, f);
		if (l)
			return l.m_vPos;

		if (m_GM.GetLeaders() && m_GM.GetLeaders().GetStronghold(f))
			return m_GM.GetLeaders().GetStronghold(f).m_vPos;

		return m_GM.GetLabPosition();
	}

	//------------------------------------------------------------------------------------------------
	//! The faction's seat (an outpost or its stronghold) nearest `pos`.
	protected vector NearestSeat(int f, vector pos)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		vector best = StrongholdPos(f);
		if (!territory || !territory.IsActive())
			return best;

		float bestD = RZ_Util.DistXZ(best, pos);
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h.m_eFaction != f)
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, pos);
			if (d < bestD)
			{
				bestD = d;
				best = h.m_vPos;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! held.store: 1.5-5 km from Bastion's stronghold, best 2-4 km from the lab and 800 m from any
	//! settlement (relaxed on small maps).
	protected vector StorePos(RZ_Rng rng)
	{
		string have = RZ_WorldFacts.Get().CanonText("held.store");
		if (!have.IsEmpty())
			return RZ_WorldFacts.ParsePos(have);

		vector base = StrongholdPos(RZ_EFaction.MILITARY);
		vector lab = m_GM.GetLabPosition();
		vector best = vector.Zero;
		float bestScore = -100000000;
		for (int i = 0; i < 30; i++)
		{
			vector p = RZ_Mission.LandPointNear(base, 1500, 5000);
			if (p == vector.Zero)
				continue;

			float score = 0;
			float dl = RZ_Util.DistXZ(p, lab);
			if (dl < 2000 || dl > 4000)
				score -= Math.AbsFloat(dl - 3000) / 1000;

			RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(p));
			if (s && RZ_Util.DistXZ(s.m_vCenter, p) < 800)
				score -= 2;

			if (score > bestScore)
			{
				bestScore = score;
				best = p;
			}
		}

		if (best == vector.Zero)
			best = RZ_Mission.LandPointNear(base, 800, 1500);

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! redhand.still: outside a small town 1-4 km from the Red Hand's stronghold, never within 1.5 km
	//! of the Long Table's farm.
	protected vector StillPos(RZ_Rng rng, vector rh)
	{
		vector farm = FarmPos();
		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			float d = RZ_Util.DistXZ(s.m_vCenter, rh);
			if (d < 1000 || d > 4000)
				continue;

			if (farm != vector.Zero && RZ_Util.DistXZ(s.m_vCenter, farm) < 1500)
				continue;

			return RZ_Mission.LandPointNear(s.m_vCenter, s.m_fRadius + 50, s.m_fRadius + 200);
		}

		return RZ_Mission.LandPointNear(rh, 1500, 2500);
	}

	//------------------------------------------------------------------------------------------------
	//! ham.house: an empty village, never within 2 km of the Long Table's farm.
	protected vector HamPos(RZ_Rng rng)
	{
		vector farm = FarmPos();
		RZ_Territory territory = m_GM.GetTerritory();
		foreach (int idx, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (farm != vector.Zero && RZ_Util.DistXZ(s.m_vCenter, farm) < 2000)
				continue;

			if (territory && territory.IsActive() && territory.FindInTown(idx))
				continue;

			if (NearestHouse(s.m_vCenter) >= 0 && RZ_Util.DistXZ(HousePos(NearestHouse(s.m_vCenter)), s.m_vCenter) < s.m_fRadius)
				continue;

			return RZ_Mission.LandPointNear(s.m_vCenter, 10, s.m_fRadius);
		}

		return RZ_Mission.LandPointNear(m_GM.GetLabPosition(), 2000, 3500);
	}

	//------------------------------------------------------------------------------------------------
	protected vector FarmPos()
	{
		if (!m_GM.GetLongTable() || !m_GM.GetLongTable().IsEnabled())
			return vector.Zero;

		return m_GM.GetLongTable().GetHousePos();
	}

	// ================================================================ The rota
	//------------------------------------------------------------------------------------------------
	//! Every 3 real days at dusk the rota moves him, as data, to the next Haven house no other story
	//! holds (one house: he stays put). A Haven-locked player can move his early (MoveWoman).
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		RZ_FinalePersonSave p = Get("woman");
		if (!p || !p.alive || p.holder >= 0)
			return;

		if (RZ_SeasonClock.Now() - m_S.rotaUnix < ROTA_SECONDS * m_GM.GetQuestlines().WaitScale() || !RZ_WorldFacts.Get().IsDusk())
			return;

		MoveWoman();
	}

	//------------------------------------------------------------------------------------------------
	void MoveWoman()
	{
		RZ_FinalePersonSave p = Get("woman");
		if (!p || HouseCount() <= 1)
			return;

		int chair = RZ_HavenQuest.ChairHouse(m_GM);
		for (int step = 1; step < HouseCount(); step++)
		{
			int next = (m_S.rotaHouse + step) % HouseCount();
			if ((next == chair && HouseCount() > 2) || RZ_WorldFacts.Get().IsClaimed(RZ_WorldFacts.Safehouse(next)))
				continue;

			m_S.rotaHouse = next;
			vector at = HousePos(next);
			p.x = at[0];
			p.z = at[2];
			break;
		}

		m_S.rotaUnix = RZ_SeasonClock.Now();
		RZ_Log.Info(string.Format("Finale: the rota moved the man to house %1.", m_S.rotaHouse));
	}

	//------------------------------------------------------------------------------------------------
	int WomanHouseNow()
	{
		return m_S.rotaHouse;
	}
}
