//! The Long Table's reach (docs/DESIGN.md, "The Long Table": "Its reach"). The family stays four to
//! seven people in one farmhouse; this is how far it's felt: supper signs at the road junctions, the
//! dinner bell at dusk, a voice saying grace on the open channel, the farm laid out for supper, the
//! host selling jars at a market, the boy asking travellers if they're hungry, one of them lying
//! "hurt" on a quiet road at dusk, their jars in the loot, the dead taken overnight, a plate and fork
//! scratched on a door, and once a season the harvest supper. The hungrier the house, the more of all
//! of it. Everything is data, a line, or a body or a prop near players only; nobody new is taken
//! because of any of it (the takings stay as built, RZ_LongTable.TryTake).
//!
//! Owned by RZ_LongTable and ticked from its tick while the house exists. Server only.

//------------------------------------------------------------------------------------------------
//! World > Long Table > Reach.
[BaseContainerProps()]
class RZ_LongTableReachSettings
{
	[Attribute("1", desc: "The house's reach: signs, the bell, the voice at supper, the farm laid out, the market, the boy, the injured man, the jars, the dead taken, the door mark, the harvest supper (docs/DESIGN.md, The Long Table). Off = only the house as before")]
	bool m_bOn;

	[Attribute("{30A3B16FEE9783B4}Prefabs/Props/Services/ChalkBoard_01/ChalkBoard_01.et", UIWidgets.ResourcePickerThumbnail, "Supper sign: a board at road junctions near the farm (its words are its prompt: SUPPER. ALL WELCOME.). Empty = no signs", "et")]
	ResourceName m_sSignPrefab;

	[Attribute("{0089B23327C9B8CA}Prefabs/Props/Furniture/TableLong_01/TableLong_01.et", UIWidgets.ResourcePickerThumbnail, "The long table outside the farm. Empty = the farm isn't laid out", "et")]
	ResourceName m_sTablePrefab;

	[Attribute("{261F10BDC068BB35}Prefabs/Props/Furniture/ChairOld_01.et", UIWidgets.ResourcePickerThumbnail, "A chair at the long table (one more than anyone needs). Empty = none", "et")]
	ResourceName m_sChairPrefab;

	[Attribute("{799E5AAAB698E210}Prefabs/Props/Garbage/Generic/LitterInterior_01/Single/LitterInterior_USSR_01_bowl.et", UIWidgets.ResourcePickerThumbnail, "A bowl set at each place. Empty = none", "et")]
	ResourceName m_sBowlPrefab;

	[Attribute("{A971B1711AC8958A}Prefabs/Props/Commercial/Posters/Notice_A4_Vertical_V1.et", UIWidgets.ResourcePickerThumbnail, "The name card at the empty chair. Empty = none", "et")]
	ResourceName m_sCardPrefab;

	[Attribute("{8557BF971CC721B0}Prefabs/Props/Furniture/ShoeCabinetOld_01.et", UIWidgets.ResourcePickerThumbnail, "The shoe cabinet by the door, too many boots. Empty = none", "et")]
	ResourceName m_sShoesPrefab;

	[Attribute("{09AAA51D935DAF1E}Prefabs/Props/Furniture/CupboardOld_01/CupboardOld_01_large.et", UIWidgets.ResourcePickerThumbnail, "The cupboard of jars on the farm's ground floor. Empty = none", "et")]
	ResourceName m_sCupboardPrefab;

	[Attribute("{0DED617E66EE8F55}Prefabs/Props/S2/S2_CannedFood/S2_CannedFood.et", UIWidgets.ResourcePickerThumbnail, "Jars on the long table and the cupboard (a look only). Empty = none", "et")]
	ResourceName m_sJarsPropPrefab;

	[Attribute("{862432FB57C1C1B3}Prefabs/Props/Civilian/LampKerosene_01/LampKerosene_01_green.et", UIWidgets.ResourcePickerThumbnail, "Lanterns on the long table from dusk (World > Traces > Kit > Lantern, the lit twin, comes first when set). Empty = no lanterns", "et")]
	ResourceName m_sLanternPrefab;

	[Attribute("{E931C0A6240D4D27}Prefabs/Particles/Smoke/Smoke_Smokestack_Medium.et", UIWidgets.ResourcePickerThumbnail, "Chimney smoke over the farm in the evening (16:00-22:00). Empty = none", "et")]
	ResourceName m_sSmokePrefab;

	[Attribute("{7F3E2C6B9A4D1E05}Prefabs/ReforgerZ/World/RZ_TableJar.et", UIWidgets.ResourcePickerThumbnail, "The family's jar: food with a day on the lid. In loot near the farm, the host's market and the farm's trade; Halcyon can test one. Empty = no jars in loot or at the market", "et")]
	ResourceName m_sJarPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The family's looks (Sunday clothes on Puzzles > Civilian). Empty = the four tools/people_looks.py writes (Prefabs/ReforgerZ/People/LongTable/)", "et")]
	ref array<ResourceName> m_aFamilyLooks;

	[Attribute("30", desc: "The harvest supper: the evening of this broadcast day (Day 11 is the season's first), once a season. 0 = never")]
	int m_iHarvestDay;
}

//------------------------------------------------------------------------------------------------
//! A dead someone near the farm, waiting for the night to be taken (runtime).
class RZ_ReachDead
{
	vector m_vPos;
	string m_sWho;
}

//------------------------------------------------------------------------------------------------
class RZ_LongTableReach
{
	static const int IDX_BASE = 2000;		// RZ_LongTable.TAG indices from here on are the reach's
	static const int IDX_SIGN = 2000;		// + the sign's index
	static const int IDX_MARKET = 2100;
	static const int IDX_BOY = 2101;
	static const int IDX_INJURED = 2102;
	static const int IDX_CARD = 2103;
	static const int IDX_SHOES = 2104;
	static const int REPLY_TEST = 76;		// Halcyon: "Have the jar tested" (RZ_Dialogue)

	protected static const float TICK = 5;
	protected static const float PEOPLE_CHECK = 30;
	protected static const float SIGN_RADIUS = 2000;		// junctions this close to the farm
	protected static const float SIGN_NEAR = 160;			// a sign's prop exists while a player is this close
	protected static const float BELL_REACH = 2500;
	protected static const float BELL_HOUR = 18.5;
	protected static const float VOICE_HOUR = 18.75;
	protected static const float DAWN_HOUR = 5.5;
	protected static const float HARVEST_START = 17.0;
	protected static const float HARVEST_END = 23.0;
	protected static const float DRESS_RADIUS = 300;
	protected static const float MARKET_NEAR = 150;
	protected static const float MARKET_REACH = 5000;		// a market this close to the farm
	protected static const float BOY_MIN = 800;
	protected static const float BOY_MAX = 2500;			// towns this far from the farm (+1 km when hungry)
	protected static const float INJURED_MAX = 2000;
	protected static const float LEFT_SECONDS = 180;
	protected static const float DEAD_REACH = 2500;
	protected static const int MAX_SIGNS = 6;
	protected static const int MAX_DEAD = 6;
	protected static const int DAY = 86400;

	protected static RZ_LongTableReach s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected RZ_LongTable m_Table;
	protected ref RZ_LongTableReachSettings m_S;

	protected float m_fTimer;
	protected float m_fPeopleTimer;
	protected float m_fLastHour = -1;
	protected bool m_bHarvestDone;			// saved (RZ_LongTableSave.harvestDone)
	protected bool m_bHarvestTonight;
	protected int m_iMarkUnix;				// the last door mark (one a real day)
	protected IEntity m_Bell;				// the kit's bell sound, for 40 s
	protected float m_fBellLeft;
	protected string m_sInjuredName;

	// Signs
	protected vector m_vSignsFor;			// the house the junctions were found for
	protected ref array<vector> m_aJunctions = {};
	protected ref array<IEntity> m_aSigns = {};

	// The farm laid out
	protected ref array<IEntity> m_aDress = {};
	protected ref array<IEntity> m_aLanterns = {};
	protected IEntity m_Smoke;
	protected bool m_bDressed;
	protected vector m_vTable;
	protected float m_fTableYaw;

	// The host at the market
	protected vector m_vMarket;
	protected string m_sMarket;
	protected float m_fMarketLeft;
	protected IEntity m_MarketBody;

	// The boy, and the injured man
	protected IEntity m_BoyBody;
	protected int m_iBoyPlayer = -1;
	protected float m_fBoyLeft;
	protected IEntity m_InjuredBody;
	protected int m_iInjuredPlayer = -1;
	protected float m_fInjuredLeft;
	protected ref array<string> m_aMetBoy = {};
	protected ref array<string> m_aMetInjured = {};

	// The dead near the farm, taken at dawn
	protected ref array<ref RZ_ReachDead> m_aDead = {};
	protected int m_iDeadTaken;				// taken at dawn this run (the log and the test)

	//------------------------------------------------------------------------------------------------
	void RZ_LongTableReach(RZ_GameModeComponent gm, RZ_LongTable table, RZ_LongTableReachSettings settings)
	{
		m_GM = gm;
		m_Table = table;
		m_S = settings;
		if (!m_S)
			m_S = RZ_LongTableReachSettings.Cast(RZ_Util.NewWithDefaults("RZ_LongTableReachSettings"));

		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_LongTableReach Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_S && m_S.m_bOn && m_Table && m_Table.Exists();
	}

	bool IsHarvestTonight() { return m_bHarvestTonight; }
	bool HarvestDone() { return m_bHarvestDone; }
	void SetHarvestDone(bool done) { m_bHarvestDone = done; }
	int SignCount() { return m_aJunctions.Count(); }
	vector SignAt(int i) { return m_aJunctions[i]; }
	bool IsDressed() { return m_bDressed; }
	int DressCount() { return m_aDress.Count(); }
	IEntity BoyBody() { return m_BoyBody; }
	IEntity InjuredBody() { return m_InjuredBody; }
	IEntity MarketBody() { return m_MarketBody; }

	//------------------------------------------------------------------------------------------------
	//! The family's jar ("" when the reach is off or the slot is empty).
	ResourceName JarPrefab()
	{
		if (!m_S || !m_S.m_bOn)
			return "";

		return m_S.m_sJarPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! 0 (fed) to 1 (starving): how much of everything there is.
	float Hunger01()
	{
		return Math.Clamp(m_Table.GetHunger() / 100, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_LongTable.Tick, while the house exists.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
		{
			Clear();
			return;
		}

		m_fTimer += dt;
		m_fPeopleTimer += dt;
		if (m_fTimer < TICK)
			return;

		m_fTimer = 0;
		if (m_Bell)
		{
			m_fBellLeft -= TICK;
			if (m_fBellLeft <= 0)
			{
				RZ_Util.DeleteUnit(m_Bell);
				m_Bell = null;
			}
		}

		float hour = RZ_GameTime.GetHour();
		float prev = m_fLastHour;
		m_fLastHour = hour;
		bool open = m_Table.GetState() == RZ_ETableState.ACTIVE;

		if (open && prev >= 0)
			Evening(prev, hour, ids, players);

		UpdateSigns(players, open);
		UpdateDress(players, hour, open);
		UpdateMarket(players, open);
		if (m_fPeopleTimer >= PEOPLE_CHECK)
		{
			float spent = m_fPeopleTimer;
			m_fPeopleTimer = 0;
			UpdateBoy(ids, players, open, spent);
			UpdateInjured(ids, players, hour, open, spent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every 10 minutes of play (RZ_LongTable.DataTick): the host may go to market, a door mark.
	void DataTick(array<IEntity> players)
	{
		if (!IsOn() || m_Table.GetState() != RZ_ETableState.ACTIVE)
			return;

		float h = Hunger01();
		if (m_fMarketLeft <= 0 && Math.RandomFloat01() < 0.04 + 0.12 * h)
			GoToMarket();

		if (h >= 0.5)
			TryDoorMark();
	}

	//------------------------------------------------------------------------------------------------
	//! Everything spawned goes (the house moved, broke, or the reach is off).
	void Clear()
	{
		foreach (IEntity s : m_aSigns)
		{
			RZ_Util.DeleteUnit(s);
		}

		m_aSigns.Clear();
		m_aJunctions.Clear();
		m_vSignsFor = vector.Zero;
		Undress();
		EndMarket();
		BoyHome();
		InjuredHome();
	}

	// ================================================================ The evening
	//------------------------------------------------------------------------------------------------
	protected static bool Crossed(float prev, float now, float at)
	{
		if (now < prev)
			return prev < at || now >= at;

		return prev < at && now >= at;
	}

	//------------------------------------------------------------------------------------------------
	protected void Evening(float prev, float hour, array<int> ids, array<IEntity> players)
	{
		float h = Hunger01();
		int day = BroadcastDay();

		// The harvest supper: once a season, from the late afternoon of its day
		if (!m_bHarvestDone && m_S.m_iHarvestDay > 0 && day >= m_S.m_iHarvestDay && Crossed(prev, hour, HARVEST_START))
			StartHarvest(ids, players);

		if (m_bHarvestTonight && Crossed(prev, hour, HARVEST_END))
		{
			m_bHarvestTonight = false;
			m_bHarvestDone = true;
			RZ_Log.Info("Long Table: the harvest supper is over.");
		}

		// The bell: later and further the hungrier they are, and furthest at the harvest
		float bellAt = BELL_HOUR;
		if (h >= 0.7)
			bellAt = BELL_HOUR + 0.75;

		if (Crossed(prev, hour, bellAt))
			Bell(ids, players, h);

		if (Crossed(prev, hour, VOICE_HOUR))
			Voice(ids, players);

		if (Crossed(prev, hour, DAWN_HOUR))
			Dawn();
	}

	//------------------------------------------------------------------------------------------------
	protected int BroadcastDay()
	{
		if (m_GM.GetBroadcast())
			return m_GM.GetBroadcast().GetDay();

		return 12;
	}

	//------------------------------------------------------------------------------------------------
	//! The dinner bell: a handbell rung at the farm door, heard 2.5 km off (more when they're hungry,
	//! 4 km at the harvest). The kit's bell sound plays at the farm when a player is in reach.
	protected void Bell(array<int> ids, array<IEntity> players, float h)
	{
		float reach = BELL_REACH + 1000 * h;
		if (m_bHarvestTonight)
			reach = 4000;

		vector farm = m_Table.GetHousePos();
		string line = string.Format("A handbell, far off past %1. Rung slow, the way you ring for supper.", m_Table.GetTownName());
		if (m_bHarvestTonight)
			line = string.Format("The bell past %1 rings and keeps ringing. A long time. Whoever's ringing it wants everyone.", m_Table.GetTownName());

		int heard = 0;
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), farm) > reach)
				continue;

			m_GM.NotifyPlayerAs(ids[i], "A bell", line, RZ_ENoticeTag.LOW);
			heard++;
		}

		ResourceName bell;
		if (m_GM.GetTraces() && m_GM.GetTraces().GetKit())
			bell = m_GM.GetTraces().GetKit().m_sBell;

		if (heard > 0 && !bell.IsEmpty() && !m_Bell)
		{
			m_Bell = RZ_Util.SpawnPrefab(bell, RZ_Util.GroundPos(farm) + Vector(0, 2, 0));
			m_fBellLeft = 40;
		}

		RZ_Log.Info(string.Format("Long Table: the bell rang (heard by %1, reach %2 m).", heard, RZ_Util.RoundToInt(reach)));
	}

	//------------------------------------------------------------------------------------------------
	//! A voice at supper on the open channel: grace, and the last one who never arrived. Heard by
	//! everyone with a radio, every evening.
	protected void Voice(array<int> ids, array<IEntity> players)
	{
		string who = m_Table.ShortLost();
		string line = "A calm voice, an older man's, unhurried: \"For what we are about to receive.\" A pause, chairs scraping. \"Amen.\" Then the set goes off.";
		if (!who.IsEmpty())
			line = string.Format("A calm voice, an older man's, unhurried: \"For what we are about to receive. For %1, who came a long way.\" Chairs scraping. \"Amen.\" Then the set goes off.", who);

		if (m_bHarvestTonight)
			line += string.Format(" Before it does, someone else, younger: \"Harvest supper tonight. Past %1. Everyone's welcome. Everyone.\"", m_Table.GetTownName());

		int heard = 0;
		foreach (int i, IEntity p : players)
		{
			if (!RZ_Radio.HasRadio(m_GM, ids[i], p))
				continue;

			m_GM.NotifyPlayerAs(ids[i], "Open channel", line, RZ_ENoticeTag.LOW);
			heard++;
		}

		RZ_Log.Info(string.Format("Long Table: grace on the open channel (%1 heard it).", heard));
	}

	//------------------------------------------------------------------------------------------------
	//! The harvest supper: the evening everyone near is asked. The bell rings long, the voice asks
	//! everyone; at the table that night, someone who knows meat or bodies may work out what it is.
	protected void StartHarvest(array<int> ids, array<IEntity> players)
	{
		m_bHarvestTonight = true;
		vector farm = m_Table.GetHousePos();
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), farm) <= 4000)
				m_GM.NotifyPlayerAs(ids[i], "Smoke past " + m_Table.GetTownName(), "There's more smoke than usual past " + m_Table.GetTownName() + ", and the smell of roasting carries a long way on the wind. Somebody's cooking for a crowd.", RZ_ENoticeTag.LOW);
		}

		RZ_Questline.Headline(m_GM, string.Format("The farm past %1 is holding a harvest supper tonight. Everyone's asked.", m_Table.GetTownName()), false);
		RZ_Log.Info("Long Table: the harvest supper begins.");
	}

	//------------------------------------------------------------------------------------------------
	//! At the harvest supper, at the table: someone who knows meat or bodies works it out (a Doctor,
	//! a Nurse, a Hunter, a Lab Technician). "" otherwise. Called from RZ_LongTable.Eat.
	string MealClue(int playerId)
	{
		if (!IsOn() || !m_bHarvestTonight)
			return "";

		string bg = m_GM.GetProfile(playerId).m_sBackgroundId;
		string text = " Every chair is taken tonight, and they bring more. The meat is pale and sweet and falls off the bone.";
		if (bg != "doctor" && bg != "nurse" && bg != "hunter" && bg != "labtech")
			return text;

		m_Table.Notice(playerId);
		m_Table.AddSuspicion(15, "the harvest supper: someone knew the bone");
		m_Table.MarkHeard();
		m_Table.TryBeginC1();
		return text + " You know that bone. Everyone else is still eating. You put your spoon down, slowly, and nobody notices but the boy.";
	}

	//------------------------------------------------------------------------------------------------
	//! At dawn: the dead near the farm from the night are gone, with drag marks toward it (tells).
	protected void Dawn()
	{
		if (m_aDead.IsEmpty())
			return;

		vector farm = m_Table.GetHousePos();
		int taken = 0;
		foreach (RZ_ReachDead d : m_aDead)
		{
			string dir = RZ_Util.DirectionText(d.m_vPos, farm);
			m_Table.AddRoadTell(d.m_vPos, string.Format("A dark patch in the grass where someone lay, and two lines from it, heel-deep, going %1 toward the farm past %2. Whoever lay here didn't walk.", dir, m_Table.GetTownName()));
			taken++;
		}

		m_aDead.Clear();
		m_iDeadTaken += taken;
		RZ_Log.Info(string.Format("Long Table: %1 of the dead taken in the night.", taken));
	}

	//------------------------------------------------------------------------------------------------
	//! Someone died near the farm (RZ_LongTable.NoteDeath): taken in the night if they're close
	//! enough (closer than 2.5 km, further when hungry).
	void OnDeath(vector pos, string who)
	{
		if (!IsOn() || m_Table.GetState() != RZ_ETableState.ACTIVE)
			return;

		float reach = DEAD_REACH + 1500 * Hunger01();
		if (RZ_Util.DistXZ(pos, m_Table.GetHousePos()) > reach || m_aDead.Count() >= MAX_DEAD)
			return;

		RZ_ReachDead d = new RZ_ReachDead();
		d.m_vPos = pos;
		d.m_sWho = who;
		m_aDead.Insert(d);
	}

	//------------------------------------------------------------------------------------------------
	//! A plate and a fork, scratched small by the door of someone lone they've asked to supper: a
	//! house in the nearest town 2-5 km off (never within 2 km: the farm leaves no marks near it,
	//! docs/TRACES.md), once a real day.
	protected void TryDoorMark()
	{
		int now = RZ_Guidance.UnixNow();
		if (now > 0 && m_iMarkUnix > 0 && now - m_iMarkUnix < DAY)
			return;

		RZ_Traces traces = m_GM.GetTraces();
		if (!traces || !traces.GetDoors())
			return;

		vector farm = m_Table.GetHousePos();
		int best = -1;
		float bestDist = 100000;
		array<ref RZ_Settlement> towns = m_GM.GetSettlements();
		foreach (int i, RZ_Settlement s : towns)
		{
			if (i == m_Table.GetSettlement())
				continue;

			float d = RZ_Util.DistXZ(s.m_vCenter, farm);
			if (d < 2000 || d > 5000 || d >= bestDist)
				continue;

			best = i;
			bestDist = d;
		}

		if (best < 0)
			return;

		RZ_Settlement town = towns[best];
		RZ_MarkedDoor door = traces.GetDoors().Write(best, RZ_EDoorWant.HOUSE, town.m_vCenter, RZ_DoorMark.Make(RZ_EDoorSymbol.PLATE, BroadcastDay()), "the Long Table asked someone to supper");
		if (!door)
			return;

		m_iMarkUnix = now;
		RZ_Log.Info(string.Format("Long Table: a plate and a fork on a door in %1.", town.m_sName));
	}

	// ================================================================ Signs at the junctions
	//------------------------------------------------------------------------------------------------
	//! "SUPPER. ALL WELCOME." at road junctions within 2 km of the farm: more of them when hungry. A
	//! board only while a player is near; found again when the house moves.
	protected void UpdateSigns(array<IEntity> players, bool open)
	{
		vector farm = m_Table.GetHousePos();
		if (m_vSignsFor != farm)
		{
			foreach (IEntity old : m_aSigns)
			{
				RZ_Util.DeleteUnit(old);
			}

			m_aSigns.Clear();
			m_aJunctions.Clear();
			m_vSignsFor = farm;
			FindJunctions(farm, m_aJunctions);
			for (int k = 0; k < m_aJunctions.Count(); k++)
			{
				m_aSigns.Insert(null);
			}

			RZ_Log.Info(string.Format("Long Table: %1 junctions for supper signs within %2 m of the farm.", m_aJunctions.Count(), RZ_Util.RoundToInt(SIGN_RADIUS)));
		}

		int shown = 2 + RZ_Util.RoundToInt(4 * Hunger01());
		foreach (int i, vector at : m_aJunctions)
		{
			bool want = open && i < shown && !m_S.m_sSignPrefab.IsEmpty();
			float nearest = RZ_Util.NearestDistance(at, players);
			IEntity e = m_aSigns[i];
			if (e && (!want || nearest > SIGN_NEAR + 60))
			{
				RZ_Util.DeleteUnit(e);
				m_aSigns[i] = null;
				continue;
			}

			if (!e && want && nearest <= SIGN_NEAR)
			{
				vector to = farm - at;
				float yaw = Math.Atan2(to[0], to[2]) * Math.RAD2DEG;
				IEntity sign = RZ_Util.SpawnPrefab(m_S.m_sSignPrefab, at, yaw);
				if (sign && RZ_ClueComponent.Tag(sign, RZ_LongTable.TAG, IDX_SIGN + i, "Read the board"))
					m_aSigns[i] = sign;
				else
					RZ_Util.DeleteUnit(sign);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Junctions: where one road's end meets another road (within 12 m), nearest the farm first, on
	//! dry mainland ground beside the road, at most six.
	protected void FindJunctions(vector farm, notnull array<vector> result)
	{
		RoadNetworkManager roads = RZ_Perimeter.GetRoadNetwork();
		if (!roads)
			return;

		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(Vector(farm[0] - SIGN_RADIUS, -1000, farm[2] - SIGN_RADIUS), Vector(farm[0] + SIGN_RADIUS, 10000, farm[2] + SIGN_RADIUS), found);
		array<ref array<vector>> pts = {};
		foreach (BaseRoad r : found)
		{
			array<vector> p = {};
			if (r)
				r.GetPoints(p);

			pts.Insert(p);
		}

		array<vector> cand = {};
		array<float> dist = {};
		foreach (int a, array<vector> ra : pts)
		{
			if (ra.Count() < 2)
				continue;

			array<vector> ends = {ra[0], ra[ra.Count() - 1]};
			foreach (vector end : ends)
			{
				float fd = RZ_Util.DistXZ(end, farm);
				if (fd > SIGN_RADIUS || fd < 150)
					continue;

				bool meets = false;
				foreach (int b, array<vector> rb : pts)
				{
					if (b == a || meets)
						continue;

					foreach (vector q : rb)
					{
						if (RZ_Util.DistXZ(q, end) <= 12)
						{
							meets = true;
							break;
						}
					}
				}

				if (!meets)
					continue;

				bool dup = false;
				foreach (vector c : cand)
				{
					if (RZ_Util.DistXZ(c, end) < 120)
						dup = true;
				}

				if (dup)
					continue;

				// Beside the road, not on it: 5 m off toward the farm's side
				vector side = farm - end;
				side[1] = 0;
				float len = side.Length();
				vector spot = end;
				if (len > 1)
					spot = end + side * (5 / len);

				spot = RZ_Util.GroundPos(spot);
				if (RZ_Util.IsWater(spot) || !RZ_Mainland.OnMainland(spot))
					continue;

				cand.Insert(spot);
				dist.Insert(fd);
			}
		}

		// Nearest the farm first
		while (!cand.IsEmpty() && result.Count() < MAX_SIGNS)
		{
			int pick = 0;
			for (int n = 1; n < cand.Count(); n++)
			{
				if (dist[n] < dist[pick])
					pick = n;
			}

			result.Insert(cand[pick]);
			cand.Remove(pick);
			dist.Remove(pick);
		}
	}

	// ================================================================ The farm laid out
	//------------------------------------------------------------------------------------------------
	//! While the family's bodies are up: the long table outside the door set for too many, a bowl at
	//! each place, a name card at the empty chair, a shoe cabinet by the door, a cupboard of jars
	//! inside, lanterns from dusk and the chimney's smoke in the evening.
	protected void UpdateDress(array<IEntity> players, float hour, bool open)
	{
		bool want = open && m_Table.BodiesUp() && RZ_Util.NearestDistance(m_Table.GetHousePos(), players) <= DRESS_RADIUS;
		if (!want)
		{
			if (m_bDressed)
				Undress();

			return;
		}

		if (!m_bDressed)
			Dress();

		// Lanterns from dusk to dawn
		bool dark = hour >= 19 || hour < 5.5;
		if (dark && m_aLanterns.IsEmpty() && m_vTable != vector.Zero)
			Lanterns();
		else if (!dark && !m_aLanterns.IsEmpty())
			NoLanterns();

		// Smoke from the chimney, 16:00 to 22:00
		bool smoke = hour >= 16 && hour < 22 && !m_S.m_sSmokePrefab.IsEmpty();
		if (smoke && !m_Smoke)
		{
			IEntity house = FarmBuilding();
			vector top = m_Table.GetHousePos() + Vector(0, 9, 0);
			if (house)
			{
				vector mins;
				vector maxs;
				house.GetWorldBounds(mins, maxs);	// VERIFY: the building's world box tops out at its roof, so the smoke stands over the chimney
				top = Vector((mins[0] + maxs[0]) / 2, maxs[1] + 0.5, (mins[2] + maxs[2]) / 2);
			}

			m_Smoke = RZ_Util.SpawnPrefab(m_S.m_sSmokePrefab, top);
		}
		else if (!smoke && m_Smoke)
		{
			RZ_Util.DeleteUnit(m_Smoke);
			m_Smoke = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity FarmBuilding()
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(m_Table.GetHousePos(), 25, buildings, 8);
		IEntity best;
		float bestDist = 1000;
		foreach (IEntity b : buildings)
		{
			float d = RZ_Util.DistXZ(b.GetOrigin(), m_Table.GetHousePos());
			if (d < bestDist)
			{
				bestDist = d;
				best = b;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected void Dress()
	{
		m_bDressed = true;
		m_vTable = vector.Zero;
		vector farm = m_Table.GetHousePos();

		// The door: the table stands 7 m out from it, along it; the shoe cabinet beside it
		IEntity door;
		vector centre;
		vector normal;
		bool hasDoor = RZ_DoorGeometry.NearestOuterDoor(farm, 0, 40, false, door, centre, normal);
		if (!hasDoor)
		{
			centre = farm;
			normal = Vector(0, 0, 1);
		}

		normal[1] = 0;
		if (normal.Length() < 0.1)
			normal = Vector(0, 0, 1);

		normal.Normalize();
		vector along = Vector(normal[2], 0, -normal[0]);
		float yaw = Math.Atan2(along[0], along[2]) * Math.RAD2DEG;

		// The long table: a few tries out from the door until it fits its measured box
		if (!m_S.m_sTablePrefab.IsEmpty())
		{
			for (int t = 0; t < 6; t++)
			{
				vector at = RZ_Util.GroundPos(centre + normal * (6 + t * 1.5) + along * Math.RandomFloat(-3, 3));
				if (!RZ_CampLife.BoxFits(at, yaw, Vector(-1.6, 0, -1.3), Vector(1.6, 0, 1.3), false))
					continue;

				IEntity table = Put(m_S.m_sTablePrefab, at, yaw);
				if (!table)
					break;

				m_vTable = at;
				m_fTableYaw = yaw;
				SetTable(at, along, normal);
				break;
			}
		}

		if (m_vTable == vector.Zero)
			RZ_Log.Info("Long Table: no room for the long table outside the farm; the farm isn't laid out.");

		// Too many boots by the door
		if (hasDoor && !m_S.m_sShoesPrefab.IsEmpty())
		{
			vector shoes = RZ_Util.GroundPos(centre + normal * 0.6 + along * 1.4);
			if (RZ_CampLife.BoxFits(shoes, yaw, Vector(-0.5, 0, -0.25), Vector(0.5, 0, 0.25), false))
			{
				IEntity cab = Put(m_S.m_sShoesPrefab, shoes, yaw);
				if (cab)
					RZ_ClueComponent.Tag(cab, RZ_LongTable.TAG, IDX_SHOES, "Look at the boots");
			}
		}

		// A cupboard of jars inside, on the ground floor
		IEntity house = FarmBuilding();
		vector inside;
		if (house && !m_S.m_sCupboardPrefab.IsEmpty() && RZ_WorkSites.FloorOf(house, inside))
		{
			IEntity cup = Put(m_S.m_sCupboardPrefab, inside, Math.RandomFloat(0, 360));
			if (cup && !m_S.m_sJarsPropPrefab.IsEmpty())
				Put(m_S.m_sJarsPropPrefab, inside + Vector(0, 1.0, 0), Math.RandomFloat(0, 360));
		}

		RZ_Log.Info(string.Format("Long Table: the farm laid out for supper (%1 things).", m_aDress.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! Eight chairs, a bowl at each, jars down the middle, and the name card at the end chair.
	protected void SetTable(vector at, vector along, vector across)
	{
		float top = 0.82;	// TableLong_01's top (0.9 m box): VERIFY the bowls sit on it, not in it
		for (int i = 0; i < 8; i++)
		{
			int col = i - 4 * (i / 4);
			float x = -0.9 + col * 0.6;
			float side = 0.85;
			float faceYaw = Math.Atan2(-across[0], -across[2]) * Math.RAD2DEG;
			if (i >= 4)
			{
				side = -0.85;
				faceYaw = Math.Atan2(across[0], across[2]) * Math.RAD2DEG;
			}

			vector seat = RZ_Util.GroundPos(at + along * x + across * side);
			if (!m_S.m_sChairPrefab.IsEmpty())
			{
				IEntity chair = Put(m_S.m_sChairPrefab, seat, faceYaw);
				if (chair && i == 7 && !m_S.m_sCardPrefab.IsEmpty())
				{
					IEntity card = Put(m_S.m_sCardPrefab, seat + Vector(0, 0.48, 0), faceYaw);
					if (card)
						RZ_ClueComponent.Tag(card, RZ_LongTable.TAG, IDX_CARD, "Read the name card");
				}
			}

			if (!m_S.m_sBowlPrefab.IsEmpty())
				Put(m_S.m_sBowlPrefab, at + along * x + across * (side * 0.35) + Vector(0, top, 0), 0);
		}

		if (!m_S.m_sJarsPropPrefab.IsEmpty())
		{
			Put(m_S.m_sJarsPropPrefab, at + along * -0.3 + Vector(0, top, 0), 0);
			Put(m_S.m_sJarsPropPrefab, at + along * 0.3 + Vector(0, top, 0), 90);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity Put(ResourceName prefab, vector at, float yaw)
	{
		IEntity e = RZ_Util.SpawnPrefab(prefab, at, yaw);
		if (e)
			m_aDress.Insert(e);

		return e;
	}

	//------------------------------------------------------------------------------------------------
	protected void Lanterns()
	{
		ResourceName lantern;
		if (m_GM.GetTraces() && m_GM.GetTraces().GetKit())
			lantern = m_GM.GetTraces().GetKit().m_sLantern;

		if (lantern.IsEmpty())
			lantern = m_S.m_sLanternPrefab;

		if (lantern.IsEmpty())
			return;

		vector along = Vector(Math.Sin(m_fTableYaw * Math.DEG2RAD), 0, Math.Cos(m_fTableYaw * Math.DEG2RAD));
		array<float> xs = {-0.9, 0.9};
		foreach (float x : xs)
		{
			IEntity e = RZ_Util.SpawnPrefab(lantern, m_vTable + along * x + Vector(0, 0.82, 0));
			if (e)
				m_aLanterns.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void NoLanterns()
	{
		foreach (IEntity e : m_aLanterns)
		{
			RZ_Util.DeleteUnit(e);
		}

		m_aLanterns.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void Undress()
	{
		foreach (IEntity e : m_aDress)
		{
			RZ_Util.DeleteUnit(e);
		}

		m_aDress.Clear();
		NoLanterns();
		RZ_Util.DeleteUnit(m_Smoke);
		m_Smoke = null;
		m_bDressed = false;
		m_vTable = vector.Zero;
	}

	// ================================================================ The family's own looks
	//------------------------------------------------------------------------------------------------
	//! A family member's body: one of their Sunday-clothes looks, the same one each time.
	ResourceName LookFor(ResourceName fallback, int seed)
	{
		if (!m_S || !m_S.m_bOn || fallback.IsEmpty())
			return fallback;

		if (m_S.m_aFamilyLooks && !m_S.m_aFamilyLooks.IsEmpty())
			return RZ_Looks.PickFrom(fallback, m_S.m_aFamilyLooks, seed);

		// The four tools/people_looks.py writes, on the puzzle civilian: only when that's the body
		if (!fallback.Contains("RZ_Civilian.et"))
			return fallback;

		array<ResourceName> sunday = {
			"{E5AFAA7C994EA7DD}Prefabs/ReforgerZ/People/LongTable/RZ_Table_SundayBrown.et",
			"{C7A56093E82D60D5}Prefabs/ReforgerZ/People/LongTable/RZ_Table_SundayGrey.et",
			"{19482B530775221F}Prefabs/ReforgerZ/People/LongTable/RZ_Table_SundayBlack.et",
			"{9919F897707CCF9F}Prefabs/ReforgerZ/People/LongTable/RZ_Table_Collar.et"
		};
		return RZ_Looks.PickFrom(fallback, sunday, seed);
	}

	// ================================================================ The host at the market
	//------------------------------------------------------------------------------------------------
	//! The host goes to a market for an hour of play: the nearest Haven safehouse or Red Hand holding
	//! within 5 km of the farm.
	protected void GoToMarket()
	{
		if (m_S.m_sJarPrefab.IsEmpty() || !m_Table.IsAlive(RZ_ETableRole.HOST))
			return;

		vector farm = m_Table.GetHousePos();
		float best = MARKET_REACH;
		bool found = false;
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (!h)
					continue;

				float d = RZ_Util.DistXZ(h.m_vPos, farm);
				if (d < best)
				{
					best = d;
					m_vMarket = h.m_vPos;
					m_sMarket = "the safehouse in " + h.m_sName;
					found = true;
				}
			}
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			foreach (RZ_Holding hold : territory.GetHoldings())
			{
				if (!hold || hold.m_eFaction != RZ_EFaction.BANDITS)
					continue;

				float hd = RZ_Util.DistXZ(hold.m_vPos, farm);
				if (hd < best)
				{
					best = hd;
					m_vMarket = hold.m_vPos;
					m_sMarket = "the Red Hand's fence at " + hold.m_sName;
					found = true;
				}
			}
		}

		if (!found)
			return;

		m_fMarketLeft = 3600;
		RZ_Log.Info(string.Format("Long Table: the host goes to market at %1.", m_sMarket));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateMarket(array<IEntity> players, bool open)
	{
		if (m_fMarketLeft <= 0)
			return;

		m_fMarketLeft -= TICK;
		if (m_fMarketLeft <= 0 || !open || !m_Table.IsAlive(RZ_ETableRole.HOST))
		{
			EndMarket();
			return;
		}

		float nearest = RZ_Util.NearestDistance(m_vMarket, players);
		if (m_MarketBody && nearest > MARKET_NEAR + 60)
		{
			RZ_Util.DeleteUnit(m_MarketBody);
			m_MarketBody = null;
			return;
		}

		if (m_MarketBody || nearest > MARKET_NEAR)
			return;

		ResourceName prefab = LookFor(m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN), 1);
		if (prefab.IsEmpty())
			return;

		vector at = RZ_Mission.LandPointNear(m_vMarket, 8, 18);
		m_MarketBody = RZ_Util.SpawnPrefab(prefab, at, Math.RandomFloat(0, 360));
		if (!m_MarketBody)
			return;

		RZ_PuzzleObjectComponent.Tag(m_MarketBody, RZ_LongTable.TAG, IDX_MARKET, "", "Buy a jar of preserves (ammunition or medicine)|Ask where they're from");
		RZ_ClueComponent.Tag(m_MarketBody, RZ_LongTable.TAG, IDX_MARKET, "Talk to " + m_Table.NameOf(RZ_ETableRole.HOST));
	}

	//------------------------------------------------------------------------------------------------
	protected void EndMarket()
	{
		m_fMarketLeft = 0;
		RZ_Util.DeleteUnit(m_MarketBody);
		m_MarketBody = null;
	}

	// ================================================================ The boy at the well
	//------------------------------------------------------------------------------------------------
	//! In towns near the farm, the boy asks a lone traveller if they're hungry: once a session each,
	//! further out the hungrier the house.
	protected void UpdateBoy(array<int> ids, array<IEntity> players, bool open, float spent)
	{
		if (m_BoyBody)
		{
			m_fBoyLeft -= spent;
			IEntity met = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iBoyPlayer);
			if (!met || m_fBoyLeft <= 0 || RZ_Util.DistXZ(met.GetOrigin(), m_BoyBody.GetOrigin()) > 300)
				BoyHome();

			return;
		}

		if (!open || !m_Table.IsAlive(RZ_ETableRole.BOY))
			return;

		vector farm = m_Table.GetHousePos();
		float maxR = BOY_MAX + 1000 * Hunger01();
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			string ident = m_GM.GetProfile(playerId).m_sIdentity;
			if (ident.IsEmpty() || m_aMetBoy.Contains(ident) || RZ_Util.IsInVehicle(p))
				continue;

			float d = RZ_Util.DistXZ(p.GetOrigin(), farm);
			if (d < BOY_MIN || d > maxR || !InTown(p.GetOrigin()) || !Alone(i, players))
				continue;

			if (Math.RandomFloat01() > 0.15)
				continue;

			RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
			if (lanes && !lanes.TryContact(playerId, RZ_EContact.HOST))
				continue;

			ResourceName prefab = LookFor(m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN), 3);
			if (prefab.IsEmpty())
				return;

			vector at = Ahead(p, farm, 20);
			m_BoyBody = RZ_Util.SpawnPrefab(prefab, at, Math.RandomFloat(0, 360));
			if (!m_BoyBody)
				return;

			m_iBoyPlayer = playerId;
			m_fBoyLeft = LEFT_SECONDS;
			m_aMetBoy.Insert(ident);
			m_Table.Notice(playerId);
			RZ_PuzzleObjectComponent.Tag(m_BoyBody, RZ_LongTable.TAG, IDX_BOY, "", "Say you'll come to supper|Send him home");
			RZ_ClueComponent.Tag(m_BoyBody, RZ_LongTable.TAG, IDX_BOY, "Talk to the boy");
			m_GM.Say(playerId, "A boy", -1, "Are you hungry? Father sets a place for everyone. The farm past " + m_Table.GetTownName() + ". Supper's at half six.", m_BoyBody);
			RZ_Log.Info("Long Table: the boy asks a traveller to supper.");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BoyHome()
	{
		RZ_Util.DeleteUnit(m_BoyBody);
		m_BoyBody = null;
		m_iBoyPlayer = -1;
	}

	// ================================================================ The injured man at dusk
	//------------------------------------------------------------------------------------------------
	//! At dusk, on a quiet road within 2 km of the farm, one of them lies "hurt" and asks a lone
	//! player to help him home. Once a session each; helping makes you the house's guest tonight.
	protected void UpdateInjured(array<int> ids, array<IEntity> players, float hour, bool open, float spent)
	{
		if (m_InjuredBody)
		{
			m_fInjuredLeft -= spent;
			IEntity met = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iInjuredPlayer);
			if (!met || m_fInjuredLeft <= 0 || RZ_Util.DistXZ(met.GetOrigin(), m_InjuredBody.GetOrigin()) > 350)
				InjuredHome();

			return;
		}

		bool dusk = hour >= 19 && hour < 21.5;
		if (!open || !dusk)
			return;

		int role = RZ_ETableRole.SON;
		if (!m_Table.IsAlive(role))
			role = RZ_ETableRole.OTHER;

		if (!m_Table.IsAlive(role))
			return;

		vector farm = m_Table.GetHousePos();
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			string ident = m_GM.GetProfile(playerId).m_sIdentity;
			if (ident.IsEmpty() || m_aMetInjured.Contains(ident) || RZ_Util.IsInVehicle(p))
				continue;

			float d = RZ_Util.DistXZ(p.GetOrigin(), farm);
			if (d < 400 || d > INJURED_MAX || InTown(p.GetOrigin()) || !Alone(i, players))
				continue;

			if (RZ_PlaceCheck.RoadDistance(p.GetOrigin(), 60) > 40 || Math.RandomFloat01() > 0.2 + 0.2 * Hunger01())
				continue;

			ResourceName prefab = LookFor(m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN), 2);
			if (prefab.IsEmpty())
				return;

			vector at = Ahead(p, farm, 30);
			m_InjuredBody = RZ_Util.SpawnPrefab(prefab, at, Math.RandomFloat(0, 360));
			if (!m_InjuredBody)
				return;

			RZ_DutyRound.LieDown(m_InjuredBody);
			m_iInjuredPlayer = playerId;
			m_fInjuredLeft = LEFT_SECONDS * 2;
			m_aMetInjured.Insert(ident);
			m_Table.Notice(playerId);
			string name = m_Table.NameOf(role);
			m_sInjuredName = name;
			RZ_PuzzleObjectComponent.Tag(m_InjuredBody, RZ_LongTable.TAG, IDX_INJURED, "", "Help him up and walk him home|Leave him");
			RZ_ClueComponent.Tag(m_InjuredBody, RZ_LongTable.TAG, IDX_INJURED, "Talk to " + name);
			m_GM.Say(playerId, name, -1, "Over here. I've done my ankle. I'm only up the road, the farm past " + m_Table.GetTownName() + ". Help me home?", m_InjuredBody);
			RZ_Log.Info("Long Table: a man lies hurt on the road at dusk.");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void InjuredHome()
	{
		RZ_Util.DeleteUnit(m_InjuredBody);
		m_InjuredBody = null;
		m_iInjuredPlayer = -1;
	}

	// ================================================================ Helpers
	//------------------------------------------------------------------------------------------------
	protected bool InTown(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		return s && RZ_Util.DistXZ(s.m_vCenter, pos) <= s.m_fRadius + 50;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Alone(int idx, array<IEntity> players)
	{
		foreach (int j, IEntity other : players)
		{
			if (j != idx && RZ_Util.DistXZ(other.GetOrigin(), players[idx].GetOrigin()) <= 150)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A spot `metres` ahead of the player, toward the farm.
	protected static vector Ahead(IEntity p, vector farm, float metres)
	{
		vector toward = farm - p.GetOrigin();
		toward[1] = 0;
		float len = toward.Length();
		if (len < 1)
			return RZ_Mission.LandPointNear(p.GetOrigin(), 10, metres);

		return RZ_Mission.LandPointNear(p.GetOrigin() + toward * (metres / len), 0, 6);
	}

	// ================================================================ Talking
	//------------------------------------------------------------------------------------------------
	//! "Read the board", "Read the name card", "Look at the boots", or "Talk to" one of the reach's
	//! people (RZ_LongTable.HandleRead).
	void HandleRead(int playerId, int idx, IEntity owner)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!player)
			return;

		if (idx >= IDX_SIGN && idx < IDX_SIGN + MAX_SIGNS)
		{
			string dir = RZ_Util.DirectionText(player.GetOrigin(), m_Table.GetHousePos());
			m_GM.NotifyPlayerAs(playerId, "A board", string.Format("Painted by hand, neat, in white: SUPPER. ALL WELCOME. And an arrow, %1: the farm past %2.", dir, m_Table.GetTownName()), RZ_ENoticeTag.LOW);
			return;
		}

		if (idx == IDX_CARD)
		{
			string name = m_Table.ShortLost();
			if (name.IsEmpty())
				name = "a name nobody here would know";

			m_Table.Notice(playerId);
			m_GM.NotifyPlayerAs(playerId, "The name card", string.Format("A folded card at the end chair, in careful pencil: %1. The chair is pushed in. The bowl in front of it is clean.", name), RZ_ENoticeTag.LOW);
			return;
		}

		if (idx == IDX_SHOES)
		{
			m_Table.Notice(playerId);
			m_Table.SeeBoots(playerId);
			m_GM.NotifyPlayerAs(playerId, "By the door", "A cabinet, and boots on it, under it, beside it. Walking boots, work boots, a child's wellington. Too many sizes for one family.", RZ_ENoticeTag.LOW);
			return;
		}

		if (idx == IDX_MARKET && m_MarketBody)
		{
			m_GM.Say(playerId, m_Table.NameOf(RZ_ETableRole.HOST), -1, "Preserves. Father puts them up himself. A day on every lid, so you know how fresh. Cartridges or something for a cut, whatever you can spare.", m_MarketBody);
			return;
		}

		if (idx == IDX_BOY && m_BoyBody)
		{
			m_GM.Say(playerId, "The boy", -1, "Father says nobody should eat alone. There's always a bowl. Those are good boots.", m_BoyBody);
			return;
		}

		if (idx == IDX_INJURED && m_InjuredBody)
			m_GM.Say(playerId, m_sInjuredName, -1, "It's only up the road. They'll have the kettle on. Please.", m_InjuredBody);
	}

	//------------------------------------------------------------------------------------------------
	//! A choice on one of the reach's people (RZ_LongTable.HandleChoice).
	void HandleChoice(int playerId, int idx, int option, IEntity owner, IEntity user)
	{
		if (!user)
			return;

		m_Table.Notice(playerId);
		if (idx == IDX_MARKET && m_MarketBody)
		{
			string host = m_Table.NameOf(RZ_ETableRole.HOST);
			if (option == 1)
			{
				m_GM.Say(playerId, host, -1, string.Format("The farm past %1. We keep pigs. Kept.", m_Table.GetTownName()), m_MarketBody);
				return;
			}

			if (!m_Table.TakesTrade(user))
			{
				m_GM.Say(playerId, host, -1, "Cartridges, or something for a cut. Whatever you can spare.", m_MarketBody);
				return;
			}

			if (!RZ_QuartermasterHelper.SpawnNear(m_S.m_sJarPrefab, owner, user))
				return;

			m_Table.TakeTrade(user);
			m_GM.Say(playerId, host, -1, string.Format("There. A day on the lid, %1. It keeps.", RZ_NoteText.Day(BroadcastDay())), m_MarketBody);
			return;
		}

		if (idx == IDX_BOY && m_BoyBody)
		{
			if (option == 0)
				m_Table.Invite(playerId);

			if (option == 0)
				m_GM.Say(playerId, "The boy", -1, "Half six, then. Don't be late. Father worries.", m_BoyBody);
			else
				m_GM.Say(playerId, "The boy", -1, "Suit yourself.", m_BoyBody);

			BoyHome();
			return;
		}

		if (idx == IDX_INJURED && m_InjuredBody)
		{
			string son = m_sInjuredName;
			if (option == 0)
			{
				m_GM.Say(playerId, son, -1, string.Format("Better already. You're kind. Come and eat with us, the farm past %1, %2. We owe you a bowl.", m_Table.GetTownName(), RZ_Util.DirectionText(user.GetOrigin(), m_Table.GetHousePos())), m_InjuredBody);
				m_Table.Invite(playerId);
			}
			else
			{
				m_GM.Say(playerId, son, -1, "Fine. I can manage.", m_InjuredBody);
				m_Table.AddSuspicion(3, "the injured man walked off");
			}

			InjuredHome();
		}
	}

	// ================================================================ The jars
	//------------------------------------------------------------------------------------------------
	//! Loot near the farm (RZ_Search.PickHouseLootAt): within 3 km a house sometimes has one of
	//! their jars, more often the hungrier they are.
	static void AddLoot(vector pos, notnull array<ResourceName> picks)
	{
		RZ_LongTableReach r = s_Instance;
		if (!r || !r.IsOn() || r.m_S.m_sJarPrefab.IsEmpty() || r.m_Table.GetState() != RZ_ETableState.ACTIVE)
			return;

		if (RZ_Util.DistXZ(pos, r.m_Table.GetHousePos()) > 3000)
			return;

		if (Math.RandomFloat01() < 0.06 + 0.08 * r.Hunger01())
			picks.Insert(r.m_S.m_sJarPrefab);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOurs(int replyId)
	{
		return replyId == REPLY_TEST;
	}

	//------------------------------------------------------------------------------------------------
	//! At Halcyon's people, whoever carries one of the jars: "[Errand] Have the jar tested".
	void AddReplies(RZ_TalkSession s, IEntity user, notnull array<ref RZ_TalkReply> replies, int faction)
	{
		if (!user || faction != RZ_EFaction.INSTITUTE || !CarriesJar(user))
			return;

		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = REPLY_TEST;
		r.m_sLabel = "Have a jar of preserves tested";
		r.m_sTag = RZ_TalkJobs.Tag("Errand");
		r.m_bOwn = true;
		r.m_iRank = RZ_TalkReply.RANK_ON;
		replies.Insert(r);
	}

	//------------------------------------------------------------------------------------------------
	bool CarriesJar(IEntity user)
	{
		return user && RZ_ItemHelper.Has(user, "rz_tablejar");
	}

	//------------------------------------------------------------------------------------------------
	//! The test: the jar goes, the technician goes quiet, and the questline can start.
	void Pick(RZ_TalkSession s, int replyId, IEntity user)
	{
		if (!CarriesJar(user))
		{
			s.m_aNew.Insert("What jar?");
			return;
		}

		RZ_ItemHelper.Take(user, "rz_tablejar", 1);
		int playerId = s.m_iPlayer;
		s.m_aNew.Insert("They take it to the back and you hear the lid go. It's a while. When they come back they don't have the jar, and they don't sit down. \"Where did you get this?\" Not pork. Not anything you'd buy. \"Don't eat any more of it. And don't tell anyone we said so, not yet.\"");
		m_Table.Notice(playerId);
		m_Table.AddSuspicion(15, "Halcyon tested a jar");
		m_Table.MarkHeard();
		m_Table.TryBeginC1();
		RZ_Log.Info("Long Table: Halcyon tested a jar.");
	}
}
