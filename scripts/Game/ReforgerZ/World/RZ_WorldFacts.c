//! World facts (docs/DESIGN.md, "World facts"): one place where content (missions, events, arcs,
//! questlines, rumours, voice, guidance) reads what is true in the world right now, so text and
//! weights never contradict it. No new simulation: every query reads a system that already owns
//! the fact. On top of that it keeps four small things of its own:
//!
//! - Season flags: ints any content can read and write ("heli.down_until", "<questline>.ending").
//!   The questlines' world flags are these. Saved in state.json, wiped with the season.
//! - Canon: fixed facts of the setting made once a season from its seed (where the outbreak
//!   started, the first patient, what the last orders said, where the Red Hand's pit is), so
//!   every mission that mentions one says the same thing all season. Saved, wiped with the season.
//! - Claims: who is using a subject right now ("lab", "town:7", "safehouse:2", "leader:12",
//!   "wanderer:5", "truck:1", "heli", "pit"), so two stories don't use the same house, leader or
//!   wanderer at once. Runtime only: after a restart, running content claims again.
//! - Gossip: rumours wanderers and leaders repeat for some hours (Changed(), questline endings).
//!   Saved.
//! - World memory (Memory(), World/RZ_WorldMemory.c): reasons, the signal ring, the town log and
//!   world records. Saved, wiped with the season.
//!
//! Server only. Reached as gm.GetFacts() or RZ_WorldFacts.Get() (never null: without a game mode
//! every query answers with its default).

//------------------------------------------------------------------------------------------------
//! What holds a town: TownOwner().
enum RZ_ETownKind
{
	NONE,			// abandoned: nobody lives here
	SAFEHOUSE,		// a Haven safehouse
	OUTPOST,		// a faction outpost (territory)
	STRONGHOLD,		// a faction stronghold (never falls)
	LAB				// the Halcyon lab
}

//------------------------------------------------------------------------------------------------
//! Who holds a town and how. faction -1 = nobody.
class RZ_TownOwner
{
	int m_iSettlement = -1;
	int m_iFaction = -1;
	RZ_ETownKind m_eKind = RZ_ETownKind.NONE;
	int m_iRef = -1;		// the holding id (outpost, stronghold) or safehouse index, -1 otherwise
	string m_sTown;

	//------------------------------------------------------------------------------------------------
	bool IsHeld()
	{
		return m_eKind != RZ_ETownKind.NONE;
	}

	//------------------------------------------------------------------------------------------------
	bool IsHeldBy(int faction)
	{
		return m_eKind != RZ_ETownKind.NONE && m_iFaction == faction;
	}
}

//------------------------------------------------------------------------------------------------
//! A safehouse as it is right now: House(). m_bValid false if there's no such house.
class RZ_HouseFacts
{
	bool m_bValid;
	int m_iIndex = -1;
	int m_iSettlement = -1;
	string m_sTown;
	vector m_vPos;
	int m_iPeople;
	int m_iFood;
	bool m_bShort;			// the last meal ran short
	bool m_bHasMedic;		// a Settlement (12 people) or bigger
	bool m_bHasRadio;
	int m_iLost;			// people gone lately, for any reason: left, moved, taken or dead (fades by one an hour)
	int m_iDied;			// of those, the dead (fades the same way): "buried" only means these
	bool m_bEmpty;			// nobody lives there now (the house stands, its tier is the lowest)
	string m_sTier;			// Shelter, Community, Settlement, Town
}

//------------------------------------------------------------------------------------------------
//! Which towns a pick may use (RZ_WorldFacts.CollectTowns, PickTown). The defaults take any town
//! no mission or story is using; set only what the job needs.
class RZ_TownFilter
{
	float m_fMinInfection = 0;
	float m_fMaxInfection = 100;
	int m_iHeldBy = -2;			// -2 any; -1 nobody lives there (IsLivedIn false); else held by this faction
	int m_iNotHeldBy;			// RZ_Factions.Bit(f) of factions whose towns are skipped (an enemy of the giver)
	int m_iKinds;				// RZ_WorldFacts.KindBit(RZ_ETownKind...) bits; 0 = any kind
	int m_iMinSize;				// the settlement's size, 0 any (RZ_Settlement.m_iSize: 2+ is a town or city)
	int m_iExclude = -1;		// a settlement to leave out (the job's own start)
	vector m_vFrom;				// with a distance: measured from here to the town's centre
	float m_fMinDist;			// 0 = no limit
	float m_fMaxDist;			// 0 = no limit
}

//------------------------------------------------------------------------------------------------
//! A rule that makes one canon fact, for keys the built-in rules don't know. Return the value
//! as text: a number, a settlement index, RZ_WorldFacts.PosText(pos), or plain words. "" = can't
//! make it now (asked again next time).
class RZ_CanonRule
{
	string Make(RZ_GameModeComponent gm, RZ_WorldFacts facts, RZ_Rng rng)
	{
		return "";
	}
}

//------------------------------------------------------------------------------------------------
class RZ_WorldFactsSave
{
	int seed;
	float clock;
	ref array<string> flagKeys = {};
	ref array<int> flagValues = {};
	ref array<string> canonKeys = {};
	ref array<string> canonValues = {};
	ref array<string> rumours = {};
	ref array<float> rumourUntil = {};
	// World memory (World/RZ_WorldMemory.c): reasons, the signal ring, the town log, world records.
	// Older saves have none: it starts empty
	ref RZ_WorldMemorySave memory;
}

//------------------------------------------------------------------------------------------------
class RZ_WorldFacts
{
	// Claim subjects without an index
	static const string LAB = "lab";
	static const string HELI = "heli";
	static const string PIT = "pit";

	// Season flags the layer itself reads
	static const string FLAG_HELI_DOWN_UNTIL = "heli.down_until";	// the helicopter is down through this day
	static const string FLAG_HELI_LOST = "heli.lost";				// 1 = gone for the season

	// The lore's dates (docs/LORE.md): Command's last signal came on Day 3
	static const int COMMAND_LAST_DAY = 3;

	// Time of day (game clock), as Territory's routine and RZ_GameTime count it
	static const float DUSK_START = 18.5;

	protected static const int MAX_RUMOURS = 8;
	protected static const float UNREAD_REACH = 120;	// a holding's camp unread: it may move 90 m into a building, its pieces round it
	protected static const float HOUSE_REACH = 60;		// a safehouse's camp unread: 30 m, and its pieces
	protected static const float POST_REACH = 15;		// a road post's barrier, crew and pieces
	protected static const string CURE_OWNER = "cure";

	protected static ref RZ_WorldFacts s_Empty;

	protected RZ_GameModeComponent m_GM;
	protected int m_iSeed;
	protected float m_fClock;		// play clock: runs only while someone is online (rumours)
	protected ref map<string, int> m_mFlags = new map<string, int>();
	protected ref map<string, string> m_mCanon = new map<string, string>();
	protected ref array<string> m_aMaking = {};		// canon keys being made right now (CanonText)
	protected ref map<string, string> m_mClaims = new map<string, string>();	// subject -> owner
	protected ref map<string, string> m_mSoft = new map<string, string>();		// subject -> errand (soft holds)
	protected ref array<string> m_aRumours = {};
	protected ref array<float> m_aRumourUntil = {};
	protected ref RZ_WorldFactsSave m_Restored;
	protected ref RZ_WorldMemory m_Memory = new RZ_WorldMemory();
	protected ref RZ_SeasonMood m_Mood;

	//------------------------------------------------------------------------------------------------
	void RZ_WorldFacts(RZ_GameModeComponent gm)
	{
		m_Mood = new RZ_SeasonMood(gm);
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! What the Zone remembers this season: reasons, the signal ring, the town log, world records.
	RZ_WorldMemory Memory()
	{
		return m_Memory;
	}

	//------------------------------------------------------------------------------------------------
	//! The server's facts, or an empty stand-in that answers every query with its default.
	static RZ_WorldFacts Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetFacts())
			return gm.GetFacts();

		if (!s_Empty)
			s_Empty = new RZ_WorldFacts(null);

		return s_Empty;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_WorldFactsSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	//! After the state loaded (or didn't): a fresh season gets a new seed and nothing else.
	void Setup()
	{
		if (m_Restored)
		{
			m_iSeed = m_Restored.seed;
			m_fClock = m_Restored.clock;
			if (m_Restored.flagKeys && m_Restored.flagValues)
			{
				for (int f = 0; f < m_Restored.flagKeys.Count() && f < m_Restored.flagValues.Count(); f++)
				{
					m_mFlags.Set(m_Restored.flagKeys[f], m_Restored.flagValues[f]);
				}
			}

			if (m_Restored.canonKeys && m_Restored.canonValues)
			{
				for (int c = 0; c < m_Restored.canonKeys.Count() && c < m_Restored.canonValues.Count(); c++)
				{
					m_mCanon.Set(m_Restored.canonKeys[c], m_Restored.canonValues[c]);
				}
			}

			if (m_Restored.rumours && m_Restored.rumourUntil)
			{
				for (int r = 0; r < m_Restored.rumours.Count() && r < m_Restored.rumourUntil.Count(); r++)
				{
					m_aRumours.Insert(m_Restored.rumours[r]);
					m_aRumourUntil.Insert(m_Restored.rumourUntil[r]);
				}
			}

			m_Memory.Restore(m_Restored.memory);
		}

		m_Restored = null;
		if (m_iSeed == 0)
			m_iSeed = Math.RandomInt(1, 2000000000);

		RZ_Log.Info(string.Format("World facts: season seed %1, %2 canon facts, %3 season flags.", m_iSeed, m_mCanon.Count(), m_mFlags.Count()));
	}

	//------------------------------------------------------------------------------------------------
	RZ_WorldFactsSave Save()
	{
		RZ_WorldFactsSave data = new RZ_WorldFactsSave();
		data.seed = m_iSeed;
		data.clock = m_fClock;
		foreach (string fkey, int fvalue : m_mFlags)
		{
			data.flagKeys.Insert(fkey);
			data.flagValues.Insert(fvalue);
		}

		foreach (string ckey, string cvalue : m_mCanon)
		{
			data.canonKeys.Insert(ckey);
			data.canonValues.Insert(cvalue);
		}

		foreach (int i, string rumour : m_aRumours)
		{
			data.rumours.Insert(rumour);
			data.rumourUntil.Insert(m_aRumourUntil[i]);
		}

		data.memory = m_Memory.Save();
		return data;
	}

	//------------------------------------------------------------------------------------------------
	//! Server tick: the gossip clock, and the claims the systems hold themselves (the lab during
	//! the final stand).
	void Tick(float dt, array<int> ids)
	{
		if (!m_GM)
			return;

		if (!ids.IsEmpty())
		{
			m_fClock += dt;
			ExpireRumours();
			m_Memory.Tick();
			m_Mood.Tick(dt);
		}

		// The lab is held during the stand, and by the cure finale's controller during the batch
		// hold (its route stages never claim it; docs/FINALE.md)
		RZ_CureResearch cure = m_GM.GetCure();
		RZ_CureFinale finale = RZ_CureFinale.Get();
		bool hold = finale && finale.IsOpen() && finale.IsFrozen();
		if (cure && (cure.IsFinalStand() || hold))
			Claim(LAB, CURE_OWNER);
		else
			Release(LAB, CURE_OWNER);
	}

	// ================================================================ The season mood
	//------------------------------------------------------------------------------------------------
	//! The season mood (Director/RZ_SeasonMood.c): Epilogue, Hope, Bleak, Grim or Early, with its
	//! holding rules. Every voice and the broadcast read it.
	RZ_EMood SeasonMood()
	{
		return m_Mood.Get();
	}

	//------------------------------------------------------------------------------------------------
	//! The mood a faction's own lines use: the season mood moved one step by its morale.
	RZ_EMood MoodFor(int faction)
	{
		return m_Mood.For(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Voice > Season mood on: the broadcast reads the mood's ladder.
	bool SeasonMoodOn()
	{
		return RZ_VoiceSettings.Get().m_bSeasonMood;
	}

	// ================================================================ The cure and the season
	//------------------------------------------------------------------------------------------------
	//! The final stand was won: the cure is out (docs/LORE.md, "Endings").
	bool IsCured()
	{
		if (!m_GM || !m_GM.GetCure())
			return false;

		return m_GM.GetCure().IsCured();
	}

	//------------------------------------------------------------------------------------------------
	//! The season is over and waits for its wipe: cured (the epilogue), or out of time.
	bool IsEpilogue()
	{
		if (IsCured())
			return true;

		if (!m_GM || !m_GM.GetSeasons())
			return false;

		return m_GM.GetSeasons().IsWipeDue();
	}

	//------------------------------------------------------------------------------------------------
	//! No cure, and an After-weeks wipe is under Season > Warn hours away (the Unsigned last day).
	//! IsEpilogue stays false: research and the cure can still come.
	bool IsLastDay()
	{
		if (IsCured() || !m_GM || !m_GM.GetSeasons())
			return false;

		return m_GM.GetSeasons().IsLastDay();
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue with Epilogue > The world settles: no holding changes hands (docs/EPILOGUE.md).
	bool HoldingsFrozen()
	{
		RZ_Epilogue epi = RZ_Epilogue.Get();
		return epi && epi.HoldingsFrozen();
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand is running at the lab.
	bool IsFinalStand()
	{
		if (!m_GM || !m_GM.GetCure())
			return false;

		return m_GM.GetCure().IsFinalStand();
	}

	//------------------------------------------------------------------------------------------------
	//! Research the lab has worked through, 0 to 100 (the milestones follow it).
	float CurePercent()
	{
		if (!m_GM || !m_GM.GetCure())
			return 0;

		// With the finale's routes open: 90 plus the furthest track (RZ_CureResearch.ShownProgress)
		return m_GM.GetCure().ShownProgress();
	}

	//------------------------------------------------------------------------------------------------
	//! Research delivered and still waiting in the lab's backlog, in percent.
	float LabWaiting()
	{
		if (!m_GM || !m_GM.GetCure())
			return 0;

		return m_GM.GetCure().GetBacklog();
	}

	//------------------------------------------------------------------------------------------------
	//! The lab's backlog is full: it refuses samples, and mission research beyond it is lost.
	bool LabFull()
	{
		if (!m_GM || !m_GM.GetCure())
			return false;

		return m_GM.GetCure().IsLabFull();
	}

	//------------------------------------------------------------------------------------------------
	//! Research waits at 90 percent for a cure finale (only while a finale questline is registered).
	bool AtFinaleGate()
	{
		if (!m_GM || !m_GM.GetCure())
			return false;

		return m_GM.GetCure().IsAtFinaleGate();
	}

	//------------------------------------------------------------------------------------------------
	//! Doses on the Red Hand's black market.
	int StolenDoses()
	{
		if (!m_GM || !m_GM.GetCure())
			return 0;

		return m_GM.GetCure().GetStolenAntidotes();
	}

	//------------------------------------------------------------------------------------------------
	RZ_EOutbreakStage Stage()
	{
		if (!m_GM || !m_GM.GetDirector())
			return RZ_EOutbreakStage.EARLY_OUTBREAK;

		return m_GM.GetDirector().GetStage();
	}

	//------------------------------------------------------------------------------------------------
	//! How much of Bastion is still out on the roads, 0 to 1 (the stage table's Bastion presence).
	float BastionPresence()
	{
		if (!m_GM || !m_GM.GetDirector())
			return 1;

		return m_GM.GetDirector().GetStageSettings().m_fMilitaryPresence;
	}

	// ================================================================ Time
	//------------------------------------------------------------------------------------------------
	//! The day as the broadcast counts it ("Day 14"): days since the gates closed.
	int Day()
	{
		if (!m_GM || !m_GM.GetBroadcast())
			return 11;

		return m_GM.GetBroadcast().GetDay();
	}

	//------------------------------------------------------------------------------------------------
	//! Whole days since `day` (0 on the day itself).
	int DaysSince(int day)
	{
		return Math.Max(0, Day() - day);
	}

	//------------------------------------------------------------------------------------------------
	//! Days since Command's last signal (Day 3, docs/LORE.md): "Command hasn't called in N days".
	int DaysSinceCommand()
	{
		return DaysSince(COMMAND_LAST_DAY);
	}

	//------------------------------------------------------------------------------------------------
	//! The game clock knows the time (a time and weather manager exists).
	bool KnowsTime()
	{
		return RZ_GameTime.GetHour() >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! 20:30 to 05:00 on the game clock. False if the clock is unknown.
	bool IsNight()
	{
		return RZ_GameTime.IsNight();
	}

	//------------------------------------------------------------------------------------------------
	//! 18:30 to 20:30: the dusk meal, Bastion's curfew. False if the clock is unknown.
	bool IsDusk()
	{
		float h = RZ_GameTime.GetHour();
		if (h < 0)
			return false;

		return h >= DUSK_START && h < RZ_GameTime.NIGHT_START;
	}

	//------------------------------------------------------------------------------------------------
	//! Daylight: neither dusk nor night. False if the clock is unknown.
	bool IsDaytime()
	{
		return KnowsTime() && !IsNight() && !IsDusk();
	}

	// ================================================================ The Perimeter and the helicopter
	//------------------------------------------------------------------------------------------------
	//! The cordon is up and shooting: the Perimeter runs and the cure isn't out. After the cure the
	//! line stands down (no drones, no shots) and the gates open with the wipe.
	bool PerimeterSealed()
	{
		if (!m_GM || !m_GM.GetPerimeter() || !m_GM.GetPerimeter().IsActive())
			return false;

		return !IsCured();
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's last helicopter can fly: no storm, not down (season flag heli.down_until is a day
	//! number, heli.lost = 1 for good).
	bool HeliAvailable()
	{
		if (Flag(FLAG_HELI_LOST) > 0)
			return false;

		if (Flag(FLAG_HELI_DOWN_UNTIL) >= Day())
			return false;

		if (m_GM && m_GM.GetEnvironment() && m_GM.GetEnvironment().IsHelicopterGrounded())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The helicopter is down for `days` more days (today counts), or for good with days < 0.
	void SetHeliDown(int days)
	{
		if (days < 0)
		{
			SetFlag(FLAG_HELI_LOST, 1);
			return;
		}

		SetFlag(FLAG_HELI_DOWN_UNTIL, Math.Max(Flag(FLAG_HELI_DOWN_UNTIL), Day() + days - 1));
	}

	// ================================================================ Towns
	//------------------------------------------------------------------------------------------------
	//! Who holds a town: the lab, a faction holding (stronghold or outpost), a Haven safehouse, or
	//! nobody. The lab wins, then a holding, then a safehouse.
	RZ_TownOwner TownOwner(int idx)
	{
		RZ_TownOwner o = new RZ_TownOwner();
		o.m_iSettlement = idx;
		if (!m_GM)
			return o;

		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return o;

		o.m_sTown = s.m_sName;
		if (m_GM.HasLab() && m_GM.GetLabSettlement() == idx)
		{
			o.m_iFaction = RZ_EFaction.INSTITUTE;
			o.m_eKind = RZ_ETownKind.LAB;
			return o;
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			RZ_Holding h = territory.FindInTown(idx);
			if (h)
			{
				o.m_iFaction = h.m_eFaction;
				o.m_iRef = h.m_iId;
				if (h.m_bStronghold)
					o.m_eKind = RZ_ETownKind.STRONGHOLD;
				else
					o.m_eKind = RZ_ETownKind.OUTPOST;

				return o;
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse house = houses.Get(i);
				if (house && house.m_iSettlement == idx)
				{
					o.m_iFaction = RZ_EFaction.SURVIVORS;
					o.m_eKind = RZ_ETownKind.SAFEHOUSE;
					o.m_iRef = i;
					return o;
				}
			}
		}

		return o;
	}

	//------------------------------------------------------------------------------------------------
	//! People live here or a faction holds ground here: a safehouse, a holding, the lab.
	bool IsLivedIn(int idx)
	{
		return TownOwner(idx).IsHeld();
	}

	//------------------------------------------------------------------------------------------------
	//! "the Haven", "Bastion", "the Red Hand", "Halcyon", or "nobody".
	string OwnerText(int idx)
	{
		RZ_TownOwner o = TownOwner(idx);
		if (!o.IsHeld())
			return "nobody";

		return RZ_Factions.GetNameInText(o.m_iFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! A bit for RZ_TownFilter.m_iKinds.
	static int KindBit(RZ_ETownKind kind)
	{
		return 1 << kind;
	}

	//------------------------------------------------------------------------------------------------
	//! Every settlement the filter allows that no other mission or story is using (with `asker`,
	//! what it or the story that started it holds counts as free). Filter here, not after a random
	//! pick: on a map where most towns are held, a pick-then-check fails.
	void CollectTowns(RZ_TownFilter filter, notnull array<int> result, RZ_Mission asker = null)
	{
		if (!m_GM || !filter)
			return;

		RZ_MissionManager missions = m_GM.GetMissions();
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (i == filter.m_iExclude || s.m_iSize < filter.m_iMinSize)
				continue;

			if (s.m_fInfection < filter.m_fMinInfection || s.m_fInfection > filter.m_fMaxInfection)
				continue;

			if (filter.m_fMinDist > 0 || filter.m_fMaxDist > 0)
			{
				float dist = RZ_Util.DistXZ(filter.m_vFrom, s.m_vCenter);
				if (dist < filter.m_fMinDist || (filter.m_fMaxDist > 0 && dist > filter.m_fMaxDist))
					continue;
			}

			if (missions && missions.IsSettlementBusy(i, asker))
				continue;

			RZ_TownOwner o = TownOwner(i);
			if (filter.m_iHeldBy == -1 && o.IsHeld())
				continue;

			if (filter.m_iHeldBy >= 0 && !o.IsHeldBy(filter.m_iHeldBy))
				continue;

			if (filter.m_iNotHeldBy != 0 && o.IsHeld() && RZ_Factions.IsValid(o.m_iFaction) && (filter.m_iNotHeldBy & RZ_Factions.Bit(o.m_iFaction)) != 0)
				continue;

			if (filter.m_iKinds != 0 && (filter.m_iKinds & KindBit(o.m_eKind)) == 0)
				continue;

			result.Insert(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A random settlement from CollectTowns, -1 if there's none.
	int PickTown(RZ_TownFilter filter, RZ_Mission asker = null)
	{
		array<int> towns = {};
		CollectTowns(filter, towns, asker);
		if (towns.IsEmpty())
			return -1;

		return towns.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Lights at pos: the power grid's region, a power surge, or a running generator close by.
	bool IsPowered(vector pos)
	{
		return RZ_PowerGrid.IsPoweredAt(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! People live within `radius` of pos: a Haven safehouse, a faction holding, a road post, or the
	//! lab. A camp counts from its yard and as far as its pieces reach (RZ_CampLife.SiteReachOf); one
	//! whose building isn't read yet, from its middle plus the most a camp may reach (UNREAD_REACH).
	//! What shelling, fire, scenes and other harm keep away from (Bastion's Fire Mission).
	bool IsNearLivedIn(vector pos, float radius)
	{
		if (!m_GM)
			return false;

		RZ_CampLife camp = m_GM.GetCampLife();
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			if (houses.FindNearest(pos, radius) >= 0)
				return true;

			int near = houses.FindNearest(pos, radius + HOUSE_REACH);
			if (near >= 0 && NearCamp(camp, "s" + near.ToString(), houses.Get(near).m_vPos, pos, radius, HOUSE_REACH))
				return true;
		}

		if (m_GM.HasLab() && RZ_Util.DistXZ(pos, m_GM.GetLabPosition()) < radius)
			return true;

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				float d = RZ_Util.DistXZ(h.m_vPos, pos);
				if (d < radius)
					return true;

				if (d < radius + UNREAD_REACH && NearCamp(camp, "h" + h.m_iId.ToString(), h.m_vPos, pos, radius, UNREAD_REACH))
					return true;
			}
		}

		RZ_Checkpoints checkpoints = m_GM.GetCheckpoints();
		if (checkpoints)
		{
			array<RZ_RoadPost> posts = {};
			checkpoints.PostsNear(-1, pos, radius + POST_REACH, posts);
			if (!posts.IsEmpty())
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! pos is within radius of the camp `key` (at `middle`) as its pieces reach; unread, unread more.
	protected bool NearCamp(RZ_CampLife camp, string key, vector middle, vector pos, float radius, float unread)
	{
		vector home;
		float reach;
		if (camp && camp.SiteReachOf(key, home, reach))
			return RZ_Util.DistXZ(home, pos) < radius + reach;

		return RZ_Util.DistXZ(middle, pos) < radius + unread;
	}

	// ================================================================ People
	//------------------------------------------------------------------------------------------------
	//! A safehouse as it is right now (m_bValid false if there's no such house).
	RZ_HouseFacts House(int idx)
	{
		RZ_HouseFacts f = new RZ_HouseFacts();
		f.m_iIndex = idx;
		if (!m_GM || !m_GM.GetSafehouses())
			return f;

		RZ_Safehouse h = m_GM.GetSafehouses().Get(idx);
		if (!h)
			return f;

		f.m_bValid = true;
		f.m_iSettlement = h.m_iSettlement;
		f.m_sTown = h.m_sName;
		f.m_vPos = h.m_vPos;
		f.m_iPeople = h.m_iPopulation;
		f.m_iFood = h.m_iFood;
		f.m_bShort = h.m_bShort;
		f.m_bHasMedic = h.HasMedic();
		f.m_bHasRadio = h.HasRadio();
		f.m_iLost = h.m_iLost;
		f.m_iDied = h.m_iDied;
		f.m_bEmpty = h.m_iPopulation <= 0;
		f.m_sTier = RZ_Safehouse.TierName(h.GetTier());
		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! The leader in this seat now ("" if none). Seats keep their id when someone new takes the
	//! chair, so read the name when you write the line, never copy it at setup.
	string LeaderName(int leaderId)
	{
		RZ_Leader l = FindLeader(leaderId);
		if (!l || !l.IsPresent())
			return "";

		return l.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone sits in this seat. With generation >= 0 (LeaderGeneration when you picked them),
	//! false once that person is gone and someone new has the chair.
	bool LeaderAlive(int leaderId, int generation = -1)
	{
		RZ_Leader l = FindLeader(leaderId);
		if (!l || !l.IsPresent())
			return false;

		return generation < 0 || l.m_iGeneration == generation;
	}

	//------------------------------------------------------------------------------------------------
	//! Which person in this seat (rises by one with each replacement), -1 if there's no seat.
	int LeaderGeneration(int leaderId)
	{
		RZ_Leader l = FindLeader(leaderId);
		if (!l)
			return -1;

		return l.m_iGeneration;
	}

	//------------------------------------------------------------------------------------------------
	//! In the seat, not used by a mission (held) and not claimed by a story.
	bool LeaderFree(int leaderId)
	{
		RZ_Leader l = FindLeader(leaderId);
		if (!l || !l.IsPresent() || l.m_iHeldBy >= 0)
			return false;

		return !IsClaimed(Leader(leaderId));
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Leader FindLeader(int leaderId)
	{
		if (!m_GM || !m_GM.GetLeaders())
			return null;

		return m_GM.GetLeaders().Get(leaderId);
	}

	//------------------------------------------------------------------------------------------------
	//! Alive, not held by a mission and not claimed by a story.
	bool WandererFree(int wandererId)
	{
		if (!m_GM || !m_GM.GetWanderers() || !m_GM.GetWanderers().IsAlive(wandererId))
			return false;

		if (m_GM.GetWanderers().IsHeld(wandererId))
			return false;

		return !IsClaimed(Wanderer(wandererId));
	}

	// ================================================================ Season flags
	//------------------------------------------------------------------------------------------------
	//! A season flag, `def` if never set. Name them "<owner>.<flag>" ("heli.down_until",
	//! "haven_short_weight.ending"). The questlines' world flags are these.
	int Flag(string key, int def = 0)
	{
		int v;
		if (m_mFlags.Find(key, v))
			return v;

		return def;
	}

	//------------------------------------------------------------------------------------------------
	void SetFlag(string key, int value)
	{
		if (key.IsEmpty())
			return;

		m_mFlags.Set(key, value);
	}

	//------------------------------------------------------------------------------------------------
	bool HasFlag(string key)
	{
		return m_mFlags.Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	void ClearFlag(string key)
	{
		m_mFlags.Remove(key);
	}

	// ================================================================ Canon
	//------------------------------------------------------------------------------------------------
	//! True once the fact has been made this season.
	bool HasCanon(string key)
	{
		return m_mCanon.Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	//! Set a fact you worked out yourself. Only the first value counts: later calls are ignored,
	//! so call it only when !HasCanon(key), or use a rule.
	void SetCanon(string key, string value)
	{
		if (key.IsEmpty() || value.IsEmpty() || m_mCanon.Contains(key))
			return;

		m_mCanon.Set(key, value);
		RZ_Log.Info(string.Format("Canon: %1 = %2.", key, value));
	}

	//------------------------------------------------------------------------------------------------
	//! The fact as text: made once (by `rule`, else the built-in rule for the key), then the same
	//! all season. "" if it can't be made yet. A rule that asks for the fact it is making (directly
	//! or through another fact) gets "" instead of going round for ever.
	string CanonText(string key, RZ_CanonRule rule = null)
	{
		string v;
		if (m_mCanon.Find(key, v))
			return v;

		if (m_aMaking.Contains(key))
			return "";

		m_aMaking.Insert(key);
		RZ_Rng rng = KeyRng(key);
		if (rule)
			v = rule.Make(m_GM, this, rng);
		else
			v = DefaultRule(key, rng);

		m_aMaking.RemoveItem(key);
		SetCanon(key, v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! A number from min to max (both included), the same all season. A built-in fact keeps its
	//! own range (origin.told is 3-24) and a value made earlier is kept, but what comes back is
	//! always inside min..max.
	int CanonInt(string key, int min, int max)
	{
		string v = CanonText(key);
		if (v.IsEmpty())
		{
			v = KeyRng(key).Int(min, max + 1).ToString();
			SetCanon(key, v);
		}

		return Math.ClampInt(v.ToInt(min), min, max);
	}

	//------------------------------------------------------------------------------------------------
	//! One of `options`, the same all season (the text is kept, so later edits to the list don't
	//! change a season already running).
	string CanonPick(string key, notnull array<string> options)
	{
		string v;
		if (m_mCanon.Find(key, v))
			return v;

		v = KeyRng(key).Pick(options);
		SetCanon(key, v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! A settlement index, the same all season. -1 if none can be made. The built-in rule (or a
	//! town nobody lives in) unless `rule` says otherwise. Check TownOwner when you use it: a
	//! town can change hands during the season.
	int CanonSettlement(string key, RZ_CanonRule rule = null)
	{
		string v = CanonText(key, rule);
		if (v.IsEmpty())
		{
			int pick = AnyQuietTown(KeyRng(key));
			if (pick < 0)
				return -1;

			v = pick.ToString();
			SetCanon(key, v);
		}

		int idx = v.ToInt(-1);
		if (!m_GM || !m_GM.GetSettlement(idx))
			return -1;

		return idx;
	}

	//------------------------------------------------------------------------------------------------
	//! A spot on the ground, the same all season. vector.Zero if none can be made. Without a rule
	//! or a built-in one, a dry spot just outside CanonSettlement(key).
	vector CanonPos(string key, RZ_CanonRule rule = null)
	{
		string v = CanonText(key, rule);
		if (v.IsEmpty() && m_GM)
		{
			RZ_Settlement s = m_GM.GetSettlement(CanonSettlement(key + ".town"));
			if (s)
			{
				v = PosText(RZ_Mission.LandPointNear(s.m_vCenter, s.m_fRadius * 0.9, s.m_fRadius + 100));
				SetCanon(key, v);
			}
		}

		return ParsePos(v);
	}

	//------------------------------------------------------------------------------------------------
	//! "1234.5 6789.0": how a position is kept as canon text.
	static string PosText(vector pos)
	{
		return string.Format("%1 %2", pos[0], pos[2]);
	}

	//------------------------------------------------------------------------------------------------
	static vector ParsePos(string text)
	{
		if (text.IsEmpty())
			return vector.Zero;

		array<string> parts = {};
		text.Split(" ", parts, true);
		if (parts.Count() < 2)
			return vector.Zero;

		return RZ_Util.GroundPos(Vector(parts[0].ToFloat(), 0, parts[1].ToFloat()));
	}

	//------------------------------------------------------------------------------------------------
	//! The same generator for the same key all season.
	protected RZ_Rng KeyRng(string key)
	{
		return new RZ_Rng(m_iSeed + key.Hash());
	}

	//------------------------------------------------------------------------------------------------
	//! The built-in rules for the setting's fixed facts (docs/DESIGN.md, "World facts"). "" for
	//! keys it doesn't know.
	protected string DefaultRule(string key, RZ_Rng rng)
	{
		if (!m_GM)
			return "";

		switch (key)
		{
			case "origin.site": return OriginSite(rng);
			case "origin.site.town":
			{
				// Read the site as text: CanonPos would fall back to this very key
				string siteText = CanonText("origin.site");
				if (siteText.IsEmpty())
					return "";

				int siteTown = m_GM.FindNearestSettlement(ParsePos(siteText));
				if (siteTown < 0)
					return "";

				return siteTown.ToString();
			}
			case "origin.clinic": return OriginClinic(rng);
			case "origin.patient": return RZ_NoteText.Surname(rng);
			case "origin.told": return rng.Int(3, 25).ToString();
			case "origin.told.month": return rng.Int(1, 13).ToString();
			// Pinned (the majors, Await Relief): the Day 3 orders said "expended". "Command meant to
			// come and couldn't" is only what some posts believe, in rumours
			case "bastion.orders": return "expended";
			case "redhand.crossroads": return RedHandCrossroads();
			case "day2.checkpoint": return Day2Checkpoint();
			case "redhand.pit": return RedHandPit(rng);
			case "redhand.pit.seat":
			{
				// Made with the pit; -1 when the pit isn't behind a crew boss's seat
				CanonText("redhand.pit");
				string seat;
				if (m_mCanon.Find("redhand.pit.seat", seat))
					return seat;

				return "-1";
			}
			case "redhand.pit.town":
			{
				string pitText = CanonText("redhand.pit");
				if (pitText.IsEmpty())
					return "";

				int pitTown = m_GM.FindNearestSettlement(ParsePos(pitText));
				if (pitTown < 0)
					return "";

				return pitTown.ToString();
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The closed site in the hills: a high, dry spot between two towns, well clear of every
	//! settlement and the lab, at a real factory, mast or barracks there if there is one, and never
	//! nearest the lab's own town (The Three Weeks' rules). Falls back to high ground outside the
	//! town farthest from the lab, checked the same way where it can be.
	protected string OriginSite(RZ_Rng rng)
	{
		array<ref RZ_Settlement> all = m_GM.GetSettlements();
		int n = all.Count();
		if (n == 0)
			return "";

		array<string> kinds = {"military", "industrial", "radio", "power", "tower"};
		int labTown = -1;
		if (m_GM.HasLab())
			labTown = m_GM.GetLabSettlement();

		for (int t = 0; t < 40; t++)
		{
			float need = 450;
			if (t >= 25)
				need = 300;

			int ia = rng.Int(0, n);
			int ib = rng.Int(0, n);
			if (ia == ib)
				continue;

			vector ca = all[ia].m_vCenter;
			vector cb = all[ib].m_vCenter;
			float gap = RZ_Util.DistXZ(ca, cb);
			if (gap < 1400 || gap > 6000)
				continue;

			vector line = cb - ca;
			vector side = Vector(-line[2], 0, line[0]) * (1 / gap);
			vector mid = ca + line * rng.Range(0.4, 0.6) + side * rng.Range(-gap * 0.2, gap * 0.2);
			vector p = RZ_Mission.HighPointNear(RZ_Util.GroundPos(mid), 0, 200, 12);
			if (!IsRemote(p, need))
				continue;

			// A real building to have been the site, if one stands out there
			RZ_Landmark mark = RZ_LandmarkFinder.FindNearest(p, 400, kinds);
			if (mark && IsRemote(mark.m_vPos, need - 100))
				p = RZ_Mission.LandPointNear(mark.m_vPos, 4, 10);

			int nearest = m_GM.FindNearestSettlement(p);
			if (nearest < 0 || nearest == labTown)
				continue;

			return PosText(p);
		}

		int farthest = -1;
		float farDist = -1;
		vector lab = m_GM.GetLabPosition();
		foreach (int i, RZ_Settlement s : all)
		{
			float d = RZ_Util.DistXZ(s.m_vCenter, lab);
			if (d > farDist)
			{
				farDist = d;
				farthest = i;
			}
		}

		RZ_Settlement edge = m_GM.GetSettlement(farthest);
		if (!edge)
			return "";

		// Remote if it can be, else at least dry, inside the line and not nearest the lab's town
		vector fallback = vector.Zero;
		for (int f = 0; f < 12; f++)
		{
			vector q = RZ_Mission.HighPointNear(edge.m_vCenter, edge.m_fRadius + 400, edge.m_fRadius + 900);
			if (m_GM.FindNearestSettlement(q) == labTown)
				continue;

			if (IsRemote(q, 300))
				return PosText(q);

			if (fallback == vector.Zero && !RZ_Util.IsWater(q) && !RZ_Perimeter.IsNearLineAt(q))
				fallback = q;
		}

		if (fallback == vector.Zero)
			fallback = RZ_Mission.LandPointNear(edge.m_vCenter, edge.m_fRadius + 400, edge.m_fRadius + 900);

		return PosText(fallback);
	}

	//------------------------------------------------------------------------------------------------
	//! Dry, inside the line, not behind a lock, at least `gap` past every settlement's edge and
	//! 1.5 km from the lab.
	protected bool IsRemote(vector p, float gap)
	{
		if (RZ_Util.IsWater(p) || RZ_Perimeter.IsNearLineAt(p) || RZ_LockedSpots.IsBlocked(p))
			return false;

		if (m_GM.HasLab() && RZ_Util.DistXZ(p, m_GM.GetLabPosition()) < 1500)
			return false;

		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			if (RZ_Util.DistXZ(p, s.m_vCenter) < s.m_fRadius + gap)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Rudi's crossroads (the Red Hand's major), a spot: the road junction nearest the Red Hand
	//! stronghold's seat, 300-800 m out (outside the camp), not within 400 m of a safehouse or the lab
	//! and off the line. A junction is the road point with the most other road points within 30 m.
	//! Without roads: a dry spot 300-800 m from the seat.
	protected string RedHandCrossroads()
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (!leaders)
			return "";

		RZ_Leader boss = leaders.GetStronghold(RZ_EFaction.BANDITS);
		if (!boss)
			return "";

		array<vector> points = {};
		RZ_Perimeter.RoadPointsNear(boss.m_vPos, 800, points);
		vector best = vector.Zero;
		int bestCount = -1;
		RZ_Safehouses houses = m_GM.GetSafehouses();
		for (int i = 0; i < points.Count(); i += 3)
		{
			vector p = points[i];
			float d = RZ_Util.DistXZ(p, boss.m_vPos);
			if (d < 300 || d > 800 || RZ_Perimeter.IsNearLineAt(p))
				continue;

			if (m_GM.HasLab() && RZ_Util.DistXZ(p, m_GM.GetLabPosition()) < 400)
				continue;

			if (houses && houses.FindNearest(p, 400) >= 0)
				continue;

			int near = 0;
			foreach (vector q : points)
			{
				if (RZ_Util.DistXZ(p, q) <= 30)
					near++;
			}

			if (near > bestCount)
			{
				bestCount = near;
				best = p;
			}
		}

		if (best == vector.Zero)
			best = RZ_Mission.LandPointNear(boss.m_vPos, 300, 800);

		return PosText(best);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the Day 2 column was turned back (the majors' codas), a spot: the sealed road checkpoint
	//! nearest the Haven stronghold's safehouse, preferring the north. Without checkpoints, the
	//! stronghold safehouse's gate. The stand point and the paper's table are measured from it
	//! (RZ_Majors.NorthRoad).
	protected string Day2Checkpoint()
	{
		vector home = vector.Zero;
		int chair = RZ_HavenQuest.ChairHouse(m_GM);
		if (chair >= 0)
			home = m_GM.GetSafehouses().Get(chair).m_vPos;
		else if (m_GM.GetSafehouses() && m_GM.GetSafehouses().Count() > 0)
			home = m_GM.GetSafehouses().Get(0).m_vPos;

		array<vector> posts = {};
		if (m_GM.GetPerimeter())
			m_GM.GetPerimeter().GetCheckpoints(posts);

		vector best = home;
		float bestScore = float.MAX;
		foreach (vector cp : posts)
		{
			// North counts: a checkpoint 1 km further but north of the house scores as close
			float score = RZ_Util.DistXZ(cp, home) - Math.Clamp(cp[2] - home[2], -1000, 1000) * 0.3;
			if (score < bestScore)
			{
				bestScore = score;
				best = cp;
			}
		}

		if (best == vector.Zero)
			return "";

		return PosText(best);
	}

	//------------------------------------------------------------------------------------------------
	//! The clinic the first patient was brought to: the town (or city) nearest the site, not the
	//! lab's. Any other settlement if there's no town.
	protected string OriginClinic(RZ_Rng rng)
	{
		vector site = CanonPos("origin.site");
		int siteTown = CanonSettlement("origin.site.town");
		int lab = m_GM.GetLabSettlement();
		int best = -1;
		float bestDist = float.MAX;
		int fallback = -1;
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (i == lab || i == siteTown)
				continue;

			if (fallback < 0 || rng.Chance(0.3))
				fallback = i;

			if (s.m_iSize < 2)
				continue;

			float d = RZ_Util.DistXZ(s.m_vCenter, site);
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		if (best < 0)
			best = fallback;

		if (best < 0)
			return "";

		return best.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's fight pit, a spot: in the garages behind the boss's seat at the stronghold
	//! (strongholds never fall), else behind a crew boss's seat, else outside the Red Hand's
	//! stronghold or one of its outposts, else outside a quiet town. Also keeps redhand.pit.seat
	//! (that seat's leader id, -1 if none). One pit all season.
	protected string RedHandPit(RZ_Rng rng)
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
		{
			RZ_Leader seat = leaders.GetStronghold(RZ_EFaction.BANDITS);
			if (!seat)
			{
				array<RZ_Leader> crews = {};
				leaders.GetFactionLeaders(RZ_EFaction.BANDITS, crews);
				if (!crews.IsEmpty())
					seat = crews[rng.Int(0, crews.Count())];
			}

			if (seat)
			{
				SetCanon("redhand.pit.seat", seat.m_iId.ToString());
				return PosText(RZ_Mission.LandPointNear(seat.m_vPos, 60, 150));
			}
		}

		SetCanon("redhand.pit.seat", "-1");
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			RZ_Holding top = territory.GetStronghold(RZ_EFaction.BANDITS);
			if (!top)
			{
				array<RZ_Holding> posts = {};
				territory.GetOutposts(RZ_EFaction.BANDITS, posts);
				if (!posts.IsEmpty())
					top = posts[rng.Int(0, posts.Count())];
			}

			if (top)
				return PosText(RZ_Mission.LandPointNear(top.m_vPos, 60, 150));
		}

		RZ_Settlement quiet = m_GM.GetSettlement(AnyQuietTown(rng));
		if (!quiet)
			return "";

		return PosText(RZ_Mission.LandPointNear(quiet.m_vCenter, quiet.m_fRadius * 0.9, quiet.m_fRadius + 100));
	}

	//------------------------------------------------------------------------------------------------
	//! A town nobody lives in, under 70 infection if there is one. -1 without settlements.
	protected int AnyQuietTown(RZ_Rng rng)
	{
		if (!m_GM)
			return -1;

		array<int> quiet = {};
		array<int> any = {};
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			any.Insert(i);
			if (s.m_fInfection <= 70 && !IsLivedIn(i))
				quiet.Insert(i);
		}

		if (!quiet.IsEmpty())
			return quiet[rng.Int(0, quiet.Count())];

		if (!any.IsEmpty())
			return any[rng.Int(0, any.Count())];

		return -1;
	}

	// ================================================================ Claims
	//------------------------------------------------------------------------------------------------
	//! Subjects, so every piece of content spells them the same way.
	static string Town(int settlement) { return "town:" + settlement.ToString(); }
	static string Safehouse(int idx) { return "safehouse:" + idx.ToString(); }
	static string Leader(int leaderId) { return "leader:" + leaderId.ToString(); }
	static string Wanderer(int wandererId) { return "wanderer:" + wandererId.ToString(); }
	static string Truck(int faction) { return "truck:" + faction.ToString(); }

	//------------------------------------------------------------------------------------------------
	//! Owners: a mission, a story arc, a questline (by its key).
	static string MissionOwner(int missionId) { return "mission:" + missionId.ToString(); }
	static string ArcOwner(int arcKind) { return "arc:" + arcKind.ToString(); }
	static string QuestOwner(string questKey) { return "quest:" + questKey; }

	//------------------------------------------------------------------------------------------------
	//! Hold `subject` for `owner`. True if it was free or already this owner's.
	bool Claim(string subject, string owner)
	{
		if (subject.IsEmpty() || owner.IsEmpty())
			return false;

		string holder;
		if (m_mClaims.Find(subject, holder))
			return holder == owner;

		m_mClaims.Set(subject, owner);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Claim for `owner`, unless `parent` (the story that started it) holds it already: then it's
	//! the parent's to share, and true. "" parent = Claim.
	bool ClaimFor(string subject, string owner, string parent)
	{
		if (!parent.IsEmpty() && ClaimedBy(subject) == parent)
			return true;

		return Claim(subject, owner);
	}

	//------------------------------------------------------------------------------------------------
	bool IsClaimed(string subject)
	{
		return m_mClaims.Contains(subject);
	}

	//------------------------------------------------------------------------------------------------
	//! Claimed by someone other than `owner`.
	bool IsClaimedByOther(string subject, string owner)
	{
		string holder;
		if (!m_mClaims.Find(subject, holder))
			return false;

		return holder != owner;
	}

	//------------------------------------------------------------------------------------------------
	//! Who holds it, "" if nobody.
	string ClaimedBy(string subject)
	{
		string holder;
		m_mClaims.Find(subject, holder);
		return holder;
	}

	//------------------------------------------------------------------------------------------------
	//! Let go of one subject (only if `owner` holds it).
	void Release(string subject, string owner)
	{
		string holder;
		if (m_mClaims.Find(subject, holder) && holder == owner)
			m_mClaims.Remove(subject);
	}

	//------------------------------------------------------------------------------------------------
	//! Let go of everything `owner` holds (a mission ended, an arc ended, a questline ended).
	void ReleaseAll(string owner)
	{
		array<string> mine = {};
		foreach (string subject, string holder : m_mClaims)
		{
			if (holder == owner)
				mine.Insert(subject);
		}

		foreach (string gone : mine)
		{
			m_mClaims.Remove(gone);
		}
	}

	// ================================================================ Soft holds
	//------------------------------------------------------------------------------------------------
	//! Soft holds (errands, docs/ERRANDS.md "Keeping them true"): an errand keeps its subject from a
	//! second errand. Kept apart from claims: a soft hold never makes a town busy for anyone, and a
	//! claim on the subject overtakes the errand at its next tick (RZ_Errands). Runtime only.
	//! True if the subject was free or already this errand's.
	bool SoftHold(string subject, string errand)
	{
		if (subject.IsEmpty() || errand.IsEmpty())
			return false;

		string holder;
		if (m_mSoft.Find(subject, holder))
			return holder == errand;

		m_mSoft.Set(subject, errand);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The errand holding `subject` softly, "" if none.
	string SoftHeldBy(string subject)
	{
		string holder;
		m_mSoft.Find(subject, holder);
		return holder;
	}

	//------------------------------------------------------------------------------------------------
	void SoftRelease(string subject, string errand)
	{
		string holder;
		if (m_mSoft.Find(subject, holder) && holder == errand)
			m_mSoft.Remove(subject);
	}

	//------------------------------------------------------------------------------------------------
	void SoftReleaseAll(string errand)
	{
		array<string> mine = {};
		foreach (string subject, string holder : m_mSoft)
		{
			if (holder == errand)
				mine.Insert(subject);
		}

		foreach (string gone : mine)
		{
			m_mSoft.Remove(gone);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The season's seed (the truth deck draws from it, with the story key and the subject).
	int GetSeed()
	{
		return m_iSeed;
	}

	// ================================================================ Gossip
	//------------------------------------------------------------------------------------------------
	//! A line wanderers and leaders repeat for `hours` of play (at most 48). The newest 8 are kept.
	void AddRumour(string text, float hours)
	{
		if (text.IsEmpty() || hours <= 0 || m_aRumours.Contains(text))
			return;

		m_aRumours.Insert(text);
		m_aRumourUntil.Insert(m_fClock + Math.Min(hours, 48) * 3600);
		while (m_aRumours.Count() > MAX_RUMOURS)
		{
			m_aRumours.RemoveOrdered(0);
			m_aRumourUntil.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The rumours still going round, oldest first.
	void GetRumours(notnull array<string> result)
	{
		foreach (string r : m_aRumours)
		{
			result.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Rumours from an older save's questlines (their clock), kept for what's left of their time.
	void AdoptRumour(string text, float hoursLeft)
	{
		AddRumour(text, hoursLeft);
	}

	//------------------------------------------------------------------------------------------------
	protected void ExpireRumours()
	{
		// RemoveOrdered keeps the oldest-first order AddRumour trims by
		for (int i = m_aRumours.Count() - 1; i >= 0; i--)
		{
			if (m_fClock >= m_aRumourUntil[i])
			{
				m_aRumours.RemoveOrdered(i);
				m_aRumourUntil.RemoveOrdered(i);
			}
		}
	}

	// ================================================================ World changes
	//------------------------------------------------------------------------------------------------
	//! Something in the world changed: one call for the headline (server news, announced to
	//! everyone if `announce`), a rumour for `rumourHours`, and a moment the factions remember
	//! (a phrase that fits "Not since %1."). Any of them may be "". The season flag `key` is set
	//! to today's day number, so content can ask HasFlag(key) and DaysSince(Flag(key)).
	void Changed(string key, string headline, string rumour, string moment = "", float rumourHours = 6, bool announce = false)
	{
		if (!m_GM)
			return;

		if (!key.IsEmpty())
			SetFlag(key, Day());

		string news = Fill(headline);
		if (!news.IsEmpty())
		{
			if (m_GM.GetLeaders())
				m_GM.GetLeaders().AddHeadline(news, announce);
			else if (announce)
				m_GM.NotifyAll("Word on the roads", news);
		}

		AddRumour(Fill(rumour), rumourHours);
		if (!moment.IsEmpty() && m_GM.GetFactionLife())
			m_GM.GetFactionLife().RecordMoment(Fill(moment));

		RZ_Log.Info(string.Format("World change: %1. %2", key, news));
	}

	// ================================================================ Text
	//------------------------------------------------------------------------------------------------
	//! Fills the world's tokens into player-facing text, so no brief, rumour or line hard-codes a
	//! fact: {day} (14), {day_text} (Day 14), {days_since_command} (11) and {days_since_command_words}
	//! (eleven), {cure_state}, {cure_percent}, {stage}, {stage_tone}, {time_of_day}, {lab_town},
	//! {stolen_doses}, {town:N}, {owner:N}, {flag:key}, {canon:key}, {canon_town:key},
	//! {leader:id}, {leader_short:id}, {leader_place:id}, {tribe:N}, {alpha:N}, {young:N} (a pack in
	//! a speaker's word: {tribe:N.bastion}). Unknown tokens are left alone (a
	//! questline fills its own first). Mission briefs are filled when shown (RZ_Mission.GetBrief).
	string Fill(string text)
	{
		if (text.IsEmpty() || !text.Contains("{"))
			return text;

		string result = "";
		int at = 0;
		int n = text.Length();
		while (at < n)
		{
			int open = text.IndexOfFrom(at, "{");
			if (open < 0)
			{
				result += text.Substring(at, n - at);
				break;
			}

			int close = text.IndexOfFrom(open, "}");
			if (close < 0)
			{
				result += text.Substring(at, n - at);
				break;
			}

			result += text.Substring(at, open - at);
			string token = text.Substring(open + 1, close - open - 1);
			string value;
			if (Resolve(token, value))
				result += value;
			else
				result += text.Substring(open, close - open + 1);

			at = close + 1;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! One token's text. False if the token isn't a world fact.
	protected bool Resolve(string token, out string value)
	{
		value = "";
		switch (token)
		{
			case "day": value = Day().ToString(); return true;
			case "day_text": value = RZ_NoteText.Day(Day()); return true;
			case "days_since_command": value = DaysSinceCommand().ToString(); return true;
			case "days_since_command_words": value = NumberWords(DaysSinceCommand()); return true;
			case "cure_state": value = CureState(); return true;
			case "cure_percent": value = RZ_Util.RoundToInt(CurePercent()).ToString(); return true;
			case "stage": value = RZ_StageSettings.GetStageName(Stage()); return true;
			case "stage_tone": value = StageTone(); return true;
			case "time_of_day": value = TimeOfDay(); return true;
			case "lab_town": value = LabTown(); return true;
			case "stolen_doses": value = StolenDoses().ToString(); return true;
		}

		int colon = token.IndexOf(":");
		if (colon <= 0)
			return false;

		string kind = token.Substring(0, colon);
		string arg = token.Substring(colon + 1, token.Length() - colon - 1);
		switch (kind)
		{
			case "town":
			{
				if (!m_GM || !m_GM.GetSettlement(arg.ToInt(-1)))
					return false;

				value = m_GM.GetSettlement(arg.ToInt(-1)).m_sName;
				return true;
			}
			case "owner": value = OwnerText(arg.ToInt(-1)); return true;
			case "flag": value = Flag(arg).ToString(); return true;
			case "canon":
			{
				// Text never invents a fact: only one already made, or a built-in one
				if (!HasCanon(arg) && !IsBuiltInCanon(arg))
					return false;

				value = CanonText(arg);
				return !value.IsEmpty();
			}
			case "canon_town":
			{
				if (!HasCanon(arg) && !IsBuiltInCanon(arg))
					return false;

				int canonTown = CanonSettlement(arg);
				if (!m_GM || !m_GM.GetSettlement(canonTown))
					return false;

				value = m_GM.GetSettlement(canonTown).m_sName;
				return true;
			}
			case "tribe": return PackText(arg, 0, value);
			case "alpha": return PackText(arg, 1, value);
			case "young": return PackText(arg, 2, value);
			case "leader": return LeaderText(arg.ToInt(-1), 0, value);
			case "leader_short": return LeaderText(arg.ToInt(-1), 1, value);
			case "leader_place": return LeaderText(arg.ToInt(-1), 2, value);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! {tribe:N}, {alpha:N} and {young:N}, in a speaker's own word ({tribe:N.bastion}): the packs'
	//! people own them (RZ_TribePeople.TokenText). False with the packs off, or no such pack.
	protected bool PackText(string arg, int what, out string value)
	{
		if (!m_GM || !m_GM.GetTribes() || !m_GM.GetTribes().GetPeople().IsOn())
			return false;

		return m_GM.GetTribes().GetPeople().TokenText(arg, what, value);
	}

	//------------------------------------------------------------------------------------------------
	//! The built-in canon keys (DefaultRule knows them).
	static bool IsBuiltInCanon(string key)
	{
		array<string> keys = {"origin.site", "origin.site.town", "origin.clinic", "origin.patient", "origin.told", "origin.told.month",
			"bastion.orders", "redhand.pit", "redhand.pit.town", "redhand.pit.seat", "redhand.crossroads", "day2.checkpoint"};
		return keys.Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	//! A seat's live text for {leader:id} (what 0: the name; 1: the short name; 2: the place). An
	//! empty seat reads as its title ("the outpost commander in Morton"). False if there's no seat.
	protected bool LeaderText(int leaderId, int what, out string value)
	{
		RZ_Leader l = FindLeader(leaderId);
		if (!l)
			return false;

		if (what == 2)
		{
			value = l.m_sPlace;
			return true;
		}

		if (!l.IsPresent())
		{
			value = "the " + l.m_sTitle;
			return true;
		}

		if (what == 1)
			value = l.m_sShort;
		else
			value = l.m_sName;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "the cure is out", "the lab is holding its final stand", "the cure is at 40 percent".
	string CureState()
	{
		if (IsCured())
			return "the cure is out";

		if (IsFinalStand())
			return "the lab is holding its final stand";

		return string.Format("the cure is at %1 percent", RZ_Util.RoundToInt(CurePercent()));
	}

	//------------------------------------------------------------------------------------------------
	//! How the Zone feels, in a few words, by stage (and after the cure).
	string StageTone()
	{
		if (IsCured())
			return "the cure is out and the line has stood down";

		switch (Stage())
		{
			case RZ_EOutbreakStage.EARLY_OUTBREAK: return "the towns are still standing";
			case RZ_EOutbreakStage.COLLAPSE: return "the towns are going dark";
			case RZ_EOutbreakStage.OVERRUN: return "the dead have the roads";
			case RZ_EOutbreakStage.AFTERMATH: return "the Zone has gone quiet";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! "night", "dusk" or "day" ("day" if the clock is unknown).
	string TimeOfDay()
	{
		if (IsNight())
			return "night";

		if (IsDusk())
			return "dusk";

		return "day";
	}

	//------------------------------------------------------------------------------------------------
	//! The lab's town, "the hills" without a lab.
	string LabTown()
	{
		if (!m_GM || !m_GM.HasLab())
			return "the hills";

		RZ_Settlement s = m_GM.GetSettlement(m_GM.GetLabSettlement());
		if (!s)
			return "the hills";

		return s.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! "eleven", "twenty-three"; digits from a hundred on.
	static string NumberWords(int n)
	{
		array<string> small = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten",
			"eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
		array<string> tens = {"", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
		if (n < 0 || n >= 100)
			return n.ToString();

		if (n < 20)
			return small[n];

		string word = tens[n / 10];
		if (n % 10 != 0)
			word += "-" + small[n % 10];

		return word;
	}
}
